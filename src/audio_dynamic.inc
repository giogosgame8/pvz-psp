enum { MUSIC_ORDERS=128,NIGHT_PRIME=8192 };
typedef struct {uint32_t sample,order;} MusicOrder;
typedef struct {uint32_t track,scheme,samples,loop,count,night_samples,night_loop,night_order1;MusicOrder orders[MUSIC_ORDERS];} MusicPlan;
static MusicPlan music_plans[5];static int music_plan_count;
static const MusicPlan *playing_plan;
static AdaptiveMusic battle_music;
static volatile int battle_count,battle_active;static volatile float battle_huge;
static uint32_t source_position,tick_fraction,burst_count,layer_boundaries,night_restarts;
static int wave_music_cued,night_prime_ready,night_seek_entry;
static volatile uint32_t night_seek_generation;
static int16_t night_prime[2][NIGHT_PRIME];
static int dynamic_test_active,dynamic_test_paused;
static const MusicPlan *music_plan(int track){for(int n=0;n<music_plan_count;n++)if(music_plans[n].track==(unsigned)track)return &music_plans[n];return 0;}
static int dynamic_load(void){
    FILE *f=fopen("data/music-dynamics.bin","rb");if(!f)return 0;uint32_t h[3];int ok=fread(h,sizeof(h),1,f)==1&&h[0]==0x4d415650&&h[1]==1&&h[2]==5;
    for(int n=0;n<5&&ok;n++){
        MusicPlan *p=&music_plans[n];memset(p,0,sizeof(*p));ok=fread(p,32,1,f)==1&&p->track<MUSIC_COUNT&&p->scheme>=1&&p->scheme<=2&&p->samples>0&&p->samples<22050*1200&&p->loop<p->samples&&p->count>0&&p->count<=MUSIC_ORDERS;
        if(ok)ok=fread(p->orders,p->count*sizeof(MusicOrder),1,f)==1;
        for(unsigned k=0;k<p->count&&ok;k++)ok=p->orders[k].sample<p->samples&&(!k||p->orders[k].sample>p->orders[k-1].sample);
        for(int k=0;k<n;k++)ok=ok&&music_plans[k].track!=p->track;
        if(p->scheme==2)ok=ok&&p->night_samples>NIGHT_PRIME&&p->night_samples<22050*1200&&p->night_loop<p->night_samples&&p->night_order1+NIGHT_PRIME<p->night_samples;
    }
    ok=ok&&fgetc(f)==EOF;fclose(f);music_plan_count=ok?5:0;return ok;
}
void audio_battle_state(int zombies,int active,float huge_remaining){if(!dynamic_test_active){battle_count=zombies;battle_active=!!active;battle_huge=huge_remaining;}}
static int music_order(const MusicPlan *p,uint32_t sample){
    unsigned low=0,high=p->count;while(low+1<high){unsigned mid=(low+high)/2;if(p->orders[mid].sample<=sample)low=mid;else high=mid;}return p->orders[low].order;
}
static void music_battle_tick(void){
    if(!playing_plan||!battle_active)return;
    int old=battle_music.burst,drums=battle_music.drums;
    if(battle_huge<=0)wave_music_cued=0;
    else if(!wave_music_cued&&battle_huge<=(playing_plan->scheme==2?7:4)){adaptive_music_start(&battle_music);wave_music_cued=1;}
    adaptive_music_tick(&battle_music,battle_count,music_order(playing_plan,source_position));
    if(!old&&battle_music.burst)burst_count++;
    if(drums!=battle_music.drums)layer_boundaries++;
    if(battle_music.night_jump>=0&&night_prime_ready){
        int entry=battle_music.night_jump;memcpy(music_ring[1],night_prime[entry],sizeof(night_prime[entry]));
        ring_read[1]=0;ring_write[1]=ring_count[1]=NIGHT_PRIME;night_seek_entry=entry;night_seek_generation++;night_restarts++;
    }
}
static void mix(void *buffer,unsigned int frames,void *userdata){
    (void)userdata;int16_t *output=buffer;sceKernelWaitSema(lock,1,0);
    int music_gain=music_volume*4096/100;
    for(unsigned int i=0;i<frames;i++){
        int value=0,night=playing_plan&&playing_plan->scheme==2;
        int layers=playing_plan?(night?2:3):1;
        int need_drums=!night||battle_music.drums>=2;
        int ready=ring_count[0]>=2;
        if(layers>1&&need_drums)ready&=ring_count[1]>=2;if(layers>2)ready&=ring_count[2]>=2;
        if(ready&&play_music){
            int gains[3]={playing_plan&&battle_active?battle_music.main_gain:4096,playing_plan&&battle_active?battle_music.drum_gain:0,playing_plan&&battle_active?battle_music.hat_gain:0};
            int sample=0;
            for(int layer=0;layer<layers;layer++){
                if(layer==1&&night&&!need_drums)continue;
                unsigned at=ring_read[layer],next=(at+1)&(MUSIC_RING-1);int v=music_ring[layer][at];if(music_half)v=(v+music_ring[layer][next])/2;
                sample+=v*gains[layer]/4096;
            }
            if(music_fade<2048)music_fade++;sample=sample*music_fade/2048;
            value=sample*music_gain/1024;music_half^=1;
            if(!music_half){
                for(int layer=0;layer<layers;layer++){if(layer==1&&night&&!need_drums)continue;ring_read[layer]=(ring_read[layer]+1)&(MUSIC_RING-1);ring_count[layer]--;}
                music_position++;source_position++;
                if(playing_plan&&source_position>=playing_plan->samples)source_position=playing_plan->loop;
                tick_fraction+=100;if(tick_fraction>=22050){tick_fraction-=22050;music_battle_tick();}
            }
        }else if(i==0&&play_music&&music_track>=0)underruns++;
        output[i*2]=output[i*2+1]=limit_sample(value);
    }
    sceKernelSignalSema(lock,1);foley_mix(output,frames,effects_volume,music_volume);
}
static int open_music_bundle(MusicReader readers[3],int wanted,const MusicPlan *p,int16_t prime[NIGHT_PRIME]){
    int layers=p?(p->scheme==2?2:3):1;
    for(int n=0;n<layers;n++){
        char path[64];snprintf(path,sizeof(path),"data/music%d%s.wav",wanted,n==0?"":n==1?"_drums":"_hats");
        if(!music_reader_open(&readers[n],path))return 0;
        if(p){uint32_t total=n==1&&p->scheme==2?p->night_samples:p->samples;if(readers[n].total_samples!=total)return 0;readers[n].loop_start=n==1&&p->scheme==2?p->night_loop:p->loop;}
    }
    if(test_read_delay)sceKernelDelayThread(test_read_delay);
    if(p&&p->scheme==2)for(int entry=0;entry<2;entry++){
        if(!music_reader_seek(&readers[1],entry?p->night_order1:0)||music_reader_read(&readers[1],prime,NIGHT_PRIME)!=NIGHT_PRIME)return 0;
        sceKernelWaitSema(lock,1,0);memcpy(night_prime[entry],prime,NIGHT_PRIME*sizeof(int16_t));sceKernelSignalSema(lock,1);
    }
    return 1;
}
static int fill_music_bundle(MusicReader readers[3],const MusicPlan *plan,uint32_t *handled_seek,int16_t chunk[3][MUSIC_CHUNK]){
    int night=plan&&plan->scheme==2,layers=plan?(night?2:3):1,worked=0;
    unsigned counts[3];uint32_t seek;int entry;
    sceKernelWaitSema(lock,1,0);memcpy(counts,ring_count,sizeof(counts));seek=night_seek_generation;entry=night_seek_entry;sceKernelSignalSema(lock,1);
    if(night&&seek!=*handled_seek){
        if(!music_reader_seek(&readers[1],(entry?plan->night_order1:0)+NIGHT_PRIME))return -1;*handled_seek=seek;
    }
    for(int n=0;n<layers;n++){
        if(night&&n==1&&!*handled_seek)continue;
        if(MUSIC_RING-counts[n]<MUSIC_CHUNK)continue;
        if(music_reader_read(&readers[n],chunk[n],MUSIC_CHUNK)!=MUSIC_CHUNK)return -1;
        sceKernelWaitSema(lock,1,0);
        if(!night||n!=1||seek==night_seek_generation){
            unsigned first=MUSIC_RING-ring_write[n];if(first>MUSIC_CHUNK)first=MUSIC_CHUNK;
            memcpy(music_ring[n]+ring_write[n],chunk[n],first*sizeof(int16_t));memcpy(music_ring[n],chunk[n]+first,(MUSIC_CHUNK-first)*sizeof(int16_t));
            ring_write[n]=(ring_write[n]+MUSIC_CHUNK)&(MUSIC_RING-1);ring_count[n]+=MUSIC_CHUNK;
        }
        sceKernelSignalSema(lock,1);worked=1;
    }
    return worked;
}
static int stream_worker(SceSize args,void *argp){
    (void)args;(void)argp;
    /* Keep both banks at fixed addresses: stdio's setvbuf points into each
       reader. The active bank remains readable while a candidate is opened. */
    MusicReader banks[2][3]={{{0}}};int16_t chunk[3][MUSIC_CHUNK],prime[NIGHT_PRIME];
    int active=-1,current=-1,failed=-1;uint32_t current_generation=0,failed_generation=0,handled_seek=0;const MusicPlan *plan=0;
    while(worker_running){
        if(settings_requested!=settings_written)save_queued_settings();
        int worked=0;
        if(active>=0){
            worked=fill_music_bundle(banks[active],plan,&handled_seek,chunk);
            if(worked<0){failed_track=failed=current;failed_generation=current_generation;read_errors++;for(int n=0;n<3;n++)music_reader_close(&banks[active][n]);active=-1;}
        }
        int wanted=requested_track;uint32_t generation=requested_generation;
        if((wanted!=current||generation!=current_generation||active<0)&&!(wanted==failed&&generation==failed_generation)){
            /* Fill the old song before a potentially slow storage operation.
               Rapid stale requests therefore cannot starve active playback. */
            if(active>=0&&worked>0)continue;
            int next=active==0?1:0;const MusicPlan *candidate=music_plan(wanted);
            for(int n=0;n<3;n++)music_reader_close(&banks[next][n]);
            int ok=wanted>=0&&wanted<MUSIC_COUNT&&open_music_bundle(banks[next],wanted,candidate,prime);
            int layers=candidate?(candidate->scheme==2?2:3):1,night=candidate&&candidate->scheme==2;
            if(ok)for(int n=0;n<layers;n++)if(!(night&&n==1)&&music_reader_read(&banks[next][n],chunk[n],MUSIC_CHUNK)!=MUSIC_CHUNK)ok=0;
            if(!ok){failed_track=failed=wanted;failed_generation=generation;read_errors++;for(int n=0;n<3;n++)music_reader_close(&banks[next][n]);continue;}
            if(!worker_running||wanted!=requested_track||generation!=requested_generation)continue;
            sceKernelWaitSema(lock,1,0);
            memset(ring_read,0,sizeof(ring_read));memset(ring_write,0,sizeof(ring_write));memset(ring_count,0,sizeof(ring_count));
            for(int n=0;n<layers;n++)if(!(night&&n==1)){memcpy(music_ring[n],chunk[n],sizeof(chunk[n]));ring_write[n]=ring_count[n]=MUSIC_CHUNK;}
            music_position=source_position=tick_fraction=0;music_half=music_fade=0;playing_plan=candidate;adaptive_music_reset(&battle_music,candidate?candidate->scheme:0);wave_music_cued=0;
            night_prime_ready=night;night_seek_generation=handled_seek=0;music_track=wanted;music_generation=generation;failed_track=failed=-1;
            sceKernelSignalSema(lock,1);
            if(active>=0)for(int n=0;n<3;n++)music_reader_close(&banks[active][n]);
            active=next;current=wanted;current_generation=generation;plan=candidate;worked=1;
        }
        if(!worked)sceKernelDelayThread(5000);
    }
    for(int bank=0;bank<2;bank++)for(int n=0;n<3;n++)music_reader_close(&banks[bank][n]);save_queued_settings();return 0;
}
