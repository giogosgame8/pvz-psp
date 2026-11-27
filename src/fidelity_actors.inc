/* Return true while an action owns locomotion. All clocks are saved. */
static int fidelity_actor_step(Game *g,int i,float dt){
 Zombie *z=&g->zombies[i];int action=g->polish.action[i];
 if(action<ACTION_KELP)return 0;
 float before=g->polish.action_time[i];g->polish.action_time[i]+=dt*(z->slow>0?.5f:1);float t=g->polish.action_time[i];z->eating=0;
 if(action==ACTION_POOL_RISE){float duration=game_water_row(g,z->row)?.5f:1;g->polish024.altitude[i]=-(game_water_row(g,z->row)?49.5f:36)*(1-fminf(1,t/duration));if(t>=duration){g->polish.action[i]=0;g->polish024.altitude[i]=0;z->age=g->polish.pose[i]=0;}return 1;}
 if(action==ACTION_SKY_DROP){int parent=actor_by_id024(g,g->polish024.leader_id[i]);if(parent>=0&&g->zombies[parent].death<=0&&g->fidelity.bungee_phase[parent]==0){g->polish024.altitude[i]=fmaxf(0,g->fidelity.actor_aux[parent]-g->special[parent])*380;return 1;}g->polish024.altitude[i]=fmaxf(0,g->polish024.altitude[i]-380*dt);if(!g->polish024.altitude[i]){g->polish.action[i]=0;g->polish024.leader_id[i]=0;z->age=g->polish.pose[i]=0;}return 1;}
 if(action==ACTION_KELP){
  int cell=g->polish.target[i],r=cell/COLS,c=cell%COLS;
  if(r>=ROWS||c>=COLS||g->plants[r][c].kind!=KELP||!g->burst_left[r][c]){g->polish.action[i]=0;return 0;}return 1;
 }
 if(action==ACTION_SNORKEL_RISE||action==ACTION_SNORKEL_DIVE){
  int cell=g->fidelity.actor_target[i],r=cell/COLS,c=cell%COLS;
  int target=0;
  if(r==z->row&&(unsigned)r<ROWS&&(unsigned)c<COLS){
   int anchor=bite_plant_col(g,r,c);
   int plant=squash_in_flight(g,r,anchor)?-1:g->plants[r][anchor].kind;
   target=tile_is_bite_target(g,r,c,plant,z->kind)&&bite_contact(z->x,plant_x(c))&&game_zombie_has_head(g,z);
  }
  if(action==ACTION_SNORKEL_RISE&&!target){g->polish.action[i]=ACTION_SNORKEL_DIVE;g->polish.action_time[i]=fmaxf(0,11/24.0f-t);return 1;}
  z->eating=action==ACTION_SNORKEL_RISE;
  if(t>=11/24.0f){g->polish.action[i]=0;g->fidelity.actor_aux[i]=action==ACTION_SNORKEL_RISE?1:0;z->age=g->polish.pose[i]=0;}return 1;
 }
 if(action==ACTION_LADDER){
  int cell=g->polish.target[i],r=cell/COLS,c=cell%COLS;
  if(r!=z->row||(unsigned)r>=ROWS||(unsigned)c>=COLS||z->armor<=0||
     !bite_contact(z->x,plant_x(c))||ladder_attachment(g,r,c)){
   g->polish.action[i]=0;return 0;
  }
  int attachment=g->fidelity.actor_target[i];
  if(!attachment){ /* Older saves only record the target tile. */
   attachment=ladder_surface(g,r,c);
   g->fidelity.actor_target[i]=attachment;
   if(attachment)g->fidelity.grabbed_id[i]=ladder_plant_id(g,r,c,attachment);
  }
  if(!ladder_surface_live(g,r,c,attachment)||
     g->fidelity.grabbed_id[i]!=ladder_plant_id(g,r,c,attachment)){
   g->polish.action[i]=0;return 0;
  }
  if(t>=11/24.0f){
   g->ladders[r][c]=attachment;z->armor=0;g->fidelity.equipment_lost[i]=1;cue(GAME_SAMPLE(FX_LADDER_ZOMBIE));
   g->polish.action[i]=0;z->age=g->polish.pose[i]=0;
  }return 1;
 }
 if(action==ACTION_BALLOON_FALL){if(t>=27/24.0f){g->polish.action[i]=0;z->age=g->polish.pose[i]=0;z->speed=.23f+random_unit(g)*.14f;cue(FOLEY_ZOMBIE_FALLING);}return 1;}
 if(action==ACTION_DIGGER_RISE){if(t>=.6f+13/16.0f){g->polish.action[i]=0;z->age=g->polish.pose[i]=0;g->special[i]=0;z->speed=.23f+random_unit(g)*.14f;}return 1;}
 if(action==ACTION_CATAPULT){
  if(before<.545f*(31/24.0f)&&t>=.545f*(31/24.0f)){int cell=g->fidelity.actor_target[i],r=cell/COLS,c=cell%COLS;
   if(r>=0&&r<ROWS&&c>=0&&c<COLS){
    float x=z->x-12;
    int n=allocate_shot(g,r,x,14);
    if(n>=0){
     float y=game_ground_y(g,r)+game_roof_offset(g,x)-32,dest=plant_x(c),target_y=game_ground_y(g,r)+game_roof_offset(g,dest)-14;
     g->shot_y[n]=y;g->shot_vx[n]=dest-x;g->shot_vy[n]=target_y-y-150;g->shot_remaining[n]=1;g->shot_target[n]=cell;g->shot_damage[n]=75;cue(FOLEY_BASKETBALL);
    }
   }
  }
  if(t>=31/24.0f){g->fidelity.actor_aux[i]=fmaxf(0,g->fidelity.actor_aux[i]-1);g->polish.action[i]=0;z->age=g->polish.pose[i]=0;}return 1;
 }
 return 0;
}
