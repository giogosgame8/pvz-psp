#include "checkpoint.h"
#include <pspkernel.h>
#include <stdlib.h>
#include <string.h>
/* Two immutable snapshots: one can be writing while the newest waits.
   Only the drawing thread modifies live game/profile state. */
typedef struct {Profile profile;Game game;Progress progress;char profile_path[128],game_path[128],progress_path[128];unsigned sequence;int state;} Snapshot;
static Snapshot *slots;
static SceUID lock=-1,wake=-1,thread=-1;
static int stopping,errors;
static unsigned sequence;
static volatile unsigned test_delay;
void checkpoint_test_delay(unsigned ms){test_delay=ms;}
static void acquire(void){sceKernelWaitSema(lock,1,0);}
static void release(void){sceKernelSignalSema(lock,1);}
static int worker(SceSize bytes,void *arg){
 (void)bytes;(void)arg;
 for(;;){
  acquire();int index=-1;for(int i=0;i<2;i++)if(slots[i].state==1&&(index<0||slots[i].sequence<slots[index].sequence))index=i;
  if(index<0){int stop=stopping;release();if(stop)break;sceKernelWaitSema(wake,1,0);continue;}
  Snapshot *s=&slots[index];s->state=2;release();
  if(test_delay)sceKernelDelayThread(test_delay*1000);
  int ok=profile_save(&s->profile,s->profile_path);
  if(ok&&s->game_path[0])ok=game_save(&s->game,s->game_path);
  if(ok&&s->progress_path[0])ok=progress_save(&s->progress,s->progress_path);
  acquire();if(!ok)errors++;s->state=0;release();
 }return 0;
}
int checkpoint_init(void){
 if(thread>=0)return 1;slots=calloc(2,sizeof(*slots));if(!slots)return 0;
 lock=sceKernelCreateSema("pvzsave-lock",0,1,1,0);wake=sceKernelCreateSema("pvzsave-wake",0,0,1,0);
 if(lock>=0&&wake>=0)thread=sceKernelCreateThread("pvz-checkpoint",worker,0x30,16384,THREAD_ATTR_USER,0);
 if(thread<0||sceKernelStartThread(thread,0,0)<0){if(thread>=0)sceKernelDeleteThread(thread);if(lock>=0)sceKernelDeleteSema(lock);if(wake>=0)sceKernelDeleteSema(wake);thread=lock=wake=-1;free(slots);slots=0;return 0;}return 1;
}
int checkpoint_queue(const Profile *p,const char *pp,const Game *g,const char *gp,const Progress *h,const char *hp){
 if(thread<0||!p||!pp||strlen(pp)>=128||(gp&&(!g||strlen(gp)>=128))||(hp&&(!h||strlen(hp)>=128)))return 0;
 acquire();int index=-1;
 /* Replace only an unstarted snapshot for the same files. Never discard
    a queued save for another mode/profile. Callers serialize such changes. */
 for(int i=0;i<2;i++)if(slots[i].state==1&&!strcmp(slots[i].profile_path,pp)&&!strcmp(slots[i].game_path,gp?gp:"")&&!strcmp(slots[i].progress_path,hp?hp:""))index=i;
 if(index<0)for(int i=0;i<2;i++)if(!slots[i].state){index=i;break;}
 if(index<0){release();return 0;}
 Snapshot *s=&slots[index];s->profile=*p;if(gp)s->game=*g;if(hp)s->progress=*h;
 strcpy(s->profile_path,pp);strcpy(s->game_path,gp?gp:"");strcpy(s->progress_path,hp?hp:"");s->sequence=++sequence;s->state=1;release();sceKernelSignalSema(wake,1);return 1;
}
int checkpoint_busy(void){if(thread<0)return 0;acquire();int busy=slots[0].state||slots[1].state;release();return busy;}
int checkpoint_flush(void){while(checkpoint_busy())sceKernelDelayThread(1000);if(thread<0)return 1;acquire();int ok=!errors;release();return ok;}
int checkpoint_take_error(void){if(thread<0)return 0;acquire();int n=errors;errors=0;release();return n;}
void checkpoint_shutdown(void){if(thread<0)return;checkpoint_flush();acquire();stopping=1;release();sceKernelSignalSema(wake,1);sceKernelWaitThreadEnd(thread,0);sceKernelDeleteThread(thread);sceKernelDeleteSema(lock);sceKernelDeleteSema(wake);thread=lock=wake=-1;free(slots);slots=0;stopping=errors=0;}
int checkpoint_selftest(FILE *out){
 int pass=0,total=0;Profile p,q;Game g,loaded;Progress h={1,1},history={0};profile_init(&p);game_start_level(&g,1,242);g.sun=125;
#define C(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"checkpoint %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 test_delay=150;uint64_t start=sceKernelGetSystemTimeWide();int queued=checkpoint_queue(&p,"selftest-queue-profile.dat",&g,"selftest-queue-game.dat",&h,"selftest-queue-progress.dat");unsigned us=sceKernelGetSystemTimeWide()-start;
 fprintf(out,"CHECKPOINT ENQUEUE: %u us with 150 ms injected storage delay\n",us);
 C("slow storage cannot block the enqueue call",queued&&us<30000&&checkpoint_busy());g.sun=900;p.money=20;
 C("immutable snapshot survives live state changes",checkpoint_flush()&&game_load(&loaded,"selftest-queue-game.dat")&&loaded.sun==125&&profile_load(&q,"selftest-queue-profile.dat")&&q.money==0&&progress_load(&history,"selftest-queue-progress.dat")&&history.completed==1);
 int all=1;for(int n=0;n<20;n++){g.sun=200+n;all&=checkpoint_queue(&p,"selftest-queue-profile.dat",&g,"selftest-queue-game.dat",&h,"selftest-queue-progress.dat");}
 C("rapid updates finish with the newest complete snapshot",all&&checkpoint_flush()&&game_load(&loaded,"selftest-queue-game.dat")&&loaded.sun==219);
 C("invalid target reports failure without corrupting the last save",checkpoint_queue(&p,"selftest-missing-dir/profile.dat",&g,"selftest-queue-game.dat",0,0)&&!checkpoint_flush()&&checkpoint_take_error()>0&&game_load(&loaded,"selftest-queue-game.dat")&&loaded.sun==219);
 test_delay=0;C("a later checkpoint can recover from an I/O failure",checkpoint_queue(&p,"selftest-queue-profile.dat",&g,"selftest-queue-game.dat",0,0)&&checkpoint_flush());
 for(int i=0;i<3;i++){const char *path=(const char*[]){"selftest-queue-profile.dat","selftest-queue-game.dat","selftest-queue-progress.dat"}[i];char backup[160];remove(path);snprintf(backup,sizeof(backup),"%s.bak",path);remove(backup);}
 fprintf(out,"CHECKPOINT RESULT %d/%d\n",pass,total);return pass==total;
#undef C
}
