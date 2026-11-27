#include "checkpoint.h"
#include <pspkernel.h>
#include <stdlib.h>
#include <string.h>
/* Two slots serialize writes and reads. Only the drawing thread modifies
   live game/profile state. A completed read owns its slot until released. */
enum { SLOT_FREE, SLOT_QUEUED, SLOT_ACTIVE, SLOT_READ_OK, SLOT_READ_FAILED };
typedef struct {Profile profile;Game game;Progress progress;char profile_path[128],game_path[128],progress_path[128];unsigned sequence;int state,observed,loading,discard,recovered;} Snapshot;
static Snapshot *slots;
static SceUID lock=-1,wake=-1,thread=-1;
static int stopping,errors;
static int load_slot=-1;
static CheckpointResult observed_result;
static unsigned sequence;
static volatile unsigned test_delay;
void checkpoint_test_delay(unsigned ms){test_delay=ms;}
static void acquire(void){sceKernelWaitSema(lock,1,0);}
static void release(void){sceKernelSignalSema(lock,1);}
static int worker(SceSize bytes,void *arg){
 (void)bytes;(void)arg;
 for(;;){
  acquire();int index=-1;for(int i=0;i<2;i++)if(slots[i].state==SLOT_QUEUED&&(index<0||slots[i].sequence<slots[index].sequence))index=i;
  if(index<0){int stop=stopping;release();if(stop)break;sceKernelWaitSema(wake,1,0);continue;}
  Snapshot *s=&slots[index];s->state=SLOT_ACTIVE;release();
  if(test_delay)sceKernelDelayThread(test_delay*1000);
  if(s->loading){
   int ok=game_read_staged(&s->game,s->game_path,&s->recovered);
   acquire();
   if(s->discard){s->state=SLOT_FREE;load_slot=-1;}
   else s->state=ok?SLOT_READ_OK:SLOT_READ_FAILED;
   release();
   continue;
  }
  int ok=profile_save(&s->profile,s->profile_path);
  /* Keep the prior board if a prerequisite for leaving fails. */
  if(ok&&s->observed&&s->progress_path[0])ok=progress_save(&s->progress,s->progress_path);
  if(ok&&s->game_path[0])ok=game_save(&s->game,s->game_path);
  if(ok&&!s->observed&&s->progress_path[0])ok=progress_save(&s->progress,s->progress_path);
  acquire();
  if(!ok)errors++;
  if(s->observed)observed_result=ok?CHECKPOINT_SAVED:CHECKPOINT_FAILED;
  s->state=SLOT_FREE;release();
 }return 0;
}
int checkpoint_init(void){
 if(thread>=0)return 1;slots=calloc(2,sizeof(*slots));if(!slots)return 0;
 lock=sceKernelCreateSema("pvzsave-lock",0,1,1,0);wake=sceKernelCreateSema("pvzsave-wake",0,0,1,0);
 if(lock>=0&&wake>=0)thread=sceKernelCreateThread("pvz-checkpoint",worker,0x30,16384,THREAD_ATTR_USER,0);
 if(thread<0||sceKernelStartThread(thread,0,0)<0){if(thread>=0)sceKernelDeleteThread(thread);if(lock>=0)sceKernelDeleteSema(lock);if(wake>=0)sceKernelDeleteSema(wake);thread=lock=wake=-1;free(slots);slots=0;return 0;}return 1;
}
static int queue_snapshot(const Profile *p,const char *pp,const Game *g,const char *gp,const Progress *h,const char *hp,int observed){
 if(hp&&!*hp)hp=NULL;
 if(thread<0||!p||!pp||strlen(pp)>=128||(gp&&(!g||strlen(gp)>=128))||(hp&&(!h||strlen(hp)>=128)))return -1;
 acquire();int index=-1;
 if(observed&&observed_result==CHECKPOINT_WRITING){release();return 0;}
 /* Replace only an unstarted save for the same board and profile. A board-only
    save can pick up history work, or keep history an earlier save requested. */
 for(int i=0;i<2;i++){
    Snapshot *slot=&slots[i];
    if(slot->state==SLOT_QUEUED&&!slot->loading&&!slot->observed&&
       !strcmp(slot->profile_path,pp)&&!strcmp(slot->game_path,gp?gp:"")&&
       (!strcmp(slot->progress_path,hp?hp:"")||(!observed&&(!hp||!slot->progress_path[0]))))index=i;
 }
 /* Replacing an earlier write must not let a queued read pass it. */
 if(index>=0&&load_slot>=0&&slots[load_slot].state==SLOT_QUEUED&&
    slots[index].sequence<slots[load_slot].sequence)index=-1;
 int replace=index>=0;
 if(index<0)for(int i=0;i<2;i++)if(slots[i].state==SLOT_FREE){index=i;break;}
 if(index<0){release();return 0;}
 Snapshot *s=&slots[index];s->profile=*p;if(gp)s->game=*g;
 if(hp){s->progress=*h;strcpy(s->progress_path,hp);}
 else if(!replace)s->progress_path[0]=0;
 strcpy(s->profile_path,pp);strcpy(s->game_path,gp?gp:"");s->sequence=++sequence;
 s->observed=observed;s->loading=0;s->state=SLOT_QUEUED;
 if(observed)observed_result=CHECKPOINT_WRITING;
 release();sceKernelSignalSema(wake,1);return 1;
}
int checkpoint_queue(const Profile *p,const char *pp,const Game *g,const char *gp,const Progress *h,const char *hp){
 return queue_snapshot(p,pp,g,gp,h,hp,0)>0;
}
int checkpoint_queue_observed(const Profile *p,const char *pp,const Game *g,const char *gp,const Progress *h,const char *hp){
 return queue_snapshot(p,pp,g,gp,h,hp,1);
}
CheckpointResult checkpoint_observed_result(void){
 if(thread<0)return CHECKPOINT_IDLE;
 acquire();CheckpointResult result=observed_result;release();return result;
}
int checkpoint_queue_load(const char *path){
 if(thread<0||!path||strlen(path)>=sizeof(slots[0].game_path))return -1;
 acquire();
 if(load_slot>=0){release();return 0;}
 for(int i=0;i<2;i++)if(slots[i].state==SLOT_FREE){
  Snapshot *s=&slots[i];
  load_slot=i;s->loading=1;s->discard=0;s->sequence=++sequence;
  strcpy(s->game_path,path);s->state=SLOT_QUEUED;
  release();sceKernelSignalSema(wake,1);return 1;
 }
 release();return 0;
}
const Game *checkpoint_loaded_game(int *pending,int *recovered){
 *pending=*recovered=0;
 if(thread<0)return NULL;
 acquire();
 const Game *board=NULL;
 if(load_slot>=0){
  Snapshot *s=&slots[load_slot];
  *pending=s->state==SLOT_QUEUED||s->state==SLOT_ACTIVE;
  if(s->state==SLOT_READ_OK){board=&s->game;*recovered=s->recovered;}
 }
 release();
 return board;
}
void checkpoint_release_load(void){
 if(thread<0)return;
 acquire();
 if(load_slot>=0){
  Snapshot *s=&slots[load_slot];
  if(s->state==SLOT_ACTIVE)s->discard=1;
  else{s->state=SLOT_FREE;load_slot=-1;}
 }
 release();
}
int checkpoint_busy(void){
 if(thread<0)return 0;
 acquire();
 int busy=0;
 for(int i=0;i<2;i++)busy|=slots[i].state==SLOT_QUEUED||slots[i].state==SLOT_ACTIVE;
 release();return busy;
}
int checkpoint_flush(void){while(checkpoint_busy())sceKernelDelayThread(1000);if(thread<0)return 1;acquire();int ok=!errors;release();return ok;}
int checkpoint_take_error(void){if(thread<0)return 0;acquire();int n=errors;errors=0;release();return n;}
void checkpoint_shutdown(void){if(thread<0)return;checkpoint_flush();acquire();stopping=1;release();sceKernelSignalSema(wake,1);sceKernelWaitThreadEnd(thread,0);sceKernelDeleteThread(thread);sceKernelDeleteSema(lock);sceKernelDeleteSema(wake);thread=lock=wake=-1;free(slots);slots=0;load_slot=-1;stopping=errors=0;observed_result=CHECKPOINT_IDLE;}
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
 test_delay=75;
 int observed=checkpoint_queue_observed(&p,"selftest-queue-profile.dat",&g,"selftest-queue-game.dat",NULL,NULL);
 int unrelated=checkpoint_queue(&p,"selftest-missing-dir/profile.dat",NULL,NULL,NULL,NULL);
 C("an observed save reports its own result while another write fails",
   observed>0&&unrelated&&!checkpoint_flush()&&checkpoint_observed_result()==CHECKPOINT_SAVED&&checkpoint_take_error()>0);
 test_delay=0;
 g.sun=600;
 observed=checkpoint_queue_observed(&p,"selftest-queue-profile.dat",&g,"selftest-queue-game.dat",&h,"selftest-missing-dir/progress.dat");
 C("a failed history write does not replace the observed board",
   observed>0&&!checkpoint_flush()&&checkpoint_observed_result()==CHECKPOINT_FAILED&&checkpoint_take_error()>0&&
   game_load(&loaded,"selftest-queue-game.dat")&&loaded.sun==219);
 test_delay=100;g.sun=710;
 int saved=checkpoint_queue(&p,"selftest-queue-profile.dat",&g,"selftest-queue-game.dat",NULL,NULL);
 int read=checkpoint_queue_load("selftest-queue-game.dat");
 int pending,recovered;
 checkpoint_loaded_game(&pending,&recovered);
 C("a checkpoint read queues behind an earlier save",saved&&read>0&&pending);
 g.sun=900;
 C("a new save cannot replace a write that the waiting read depends on",
   !checkpoint_queue(&p,"selftest-queue-profile.dat",&g,"selftest-queue-game.dat",NULL,NULL));
 int flushed=checkpoint_flush();
 const Game *result=checkpoint_loaded_game(&pending,&recovered);
 C("the read returns the newest complete board",flushed&&result&&!pending&&!recovered&&result->sun==710);
 checkpoint_release_load();test_delay=0;
 const char *pp="selftest-queue-profile.dat",*gp="selftest-queue-game.dat",*hp="selftest-queue-progress.dat";
 test_delay=200;
 int blocker=checkpoint_queue(&p,pp,NULL,NULL,NULL,NULL);
 sceKernelDelayThread(1000);
 g.sun=910;
 int board_only=checkpoint_queue(&p,pp,&g,gp,NULL,NULL);
 g.sun=920;h=(Progress){.played=7,.completed=3};
 int with_history=checkpoint_queue(&p,pp,&g,gp,&h,hp);
 int other_history=checkpoint_queue(&p,pp,&g,gp,&h,"selftest-distinct-progress.dat");
 g.sun=930;p.money=30;
 int newest=checkpoint_queue(&p,pp,&g,gp,NULL,NULL);
 C("a full queue combines saves for the same board with optional history",blocker&&board_only&&with_history&&newest&&!other_history);
 test_delay=0;
 C("a newer board-only save preserves the queued history and latest profile",checkpoint_flush()&&
   game_load(&loaded,gp)&&loaded.sun==930&&profile_load(&q,pp)&&q.money==30&&
   progress_load(&history,hp)&&history.played==7&&history.completed==3);
 remove("selftest-distinct-progress.dat");remove("selftest-distinct-progress.dat.bak");
 FILE *legacy=fopen("legacy08-save.dat","rb");
 if(legacy){
  fclose(legacy);
  int expected=game_load(&loaded,"legacy08-save.dat");
  read=checkpoint_queue_load("legacy08-save.dat");
  flushed=checkpoint_flush();
  result=checkpoint_loaded_game(&pending,&recovered);
  C("the worker migrates a legacy board with the same result as a direct load",
    expected&&read>0&&flushed&&result&&!pending&&!memcmp(result,&loaded,sizeof(loaded)));
  checkpoint_release_load();
 }
 for(int i=0;i<3;i++){const char *path=(const char*[]){"selftest-queue-profile.dat","selftest-queue-game.dat","selftest-queue-progress.dat"}[i];char backup[160];remove(path);snprintf(backup,sizeof(backup),"%s.bak",path);remove(backup);}
 fprintf(out,"CHECKPOINT RESULT %d/%d\n",pass,total);return pass==total;
#undef C
}
