/* Streaming effects: no file access, allocation or decoding in the callback. */
#include "foley.h"
#include "music.h"
#include <pspkernel.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
enum { VOICES=12,RING=16384,PACKED=4096,REQUESTS=64 };
typedef struct {uint32_t offset,bytes,samples,crc;} SampleInfo;
typedef struct {int sample,group,flags,step;uint32_t epoch;} Request;
typedef struct {volatile int active;int sample,group,flags,step;uint32_t serial;unsigned read,write,count,fraction;int eof;int16_t pcm[RING];} Voice;
typedef struct {uint32_t serial,block,position;unsigned offset,count;unsigned char packed[PACKED];} Decoder;
static SampleInfo samples[FX_SAMPLE_COUNT];static Voice voices[VOICES];static Decoder decoders[VOICES];
static Request requests[REQUESTS];static unsigned request_read,request_write,request_count;
static FILE *bank;static SceUID mutex=-1,worker=-1;static volatile int running,paused;
static unsigned available,io_errors,decode_errors,underruns,dropped,started,peak_voices;
static uint32_t rng=0x18f01e7,epochs[FOLEY_COUNT],last_variant[FOLEY_COUNT];
static uint64_t last_time[FOLEY_COUNT];static unsigned char loops[FOLEY_COUNT];
static uint32_t next_random(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static void lock(void){sceKernelWaitSema(mutex,1,0);}
static void unlock(void){sceKernelSignalSema(mutex,1);}
static int pick_variant(int group){
 const FoleyDefinition *d=&foley_definitions[group];int choices[10],n=0;
 for(int i=0;i<d->count;i++)if(!(d->flags&16)||d->count==1||last_variant[group]!=(uint32_t)i)choices[n++]=i;
 int i=choices[next_random()%n];last_variant[group]=i;return d->samples[i];
}
static void queue(int sample,int group,int flags,float pitch){
 if(!running||sample<0||sample>=FX_SAMPLE_COUNT)return;
 int step=(int)(32768*powf(1.059463094f,pitch));if(step<8192)step=8192;if(step>131072)step=131072;
 lock();
 if(group>=0){uint64_t now=sceKernelGetSystemTimeWide();
  if(!(flags&1)&&last_time[group]&&now-last_time[group]<100000){unlock();return;}
  if(flags&3)for(int i=0;i<VOICES;i++)if(voices[i].active&&voices[i].group==group){unlock();return;}
  if(request_count<REQUESTS){sample=pick_variant(group);last_time[group]=now;}
 }
 if(request_count<REQUESTS){requests[request_write]=(Request){sample,group,flags,step,group>=0?epochs[group]:0};request_write=(request_write+1)%REQUESTS;request_count++;}else dropped++;
 unlock();
}
void foley_play(int group){if(group<0||group>=FOLEY_COUNT||!running)return;const FoleyDefinition *d=&foley_definitions[group];int sample=d->samples[0];float pitch=d->pitch*((next_random()&65535)/65536.0f);queue(sample,group,d->flags,pitch);}
void foley_sample(int sample){queue(sample,-1,0,0);}
void foley_stop(int group){if(!running||group<0||group>=FOLEY_COUNT)return;lock();epochs[group]++;loops[group]=0;for(int i=0;i<VOICES;i++)if(voices[i].group==group){voices[i].active=0;voices[i].serial++;}unlock();}
void foley_loop(int group,int enabled){if(!running||group<0||group>=FOLEY_COUNT)return;if(enabled){if(!loops[group]){loops[group]=1;foley_play(group);}}else if(loops[group])foley_stop(group);}
void foley_pause(int value){paused=value;}
static int decode(int slot,int16_t *out){
 Voice *v=&voices[slot];Decoder *d=&decoders[slot];SampleInfo *s=&samples[v->sample];
 if(d->position>=s->samples){if(!(v->flags&1))return 0;d->block=d->position=d->offset=d->count=0;}
 if(d->offset+256>d->count){
  uint32_t at=d->block*256;unsigned bytes=s->bytes-at;if(bytes>PACKED)bytes=PACKED;
  if(bytes<256||fseek(bank,s->offset+at,SEEK_SET)||fread(d->packed,bytes,1,bank)!=1){io_errors++;return -1;}
  d->offset=0;d->count=bytes;
 }
 int count=music_decode_block(d->packed+d->offset,256,out);if(count!=505){decode_errors++;return -1;}
 d->offset+=256;d->block++;if((uint32_t)count>s->samples-d->position)count=s->samples-d->position;d->position+=count;return count;
}
static int fill(int slot){
 Voice *v=&voices[slot];uint32_t serial;unsigned free_samples;lock();serial=v->serial;free_samples=RING-v->count;int active=v->active;unlock();
 if(!active||free_samples<505)return 0;int16_t pcm[505];int count=decode(slot,pcm);
 lock();if(serial==v->serial&&v->active){
  if(count<0){v->active=0;}else if(count==0)v->eof=1;else{unsigned first=RING-v->write;if(first>(unsigned)count)first=count;memcpy(v->pcm+v->write,pcm,first*2);memcpy(v->pcm,pcm+first,(count-first)*2);v->write=(v->write+count)&(RING-1);v->count+=count;}
 }unlock();return count>0;
}
static void start_request(Request r){
 lock();if(r.group>=0&&r.epoch!=epochs[r.group]){unlock();return;}
 int slot=-1;for(int i=0;i<VOICES;i++)if(!voices[i].active){slot=i;break;}
 if(slot<0){/* UI/character cues may replace the oldest non-looping voice. */
  uint32_t oldest=UINT32_MAX;for(int i=0;i<VOICES;i++)if(!(voices[i].flags&1)&&voices[i].serial<oldest){oldest=voices[i].serial;slot=i;}
 }
 if(slot<0){dropped++;unlock();return;}
 Voice *v=&voices[slot];uint32_t serial=++started;memset(v,0,sizeof(*v));v->sample=r.sample;v->group=r.group;v->flags=r.flags;v->step=r.step;v->serial=serial;v->active=2;
 memset(&decoders[slot],0,sizeof(decoders[slot]));decoders[slot].serial=serial;
 unsigned active=0;for(int i=0;i<VOICES;i++)active+=voices[i].active!=0;if(active>peak_voices)peak_voices=active;unlock();
 /* Prime outside the callback; the worker owns all compressed data. */
 for(int i=0;i<8;i++)if(!fill(slot))break;
 lock();if(v->serial==serial&&v->active)v->active=v->count?1:0;unlock();
}
static int stream(SceSize args,void *arg){(void)args;(void)arg;
 while(running){int work=0;Request r;lock();int pending=request_count!=0;if(pending){r=requests[request_read];request_read=(request_read+1)%REQUESTS;request_count--;}unlock();
  if(pending){start_request(r);work=1;}
  for(int i=0;i<VOICES;i++){lock();int fillable=voices[i].active&&!voices[i].eof&&voices[i].count<RING-505;unlock();if(fillable)work|=fill(i);}
  sceKernelDelayThread(work?500:2000);
 }return 0;
}
void foley_mix(int16_t *stereo,unsigned frames,int volume,int music_volume){
 if(!running)return;lock();
 for(unsigned frame=0;frame<frames;frame++){
  int value=stereo[frame*2];
  for(int i=0;i<VOICES;i++){Voice *v=&voices[i];if(v->active!=1||(paused&&(v->flags&4)))continue;
   if(!v->count){if(v->eof)v->active=0;else if(frame==0)underruns++;continue;}
   int a=v->pcm[v->read],b=v->count>1?v->pcm[(v->read+1)&(RING-1)]:a;
   int sample=a+((b-a)*(int)(v->fraction>>1)>>15);value+=sample*((v->flags&8)?music_volume:volume)/100;
   v->fraction+=v->step;unsigned consumed=v->fraction>>16;v->fraction&=65535;if(consumed>v->count)consumed=v->count;v->read=(v->read+consumed)&(RING-1);v->count-=consumed;
  }
  if(value>32767)value=32767;if(value< -32768)value=-32768;stereo[frame*2]=stereo[frame*2+1]=value;
 }unlock();
}
int foley_init(void){
 bank=fopen("data/foley.pvf","rb");if(!bank)return 0;uint32_t h[4];int ok=fread(h,sizeof(h),1,bank)==1&&h[0]==0x58465650&&h[1]==1&&h[2]==FX_SAMPLE_COUNT&&h[3]==22050;
 if(ok)ok=fread(samples,sizeof(samples),1,bank)==1;long size=0;if(ok){fseek(bank,0,SEEK_END);size=ftell(bank);}
 for(int i=0;i<FX_SAMPLE_COUNT&&ok;i++){SampleInfo *s=&samples[i];ok=s->offset>=16+sizeof(samples)&&s->bytes>0&&s->bytes%256==0&&s->samples>0&&s->samples<=s->bytes/256*505&&(uint64_t)s->offset+s->bytes<=(uint64_t)size;if(ok)available++;}
 if(!ok){fclose(bank);bank=0;return 0;}
 for(int i=0;i<FOLEY_COUNT;i++)last_variant[i]=UINT32_MAX;
 mutex=sceKernelCreateSema("pvzfoley",0,1,1,0);if(mutex<0)return 0;
 running=1;worker=sceKernelCreateThread("pvzfoleyio",stream,0x18,64*1024,THREAD_ATTR_USER,0);
 if(worker<0||sceKernelStartThread(worker,0,0)<0){running=0;io_errors++;return 0;}return 1;
}
void foley_shutdown(void){running=0;if(worker>=0){sceKernelWaitThreadEnd(worker,0);sceKernelDeleteThread(worker);worker=-1;}if(bank)fclose(bank);bank=0;if(mutex>=0){sceKernelDeleteSema(mutex);mutex=-1;}}
int foley_ok(void){return running&&available==FX_SAMPLE_COUNT&&!io_errors&&!decode_errors;}
void foley_report(FILE *f){if(f){fprintf(f,"Foley: %u/%d samples, %d groups, %u requests played, peak %u/%d voices, %u I/O errors, %u decode errors, %u underruns, %u voice-limit drops; buffers %u bytes\n",available,FX_SAMPLE_COUNT,FOLEY_COUNT,started,peak_voices,VOICES,io_errors,decode_errors,underruns,dropped,(unsigned)(sizeof(voices)+sizeof(decoders)+sizeof(requests)));fflush(f);}}
int foley_selftest(FILE *f){
 int valid=FOLEY_COUNT==104&&FX_SAMPLE_COUNT==194;for(int g=0;g<FOLEY_COUNT;g++){const FoleyDefinition *d=&foley_definitions[g];valid&=d->count>0&&d->count<=10&&isfinite(d->pitch);for(int n=0;n<d->count;n++)valid&=d->samples[n]>=0&&d->samples[n]<FX_SAMPLE_COUNT;}
 int weighted=foley_definitions[FOLEY_THROW].count==4&&foley_definitions[FOLEY_THROW].samples[0]==foley_definitions[FOLEY_THROW].samples[2];
 int repeats=1;uint32_t original=rng,old=last_variant[FOLEY_CRAZY_DAVE_SHORT];for(int i=0;i<30;i++){uint32_t prior=last_variant[FOLEY_CRAZY_DAVE_SHORT];pick_variant(FOLEY_CRAZY_DAVE_SHORT);repeats&=prior!=last_variant[FOLEY_CRAZY_DAVE_SHORT];}last_variant[FOLEY_CRAZY_DAVE_SHORT]=old;rng=original;
 int bounded=sizeof(voices)+sizeof(decoders)+sizeof(requests)<512*1024;
 fprintf(f,"foley all 104 original group definitions reference the 194 owned samples: %s\nfoley weighted throw variants retain the PS3 distribution: %s\nfoley Dave variants obey the original no-repeat flag: %s\nfoley streaming buffers stay below 512 KiB: %s\nFOLEY RESULT %d/4\n",valid?"PASS":"FAIL",weighted?"PASS":"FAIL",repeats?"PASS":"FAIL",bounded?"PASS":"FAIL",valid+weighted+repeats+bounded);return valid&&weighted&&repeats&&bounded;
}
int foley_data_selftest(FILE *f){
 FILE *src=fopen("data/foley.pvf","rb");if(!src){fprintf(f,"FOLEY DATA RESULT 0/194\n");return 0;}int pass=0;unsigned char packed[4096];int16_t pcm[505];
 for(int i=0;i<FX_SAMPLE_COUNT;i++){SampleInfo *s=&samples[i];int ok=s->bytes>0&&!fseek(src,s->offset,SEEK_SET);uint32_t left=s->bytes,crc=crc32(0,0,0);
  while(left&&ok){unsigned bytes=left>sizeof(packed)?sizeof(packed):left;ok=fread(packed,bytes,1,src)==1;if(ok)crc=crc32(crc,packed,bytes);left-=bytes;}
  ok=ok&&crc==s->crc&&!fseek(src,s->offset,SEEK_SET)&&fread(packed,256,1,src)==1&&music_decode_block(packed,256,pcm)==505&&!fseek(src,s->offset+s->bytes-256,SEEK_SET)&&fread(packed,256,1,src)==1&&music_decode_block(packed,256,pcm)==505;
  pass+=ok;fprintf(f,"foley sample %d checksum and first/last decode: %s\n",i,ok?"PASS":"FAIL");
 }fclose(src);fprintf(f,"FOLEY DATA RESULT %d/%d\n",pass,FX_SAMPLE_COUNT);return pass==FX_SAMPLE_COUNT;
}

int foley_runtime_selftest(FILE *f){
 unsigned before=started;for(int n=0;n<12;n++){foley_play(FOLEY_THROW);foley_play(n%2?FOLEY_CHOMP:FOLEY_SPLAT);foley_sample(FX_BUTTONCLICK);sceKernelDelayThread(180000);}
 foley_loop(FOLEY_RAIN,1);foley_loop(FOLEY_JACKINTHEBOX,1);sceKernelDelayThread(300000);foley_pause(1);sceKernelDelayThread(50000);foley_pause(0);foley_loop(FOLEY_RAIN,0);foley_loop(FOLEY_JACKINTHEBOX,0);sceKernelDelayThread(30000);
 int ok=foley_ok()&&started>=before+30&&peak_voices>=3;foley_report(f);fprintf(f,"foley concurrent stream, variant, loop, stop and pause playback: %s\nFOLEY RUNTIME RESULT %d/1\n",ok?"PASS":"FAIL",ok);return ok;
}
