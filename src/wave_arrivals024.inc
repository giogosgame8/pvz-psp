static int rise_kind024(Game *g,int pool){
 int choices[3]={Z_NORMAL,Z_CONE,Z_BUCKET},weights[3]={zombie_weight[Z_NORMAL],0,0};
 if(allowed_in_game(g,Z_CONE))weights[1]=zombie_weight[Z_CONE];
 if(pool&&allowed_in_game(g,Z_BUCKET))weights[2]=zombie_weight[Z_BUCKET];
 int pick=random_next(g)%(weights[0]+weights[1]+weights[2]);
 for(int n=0;n<3;n++){if(pick<weights[n])return choices[n];pick-=weights[n];}return Z_NORMAL;
}
static void wave_arrivals024(Game *g,float dt){
 if(g->polish024.wave_voice_pending){g->polish024.wave_voice_delay=fmaxf(0,g->polish024.wave_voice_delay-dt);if(!g->polish024.wave_voice_delay){g->polish024.wave_voice_pending=0;cue(GAME_SAMPLE(FX_FINALWAVE));}}
 if(g->polish024.rise_state!=1&&g->polish024.rise_state!=2)return;
 g->polish024.rise_delay=fmaxf(0,g->polish024.rise_delay-dt);if(g->polish024.rise_delay>0)return;
 g->polish024.rise_state=2;
 int terrain=game_world(g);
 if(terrain==2||terrain==3||terrain==4){
  int count=(g->level==21||g->level==22||g->level==31||g->level==32||g->level==41||g->level==42)?2:3;
  while(g->polish024.pool_arrivals<count){
   if(!zombie_slots_available(g,terrain==4?2:1))return;
   int slots[25],size=0;if(terrain==4){for(int c=4;c<9;c++)for(int r=0;r<5;r++)if(!(g->polish024.pool_used&(1u<<(r*5+c-4))))slots[size++]=r*COLS+c;}
   else for(int c=5;c<9;c++)for(int r=2;r<=3;r++)if(!(g->polish024.pool_used&(1u<<((c-5)*2+r-2))))slots[size++]=r*COLS+c;
   if(!size)break;int cell=slots[random_next(g)%size],r=cell/COLS,c=cell%COLS,kind=rise_kind024(g,1),id;
   if(terrain==4){
    int carrier=game_spawn_zombie(g,r,Z_BUNGEE,plant_x(c));if(carrier<0)return;g->zombies[carrier].x=plant_x(c);id=game_spawn_zombie(g,r,kind,plant_x(c));if(id<0){g->zombies[carrier].used=0;return;}
    g->polish024.bungee_child_id[carrier]=g->fidelity.actor_id[id];g->polish024.leader_id[id]=g->fidelity.actor_id[carrier];g->polish.action[id]=ACTION_SKY_DROP;g->polish024.altitude[id]=g->fidelity.actor_aux[carrier]*380;g->polish024.pool_used|=1u<<(r*5+c-4);
   }else{
    id=game_spawn_zombie(g,r,Z_DUCK,plant_x(c));if(id<0)return;g->polish024.pool_gear[id]=kind;g->zombies[id].armor=kind==Z_CONE?370:kind==Z_BUCKET?1100:0;
    g->polish.action[id]=ACTION_POOL_RISE;g->polish.action_time[id]=0;g->polish024.altitude[id]=-49.5f;g->polish024.pool_used|=1u<<((c-5)*2+r-2);cue(FOLEY_ZOMBIESPLASH);
   }
   g->polish024.pool_arrivals++;
  }
 }
 if(terrain==1){
  while(g->polish024.rise_cell<ROWS*COLS){int cell=g->polish024.rise_cell,r=cell/COLS,c=cell%COLS;
   if(g->graves[r][c]){if(!zombie_slots_available(g,1))return;int id=game_spawn_zombie(g,r,rise_kind024(g,0),plant_x(c));if(id<0)return;g->polish.action[id]=ACTION_POOL_RISE;g->polish.action_time[id]=0;g->polish024.altitude[id]=-36;cue(FOLEY_GRAVESTONE_RUMBLE);}
   g->polish024.rise_cell++;
  }
 }
 g->polish024.rise_state=3;
}
static int wave_arrivals_pending024(const Game *g){return g->polish024.rise_state==1||g->polish024.rise_state==2;}
