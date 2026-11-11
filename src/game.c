#include "save_file.h"
#include "game.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "campaign_data.h"
#include "plant_data.h"
#include "legacy_game.h"
#include "legacy_071.h"
#include "legacy_08.h"
#include "legacy_09.h"
_Static_assert(sizeof(Legacy09)==36424,"0.9 save layout");
_Static_assert(offsetof(Game,polish)==42480,"version 10 complete prefix");
_Static_assert(offsetof(Game,completion)==44424,"version 11 complete prefix");
_Static_assert(offsetof(Game,fidelity)==48456,"version 12 complete prefix");
#include "profile.h"
#include "death_data.h"

const char *seed_names[ALL_SEEDS] = {"PEA","SUN","NUT","BOMB","MINE","SNOW","CHOMP","REPEAT","PUFF","SUNSHROOM","FUME","GRAVEBUSTER","HYPNO","SCAREDY","ICE","DOOM","LILY","SQUASH","THREEPEA","KELP","JALAPENO","SPIKE","TORCH","TALLNUT","SEA","PLANTERN","CACTUS","BLOVER","SPLIT","STAR","PUMPKIN","MAGNET","CABBAGE","POT","KERNEL","COFFEE","GARLIC","UMBRELLA","MARIGOLD","MELON","GATLING","TWIN SUN","GLOOM","CATTAIL","WINTER","GOLD MAG","SPIKEROCK","COB","IMITATER"};
_Static_assert(sizeof(Legacy08)==29224,"0.8 save layout");
_Static_assert(sizeof(Legacy071)==26680,"0.7.1 save layout");
_Static_assert(sizeof(LegacyGame)==18748,"0.6 save layout");
static int survival_allowed(const Game*,int);
static int extra_break_vase(Game*,int,int);
static void adventure_vases(Game*);
static int bobsled_step(Game*,int,float);
static int boss_exposed(const Game*);
static int extra_iz_zombie(Game*,int,float);
static void extra_plant_damage(Game*,int,int,float,float);
int game_world(const Game *g){return g->level==35?1:(g->level-1)/10;}
int game_is_night(const Game *g){int w=game_world(g);return w==1||w==3||g->level==50;}
int game_rows(const Game *g){int w=game_world(g);return w==2||w==3?6:5;}
float game_row_height(const Game *g){return game_rows(g)==6?31:game_world(g)==4?32:36;}
float game_ground_y(const Game *g,int r){return GRID_Y+(r+1)*game_row_height(g)-2;}
float game_roof_offset(const Game *g,float x){return game_world(g)==4?fmaxf(0,GRID_X+4*CELL_W-x)*.20f:0;}
int game_water_row(const Game *g,int r){return (game_world(g)==2||game_world(g)==3)&&(r==2||r==3);}
int game_cleaner_active(const Game *g,int row){return game_row_active(g,row)&&g->mode!=MODE_VASE&&g->mode!=MODE_IZOMBIE&&(game_world(g)!=4||(g->store_flags&STORE_ROOF));}
int game_music_theme(const Game *g){if(g->mode==MODE_IZOMBIE)return 9;if(g->challenge&&g->mini.id>=1&&g->mini.id<=20){static const int themes[20]={1,3,1,1,5,6,1,5,5,3,5,4,6,3,3,6,6,3,8,10};return themes[g->mini.id-1];}if(g->level==40)return -1;if(g->mode==MODE_BOSS)return 10;if(g->mode==MODE_VASE)return 9;if(g->mode==MODE_BOWLING||g->mode==MODE_WHACK||g->mode==MODE_BUNGEE||g->level==25)return 3;if(g->mode==MODE_CONVEYOR||g->mode==MODE_BUNGEE)return 4;static const int themes[5]={1,5,6,7,8};return themes[game_world(g)];}
int game_seed_cost(const Game *g,int k){if(k==IMITATER)k=g->imitater_seed>=0&&g->imitater_seed<BASE_SEEDS?g->imitater_seed:0;int cost=k<SEEDS?g->seed_cost[k]:g->night_cost[k-SEEDS];if(g->extra.category==EXTRA_SURVIVAL&&g->extra.difficulty==SURVIVAL_ENDLESS&&k>=GATLING&&k<=COB)for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)if(g->plants[r][c].kind==k)cost+=50;return cost;}
int game_seed_refresh(const Game *g,int k){if(k==IMITATER)k=g->imitater_seed>=0&&g->imitater_seed<BASE_SEEDS?g->imitater_seed:0;return k<SEEDS?g->seed_refresh[k]:g->night_refresh[k-SEEDS];}
float *game_seed_cooldown(Game *g,int k){return k<SEEDS?&g->cooldown[k]:&g->night_cooldown[k-SEEDS];}

static uint32_t random_next(Game *g) { uint32_t x=g->rng; x^=x<<13; x^=x>>17; x^=x<<5; return g->rng=x; }
static float random_unit(Game *g) { return (random_next(g)&0xffff)/65535.0f; }
static float plant_x(int col) { return GRID_X+(col+.5f)*CELL_W; }
static int zombie_airborne(const Game *g,const Zombie *z){return z->kind==Z_BALLOON&&(!g->fidelity.equipment_lost[z-g->zombies]||g->polish.action[z-g->zombies]==ACTION_BALLOON_FALL);}
static int bungee_away(const Game *g,const Zombie *z){int phase=g->fidelity.bungee_phase[z-g->zombies];return z->kind==Z_BUNGEE&&phase!=1&&phase!=2;}

static void (*sound_listener)(int);
void game_sound_listener(void (*listener)(int)){sound_listener=listener;}
static void cue(int event){if(sound_listener)sound_listener(event);}
static void sound_as(Game *g,int type,int event){if(g->events[type]<8)g->events[type]++;cue(event);}
static void sound(Game *g,int type){static const int groups[SFX_COUNT]={FOLEY_PLANT,FOLEY_THROW,FOLEY_SUN,FOLEY_CHOMP,FOLEY_CHERRYBOMB,FOLEY_GROAN,FOLEY_LAWNMOWER,FOLEY_WINMUSIC};sound_as(g,type,type==SFX_WIN&&!g->challenge&&g->level==50?FOLEY_FINAL_FANFARE:groups[type]);}
static int bite_contact(float zombie_x,float plant_center) {
    return zombie_x>=plant_center-ZOMBIE_BITE_BACK&&zombie_x<=plant_center+ZOMBIE_BITE_FRONT;
}
typedef struct { uint32_t count;float fps,x[256]; } MotionProfile;
static MotionProfile motion[8];
int game_load_motion(const char *filename){
    FILE *f=fopen(filename,"rb");if(!f)return 0;char magic[4];uint32_t count;MotionProfile loaded[8]={0};
    int ok=fread(magic,4,1,f)==1&&!memcmp(magic,"PVM1",4)&&fread(&count,4,1,f)==1&&count>=3&&count<=8;
    for(uint32_t i=0;i<count&&ok;i++){
        MotionProfile *p=&loaded[i];ok=fread(&p->count,4,1,f)==1&&fread(&p->fps,4,1,f)==1&&p->count>=2&&p->count<=256&&p->fps>0&&p->fps<=60;
        if(ok)ok=fread(p->x,4,p->count,f)==p->count;
        for(uint32_t j=0;j<p->count&&ok;j++)ok=isfinite(p->x[j])&&fabsf(p->x[j])<10000;
        if(ok)ok=fabsf(p->x[0])<.001f&&p->x[p->count-1]>0;
    }
    if(ok)ok=fgetc(f)==EOF;fclose(f);if(ok)memcpy(motion,loaded,sizeof(motion));return ok;
}
static const MotionProfile *zombie_motion(const Zombie *z){
    int i=z->kind==Z_POLE?(z->vaulted?2:1):z->kind==Z_PAPER?(z->armor>0?3:4):z->kind==Z_FOOTBALL?5:z->kind==Z_DANCER?6:z->kind==Z_BACKUP?7:0;
    return &motion[motion[i].count>1?i:0];
}
static float ground_position(const MotionProfile *p,float phase){
    float loops=floorf(phase),frame=(phase-loops)*(p->count-1);int i=(int)frame;
    if(i>=(int)p->count-1)i=p->count-2;
    return loops*p->x[p->count-1]+p->x[i]+(p->x[i+1]-p->x[i])*(frame-i);
}
float game_zombie_walk(Zombie *z,float dt){
    const MotionProfile *p=zombie_motion(z);float velocity=z->speed,slow=z->slow>0?.5f:1;
    if(p->count<2){z->age=fmodf(z->age+dt,1);return velocity*47*(CELL_W/80.0f)*dt*slow;}
    float distance=p->x[p->count-1];
    // Original PS3 UpdateAnimSpeed: velocity * frames / ground_distance * 47.
    // Walking then follows GetTrackVelocity, rather than velocity * 100 ticks.
    float rate=velocity*p->count*47/distance;
    float old=z->age,next=old+dt*slow*rate/p->count;
    float moved=(ground_position(p,next)-ground_position(p,old))*p->count/(p->count-1);
    z->age=next-floorf(next);return moved*(CELL_W/80.0f);
}
float game_zombie_animation_time(const Zombie *z){
    if(z->death>0)return z->age*z->vault_from/death_source_fps[z->kind];
    if(z->eating||z->vault>0)return z->age;
    const MotionProfile *p=zombie_motion(z);return p->count>1?z->age*(p->count-1)/p->fps:z->age;
}

static const int seed_unlock[ALL_SEEDS]={1,2,4,3,6,7,8,9,11,12,13,14,16,17,18,19,21,22,23,24,26,27,28,29,31,32,33,34,36,37,38,39,41,42,43,44,46,47,48,49,99,99,99,99,99,99,99,99,99};
#include "zombie_spawn_data.h"
int game_zombie_allowed(int level,int kind){
    return kind>=0&&kind<Z_TYPES&&kind!=Z_FLAG&&kind!=Z_BACKUP&&level>=1&&level<=50&&campaign_allowed[kind][level-1];
}
static int mini_allowed(const Game*,int);
static void mini_spawned(Game*,int);
static int allowed_in_game(const Game *g,int kind){
    if(g->extra.category==EXTRA_SURVIVAL)return survival_allowed(g,kind);
    if(g->mini.id)return mini_allowed(g,kind);
    if(g->challenge==1)return kind==Z_NORMAL||kind==Z_CONE||kind==Z_BUCKET||kind==Z_PAPER||kind==Z_DOOR;
    if(g->challenge==2)return kind==Z_NORMAL||kind==Z_CONE||kind==Z_BUCKET;
    if(kind==Z_YETI)return g->completion.adventure_run&&(g->completion.adventure_run>=2||g->level>40)&&g->level!=40;
    return game_zombie_allowed(g->level,kind);
}
int game_flag_wave(const Game *g,int index){
    if(g->mode==MODE_VASE||g->mode==MODE_BOSS)return 0;
    int span=g->mode==MODE_WHACK?g->num_waves:g->num_waves<10?g->num_waves:10;
    return (g->level!=1||g->completion.adventure_run)&&index>=0&&index<g->num_waves&&index%span==span-1;
}
/* Supporting reconstruction interprets the flag-scaled caps; values and
   first-allowed waves themselves are checked against the owned PS3 table. */
static int flag_curve(int flags,int start,int end,int low,int high){if(flags<=start)return low;if(flags>=end)return high;return low+(high-low)*(flags-start)/(end-start);}
static int spawn_first_wave(const Game *g,int kind){
    int first=zombie_first_wave[kind];
    if(game_mini_active(g,MINI_BOBSLED)||game_mini_active(g,MINI_POGO))return 1;
    if(g->extra.category==EXTRA_SURVIVAL&&g->extra.difficulty==SURVIVAL_ENDLESS){first-=flag_curve(g->extra.flags,18,50,0,15);if(first<1)first=1;}
    return first;
}
static int garg_pick_weight(const Game *g,int wave,int giga){
    if(wave+1<spawn_first_wave(g,Z_GARG))return 0;
    if(giga?!g->completion.giga_allowed:!allowed_in_game(g,Z_GARG))return 0;
    if(!giga&&g->extra.category==EXTRA_SURVIVAL&&!(g->extra.allowed_zombies&(1u<<Z_GARG)))return 0;
    int weight=giga?6000:zombie_weight[Z_GARG];
    if(g->extra.category!=EXTRA_SURVIVAL)return giga?0:weight;
    int flag=game_flag_wave(g,wave),count=0,limit=flag_curve(g->extra.flags,10,50,2,50);
    if(giga){limit=flag?flag_curve(g->extra.flags,14,100,1,50):flag_curve(g->extra.flags,10,110,1,50);if(!flag)weight=1000;}
    for(int w=0;w<=wave;w++)if(w==wave||(giga&&!flag))for(int n=0;n<g->wave_count[w];n++)
        count+=g->wave_types[w][n]==Z_GARG&&!!g->completion.wave_giga[w][n]==giga;
    return count<limit?weight:0;
}
static int wave_pick_weight(const Game *g,int wave,int kind){
    if(kind==Z_GARG)return garg_pick_weight(g,wave,0)+garg_pick_weight(g,wave,1);
    if(wave+1<spawn_first_wave(g,kind))return 0;
    int weight=zombie_weight[kind];
    if(g->extra.category==EXTRA_SURVIVAL){
        if(g->extra.difficulty==SURVIVAL_ENDLESS&&kind==Z_BUNGEE&&!game_flag_wave(g,wave))return 0;
        if(kind==Z_NORMAL)weight=flag_curve(g->extra.flags,10,50,weight,weight/10);
        else if(kind==Z_CONE)weight=flag_curve(g->extra.flags,10,50,weight,weight/4);
        else if(kind==Z_ZOMBONI){int count=0;for(int n=0;n<g->wave_count[wave];n++)count+=g->wave_types[wave][n]==kind;if(count>=flag_curve(g->extra.flags,10,50,2,50))return 0;}
    }return weight;
}
static void put_wave(Game *g,int wave,int kind,int *points,int *counts){
    int *count=&g->wave_count[wave];if(*count>=WAVE_CAPACITY)return;
    int giga=0;if(kind==Z_GARG){int normal_weight=garg_pick_weight(g,wave,0),red_weight=garg_pick_weight(g,wave,1),total=normal_weight+red_weight;if(!total)return;giga=red_weight&&(!normal_weight||random_next(g)%(uint32_t)total>=(uint32_t)normal_weight);}
    int entry=(*count)++;g->wave_types[wave][entry]=kind;g->completion.wave_giga[wave][entry]=giga;
    *points-=zombie_value[kind];counts[kind]++;
}
static void plan_waves(Game *g){
    int introduced=g->level==3?Z_CONE:g->level==6?Z_POLE:g->level==8?Z_BUCKET:g->level==11?Z_PAPER:g->level==13?Z_DOOR:g->level==16?Z_FOOTBALL:g->level==18?Z_DANCER:-1;
    memset(g->completion.wave_giga,0,sizeof(g->completion.wave_giga));
    for(int w=0;w<g->num_waves;w++){
        int points=w/3+1,counts[Z_TYPES]={0};
        if(g->extra.category==EXTRA_SURVIVAL&&(g->extra.stage||g->extra.difficulty==SURVIVAL_ENDLESS))points=(g->extra.stage*g->num_waves+w+(g->extra.difficulty==SURVIVAL_ENDLESS?10:0))*2/5+1;
        if(g->mode==MODE_BUNGEE&&game_flag_wave(g,w)){int extra=100;for(int i=0;i<5;i++)put_wave(g,w,Z_BUNGEE,&extra,counts);if(w!=g->num_waves-1)continue;}
        if(game_flag_wave(g,w)){
            int plain=points<8?points:8;points=(int)(points*2.5f);
            for(int n=0;n<plain;n++)put_wave(g,w,Z_NORMAL,&points,counts);
            put_wave(g,w,Z_FLAG,&points,counts);
        }
        if(g->mode==MODE_BOWLING||g->level==25)points*=4;
        if(g->level==45)points*=2;
        if(g->level%10==0)points*=3;
        if(!g->challenge&&introduced>=0&&(w==g->num_waves/2||w==g->num_waves-1))put_wave(g,w,introduced,&points,counts);
        if(w==g->num_waves-1)for(int k=0;k<Z_TYPES;k++)
            if(k!=Z_YETI&&allowed_in_game(g,k)&&counts[k]==0)put_wave(g,w,k,&points,counts);
        while(points>0&&g->wave_count[w]<WAVE_CAPACITY){
            int weights[Z_TYPES]={0},total=0;
            for(int k=0;k<Z_TYPES;k++)if(allowed_in_game(g,k)&&zombie_value[k]<=points)total+=(weights[k]=wave_pick_weight(g,w,k));
            if(!total){put_wave(g,w,Z_NORMAL,&points,counts);continue;}
            int pick=(int)(random_next(g)%(uint32_t)total),kind=0;
            for(;kind<Z_TYPES-1;kind++){if(pick<weights[kind])break;pick-=weights[kind];}
            put_wave(g,w,kind,&points,counts);
        }
    }
}
int game_row_active(const Game *g,int row){return row>=0&&row<ROWS&&(g->row_mask&(1<<row));}
int game_seed_available(const Game *g,int kind){return kind>=0&&kind<ALL_SEEDS&&(g->seed_mask&(UINT64_C(1)<<kind));}
float game_sunflower_glow(const Plant *p){
    return p->kind==SUNFLOWER?fminf(1,fmaxf(p->flash,1-p->attack)):0;
}
int game_selected_kind(const Game *g){
    return (g->mode==MODE_LAWN||g->mode==MODE_WHACK)?g->selected:g->selected>=0&&g->selected<g->packet_count?g->packets[g->selected]:-1;
}
void game_cycle_seed(Game *g,int direction){
    if(g->mode==MODE_IZOMBIE){g->selected=(g->selected+g->extra.card_count+direction)%g->extra.card_count;return;}
    if(g->mode!=MODE_LAWN&&g->mode!=MODE_WHACK){g->selected=g->packet_count?(g->selected+g->packet_count+direction)%g->packet_count:0;return;}
    for(int i=0;i<ALL_SEEDS;i++){g->selected=(g->selected+ALL_SEEDS+direction)%ALL_SEEDS;if(game_seed_available(g,g->selected))break;}
}
static void chapter_start(Game *g);
static void setup_vases(Game *g);
void game_start_level(Game *g,int level,uint32_t seed) {
    if(level<1||level>CAMPAIGN_LEVELS)level=1;
    memset(g,0,sizeof(*g)); g->rng=seed?seed:0x51a7c0de;g->level=level;
    g->num_waves=level==15?8:campaign_waves[level-1];g->mode=level==5?MODE_BOWLING:level==15?MODE_WHACK:level==25?MODE_CONVEYOR:level==35?MODE_VASE:level==45?MODE_BUNGEE:level==50?MODE_BOSS:level%10==0?MODE_CONVEYOR:MODE_LAWN;
    g->row_mask=level==1?4:level<=3?14:game_rows(g)==6?63:31;
    for(int k=0;k<ALL_SEEDS;k++)if(level>=seed_unlock[k])g->unlocked_mask|=UINT64_C(1)<<k;
    int chosen=0;
    for(int k=0;k<ALL_SEEDS;k++)if((g->unlocked_mask&(UINT64_C(1)<<k))&&chosen<6){g->seed_mask|=UINT64_C(1)<<k;chosen++;}
    g->sun=g->mode==MODE_LAWN?(level==1?150:50):0;
    g->spawn_timer=level==2?50:level==5?2:18;
    g->sky_timer=4.25f+random_unit(g)*2.75f; g->cursor_row=2; g->wave=1;
    g->conveyor_timer=4;g->last_packet=-1;g->seed_slots=6;g->imitater_seed=-1;
    if(g->mode==MODE_BOWLING){g->packets[0]=WALLNUT;g->packet_count=1;g->shovel_tutorial=3;}
    for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)g->plants[r][c].kind=-1;
    memcpy(g->seed_cost,all_costs,8*sizeof(int));memcpy(g->seed_refresh,all_refresh,8*sizeof(int));memcpy(g->night_cost,all_costs+8,41*sizeof(int));memcpy(g->night_refresh,all_refresh+8,41*sizeof(int));
    if(game_world(g)==1&&level!=35){
        int count=level==15?9:4+(level-11)/2;
        for(int i=0;i<count;i++){int row=random_next(g)%game_rows(g),col=4+random_next(g)%5;g->graves[row][col]=1;}
        if(g->mode==MODE_WHACK){g->sun=0;g->seed_mask=(1<<GRAVEBUSTER)|(1<<ICESHROOM)|(1<<CHERRY);g->selected=GRAVEBUSTER;}
    }
    for(int r=0;r<ROWS;r++){g->mower_x[r]=GRID_X-19;g->row_mowed_wave[r]=-100;}
    if(g->shovel_tutorial)for(int c=0;c<3;c++)g->plants[2][c]=(Plant){.kind=WALLNUT,.hp=4000};
    plan_waves(g);
    chapter_start(g);
}
#include "completion_rules.inc"
void game_init(Game *g,uint32_t seed){game_start_level(g,1,seed);}
int game_sunflower_count(const Game *g){
    int count=0;for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)count+=g->plants[r][c].kind==SUNFLOWER;return count;
}
static void tutorial_planting(Game *g){
    if(g->total_spawned||(g->level_flags&LEVEL_TUTORIAL_COMPLETE)||g->level>2)return;
    int count=0;
    if(g->level==1){for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)count+=g->plants[r][c].kind==PEA;}
    else count=game_sunflower_count(g);
    if(count>=(g->level==1?2:3)){
        // PS3 SetTutorialState writes 100 ticks for 1-1 and 1000 for 1-2.
        // The 50-second initial 1-2 timer is only a tutorial fallback.
        g->spawn_timer=g->level==1?1:10;g->level_flags|=LEVEL_TUTORIAL_COMPLETE;
    }
}
int game_next_level(Game *g){
    if(g->status!=WON||!(g->level_flags&LEVEL_REWARD_COLLECTED)||g->level>=CAMPAIGN_LEVELS)return 0;
    int level=g->level+1;uint32_t seed=g->rng,run=g->completion.adventure_run;game_start_adventure(g,level,seed,run);return 1;
}

int game_upgrade_base(int kind){static const int bases[8]={REPEATER,SUNFLOWER,FUME,LILY,MELON,MAGNET,SPIKEWEED,KERNEL};return kind>=GATLING&&kind<=COB?bases[kind-GATLING]:-1;}
static int placement_check(Game *g,int row,int col,int kind){
    Plant *p=&g->plants[row][col];
    if(g->morph_timer[row][col]>0)return -2;
    if(game_mini_active(g,MINI_PORTAL))for(int i=0;i<4;i++)if(g->mini.portals[i]==row*10+col)return -9;
    if(g->vases[row][col]||g->ice_tiles[row][col]>0||g->craters[row][col]>0)return -9;
    if(kind==IMITATER){if(g->imitater_seed<0||g->imitater_seed>=BASE_SEEDS)return -13;return placement_check(g,row,col,g->imitater_seed);}
    int base=game_upgrade_base(kind);
    if(base>=0){
        if(kind==CATTAIL)return g->support[row][col]==LILY+1&&p->kind==-1?0:-12;
        if(p->kind!=base)return -12;
        if(kind==COB&&(col>=COLS-1||g->plants[row][col+1].kind!=KERNEL))return -12;
        return 0;
    }
    if((g->store_flags&STORE_FIRST_AID)&&((kind==WALLNUT||kind==TALLNUT)&&p->kind==kind))return 0;
    if((g->store_flags&STORE_FIRST_AID)&&kind==PUMPKIN&&g->pumpkin_hp[row][col]>0)return 0;
    if(kind==COFFEE)return p->kind>=0&&g->asleep[row][col]&&g->fidelity.wake[row][col]<=0?0:-11;
    if(kind==PUMPKIN){if((game_water_row(g,row)&&g->support[row][col]!=LILY+1&&p->kind!=CATTAIL)||(game_world(g)==4&&g->support[row][col]!=POT+1))return -11;return g->pumpkin_hp[row][col]>0?-2:0;}
    if(game_world(g)==4&&(kind==MINE||kind==SPIKEWEED))return -11;
    if(game_water_row(g,row)&&(kind==MINE||kind==SPIKEWEED||kind==SPIKEROCK||kind==POT))return -11;
    if(kind==LILY||kind==POT){if(g->support[row][col])return -2;if(kind==LILY&&!game_water_row(g,row))return -11;return p->kind>=0?-2:0;}
    if(p->kind!=-1)return -2;
    if(g->graves[row][col]!=(kind==GRAVEBUSTER))return -10;
    if(kind==KELP||kind==SEA){if(!game_water_row(g,row))return -11;}
    else if(game_water_row(g,row)&&g->support[row][col]!=LILY+1&&p->kind!=CATTAIL)return -11;
    if(game_world(g)==4&&g->support[row][col]!=POT+1)return -11;
    return 0;
}
static void destroy_square(Game *g,int row,int col,int layers){
 if(row<0||row>=ROWS||col<0||col>=COLS)return;Plant *p=&g->plants[row][col];
 if(g->cob_aiming&&g->cob_row==row&&(g->cob_col==col||g->cob_col+1==col)){g->cob_aiming=0;g->cob_row=g->cob_col=-1;}
 if(p->kind==-2&&col>0&&g->plants[row][col-1].kind==COB)destroy_square(g,row,col-1,layers);
 if(p->kind==COB&&col+1<COLS)g->plants[row][col+1].kind=-1;
 extra_plant_damage(g,row,col,p->hp,0);p->kind=-1;g->morph_timer[row][col]=0;g->burst_left[row][col]=0;g->pea_shoot_time[row][col]=0;g->asleep[row][col]=0;
 if(layers){g->pumpkin_hp[row][col]=0;g->support[row][col]=0;g->support_hp[row][col]=0;}
}
static void put_plant(Game *g,int row,int col,int kind){
    if(kind!=COFFEE){int layer=kind==LILY||kind==POT?2:kind==PUMPKIN?1:0;g->fidelity.plant_id[layer][row][col]=++g->fidelity.next_plant;}
    if(kind==IMITATER){g->morph_kind[row][col]=g->imitater_seed;g->morph_timer[row][col]=2+30.0f/26;g->morph_hp[row][col]=300;return;}
    if(kind==LILY||kind==POT){g->support[row][col]=kind+1;g->support_hp[row][col]=300;g->fidelity.support_safe[row][col]=1;g->imitated[row][col]&=~2;return;}
    if(kind==PUMPKIN){g->pumpkin_hp[row][col]=4000;g->imitated[row][col]&=~4;return;}
    if(kind==COFFEE){g->fidelity.wake[row][col]=1;cue(FOLEY_COFFEE);return;}
    g->fidelity.wake[row][col]=0;g->fidelity.plant_target_id[row][col]=0;g->extra.left_facing[row][col]=0;g->pea_shoot_time[row][col]=0;g->burst[row][col]=0;g->burst_target[row][col]=-1;Plant *p=&g->plants[row][col];int asleep=g->asleep[row][col];
    if(kind==CATTAIL){g->support[row][col]=0;g->imitated[row][col]&=~2;}
    if(kind==COB)g->plants[row][col+1]=(Plant){.kind=-2};
    *p=(Plant){.kind=kind,.hp=kind==WALLNUT?4000:kind==TALLNUT?8000:300,.attack=(kind==SUNFLOWER||kind==SUNSHROOM)?3+random_unit(g)*9.5f:.3f};
    g->asleep[row][col]=(kind==GLOOM?asleep:0)||(((kind>=PUFF&&kind<=DOOM&&kind!=GRAVEBUSTER)||kind==SEA||kind==MAGNET||kind==GLOOM)&&!game_is_night(g));
    if(kind==COB)p->attack=5+14.0f/12;
    if(kind==SPIKEROCK)p->hp=450;
    if(kind==IMITATER){p->digest=g->imitater_seed;g->asleep[row][col]=0;}
    else{g->imitated[row][col]&=~1;g->burst_left[row][col]=0;}
}
int game_place(Game *g,int row,int col,int kind){
    if(g->status!=PLAYING||!game_row_active(g,row)||col<0||col>=COLS||kind<0||kind>LEFT_PACKET||(kind==LEFT_PACKET&&g->mode!=MODE_VASE))return -1;
    if(game_mini_active(g,MINI_LASTSTAND)){int actual=kind==IMITATER?g->imitater_seed:kind;if(actual==SUNFLOWER||actual==SUNSHROOM||actual==TWIN)return -5;}
    int belt=g->mode!=MODE_LAWN&&g->mode!=MODE_WHACK;
    if(belt){
        if(g->shovel_tutorial)return -8;
        if(g->selected<0||g->selected>=g->packet_count||g->packets[g->selected]!=kind)return -5;
        if(g->mode==MODE_BOWLING){
            if(col>2)return -6;int n;for(n=0;n<MAX_NUTS;n++)if(!g->nuts[n].used)break;if(n==MAX_NUTS)return -7;
            g->nuts[n]=(BowlingNut){.used=1,.explosive=kind==GIANT_NUT?2:kind==EXPLODE_NUT,.row=row,.target_row=row,.last_hit=-1,.x=plant_x(col),.y=game_ground_y(g,row)};
        }else{int actual=kind==LEFT_PACKET?REPEATER:kind;if(actual>=ALL_SEEDS)return -1;int e=placement_check(g,row,col,actual);if(e)return e;put_plant(g,row,col,actual);if(kind==LEFT_PACKET)g->extra.left_facing[row][col]=1;
            if(game_mini_active(g,MINI_COLUMN))for(int r=0;r<game_rows(g);r++)if(r!=row&&!placement_check(g,r,col,kind))put_plant(g,r,col,kind);
        }
        for(int i=g->selected;i<g->packet_count-1;i++){g->packets[i]=g->packets[i+1];g->extra.packet_age[i]=g->extra.packet_age[i+1];}g->packet_count--;if(g->selected>=g->packet_count)g->selected=g->packet_count?g->packet_count-1:0;
    }else{
        if(kind>=ALL_SEEDS||!game_seed_available(g,kind))return -5;int e=placement_check(g,row,col,kind);if(e)return e;
        if(g->sun<game_seed_cost(g,kind))return -3;if(*game_seed_cooldown(g,kind)>.001f)return -4;
        put_plant(g,row,col,kind);g->sun-=game_seed_cost(g,kind);*game_seed_cooldown(g,kind)=game_seed_refresh(g,kind)/100.0f;tutorial_planting(g);
    }sound_as(g,SFX_PLANT,game_water_row(g,row)?FOLEY_PLANT_WATER:FOLEY_PLANT);if(kind==GRAVEBUSTER)cue(FOLEY_GRAVEBUSTERCHOMP);if(kind==POT)cue(FOLEY_CERAMIC);if(kind==PLANTERN)cue(GAME_SAMPLE(FX_PLANTERN));if(kind==DOOM)cue(FOLEY_REVERSE_EXPLOSION);return 0;
}
void game_shovel(Game *g,int row,int col){
    if((g->level<5&&!g->completion.adventure_run)||!game_row_active(g,row)||col<0||col>=COLS)return;
    if(g->morph_timer[row][col]>0){g->morph_timer[row][col]=0;sound_as(g,SFX_PLANT,FOLEY_USE_SHOVEL);return;}
    if(g->plants[row][col].kind==-2&&col>0)col--;
    if(g->plants[row][col].kind>=0){destroy_square(g,row,col,0);if(g->shovel_tutorial)g->shovel_tutorial--;}
    else if(g->pumpkin_hp[row][col]>0)g->pumpkin_hp[row][col]=0;else g->support[row][col]=0;sound_as(g,SFX_PLANT,FOLEY_USE_SHOVEL);
}

static void add_sun(Game *g,float x,float y,float target) {
    for(int i=0;i<MAX_SUNS;i++)if(!g->suns[i].used){g->suns[i]=(Sun){.used=1,.x=x,.y=y,.target_y=target,.vy=28};return;}
}
static int plant_sun(Game *g,float x,float y){
    for(int i=0;i<MAX_SUNS;i++)if(!g->suns[i].used){
        g->suns[i]=(Sun){.used=1,.from_plant=1,.x=x,.y=y,.target_y=y+6+random_unit(g)*7.2f,
            .vx=(-.4f+random_unit(g)*.8f)*47.5f,.vy=(-1.7f-random_unit(g)*1.7f)*36};return i;
    }return -1;
}

int game_collect(Game *g) {
    if(g->mode==MODE_HEAVY)return 0;
    int count=0;
    for(int i=0;i<MAX_SUNS;i++)if(g->suns[i].used==1){
        Sun *s=&g->suns[i];s->used=2;s->vx=s->x;s->vy=s->y;s->age=0;s->flight=.30f+hypotf(s->x-22,s->y-20)*.0005f;count++;
    }
    if(count)sound(g,SFX_SUN);return count+game_collect_coins(g);
}
static void coin_step(Game *g,float dt);
void game_animate_pickups(Game *g,float dt){
    coin_step(g,dt);
    for(int i=0;i<MAX_SUNS;i++)if(g->suns[i].used==2){
        Sun *s=&g->suns[i];s->age+=dt;float t=fminf(1,s->age/s->flight),ease=t*t*(3-2*t);
        s->x=s->vx+(22-s->vx)*ease;s->y=s->vy+(20-s->vy)*ease;
        if(t>=1){s->used=0;g->sun+=s->from_plant>1?s->from_plant:25;}
    }
}

void game_add_coin(Game *g,float x,float y,int value){for(int i=0;i<MAX_COINS;i++)if(!g->money[i].used){g->money[i]=(Coin){.used=1,.value=value,.x=x,.y=y,.vx=-12+random_unit(g)*24,.vy=-45,.target_y=y+25};return;}}
int game_collect_coins(Game *g){int n=0;for(int i=0;i<MAX_COINS;i++)if(g->money[i].used==1){Coin *c=&g->money[i];c->used=2;c->age=0;c->vx=c->x;c->vy=c->y;cue(c->value>=100?FOLEY_ART_CHALLENGE:c->value<0?FOLEY_PRIZE:FOLEY_COIN);n++;}return n;}
static void coin_step(Game *g,float dt){for(int i=0;i<MAX_COINS;i++){Coin *c=&g->money[i];if(!c->used)continue;c->age+=dt;if(c->used==2){float t=fminf(1,c->age/.4f);c->x=c->vx+(440-c->vx)*t;c->y=c->vy+(238-c->vy)*t;if(t>=1){if(c->value>0)g->money_collected+=c->value;else{if(c->value==-200)g->store_flags|=64;if(c->value==-201)g->store_flags|=128;g->extra.garden_drops[g->extra.garden_drop_count%32]=c->value;g->extra.garden_drop_count++;}c->used=0;}}else if(c->vy<0||c->y<c->target_y){c->x+=c->vx*dt;c->y=fminf(c->target_y,c->y+c->vy*dt);c->vy+=150*dt;}else if(c->age>15)c->used=0;}}
int game_spawn_zombie(Game *g,int row,int kind,float x) {
    if(row<0||row>=ROWS||kind<0||kind>=Z_TYPES)return -1;
    for(int i=0;i<MAX_ZOMBIES;i++)if(!g->zombies[i].used){
        g->fidelity.actor_id[i]=++g->fidelity.next_actor;if(!g->fidelity.actor_id[i])g->fidelity.actor_id[i]=++g->fidelity.next_actor;g->fidelity.equipment_lost[i]=0;g->fidelity.actor_aux[i]=0;g->fidelity.actor_target[i]=0;
        g->polish.pose[i]=g->polish.action_time[i]=0;g->polish.action[i]=g->polish.target[i]=g->polish.death_style[i]=0;g->frozen[i]=0;g->special[i]=kind==Z_DANCER?3:0;g->hypnotized[i]=0;g->zombies[i]=(Zombie){.used=1,.row=row,.kind=kind,.x=x,.hp=kind==Z_POLE||kind==Z_DANCER?500:270,
                            .armor=kind==Z_CONE?370:kind==Z_BUCKET||kind==Z_DOOR?1100:kind==Z_PAPER?150:kind==Z_FOOTBALL?1400:0,.wave=g->wave,
                            .speed=kind==Z_DANCER?.5f:kind==Z_FOOTBALL?.67f:kind==Z_FLAG?.45f:kind==Z_POLE?.66f+random_unit(g)*.02f:.23f+random_unit(g)*.14f};
        Zombie *z=&g->zombies[i];g->extra.bobsled_team[i]=0;g->extra.bobsled_phase[i]=0;g->extra.last_portal_zombie[i]=0;g->extra.brain_eating[i]=0;g->extra.hit_flash[i]=0;
        if(kind==Z_ZOMBONI||kind==Z_CATAPULT){z->hp=kind==Z_CATAPULT?850:1350;z->speed=.45f;if(kind==Z_CATAPULT)g->fidelity.actor_aux[i]=20;}
        if(kind==Z_DOLPHIN||kind==Z_JACK||kind==Z_POGO)z->hp=500;
        if(kind==Z_GARG){z->hp=3000;z->speed=.28f;}
        g->completion.yeti_timer[i]=0;g->completion.yeti_retreat[i]=0;
        if(kind==Z_YETI){z->hp=1350;z->speed=.4f;g->completion.yeti_timer[i]=15+random_unit(g)*5;g->completion.yeti_spawned=1;}
        g->completion.giga[i]=0;
        if(kind==Z_IMP){z->hp=g->extra.category==EXTRA_IZOMBIE?70:270;z->speed=.65f;}
        if(kind==Z_LADDER){z->hp=500;z->armor=500;z->speed=.5f;}
        if(kind==Z_DOLPHIN||kind==Z_POGO||kind==Z_JACK||kind==Z_BOBSLED)z->speed=.65f;
        if(kind==Z_JACK){int distance=450+random_next(g)%300;if(random_next(g)%20==0)distance/=3;g->fidelity.actor_aux[i]=(int)(distance/z->speed)*2/100.0f;if(g->mode==MODE_VASE)g->fidelity.actor_aux[i]=.1f;}
        if(kind==Z_BUNGEE){z->hp=450;z->x=plant_x(random_next(g)%COLS);g->fidelity.actor_aux[i]=(3000+random_next(g)%151)/800.0f;g->fidelity.bungee_phase[i]=g->fidelity.carried_kind[i]=0;g->fidelity.grabbed_id[i]=0;}
        if(g->level==25){z->hp*=.25f;z->armor*=.25f;}
        mini_spawned(g,i);g->seen_zombies|=1u<<kind;g->total_spawned++;if(g->events[SFX_ZOMBIE]<8)g->events[SFX_ZOMBIE]++;
        if(kind==Z_DOLPHIN)cue(FOLEY_DOLPHIN_APPEARS);else if(kind==Z_BALLOON)cue(FOLEY_BALLOONINFLATE);else if(kind==Z_ZOMBONI)cue(FOLEY_ZAMBONI);else if(kind==Z_DANCER)cue(FOLEY_DANCER);else if(kind==Z_BUNGEE)cue(FOLEY_BUNGEE_SCREAM);else if(kind==Z_IMP)cue(FOLEY_IMP);else if(kind==Z_DUCK||kind==Z_SNORKEL)cue(FOLEY_ZOMBIE_ENTERING_WATER);return i;
    }
    return -1;
}

int game_alive(const Game *g) {
    int count=0;for(int i=0;i<MAX_ZOMBIES;i++)if(g->zombies[i].used&&g->zombies[i].death<=0&&!g->hypnotized[i])count++;return count;
}
int game_zombie_max_health(const Game *g,const Zombie *z){
 int k=z->kind;if(k==Z_GARG)return g&&g->completion.giga[z-g->zombies]?6000:3000;if(k==Z_YETI||k==Z_ZOMBONI)return 1350;if(k==Z_CATAPULT)return 850;if(k==Z_BUNGEE)return 450;
 if(k==Z_POLE||k==Z_DANCER||k==Z_DOLPHIN||k==Z_JACK||k==Z_POGO||k==Z_LADDER)return 500;
 if(k==Z_IMP&&g&&g->extra.category==EXTRA_IZOMBIE)return 70;return 270;
}
int game_zombie_body_stage_for(const Game *g,const Zombie *z){int maximum=game_zombie_max_health(g,z);return z->hp<maximum/3?2:z->hp<maximum*2/3?1:0;}
int game_zombie_body_stage(const Zombie *z){return game_zombie_body_stage_for(0,z);}
int game_zombie_has_head(const Game *g,const Zombie *z){
 if(g->level==25||game_mini_active(g,MINI_LITTLE)||z->kind==Z_GARG||z->kind==Z_BOSS||z->kind==Z_BUNGEE||z->kind==Z_ZOMBONI||z->kind==Z_CATAPULT||zombie_airborne(g,z)||(z->kind==Z_BOBSLED&&g->extra.bobsled_team[z-g->zombies]&&g->extra.bobsled_phase[z-g->zombies]>=0))return 1;
 return game_zombie_body_stage_for(g,z)<2;
}
float game_vault_duration(const Zombie *z){return z->kind==Z_DOLPHIN?22/24.0f:z->kind==Z_POGO?.8f:43/24.0f;}
int game_zombie_armor_stage(const Zombie *z){
    int maximum=z->kind==Z_CONE?370:z->kind==Z_BUCKET?1100:0;
    return !maximum||z->armor<=0?0:z->armor<maximum/3?2:z->armor<maximum*2/3?1:0;
}

/* Vault motion is inactive after death; vault_from then stores the source
   death playback rate, preserving the existing on-disk Zombie layout. */
static float death_rate(Game *g,int kind){return kind==Z_FOOTBALL?24:kind==Z_GARG||kind==Z_SNORKEL||kind==Z_YETI?14:kind==Z_DIGGER||kind==Z_BOSS?18:24+random_unit(g)*6;}
float game_death_duration(const Zombie *z){return (death_frames[z->kind]-1)/z->vault_from+.60f;}
float game_death_duration_for(const Game *g,const Zombie *z){int style=g->polish.death_style[z-g->zombies];return style==DEATH_CHARRED?42/15.0f+.60f:style==DEATH_WATER?24/12.0f+.60f:game_death_duration(z);}
float game_death_alpha(const Zombie *z){return fminf(1,fmaxf(0,z->death/.30f));}
static void tick_death(Zombie *z,float dt){z->age+=dt;z->death=fmaxf(0,z->death-dt);if(z->death<=0)z->used=0;}
int game_dying_count(const Game *g){int n=0;for(int i=0;i<MAX_ZOMBIES;i++)n+=g->zombies[i].used&&g->zombies[i].death>0;return n;}
static void effects_step(Game *g,float dt){completion_effects_step(g,dt);for(int i=0;i<32;i++)g->extra.effect_time[i]=fmaxf(0,g->extra.effect_time[i]-dt);for(int i=0;i<MAX_ZOMBIES;i++)g->extra.hit_flash[i]=fmaxf(0,g->extra.hit_flash[i]-dt);}
static void combat_effect(Game *g,float x,float y,int kind){for(int i=0;i<32;i++)if(g->extra.effect_time[i]<=0){g->extra.effect_time[i]=kind?.5f:.2f;g->extra.effect_x[i]=x;g->extra.effect_y[i]=y;g->extra.effect_kind[i]=kind;break;}}
/* Keep cosmetic debris in the existing effect pool, without changing saves or
   drawing random numbers from the gameplay generator. The owned particle XML
   specifies a 180-tick life, gravity, ground contact and 0.3 reflection. */
static void head_effect(Game *g,const Zombie *z){
    int slot=-1;for(int i=0;i<32;i++)if(g->extra.effect_time[i]<=0){slot=i;break;}
    if(slot<0)for(int i=0;i<32;i++)if(g->extra.effect_kind[i]<2){slot=i;break;}
    if(slot<0)return;
    int kind=z->kind==Z_POLE?1:z->kind==Z_FOOTBALL?2:z->kind==Z_DANCER?3:z->kind==Z_BACKUP?4:z->kind==Z_BALLOON?5:z->kind==Z_BOBSLED?6:z->kind==Z_DIGGER?7:z->kind==Z_DOLPHIN?8:z->kind==Z_IMP?9:z->kind==Z_LADDER?10:z->kind==Z_POGO?11:z->kind==Z_YETI?12:0;
    g->extra.effect_time[slot]=1.8f;g->extra.effect_kind[slot]=2+kind;g->extra.effect_x[slot]=z->x-5;
    g->extra.effect_y[slot]=game_ground_y(g,z->row)+game_roof_offset(g,z->x)+2;cue(FOLEY_LIMBS_POP);
}
int game_head_particle(const Game *g,int index,HeadParticle *out){
    if(index<0||index>=32||g->extra.effect_time[index]<=0||g->extra.effect_kind[index]<2||g->extra.effect_kind[index]>14)return 0;
    float age=1.8f-g->extra.effect_time[index],remaining=fmaxf(0,age),height=-32,velocity=-42;
    for(int bounce=0;bounce<4;bounce++){
        float hit=(-velocity+sqrtf(velocity*velocity-420*height))/210;
        if(remaining<hit){height+=velocity*remaining+105*remaining*remaining;break;}
        remaining-=hit;velocity=-(velocity+210*hit)*.3f;height=0;
        if(bounce==3)remaining=0;
    }
    float spin=fminf(age,.72f),direction=index%2?1:-1;
    out->kind=g->extra.effect_kind[index]-2;out->x=g->extra.effect_x[index]-(14+index%3*4)*age;
    out->y=g->extra.effect_y[index]+fminf(0,height);out->rotation=direction*12.56637f*(spin-spin*spin/1.44f);
    out->alpha=fminf(1,g->extra.effect_time[index]/.09f);return 1;
}
void game_animate_deaths(Game *g,float dt){if(dt<=0)return;effects_step(g,dt);g->flash=fmaxf(0,g->flash-dt);for(int i=0;i<MAX_ZOMBIES;i++)if(g->zombies[i].used&&g->zombies[i].death>0)tick_death(&g->zombies[i],dt);}
static void kill_zombie(Game *g,Zombie *z){
    if(z->death<=0){if(z->kind==Z_GARG)cue(FOLEY_GARGANTUDEATH);else if(z->kind==Z_ZOMBONI)cue(FOLEY_EXPLOSION);z->vault_from=death_rate(g,z->kind);z->death=game_death_duration_for(g,z);z->age=0;z->eating=0;z->vault=0;g->polish.action[z-g->zombies]=0;g->kills++;completion_death_effect(g,z);
        if(!g->challenge&&g->wave>5&&((g->level==22&&!(g->store_flags&64))||(g->level==36&&!(g->store_flags&128)))){int value=g->level==22?-200:-201,exists=0;for(int k=0;k<MAX_COINS;k++)exists|=g->money[k].used&&g->money[k].value==value;if(!exists)game_add_coin(g,z->x,game_ground_y(g,z->row)-15,value);}
        if(z->kind==Z_YETI){for(int i=0;i<4;i++)game_add_coin(g,z->x-i*8,game_ground_y(g,z->row)-15,100);}
        else if(g->level>=11&&g->mode!=MODE_BOWLING){int factor=zombie_value[z->kind],hit=random_next(g)%30000;int present=(g->store_flags&STORE_GARDEN)&&g->extra.category!=EXTRA_IZOMBIE?(g->extra.category==EXTRA_SURVIVAL&&g->extra.difficulty==SURVIVAL_ENDLESS?3:12)*factor:0;int chocolate=(g->store_flags&STORE_CHOCOLATE)?(g->extra.category==EXTRA_SURVIVAL&&g->extra.difficulty==SURVIVAL_ENDLESS?9:36)*factor:0;if(g->wave+g->extra.stage*g->num_waves>70){present=chocolate=0;}if(g->level!=25||random_next(g)%4==0){if(hit<present)game_add_coin(g,z->x,game_ground_y(g,z->row)-15,-1-(int)(random_next(g)%40));else if(hit<present+chocolate)game_add_coin(g,z->x,game_ground_y(g,z->row)-15,-100);else if(hit<14*factor+present+chocolate)game_add_coin(g,z->x,game_ground_y(g,z->row)-15,g->seed_slots>6?100:5);else if(hit<250*factor)game_add_coin(g,z->x,game_ground_y(g,z->row)-15,5);else if(hit<2500*factor)game_add_coin(g,z->x,game_ground_y(g,z->row)-15,1);}}
}
}
int game_reward_seed(const Game *g){if(g->challenge||g->completion.adventure_run)return -1;for(int k=0;k<ALL_SEEDS;k++)if(seed_unlock[k]==g->level+1)return k;return -1;}
int game_reward_type(const Game *g){
    if(g->challenge)return REWARD_TROPHY;
    if(g->level%10==9)return REWARD_NOTE;
    if(g->completion.adventure_run)return REWARD_MONEY_BAG;
    switch(g->level){case 4:return REWARD_SHOVEL;case 14:return REWARD_ALMANAC;case 24:return REWARD_KEYS;case 34:return REWARD_TACO;case 44:return REWARD_WATERING_CAN;case 50:return REWARD_SILVER_SUNFLOWER;default:return REWARD_SEED;}
}
int game_collect_reward(Game *g){if(g->status!=WON||game_dying_count(g)||(g->level_flags&LEVEL_REWARD_COLLECTED))return 0;g->level_flags|=LEVEL_REWARD_COLLECTED;if(game_reward_type(g)==REWARD_MONEY_BAG){int value=g->level==50?300:25;g->money_collected+=value;}game_collect_coins(g);return 1;}

static int can_chill(const Game *g,const Zombie *z){int i=(int)(z-g->zombies);return z->death<=0&&!g->hypnotized[i]&&!bungee_away(g,z)&&z->kind!=Z_ZOMBONI&&!(z->kind==Z_BOBSLED&&g->extra.bobsled_team[i]&&g->extra.bobsled_phase[i]>=0)&&!(z->kind==Z_DIGGER&&(!z->vaulted||g->special[i]>0));}
static int can_freeze(const Game *g,const Zombie *z){int i=(int)(z-g->zombies);return can_chill(g,z)&&z->vault<=0&&!zombie_airborne(g,z)&&g->polish.action[i]!=ACTION_IMP_FLIGHT&&g->polish.action[i]!=ACTION_IMP_LAND&&g->polish.action[i]!=ACTION_JACK&&!(z->kind==Z_BOBSLED&&g->extra.bobsled_team[i]&&g->extra.bobsled_phase[i]<0)&&!bungee_away(g,z);}
static void hurt_zombie(Game *g,Zombie *z,float damage,int ice) {
    if(z->death>0||bungee_away(g,z))return;
    float before_hp=z->hp;
    if(damage>0){g->extra.hit_flash[z-g->zombies]=.09f;if(damage>=10&&damage<1800){combat_effect(g,z->x-8,game_ground_y(g,z->row)-18,0);cue(z->armor>0?(z->kind==Z_CONE?FOLEY_PLASTIC_HIT:FOLEY_SHIELD_HIT):FOLEY_SPLAT);}}
    float before_armor=z->armor;
    if(z->armor>0){float used=fminf(z->armor,damage);z->armor-=used;damage-=used;}
    if(before_armor>0&&z->armor==0&&z->kind==Z_PAPER){z->speed=.7f;z->age=0;g->polish.action[z-g->zombies]=ACTION_PAPER;g->polish.action_time[z-g->zombies]=0;cue(FOLEY_NEWSPAPER_RIP);}
    z->hp-=damage;if(ice&&can_chill(g,z)){z->slow=8;cue(FOLEY_SNOW_PEA_SPARKLES);}
    int maximum=game_zombie_max_health(g,z);if(before_hp>=maximum*2.0f/3&&z->hp<maximum*2.0f/3)cue(FOLEY_LIMBS_POP);
    if((z->kind<Z_DUCK||z->kind==Z_SNORKEL||z->kind==Z_DOLPHIN||z->kind==Z_JACK||z->kind==Z_BALLOON||z->kind==Z_DIGGER||z->kind==Z_POGO||z->kind==Z_YETI||z->kind==Z_LADDER||z->kind==Z_IMP)&&g->level!=25&&!game_mini_active(g,MINI_LITTLE)&&!g->mini.zombie_heads[z-g->zombies]&&before_hp>=maximum/3.0f&&z->hp<maximum/3.0f&&z->hp>0)head_effect(g,z);
    if(z->hp<=0)kill_zombie(g,z);
}

static void hurt_charred(Game *g,Zombie *z,float damage){int i=(int)(z-g->zombies),old=g->polish.death_style[i];if(z->kind<Z_DUCK||z->kind==Z_DUCK)g->polish.death_style[i]=DEATH_CHARRED;hurt_zombie(g,z,damage,0);if(z->death<=0)g->polish.death_style[i]=old;}
static void shoot(Game *g,int row,float x,int ice) {
    for(int i=0;i<MAX_SHOTS;i++)if(!g->shots[i].used){g->shots[i]=(Shot){1,row,ice,x};g->shot_remaining[i]=ice==2?CELL_W*3:1000;g->shot_vx[i]=150;g->shot_vy[i]=0;g->shot_y[i]=game_ground_y(g,row)+game_roof_offset(g,x)-17;g->shot_damage[i]=20;g->shot_target[i]=-1;g->extra.last_portal_shot[i]=0;sound_as(g,SFX_SHOOT,ice==2?FOLEY_PUFF:ice==3?FOLEY_FUME:FOLEY_THROW);return;}
}
int game_whack(Game *g,int row,int col){
    if(g->mode!=MODE_WHACK||g->status!=PLAYING)return 0;
    for(int i=0;i<MAX_ZOMBIES;i++){Zombie *z=&g->zombies[i];if(z->used&&z->death<=0&&z->row==row&&fabsf(z->x-plant_x(col))<29){
        if(z->armor>0){z->armor=fmaxf(0,z->armor-900);g->special[i]=.15f;}
        else{hurt_zombie(g,z,400,0);if(z->death>0)g->sun+=15;}
        return 1;
    }}return 0;
}
void game_start_minigame(Game *g,int kind,uint32_t seed){
    game_start_mini(g,kind==1?MINI_WHACK:MINI_BOWLING,seed);
}

static int target_in_lane(const Game *g,int row,float x) {
    for(int i=0;i<MAX_ZOMBIES;i++){
        const Zombie *z=&g->zombies[i];
        if(z->used&&!zombie_airborne(g,z)&&!(z->kind==Z_SNORKEL&&!z->eating)&&!(z->kind==Z_DIGGER&&!z->vaulted)&&!g->hypnotized[i]&&z->death<=0&&z->row==row&&z->x>=x-6&&z->x<480)return 1;
    }return 0;
}

static void conveyor_step(Game *g,float dt){
    if(g->mode==MODE_LAWN||g->mode==MODE_WHACK||g->mode==MODE_VASE||g->mode==MODE_MATCH||g->mode==MODE_AQUARIUM||g->mode==MODE_HEAVY||g->mode==MODE_SLOT)return;
    g->conveyor_timer-=dt;if(g->conveyor_timer>0)return;
    int count=g->packet_count;
    if(count>=MAX_PACKETS){g->conveyor_timer=0;return;}
    g->conveyor_timer=count>8?10:count>6?5:count>4?4.25f:4;
    if(game_mini_active(g,MINI_PORTAL))g->conveyor_timer*=1.5f;
    if(game_mini_active(g,MINI_INVIS))g->conveyor_timer*=2;
    if(game_mini_active(g,MINI_COLUMN))g->conveyor_timer*=3;
    if(g->mode==MODE_BOWLING){int pick=random_next(g)%(game_mini_active(g,MINI_BOWLING2)?115:100);g->packets[g->packet_count++]=pick<85?WALLNUT:pick<100?EXPLODE_NUT:GIANT_NUT;}
    else{
        static const int day[]={PEA,CHERRY,WALLNUT,REPEATER,SNOW,CHOMPER,MINE},dw[]={20,20,15,20,10,5,10};
        static const int night[]={GRAVEBUSTER,ICESHROOM,DOOM,HYPNO,SCAREDY,FUME,PUFF},nw[]={20,15,15,10,15,15,10};
        static const int pool[]={LILY,SQUASH,THREEPEATER,KELP,JALAPENO,SPIKEWEED,TORCHWOOD,TALLNUT},pw[]={25,5,25,5,10,10,10,10};
        static const int little[]={LILY,WALLNUT,PEA,CHERRY},lw[]={25,15,25,35};
        static const int storm[]={LILY,SEA,MAGNET,BLOVER,CACTUS,STAR,SPLIT,PUMPKIN},sw[]={25,10,5,5,15,25,5,10};
        static const int bungee[]={POT,CHOMPER,PUMPKIN,CHERRY},bw[]={50,25,15,10};
        static const int boss[]={POT,MELON,JALAPENO,CABBAGE,KERNEL,ICESHROOM},zw[]={55,10,12,10,5,8};
        const int *kinds=day,*base=dw;int n=7;
        if(g->level==20){kinds=night;base=nw;}
        if(g->level==25||game_mini_active(g,MINI_LITTLE)){kinds=little;base=lw;n=4;}
        if(g->level==30){kinds=pool;base=pw;n=8;}
        if(g->level==40){kinds=storm;base=sw;n=8;}
        if(g->mode==MODE_BUNGEE){kinds=bungee;base=bw;n=4;}
        if(g->mode==MODE_BOSS){kinds=boss;base=zw;n=6;g->conveyor_timer*=.875f;}
        static const int portal[]={PEA,REPEATER,TORCHWOOD,CACTUS,WALLNUT,CHERRY},ptw[]={25,20,10,15,15,15};
        static const int column[]={POT,MELON,CHOMPER,PUMPKIN,JALAPENO,SQUASH},clw[]={155,5,5,15,10,10};
        static const int invis[]={PEA,WALLNUT,KERNEL,SQUASH,LILY,ICESHROOM},ivw[]={25,15,5,15,30,10};
        if(game_mini_active(g,MINI_PORTAL)){kinds=portal;base=ptw;n=6;}
        if(game_mini_active(g,MINI_COLUMN)){kinds=column;base=clw;n=6;}
        if(game_mini_active(g,MINI_INVIS)){kinds=invis;base=ivw;n=6;}
        int weights[8],total=0;
        int graves=0,empty_pots=0;for(int r=0;r<game_rows(g);r++)for(int c=0;c<COLS;c++){graves+=!!g->graves[r][c];empty_pots+=g->support[r][c]==POT+1&&g->plants[r][c].kind<0&&g->pumpkin_hp[r][c]<=0;}
        for(int i=0;i<n;i++){
            int kind=kinds[i],bank=0,planted=0;for(int j=0;j<count;j++)bank+=g->packets[j]==kind;
            for(int r=0;r<game_rows(g);r++)for(int c=0;c<COLS;c++)planted+=kind==LILY||kind==POT?g->support[r][c]==kind+1:g->plants[r][c].kind==kind;
            int weight=base[i],all=bank+planted;
            if(kind==GRAVEBUSTER&&graves<=all){weights[i]=0;continue;}
            if(kind==LILY||kind==POT){int cap=kind==LILY?18:game_mini_active(g,MINI_COLUMN)?45:35;float t=fminf(1,all/(float)cap);weight=(int)(weight+(1-weight)*t);}
            if(g->mode==MODE_BOSS){if(kind==CABBAGE||kind==KERNEL||kind==MELON)weight/=empty_pots<=2?5:empty_pots<=5?3:1;if(kind==POT&&g->extra.boss_action==B_RV)weight=500;}
            weights[i]=bank>=4?1:bank>=3?5:kind==g->last_packet?weight/2:weight;total+=weights[i];
        }
        if(!total)return;
        int pick=random_next(g)%total,k=0;for(;k<n-1;k++){if(pick<weights[k])break;pick-=weights[k];}
        g->last_packet=kinds[k];g->packets[g->packet_count++]=kinds[k];
    }
}
static void bowling_step(Game *g,float dt){
    for(int i=0;i<MAX_NUTS;i++){
        BowlingNut *n=&g->nuts[i];if(!n->used)continue;
        float old_x=n->x,old_y=n->y;n->age+=dt;n->x+=140*dt;
        float target=game_ground_y(g,n->target_row),dy=target-n->y;
        if(fabsf(dy)>1)n->y+=fminf(fabsf(dy),72*dt)*(dy<0?-1:1);
        if(fabsf(target-n->y)<=1)n->row=n->target_row;
        if(n->last_hit>=0){Zombie *last=&g->zombies[n->last_hit];if(!last->used||fabsf(last->x-n->x)>37||fabsf(game_ground_y(g,last->row)-n->y)>30)n->last_hit=-1;}
        int hit=-1;float nearest=2;
        for(int j=0;j<MAX_ZOMBIES;j++){
            Zombie *z=&g->zombies[j];if(!z->used||z->death>0||j==n->last_hit)continue;
            // Sweep the same nut + zombie collision volume before and after a
            // bounce. Diagonal travel must not disable collision detection.
            float ax=(old_x-z->x)/(n->explosive==2?38:26),ay=(old_y-game_ground_y(g,z->row))/(n->explosive==2?30:20);
            float dx=(n->x-old_x)/(n->explosive==2?38:26),dy=(n->y-old_y)/(n->explosive==2?30:20),length=dx*dx+dy*dy;
            float t=length>0?fminf(1,fmaxf(0,-(ax*dx+ay*dy)/length)):0;
            float x=ax+t*dx,y=ay+t*dy;if(x*x+y*y<=1&&t<nearest){hit=j;nearest=t;}
        }
        if(hit>=0){
            Zombie *z=&g->zombies[hit];n->row=z->row;
            if(n->explosive==2){hurt_zombie(g,z,100000,0);n->last_hit=hit;sound_as(g,SFX_CHOMP,FOLEY_BOWLINGIMPACT);continue;}
            if(n->explosive==1){
                combat_effect(g,n->x,n->y-10,1);
                for(int k=0;k<MAX_ZOMBIES;k++){
                    Zombie *near=&g->zombies[k];if(near->used&&near->death<=0&&abs(near->row-n->row)<=1&&fabsf(near->x-n->x)<CELL_W*1.125f)hurt_zombie(g,near,1800,0);
                }n->used=0;g->flash=.25f;sound(g,SFX_EXPLODE);continue;
            }
            if(z->armor>0)z->armor=fmaxf(0,z->armor-900);else hurt_zombie(g,z,1800,0);
            sound_as(g,SFX_CHOMP,FOLEY_BOWLINGIMPACT);n->last_hit=hit;
            n->direction=n->row==0?1:n->row==4?-1:n->direction?-n->direction:(random_next(g)%2?1:-1);
            n->target_row=n->row+n->direction;
        }
        if(fabsf(game_ground_y(g,n->target_row)-n->y)<=1&&n->direction){
            if(n->row==0)n->direction=1;if(n->row==4)n->direction=-1;n->target_row=n->row+n->direction;
        }
        if(n->x>510)n->used=0;
    }
}
#include "chapter_game.inc"

static int choose_row(Game *g){
    if((g->store_flags&STORE_RAKE)&&g->rake_state==1&&!g->total_spawned)return g->rake_row;
    // Original smooth lane selection: normalize mower-protection weights, then
    // bias by the last two selections. Random sequences differ from the PS3.
    float weights[ROWS]={0},total=0;
    for(int r=0;r<ROWS;r++)if(game_row_active(g,r)){
        int since=g->wave-1-g->row_mowed_wave[r];weights[r]=since<=1?.01f:since<=2?.5f:1;total+=weights[r];
    }
    float adjusted=0;
    for(int r=0;r<ROWS;r++){
        float p=weights[r]/total;
        weights[r]=p*fminf(100,fmaxf(.01f,p*(1.5f*(g->row_history[r]+1)+.25f*(g->row_second[r]+1))-1));adjusted+=weights[r];
    }
    float pick=random_unit(g)*adjusted;int chosen=2;
    for(int r=0;r<ROWS;r++)if(weights[r]>0){chosen=r;pick-=weights[r];if(pick<=0)break;}
    for(int r=0;r<ROWS;r++)if(game_row_active(g,r)){g->row_history[r]++;g->row_second[r]++;}
    g->row_second[chosen]=g->row_history[chosen];g->row_history[chosen]=0;return chosen;
}
static void finish_wave_entries(Game *g){
    if(!g->polish.wave_pending)return;int index=g->wave-1;
    while(g->polish.wave_entry<g->wave_count[index]){
        int kind=g->wave_types[index][g->polish.wave_entry],free=0;for(int n=0;n<MAX_ZOMBIES;n++)free+=!g->zombies[n].used;
        if(free<(kind==Z_BOBSLED?4:1))return;
        int row=choose_row(g);
        int aquatic=kind==Z_DUCK||kind==Z_SNORKEL||kind==Z_DOLPHIN;
        if(game_rows(g)==6){if(aquatic)row=2+random_next(g)%2;else if(game_water_row(g,row))row=(int[]){0,1,4,5}[random_next(g)%4];}
        float x=459+random_unit(g)*30;
        if(g->mode==MODE_BUNGEE&&kind!=Z_BUNGEE)x=plant_x(4+random_next(g)%5);
        if(g->mode==MODE_WHACK){int graves[45],n=0;for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)if(g->graves[r][c])graves[n++]=r*COLS+c;
            if(n){int spot=graves[random_next(g)%n];row=spot/COLS;x=plant_x(spot%COLS);}kind=g->wave>6&&random_next(g)%4==0?Z_BUCKET:g->wave>3&&random_next(g)%3==0?Z_CONE:Z_NORMAL;
        }
        int id=g->completion.wave_giga[g->wave-1][g->polish.wave_entry]?game_spawn_giga(g,row,x):game_spawn_zombie(g,row,kind,x);
        if(id>=0){if(kind==Z_BOBSLED){g->extra.bobsled_team[id]=id+1;g->extra.bobsled_position[id]=0;for(int n=1;n<4;n++){int member=game_spawn_zombie(g,row,Z_BOBSLED,x+n*14);if(member>=0){g->extra.bobsled_team[member]=id+1;g->extra.bobsled_position[member]=n;g->polish.wave_health+=g->zombies[member].hp+g->zombies[member].armor;}}}if(g->mode==MODE_BUNGEE&&kind!=Z_BUNGEE)g->special[id]=-2;g->spawned++;g->polish.wave_health+=g->zombies[id].hp+g->zombies[id].armor;g->polish.wave_entry++;}

    }
    if(game_world(g)==1&&g->mode!=MODE_WHACK&&g->mode!=MODE_VASE&&g->wave==g->num_waves){
        while(g->polish.grave_entry<ROWS*COLS){int cell=g->polish.grave_entry,r=cell/COLS,c=cell%COLS;
            if(g->graves[r][c]){int id=game_spawn_zombie(g,r,Z_NORMAL,plant_x(c));if(id<0)return;g->polish.wave_health+=g->zombies[id].hp;}g->polish.grave_entry++;
        }
    }
    int next_flag=game_flag_wave(g,g->wave);
    g->polish.wave_fraction=next_flag&&g->mode!=MODE_BOWLING?0:(.5f+random_unit(g)*.15f);
    g->wave_health_threshold=g->polish.wave_fraction*g->polish.wave_health;
    g->spawn_timer=next_flag&&g->mode!=MODE_BOWLING?45:(2500+(random_next(g)%600))/100.0f;
    g->wave_delay=0;g->huge_wave_announced=0;g->polish.wave_pending=0;
}
static void spawn_wave(Game *g){
    if(g->polish.wave_pending){finish_wave_entries(g);return;}
    g->level_flags|=LEVEL_TUTORIAL_COMPLETE;if(g->spawned>0)g->wave++;
    g->spawned=0;g->polish.wave_pending=1;g->polish.wave_entry=0;g->polish.grave_entry=0;g->polish.wave_health=0;
    g->polish.wave_fraction=0;finish_wave_entries(g);
}

#include "minigames.inc"
#include "extra_rules.inc"
#include "fidelity_actors.inc"
#include "polish_rules0190.inc"
#include "fidelity_plants.inc"

void game_animate_attacks(Game *g,float dt){for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)if(g->plants[r][c].kind==SQUASH&&g->burst_left[r][c]>0){g->plants[r][c].age+=dt;chapter_plant(g,r,c,dt);}}
void game_step(Game *g,float dt) {
    if(g->status!=PLAYING||dt<=0||dt>.1f)return;
    if(game_mini_active(g,MINI_SPEED))dt*=2;
    g->time+=dt;g->flash=fmaxf(0,g->flash-dt);g->fog_clear=fmaxf(0,g->fog_clear-dt);
    game_animate_pickups(g,dt);
    store_step(g,dt);
    if(g->shovel_tutorial){for(int c=0;c<COLS;c++)if(g->plants[2][c].kind>=0)g->plants[2][c].age+=dt;return;}
    for(int s=0;s<ALL_SEEDS;s++){float *cooldown=game_seed_cooldown(g,s);*cooldown=fmaxf(0,*cooldown-dt);}
    effects_step(g,dt);
    if(extra_pre_step(g,dt)){for(int i=0;i<MAX_ZOMBIES;i++)if(g->zombies[i].used&&g->zombies[i].death>0)tick_death(&g->zombies[i],dt);return;}
    if(mini_pre_step(g,dt))return;
    conveyor_step(g,dt);
    if((g->mode==MODE_LAWN||g->mode==MODE_SLOT||g->mode==MODE_HEAVY)&&!game_is_night(g)&&!game_mini_active(g,MINI_LASTSTAND)){
        g->sky_timer-=dt;
        if(g->sky_timer<=0){
            g->sky_count++;g->sky_timer=fminf(9.5f,4.25f+g->sky_count*.1f)+random_unit(g)*2.75f;
            add_sun(g,GRID_X+20+random_unit(g)*260,25,90+random_unit(g)*135);
        }
    }
    for(int i=0;i<MAX_SUNS;i++)if(g->suns[i].used==1){
        Sun *s=&g->suns[i];s->flight+=dt;
        if(s->y<s->target_y||s->vy<0){
            s->y=fminf(s->target_y,s->y+s->vy*dt);s->x+=s->vx*dt;
            if(s->from_plant)s->vy+=324*dt;
        }else{s->age+=dt;if(s->age>7.5f)s->used=0;}
    }
    for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++){
        g->craters[r][c]=fmaxf(0,g->craters[r][c]-dt);g->ice_tiles[r][c]=fmaxf(0,g->ice_tiles[r][c]-dt);
        if(g->morph_timer[r][c]>0){g->morph_timer[r][c]=fmaxf(0,g->morph_timer[r][c]-dt);if(g->morph_timer[r][c]==0){int copied=g->morph_kind[r][c];put_plant(g,r,c,copied);g->imitated[r][c]|=copied==COFFEE?0:copied==LILY||copied==POT?2:copied==PUMPKIN?4:1;}}
        g->fidelity.support_safe[r][c]=fmaxf(0,g->fidelity.support_safe[r][c]-dt);
        if(g->fidelity.wake[r][c]>0){g->fidelity.wake[r][c]=fmaxf(0,g->fidelity.wake[r][c]-dt);if(!g->fidelity.wake[r][c]){g->asleep[r][c]=0;cue(FOLEY_WAKEUP);}}
        Plant *p=&g->plants[r][c];if(p->kind<0)continue;
        p->age+=dt;if(g->asleep[r][c])continue;p->flash=fmaxf(0,p->flash-dt);p->attack-=dt;p->digest=fmaxf(0,p->digest-dt);
        float x=plant_x(c);
        if(fidelity_plant_step(g,r,c,dt))continue;
        chapter_plant(g,r,c,dt);if(p->kind<0)continue;
        if(p->kind==SUNSHROOM&&p->attack<=0&&!(p->age>=120&&p->age<120+11/12.0f)){
            p->attack=23.5f+random_unit(g)*1.5f;p->flash=1;int sun=plant_sun(g,x,game_ground_y(g,r)-16);if(sun>=0)g->suns[sun].from_plant=p->age<120?15:25;sound_as(g,SFX_SHOOT,FOLEY_SPAWN_SUN);
        }
        if(p->kind==GRAVEBUSTER&&p->age>=4.5f){g->graves[r][c]=0;p->kind=-1;cue(FOLEY_GRAVESTONE_RUMBLE);continue;}
        if((p->kind==ICESHROOM||p->kind==DOOM)&&p->age>=1){
            for(int z=0;z<MAX_ZOMBIES;z++)if(g->zombies[z].used&&!g->hypnotized[z]&&g->zombies[z].death<=0){
                if(p->kind==ICESHROOM){if(can_freeze(g,&g->zombies[z]))g->frozen[z]=4;if(can_chill(g,&g->zombies[z]))g->zombies[z].slow=15;}
                else if(abs(g->zombies[z].row-r)<=2&&fabsf(g->zombies[z].x-x)<CELL_W*3.2f)hurt_charred(g,&g->zombies[z],1800);
            }
            if(p->kind==DOOM){g->craters[r][c]=180;g->flash=.4f;sound_as(g,SFX_EXPLODE,GAME_SAMPLE(FX_DOOMSHROOM));}else cue(FOLEY_FROZEN);p->kind=-1;continue;
        }
        if((p->kind==PUFF||p->kind==FUME||p->kind==SCAREDY)&&p->attack<=0){
            int target=0,scared=0;
            for(int z=0;z<MAX_ZOMBIES;z++){Zombie *enemy=&g->zombies[z];if(!enemy->used||enemy->death>0||g->hypnotized[z])continue;
                if(abs(enemy->row-r)<=1&&fabsf(enemy->x-x)<CELL_W*1.5f)scared=1;
                if(enemy->row==r&&enemy->x>x-6&&enemy->x<480&&(p->kind==SCAREDY||enemy->x-x<CELL_W*(p->kind==FUME?4:3)))target=1;
            }
            if(target&&!(p->kind==SCAREDY&&scared)){
                p->attack=1.4f;p->flash=.3f;
                if(p->kind==FUME){for(int z=0;z<MAX_ZOMBIES;z++){Zombie *enemy=&g->zombies[z];if(enemy->used&&!g->hypnotized[z]&&enemy->death<=0&&enemy->row==r&&enemy->x>x-6&&enemy->x-x<CELL_W*4){
                    float shield=enemy->armor;if(enemy->kind==Z_DOOR||enemy->kind==Z_PAPER)enemy->armor=0;hurt_zombie(g,enemy,20,0);if(enemy->kind==Z_DOOR||enemy->kind==Z_PAPER)enemy->armor=shield;
                }}sound(g,SFX_SHOOT);}else shoot(g,r,x+8,p->kind==PUFF?2:3);
            }
            if(p->kind==SCAREDY&&scared)p->digest=.2f;
        }
        if(p->kind==SUNFLOWER&&g->extra.category!=EXTRA_IZOMBIE&&p->attack<=0){p->attack=23.5f+random_unit(g)*1.5f;p->flash=1;plant_sun(g,x,game_ground_y(g,r)-22);sound_as(g,SFX_SHOOT,FOLEY_SPAWN_SUN);}
        if(p->kind==PEA||p->kind==SNOW||p->kind==REPEATER){
            float *clip=&g->pea_shoot_time[r][c],rate=p->kind==REPEATER?45:35;
            if(*clip>0){float old=*clip;*clip+=dt;float launch=p->kind==REPEATER?.25f:.34f;
                if(old<launch&&*clip>=launch)extra_shoot(g,r,c,x+13,p->kind==SNOW);
                if(p->kind==REPEATER&&old<launch+.18f&&*clip>=launch+.18f)extra_shoot(g,r,c,x+13,0);
                float end=24/rate;if(old<end&&*clip>=end)g->burst[r][c]=0;
                if(*clip>=end)g->burst[r][c]+=dt;
                if(*clip>=end+.12f)*clip=0;
            }else g->burst[r][c]+=dt;
            if(p->attack<=0&&extra_target(g,r,c,x)&&*clip==0){p->attack=1.4f+random_unit(g)*.1f;p->flash=.30f;*clip=.0001f;}
        }
        if(p->kind==CHERRY&&p->age>=1.2f){
            for(int i=0;i<MAX_ZOMBIES;i++){
                Zombie *z=&g->zombies[i];
                if(z->used&&!g->hypnotized[i]&&z->death<=0&&abs(z->row-r)<=1&&fabsf(z->x-x)<CELL_W*1.65f)hurt_charred(g,z,1800);
            }p->kind=-1;g->flash=.32f;sound(g,SFX_EXPLODE);cue(FOLEY_JUICY);
        }
        if(p->kind==MINE&&p->age>=MINE_ARM_SECONDS){
            for(int i=0;i<MAX_ZOMBIES;i++){
                Zombie *z=&g->zombies[i];
                // Trigger at the same contact distance used by the bite logic.
                // Otherwise the zombie stops outside the old, smaller trigger radius.
                if(z->used&&!g->hypnotized[i]&&z->death<=0&&z->vault<=0&&!zombie_airborne(g,z)&&z->kind!=Z_BUNGEE&&!(z->kind==Z_DIGGER&&!z->vaulted)&&z->row==r&&bite_contact(z->x,x)){
                    for(int j=0;j<MAX_ZOMBIES;j++){
                        Zombie *nearby=&g->zombies[j];
                        if(nearby->used&&!g->hypnotized[j]&&nearby->death<=0&&!zombie_airborne(g,nearby)&&nearby->kind!=Z_BUNGEE&&nearby->vault<=0&&nearby->row==r&&fabsf(nearby->x-x)<=32)
                            hurt_zombie(g,nearby,1800,0);
                    }
                    p->kind=-1;g->flash=.16f;sound_as(g,SFX_EXPLODE,GAME_SAMPLE(FX_POTATO_MINE));break;
                }
            }
        }

    }
    chapter_shots(g,dt);
    bowling_step(g,dt);
    for(int i=0;i<MAX_ZOMBIES;i++){
        Zombie *z=&g->zombies[i];if(!z->used)continue;
        if(z->death>0){tick_death(z,dt);continue;}
        if(g->mode==MODE_WHACK)g->special[i]=fmaxf(0,g->special[i]-dt);
        if(g->frozen[i]>0){g->frozen[i]=fmaxf(0,g->frozen[i]-dt);continue;}
        z->slow=fmaxf(0,z->slow-dt);g->polish.pose[i]+=dt*(z->slow>0?.5f:1);
        int detached=!game_zombie_has_head(g,z);if(detached){z->hp-=dt*(z->kind==Z_POLE?60:20);if(z->hp<=0){kill_zombie(g,z);continue;}}
        if(completion_yeti_step(g,i,dt))continue;
        if(fidelity_actor_step(g,i,dt))continue;
        if(polish_actor_step(g,i,dt))continue;
        if(extra_iz_zombie(g,i,dt))continue;
        if(bobsled_step(g,i,dt))continue;
        if(chapter_zombie(g,i,dt))continue;
        if(mini_zombie(g,i,dt))continue;
        if(g->hypnotized[i]){
            int target=-1;for(int j=0;j<MAX_ZOMBIES;j++)if(g->zombies[j].used&&g->zombies[j].death<=0&&!g->hypnotized[j]&&g->zombies[j].row==z->row&&fabsf(g->zombies[j].x-z->x)<25){target=j;break;}
            if(target>=0&&!detached){hurt_zombie(g,&g->zombies[target],100*dt,0);hurt_zombie(g,z,100*dt,0);if(!z->eating){z->age=0;g->polish.pose[i]=0;}z->eating=1;z->age+=dt*(z->slow>0?.5f:1);}
            else{if(z->eating){z->age=0;g->polish.pose[i]=0;}z->eating=0;z->x+=game_zombie_walk(z,dt);if(z->x>520)z->used=0;}continue;
        }
        if(z->kind==Z_DANCER&&game_zombie_has_head(g,z)){g->special[i]-=dt;if(g->special[i]<=0){g->special[i]=12;
            for(int slot=0;slot<4;slot++){
                int row=z->row+(slot==0?-1:slot==1?1:0);float x=z->x+(slot==2?-38:slot==3?38:0);if(row<0||row>=ROWS)continue;
                int backup=0;for(int j=0;j<MAX_ZOMBIES;j++)backup+=g->zombies[j].used&&g->zombies[j].death<=0&&g->zombies[j].kind==Z_BACKUP&&g->zombies[j].row==row&&fabsf(g->zombies[j].x-x)<22;
                if(!backup)game_spawn_zombie(g,row,Z_BACKUP,x);
            }
        }}
        int was_eating=z->eating;z->eating=0;
        int headless=!game_zombie_has_head(g,z);
        if(headless){
            if(z->kind==Z_POLE&&!z->vaulted){z->vaulted=1;z->vault=0;z->age=0;z->speed=.23f+random_unit(g)*.14f;}
            /* All detached heads use the shared decay above. */
            if(z->hp<=0){kill_zombie(g,z);continue;}
        }
        if((z->kind==Z_POLE||z->kind==Z_DOLPHIN||z->kind==Z_POGO)&&z->vault>0){
            z->age+=dt;
            z->vault=fmaxf(0,z->vault-dt);z->x=z->vault_from+(z->vault_to-z->vault_from)*(1-z->vault/game_vault_duration(z));
            if(z->vault==0){z->vaulted=1;z->age=0;z->speed=.23f+random_unit(g)*.14f;}continue;
        }
        for(int c=COLS-1;c>=0&&!headless;c--){
            Plant *p=&g->plants[z->row][c];if(p->kind==SQUASH&&g->burst_left[z->row][c]>=3)continue;int plant_col=c;float px=plant_x(c);if(p->kind==-2&&c>0&&g->plants[z->row][c-1].kind==COB){p=&g->plants[z->row][c-1];plant_col=c-1;}
            if((z->kind==Z_POLE||z->kind==Z_DOLPHIN||z->kind==Z_POGO)&&!z->vaulted&&p->kind>=0&&p->kind!=TALLNUT&&z->x>=px&&z->x<=px+42){
                z->vault=game_vault_duration(z);z->vault_from=z->x;z->vault_to=px-28;z->age=0;z->eating=1;cue(z->kind==Z_DOLPHIN?FOLEY_DOLPHIN_BEFORE_JUMPING:z->kind==Z_POGO?FOLEY_POGO_ZOMBIE:FOLEY_POLEVAULT);break;
            }
            if((p->kind>=0||g->support[z->row][c]||g->pumpkin_hp[z->row][c]>0||g->morph_timer[z->row][c]>0)&&(z->kind==Z_GARG||(p->kind!=SPIKEWEED&&p->kind!=SPIKEROCK))&&bite_contact(z->x,px)){
                if(g->ladders[z->row][c])continue;
                if(z->kind==Z_SNORKEL&&game_water_row(g,z->row)&&!g->fidelity.actor_aux[i]){g->polish.action[i]=ACTION_SNORKEL_RISE;g->polish.action_time[i]=0;g->fidelity.actor_target[i]=z->row*COLS+c;z->eating=1;break;}
                if(z->kind==Z_ZOMBONI||z->kind==Z_CATAPULT){destroy_square(g,z->row,plant_col,1);cue(FOLEY_SQUISH);continue;}
                if(z->kind==Z_LADDER&&z->armor>0&&(p->kind==WALLNUT||p->kind==TALLNUT||g->pumpkin_hp[z->row][c]>0)){g->polish.action[i]=ACTION_LADDER;g->polish.action_time[i]=0;g->polish.target[i]=z->row*COLS+c;z->eating=1;break;}
                if(z->kind==Z_GARG){float old=g->special[i];g->special[i]+=dt*(z->slow>0?.5f:1);z->eating=1;
                    if(old<.64f*(33/16.0f)&&g->special[i]>=.64f*(33/16.0f)){cue(FOLEY_THUMP);
                        if(p->kind==SPIKEROCK){p->hp-=50;hurt_zombie(g,z,20,0);if(p->hp<=0)p->kind=-1;}
                        else destroy_square(g,z->row,plant_col,1);
                    }if(g->special[i]>=33/16.0f)g->special[i]-=33/16.0f;break;}
                float damage=100*dt*(z->slow>0?.5f:1);if(z->kind==Z_GARG||z->kind==Z_ZOMBONI)damage=10000*dt;
                if(g->pumpkin_hp[z->row][c]>0)g->pumpkin_hp[z->row][c]=fmaxf(0,g->pumpkin_hp[z->row][c]-damage);
                else if(g->morph_timer[z->row][c]>0){g->morph_hp[z->row][c]-=damage;if(g->morph_hp[z->row][c]<=0)g->morph_timer[z->row][c]=0;}
                else if(p->kind>=0){extra_plant_damage(g,z->row,plant_col,p->hp,p->hp-damage);p->hp-=damage;}else{if(g->fidelity.support_safe[z->row][c]>0)damage=0;g->support_hp[z->row][c]-=damage;if(g->support_hp[z->row][c]<=0)g->support[z->row][c]=0;}z->eating=1;
                if(p->kind==GARLIC&&!g->polish.action[i]){int rows[2],count=0;for(int row=z->row-1;row<=z->row+1;row+=2)if(game_row_active(g,row)&&!game_water_row(g,row))rows[count++]=row;
                    int target=count?rows[random_next(g)%count]:z->row;g->polish.target[i]=z->row*ROWS+target;g->polish.action[i]=ACTION_GARLIC;g->polish.action_time[i]=0;p->hp-=20;}
                if(p->hp<=0){if(p->kind==COB&&plant_col+1<COLS)g->plants[z->row][plant_col+1].kind=-1;if(p->kind==HYPNO){g->hypnotized[i]=1;z->age=0;cue(GAME_SAMPLE(FX_MINDCONTROLLED));}p->kind=-1;}
                break;
            }
        }
        if(z->kind==Z_GARG&&!z->eating)g->special[i]=0;
        if(z->eating!=was_eating&&z->vault<=0){z->age=0;g->polish.pose[i]=0;}
        if(z->kind==Z_SNORKEL&&!z->eating&&g->fidelity.actor_aux[i]>0){g->polish.action[i]=ACTION_SNORKEL_DIVE;g->polish.action_time[i]=0;continue;}
        if(!z->eating)z->x-=game_zombie_walk(z,dt);
        else if(z->vault<=0)z->age+=dt*(z->slow>0?.5f:1);
        if(z->x<GRID_X-13&&!g->mower_used[z->row]&&game_cleaner_active(g,z->row)){g->mower_used[z->row]=1;g->mower_x[z->row]=GRID_X-24;g->row_mowed_wave[z->row]=g->wave-1;sound_as(g,SFX_MOWER,game_water_row(g,z->row)?FOLEY_POOL_CLEANER:FOLEY_LAWNMOWER);}
        if(z->kind==Z_POGO&&z->vaulted&&!g->fidelity.equipment_lost[i])z->vaulted=0;
        if(z->x<12){if(headless)kill_zombie(g,z);else{g->status=LOST;cue(FOLEY_SCREAM);cue(GAME_SAMPLE(FX_LOSEMUSIC));return;}}
    }
    for(int r=0;r<ROWS;r++)if(game_cleaner_active(g,r)&&g->mower_used[r]&&g->mower_x[r]<520){
        g->mower_x[r]+=210*dt;
        if(game_water_row(g,r)&&!(g->store_flags&STORE_POOL)&&g->mower_x[r]>GRID_X){g->mower_x[r]=600;continue;}
        for(int i=0;i<MAX_ZOMBIES;i++){
            Zombie *z=&g->zombies[i];
            if(z->used&&!g->hypnotized[i]&&z->death<=0&&!zombie_airborne(g,z)&&!(z->kind==Z_DIGGER&&!z->vaulted)&&z->kind!=Z_BUNGEE&&z->row==(g->extra.mower_row[r]?g->extra.mower_row[r]-1:r)&&fabsf(z->x-g->mower_x[r])<24)kill_zombie(g,z);
        }
    }
    if(g->polish.wave_pending){finish_wave_entries(g);return;}
    if(extra_progress(g))return;
    if(mini_progress(g))return;
    if(chapter_progress(g,dt))return;
    if(g->wave==g->num_waves&&g->spawned>0&&game_alive(g)==0){
        if(g->mode!=MODE_MATCH&&g->mode!=MODE_SLOT&&!game_mini_active(g,MINI_STARS)){g->status=WON;sound(g,SFX_WIN);return;}
    }
    if(g->wave==g->num_waves&&g->spawned>0)return;
    if(g->huge_wave_timer>0){g->huge_wave_timer=fmaxf(0,g->huge_wave_timer-dt);if(g->huge_wave_timer==0)spawn_wave(g);return;}
    g->spawn_timer-=dt;
    if(g->spawned>0){
        g->wave_delay+=dt;float remaining=0;
        for(int i=0;i<MAX_ZOMBIES;i++){
            Zombie *z=&g->zombies[i];
            if(z->used&&z->death<=0&&!g->hypnotized[i]&&z->wave==g->wave)remaining+=z->hp+z->armor;
        }
        if(g->wave_delay>4&&remaining<=g->wave_health_threshold&&g->spawn_timer>2)g->spawn_timer=2;
    }
    int next_index=g->spawned>0?g->wave:g->wave-1;
    if(g->spawn_timer<=.05f&&!g->huge_wave_announced&&game_flag_wave(g,next_index)){
        g->huge_wave_announced=1;g->huge_wave_timer=7.5f;return;
    }
    if(g->spawn_timer<=0)spawn_wave(g);
}

static uint32_t checksum(const void *memory,size_t count) {
    const unsigned char *p=memory;uint32_t value=2166136261u;
    for(size_t i=0;i<count;i++){value^=p[i];value*=16777619u;}return value;
}
int game_save(const Game *g,const char *filename) {
    uint32_t header[4]={0x31535650,13,sizeof(*g),checksum(g,sizeof(*g))};
    return save_file_replace(filename,header,g);
}
static void legacy_import(Game *g,const LegacyGame *old){
 game_start_level(g,old->level,old->rng);
 g->rng=old->rng;
 g->status=old->status;
 g->sun=old->sun;
 g->wave=old->wave;
 g->spawned=old->spawned;
 g->kills=old->kills;
 g->total_spawned=old->total_spawned;
 g->selected=old->selected;
 g->shovel=old->shovel;
 g->cursor_row=old->cursor_row;
 g->cursor_col=old->cursor_col;
 memcpy(g->mower_used,old->mower_used,sizeof(old->mower_used));
 memcpy(g->events,old->events,sizeof(old->events));
 g->time=old->time;
 g->spawn_timer=old->spawn_timer;
 g->sky_timer=old->sky_timer;
 g->wave_delay=old->wave_delay;
 g->flash=old->flash;
 memcpy(g->cooldown,old->cooldown,sizeof(old->cooldown));
 memcpy(g->mower_x,old->mower_x,sizeof(old->mower_x));
 memcpy(g->seed_cost,old->seed_cost,sizeof(old->seed_cost));
 memcpy(g->seed_refresh,old->seed_refresh,sizeof(old->seed_refresh));
 memcpy(g->plants,old->plants,sizeof(old->plants));
 memcpy(g->zombies,old->zombies,sizeof(old->zombies));
 memcpy(g->shots,old->shots,sizeof(old->shots));
 memcpy(g->suns,old->suns,sizeof(old->suns));
 g->wave_health_threshold=old->wave_health_threshold;
 g->level=old->level;
 g->num_waves=old->num_waves;
 g->row_mask=old->row_mask;
 g->unlocked_mask=old->unlocked_mask;
 g->seed_mask=old->seed_mask;
 g->mode=old->mode;
 memcpy(g->wave_count,old->wave_count,sizeof(old->wave_count));
 memcpy(g->wave_types,old->wave_types,sizeof(old->wave_types));
 memcpy(g->row_history,old->row_history,sizeof(old->row_history));
 memcpy(g->row_second,old->row_second,sizeof(old->row_second));
 memcpy(g->row_mowed_wave,old->row_mowed_wave,sizeof(old->row_mowed_wave));
 memcpy(g->packets,old->packets,sizeof(old->packets));
 g->packet_count=old->packet_count;
 g->last_packet=old->last_packet;
 g->sky_count=old->sky_count;
 g->conveyor_timer=old->conveyor_timer;
 g->huge_wave_timer=old->huge_wave_timer;
 g->huge_wave_announced=old->huge_wave_announced;
 memcpy(g->nuts,old->nuts,sizeof(old->nuts));
 g->shovel_tutorial=old->shovel_tutorial;
 g->level_flags=old->level_flags;
 memcpy(g->night_cooldown,old->night_cooldown,sizeof(old->night_cooldown));
 memcpy(g->night_cost,old->night_cost,sizeof(old->night_cost));
 memcpy(g->night_refresh,old->night_refresh,sizeof(old->night_refresh));
 memcpy(g->graves,old->graves,sizeof(old->graves));
 memcpy(g->hypnotized,old->hypnotized,sizeof(old->hypnotized));
 memcpy(g->craters,old->craters,sizeof(old->craters));
 memcpy(g->frozen,old->frozen,sizeof(old->frozen));
 memcpy(g->special,old->special,sizeof(old->special));
 g->challenge=old->challenge;
 g->whack_goal=old->whack_goal;
 memcpy(g->shot_remaining,old->shot_remaining,sizeof(old->shot_remaining));
 for(int i=0;i<MAX_SHOTS;i++){g->shot_vx[i]=150;g->shot_y[i]=game_ground_y(g,g->shots[i].row)-17;g->shot_damage[i]=20;g->shot_target[i]=-1;}
}
static void legacy_export(LegacyGame *old,const Game *g){
 memset(old,0,sizeof(*old));
 old->rng=g->rng;
 old->status=g->status;
 old->sun=g->sun;
 old->wave=g->wave;
 old->spawned=g->spawned;
 old->kills=g->kills;
 old->total_spawned=g->total_spawned;
 old->selected=g->selected;
 old->shovel=g->shovel;
 old->cursor_row=g->cursor_row;
 old->cursor_col=g->cursor_col;
 memcpy(old->mower_used,g->mower_used,sizeof(old->mower_used));
 memcpy(old->events,g->events,sizeof(old->events));
 old->time=g->time;
 old->spawn_timer=g->spawn_timer;
 old->sky_timer=g->sky_timer;
 old->wave_delay=g->wave_delay;
 old->flash=g->flash;
 memcpy(old->cooldown,g->cooldown,sizeof(old->cooldown));
 memcpy(old->mower_x,g->mower_x,sizeof(old->mower_x));
 memcpy(old->seed_cost,g->seed_cost,sizeof(old->seed_cost));
 memcpy(old->seed_refresh,g->seed_refresh,sizeof(old->seed_refresh));
 memcpy(old->plants,g->plants,sizeof(old->plants));
 memcpy(old->zombies,g->zombies,sizeof(old->zombies));
 memcpy(old->shots,g->shots,sizeof(old->shots));
 memcpy(old->suns,g->suns,sizeof(old->suns));
 old->wave_health_threshold=g->wave_health_threshold;
 old->level=g->level;
 old->num_waves=g->num_waves;
 old->row_mask=g->row_mask;
 old->unlocked_mask=g->unlocked_mask;
 old->seed_mask=g->seed_mask;
 old->mode=g->mode;
 memcpy(old->wave_count,g->wave_count,sizeof(old->wave_count));
 memcpy(old->wave_types,g->wave_types,sizeof(old->wave_types));
 memcpy(old->row_history,g->row_history,sizeof(old->row_history));
 memcpy(old->row_second,g->row_second,sizeof(old->row_second));
 memcpy(old->row_mowed_wave,g->row_mowed_wave,sizeof(old->row_mowed_wave));
 memcpy(old->packets,g->packets,sizeof(old->packets));
 old->packet_count=g->packet_count;
 old->last_packet=g->last_packet;
 old->sky_count=g->sky_count;
 old->conveyor_timer=g->conveyor_timer;
 old->huge_wave_timer=g->huge_wave_timer;
 old->huge_wave_announced=g->huge_wave_announced;
 memcpy(old->nuts,g->nuts,sizeof(old->nuts));
 old->shovel_tutorial=g->shovel_tutorial;
 old->level_flags=g->level_flags;
 memcpy(old->night_cooldown,g->night_cooldown,sizeof(old->night_cooldown));
 memcpy(old->night_cost,g->night_cost,sizeof(old->night_cost));
 memcpy(old->night_refresh,g->night_refresh,sizeof(old->night_refresh));
 memcpy(old->graves,g->graves,sizeof(old->graves));
 memcpy(old->hypnotized,g->hypnotized,sizeof(old->hypnotized));
 memcpy(old->craters,g->craters,sizeof(old->craters));
 memcpy(old->frozen,g->frozen,sizeof(old->frozen));
 memcpy(old->special,g->special,sizeof(old->special));
 old->challenge=g->challenge;
 old->whack_goal=g->whack_goal;
 memcpy(old->shot_remaining,g->shot_remaining,sizeof(old->shot_remaining));
}
static void legacy071_import(Game *g,const Legacy071 *old){
 game_start_level(g,old->level,old->rng);
 g->rng=old->rng;
 g->status=old->status;
 g->sun=old->sun;
 g->wave=old->wave;
 g->spawned=old->spawned;
 g->kills=old->kills;
 g->total_spawned=old->total_spawned;
 g->selected=old->selected;
 g->shovel=old->shovel;
 g->cursor_row=old->cursor_row;
 g->cursor_col=old->cursor_col;
 memcpy(g->mower_used,old->mower_used,sizeof(old->mower_used));
 memcpy(g->events,old->events,sizeof(old->events));
 g->time=old->time;
 g->spawn_timer=old->spawn_timer;
 g->sky_timer=old->sky_timer;
 g->wave_delay=old->wave_delay;
 g->flash=old->flash;
 memcpy(g->cooldown,old->cooldown,sizeof(old->cooldown));
 memcpy(g->mower_x,old->mower_x,sizeof(old->mower_x));
 memcpy(g->seed_cost,old->seed_cost,sizeof(old->seed_cost));
 memcpy(g->seed_refresh,old->seed_refresh,sizeof(old->seed_refresh));
 memcpy(g->plants,old->plants,sizeof(old->plants));
 memcpy(g->zombies,old->zombies,sizeof(old->zombies));
 memcpy(g->shots,old->shots,sizeof(old->shots));
 memcpy(g->suns,old->suns,sizeof(old->suns));
 g->wave_health_threshold=old->wave_health_threshold;
 g->level=old->level;
 g->num_waves=old->num_waves;
 g->row_mask=old->row_mask;
 g->unlocked_mask=old->unlocked_mask;
 g->seed_mask=old->seed_mask;
 g->mode=old->mode;
 memcpy(g->wave_count,old->wave_count,sizeof(old->wave_count));
 memcpy(g->wave_types,old->wave_types,sizeof(old->wave_types));
 memcpy(g->row_history,old->row_history,sizeof(old->row_history));
 memcpy(g->row_second,old->row_second,sizeof(old->row_second));
 memcpy(g->row_mowed_wave,old->row_mowed_wave,sizeof(old->row_mowed_wave));
 memcpy(g->packets,old->packets,sizeof(old->packets));
 g->packet_count=old->packet_count;
 g->last_packet=old->last_packet;
 g->sky_count=old->sky_count;
 g->conveyor_timer=old->conveyor_timer;
 g->huge_wave_timer=old->huge_wave_timer;
 g->huge_wave_announced=old->huge_wave_announced;
 memcpy(g->nuts,old->nuts,sizeof(old->nuts));
 g->shovel_tutorial=old->shovel_tutorial;
 g->level_flags=old->level_flags;
 memcpy(g->night_cooldown,old->night_cooldown,sizeof(old->night_cooldown));
 memcpy(g->night_cost,old->night_cost,sizeof(old->night_cost));
 memcpy(g->night_refresh,old->night_refresh,sizeof(old->night_refresh));
 memcpy(g->graves,old->graves,sizeof(old->graves));
 memcpy(g->hypnotized,old->hypnotized,sizeof(old->hypnotized));
 memcpy(g->craters,old->craters,sizeof(old->craters));
 memcpy(g->frozen,old->frozen,sizeof(old->frozen));
 memcpy(g->special,old->special,sizeof(old->special));
 g->challenge=old->challenge;
 g->whack_goal=old->whack_goal;
 memcpy(g->shot_remaining,old->shot_remaining,sizeof(old->shot_remaining));
 memcpy(g->shot_y,old->shot_y,sizeof(old->shot_y));
 memcpy(g->shot_vx,old->shot_vx,sizeof(old->shot_vx));
 memcpy(g->shot_vy,old->shot_vy,sizeof(old->shot_vy));
 memcpy(g->shot_damage,old->shot_damage,sizeof(old->shot_damage));
 memcpy(g->shot_target,old->shot_target,sizeof(old->shot_target));
 memcpy(g->support,old->support,sizeof(old->support));
 memcpy(g->vases,old->vases,sizeof(old->vases));
 memcpy(g->asleep,old->asleep,sizeof(old->asleep));
 memcpy(g->ladders,old->ladders,sizeof(old->ladders));
 memcpy(g->support_hp,old->support_hp,sizeof(old->support_hp));
 memcpy(g->pumpkin_hp,old->pumpkin_hp,sizeof(old->pumpkin_hp));
 memcpy(g->ice_tiles,old->ice_tiles,sizeof(old->ice_tiles));
 g->fog_clear=old->fog_clear;
 g->boss_hp=old->boss_hp;
 g->boss_timer=old->boss_timer;
 g->boss_phase=old->boss_phase;
 g->boss_ball_x=old->boss_ball_x;
 g->boss_row=old->boss_row;
 g->boss_ball_kind=old->boss_ball_kind;
 g->stage=old->stage;
 g->coins=old->coins;
 g->boss_ball_row=old->boss_ball_row;
}
static void legacy08_import(Game *g,const Legacy08 *old){
 game_start_level(g,old->level,old->rng);
 g->rng=old->rng;
 g->status=old->status;
 g->sun=old->sun;
 g->wave=old->wave;
 g->spawned=old->spawned;
 g->kills=old->kills;
 g->total_spawned=old->total_spawned;
 g->selected=old->selected;
 g->shovel=old->shovel;
 g->cursor_row=old->cursor_row;
 g->cursor_col=old->cursor_col;
 memcpy(g->mower_used,old->mower_used,sizeof(old->mower_used));
 memcpy(g->events,old->events,sizeof(old->events));
 g->time=old->time;
 g->spawn_timer=old->spawn_timer;
 g->sky_timer=old->sky_timer;
 g->wave_delay=old->wave_delay;
 g->flash=old->flash;
 memcpy(g->cooldown,old->cooldown,sizeof(old->cooldown));
 memcpy(g->mower_x,old->mower_x,sizeof(old->mower_x));
 memcpy(g->seed_cost,old->seed_cost,sizeof(old->seed_cost));
 memcpy(g->seed_refresh,old->seed_refresh,sizeof(old->seed_refresh));
 memcpy(g->plants,old->plants,sizeof(old->plants));
 memcpy(g->zombies,old->zombies,sizeof(old->zombies));
 memcpy(g->shots,old->shots,sizeof(old->shots));
 memcpy(g->suns,old->suns,sizeof(old->suns));
 g->wave_health_threshold=old->wave_health_threshold;
 g->level=old->level;
 g->num_waves=old->num_waves;
 g->row_mask=old->row_mask;
 g->unlocked_mask=old->unlocked_mask;
 g->seed_mask=old->seed_mask;
 g->mode=old->mode;
 memcpy(g->wave_count,old->wave_count,sizeof(old->wave_count));
 memcpy(g->wave_types,old->wave_types,sizeof(old->wave_types));
 memcpy(g->row_history,old->row_history,sizeof(old->row_history));
 memcpy(g->row_second,old->row_second,sizeof(old->row_second));
 memcpy(g->row_mowed_wave,old->row_mowed_wave,sizeof(old->row_mowed_wave));
 memcpy(g->packets,old->packets,sizeof(old->packets));
 g->packet_count=old->packet_count;
 g->last_packet=old->last_packet;
 g->sky_count=old->sky_count;
 g->conveyor_timer=old->conveyor_timer;
 g->huge_wave_timer=old->huge_wave_timer;
 g->huge_wave_announced=old->huge_wave_announced;
 memcpy(g->nuts,old->nuts,sizeof(old->nuts));
 g->shovel_tutorial=old->shovel_tutorial;
 g->level_flags=old->level_flags;
 memcpy(g->night_cooldown,old->night_cooldown,sizeof(old->night_cooldown));
 memcpy(g->night_cost,old->night_cost,sizeof(old->night_cost));
 memcpy(g->night_refresh,old->night_refresh,sizeof(old->night_refresh));
 memcpy(g->graves,old->graves,sizeof(old->graves));
 memcpy(g->hypnotized,old->hypnotized,sizeof(old->hypnotized));
 memcpy(g->craters,old->craters,sizeof(old->craters));
 memcpy(g->frozen,old->frozen,sizeof(old->frozen));
 memcpy(g->special,old->special,sizeof(old->special));
 g->challenge=old->challenge;
 g->whack_goal=old->whack_goal;
 memcpy(g->shot_remaining,old->shot_remaining,sizeof(old->shot_remaining));
 memcpy(g->shot_y,old->shot_y,sizeof(old->shot_y));
 memcpy(g->shot_vx,old->shot_vx,sizeof(old->shot_vx));
 memcpy(g->shot_vy,old->shot_vy,sizeof(old->shot_vy));
 memcpy(g->shot_damage,old->shot_damage,sizeof(old->shot_damage));
 memcpy(g->shot_target,old->shot_target,sizeof(old->shot_target));
 memcpy(g->support,old->support,sizeof(old->support));
 memcpy(g->vases,old->vases,sizeof(old->vases));
 memcpy(g->asleep,old->asleep,sizeof(old->asleep));
 memcpy(g->ladders,old->ladders,sizeof(old->ladders));
 memcpy(g->support_hp,old->support_hp,sizeof(old->support_hp));
 memcpy(g->pumpkin_hp,old->pumpkin_hp,sizeof(old->pumpkin_hp));
 memcpy(g->ice_tiles,old->ice_tiles,sizeof(old->ice_tiles));
 g->fog_clear=old->fog_clear;
 g->boss_hp=old->boss_hp;
 g->boss_timer=old->boss_timer;
 g->boss_phase=old->boss_phase;
 g->boss_ball_x=old->boss_ball_x;
 g->boss_row=old->boss_row;
 g->boss_ball_kind=old->boss_ball_kind;
 g->stage=old->stage;
 g->coins=old->coins;
 g->boss_ball_row=old->boss_ball_row;
 g->purchased_mask=old->purchased_mask;
 g->seed_slots=old->seed_slots;
 g->imitater_seed=old->imitater_seed;
 g->cob_row=old->cob_row;
 g->cob_col=old->cob_col;
 g->cob_aiming=old->cob_aiming;
 g->store_flags=old->store_flags;
 g->session_id=old->session_id;
 g->money_collected=old->money_collected;
 g->seen_zombies=old->seen_zombies;
 memcpy(g->money,old->money,sizeof(old->money));
 memcpy(g->imitated,old->imitated,sizeof(old->imitated));
 memcpy(g->morph_kind,old->morph_kind,sizeof(old->morph_kind));
 memcpy(g->morph_timer,old->morph_timer,sizeof(old->morph_timer));
 memcpy(g->burst,old->burst,sizeof(old->burst));
 memcpy(g->burst_left,old->burst_left,sizeof(old->burst_left));
 g->rake_row=old->rake_row;
 g->rake_col=old->rake_col;
 g->rake_state=old->rake_state;
 g->rake_timer=old->rake_timer;
 memcpy(g->burst_target,old->burst_target,sizeof(old->burst_target));
 memcpy(g->morph_hp,old->morph_hp,sizeof(old->morph_hp));
 if(g->challenge==1)g->mini.id=2;else if(g->challenge==2)g->mini.id=15;
}
static int game_load_one(Game *g,const char *filename){
 FILE *f=fopen(filename,"rb");if(!f)return 0;uint32_t h[4];Game loaded;int ok=fread(h,sizeof(h),1,f)==1&&h[0]==0x31535650;
 if(ok&&h[1]==13&&h[2]==sizeof(Game))ok=fread(&loaded,sizeof(loaded),1,f)==1&&checksum(&loaded,sizeof(loaded))==h[3];
 else if(ok&&h[1]==12&&h[2]==offsetof(Game,fidelity)){memset(&loaded,0,sizeof(loaded));ok=fread(&loaded,h[2],1,f)==1&&checksum(&loaded,h[2])==h[3];}
 else if(ok&&h[1]==11&&h[2]==offsetof(Game,completion)){memset(&loaded,0,sizeof(loaded));ok=fread(&loaded,h[2],1,f)==1&&checksum(&loaded,h[2])==h[3];}
 else if(ok&&h[1]==10&&h[2]==offsetof(Game,polish)){memset(&loaded,0,sizeof(loaded));ok=fread(&loaded,h[2],1,f)==1&&checksum(&loaded,h[2])==h[3];}
 else if(ok&&h[1]==9&&h[2]==sizeof(Legacy09)){Legacy09 old;ok=fread(&old,sizeof(old),1,f)==1&&checksum(&old,sizeof(old))==h[3];if(ok){memset(&loaded,0,sizeof(loaded));memcpy(&loaded,&old,sizeof(old));}}
 else if(ok&&h[1]==8&&h[2]==sizeof(Legacy08)){Legacy08 old;ok=fread(&old,sizeof(old),1,f)==1&&checksum(&old,sizeof(old))==h[3];if(ok)legacy08_import(&loaded,&old);}
 else if(ok&&(h[1]==6||h[1]==7)&&h[2]==sizeof(Legacy071)){Legacy071 old;ok=fread(&old,sizeof(old),1,f)==1&&checksum(&old,sizeof(old))==h[3];if(ok){legacy071_import(&loaded,&old);if(loaded.mode==MODE_BOWLING)for(int i=0;i<loaded.packet_count;i++)if(loaded.packets[i]==40)loaded.packets[i]=EXPLODE_NUT;}}
 else if(ok&&((h[1]==5&&h[2]==sizeof(LegacyGame))||((h[1]==3||h[1]==4)&&h[2]==16528))){
  LegacyGame old;memset(&old,0,sizeof(old));memcpy(old.night_cost,all_costs+8,8*sizeof(int));memcpy(old.night_refresh,all_refresh+8,8*sizeof(int));
  ok=fread(&old,h[2],1,f)==1&&checksum(&old,h[2])==h[3]&&old.level>=1&&old.level<=20;
  if(ok){legacy_import(&loaded,&old);if(loaded.mode==MODE_BOWLING)for(int p=0;p<loaded.packet_count&&p<MAX_PACKETS;p++)if(loaded.packets[p]==(h[1]==5?16:8))loaded.packets[p]=EXPLODE_NUT;}
 }else ok=0;
 if(ok)ok=fgetc(f)==EOF;fclose(f);
 if(ok)ok=loaded.status>=0&&loaded.status<=2&&loaded.level>=1&&loaded.level<=50&&loaded.num_waves>=1&&loaded.num_waves<=40&&loaded.wave>=1&&loaded.wave<=loaded.num_waves&&loaded.sun>=0&&loaded.sun<=1000000&&loaded.selected>=0&&loaded.selected<ALL_SEEDS&&loaded.packet_count>=0&&loaded.packet_count<=MAX_PACKETS;
 if(ok&&h[1]<10&&loaded.mode==MODE_VASE){for(int r=0;r<5;r++)for(int c=0;c<9;c++)if(loaded.vases[r][c]){int slot=r*(3+loaded.stage)+(c-(COLS-3-loaded.stage));static const int kinds[]={REPEATER,SQUASH,WALLNUT,CHERRY,SNOW,PEA};loaded.extra.vase_content[r][c]=slot%3==1?-1-(slot%7==0?Z_BUCKET:slot%4==0?Z_CONE:Z_NORMAL):kinds[(slot+loaded.stage)%6];}}
 if(ok){ExtraState *e=&loaded.extra;ok=loaded.mode>=0&&loaded.mode<=MODE_IZOMBIE&&loaded.cursor_row>=0&&loaded.cursor_row<ROWS&&loaded.cursor_col>=0&&loaded.cursor_col<COLS&&e->category>=0&&e->category<=EXTRA_IZOMBIE&&e->card_count>=0&&e->card_count<=12&&e->selection>=0&&e->selection<(e->category==EXTRA_SURVIVAL?5:10)&&e->difficulty>=0&&e->difficulty<=2&&e->stage>=0&&e->stage<1000000&&e->garden_drop_count>=0;
  if(e->category==EXTRA_IZOMBIE)ok=ok&&e->card_count>0;for(int i=0;i<e->card_count&&ok;i++)ok=e->zombie_cards[i]>=0&&e->zombie_cards[i]<Z_TYPES;
  for(int i=0;i<MAX_ZOMBIES&&ok;i++)if(loaded.zombies[i].used){Zombie *z=&loaded.zombies[i];ok=z->row>=0&&z->row<ROWS&&z->kind>=0&&z->kind<Z_TYPES&&isfinite(z->hp)&&isfinite(z->x)&&isfinite(z->age);}
  for(int i=0;i<loaded.packet_count&&ok;i++)ok=loaded.packets[i]>=0&&loaded.packets[i]<=LEFT_PACKET;
  for(int w=0;w<loaded.num_waves&&ok;w++){ok=loaded.wave_count[w]>=0&&loaded.wave_count[w]<=WAVE_CAPACITY;for(int n=0;n<loaded.wave_count[w]&&ok;n++)ok=loaded.wave_types[w][n]<Z_TYPES;}
  if(h[1]>=11){PolishState *p=&loaded.polish;ok=ok&&p->wave_pending>=0&&p->wave_pending<=1&&p->wave_entry>=0&&p->wave_entry<=WAVE_CAPACITY&&(!p->wave_pending||p->wave_entry<=loaded.wave_count[loaded.wave-1])&&p->grave_entry>=0&&p->grave_entry<=ROWS*COLS&&isfinite(p->wave_health)&&p->wave_health>=0&&p->wave_health<1e9f&&isfinite(p->wave_fraction)&&p->wave_fraction>=0&&p->wave_fraction<=1;
   for(int i=0;i<MAX_ZOMBIES&&ok;i++)ok=isfinite(p->pose[i])&&p->pose[i]>=0&&isfinite(p->action_time[i])&&p->action_time[i]>=0&&p->action[i]>=ACTION_NONE&&p->action[i]<ACTION_COUNT&&p->death_style[i]>=DEATH_NORMAL&&p->death_style[i]<=DEATH_WATER&&p->target[i]>=0&&p->target[i]<=1000&&(p->action[i]!=ACTION_GARLIC||p->target[i]<ROWS*ROWS);
  }
 }
 if(ok&&h[1]>=12)ok=completion_valid(&loaded);
 if(ok&&h[1]>=13){for(int i=0;i<MAX_ZOMBIES&&ok;i++)ok=loaded.fidelity.equipment_lost[i]<=1&&loaded.fidelity.bungee_phase[i]<=4&&loaded.fidelity.carried_kind[i]<=ALL_SEEDS&&isfinite(loaded.fidelity.carried_age[i])&&loaded.fidelity.carried_age[i]>=0&&isfinite(loaded.fidelity.actor_aux[i])&&fabsf(loaded.fidelity.actor_aux[i])<10000;for(int r=0;r<ROWS;r++)for(int c=0;c<COLS&&ok;c++)ok=isfinite(loaded.fidelity.support_safe[r][c])&&loaded.fidelity.support_safe[r][c]>=0&&loaded.fidelity.support_safe[r][c]<=1&&isfinite(loaded.fidelity.wake[r][c])&&loaded.fidelity.wake[r][c]>=0&&loaded.fidelity.wake[r][c]<=2;}
 if(ok){
  if(h[1]<13)for(int i=0;i<MAX_ZOMBIES;i++)if(loaded.zombies[i].used){Zombie *z=&loaded.zombies[i];loaded.fidelity.actor_id[i]=++loaded.fidelity.next_actor;loaded.fidelity.actor_aux[i]=z->kind==Z_CATAPULT?20:z->kind==Z_JACK?12:z->kind==Z_BUNGEE?3.75f:0;if(z->vault>0)z->vault*=game_vault_duration(z)/(43/24.0f);}
  if(h[1]<11)for(int i=0;i<MAX_ZOMBIES;i++)loaded.polish.pose[i]=loaded.zombies[i].kind>=Z_DUCK?loaded.time:loaded.zombies[i].age;
  if(h[1]<12)for(int i=0;i<MAX_ZOMBIES;i++)if(loaded.zombies[i].used&&loaded.zombies[i].kind==Z_YETI)loaded.completion.yeti_timer[i]=15;
  if(h[1]==3)for(int i=0;i<MAX_ZOMBIES;i++)if(loaded.zombies[i].used){Zombie *z=&loaded.zombies[i];float speed=z->speed/47.5f;z->speed=z->kind==Z_FLAG?.45f:z->kind==Z_POLE&&!z->vaulted?.66f+fminf(1,fmaxf(0,(speed-.23f)/.14f))*.02f:speed;if(z->death<=0&&!z->eating&&z->vault<=0)z->age=0;}
  if(loaded.level==2&&!loaded.total_spawned&&!(loaded.level_flags&LEVEL_TUTORIAL_COMPLETE)&&game_sunflower_count(&loaded)>=3){loaded.spawn_timer=fminf(loaded.spawn_timer,10);loaded.level_flags|=LEVEL_TUTORIAL_COMPLETE;}
  if(h[1]<7)for(int i=0;i<MAX_ZOMBIES;i++)if(loaded.zombies[i].used&&loaded.zombies[i].death>0){Zombie *z=&loaded.zombies[i];z->vault_from=27;if(z->kind==Z_GARG||z->kind==Z_SNORKEL||z->kind==Z_YETI)z->vault_from=14;else if(z->kind==Z_DIGGER||z->kind==Z_BOSS)z->vault_from=18;z->death=fmaxf(.01f,game_death_duration(z)-z->age);}
  if(loaded.challenge&&!loaded.extra.category&&!loaded.mini.id)loaded.mini.id=loaded.challenge==1?MINI_BOWLING+1:MINI_WHACK+1;
  *g=loaded;
 }return ok;
}
static int game_recovered;
int game_load_backup_used(void){return game_recovered;}
int game_load(Game *g,const char *filename){
 game_recovered=0;if(game_load_one(g,filename))return 1;
 char backup[512];if(snprintf(backup,sizeof(backup),"%s.bak",filename)>=(int)sizeof(backup))return 0;
 game_recovered=game_load_one(g,backup);return game_recovered;
}
static int legacy_fixture(const Game *g,const char *path,int version){LegacyGame old;legacy_export(&old,g);int bytes=version==5?sizeof(old):16528;uint32_t h[4]={0x31535650,version,bytes,checksum(&old,bytes)};FILE *f=fopen(path,"wb");if(!f)return 0;int ok=fwrite(h,sizeof(h),1,f)==1&&fwrite(&old,bytes,1,f)==1;fclose(f);return ok;}

static int legacy071_fixture(const Game *g,const char *path,int version){
 Legacy071 value,*old=&value;memset(old,0,sizeof(*old));
 old->rng=g->rng;
 old->status=g->status;
 old->sun=g->sun;
 old->wave=g->wave;
 old->spawned=g->spawned;
 old->kills=g->kills;
 old->total_spawned=g->total_spawned;
 old->selected=g->selected;
 old->shovel=g->shovel;
 old->cursor_row=g->cursor_row;
 old->cursor_col=g->cursor_col;
 memcpy(old->mower_used,g->mower_used,sizeof(old->mower_used));
 memcpy(old->events,g->events,sizeof(old->events));
 old->time=g->time;
 old->spawn_timer=g->spawn_timer;
 old->sky_timer=g->sky_timer;
 old->wave_delay=g->wave_delay;
 old->flash=g->flash;
 memcpy(old->cooldown,g->cooldown,sizeof(old->cooldown));
 memcpy(old->mower_x,g->mower_x,sizeof(old->mower_x));
 memcpy(old->seed_cost,g->seed_cost,sizeof(old->seed_cost));
 memcpy(old->seed_refresh,g->seed_refresh,sizeof(old->seed_refresh));
 memcpy(old->plants,g->plants,sizeof(old->plants));
 memcpy(old->zombies,g->zombies,sizeof(old->zombies));
 memcpy(old->shots,g->shots,sizeof(old->shots));
 memcpy(old->suns,g->suns,sizeof(old->suns));
 old->wave_health_threshold=g->wave_health_threshold;
 old->level=g->level;
 old->num_waves=g->num_waves;
 old->row_mask=g->row_mask;
 old->unlocked_mask=g->unlocked_mask;
 old->seed_mask=g->seed_mask;
 old->mode=g->mode;
 memcpy(old->wave_count,g->wave_count,sizeof(old->wave_count));
 memcpy(old->wave_types,g->wave_types,sizeof(old->wave_types));
 memcpy(old->row_history,g->row_history,sizeof(old->row_history));
 memcpy(old->row_second,g->row_second,sizeof(old->row_second));
 memcpy(old->row_mowed_wave,g->row_mowed_wave,sizeof(old->row_mowed_wave));
 memcpy(old->packets,g->packets,sizeof(old->packets));
 old->packet_count=g->packet_count;
 old->last_packet=g->last_packet;
 old->sky_count=g->sky_count;
 old->conveyor_timer=g->conveyor_timer;
 old->huge_wave_timer=g->huge_wave_timer;
 old->huge_wave_announced=g->huge_wave_announced;
 memcpy(old->nuts,g->nuts,sizeof(old->nuts));
 old->shovel_tutorial=g->shovel_tutorial;
 old->level_flags=g->level_flags;
 memcpy(old->night_cooldown,g->night_cooldown,sizeof(old->night_cooldown));
 memcpy(old->night_cost,g->night_cost,sizeof(old->night_cost));
 memcpy(old->night_refresh,g->night_refresh,sizeof(old->night_refresh));
 memcpy(old->graves,g->graves,sizeof(old->graves));
 memcpy(old->hypnotized,g->hypnotized,sizeof(old->hypnotized));
 memcpy(old->craters,g->craters,sizeof(old->craters));
 memcpy(old->frozen,g->frozen,sizeof(old->frozen));
 memcpy(old->special,g->special,sizeof(old->special));
 old->challenge=g->challenge;
 old->whack_goal=g->whack_goal;
 memcpy(old->shot_remaining,g->shot_remaining,sizeof(old->shot_remaining));
 memcpy(old->shot_y,g->shot_y,sizeof(old->shot_y));
 memcpy(old->shot_vx,g->shot_vx,sizeof(old->shot_vx));
 memcpy(old->shot_vy,g->shot_vy,sizeof(old->shot_vy));
 memcpy(old->shot_damage,g->shot_damage,sizeof(old->shot_damage));
 memcpy(old->shot_target,g->shot_target,sizeof(old->shot_target));
 memcpy(old->support,g->support,sizeof(old->support));
 memcpy(old->vases,g->vases,sizeof(old->vases));
 memcpy(old->asleep,g->asleep,sizeof(old->asleep));
 memcpy(old->ladders,g->ladders,sizeof(old->ladders));
 memcpy(old->support_hp,g->support_hp,sizeof(old->support_hp));
 memcpy(old->pumpkin_hp,g->pumpkin_hp,sizeof(old->pumpkin_hp));
 memcpy(old->ice_tiles,g->ice_tiles,sizeof(old->ice_tiles));
 old->fog_clear=g->fog_clear;
 old->boss_hp=g->boss_hp;
 old->boss_timer=g->boss_timer;
 old->boss_phase=g->boss_phase;
 old->boss_ball_x=g->boss_ball_x;
 old->boss_row=g->boss_row;
 old->boss_ball_kind=g->boss_ball_kind;
 old->stage=g->stage;
 old->coins=g->coins;
 old->boss_ball_row=g->boss_ball_row;

 uint32_t h[4]={0x31535650,version,sizeof(*old),checksum(old,sizeof(*old))};FILE *f=fopen(path,"wb");if(!f)return 0;int ok=fwrite(h,sizeof(h),1,f)==1&&fwrite(old,sizeof(*old),1,f)==1;fclose(f);return ok;
}
static void advance(Game *g,float seconds) { for(int i=0;i<(int)(seconds*60);i++)game_step(g,1.0f/60); }
static void test_arena(Game *g,uint32_t seed){game_init(g,seed);g->sun=50;g->row_mask=31;g->seed_mask=g->unlocked_mask=255;}
#define game_init test_arena
int game_selftest(FILE *out) {
    Game g;int passed=0,total=0;
    #define CHECK(label,condition) do { int ok=(condition);total++;passed+=ok;fprintf(out,"%s: %s\n",label,ok?"PASS":"FAIL"); } while(0)
    game_init(&g,1);g.sun=1000;
    CHECK("plant deducts exact sun",game_place(&g,2,2,PEA)==0&&g.sun==900);
    CHECK("occupied tile rejected",game_place(&g,2,2,SUNFLOWER)==-2);
    CHECK("seed cooldown enforced",game_place(&g,2,3,PEA)==-4);
    g.spawn_timer=10000;game_spawn_zombie(&g,2,0,459);advance(&g,26);
    CHECK("peas kill approaching zombie without a mower",g.kills==1&&!g.mower_used[2]&&g.plants[2][2].hp==300);
    game_init(&g,2);g.sun=1000;g.spawn_timer=10000;game_place(&g,1,3,WALLNUT);game_spawn_zombie(&g,1,0,plant_x(3)+20);advance(&g,2);
    CHECK("zombie bites and stops at plant",g.plants[1][3].hp<3900&&g.zombies[0].eating&&g.zombies[0].x>plant_x(3));
    game_init(&g,3);g.spawn_timer=10000;int z=game_spawn_zombie(&g,0,1,300);hurt_zombie(&g,&g.zombies[z],20,1);
    CHECK("armor absorbs damage and frost slows",g.zombies[z].armor==350&&g.zombies[z].hp==270&&g.zombies[z].slow==8);
    game_init(&g,4);g.sun=1000;g.spawn_timer=10000;game_place(&g,2,4,CHERRY);game_spawn_zombie(&g,1,2,plant_x(4)+10);game_spawn_zombie(&g,3,0,plant_x(4)+15);advance(&g,1.4f);
    CHECK("cherry area hits adjacent lanes",g.kills==2&&g.plants[2][4].kind==-1);
    game_init(&g,5);g.sun=1000;g.spawn_timer=10000;game_place(&g,2,4,MINE);g.plants[2][4].age=16;game_spawn_zombie(&g,2,2,plant_x(4)+5);advance(&g,.1f);
    CHECK("armed mine destroys armored zombie",g.kills==1&&g.plants[2][4].kind==-1);
    game_init(&g,50);g.sun=1000;g.spawn_timer=10000;game_place(&g,2,4,MINE);g.plants[2][4].age=MINE_ARM_SECONDS;
    game_spawn_zombie(&g,2,2,plant_x(4)+ZOMBIE_BITE_FRONT);advance(&g,.1f);
    CHECK("armed mine triggers at bite boundary before damage",g.kills==1&&g.plants[2][4].kind==-1&&g.plants[2][4].hp==300);
    game_init(&g,51);g.sun=1000;g.spawn_timer=10000;game_place(&g,2,4,MINE);g.plants[2][4].age=MINE_ARM_SECONDS;
    game_spawn_zombie(&g,2,0,plant_x(4)+60);advance(&g,9);
    CHECK("approaching zombie reaches mine trigger without eating it",g.kills==1&&g.plants[2][4].hp==300);
    game_init(&g,52);g.sun=1000;g.spawn_timer=10000;game_place(&g,2,4,MINE);
    game_spawn_zombie(&g,2,0,plant_x(4)+23);advance(&g,1);
    CHECK("unarmed mine still needs time to arm",g.kills==0&&g.plants[2][4].kind==MINE&&g.plants[2][4].hp<300);
    g.plants[2][4].age=MINE_ARM_SECONDS-.001f;advance(&g,.1f);
    CHECK("mine arms underneath stopped zombie and detonates",g.kills==1&&g.plants[2][4].kind==-1);
    game_init(&g,53);g.sun=1000;g.spawn_timer=10000;game_place(&g,2,4,MINE);g.plants[2][4].age=MINE_ARM_SECONDS;
    game_spawn_zombie(&g,2,0,plant_x(4)+23);game_spawn_zombie(&g,2,1,plant_x(4)+25);game_spawn_zombie(&g,1,0,plant_x(4)+23);advance(&g,.1f);
    CHECK("mine blast hits nearby zombies in its lane only",g.kills==2&&game_alive(&g)==1);
    game_init(&g,6);g.spawn_timer=10000;game_spawn_zombie(&g,3,2,GRID_X-15);advance(&g,.5f);
    CHECK("mower rescues lane",g.mower_used[3]&&g.kills==1);
    game_init(&g,7);g.spawn_timer=10000;g.mower_used[0]=1;g.mower_x[0]=600;game_spawn_zombie(&g,0,0,12);advance(&g,.1f);
    CHECK("unprotected breach loses level",g.status==LOST);
    game_init(&g,8);g.spawn_timer=10000;add_sun(&g,100,100,100);add_sun(&g,200,100,100);
    CHECK("sun collection starts flight and original collection cue",game_collect(&g)==2&&g.sun==50&&g.suns[0].used==2&&g.events[SFX_SUN]);
    CHECK("flying sun cannot be collected twice",game_collect(&g)==0);
    game_animate_pickups(&g,.15f);
    CHECK("collected sun flies toward the counter before credit",g.suns[0].x<100&&g.suns[0].x>22&&g.suns[0].y<100&&g.sun==50);
    game_animate_pickups(&g,1);
    CHECK("sun credits 25 each on reaching the counter",g.sun==100&&!g.suns[0].used&&!g.suns[1].used);
    game_init(&g,9);g.sun=1000;g.spawn_timer=10000;game_place(&g,1,1,SUNFLOWER);advance(&g,14);
    CHECK("sunflower produces collectible sun",game_collect(&g)>=1);
    game_init(&g,10);g.wave=OPENING_WAVES;g.spawned=2;g.spawn_timer=10000;advance(&g,3.2f);
    CHECK("last cleared wave wins",g.status==WON);
    game_init(&g,11);g.sun=1000;game_place(&g,1,1,CHOMPER);g.spawn_timer=10000;game_spawn_zombie(&g,1,2,plant_x(1)+32);advance(&g,1.2f);
    CHECK("chomper swallows and begins digesting",g.kills==1&&g.plants[1][1].digest>39);
    game_init(&g,12);g.row_mask=4;advance(&g,17);
    CHECK("PS3 initial countdown does not spawn early",g.total_spawned==0);
    advance(&g,1.1f);
    CHECK("PS3 opening wave is one middle-lane regular zombie",g.total_spawned==1&&g.zombies[0].row==2&&g.zombies[0].kind==0);
    for(int i=0;i<24*60;i++){g.zombies[0].x=450;game_step(&g,1.0f/60);}
    CHECK("healthy first zombie does not cause repeated rapid spawns",g.total_spawned==1);
    for(int i=0;i<8*60;i++){for(int j=0;j<MAX_ZOMBIES;j++)if(g.zombies[j].used)g.zombies[j].x=450;game_step(&g,1.0f/60);}
    CHECK("ordinary PS3 countdown advances to second single zombie wave",g.total_spawned==2&&g.wave==2);
    game_init(&g,13);g.spawn_timer=.001f;advance(&g,.1f);hurt_zombie(&g,&g.zombies[0],150,0);advance(&g,3.7f);
    CHECK("PS3 health rule waits four seconds before shortening countdown",g.total_spawned==1&&g.spawn_timer>2);
    advance(&g,.3f);
    CHECK("PS3 health rule starts two second early countdown",g.spawn_timer<=2&&g.spawn_timer>1.7f);
    advance(&g,2.1f);
    CHECK("PS3 health-based countdown starts next wave",g.total_spawned==2&&g.wave==2);
    game_init(&g,14);g.wave=4;g.spawn_timer=.001f;advance(&g,.1f);
    CHECK("PS3 fourth opening wave has two regular zombies",g.total_spawned==2&&g.zombies[0].kind==0&&g.zombies[1].kind==0);
    Game restored;int saved=game_save(&g,"selftest-save.dat");
    CHECK("new wave state survives save and load",saved&&game_load(&restored,"selftest-save.dat")&&restored.wave==4&&restored.wave_health_threshold==g.wave_health_threshold&&restored.zombies[0].wave==4);
    remove("selftest-save.dat");
    #undef game_init
    game_init(&g,21);
    CHECK("adventure begins at 1-1 with 150 sun and only peashooter",g.level==1&&g.sun==150&&g.row_mask==4&&g.seed_mask==(1<<PEA));
    CHECK("dirt rows reject planting",game_place(&g,0,0,PEA)==-1);
    CHECK("unearned seeds reject planting",game_place(&g,2,0,SUNFLOWER)==-5);
    CHECK("ordinary level cannot be skipped",!game_next_level(&g)&&g.level==1);
    g.status=WON;game_collect_reward(&g);CHECK("winning advances to 1-2 with sunflower",game_next_level(&g)&&g.level==2&&(g.seed_mask&(1<<SUNFLOWER))&&g.row_mask==14&&g.spawn_timer==50);
    g.status=WON;game_collect_reward(&g);game_next_level(&g);
    CHECK("1-3 unlocks cherry and introduces coneheads",g.level==3&&(g.seed_mask&(1<<CHERRY))&&g.wave_types[4][0]==Z_CONE);
    int inactive=0,seen=0;for(int i=0;i<300;i++){int row=choose_row(&g);seen|=1<<row;inactive+=!game_row_active(&g,row);g.total_spawned++;}
    CHECK("three-row spawns visit every unlocked lane and no dirt",seen==14&&inactive==0);
    game_start_level(&g,4,22);seen=0;for(int i=0;i<300;i++){seen|=1<<choose_row(&g);g.total_spawned++;}
    CHECK("1-4 spawns across all five lanes",seen==31&&g.row_mask==31&&(g.seed_mask&(1<<WALLNUT)));
    int all_tables=1,no_early=1,grows=1,flags=1;
    for(int level=1;level<=CAMPAIGN_LEVELS;level++){
        game_start_level(&g,level,23+level);all_tables&=g.num_waves==(level==15?8:level==35?3:campaign_waves[level-1]);
        if(level==35||level==50)continue;
        grows&=g.wave_count[g.num_waves-1]>g.wave_count[0];
        for(int w=0;w<g.num_waves;w++)for(int j=0;j<g.wave_count[w];j++){
            int kind=g.wave_types[w][j];no_early&=(level==45&&kind==Z_BUNGEE)||kind==Z_FLAG||game_zombie_allowed(level,kind);
            if(kind==Z_POLE&&w<4)no_early=0;
        }
        if(level>1&&level!=45){int found=0;for(int j=0;j<g.wave_count[g.num_waves-1];j++)found+=g.wave_types[g.num_waves-1][j]==Z_FLAG;flags&=found==1;}
    }
    CHECK("all fifty levels use disc wave counts or their challenge rules",all_tables);
    CHECK("later planned waves increase across wave-based levels",grows);
    CHECK("zombie roster obeys level and first-wave restrictions",no_early);
    CHECK("every wave-based finale except tutorial includes one flag",flags);
    game_start_level(&g,7,38);CHECK("1-7 has flags at waves 10 and 20",game_flag_wave(&g,9)&&game_flag_wave(&g,19)&&!game_flag_wave(&g,8));
    g.wave=9;g.spawned=1;g.spawn_timer=.01f;game_spawn_zombie(&g,2,Z_NORMAL,450);advance(&g,.1f);
    CHECK("flag announcement pauses spawns for original 750 ticks",g.wave==9&&g.huge_wave_timer>7.3f);
    for(int i=0;i<7*60;i++){for(int j=0;j<MAX_ZOMBIES;j++)if(g.zombies[j].used)g.zombies[j].x=450;game_step(&g,1.0f/60);}
    CHECK("flag wave does not appear before announcement finishes",g.wave==9);
    advance(&g,.6f);CHECK("flag wave spawns after announcement",g.wave==10&&g.spawned==g.wave_count[9]);
    game_start_level(&g,2,40);g.sun=1000;g.sky_timer=g.spawn_timer=10000;game_place(&g,2,1,SUNFLOWER);
    CHECK("sunflower first production follows 300 to 1250 ticks",g.plants[2][1].attack>=3&&g.plants[2][1].attack<=12.5f);
    g.plants[2][1].attack=.7f;float glow=game_sunflower_glow(&g.plants[2][1]);advance(&g,.3f);
    CHECK("sunflower brightens before sun release",glow>.29f&&game_sunflower_glow(&g.plants[2][1])>glow&&game_collect(&g)==0);
    advance(&g,.5f);int suns=0,from=-1;for(int i=0;i<MAX_SUNS;i++)if(g.suns[i].used){suns++;from=i;}
    CHECK("sunflower releases one rising sun with flash",suns==1&&from>=0&&g.suns[from].from_plant&&g.suns[from].vy<0&&game_sunflower_glow(&g.plants[2][1])>.7f);
    CHECK("sunflower repeat interval follows 2350 to 2500 ticks",g.plants[2][1].attack>23&&g.plants[2][1].attack<=25);
    advance(&g,2);CHECK("generated sun lands without vanishing",from>=0&&g.suns[from].used&&g.suns[from].y==g.suns[from].target_y);
    game_start_level(&g,5,41);advance(&g,5);
    CHECK("Dave shovel practice holds back bowling spawns",g.shovel_tutorial==3&&g.total_spawned==0&&g.spawn_timer==2);
    for(int c=0;c<3;c++)game_shovel(&g,2,c);
    CHECK("shoveling all three wallnuts starts bowling",g.shovel_tutorial==0&&g.plants[2][2].kind==-1);
    CHECK("bowling blocks placement beyond red line",game_place(&g,2,3,WALLNUT)==-6);
    CHECK("bowling uses delivered nut without sun or cooldown",game_place(&g,2,1,WALLNUT)==0&&g.packet_count==0&&g.sun==0&&g.cooldown[WALLNUT]==0);
    g.spawn_timer=10000;game_spawn_zombie(&g,2,Z_CONE,g.nuts[0].x+13);advance(&g,.1f);
    CHECK("bowling nut strips cone and ricochets",g.zombies[0].armor==0&&g.nuts[0].target_row!=2);
    g.nuts[0].used=0;g.packets[0]=EXPLODE_NUT;g.packet_count=1;g.selected=0;game_place(&g,2,1,EXPLODE_NUT);
    game_spawn_zombie(&g,2,Z_BUCKET,g.nuts[0].x+13);advance(&g,.1f);
    CHECK("red bowling nut explodes against armored zombie",g.kills>=1&&!g.nuts[0].used);
    game_start_level(&g,5,410);g.shovel_tutorial=0;g.spawn_timer=10000;
    int old_hit=game_spawn_zombie(&g,2,Z_CONE,220),next_hit=game_spawn_zombie(&g,1,Z_NORMAL,250);
    g.nuts[0]=(BowlingNut){.used=1,.row=2,.target_row=1,.direction=-1,.last_hit=old_hit,.x=240,.y=game_ground_y(&g,1)+12,.age=1};
    bowling_step(&g,.05f);
    CHECK("ricocheting nut hits adjacent-lane zombie while still diagonal",g.zombies[next_hit].death>0&&g.nuts[0].last_hit==next_hit&&g.kills==1);
    game_start_level(&g,5,411);g.shovel_tutorial=0;g.spawn_timer=10000;
    int armored=game_spawn_zombie(&g,2,Z_BUCKET,260);
    g.nuts[0]=(BowlingNut){.used=1,.row=2,.target_row=2,.last_hit=-1,.x=240,.y=game_ground_y(&g,2)};
    bowling_step(&g,.01f);float armor_after=g.zombies[armored].armor;bowling_step(&g,.01f);
    CHECK("one overlap cannot repeatedly damage same armored zombie",armor_after==200&&g.zombies[armored].armor==armor_after&&g.zombies[armored].hp==270);
    g.nuts[0].x=300;bowling_step(&g,.01f);
    CHECK("bowling collision releases previous target after separating",g.nuts[0].last_hit==-1);
    game_start_level(&g,5,412);g.shovel_tutorial=0;g.spawn_timer=10000;
    game_spawn_zombie(&g,2,Z_NORMAL,265);g.nuts[0]=(BowlingNut){.used=1,.row=2,.target_row=2,.last_hit=-1,.x=240,.y=game_ground_y(&g,2),.age=3};
    bowling_step(&g,.1f);
    CHECK("older bowling nut retains full collision radius",g.kills==1);
    for(int nut_kind=0;nut_kind<3;nut_kind++){
        game_start_level(&g,5,413+nut_kind);g.shovel_tutorial=0;g.spawn_timer=10000;
        int victim=game_spawn_zombie(&g,2,Z_NORMAL,240);
        g.nuts[0]=(BowlingNut){.used=1,.explosive=nut_kind,.row=2,.target_row=2,.last_hit=-1,.x=230,.y=game_ground_y(&g,2)};
        bowling_step(&g,.01f);int blasts=0;for(int e=0;e<32;e++)blasts+=g.extra.effect_time[e]>0&&g.extra.effect_kind[e]==1;
        CHECK(nut_kind==0?"regular bowling hit kills without a blast or plant animation":nut_kind==1?"red bowling nut creates exactly one blast and is consumed":"giant bowling hit kills without a blast or plant animation",g.kills==1&&g.zombies[victim].death>0&&blasts==(nut_kind==1)&&g.nuts[0].used==(nut_kind!=1));
    }
    game_start_level(&g,10,42);g.spawn_timer=10000;advance(&g,4.2f);
    CHECK("day finale delivers conveyor packet and no sky sun",g.packet_count==1&&g.sun==0&&g.sky_count==0&&game_selected_kind(&g)!=SUNFLOWER);
    int kind=game_selected_kind(&g);CHECK("finale packet plants for free and is consumed",game_place(&g,2,2,kind)==0&&g.packet_count==0&&g.sun==0&&g.cooldown[kind]==0);
    game_start_level(&g,6,43);g.spawn_timer=10000;g.sun=1000;game_place(&g,2,3,WALLNUT);int pole=game_spawn_zombie(&g,2,Z_POLE,plant_x(3)+40);advance(&g,.1f);
    CHECK("pole vaulter starts vault without eating obstacle",g.zombies[pole].vault>0&&g.plants[2][3].hp==4000);
    advance(&g,2);CHECK("pole vaulter lands behind plant and uses pole once",g.zombies[pole].vaulted&&g.zombies[pole].x<plant_x(3)&&g.plants[2][3].hp==4000);
    game_start_level(&g,9,44);g.wave=12;g.spawned=2;g.huge_wave_timer=3.2f;
    saved=game_save(&g,"selftest-save.dat");CHECK("campaign and pending flag countdown survive save",saved&&game_load(&restored,"selftest-save.dat")&&restored.level==9&&restored.wave==12&&restored.num_waves==20&&restored.huge_wave_timer==g.huge_wave_timer&&memcmp(restored.wave_types,g.wave_types,sizeof(g.wave_types))==0);
    remove("selftest-save.dat.bak");FILE *corrupt=fopen("selftest-save.dat","r+b");if(corrupt){fseek(corrupt,32,SEEK_SET);fputc(255,corrupt);fclose(corrupt);}CHECK("damaged campaign save without a backup is rejected",!game_load(&restored,"selftest-save.dat"));remove("selftest-save.dat");
    game_start_level(&g,10,45);g.status=WON;game_collect_reward(&g);CHECK("day finale advances to original night chapter",game_next_level(&g)&&g.level==11&&(g.unlocked_mask&(1<<PUFF)));
    game_init(&g,46);int sequence=1;
    for(int level=1;level<=CAMPAIGN_LEVELS;level++){
        if(g.level!=level){sequence=0;break;}
        if(g.shovel_tutorial)for(int c=0;c<3;c++)game_shovel(&g,2,c);
        if(g.mode==MODE_VASE){g.stage=2;memset(g.vases,0,sizeof(g.vases));}if(g.mode==MODE_BOSS)g.boss_hp=0;
        for(int turns=0;turns<MAX_WAVES*3&&g.status==PLAYING;turns++){
            g.spawn_timer=0;if(g.huge_wave_timer>0)g.huge_wave_timer=.001f;
            game_step(&g,1.0f/60);
            for(int z=0;z<MAX_ZOMBIES;z++)if(g.zombies[z].used){kill_zombie(&g,&g.zombies[z]);g.zombies[z].used=0;}
        }
        sequence&=g.status==WON&&(level==35||level==50||g.wave==(level==15?8:campaign_waves[level-1]));
        if(g.status!=WON)fprintf(out,"campaign check stuck at level %d, wave %d, spawned %d\n",level,g.wave,g.spawned);
        if(level<CAMPAIGN_LEVELS){game_collect_reward(&g);sequence&=game_next_level(&g);}
    }
    CHECK("completion-state transitions traverse all fifty Adventure levels",sequence);
    game_start_level(&g,2,60);g.sun=1000;advance(&g,2);float before=g.spawn_timer;
    game_place(&g,1,0,SUNFLOWER);g.cooldown[SUNFLOWER]=0;game_place(&g,2,0,SUNFLOWER);
    CHECK("1-2 tutorial waits for third sunflower before override",g.total_spawned==0&&g.spawn_timer==before&&!(g.level_flags&LEVEL_TUTORIAL_COMPLETE));
    g.cooldown[SUNFLOWER]=0;game_place(&g,3,0,SUNFLOWER);
    CHECK("third sunflower applies PS3 1000 tick spawn countdown",g.spawn_timer==10&&(g.level_flags&LEVEL_TUTORIAL_COMPLETE));
    advance(&g,1);before=g.spawn_timer;g.cooldown[SUNFLOWER]=0;game_place(&g,1,1,SUNFLOWER);
    CHECK("additional sunflowers never restart tutorial countdown",g.spawn_timer==before);
    advance(&g,8.8f);CHECK("1-2 spawn respects shortened countdown",g.total_spawned==0);
    advance(&g,.4f);CHECK("1-2 first wave arrives after tutorial countdown",g.total_spawned==1&&g.wave==1&&game_row_active(&g,g.zombies[0].row));
    game_start_level(&g,1,61);g.sun=1000;game_place(&g,2,0,PEA);g.cooldown[PEA]=0;game_place(&g,2,1,PEA);
    CHECK("two peashooters use PS3 100 tick 1-1 tutorial trigger",g.spawn_timer==1&&(g.level_flags&LEVEL_TUTORIAL_COMPLETE));
    game_start_level(&g,2,62);g.level_flags=LEVEL_READY;g.spawn_timer=40;
    for(int r=1;r<=3;r++)g.plants[r][0]=(Plant){.kind=SUNFLOWER,.hp=300};
    saved=game_save(&g,"selftest-save.dat");
    CHECK("v0.3 in-progress 1-2 save repairs missed trigger without resetting progress",saved&&game_load(&restored,"selftest-save.dat")&&restored.level==2&&restored.spawn_timer==10&&(restored.level_flags&LEVEL_READY)&&game_sunflower_count(&restored)==3&&offsetof(LegacyGame,night_cooldown)==16528);remove("selftest-save.dat");
    game_start_level(&g,4,63);g.spawn_timer=10000;z=game_spawn_zombie(&g,2,Z_CONE,400);
    hurt_zombie(&g,&g.zombies[z],140,0);CHECK("cone enters first visible armor damage stage",g.zombies[z].armor==230&&game_zombie_armor_stage(&g.zombies[z])==1&&g.zombies[z].hp==270);
    hurt_zombie(&g,&g.zombies[z],120,0);CHECK("cone enters severe visible armor damage stage",g.zombies[z].armor==110&&game_zombie_armor_stage(&g.zombies[z])==2);
    hurt_zombie(&g,&g.zombies[z],110,0);CHECK("destroyed cone exposes undamaged zombie body",g.zombies[z].armor==0&&g.zombies[z].hp==270&&game_zombie_body_stage(&g.zombies[z])==0);
    z=game_spawn_zombie(&g,3,Z_BUCKET,400);hurt_zombie(&g,&g.zombies[z],380,0);
    CHECK("bucket enters first visible armor damage stage",game_zombie_armor_stage(&g.zombies[z])==1);
    hurt_zombie(&g,&g.zombies[z],380,0);CHECK("bucket enters severe visible armor damage stage",game_zombie_armor_stage(&g.zombies[z])==2);
    game_start_level(&g,4,64);g.spawn_timer=10000;z=game_spawn_zombie(&g,2,Z_NORMAL,400);
    for(int i=0;i<5;i++)hurt_zombie(&g,&g.zombies[z],20,0);
    CHECK("five pea hits remove regular zombie arm",g.zombies[z].hp==170&&game_zombie_body_stage(&g.zombies[z])==1);
    for(int i=0;i<4;i++)hurt_zombie(&g,&g.zombies[z],20,0);
    CHECK("head stays attached exactly at one-third body health",g.zombies[z].hp==90&&game_zombie_body_stage(&g.zombies[z])==1);
    uint32_t head_rng=g.rng;hurt_zombie(&g,&g.zombies[z],20,0);CHECK("next pea hit leaves a visibly headless dying zombie",g.zombies[z].hp==70&&game_zombie_body_stage(&g.zombies[z])==2&&g.zombies[z].death==0);
    {int slot=-1,heads=0;for(int i=0;i<32;i++)if(g.extra.effect_time[i]>0&&g.extra.effect_kind[i]>=2){heads++;slot=i;}
     HeadParticle start,ground;int shown=slot>=0&&game_head_particle(&g,slot,&start);
     CHECK("head loss creates exactly one detached head without consuming gameplay randomness",heads==1&&shown&&g.rng==head_rng);
     if(slot>=0){float life=g.extra.effect_time[slot];g.extra.effect_time[slot]=.2f;int landed=game_head_particle(&g,slot,&ground);
       CHECK("detached head falls onto the ground before fading",shown&&landed&&ground.y>start.y+25&&ground.y<=g.extra.effect_y[slot]+.01f&&ground.alpha==1);
       g.extra.effect_time[slot]=.04f;CHECK("detached head fades at end of original 180 tick lifetime",game_head_particle(&g,slot,&ground)&&ground.alpha>0&&ground.alpha<.5f);
       g.extra.effect_time[slot]=0;CHECK("expired detached head is no longer drawn",!game_head_particle(&g,slot,&ground));g.extra.effect_time[slot]=life;}}
    g.sun=1000;game_place(&g,2,4,WALLNUT);g.zombies[z].x=plant_x(4)+20;advance(&g,1);
    CHECK("headless zombie stops eating and loses remaining health",g.plants[2][4].hp==4000&&!g.zombies[z].eating&&g.zombies[z].hp<51&&g.zombies[z].hp>49);
    advance(&g,3);CHECK("headless regular zombie finishes dying",g.kills==1);
    game_start_level(&g,6,65);g.spawn_timer=10000;z=game_spawn_zombie(&g,2,Z_POLE,400);hurt_zombie(&g,&g.zombies[z],340,0);advance(&g,.1f);
    CHECK("headless pole vaulter drops pole and cannot start another vault",game_zombie_body_stage(&g.zombies[z])==2&&g.zombies[z].vaulted&&g.zombies[z].vault==0);
    CHECK("original ground profiles loaded for walk, run and post-vault",motion[0].count==47&&motion[1].count==37&&motion[2].count==45);
    Zombie walker={.kind=Z_NORMAL,.speed=.3f,.hp=270};
    float cycle=motion[0].x[motion[0].count-1]/(.3f*47),distance=game_zombie_walk(&walker,cycle),pace=distance/cycle;
    CHECK("ordinary walking uses recovered PS3 pace instead of double speed",pace>6.83f&&pace<6.85f&&CELL_W/pace>5.54f&&CELL_W/pace<5.57f);
    Zombie at30={.kind=Z_NORMAL,.speed=.3f},at60=at30;float d30=0,d60=0;
    for(int i=0;i<300;i++)d30+=game_zombie_walk(&at30,1.0f/30);
    for(int i=0;i<600;i++)d60+=game_zombie_walk(&at60,1.0f/60);
    CHECK("30 and 60 FPS cover the same ground and animation phase",fabsf(d30-d60)<.01f&&fabsf(at30.age-at60.age)<.0001f);
    Zombie cold={.kind=Z_NORMAL,.speed=.3f,.slow=100},warm={.kind=Z_NORMAL,.speed=.3f};
    float cold_d=game_zombie_walk(&cold,cycle*2),warm_d=game_zombie_walk(&warm,cycle);
    CHECK("snow slows footsteps and travel together by half",fabsf(cold_d-warm_d)<.001f&&fabsf(cold.age-warm.age)<.0001f);
    walker=(Zombie){.kind=Z_NORMAL,.speed=.3f};float least=100,most=0;
    for(int i=0;i<400;i++){float step=game_zombie_walk(&walker,.01f);least=fminf(least,step);most=fmaxf(most,step);}
    CHECK("walking follows nonuniform original footsteps rather than gliding",most>least+.01f);
    Zombie runner={.kind=Z_POLE,.speed=.67f},after={.kind=Z_POLE,.vaulted=1,.speed=.3f},flag={.kind=Z_FLAG,.speed=.45f};
    float run_cycle=motion[1].x[motion[1].count-1]/(.67f*47),post_cycle=motion[2].x[motion[2].count-1]/(.3f*47),flag_cycle=motion[0].x[motion[0].count-1]/(.45f*47);
    float run_pace=game_zombie_walk(&runner,run_cycle)/run_cycle,post_pace=game_zombie_walk(&after,post_cycle)/post_cycle,flag_pace=game_zombie_walk(&flag,flag_cycle)/flag_cycle;
    CHECK("pole vaulter runs faster and returns to ordinary pace after vault",run_pace>15&&run_pace<16&&post_pace>6.8f&&post_pace<6.9f);
    CHECK("flag zombie retains its original faster gait",flag_pace>10.2f&&flag_pace<10.3f);
    game_start_level(&g,9,70);g.spawn_timer=10000;z=game_spawn_zombie(&g,2,Z_NORMAL,400);g.zombies[z].speed=.3f;g.zombies[z].age=.61f;g.level_flags|=LEVEL_READY;
    saved=game_save(&g,"selftest-save.dat");
    CHECK("new save preserves walking phase and corrected speed",saved&&game_load(&restored,"selftest-save.dat")&&restored.zombies[z].age==.61f&&restored.zombies[z].speed==.3f);remove("selftest-save.dat");
    g.zombies[z].speed=14.25f;g.zombies[z].age=7;
    int legacy_written=legacy_fixture(&g,"selftest-save.dat",3);
    CHECK("v0.3 to v0.4.1 saves migrate movement without resetting level or position",legacy_written&&game_load(&restored,"selftest-save.dat")&&restored.level==9&&restored.zombies[z].x==400&&fabsf(restored.zombies[z].speed-.3f)<.00001f&&restored.zombies[z].age==0&&(restored.level_flags&LEVEL_READY));remove("selftest-save.dat");
    game_start_level(&g,11,101);g.spawn_timer=10000;g.sky_timer=.01f;advance(&g,10);
    CHECK("night has no falling sky sun",g.sky_count==0&&game_collect(&g)==0&&g.num_waves==10);
    memset(g.graves,0,sizeof(g.graves));g.seed_mask|=1<<PUFF;g.sun=0;
    CHECK("Puff-shroom plants without spending sun",game_place(&g,2,1,PUFF)==0&&g.sun==0);
    z=game_spawn_zombie(&g,2,Z_NORMAL,plant_x(1)+CELL_W*4);advance(&g,1);
    CHECK("Puff-shroom holds fire beyond short range",g.zombies[z].hp==270);
    g.zombies[z].x=plant_x(1)+CELL_W*2;advance(&g,2);CHECK("Puff-shroom damages a nearby zombie",g.zombies[z].hp<270);
    game_start_level(&g,14,102);g.spawn_timer=10000;g.sun=1000;g.seed_mask|=1<<GRAVEBUSTER;g.graves[2][4]=1;
    CHECK("ordinary plants cannot occupy graves",game_place(&g,2,4,PEA)==-10);
    CHECK("Grave Buster requires an existing grave",game_place(&g,2,0,GRAVEBUSTER)==-10);
    game_place(&g,2,4,GRAVEBUSTER);advance(&g,4.6f);CHECK("Grave Buster consumes its grave",!g.graves[2][4]&&g.plants[2][4].kind==-1);
    game_start_level(&g,12,103);g.spawn_timer=10000;g.sun=1000;g.seed_mask|=1<<SUNSHROOM;memset(g.graves,0,sizeof(g.graves));game_place(&g,2,1,SUNSHROOM);g.plants[2][1].attack=.01f;advance(&g,.1f);int original_sun=g.sun;game_collect(&g);game_animate_pickups(&g,1);
    CHECK("young Sun-shroom produces fifteen sun",g.sun==original_sun+15);
    g.plants[2][1].age=121;g.plants[2][1].attack=.01f;advance(&g,.1f);original_sun=g.sun;game_collect(&g);game_animate_pickups(&g,1);
    CHECK("grown Sun-shroom produces twenty-five sun",g.sun==original_sun+25);
    game_start_level(&g,16,104);g.spawn_timer=10000;g.sun=1000;g.seed_mask|=(1<<FUME)|(1<<HYPNO);memset(g.graves,0,sizeof(g.graves));game_place(&g,2,1,FUME);
    z=game_spawn_zombie(&g,2,Z_DOOR,plant_x(1)+60);int behind=game_spawn_zombie(&g,2,Z_NORMAL,plant_x(1)+85);advance(&g,.5f);
    CHECK("fumes penetrate door and hit zombies behind it",g.zombies[z].hp==250&&g.zombies[z].armor==1100&&g.zombies[behind].hp==250);
    game_start_level(&g,16,105);g.spawn_timer=10000;g.sun=1000;g.seed_mask|=1<<HYPNO;memset(g.graves,0,sizeof(g.graves));game_place(&g,2,1,HYPNO);z=game_spawn_zombie(&g,2,Z_NORMAL,plant_x(1)+20);g.plants[2][1].hp=1;advance(&g,.1f);float previous_x=g.zombies[z].x;advance(&g,.5f);
    CHECK("eaten Hypno-shroom turns zombie away from house",g.hypnotized[z]&&g.zombies[z].x>previous_x&&game_alive(&g)==0);
    game_start_level(&g,18,106);g.spawn_timer=10000;g.sun=1000;g.seed_mask|=1<<ICESHROOM;memset(g.graves,0,sizeof(g.graves));game_place(&g,2,1,ICESHROOM);z=game_spawn_zombie(&g,3,Z_NORMAL,400);advance(&g,1.1f);previous_x=g.zombies[z].x;advance(&g,1);
    CHECK("Ice-shroom freezes other lanes without movement",g.frozen[z]>2&&g.zombies[z].x==previous_x&&g.zombies[z].slow>10);
    game_start_level(&g,19,107);g.spawn_timer=10000;g.sun=1000;g.seed_mask|=1<<DOOM;memset(g.graves,0,sizeof(g.graves));game_place(&g,2,2,DOOM);game_spawn_zombie(&g,1,Z_BUCKET,plant_x(2)+40);advance(&g,1.1f);
    CHECK("Doom-shroom leaves a blocking crater after its blast",g.kills==1&&g.craters[2][2]>178&&game_place(&g,2,2,PEA)==-9);
    z=game_spawn_zombie(&g,4,Z_PAPER,400);previous_x=g.zombies[z].speed;hurt_zombie(&g,&g.zombies[z],150,0);
    CHECK("breaking newspaper speeds up its owner",g.zombies[z].armor==0&&g.zombies[z].speed>previous_x*2);
    z=game_spawn_zombie(&g,4,Z_FOOTBALL,450);CHECK("football starts with original fourteen-hundred helmet health",g.zombies[z].armor==1400);
    game_start_level(&g,18,108);g.spawn_timer=10000;z=game_spawn_zombie(&g,2,Z_DANCER,400);advance(&g,3.1f);int backups=0;for(int j=0;j<MAX_ZOMBIES;j++)backups+=g.zombies[j].used&&g.zombies[j].kind==Z_BACKUP;
    CHECK("dancer summons its four backup positions",backups==4);
    game_start_minigame(&g,1,109);g.spawn_timer=0;advance(&g,.1f);int whacked=0;for(int j=0;j<MAX_ZOMBIES;j++)if(g.zombies[j].used)whacked=game_whack(&g,g.zombies[j].row,(int)((g.zombies[j].x-GRID_X)/CELL_W));
    CHECK("Whack a Zombie strikes a grave spawn and awards sun",g.mode==MODE_WHACK&&whacked&&g.kills==1&&g.sun==15);
    saved=game_save(&g,"selftest-night.dat");CHECK("night mechanics survive a save roundtrip",saved&&game_load(&restored,"selftest-night.dat")&&restored.level==15&&restored.challenge==2&&!memcmp(g.graves,restored.graves,sizeof(g.graves)));remove("selftest-night.dat");
    game_start_level(&g,5,110);g.packets[0]=8;legacy_written=legacy_fixture(&g,"selftest-legacy4.dat",4);
    CHECK("0.5.2 save imports red packet without confusing Puff-shroom",legacy_written&&game_load(&restored,"selftest-legacy4.dat")&&restored.packets[0]==EXPLODE_NUT&&restored.night_cost[1]==25&&!restored.graves[2][4]);remove("selftest-legacy4.dat");
    #include "chapter_tests.inc"
    #include "interaction_tests.inc"
    #include "upgrade_tests.inc"
    #include "polish_tests0190.inc"
    #include "completion_tests.inc"
    #include "fidelity_tests.inc"
    fprintf(out,"RESULT %d/%d\n",passed,total);fflush(out);
    #undef CHECK
    return passed==total;
}

#include "minigame_tests.inc"

#include "extra_tests.inc"

#include "audio_event_tests018.inc"
