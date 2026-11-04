#ifndef PVZ_PSP_GAME_H
#define PVZ_PSP_GAME_H
#include <stdint.h>
#include <stdio.h>
#include "minigame_state.h"
#include "extra_state.h"
#include "foley_defs.h"
/* Transient presentation hook, excluded from every saved Game layout. */
void game_sound_listener(void (*listener)(int event));
#define GAME_SAMPLE(sample) (FOLEY_COUNT+(sample))

#define ROWS 6
#define COLS 9
#define SEEDS 8
#define ALL_SEEDS 49
#define BASE_SEEDS 40
#define MAX_COINS 32
#define MAX_ZOMBIES 96
#define MAX_SHOTS 256
#define MAX_SUNS 64
#define GRID_X 62
#define GRID_Y 58
#define CELL_W 38
#define CELL_H 36
#define MINE_ARM_SECONDS 15.0f
#define ZOMBIE_BITE_FRONT 23.0f
#define ZOMBIE_BITE_BACK 9.0f
#define OPENING_WAVES 4
#define CAMPAIGN_LEVELS 50
#define MAX_WAVES 40
#define WAVE_CAPACITY 64
#define MAX_PACKETS 10
#define MAX_NUTS 32

enum { PEA, SUNFLOWER, WALLNUT, CHERRY, MINE, SNOW, CHOMPER, REPEATER, PUFF, SUNSHROOM, FUME, GRAVEBUSTER, HYPNO, SCAREDY, ICESHROOM, DOOM, LILY, SQUASH, THREEPEATER, KELP, JALAPENO, SPIKEWEED, TORCHWOOD, TALLNUT, SEA, PLANTERN, CACTUS, BLOVER, SPLIT, STAR, PUMPKIN, MAGNET, CABBAGE, POT, KERNEL, COFFEE, GARLIC, UMBRELLA, MARIGOLD, MELON, GATLING, TWIN, GLOOM, CATTAIL, WINTER, GOLDMAGNET, SPIKEROCK, COB, IMITATER };
enum { Z_NORMAL, Z_CONE, Z_BUCKET, Z_POLE, Z_FLAG, Z_PAPER, Z_DOOR, Z_FOOTBALL, Z_DANCER, Z_BACKUP, Z_DUCK, Z_SNORKEL, Z_ZOMBONI, Z_BOBSLED, Z_DOLPHIN, Z_JACK, Z_BALLOON, Z_DIGGER, Z_POGO, Z_YETI, Z_BUNGEE, Z_LADDER, Z_CATAPULT, Z_GARG, Z_IMP, Z_BOSS, Z_TYPES };
enum { MODE_LAWN, MODE_BOWLING, MODE_CONVEYOR, MODE_WHACK, MODE_VASE, MODE_BUNGEE, MODE_BOSS, MODE_MATCH, MODE_AQUARIUM, MODE_HEAVY, MODE_SLOT, MODE_IZOMBIE };
#define EXPLODE_NUT 49
#define GIANT_NUT 50
#define LEFT_PACKET 51
enum { PLAYING, WON, LOST };
enum { REWARD_SEED,REWARD_SHOVEL,REWARD_ALMANAC,REWARD_KEYS,REWARD_TACO,REWARD_WATERING_CAN,REWARD_NOTE,REWARD_TROPHY,REWARD_SILVER_SUNFLOWER };
enum { LEVEL_READY=1, LEVEL_TUTORIAL_COMPLETE=2, LEVEL_REWARD_COLLECTED=4 };
enum { SFX_PLANT, SFX_SHOOT, SFX_SUN, SFX_CHOMP, SFX_EXPLODE, SFX_ZOMBIE, SFX_MOWER, SFX_WIN, SFX_COUNT };

typedef struct { int kind; float hp, age, attack, flash, digest; } Plant;
typedef struct { int used, row, kind, eating; float x, hp, armor, age, slow, death; int wave;
    int vaulted; float vault, vault_from, vault_to, speed; } Zombie;
typedef struct { int used, row, ice; float x; } Shot;
typedef struct { int used; float x, y, target_y, age; float vx,vy; int from_plant; float flight; } Sun;
typedef struct { int used,explosive,row,target_row,last_hit,direction; float x,y,age; } BowlingNut;
typedef struct { int used,value;float x,y,age,vx,vy,target_y; } Coin;
typedef struct { int kind;float x,y,rotation,alpha; } HeadParticle;
typedef struct {
    uint32_t rng;
    int status, sun, wave, spawned, kills, total_spawned, selected, shovel;
    int cursor_row, cursor_col, mower_used[ROWS], events[SFX_COUNT];
    float time, spawn_timer, sky_timer, wave_delay, flash, cooldown[SEEDS], mower_x[ROWS];
    int seed_cost[SEEDS], seed_refresh[SEEDS];
    Plant plants[ROWS][COLS];
    Zombie zombies[MAX_ZOMBIES];
    Shot shots[MAX_SHOTS];
    Sun suns[MAX_SUNS];
    float wave_health_threshold;
    int level,num_waves,row_mask;uint64_t unlocked_mask,seed_mask;int mode;
    int wave_count[MAX_WAVES];uint8_t wave_types[MAX_WAVES][WAVE_CAPACITY];
    int row_history[ROWS],row_second[ROWS],row_mowed_wave[ROWS],packets[MAX_PACKETS],packet_count,last_packet,sky_count;
    float conveyor_timer,huge_wave_timer;
    int huge_wave_announced;
    BowlingNut nuts[MAX_NUTS];
    int shovel_tutorial,level_flags;
    /* Version 6 imports the old layouts explicitly; see legacy_game.h. */
    float night_cooldown[41];int night_cost[41],night_refresh[41];
    uint8_t graves[ROWS][COLS],hypnotized[MAX_ZOMBIES];
    float craters[ROWS][COLS],frozen[MAX_ZOMBIES],special[MAX_ZOMBIES];
    int challenge,whack_goal;
    float shot_remaining[MAX_SHOTS];
    float shot_y[MAX_SHOTS],shot_vx[MAX_SHOTS],shot_vy[MAX_SHOTS],shot_damage[MAX_SHOTS];int shot_target[MAX_SHOTS];
    int support[ROWS][COLS],vases[ROWS][COLS],asleep[ROWS][COLS],ladders[ROWS][COLS];
    float support_hp[ROWS][COLS],pumpkin_hp[ROWS][COLS],ice_tiles[ROWS][COLS];
    float fog_clear,boss_hp,boss_timer,boss_phase,boss_ball_x;int boss_row,boss_ball_kind,stage,coins,boss_ball_row;
    uint64_t purchased_mask;int seed_slots,imitater_seed,cob_row,cob_col,cob_aiming,store_flags;
    uint32_t session_id,money_collected,seen_zombies;Coin money[MAX_COINS];uint8_t imitated[ROWS][COLS];int morph_kind[ROWS][COLS];float morph_timer[ROWS][COLS];float burst[ROWS][COLS];int burst_left[ROWS][COLS];
    int rake_row,rake_col,rake_state;float rake_timer;int burst_target[ROWS][COLS];float morph_hp[ROWS][COLS];

    MiniState mini;float pea_shoot_time[ROWS][COLS];
    ExtraState extra;
} Game;

extern const char *seed_names[ALL_SEEDS];
int game_seed_cost(const Game *g,int kind);
int game_seed_refresh(const Game *g,int kind);
float *game_seed_cooldown(Game *g,int kind);
int game_is_night(const Game *g);
int game_world(const Game *g);
int game_cleaner_active(const Game *g,int row);
int game_rows(const Game *g);
float game_row_height(const Game *g);
float game_ground_y(const Game *g,int row);
float game_roof_offset(const Game *g,float x);
int game_water_row(const Game *g,int row);
int game_zombie_allowed(int level,int kind);
void game_add_coin(Game *g,float x,float y,int value);
int game_collect_coins(Game *g);
int game_cob_select(Game *g,int row,int col);
int game_cob_fire(Game *g,int row,int col);
int game_upgrade_base(int kind);
int game_break_vase(Game *g,int row,int col);
int game_music_theme(const Game *g);
int game_reward_type(const Game *g);
int game_reward_seed(const Game *g);
int game_collect_reward(Game *g);
int game_dying_count(const Game *g);
void game_animate_deaths(Game *g,float dt);
void game_animate_attacks(Game *g,float dt);
float game_death_duration(const Zombie *z);
float game_death_alpha(const Zombie *z);
int game_whack(Game *g,int row,int col);
void game_start_minigame(Game *g,int kind,uint32_t seed);
void game_start_mini(Game *g,int id,uint32_t seed);
int game_mini_action(Game *g,int action);
int game_mini_total(const Game *g);
void game_heavy_control(Game *g,float move,float aim,int fire,float dt);
int game_mini_active(const Game *g,int id);
extern const char *mini_names[20];
int game_mini_selftest(FILE *out);
void game_start_survival(Game*,int terrain,int difficulty,uint32_t seed);
void game_start_puzzle(Game*,int family,int selection,uint32_t seed);
int game_extra_action(Game*,int action);
int game_extra_continue(Game*);
int game_extra_selftest(FILE*);
int game_zombie_cost(int kind);
extern const char *puzzle_names[20];
void game_init(Game *g, uint32_t seed);
void game_start_level(Game *g,int level,uint32_t seed);
int game_next_level(Game *g);
int game_row_active(const Game *g,int row);
int game_seed_available(const Game *g,int kind);
int game_flag_wave(const Game *g,int wave_index);
float game_sunflower_glow(const Plant *p);
int game_selected_kind(const Game *g);
void game_cycle_seed(Game *g,int direction);
void game_step(Game *g, float dt);
int game_place(Game *g, int row, int col, int kind);
void game_shovel(Game *g, int row, int col);
int game_collect(Game *g);
void game_animate_pickups(Game *g,float dt);
int game_spawn_zombie(Game *g, int row, int kind, float x);
int game_alive(const Game *g);
int game_zombie_body_stage(const Zombie *z);
int game_head_particle(const Game *g,int index,HeadParticle *out);
int game_zombie_armor_stage(const Zombie *z);
int game_sunflower_count(const Game *g);
int game_load_motion(const char *filename);
float game_zombie_animation_time(const Zombie *z);
float game_zombie_walk(Zombie *z,float dt);
int game_selftest(FILE *out);
int game_audio018_selftest(FILE*);
int game_campaign_selftest(FILE *out);
int game_save(const Game *g, const char *filename);
int game_load(Game *g, const char *filename);
#endif
