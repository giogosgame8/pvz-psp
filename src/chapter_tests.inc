    int chapter_layout=1,music_mapping=1,rosters=1,save50=1;
    for(int level=1;level<=50;level++){
        game_start_level(&g,level,700+level);
        int world=level==35?1:(level-1)/10;
        chapter_layout&=game_world(&g)==world&&game_rows(&g)==((world==2||world==3)?6:5)&&game_ground_y(&g,game_rows(&g)-1)<250;
        int expected=level==40?-1:level==50?10:level==35?9:level==5||level==15||level==25||level==45?3:level%10==0||level==45?4:(int[]){1,5,6,7,8}[world];
        music_mapping&=game_music_theme(&g)==expected;
        if(g.mode!=MODE_VASE&&g.mode!=MODE_BOSS){g.spawn_timer=0;advance(&g,.1f);for(int i=0;i<MAX_ZOMBIES;i++)if(g.zombies[i].used){int k=g.zombies[i].kind;int water=k==Z_DUCK||k==Z_SNORKEL||k==Z_DOLPHIN;rosters&=!((world==2||world==3)&&game_water_row(&g,g.zombies[i].row)!=water);}}
        if(level>=21){g.level_flags|=LEVEL_READY;int saved_chapter=game_save(&g,"selftest-chapter.dat");save50&=saved_chapter&&game_load(&restored,"selftest-chapter.dat")&&!memcmp(&g,&restored,sizeof(g));}
    }remove("selftest-chapter.dat");
    CHECK("all fifty level layouts fit the handheld screen",chapter_layout);
    CHECK("all fifty levels select their world or challenge music",music_mapping);
    CHECK("pool and fog spawns separate aquatic and land zombies",rosters);
    CHECK("all thirty added levels preserve their exact state on save",save50);
    game_start_level(&g,29,801);g.sun=10000;g.seed_mask=(UINT64_C(1)<<40)-1;g.spawn_timer=10000;
    CHECK("pool plants require a lily pad",game_place(&g,2,1,PEA)==-11);
    CHECK("lily pad occupies the support layer",game_place(&g,2,1,LILY)==0&&g.support[2][1]==LILY+1&&g.plants[2][1].kind==-1);
    CHECK("peashooter can be planted on a lily pad",game_place(&g,2,1,PEA)==0&&g.plants[2][1].kind==PEA);
    CHECK("lily pad rejected on dry land",game_place(&g,0,0,LILY)==-11);
    CHECK("tangle kelp rejected on dry land",game_place(&g,0,0,KELP)==-11);
    CHECK("sea-shroom grows on water without a pad",game_place(&g,3,2,SEA)==0&&g.asleep[3][2]);
    game_start_level(&g,49,802);g.sun=10000;g.seed_mask=(UINT64_C(1)<<40)-1;g.spawn_timer=10000;
    CHECK("roof plants need a flower pot",game_place(&g,2,6,CABBAGE)==-11);
    CHECK("roof pot and cabbage use separate layers",game_place(&g,2,6,POT)==0&&game_place(&g,2,6,CABBAGE)==0&&g.support[2][6]==POT+1);
    CHECK("pumpkin protects an occupied roof tile",game_place(&g,2,6,PUMPKIN)==0&&g.pumpkin_hp[2][6]==4000&&g.plants[2][6].kind==CABBAGE);
    int roof_z=game_spawn_zombie(&g,2,Z_DOOR,350);g.plants[2][6].attack=0;advance(&g,1.4f);
    CHECK("cabbage arcs over the roof and bypasses a screen door",g.zombies[roof_z].hp<270&&g.zombies[roof_z].armor==1100);
    game_start_level(&g,33,803);g.sun=10000;g.seed_mask=(UINT64_C(1)<<40)-1;g.spawn_timer=10000;int balloon=game_spawn_zombie(&g,0,Z_BALLOON,250);
    game_place(&g,0,0,CACTUS);advance(&g,2);
    CHECK("cactus pops an airborne balloon",g.zombies[balloon].kind==Z_BALLOON&&g.fidelity.equipment_lost[balloon]);
    int balloon2=game_spawn_zombie(&g,1,Z_BALLOON,330);game_place(&g,1,0,BLOVER);advance(&g,1.2f);
    CHECK("blover clears fog and removes balloons",g.fog_clear>23&&g.zombies[balloon2].death>0);
    game_start_level(&g,29,804);g.sun=10000;g.seed_mask=(UINT64_C(1)<<40)-1;g.spawn_timer=10000;
    game_place(&g,2,4,KELP);int duck=game_spawn_zombie(&g,2,Z_DUCK,plant_x(4)+20);advance(&g,1.2f);
    CHECK("tangle kelp pulls a swimmer underwater",!g.zombies[duck].used&&g.plants[2][4].kind==-1);
    game_place(&g,0,3,SQUASH);int cone=game_spawn_zombie(&g,0,Z_CONE,plant_x(3)+30);advance(&g,.1f);
    CHECK("squash looks before jumping without instant damage",g.zombies[cone].death==0&&g.zombies[cone].armor==370&&g.burst_left[0][3]==1);
    advance(&g,1.3f);CHECK("squash rises above its target before impact",g.zombies[cone].death==0&&g.burst_left[0][3]==3);
    advance(&g,.5f);CHECK("squash damages an armored zombie only on landing",g.zombies[cone].death>0&&g.burst_left[0][3]==5);
    advance(&g,1.1f);CHECK("flattened squash finishes its animation and disappears",g.plants[0][3].kind==-1);
    game_place(&g,1,3,JALAPENO);int bucket=game_spawn_zombie(&g,1,Z_BUCKET,400);g.ice_tiles[1][7]=20;advance(&g,1.2f);
    CHECK("jalapeno burns its row and clears the ice trail",g.zombies[bucket].death>0&&g.ice_tiles[1][7]==0);
    game_start_level(&g,38,805);g.sun=10000;g.seed_mask=(UINT64_C(1)<<40)-1;g.spawn_timer=10000;
    game_place(&g,0,2,MAGNET);bucket=game_spawn_zombie(&g,0,Z_BUCKET,plant_x(2)+80);advance(&g,.5f);
    CHECK("magnet-shroom removes nearby metal armor",g.zombies[bucket].armor==0);
    game_start_level(&g,39,815);g.sun=10000;g.seed_mask=(UINT64_C(1)<<40)-1;g.spawn_timer=10000;game_place(&g,0,3,MAGNET);int jack=game_spawn_zombie(&g,0,Z_JACK,plant_x(3)+80);advance(&g,.5f);
    CHECK("magnet removes a Jack-in-the-box before it explodes",g.zombies[jack].kind==Z_JACK&&g.fidelity.equipment_lost[jack]);
    game_start_level(&g,49,806);g.sun=10000;g.seed_mask=(UINT64_C(1)<<40)-1;g.spawn_timer=10000;
    game_place(&g,2,2,UMBRELLA);game_place(&g,2,3,POT);game_place(&g,2,3,CABBAGE);int bungee=game_spawn_zombie(&g,2,Z_BUNGEE,plant_x(3));g.zombies[bungee].x=plant_x(3);advance(&g,5.0f);
    CHECK("umbrella protects adjacent plants from bungee theft",g.plants[2][3].kind==CABBAGE&&!g.zombies[bungee].used);
    game_start_level(&g,35,807);g.extra.vase_content[0][6]=PEA;g.extra.vase_content[0][7]=-1-Z_NORMAL;CHECK("4-5 starts three-round Vasebreaker on a night lawn",g.mode==MODE_VASE&&g.num_waves==3&&game_rows(&g)==5&&g.vases[0][6]);
    CHECK("breaking a vase reveals a seed packet",game_break_vase(&g,0,6)&&!g.vases[0][6]&&g.packet_count==1);
    CHECK("a broken vase cannot be opened twice",!game_break_vase(&g,0,6));
    game_break_vase(&g,0,7);CHECK("zombie vases release a live enemy",game_alive(&g)==1);
    game_start_level(&g,25,808);int small=game_spawn_zombie(&g,0,Z_BUCKET,400);
    CHECK("3-5 uses miniature zombie health and a conveyor",g.mode==MODE_CONVEYOR&&g.zombies[small].hp==67.5f&&g.zombies[small].armor==275);
    game_start_level(&g,50,809);g.boss_timer=10000;g.packet_count=1;g.packets[0]=ICESHROOM;g.boss_ball_kind=1;g.boss_ball_x=445;
    CHECK("boss ice-shroom is awake on the night roof",game_place(&g,0,2,ICESHROOM)==0&&!g.asleep[0][2]);advance(&g,1.1f);
    CHECK("ice-shroom counters the boss fireball",g.boss_ball_kind==0);
    g.packet_count=1;g.selected=0;g.packets[0]=JALAPENO;g.extra.boss_action=B_HEAD_WAIT;g.boss_ball_kind=2;g.boss_ball_row=1;g.boss_ball_x=445;game_place(&g,1,2,JALAPENO);float hp=g.boss_hp;advance(&g,1.1f);
    CHECK("jalapeno counters the boss iceball and damages the boss",g.boss_ball_kind==0&&g.boss_hp<hp-1700);
    CHECK("boss is still alive after one counterattack",g.status==PLAYING&&g.boss_hp>0);
    game_start_level(&g,20,810);g.status=WON;game_collect_reward(&g);CHECK("night finale advances into the pool chapter",game_next_level(&g)&&g.level==21&&game_rows(&g)==6);
    game_start_level(&g,30,811);g.status=WON;game_collect_reward(&g);CHECK("pool finale advances into fog",game_next_level(&g)&&g.level==31&&game_music_theme(&g)==7);
    game_start_level(&g,40,812);g.status=WON;game_collect_reward(&g);CHECK("fog finale advances onto the roof",game_next_level(&g)&&g.level==41&&game_music_theme(&g)==8);
    game_start_level(&g,50,813);g.status=WON;game_collect_reward(&g);CHECK("final boss completion ends Adventure without wrapping",!game_next_level(&g)&&g.level==50);
    game_start_level(&g,19,814);g.packets[0]=16;saved=legacy_fixture(&g,"selftest-legacy5.dat",5);
    CHECK("0.6 night save migrates into the six-row layout",saved&&game_load(&restored,"selftest-legacy5.dat")&&restored.level==19&&restored.sun==g.sun&&!memcmp(restored.plants,g.plants,5*COLS*sizeof(Plant)));remove("selftest-legacy5.dat");
    {
    game_start_level(&g,9,817);
    int other_lane=game_spawn_zombie(&g,3,Z_NORMAL,208);
    int first=game_spawn_zombie(&g,2,Z_NORMAL,208);
    int second=game_spawn_zombie(&g,2,Z_NORMAL,208);
    shoot(&g,2,200,0);shoot(&g,2,200,0);
    g.shot_vx[0]=g.shot_vx[1]=0;g.shot_damage[0]=270;
    chapter_shots(&g,1.0f/60);
    CHECK("projectiles resolve tied targets in slot order and skip a fresh death",
          g.zombies[first].death>0&&g.zombies[second].hp==250&&g.zombies[other_lane].hp==270);

    game_start_level(&g,9,818);
    int edge=game_spawn_zombie(&g,2,Z_NORMAL,221);
    shoot(&g,2,200,0);g.shot_vx[0]=0;
    chapter_shots(&g,1.0f/60);
    int edge_misses=g.shots[0].used&&g.zombies[edge].hp==270;
    g.zombies[edge].x=220.75f;
    chapter_shots(&g,1.0f/60);
    CHECK("projectiles retain the strict impact radius",edge_misses&&!g.shots[0].used&&g.zombies[edge].hp==250);

    game_start_level(&g,9,819);
    int cross_lane=game_spawn_zombie(&g,1,Z_NORMAL,208);
    shoot(&g,2,200,8);g.shot_vx[0]=0;g.shot_y[0]=game_ground_y(&g,1)-17;
    chapter_shots(&g,1.0f/60);
    CHECK("Starfruit retains cross-lane contact",!g.shots[0].used&&g.zombies[cross_lane].hp==250);

    shoot(&g,-1,200,0);
    chapter_shots(&g,1.0f/60);
    CHECK("an invalid projectile lane cannot index the board",!g.shots[0].used);
    }

    {
    float torch=plant_x(4);
    game_start_level(&g,9,823);
    put_plant(&g,2,4,TORCHWOOD);
    shoot(&g,2,torch-9,1);
    chapter_shots(&g,.02f);
    CHECK("a single Torchwood thaws an ice pea without increasing its damage",
          g.shots[0].ice==0&&g.shot_damage[0]==20);
    int recovered=game_save(&g,"selftest-torchwood.dat")&&game_load(&restored,"selftest-torchwood.dat");
    if(recovered)chapter_shots(&restored,.1f);
    CHECK("a saved thawed pea leaves the same Torchwood without turning into fire",
          recovered&&restored.shots[0].used&&restored.shots[0].ice==0&&restored.shot_damage[0]==20);
    remove("selftest-torchwood.dat");
    remove("selftest-torchwood.dat.bak");
    chapter_shots(&g,.02f);
    CHECK("successive ticks inside one Torchwood do not ignite a thawed pea",
          g.shots[0].ice==0&&g.shot_damage[0]==20);

    game_start_level(&g,9,824);
    put_plant(&g,2,4,TORCHWOOD);
    put_plant(&g,2,3,TORCHWOOD);
    shoot(&g,2,torch+13,1);g.shot_vx[0]=-680;
    chapter_shots(&g,.1f);
    CHECK("a leftward ice pea crosses two distinct Torchwoods in travel order",
          g.shots[0].used&&g.shots[0].ice==10&&g.shot_damage[0]==40&&g.shot_target[0]==2*COLS+3);

    game_start_level(&g,9,825);
    put_plant(&g,2,4,TORCHWOOD);
    int victim=game_spawn_zombie(&g,2,Z_NORMAL,torch-8);
    shoot(&g,2,torch-32,0);g.shot_vx[0]=320;
    chapter_shots(&g,.1f);
    CHECK("a pea that hits before Torchwood deals normal damage",!g.shots[0].used&&
          g.shots[0].ice==0&&g.zombies[victim].hp==250);
    g.zombies[victim].x=torch+32;
    shoot(&g,2,torch-32,0);g.shot_vx[0]=690;
    chapter_shots(&g,.1f);
    CHECK("a pea sweeping through Torchwood deals fire damage on impact beyond it",!g.shots[0].used&&
          g.shots[0].ice==10&&g.zombies[victim].hp==210);
    }

    {
    game_start_level(&g,9,820);
    int front=game_spawn_zombie(&g,2,Z_NORMAL,230);
    int digger=game_spawn_zombie(&g,2,Z_DIGGER,250);
    game_spawn_zombie(&g,1,Z_NORMAL,235);
    ZombieRows targets;
    targets.count=-1;
    g.zombies[digger].vaulted=0;
    int began=target_in_lane(&g,2,200,&targets);
    g.zombies[front].used=0;
    int removed=!target_in_lane(&g,2,200,&targets);
    g.zombies[digger].vaulted=1;
    CHECK("plant targeting rechecks removals and eligibility within a tick",
          began&&removed&&target_in_lane(&g,2,200,&targets));
    shoot(&g,2,222,0);g.shot_vx[0]=0;
    chapter_shots_step(&g,1.0f/60,&targets);
    CHECK("projectiles ignore targets removed after the shared scan",g.shots[0].used);
    }

    {
    game_start_level(&g,9,821);
    float x=plant_x(3);
    put_plant(&g,2,3,SPLIT);g.plants[2][3].attack=0;
    game_spawn_zombie(&g,2,Z_NORMAL,x-50);
    int other_lane=shoot(&g,1,x-12,0),same_lane=shoot(&g,2,x-18,0);
    ZombieRows targets;
    targets.count=-1;
    chapter_plant(&g,2,3,1.0f/60,&targets);
    CHECK("Split Pea reverses only its own new rear shots",
          other_lane==0&&same_lane==1&&g.shot_vx[0]==150&&g.shot_vx[1]==150&&
          g.shots[2].used&&g.shots[3].used&&g.shot_vx[2]==-150&&g.shot_vx[3]==-150);

    game_start_level(&g,9,822);
    for(int i=0;i<MAX_SHOTS;i++)g.shots[i].used=1;
    int full=shoot(&g,2,x,0)==-1&&g.events[SFX_SHOOT]==0;
    g.shots[6].used=0;g.shot_target[6]=9;g.polish024.shot_actor[6]=12;
    g.extra.last_portal_shot[6]=20;g.mini.shot_portal[6]=1;
    CHECK("a reused projectile slot clears previous target and portal state",
          full&&shoot(&g,2,x,0)==6&&g.shot_target[6]==-1&&!g.polish024.shot_actor[6]&&
          !g.extra.last_portal_shot[6]&&!g.mini.shot_portal[6]);

    g.shots[6].used=0;g.shots[MAX_SHOTS-1].used=0;
    put_plant(&g,2,3,STAR);g.plants[2][3].attack=0;
    game_spawn_zombie(&g,2,Z_NORMAL,400);targets.count=-1;
    chapter_plant(&g,2,3,1.0f/60,&targets);
    CHECK("a crowded Starfruit volley fills available slots in direction order",
          g.shots[6].ice==8&&g.shot_vx[6]==-150&&g.shot_vy[6]==0&&
          g.shots[MAX_SHOTS-1].ice==8&&g.shot_vx[MAX_SHOTS-1]==0&&g.shot_vy[MAX_SHOTS-1]==-150);
    }

    {
    game_start_level(&g,9,826);
    float x=plant_x(4);
    put_plant(&g,2,4,SPLIT);g.plants[2][4].attack=0;
    int ally=game_spawn_zombie(&g,2,Z_NORMAL,x-50);
    g.hypnotized[ally]=1;
    int buried=game_spawn_zombie(&g,2,Z_DIGGER,x-60);
    int flying=game_spawn_zombie(&g,2,Z_IMP,x+52);
    g.polish.action[flying]=ACTION_IMP_FLIGHT;
    ZombieRows targets={.count=-1};
    chapter_plant(&g,2,4,1.0f/60,&targets);
    g.extra.left_facing[2][4]=1;
    CHECK("Split Pea and left-facing shooters ignore friendly, underground and airborne actors",
          !g.shots[0].used&&g.plants[2][4].attack==0&&!extra_target(&g,2,4,x,&targets));
    g.zombies[buried].vaulted=1;
    g.polish.action[buried]=ACTION_DIGGER_RISE;
    CHECK("a Digger is untargetable while it emerges",!extra_target(&g,2,4,x,&targets));
    g.polish.action[buried]=ACTION_NONE;
    CHECK("left-facing shooters can target a surfaced Digger",extra_target(&g,2,4,x,&targets));
    for(int i=0;i<MAX_SHOTS;i++)g.shots[i].used=1;
    chapter_plant(&g,2,4,1.0f/60,&targets);
    CHECK("Split Pea stays ready when no projectile slot is available",g.plants[2][4].attack==0);
    g.shots[7].used=0;
    chapter_plant(&g,2,4,1.0f/60,&targets);
    CHECK("Split Pea fires backwards as soon as one slot frees up",
          g.shots[7].used&&g.shot_vx[7]==-150&&g.plants[2][4].attack>0);
    }

    {
    game_start_level(&g,33,827);
    float x=plant_x(4);
    put_plant(&g,0,4,CACTUS);g.plants[0][4].attack=0;
    int balloon=game_spawn_zombie(&g,0,Z_BALLOON,x+90);
    int walker=game_spawn_zombie(&g,0,Z_NORMAL,x+45);
    ZombieRows targets={.count=-1};
    chapter_plant(&g,0,4,1.0f/60,&targets);
    int high=g.shots[0].used&&g.shot_target[0]==SHOT_HITS_BALLOON&&
             g.shot_y[0]==game_ground_y(&g,0)-17-BALLOON_LIFT;
    g.shot_vx[0]=850;
    int recovered=game_save(&g,"selftest-cactus.dat")&&game_load(&restored,"selftest-cactus.dat");
    if(recovered)chapter_shots(&restored,.1f);
    CHECK("a saved high Cactus spike passes a walker and pops the balloon behind it",
          high&&recovered&&!restored.shots[0].used&&restored.zombies[walker].hp==270&&
          restored.fidelity.equipment_lost[balloon]&&restored.polish.action[balloon]==ACTION_BALLOON_FALL);
    remove("selftest-cactus.dat");
    remove("selftest-cactus.dat.bak");

    game_start_level(&g,33,828);
    put_plant(&g,0,4,CACTUS);g.plants[0][4].attack=0;
    balloon=game_spawn_zombie(&g,0,Z_BALLOON,x-45);
    targets.count=-1;
    chapter_plant(&g,0,4,1.0f/60,&targets);
    CHECK("Cactus does not fire at a balloon behind it",!g.shots[0].used&&g.plants[0][4].attack==0);
    walker=game_spawn_zombie(&g,0,Z_NORMAL,x+82);
    targets.count=-1;
    chapter_plant(&g,0,4,1.0f/60,&targets);
    int low=g.shots[0].used&&g.shot_target[0]==SHOT_HITS_GROUND&&g.shot_y[0]==game_ground_y(&g,0)-17;
    g.zombies[balloon].x=x+42;g.shot_vx[0]=850;
    chapter_shots(&g,.1f);
    CHECK("a low Cactus spike passes below a balloon and hits a walker",
          low&&!g.shots[0].used&&!g.fidelity.equipment_lost[balloon]&&g.zombies[walker].hp==250);
    }
