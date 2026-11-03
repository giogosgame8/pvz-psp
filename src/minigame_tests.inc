int game_mini_selftest(FILE *out){
    Game g,q;int pass=0,total=0;
    #define MT(label,x) do{int ok=(x);pass+=ok;total++;fprintf(out,"mini %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    for(int id=0;id<20;id++){
        game_start_mini(&g,id,2010+id);int valid=g.mini.id==id+1&&g.challenge&&g.num_waves>=1&&g.num_waves<=40&&g.status==PLAYING&&game_reward_type(&g)==REWARD_TROPHY;
        for(int w=0;w<g.num_waves;w++)valid&=g.wave_count[w]>0&&g.wave_count[w]<=WAVE_CAPACITY;
        char title[96];snprintf(title,sizeof(title),"%s initializes its challenge and wave plan",mini_names[id]);MT(title,valid);
        int saved=game_save(&g,"selftest-mini.dat");snprintf(title,sizeof(title),"%s saves its complete state",mini_names[id]);MT(title,saved&&game_load(&q,"selftest-mini.dat")&&!memcmp(&g,&q,sizeof(g)));
        g.wave=g.num_waves;g.spawned=1;g.spawn_timer=100000;g.mini.stage=4;g.mini.phase=1;g.boss_hp=0;
        if(g.mode==MODE_MATCH)g.mini.score=75;if(g.mode==MODE_SLOT)g.sun=2000;
        if(id==MINI_STARS){static const int stars[5]={8,24,126,56,72};for(int r=0;r<5;r++)for(int c=0;c<9;c++)if(stars[r]&(1<<c))put_plant(&g,r,c,STAR);g.mini.star_count=14;}
        if(g.mode==MODE_AQUARIUM){g.sun=1000;g.mini.upgrade=1;game_mini_action(&g,1);}else game_step(&g,.05f);
        snprintf(title,sizeof(title),"%s reaches its own completion condition",mini_names[id]);MT(title,g.status==WON&&game_collect_reward(&g));
    }
    remove("selftest-mini.dat");
    game_start_mini(&g,MINI_BEGHOULED,1);g.spawn_timer=100000;for(int r=0;r<5;r++)for(int c=0;c<8;c++)g.plants[r][c].kind=(r+c)%2?PEA:WALLNUT;
    g.plants[0][0].kind=g.plants[0][1].kind=PEA;g.plants[0][2].kind=WALLNUT;g.plants[1][2].kind=PEA;
    g.cursor_row=0;g.cursor_col=2;game_mini_action(&g,0);g.cursor_row=1;g.cursor_col=2;
    MT("adjacent swap scores actual matching groups",game_mini_action(&g,0)>0&&g.mini.score>0&&g.mini.timer>0);
    game_start_mini(&g,MINI_BEGHOULED,2);g.sun=1000;g.selected=0;MT("Beghouled upgrade charges original 1000 sun",game_mini_action(&g,1)>0&&g.sun==0&&(g.mini.upgrade&1));
    g.sun=1000;MT("Beghouled upgrade cannot be bought twice",game_mini_action(&g,1)<0&&g.sun==1000);
    game_start_mini(&g,MINI_TWIST,3);g.cursor_row=4;g.cursor_col=8;MT("Twist rejects rotations outside its 8 by 5 board",game_mini_action(&g,0)<0);
    game_start_mini(&g,MINI_BEGHOULED,31);for(int r=0;r<5;r++)for(int c=0;c<8;c++)g.plants[r][c].kind=r*8+c;MT("stuck-board detection checks possible moves without scoring",!mini_has_move(&g)&&!g.mini.score);for(int n=0;n<53;n++)mini_pre_step(&g,.1f);MT("a stuck board reshuffles without charging sun",!g.mini.phase&&!g.sun&&!g.mini.score);
    game_start_mini(&g,MINI_AQUARIUM,4);MT("aquarium starts with two living snorkel zombies",g.mini.fish[0].used&&g.mini.fish[1].used&&!g.mini.fish[2].used);
    g.sun=20;int brain=game_mini_action(&g,0);game_mini_action(&g,0);game_mini_action(&g,0);MT("feeding charges 5 sun and caps loose brains at three",brain>0&&g.sun==5&&game_mini_action(&g,0)<0&&g.sun==5);
    for(int i=0;i<100;i++)g.mini.fish[i].used=0;game_step(&g,.05f);MT("losing all aquarium zombies loses the challenge",g.status==LOST);
    game_start_mini(&g,MINI_SLOT,5);g.sun=50;MT("slot spin charges 25 sun and cannot overlap",game_mini_action(&g,1)>0&&g.sun==25&&game_mini_action(&g,1)<0);g.spawn_timer=100000;for(int i=0;i<65;i++)game_step(&g,.05f);MT("slot reels stop on legal symbols",!g.mini.spinning&&g.mini.reels[0]>=0&&g.mini.reels[0]<=51);
    game_start_mini(&g,MINI_PORTAL,6);g.spawn_timer=100000;int z=game_spawn_zombie(&g,0,Z_NORMAL,plant_x(2));mini_portals(&g,.01f);MT("square portal moves zombie to its paired lane",g.zombies[z].row==1&&g.mini.zombie_portal[z]>0);
    g.shots[0]=(Shot){1,3,0,plant_x(9)};g.shot_vx[0]=150;mini_portals(&g,.01f);MT("round portal also routes projectiles",g.shots[0].row==4&&g.shots[0].x>plant_x(2)&&g.mini.shot_portal[0]>0);
    g.mini.portal_timer=.01f;mini_portals(&g,.02f);MT("portals relocate after 90 seconds",g.mini.portal_generation==1&&g.mini.portal_timer==60);
    game_start_mini(&g,MINI_COLUMN,7);g.packets[0]=MELON;g.packet_count=1;g.selected=0;int place=game_place(&g,2,1,MELON),count=0;for(int r=0;r<5;r++)count+=g.plants[r][1].kind==MELON;MT("one conveyor packet fills all five supported column tiles",place==0&&count==5&&!g.packet_count);
    game_start_mini(&g,MINI_LITTLE,8);z=game_spawn_zombie(&g,1,Z_CONE,400);MT("little zombies use quarter body and helmet health",g.zombies[z].hp==67.5f&&g.zombies[z].armor==92.5f);
    game_start_mini(&g,MINI_SPEED,9);game_step(&g,.05f);MT("speed challenge advances the entire simulation at double speed",fabsf(g.time-.10f)<.001f);
    game_start_mini(&g,MINI_LASTSTAND,10);float timer=g.spawn_timer;game_step(&g,.05f);MT("Last Stand preparation cannot spawn zombies",g.sun==5000&&g.spawn_timer==timer&&!g.total_spawned&&!(g.unlocked_mask&(UINT64_C(1)<<SUNFLOWER)));
    MT("Last Stand explicitly starts its onslaught",game_mini_action(&g,4)>0&&g.mini.phase==1&&g.spawn_timer==2);
    g.wave=g.num_waves;g.spawned=1;g.sun=100;game_step(&g,.05f);MT("Last Stand awards 250 sun and preparation between flags",g.mini.stage==1&&!g.mini.phase&&g.sun==350&&g.status==PLAYING);
    game_start_mini(&g,MINI_BOWLING2,11);g.packets[0]=GIANT_NUT;g.packet_count=1;game_place(&g,2,1,GIANT_NUT);z=game_spawn_zombie(&g,2,Z_BUCKET,g.nuts[0].x+13);bowling_step(&g,.05f);MT("giant nut kills armored zombies and continues straight",g.zombies[z].death>0&&g.nuts[0].used&&g.nuts[0].target_row==2);
    game_start_mini(&g,MINI_ZOMBOTANY,12);z=game_spawn_zombie(&g,2,Z_BUCKET,300);MT("Wall-nut zombie has 1100 head armor",g.mini.zombie_heads[z]==2&&g.zombies[z].armor==1100);
    z=game_spawn_zombie(&g,2,Z_NORMAL,300);mini_zombie(&g,z,2);MT("Peashooter zombie fires toward the player's plants",g.mini.enemy_used[0]&&g.mini.enemy_x[0]<g.zombies[z].x);
    put_plant(&g,2,4,WALLNUT);put_plant(&g,1,4,WALLNUT);g.mini.enemy_x[0]=plant_x(4)+2;mini_pre_step(&g,.01f);MT("hostile peas damage the matching lane only",g.plants[2][4].hp==3980&&g.plants[1][4].hp==4000);
    game_start_mini(&g,MINI_BOSS,13);MT("standalone Zomboss has 60000 health",g.boss_hp==60000);
    game_start_mini(&g,MINI_HEAVY,14);game_heavy_control(&g,1,0,1,.05f);MT("Heavy Weapon moves mower and fires up through the lawn",g.mini.heavy_x>100&&g.shots[0].used&&g.shot_vy[0]<0);
    g.sun=1000;g.selected=3;MT("Threepeater requires the Repeater first",game_mini_action(&g,1)<0&&g.sun==1000);
    g.selected=1;int first=game_mini_action(&g,1);MT("Repeater upgrade costs 200 sun",first>0&&g.sun==800&&g.extra.heavy_level==1);memset(g.shots,0,sizeof(g.shots));g.mini.heavy_fire=0;game_heavy_control(&g,0,0,1,.05f);count=0;for(int i=0;i<MAX_SHOTS;i++)count+=g.shots[i].used;MT("Repeater fires two spread shots",count==2);
    g.selected=3;game_mini_action(&g,1);memset(g.shots,0,sizeof(g.shots));g.mini.heavy_fire=0;game_heavy_control(&g,0,0,1,.05f);count=0;for(int i=0;i<MAX_SHOTS;i++)count+=g.shots[i].used;MT("Threepeater costs 400 and fires three shots",g.sun==400&&count==3);
    g.selected=4;game_mini_action(&g,1);memset(g.shots,0,sizeof(g.shots));g.mini.heavy_fire=0;game_heavy_control(&g,0,0,1,.05f);MT("Torchwood costs 125 and sets peas on fire temporarily",g.sun==275&&g.shot_damage[0]==40&&g.extra.heavy_fire_time>0);
    g.selected=5;game_mini_action(&g,1);memset(g.shots,0,sizeof(g.shots));g.mini.heavy_fire=0;game_heavy_control(&g,0,0,1,.05f);MT("Cattail upgrade launches homing spikes",g.sun==75&&g.extra.heavy_level==3&&g.shots[0].ice==11);
    g.suns[0]=(Sun){.used=1,.x=g.mini.heavy_x,.y=230};mini_pre_step(&g,.01f);MT("Heavy Weapon collects sun using the mower",g.suns[0].used==2&&game_collect(&g)==0);
    memset(g.shots,0,sizeof(g.shots));memset(g.zombies,0,sizeof(g.zombies));z=game_spawn_zombie(&g,2,Z_NORMAL,210);g.shots[0]=(Shot){1,2,8,200};g.shot_y[0]=game_ground_y(&g,2)-17;g.shot_vx[0]=150;g.shot_vy[0]=0;g.shot_damage[0]=20;g.shot_target[0]=-1;chapter_shots(&g,.05f);MT("ordinary mower peas stop after one hit",!g.shots[0].used&&g.zombies[z].hp==250);
    game_start_level(&g,9,15);g.sun=1000;game_place(&g,2,1,PEA);g.spawn_timer=100000;game_spawn_zombie(&g,2,Z_NORMAL,350);g.plants[2][1].attack=0;game_step(&g,.05f);int shots=0;for(int i=0;i<MAX_SHOTS;i++)shots+=g.shots[i].used;MT("Peashooter starts shooting clip before releasing its pea",g.pea_shoot_time[2][1]>0&&!shots);for(int i=0;i<8;i++)game_step(&g,.05f);shots=0;for(int i=0;i<MAX_SHOTS;i++)shots+=g.shots[i].used;MT("Peashooter releases pea at the shooting pose",shots==1);
    FILE *legacy=fopen("legacy08-save.dat","rb");if(legacy){fclose(legacy);MT("0.8 save imports into expanded challenge state",game_load(&q,"legacy08-save.dat")&&q.level>=1&&q.level<=50&&q.seed_slots>=6);}
    Profile p;profile_init(&p);game_start_mini(&g,MINI_BOSS,80);g.status=WON;g.level_flags|=LEVEL_REWARD_COLLECTED;profile_apply(&p,&g);MT("mini-game proxy level cannot unlock Adventure",p.highest==1&&!p.adventure_finished&&(p.mini_trophies&(1u<<MINI_BOSS)));
    legacy=fopen("legacy08-profile.dat","rb");if(legacy){fclose(legacy);MT("0.8 wallet imports without granting mini trophies",profile_load(&p,"legacy08-profile.dat")&&!p.mini_trophies);}
    #undef MT
    fprintf(out,"MINIGAME RESULT %d/%d\n",pass,total);return pass==total;
}
