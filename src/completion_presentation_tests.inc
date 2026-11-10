static int completion_presentation_selftest(FILE *out){
    Game saved=game;Profile old_player=player;int old_screen=screen,old_house=house_cursor,old_return=completion_return;
    int pass=0,total=0;
    #define CP(label,test) do{int ok=(test);total++;pass+=ok;fprintf(out,"completion UI %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    const int giga[]={A_GIGA_WALK,A_GIGA_WALK_NOIMP,A_GIGA_EAT,A_GIGA_EAT_NOIMP,A_GIGA_DEATH,A_GIGA_DEATH_NOIMP,A_GIGA_THROW,A_GIGA_THROW_NOIMP};
    int ready=1;for(int n=0;n<8;n++)ready&=assets[giga[n]].loaded;
    CP("all eight red-eyed Gargantuar banks are present",ready);
    game_start_level(&game,49,2140);int id=game_spawn_giga(&game,2,350);Zombie *z=&game.zombies[id];
    CP("Giga selects its red-eyed walk",zombie_visible_clip(z,id,0)==&assets[A_GIGA_WALK]);
    z->eating=1;CP("Giga retains its own eating clip",zombie_visible_clip(z,id,0)==&assets[A_GIGA_EAT]);
    game.polish.action[id]=ACTION_GARG_THROW;CP("Giga uses its own throw",zombie_visible_clip(z,id,0)==&assets[A_GIGA_THROW]);
    z->vaulted=1;CP("Giga drops the attached passenger after its release cue",zombie_visible_clip(z,id,0)==&assets[A_GIGA_THROW_NOIMP]);
    game_start_level(&game,9,2141);id=game_spawn_zombie(&game,2,Z_NORMAL,350);z=&game.zombies[id];game.completion.cosmetics=COS_DANCE;
    CP("the dance code uses the original normal-zombie dance bank",zombie_visible_clip(z,id,0)==&assets[A_ZOMBIE_DANCE]);
    z->eating=1;CP("eating interrupts the cosmetic dance",zombie_visible_clip(z,id,0)==&assets[A_ZOMBIE_EAT]);
    CP("all house styles and trophy tiers have their own frame",assets[A_HOUSE_PREVIEWS].loaded&&assets[A_HOUSE_PREVIEWS].frames==25&&assets[A_HOUSE_PREVIEWS].width==282&&assets[A_HOUSE_PREVIEWS].height==156);
    CP("pinata pieces retain the five original artwork variants",assets[A_SECRET_PINATA].loaded&&assets[A_SECRET_PINATA].frames==5);
    CP("secret face and moustache components cover every dance frame",avatar_assets()&&avatar_art[AV_FACE_DANCE].frames==5*assets[A_ZOMBIE_DANCE].frames&&avatar_art[AV_MUSTACHE_DANCE].frames==3*assets[A_ZOMBIE_DANCE].frames);
    game_start_adventure(&game,1,2145,1);CP("repeat Adventure uses the original Dave seed-picking dialogue",story_level_id()==1601);
    game_start_adventure(&game,11,2146,1);CP("repeat Adventure does not replay the first night tutorial",!story_level_id());
    game_start_adventure(&game,5,2147,1);CP("repeat bowling uses its own original Dave passage",story_level_id()==2411);
    screen=29;house_cursor=0;completion_ui_input(PSP_CTRL_LEFT);CP("house D-pad navigation wraps left",house_cursor==4&&screen==29);
    completion_ui_input(PSP_CTRL_RIGHT);CP("house D-pad navigation wraps right",house_cursor==0);
    completion_ui_input(PSP_CTRL_CIRCLE);CP("leaving the house chooser returns to Player",screen==25);
    screen=28;completion_return=25;completion_ui_input(PSP_CTRL_CIRCLE);CP("trophy Back returns without collecting a pending reward",screen==25&&!memcmp(&player,&old_player,sizeof(player)));
    player.completion.cosmetics=COS_SUKHBIR;CP("Sukhbir routes ambient groans to the disc's weighted Foley group",scene_voice_event(FOLEY_GROAN)==FOLEY_SUKHBIR&&scene_voice_event(FOLEY_LOW_GROAN)==FOLEY_LOW_GROAN&&scene_voice_event(FOLEY_BRAINS)==FOLEY_BRAINS);
    player.completion.cosmetics=0;CP("disabling Sukhbir restores normal groans",scene_voice_event(FOLEY_GROAN)==FOLEY_GROAN);
    CP("Tall-nut and Pumpkin retain both original damaged artwork stages",damaged[A_TALLNUT][0].loaded&&damaged[A_TALLNUT][1].loaded&&damaged[A_PUMPKIN_FRONT][0].loaded&&damaged[A_PUMPKIN_FRONT][1].loaded);
    CP("defensive damage art changes below the original third-health boundaries",defender_stage(5333,8000)==0&&defender_stage(5332,8000)==1&&defender_stage(2666,8000)==1&&defender_stage(2665,8000)==2&&defender_stage(2666,4000)==0&&defender_stage(2665,4000)==1&&defender_stage(1332,4000)==2);
    const char *old_profile_path=profile_path,*old_adventure_path=adventure_path;int old_resume=can_resume,old_replay=replaying;
    FILE *blocker=fopen("selftest-save-blocker","wb");int blocked=blocker&&fwrite("file",4,1,blocker)==1;if(blocker)fclose(blocker);
    profile_path="selftest-save-blocker/profile.dat";adventure_path="selftest-save-protected.dat";can_resume=1;replaying=0;game_start_level(&game,9,2148);game.sun=100;int committed=game_save(&game,adventure_path);game.sun=200;
    int failed=!save_current();Game disk;CP("failed profile writes warn and preserve the committed Adventure",blocked&&committed&&failed&&strstr(message,"COULD NOT SAVE")&&game_load(&disk,adventure_path)&&disk.sun==100);
    remove(adventure_path);remove("selftest-save-protected.dat.bak");remove("selftest-save-blocker");profile_path=old_profile_path;adventure_path=old_adventure_path;can_resume=old_resume;replaying=old_replay;message_time=0;
    game=saved;player=old_player;screen=old_screen;house_cursor=old_house;completion_return=old_return;
    #undef CP
    fprintf(out,"COMPLETION UI RESULT %d/%d\n",pass,total);return pass==total;
}
static int review_completion(int n){
    switch(n){
    case 0:
        game_start_adventure(&game,1,2142,1);screen=3;game.seed_slots=6;
        for(int k=0;k<BASE_SEEDS&&seed_count(game.seed_mask)<6;k++)game.seed_mask|=UINT64_C(1)<<k;
        game.selected=game_seed_slot(&game,0);reset_level_ui();break;
    case 1:
        render_debug_frame("review-repeat-locked-seeds.ppm");game_start_level(&game,49,2143);screen=1;
        game_spawn_giga(&game,2,370);game_spawn_zombie(&game,1,Z_GARG,370);game_spawn_zombie(&game,3,Z_YETI,350);reset_level_ui();break;
    case 2:
        render_debug_frame("review-giga-and-yeti.ppm");game_start_level(&game,9,2144);game.completion.cosmetics=COS_DANCE|COS_FUTURE|COS_MUSTACHE|COS_TRICKED;screen=1;
        game_spawn_zombie(&game,1,Z_NORMAL,250);game_spawn_zombie(&game,2,Z_CONE,330);game_spawn_zombie(&game,3,Z_BUCKET,390);reset_level_ui();break;
    case 3:
        render_debug_frame("review-secret-cosmetics.ppm");profile_init(&player);player.adventure_finished=1;player.completion.adventure_runs=2;player.completion.gold_sunflower=1;player.mini_trophies=0xfffff;player.garden.puzzle_wins=0x7fdff;player.garden.survival_wins=1023;screen=28;break;
    case 4:render_debug_frame("review-gold-sunflower-award.ppm");screen=29;house_cursor=1;break;
    case 5:render_debug_frame("review-scary-house.ppm");house_cursor=4;break;
    case 6:render_debug_frame("review-fun-house.ppm");screen=25;player_hub_row=2;break;
    case 7:render_debug_frame("review-player-completion.ppm");secret_begin();break;
    case 8:render_debug_frame("review-secret-code-entry.ppm");review_win(1,0,0);game.completion.adventure_run=1;break;
    case 9:
        render_debug_frame("review-repeat-moneybag.ppm");game_start_level(&game,9,2180);screen=1;reset_level_ui();
        for(int row=0;row<4;row++){game.plants[row][2]=(Plant){.kind=TALLNUT,.hp=row==0?8000:row==1?5000:2000};game.plants[row][4]=(Plant){.kind=SUNFLOWER,.hp=300,.attack=20};game.pumpkin_hp[row][4]=row==0?4000:row==1?2500:900;if(row==3){game.imitated[row][2]=1;game.imitated[row][4]=4;}}break;
    case 10:render_debug_frame("review-defender-damage.ppm");return 1;
    }return 0;
}
