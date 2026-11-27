static int presentation_selftest(FILE *out){
 Game original=game;int old_screen=screen,old_bars=health_bars,old_row=menu_volume_row,old_replaying=replaying;const char *old_settings=ui_settings_path;ui_settings_path="selftest-ui-settings.dat";
 int pass=0,total=0;SceCtrlData pad;memset(&pad,0,sizeof(pad));pad.Lx=pad.Ly=128;previous_buttons=0;help=gallery=0;
 #define FIX(label,test) do{int fixed_ok=(test);pass+=fixed_ok;total++;fprintf(out,"presentation %s: %s\n",label,fixed_ok?"PASS":"FAIL");}while(0)
 #define KEY(button) do{pad.Buttons=0;input(&pad,.016f);pad.Buttons=button;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);}while(0)
 const char *backup="selftest-ui-settings.dat.bak";remove(ui_settings_path);remove(backup);
 for(int i=0;i<4;i++){health_bars=i;int ok=ui_settings_save();health_bars=(i+1)%4;FIX("each health-bar preference survives a restart",ok&&ui_settings_load()&&health_bars==i);}
 FILE *bad=fopen(ui_settings_path,"wb");if(bad){fputs("bad data",bad);fclose(bad);}health_bars=0;FIX("invalid preferences recover the previous settings",bad!=NULL&&ui_settings_load()&&health_bars==2);
 uint32_t previous[3]={0x49555650,1,3};bad=fopen(ui_settings_path,"wb");
 int previous_written=bad&&fwrite(previous,sizeof(previous),1,bad)==1;if(bad&&fclose(bad))previous_written=0;
 FIX("version 1 preferences still load from the primary",previous_written&&ui_settings_load()&&health_bars==3&&classic_dancer==0);
 bad=fopen(ui_settings_path,"ab");int trailing=bad&&fputc(0,bad)!=EOF;if(bad&&fclose(bad))trailing=0;
 FIX("partial trailing bytes cannot override the backup",trailing&&ui_settings_load()&&health_bars==2);
 const char *view="selftest-challenge-view.dat",*view_backup="selftest-challenge-view.dat.bak";
 remove(view);remove(view_backup);
 uint32_t viewed[6]={0x184c4843,1,1,3,7,15},loaded[6];
 int first=record_replace(view,viewed,sizeof(viewed),challenge_seen_valid);viewed[2]=2;
 int second=record_replace(view,viewed,sizeof(viewed),challenge_seen_valid);
 int deleted=remove(view)==0;
 FIX("missing challenge-view data recovers the previous masks",first&&second&&deleted&&
     record_load(view,loaded,challenge_seen_read)&&loaded[2]==1&&loaded[3]==3&&loaded[4]==7&&loaded[5]==15);
 remove(view_backup);
 game_start_level(&game,10,3699);game.packet_count=0;reset_level_ui();
 game.packet_count=MAX_PACKETS;game.selected=MAX_PACKETS-1;update_level_ui(.016f);
 int conveyor_fits=packet_x[0]>=44&&packet_x[game.selected]+29<=430;
 for(int n=1;n<MAX_PACKETS;n++)conveyor_fits&=packet_x[n]-packet_x[n-1]>=36.99f;
 FIX("a full conveyor has no hidden or overlapping selection",conveyor_fits);
 game_start_level(&game,36,3700);
 int matching_light=1;
 for(int source=0;source<ROWS*COLS;source++){
  int row=source/COLS,col=source%COLS;
  game.plants[row][col].kind=PLANTERN;
  for(int source_rows=5;source_rows<=ROWS;source_rows++){
   unsigned lit[ROWS];lantern_coverage(&game,source_rows,lit);
   for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++){
    int expected=row<source_rows&&abs(row-r)<=1&&abs(col-c)<=1;
    matching_light&=!!(lit[r]&(1u<<c))==expected;
   }
  }
  game.plants[row][col].kind=-1;
 }
 FIX("fog and vase lighting preserve all tile boundaries",matching_light);
 game.plants[0][0].kind=game.plants[0][8].kind=game.plants[5][4].kind=PLANTERN;
 unsigned lit[ROWS];lantern_coverage(&game,ROWS,lit);
 FIX("separate lanterns combine without crossing board edges",lit[0]==0x183&&lit[1]==0x183&&lit[2]==0&&lit[4]==0x38&&lit[5]==0x38);
 screen=4;menu_volume_row=3;health_bars=0;KEY(PSP_CTRL_CROSS);FIX("X cycles health bars in Options",health_bars==1&&screen==4);KEY(PSP_CTRL_LEFT);FIX("Left disables health bars",health_bars==0);KEY(PSP_CTRL_RIGHT);FIX("Right enables plant health bars",health_bars==1);
 game_start_puzzle(&game,0,0,3700);reset_level_ui();game.packet_count=MAX_PACKETS;for(int i=0;i<MAX_PACKETS;i++)game.packets[i]=PEA;update_level_ui(.016f);int spaced=1;for(int i=1;i<MAX_PACKETS;i++)spaced&=packet_x[i]-packet_x[i-1]>=30;FIX("simultaneously released Vasebreaker packets do not overlap",spaced);FIX("Vasebreaker packets fit inside the bank",packet_x[MAX_PACKETS-1]+29<=430);
 game_start_level(&game,9,3701);game.spawned=1;game.wave=2;game.num_waves=4;game.spawn_timer=20;game.wave_health_threshold=50;game_spawn_zombie(&game,2,Z_NORMAL,400);reset_level_ui();float start=wave_meter_target();game.spawn_timer=10;float half=wave_meter_target();FIX("wave meter advances continuously as the wave timer runs",half>start&&half<.5f);game.spawn_timer=0;FIX("wave timer reaches the next flag without skipping waves",wave_meter_target()>half&&wave_meter_target()<=.5f);
 game.wave=4;game.spawn_timer=10;for(int i=0;i<MAX_ZOMBIES;i++)game.zombies[i].used=0;int z=game_spawn_zombie(&game,2,Z_NORMAL,400);meter_wave=-1;start=wave_meter_target();game.zombies[z].hp*=.5f;FIX("final wave meter advances as zombies take damage",wave_meter_target()>start);game.status=WON;FIX("completed level fills the whole wave meter",wave_meter_target()==1);
 game_start_level(&game,9,3702);game.sun=10000;game.seed_mask=UINT64_MAX;game.spawn_timer=100000;game_place(&game,2,3,PEA);z=game_spawn_zombie(&game,2,Z_BUCKET,400);game.plants[2][3].attack=0;int idle_resumed=0;for(int i=0;i<80;i++){float before=game.burst[2][3],action=game.pea_shoot_time[2][3];game_step(&game,1.0f/60);if(action>0&&game.pea_shoot_time[2][3]==0){idle_resumed=game.burst[2][3]>.1f&&game.burst[2][3]<.16f;float end=game.burst[2][3];game_step(&game,1.0f/60);idle_resumed&=fabsf(game.burst[2][3]-end-1.0f/60)<.001f;break;}(void)before;}FIX("shooter returns to a continuous idle phase after firing",idle_resumed);
 game_start_level(&game,29,3703);game.sun=10000;game.seed_mask=UINT64_MAX;game.spawn_timer=100000;game_place(&game,0,3,SQUASH);z=game_spawn_zombie(&game,0,Z_GARG,center(3)+25);int neighbour=game_spawn_zombie(&game,1,Z_NORMAL,center(3)+25);for(int i=0;i<88;i++)game_step(&game,1.0f/60);int saved=game_save(&game,"selftest-squash.dat");Game disk;FIX("airborne Squash attack survives saving and loading",saved&&game_load(&disk,"selftest-squash.dat")&&disk.burst_left[0][3]==3&&disk.pea_shoot_time[0][3]==game.pea_shoot_time[0][3]);game=disk;float hp=game.zombies[z].hp;for(int i=0;i<45;i++)game_step(&game,1.0f/60);FIX("Squash applies one landing hit and preserves adjacent lanes",game.zombies[z].hp==hp-1800&&game.zombies[neighbour].hp==270);game.status=WON;for(int i=0;i<70;i++)game_animate_attacks(&game,1.0f/60);FIX("Squash finishes flattening even after winning",game.plants[0][3].kind==-1);
 game_start_level(&game,1,3704);game.status=WON;z=game_spawn_zombie(&game,2,Z_NORMAL,311);game.zombies[z].death=.2f;game.zombies[z].age=0;reset_level_ui();update_reward(.01f);FIX("reward stays at the defeated zombie position instead of snapping to a tile",fabsf(reward_x-311)<.01f);
 game_start_mini(&game,MINI_BOWLING,3705);game.status=WON;screen=1;replaying=1;reset_level_ui();update_reward(1);KEY(PSP_CTRL_CROSS);FIX("mini-game trophy is collected before advancing",screen==1&&(game.level_flags&LEVEL_REWARD_COLLECTED)&&reward_collect_time==0);KEY(PSP_CTRL_CROSS);FIX("trophy collection flight cannot be skipped",screen==1);update_reward(REWARD_FLIGHT_DURATION);KEY(PSP_CTRL_CROSS);board_exit_test_wait();FIX("trophy Continue returns to mini-game selection",screen==7);
 game_start_puzzle(&game,0,9,3706);game.extra.stage=9;game.extra.pending=2;screen=1;reset_level_ui();update_reward(1);game.cursor_row=2;game.cursor_col=4;KEY(PSP_CTRL_CROSS);FIX("endless milestone trophy opens its award before the next board",game.extra.stage==9&&(game.level_flags&LEVEL_REWARD_COLLECTED));KEY(PSP_CTRL_CROSS);FIX("endless trophy flight waits for completion",game.extra.stage==9);update_reward(REWARD_FLIGHT_DURATION);KEY(PSP_CTRL_CROSS);FIX("endless trophy Continue opens a fresh uncollected board",game.extra.stage==10&&!game.extra.pending&&!(game.level_flags&LEVEL_REWARD_COLLECTED));
 remove("mode-save.dat");replaying=old_replaying;remove(ui_settings_path);remove(backup);remove("selftest-squash.dat");ui_settings_path=old_settings;health_bars=old_bars;ui_settings_dirty=0;game=original;screen=old_screen;menu_volume_row=old_row;previous_buttons=0;reset_level_ui();
 #undef FIX
 #undef KEY
 fprintf(out,"PRESENTATION RESULT %d/%d\n",pass,total);return pass==total;
}
