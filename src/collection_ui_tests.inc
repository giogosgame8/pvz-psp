static int collection_ui_selftest(FILE *out){
    Game original=game,backup=adventure_backup;Profile old_player=player;Platform old_hardware=hardware;uint32_t old_modes=menu_modes;
    int old_screen=screen,old_resume=can_resume,old_replay=replaying,old_house=house_cursor,passed=0,total=0;
    const char *old_profile_path=profile_path,*old_adventure_path=adventure_path;
    profile_path="selftest-ui-profile.dat";adventure_path="selftest-ui-adventure.dat";can_resume=0;replaying=0;
    SceCtrlData pad;memset(&pad,0,sizeof(pad));pad.Lx=pad.Ly=128;
    #define C_CHECK(label,test) do{int ok=(test);passed+=ok;total++;fprintf(out,"collection %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    int valid=1;
    for(int i=0;i<BOOK_ENTRIES;i++)valid&=load_book_entry(i)&&book_title[0]&&book_line_count>0&&book_lines[0][0];
    C_CHECK("all 75 original Almanac entries can be read and wrapped",valid);
    C_CHECK("changing Almanac entries uses the in-memory book and closes its file",
            valid&&book_image&&!book_stream&&load_book_entry(0)&&load_book_entry(BOOK_ENTRIES-1));
    if(book_image){
        uint32_t offset,bad=book_size;
        memcpy(&offset,book_image+8,sizeof(offset));memcpy(book_image+8,&bad,sizeof(bad));
        C_CHECK("a malformed Almanac entry leaves other cached pages readable",
                !load_book_entry(0)&&!load_book_entry(0)&&load_book_entry(1));
        memcpy(book_image+8,&offset,sizeof(offset));
    }
    book_close();
    const char *fixture="selftest-almanac.dat";
    remove(fixture);
    int missing=!book_open(fixture)&&!load_book_entry(0)&&!book_image&&!book_stream;
    book_close();
    C_CHECK("a missing Almanac is only attempted once and can reopen on a new visit",
            missing&&load_book_entry(0)&&book_image&&!book_stream);
    book_close();
    uint32_t header[2+BOOK_ENTRIES]={0x31425650,BOOK_ENTRIES,BOOK_CACHE_LIMIT+4};
    uint32_t sizes[2]={1,1};
    FILE *large=fopen(fixture,"wb");
    int written=large&&fwrite(header,sizeof(header),1,large)==1&&
        !fseek(large,header[2],SEEK_SET)&&fwrite(sizes,sizeof(sizes),1,large)==1&&fwrite("AB",1,2,large)==2;
    if(large&&fclose(large))written=0;
    C_CHECK("larger prepared Almanacs use the bounded stream fallback",
            written&&book_open(fixture)&&book_stream&&!book_image&&load_book_entry(0)&&
            !strcmp(book_title,"A")&&!strcmp(book_lines[0],"B"));
    book_close();remove(fixture);
    char long_word[BOOK_LINE_WIDTH+4];
    memset(long_word,'A',sizeof(long_word)-1);long_word[sizeof(long_word)-1]=0;
    int lines=wrap_book(long_word);
    C_CHECK("cached Almanac lines fit an overlong word without losing text",
        lines==2&&strlen(book_lines[0])==BOOK_LINE_WIDTH&&!strcmp(book_lines[1],"AAA"));
    book_close();
    profile_init(&player);player.highest=25;player.money=1000;screen=9;store_cursor=4;previous_buttons=0;
    game_start_level(&game,24,1401);can_resume=1;profile_apply(&player,&game);adventure_backup=game;
    int baseline_saved=save_current();
    game.sun=200;
    int profile_blocked=sceIoMkdir("selftest-ui-profile.dat.tmp",0777)==0;
    int board_blocked=sceIoMkdir("selftest-ui-adventure.dat.tmp",0777)==0;
    const uint64_t gatling=UINT64_C(1)<<GATLING;
    Profile saved;
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("a failed purchase leaves the live and saved coins and plants intact",
        baseline_saved&&profile_blocked&&player.money==1000&&!player.purchases[SHOP_GATLING]&&
        !(game.purchased_mask&gatling)&&!(adventure_backup.purchased_mask&gatling)&&
        strstr(message,"COULD NOT SAVE YOUR PURCHASE")&&profile_load(&saved,profile_path)&&saved.money==1000&&
        !saved.purchases[SHOP_GATLING]);
    if(profile_blocked)sceIoRmdir("selftest-ui-profile.dat.tmp");
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("retry charges once and updates both boards after committing the profile",
        player.money==500&&player.purchases[SHOP_GATLING]==1&&(game.purchased_mask&gatling)&&
        (adventure_backup.purchased_mask&gatling)&&!strcmp(message,"PURCHASED"));
    C_CHECK("purchase survives profile reload",profile_load(&saved,profile_path)&&saved.money==500&&saved.purchases[SHOP_GATLING]&&
        (saved.seen_plants&gatling));
    Game saved_board;
    C_CHECK("shop purchases can commit without rewriting the Adventure checkpoint",
        board_blocked&&game_load(&saved_board,adventure_path)&&saved_board.sun!=game.sun&&
        !(saved_board.purchased_mask&gatling)&&!strcmp(message,"PURCHASED"));
    if(board_blocked)sceIoRmdir("selftest-ui-adventure.dat.tmp");
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);C_CHECK("buying an owned plant cannot deduct money twice",player.money==500);
    game.session_id=adventure_backup.session_id=0;store_cursor=2;
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("buying a rake keeps all charges for future lawns",
        player.money==480&&player.purchases[SHOP_RAKE]==3&&!(game.store_flags&STORE_RAKE)&&
        !(adventure_backup.store_flags&STORE_RAKE)&&!game.session_id&&!adventure_backup.session_id);
    player.money=2000;screen=29;house_cursor=1;
    profile_blocked=sceIoMkdir("selftest-ui-profile.dat.tmp",0777)==0;
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("a failed blueprint purchase keeps the current house and wallet",
        profile_blocked&&player.money==2000&&!profile_house_owned(&player,1)&&
        player.completion.house_selected==0&&strstr(message,"COULD NOT SAVE YOUR PURCHASE"));
    if(profile_blocked)sceIoRmdir("selftest-ui-profile.dat.tmp");
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("retry commits the blueprint and selects the house once",player.money==1200&&
        profile_load(&saved,profile_path)&&saved.money==1200&&saved.purchases[SHOP_HOUSE_SCARY]==1&&
        saved.completion.house_selected==1);
    game_start_level(&game,9,1400);game.seed_slots=6;game.unlocked_mask=(UINT64_C(1)<<ALL_SEEDS)-1;game.seed_mask=1;game.selected=IMITATER;screen=3;
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);C_CHECK("Imitater opens its copy chooser",screen==13);
    imit_cursor=MINE;pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("copy chooser adds the chosen packet",screen==3&&game.imitater_seed==MINE&&(game.seed_mask&(UINT64_C(1)<<IMITATER)));
    pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);C_CHECK("selected Imitater can be removed from the bank",screen==3&&!(game.seed_mask&(UINT64_C(1)<<IMITATER)));
    screen=10;book_zombies=0;book_cursor=48;book_scroll=9;pad.Buttons=PSP_CTRL_TRIANGLE;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("Almanac tabs reset entry and scroll",book_zombies&&book_cursor==0&&book_scroll==0);
    pad.Buttons=PSP_CTRL_LEFT;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("wrapping to the boss preserves its undiscovered entry",book_cursor==Z_BOSS&&!book_known());
    player.seen_zombies|=1u<<Z_BOSS;
    C_CHECK("the unlocked boss uses the exposed-head portrait",book_known()&&book_portrait()==&assets[A_BOSS_EAT]&&
        book_portrait()->loaded&&book_portrait()->frames>0);
    int opened=load_book_entry(BOOK_ENTRIES-1);
    pad.Buttons=PSP_CTRL_CIRCLE;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    C_CHECK("leaving the Almanac releases its cache",
            opened&&screen==11&&!book_image&&!book_stream&&!book_attempted&&book_loaded==-1);
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
    remove("selftest-ui-profile.dat.bak");remove("selftest-ui-adventure.dat.bak");
    game=original;adventure_backup=backup;player=old_player;hardware=old_hardware;menu_modes=old_modes;screen=old_screen;can_resume=old_resume;replaying=old_replay;house_cursor=old_house;previous_buttons=0;book_close();reset_level_ui();
    fprintf(out,"COLLECTION UI RESULT %d/%d\n",passed,total);return passed==total;
    #undef C_CHECK
}
