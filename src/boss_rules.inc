static int boss_exposed(const Game *g){return g->extra.boss_action>=B_HEAD_WAIT&&g->extra.boss_action<=B_HEAD_AFTER;}
static void boss_set(Game *g,int action){
 g->extra.boss_action=action;g->extra.boss_action_time=0;
 if(action==B_IDLE)g->polish024.boss_idle=1+random_unit(g);
 else if(action==B_HEAD_ENTER||action==B_HEAD_LEAVE)cue(FOLEY_HYDRAULIC);
 else if(action==B_SPAWN||action==B_BUNGEE||action==B_BUNGEE_LEAVE)cue(FOLEY_HYDRAULIC_SHORT);
 else if(action==B_RV){g->boss_row=random_next(g)%4;g->extra.boss_target_col=random_next(g)%3;cue(GAME_SAMPLE(FX_RVTHROW));}
 else if(action==B_HEAD_SPIT)cue(FOLEY_BOSS_BOULDER_ATTACK);
 if(action==B_BUNGEE){g->extra.boss_bungee_delay=40+random_unit(g)*10;g->extra.boss_target_col=random_next(g)%3;memset(g->polish024.boss_bungee_id,0,sizeof(g->polish024.boss_bungee_id));cue(FOLEY_BUNGEE_SCREAM);}
}
static void boss_squish(Game *g,int row,int col,int width){for(int r=row;r<=row+1&&r<5;r++)for(int c=col;c<col+width&&c<9;c++)destroy_square(g,r,c,1);g->flash=.13f;sound_as(g,SFX_EXPLODE,FOLEY_THUMP);}
static int boss_stomp_row(Game *g){
 int choices[4],count=0;for(int row=0;row<4;row++){int plants=0;for(int r=row;r<=row+1;r++)for(int c=5;c<9;c++)plants+=g->plants[r][c].kind>=0||g->pumpkin_hp[r][c]>0||g->support[r][c];if(plants)choices[count++]=row;}
 return count?choices[random_next(g)%count]:-1;
}
static void boss_ball_step024(Game *g,float dt){
 if(!g->boss_ball_kind)return;
 float before=fmaxf(0,g->polish024.boss_ball_age-15/16.0f);g->polish024.boss_ball_age+=dt;
 float after=fmaxf(0,g->polish024.boss_ball_age-15/16.0f);
 const MotionProfile *p=&motion[7+g->boss_ball_kind];
 if(p->count>1)g->boss_ball_x-=(ground_position(p,after*2/(p->count-1))-ground_position(p,before*2/(p->count-1)))*.475f;
 else g->boss_ball_x-=(after-before)*17.6f;
 if(after<=0)return;
 int col=(int)floorf((g->boss_ball_x-GRID_X)/CELL_W);if(col>=0&&col<COLS)destroy_square(g,g->boss_ball_row,col,1);
 int row=g->boss_ball_row;
 if(!g->mower_used[row]&&g->boss_ball_x<GRID_X-8){g->mower_used[row]=1;g->mower_x[row]=600;cue(FOLEY_THUMP);}
 if(g->boss_ball_x<GRID_X-180*.475f){g->boss_ball_kind=0;g->polish024.boss_ball_age=0;}
}
static int boss_step(Game *g,float dt){
 if(g->boss_hp<=0){cue(GAME_SAMPLE(FX_BOSSEXPLOSION));g->status=WON;sound(g,SFX_WIN);return 1;}
 float maximum=g->mini.id==MINI_BOSS+1?60000:40000;int phase=g->boss_hp<maximum/3?2:g->boss_hp<maximum*2/3?1:0;
 if(g->extra.boss_head_time==0&&!g->extra.boss_action&&g->time<.2f){g->extra.boss_head_time=50;g->extra.boss_stomp_delay=40;g->extra.boss_bungee_delay=65;g->polish024.boss_idle=1+random_unit(g);}
 g->boss_phase=fmaxf(0,g->boss_phase-dt);float step=g->boss_phase>0?0:dt;
 g->boss_timer-=step;g->extra.boss_head_time-=step;g->extra.boss_stomp_delay-=step;g->extra.boss_bungee_delay-=step;
 float before=g->extra.boss_action_time;g->extra.boss_action_time+=step;float t=g->extra.boss_action_time;
 switch(g->extra.boss_action){
 case B_IDLE:
  g->polish024.boss_idle=fmaxf(0,g->polish024.boss_idle-step);if(g->polish024.boss_idle>0)break;
  if(phase>g->extra.boss_stomps){g->extra.boss_stomps=phase;boss_set(g,phase==1?B_BUNGEE:B_RV);}
  else if(phase&&g->extra.boss_stomp_delay<=0){int row=boss_stomp_row(g);g->extra.boss_stomp_delay=55+random_unit(g)*10;if(row>=0){g->boss_row=row;boss_set(g,B_STOMP);}else boss_set(g,B_IDLE);}
  else if(phase>=2&&g->extra.boss_bungee_delay<=0){g->extra.boss_bungee_delay=40+random_unit(g)*10;boss_set(g,random_next(g)%(g->challenge?2:4)==0?B_RV:B_BUNGEE);}
  else if(g->extra.boss_head_time<=0){boss_set(g,B_HEAD_ENTER);g->extra.boss_head_time=40+random_unit(g)*10;}
  else if(g->boss_timer<=0&&game_alive(g)<20){g->boss_row=random_next(g)%5;boss_set(g,B_SPAWN);g->boss_timer=(phase==0?4.5f:phase==1?3.5f:1.5f)+random_unit(g);}
  else boss_set(g,B_IDLE);
  break;
 case B_SPAWN:{float duration=(g->boss_row==0?21:20)/12.0f;
  if(before<duration*.6f&&t>=duration*.6f){static const int kinds[]={Z_BUCKET,Z_DOOR,Z_FOOTBALL,Z_LADDER,Z_CATAPULT,Z_GARG};int kind=g->time<35?Z_NORMAL:g->time<80?Z_CONE:g->time<125?Z_BUCKET:kinds[random_next(g)%(g->boss_row?6:5)];game_spawn_zombie(g,g->boss_row,kind,400);}
  if(t>=duration)boss_set(g,B_IDLE);break;}
 case B_HEAD_ENTER:if(t>=56/12.0f)boss_set(g,B_HEAD_WAIT);break;
 case B_HEAD_WAIT:if(t>=5){g->boss_ball_row=g->boss_row=random_next(g)%5;g->boss_ball_kind=0;boss_set(g,B_HEAD_SPIT);}break;
 case B_HEAD_SPIT:{float duration=(g->boss_row==4?42:39)/12.0f;
  if(before<duration*.37f&&t>=duration*.37f){g->boss_ball_kind=1+random_next(g)%2;g->boss_ball_row=g->boss_row;g->boss_ball_x=440;g->polish024.boss_ball_age=0;}
  if(t>=duration)boss_set(g,B_HEAD_AFTER);break;}
 case B_HEAD_AFTER:if(t>=3)boss_set(g,B_HEAD_LEAVE);break;
 case B_HEAD_LEAVE:if(t>=44/12.0f)boss_set(g,B_IDLE);break;
 case B_STOMP:{float duration=(g->boss_row<2?30:28)/12.0f,contact=duration*(g->boss_row<2?.5f:.55f);if(before<contact&&t>=contact)boss_squish(g,g->boss_row,5,4);if(t>=duration)boss_set(g,B_IDLE);break;}
 case B_BUNGEE:
  if(t<.8f)break;
  if(!zombie_slots_available(g,3)){
   // Leave time for dying actors to clear before ending a blocked attack.
   if(t>=1.8f)boss_set(g,B_BUNGEE_LEAVE);
   break;
  }
  for(int n=0;n<3;n++){
   int col=g->extra.boss_target_col+n,rows[5],count=0;
   for(int r=0;r<5;r++)if(g->plants[r][col].kind>=0||g->support[r][col]||g->pumpkin_hp[r][col]>0)rows[count++]=r;
   int row=count?rows[random_next(g)%count]:random_next(g)%5;
   int z=game_spawn_zombie(g,row,Z_BUNGEE,plant_x(col));
   if(z<0)break;
   g->zombies[z].x=plant_x(col);
   g->polish024.boss_bungee_id[n]=g->fidelity.actor_id[z];
  }
  boss_set(g,B_BUNGEE_DROP);
  break;
 case B_BUNGEE_DROP:{
  int descending=0;
  for(int n=0;n<3;n++){
   int i=actor_by_id024(g,g->polish024.boss_bungee_id[n]);
   if(i>=0&&g->zombies[i].death<=0&&g->fidelity.bungee_phase[i]<3){descending=1;break;}
  }
  if(!descending)boss_set(g,B_BUNGEE_LEAVE);
  break;
 }
 case B_BUNGEE_LEAVE:if(t>=17/18.0f)boss_set(g,B_IDLE);break;
 case B_RV:{float contact=.65f*(37/16.0f);if(before<contact&&t>=contact){boss_squish(g,g->boss_row,g->extra.boss_target_col,3);g->boss_timer=5;g->extra.boss_head_time=50;if(phase)g->extra.boss_stomp_delay=40;if(phase>=2)g->extra.boss_bungee_delay=65;}if(t>=37/16.0f)boss_set(g,B_IDLE);break;}
 }
 boss_ball_step024(g,dt);
 g->wave=1+(int)((maximum-g->boss_hp)/maximum*g->num_waves);if(g->wave>g->num_waves)g->wave=g->num_waves;g->spawned=1;return 1;
}
