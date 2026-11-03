static int extra_ui_selftest(FILE *out){
 Game old_game=game,old_backup=adventure_backup;Profile old_player=player;int old_screen=screen,old_replay=replaying,old_resume=can_resume;const char *old_profile=profile_path;profile_path="selftest-extra-ui-profile.dat";
 int pass=0,total=0;
 #define U(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"extra UI %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 profile_init(&player);player.highest=50;player.adventure_finished=1;player.garden.survival_wins=31;player.garden.puzzle_wins=7u<<10;player.money=1000;can_resume=0;replaying=0;
 open_extra(EXTRA_SURVIVAL,2,SURVIVAL_HARD);U("Survival starts a separate saved session",replaying&&game.extra.category==EXTRA_SURVIVAL&&screen==2);game.sun=825;game.level_flags|=LEVEL_READY;save_current();game.sun=0;resume_extra();U("Survival resumes its own sun and terrain",game.sun==825&&game.extra.selection==2&&screen==1);
 open_extra(EXTRA_IZOMBIE,3,0);game.cursor_col=8;uint32_t money=player.money;game_extra_action(&game,0);save_current();U("I Zombie cannot unlock proxy Adventure levels",player.highest==50&&player.money==money&&game.extra.category==EXTRA_IZOMBIE);
 story_begin(1501,3,0,0);U("Dave dialogue keeps the original mixed-case text",screen==18&&strstr(story_text,"seed"));story_id=1503;story_phase=1;story_load();story_line_time=1;story_input(PSP_CTRL_CROSS);U("Dave's purchase offer buys a seed slot once",player.purchases[SHOP_SLOTS]==1&&player.money==925&&story_id==1510);
 story_id=1503;story_phase=1;story_load();story_line_time=1;story_input(PSP_CTRL_SQUARE);U("declining an offer does not spend money",player.money==925&&story_id==1520);
 int valid=1;for(int i=0;i<STORY_LINES;i++){story_library=1;story_index=i;story_load();valid&=story_text[0]!=0&&strchr(story_text,'{')==0;}U("all imported Dave passages render with command markup removed",valid);story_library=0;
 garden_initialize(&player,(int64_t)time(NULL));screen=16;garden_tool=TOOL_SELL;garden_slot=0;SceCtrlData pad;memset(&pad,0,sizeof(pad));extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);int count=garden_count(&player,GARDEN_MAIN);U("selling first opens a confirmation",garden_sale>=0&&count==2);extra_menu_input(PSP_CTRL_CIRCLE,&pad,.016f);U("cancel leaves the plant in the garden",garden_sale==-1&&garden_count(&player,GARDEN_MAIN)==2);
 remove("mode-save.dat");remove(profile_path);profile_path=old_profile;game=old_game;adventure_backup=old_backup;player=old_player;screen=old_screen;replaying=old_replay;can_resume=old_resume;story_library=story_phase=0;previous_buttons=0;garden_tool=0;garden_sale=garden_held=-1;reset_level_ui();
 #undef U
 fprintf(out,"EXTRA UI RESULT %d/%d\n",pass,total);return pass==total;
}
