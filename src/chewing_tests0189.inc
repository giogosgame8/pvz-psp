static int chewing_selftest(FILE *out){
 Game old=game;SoundActor saved[MAX_ZOMBIES];memcpy(saved,sound_actors,sizeof(saved));int pass=0,total=0;
 #define C(label,test) do{int ok=(test);total++;pass+=ok;fprintf(out,"chewing %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 const int kinds[]={Z_NORMAL,Z_POLE,Z_PAPER,Z_LADDER,Z_JACK,Z_BOBSLED,Z_IMP};
 const float markers[][2]={{.14f,.68f},{.38f,.8f},{.42f,.42f},{.42f,.42f},{.53f,.53f},{.33f,.83f},{.33f,.79f}};
 int match=1;for(int i=0;i<7;i++){float a,b;chew_markers(kinds[i],&a,&b);match&=a==markers[i][0]&&b==markers[i][1];}
 C("all seven cue profiles match the PS3 executable",match);
 C("PS3 eating playback overrides the source file rate",zombie_eating_rate(Z_NORMAL)==36&&zombie_eating_rate(Z_POLE)==20&&zombie_eating_rate(Z_IMP)==20&&zombie_eating_rate(Z_DIGGER)==20&&zombie_eating_rate(Z_JACK)==20&&zombie_eating_rate(Z_SNORKEL)==20&&zombie_eating_rate(Z_YETI)==20&&zombie_eating_rate(Z_BALLOON)==20);
 C("cue crossing fires once at the boundary",chew_crossings(0,.14f,.14f)==1&&chew_crossings(.14f,.14f,.14f)==0&&chew_crossings(.14f,.15f,.14f)==0);
 C("a cue after loop wrap is retained",chew_crossings(.99f,1.15f,.14f)==1);
 Animation clip={.loaded=1,.frames=40,.fps=12};float duration=clip.frames/clip.fps;
 for(int k=0;k<7;k++)for(int test=0;test<3;test++){
  SoundActor state={0};float seconds=0;int events=0,n=0;float end=duration*4;
  while(seconds<end){float dt=test==0?1/30.0f:test==1?1/60.0f:(float[]){.009f,.041f,.017f,.083f}[n++%4];dt=fminf(dt,end-seconds);seconds+=dt;events+=chew_advance(&state,&clip,kinds[k],seconds,dt);}
  char label[96];snprintf(label,sizeof(label),"type %d has no lost or doubled cues at %s pacing",kinds[k],test==0?"30 FPS":test==1?"60 FPS":"uneven frame");
  C(label,events==(markers[k][0]==markers[k][1]?4:8));
 }
 SoundActor state={0};int normal=0,slow=0;float end=duration*4;
 for(int n=1;n<=1600;n++){float t=end*n/1600;normal+=chew_advance(&state,&clip,Z_NORMAL,t,end/1600);}state=(SoundActor){0};
 for(int n=1;n<=1600;n++){float t=end*n/1600;slow+=chew_advance(&state,&clip,Z_NORMAL,t*.5f,end/1600);}
 C("slower playback produces proportionally fewer cues",normal==8&&slow==4);
 state=(SoundActor){0};int initial=chew_advance(&state,&clip,Z_NORMAL,.01f,.01f);int first=chew_advance(&state,&clip,Z_NORMAL,duration*.15f,duration*.15f-.01f);
 C("entering an eating loop is silent until its first cue",initial==0&&first==1);
 int repeat=chew_advance(&state,&clip,Z_NORMAL,duration*.15f,.1f);C("a frozen animation cannot repeat a bite",repeat==0);
 state=(SoundActor){0};C("loading an old animation position cannot replay earlier loops",chew_advance(&state,&clip,Z_NORMAL,duration*100+.01f,.01f)==0);
 state=(SoundActor){.chewing=1,.clip=&clip,.chew=100};C("restarting a clip clears its old cue history",chew_advance(&state,&clip,Z_NORMAL,.01f,.01f)==0);
 Animation other=clip;C("changing animation banks cannot replay old cues",chew_advance(&state,&other,Z_NORMAL,.02f,.01f)==0);
 game_start_level(&game,9,9189);game.spawn_timer=10000;memset(game.zombies,0,sizeof(game.zombies));memset(sound_actors,0,sizeof(sound_actors));
 int id=game_spawn_zombie(&game,2,Z_NORMAL,center(3)+10);Zombie *z=&game.zombies[id];z->eating=1;
 C("ordinary plants select the original hard chomp group",zombie_chew_group(id)==FOLEY_CHOMP);
 game.plants[2][3]=(Plant){.kind=WALLNUT,.hp=4000};C("Wall-nuts select the original soft chomp group",zombie_chew_group(id)==FOLEY_CHOMP_SOFT);
 game.plants[2][3].kind=PEA;game.pumpkin_hp[2][3]=100;C("Pumpkin armor selects soft chomps",zombie_chew_group(id)==FOLEY_CHOMP_SOFT);game.pumpkin_hp[2][3]=0;game.hypnotized[id]=1;
 C("hypnotized combat selects soft chomps",zombie_chew_group(id)==FOLEY_CHOMP_SOFT);game.hypnotized[id]=0;
 game.frozen[id]=1;z->age=1;C("frozen zombies emit no new bite",zombie_chew_events(id,.1f)==0);game.frozen[id]=0;
 z->eating=0;C("walking zombies cannot emit chewing cues",zombie_chew_events(id,.1f)==0);z->eating=1;z->vault=1;
 C("vaulting cannot trigger chewing cues",zombie_chew_events(id,.1f)==0);z->vault=0;z->hp=30;
 C("headless zombies cannot emit chewing cues",zombie_chew_events(id,.1f)==0);z->hp=270;z->death=1;
 C("dead zombies cannot emit chewing cues",zombie_chew_events(id,.1f)==0);z->death=0;z->kind=Z_DUCK;z->age=.25f;game.time=100;
 C("later zombie types draw and sound from their own eating clock",fabsf(zombie_visible_time(z,&assets[A_DUCK_EAT])*assets[A_DUCK_EAT].fps-9)<.001f&&zombie_visible_clip(z,id,0)==&assets[A_DUCK_EAT]);
 z->kind=Z_PAPER;z->hp=270;z->armor=0;C("an angry Newspaper uses the same clip for drawing and sound",zombie_visible_clip(z,id,0)==&assets[A_PAPER_ANGRY_EAT]);
 z->kind=Z_BOBSLED;C("Bobsled eating uses its actual eating animation",zombie_visible_clip(z,id,0)==&assets[A_BOBSLED_EAT]);
 z->kind=Z_POLE;z->vault=1;C("the eating rate never changes a Pole Vaulter jump",zombie_visible_time(z,&assets[A_POLE_JUMP])==z->age);z->vault=0;
 z->kind=Z_GARG;C("Gargantuar attacks do not use the chewing rate",zombie_visible_time(z,&assets[A_GARG_EAT])==game.time&&!zombie_chewing_kind(z->kind));
 z->kind=Z_ZOMBONI;C("Zomboni driving does not use the chewing rate",zombie_visible_time(z,&assets[A_ZAMBONI_EAT])==game.time&&!zombie_chewing_kind(z->kind));
 z->kind=Z_CATAPULT;C("Catapult attacks do not use the chewing rate",zombie_visible_time(z,&assets[A_CATAPULT_EAT])==game.time&&!zombie_chewing_kind(z->kind));
 C("custom flag bodies keep the same eating clock as the original flag clip",avatar_assets()&&avatar_art[AV_BODY+1].frames==assets[A_FLAG_EAT].frames&&avatar_art[AV_BODY+1].fps==assets[A_FLAG_EAT].fps);
 for(int chill=0;chill<2;chill++){
  game_start_level(&game,9,9190+chill);game.level_flags|=LEVEL_READY;game.spawn_timer=10000;memset(game.zombies,0,sizeof(game.zombies));memset(sound_actors,0,sizeof(sound_actors));
  game.plants[2][3]=(Plant){.kind=WALLNUT,.hp=4000};id=game_spawn_zombie(&game,2,Z_NORMAL,center(3)+10);z=&game.zombies[id];z->slow=chill?20:0;int bites=0;
  for(int frame=0;frame<240;frame++){game_step(&game,1/60.0f);bites+=zombie_chew_events(id,1/60.0f);}
  C(chill?"chilled gameplay halves eating motion and keeps four matching cues":"normal gameplay produces seven cues at PS3 playback speed",fabsf(z->age-(chill?2:4))<.03f&&bites==(chill?4:7));
 }
 game=old;memcpy(sound_actors,saved,sizeof(saved));fprintf(out,"CHEWING RESULT %d/%d\n",pass,total);return pass==total;
 #undef C
}
