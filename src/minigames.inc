/* Standalone challenge rules. Content IDs follow the supplied console disc.
   Rendering and controller mappings live in mini_ui.inc. */
const char *mini_names[20]={"ZOMBOTANY","WALL-NUT BOWLING","SLOT MACHINE","HEAVY WEAPON","BEGHOULED","INVISI-GHOUL","SEEING STARS","ZOMBIQUARIUM","BEGHOULED TWIST","BIG TROUBLE LITTLE ZOMBIE","PORTAL COMBAT","COLUMN LIKE YOU SEE 'EM","BOBSLED BONANZA","ZOMBIES ON SPEED","WHACK A ZOMBIE","LAST STAND","ZOMBOTANY 2","BOWLING 2","POGO PARTY","DR. ZOMBOSS'S REVENGE"};
int game_mini_active(const Game *g,int id){return g->challenge&&g->mini.id==id+1;}
static int mini_allowed(const Game *g,int k){
    switch(g->mini.id-1){
    case MINI_BOBSLED:return k==Z_ZOMBONI||k==Z_BOBSLED;
    case MINI_POGO:return k==Z_POGO;
    case MINI_HEAVY:return k==Z_NORMAL||k==Z_POLE||k==Z_POGO||k==Z_FOOTBALL||k==Z_GARG;
    case MINI_SPEED:return k==Z_NORMAL||k==Z_CONE||k==Z_DOLPHIN||k==Z_POLE;
    case MINI_PORTAL:return k==Z_NORMAL||k==Z_BUCKET||k==Z_FOOTBALL||k==Z_BALLOON;
    case MINI_LITTLE:return k==Z_NORMAL||k==Z_CONE||k==Z_FOOTBALL||k==Z_SNORKEL;
    case MINI_INVIS:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET||k==Z_DOLPHIN||k==Z_ZOMBONI||k==Z_JACK;
    case MINI_COLUMN:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET||k==Z_FOOTBALL;
    case MINI_BEGHOULED:case MINI_TWIST:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET||k==Z_DOOR||k==Z_FOOTBALL||k==Z_PAPER;
    case MINI_LASTSTAND:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET||k==Z_DOOR||k==Z_FOOTBALL||k==Z_PAPER||k==Z_JACK||k==Z_POLE||k==Z_DOLPHIN||k==Z_LADDER;
    case MINI_BOWLING:case MINI_BOWLING2:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET||k==Z_POLE||k==Z_PAPER||(game_mini_active(g,MINI_BOWLING2)&&(k==Z_DOOR||k==Z_DANCER));
    case MINI_ZOMBOTANY:case MINI_ZOMBOTANY2:return k==Z_NORMAL||k==Z_BUCKET;
    default:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET;
    }
}
#include "heavy_weapon.inc"
static int mini_zombie(Game *g,int i,float dt){
    if(!g->challenge)return 0;
    Zombie *z=&g->zombies[i];
    int head=g->mini.zombie_heads[i];
    if(!head||z->death>0||g->frozen[i]>0)return 0;
    if((head==2||head==6)&&z->armor<=0)g->mini.zombie_heads[i]=0;
    int can_attack=!g->hypnotized[i]&&game_zombie_has_head(g,z);
    float *jump=&g->extra.zombotany_jump[i];
    if(head==5&&*jump>0){
        float before=*jump;
        *jump=fmaxf(0,before-dt);
        int col=g->extra.squash_col[i];
        if(can_attack&&(unsigned)col<COLS&&before>.2f&&*jump<=.2f){
            destroy_square(g,z->row,col,1);
            sound(g,SFX_CHOMP);
        }
        if(can_attack&&!*jump)kill_zombie(g,z);
        return 1;
    }
    if(!can_attack)return 0;
    if(head==1||head==4){g->mini.zombie_attack[i]-=dt;if(g->mini.zombie_attack[i]<=0){
       if(head==4){if(!g->extra.zombotany_burst[i])g->extra.zombotany_burst[i]=4;g->extra.zombotany_burst[i]--;g->mini.zombie_attack[i]=g->extra.zombotany_burst[i]?.17f:.99f;}else g->mini.zombie_attack[i]=1.5f;
       for(int j=0;j<96;j++)if(!g->mini.enemy_used[j]){g->mini.enemy_used[j]=1;g->mini.enemy_x[j]=z->x-18;g->mini.enemy_y[j]=game_ground_y(g,z->row)-18;break;}}}
    if(head==3){g->mini.zombie_attack[i]-=dt;if(g->mini.zombie_attack[i]<=-10){for(int c=0;c<COLS;c++){destroy_square(g,z->row,c,1);}kill_zombie(g,z);g->flash=.25f;sound(g,SFX_EXPLODE);return 1;}}
    if(head==5)for(int c=0;c<COLS;c++)
        if(g->plants[z->row][c].kind>=0&&fabsf(z->x-plant_x(c))<30){
            g->extra.squash_col[i]=c;*jump=.95f;z->eating=0;return 1;
        }
    return 0;
}
enum { MATCH_ROWS=5, MATCH_COLS=8 };
typedef struct { int row,col; } MatchCell;

static int mini_match_kind(int k){
    if(k==REPEATER)k=PEA;if(k==FUME)k=PUFF;if(k==TALLNUT)k=WALLNUT;return k;
}
static int mini_match_seed(Game *g){
    static const int types[]={PUFF,STAR,MAGNET,SNOW,WALLNUT,PEA};
    int k=types[random_next(g)%6];if(k==PEA&&(g->mini.upgrade&1))k=REPEATER;if(k==PUFF&&(g->mini.upgrade&2))k=FUME;if(k==WALLNUT&&(g->mini.upgrade&4))k=TALLNUT;return k;
}
static void mini_move_plant(Game *g,MatchCell to,MatchCell from){
    int r=to.row,c=to.col,sr=from.row,sc=from.col;
    g->plants[r][c]=g->plants[sr][sc];
    g->asleep[r][c]=g->asleep[sr][sc];
    g->fidelity.plant_id[0][r][c]=g->fidelity.plant_id[0][sr][sc];
    g->pea_shoot_time[r][c]=g->pea_shoot_time[sr][sc];
    g->burst[r][c]=g->burst[sr][sc];
    g->burst_left[r][c]=g->burst_left[sr][sc];
}
static void mini_rotate_plants(Game *g,const MatchCell *cells,int count){
    MatchCell last=cells[count-1];
    Plant plant=g->plants[last.row][last.col];
    int asleep=g->asleep[last.row][last.col],phase=g->burst_left[last.row][last.col];
    uint32_t id=g->fidelity.plant_id[0][last.row][last.col];
    float shoot=g->pea_shoot_time[last.row][last.col],burst=g->burst[last.row][last.col];
    for(int i=count-1;i>0;i--)mini_move_plant(g,cells[i],cells[i-1]);
    MatchCell first=cells[0];
    g->plants[first.row][first.col]=plant;
    g->asleep[first.row][first.col]=asleep;
    g->fidelity.plant_id[0][first.row][first.col]=id;
    g->pea_shoot_time[first.row][first.col]=shoot;
    g->burst[first.row][first.col]=burst;
    g->burst_left[first.row][first.col]=phase;
}
static void mini_fill_matches(Game *g){
    for(int c=0;c<MATCH_COLS;c++)for(int r=MATCH_ROWS-1;r>=0;r--)if(g->plants[r][c].kind<0&&g->craters[r][c]<=0){
        int source=r-1;while(source>=0&&g->plants[source][c].kind<0)source--;
        if(source>=0){mini_move_plant(g,(MatchCell){r,c},(MatchCell){source,c});g->plants[source][c].kind=-1;g->mini.match_fall[r][c]=-(r-source)*game_row_height(g);}
        else {put_plant(g,r,c,mini_match_seed(g));g->mini.match_fall[r][c]=-game_row_height(g)*(r+1);}
    }
    g->mini.timer=.30f;
}
static int mini_match_scan(Game *g){
    unsigned char remove[5][8]={{0}};int groups=0;
    for(int r=0;r<5;r++)for(int c=0;c<8;c++){
        int k=mini_match_kind(g->plants[r][c].kind);if(k<0)continue;
        if(c==0||mini_match_kind(g->plants[r][c-1].kind)!=k){int end=c+1;while(end<8&&mini_match_kind(g->plants[r][end].kind)==k)end++;if(end-c>=3){groups++;for(int j=c;j<end;j++)remove[r][j]=1;for(int j=0;j<end-c-2&&j<5;j++)plant_sun(g,plant_x(c)+j*6,game_ground_y(g,r)-15);}}
        if(r==0||mini_match_kind(g->plants[r-1][c].kind)!=k){int end=r+1;while(end<5&&mini_match_kind(g->plants[end][c].kind)==k)end++;if(end-r>=3){groups++;for(int j=r;j<end;j++)remove[j][c]=1;plant_sun(g,plant_x(c),game_ground_y(g,r)-15);}}
    }
    if(groups){g->mini.score+=groups;for(int r=0;r<5;r++)for(int c=0;c<8;c++)if(remove[r][c])g->plants[r][c].kind=-1;mini_fill_matches(g);sound_as(g,SFX_PLANT,FOLEY_CHIME);}
    return groups;
}
static void mini_shuffle(Game *g){
    for(int r=0;r<MATCH_ROWS;r++)for(int c=0;c<MATCH_COLS;c++)if(g->craters[r][c]<=0){
        int k,kind;
        int left=c>=2?mini_match_kind(g->plants[r][c-1].kind):-1;
        int above=r>=2?mini_match_kind(g->plants[r-1][c].kind):-1;
        if(c>=2&&left!=mini_match_kind(g->plants[r][c-2].kind))left=-1;
        if(r>=2&&above!=mini_match_kind(g->plants[r-2][c].kind))above=-1;
        do{k=mini_match_seed(g);kind=mini_match_kind(k);}while(kind==left||kind==above);
        put_plant(g,r,c,k);g->mini.match_fall[r][c]=-20;
    }
    g->mini.selected_row=g->mini.selected_col=-1;
    g->mini.phase=0;
    g->mini.secondary=.5f;
}
static void mini_match_board(const Game *g,int board[MATCH_ROWS][MATCH_COLS]){
    for(int r=0;r<MATCH_ROWS;r++)for(int c=0;c<MATCH_COLS;c++)board[r][c]=mini_match_kind(g->plants[r][c].kind);
}
static int mini_match_at(const int board[MATCH_ROWS][MATCH_COLS],MatchCell cell){
    int r=cell.row,c=cell.col,k=board[r][c],count=1;
    for(int i=c-1;i>=0&&board[r][i]==k;i--)count++;
    for(int i=c+1;i<MATCH_COLS&&board[r][i]==k;i++)count++;
    if(count>=3)return 1;
    count=1;
    for(int i=r-1;i>=0&&board[i][c]==k;i--)count++;
    for(int i=r+1;i<MATCH_ROWS&&board[i][c]==k;i++)count++;
    return count>=3;
}
static int mini_move_matches(int board[MATCH_ROWS][MATCH_COLS],const MatchCell *cells,int count){
    int original[4],found=0;
    for(int i=0;i<count;i++){
        original[i]=board[cells[i].row][cells[i].col];
        if(original[i]<0)return 0;
    }
    int previous=count-1;
    for(int i=0;i<count;i++){board[cells[i].row][cells[i].col]=original[previous];previous=i;}
    for(int i=0;i<count&&!found;i++)found=mini_match_at(board,cells[i]);
    for(int i=0;i<count;i++)board[cells[i].row][cells[i].col]=original[i];
    return found;
}
static int mini_has_move(const Game *g){
    int board[MATCH_ROWS][MATCH_COLS],twist=game_mini_active(g,MINI_TWIST);
    mini_match_board(g,board);
    for(int r=0;r<MATCH_ROWS;r++)for(int c=0;c<MATCH_COLS;c++){
        MatchCell cells[4]={{r,c},{r,c+1},{r+1,c+1},{r+1,c}};
        if(twist){
            if(r+1<MATCH_ROWS&&c+1<MATCH_COLS&&mini_move_matches(board,cells,4))return 1;
        }else{
            if(c+1<MATCH_COLS&&mini_move_matches(board,cells,2))return 1;
            cells[1]=cells[3];
            if(r+1<MATCH_ROWS&&mini_move_matches(board,cells,2))return 1;
        }
    }
    return 0;
}
static int mini_match_action(Game *g){
    int r=g->cursor_row,c=g->cursor_col;
    if(r<0||r>=MATCH_ROWS||c<0||c>=MATCH_COLS||g->mini.timer>0)return -1;
    MatchCell cells[4]={{r,c},{r,c+1},{r+1,c+1},{r+1,c}};
    int count=4;
    if(game_mini_active(g,MINI_TWIST)){
        if(r+1>=MATCH_ROWS||c+1>=MATCH_COLS)return -1;
    }else{
        int sr=g->mini.selected_row,sc=g->mini.selected_col;
        g->mini.selected_row=g->mini.selected_col=-1;
        if(g->plants[r][c].kind<0)return -1;
        if(sr<0||sr>=MATCH_ROWS||sc<0||sc>=MATCH_COLS||g->plants[sr][sc].kind<0){
            g->mini.selected_row=r;g->mini.selected_col=c;
            return 1;
        }
        if(abs(r-sr)+abs(c-sc)!=1)return -1;
        cells[1]=(MatchCell){sr,sc};
        count=2;
    }
    int board[MATCH_ROWS][MATCH_COLS];
    mini_match_board(g,board);
    if(!mini_move_matches(board,cells,count))return -1;
    mini_rotate_plants(g,cells,count);
    mini_match_scan(g);
    g->mini.phase=0;
    g->mini.secondary=.5f;
    return 1;
}
static void mini_match_step(Game *g,float dt){
    float before=g->mini.timer;
    g->mini.timer=fmaxf(0,before-dt);
    float remaining=before>0?g->mini.timer/before:0;
    for(int r=0;r<MATCH_ROWS;r++)for(int c=0;c<MATCH_COLS;c++){
        float *fall=&g->mini.match_fall[r][c];
        *fall=before>0?*fall*remaining:fminf(0,*fall+180*dt);
        if(g->plants[r][c].kind<0&&g->craters[r][c]<=0)g->craters[r][c]=100000;
    }
    int sr=g->mini.selected_row,sc=g->mini.selected_col;
    if(sr>=0&&(sr>=MATCH_ROWS||sc<0||sc>=MATCH_COLS||g->plants[sr][sc].kind<0))
        g->mini.selected_row=g->mini.selected_col=-1;
    if(g->mini.timer>0)return;
    if((before>0||g->mini.secondary<=dt)&&mini_match_scan(g)){
        g->mini.phase=0;g->mini.secondary=.5f;
        return;
    }
    g->mini.secondary-=dt;
    if(g->mini.secondary>0)return;
    if(mini_has_move(g)){g->mini.phase=0;g->mini.secondary=.5f;}
    else if(g->mini.phase==1)mini_shuffle(g);
    else {g->mini.phase=1;g->mini.secondary=5;}
}
static int mini_fish(Game *g){for(int i=0;i<100;i++)if(!g->mini.fish[i].used){AquariumFish *f=&g->mini.fish[i];*f=(AquariumFish){.used=1,.x=70+random_unit(g)*330,.y=90+random_unit(g)*130,.hunger=27,.produce=10+random_unit(g)*5,.dx=15,.dy=5};return 1;}return 0;}
enum { SLOT_COST=25, SLOT_WIN_SUN=2000, SLOT_REELS=3 };
static const float slot_spin_duration=3.0f,slot_reel_gap=.30f;
static const int slot_symbols[SLOT_SYMBOLS]={SUNFLOWER,PEA,SNOW,WALLNUT,SLOT_SUN,SLOT_DIAMOND};

int game_slot_spin_status(const Game *g){
    if(g->status!=PLAYING)return g->status==WON?SLOT_GOAL:SLOT_INACTIVE;
    if(g->mini.spinning!=SLOT_READY)return g->mini.spinning;
    if(g->sun>=SLOT_WIN_SUN)return SLOT_GOAL;
    if(g->packet_count>MAX_PACKETS-SLOT_REELS)return SLOT_TRAY_BLOCKED;
    return g->sun<SLOT_COST?SLOT_NEEDS_SUN:SLOT_READY;
}
int game_slot_symbol(unsigned index){return slot_symbols[index%SLOT_SYMBOLS];}
int game_slot_reel_rolling(const Game *g,int reel){
    return g->mini.spinning==SLOT_ROLLING&&(!g->mini.stage||g->mini.spin_time>(SLOT_REELS-1-reel)*slot_reel_gap);
}
static void mini_replan(Game *g){memset(g->wave_count,0,sizeof(g->wave_count));memset(g->wave_types,0,sizeof(g->wave_types));plan_waves(g);}
void game_start_mini(Game *g,int id,uint32_t seed){
    static const int levels[20]={9,5,9,9,19,29,9,29,19,29,19,49,29,29,15,29,29,9,49,50};
    static const int waves[20]={20,20,40,20,40,20,40,1,40,30,20,30,40,40,12,10,30,30,30,40};
    if(id<0||id>=MINI_COUNT)id=0;game_start_level(g,levels[id],seed);g->challenge=id==MINI_BOWLING?1:id==MINI_WHACK?2:id+3;g->mini.id=id+1;
    g->mode=MODE_LAWN;g->num_waves=waves[id];g->row_mask=game_rows(g)==6?63:31;g->sun=50;g->spawn_timer=18;g->shovel_tutorial=0;g->packet_count=0;g->mini.selected_row=g->mini.selected_col=-1;g->level_flags=LEVEL_TUTORIAL_COMPLETE;
    memset(g->graves,0,sizeof(g->graves));memset(g->support,0,sizeof(g->support));memset(g->support_hp,0,sizeof(g->support_hp));
    for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)g->plants[r][c].kind=-1;
    g->unlocked_mask=(UINT64_C(1)<<40)-1;g->seed_mask=(UINT64_C(1)<<PEA)|(UINT64_C(1)<<SUNFLOWER)|(UINT64_C(1)<<WALLNUT)|(UINT64_C(1)<<CHERRY)|(UINT64_C(1)<<MINE)|(UINT64_C(1)<<SNOW);g->selected=PEA;
    if(id==MINI_STARS){g->seed_mask&=~(UINT64_C(1)<<MINE);g->seed_mask|=UINT64_C(1)<<STAR;}
    if(id==MINI_BOWLING||id==MINI_BOWLING2){g->mode=MODE_BOWLING;g->sun=0;g->packets[g->packet_count++]=WALLNUT;g->spawn_timer=2;}
    if(id==MINI_SLOT){g->mode=MODE_SLOT;g->seed_mask=0;for(int j=0;j<SLOT_REELS;j++)g->mini.reels[j]=slot_symbols[j];}
    if(id==MINI_HEAVY){g->mode=MODE_HEAVY;g->sun=0;g->mini.heavy_x=100;g->mini.heavy_y=150;g->spawn_timer=3;}
    if(id==MINI_BEGHOULED||id==MINI_TWIST){g->mode=MODE_MATCH;g->sun=0;mini_shuffle(g);g->mini.timer=g->mini.secondary=0;}
    if(id==MINI_AQUARIUM){g->mode=MODE_AQUARIUM;g->sun=50;mini_fish(g);mini_fish(g);}
    if(id==MINI_INVIS||id==MINI_LITTLE||id==MINI_PORTAL||id==MINI_COLUMN){g->mode=MODE_CONVEYOR;g->sun=0;g->conveyor_timer=0;}
    if(id==MINI_PORTAL){g->mini.portals[0]=2;g->mini.portals[1]=19;g->mini.portals[2]=39;g->mini.portals[3]=42;g->mini.portal_timer=90;}
    if(id==MINI_COLUMN||id==MINI_POGO)for(int r=0;r<5;r++)for(int c=0;c<3;c++){g->support[r][c]=POT+1;g->support_hp[r][c]=300;}
    if(id==MINI_BOBSLED)for(int r=0;r<6;r++)if(!game_water_row(g,r))for(int c=4;c<9;c++)g->ice_tiles[r][c]=100000;
    if(id==MINI_WHACK){g->mode=MODE_WHACK;g->sun=0;g->seed_mask=(UINT64_C(1)<<GRAVEBUSTER)|(UINT64_C(1)<<ICESHROOM)|(UINT64_C(1)<<CHERRY);g->selected=GRAVEBUSTER;for(int i=0;i<9;i++)g->graves[random_next(g)%5][4+random_next(g)%5]=1;}
    if(id==MINI_LASTSTAND){g->sun=5000;g->mini.phase=0;g->seed_mask&=~(UINT64_C(1)<<SUNFLOWER);g->unlocked_mask&=~((UINT64_C(1)<<SUNFLOWER)|(UINT64_C(1)<<SUNSHROOM)|(UINT64_C(1)<<TWIN));}
    if(id==MINI_BOSS){g->mode=MODE_BOSS;g->sun=0;g->boss_hp=60000;g->boss_timer=8;for(int r=0;r<5;r++)for(int c=0;c<3;c++){g->support[r][c]=POT+1;g->support_hp[r][c]=300;}}
    mini_replan(g);
}
/* action: 0 tile, 1 special, 2 previous tool, 3 next tool, 4 onslaught. */
int game_mini_action(Game *g,int action){
    if(!g->challenge||g->status!=PLAYING)return 0;
    if(game_mini_active(g,MINI_LASTSTAND)&&action==4&&g->mini.phase==0){g->mini.phase=1;g->spawn_timer=2;return 1;}
    if(g->mode==MODE_SLOT&&action==1){
        if(game_slot_spin_status(g)!=SLOT_READY)return -1;
        g->sun-=SLOT_COST;g->mini.spinning=SLOT_ROLLING;g->mini.spin_time=slot_spin_duration;
        g->mini.stage=0;g->mini.score=0;
        cue(GAME_SAMPLE(FX_SLOT_MACHINE));return 1;
    }
    if(g->mode==MODE_HEAVY){if(action==2||action==3){g->selected=(g->selected+(action==2?HEAVY_UPGRADES-1:1))%HEAVY_UPGRADES;return 1;}if(action==1)return heavy_buy(g);}
    if(g->mode==MODE_AQUARIUM){
        if(action==2||action==3){g->mini.upgrade^=1;return 1;}
        if(action==1){
            if(g->mini.upgrade){
                if(g->sun<1000)return -1;
                g->sun-=1000;g->status=WON;sound(g,SFX_WIN);
            }else{
                if(g->sun<100||!mini_fish(g))return -1;
                g->sun-=100;
            }
            return 1;
        }
        if(action==0){if(g->sun<5)return -1;for(int i=0;i<3;i++)if(!g->mini.brains[i].used){g->mini.brains[i]=(AquariumBrain){1,plant_x(g->cursor_col),game_ground_y(g,g->cursor_row)-15,0};g->sun-=5;return 1;}return -1;}
    }
    if(g->mode==MODE_MATCH){
        if(action==2||action==3){g->selected=(g->selected+(action==2?4:1))%5;return 1;}
        if(action==1){static const int costs[]={1000,500,250,100,200};int tool=g->selected%5;if(g->sun<costs[tool]||g->mini.timer>0)return -1;
            if(tool<3){int bit=1<<tool;if(g->mini.upgrade&bit)return -1;g->mini.upgrade|=bit;for(int r=0;r<5;r++)for(int c=0;c<8;c++){Plant *p=&g->plants[r][c];if(p->kind==(int[]){PEA,PUFF,WALLNUT}[tool])put_plant(g,r,c,(int[]){REPEATER,FUME,TALLNUT}[tool]);}}
            if(tool==3)mini_shuffle(g);
            if(tool==4){
                int holes=0;
                for(int r=0;r<MATCH_ROWS;r++)for(int c=0;c<MATCH_COLS;c++)holes+=g->plants[r][c].kind<0;
                if(!holes)return -1;
                memset(g->craters,0,sizeof(g->craters));
                g->mini.selected_row=g->mini.selected_col=-1;
                mini_fill_matches(g);
            }
            g->sun-=costs[tool];return 1;}
        if(action==0)return mini_match_action(g);
    }
    return 0;
}
static void mini_spawned(Game *g,int i){
    if(i<0||!g->challenge)return;Zombie *z=&g->zombies[i];g->extra.bobsled_team[i]=0;g->extra.brain_eating[i]=0;g->extra.zombotany_burst[i]=0;g->extra.zombotany_jump[i]=0;g->extra.last_portal_zombie[i]=0;g->mini.zombie_heads[i]=0;g->mini.zombie_attack[i]=1.5f;g->mini.zombie_portal[i]=0;
    if(game_mini_active(g,MINI_ZOMBOTANY)||game_mini_active(g,MINI_ZOMBOTANY2)){
        int head=game_mini_active(g,MINI_ZOMBOTANY2)?1+random_next(g)%6:z->kind==Z_BUCKET?2:1;g->mini.zombie_heads[i]=head;z->kind=Z_NORMAL;z->armor=head==2?1100:head==6?2200:0;
    }
    if(game_mini_active(g,MINI_LITTLE)){z->hp*=.25f;z->armor*=.25f;}
    if(g->mode==MODE_HEAVY)g->mini.hostile_y[i]=58+random_unit(g)*175;
}
static void mini_slot_draw(Game *g){
    int peas=0;for(int r=0;r<5;r++)for(int c=0;c<9;c++)peas+=g->plants[r][c].kind==PEA;
    for(int reel=0;reel<SLOT_REELS;reel++){
        int weights[SLOT_SYMBOLS],total=0;
        for(int i=0;i<SLOT_SYMBOLS;i++){
            int symbol=slot_symbols[i];
            int weight=symbol==PEA?200-20*(peas>5?5:peas):symbol==SLOT_DIAMOND?30:100;
            if(reel==2&&symbol!=SLOT_DIAMOND&&(symbol==g->mini.reels[0]||symbol==g->mini.reels[1]))weight+=weight/2;
            weights[i]=weight;total+=weight;
        }
        int pick=random_next(g)%total,k=0;
        while(k<SLOT_SYMBOLS-1&&pick>=weights[k])pick-=weights[k++];
        g->mini.reels[reel]=slot_symbols[k];
    }
    g->mini.stage=1;
}
static int mini_slot_prize(const Game *g,int *kind){
    int a=g->mini.reels[0],b=g->mini.reels[1],c=g->mini.reels[2];
    *kind=a==b||a==c?a:b==c?b:-1;
    if(*kind<0)return 0;
    int triple=a==b&&b==c;
    if(*kind==SLOT_SUN)return triple?20:4;
    if(*kind==SLOT_DIAMOND)return triple?5:1;
    return triple?3:1;
}
static void mini_slot_step(Game *g,float dt){
    if(g->mini.spinning==SLOT_ROLLING){
        g->mini.spin_time=fmaxf(0,g->mini.spin_time-dt);
        if(!g->mini.stage&&g->mini.spin_time<=2*slot_reel_gap)mini_slot_draw(g);
        if(g->mini.spin_time>0)return;
        int kind;
        g->mini.score=mini_slot_prize(g,&kind);
        g->mini.stage=0;
        g->mini.spinning=g->mini.score?SLOT_PAYING:SLOT_READY;
        cue(g->mini.score?FOLEY_CHIME:GAME_SAMPLE(FX_BUZZER));
    }
    if(g->mini.spinning==SLOT_PAYING){
        int kind,total=mini_slot_prize(g,&kind);
        while(g->mini.score>0){
            int i=total-g->mini.score;
            if(kind==SLOT_SUN){if(plant_sun(g,200+i*4,55)<0)return;}
            else if(kind==SLOT_DIAMOND){if(!game_add_coin(g,200+i*8,60,100))return;}
            else{
                if(g->packet_count>=MAX_PACKETS)return;
                g->extra.packet_age[g->packet_count]=0;
                g->packets[g->packet_count++]=kind;
            }
            g->mini.score--;
        }
        g->mini.spinning=SLOT_READY;
    }
}
static void mini_repair_slot(Game *g){
    int valid=1;
    for(int j=0;j<SLOT_REELS;j++){
        int k=0;while(k<SLOT_SYMBOLS&&g->mini.reels[j]!=slot_symbols[k])k++;
        if(k==SLOT_SYMBOLS){g->mini.reels[j]=slot_symbols[j];valid=0;}
    }
    if(g->mini.spinning==SLOT_ROLLING){
        if(!isfinite(g->mini.spin_time))g->mini.spin_time=slot_spin_duration;
        g->mini.spin_time=fminf(slot_spin_duration,fmaxf(0,g->mini.spin_time));
        if(!valid||g->mini.stage!=1)g->mini.stage=0;
    }else if(g->mini.spinning==SLOT_PAYING){
        int kind,max=mini_slot_prize(g,&kind);
        if(!valid||g->mini.score<1||g->mini.score>max){g->mini.score=0;g->mini.spinning=SLOT_READY;}
    }else g->mini.spinning=SLOT_READY;
}
static void mini_portals(Game *g,float dt){
 g->mini.portal_timer-=dt;if(g->mini.portal_timer<=0){g->mini.portal_timer=60;g->mini.portal_generation++;cue(FOLEY_PORTAL);int at=random_next(g)%4,other=g->mini.portals[at^1],candidates[50],count=0;for(int row=0;row<5;row++)for(int col=0;col<10;col++){int cell=row*10+col,free=col!=other%10&&row!=other/10;for(int p=0;p<4;p++)if(g->mini.portals[p]==cell)free=0;if(free)candidates[count++]=cell;}if(count)g->mini.portals[at]=candidates[random_next(g)%count];}
 for(int i=0;i<MAX_ZOMBIES;i++){
    Zombie *z=&g->zombies[i];
    if(!z->used||z->death>0)continue;
    int last=g->extra.last_portal_zombie[i];
    if(last&&((last-1)/10!=z->row||fabsf(z->x-plant_x((last-1)%10))>26))
        g->extra.last_portal_zombie[i]=0;
    for(int p=0;p<4;p++){
        int cell=g->mini.portals[p];
        if(z->row!=cell/10||cell+1==g->extra.last_portal_zombie[i]||
           !(fabsf(z->x-plant_x(cell%10))<21))continue;
        int destination=g->mini.portals[p^1];
        float delta=plant_x(destination%10)-12-z->x;
        z->row=destination/10;z->x+=delta;
        if(z->vault>0||g->polish.action[i]==ACTION_IMP_FLIGHT){
            z->vault_from+=delta;z->vault_to+=delta;
        }else if(z->eating){
            z->eating=0;z->age=0;g->polish.pose[i]=0;
        }
        g->extra.last_portal_zombie[i]=destination+1;
        g->mini.zombie_portal[i]=1;
        cue(FOLEY_PORTAL);
        break;
    }
 }
 for(int i=0;i<ROWS;i++)if(g->mower_used[i]&&g->mower_x[i]<520){int row=g->extra.mower_row[i]?g->extra.mower_row[i]-1:i,last=g->extra.last_portal_mower[i];if(last&&fabsf(g->mower_x[i]-plant_x((last-1)%10))>26)g->extra.last_portal_mower[i]=0;
  for(int p=0;p<4;p++){int cell=g->mini.portals[p];if(row==cell/10&&cell+1!=g->extra.last_portal_mower[i]&&fabsf(g->mower_x[i]-plant_x(cell%10))<17){int to=g->mini.portals[p^1];g->mower_x[i]=plant_x(to%10)+12;g->extra.mower_row[i]=to/10+1;g->extra.last_portal_mower[i]=to+1;cue(FOLEY_PORTAL);break;}}}
}
static int mini_portal_checkpoint_valid(const Game *g){
    if(!game_mini_active(g,MINI_PORTAL))return 1;
    if(!isfinite(g->mini.portal_timer))return 0;
    uint64_t seen=0;
    for(int p=0;p<4;p++){
        unsigned cell=g->mini.portals[p];
        if(cell>=50||!game_row_active(g,cell/10)||(seen&(UINT64_C(1)<<cell)))return 0;
        seen|=UINT64_C(1)<<cell;
    }
    return 1;
}
static void mini_repair_aquarium(Game *g){
    for(int i=0;i<100;i++)if(g->mini.fish[i].used){
        AquariumFish *f=&g->mini.fish[i];
        if(!isfinite(f->x)||!isfinite(f->y)){f->x=245;f->y=152.5f;}
        if(!isfinite(f->dx)||!isfinite(f->dy)){f->dx=15;f->dy=5;}
    }
}
static void mini_aquarium_step(Game *g,float dt){
    for(int i=0;i<3;i++)if(g->mini.brains[i].used){
        AquariumBrain *b=&g->mini.brains[i];
        b->age+=dt;b->y+=5.7f*dt;
        if(b->y>240)b->used=0;
    }
    int alive=0;
    for(int i=0;i<100;i++)if(g->mini.fish[i].used){
        AquariumFish *f=&g->mini.fish[i];
        f->hunger-=dt;f->produce-=dt;
        if(f->hunger<=0){cue(GAME_SAMPLE(FX_ZOMBAQUARIUM_DIE));f->used=0;continue;}
        alive++;
        AquariumBrain *food=NULL;
        float nearest=INFINITY,dx=0,dy=0;
        if(f->hunger<=15)for(int j=0;j<3;j++)if(g->mini.brains[j].used){
            AquariumBrain *b=&g->mini.brains[j];
            float bx=b->x-f->x,by=b->y-f->y,distance_squared=bx*bx+by*by;
            if(distance_squared<nearest){food=b;nearest=distance_squared;dx=bx;dy=by;}
        }
        if(food){
            if(nearest<19*19&&food->age>=.15f){food->used=0;f->hunger=fminf(27,f->hunger+20);}
            else if(nearest>0){
                float distance=sqrtf(nearest),speed=fminf(19,distance/dt);
                f->dx=dx/distance*speed;f->dy=dy/distance*speed;
            }else f->dx=f->dy=0;
        }else if(random_unit(g)<dt*.3f){
            float angle=random_unit(g)*6.283185f;
            f->dx=cosf(angle)*15;f->dy=sinf(angle)*12;
        }
        f->x+=f->dx*dt;f->y+=f->dy*dt;
        if(f->x<65||f->x>425){f->dx=-f->dx;f->x=fminf(425,fmaxf(65,f->x));}
        if(f->y<80||f->y>225){f->dy=-f->dy;f->y=fminf(225,fmaxf(80,f->y));}
        if(f->produce<=0){f->produce=10+random_unit(g)*5;plant_sun(g,f->x,f->y);}
    }
    step_uncollected_suns(g,dt);
    if(!alive)g->status=LOST;
}
static int mini_pre_step(Game *g,float dt){
    if(!g->challenge)return 0;
    if(game_mini_active(g,MINI_LASTSTAND)&&g->mini.phase==0)return 1;
    if(g->mode==MODE_HEAVY){g->extra.heavy_fire_time=fmaxf(0,g->extra.heavy_fire_time-dt);g->extra.heavy_ice_time=fmaxf(0,g->extra.heavy_ice_time-dt);
 for(int i=0;i<MAX_SUNS;i++)if(g->suns[i].used==1&&fabsf(g->suns[i].x-g->mini.heavy_x)<22&&g->suns[i].y>215){Sun *v=&g->suns[i];v->used=2;v->age=0;v->vx=v->x;v->vy=v->y;v->flight=.35f;}
 for(int i=0;i<MAX_COINS;i++)if(g->money[i].used==1&&fabsf(g->money[i].x-g->mini.heavy_x)<22){Coin *v=&g->money[i];v->used=2;v->age=0;v->vx=v->x;v->vy=v->y;}}

    for(int i=0;i<96;i++)if(g->mini.enemy_used[i]){
        float before=g->mini.enemy_x[i];g->mini.enemy_x[i]-=150*dt;
        int row=(int)((g->mini.enemy_y[i]+20.01f-GRID_Y)/game_row_height(g))-1;
        if(row<0)row=0;if(row>=game_rows(g))row=game_rows(g)-1;
        for(int col=COLS-1;col>=0;col--){
            float center=plant_x(col);
            if(before<center-12||g->mini.enemy_x[i]>center+12)continue;
            int anchor=col;
            if(col>0&&g->plants[row][col].kind==-2&&g->plants[row][col-1].kind==COB)anchor--;
            Plant *plant=&g->plants[row][anchor];
            if(g->pumpkin_hp[row][col]>0)damage_pumpkin(g,row,col,20);
            else if(plant->kind>=0){plant->hp-=20;if(plant->hp<=0)destroy_square(g,row,anchor,0);}
            else continue;
            g->mini.enemy_used[i]=0;cue(FOLEY_SPLAT);break;
        }
        if(g->mini.enemy_x[i]<0)g->mini.enemy_used[i]=0;
    }
    if(g->mode==MODE_AQUARIUM){mini_aquarium_step(g,dt);return 1;}
    if(g->mode==MODE_SLOT&&g->mini.spinning)mini_slot_step(g,dt);
    if(g->mode==MODE_MATCH)mini_match_step(g,dt);
    if(game_mini_active(g,MINI_PORTAL))mini_portals(g,dt);
    return 0;
}
unsigned game_mini_star_targets(int row){
    static const unsigned targets[]={0x008,0x018,0x07e,0x038,0x048};
    return (unsigned)row<sizeof(targets)/sizeof(targets[0])?targets[row]:0;
}
int game_mini_star_count(const Game *g){
    int count=0;
    for(int r=0;r<ROWS;r++)for(unsigned cells=game_mini_star_targets(r);cells;cells&=cells-1)
        count+=g->plants[r][__builtin_ctz(cells)].kind==STAR;
    return count;
}
static int mini_progress(Game *g){
    if(!g->challenge)return 0;
    int stars=game_mini_active(g,MINI_STARS);
    if(stars)g->mini.star_count=game_mini_star_count(g);
    if(g->mode==MODE_MATCH||g->mode==MODE_SLOT||stars){
        int win=g->mode==MODE_MATCH?g->mini.score>=75:g->mode==MODE_SLOT?
            g->sun>=SLOT_WIN_SUN&&g->mini.spinning==SLOT_READY:g->mini.star_count==game_mini_total(g);
        if(win){g->status=WON;sound(g,SFX_WIN);return 1;}
        if(g->wave==g->num_waves&&g->spawned>0&&!g->polish.wave_pending&&
           !wave_arrivals_pending024(g)&&!game_alive(g)){
            g->wave=g->num_waves-1;g->spawned=0;g->spawn_timer=5;
        }
    }
    if(game_mini_active(g,MINI_LASTSTAND)&&g->wave==g->num_waves&&g->spawned>0&&
       !g->polish.wave_pending&&!wave_arrivals_pending024(g)&&!game_alive(g)&&g->mini.stage<4){
        g->mini.stage++;
        memset(&g->polish024,0,sizeof(g->polish024));
        g->mini.phase=0;g->sun+=250;g->wave=1;g->spawned=0;g->huge_wave_announced=0;g->huge_wave_timer=0;
        mini_replan(g);
        return 1;
    }
    return 0;
}
int game_mini_total(const Game *g){return g->mode==MODE_MATCH?75:g->mode==MODE_SLOT?SLOT_WIN_SUN:g->mode==MODE_AQUARIUM?1000:game_mini_active(g,MINI_STARS)?14:g->num_waves;}
