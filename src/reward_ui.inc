static const char *reward_name(void){
    int type=game_reward_type(&game),seed=game_reward_seed(&game);
    switch(type){case REWARD_SHOVEL:return "SHOVEL";case REWARD_ALMANAC:return "ALMANAC";case REWARD_KEYS:return "CAR KEYS";case REWARD_TACO:return "TACO";case REWARD_WATERING_CAN:return "WATERING CAN";case REWARD_NOTE:return "ZOMBIE NOTE";case REWARD_TROPHY:return "TROPHY";case REWARD_SILVER_SUNFLOWER:return "SILVER SUNFLOWER";case REWARD_MONEY_BAG:return "MONEY BAG";default:return seed>=0?full_seed_names[seed]:"SEED PACKET";}
}
static void update_reward(float dt){
    if(game.extra.pending==2){if(reward_delay<0){reward_delay=.2f;reward_row=2;reward_col=4;reward_x=center(4);reward_ground=plant_base(2,4);}reward_elapsed+=dt;if(reward_collect_time>=0)reward_collect_time+=dt;return;}
    if(game.status!=WON)return;
    if(reward_delay<0){
        reward_row=2;reward_col=4;reward_x=center(4);float youngest=10000;reward_delay=.20f;
        for(int i=0;i<MAX_ZOMBIES;i++)if(game.zombies[i].used&&game.zombies[i].death>0){
            Zombie *z=&game.zombies[i];reward_delay=fmaxf(reward_delay,z->death+.20f);
            if(z->age<youngest){youngest=z->age;reward_row=z->row;reward_x=fminf(420,fmaxf(82,z->x));reward_col=(int)((reward_x-GRID_X)/CELL_W);}
        }
        if(reward_col<0)reward_col=0;if(reward_col>=COLS)reward_col=COLS-1;
        reward_ground=baseline(reward_row)+game_roof_offset(&game,reward_x);
        if(game.level==50)reward_delay=fmaxf(reward_delay,(assets[A_BOSS_DEATH].frames-1)/18.0f+.60f);
        game.cursor_row=reward_row;game.cursor_col=reward_col;game.shovel=0;
    }
    reward_elapsed+=dt;if(reward_collect_time>=0)reward_collect_time+=dt;
}
static void draw_reward_glow(float x,float foot,float time,float scale){
    float pulse=.85f+.1f*sinf(time*3);render_image(&assets[A_AWARD_GLOW],x-32*scale,foot-45*scale,64*scale,64*scale,0,RGBA(255,255,255,(int)(180*pulse)));
    render_anim_rotated(&assets[A_AWARD_RAYS],x,foot+29*scale,time,.52f*scale,RGBA(255,255,255,150),time*.15f);
}
static void draw_reward_flight(int type,int seed,float elapsed,float startX,float startY){
    float t=fminf(1,elapsed/.75f),ease=t*t*(3-2*t),x=startX+(240-startX)*ease;
    int packet=type==REWARD_SEED;float target=packet?130:120,y=startY+(target-startY)*ease,h=packet?33+28*ease:24+14*ease,w=packet?24+20*ease:29+17*ease;
    render_rect(0,0,480,272,RGBA(0,0,0,(int)(130*ease)));draw_reward_glow(x,y,elapsed,1+ease);
    render_image(&assets[packet?A_SEED_CARDS:A_REWARD_TROPHY],x-w*.5f,y-h,w,h,packet?seed:0,WHITE);
}
static void draw_trophy_award(const char *subtitle){
    render_image(&assets[A_AWARD_BACKGROUND],0,0,480,272,0,WHITE);ui_center(123,31,234,"You got a trophy!",UI_GOLD,.85f);
    render_clip(202,66,72,63);draw_reward_glow(240,120,reward_collect_time,1);render_image(&assets[A_REWARD_TROPHY],217,82,46,38,0,WHITE);render_clip(0,0,480,272);ui_center(187,138,106,"Trophy",UI_GOLD,.67f);
    char title[100];ui_title_case(title,sizeof(title),subtitle);disc_text(187,163,106,title,UI_INK,.66f);ui_button(163,225,154,"X: Continue",1);ui_center(60,258,360,"O: Main menu",UI_INK,.6f);
}
static void draw_reward(void){
    render_rect(0,248,314,24,RGBA(30,42,29,255));render_text(5,251,"LEVEL CLEARED!",RGBA(255,238,154,255),1);
    if(reward_delay<0||reward_elapsed<reward_delay){render_text(5,263,"YOUR REWARD IS COMING...",WHITE,1);return;}
    float t=fminf(.7f,reward_elapsed-reward_delay),drop=t<.7f?-8-140*t+(106/.49f)*t*t:0;
    float x=reward_x,foot=reward_ground+drop;
    int type=game_reward_type(&game),seed=game_reward_seed(&game),selected=game.cursor_row==reward_row&&game.cursor_col==reward_col;
    if(type==REWARD_SEED&&seed>=0){
        draw_reward_glow(x,foot,reward_elapsed,1);render_image(&assets[A_SEED_CARDS],x-12,foot-33,24,33,seed,WHITE);
    }else{
        int asset=type==REWARD_SHOVEL?A_REWARD_SHOVEL:type==REWARD_ALMANAC?A_REWARD_ALMANAC:type==REWARD_KEYS?A_REWARD_KEYS:type==REWARD_TACO?A_REWARD_TACO:type==REWARD_WATERING_CAN?A_REWARD_WATERING_CAN:type==REWARD_NOTE?A_REWARD_NOTE:type==REWARD_SILVER_SUNFLOWER?A_REWARD_SILVER_SUNFLOWER:type==REWARD_MONEY_BAG?A_REWARD_MONEYBAG:A_REWARD_TROPHY;
        draw_reward_glow(x,foot,reward_elapsed,1);render_anim(&assets[asset],x,foot,0,type==REWARD_TROPHY?.6f:.8f,WHITE,0);
    }
    if(selected&&t>=.7f)render_image(&assets[A_AWARD_ARROW],x-8,foot-51-3*sinf(reward_elapsed*5),16,13,0,WHITE);
    render_text(5,263,t<.7f?"WAIT FOR YOUR REWARD TO LAND":selected?"X: COLLECT REWARD":"SELECT THE REWARD, THEN PRESS X",WHITE,1);
}

static void draw_plant_award(int seed){
    if(reward_collect_time>=0&&reward_collect_time<.85f){
        draw_reward_flight(REWARD_SEED,seed,reward_collect_time,reward_x,reward_ground);return;
    }
    render_image(&assets[A_PLANT_AWARDS],0,0,480,272,seed,WHITE);
    render_rect(187,231,106,26,RGBA(103,116,44,255));render_outline(187,231,106,26,RGBA(60,67,28,255));render_text(204,241,"X NEXT LEVEL",RGBA(255,241,154,255),1);
    render_text(8,261,"O MAIN MENU",RGBA(70,53,31,255),1);
}
