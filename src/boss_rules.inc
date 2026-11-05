static int boss_exposed(const Game *g){return g->extra.boss_action>=B_HEAD_ENTER&&g->extra.boss_action<=B_HEAD_LEAVE;}
static void boss_set(Game *g,int action){g->extra.boss_action=action;g->extra.boss_action_time=0;if(action==B_HEAD_ENTER||action==B_HEAD_LEAVE)cue(FOLEY_HYDRAULIC);else if(action==B_SPAWN)cue(FOLEY_HYDRAULIC_SHORT);else if(action==B_RV)cue(GAME_SAMPLE(FX_RVTHROW));else if(action==B_HEAD_SPIT)cue(FOLEY_BOSS_BOULDER_ATTACK);if(action==B_RV||action==B_STOMP)cue(FOLEY_BOSS_EXPLOSION_SMALL);}
static void boss_squish(Game *g,int row,int col,int width){for(int r=row;r<=row+1&&r<5;r++)for(int c=col;c<col+width&&c<9;c++){destroy_square(g,r,c,1);}g->flash=.13f;sound_as(g,SFX_EXPLODE,FOLEY_THUMP);}
static int boss_step(Game *g,float dt){
 if(g->boss_hp<=0){cue(GAME_SAMPLE(FX_BOSSEXPLOSION));g->status=WON;sound(g,SFX_WIN);return 1;}
 float maximum=g->mini.id==MINI_BOSS+1?60000:40000;int phase=g->boss_hp<maximum/3?2:g->boss_hp<maximum*2/3?1:0;
 if(g->extra.boss_head_time==0&&!g->extra.boss_action&&g->time<.2f){g->extra.boss_head_time=50;g->extra.boss_stomp_delay=40;g->extra.boss_bungee_delay=65;}
 g->boss_phase=fmaxf(0,g->boss_phase-dt);float step=g->boss_phase>0?0:dt;
 g->boss_timer-=step;g->extra.boss_head_time-=step;g->extra.boss_stomp_delay-=step;g->extra.boss_bungee_delay-=step;
 float before=g->extra.boss_action_time;g->extra.boss_action_time+=step;float t=g->extra.boss_action_time;
 switch(g->extra.boss_action){
 case B_IDLE:
  if(phase>g->extra.boss_stomps){g->extra.boss_stomps=phase;g->boss_row=random_next(g)%4;g->extra.boss_target_col=random_next(g)%3;boss_set(g,phase==1?B_BUNGEE:B_RV);}
  else if(g->extra.boss_head_time<=0){boss_set(g,B_HEAD_ENTER);g->extra.boss_head_time=40+random_unit(g)*10;}
  else if(phase&&g->extra.boss_stomp_delay<=0){g->boss_row=random_next(g)%4;g->extra.boss_stomp_delay=55+random_unit(g)*10;boss_set(g,B_STOMP);}
  else if(phase>=2&&g->extra.boss_bungee_delay<=0){g->extra.boss_target_col=random_next(g)%3;g->boss_row=random_next(g)%4;g->extra.boss_bungee_delay=40+random_unit(g)*10;boss_set(g,random_next(g)%2?B_BUNGEE:B_RV);}
  else if(g->boss_timer<=0&&game_alive(g)<20){g->boss_row=random_next(g)%5;boss_set(g,B_SPAWN);g->boss_timer=(phase==0?4.5f:phase==1?3.5f:1.5f)+random_unit(g);}
  break;
 case B_SPAWN:
  if(before<1&&t>=1){static const int kinds[]={Z_BUCKET,Z_DOOR,Z_FOOTBALL,Z_LADDER,Z_CATAPULT,Z_GARG};int kind=g->time<35?Z_NORMAL:g->time<80?Z_CONE:g->time<125?Z_BUCKET:kinds[random_next(g)%(g->boss_row?6:5)];game_spawn_zombie(g,g->boss_row,kind,400);}
  if(t>=21/12.0f)boss_set(g,B_IDLE);break;
 case B_HEAD_ENTER:if(t>=57/12.0f)boss_set(g,B_HEAD_WAIT);break;
 case B_HEAD_WAIT:if(t>=5){g->boss_ball_row=g->boss_row=random_next(g)%5;boss_set(g,B_HEAD_SPIT);}break;
 case B_HEAD_SPIT:
  if(before<1.4f&&t>=1.4f&&!g->boss_ball_kind){g->boss_ball_kind=1+random_next(g)%2;g->boss_ball_row=g->boss_row;g->boss_ball_x=440;}
  if(t>=40/12.0f)boss_set(g,B_HEAD_AFTER);break;
 case B_HEAD_AFTER:if(t>=3)boss_set(g,B_HEAD_LEAVE);break;
 case B_HEAD_LEAVE:if(t>=45/12.0f)boss_set(g,B_IDLE);break;
 case B_STOMP:if(before<1.3f&&t>=1.3f)boss_squish(g,g->boss_row,5,4);if(t>=31/12.0f)boss_set(g,B_IDLE);break;
 case B_BUNGEE:
  if(before<.85f&&t>=.85f)for(int i=0;i<3;i++){int row=random_next(g)%5,z=game_spawn_zombie(g,row,Z_BUNGEE,plant_x(g->extra.boss_target_col+i));if(z>=0)g->zombies[z].x=plant_x(g->extra.boss_target_col+i);}
  if(t>=5.5f)boss_set(g,B_IDLE);break;
 case B_RV:if(before<1.54f&&t>=1.54f)boss_squish(g,g->boss_row,g->extra.boss_target_col,3);if(t>=38/16.0f)boss_set(g,B_IDLE);break;
 }
 if(g->boss_ball_kind){g->boss_ball_x-=32*dt;int col=(int)((g->boss_ball_x-GRID_X)/CELL_W);if(col>=0&&col<COLS){destroy_square(g,g->boss_ball_row,col,1);}if(g->boss_ball_x<12)g->status=LOST;}
 g->wave=1+(int)((maximum-g->boss_hp)/maximum*g->num_waves);if(g->wave>g->num_waves)g->wave=g->num_waves;g->spawned=1;return 1;
}
