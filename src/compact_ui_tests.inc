static int compact_ui_selftest(FILE *out){
 Game original=game,backup=adventure_backup;Profile old_player=player;Platform old_hardware=hardware;Progress old_progress=progress;int old_screen=screen,old_resume=can_resume,old_replay=replaying;uint32_t old_modes=menu_modes;const char *old_profile=profile_path,*old_save=adventure_path,*old_history=progress_path;
 profile_path="selftest-compact-profile.dat";adventure_path="selftest-compact-save.dat";progress_path="selftest-compact-history.dat";SceCtrlData pad;memset(&pad,0,sizeof(pad));pad.Lx=pad.Ly=128;int pass=0,total=0;ready=1;previous_buttons=0;help=gallery=0;replaying=0;
 #define UI(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"compact %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 #define PRESS(button) do{pad.Buttons=0;input(&pad,.016f);pad.Buttons=(button);input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);}while(0)
 profile_init(&player);player.highest=50;player.money=1000;player.garden.story_seen=UINT64_MAX;hardware=platform_from_model(10);menu_modes=APP_BUILT_MODES;
 UI("More Ways contains only the three challenge families and Back",more_count()==4&&more_id(0)==1&&more_id(1)==8&&more_id(2)==7&&more_id(3)==12);
 UI("original paper backing fits behind all five notes",assets[A_NOTE_PAPER].loaded&&assets[A_NOTE_PAPER].width==372&&assets[A_NOTE_PAPER].height==236);
 int titles_fit=1;for(int i=0;i<20;i++){char name[80];ui_title_case(name,sizeof(name),puzzle_names[i]);titles_fit&=disc_text_height(name,68,ui_card_scale(name,82))<=20;}UI("all twenty puzzle titles stay inside their cards",titles_fit);
 titles_fit=1;for(int i=0;i<20;i++){char name[80];ui_title_case(name,sizeof(name),mini_names[i]);titles_fit&=disc_text_height(name,86,ui_card_scale(name,100))<=20;}UI("all twenty mini-game titles stay inside their cards",titles_fit);
 screen=0;menu_selected=2;PRESS(PSP_CTRL_CROSS);UI("Extras opens the original collection page",screen==11);
 collection_cursor=0;PRESS(PSP_CTRL_CROSS);UI("shop belongs to Extras",screen==9&&store_return==11);PRESS(PSP_CTRL_CIRCLE);UI("shop Back returns to Extras",screen==11);
 collection_cursor=1;PRESS(PSP_CTRL_CROSS);PRESS(PSP_CTRL_CIRCLE);UI("Almanac returns to Extras",screen==11);
 collection_cursor=3;PRESS(PSP_CTRL_CROSS);UI("mailbox opens zombie notes",screen==8);PRESS(PSP_CTRL_CIRCLE);UI("notes Back returns to Extras",screen==11);
 collection_cursor=2;PRESS(PSP_CTRL_CROSS);UI("garden is reached from Extras",screen==16);PRESS(PSP_CTRL_TRIANGLE);UI("garden chooser groups the Tree with the gardens",screen==22);player.purchases[SHOP_TREE]=1;garden_picker_cursor=4;PRESS(PSP_CTRL_CROSS);UI("Tree opens from the garden chooser",screen==17);PRESS(PSP_CTRL_CIRCLE);UI("Tree Back returns to the garden",screen==16);
 screen=0;menu_selected=0;can_resume=1;game_start_level(&game,9,3500);adventure_backup=game;PRESS(PSP_CTRL_CROSS);UI("Adventure groups Continue, Replay and New",screen==21);adventure_cursor=1;PRESS(PSP_CTRL_CROSS);UI("Replay is inside Adventure",screen==5);PRESS(PSP_CTRL_CIRCLE);UI("replay Back returns to Adventure",screen==21);
 screen=0;PRESS(PSP_CTRL_START);UI("Start opens compact Options",screen==4);menu_volume_row=5;PRESS(PSP_CTRL_CROSS);UI("ending replay is inside Credits and extras",screen==20);PRESS(PSP_CTRL_CIRCLE);UI("Credits returns to Options",screen==4);
 for(int chapter=0;chapter<5;chapter++){game_start_level(&game,chapter*10+9,3600+chapter);game.status=WON;game.level_flags|=LEVEL_READY|LEVEL_REWARD_COLLECTED;reward_collect_time=1;screen=1;replaying=0;char label[80];snprintf(label,sizeof(label),"chapter %d shows its note before the finale",chapter+1);UI(label,game_reward_type(&game)==REWARD_NOTE&&game.level/10==chapter);PRESS(PSP_CTRL_CROSS);snprintf(label,sizeof(label),"chapter %d finale waits at its intro",chapter+1);UI(label,game.level==chapter*10+10&&screen==2&&!game.total_spawned);}
 game_start_level(&game,50,3610);game.status=WON;game.level_flags|=LEVEL_READY|LEVEL_REWARD_COLLECTED;reward_collect_time=.4f;screen=1;replaying=0;PRESS(PSP_CTRL_CROSS);UI("final reward flight still completes before opening credits",screen==1&&!(game.level_flags&32));
 reward_collect_time=1;PRESS(PSP_CTRL_CROSS);UI("quick Continue after the final reward opens the ending",screen==19&&(game.level_flags&32));if(screen==19)ending_finish();
 remove(profile_path);remove(adventure_path);remove(progress_path);profile_path=old_profile;adventure_path=old_save;progress_path=old_history;game=original;adventure_backup=backup;player=old_player;progress=old_progress;hardware=old_hardware;menu_modes=old_modes;screen=old_screen;can_resume=old_resume;replaying=old_replay;previous_buttons=0;reset_level_ui();
 #undef PRESS
 #undef UI
 fprintf(out,"COMPACT UI RESULT %d/%d\n",pass,total);return pass==total;
}
