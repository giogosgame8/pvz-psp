/* A separate preference keeps existing health-bar and gameplay saves compatible. */
#ifdef TEST_PSP1000_60FPS
static int psp1000_fps=60;
#else
static int psp1000_fps=30;
#endif
static int performance_dirty;
static PerformanceProfile performance;
static const char *performance_path="performance.dat";
static FILE *performance_log;
static uint32_t performance_check(const uint32_t *data){uint32_t value=2166136261u;for(int i=0;i<3;i++)for(int b=0;b<4;b++)value=(value^((data[i]>>(b*8))&255))*16777619u;return value;}
static int performance_load(void){
 uint32_t data[4];FILE *f=fopen(performance_path,"rb");if(!f)return 0;
 int ok=fread(data,sizeof(data),1,f)==1&&data[0]==0x46505650&&data[1]==1&&(data[2]==30||data[2]==60)&&data[3]==performance_check(data)&&fgetc(f)==EOF;fclose(f);
 if(ok)psp1000_fps=data[2];return ok;
}
static int performance_save(void){
 uint32_t data[4]={0x46505650,1,psp1000_fps,0};data[3]=performance_check(data);char temporary[256];snprintf(temporary,sizeof(temporary),"%s.tmp",performance_path);
 FILE *f=fopen(temporary,"wb");if(!f)return 0;int ok=fwrite(data,sizeof(data),1,f)==1;if(fclose(f))ok=0;if(ok){remove(performance_path);ok=rename(temporary,performance_path)==0;}if(ok)performance_dirty=0;return ok;
}
static int performance_option(void){
 #ifdef LOW_MEMORY
 return 0;
 #else
 return hardware.known&&hardware.model==0;
 #endif
}
static int options_count(void){return performance_option()?9:8;}
static int options_back(void){return options_count()-1;}
static void performance_apply(void){performance=platform_frame_preference(&hardware,psp1000_fps);render_set_fps(performance.target_fps);}
static void performance_select(int fps){
 if(!performance_option()||(fps!=30&&fps!=60)||psp1000_fps==fps)return;
 psp1000_fps=fps;performance_dirty=1;performance_apply();performance_save();
 if(performance_log){fprintf(performance_log,"Frame rate changed: %d FPS; smaller PSP-1000 caches retained\n",performance.target_fps);fflush(performance_log);}
}
