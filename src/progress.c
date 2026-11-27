#include "progress.h"
#include "record_file.h"
#include <string.h>
void progress_visit(Progress *p,int level,int won){if(level<1||level>CAMPAIGN_LEVELS)return;p->played|=UINT64_C(1)<<(level-1);if(won)p->completed|=UINT64_C(1)<<(level-1);}
int progress_played(const Progress *p,int level){return level>=1&&level<=CAMPAIGN_LEVELS&&!!(p->played&(UINT64_C(1)<<(level-1)));}
void progress_import(Progress *p,const Game *g){
    if(g->level<1||g->level>CAMPAIGN_LEVELS)return;
    p->played|=(UINT64_C(1)<<g->level)-1;p->completed|=(UINT64_C(1)<<(g->level-1))-1;
    if(g->status==WON)p->completed|=UINT64_C(1)<<(g->level-1);
}
static uint32_t progress_check(const uint32_t *r,int n){uint32_t c=0x1cedc0de;for(int i=0;i<n;i++)c=(c*31)^r[i];return c;}
static int progress_read(FILE *f,void *buffer){
 uint32_t r[7]={0};
 size_t bytes=fread(r,1,sizeof(r),f);int end=fgetc(f)==EOF&&!ferror(f);
 uint64_t played=0,completed=0;
 int ok=end&&r[0]==0x35505650;
 if(ok&&r[1]==1&&bytes==5*sizeof(*r)){ok=r[4]==(r[2]^(r[3]*31)^0x1cedc0de);played=r[2];completed=r[3];}
 else if(ok&&r[1]==2&&bytes==sizeof(r)){ok=r[6]==progress_check(r,6);played=r[2]|((uint64_t)r[3]<<32);completed=r[4]|((uint64_t)r[5]<<32);}else ok=0;
 if(ok)ok=!(played&~((UINT64_C(1)<<50)-1))&&!(completed&~played);if(ok)*(Progress *)buffer=(Progress){played,completed};return ok;
}
static int progress_valid(const char *path){Progress probe;return record_inspect(path,&probe,progress_read);}
int progress_save(const Progress *p,const char *filename){
 uint32_t record[7]={0x35505650,2,p->played,p->played>>32,p->completed,p->completed>>32,0};
 record[6]=progress_check(record,6);
 return record_replace(filename,record,sizeof(record),progress_valid);
}
int progress_load(Progress *p,const char *filename){Progress restored;if(!record_load(filename,&restored,progress_read))return 0;*p=restored;return 1;}
int progress_selftest(FILE *out){
    Progress p={0},restored={0};Game g;int pass=0,total=0;
    #define VERIFY(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"progress %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    game_start_level(&g,4,1);progress_import(&p,&g);VERIFY("old Adventure unlocks only visited levels",p.played==15&&p.completed==7&&!progress_played(&p,5));
    progress_visit(&p,4,1);VERIFY("replay win does not unlock an unvisited level",p.completed==15&&!progress_played(&p,5));
    progress_visit(&p,5,0);VERIFY("visiting next Adventure level unlocks replay",progress_played(&p,5)&&!(p.completed&16));
    int saved=progress_save(&p,"selftest-progress.dat");VERIFY("replay history persists",saved&&progress_load(&restored,"selftest-progress.dat")&&restored.played==p.played&&restored.completed==p.completed);
    progress_visit(&p,50,1);progress_visit(&p,33,1);saved=progress_save(&p,"selftest-progress.dat");VERIFY("levels above 32 survive history save",saved&&progress_load(&restored,"selftest-progress.dat")&&progress_played(&restored,50)&&progress_played(&restored,33)&&restored.completed==p.completed);
    FILE *corrupt=fopen("selftest-progress.dat","wb");if(corrupt){fputs("interrupted",corrupt);fclose(corrupt);}
    VERIFY("replay history recovers the last valid backup",corrupt&&progress_load(&restored,"selftest-progress.dat")&&restored.played==31);
    VERIFY("repair preserves the valid backup",progress_save(&p,"selftest-progress.dat")&&record_inspect("selftest-progress.dat.bak",&restored,progress_read)>0&&restored.played==31);
    uint32_t old[5]={0x35505650,1,3,1,3^31^0x1cedc0de};
    FILE *legacy=fopen("selftest-progress.dat","wb");
    int wrote=legacy&&fwrite(old,sizeof(old),1,legacy)==1;if(legacy&&fclose(legacy))wrote=0;
    VERIFY("old replay records retain compatibility",wrote&&progress_load(&restored,"selftest-progress.dat")&&restored.played==3&&restored.completed==1);
    legacy=fopen("selftest-progress.dat","ab");
    int trailing=legacy&&fputc(0,legacy)!=EOF;if(legacy&&fclose(legacy))trailing=0;
    VERIFY("partial trailing bytes cannot shadow the replay backup",trailing&&progress_load(&restored,"selftest-progress.dat")&&restored.played==31);
    remove("selftest-progress.dat");remove("selftest-progress.dat.bak");fprintf(out,"PROGRESS RESULT %d/%d\n",pass,total);
    #undef VERIFY
    return pass==total;
}
