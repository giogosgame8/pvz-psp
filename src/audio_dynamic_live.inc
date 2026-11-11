/* Opt-in diagnostic: actual callback/worker under the normal display clock. */
static struct {
    int phase,index,saw_queue,saw_on,pause_phase,pause_ok,passed;
    float seconds,pause_seconds;uint32_t generation,pause_position,errors,underflows;
} dynamic_test;
static int dynamic_live_step(FILE *out,float elapsed){
    static const int tracks[5]={MUSIC_DAY,MUSIC_NIGHT,MUSIC_POOL,MUSIC_FOG,MUSIC_ROOF};
    if(!dynamic_test_active){memset(&dynamic_test,0,sizeof(dynamic_test));dynamic_test_active=1;dynamic_test.errors=read_errors;dynamic_test.underflows=underruns;battle_active=0;battle_huge=0;}
    if(!dynamic_test.phase){
        int track=tracks[dynamic_test.index];audio_set_preferred_track(track);audio_restart_music(track);
        dynamic_test.generation=requested_generation;dynamic_test.phase=1;dynamic_test.seconds=0;dynamic_test.saw_queue=dynamic_test.saw_on=dynamic_test.pause_phase=dynamic_test.pause_ok=0;
    }
    dynamic_test.seconds+=elapsed;
    sceKernelWaitSema(lock,1,0);AdaptiveMusic state=battle_music;uint32_t position=music_position;
    int loaded=music_track==tracks[dynamic_test.index]&&music_generation==dynamic_test.generation&&playing_plan;
    sceKernelSignalSema(lock,1);
    if(dynamic_test.phase==1&&loaded){dynamic_test.phase=2;dynamic_test.seconds=0;battle_count=10;battle_active=1;}
    if(dynamic_test.phase==2){
        if(state.drums==1)dynamic_test.saw_queue=1;
        if(!dynamic_test.pause_phase&&dynamic_test.seconds>1){dynamic_test_paused=1;dynamic_test.pause_phase=1;dynamic_test.pause_seconds=0;}
        if(dynamic_test.pause_phase==1){
            dynamic_test.pause_seconds+=elapsed;
            if(dynamic_test.pause_seconds>.08f){dynamic_test.pause_position=position;dynamic_test.pause_phase=2;}
        }else if(dynamic_test.pause_phase==2){
            dynamic_test.pause_seconds+=elapsed;
            if(dynamic_test.pause_seconds>.35f){dynamic_test.pause_ok=position==dynamic_test.pause_position;dynamic_test_paused=0;dynamic_test.pause_phase=3;}
        }
        if(state.burst==2&&state.drums==2&&state.drum_gain==4096&&(state.scheme==2?!state.main_gain:state.hat_gain==4096)){
            dynamic_test.saw_on=1;battle_count=0;dynamic_test.phase=3;
        }
    }
    int finished=dynamic_test.phase==3&&!state.burst&&!state.drums&&state.main_gain==4096&&!state.hat_gain;
    if(finished||dynamic_test.seconds>40||failed_track==tracks[dynamic_test.index]){
        int ok=finished&&dynamic_test.saw_queue&&dynamic_test.saw_on&&dynamic_test.pause_ok;
        fprintf(out,"live adaptive track %d rises, enters layers, holds, recovers and pauses without advancing: %s (%.2fs)\n",tracks[dynamic_test.index],ok?"PASS":"FAIL",dynamic_test.seconds);fflush(out);
        dynamic_test.passed+=ok;dynamic_test_paused=0;battle_active=0;
        if(++dynamic_test.index==5){
            int clean=read_errors==dynamic_test.errors&&underruns==dynamic_test.underflows;
            fprintf(out,"live adaptive streaming has no read errors or underflows: %s\nLIVE ADAPTIVE RESULT %d/6\n",clean?"PASS":"FAIL",dynamic_test.passed+clean);fflush(out);
            dynamic_test_active=0;return 1;
        }
        dynamic_test.phase=0;
    }
    return 0;
}
