/* Live actor/audio state deliberately stays outside deterministic saved gameplay. */
static int scene_voice_event(int event){return event==FOLEY_GROAN&&(player.completion.cosmetics&COS_SUKHBIR)?FOLEY_SUKHBIR:event;}
static void native_sound(int event){event=scene_voice_event(event);if(event<FOLEY_COUNT)foley_play(event);else foley_sample(event-FOLEY_COUNT);}
static uint32_t scene_rng=0x18a0d10;static float groan_wait=7,thunder_wait=8;
static int audio_last_screen=-1;
static unsigned scene_random(void){scene_rng^=scene_rng<<13;scene_rng^=scene_rng>>17;scene_rng^=scene_rng<<5;return scene_rng;}
typedef struct {int used,kind,vaulted,chewing;float chew,death;const Animation *clip;} SoundActor;
static SoundActor sound_actors[MAX_ZOMBIES];
/* Normalized eating-loop cues recovered from the owner's PS3 executable:
   Animate at 0x2cc3bc-0x2cc4e4, timed-event calls to 0x2d9a48. */
static void chew_markers(int kind,float *left,float *right){
 *left=.14f;*right=.68f;
 switch(kind){case Z_POLE:*left=.38f;*right=.8f;break;
 case Z_PAPER:case Z_LADDER:*left=*right=.42f;break;
 case Z_JACK:*left=*right=.53f;break;
 case Z_BOBSLED:*left=.33f;*right=.83f;break;
 case Z_IMP:*left=.33f;*right=.79f;break;}
}
static int chew_crossings(float from,float to,float marker){
 if(!isfinite(from)||!isfinite(to)||to<=from)return 0;
 return (int)(floorf(to-marker+1e-6f)-floorf(from-marker+1e-6f));
}
static int chew_advance(SoundActor *state,const Animation *clip,int kind,float seconds,float dt){
 if(!clip||!clip->loaded||!clip->frames||clip->fps<=0||!isfinite(seconds)||seconds<0){state->chewing=0;return 0;}
 float rate=clip->fps/clip->frames,position=seconds*rate;
 /* A save load, new eater or changed clip must not replay earlier loop cues. */
 float before=state->chewing&&state->clip==clip&&position>=state->chew?state->chew:fmaxf(0,position-fmaxf(0,dt)*rate);
 float left,right;chew_markers(kind,&left,&right);
 int events=chew_crossings(before,position,left)+(left==right?0:chew_crossings(before,position,right));
 state->chewing=1;state->clip=clip;state->chew=position;return events;
}
static int zombie_chew_events(int index,float dt){
 Zombie *z=&game.zombies[index];SoundActor *state=&sound_actors[index];
 if(!z->used||z->death>0||!z->eating||z->vault>0||!zombie_chewing_kind(z->kind)){state->chewing=0;return 0;}
 if(game.frozen[index]>0)return 0;
 if(!game_zombie_has_head(&game,z)){state->chewing=0;return 0;}
 const Animation *clip=zombie_visible_clip(z,index,0);
 float seconds=zombie_visible_time(z,clip),step=dt*zombie_eating_rate(z->kind)/fmaxf(1,clip->fps);
 return chew_advance(state,clip,z->kind,seconds,step);
}
static int zombie_chew_group(int index){
 Zombie *z=&game.zombies[index];int soft=game.hypnotized[index];
 for(int c=0;c<COLS;c++)if(fabsf(z->x-center(c))<28){int k=game.plants[z->row][c].kind;soft|=k==WALLNUT||k==TALLNUT||game.pumpkin_hp[z->row][c]>0;}
 return soft?FOLEY_CHOMP_SOFT:FOLEY_CHOMP;
}
static void audio_scene_step(float dt){
 if(screen!=audio_last_screen){if(screen==9){foley_sample(FX_HATCHBACK_OPEN);foley_sample(FX_SHOP);}else if(audio_last_screen==9)foley_sample(FX_HATCHBACK_CLOSE);else if(screen==10)foley_sample(FX_ALMANAC);else if(screen==7)foley_sample(FX_MINIGAMES);else if(screen==14)foley_sample(FX_SURVIVAL);else if(screen==15)foley_sample(FX_PUZZLE);else if(screen==16&&audio_last_screen!=22)foley_sample(FX_ZEN);audio_last_screen=screen;}

 int active=screen==1&&game.status==PLAYING,advance=active&&!paused&&!help;int jack=0,digger=0;
 foley_pause(paused||help);
 for(int n=0;n<MAX_ZOMBIES;n++){
  Zombie *z=&game.zombies[n];if(screen!=1||!z->used){memset(&sound_actors[n],0,sizeof(sound_actors[n]));continue;}
  if(!sound_actors[n].used||sound_actors[n].kind!=z->kind){sound_actors[n]=(SoundActor){.used=1,.kind=z->kind,.vaulted=z->vaulted};}
  if(z->death<=0){jack|=z->kind==Z_JACK&&game.polish.action[n]!=ACTION_JACK;digger|=z->kind==Z_DIGGER&&!z->vaulted;
   if(advance){int bites=zombie_chew_events(n,dt);for(int b=0;b<bites;b++)foley_play(zombie_chew_group(n));
    if(z->kind==Z_DIGGER&&z->vaulted&&!sound_actors[n].vaulted)foley_play(FOLEY_DIRT_RISE);
    sound_actors[n].vaulted=z->vaulted||z->vault>0;
   }
  }else if(advance||game.status==WON){float normalized=1-z->death/game_death_duration_for(&game,z),threshold=z->kind==Z_POLE?.68f:z->kind==Z_FOOTBALL?.52f:z->kind==Z_PAPER?.63f:z->kind==Z_DANCER||z->kind==Z_BACKUP?.83f:z->kind==Z_GARG?.86f:z->kind==Z_DIGGER?.85f:.77f;if(!game.polish.death_style[n]&&sound_actors[n].death<threshold&&normalized>=threshold)foley_play(FOLEY_ZOMBIE_FALLING);sound_actors[n].death=normalized;}
 }
 foley_loop(FOLEY_JACKINTHEBOX,active&&jack);foley_loop(FOLEY_DIGGER,active&&digger);foley_loop(FOLEY_RAIN,active&&(game.level==40||(live_benchmark&&live_phase==1)));
 if(advance){groan_wait-=dt;if(groan_wait<=0){int live[MAX_ZOMBIES],count=0;for(int n=0;n<MAX_ZOMBIES;n++)if(game.zombies[n].used&&game.zombies[n].death<=0)live[count++]=n;if(count){Zombie *z=&game.zombies[live[scene_random()%count]];native_sound(z->kind==Z_GARG?FOLEY_LOW_GROAN:scene_random()%10==0?FOLEY_BRAINS:FOLEY_GROAN);}groan_wait=5+(scene_random()%1000)*.01f;}
  if(game.level==40){thunder_wait-=dt;if(thunder_wait<=0){foley_play(FOLEY_THUNDER);thunder_wait=8+scene_random()%12;}}
 }else if(!active){groan_wait=7;thunder_wait=8;}
}
