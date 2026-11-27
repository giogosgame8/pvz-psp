static void menu_read_test_wait(void){
 for(int i=0;i<400;i++){
  if(menu_read==READ_NONE&&(menu_launch.mode==LAUNCH_NONE||menu_launch.snapshot==CHECKPOINT_FAILED))return;
  sceKernelDelayThread(5000);
  if(menu_read!=READ_NONE)menu_read_step();
  else menu_launch_input(0);
 }
}
static int extra_ui_selftest(FILE *out){
 Game old_game=game,old_backup=adventure_backup;Profile old_player=player;int old_screen=screen,old_replay=replaying,old_resume=can_resume;const char *old_profile=profile_path;profile_path="selftest-extra-ui-profile.dat";
 int pass=0,total=0;
 #define U(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"extra UI %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 profile_init(&player);player.highest=50;player.adventure_finished=1;player.garden.survival_wins=31;player.garden.puzzle_wins=7u<<10;player.money=1000;can_resume=0;replaying=0;
 open_extra(EXTRA_SURVIVAL,2,SURVIVAL_HARD);U("Survival starts a separate saved session",replaying&&game.extra.category==EXTRA_SURVIVAL&&screen==2);game.sun=825;game.level_flags|=LEVEL_READY;save_current();game.sun=0;resume_extra();menu_read_test_wait();U("Survival resumes its own sun and terrain",game.sun==825&&game.extra.selection==2&&screen==1);
 screen=14;survival_bonus=0;survival_cursor=12;extra_menu_input(PSP_CTRL_SQUARE,0,0);
 U("bonus boards stay locked until Pool Endless unlocks",!survival_bonus);
 player.garden.survival_wins=1023;extra_menu_input(PSP_CTRL_SQUARE,0,0);extra_menu_input(PSP_CTRL_LEFT,0,0);
 U("bonus page wraps from Day to Roof",survival_bonus&&bonus_terrain[bonus_cursor]==4);
 extra_menu_input(PSP_CTRL_CROSS,0,0);
 U("Roof starts as endless with its own record slot",screen==2&&game_world(&game)==4&&game.extra.goal==0&&game.extra.difficulty==SURVIVAL_ENDLESS);
 int before_bonus=profile_trophy_count(&player);game.extra.flags=6;save_current();resume_extra();menu_read_test_wait();
 U("bonus Roof can be saved and resumed",game.extra.flags==6&&game.extra.selection==4&&player.garden.survival_best[14]==6&&profile_trophy_count(&player)==before_bonus);
 survival_bonus=0;
 open_extra(EXTRA_IZOMBIE,3,0);game.cursor_col=8;uint32_t money=player.money;game_extra_action(&game,0);save_current();U("I Zombie cannot unlock proxy Adventure levels",player.highest==50&&player.money==money&&game.extra.category==EXTRA_IZOMBIE);
 story_begin(1501,3,0,0);U("Dave dialogue keeps the original mixed-case text",screen==18&&strstr(story_text,"seed"));
 story_id=1503;story_phase=1;story_load();story_line_time=1;
 const char *blocked_purchase="selftest-extra-ui-profile.dat.tmp";
 int blocked=sceIoMkdir(blocked_purchase,0777)==0;
 story_input(PSP_CTRL_CROSS);
 U("Dave keeps the offer open and charges nothing when the save fails",
   blocked&&story_id==1503&&player.money==1000&&!player.purchases[SHOP_SLOTS]&&game.seed_slots==6&&
   strstr(message,"COULD NOT SAVE YOUR PURCHASE"));
 if(blocked)sceIoRmdir(blocked_purchase);
 story_input(PSP_CTRL_CROSS);
 U("Dave's purchase offer buys a seed slot once",player.purchases[SHOP_SLOTS]==1&&player.money==925&&story_id==1510&&game.seed_slots==7);
 story_id=1503;story_phase=1;story_load();story_line_time=1;story_input(PSP_CTRL_SQUARE);U("declining an offer does not spend money",player.money==925&&story_id==1520);
 int valid=1;for(int i=0;i<STORY_LINES;i++){story_library=1;story_index=i;story_load();valid&=story_text[0]!=0&&strchr(story_text,'{')==0;}U("all imported Dave passages render with command markup removed",valid);story_library=0;
 garden_initialize(&player,(int64_t)time(NULL));screen=16;garden_tool=TOOL_SELL;garden_slot=0;SceCtrlData pad;memset(&pad,0,sizeof(pad));extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);int count=garden_count(&player,GARDEN_MAIN);U("selling first opens a confirmation",garden_sale>=0&&count==2);extra_menu_input(PSP_CTRL_CIRCLE,&pad,.016f);U("cancel leaves the plant in the garden",garden_sale==-1&&garden_count(&player,GARDEN_MAIN)==2);
 Profile saved;
 money=player.money;
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 int sale_index=garden_sale,sale_price=garden_sell_price(&player,sale_index);
 blocked=sceIoMkdir(blocked_purchase,0777)==0;
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 U("a failed sale keeps the plant, wallet and confirmation",
     blocked&&sale_price>0&&garden_sale==sale_index&&garden_find(&player,GARDEN_MAIN,0)==sale_index&&
     player.money==money&&strstr(message,"COULD NOT SAVE THE SALE"));
 if(blocked)sceIoRmdir(blocked_purchase);
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 U("retry sells the plant and credits the saved wallet once",
     garden_sale==-1&&garden_find(&player,GARDEN_MAIN,0)==-1&&player.money==money+sale_price&&
     profile_load(&saved,profile_path)&&saved.money==player.money&&garden_find(&saved,GARDEN_MAIN,0)==-1);

 player.purchases[SHOP_GLOVE]=1;garden_tool=TOOL_GLOVE;garden_slot=1;
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 int held=garden_held;garden_slot=6;
 blocked=sceIoMkdir(blocked_purchase,0777)==0;
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 U("a failed move leaves the plant at its source and ready to place",
     blocked&&held>=0&&garden_held==held&&garden_find(&player,GARDEN_MAIN,1)==held&&
     garden_find(&player,GARDEN_MAIN,6)==-1&&strstr(message,"COULD NOT SAVE THE MOVE"));
 if(blocked)sceIoRmdir(blocked_purchase);
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 U("retry saves the moved plant in one destination",
     garden_held==-1&&profile_load(&saved,profile_path)&&garden_find(&saved,GARDEN_MAIN,1)==-1&&
     garden_find(&saved,GARDEN_MAIN,6)==held);

 int64_t care_time=(int64_t)time(NULL);
 garden_add(&player,MARIGOLD,care_time);
 int second=garden_find(&player,GARDEN_MAIN,0);
 GardenPlant *first_plant=&player.garden.plants[held];
 GardenPlant *second_plant=&player.garden.plants[second];
 first_plant->waterings=first_plant->required;first_plant->watered=care_time-60;
 second_plant->waterings=second_plant->required;second_plant->watered=care_time-60;
 player.purchases[SHOP_FERTILIZER]=1;
 garden_tool=TOOL_FERTILIZER;garden_slot=6;
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 garden_slot=0;
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 U("two plants cannot reserve the same fertilizer",
   garden_busy(held)&&!garden_busy(second)&&!garden_available_supply(SHOP_FERTILIZER));
 player.garden.coins[0]=(GardenCoin){.used=1,.value=5};money=player.money;
 extra_menu_input(PSP_CTRL_SQUARE,&pad,.016f);
 U("saving coins during care preserves the unspent supply in the save",
   garden_busy(held)&&profile_load(&saved,profile_path)&&saved.money==money+5&&
   saved.purchases[SHOP_FERTILIZER]==1&&!saved.garden.plants[held].age);
 blocked=sceIoMkdir(blocked_purchase,0777)==0;
 garden_present_step(2);
 U("failed timed care leaves the plant, fertilizer and reward unchanged",
   blocked&&!garden_pending()&&!first_plant->age&&garden_available_supply(SHOP_FERTILIZER)==1&&
   !player.garden.coins[0].used&&strstr(message,"COULD NOT SAVE PLANT CARE"));
 garden_slot=6;
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 extra_menu_input(PSP_CTRL_TRIANGLE,&pad,.016f);
 U("a failed care save during navigation keeps the player in the garden",
   screen==16&&!garden_pending()&&!first_plant->age&&player.purchases[SHOP_FERTILIZER]==1&&
   strstr(message,"COULD NOT SAVE PLANT CARE"));
 if(blocked)sceIoRmdir(blocked_purchase);
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 extra_menu_input(PSP_CTRL_TRIANGLE,&pad,.016f);
 U("retry saves plant growth, fertilizer and its reward exactly once",
   screen==22&&!garden_pending()&&profile_load(&saved,profile_path)&&!saved.purchases[SHOP_FERTILIZER]&&
   saved.garden.plants[held].age==1&&saved.garden.coins[0].used&&saved.garden.coins[0].value==5&&
   !saved.garden.coins[1].used);

 const uint64_t seen_tree=UINT64_C(1)<<61;
 player.purchases[SHOP_TREE]=1;player.purchases[SHOP_TREE_FOOD]=2;
 player.garden.tree_height=99;player.garden.tree_tip=49;player.garden.story_seen&=~seen_tree;
 money=player.money;
 screen=22;garden_picker_cursor=4;
 blocked=sceIoMkdir(blocked_purchase,0777)==0;
 hub_input(PSP_CTRL_CROSS);
 U("a failed Tree visit keeps the picker, uncollected coins and introduction",
     blocked&&screen==22&&player.money==money&&player.garden.coins[0].used&&!(player.garden.story_seen&seen_tree));
 if(blocked)sceIoRmdir(blocked_purchase);
 hub_input(PSP_CTRL_CROSS);
 U("entering the Tree saves its introduction and credits the coins once",
     screen==18&&story_id==3200&&story_return==17&&profile_load(&saved,profile_path)&&
     saved.money==money+5&&!saved.garden.coins[0].used&&(saved.garden.story_seen&seen_tree));
 screen=17;load_tree_tip(player.garden.tree_tip);
 char old_tip[sizeof(tree_tip_text)];snprintf(old_tip,sizeof(old_tip),"%s",tree_tip_text);
 blocked=sceIoMkdir(blocked_purchase,0777)==0;
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 U("a failed tree feed keeps the food, height and tip",blocked&&player.purchases[SHOP_TREE_FOOD]==2&&
     player.garden.tree_height==99&&player.garden.tree_tip==49&&strstr(message,"COULD NOT SAVE TREE GROWTH"));
 if(blocked)sceIoRmdir(blocked_purchase);
 extra_menu_input(PSP_CTRL_CROSS,&pad,.016f);
 U("retry saves growth and spends one tree food",profile_load(&saved,profile_path)&&
     saved.purchases[SHOP_TREE_FOOD]==1&&saved.garden.tree_height==100&&saved.garden.tree_tip==800);
 load_tree_tip(player.garden.tree_tip);
 U("feeding refreshes the cached tip to the new milestone",
     tree_tip_loaded==800&&old_tip[0]&&tree_tip_text[0]&&strcmp(old_tip,tree_tip_text));
 load_tree_tip(0);
 U("an unset tip displays the first entry",tree_tip_loaded==1&&tree_tip_text[0]);
 load_tree_tip(UINT32_MAX);
 U("a missing tip clears the previous tip",tree_tip_loaded==UINT32_MAX&&!tree_tip_text[0]);
 extra_menu_input(PSP_CTRL_CIRCLE,&pad,.016f);
 U("leaving the Tree releases its cached entry for retry",screen==16&&tree_tip_loaded==0);

 screen=16;garden_return=11;
 player.garden.coins[0]=(GardenCoin){.used=1,.value=25};money=player.money;
 blocked=sceIoMkdir(blocked_purchase,0777)==0;
 extra_menu_input(PSP_CTRL_SQUARE,&pad,.016f);
 U("failed manual collection leaves the coin and wallet intact",
     blocked&&player.garden.coins[0].used&&player.money==money&&strstr(message,"COULD NOT SAVE THE COINS"));
 extra_menu_input(PSP_CTRL_CIRCLE,&pad,.016f);
 U("a failed garden exit keeps the player on the garden screen",
     screen==16&&player.garden.coins[0].used&&player.money==money&&strstr(message,"COULD NOT SAVE THE GARDEN"));
 if(blocked)sceIoRmdir(blocked_purchase);
 extra_menu_input(PSP_CTRL_CIRCLE,&pad,.016f);
 U("exiting after retry saves the pending coins once",
     screen==11&&profile_load(&saved,profile_path)&&saved.money==money+25&&!saved.garden.coins[0].used);
 remove("mode-save.dat");remove(profile_path);profile_path=old_profile;game=old_game;adventure_backup=old_backup;player=old_player;screen=old_screen;replaying=old_replay;can_resume=old_resume;story_library=story_phase=0;previous_buttons=0;garden_tool=0;garden_sale=garden_held=-1;reset_level_ui();
 remove("selftest-extra-ui-profile.dat.bak");message_time=0;
 #undef U
 fprintf(out,"EXTRA UI RESULT %d/%d\n",pass,total);return pass==total;
}

static void pause_test_press(uint32_t button){
    SceCtrlData pad={.Buttons=button,.Lx=128,.Ly=128};
    previous_buttons=0;
    input(&pad,1.0f/60);
}
static void pause_test_wait(void){
    for(int i=0;i<400&&pause_return_pending;i++){
        sceKernelDelayThread(5000);
        pause_test_press(0);
    }
}
static void board_exit_test_wait(void){
    for(int i=0;i<400&&board_exit_screen>=0&&board_exit_snapshot!=CHECKPOINT_FAILED;i++){
        sceKernelDelayThread(5000);
        pause_test_press(0);
    }
}
static void menu_launch_test_wait(void){
    for(int i=0;i<400&&menu_launch.mode!=LAUNCH_NONE&&menu_launch.snapshot!=CHECKPOINT_FAILED;i++){
        sceKernelDelayThread(5000);
        pause_test_press(0);
    }
}
static int menu_launch_selftest(FILE *out){
    Game old_game=game,old_backup=adventure_backup,disk;
    Profile old_player=player;Progress old_progress=progress;
    int old_screen=screen,old_resume=can_resume,old_replay=replaying,old_pause=paused,old_help=help;
    int old_mini=mini_cursor,old_replay_cursor=replay_cursor,old_puzzle=puzzle_cursor,old_family=puzzle_family;
    int old_survival=survival_cursor,old_bonus=survival_bonus;
    const char *old_paths[]={adventure_path,profile_path,progress_path,challenge_path,mini_save_path};
    adventure_path="selftest-launch-adventure.dat";profile_path="selftest-launch-profile.dat";
    progress_path="selftest-launch-progress.dat";challenge_path="selftest-launch-challenge.dat";
    mini_save_path="selftest-launch-mini.dat";
    const char *paths[]={adventure_path,profile_path,progress_path,challenge_path,mini_save_path};
    const char *blocked_path="selftest-launch-adventure.dat.tmp";
    const struct { const char *name; LaunchMode mode; int menu,category; } cases[]={
        {"Replay",LAUNCH_REPLAY,5,0}, {"Mini-games",LAUNCH_MINI,7,0},
        {"Survival",LAUNCH_SURVIVAL,14,EXTRA_SURVIVAL}, {"Vasebreaker",LAUNCH_VASE,15,EXTRA_VASE},
        {"I, Zombie",LAUNCH_IZOMBIE,15,EXTRA_IZOMBIE}
    };
    int pass=0,total=0;
    #define L(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"mode launch %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    for(unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);i++){
        profile_init(&player);player.adventure_finished=1;player.highest=50;
        progress=(Progress){.played=7,.completed=3};
        can_resume=1;replaying=paused=help=0;
        game_start_level(&game,9,251+i);
        game.level_flags|=LEVEL_READY;game.sun=75;
        int baseline=save_current();
        screen=cases[i].menu;
        mini_cursor=survival_cursor=survival_bonus=puzzle_cursor=0;
        replay_cursor=2;puzzle_family=cases[i].mode==LAUNCH_IZOMBIE;
        game.sun=225+i;
        int blocked=sceIoMkdir(blocked_path,0777)==0;
        checkpoint_test_delay(150);
        uint64_t start=sceKernelGetSystemTimeWide();
        pause_test_press(PSP_CTRL_CROSS);
        unsigned elapsed=sceKernelGetSystemTimeWide()-start;
        fprintf(out,"MODE LAUNCH %s: %u us with a 150 ms injected write delay\n",cases[i].name,elapsed);
        L("input returns promptly during the Adventure write",
            menu_launch.mode==cases[i].mode&&menu_launch.snapshot==CHECKPOINT_WRITING&&
            screen==cases[i].menu&&elapsed<30000);
        menu_launch_test_wait();
        L("failure keeps the live Adventure and the previous saved board",
            baseline&&blocked&&menu_launch.snapshot==CHECKPOINT_FAILED&&screen==cases[i].menu&&
            !replaying&&game.sun==225+i&&game_load(&disk,adventure_path)&&disk.sun==75);
        checkpoint_take_error();
        if(blocked)sceIoRmdir(blocked_path);
        checkpoint_test_delay(0);
        pause_test_press(PSP_CTRL_CROSS);menu_launch_test_wait();
        int new_mode=cases[i].category?game.extra.category==cases[i].category:
            cases[i].mode==LAUNCH_MINI?mini_session_index(&game)==0:game.level==3&&!game.challenge;
        L("retry starts the chosen mode and saves the Adventure board",
            screen==2&&replaying&&menu_launch.mode==LAUNCH_NONE&&new_mode&&checkpoint_flush()&&
            game_load(&disk,adventure_path)&&disk.level==9&&disk.sun==225+i&&
            (!cases[i].category||(game_load(&disk,challenge_path)&&disk.extra.category==cases[i].category)));
        open_adventure();
        L("Continue restores the saved Adventure board",
            screen==1&&!replaying&&game.level==9&&game.sun==225+i);
        if(cases[i].mode==LAUNCH_SURVIVAL){
            screen=14;checkpoint_test_delay(150);
            blocked=sceIoMkdir(blocked_path,0777)==0;
            start=sceKernelGetSystemTimeWide();
            pause_test_press(PSP_CTRL_TRIANGLE);
            elapsed=sceKernelGetSystemTimeWide()-start;
            fprintf(out,"SURVIVAL READ INPUT: %u us with 150 ms delay\n",elapsed);
            L("Survival Resume keeps the menu responsive while reading",
                menu_read==READ_EXTRA_ROUND&&menu_read_queued&&screen==14&&elapsed<30000);
            menu_read_test_wait();
            L("a loaded Survival board waits if the Adventure save fails",
                blocked&&menu_launch.mode==LAUNCH_RESUME&&menu_launch.snapshot==CHECKPOINT_FAILED&&
                screen==14&&!replaying&&game.level==9&&game.sun==225+i);
            checkpoint_take_error();
            if(blocked)sceIoRmdir(blocked_path);
            checkpoint_test_delay(0);
            pause_test_press(PSP_CTRL_CROSS);menu_read_test_wait();
            L("retry resumes the validated Survival checkpoint",
                menu_launch.mode==LAUNCH_NONE&&replaying&&screen==2&&
                game.extra.category==EXTRA_SURVIVAL&&checkpoint_flush());
            open_adventure();
        }
    }
    screen=5;replay_cursor=2;checkpoint_test_delay(150);
    pause_test_press(PSP_CTRL_CROSS);
    int pending=menu_launch.snapshot==CHECKPOINT_WRITING;
    pause_test_press(PSP_CTRL_CIRCLE);
    checkpoint_flush();pause_test_press(0);
    L("cancelling a pending launch returns to the replay menu",
        pending&&menu_launch.mode==LAUNCH_NONE&&screen==5&&!replaying&&game.level==9);
    checkpoint_test_delay(0);checkpoint_take_error();
    for(int i=0;i<5;i++){
        char backup[128];snprintf(backup,sizeof(backup),"%s.bak",paths[i]);
        remove(paths[i]);remove(backup);
    }
    adventure_path=old_paths[0];profile_path=old_paths[1];progress_path=old_paths[2];
    challenge_path=old_paths[3];mini_save_path=old_paths[4];
    game=old_game;adventure_backup=old_backup;player=old_player;progress=old_progress;
    screen=old_screen;can_resume=old_resume;replaying=old_replay;paused=old_pause;help=old_help;
    mini_cursor=old_mini;replay_cursor=old_replay_cursor;puzzle_cursor=old_puzzle;puzzle_family=old_family;
    survival_cursor=old_survival;survival_bonus=old_bonus;menu_launch.mode=LAUNCH_NONE;
    previous_buttons=0;message_time=0;reset_level_ui();
    #undef L
    fprintf(out,"MODE LAUNCH RESULT %d/%d\n",pass,total);return pass==total;
}
static int pause_checkpoint_ui_selftest(FILE *out){
    Game old_game=game,old_backup=adventure_backup,disk;
    Profile old_player=player;
    Progress old_progress=progress,history;
    int old_screen=screen,old_replay=replaying,old_resume=can_resume,old_pause=paused,old_help=help;
    int old_cursor=pause_cursor,old_pending=pause_return_pending;
    CheckpointResult old_snapshot=pause_snapshot;
    const char *old_paths[]={adventure_path,profile_path,progress_path,challenge_path};
    adventure_path="selftest-pause-adventure.dat";profile_path="selftest-pause-profile.dat";
    progress_path="selftest-pause-history.dat";challenge_path="selftest-pause-challenge.dat";
    const char *const fixtures[]={adventure_path,profile_path,progress_path,challenge_path};
    int pass=0,total=0;
    #define P(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"pause checkpoint %s %s: %s\n",cases[i].name,label,ok?"PASS":"FAIL");}while(0)
    const struct { int category; const char *name; } cases[]={
        {0,"Adventure"}, {EXTRA_SURVIVAL,"Survival"}, {EXTRA_VASE,"Vasebreaker"}, {EXTRA_IZOMBIE,"I, Zombie"}
    };
    profile_init(&player);
    progress=(Progress){0};
    can_resume=1;
    for(int i=0;i<4;i++){
        replaying=cases[i].category!=0;
        if(!replaying)game_start_level(&game,6,201);
        else if(cases[i].category==EXTRA_SURVIVAL)game_start_survival(&game,2,SURVIVAL_HARD,201);
        else game_start_puzzle(&game,cases[i].category==EXTRA_IZOMBIE,3,201);
        game.level_flags|=LEVEL_READY;game.sun=75;
        screen=1;paused=help=0;
        const char *board_path=current_board_path();
        int baseline=save_current();
        char blocked_temp[128],backup[128];
        snprintf(blocked_temp,sizeof(blocked_temp),"%s.tmp",board_path);
        snprintf(backup,sizeof(backup),"%s.bak",board_path);
        int blocked=sceIoMkdir(blocked_temp,0777)==0;
        game.sun=125+i;
        progress_visit(&progress,6+i,1);
        checkpoint_test_delay(100);
        pause_test_press(PSP_CTRL_START);
        uint64_t start=sceKernelGetSystemTimeWide();
        pause_test_press(PSP_CTRL_TRIANGLE);
        unsigned elapsed=sceKernelGetSystemTimeWide()-start;
        P("opens a responsive save screen during a slow write",
          screen==1&&paused&&pause_return_pending&&elapsed<30000);
        pause_test_wait();
        checkpoint_test_delay(0);
        P("holds the live board on write failure and preserves the last saved board",
          baseline&&blocked&&screen==1&&paused&&!pause_return_pending&&
          pause_snapshot==CHECKPOINT_FAILED&&game.sun==125+i&&
          game_load(&disk,board_path)&&disk.sun==75);
        checkpoint_take_error();
        if(blocked)sceIoRmdir(blocked_temp);
        pause_test_press(PSP_CTRL_CROSS);pause_test_wait();
        P("retries the pause checkpoint and returns to the main menu",
          screen==0&&!paused&&message_time==0&&game_load(&disk,backup)&&disk.sun==75);
        if(replaying){game.sun=0;resume_extra();menu_read_test_wait();}
        else{
            /* Continue restores the Adventure board after another mode is opened. */
            game.sun=0;replaying=1;open_adventure();
        }
        P("restores the live board and replay history from the confirmed checkpoint",
          screen==1&&game.sun==125+i&&game.extra.category==cases[i].category&&
          game_load(&disk,board_path)&&disk.sun==125+i&&
          progress_load(&history,progress_path)&&progress_played(&history,6+i));
    }
    #undef P
    #define E(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"result checkpoint %s %s: %s\n",names[i],label,ok?"PASS":"FAIL");}while(0)
    const char *const names[]={"Survival intermission","Endless puzzle trophy","Adventure award"};
    for(int i=0;i<3;i++){
        replaying=i!=2;
        if(i==0)game_start_survival(&game,2,SURVIVAL_HARD,188);
        else if(i==1)game_start_puzzle(&game,0,9,188);
        else game_start_level(&game,6,188);
        reset_level_ui();
        game.level_flags|=LEVEL_READY;
        if(i!=0)game.level_flags|=LEVEL_REWARD_COLLECTED;
        game.extra.pending=i<2?i+1:0;
        if(i==2)game.status=WON;
        screen=1;paused=help=0;game.sun=75;
        const char *path=current_board_path();
        int baseline=save_current();
        game.sun=225+i;
        char temporary[128];
        snprintf(temporary,sizeof(temporary),"%s.tmp",path);
        int blocked=sceIoMkdir(temporary,0777)==0;
        checkpoint_test_delay(100);
        uint64_t started=sceKernelGetSystemTimeWide();
        pause_test_press(PSP_CTRL_CIRCLE);
        unsigned elapsed=sceKernelGetSystemTimeWide()-started;
        int destination=i<2?6:0;
        E("starts saving without stalling the menu",
          screen==1&&board_exit_screen==destination&&board_exit_snapshot==CHECKPOINT_WRITING&&elapsed<30000);
        board_exit_test_wait();
        E("keeps the result and previous checkpoint after a failed write",
          baseline&&blocked&&screen==1&&game.sun==225+i&&board_exit_snapshot==CHECKPOINT_FAILED&&
          game_load(&disk,path)&&disk.sun==75);
        checkpoint_take_error();
        if(blocked)sceIoRmdir(temporary);
        if(i==2){
            pause_test_press(PSP_CTRL_CROSS);
            E("X retries the same exit without advancing to the next lawn",
              screen==1&&board_exit_screen==destination&&board_exit_snapshot==CHECKPOINT_WRITING&&game.level==6);
        }else{
            pause_test_press(PSP_CTRL_CIRCLE);
            E("Circle dismisses the failure and keeps the result screen",screen==1&&board_exit_screen<0);
            pause_test_press(PSP_CTRL_CIRCLE);
            pause_test_press(PSP_CTRL_CIRCLE);
            E("Circle can cancel leaving while the worker writes",screen==1&&board_exit_screen<0);
            checkpoint_flush();
            pause_test_press(PSP_CTRL_CIRCLE);
        }
        board_exit_test_wait();checkpoint_test_delay(0);
        E("a successful save returns to the chosen menu and preserves the board",
          screen==destination&&board_exit_screen<0&&game_load(&disk,path)&&disk.sun==225+i&&
          disk.extra.pending==game.extra.pending&&disk.status==game.status&&
          !((disk.level_flags^game.level_flags)&LEVEL_REWARD_COLLECTED));
    }
    #undef E
    checkpoint_flush();
    for(int i=0;i<4;i++){
        char backup[128];snprintf(backup,sizeof(backup),"%s.bak",fixtures[i]);
        remove(fixtures[i]);remove(backup);
    }
    adventure_path=old_paths[0];profile_path=old_paths[1];progress_path=old_paths[2];challenge_path=old_paths[3];
    game=old_game;adventure_backup=old_backup;player=old_player;progress=old_progress;
    screen=old_screen;replaying=old_replay;can_resume=old_resume;paused=old_pause;help=old_help;
    pause_cursor=old_cursor;pause_return_pending=old_pending;pause_snapshot=old_snapshot;
    previous_buttons=0;message_time=0;reset_level_ui();
    fprintf(out,"BOARD CHECKPOINT UI RESULT %d/%d\n",pass,total);
    return pass==total;
}
static int mini_checkpoint_ui_selftest(FILE *out){
    Game old_game=game,old_backup=adventure_backup,disk;
    Profile old_player=player;
    Progress old_progress=progress;
    int old_screen=screen,old_replaying=replaying,old_resume=can_resume;
    int old_cursor=mini_cursor,old_mini_resume=mini_resume_id,old_pause=paused,old_help=help;
    int old_return_pending=pause_return_pending,old_pause_snapshot=pause_snapshot;
    const char *paths[]={adventure_path,profile_path,progress_path,mini_save_path};
    adventure_path="selftest-mini-ui-adventure.dat";profile_path="selftest-mini-ui-profile.dat";
    progress_path="selftest-mini-ui-progress.dat";mini_save_path="selftest-mini-ui-round.dat";
    const char *fixtures[]={adventure_path,profile_path,progress_path,mini_save_path};
    for(int i=0;i<4;i++){
        char backup[100];snprintf(backup,sizeof(backup),"%s.bak",fixtures[i]);
        remove(fixtures[i]);remove(backup);
    }
    int pass=0,total=0;
    #define MC(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"mini checkpoint %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    profile_init(&player);player.highest=27;player.garden.story_seen=CHALLENGE_MINI;
    game_start_level(&game,6,201);game.level_flags|=LEVEL_READY;game.sun=725;
    can_resume=1;replaying=0;
    int adventure_saved=save_current();
    open_mini_menu(MINI_SLOT);
    MC("a fresh menu has no round to resume",screen==7&&mini_resume_id<0);
    pause_test_press(PSP_CTRL_CROSS);menu_launch_test_wait();
    pause_test_press(PSP_CTRL_CROSS);
    game.spawn_timer=game.sky_timer=100000;
    pause_test_press(PSP_CTRL_TRIANGLE);
    for(int i=0;i<49;i++)game_step(&game,.05f);
    float time=game.mini.spin_time;
    MC("normal controls launch a paid spin",replaying&&screen==1&&(game.level_flags&LEVEL_READY)&&
       game_mini_active(&game,MINI_SLOT)&&game.sun==25&&game.mini.spinning==SLOT_ROLLING&&
       !game_slot_reel_rolling(&game,0));
    game.sun=75;int prior_saved=save_current();game.sun=25;
    checkpoint_test_delay(200);
    pause_test_press(PSP_CTRL_START);pause_test_press(PSP_CTRL_DOWN);
    uint64_t started=sceKernelGetSystemTimeWide();
    pause_test_press(PSP_CTRL_CROSS);
    unsigned elapsed=sceKernelGetSystemTimeWide()-started;
    fprintf(out,"MINI SAVE AND RETURN INPUT: %u us with 200 ms injected storage delay\n",elapsed);
    MC("Save and return stays responsive while the pause snapshot writes",
       pause_return_pending&&paused&&screen==1&&elapsed<30000);
    pause_test_press(PSP_CTRL_CIRCLE);
    MC("Circle cancels leaving while the original save can finish",paused&&screen==1&&!pause_return_pending);
    pause_test_press(PSP_CTRL_CROSS);pause_test_wait();
    checkpoint_test_delay(0);
    MC("Save and return adds the running mini-game to the menu",
       screen==7&&!paused&&mini_resume_id==MINI_SLOT&&mini_cursor==MINI_SLOT);
    MC("the checkpoint contains the paid spin and exact time remaining",
       game_load(&disk,mini_save_path)&&disk.sun==25&&disk.mini.spinning==SLOT_ROLLING&&disk.mini.spin_time==time);
    MC("leaving reuses the pause snapshot and retains the earlier backup",
       prior_saved&&game_load(&disk,"selftest-mini-ui-round.dat.bak")&&disk.sun==75);
    MC("mini-game saving leaves the Adventure checkpoint intact",
       adventure_saved&&game_load(&disk,adventure_path)&&disk.level==6&&disk.sun==725);
    mini_cursor=MINI_ZOMBOTANY;
    pause_test_press(PSP_CTRL_CROSS);
    MC("choosing another mini-game opens its own intro",screen==2&&game_mini_active(&game,MINI_ZOMBOTANY));
    pause_test_press(PSP_CTRL_CIRCLE);menu_read_test_wait();
    MC("backing out of an intro keeps the earlier checkpoint",screen==7&&mini_resume_id==MINI_SLOT);
    pause_test_press(PSP_CTRL_TRIANGLE);menu_read_test_wait();
    MC("Triangle resumes the saved round instead of the selected card",
       screen==1&&mini_cursor==MINI_SLOT&&game.sun==25&&game.mini.spin_time==time&&game.mini.spinning==SLOT_ROLLING);
    const char *blocked_temp="selftest-mini-ui-round.dat.tmp";
    int blocked=sceIoMkdir(blocked_temp,0777)==0;
    game.sun=125;
    checkpoint_test_delay(100);
    pause_test_press(PSP_CTRL_START);pause_test_press(PSP_CTRL_DOWN);pause_test_press(PSP_CTRL_CROSS);
    pause_test_wait();checkpoint_test_delay(0);
    MC("a failed write keeps the current board paused with a warning",
       blocked&&screen==1&&paused&&!pause_return_pending&&game.sun==125&&strstr(message,"COULD NOT SAVE"));
    MC("a failed write preserves the last complete checkpoint",
       game_load(&disk,mini_save_path)&&disk.sun==25);
    checkpoint_take_error();
    if(blocked)sceIoRmdir(blocked_temp);
    pause_test_press(PSP_CTRL_CROSS);pause_test_wait();
    MC("retry saves the live board before returning to the list",
       screen==7&&mini_resume_id==MINI_SLOT&&message_time==0&&game_load(&disk,mini_save_path)&&disk.sun==125);
    pause_test_press(PSP_CTRL_TRIANGLE);menu_read_test_wait();
    checkpoint_test_delay(100);
    int previous_write=checkpoint_queue_observed(&player,profile_path,NULL,NULL,NULL,NULL);
    game.sun=130;pause_test_press(PSP_CTRL_START);
    int paused_before_queue=paused&&pause_snapshot==CHECKPOINT_IDLE;
    checkpoint_flush();pause_test_press(0);
    int queued_after_pause=pause_snapshot==CHECKPOINT_WRITING&&checkpoint_flush();
    checkpoint_test_delay(0);
    MC("a busy worker picks up the new pause snapshot when it is free",
       previous_write>0&&paused_before_queue&&queued_after_pause&&
       game_load(&disk,mini_save_path)&&disk.sun==130);
    pause_test_press(PSP_CTRL_START);
    int saved=save_current();
    FILE *damaged=fopen(mini_save_path,"wb");
    int corrupted=0;
    if(damaged){int wrote=fputc(0,damaged);corrupted=fclose(damaged)==0&&wrote!=EOF;}
    checkpoint_test_delay(150);
    started=sceKernelGetSystemTimeWide();
    open_mini_menu(-1);
    elapsed=sceKernelGetSystemTimeWide()-started;
    MC("the list opens while the worker checks for a saved Mini-game",
       screen==7&&menu_read==READ_MINI_HINT&&menu_read_queued&&elapsed<30000);
    started=sceKernelGetSystemTimeWide();
    pause_test_press(PSP_CTRL_TRIANGLE);
    unsigned load_elapsed=sceKernelGetSystemTimeWide()-started;
    fprintf(out,"MINI READ INPUT: list %u us, resume %u us with 150 ms delay\n",elapsed,load_elapsed);
    MC("Triangle reuses the pending Mini-games lookup",
       screen==7&&menu_read==READ_MINI_ROUND&&menu_read_queued&&load_elapsed<30000);
    pause_test_press(PSP_CTRL_CIRCLE);
    checkpoint_flush();pause_test_press(0);
    MC("cancelling the read keeps the list open",
       screen==7&&menu_read==READ_NONE&&menu_launch.mode==LAUNCH_NONE);
    checkpoint_test_delay(0);
    pause_test_press(PSP_CTRL_TRIANGLE);menu_read_test_wait();
    MC("a damaged primary round resumes from its backup",
       saved&&corrupted&&screen==1&&game.mini.spin_time==time&&strstr(message,"RECOVERED"));
    game.status=WON;game.mini.spinning=SLOT_READY;
    pause_test_press(PSP_CTRL_CIRCLE);board_exit_test_wait();
    pause_test_press(PSP_CTRL_TRIANGLE);menu_read_test_wait();
    MC("an unclaimed trophy survives leaving and resuming",
       screen==1&&game.status==WON&&!(game.level_flags&LEVEL_REWARD_COLLECTED));
    reward_elapsed=2;reward_delay=0;reward_row=game.cursor_row=2;reward_col=game.cursor_col=4;
    pause_test_press(PSP_CTRL_CROSS);
    reward_collect_time=REWARD_FLIGHT_DURATION;pause_test_press(PSP_CTRL_CROSS);board_exit_test_wait();
    MC("a claimed trophy is recorded and clears Resume",
       screen==7&&mini_resume_id<0&&(player.mini_trophies&(1u<<MINI_SLOT)));
    mini_cursor=MINI_ZOMBOTANY;
    pause_test_press(PSP_CTRL_CROSS);pause_test_press(PSP_CTRL_CROSS);
    // Lawn minigames can still be on the seed picker before play begins.
    game.level_flags|=LEVEL_READY;game.status=LOST;screen=1;
    pause_test_press(PSP_CTRL_CROSS);board_exit_test_wait();
    MC("a lost round returns to its card without a Resume option",
       screen==7&&mini_cursor==MINI_ZOMBOTANY&&mini_resume_id<0);
    checkpoint_flush();
    for(int i=0;i<4;i++){
        char backup[100];snprintf(backup,sizeof(backup),"%s.bak",fixtures[i]);
        remove(fixtures[i]);remove(backup);
    }
    adventure_path=paths[0];profile_path=paths[1];progress_path=paths[2];mini_save_path=paths[3];
    game=old_game;adventure_backup=old_backup;player=old_player;progress=old_progress;
    screen=old_screen;replaying=old_replaying;can_resume=old_resume;mini_cursor=old_cursor;
    mini_resume_id=old_mini_resume;paused=old_pause;help=old_help;previous_buttons=0;
    pause_return_pending=old_return_pending;pause_snapshot=old_pause_snapshot;
    message_time=0;reset_level_ui();
    #undef MC
    fprintf(out,"MINI CHECKPOINT UI RESULT %d/%d\n",pass,total);
    return pass==total;
}
