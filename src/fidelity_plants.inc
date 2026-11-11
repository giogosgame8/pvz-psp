static int ground_enemy(const Game *g,int i,int row,float left,float right){
 const Zombie *z=&g->zombies[i];return z->used&&z->death<=0&&!g->hypnotized[i]&&z->row==row&&z->x>=left&&z->x<=right&&!bungee_away(g,z)&&!zombie_airborne(g,z)&&!(z->kind==Z_DIGGER&&!z->vaulted);
}
static int chomper_target(const Game *g,int r,float x){
 int result=-1;float best=10000;for(int i=0;i<MAX_ZOMBIES;i++)if(ground_enemy(g,i,r,x-8,x+42)&&fabsf(g->zombies[i].x-x)<best){result=i;best=fabsf(g->zombies[i].x-x);}return result;
}
static int pult_target(const Game *g,int r,float x){
 int result=-1;float best=10000;for(int i=0;i<MAX_ZOMBIES;i++)if(ground_enemy(g,i,r,x,490)&&g->zombies[i].vault<=0&&g->zombies[i].x<best){result=i;best=g->zombies[i].x;}return result;
}
static int fidelity_plant_step(Game *g,int r,int c,float dt){
 Plant *p=&g->plants[r][c];int k=p->kind,*phase=&g->burst_left[r][c];float *clock=&g->burst[r][c],x=plant_x(c);
 if(k==CHOMPER){
  if(!*phase&&p->digest>0){*phase=2;*clock=0;} /* Existing digest saves. */
  if(!*phase){if(chomper_target(g,r,x)>=0){*phase=1;*clock=0;}return 1;}
  float old=*clock;*clock+=dt;
  if(*phase==1){
   if(old<.7f&&*clock>=.7f){
    int id=chomper_target(g,r,x);cue(FOLEY_BIGCHOMP);
    if(id<0||((g->zombies[id].vault>0||g->zombies[id].kind==Z_POGO&&!g->fidelity.equipment_lost[id])&&g->frozen[id]<=0)){*phase=4;}
    else if(g->zombies[id].kind==Z_GARG||g->zombies[id].kind==Z_BOSS){hurt_zombie(g,&g->zombies[id],40,0);*phase=4;cue(FOLEY_SPLAT);}
    else{kill_zombie(g,&g->zombies[id]);g->zombies[id].used=0;*phase=5;p->digest=0;}
   }
   if(*clock>=24/24.0f){*phase=*phase==5?2:0;if(*phase==2)p->digest=40;*clock=0;}
  }else if(*phase==5){if(*clock>=1){*phase=2;*clock=0;p->digest=40;}}else if(*phase==4){if(*clock>=1){*phase=0;*clock=0;}}
  else if(*phase==2){if(p->digest<=0){*phase=3;*clock=0;}}
  else if(*phase==3&&*clock>=27/12.0f){*phase=0;*clock=0;}
  return 1;
 }
 if(k==CABBAGE||k==KERNEL||k==MELON||k==WINTER){
  if(!*phase){if(p->attack<=0&&(pult_target(g,r,x)>=0||g->mode==MODE_BOSS&&boss_exposed(g))){
    *phase=1;*clock=0;g->burst_target[r][c]=k==CABBAGE?4:k==MELON?7:k==WINTER?12:random_next(g)%4?5:6;p->attack=2.85f;
  }return 1;}
  float before=*clock;*clock+=dt;float launch=k==CABBAGE?.31f:k==KERNEL?.29f:.35f;
  if(*phase==1&&before<launch&&*clock>=launch){int target=pult_target(g,r,x);if(target>=0||g->mode==MODE_BOSS&&boss_exposed(g))lob(g,r,x,g->burst_target[r][c],target);*phase=2;}
  if(*clock>=(k==KERNEL?33:37)/35.0f){*phase=0;*clock=0;}return 1;
 }
 if(k==KELP){
  if(!*phase){for(int i=0;i<MAX_ZOMBIES;i++)if(ground_enemy(g,i,r,x-28,x+28)&&g->zombies[i].vault<=0){
    *phase=1;*clock=0;g->burst_target[r][c]=i;g->fidelity.plant_target_id[r][c]=g->fidelity.actor_id[i];
    g->polish.action[i]=ACTION_KELP;g->polish.action_time[i]=0;g->polish.target[i]=r*COLS+c;g->zombies[i].eating=0;cue(FOLEY_FLOOP);break;
  }return 1;}
  float before=*clock;*clock+=dt;int id=g->burst_target[r][c];
  int valid=id>=0&&id<MAX_ZOMBIES&&g->fidelity.actor_id[id]==g->fidelity.plant_target_id[r][c]&&ground_enemy(g,id,r,x-40,x+40)&&g->polish.action[id]==ACTION_KELP;
  if(!valid){p->kind=-1;*phase=0;return 1;}
  if(*clock>=.5f)*phase=2;
  if(before<.8f&&*clock>=.8f){cue(FOLEY_ZOMBIE_ENTERING_WATER);cue(FOLEY_ZOMBIESPLASH);}
  if(*clock>=1){kill_zombie(g,&g->zombies[id]);g->zombies[id].used=0;p->kind=-1;*phase=0;cue(FOLEY_SLURP);}return 1;
 }
 if(k==MAGNET){
  if(p->attack<=0)for(int i=0;i<MAX_ZOMBIES;i++){
   Zombie *z=&g->zombies[i];if(!z->used||z->death>0||g->hypnotized[i]||g->fidelity.equipment_lost[i]||abs(z->row-r)>2||fabsf(z->x-x)>=CELL_W*3)continue;
   int metal=z->armor>0&&(z->kind==Z_BUCKET||z->kind==Z_DOOR||z->kind==Z_FOOTBALL||z->kind==Z_LADDER);
   if(!metal&&z->kind!=Z_POGO&&z->kind!=Z_DIGGER&&z->kind!=Z_JACK)continue;
   z->armor=0;g->fidelity.equipment_lost[i]=1;
   if(z->kind==Z_DIGGER&&!z->vaulted){z->vaulted=1;g->polish.action[i]=ACTION_DIGGER_RISE;g->polish.action_time[i]=0;g->special[i]=0;cue(FOLEY_DIGGER);}
   if(z->kind==Z_POGO){z->vaulted=1;z->vault=0;z->speed=.23f+random_unit(g)*.14f;}
   if(z->kind==Z_LADDER&&g->polish.action[i]==ACTION_LADDER)g->polish.action[i]=0;
   p->attack=15;*clock=0;*phase=1;cue(FOLEY_MAGNETSHROOM);break;
  }
  if(*phase){*clock+=dt;if(*clock>=25/12.0f){*phase=0;*clock=0;}}return 1;
 }
 return 0;
}
