#include "audio.h"
#include "game.h"
#include "music.h"
#include "adaptive_music.h"
#include <pspkernel.h>
#include <pspaudiolib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { MUSIC_RING=16384,MUSIC_CHUNK=2048 };
static int16_t music_ring[3][MUSIC_RING];static unsigned int ring_read[3],ring_write[3],ring_count[3];
static volatile uint32_t music_position,underruns,read_errors;static int music_half,music_fade;
static int initialized;static volatile int play_music,worker_running;static SceUID lock=-1,worker=-1;
static volatile int music_volume=85,effects_volume=50;
static volatile int music_track=-1,requested_track=-1,failed_track=-1,test_read_delay;
static volatile uint32_t requested_generation,music_generation;
static int preferred_track=-1,available_tracks,track_available[MUSIC_COUNT],stream_test_active;
static unsigned int max_request_us;
static volatile unsigned int settings_requested,settings_written;
static uint32_t settings_record[5];static volatile int settings_result=1;
static const char *track_names[MUSIC_COUNT]={"CRAZY DAVE","GRASSWALK","CHOOSE YOUR SEEDS","LOONBOON","CONVEYOR","MOONGRAINS","WATERY GRAVES","RIGOR MORMIST","GRAZE THE ROOF","CEREBRAWL","BRAINIAC MANIAC","ZEN GARDEN","ZOMBIES ON YOUR LAWN"};
int audio_preferred_track(void){return preferred_track;}
void audio_set_preferred_track(int track){preferred_track=track>=0&&track<MUSIC_COUNT?track:-1;}
const char *audio_track_name(int track){return track>=0&&track<MUSIC_COUNT?track_names[track]:"AUTO - ORIGINAL CONTEXT";}
static int clamp_volume(int value){return value<0?0:value>100?100:value;}
int audio_music_volume(void){return music_volume;}
int audio_effects_volume(void){return effects_volume;}
void audio_set_volumes(int music_level,int effects_level){music_volume=clamp_volume(music_level);effects_volume=clamp_volume(effects_level);}
static int gain_sample(int sample,int volume,int boost){return sample*(volume*1024*boost/100)/1024;}
static int16_t limit_sample(int value){return value>32767?32767:value< -32768?-32768:(int16_t)value;}
static int write_settings(const char *filename,const uint32_t data[5]){
    char temp[512];snprintf(temp,sizeof(temp),"%s.tmp",filename);
    FILE *f=fopen(temp,"wb");if(!f)return 0;int ok=fwrite(data,20,1,f)==1;if(fclose(f)!=0)ok=0;
    if(ok){remove(filename);ok=rename(temp,filename)==0;}return ok;
}
int audio_save_settings(const char *filename){
    uint32_t data[5]={0x55415650,2,(uint32_t)music_volume,(uint32_t)effects_volume,(uint32_t)preferred_track};return write_settings(filename,data);
}
int audio_load_settings(const char *filename){
    uint32_t data[5];FILE *f=fopen(filename,"rb");if(!f)return 0;
    int ok=fread(data,16,1,f)==1&&data[0]==0x55415650&&(data[1]==1||data[1]==2)&&data[2]<=100&&data[3]<=100;
    data[4]=0xffffffffu;if(ok&&data[1]==2)ok=fread(&data[4],4,1,f)==1&&(data[4]==0xffffffffu||data[4]<MUSIC_COUNT);
    if(ok)ok=fgetc(f)==EOF;fclose(f);if(ok){audio_set_volumes(data[2],data[3]);audio_set_preferred_track((int)data[4]);}return ok;
}
void audio_queue_settings(void){
    if(!worker_running){settings_result=audio_save_settings("audio-settings.dat");return;}
    sceKernelWaitSema(lock,1,0);
    uint32_t data[5]={0x55415650,2,(uint32_t)music_volume,(uint32_t)effects_volume,(uint32_t)preferred_track};
    memcpy(settings_record,data,sizeof(data));settings_requested++;sceKernelSignalSema(lock,1);
}
int audio_flush_settings(void){
    uint64_t start=sceKernelGetSystemTimeWide();
    while(settings_written!=settings_requested&&worker_running){
        if(sceKernelGetSystemTimeWide()-start>2000000)return 0;sceKernelDelayThread(1000);
    }return settings_result&&settings_written==settings_requested;
}
static void save_queued_settings(void){
    uint32_t data[5];sceKernelWaitSema(lock,1,0);unsigned int generation=settings_requested;memcpy(data,settings_record,sizeof(data));sceKernelSignalSema(lock,1);
    if(generation!=settings_written){settings_result=write_settings("audio-settings.dat",data);settings_written=generation;}
}

#include "audio_dynamic.inc"
int audio_select_music(int track){
    uint64_t start=stream_test_active?sceKernelGetSystemTimeWide():0;
    if(track<0||track>=MUSIC_COUNT)return 0;
    requested_track=track; // Latest request wins; no allocation, locks or I/O here.
    if(stream_test_active){unsigned int us=sceKernelGetSystemTimeWide()-start;if(us>max_request_us)max_request_us=us;}
    return track_available[track];
}
void audio_restart_music(int track){if(track<0||track>=MUSIC_COUNT)return;requested_track=track;requested_generation++;}
float audio_music_seconds(int track){return music_track==track&&requested_track==track&&music_generation==requested_generation?music_position/22050.0f:-1;}
int audio_init(void){
    audio_load_settings("audio-settings.dat");
    dynamic_load();int effects_ready=foley_init();
    for(int i=0;i<MUSIC_COUNT;i++){char path[64];snprintf(path,sizeof(path),"data/music%d.wav",i);FILE *f=fopen(path,"rb");if(f){track_available[i]=1;available_tracks++;fclose(f);}}
    lock=sceKernelCreateSema("pvzaudio",0,1,1,0);
    if(lock<0)return 0;
    initialized=pspAudioInit()>=0;
    if(initialized){pspAudioSetVolume(0,0x6000,0x6000);pspAudioSetChannelCallback(0,mix,0);}
    worker_running=1;worker=sceKernelCreateThread("pvzmusic",stream_worker,0x18,80*1024,THREAD_ATTR_USER,0);
    if(worker<0||sceKernelStartThread(worker,0,0)<0){worker_running=0;if(worker>=0)sceKernelDeleteThread(worker);worker=-1;read_errors++;}
    audio_select_music(MUSIC_MENU);
    return initialized&&effects_ready;
}
void audio_play(int effect){
 if(!initialized||effect<0||effect>=AUDIO_EFFECTS)return;
 static const int groups[AUDIO_EFFECTS]={FOLEY_PLANT,FOLEY_THROW,FOLEY_SUN,FOLEY_CHOMP,FOLEY_CHERRYBOMB,FOLEY_GROAN,FOLEY_LAWNMOWER,FOLEY_WINMUSIC,-FX_HUGE_WAVE-1,-FX_FINALWAVE-1,-FX_SEEDLIFT-1,FOLEY_RAIN,FOLEY_THUNDER,FOLEY_SWING,FOLEY_BONK,FOLEY_PLASTIC_HIT,FOLEY_SHIELD_HIT,FOLEY_CRAZY_DAVE_SHORT,FOLEY_CRAZY_DAVE_LONG,FOLEY_CRAZY_DAVE_SCREAM,FOLEY_CRAZY_DAVE_SCREAM_2,FOLEY_SQUASH_HMM,FOLEY_THUMP};
 int id=groups[effect];if(id<0)foley_sample(-1-id);else foley_play(id);
}
void audio_music(int enabled){play_music=enabled&&!dynamic_test_paused;}
void audio_write_status(FILE *out){
    if(!out)return;
    fprintf(out,"Adaptive music: %d/5 themes, %u bursts, %u layer transitions, %u night entries; PCM buffers %u bytes\n",music_plan_count,(unsigned)burst_count,(unsigned)layer_boundaries,(unsigned)night_restarts,(unsigned)(sizeof(music_ring)+sizeof(night_prime)));
    fprintf(out,"Audio: IMA ADPCM streaming, buffer %u bytes, songs %d/%d, effects %d/%d, sliders %d/%d\n",(unsigned int)sizeof(music_ring),available_tracks,MUSIC_COUNT,foley_ok()?FX_SAMPLE_COUNT:0,FX_SAMPLE_COUNT,music_volume,effects_volume);foley_report(out);fflush(out);
}
void audio_shutdown(void){
    worker_running=0;if(worker>=0){sceKernelWaitThreadEnd(worker,0);sceKernelDeleteThread(worker);worker=-1;}
    if(initialized){pspAudioEnd();initialized=0;}
    foley_shutdown();
    if(lock>=0)sceKernelDeleteSema(lock);
}
int audio_stream_data_selftest(FILE *out){
    FILE *f=fopen("data/music-checks.bin","rb");char magic[4];uint32_t count=0,checks[MUSIC_COUNT][2];
    int ok=f&&fread(magic,4,1,f)==1&&!memcmp(magic,"PVMC",4)&&fread(&count,4,1,f)==1&&count==MUSIC_COUNT&&fread(checks,sizeof(checks),1,f)==1;
    if(f)fclose(f);int passed=0;MusicReader reader={0};int16_t decoded[4096];
    if(ok)for(int i=0;i<MUSIC_COUNT;i++){
        char path[64];snprintf(path,sizeof(path),"data/music%d.wav",i);
        int match=checks[i][0]==4096&&music_reader_open(&reader,path)&&music_reader_read(&reader,decoded,4096)==4096&&music_fingerprint(decoded,4096)==checks[i][1];
        fprintf(out,"compressed song %d matches FFmpeg decoder: %s\n",i,match?"PASS":"FAIL");passed+=match;music_reader_close(&reader);
    }
    fprintf(out,"DECODE RESULT %d/%d\n",passed,MUSIC_COUNT);int base_ok=passed==MUSIC_COUNT;
    f=fopen("data/music-layer-checks.bin","rb");uint32_t layers[9][4];
    ok=f&&fread(magic,4,1,f)==1&&!memcmp(magic,"PVLC",4)&&fread(&count,4,1,f)==1&&count==9&&fread(layers,sizeof(layers),1,f)==1&&fgetc(f)==EOF;
    if(f)fclose(f);passed=0;
    if(ok)for(int n=0;n<9;n++){
        char path[64];snprintf(path,sizeof(path),"data/music%u%s.wav",(unsigned)layers[n][0],layers[n][1]==1?"_drums":"_hats");
        int match=layers[n][0]<MUSIC_COUNT&&layers[n][1]>=1&&layers[n][1]<=2&&layers[n][2]==4096&&music_reader_open(&reader,path)&&music_reader_read(&reader,decoded,4096)==4096&&music_fingerprint(decoded,4096)==layers[n][3];
        fprintf(out,"adaptive layer %u/%u matches FFmpeg decoder: %s\n",(unsigned)layers[n][0],(unsigned)layers[n][1],match?"PASS":"FAIL");passed+=match;music_reader_close(&reader);
    }
    fprintf(out,"LAYER DECODE RESULT %d/9\n",passed);return base_ok&&passed==9&&music_plan_count==5;
}
static struct {
    int phase,target,frames,rapid_changes,old_preferred,loaded,failures,target_fps;
    float stage,seconds,max_frame,max_ready;
    unsigned int initial_errors,initial_underruns;
} stream_test;
#include "audio_dynamic_live.inc"
void audio_stream_test_begin(int target_fps){
    memset(&stream_test,0,sizeof(stream_test));stream_test.old_preferred=preferred_track;
    stream_test.initial_errors=read_errors;stream_test.initial_underruns=underruns;
    stream_test.target_fps=target_fps<=30?30:60;stream_test_active=1;max_request_us=0;test_read_delay=250000;
}
int audio_stream_test_step(FILE *out,float elapsed){
    stream_test.stage+=elapsed;
    if(!stream_test.phase){if(stream_test.stage<.3f)return 0;stream_test.phase=1;stream_test.stage=0;audio_set_preferred_track(0);return 0;}
    stream_test.frames++;stream_test.seconds+=elapsed;if(elapsed>stream_test.max_frame)stream_test.max_frame=elapsed;
    if(stream_test.phase==1){
        int loaded=music_track==stream_test.target;
        if((loaded&&stream_test.stage>=.4f)||stream_test.stage>3||failed_track==stream_test.target){
            if(loaded)stream_test.loaded|=1<<stream_test.target;else stream_test.failures++;
            if(stream_test.stage>stream_test.max_ready)stream_test.max_ready=stream_test.stage;
            stream_test.target++;stream_test.stage=0;
            if(stream_test.target==MUSIC_COUNT)stream_test.phase=2;else audio_set_preferred_track(stream_test.target);
        }
    }else if(stream_test.phase==2){
        // Faster than the simulated disk read: stale requests must be discarded.
        int target=((int)(stream_test.stage*30))%MUSIC_COUNT;
        if(preferred_track!=target){audio_set_preferred_track(target);stream_test.rapid_changes++;}
        if(stream_test.stage>2){stream_test.phase=3;stream_test.stage=0;audio_set_preferred_track(MUSIC_MENU);}
    }else if(stream_test.phase==3&&stream_test.stage>.6f&&(music_track==MUSIC_MENU||stream_test.stage>3)){
        float fps=stream_test.frames/stream_test.seconds;int passed=0,total=0;
        #define CHECK(label,test) do{int ok=(test);passed+=ok;total++;if(out)fprintf(out,"stream %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
        CHECK("all thirteen compressed songs become ready",stream_test.loaded==((1<<MUSIC_COUNT)-1)&&!stream_test.failures);
        CHECK("rapid changes settle on latest requested track",stream_test.rapid_changes>30&&music_track==MUSIC_MENU&&requested_track==MUSIC_MENU);
        CHECK("menu requests avoid blocking I/O",max_request_us<5000);
        CHECK("menu stays responsive during 250 ms storage reads",fps>stream_test.target_fps*11.0f/12&&stream_test.max_frame<(stream_test.target_fps==30?.075f:.050f));
        CHECK("music buffer avoids underflow and read errors",read_errors==stream_test.initial_errors&&underruns==stream_test.initial_underruns);
        if(out){fprintf(out,"STREAM UI: %.2f FPS, max frame %.3f ms, max request %.3f ms, %d rapid changes, injected read delay 250 ms\n",fps,stream_test.max_frame*1000,max_request_us/1000.0f,stream_test.rapid_changes);
            fprintf(out,"STREAM RESULT %d/%d\n",passed,total);fflush(out);}
        #undef CHECK
        test_read_delay=0;stream_test.phase=4;return 0;
    }else if(stream_test.phase==4&&dynamic_live_step(out,elapsed)){
        stream_test_active=0;audio_set_preferred_track(stream_test.old_preferred);return 1;
    }return 0;
}
int audio_selftest(FILE *out){
    int adaptive_ok=adaptive_music_selftest(out);
    int passed=0,total=0,old_music=music_volume,old_effects=effects_volume,old_track=preferred_track;
    #define VERIFY(label,condition) do{int ok=(condition);passed+=ok;total++;fprintf(out,"audio %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    audio_set_volumes(-10,150);VERIFY("sliders clamp to 0-100",music_volume==0&&effects_volume==100);
    VERIFY("music and effects mute independently",gain_sample(4349,0,4)==0&&gain_sample(30000,50,1)==15000);
    VERIFY("music default is louder with headroom",gain_sample(4349,85,4)>4349/3&&gain_sample(4349,100,4)==17396);
    VERIFY("loud mixed samples saturate without wrapping",limit_sample(70000)==32767&&limit_sample(-70000)==-32768);
    audio_set_volumes(65,30);int saved=audio_save_settings("selftest-audio.dat");audio_set_volumes(0,0);
    VERIFY("separate slider values persist",saved&&audio_load_settings("selftest-audio.dat")&&music_volume==65&&effects_volume==30);
    FILE *f=fopen("selftest-audio.dat","wb");if(f){uint32_t bad[4]={0x55415650,1,101,30};fwrite(bad,sizeof(bad),1,f);fclose(f);}
    VERIFY("invalid settings leave current volume intact",!audio_load_settings("selftest-audio.dat")&&music_volume==65&&effects_volume==30);
    audio_set_preferred_track(MUSIC_POOL);saved=audio_save_settings("selftest-audio.dat");audio_set_preferred_track(-1);
    VERIFY("music selection persists with independent volumes",saved&&audio_load_settings("selftest-audio.dat")&&preferred_track==MUSIC_POOL&&music_volume==65&&effects_volume==30);
    f=fopen("selftest-audio.dat","wb");if(f){uint32_t old[4]={0x55415650,1,80,45};fwrite(old,sizeof(old),1,f);fclose(f);}
    VERIFY("older volume settings migrate with automatic soundtrack",audio_load_settings("selftest-audio.dat")&&music_volume==80&&effects_volume==45&&preferred_track==-1);
    remove("selftest-audio.dat");audio_set_volumes(old_music,old_effects);audio_set_preferred_track(old_track);
    fprintf(out,"AUDIO RESULT %d/%d\n",passed,total);
    #undef VERIFY
    return passed==total&&adaptive_ok;
}
