static int collection_ui_selftest(FILE *out){
    Game original=game,backup=adventure_backup;Profile old_player=player;Platform old_hardware=hardware;uint32_t old_modes=menu_modes;
    int old_screen=screen,old_resume=can_resume,old_replay=replaying,passed=0,total=0;
    const char *old_profile_path=profile_path,*old_adventure_path=adventure_path;
    profile_path="selftest-ui-profile.dat";adventure_path="selftest-ui-adventure.dat";can_resume=0;replaying=0;
    SceCtrlData pad;memset(&pad,0,sizeof(pad));pad.Lx=pad.Ly=128;
    #define C_CHECK(label,test) do{int ok=(test);passed+=ok;total++;fprintf(out,"collection %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    int valid=1;for(int i=0;i<75;i++)valid&=load_book_entry(i)&&book_title[0]&&book_body[0];C_CHECK("all 75 original Almanac entries can be read",valid);
    profile_init(&player);player.highest=25;player.money=1000;screen=9;store_cursor=4;previous_buttons=0;
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("shop purchase persists its cost and ownership",player.money==500&&player.purchases[SHOP_GATLING]==1);
    Profile saved;C_CHECK("purchase survives profile reload",profile_load(&saved,profile_path)&&saved.money==500&&saved.purchases[SHOP_GATLING]);
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);C_CHECK("buying an owned plant cannot deduct money twice",player.money==500);
    game_start_level(&game,9,1400);game.seed_slots=6;game.unlocked_mask=(UINT64_C(1)<<ALL_SEEDS)-1;game.seed_mask=1;game.selected=IMITATER;screen=3;
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);C_CHECK("Imitater opens its copy chooser",screen==13);
    imit_cursor=MINE;pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("copy chooser adds the chosen packet",screen==3&&game.imitater_seed==MINE&&(game.seed_mask&(UINT64_C(1)<<IMITATER)));
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);C_CHECK("selected Imitater can be removed from the bank",screen==3&&!(game.seed_mask&(UINT64_C(1)<<IMITATER)));
    screen=10;book_zombies=0;book_cursor=48;book_scroll=9;pad.Buttons=PSP_CTRL_TRIANGLE;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("Almanac tabs reset entry and scroll",book_zombies&&book_cursor==0&&book_scroll==0);
    menu_modes=(UINT32_C(1)<<APP_MODE_COUNT)-1;hardware=platform_from_model(10);
    C_CHECK("Street menu removes both wireless entries",more_count()==4&&more_id(3)==12);
    hardware=platform_from_model(1);C_CHECK("Wi-Fi model can expose implemented multiplayer entries",more_count()==6&&more_id(3)==5&&more_id(4)==6&&more_id(5)==12);
    hardware=platform_from_model(2);menu_modes=APP_BUILT_MODES;
    C_CHECK("current 3000 menu hides unfinished modes",more_count()==4&&more_id(3)==12);
    hardware=platform_from_model(10);menu_modes=(UINT32_C(1)<<APP_MODE_COUNT)-1;screen=6;more_cursor=0;
    pad.Buttons=PSP_CTRL_UP;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("navigation skips hidden entries and opens Back",more_cursor==3&&screen==0);
    hardware=platform_from_model(-1);C_CHECK("unknown-model menu keeps safe offline entries",more_count()==4&&more_id(3)==12);
    remove(profile_path);remove(adventure_path);profile_path=old_profile_path;adventure_path=old_adventure_path;
    game=original;adventure_backup=backup;player=old_player;hardware=old_hardware;menu_modes=old_modes;screen=old_screen;can_resume=old_resume;replaying=old_replay;previous_buttons=0;book_loaded=-1;reset_level_ui();
    fprintf(out,"COLLECTION UI RESULT %d/%d\n",passed,total);return passed==total;
    #undef C_CHECK
}
