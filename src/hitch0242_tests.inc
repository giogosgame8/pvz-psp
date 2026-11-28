static int hitch0242_selftest(FILE *out){
 int pass=0,total=0,saved_screen=screen,saved_page=menu_crow_page;float saved_clock=crow_clock;
#define H(label,test) do{int ok=(test);total++;pass+=ok;fprintf(out,"hitch0242 %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 FrameTiming timing={0};
 TimingScene play={TIMING_BOARD|TIMING_ACTIVE,2,3}, moved=play;
 moved.column++;
 TimingScene pause_scene={TIMING_BOARD|TIMING_PAUSED,2,4};
 TimingScene won={TIMING_BOARD|TIMING_WON,2,4}, revealed=won, collected=won;
 revealed.flags|=TIMING_REWARD;
 collected.flags|=TIMING_COLLECTED;
 timing_present(&timing,play,(TimingFrame){play,1000000,10,3,60});
 timing_present(&timing,play,(TimingFrame){moved,1016667,10,3,60});
 timing_present(&timing,moved,(TimingFrame){moved,1050000,12,4,60});
 timing_present(&timing,moved,(TimingFrame){pause_scene,1090000,13,5,60});
 timing_present(&timing,pause_scene,(TimingFrame){pause_scene,1150000,13,5,60});
 H("frame timing separates cursor and pause spikes from ordinary play",
   timing.groups[TIMING_PLAY].samples==2&&timing.groups[TIMING_PLAY].long_frames==1&&
   timing.groups[TIMING_PLAY].reads==2&&timing.groups[TIMING_PLAY].waits==1&&
   timing.groups[TIMING_CURSOR].samples==1&&timing.groups[TIMING_CURSOR].peak_us==16667&&
   timing.groups[TIMING_PAUSE].samples==1&&timing.groups[TIMING_PAUSE].peak_us==40000);
 timing_present(&timing,pause_scene,(TimingFrame){moved,2000000,19,5,30});
 timing_present(&timing,moved,(TimingFrame){moved,2040000,19,5,30});
 timing_present(&timing,moved,(TimingFrame){won,2073300,19,5,30});
 timing_present(&timing,won,(TimingFrame){revealed,2133300,22,6,30});
 timing_present(&timing,revealed,(TimingFrame){collected,2166600,22,6,30});
 H("frame timing uses the display target and measures reward transitions individually",
   timing.groups[TIMING_PLAY].samples==3&&timing.groups[TIMING_PLAY].long_frames==1&&
   timing.groups[TIMING_VICTORY].peak_us==33300&&timing.groups[TIMING_REVEAL].peak_us==60000&&
   timing.groups[TIMING_REVEAL].reads==3&&timing.groups[TIMING_REVEAL].long_frames==1&&
   timing.groups[TIMING_PICKUP].peak_us==33300&&timing.groups[TIMING_PICKUP].long_frames==0);
 timing_write(NULL,&timing);
 H("disabling timing discards the previous timestamp and all samples",
   timing.previous.target_fps==0&&timing.groups[TIMING_PLAY].samples==0&&
   timing.groups[TIMING_PICKUP].samples==0);
 Animation probe={0};
 int loaded=animation_load(&probe,"data/pea_idle.pva");
 render_begin();
 render_anim(&probe,630,120,0,1,WHITE,1);
 render_anim_frame(&probe,-150,120,0,1,WHITE,1);
 render_clip(100,50,200,170);
 render_image(&probe,320,100,32,32,0,WHITE);
 render_clip(0,0,480,272);
 int hidden=loaded&&!animation_bank_ready(&probe)&&render_stats().draw_calls==0;
 render_image(&probe,472,100,20,20,0,WHITE);
 int edge=animation_bank_ready(&probe)&&render_stats().draw_calls==1;
 render_end();animation_free(&probe);
 H("hidden sprites skip animation loading before submission",hidden);
 H("partially visible sprites still decode and draw",edge);

 animation_cache_suspend();
 AnimationIOStats hidden_text_io=animation_io_stats();
 render_begin();disc_text(500,20,400,"Offscreen paragraph",WHITE,.8f);render_end();
 H("offscreen atlas text submits nothing and leaves its bank unloaded",
   render_stats().draw_calls==0&&!animation_bank_ready(&assets[A_DISC_FONT])&&
   hidden_text_io.sync_reads==animation_io_stats().sync_reads);
 render_begin();disc_text(9,12,460,"ABCDEFGHIJKLMNOPQRSTUVWXYZ",WHITE,.8f);render_end();
 H("26 atlas glyphs submit as one draw and one texture bind",
   render_stats().draw_calls==1&&render_stats().texture_binds==1);
 render_begin();disc_text(9,12,460,"ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz",WHITE,.8f);render_end();
 H("a two-line paragraph crosses the batch boundary in order",
   render_stats().draw_calls==2&&render_stats().texture_binds==1);
 const Animation *stamp=&assets[A_METER_HEAD];
 const TexturePart parts[]={
  {0,0,stamp->width,stamp->height,20,35,45,45},
  {0,0,stamp->width,stamp->height,50,48,45,45},
  {0,0,stamp->width,stamp->height,95,65,45,45},
  {0,0,stamp->width,stamp->height,250,250,45,45}
 };
 unsigned separate_draws;
 int captured=1;
 for(int grouped=0;grouped<2;grouped++){
  render_begin();render_clip(38,40,100,80);
  if(grouped)render_parts(stamp,0,parts,sizeof(parts)/sizeof(*parts),RGBA(255,255,255,175));
  else for(unsigned i=0;i<sizeof(parts)/sizeof(*parts);i++){
   const TexturePart *p=&parts[i];
   render_part(stamp,0,p->sx,p->sy,p->sw,p->sh,p->x,p->y,p->w,p->h,RGBA(255,255,255,175));
  }
  render_clip(0,0,480,272);render_end();
  if(!grouped)separate_draws=render_stats().draw_calls;
  captured&=render_debug_frame(grouped?"review-batch-grouped.ppm":"review-batch-separate.ppm");
 }
 H("a clipped and overlapping sprite group uses one submission instead of three",
   captured&&separate_draws==3&&render_stats().draw_calls==1&&animation_cache_ok());
 H("a focus ring uses one texture-free submission and matches the previous strip pixels",render_focus_selftest());

 render_begin();render_background(&assets[A_MENU_BG]);render_end();
 animation_cache_suspend();
 render_begin();render_background(&assets[A_MENU_BG]);render_end();
 H("a resident background draws without rereading its animation bank",
   render_stats().draw_calls==1&&!animation_bank_ready(&assets[A_MENU_BG]));
 animation_cache_suspend();shared_cursor=0;screen=0;for(int n=0;n<shared_count();n++)shared_ui_step(1);
 int retained=1;for(int n=0;n<shared_count();n++)retained&=animation_frame_retained(&assets[shared_asset(n)],shared_frame(n));
 H("pause controls, reward art and all 49 packets are retained",retained);
 /* More decoded combat pixels than the smaller profile can hold. */
 for(int n=0;n<40;n++){animation_cache_next_frame();animation_frame(&assets[A_BOSS_DEATH],n);}
 AnimationIOStats before=animation_io_stats();retained=1;
 for(int n=0;n<shared_count();n++){Animation *a=&assets[shared_asset(n)];int f=shared_frame(n);retained&=animation_frame_retained(a,f)&&animation_frame(a,f)!=0;}
 AnimationIOStats after=animation_io_stats();H("cache churn leaves pause and reward drawing free of file reads",retained&&before.sync_reads==after.sync_reads&&before.waits==after.waits);
 Game old_game=game,old_backup=adventure_backup;Profile old_player=player;Progress old_progress=progress;const char *pp=profile_path,*gp=adventure_path,*hp=progress_path;int old_resume=can_resume,old_replay=replaying,old_pause=paused;
 BoardActorRows actors;
 for(int i=0;i<MAX_ZOMBIES;i++){game.zombies[i].used=i%7!=0;game.zombies[i].row=i%(ROWS+2)-1;}
 for(int i=0;i<MAX_SHOTS;i++){game.shots[i].used=i%5!=0;game.shots[i].row=i%(ROWS+2)-1;}
 int order_ok=1;
 for(int rows=5;rows<=ROWS;rows++){
  board_actor_rows(&game,rows,0,&actors);
  for(int row=0;row<rows;row++){
   int zombie=actors.zombie_head[row],shot=actors.shot_head[row];
   for(int i=0;i<MAX_ZOMBIES;i++)if(game.zombies[i].used&&game.zombies[i].row==row){
    if(zombie!=i){order_ok=0;break;}zombie=actors.zombie_next[i];
   }
   for(int i=0;i<MAX_SHOTS;i++)if(game.shots[i].used&&game.shots[i].row==row){
    if(shot!=i){order_ok=0;break;}shot=actors.shot_next[i];
   }
   order_ok&=zombie==-1&&shot==-1;
  }
 }
 H("five- and six-lane boards keep visible actors in slot order",order_ok);
 board_actor_rows(&game,ROWS,1,&actors);
 int invisible_ok=1;
 for(int row=0;row<ROWS;row++)invisible_ok&=actors.zombie_head[row]==-1&&actors.shot_head[row]>=0;
 H("invisible zombies are skipped without hiding projectiles",invisible_ok);
 checkpoint_flush();profile_path="selftest-hitch-profile.dat";adventure_path="selftest-hitch-save.dat";progress_path="selftest-hitch-progress.dat";
 profile_init(&player);game_start_level(&game,9,24202);game.level_flags|=LEVEL_READY;can_resume=1;replaying=paused=help=gallery=0;screen=1;previous_buttons=0;
 SceCtrlData pad;memset(&pad,0,sizeof(pad));pad.Lx=pad.Ly=128;pad.Buttons=PSP_CTRL_START|PSP_CTRL_CROSS;checkpoint_test_delay(200);
 uint64_t start=sceKernelGetSystemTimeWide();input(&pad,.016f);unsigned pause_us=sceKernelGetSystemTimeWide()-start;
 H("Start opens pause while its checkpoint is still writing",paused&&checkpoint_busy()&&pause_us<30000);
 H("simultaneous confirmation cannot undo a newly opened pause",paused&&pause_cursor==0&&game.plants[game.cursor_row][game.cursor_col].kind<0);
 pad.Buttons=0;input(&pad,.016f);pad.Buttons=PSP_CTRL_START;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
 int first_col=game.cursor_col;pad.Buttons=PSP_CTRL_RIGHT;repeat_timer=0;start=sceKernelGetSystemTimeWide();input(&pad,.016f);unsigned cursor_us=sceKernelGetSystemTimeWide()-start;
 H("tile navigation remains responsive during a pending checkpoint",!paused&&game.cursor_col==(first_col+1)%COLS&&cursor_us<30000);
 game.status=WON;start=sceKernelGetSystemTimeWide();int queued=queue_current();unsigned reward_us=sceKernelGetSystemTimeWide()-start;
 H("winning queues a reward checkpoint without waiting for storage",queued&&reward_us<30000&&checkpoint_flush());
 fprintf(out,"HITCH INPUT TIMES: pause %u us, cursor %u us, victory enqueue %u us (200 ms injected storage delay)\n",pause_us,cursor_us,reward_us);
 const char *seed_fixture="selftest-hitch-seed-costs.dat";
 uint32_t configured[SEEDS*2]={0};
 configured[PEA*2]=125;configured[PEA*2+1]=675;
 configured[SUNFLOWER*2]=1001;configured[SUNFLOWER*2+1]=500;
 FILE *seed_file=fopen(seed_fixture,"wb");
 int seed_written=0;
 if(seed_file){
  seed_written=fwrite("PVS1",4,1,seed_file)==1&&fwrite(configured,sizeof(configured),1,seed_file)==1;
  if(fclose(seed_file))seed_written=0;
 }
 load_seed_costs(seed_fixture);
 remove(seed_fixture);
 game_start_adventure(&game,9,24203,0);game.status=WON;game.level_flags|=LEVEL_READY|LEVEL_REWARD_COLLECTED;
 reward_collect_time=REWARD_FLIGHT_DURATION;
 progress=(Progress){0};progress_visit(&progress,9,1);
 profile_apply(&player,&game);
 player.purchases[SHOP_RAKE]=2;
 int old_queued=queue_current();
 start=sceKernelGetSystemTimeWide();pause_test_press(PSP_CTRL_CROSS);
 unsigned next_us=sceKernelGetSystemTimeWide()-start;
 H("Next level opens its intro while an earlier checkpoint is pending",
   old_queued&&screen==2&&game.level==10&&game.status==PLAYING&&checkpoint_busy()&&next_us<30000);
 H("a new board uses cached seed costs and keeps defaults for invalid entries",
   seed_written&&imported_seed_costs_ready&&game.seed_cost[PEA]==125&&game.seed_refresh[PEA]==675&&
   game.seed_cost[SUNFLOWER]==50&&game.seed_refresh[SUNFLOWER]==750);
 checkpoint_test_delay(0);
 Game next_board;
 Profile next_profile;
 Progress history;
 H("the queued intro saves the board profile and replay history",checkpoint_flush()&&
   game_load(&next_board,adventure_path)&&next_board.level==10&&next_board.session_id==game.session_id&&
   profile_load(&next_profile,profile_path)&&next_profile.highest==10&&next_profile.purchases[SHOP_RAKE]==1&&
   progress_load(&history,progress_path)&&progress_played(&history,10)&&(history.completed&(UINT64_C(1)<<8)));
 game.status=LOST;game.level_flags|=LEVEL_READY;screen=1;
 checkpoint_test_delay(200);old_queued=queue_current();
 start=sceKernelGetSystemTimeWide();pause_test_press(PSP_CTRL_CROSS);
 unsigned retry_us=sceKernelGetSystemTimeWide()-start;
 H("Retry opens the same intro without waiting for the loss checkpoint",old_queued&&screen==2&&
   game.level==10&&game.status==PLAYING&&checkpoint_busy()&&retry_us<30000);
 checkpoint_test_delay(0);
 H("Retry persists a fresh board without advancing the replay history",checkpoint_flush()&&
   game_load(&next_board,adventure_path)&&next_board.level==10&&next_board.status==PLAYING&&
   (next_board.store_flags&STORE_RAKE)&&next_board.session_id==game.session_id&&
   profile_load(&next_profile,profile_path)&&!next_profile.purchases[SHOP_RAKE]&&
   progress_load(&history,progress_path)&&!progress_played(&history,11));
 fprintf(out,"HITCH LEVEL INPUT TIMES: next %u us, retry %u us (200 ms injected storage delay)\n",next_us,retry_us);

 game_start_adventure(&game,50,24204,0);game.status=WON;
 game.level_flags|=LEVEL_READY|LEVEL_REWARD_COLLECTED;
 progress_visit(&progress,50,1);screen=1;
 checkpoint_test_delay(200);
 int profile_write=checkpoint_queue(&player,profile_path,NULL,NULL,NULL,NULL);
 sceKernelDelayThread(1000);
 int final_write=queue_current();
 start=sceKernelGetSystemTimeWide();
 start_adventure_ending();
 unsigned ending_us=sceKernelGetSystemTimeWide()-start;
 H("credits open while a full checkpoint queue retains Adventure history",
   profile_write&&final_write&&screen==19&&movie_active()&&checkpoint_busy()&&ending_us<100000);
 movie_close();
 checkpoint_test_delay(0);
 H("the queued credits entry writes its marker and preserves level 5-10 history",checkpoint_flush()&&
   game_load(&next_board,adventure_path)&&next_board.level==50&&(next_board.level_flags&32)&&
   progress_load(&history,progress_path)&&progress_played(&history,50)&&(history.completed&(UINT64_C(1)<<49)));
 fprintf(out,"HITCH CREDITS ENTRY: %u us with both checkpoint slots occupied and 200 ms injected storage delay\n",ending_us);

 load_seed_costs("data/seeds.bin");
 animation_cache_suspend();
 animation_prefetch_test_delay(150);
 game_start_level(&game,9,24204);game.unlocked_mask=(UINT64_C(1)<<ALL_SEEDS)-1;
 reset_level_ui();screen=2;
 AnimationIOStats cold=animation_io_stats();
 prefetch_level_art();
 AnimationIOStats prefetch=animation_io_stats();
 H("the intro starts a single asynchronous opening-art read",
   prefetch.pending&&prefetch.async_reads==cold.async_reads+1&&prefetch.sync_reads==cold.sync_reads);
 pause_test_press(PSP_CTRL_CROSS);
 int picker=screen==3;
 game.seed_mask=(UINT64_C(1)<<PEA)|(UINT64_C(1)<<SUNFLOWER)|(UINT64_C(1)<<WALLNUT)|
   (UINT64_C(1)<<MINE)|(UINT64_C(1)<<CHERRY)|(UINT64_C(1)<<POT);
 start=sceKernelGetSystemTimeWide();pause_test_press(PSP_CTRL_START);
 unsigned begin_us=sceKernelGetSystemTimeWide()-start;
 AnimationIOStats confirmed=animation_io_stats();
 H("confirming a lineup enters play without synchronous artwork reads or a prefetch wait",
   picker&&screen==1&&(game.level_flags&LEVEL_READY)&&confirmed.sync_reads==cold.sync_reads&&
   confirmed.waits==cold.waits&&begin_us<30000);
 animation_prefetch_test_delay(0);
 int loaded_openers=0;
 for(int step=0;step<1000;step++){
  prefetch_level_art();
  if(animation_bank_ready(&assets[A_MOWER])&&animation_bank_ready(&assets[A_SEEDBANK])&&
     animation_bank_ready(&assets[A_PEA_BODY])&&animation_bank_ready(&assets[A_PEA_HEAD])&&
     animation_bank_ready(&assets[A_SUNFLOWER])&&animation_bank_ready(&assets[A_ZOMBIE])){
   loaded_openers=1;break;
  }
  sceKernelDelayThread(1000);
 }
 AnimationIOStats staged=animation_io_stats();
 H("opening artwork arrives through the cache within one quarter of its bank budget",
   loaded_openers&&staged.sync_reads==cold.sync_reads&&level_art_bytes<=staged.limit/4);
 H("the prefetch skips unlocked plants left out of the lineup",
   !level_art_attempted[A_COB]&&!animation_bank_ready(&assets[A_COB]));
 fprintf(out,"HITCH SEED PICKER INPUT: %u us with a 150 ms pending artwork read (%u async reads, %u bytes)\n",
   begin_us,(unsigned)(staged.async_reads-cold.async_reads),(unsigned)level_art_bytes);
 render_begin();draw_board();draw_hud(performance.target_fps);render_end();
 H("the prefetched opening lawn renders without a cache fault",
   render_stats().draw_calls>0&&animation_cache_ok()&&render_debug_frame("review-opening-art.ppm"));
 AnimationIOStats cursor_before=animation_io_stats();
 SceCtrlData cursor_pad={.Lx=128,.Ly=128};
 int distinct_tiles=0;
 for(int step=0;step<game_rows(&game)*COLS;step++){
  int prev_col=game.cursor_col,prev_row=game.cursor_row;
  cursor_pad.Buttons=PSP_CTRL_RIGHT|(step%COLS==COLS-1?PSP_CTRL_DOWN:0);
  repeat_timer=0;move_cursor(&cursor_pad,.016f);
  distinct_tiles+=game.cursor_col!=prev_col||game.cursor_row!=prev_row;
  render_begin();draw_board();draw_hud(performance.target_fps);render_end();
 }
 AnimationIOStats cursor_after=animation_io_stats();
 H("moving across the warmed lawn draws every cursor position without a graphics read or wait",
   distinct_tiles==game_rows(&game)*COLS&&cursor_after.sync_reads==cursor_before.sync_reads&&
   cursor_after.waits==cursor_before.waits&&animation_cache_ok());
 int staged_fire=0;
 for(int step=0;step<1000;step++){
  prefetch_level_art();
  if(animation_bank_ready(&assets[A_PEA_FIRE])&&animation_bank_ready(&assets[A_PROJECTILE_PEA])){
   staged_fire=1;break;
  }
  sceKernelDelayThread(1000);
 }
 AnimationIOStats prefire=animation_io_stats();
 H("the opening lineup prefetch includes the Peashooter's firing pose and projectile",
   staged_fire&&prefire.sync_reads==cursor_after.sync_reads&&level_art_bytes<=prefire.limit/4);
 game.sun=1000;
 int planted=game_place(&game,2,2,PEA)==0;
 game.pea_shoot_time[2][2]=.34f;
 game.shots[0]=(Shot){.used=1,.row=2,.ice=0,.x=center(2)+22};
 game.shot_y[0]=baseline(2)-19;
 render_begin();draw_board();draw_hud(performance.target_fps);render_end();
 AnimationIOStats first_fire=animation_io_stats();
 H("the first Peashooter shot renders without a synchronous graphics read or wait",
   planted&&staged_fire&&first_fire.sync_reads==prefire.sync_reads&&first_fire.waits==prefire.waits&&animation_cache_ok());
 checkpoint_flush();
 for(int i=0;i<3;i++){const char *path=(const char*[]){profile_path,adventure_path,progress_path}[i];char backup[160];remove(path);snprintf(backup,sizeof(backup),"%s.bak",path);remove(backup);}
 profile_path=pp;adventure_path=gp;progress_path=hp;game=old_game;adventure_backup=old_backup;player=old_player;progress=old_progress;can_resume=old_resume;replaying=old_replay;paused=old_pause;previous_buttons=0;screen=0;
 menu_crow_page=-1;menu_motion_step(3,0);float main_x=crow_x,main_y=crow_y;screen=6;menu_motion_step(.01f,0);
 H("changing menu pages starts flight from the current perch",menu_crow_page==1&&fabsf(crow_from_x-main_x)<.01f&&fabsf(crow_from_y-main_y)<.01f&&crow_clock<1.1f);
 menu_motion_step(3,0);H("the crow lands on the Mini-games gravestone",fabsf(crow_x-235)<.01f&&fabsf(crow_y-114)<.01f);
 screen=11;menu_motion_step(3,0);H("the crow perches on the Extras glass orb",menu_crow_page==2&&fabsf(crow_x-112)<.01f&&fabsf(crow_y-128)<.01f);
 screen=0;menu_motion_step(3,0);H("the crow returns to the Home gravestone",menu_crow_page==0&&fabsf(crow_x-75)<.01f&&fabsf(crow_y-125)<.01f);
 if(crow_rate){
  H("all four menu directions select distinct disc keyframe routes",crow_route_for(0,1)==2&&crow_route_for(1,0)==3&&crow_route_for(0,2)==4&&crow_route_for(2,0)==5);
  int endpoints=1;const int destinations[6]={0,2,1,0,2,0};
  for(int route=0;route<CROW_ROUTE_COUNT;route++){float x,y;crow_destination(destinations[route],&x,&y);float last=crow_routes[route].frames-1;endpoints&=crow_position(route,0,last)==x&&crow_position(route,1,last)==y;}
  H("all six disc paths end exactly at the port's perches",endpoints);
  int distinct=1;for(int outward=2;outward<=4;outward+=2){
   float difference=0,last=crow_routes[outward].frames-1,return_last=crow_routes[outward+1].frames-1;
   for(int point=1;point<11;point++){
    float progress=point/11.0f;
    float dx=crow_position(outward,0,progress*last)-crow_position(outward+1,0,(1-progress)*return_last);
    float dy=crow_position(outward,1,progress*last)-crow_position(outward+1,1,(1-progress)*return_last);
    difference=fmaxf(difference,hypotf(dx,dy));
   }
   distinct&=difference>5;
  }
  H("original return paths differ from reversed outward paths",distinct);
  menu_motion_step(2,0);screen=11;menu_motion_step(.33f,0);float interrupted_x=crow_x,interrupted_y=crow_y;
  screen=0;menu_motion_step(0,0);H("reversing midway continues from the visible crow position",fabsf(crow_x-interrupted_x)<.01f&&fabsf(crow_y-interrupted_y)<.01f&&crow_route==5);
  menu_motion_step(2,0);
  FILE *invalid=fopen("selftest-crow-paths.bin","wb");if(invalid){fputs("PVCR",invalid);fclose(invalid);}
  H("invalid optional paths leave the built-in crow usable",invalid&&!menu_crow_load_routes("selftest-crow-paths.bin")&&crow_route_for(0,1)<0);
  remove("selftest-crow-paths.bin");H("valid disc paths reload after a bad file",menu_crow_load_routes("data/crow-paths.bin"));
 }
 screen=saved_screen;menu_crow_page=saved_page;crow_clock=saved_clock;shared_cursor=0;
#undef H
 fprintf(out,"HITCH0242 RESULT %d/%d\n",pass,total);return pass==total;
}
