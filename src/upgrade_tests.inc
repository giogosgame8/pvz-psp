    game_start_level(&g,9,1201);g.seed_mask=g.unlocked_mask=(UINT64_C(1)<<ALL_SEEDS)-1;g.sun=100000;g.spawn_timer=100000;
    CHECK("Gatling rejects bare lawn without spending sun",game_place(&g,2,2,GATLING)==-12&&g.sun==100000);
    CHECK("Gatling replaces a Repeater",!game_place(&g,2,2,REPEATER)&&!game_place(&g,2,2,GATLING)&&g.plants[2][2].kind==GATLING&&g.sun==99550);
    int target=game_spawn_zombie(&g,2,Z_BUCKET,450);g.zombies[target].speed=0;g.plants[2][2].attack=0;advance(&g,1.4f);int peas=0;for(int i=0;i<MAX_SHOTS;i++)peas+=g.shots[i].used&&g.shots[i].ice==0;
    CHECK("Gatling fires four separated peas in one attack",peas==4);
    game_start_level(&g,29,1202);g.seed_mask=g.unlocked_mask=(UINT64_C(1)<<ALL_SEEDS)-1;g.sun=100000;g.spawn_timer=100000;
    CHECK("Cattail requires a Lily Pad",game_place(&g,2,2,CATTAIL)==-12&&!game_place(&g,2,2,LILY)&&!game_place(&g,2,2,CATTAIL)&&g.plants[2][2].kind==CATTAIL&&!g.support[2][2]);
    game_start_level(&g,9,1203);g.seed_mask=g.unlocked_mask=(UINT64_C(1)<<ALL_SEEDS)-1;g.sun=100000;g.spawn_timer=100000;g.imitater_seed=PEA;
    CHECK("Imitater begins as a pending transformation",!game_place(&g,2,2,IMITATER)&&g.plants[2][2].kind==-1&&g.morph_timer[2][2]>3);
    CHECK("a pending Imitater reserves its tile",game_place(&g,2,2,PEA)==-2);
    advance(&g,3.3f);CHECK("Imitater finishes as its selected plant",g.plants[2][2].kind==PEA&&g.imitated[2][2]);
    CHECK("copied seed and original have independent recharge",*game_seed_cooldown(&g,IMITATER)>0&&*game_seed_cooldown(&g,PEA)==0);
    game_start_level(&g,9,1204);g.seed_mask=g.unlocked_mask=(UINT64_C(1)<<ALL_SEEDS)-1;g.sun=100000;g.spawn_timer=100000;
    game_place(&g,2,2,KERNEL);*game_seed_cooldown(&g,KERNEL)=0;game_place(&g,2,3,KERNEL);
    CHECK("Cob occupies two adjacent Kernel-pults",!game_place(&g,2,2,COB)&&g.plants[2][2].kind==COB&&g.plants[2][3].kind==-2);
    CHECK("Cob cannot fire until its initial loading completes",!game_cob_select(&g,2,3));advance(&g,6.3f);
    CHECK("either half selects a ready Cob",game_cob_select(&g,2,3)&&g.cob_col==2&&game_cob_fire(&g,3,7)&&g.plants[2][2].attack>34);
    advance(&g,2.2f);int cob_shot=-1;for(int i=0;i<MAX_SHOTS;i++)if(g.shots[i].used&&g.shots[i].ice==13)cob_shot=i;
    CHECK("Cob retains the selected row and column during launch delay",cob_shot>=0&&g.shots[cob_shot].row==3&&g.shot_damage[cob_shot]==7);
    game_shovel(&g,2,3);CHECK("shovelling either half removes the complete Cob",g.plants[2][2].kind==-1&&g.plants[2][3].kind==-1);
    game_start_level(&g,9,1205);g.seed_mask=g.unlocked_mask=(UINT64_C(1)<<ALL_SEEDS)-1;g.sun=100000;g.store_flags=STORE_FIRST_AID;g.spawn_timer=100000;
    game_place(&g,2,2,WALLNUT);g.plants[2][2].hp=100;*game_seed_cooldown(&g,WALLNUT)=0;
    CHECK("First Aid permits replacing a damaged Wall-nut",!game_place(&g,2,2,WALLNUT)&&g.plants[2][2].hp==4000);
    game_add_coin(&g,200,180,5);CHECK("coins are only credited when collection reaches the wallet",game_collect_coins(&g)==1&&!g.money_collected);game_animate_pickups(&g,.5f);
    CHECK("gold coins credit five ten-dollar units once",g.money_collected==5&&!game_collect_coins(&g));
    saved=game_save(&g,"selftest-upgrades.dat");CHECK("upgrade state and pickups survive save/load",saved&&game_load(&restored,"selftest-upgrades.dat")&&!memcmp(&g,&restored,sizeof(g)));remove("selftest-upgrades.dat");
    game_start_level(&g,49,1206);CHECK("roof cleaners require the shop upgrade",!game_cleaner_active(&g,0));g.store_flags=STORE_ROOF;CHECK("purchased roof cleaners protect roof rows",game_cleaner_active(&g,0));
    game_start_level(&g,29,1207);g.spawn_timer=100000;g.mower_used[2]=1;g.mower_x[2]=GRID_X-1;advance(&g,.1f);CHECK("ordinary mowers sink on reaching pool water",g.mower_x[2]>520);
    g.store_flags=STORE_POOL;g.mower_x[2]=GRID_X-1;advance(&g,.1f);CHECK("purchased pool cleaners cross the water",g.mower_x[2]>GRID_X&&g.mower_x[2]<520);
    game_start_level(&g,9,1208);g.spawn_timer=100000;g.store_flags=STORE_RAKE;advance(&g,.02f);target=game_spawn_zombie(&g,g.rake_row,Z_BUCKET,plant_x(g.rake_col));g.zombies[target].speed=0;advance(&g,.5f);
    CHECK("rake triggers on contact and damages an armored zombie",g.rake_state==2&&g.zombies[target].death>0);advance(&g,2.2f);CHECK("a used rake does not reset during the same level",g.rake_state==3);
    game_start_level(&g,9,1210);g.spawn_timer=100000;g.store_flags=STORE_RAKE;advance(&g,.02f);
    float rake_x=plant_x(g.rake_col);
    int ally=game_spawn_zombie(&g,g.rake_row,Z_NORMAL,rake_x);
    g.hypnotized[ally]=1;g.frozen[ally]=5;
    int flying=game_spawn_zombie(&g,g.rake_row,Z_IMP,rake_x);
    g.polish.action[flying]=ACTION_IMP_FLIGHT;g.frozen[flying]=5;
    int underground=game_spawn_zombie(&g,g.rake_row,Z_DIGGER,rake_x);
    g.frozen[underground]=5;
    advance(&g,.5f);
    CHECK("rake stays armed under a friendly, airborne or digging zombie",g.rake_state==1);
    target=game_spawn_zombie(&g,g.rake_row,Z_BUCKET,rake_x);
    g.frozen[target]=5;advance(&g,.5f);
    CHECK("rake impact skips ineligible zombies that precede a valid target",
          g.rake_state==2&&g.zombies[target].death>0&&!g.zombies[ally].death&&
          !g.zombies[flying].death&&!g.zombies[underground].death);
    game_start_level(&g,9,1211);g.spawn_timer=100000;g.store_flags=STORE_RAKE;advance(&g,.02f);
    rake_x=plant_x(g.rake_col);
    target=game_spawn_zombie(&g,g.rake_row,Z_NORMAL,rake_x);
    g.frozen[target]=5;advance(&g,.1f);
    int started=g.rake_state==2;
    g.hypnotized[target]=1;advance(&g,.5f);
    CHECK("rake rechecks a zombie that turns friendly before impact",started&&g.rake_state==2&&!g.zombies[target].death);

    game_start_level(&g,9,1212);g.spawn_timer=100000;
    g.plants[2][3]=(Plant){.kind=MINE,.hp=300,.age=MINE_ARM_SECONDS};
    underground=game_spawn_zombie(&g,2,Z_DIGGER,plant_x(3));
    g.frozen[underground]=5;
    flying=game_spawn_zombie(&g,2,Z_IMP,plant_x(3));
    g.polish.action[flying]=ACTION_IMP_FLIGHT;g.frozen[flying]=5;
    advance(&g,.1f);
    CHECK("an armed mine ignores underground and airborne contact",g.plants[2][3].kind==MINE);
    target=game_spawn_zombie(&g,2,Z_NORMAL,plant_x(3));advance(&g,.1f);
    CHECK("a mine explosion leaves underground and airborne actors unharmed",
          g.plants[2][3].kind<0&&g.zombies[target].death>0&&!g.zombies[underground].death&&!g.zombies[flying].death);

    game_start_level(&g,9,1213);g.spawn_timer=100000;
    g.plants[2][3]=(Plant){.kind=SPIKEWEED,.hp=300};
    int front_vehicle=game_spawn_zombie(&g,2,Z_ZOMBONI,plant_x(3));
    int rear_vehicle=game_spawn_zombie(&g,2,Z_CATAPULT,plant_x(3)+8);
    g.frozen[front_vehicle]=g.frozen[rear_vehicle]=5;advance(&g,.1f);
    CHECK("spikeweed pops just one vehicle when consumed",g.plants[2][3].kind<0&&
          g.zombies[front_vehicle].death>0&&g.zombies[rear_vehicle].death<=0);
    g.plants[2][3]=(Plant){.kind=SPIKEROCK,.hp=450};
    ally=game_spawn_zombie(&g,2,Z_NORMAL,plant_x(3));
    g.hypnotized[ally]=1;g.frozen[ally]=5;
    underground=game_spawn_zombie(&g,2,Z_DIGGER,plant_x(3));
    g.frozen[underground]=5;
    target=game_spawn_zombie(&g,2,Z_NORMAL,plant_x(3));
    g.frozen[target]=5;advance(&g,.1f);
    CHECK("spikerock hits ground enemies and survives a vehicle without hurting friendly or underground actors",
          g.plants[2][3].kind==SPIKEROCK&&g.plants[2][3].hp==400&&g.zombies[rear_vehicle].death>0&&
          g.zombies[target].hp==230&&g.zombies[ally].hp==270&&g.zombies[underground].hp==270);
    game_start_level(&g,9,1209);g.sun=10000;g.seed_mask=g.unlocked_mask=(UINT64_C(1)<<ALL_SEEDS)-1;g.spawn_timer=100000;game_place(&g,2,2,SPIKEWEED);game_place(&g,2,2,SPIKEROCK);target=game_spawn_zombie(&g,2,Z_GARG,plant_x(2)+20);g.zombies[target].speed=0;advance(&g,1.4f);
    CHECK("Gargantuar strikes a Spikerock instead of walking through it",g.zombies[target].eating&&g.plants[2][2].kind==SPIKEROCK&&g.plants[2][2].hp==400);
