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
        if(id==MINI_STARS)for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)
            if(game_mini_star_targets(r)&(1u<<c))put_plant(&g,r,c,STAR);
        if(g.mode==MODE_AQUARIUM){g.sun=1000;g.mini.upgrade=1;game_mini_action(&g,1);}else game_step(&g,.05f);
        snprintf(title,sizeof(title),"%s reaches its own completion condition",mini_names[id]);MT(title,g.status==WON&&game_collect_reward(&g));
    }
    remove("selftest-mini.dat");
    game_start_mini(&g,MINI_STARS,700);
    g.spawn_timer=100000;
    for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)
        if(game_mini_star_targets(r)&(1u<<c))put_plant(&g,r,c,STAR);
    MT("Seeing Stars has fourteen marked spaces within the active lawn",
       game_mini_star_count(&g)==14&&game_mini_total(&g)==14&&game_mini_star_targets(5)==0);
    g.mini.star_count=14;
    g.plants[4][6].hp=1;
    int eater=game_spawn_zombie(&g,4,Z_NORMAL,plant_x(6)+8);
    put_plant(&g,4,8,STAR);
    game_step(&g,1.0f/60);
    MT("Seeing Stars stays open if a zombie destroys a required fruit",
       eater>=0&&g.plants[4][6].kind<0&&g.status==PLAYING&&g.mini.star_count==13);
    game_shovel(&g,4,8);
    MT("Starfruit outside the pattern do not contribute to completion",game_mini_star_count(&g)==13);
    if(eater>=0)g.zombies[eater].used=0;
    g.sun=game_seed_cost(&g,STAR);
    int placed=game_place(&g,4,6,STAR);
    game_step(&g,1.0f/60);
    MT("Seeing Stars awards the trophy on the step the last plant survives",
       placed==0&&g.status==WON&&g.mini.star_count==14&&game_reward_type(&g)==REWARD_TROPHY);
    game_start_mini(&g,MINI_STARS,703);
    for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)
        if(game_mini_star_targets(r)&(1u<<c))put_plant(&g,r,c,STAR);
    g.polish.wave_pending=1;
    game_step(&g,1.0f/60);
    MT("a completed star pattern stops new wave entries before awarding its trophy",
       g.status==WON&&g.mini.star_count==14&&!g.total_spawned);
    game_start_mini(&g,MINI_STARS,701);
    g.wave=g.num_waves;g.spawned=1;g.polish024.rise_state=1;
    mini_progress(&g);
    MT("the objective waits for pending final-wave reinforcements",
       g.wave==g.num_waves&&g.spawned==1&&g.status==PLAYING);
    g.polish024.rise_state=0;mini_progress(&g);
    MT("an unfinished objective keeps the challenge active after the last enemy",
       g.wave==g.num_waves-1&&!g.spawned&&g.spawn_timer==5&&g.status==PLAYING);
    game_start_mini(&g,MINI_BEGHOULED,702);
    g.polish.wave_pending=1;g.mini.timer=1;g.mini.score=game_mini_total(&g);
    for(int i=0;i<MAX_ZOMBIES;i++)game_spawn_zombie(&g,0,Z_NORMAL,475);
    game_step(&g,1.0f/60);
    MT("a completed match score wins even when a full zombie pool blocks the next wave",
       g.status==WON&&g.polish.wave_pending&&game_alive(&g)==MAX_ZOMBIES&&game_reward_type(&g)==REWARD_TROPHY);
    game_start_mini(&g,MINI_BEGHOULED,1);g.spawn_timer=100000;for(int r=0;r<5;r++)for(int c=0;c<8;c++)g.plants[r][c].kind=(r+c)%2?PEA:WALLNUT;
    g.plants[0][0].kind=g.plants[0][1].kind=PEA;g.plants[0][2].kind=WALLNUT;g.plants[1][2].kind=PEA;
    g.cursor_row=0;g.cursor_col=2;game_mini_action(&g,0);g.cursor_row=1;g.cursor_col=2;
    MT("adjacent swap scores actual matching groups",game_mini_action(&g,0)>0&&g.mini.score>0&&g.mini.timer>0);
    game_start_mini(&g,MINI_BEGHOULED,2);g.sun=1000;g.selected=0;MT("Beghouled upgrade charges original 1000 sun",game_mini_action(&g,1)>0&&g.sun==0&&(g.mini.upgrade&1));
    g.sun=1000;MT("Beghouled upgrade cannot be bought twice",game_mini_action(&g,1)<0&&g.sun==1000);
    game_start_mini(&g,MINI_TWIST,3);g.cursor_row=4;g.cursor_col=8;MT("Twist rejects rotations outside its 8 by 5 board",game_mini_action(&g,0)<0);
    game_start_mini(&g,MINI_BEGHOULED,31);for(int r=0;r<5;r++)for(int c=0;c<8;c++)g.plants[r][c].kind=r*8+c;MT("stuck-board detection checks possible moves without scoring",!mini_has_move(&g)&&!g.mini.score);for(int n=0;n<53;n++)mini_pre_step(&g,.1f);MT("a stuck board reshuffles without charging sun",!g.mini.phase&&!g.sun&&!g.mini.score);
    game_start_mini(&g,MINI_TWIST,301);
    g.cursor_row=0;g.cursor_col=0;g.plants[0][0].kind=-1;g.craters[0][0]=100000;q=g;
    MT("Twist rejects a block containing a crater without altering plant state",
       game_mini_action(&g,0)<0&&!memcmp(&g,&q,sizeof(g)));
    game_start_mini(&g,MINI_BEGHOULED,302);
    g.cursor_row=0;g.cursor_col=0;g.plants[0][0].kind=-1;g.craters[0][0]=100000;
    MT("Beghouled cannot select an empty crater",game_mini_action(&g,0)<0&&g.mini.selected_row<0);
    g.mini.selected_row=0;g.mini.selected_col=0;
    g.cursor_col=1;
    MT("a destroyed selection is replaced by the next valid plant",
       game_mini_action(&g,0)>0&&g.mini.selected_row==0&&g.mini.selected_col==1&&!g.mini.score);
    g.sun=200;g.selected=4;
    MT("Fill Holes repairs the board and resets the selection",
       game_mini_action(&g,1)>0&&g.sun==0&&g.craters[0][0]==0&&g.plants[0][0].kind>=0&&g.mini.selected_row<0);
    g.sun=200;g.mini.timer=0;
    MT("Fill Holes leaves sun alone on a full board",game_mini_action(&g,1)<0&&g.sun==200);
    g.selected=3;g.mini.selected_row=0;g.mini.selected_col=1;
    MT("Shuffle clears selection and the pending dead-board countdown",
       game_mini_action(&g,1)>0&&g.sun==100&&g.mini.selected_row<0&&g.mini.phase==0);
    game_start_mini(&g,MINI_BEGHOULED,303);
    for(int r=0;r<5;r++)for(int c=0;c<8;c++){
        g.plants[r][c].kind=-1;g.craters[r][c]=100000;
    }
    put_plant(&g,0,0,PEA);g.pea_shoot_time[0][0]=.18f;g.burst[0][0]=.45f;
    uint32_t falling_id=g.fidelity.plant_id[0][0][0];
    g.craters[4][0]=0;mini_fill_matches(&g);
    MT("falling plant retains its identity and queued shot",
       g.plants[4][0].kind==PEA&&g.fidelity.plant_id[0][4][0]==falling_id&&
       g.pea_shoot_time[4][0]==.18f&&g.burst[4][0]==.45f);
    mini_match_step(&g,.15f);
    MT("deep match falls continue smoothly at half the settle time",
       fabsf(g.mini.match_fall[4][0]+game_row_height(&g)*2)<.001f&&g.mini.timer>0);
    mini_match_step(&g,.15f);
    MT("the last falling plant lands before the next match",g.mini.match_fall[4][0]==0&&g.mini.timer==0);
    game_start_mini(&g,MINI_AQUARIUM,4);MT("aquarium starts with two living snorkel zombies",g.mini.fish[0].used&&g.mini.fish[1].used&&!g.mini.fish[2].used);
    g.sun=20;int brain=game_mini_action(&g,0);game_mini_action(&g,0);game_mini_action(&g,0);MT("feeding charges 5 sun and caps loose brains at three",brain>0&&g.sun==5&&game_mini_action(&g,0)<0&&g.sun==5);
    for(int i=0;i<100;i++)g.mini.fish[i].used=0;game_step(&g,.05f);MT("losing all aquarium zombies loses the challenge",g.status==LOST);
    {
        const float dt=1.0f/60;
        game_start_mini(&g,MINI_AQUARIUM,400);
        g.mini.brains[0]=(AquariumBrain){1,200,120,0};
        g.mini.fish[0]=(AquariumFish){.used=1,.x=200,.y=120+5.7f*dt,.hunger=5,.produce=100,.dx=15,.dy=5};
        g.mini.fish[1]=g.mini.fish[0];
        game_step(&g,dt);
        MT("hungry fish wait at a coincident brain without losing their position",
           g.mini.brains[0].used&&g.mini.fish[0].x==200&&g.mini.fish[0].y==120+5.7f*dt&&
           g.mini.fish[0].dx==0&&g.mini.fish[0].dy==0&&g.mini.fish[1].dx==0&&g.mini.fish[1].dy==0);
        g.mini.brains[0].age=.15f;
        game_step(&g,dt);
        MT("each aquarium brain feeds only one fish",!g.mini.brains[0].used&&
           g.mini.fish[0].hunger>24&&g.mini.fish[1].hunger<5);

        g.mini.fish[0].produce=.001f;game_step(&g,dt);
        MT("aquarium sun animates after a fish produces it",g.suns[0].used==1&&
           g.suns[0].flight>0&&g.suns[0].y<g.mini.fish[0].y);
        g.suns[0].y=g.suns[0].target_y;g.suns[0].vy=0;g.suns[0].age=7.49f;
        game_step(&g,.02f);
        MT("expired aquarium sun frees its slot",!g.suns[0].used&&g.sun==50);

        for(int i=0;i<100;i++)g.mini.fish[i].used=1;
        g.sun=100;
        MT("a full aquarium rejects a fish purchase and preserves sun",game_mini_action(&g,1)<0&&g.sun==100);
        g.mini.fish[99].used=0;
        MT("an aquarium purchase charges sun only after a fish is added",
           game_mini_action(&g,1)>0&&g.mini.fish[99].used&&g.sun==0);

        g.mini.fish[0].x=g.mini.fish[0].y=g.mini.fish[0].dx=g.mini.fish[0].dy=NAN;
        const char *save="selftest-aquarium-recovery.dat";
        MT("aquarium saves recover affected fish without resetting hunger or sun",
           game_save(&g,save)&&game_load(&q,save)&&q.mini.fish[0].used&&q.mini.fish[0].x==245&&
           q.mini.fish[0].y==152.5f&&isfinite(q.mini.fish[0].dx)&&isfinite(q.mini.fish[0].dy)&&
           q.mini.fish[0].hunger==g.mini.fish[0].hunger&&q.sun==g.sun);
        remove(save);remove("selftest-aquarium-recovery.dat.bak");
    }
    game_start_mini(&g,MINI_SLOT,5);
    g.spawn_timer=g.sky_timer=100000;
    MT("slot spin charges 25 sun and cannot overlap",game_mini_action(&g,1)>0&&g.sun==25&&game_mini_action(&g,1)<0);
    for(int i=0;i<49;i++)game_step(&g,.05f);
    MT("slot machine locks the first reel while the other two roll",
       !game_slot_reel_rolling(&g,0)&&game_slot_reel_rolling(&g,1)&&game_slot_reel_rolling(&g,2));
    int locked=g.mini.reels[0];
    const char *slot_save="selftest-slot.dat";
    int stored=game_save(&g,slot_save)&&game_load(&q,slot_save);
    for(int i=0;i<6;i++){game_step(&g,.05f);if(stored)game_step(&q,.05f);}
    MT("the second reel stops before the third without changing the first",
       g.mini.reels[0]==locked&&!game_slot_reel_rolling(&g,1)&&game_slot_reel_rolling(&g,2));
    for(int i=0;i<10;i++){game_step(&g,.05f);if(stored)game_step(&q,.05f);}
    MT("resuming a spin preserves the outcome and pays once",
       stored&&g.mini.spinning==SLOT_READY&&!memcmp(&g,&q,sizeof(g))&&g.mini.reels[0]==locked);
    g.packet_count=MAX_PACKETS-2;
    MT("a full seed tray rejects a spin without a charge",
       game_slot_spin_status(&g)==SLOT_TRAY_BLOCKED&&game_mini_action(&g,1)<0&&g.sun==25);
    g.packet_count=0;g.sun=0;g.mini.spinning=SLOT_ROLLING;g.mini.spin_time=.001f;g.mini.stage=1;
    for(int j=0;j<3;j++)g.mini.reels[j]=SLOT_SUN;
    for(int j=0;j<MAX_SUNS-1;j++)g.suns[j]=(Sun){.used=1,.x=70,.y=80,.target_y=80};
    mini_slot_step(&g,.05f);
    MT("a full sun pool retains the unpaid portion of a jackpot",
       g.mini.spinning==SLOT_PAYING&&g.mini.score==19&&g.suns[MAX_SUNS-1].from_plant==1);
    stored=game_save(&g,slot_save)&&game_load(&q,slot_save)&&!memcmp(&g,&q,sizeof(g));
    for(int i=0;i<MAX_SUNS-1;i++){g.suns[i].used=0;if(stored)q.suns[i].used=0;}
    mini_slot_step(&g,.05f);if(stored)mini_slot_step(&q,.05f);
    int paid_suns=0;
    for(int i=0;i<MAX_SUNS;i++)paid_suns+=g.suns[i].used&&g.suns[i].from_plant;
    MT("queued jackpot survives a save and emits all twenty suns",
       stored&&paid_suns==20&&g.mini.spinning==SLOT_READY&&!memcmp(&g,&q,sizeof(g)));
    remove(slot_save);remove("selftest-slot.dat.bak");
    game_start_mini(&g,MINI_SLOT,501);
    g.sun=SLOT_WIN_SUN;g.spawn_timer=g.sky_timer=100000;
    g.mini.spinning=SLOT_ROLLING;g.mini.spin_time=.16f;g.mini.stage=1;
    g.polish.wave_pending=1;
    for(int i=0;i<MAX_ZOMBIES;i++)game_spawn_zombie(&g,0,Z_NORMAL,475);
    for(int j=0;j<3;j++)g.mini.reels[j]=SLOT_DIAMOND;
    for(int j=0;j<MAX_COINS;j++)g.money[j]=(Coin){.used=1,.value=5,.x=70,.y=80,.target_y=80};
    game_step(&g,.05f);
    MT("reaching the sun goal during a spin keeps it running",g.status==PLAYING&&g.mini.spinning==SLOT_ROLLING);
    for(int i=0;i<4;i++)game_step(&g,.05f);
    MT("a full coin pool keeps the paid spin pending",g.status==PLAYING&&g.mini.spinning==SLOT_PAYING&&g.mini.score==5);
    for(int j=0;j<MAX_COINS;j++)g.money[j].used=0;
    game_step(&g,.05f);
    int paid_coins=0;
    for(int j=0;j<MAX_COINS;j++)paid_coins+=g.money[j].used&&g.money[j].value==100;
    MT("the trophy appears only after all five diamonds are emitted",
       g.status==WON&&g.mini.spinning==SLOT_READY&&paid_coins==5&&game_reward_type(&g)==REWARD_TROPHY);
    MT("a queued wave cannot delay the Slot Machine trophy once payout is done",
       g.status==WON&&g.polish.wave_pending&&game_alive(&g)==MAX_ZOMBIES);
    game_start_mini(&g,MINI_SLOT,502);
    g.mini.spinning=SLOT_ROLLING;g.mini.stage=1;g.mini.spin_time=.001f;
    for(int j=0;j<3;j++)g.mini.reels[j]=SNOW;
    g.packet_count=MAX_PACKETS-1;
    g.extra.packet_age[MAX_PACKETS-1]=11;
    mini_slot_step(&g,.05f);
    MT("a seed jackpot fills the last seat and retains the other two packets",
       g.mini.spinning==SLOT_PAYING&&g.mini.score==2&&g.packet_count==MAX_PACKETS&&
       g.packets[MAX_PACKETS-1]==SNOW&&g.extra.packet_age[MAX_PACKETS-1]==0);
    g.packet_count-=2;
    mini_slot_step(&g,.05f);
    MT("queued seed packets appear once the tray has room",
       g.mini.spinning==SLOT_READY&&g.packet_count==MAX_PACKETS&&
       g.packets[MAX_PACKETS-2]==SNOW&&g.packets[MAX_PACKETS-1]==SNOW);
    g.status=LOST;
    MT("a lost round disables its spin control",game_slot_spin_status(&g)==SLOT_INACTIVE);
    game_start_mini(&g,MINI_PORTAL,6);g.spawn_timer=100000;int z=game_spawn_zombie(&g,0,Z_NORMAL,plant_x(2));mini_portals(&g,.01f);MT("square portal moves zombie to its paired lane",g.zombies[z].row==1&&g.mini.zombie_portal[z]>0);
    shoot(&g,3,plant_x(9),0);chapter_shots(&g,.01f);MT("round portal also routes projectiles",g.shots[0].row==4&&g.shots[0].x>plant_x(2)&&g.mini.shot_portal[0]>0);
    g.mini.portal_timer=.01f;mini_portals(&g,.02f);MT("portals relocate after 90 seconds",g.mini.portal_generation==1&&g.mini.portal_timer==60);
    {
        game_start_mini(&g,MINI_PORTAL,639);
        int chewer=game_spawn_zombie(&g,0,Z_NORMAL,plant_x(2));
        g.zombies[chewer].eating=1;g.zombies[chewer].age=.35f;g.polish.pose[chewer]=.35f;
        mini_portals(&g,.01f);
        MT("a zombie leaves its old bite animation behind when crossing a portal",
           g.zombies[chewer].row==1&&!g.zombies[chewer].eating&&!g.zombies[chewer].age&&!g.polish.pose[chewer]);
        const char *portal_save="selftest-portal.dat";
        remove(portal_save);remove("selftest-portal.dat.bak");
        int first_save=game_save(&g,portal_save);
        float exit_x=g.zombies[chewer].x;
        int loaded=first_save&&game_load(&q,portal_save);
        if(loaded)mini_portals(&q,.01f);
        MT("a resumed zombie keeps the exit lock until it walks clear",
           loaded&&q.zombies[chewer].row==1&&q.zombies[chewer].x==exit_x&&q.extra.last_portal_zombie[chewer]==20);
        g.sun=75;
        int second_save=game_save(&g,portal_save);
        g.mini.portals[1]=g.mini.portals[0];
        MT("a duplicate portal endpoint cannot replace a playable checkpoint",
           second_save&&!game_save(&g,portal_save)&&game_load(&q,portal_save)&&
           !game_load_backup_used()&&q.sun==75&&q.mini.portals[1]==19);
        g.mini.portals[1]=50;
        MT("an invalid exit in a committed checkpoint recovers from the backup",
           write_board_fixture(&g,portal_save)&&game_load(&q,portal_save)&&
           game_load_backup_used()&&q.sun==0&&q.mini.portals[1]==19);
        remove(portal_save);remove("selftest-portal.dat.bak");

        game_start_mini(&g,MINI_PORTAL,640);
        float entry=plant_x(2),exit=plant_x(9);
        int in_front=game_spawn_zombie(&g,0,Z_NORMAL,entry-35);
        shoot(&g,0,entry-63,0);g.shot_vx[0]=1500;
        chapter_shots(&g,.06f);
        MT("a projectile can hit a zombie before reaching a portal",
           g.zombies[in_front].hp==250&&!g.shots[0].used&&!g.extra.last_portal_shot[0]);

        game_start_mini(&g,MINI_PORTAL,641);
        int beyond=game_spawn_zombie(&g,0,Z_NORMAL,entry+24);
        int receiving=game_spawn_zombie(&g,1,Z_NORMAL,exit+33);
        shoot(&g,0,entry-63,0);g.shot_vx[0]=1500;
        chapter_shots(&g,.06f);
        MT("fast projectiles cross the portal before hitting in its exit lane",
           g.zombies[beyond].hp==270&&g.zombies[receiving].hp==250&&
           g.shots[0].row==1&&!g.shots[0].used&&g.extra.last_portal_shot[0]==20);

        game_start_mini(&g,MINI_PORTAL,642);
        shoot(&g,1,exit+35,0);g.shot_vx[0]=-800;
        chapter_shots(&g,.06f);
        MT("a leftward projectile keeps its velocity and remaining movement",
           g.shots[0].used&&g.shots[0].row==0&&g.shot_vx[0]==-800&&
           fabsf(g.shots[0].x-(entry-48))<.01f&&g.shot_y[0]==game_ground_y(&g,0)-17);

        game_start_mini(&g,MINI_PORTAL,643);
        shoot(&g,0,entry,0);g.shot_vx[0]=0;
        chapter_shots(&g,.01f);
        float position=g.shots[0].x;
        g.mini.portal_timer=0;mini_portals(&g,.01f);chapter_shots(&g,.01f);
        MT("a stationary projectile stays at its exit when portals relocate",
           g.shots[0].row==1&&g.shots[0].x==position&&g.extra.last_portal_shot[0]==20);

        game_start_mini(&g,MINI_PORTAL,644);
        g.mini.portals[1]=14;
        put_plant(&g,0,0,TORCHWOOD);
        shoot(&g,0,plant_x(0)-9,0);g.shot_vx[0]=1000;
        chapter_shots(&g,.085f);
        MT("a projectile keeps its Torchwood fire damage through a portal",
           g.shots[0].used&&g.shots[0].row==1&&g.shots[0].ice==10&&g.shot_damage[0]==40);
    }
    game_start_mini(&g,MINI_COLUMN,7);g.packets[0]=MELON;g.packet_count=1;g.selected=0;int place=game_place(&g,2,1,MELON),count=0;for(int r=0;r<5;r++)count+=g.plants[r][1].kind==MELON;MT("one conveyor packet fills all five supported column tiles",place==0&&count==5&&!g.packet_count);
    game_start_mini(&g,MINI_COLUMN,708);
    g.packets[0]=CHOMPER;g.packets[1]=MELON;g.packets[2]=PUMPKIN;
    g.packet_count=3;g.selected=1;g.extra.packet_age[2]=1.5f;
    put_plant(&g,2,1,CHOMPER);
    g.craters[0][1]=15;
    g.support[3][1]=0;
    g.row_mask&=~(1u<<4);
    q=g;
    MT("column preview skips occupied, unsupported, cratered and inactive tiles",
       game_column_plantable(&g,1,MELON)==(1u<<1)&&!memcmp(&q,&g,sizeof(g)));
    MT("an occupied cursor tile can plant in other rows of its column",
       game_place(&g,2,1,MELON)==0&&g.plants[1][1].kind==MELON&&g.plants[2][1].kind==CHOMPER&&
       g.plants[0][1].kind<0&&g.plants[3][1].kind<0&&g.plants[4][1].kind<0);
    MT("partial column placement consumes exactly one packet and preserves the following packet age",
       g.packet_count==2&&g.selected==1&&g.packets[0]==CHOMPER&&
       g.packets[1]==PUMPKIN&&g.extra.packet_age[1]==1.5f);
    q=g;
    MT("an unsupported column rejects a packet without changing game state",
       game_place(&g,2,8,PUMPKIN)==-9&&!memcmp(&q,&g,sizeof(g)));
    g.packets[1]=POT;
    MT("a Flower Pot packet fills bare rows of a partially supported column",
       game_column_plantable(&g,1,POT)==(1u<<3)&&game_place(&g,1,1,POT)==0&&
       g.support[3][1]==POT+1&&g.support_hp[3][1]==300&&g.packet_count==1);
    g.packets[0]=PUMPKIN;
    MT("Pumpkins protect supported plants without consuming the underlying plant",
       game_place(&g,2,1,PUMPKIN)==0&&g.pumpkin_hp[1][1]==4000&&g.pumpkin_hp[2][1]==4000&&
       g.plants[1][1].kind==MELON&&g.plants[2][1].kind==CHOMPER);
    game_start_mini(&g,MINI_COLUMN,1547);
    conveyor_step(&g,1.0f/60);
    MT("the roof conveyor supplies a usable attacker when the lawn lacks one",
       g.packet_count==1&&g.packets[0]==MELON&&game_column_plantable(&g,1,MELON)==31);
    uint32_t column_rng=g.rng;
    g.conveyor_timer=0;conveyor_step(&g,1.0f/60);
    MT("a queued Melon-pult lets the roof conveyor draw from its normal deck",
       g.packet_count==2&&g.rng!=column_rng);
    game_start_mini(&g,MINI_COLUMN,1547);
    for(int r=0;r<5;r++)for(int c=0;c<2;c++)put_plant(&g,r,c,MELON);
    column_rng=g.rng;conveyor_step(&g,1.0f/60);
    MT("enough surviving attackers lets the roof conveyor draw normally",
       g.packet_count==1&&g.rng!=column_rng);
    game_start_mini(&g,MINI_COLUMN,1547);
    memset(g.support,0,sizeof(g.support));memset(g.support_hp,0,sizeof(g.support_hp));
    conveyor_step(&g,1.0f/60);
    MT("without a supported landing tile the roof conveyor supplies a Flower Pot",
       g.packet_count==1&&g.packets[0]==POT);
    game_place(&g,0,1,POT);g.conveyor_timer=0;conveyor_step(&g,1.0f/60);
    MT("after the Flower Pot is planted the recovery packet is usable",
       g.packet_count==1&&g.packets[0]==MELON&&game_column_plantable(&g,1,MELON)==31);
    game_start_mini(&g,MINI_LITTLE,8);z=game_spawn_zombie(&g,1,Z_CONE,400);MT("little zombies use quarter body and helmet health",g.zombies[z].hp==67.5f&&g.zombies[z].armor==92.5f);
    game_start_mini(&g,MINI_SPEED,9);game_step(&g,.05f);MT("speed challenge advances the entire simulation at double speed",fabsf(g.time-.10f)<.001f);
    game_start_mini(&g,MINI_LASTSTAND,10);float timer=g.spawn_timer;game_step(&g,.05f);MT("Last Stand preparation cannot spawn zombies",g.sun==5000&&g.spawn_timer==timer&&!g.total_spawned&&!(g.unlocked_mask&(UINT64_C(1)<<SUNFLOWER)));
    MT("Last Stand explicitly starts its onslaught",game_mini_action(&g,4)>0&&g.mini.phase==1&&g.spawn_timer==2);
    g.wave=g.num_waves;g.spawned=1;g.sun=100;game_step(&g,.05f);MT("Last Stand awards 250 sun and preparation between flags",g.mini.stage==1&&!g.mini.phase&&g.sun==350&&g.status==PLAYING);
    game_start_mini(&g,MINI_LASTSTAND,1001);
    g.mini.phase=1;g.wave=g.num_waves;g.spawned=1;g.polish.wave_pending=1;
    game_step(&g,.05f);
    MT("Last Stand waits for the queued final wave before ending the round",
       g.mini.stage==0&&g.mini.phase==1&&g.status==PLAYING&&game_alive(&g)>0);
    game_start_mini(&g,MINI_BOWLING2,11);g.packets[0]=GIANT_NUT;g.packet_count=1;game_place(&g,2,1,GIANT_NUT);z=game_spawn_zombie(&g,2,Z_BUCKET,g.nuts[0].x+13);bowling_step(&g,.05f);MT("giant nut kills armored zombies and continues straight",g.zombies[z].death>0&&g.nuts[0].used&&g.nuts[0].target_row==2);
    game_start_mini(&g,MINI_ZOMBOTANY,12);z=game_spawn_zombie(&g,2,Z_BUCKET,300);MT("Wall-nut zombie has 1100 head armor",g.mini.zombie_heads[z]==2&&g.zombies[z].armor==1100);
    z=game_spawn_zombie(&g,2,Z_NORMAL,300);mini_zombie(&g,z,2);MT("Peashooter zombie fires toward the player's plants",g.mini.enemy_used[0]&&g.mini.enemy_x[0]<g.zombies[z].x);
    put_plant(&g,2,4,WALLNUT);put_plant(&g,1,4,WALLNUT);g.mini.enemy_x[0]=plant_x(4)+2;mini_pre_step(&g,.01f);MT("hostile peas damage the matching lane only",g.plants[2][4].hp==3980&&g.plants[1][4].hp==4000);
    game_start_mini(&g,MINI_ZOMBOTANY2,1201);
    int headless=game_spawn_zombie(&g,0,Z_NORMAL,plant_x(4)+15);
    int hypnotized=game_spawn_zombie(&g,1,Z_NORMAL,plant_x(4)+15);
    g.mini.zombie_heads[headless]=g.mini.zombie_heads[hypnotized]=3;
    g.zombies[headless].hp=60;g.zombies[headless].armor=0;g.hypnotized[hypnotized]=1;
    g.mini.zombie_attack[headless]=g.mini.zombie_attack[hypnotized]=-9.99f;
    put_plant(&g,0,4,WALLNUT);put_plant(&g,1,4,WALLNUT);
    game_step(&g,.05f);
    MT("headless and hypnotized Jalapeno zombies cannot burn the lawn",
       g.plants[0][4].hp==4000&&g.plants[1][4].hp==4000&&
       !g.zombies[headless].death&&!g.zombies[hypnotized].death);
    game_start_mini(&g,MINI_ZOMBOTANY2,1202);
    int squash[]={game_spawn_zombie(&g,0,Z_NORMAL,plant_x(4)+15),
                  game_spawn_zombie(&g,1,Z_NORMAL,plant_x(4)+15),
                  game_spawn_zombie(&g,4,Z_NORMAL,plant_x(4)+15)};
    for(int i=0;i<3;i++){
        int row=g.zombies[squash[i]].row;
        put_plant(&g,row,4,WALLNUT);
        g.mini.zombie_heads[squash[i]]=5;
        g.extra.squash_col[squash[i]]=4;
        g.extra.zombotany_jump[squash[i]]=.21f;
    }
    g.zombies[squash[1]].hp=60;g.zombies[squash[1]].armor=0;g.hypnotized[squash[2]]=1;
    game_step(&g,.05f);
    MT("only an intact hostile Squash damages its target before landing",
       g.plants[0][4].kind<0&&g.plants[1][4].hp==4000&&g.plants[4][4].hp==4000&&
       g.extra.zombotany_jump[squash[1]]>0&&g.extra.zombotany_jump[squash[2]]>0);
    advance(&g,.25f);
    MT("interrupted Squash jumps land without triggering an attack",
       g.zombies[squash[0]].death>0&&!g.zombies[squash[1]].death&&!g.zombies[squash[2]].death&&
       !g.extra.zombotany_jump[squash[1]]&&!g.extra.zombotany_jump[squash[2]]&&
       g.plants[1][4].hp==4000&&g.plants[4][4].hp==4000);
    game_start_mini(&g,MINI_BOSS,13);MT("standalone Zomboss has 60000 health",g.boss_hp==60000);
    game_start_mini(&g,MINI_HEAVY,14);game_heavy_control(&g,1,0,1,.05f);MT("Heavy Weapon moves mower and fires up through the lawn",g.mini.heavy_x>100&&g.shots[0].used&&g.shot_vy[0]<0);
    g.sun=1800;int gated=game_heavy_upgrade_open(&g,HEAVY_REPEATER);
    for(int tool=1;tool<HEAVY_UPGRADES;tool++){g.selected=tool;gated&=!game_heavy_upgrade_open(&g,tool)&&game_mini_action(&g,1)<0;}
    MT("Repeater is the only initial upgrade",gated&&g.sun==1800);
    g.selected=HEAVY_REPEATER;int first=game_mini_action(&g,1);MT("Repeater costs 200 and unlocks the remaining shop",first>0&&g.sun==1600&&g.extra.heavy_level==1&&game_heavy_upgrade_open(&g,HEAVY_CATTAIL));
    memset(g.shots,0,sizeof(g.shots));g.mini.heavy_fire=0;game_heavy_control(&g,0,0,1,.01f);
    MT("Repeater launches two peas at the disc's 20-degree spread",g.shots[0].used&&g.shots[1].used&&!g.shots[2].used&&fabsf(g.shot_vx[0]+27.47f)<.05f&&fabsf(g.shot_vx[1]-27.47f)<.05f);
    g.selected=HEAVY_CATTAIL;MT("Cattail can be bought before Threepeater and uses its own timer",game_mini_action(&g,1)>0&&g.sun==1400&&g.extra.heavy_level==1&&g.mini.secondary==1);
    g.selected=HEAVY_THREEPEATER;game_mini_action(&g,1);memset(g.shots,0,sizeof(g.shots));g.mini.heavy_fire=g.mini.secondary=0;game_heavy_control(&g,0,0,1,.01f);
    MT("Threepeater and Cattail fire together on independent timers",g.sun==1000&&g.shots[0].used&&g.shots[2].used&&g.shots[3].ice==11&&g.mini.heavy_fire==.25f&&g.mini.secondary==1);
    g.selected=HEAVY_CATTAIL;int spikes=game_mini_action(&g,1)>0&&g.mini.secondary==.75f&&game_mini_action(&g,1)>0&&g.mini.secondary==.5f;
    MT("Cattail has three paid ranks with 1.00, 0.75, and 0.50 s timing",spikes&&g.sun==600&&game_mini_action(&g,1)<0);
    g.selected=HEAVY_SNOW;int snow=game_mini_action(&g,1)>0&&g.extra.heavy_ice_time==6&&game_mini_action(&g,1)<0;
    g.selected=HEAVY_TORCH;int torch=game_mini_action(&g,1)>0&&g.extra.heavy_ice_time==0&&g.extra.heavy_fire_time==6;
    memset(g.shots,0,sizeof(g.shots));g.mini.heavy_fire=0;game_heavy_control(&g,0,0,1,.01f);
    MT("six-second fire and ice replace each other, and fire peas deal 40",snow&&torch&&g.sun==375&&g.shots[0].ice==10&&g.shot_damage[0]==40);
    mini_pre_step(&g,6);MT("elemental shots return to normal when their timer expires",g.extra.heavy_fire_time==0&&game_heavy_upgrade_open(&g,HEAVY_TORCH));
    int close=game_spawn_zombie(&g,2,Z_NORMAL,plant_x(4)),far=game_spawn_zombie(&g,2,Z_NORMAL,480);
    g.selected=HEAVY_DOOM;game_mini_action(&g,1);MT("Doom hits the central blast radius, not zombies outside it",g.zombies[close].death>0&&g.zombies[far].hp==270&&g.craters[2][4]==180);
    int cadence=1;for(int rate=30;rate<=60;rate+=30){
        game_start_mini(&g,MINI_HEAVY,2408);for(int tick=0;tick<rate;tick++)game_heavy_control(&g,0,0,1,1.0f/rate);
        count=0;for(int i=0;i<MAX_SHOTS;i++)count+=g.shots[i].used;cadence&=count==4;
    }
    MT("held fire produces four primary shots per second at 30 and 60 FPS",cadence);
    const char *save_path="selftest-heavy-state.dat",*save_backup="selftest-heavy-state.dat.bak";
    remove(save_path);remove(save_backup);
    Game previous;game_start_mini(&previous,MINI_HEAVY,2410);
    previous.extra.heavy_level=3;previous.extra.heavy_cattail=2;previous.selected=5;
    previous.extra.heavy_fire_time=20;previous.extra.heavy_ice_time=20;
    int restored=game_save(&previous,save_path)&&game_load(&q,save_path);
    MT("older mode checkpoints restore purchased upgrades to valid slots",restored&&q.extra.heavy_level==2&&
       q.extra.heavy_cattail==2&&q.selected==HEAVY_CATTAIL&&q.extra.heavy_fire_time==6&&q.extra.heavy_ice_time==0);
    if(restored){q.selected=HEAVY_TORCH;restored=game_save(&q,save_path)&&game_load(&previous,save_path);}
    MT("resaved checkpoints keep the current slot ordering",restored&&previous.selected==HEAVY_TORCH);
    remove(save_path);remove(save_backup);
    int upper=game_spawn_zombie(&g,1,Z_NORMAL,140),lower=game_spawn_zombie(&g,4,Z_NORMAL,300);
    MT("Heavy Weapon Cattail prioritizes the zombie closest to the mower's end of the lawn",upper>=0&&lower>=0&&heavy_homing_target(&g,140)==lower);
    g.suns[0]=(Sun){.used=1,.x=g.mini.heavy_x,.y=230};mini_pre_step(&g,.01f);MT("Heavy Weapon collects sun using the mower",g.suns[0].used==2&&game_collect(&g)==0);
    memset(g.shots,0,sizeof(g.shots));memset(g.zombies,0,sizeof(g.zombies));z=game_spawn_zombie(&g,2,Z_NORMAL,210);g.shots[0]=(Shot){1,2,8,200};g.shot_y[0]=game_ground_y(&g,2)-17;g.shot_vx[0]=150;g.shot_vy[0]=0;g.shot_damage[0]=20;g.shot_target[0]=-1;chapter_shots(&g,.05f);MT("ordinary mower peas stop after one hit",!g.shots[0].used&&g.zombies[z].hp==250);
    game_start_level(&g,9,15);g.sun=1000;game_place(&g,2,1,PEA);g.spawn_timer=100000;game_spawn_zombie(&g,2,Z_NORMAL,350);g.plants[2][1].attack=0;game_step(&g,.05f);int shots=0;for(int i=0;i<MAX_SHOTS;i++)shots+=g.shots[i].used;MT("Peashooter starts shooting clip before releasing its pea",g.pea_shoot_time[2][1]>0&&!shots);for(int i=0;i<8;i++)game_step(&g,.05f);shots=0;for(int i=0;i<MAX_SHOTS;i++)shots+=g.shots[i].used;MT("Peashooter releases pea at the shooting pose",shots==1);
    FILE *legacy=fopen("legacy08-save.dat","rb");if(legacy){fclose(legacy);MT("0.8 save imports into expanded challenge state",game_load(&q,"legacy08-save.dat")&&q.level>=1&&q.level<=50&&q.seed_slots>=6);}
    Profile p;profile_init(&p);game_start_mini(&g,MINI_BOSS,80);g.status=WON;g.level_flags|=LEVEL_REWARD_COLLECTED;profile_apply(&p,&g);MT("mini-game proxy level cannot unlock Adventure",p.highest==1&&!p.adventure_finished&&(p.mini_trophies&(1u<<MINI_BOSS)));
    legacy=fopen("legacy08-profile.dat","rb");if(legacy){fclose(legacy);MT("0.8 wallet imports without granting mini trophies",profile_load(&p,"legacy08-profile.dat")&&!p.mini_trophies);}
    #undef MT
    fprintf(out,"MINIGAME RESULT %d/%d\n",pass,total);return pass==total;
}
