/* Live actor/audio state deliberately stays outside deterministic saved gameplay. */
static void native_sound(int event){if(event<FOLEY_COUNT)foley_play(event);else foley_sample(event-FOLEY_COUNT);}
static uint32_t scene_rng=0x18a0d10;static float groan_wait=7,thunder_wait=8;
static int audio_last_screen=-1;
static unsigned scene_random(void){scene_rng^=scene_rng<<13;scene_rng^=scene_rng>>17;scene_rng^=scene_rng<<5;return scene_rng;}
static struct {int used,kind,vaulted;float bite,death;} sound_actors[MAX_ZOMBIES];
static void audio_scene_step(float dt){
 if(screen!=audio_last_screen){if(screen==9){foley_sample(FX_HATCHBACK_OPEN);foley_sample(FX_SHOP);}else if(audio_last_screen==9)foley_sample(FX_HATCHBACK_CLOSE);else if(screen==10)foley_sample(FX_ALMANAC);else if(screen==7)foley_sample(FX_MINIGAMES);else if(screen==14)foley_sample(FX_SURVIVAL);else if(screen==15)foley_sample(FX_PUZZLE);else if(screen==16&&audio_last_screen!=22)foley_sample(FX_ZEN);audio_last_screen=screen;}

 int active=screen==1&&game.status==PLAYING,advance=active&&!paused&&!help;int jack=0,digger=0;
 foley_pause(paused||help);
 for(int n=0;n<MAX_ZOMBIES;n++){
  Zombie *z=&game.zombies[n];if(screen!=1||!z->used){memset(&sound_actors[n],0,sizeof(sound_actors[n]));continue;}
  if(!sound_actors[n].used||sound_actors[n].kind!=z->kind){sound_actors[n].used=1;sound_actors[n].kind=z->kind;sound_actors[n].bite=0;sound_actors[n].death=0;sound_actors[n].vaulted=z->vaulted;}
  if(z->death<=0){jack|=z->kind==Z_JACK;digger|=z->kind==Z_DIGGER&&!z->vaulted;
   if(advance){if(z->eating&&z->kind!=Z_GARG&&z->kind!=Z_ZOMBONI&&z->kind!=Z_CATAPULT){sound_actors[n].bite-=dt;if(sound_actors[n].bite<=0){int soft=game.hypnotized[n];for(int c=0;c<COLS;c++)if(fabsf(z->x-center(c))<28){int k=game.plants[z->row][c].kind;soft|=k==WALLNUT||k==TALLNUT||game.pumpkin_hp[z->row][c]>0;}foley_play(soft?FOLEY_CHOMP_SOFT:FOLEY_CHOMP);sound_actors[n].bite=.6f;}}else sound_actors[n].bite=0;
    if(z->kind==Z_DIGGER&&z->vaulted&&!sound_actors[n].vaulted)foley_play(FOLEY_DIRT_RISE);
    sound_actors[n].vaulted=z->vaulted||z->vault>0;
   }
  }else if(advance||game.status==WON){float normalized=1-z->death/game_death_duration(z),threshold=z->kind==Z_POLE?.68f:z->kind==Z_FOOTBALL?.52f:z->kind==Z_PAPER?.63f:z->kind==Z_DANCER||z->kind==Z_BACKUP?.83f:z->kind==Z_GARG?.86f:z->kind==Z_DIGGER?.85f:.77f;if(sound_actors[n].death<threshold&&normalized>=threshold)foley_play(FOLEY_ZOMBIE_FALLING);sound_actors[n].death=normalized;}
 }
 foley_loop(FOLEY_JACKINTHEBOX,active&&jack);foley_loop(FOLEY_DIGGER,active&&digger);foley_loop(FOLEY_RAIN,active&&(game.level==40||(live_benchmark&&live_phase==1)));
 if(advance){groan_wait-=dt;if(groan_wait<=0){int live[MAX_ZOMBIES],count=0;for(int n=0;n<MAX_ZOMBIES;n++)if(game.zombies[n].used&&game.zombies[n].death<=0)live[count++]=n;if(count){Zombie *z=&game.zombies[live[scene_random()%count]];foley_play(z->kind==Z_GARG?FOLEY_LOW_GROAN:scene_random()%10==0?FOLEY_BRAINS:FOLEY_GROAN);}groan_wait=5+(scene_random()%1000)*.01f;}
  if(game.level==40){thunder_wait-=dt;if(thunder_wait<=0){foley_play(FOLEY_THUNDER);thunder_wait=8+scene_random()%12;}}
 }else if(!active){groan_wait=7;thunder_wait=8;}
}
