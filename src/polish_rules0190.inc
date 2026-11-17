/* Explicit, saved transitions. Clip lengths are from the owned disc. */
static int polish_actor_step(Game *g,int i,float dt){
 Zombie *z=&g->zombies[i];int action=g->polish.action[i];if(!action)return 0;
 /* Animation-driven recovery slows with chill. Countdown-driven reactions
    and airborne physics keep simulation time; freeze was handled by the caller. */
 float before=g->polish.action_time[i],step=dt*((action==ACTION_PAPER||action==ACTION_GARG_THROW||action==ACTION_IMP_LAND)&&z->slow>0?.5f:1);g->polish.action_time[i]+=step;float t=g->polish.action_time[i];
 if(action==ACTION_GARLIC){
  if(before<.7f&&t>=.7f){z->eating=0;z->age=0;g->polish.pose[i]=0;cue(FOLEY_YUCK);}
  if(t<.7f)z->age+=dt*(z->slow>0?.5f:1);
  if(before<1.7f&&t>=1.7f)z->row=g->polish.target[i]%ROWS;
  if(t>=2.7f){g->polish.action[i]=0;z->age=0;g->polish.pose[i]=0;}return 1;
 }
 if(action==ACTION_PAPER){z->eating=0;if(t>=13/8.0f){g->polish.action[i]=0;g->polish.pose[i]=0;z->age=0;cue(FOLEY_NEWSPAPER_RARRGH);}return 1;}
 if(action==ACTION_JACK){
  z->eating=0;if(before<.3f&&t>=.3f)cue(FOLEY_JACK_SURPRISE);
  if(t>=1.1f){for(int r=0;r<game_rows(g);r++)for(int c=0;c<COLS;c++)if(abs(r-z->row)<=1&&fabsf(plant_x(c)-z->x)<60)destroy_square(g,r,c,1);kill_zombie(g,z);g->flash=.25f;sound_as(g,SFX_EXPLODE,FOLEY_EXPLOSION);}return 1;
 }
 if(action==ACTION_GARG_THROW){
  z->eating=0;
  if(!z->vaulted&&t>=.74f*(34/24.0f)){
   /* Release is consumed even when the actor pool is full. */
   z->vaulted=1;
   int child=game_spawn_zombie(g,z->row,Z_IMP,z->x-12);
   if(child>=0){
    Zombie *imp=&g->zombies[child];float distance=(z->x-GRID_X)/.475f-360;
    float minimum=game_world(g)==4?-140:40;if(game_world(g)==4)distance-=180;
    if(distance<minimum)distance=minimum;else if(distance>140)distance-=random_unit(g)*100;
    imp->x=z->x-133*.475f;imp->vault_from=imp->x;imp->vault_to=0;
    g->polish024.altitude[child]=88*.4464f;g->polish024.velocity_y[child]=.5f*(distance/3)*.05f*100*.4464f;
    g->polish.action[child]=ACTION_IMP_FLIGHT;g->polish.target[child]=0;g->polish.action_time[child]=0;cue(FOLEY_SWING);
   }
  }
  if(z->vaulted&&t>=34/24.0f){g->polish.action[i]=0;g->polish.pose[i]=0;z->age=0;}return 1;
 }
 if(action==ACTION_IMP_FLIGHT){
  z->eating=0;z->x=z->vault_from-142.5f*t;
  float height=88*.4464f+g->polish024.velocity_y[i]*t-111.6f*t*t;
  height+=game_roof_offset(g,z->x)-game_roof_offset(g,z->vault_from);
  g->polish024.altitude[i]=fmaxf(0,height);
  if(height<=0){g->polish024.altitude[i]=0;g->polish.action[i]=ACTION_IMP_LAND;g->polish.action_time[i]=0;z->age=0;cue(FOLEY_THUMP);}return 1;
 }
 if(action==ACTION_IMP_LAND){if(t>=6/24.0f){g->polish.action[i]=0;g->polish.pose[i]=0;z->age=0;}return 1;}
 return 0;
}
