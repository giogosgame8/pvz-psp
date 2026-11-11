#include "adaptive_music.h"
#include <string.h>

/* PS3 music controller: 100 Hz counters, entry >=10, exit <4, 400-tick
   rise, 800-tick minimum hold, and 800/1100-tick recovery. Recovered at
   0x23c9d8 and 0x23cad0–0x23d228; source executable remains private. */
static int ramp(int value,int length){if(value<=0)return 0;if(value>=length)return MUSIC_GAIN_ONE;return value*MUSIC_GAIN_ONE/length;}
void adaptive_music_reset(AdaptiveMusic *s,int scheme){memset(s,0,sizeof(*s));s->scheme=scheme;s->main_gain=MUSIC_GAIN_ONE;s->night_jump=-1;}
void adaptive_music_start(AdaptiveMusic *s){if(s->scheme&&!s->burst){s->burst=1;s->ticks=400;}}
void adaptive_music_tick(AdaptiveMusic *s,int zombies,int phase){
    s->night_jump=-1;s->main_gain=MUSIC_GAIN_ONE;s->drum_gain=s->hat_gain=0;
    if(!s->scheme)return;
    if(s->ticks>0)s->ticks--;if(s->drum_ticks>0)s->drum_ticks--;
    if(!s->burst){if(zombies>=10)adaptive_music_start(s);}
    else if(s->burst==1){
        if(s->scheme==1){
            s->hat_gain=ramp(400-s->ticks,400);
            if(s->ticks==100){s->drums=1;s->queued_phase=phase;}
            if(!s->ticks){s->burst=2;s->ticks=800;}
        }else{
            if(!s->drums){s->drums=1;s->queued_phase=phase;s->ticks=400;}
            else if(s->drums==1)s->ticks=400;
            else{s->main_gain=ramp(s->ticks,400);if(!s->ticks){s->burst=2;s->ticks=800;}}
        }
    }else if(s->burst==2){
        s->hat_gain=MUSIC_GAIN_ONE;if(s->scheme==2)s->main_gain=0;
        if(!s->ticks&&zombies<4){s->burst=3;s->ticks=s->scheme==1?800:1100;
            s->drums=s->scheme==1?3:4;s->queued_phase=phase;s->drum_ticks=s->scheme==1?0:800;}
    }else{
        if(s->scheme==1)s->hat_gain=ramp(s->ticks,800);else s->main_gain=ramp(400-s->ticks,400);
        if(!s->ticks&&!s->drums)s->burst=0;
    }
    if(s->drums==1&&phase!=s->queued_phase){s->drums=2;if(s->scheme==2)s->night_jump=phase&1;}
    else if(s->drums==3&&phase!=s->queued_phase){s->drums=4;s->drum_ticks=50;}
    if(s->drums==2||s->drums==3)s->drum_gain=MUSIC_GAIN_ONE;
    else if(s->drums==4){s->drum_gain=ramp(s->drum_ticks,s->scheme==1?50:800);if(!s->drum_ticks)s->drums=0;}
}
int adaptive_music_selftest(FILE *out){
    int pass=0,total=0;AdaptiveMusic s;
    #define A(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"adaptive music %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    adaptive_music_reset(&s,1);for(int n=0;n<500;n++)adaptive_music_tick(&s,9,0);
    A("nine zombies retain the calm arrangement",!s.burst&&!s.drum_gain&&!s.hat_gain&&s.main_gain==4096);
    adaptive_music_tick(&s,10,0);A("ten zombies trigger a four-second rise",s.burst==1&&s.ticks==400);
    for(int n=0;n<300;n++)adaptive_music_tick(&s,10,0);A("drums queue after three seconds without interrupting the song",s.drums==1&&!s.drum_gain&&s.hat_gain==3072);
    adaptive_music_tick(&s,10,1);A("queued drums start on a musical boundary",s.drums==2&&s.drum_gain==4096);
    for(int n=0;n<99;n++)adaptive_music_tick(&s,0,1);A("the rise finishes before the minimum hold starts",s.burst==2&&s.ticks==800);
    for(int n=0;n<799;n++)adaptive_music_tick(&s,0,1);A("a quiet board cannot cancel the eight-second minimum hold",s.burst==2);
    adaptive_music_tick(&s,4,1);A("four zombies keep the intense section active",s.burst==2);
    adaptive_music_tick(&s,3,1);A("fewer than four starts the eight-second recovery",s.burst==3&&s.ticks==800&&s.drums==3);
    adaptive_music_tick(&s,0,2);for(int n=0;n<50;n++)adaptive_music_tick(&s,0,2);
    A("drums fade for half a second after the exit boundary",!s.drums&&!s.drum_gain&&s.hat_gain>0);
    for(int n=0;n<800;n++)adaptive_music_tick(&s,0,3);A("recovery returns to calm without restarting playback",!s.burst&&!s.hat_gain&&s.main_gain==4096);
    adaptive_music_reset(&s,2);adaptive_music_start(&s);adaptive_music_tick(&s,0,6);
    for(int n=0;n<1000;n++)adaptive_music_tick(&s,0,6);A("night waits for its boundary with the main arrangement audible",s.burst==1&&s.ticks==400&&s.main_gain==4096&&!s.drum_gain);
    adaptive_music_tick(&s,0,7);A("night selects the matching alternate entry",s.night_jump==1&&s.drum_gain==4096);
    for(int n=0;n<400;n++)adaptive_music_tick(&s,10,7);A("night fades the main arrangement out over four seconds",s.burst==2&&!s.main_gain);
    for(int n=0;n<800;n++)adaptive_music_tick(&s,0,8);A("night schedules its longer eleven-second recovery",s.burst==3&&s.ticks==1100);
    for(int n=0;n<800;n++)adaptive_music_tick(&s,0,8);A("night alternate finishes fading before the main arrangement returns fully",!s.drums&&s.main_gain==1024);
    for(int n=0;n<301;n++)adaptive_music_tick(&s,0,9);A("night returns to the original main arrangement",!s.burst&&s.main_gain==4096);
    adaptive_music_reset(&s,0);adaptive_music_start(&s);adaptive_music_tick(&s,96,1);A("menus and nonadaptive songs never acquire battle layers",!s.burst&&s.main_gain==4096);
    fprintf(out,"ADAPTIVE MUSIC RESULT %d/%d\n",pass,total);return pass==total;
    #undef A
}
