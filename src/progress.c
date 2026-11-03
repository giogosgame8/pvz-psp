#include "progress.h"
#include <string.h>
void progress_visit(Progress *p,int level,int won){if(level<1||level>CAMPAIGN_LEVELS)return;p->played|=UINT64_C(1)<<(level-1);if(won)p->completed|=UINT64_C(1)<<(level-1);}
int progress_played(const Progress *p,int level){return level>=1&&level<=CAMPAIGN_LEVELS&&!!(p->played&(UINT64_C(1)<<(level-1)));}
void progress_import(Progress *p,const Game *g){
    if(g->level<1||g->level>CAMPAIGN_LEVELS)return;
    p->played|=(UINT64_C(1)<<g->level)-1;p->completed|=(UINT64_C(1)<<(g->level-1))-1;
    if(g->status==WON)p->completed|=UINT64_C(1)<<(g->level-1);
}
static uint32_t progress_check(const uint32_t *r,int n){uint32_t c=0x1cedc0de;for(int i=0;i<n;i++)c=(c*31)^r[i];return c;}
int progress_save(const Progress *p,const char *filename){
 char tmp[512];snprintf(tmp,sizeof(tmp),"%s.tmp",filename);uint32_t r[7]={0x35505650,2,p->played,p->played>>32,p->completed,p->completed>>32,0};r[6]=progress_check(r,6);FILE *f=fopen(tmp,"wb");if(!f)return 0;int ok=fwrite(r,sizeof(r),1,f)==1;if(fclose(f))ok=0;if(ok){remove(filename);ok=rename(tmp,filename)==0;}return ok;
}
int progress_load(Progress *p,const char *filename){
 uint32_t r[7]={0};FILE *f=fopen(filename,"rb");if(!f)return 0;size_t n=fread(r,4,7,f);int end=fgetc(f)==EOF;fclose(f);uint64_t played=0,completed=0;
 int ok=end&&r[0]==0x35505650;
 if(ok&&r[1]==1&&n==5){ok=r[4]==(r[2]^(r[3]*31)^0x1cedc0de);played=r[2];completed=r[3];}
 else if(ok&&r[1]==2&&n==7){ok=r[6]==progress_check(r,6);played=r[2]|((uint64_t)r[3]<<32);completed=r[4]|((uint64_t)r[5]<<32);}else ok=0;
 if(ok)ok=!(played&~((UINT64_C(1)<<50)-1))&&!(completed&~played);if(ok)*p=(Progress){played,completed};return ok;
}
int progress_selftest(FILE *out){
    Progress p={0},restored={0};Game g;int pass=0,total=0;
    #define VERIFY(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"progress %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    game_start_level(&g,4,1);progress_import(&p,&g);VERIFY("old Adventure unlocks only visited levels",p.played==15&&p.completed==7&&!progress_played(&p,5));
    progress_visit(&p,4,1);VERIFY("replay win does not unlock an unvisited level",p.completed==15&&!progress_played(&p,5));
    progress_visit(&p,5,0);VERIFY("visiting next Adventure level unlocks replay",progress_played(&p,5)&&!(p.completed&16));
    int saved=progress_save(&p,"selftest-progress.dat");VERIFY("replay history persists",saved&&progress_load(&restored,"selftest-progress.dat")&&restored.played==p.played&&restored.completed==p.completed);
    progress_visit(&p,50,1);progress_visit(&p,33,1);saved=progress_save(&p,"selftest-progress.dat");VERIFY("levels above 32 survive history save",saved&&progress_load(&restored,"selftest-progress.dat")&&progress_played(&restored,50)&&progress_played(&restored,33)&&restored.completed==p.completed);
    remove("selftest-progress.dat");fprintf(out,"PROGRESS RESULT %d/%d\n",pass,total);
    #undef VERIFY
    return pass==total;
}
