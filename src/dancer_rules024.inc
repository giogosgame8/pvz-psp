/* Each group owns its four slots through generation IDs, never proximity. */
static int dancer_slot_valid024(const Game *g,const Zombie *z,int slot){int row=z->row+(slot==0?-1:slot==1?1:0);return game_row_active(g,row)&&!game_water_row(g,row);}
static int dancer_missing024(const Game *g,int leader){
 for(int slot=0;slot<4;slot++)if(dancer_slot_valid024(g,&g->zombies[leader],slot)){
  int found=0;for(int j=0;j<MAX_ZOMBIES;j++)if(g->zombies[j].used&&g->zombies[j].death<=0&&g->zombies[j].kind==Z_BACKUP&&g->polish024.leader_id[j]==g->fidelity.actor_id[leader]&&g->polish024.dancer_slot[j]==slot&&!g->hypnotized[j])found=1;
  if(!found)return 1;
 }return 0;
}
static int dancer_summon024(Game *g,int leader){
 Zombie *z=&g->zombies[leader];
 for(int slot=0;slot<4;slot++)if(dancer_slot_valid024(g,z,slot)){
  int found=0;for(int j=0;j<MAX_ZOMBIES;j++)if(g->zombies[j].used&&g->zombies[j].death<=0&&g->zombies[j].kind==Z_BACKUP&&g->polish024.leader_id[j]==g->fidelity.actor_id[leader]&&g->polish024.dancer_slot[j]==slot&&!g->hypnotized[j])found=1;
  if(found)continue;
  int row=z->row+(slot==0?-1:slot==1?1:0);float x=z->x+(slot==2?-47.5f:slot==3?47.5f:0);
  int child=game_spawn_zombie(g,row,Z_BACKUP,x);if(child<0)return 0;
  g->polish024.leader_id[child]=g->fidelity.actor_id[leader];g->polish024.dancer_slot[child]=slot;
  g->polish024.dancer_state[child]=DANCE_RISE;g->polish024.altitude[child]=-89.28f;cue(FOLEY_DIRT_RISE);
 }return 1;
}
static int dancer_beat024(const Game *g){return ((int)(g->time*100)%460)/20;}
static int dancer_walk024(const Game *g,int i){
 if(g->zombies[i].kind!=Z_DANCER&&g->zombies[i].kind!=Z_BACKUP)return 1;
 int state=g->polish024.dancer_state[i];return state==DANCE_ENTER||state==DANCE_ORPHAN||dancer_beat024(g)<12;
}
static int dancer_step024(Game *g,int i,float dt){
 Zombie *z=&g->zombies[i];if((z->kind!=Z_DANCER&&z->kind!=Z_BACKUP)||!game_zombie_has_head(g,z)||g->hypnotized[i])return 0;
 int state=g->polish024.dancer_state[i];float step=dt*(z->slow>0?.5f:1);
 if(z->kind==Z_BACKUP&&state!=DANCE_RISE){int lead=actor_by_id024(g,g->polish024.leader_id[i]);if(lead<0||g->zombies[lead].death>0||g->hypnotized[lead])g->polish024.dancer_state[i]=state=DANCE_ORPHAN;else g->polish024.dancer_state[i]=state=DANCE_GROUP;}
 if(z->eating&&state!=DANCE_RISE)return 0;
 g->polish024.actor_clock[i]+=step;float t=g->polish024.actor_clock[i];
 if(state==DANCE_RISE){g->polish024.altitude[i]=-89.28f*(1-fminf(1,t/1.5f));if(t>=1.5f){g->polish024.altitude[i]=0;g->polish024.dancer_state[i]=DANCE_GROUP;g->polish024.actor_clock[i]=0;z->age=g->polish.pose[i]=0;}return 1;}
 if(z->kind==Z_BACKUP)return 0;
 if(state==DANCE_ENTER){if(t>=g->special[i]){g->polish024.dancer_state[i]=DANCE_POINT;g->polish024.actor_clock[i]=0;z->speed=.23f+random_unit(g)*.14f;}return 0;}
 if(state==DANCE_POINT){z->eating=0;if(t>=9/24.0f&&dancer_summon024(g,i)){g->polish024.dancer_state[i]=DANCE_HOLD;g->polish024.actor_clock[i]=0;}return 1;}
 if(state==DANCE_HOLD){z->eating=0;if(t>=2){g->polish024.dancer_state[i]=DANCE_GROUP;g->polish024.actor_clock[i]=0;z->age=g->polish.pose[i]=0;}return 1;}
 if(state==DANCE_GROUP){
  if(dancer_missing024(g,i)){g->polish024.summon_delay[i]+=dt;if(g->polish024.summon_delay[i]>=1&&dancer_beat024(g)==12&&z->x<GRID_X+700*.475f){g->polish024.dancer_state[i]=DANCE_POINT;g->polish024.actor_clock[i]=0;g->polish024.summon_delay[i]=0;return 1;}}
  else g->polish024.summon_delay[i]=0;
 }return 0;
}
