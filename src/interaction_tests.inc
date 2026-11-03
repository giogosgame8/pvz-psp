    game_start_minigame(&g,1,901);g.spawn_timer=10000;int hit=game_spawn_zombie(&g,2,Z_NORMAL,plant_x(2));
    CHECK("PS3 whack: regular zombie takes one body hit",game_whack(&g,2,2)&&g.zombies[hit].death>0&&g.kills==1&&g.sun==15);
    CHECK("whacking a dying zombie cannot award sun twice",!game_whack(&g,2,2)&&g.sun==15);
    game_start_minigame(&g,1,902);g.spawn_timer=10000;hit=game_spawn_zombie(&g,2,Z_CONE,plant_x(2));
    CHECK("PS3 whack: first cone hit removes helmet without body overflow",game_whack(&g,2,2)&&g.zombies[hit].armor==0&&g.zombies[hit].hp==270&&!g.kills&&!g.sun);
    CHECK("PS3 whack: second cone hit kills the exposed zombie",game_whack(&g,2,2)&&g.kills==1&&g.zombies[hit].death>0&&g.sun==15);
    game_start_minigame(&g,1,903);g.spawn_timer=10000;hit=game_spawn_zombie(&g,2,Z_BUCKET,plant_x(2));
    CHECK("PS3 whack: first bucket hit deals 900 helmet damage",game_whack(&g,2,2)&&g.zombies[hit].armor==200&&g.zombies[hit].hp==270&&!g.kills&&!g.sun);
    CHECK("PS3 whack: second bucket hit removes the remaining helmet",game_whack(&g,2,2)&&g.zombies[hit].armor==0&&g.zombies[hit].hp==270&&!g.kills&&!g.sun);
    CHECK("PS3 whack: buckethead dies on the third hit",game_whack(&g,2,2)&&g.kills==1&&g.sun==15);
    game_start_minigame(&g,1,904);hit=game_spawn_zombie(&g,2,Z_BUCKET,plant_x(2));g.zombies[hit].armor=100;
    CHECK("damaged armor still blocks body damage for that whack",game_whack(&g,2,2)&&g.zombies[hit].armor==0&&g.zombies[hit].hp==270&&!g.kills);
    int all_falls=1;
    for(int kind=0;kind<Z_TYPES;kind++){
        game_start_level(&g,9,920+kind);hit=game_spawn_zombie(&g,2,kind,300);Zombie *corpse=&g.zombies[hit];kill_zombie(&g,corpse);
        float duration=game_death_duration(corpse),end=(death_frames[kind]-1)/corpse->vault_from,x=corpse->x;
        game_animate_deaths(&g,end*.5f);all_falls&=corpse->used&&corpse->death>0&&fabsf(game_zombie_animation_time(corpse)*death_source_fps[kind]-(death_frames[kind]-1)*.5f)<.01f&&corpse->x==x;
        game_animate_deaths(&g,end*.5f+.10f);all_falls&=corpse->used&&game_death_alpha(corpse)==1;
        game_animate_deaths(&g,.35f);all_falls&=corpse->used&&game_death_alpha(corpse)>0&&game_death_alpha(corpse)<1;
        game_animate_deaths(&g,duration);all_falls&=!corpse->used&&g.kills==1;
    }
    CHECK("all zombie death clocks reach the final frame, hold, fade, then despawn",all_falls);
    game_start_level(&g,1,950);g.spawn_timer=10000;hit=game_spawn_zombie(&g,2,Z_NORMAL,300);kill_zombie(&g,&g.zombies[hit]);g.status=WON;
    CHECK("level progression waits for collecting the reward",!game_next_level(&g)&&g.level==1);
    CHECK("reward cannot be collected while the last zombie is falling",!game_collect_reward(&g)&&game_dying_count(&g)==1);
    float corpse_time=game_death_duration(&g.zombies[hit]);game_animate_deaths(&g,corpse_time+.1f);
    CHECK("last death keeps animating after the level has been won",!game_dying_count(&g)&&g.status==WON);
    CHECK("collecting the landed reward acknowledges completion",game_collect_reward(&g)&&(g.level_flags&LEVEL_REWARD_COLLECTED));
    CHECK("reward collection happens only once",!game_collect_reward(&g));
    saved=game_save(&g,"selftest-reward.dat");CHECK("reward collection survives save and resume",saved&&game_load(&restored,"selftest-reward.dat")&&(restored.level_flags&LEVEL_REWARD_COLLECTED));remove("selftest-reward.dat");
    CHECK("collected reward permits advancement and resets the next award",game_next_level(&g)&&g.level==2&&!(g.level_flags&LEVEL_REWARD_COLLECTED));
    uint64_t awarded_seeds=0;int award_types=1;
    for(int level=1;level<=50;level++){
        game_start_level(&g,level,960+level);int type=game_reward_type(&g),seed=game_reward_seed(&g);
        int expected=level%10==9?REWARD_NOTE:level==50?REWARD_SILVER_SUNFLOWER:level==4?REWARD_SHOVEL:level==14?REWARD_ALMANAC:level==24?REWARD_KEYS:level==34?REWARD_TACO:level==44?REWARD_WATERING_CAN:REWARD_SEED;
        award_types&=type==expected;if(type==REWARD_SEED){award_types&=seed>=0;if(seed>=0)awarded_seeds|=UINT64_C(1)<<seed;}
    }
    CHECK("all fifty Adventure rewards match the original reward sequence",award_types&&awarded_seeds==(((UINT64_C(1)<<40)-1)&~UINT64_C(1)));
    game_start_minigame(&g,0,1011);int trophy=game_reward_type(&g)==REWARD_TROPHY;game_start_minigame(&g,1,1012);
    CHECK("standalone Bowling and Whack use collectible trophies",trophy&&game_reward_type(&g)==REWARD_TROPHY);
    game_start_level(&g,9,1013);hit=game_spawn_zombie(&g,2,Z_NORMAL,300);g.zombies[hit].death=1.5f;g.zombies[hit].age=.4f;
    saved=legacy071_fixture(&g,"selftest-v6.dat",6);
    CHECK("0.7 save migration preserves position and repairs a pending death",saved&&game_load(&restored,"selftest-v6.dat")&&restored.zombies[hit].x==300&&restored.zombies[hit].age==.4f&&restored.zombies[hit].vault_from==27&&restored.zombies[hit].death>.4f);remove("selftest-v6.dat");
