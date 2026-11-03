#include "puzzle_data.h"
const char *puzzle_names[20]={"VASEBREAKER","TO THE LEFT","THIRD VASE","CHAIN REACTION","M IS FOR METAL","SCARY POTTER","HOKEY POKEY","ANOTHER CHAIN REACTION","ACE OF VASE","VASEBREAKER ENDLESS","I, ZOMBIE","I, ZOMBIE TOO","CAN YOU DIG IT?","TOTALLY NUTS","DEAD ZEPPELIN","ME SMASH!","ZOMBOOGIE","THREE HIT WONDER","ALL YOUR BRAINZ","I, ZOMBIE ENDLESS"};
static int extra_is(const Game *g,int kind){return g->extra.category==kind;}
static void extra_replan(Game *g){memset(g->wave_count,0,sizeof(g->wave_count));memset(g->wave_types,0,sizeof(g->wave_types));plan_waves(g);}
static int survival_allowed(const Game *g,int k){return k>=0&&k<Z_TYPES&&!!(g->extra.allowed_zombies&(1u<<k));}
static void survival_pick(Game *g){
    g->extra.allowed_zombies=1u<<Z_NORMAL;g->extra.allowed_zombies|=1u<<(g->extra.stage&&random_next(g)%5==0?Z_PAPER:Z_CONE);
    if(!g->extra.stage){if(g->extra.difficulty!=SURVIVAL_NORMAL)g->extra.allowed_zombies|=1u<<Z_BUCKET;return;}
    int choices[Z_TYPES],n=0;
    for(int k=0;k<Z_TYPES;k++){
        if(k==Z_NORMAL||k==Z_FLAG||k==Z_BACKUP||k==Z_DUCK||k==Z_YETI||k==Z_IMP||k==Z_BOSS)continue;
        if(g->extra.difficulty==SURVIVAL_NORMAL&&k>Z_SNORKEL)continue;
        if((k==Z_SNORKEL||k==Z_DOLPHIN)&&game_rows(g)!=6)continue;
        if(game_world(g)==4&&(k==Z_DIGGER||k==Z_DANCER))continue;
        if(game_world(g)==1&&k==Z_ZOMBONI)continue;
        if(game_world(g)!=4&&g->extra.difficulty!=SURVIVAL_ENDLESS&&k==Z_BUNGEE)continue;
        if(!(g->extra.allowed_zombies&(1u<<k)))choices[n++]=k;
    }
    int capacity=g->extra.stage+1;if(capacity>9)capacity=9;
    for(int j=0;j<capacity&&n;j++){int at=random_next(g)%n;g->extra.allowed_zombies|=1u<<choices[at];choices[at]=choices[--n];}
}
void game_start_survival(Game *g,int terrain,int difficulty,uint32_t seed){
    static const int levels[5]={9,19,29,39,49};if(terrain<0||terrain>4)terrain=0;if(difficulty<0||difficulty>2)difficulty=0;
    game_start_level(g,levels[terrain],seed);g->challenge=100;g->extra.category=EXTRA_SURVIVAL;g->extra.selection=terrain;g->extra.difficulty=difficulty;g->extra.goal=difficulty==SURVIVAL_NORMAL?5:difficulty==SURVIVAL_HARD?10:0;g->mode=MODE_LAWN;g->num_waves=difficulty==SURVIVAL_NORMAL?10:20;g->sun=50;g->row_mask=game_rows(g)==6?63:31;g->unlocked_mask=(UINT64_C(1)<<40)-1;g->seed_mask=(UINT64_C(1)<<PEA)|(UINT64_C(1)<<SUNFLOWER)|(UINT64_C(1)<<WALLNUT)|(UINT64_C(1)<<CHERRY)|(UINT64_C(1)<<SNOW)|(UINT64_C(1)<<LILY);g->level_flags=LEVEL_TUTORIAL_COMPLETE;survival_pick(g);extra_replan(g);
}
static void vase_put(Game *g,int content,int count,int blocked){
    int cells[45],n=0;for(int r=0;r<5;r++)for(int c=0;c<9;c++)if(!(blocked&(1<<c))&&!g->vases[r][c])cells[n++]=r*9+c;
    while(count-->0&&n){int at=random_next(g)%n,cell=cells[at];cells[at]=cells[--n];int r=cell/9,c=cell%9;g->vases[r][c]=1;g->extra.vase_content[r][c]=content;}
}
static void puzzle_clear(Game *g){
    memset(g->zombies,0,sizeof(g->zombies));memset(g->shots,0,sizeof(g->shots));memset(g->suns,0,sizeof(g->suns));memset(g->vases,0,sizeof(g->vases));memset(g->graves,0,sizeof(g->graves));memset(g->craters,0,sizeof(g->craters));memset(g->asleep,0,sizeof(g->asleep));memset(g->hypnotized,0,sizeof(g->hypnotized));memset(g->frozen,0,sizeof(g->frozen));memset(g->extra.vase_green,0,sizeof(g->extra.vase_green));memset(g->extra.left_facing,0,sizeof(g->extra.left_facing));memset(g->extra.packet_age,0,sizeof(g->extra.packet_age));
    for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++){g->plants[r][c]=(Plant){.kind=-1};g->support[r][c]=0;g->pumpkin_hp[r][c]=0;g->morph_timer[r][c]=0;}
    g->packet_count=0;g->selected=0;g->wave=1;g->spawned=0;g->spawn_timer=100000;g->huge_wave_timer=0;g->huge_wave_announced=0;g->extra.pending=0;g->extra.transition=0;g->level_flags=LEVEL_TUTORIAL_COMPLETE;
}
static void vase_setup(Game *g){
    puzzle_clear(g);int selection=g->extra.selection,blocked,green;
    if(selection==9){int extra=g->extra.stage/10;if(extra>8)extra=8;blocked=3;green=2;
        static const VaseContent seeds[]={{51,6},{SNOW,2},{PEA,1},{THREEPEATER,2},{SQUASH,5},{MINE,1},{WALLNUT,1},{PLANTERN,1},{-1-Z_BUCKET,5},{-1-Z_JACK,1}};
        for(unsigned i=0;i<sizeof(seeds)/sizeof(seeds[0]);i++)vase_put(g,seeds[i].value,seeds[i].count,blocked);vase_put(g,-1-Z_NORMAL,8-extra,blocked);vase_put(g,-1-Z_GARG,1+extra,blocked);vase_put(g,1000,1,blocked);
    }else{const VaseDefinition *def=&vase_defs[selection];blocked=def->blocked;green=def->green;for(int i=0;i<def->n;i++)vase_put(g,def->contents[i].value,def->contents[i].count,blocked);}
    int candidates[45],n=0;for(int r=0;r<5;r++)for(int c=0;c<9;c++)if(g->vases[r][c]&&g->extra.vase_content[r][c]>=0&&g->extra.vase_content[r][c]<1000)candidates[n++]=r*9+c;
    while(green-->0&&n){int at=random_next(g)%n,cell=candidates[at];candidates[at]=candidates[--n];g->extra.vase_green[cell/9][cell%9]=1;}
}
static int extra_break_vase(Game *g,int r,int c){
    if(!game_row_active(g,r)||c<0||c>=9||!g->vases[r][c])return 0;int content=g->extra.vase_content[r][c];
    if(content>=0&&content<1000&&g->packet_count>=MAX_PACKETS)return 0;
    if(content<0){int id=game_spawn_zombie(g,r,-1-content,plant_x(c));if(id<0)return 0;}else if(content==1000){for(int i=0;i<3;i++)plant_sun(g,plant_x(c)+i*5,game_ground_y(g,r)-20);}else{g->packets[g->packet_count]=content;g->extra.packet_age[g->packet_count++]=0;}
    g->vases[r][c]=0;g->extra.vase_green[r][c]=0;g->spawned++;sound_as(g,SFX_CHOMP,FOLEY_VASE_BREAKING);return 1;
}
static void iz_put(Game *g,int kind,int count,int restricted,int columns){
    int cells[45],n=0;for(int r=0;r<5;r++)if(restricted<0||r==restricted)for(int c=0;c<columns;c++)if(g->plants[r][c].kind<0&&((kind!=WALLNUT&&kind!=TORCHWOOD)||columns-c<=3))cells[n++]=r*9+c;
    while(count-->0&&n){int at=random_next(g)%n,cell=cells[at];cells[at]=cells[--n];put_plant(g,cell/9,cell%9,kind);g->asleep[cell/9][cell%9]=0;if(kind==MINE)g->plants[cell/9][cell%9].age=MINE_ARM_SECONDS;}
}
static void iz_setup(Game *g){
    static const int cards[10][10]={{Z_NORMAL,Z_BUCKET,Z_FOOTBALL,-1},{Z_NORMAL,Z_DOOR,Z_BUCKET,-1},{Z_NORMAL,Z_BUCKET,Z_DIGGER,-1},{Z_NORMAL,Z_BUCKET,Z_LADDER,-1},{Z_NORMAL,Z_BUCKET,Z_BUNGEE,Z_BALLOON,-1},{Z_NORMAL,Z_POLE,Z_BUCKET,Z_GARG,-1},{Z_NORMAL,Z_POLE,Z_BUCKET,Z_DANCER,-1},{Z_IMP,Z_CONE,Z_BUCKET,Z_BUNGEE,Z_DIGGER,Z_LADDER,-1},{Z_IMP,Z_CONE,Z_POLE,Z_BUCKET,Z_BUNGEE,Z_DIGGER,Z_LADDER,Z_FOOTBALL,-1},{Z_IMP,Z_CONE,Z_POLE,Z_BUCKET,Z_BUNGEE,Z_DIGGER,Z_LADDER,Z_FOOTBALL,Z_DANCER,-1}};
    puzzle_clear(g);int selection=g->extra.selection;g->extra.card_count=0;for(int i=0;i<10&&cards[selection][i]>=0;i++)g->extra.zombie_cards[g->extra.card_count++]=cards[selection][i];for(int r=0;r<5;r++)g->extra.brains[r]=70;g->extra.seed_selection=selection<=4?4:selection==8?6:5;
    if(selection<9){const IZDefinition *def=&iz_defs[selection];for(int i=0;i<def->nfixed;i++){IZPlacement p=def->fixed[i];put_plant(g,p.b,p.a,p.kind);g->asleep[p.b][p.a]=0;if(p.kind==MINE)g->plants[p.b][p.a].age=MINE_ARM_SECONDS;}for(int i=0;i<def->nrandom;i++)iz_put(g,def->random[i].kind,def->random[i].a,def->random[i].b,def->columns);}
    else{int puff=g->extra.stage==0?0:g->extra.stage==1?1:2+random_next(g)%4;if(puff>7)puff=7;iz_put(g,SUNFLOWER,8-puff,-1,5);iz_put(g,PUFF,puff,-1,5);static const int groups[5][6]={{SNOW,SPLIT,REPEATER,SNOW,SPLIT,REPEATER},{MINE,CHOMPER,MINE,CHOMPER,MINE,CHOMPER},{SPIKEWEED,STAR,SPIKEWEED,STAR,SPIKEWEED,STAR},{FUME,MAGNET,FUME,MAGNET,FUME,MAGNET},{PEA,SQUASH,KERNEL,TORCHWOOD,SNOW,WALLNUT}};int group=random_next(g)%5;for(int i=0;i<17;i++)iz_put(g,groups[group][i%6],1,-1,5);}
}
void game_start_puzzle(Game *g,int family,int selection,uint32_t seed){
    if(selection<0||selection>9)selection=0;game_start_level(g,19,seed);g->challenge=200+family*10+selection;g->mode=family?MODE_IZOMBIE:MODE_VASE;g->extra.category=family?EXTRA_IZOMBIE:EXTRA_VASE;g->extra.selection=selection;g->row_mask=31;g->sun=family?150:0;g->num_waves=1;g->unlocked_mask=g->seed_mask=0;g->store_flags=0;for(int r=0;r<ROWS;r++){g->mower_used[r]=1;g->mower_x[r]=600;}if(family)iz_setup(g);else vase_setup(g);
}
static const int iz_costs[Z_TYPES]={50,75,125,75,50,75,100,175,350,50,50,100,175,100,100,100,150,125,200,100,125,150,175,300,50,1000};
int game_zombie_cost(int kind){return kind>=0&&kind<Z_TYPES?iz_costs[kind]:100000;}
int game_extra_action(Game *g,int action){
    if(!extra_is(g,EXTRA_IZOMBIE)||action!=0||g->status!=PLAYING||g->extra.pending)return 0;
    int kind=g->extra.zombie_cards[g->selected%g->extra.card_count],col=g->cursor_col,row=g->cursor_row;
    if((kind==Z_BUNGEE&&col>=g->extra.seed_selection)||(kind!=Z_BUNGEE&&col<g->extra.seed_selection))return -6;if(g->sun<iz_costs[kind])return -3;
    int i=game_spawn_zombie(g,row,kind,plant_x(col)-14);if(i<0)return -7;g->sun-=iz_costs[kind];if(kind==Z_BUNGEE)g->zombies[i].x=plant_x(col);sound(g,SFX_PLANT);return 1;
}
static void extra_plant_damage(Game *g,int r,int c,float before,float after){
    if(!extra_is(g,EXTRA_IZOMBIE)||g->plants[r][c].kind!=SUNFLOWER)return;
    int a=(int)fmaxf(0,before)/40,b=(int)fmaxf(0,after)/40;for(int i=0;i<a-b+(after<=0?1:0);i++)plant_sun(g,plant_x(c)+i*3,game_ground_y(g,r)-20);
}
static int extra_iz_zombie(Game *g,int i,float dt){
    if(!extra_is(g,EXTRA_IZOMBIE))return 0;Zombie *z=&g->zombies[i];if(z->death>0||z->kind==Z_BUNGEE||g->frozen[i]>0||z->vault>0)return 0;
    if(z->kind==Z_DIGGER)return 0;
    if(z->x<GRID_X-4){int r=z->row;if(g->extra.brains[r]>0){g->extra.brain_eating[i]+=100*dt;int damage=(int)g->extra.brain_eating[i];g->extra.brain_eating[i]-=damage;g->extra.brains[r]-=damage;z->eating=1;z->age+=dt;if(g->extra.brains[r]<=0){g->extra.brains[r]=0;sound_as(g,SFX_CHOMP,FOLEY_SQUISH);}}else{z->used=0;}return 1;}return 0;
}
static int extra_pre_step(Game *g,float dt){
    if(g->extra.pending){g->extra.transition+=dt;return 1;}
    if(g->mode==MODE_VASE)for(int i=0;i<g->packet_count;i++){g->extra.packet_age[i]+=dt;if(g->extra.packet_age[i]>=15){for(int j=i;j<g->packet_count-1;j++){g->packets[j]=g->packets[j+1];g->extra.packet_age[j]=g->extra.packet_age[j+1];}g->packet_count--;if(g->selected>=g->packet_count)g->selected=g->packet_count?g->packet_count-1:0;i--;}}
    return 0;
}
static int extra_progress(Game *g){
    int category=g->extra.category;if(!category)return 0;
    if(category==EXTRA_SURVIVAL){if(g->wave==g->num_waves&&g->spawned>0&&!game_alive(g)){
        g->extra.flags+=g->num_waves/10;if(g->extra.goal&&g->extra.flags>=g->extra.goal){g->status=WON;sound(g,SFX_WIN);}else{g->extra.pending=1;g->extra.repick=1;g->extra.transition=0;}return 1;}return 0;}
    if(category==EXTRA_VASE){int remaining=0;for(int r=0;r<5;r++)for(int c=0;c<9;c++)remaining+=!!g->vases[r][c];if(!remaining&&!game_alive(g)){if(g->extra.selection==9){g->extra.pending=(g->extra.stage+1)%10==0?2:1;g->extra.transition=0;}else{g->status=WON;sound(g,SFX_WIN);}}return 1;}
    if(category==EXTRA_IZOMBIE){int brains=0,sun=0;for(int r=0;r<5;r++)brains+=g->extra.brains[r]>0;for(int i=0;i<MAX_SUNS;i++)sun+=!!g->suns[i].used;
        if(!brains){if(g->extra.selection==9){g->extra.pending=(g->extra.stage+1)%3==0?2:1;g->extra.transition=0;}else{g->status=WON;sound(g,SFX_WIN);}}else if(!game_alive(g)&&g->sun<50&&!sun&&!game_dying_count(g)){g->status=LOST;}return 1;}
    return 0;
}
int game_extra_continue(Game *g){
    if(!g->extra.pending||(g->extra.pending==2&&!(g->level_flags&LEVEL_REWARD_COLLECTED)))return 0;g->extra.stage++;g->extra.pending=0;g->extra.transition=0;
    if(extra_is(g,EXTRA_SURVIVAL)){g->level_flags&=~LEVEL_READY;memset(g->zombies,0,sizeof(g->zombies));memset(g->shots,0,sizeof(g->shots));g->wave=1;g->spawned=0;g->wave_delay=0;g->spawn_timer=18;g->huge_wave_timer=0;g->huge_wave_announced=0;g->fog_clear=0;for(int r=0;r<ROWS;r++)g->row_mowed_wave[r]=-100;survival_pick(g);extra_replan(g);return 1;}
    if(extra_is(g,EXTRA_VASE))vase_setup(g);else if(extra_is(g,EXTRA_IZOMBIE))iz_setup(g);return 1;
}

static int extra_target(const Game *g,int r,int c,float x){
    if(!g->extra.left_facing[r][c])return target_in_lane(g,r,x);
    for(int i=0;i<MAX_ZOMBIES;i++){const Zombie *z=&g->zombies[i];if(z->used&&z->death<=0&&!g->hypnotized[i]&&z->row==r&&z->x<x+6&&z->x>0&&z->kind!=Z_BALLOON)return 1;}return 0;
}
static void extra_shoot(Game *g,int r,int c,float x,int ice){
    if(!g->extra.left_facing[r][c]){shoot(g,r,x,ice);return;}
    for(int i=0;i<MAX_SHOTS;i++)if(!g->shots[i].used){shoot(g,r,x-26,ice);g->shot_vx[i]=-150;return;}
}

static void adventure_vases(Game *g){
 memset(g->vases,0,sizeof(g->vases));memset(g->extra.vase_green,0,sizeof(g->extra.vase_green));memset(g->extra.packet_age,0,sizeof(g->extra.packet_age));g->packet_count=0;g->selected=0;
 int blocked=g->stage==0?63:g->stage==1?31:15;
 vase_put(g,PEA,5-(g->stage==1),blocked);vase_put(g,g->stage?SNOW:SQUASH,5,blocked);if(g->stage==1)vase_put(g,SQUASH,4,blocked);if(g->stage==2)vase_put(g,HYPNO,5,blocked);
 vase_put(g,-1-Z_NORMAL,g->stage==0?4:g->stage==1?5:6,blocked);vase_put(g,-1-Z_BUCKET,g->stage==2?2:1,blocked);if(g->stage==1)vase_put(g,-1-Z_FOOTBALL,1,blocked);if(g->stage==2){vase_put(g,-1-Z_DANCER,1,blocked);vase_put(g,-1-Z_JACK,1,blocked);}
 int green=g->stage==1?2:g->stage==2?3:0;for(int r=0;r<5&&green;r++)for(int c=0;c<9&&green;c++)if(g->vases[r][c]&&g->extra.vase_content[r][c]>=0){g->extra.vase_green[r][c]=1;green--;}
 g->spawned=0;g->wave=g->stage+1;g->num_waves=3;g->spawn_timer=100000;
}
static int bobsled_step(Game *g,int i,float dt){
 Zombie *z=&g->zombies[i];int team=g->extra.bobsled_team[i];if(!team)return 0;int leader=team-1;
 if(leader<0||leader>=MAX_ZOMBIES){g->extra.bobsled_team[i]=0;return 0;}
    if(g->extra.bobsled_phase[i]<0){g->extra.bobsled_phase[i]+=dt;if(g->extra.bobsled_phase[i]<0){z->age+=dt;return 1;}g->extra.bobsled_team[i]=0;z->speed=.23f+random_unit(g)*.14f;return 0;}
    int col=(int)((g->zombies[leader].x-GRID_X)/CELL_W);if(!g->zombies[leader].used||g->zombies[leader].death>0||(col>=0&&col<COLS&&g->ice_tiles[z->row][col]<=0)){for(int j=0;j<MAX_ZOMBIES;j++)if(g->extra.bobsled_team[j]==team)g->extra.bobsled_phase[j]=-.6f;return 1;}
 if(i!=leader){Zombie *head=&g->zombies[leader];z->x=head->x+g->extra.bobsled_position[i]*14;z->row=head->row;z->age=head->age;return 1;}
 z->eating=0;z->age+=dt;z->x-=47.5f*dt;return 1;
}
