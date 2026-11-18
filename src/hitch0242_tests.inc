static int hitch0242_selftest(FILE *out){
 int pass=0,total=0,saved_screen=screen,saved_page=menu_crow_page;float saved_clock=crow_clock;
#define H(label,test) do{int ok=(test);total++;pass+=ok;fprintf(out,"hitch0242 %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 animation_cache_suspend();shared_cursor=0;screen=0;for(int n=0;n<shared_count();n++)shared_ui_step(1);
 int retained=1;for(int n=0;n<shared_count();n++)retained&=animation_frame_retained(&assets[shared_asset(n)],shared_frame(n));
 H("pause controls, reward art and all 49 packets are retained",retained);
 /* More decoded combat pixels than the smaller profile can hold. */
 for(int n=0;n<40;n++){animation_cache_next_frame();animation_frame(&assets[A_BOSS_DEATH],n);}
 AnimationIOStats before=animation_io_stats();retained=1;
 for(int n=0;n<shared_count();n++){Animation *a=&assets[shared_asset(n)];int f=shared_frame(n);retained&=animation_frame_retained(a,f)&&animation_frame(a,f)!=0;}
 AnimationIOStats after=animation_io_stats();H("cache churn leaves pause and reward drawing free of file reads",retained&&before.sync_reads==after.sync_reads&&before.waits==after.waits);
 Game old_game=game,old_backup=adventure_backup;Profile old_player=player;Progress old_progress=progress;const char *pp=profile_path,*gp=adventure_path,*hp=progress_path;int old_resume=can_resume,old_replay=replaying,old_pause=paused;
 checkpoint_flush();profile_path="selftest-hitch-profile.dat";adventure_path="selftest-hitch-save.dat";progress_path="selftest-hitch-progress.dat";
 profile_init(&player);game_start_level(&game,9,24202);game.level_flags|=LEVEL_READY;can_resume=1;replaying=paused=help=gallery=0;screen=1;previous_buttons=0;
 SceCtrlData pad;memset(&pad,0,sizeof(pad));pad.Lx=pad.Ly=128;pad.Buttons=PSP_CTRL_START;checkpoint_test_delay(200);
 uint64_t start=sceKernelGetSystemTimeWide();input(&pad,.016f);unsigned pause_us=sceKernelGetSystemTimeWide()-start;
 H("Start opens pause while its checkpoint is still writing",paused&&checkpoint_busy()&&pause_us<30000);
 pad.Buttons=0;input(&pad,.016f);pad.Buttons=PSP_CTRL_START;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
 int first_col=game.cursor_col;pad.Buttons=PSP_CTRL_RIGHT;repeat_timer=0;start=sceKernelGetSystemTimeWide();input(&pad,.016f);unsigned cursor_us=sceKernelGetSystemTimeWide()-start;
 H("tile navigation remains responsive during a pending checkpoint",!paused&&game.cursor_col==(first_col+1)%COLS&&cursor_us<30000);
 game.status=WON;start=sceKernelGetSystemTimeWide();int queued=queue_current();unsigned reward_us=sceKernelGetSystemTimeWide()-start;
 H("winning queues a reward checkpoint without waiting for storage",queued&&reward_us<30000&&checkpoint_flush());
 fprintf(out,"HITCH INPUT TIMES: pause %u us, cursor %u us, victory enqueue %u us (200 ms injected storage delay)\n",pause_us,cursor_us,reward_us);
 checkpoint_test_delay(0);for(int i=0;i<3;i++){const char *path=(const char*[]){profile_path,adventure_path,progress_path}[i];char backup[160];remove(path);snprintf(backup,sizeof(backup),"%s.bak",path);remove(backup);}
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
