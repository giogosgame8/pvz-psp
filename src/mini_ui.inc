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
 "ONE CONVEYOR SEED PLANTS A WHOLE COLUMN.\nPOTS MUST SUPPORT PLANTS ON THE ROOF.",
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
 for(int i=first;i<20&&i<first+8;i++){int x=13+(i-first)%4*118,y=46+(i-first)/4*100;ui_card(x,y,100,94,icons[i],mini_names[i],i==mini_cursor,!!(player.mini_trophies&(1u<<i)),challenge_available(0,i));challenge_lock(x,y,100,94,0,i);}
 char hint[100];snprintf(hint,sizeof(hint),"Page %d/3   D-pad: Select   L/R: Page   X: Play   O: Back",page+1);ui_hint(hint);
}
static int draw_mini_custom_board(void){
    if(game.mode!=MODE_AQUARIUM)return 0;
    render_background(&assets[A_AQUARIUM_BACKGROUND]);
    for(int i=0;i<100;i++)if(game.mini.fish[i].used){AquariumFish *f=&game.mini.fish[i];uint32_t color=f->hunger<15?RGBA(145,220,110,255):WHITE;if(f->dx<0)render_anim_mirrored(&assets[A_AQUARIUM_SWIM],f->x,f->y,game.time+i*.1f,.85f,color,1);else render_anim(&assets[A_AQUARIUM_SWIM],f->x,f->y,game.time+i*.1f,.85f,color,1);}
    for(int i=0;i<3;i++)if(game.mini.brains[i].used)render_anim(&assets[A_BRAIN],game.mini.brains[i].x,game.mini.brains[i].y,0,1,WHITE,0);
    for(int i=0;i<MAX_SUNS;i++)if(game.suns[i].used)render_anim(&assets[A_SUN],game.suns[i].x,game.suns[i].y,game.time,.60f,WHITE,1);
    render_focus(GRID_X+game.cursor_col*CELL_W,GRID_Y+game.cursor_row*game_row_height(&game),CELL_W,game_row_height(&game),FOCUS_SAGE);return 1;
}
static void draw_mini_overlay(void){
    if(!game.challenge)return;
    if(game_mini_active(&game,MINI_STARS)){static const unsigned short star[5]={8,24,126,56,72};for(int r=0;r<5;r++)for(int c=0;c<9;c++)if((star[r]&(1<<c))&&game.plants[r][c].kind!=STAR)render_anim(&assets[A_STAR],center(c),plant_base(r,c),0,.68f,RGBA(255,255,180,85),1);}
    if(game_mini_active(&game,MINI_PORTAL))for(int p=0;p<4;p++){int cell=game.mini.portals[p];render_anim(&assets[p<2?A_PORTAL_SQUARE:A_PORTAL_CIRCLE],GRID_X+(cell%10+.5f)*CELL_W,baseline(cell/10),game.time,.85f,WHITE,1);}
    if(game.mode==MODE_MATCH&&game.mini.selected_row>=0)render_focus(GRID_X+game.mini.selected_col*CELL_W,GRID_Y+game.mini.selected_row*game_row_height(&game),CELL_W,game_row_height(&game),FOCUS_ACTION);
    for(int i=0;i<96;i++)if(game.mini.enemy_used[i])render_anim(&assets[A_PROJECTILE_PEA],game.mini.enemy_x[i],game.mini.enemy_y[i],0,1,RGBA(205,255,160,255),0);
    if(game.mode==MODE_HEAVY){render_anim(&assets[A_HEAVY_TANK],game.mini.heavy_x,248,game.time,.85f,WHITE,1);render_anim_rotated(&assets[game.extra.heavy_level==3?A_HEAVY_CATTAIL:A_HEAVY_TURRET],game.mini.heavy_x,222,0,1,WHITE,game.mini.heavy_power); }
}
static int draw_mini_hud(float fps){
    if(!game.challenge)return 0;char line[128];int mode=game.mode;
    if(mode!=MODE_MATCH&&mode!=MODE_AQUARIUM&&mode!=MODE_HEAVY&&mode!=MODE_SLOT)return 0;
    render_rect(0,0,480,45,RGBA(86,52,25,240));snprintf(line,sizeof(line),"SUN %d",game.sun);render_text(5,8,line,RGBA(255,240,150,255),1);
    if(mode==MODE_SLOT){static const int symbols[]={SUNFLOWER,PEA,SNOW,WALLNUT,50,51};render_image(&assets[A_SLOT_OVERLAY],150,1,150,42,0,WHITE);for(int j=0;j<3;j++){int k=game.mini.spinning?symbols[((int)(game.time*16)+j)%6]:game.mini.reels[j];int a=k==50?A_SUN:k==51?A_COIN_DIAMOND:seed_icons[k];render_anim(&assets[a],173+j*40,35,0,.4f,WHITE,1);}render_text(310,9,"TRIANGLE SPIN 25",WHITE,1);for(int i=0;i<game.packet_count;i++){float x=75+i*34;render_image(&assets[A_SEED_CARDS],x,46,25,33,game.packets[i],WHITE);if(i==game.selected)render_focus(x-1,45,27,35,FOCUS_SAGE);}}
    if(mode==MODE_MATCH){static const char *tools[]={"REPEATER 1000","FUME 500","TALL-NUT 250","SHUFFLE 100","FILL HOLES 200"};render_text(96,10,tools[game.selected%5],RGBA(255,245,170,255),1);render_text(96,27,"L/R TOOL  TRIANGLE BUY",WHITE,1);}
    if(mode==MODE_AQUARIUM)render_text(100,14,game.mini.upgrade?"TROPHY: 1000 SUN   TRIANGLE BUY":"SNORKEL ZOMBIE: 100 SUN   TRIANGLE BUY",WHITE,1);
    if(mode==MODE_HEAVY){static const char *names[]={"SNOW PEA 100","REPEATER 200","DOOM-SHROOM 200","THREEPEATER 400","TORCHWOOD 125","CATTAIL 200"};snprintf(line,sizeof(line),"%s%s",names[game.selected%6],game.extra.heavy_level==3?" UPGRADED":"");render_text(82,10,line,WHITE,1);render_text(82,28,"L/R SELECT   TRIANGLE BUY",WHITE,1);}
    char title[150];if(mode==MODE_HEAVY)snprintf(title,sizeof(title),"%s",mini_names[game.mini.id-1]);else snprintf(title,sizeof(title),"%s   %d/%d %s",mini_names[game.mini.id-1],mode==MODE_MATCH?game.mini.score:game.sun,mode==MODE_MATCH?75:mode==MODE_SLOT?2000:1000,mode==MODE_MATCH?"matches":"sun");
    ui_hud_footer(title,mode==MODE_AQUARIUM?"X: Feed   Square: Collect   L/R: Item":mode==MODE_MATCH?"X: Match   Square: Sun   Start: Pause":mode==MODE_HEAVY?"D-pad: Move   Analog: Aim   Hold X: Fire":"X: Plant   L/R: Seed   Square: Sun",mode==MODE_HEAVY);if(mode==MODE_HEAVY)draw_progress();
    if(debug_info){snprintf(line,sizeof(line),"%.0f FPS",fps);render_text(420,34,line,WHITE,1);}return 1;
}
