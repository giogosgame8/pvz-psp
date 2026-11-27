static const char *mini_advice[20]={
 "ZOMBIES HAVE PLANT HEADS.\nPEASHOOTER HEADS FIRE BACK AT YOUR PLANTS.",
 "ROLL WALL-NUTS FROM BEHIND THE RED LINE.\nRED NUTS EXPLODE AND REGULAR NUTS BOUNCE.",
 "TRIANGLE: SPIN THE REELS FOR 25 SUN.\nMATCH SYMBOLS FOR PLANTS, SUN OR DIAMONDS.\nCOLLECT 2000 SUN TO WIN.",
 "D-PAD LEFT/RIGHT: MOVE YOUR MOWER.\nANALOG LEFT/RIGHT: AIM. HOLD X TO FIRE.\nL/R: UPGRADE  TRIANGLE: BUY UPGRADE.",
 "X: SELECT A PLANT, THEN AN ADJACENT PLANT.\nMATCH THREE OR MORE TO REBUILD YOUR DEFENSE.\n75 MATCHES WIN. L/R TOOL, TRIANGLE BUY.",
 "THE ZOMBIES ARE INVISIBLE.\nUSE THE CONVEYOR'S PLANTS TO STOP THEM.",
 "FILL THE STAR-SHAPED OUTLINE WITH STARFRUIT.\nDEFEND YOUR PLANTS WHILE COMPLETING IT.",
 "X: DROP A BRAIN FOR 5 SUN. SQUARE: COLLECT.\nL/R: FISH OR TROPHY. TRIANGLE: BUY.\nFEED YOUR ZOMBIES; BUY THE 1000 SUN TROPHY.",
 "X: ROTATE A TWO-BY-TWO GROUP CLOCKWISE.\nEACH MOVE MUST MAKE A MATCH.\n75 MATCHES WIN. L/R TOOL, TRIANGLE BUY.",
 "TINY ZOMBIES ATTACK IN LARGE NUMBERS.\nPLANT THE SEEDS DELIVERED BY THE CONVEYOR.",
 "PORTALS MOVE ZOMBIES AND SHOTS IN PAIRS.\nONE MOVES AFTER 90S, THEN EVERY 60S.\nUSE THE PLANTS ON THE CONVEYOR.",
 "EACH PACKET PLANTS EVERY OPEN TILE IN A COLUMN.\nUSE FLOWER POTS TO OPEN MORE ROOF TILES.",
 "ICE COVERS THE RIGHT SIDE OF THE LAWN.\nPREPARE FOR ZOMBONIS AND BOBSLED TEAMS.",
 "EVERYTHING RUNS TWICE AS FAST.\nKEEP BUILDING SUN AND DEFEND THE POOL.",
 "X: WHACK A ZOMBIE ON THE SELECTED TILE.\nCONEHEADS NEED TWO HITS, BUCKETHEADS THREE.\nL/R TOOL, SQUARE USE TOOL.",
 "BUILD YOUR DEFENSE WITH 5000 SUN.\nPRESS START WHEN READY FOR EACH ONSLAUGHT.\nSURVIVE FIVE FLAGS; NO SUN-PRODUCING PLANTS.",
 "MORE PLANT-HEADED ZOMBIES ATTACK THE POOL.\nWATCH FOR GATLING, SQUASH AND JALAPENO HEADS.",
 "THE SECOND BOWLING CHALLENGE HAS GIANT NUTS.\nGIANT NUTS ROLL STRAIGHT THROUGH ZOMBIES.",
 "POGO ZOMBIES BOUNCE ACROSS THE ROOF.\nTALL-NUTS AND MAGNET-SHROOMS CAN HELP.",
 "DR. ZOMBOSS RETURNS WITH MORE HEALTH.\nUSE ICE-SHROOMS AND JALAPENOS AGAINST\nTHE FIRE AND ICE BALLS."
};
static void draw_mini_menu(void){
 static const int icons[20]={0,6,2,3,1,8,5,7,20,12,15,4,17,18,16,21,0,6,14,19};render_background(&assets[A_MINI_BACKGROUND]);ui_header("Mini-games");int page=mini_cursor/8,first=page*8;
 for(int i=first;i<20&&i<first+8;i++){
  int x=13+(i-first)%4*118,y=60+(i-first)/4*100;
  ui_card(x,y,100,94,icons[i],mini_names[i],i==mini_cursor,!!(player.mini_trophies&(1u<<i)),challenge_available(0,i));
  challenge_lock(x,y,100,94,0,i);
 }
 if(mini_resume_id>=0){
  char name[64],hint[104];ui_title_case(name,sizeof(name),mini_names[mini_resume_id]);
  snprintf(hint,sizeof(hint),"Triangle: Resume %s   X: New round",name);
  ui_context_hint(hint);
 }
}
static int draw_mini_custom_board(void){
    if(game.mode!=MODE_AQUARIUM)return 0;
    render_background(&assets[A_AQUARIUM_BACKGROUND]);
    for(int i=0;i<100;i++)if(game.mini.fish[i].used){AquariumFish *f=&game.mini.fish[i];uint32_t color=f->hunger<15?RGBA(145,220,110,255):WHITE;if(f->dx<0)render_anim_mirrored(&assets[A_AQUARIUM_SWIM],f->x,f->y,game.time+i*.1f,.85f,color,1);else render_anim(&assets[A_AQUARIUM_SWIM],f->x,f->y,game.time+i*.1f,.85f,color,1);}
    for(int i=0;i<3;i++)if(game.mini.brains[i].used)render_anim(&assets[A_BRAIN],game.mini.brains[i].x,game.mini.brains[i].y,0,1,WHITE,0);
    for(int i=0;i<MAX_SUNS;i++)if(game.suns[i].used)render_anim(&assets[A_SUN],game.suns[i].x,game.suns[i].y,game.time,.60f,WHITE,1);
    render_focus(GRID_X+game.cursor_col*CELL_W,GRID_Y+game.cursor_row*game_row_height(&game),CELL_W,game_row_height(&game),FOCUS_SAGE);return 1;
}
static void draw_star_targets(void){
    float height=game_row_height(&game);
    for(int row=0;row<game_rows(&game);row++)for(unsigned cells=game_mini_star_targets(row);cells;cells&=cells-1){
        int col=__builtin_ctz(cells),kind=game.plants[row][col].kind;
        if(kind==STAR)continue;
        float x=GRID_X+col*CELL_W,y=GRID_Y+row*height;
        int blocked=kind!=-1||game.craters[row][col]>0||game.morph_timer[row][col]>0||
                    game.graves[row][col]||game.vases[row][col]||game.ice_tiles[row][col]>0;
        render_rect(x+2,y+2,CELL_W-4,3,blocked?RGBA(242,161,98,160):RGBA(241,230,146,148));
        if(!blocked)render_anim(&assets[A_STAR],center(col),plant_base(row,col),0,.50f,RGBA(255,255,205,110),0);
    }
}
static void draw_mini_overlay(void){
    if(!game.challenge)return;
    if(game_mini_active(&game,MINI_PORTAL))for(int p=0;p<4;p++){int cell=game.mini.portals[p];render_anim(&assets[p<2?A_PORTAL_SQUARE:A_PORTAL_CIRCLE],GRID_X+(cell%10+.5f)*CELL_W,baseline(cell/10),game.time,.85f,WHITE,1);}
    if(game.mode==MODE_MATCH&&game.mini.selected_row>=0)render_focus(GRID_X+game.mini.selected_col*CELL_W,GRID_Y+game.mini.selected_row*game_row_height(&game),CELL_W,game_row_height(&game),FOCUS_ACTION);
    for(int i=0;i<96;i++)if(game.mini.enemy_used[i])render_anim(&assets[A_PROJECTILE_PEA],game.mini.enemy_x[i],game.mini.enemy_y[i],0,1,RGBA(205,255,160,255),0);
    if(game.mode==MODE_HEAVY){
        float x=game.mini.heavy_x;
        render_anim(&assets[A_HEAVY_TANK],x,248,game.time,.85f,WHITE,1);
        uint32_t tint=game.extra.heavy_fire_time>0?RGBA(255,188,115,255):game.extra.heavy_ice_time>0?RGBA(159,224,255,255):WHITE;
        render_anim_rotated(&assets[A_HEAVY_TURRET],x,222,0,1,tint,game.mini.heavy_power);
        if(game.extra.heavy_cattail)render_anim(&assets[A_HEAVY_CATTAIL],x+14,232,0,.53f,WHITE,0);
    }
}
static void draw_slot_symbol(int kind,int x,float y){
    int id=kind==SLOT_SUN?A_SUN:kind==SLOT_DIAMOND?A_COIN_DIAMOND:seed_icons[kind];
    render_anim(&assets[id],x,y,0,.36f,WHITE,0);
}
static void draw_slot_hud(void){
    static const int center_x[]={192,225,258};
    static const char *status[]={
        "Triangle: Spin (25 sun)", "Spinning", "Make room for the prize",
        "Plant a packet to spin", "Need 25 sun", "Goal reached", "",
    };
    render_image(&assets[A_SLOT_OVERLAY],150,1,150,42,0,WHITE);
    for(int j=0;j<3;j++){
        int x=center_x[j];
        render_clip(x-13,6,26,22);
        if(game_slot_reel_rolling(&game,j)){
            float phase=fmodf(game.time*(9+j*.7f)+j*.83f,SLOT_SYMBOLS);
            unsigned index=(unsigned)phase;
            float y=25+22*(phase-index);
            draw_slot_symbol(game_slot_symbol(index),x,y);
            draw_slot_symbol(game_slot_symbol(index+1),x,y-22);
        }else draw_slot_symbol(game.mini.reels[j],x,25);
    }
    render_clip(0,0,480,272);
    int state=game_slot_spin_status(&game);
    ui_center(307,14,168,status[state],state==SLOT_READY?UI_GOLD:WHITE,.55f);
    for(int i=0;i<game.packet_count;i++){
        float x=75+i*34;
        render_image(&assets[A_SEED_CARDS],x,46,25,33,game.packets[i],WHITE);
        if(i==game.selected)render_focus(x-1,45,27,35,FOCUS_SAGE);
    }
}
static int draw_mini_hud(float fps){
    if(!game.challenge)return 0;char line[128];int mode=game.mode;
    if(mode!=MODE_MATCH&&mode!=MODE_AQUARIUM&&mode!=MODE_HEAVY&&mode!=MODE_SLOT)return 0;
    if(mode!=MODE_HEAVY){
        render_rect(0,0,480,45,RGBA(86,52,25,240));
        snprintf(line,sizeof(line),"SUN %d",game.sun);render_text(5,8,line,RGBA(255,240,150,255),1);
    }
    if(mode==MODE_SLOT)draw_slot_hud();
    if(mode==MODE_MATCH){static const char *tools[]={"REPEATER 1000","FUME 500","TALL-NUT 250","SHUFFLE 100","FILL HOLES 200"};render_text(96,10,tools[game.selected%5],RGBA(255,245,170,255),1);render_text(96,27,"L/R TOOL  TRIANGLE BUY",WHITE,1);}
    if(mode==MODE_AQUARIUM)render_text(100,14,game.mini.upgrade?"TROPHY: 1000 SUN   TRIANGLE BUY":"SNORKEL ZOMBIE: 100 SUN   TRIANGLE BUY",WHITE,1);
    if(mode==MODE_HEAVY){
        const int upgrades=6;
        const Animation *cards=&assets[A_SEED_CARDS];
        draw_seed_bank(upgrades);
        for(int tool=0;tool<upgrades;tool++){
            int x=57+tool*42,cost=game_heavy_upgrade_cost(tool),seed=game_heavy_upgrade_seed(tool);
            render_image(cards,x,5,26,35,seed,WHITE);
            // Fill the normal price area with the blank strip next to it.
            render_part(cards,seed,5,34,2,8,x+5,32.67f,16,6.51f,WHITE);
            snprintf(line,sizeof(line),"%d",cost);
            float price_width=strlen(line)*6*.7f;
            render_text(x+13-price_width*.5f,33,line,UI_INK,.7f);
            if(!game_heavy_upgrade_open(&game,tool)||game.sun<cost)
                render_rect(x,5,26,35,RGBA(28,30,27,95));
            if(game.selected==tool)render_focus(x-2,3,30,39,FOCUS_SAGE);
        }
        if(game.extra.heavy_fire_time>0||game.extra.heavy_ice_time>0){
            int hot=game.extra.heavy_fire_time>0;
            snprintf(line,sizeof(line),"%s %.0f",hot?"Fire":"Ice",ceilf(hot?game.extra.heavy_fire_time:game.extra.heavy_ice_time));
            render_rect(317,12,63,22,RGBA(28,40,25,235));
            ui_center(320,18,57,line,UI_GOLD,.62f);
        }
    }
    char title[150];if(mode==MODE_HEAVY)snprintf(title,sizeof(title),"%s",mini_names[game.mini.id-1]);else snprintf(title,sizeof(title),"%s   %d/%d %s",mini_names[game.mini.id-1],mode==MODE_MATCH?game.mini.score:game.sun,mode==MODE_MATCH?75:mode==MODE_SLOT?2000:1000,mode==MODE_MATCH?"matches":"sun");
    ui_hud_footer(title,mode==MODE_AQUARIUM?"X: Feed   Square: Collect   L/R: Item":mode==MODE_MATCH?"X: Match   Square: Sun   Start: Pause":mode==MODE_HEAVY?"D-pad: Move   Stick: Aim   X: Fire   L/R: Item   Tri: Buy":"X: Plant   L/R: Seed   Square: Sun",mode==MODE_HEAVY);if(mode==MODE_HEAVY)draw_progress();
    if(debug_info){snprintf(line,sizeof(line),"%.0f FPS",fps);render_text(420,34,line,WHITE,1);}return 1;
}
