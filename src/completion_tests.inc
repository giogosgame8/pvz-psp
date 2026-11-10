{
    Game restored;game_start_adventure(&g,1,21001,0);
    CHECK("completion first Adventure retains its tutorial and single lane",g.row_mask==4&&g.sun==150&&!g.completion.locked_seeds);
    game_start_adventure(&g,1,21001,1);int locked_count=0;for(int k=0;k<ALL_SEEDS;k++)locked_count+=game_seed_locked(&g,k);
    CHECK("completion second Adventure opens five lanes and all forty base plants",g.row_mask==31&&g.sun==50&&g.unlocked_mask==((UINT64_C(1)<<BASE_SEEDS)-1)&&(g.level_flags&LEVEL_TUTORIAL_COMPLETE));
    CHECK("completion Dave chooses three distinct usable base packets",locked_count==3&&g.seed_mask==g.completion.locked_seeds&&!(g.seed_mask&((UINT64_C(1)<<LILY)|(UINT64_C(1)<<POT)|(UINT64_C(1)<<SEA))));
    uint64_t old_locked=g.completion.locked_seeds;g.rng=2323;game_restart_adventure(&g);
    CHECK("completion retry retains the run and Dave's original choices",g.completion.adventure_run==1&&g.completion.locked_seeds==old_locked&&g.completion.level_seed==21001);
    g.status=WON;g.level_flags|=LEVEL_REWARD_COLLECTED;int next=game_next_level(&g);
    CHECK("completion advancing a level retains the second run",next&&g.level==2&&g.completion.adventure_run==1);
    int valid_choices=1;for(int level=1;level<=50;level++){game_start_adventure(&g,level,21000+level,1);int n=0;for(int k=0;k<ALL_SEEDS;k++)n+=game_seed_locked(&g,k);valid_choices&=(g.mode==MODE_LAWN?n==3:n==0)&&completion_valid(&g);}
    CHECK("completion all fifty repeat stages have valid saved seed restrictions",valid_choices);
    game_start_adventure(&g,40,21040,0);int yetis=0;for(int w=0;w<g.num_waves;w++)for(int n=0;n<g.wave_count[w];n++)yetis+=g.wave_types[w][n]==Z_YETI;
    CHECK("completion first Adventure cannot introduce a Yeti",yetis==0);
    game_start_adventure(&g,40,21040,1);yetis=0;int midway=0;for(int w=0;w<g.num_waves;w++)for(int n=0;n<g.wave_count[w];n++)if(g.wave_types[w][n]==Z_YETI){yetis++;midway=w==g.num_waves/2;}
    CHECK("completion second 4-10 queues one Yeti at its halfway wave",yetis==1&&midway);
    g.spawn_timer=g.sky_timer=10000;int id=game_spawn_zombie(&g,1,Z_YETI,350);
    CHECK("completion Yeti has 1350 health and a fifteen-to-twenty-second escape timer",g.zombies[id].hp==1350&&g.completion.yeti_timer[id]>=15&&g.completion.yeti_timer[id]<=20);
    float countdown=g.completion.yeti_timer[id];g.frozen[id]=1;advance(&g,.5f);
    CHECK("completion freezing Yeti pauses its retreat timer",g.completion.yeti_timer[id]==countdown);
    g.frozen[id]=0;g.completion.yeti_timer[id]=.01f;float x=g.zombies[id].x;advance(&g,.1f);
    CHECK("completion Yeti turns and retreats to the right",g.completion.yeti_retreat[id]&&g.zombies[id].x>x&&!g.zombies[id].eating);
    CHECK("completion repeat-run choices and retreat phase survive saving",game_save(&g,"completion-state.dat")&&game_load(&restored,"completion-state.dat")&&!memcmp(&g.completion,&restored.completion,sizeof(g.completion)));
    g.zombies[id].x=529;advance(&g,1);CHECK("completion escaped Yeti grants no kill or loot",!g.zombies[id].used&&!g.kills&&!g.coins);
    id=game_spawn_zombie(&g,1,Z_NORMAL,350);CHECK("completion reused zombie slots clear Yeti movement state",!g.completion.yeti_retreat[id]&&!g.completion.yeti_timer[id]);
    game_start_adventure(&g,40,21041,1);id=game_spawn_zombie(&g,1,Z_YETI,350);hurt_zombie(&g,&g.zombies[id],2000,0);int diamonds=0;for(int n=0;n<MAX_COINS;n++)diamonds+=g.money[n].used&&g.money[n].value==100;
    CHECK("completion killing Yeti releases four diamonds",diamonds==4&&g.kills==1);
    game_start_adventure(&g,1,21042,2);id=game_spawn_zombie(&g,2,Z_YETI,350);hurt_zombie(&g,&g.zombies[id],2000,0);diamonds=0;for(int n=0;n<MAX_COINS;n++)diamonds+=g.money[n].used&&g.money[n].value==100;
    CHECK("completion later-run early-level Yetis retain all four diamonds",diamonds==4&&g.kills==1);
    game_start_level(&g,49,21049);id=game_spawn_giga(&g,2,350);CHECK("completion Giga is a six-thousand-health Gargantuar variant",id>=0&&g.completion.giga[id]&&g.zombies[id].kind==Z_GARG&&g.zombies[id].hp==6000);
    g.spawn_timer=g.sky_timer=10000;g.zombies[id].hp=3001;advance(&g,.1f);int no_throw=!g.polish.action[id];g.zombies[id].hp=2999;advance(&g,.1f);
    CHECK("completion Giga throws its Imp below half of its own health",no_throw&&g.polish.action[id]==ACTION_GARG_THROW);
    CHECK("completion Giga variant survives a save roundtrip",game_save(&g,"completion-giga.dat")&&game_load(&restored,"completion-giga.dat")&&restored.completion.giga[id]&&restored.zombies[id].hp==2999);
    g.zombies[id].used=0;id=game_spawn_zombie(&g,2,Z_GARG,350);CHECK("completion normal Gargantuar cannot inherit a recycled Giga flag",!g.completion.giga[id]&&g.zombies[id].hp==3000);
    uint32_t old_header[4]={0x31535650,11,offsetof(Game,completion),checksum(&g,offsetof(Game,completion))};FILE *old_file=fopen("completion-v11.dat","wb");int wrote=old_file&&fwrite(old_header,sizeof(old_header),1,old_file)==1&&fwrite(&g,old_header[2],1,old_file)==1;if(old_file)fclose(old_file);
    CHECK("completion version eleven saves import with a fresh campaign tail",wrote&&game_load(&restored,"completion-v11.dat")&&!restored.completion.adventure_run&&!restored.completion.locked_seeds&&restored.level==49);
    g.completion.yeti_timer[0]=NAN;game_save(&g,"completion-invalid.dat");CHECK("completion nonfinite campaign timers are rejected",!game_load(&restored,"completion-invalid.dat"));
    game_start_adventure(&g,1,2130,1);CHECK("completion repeat tutorial stages use full twenty-wave boards",g.num_waves==20&&game_seed_slot(&g,0)>=0&&game_seed_locked(&g,game_seed_slot(&g,0)));
    game_start_adventure(&g,1,2131,1);g.status=WON;CHECK("completion repeat seed rewards become money bags",game_reward_type(&g)==REWARD_MONEY_BAG&&game_reward_seed(&g)<0);
    game_collect_reward(&g);int paid=g.money_collected;game_collect_reward(&g);CHECK("completion repeated reward clicks cannot duplicate bag coins",paid==25&&g.money_collected==paid);
    game_start_adventure(&g,9,2132,1);g.status=WON;CHECK("completion repeat chapter notes retain their original reward",game_reward_type(&g)==REWARD_NOTE);
    game_start_level(&g,9,2133);g.spawn_timer=g.sky_timer=10000;id=game_spawn_zombie(&g,2,Z_NORMAL,300);g.completion.cosmetics=COS_PINATA|COS_DAISIES;
    uint32_t old_rng=g.rng;completion_death_effect(&g,&g.zombies[id]);int flowers=0,candy=0,upward=1;
    for(int n=0;n<24;n++){CosmeticParticle *p=&g.completion.particles[n];flowers+=p->life>0&&p->type==1;candy+=p->life>0&&p->type==2;if(p->type==2&&p->life>0)upward&=p->vy<0;}
    CHECK("completion combined death cosmetics coexist without altering gameplay RNG",flowers==3&&candy==10&&upward&&g.rng==old_rng);
    completion_effects_step(&g,2);flowers=candy=0;for(int n=0;n<24;n++){flowers+=g.completion.particles[n].life>0&&g.completion.particles[n].type==1;candy+=g.completion.particles[n].life>0&&g.completion.particles[n].type==2;}
    CHECK("completion candy expires while planted daisies remain",flowers==3&&!candy&&completion_valid(&g));
    for(int n=0;n<24;n++)g.completion.particles[n]=(CosmeticParticle){.type=1,.life=170,.age=10};completion_death_effect(&g,&g.zombies[id]);candy=0;for(int n=0;n<24;n++)candy+=g.completion.particles[n].type==2&&g.completion.particles[n].life>0;
    CHECK("completion a full daisy pool cannot suppress later pinata bursts",candy==10&&completion_valid(&g));
    game_start_adventure(&g,5,2135,1);CHECK("completion repeat bowling skips the first-run shovel exercise",!g.shovel_tutorial&&g.plants[2][0].kind<0&&g.plants[2][1].kind<0&&g.plants[2][2].kind<0);
    game_start_adventure(&g,1,2136,1);g.plants[2][0]=(Plant){.kind=PEA,.hp=300};game_shovel(&g,2,0);CHECK("completion repeat 1-1 retains the earned shovel",g.plants[2][0].kind<0);
    game_start_adventure(&g,39,2150,1);int before_yeti=allowed_in_game(&g,Z_YETI);game_start_adventure(&g,41,2151,1);int after_yeti=allowed_in_game(&g,Z_YETI);game_start_adventure(&g,1,2152,2);
    CHECK("completion rare Yeti eligibility opens after the encounter and on later runs",!before_yeti&&after_yeti&&allowed_in_game(&g,Z_YETI)&&zombie_weight[Z_YETI]==1);
    game_start_level(&g,49,2153);int premature=0;for(int w=0;w<g.num_waves;w++)for(int n=0;n<g.wave_count[w];n++){int kind=g.wave_types[w][n];premature+=w+1<zombie_first_wave[kind];}
    CHECK("pacing advanced Adventure enemies respect the disc's first-allowed waves",!premature);
    int red_seen=0,red_limits=1;for(int seed=1;seed<=20;seed++){
        game_start_survival(&g,2,SURVIVAL_ENDLESS,2154+seed);g.extra.stage=5;g.extra.flags=10;g.completion.giga_allowed=1;g.extra.allowed_zombies|=1u<<Z_GARG;memset(g.wave_count,0,sizeof(g.wave_count));plan_waves(&g);
        int ordinary_red=0;for(int w=0;w<g.num_waves;w++){int red=0;for(int n=0;n<g.wave_count[w];n++)if(g.completion.wave_giga[w][n]){red++;red_seen++;red_limits&=w>=14;}if(game_flag_wave(&g,w))red_limits&=red<=1;else ordinary_red+=red;}red_limits&=ordinary_red<=1;
    }
    CHECK("pacing early Endless Gigas have separate flag and ordinary-wave caps",red_seen>0&&red_limits);
    game_start_survival(&g,2,SURVIVAL_ENDLESS,2175);g.extra.flags=50;
    CHECK("pacing later Endless opens advanced arrivals earlier while reducing plain-zombie weight",spawn_first_wave(&g,Z_GARG)==1&&wave_pick_weight(&g,0,Z_NORMAL)==400&&wave_pick_weight(&g,0,Z_BUNGEE)==0);
    const char *recovery="completion-recovery.dat";remove(recovery);remove("completion-recovery.dat.bak");game_start_level(&g,9,2134);g.sun=100;int first=game_save(&g,recovery);g.sun=200;int second=game_save(&g,recovery);
    FILE *damaged=fopen(recovery,"wb");if(damaged){fwrite("broken",6,1,damaged);fclose(damaged);}
    CHECK("save damaged gameplay primary restores previous committed state",first&&second&&game_load(&restored,recovery)&&game_load_backup_used()&&restored.sun==100);
    restored.sun=300;int repaired=game_save(&restored,recovery);damaged=fopen(recovery,"wb");if(damaged){fwrite("broken",6,1,damaged);fclose(damaged);}
    CHECK("save repairing gameplay never rotates corrupt data into its backup",repaired&&game_load(&restored,recovery)&&game_load_backup_used()&&restored.sun==100);
    remove(recovery);remove("completion-recovery.dat.bak");restored.sun=432;CHECK("save absent gameplay files preserve the caller's state",!game_load(&restored,recovery)&&restored.sun==432);
    for(int n=0;n<4;n++){const char *path=(const char*[]){"completion-state.dat","completion-giga.dat","completion-v11.dat","completion-invalid.dat"}[n];char backup[96];snprintf(backup,sizeof(backup),"%s.bak",path);remove(path);remove(backup);}
}
