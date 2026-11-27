{
#define RESET024(level) do{game_start_level(&g,level,2400);g.spawn_timer=g.sky_timer=10000;g.level_flags|=LEVEL_READY;g.shovel_tutorial=0;memset(g.zombies,0,sizeof(g.zombies));}while(0)
 RESET024(9);CHECK("024 repeat Adventure keeps its separate increasing point budget",wave_budget024(&g,12)==5);g.completion.adventure_run=1;CHECK("024 repeat Adventure raises the later wave budget",wave_budget024(&g,12)==5&&wave_budget024(&g,18)==8);
 game_start_survival(&g,0,SURVIVAL_ENDLESS,2401);g.extra.stage=1;CHECK("024 Endless does not add ten artificial waves to the budget",wave_budget024(&g,0)==9);
 game_start_mini(&g,MINI_COLUMN,2402);int ladders=0,jacks=0;for(int n=0;n<g.wave_count[5];n++)ladders+=g.wave_types[5][n]==Z_LADDER;for(int n=0;n<g.wave_count[8];n++)jacks+=g.wave_types[8][n]==Z_JACK;CHECK("024 Column Like You See Em includes its fixed Ladder and Jack groups",ladders>=10&&jacks>=10);
 RESET024(9);int a=game_spawn_zombie(&g,2,Z_DOOR,400);CHECK("024 shield health contributes only one fifth toward wave acceleration",fabsf(wave_health024(&g,&g.zombies[a])-(270+1100*.2f))<.01f);
 g.hypnotized[a]=1;CHECK("024 friendly zombies do not hold up the next wave",wave_health024(&g,&g.zombies[a])==0);
 RESET024(9);g.wave=9;g.spawned=1;g.spawn_timer=45;g.wave_health_threshold=0;g.wave_delay=4;advance(&g,3);
 CHECK("PS3 cleared pre-flag wave keeps its scheduled countdown",g.wave==9&&g.spawn_timer>41.9f&&!g.huge_wave_announced);
 g.spawn_timer=.06f;advance(&g,.1f);CHECK("pre-flag timer still opens the warning at its scheduled time",g.wave==9&&g.huge_wave_timer>7);
 advance(&g,7.6f);CHECK("the wave arrives after its warning finishes",g.wave==10&&!g.huge_wave_timer);
 RESET024(9);g.spawned=1;g.wave_delay=4.1f;g.spawn_timer=20;g.wave_health_threshold=120.75f;
 a=game_spawn_zombie(&g,2,Z_NORMAL,400);g.zombies[a].hp=120;advance(&g,.1f);
 CHECK("PS3 health equal to the truncated threshold leaves normal timing",g.spawn_timer>19.8f);
 g.zombies[a].hp=119;advance(&g,.1f);CHECK("PS3 health below threshold starts the two-second countdown",g.spawn_timer<=2&&g.spawn_timer>1.8f);
 RESET024(29);g.wave=g.num_waves;g.spawned=1;g.polish024.rise_state=1;g.polish024.rise_delay=2;advance(&g,1.8f);CHECK("024 clearing a final pool wave cannot skip delayed arrivals",g.status==PLAYING&&game_alive(&g)==0&&wave_arrivals_pending024(&g));advance(&g,.3f);int water=0;for(int i=0;i<MAX_ZOMBIES;i++)water+=g.zombies[i].used&&game_water_row(&g,g.zombies[i].row);CHECK("024 final pool arrivals appear in three distinct water cells",water==3&&g.polish024.pool_arrivals==3&&!wave_arrivals_pending024(&g));
 advance(&g,.6f);int emerged=1;for(int i=0;i<MAX_ZOMBIES;i++)if(g.zombies[i].used)emerged&=!g.polish.action[i]&&g.polish024.altitude[i]==0;CHECK("024 pool arrivals complete their rise before walking",emerged);
 RESET024(49);g.polish024.rise_state=1;g.polish024.rise_delay=0;wave_arrivals024(&g,.01f);int carriers=0,children=0;for(int i=0;i<MAX_ZOMBIES;i++)if(g.zombies[i].used){if(g.zombies[i].kind==Z_BUNGEE){carriers++;g.fidelity.actor_aux[i]=.05f;}else children+=g.polish.action[i]==ACTION_SKY_DROP;}CHECK("024 roof arrivals use three linked Bungee passengers",carriers==3&&children==3);advance(&g,.15f);int landed=0;for(int i=0;i<MAX_ZOMBIES;i++)landed+=g.zombies[i].used&&g.zombies[i].kind!=Z_BUNGEE&&!g.polish.action[i];CHECK("024 roof passengers detach on landing without starting a theft",landed==3);
 RESET024(29);g.wave=6;int seen_water=0;for(int n=0;n<80;n++){int row=choose_row(&g,Z_CONE);seen_water|=game_water_row(&g,row);}CHECK("024 ordinary armoured zombies can use pool lanes after wave five",seen_water);g.wave=5;int early_water=0;for(int n=0;n<80;n++)early_water|=game_water_row(&g,choose_row(&g,Z_CONE));CHECK("024 ordinary zombies do not enter pool lanes during the opening five waves",!early_water);
 RESET024(18);a=game_spawn_zombie(&g,2,Z_DANCER,330);int b=game_spawn_zombie(&g,2,Z_DANCER,334);g.special[a]=g.special[b]=0;advance(&g,.6f);int group_a=0,group_b=0;for(int i=0;i<MAX_ZOMBIES;i++)if(g.zombies[i].used&&g.zombies[i].kind==Z_BACKUP){group_a+=g.polish024.leader_id[i]==g.fidelity.actor_id[a];group_b+=g.polish024.leader_id[i]==g.fidelity.actor_id[b];}CHECK("024 nearby dancers retain four separate backup slots each",group_a==4&&group_b==4);
 int child=-1;for(int i=0;i<MAX_ZOMBIES;i++)if(g.zombies[i].kind==Z_BACKUP&&g.polish024.leader_id[i]==g.fidelity.actor_id[a]){child=i;break;}g.zombies[a].used=0;int replacement=game_spawn_zombie(&g,2,Z_NORMAL,400);advance(&g,1.6f);CHECK("024 a recycled leader slot cannot adopt another dancer's backups",replacement==a&&child>=0&&g.polish024.dancer_state[child]==DANCE_ORPHAN);
 {
  RESET024(29);
  int leader=game_spawn_zombie(&g,1,Z_DANCER,330);
  unsigned land_slots=(1u<<0)|(1u<<2)|(1u<<3);
  int correct_lanes=dancer_open_slots(&g,leader)==land_slots&&dancer_summon024(&g,leader)&&!dancer_open_slots(&g,leader);
  CHECK("dancer formation excludes the neighboring pool lane",
        correct_lanes&&g.total_spawned==4&&g.zombies[1].row==0&&g.zombies[2].row==1&&g.zombies[3].row==1);
  g.hypnotized[2]=1;g.zombies[3].death=1;
  CHECK("dancer formation can replace hypnotized and dying backups",
        dancer_open_slots(&g,leader)==((1u<<2)|(1u<<3)));
  for(int i=4;i<MAX_ZOMBIES-1;i++)game_spawn_zombie(&g,0,Z_NORMAL,440);
  int waiting=!dancer_summon024(&g,leader)&&dancer_open_slots(&g,leader)==(1u<<3);
  g.zombies[4].used=0;
  CHECK("crowded dancer groups fill remaining slots in order when space reopens",
        waiting&&dancer_summon024(&g,leader)&&!dancer_open_slots(&g,leader)&&
        g.polish024.dancer_slot[MAX_ZOMBIES-1]==2&&g.polish024.dancer_slot[4]==3);
 }
 RESET024(49);a=game_spawn_zombie(&g,2,Z_GARG,330);g.zombies[a].hp=1000;advance(&g,1.2f);int imp=-1;for(int i=0;i<MAX_ZOMBIES;i++)if(g.zombies[i].used&&g.zombies[i].kind==Z_IMP)imp=i;float x=imp>=0?g.zombies[imp].x:0,height=imp>=0?g.polish024.altitude[imp]:0;advance(&g,.1f);CHECK("024 thrown Imp follows a continuous airborne arc",imp>=0&&height>0&&g.zombies[imp].x<x&&g.polish024.altitude[imp]>=0);int state_saved=game_save(&g,"polish024-flight.dat");CHECK("024 ballistic state survives save and load",state_saved&&game_load(&restored,"polish024-flight.dat")&&restored.polish024.altitude[imp]==g.polish024.altitude[imp]&&restored.polish024.velocity_y[imp]==g.polish024.velocity_y[imp]);
 RESET024(50);g.time=2;g.boss_timer=g.extra.boss_head_time=g.extra.boss_stomp_delay=g.extra.boss_bungee_delay=10000;g.boss_ball_kind=1;g.boss_ball_row=2;g.boss_ball_x=GRID_X-170*.475f;g.polish024.boss_ball_age=2;boss_step(&g,1);CHECK("024 a boss ball rolls offscreen without instantly ending the level",g.status==PLAYING&&!g.boss_ball_kind&&g.mower_used[2]);
 RESET024(50);g.time=2;g.boss_hp=20000;g.extra.boss_stomps=1;g.extra.boss_head_time=0;g.extra.boss_stomp_delay=0;put_plant(&g,2,7,CABBAGE);boss_step(&g,.01f);CHECK("024 a due boss stomp precedes a due head attack",g.extra.boss_action==B_STOMP&&(g.boss_row==1||g.boss_row==2));
 RESET024(50);g.time=2;g.boss_hp=20000;g.extra.boss_stomps=1;g.extra.boss_head_time=100;g.extra.boss_stomp_delay=0;boss_step(&g,.01f);CHECK("024 Zomboss avoids stomping an empty right half",g.extra.boss_action==B_IDLE);
 RESET024(50);boss_set(&g,B_BUNGEE);g.extra.boss_action_time=.79f;boss_step(&g,.02f);CHECK("024 boss Bungee animation waits for linked carrier state",g.extra.boss_action==B_BUNGEE_DROP);for(int n=0;n<3;n++){int i=actor_by_id024(&g,g.polish024.boss_bungee_id[n]);if(i>=0)g.zombies[i].used=0;}boss_step(&g,.02f);CHECK("024 defeated boss Bungees trigger the authored exit",g.extra.boss_action==B_BUNGEE_LEAVE);boss_step(&g,1);CHECK("024 boss returns to idle with a real recovery pause",g.extra.boss_action==B_IDLE&&g.polish024.boss_idle>=1);
 {
  RESET024(50);
  for(int i=0;i<MAX_ZOMBIES;i++)game_spawn_zombie(&g,i%5,Z_NORMAL,440);
  boss_set(&g,B_BUNGEE);g.extra.boss_action_time=.79f;boss_step(&g,.02f);
  int waiting=g.extra.boss_action==B_BUNGEE&&g.total_spawned==MAX_ZOMBIES;
  boss_step(&g,1.0f);
  CHECK("boss leaves a blocked Bungee attack after waiting for slots",
        waiting&&g.extra.boss_action==B_BUNGEE_LEAVE&&g.total_spawned==MAX_ZOMBIES);

  RESET024(50);
  for(int i=0;i<MAX_ZOMBIES-2;i++)game_spawn_zombie(&g,i%5,Z_NORMAL,440);
  boss_set(&g,B_BUNGEE);g.extra.boss_action_time=.79f;boss_step(&g,.02f);
  waiting=g.extra.boss_action==B_BUNGEE&&g.total_spawned==MAX_ZOMBIES-2;
  g.zombies[0].used=0;boss_step(&g,.02f);
  int linked[3],team=g.extra.boss_action==B_BUNGEE_DROP&&g.total_spawned==MAX_ZOMBIES+1;
  for(int n=0;n<3;n++){
   linked[n]=actor_by_id024(&g,g.polish024.boss_bungee_id[n]);
   team=team&&linked[n]>=0&&g.zombies[linked[n]].kind==Z_BUNGEE;
  }
  team=team&&linked[0]!=linked[1]&&linked[0]!=linked[2]&&linked[1]!=linked[2];
  CHECK("boss deploys all three Bungees when enough slots reopen",waiting&&team);
  int staggered=0;
  if(team){
   g.fidelity.bungee_phase[linked[0]]=3;g.zombies[linked[1]].death=1;
   boss_step(&g,.02f);staggered=g.extra.boss_action==B_BUNGEE_DROP;
   g.fidelity.bungee_phase[linked[2]]=4;boss_step(&g,.02f);
  }
  CHECK("boss waits for the last surviving Bungee before leaving",staggered&&g.extra.boss_action==B_BUNGEE_LEAVE);

  RESET024(29);g.wave=2;g.wave_count[1]=1;g.wave_types[1][0]=Z_BOBSLED;g.polish.wave_pending=1;
  for(int i=0;i<MAX_ZOMBIES-3;i++)game_spawn_zombie(&g,i%2,Z_NORMAL,440);
  finish_wave_entries(&g);
  waiting=g.polish.wave_pending&&g.polish.wave_entry==0&&g.total_spawned==MAX_ZOMBIES-3;
  g.zombies[0].used=0;finish_wave_entries(&g);
  int riders=0;
  for(int i=0;i<MAX_ZOMBIES;i++)if(g.zombies[i].used&&g.zombies[i].kind==Z_BOBSLED&&g.polish024.leader_id[i]==g.fidelity.actor_id[0])riders++;
  CHECK("crowded waves reserve four slots for a complete Bobsled team",waiting&&!g.polish.wave_pending&&riders==4);
 }
 game_start_mini(&g,MINI_HEAVY,2403);g.spawn_timer=g.sky_timer=10000;a=game_spawn_zombie(&g,0,Z_NORMAL,200);g.mini.hostile_y[a]=170;g.shots[0]=(Shot){1,0,11,195};g.shot_y[0]=game_ground_y(&g,0)-18;g.shot_target[0]=a;g.polish024.shot_actor[0]=g.fidelity.actor_id[a];chapter_shots(&g,.01f);CHECK("024 Heavy Weapon homing uses the visible enemy position",g.zombies[a].hp==250&&!g.shots[0].used);
 RESET024(9);a=game_spawn_zombie(&g,2,Z_NORMAL,400);uint32_t former=g.fidelity.actor_id[a];g.zombies[a].used=0;game_spawn_zombie(&g,2,Z_NORMAL,450);b=game_spawn_zombie(&g,2,Z_NORMAL,200);g.shots[0]=(Shot){1,2,11,195};g.shot_y[0]=game_ground_y(&g,2)-18;g.shot_target[0]=a;g.polish024.shot_actor[0]=former;chapter_shots(&g,.01f);CHECK("024 a recycled actor slot cannot retain a homing lock",g.zombies[a].hp==270&&g.zombies[b].hp==250);
 game_start_mini(&g,MINI_PORTAL,2404);g.spawn_timer=10000;int cell=g.mini.portals[0];a=game_spawn_zombie(&g,cell/10,Z_POLE,plant_x(cell%10));g.zombies[a].vault=.5f;g.zombies[a].vault_from=g.zombies[a].x;g.zombies[a].vault_to=g.zombies[a].x-50;int row=g.zombies[a].row;float before=g.zombies[a].x;mini_portals(&g,.01f);CHECK("024 a portal translates a mid-vault trajectory without snapping it back",(g.zombies[a].row!=row||g.zombies[a].x!=before)&&g.zombies[a].vault_from==g.zombies[a].x&&fabsf(g.zombies[a].vault_to-(g.zombies[a].x-50))<.001f);
 int exit_cell=g.extra.last_portal_zombie[a];g.mini.portal_timer=0;mini_portals(&g,.01f);
 CHECK("repositioning a portal does not bounce a zombie back through its exit",g.extra.last_portal_zombie[a]==exit_cell&&g.zombies[a].row==(exit_cell-1)/10);
 game_start_mini(&g,MINI_ZOMBOTANY,2440);g.spawn_timer=10000;put_plant(&g,2,4,PEA);put_plant(&g,2,4,PUMPKIN);
 g.mini.enemy_used[0]=1;g.mini.enemy_x[0]=plant_x(4)+14;g.mini.enemy_y[0]=game_ground_y(&g,2)-18;mini_pre_step(&g,.03f);
 CHECK("a Zombotany pea hits Pumpkin armor before the plant it protects",!g.mini.enemy_used[0]&&g.plants[2][4].hp==300&&g.pumpkin_hp[2][4]==3980);
 a=game_spawn_zombie(&g,2,Z_NORMAL,350);g.zombies[a].hp=1;g.mini.zombie_heads[a]=1;g.mini.zombie_attack[a]=0;mini_zombie(&g,a,.02f);
 CHECK("headless Zombotany shooters stop firing",!g.mini.enemy_used[0]);
 int bonus_ok=1;for(int terrain=0;terrain<5;terrain++){game_start_survival(&g,terrain,SURVIVAL_ENDLESS,2441+terrain);g.extra.stage=25;g.extra.flags=50;survival_pick(&g);extra_replan(&g);bonus_ok&=game_world(&g)==terrain&&g.extra.goal==0&&g.wave_count[0]>0&&g.wave_count[19]>0;}
 CHECK("late Endless rounds generate legal waves on all five terrains",bonus_ok);
 g.extra.stage=INT32_MAX;extra_replan(&g);
 int bounded=1;
 for(int w=0;w<g.num_waves;w++)bounded&=g.wave_count[w]==WAVE_CAPACITY;
 CHECK("extreme Endless rounds fill the wave buffer without overflowing the budget",bounded);
 RESET024(9);g.polish024.rise_state=2;g.polish024.rise_delay=.2f;g.polish024.pool_arrivals=2;g.polish024.pool_used=3;CHECK("024 delayed arrivals persist across a save",game_save(&g,"polish024-rise.dat")&&game_load(&restored,"polish024-rise.dat")&&restored.polish024.rise_state==2&&restored.polish024.pool_used==3);
 remove("polish024-flight.dat");remove("polish024-flight.dat.bak");remove("polish024-rise.dat");remove("polish024-rise.dat.bak");
 game_start_mini(&g,MINI_LASTSTAND,2405);int opening_budget=wave_budget024(&g,0);g.mini.stage=3;CHECK("024 Last Stand increases the budget across its own stages",wave_budget024(&g,0)>opening_budget);g.wave=g.num_waves;g.spawned=1;g.polish024.rise_state=1;g.polish024.rise_delay=1;mini_progress(&g);CHECK("024 Last Stand waits for pending pool arrivals before resetting",g.mini.stage==3&&g.mini.phase==0);g.polish024.rise_state=3;mini_progress(&g);CHECK("024 a new Last Stand stage clears old arrival state",g.mini.stage==4&&!g.polish024.rise_state);
 RESET024(29);a=game_spawn_zombie(&g,0,Z_BOBSLED,400);b=game_spawn_zombie(&g,0,Z_BOBSLED,414);uint32_t sled_id=g.fidelity.actor_id[a];g.extra.bobsled_team[a]=g.extra.bobsled_team[b]=a+1;g.polish024.leader_id[a]=g.polish024.leader_id[b]=sled_id;g.zombies[a].used=0;game_spawn_zombie(&g,0,Z_BOBSLED,450);g.extra.bobsled_team[a]=a+1;g.polish024.leader_id[a]=g.fidelity.actor_id[a];bobsled_step(&g,b,.01f);CHECK("024 an old sled cannot dismount a recycled leader's new team",g.extra.bobsled_phase[b]<0&&g.extra.bobsled_phase[a]==0);
 RESET024(29);int vacant=game_spawn_zombie(&g,0,Z_NORMAL,400);
 a=game_spawn_zombie(&g,0,Z_BOBSLED,450);g.zombies[vacant].used=0;
 b=game_spawn_zombie(&g,0,Z_BOBSLED,464);
 g.extra.bobsled_team[a]=g.extra.bobsled_team[b]=a+1;
 g.polish024.leader_id[a]=g.polish024.leader_id[b]=g.fidelity.actor_id[a];
 g.extra.bobsled_position[b]=1;
 bobsled_step(&g,b,.1f);bobsled_step(&g,a,.1f);
 CHECK("a lower-slot rider stays aligned after the sled moves",b<a&&fabsf(g.zombies[b].x-g.zombies[a].x-14)<.01f&&g.zombies[b].age==g.zombies[a].age);
 RESET024(29);
 int sled[4];
 g.ice_tiles[0][0]=30;
 for(int rider=0;rider<4;rider++){
  int slot=game_spawn_zombie(&g,0,Z_BOBSLED,GRID_X+.25f+14*rider);
  sled[rider]=slot;
  g.extra.bobsled_team[slot]=sled[0]+1;
  g.polish024.leader_id[slot]=g.fidelity.actor_id[sled[0]];
  g.extra.bobsled_position[slot]=rider;
 }
 game_step(&g,.05f);
 CHECK("a sled keeps moving while the leader is on the last icy tile",
       g.zombies[sled[0]].x<GRID_X&&!g.extra.bobsled_phase[sled[0]]);
 game_step(&g,.05f);
 int dismounting=1;
 for(int rider=0;rider<4;rider++)dismounting&=g.extra.bobsled_phase[sled[rider]]<0;
 CHECK("all four riders dismount when the sled leaves the lawn even if its last tile is icy",dismounting);
 int saved_sled=game_save(&g,"polish024-sled.dat")&&game_load(&restored,"polish024-sled.dat");
 int restored_dismount=saved_sled;
 if(saved_sled)for(int rider=0;rider<4;rider++)
  restored_dismount&=restored.extra.bobsled_phase[sled[rider]]==g.extra.bobsled_phase[sled[rider]];
 if(saved_sled)advance(&restored,.9f);
 int walking=restored_dismount;
 if(saved_sled)for(int rider=0;rider<4;rider++)walking&=!restored.extra.bobsled_team[sled[rider]];
 CHECK("a team saved during dismount resumes on foot",walking);
 remove("polish024-sled.dat");remove("polish024-sled.dat.bak");
 RESET024(9);a=game_spawn_zombie(&g,2,Z_NORMAL,333);size_t prefix=offsetof(Game,polish024);uint32_t legacy[4]={0x31535650,13,prefix,checksum(&g,prefix)};FILE *legacy_file=fopen("polish024-v13.dat","wb");int wrote=legacy_file&&fwrite(legacy,sizeof(legacy),1,legacy_file)==1&&fwrite(&g,prefix,1,legacy_file)==1;if(legacy_file)fclose(legacy_file);CHECK("024 published version 13 saves import without changing their actors",wrote&&game_load(&restored,"polish024-v13.dat")&&restored.zombies[a].x==333&&restored.fidelity.actor_id[a]==g.fidelity.actor_id[a]&&!restored.polish024.rise_state);
 g.polish024.altitude[a]=NAN;remove("polish024-invalid.dat");remove("polish024-invalid.dat.bak");CHECK("024 invalid new animation clocks cannot replace a loaded game",write_board_fixture(&g,"polish024-invalid.dat")&&!game_load(&restored,"polish024-invalid.dat"));remove("polish024-invalid.dat");remove("polish024-v13.dat");
#undef RESET024
}
