/* Return true while an action owns locomotion. All clocks are saved. */
static int fidelity_actor_step(Game *g,int i,float dt){
 Zombie *z=&g->zombies[i];int action=g->polish.action[i];
 if(action<ACTION_KELP)return 0;
 float before=g->polish.action_time[i];g->polish.action_time[i]+=dt*(z->slow>0?.5f:1);float t=g->polish.action_time[i];z->eating=0;
 if(action==ACTION_KELP){
  int cell=g->polish.target[i],r=cell/COLS,c=cell%COLS;
  if(r>=ROWS||c>=COLS||g->plants[r][c].kind!=KELP||!g->burst_left[r][c]){g->polish.action[i]=0;return 0;}return 1;
 }
 if(action==ACTION_SNORKEL_RISE||action==ACTION_SNORKEL_DIVE){
  int cell=g->fidelity.actor_target[i],r=cell/COLS,c=cell%COLS;
  int target=r>=0&&r<ROWS&&c>=0&&c<COLS&&(g->plants[r][c].kind>=0||g->support[r][c]||g->pumpkin_hp[r][c]>0)&&bite_contact(z->x,plant_x(c))&&game_zombie_has_head(g,z);
  if(action==ACTION_SNORKEL_RISE&&!target){g->polish.action[i]=ACTION_SNORKEL_DIVE;g->polish.action_time[i]=fmaxf(0,11/24.0f-t);return 1;}
  z->eating=action==ACTION_SNORKEL_RISE;
  if(t>=11/24.0f){g->polish.action[i]=0;g->fidelity.actor_aux[i]=action==ACTION_SNORKEL_RISE?1:0;z->age=g->polish.pose[i]=0;}return 1;
 }
 if(action==ACTION_LADDER){
  int cell=g->polish.target[i],r=cell/COLS,c=cell%COLS;
  if(r!=z->row||r>=ROWS||c>=COLS||z->armor<=0){g->polish.action[i]=0;return 0;}
  if(t>=11/24.0f){
   if(g->plants[r][c].kind==WALLNUT||g->plants[r][c].kind==TALLNUT||g->pumpkin_hp[r][c]>0){g->ladders[r][c]=1;z->armor=0;g->fidelity.equipment_lost[i]=1;cue(GAME_SAMPLE(FX_LADDER_ZOMBIE));}
   g->polish.action[i]=0;z->age=g->polish.pose[i]=0;
  }return 1;
 }
 if(action==ACTION_BALLOON_FALL){if(t>=27/24.0f){g->polish.action[i]=0;z->age=g->polish.pose[i]=0;z->speed=.23f+random_unit(g)*.14f;cue(FOLEY_ZOMBIE_FALLING);}return 1;}
 if(action==ACTION_DIGGER_RISE){if(t>=.6f+13/16.0f){g->polish.action[i]=0;z->age=g->polish.pose[i]=0;g->special[i]=0;z->speed=.23f+random_unit(g)*.14f;}return 1;}
 if(action==ACTION_CATAPULT){
  if(before<.545f*(31/24.0f)&&t>=.545f*(31/24.0f)){int cell=g->fidelity.actor_target[i],r=cell/COLS,c=cell%COLS;
   if(r>=0&&r<ROWS&&c>=0&&c<COLS)for(int n=0;n<MAX_SHOTS;n++)if(!g->shots[n].used){
    float x=z->x-12,y=game_ground_y(g,r)+game_roof_offset(g,x)-32,dest=plant_x(c),target_y=game_ground_y(g,r)+game_roof_offset(g,dest)-14;
    g->shots[n]=(Shot){1,r,14,x};g->shot_y[n]=y;g->shot_vx[n]=(dest-x)/1.0f;g->shot_vy[n]=target_y-y-150;g->shot_remaining[n]=1;g->shot_target[n]=cell;g->shot_damage[n]=75;cue(FOLEY_BASKETBALL);break;
   }
  }
  if(t>=31/24.0f){g->fidelity.actor_aux[i]=fmaxf(0,g->fidelity.actor_aux[i]-1);g->polish.action[i]=0;z->age=g->polish.pose[i]=0;}return 1;
 }
 return 0;
}
