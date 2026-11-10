#include "game.h"
#include "render.h"
#include "audio.h"
#include "music.h"
#include "progress.h"
#include "profile.h"
#include "garden.h"
#include "textdb.h"
#include "disc_font.h"
#include "movie.h"
#include <time.h>
#include "platform.h"
#include <pspkernel.h>
#include <pspctrl.h>
#include <psppower.h>
#include <psprtc.h>
#include <pspsysmem.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

PSP_MODULE_INFO("Plants vs. Zombies",0,0,18);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER|THREAD_ATTR_VFPU);
/* libc reserves the heap before main/model detection. Size it to the actual
   user partition, leaving room for audio/movie and callback thread stacks. */
PSP_HEAP_SIZE_KB(-1);
PSP_HEAP_THRESHOLD_SIZE_KB(1024);
PSP_MAIN_THREAD_STACK_SIZE_KB(512);
#ifdef TEST_PSP1000_60FPS
#define BUILD018_LABEL "PvZ PSP prototype 0.18.1 PSP1000 60 FPS EXPERIMENT"
#else
#define BUILD018_LABEL "PvZ PSP 0.21.0 development - completion work"
#endif

enum { A_PEA,A_PEA_SHOOT,A_SUNFLOWER,A_WALLNUT,A_CHERRY,A_CHERRY_EXPLODE,A_MINE,A_MINE_UNARMED,A_SNOW,A_SNOW_SHOOT,
       A_CHOMPER,A_CHOMPER_BITE,A_CHOMPER_CHEW,A_ZOMBIE,A_ZOMBIE_EAT,A_ZOMBIE_DEATH,A_CONE,A_CONE_EAT,A_BUCKET,A_BUCKET_EAT,
       A_FLAG_WALK,A_POLE_RUN,A_POLE_JUMP,A_POLE_WALK,A_POLE_EAT,A_POLE_DEATH,A_MOWER,A_SUN,A_BACKGROUND,A_BACKGROUND_ONE,A_BACKGROUND_THREE,A_NOTE,A_PROJECTILE_PEA,A_PROJECTILE_SNOW,
       A_REPEATER,A_REPEATER_SHOOT,A_FLAG_EAT,A_MENU_BG,A_MENU_ADVENTURE,A_MENU_START,A_MENU_MORE,A_MENU_EXTRAS,A_HUGE_WAVE,A_TITLE_REPLAY,A_TITLE_OPTIONS,A_TITLE_CONTROLS,A_TITLE_SEEDS,
       A_SEEDBANK,A_SEED_PACKET,A_CONVEYOR_STRIP,A_CONVEYOR_FRAME,A_METER_EMPTY,A_METER_FULL,A_METER_HEAD,A_METER_FLAG,A_SHOVEL,
       A_PUFF,A_PUFF_SHOOT,A_SUNSHROOM,A_SUNSHROOM_BIG,A_FUME,A_FUME_SHOOT,A_GRAVEBUSTER,A_HYPNO,A_SCAREDY,A_SCAREDY_SHOOT,A_SCAREDY_HIDE,A_ICESHROOM,A_DOOM,
       A_PAPER,A_PAPER_EAT,A_PAPER_DEATH,A_FOOTBALL,A_FOOTBALL_EAT,A_FOOTBALL_DEATH,A_DANCER,A_DANCER_EAT,A_DANCER_DEATH,A_BACKUP,A_BACKUP_EAT,A_BACKUP_DEATH,
       A_PAPER_ANGRY,A_DOOR,A_PAPER_ANGRY_EAT,A_DOOR_EAT,A_NIGHT,A_NOTE1,A_NOTE2,A_NOTE3,A_NOTE4,A_NOTE5,A_GRAVE,A_CRATER,A_SPORE,A_LILY,A_SQUASH,A_THREEPEATER,A_KELP,A_JALAPENO,A_SPIKEWEED,A_TORCHWOOD,A_TALLNUT,A_SEA,A_PLANTERN,A_CACTUS,A_BLOVER,A_SPLIT,A_STAR,A_PUMPKIN,A_MAGNET,A_CABBAGE,A_POT,A_KERNEL,A_COFFEE,A_GARLIC,A_UMBRELLA,A_MARIGOLD,A_MELON,A_DUCK_WALK,A_DUCK_EAT,A_DUCK_DEATH,A_SNORKEL_WALK,A_SNORKEL_EAT,A_SNORKEL_DEATH,A_ZAMBONI_WALK,A_ZAMBONI_EAT,A_ZAMBONI_DEATH,A_BOBSLED_WALK,A_BOBSLED_EAT,A_BOBSLED_DEATH,A_DOLPHIN_WALK,A_DOLPHIN_EAT,A_DOLPHIN_DEATH,A_JACK_WALK,A_JACK_EAT,A_JACK_DEATH,A_BALLOON_WALK,A_BALLOON_EAT,A_BALLOON_DEATH,A_DIGGER_WALK,A_DIGGER_EAT,A_DIGGER_DEATH,A_POGO_WALK,A_POGO_EAT,A_POGO_DEATH,A_YETI_WALK,A_YETI_EAT,A_YETI_DEATH,A_BUNGEE_WALK,A_BUNGEE_EAT,A_BUNGEE_DEATH,A_LADDER_WALK,A_LADDER_EAT,A_LADDER_DEATH,A_CATAPULT_WALK,A_CATAPULT_EAT,A_CATAPULT_DEATH,A_GARG_WALK,A_GARG_EAT,A_GARG_DEATH,A_IMP_WALK,A_IMP_EAT,A_IMP_DEATH,A_BOSS_WALK,A_BOSS_EAT,A_BOSS_DEATH,A_BACKGROUND_POOL,A_BACKGROUND_FOG,A_BACKGROUND_ROOF,A_BACKGROUND_BOSS,A_PROJECTILE_CABBAGE,A_PROJECTILE_KERNEL,A_PROJECTILE_BUTTER,A_PROJECTILE_MELON,A_PROJECTILE_STAR,A_PROJECTILE_CACTUS,A_VASE,A_POOL_CLEANER,A_ROOF_CLEANER,A_FOG_CLOUD,A_BOSS_FIREBALL,A_BOSS_ICEBALL,A_REWARD_TROPHY,A_REWARD_SILVER_SUNFLOWER,A_REWARD_SHOVEL,A_REWARD_ALMANAC,A_REWARD_KEYS,A_REWARD_TACO,A_REWARD_WATERING_CAN,A_REWARD_NOTE,A_GATLING,A_GATLING_SHOOT,A_TWIN,A_GLOOM,A_GLOOM_SHOOT,A_CATTAIL,A_CATTAIL_SHOOT,A_WINTER,A_WINTER_SHOOT,A_GOLDMAGNET,A_SPIKEROCK,A_COB,A_COB_UNARMED,A_COB_SHOOT,A_IMITATER,A_IMITATER_MORPH,A_COIN_SILVER,A_COIN_GOLD,A_COIN_DIAMOND,A_PROJECTILE_WINTER,A_PROJECTILE_COB,A_PROJECTILE_CATTAIL,A_STORE_BACKGROUND,A_ALMANAC_BACKGROUND,A_RAKE,A_SEED_CARDS,A_PLANT_AWARDS,A_AWARD_RAYS,A_MINI_BACKGROUND,A_AQUARIUM_BACKGROUND,A_SLOT_OVERLAY,A_PEA_BODY,A_PEA_HEAD,A_PEA_FIRE,A_REPEATER_BODY,A_REPEATER_HEAD,A_REPEATER_FIRE,A_SNOW_BODY,A_SNOW_HEAD,A_SNOW_FIRE,A_PORTAL_CIRCLE,A_PORTAL_SQUARE,A_AQUARIUM_SWIM,A_BRAIN,A_BOT_WALK,A_BOT_EAT,A_HEAVY_TANK,A_HEAVY_TURRET,A_GARDEN_BG,A_MUSHROOM_GARDEN,A_GARDEN_NEEDS,A_DIALOGUE_CARDS,A_WISDOM_CARDS,A_DAVE_IDLE,A_DAVE_TALK,A_DAVE_ENTER,A_DAVE_LEAVE,A_GARDEN_SPROUT,A_SNAIL_WALK,A_TREE_STAGES,A_DISC_FONT,A_DAVE_GRAB,A_HEAVY_CATTAIL,A_BOBSLED_SEATED,A_BOBSLED_ONFOOT,A_BOSS_HEAD_ENTER,A_BOSS_HEAD_LEAVE,A_BOSS_RV,A_BOSS_BUNGEE,A_BOSS_STOMP1,A_BOSS_STOMP2,A_BOSS_STOMP3,A_BOSS_STOMP4,A_BOSS_SPAWN1,A_BOSS_SPAWN2,A_BOSS_SPAWN3,A_BOSS_SPAWN4,A_BOSS_SPAWN5,A_BOSS_SPIT1,A_BOSS_SPIT2,A_BOSS_SPIT3,A_BOSS_SPIT4,A_BOSS_SPIT5,A_PRESENT,A_CHOCOLATE,A_SLED,A_MENU_MAIN,A_MENU_MODES,A_MENU_COLLECTION,A_UI_DIALOG,A_UI_BUTTON,A_UI_SLIDER,A_UI_KNOB,A_CHALLENGE_FRAMES,A_CHALLENGE_THUMBS,A_BOOK_PAGES,A_SEED_CHOOSER,A_SHOP_SCENE,A_SHOP_BUBBLE,A_SHOP_ICONS,A_IMPACT_DUST,A_NOTE_PAPER,A_SQUASH_LEFT,A_SQUASH_RIGHT,A_SQUASH_UP,A_SQUASH_DOWN,A_AWARD_GLOW,A_AWARD_ARROW,A_AWARD_BACKGROUND,A_CHALLENGE_LOCK,A_CHALLENGE_OPEN,A_GARDEN_BARROW,A_GARDEN_FERTILIZER,A_GARDEN_GLOVE,A_GARDEN_GOLDWATER,A_GARDEN_KELPAQUA,A_GARDEN_MARIGOLD_BODY,A_GARDEN_MARIGOLD_FULL,A_GARDEN_MARIGOLD_PETALS,A_GARDEN_MUSIC,A_GARDEN_POT,A_GARDEN_SEAAQUA,A_GARDEN_SLEEP_DOOMSHROOM,A_GARDEN_SLEEP_FUMESHROOM,A_GARDEN_SLEEP_GLOOMSHROOM,A_GARDEN_SLEEP_HYPNOSHROOM,A_GARDEN_SLEEP_ICESHROOM,A_GARDEN_SLEEP_MAGNETSHROOM,A_GARDEN_SLEEP_PUFFSHROOM,A_GARDEN_SLEEP_SCAREDYSHROOM,A_GARDEN_SLEEP_SEASHROOM,A_GARDEN_SLEEP_SUNSHROOM,A_GARDEN_SPRAY,A_GARDEN_SPROUT_PLAIN,A_GARDEN_SUNBIG,A_GARDEN_SUNBIGSLEEP,A_GARDEN_WATER,A_GARDEN_WATERPOT,A_GARDEN_Z,A_SNAIL_IDLE,A_SNAIL_IN,A_SNAIL_OUT,A_SNAIL_TURN,A_GARG_THROW,A_IMP_THROWN,A_IMP_LAND,A_JACK_POP,A_PAPER_GASP,A_DOLPHIN_JUMP,A_ZOMBIE_CHARRED,A_ZOMBIE_WATERDEATH,A_GARG_WALK_NOIMP,A_GARG_EAT_NOIMP,A_GARG_DEATH_NOIMP,A_GARG_THROW_NOIMP,A_CLASSIC_DANCER,A_CLASSIC_DANCER_EAT,A_CLASSIC_DANCER_DEATH,A_CLASSIC_DANCER_HEAD,A_CROW_IDLE,A_CROW_IDLE2,A_CROW_IDLE3,A_CROW_TALK,A_CROW_FLY,A_CROW_LAND,A_GIGA_WALK,A_GIGA_WALK_NOIMP,A_GIGA_EAT,A_GIGA_EAT_NOIMP,A_GIGA_DEATH,A_GIGA_DEATH_NOIMP,A_GIGA_THROW,A_GIGA_THROW_NOIMP,A_MOWER_TRICKED,A_REWARD_GOLD_SUNFLOWER,A_REWARD_MONEYBAG,A_HOUSE_PREVIEWS,A_SECRET_FUTURE,A_SECRET_MUSTACHE,A_SECRET_DAISY,A_SECRET_PINATA,A_ZOMBIE_DANCE,A_CONE_DANCE,A_BUCKET_DANCE,A_PUMPKIN_BACK,A_PUMPKIN_FRONT,A_COUNT };
static const char *asset_names[A_COUNT]={"pea_idle","pea_shoot","sunflower","wallnut","cherry","cherry_explode","mine","mine_unarmed","snow_idle","snow_shoot","chomper","chomper_bite","chomper_chew","zombie_walk","zombie_eat","zombie_death","cone_walk","cone_eat","bucket_walk","bucket_eat","flag_walk","pole_run","pole_jump","pole_walk","pole_eat","pole_death","mower","sun","background","background_one","background_three","note","projectile_pea","projectile_snow",
       "repeater_idle","repeater_shoot","flag_eat","menu_background","menu_adventure","menu_start","menu_more","menu_extras","huge_wave","title_replay","title_options","title_controls","title_seeds",
       "seedbank","seed_packet","conveyor_strip","conveyor_frame","meter_empty","meter_full","meter_head","meter_flag","shovel",
       "puff","puff_shoot","sunshroom","sunshroom_big","fume","fume_shoot","gravebuster","hypno","scaredy","scaredy_shoot","scaredy_hide","iceshroom","doom",
       "paper_walk","paper_eat","paper_death","football_walk","football_eat","football_death","dancer_walk","dancer_eat","dancer_death","backup_walk","backup_eat","backup_death",
       "paper_angry_walk","door_walk","paper_angry_eat","door_eat","background_night","note1","note2","note3","note4","note5","gravestone","crater","projectile_spore","lily","squash","threepeater","kelp","jalapeno","spikeweed","torchwood","tallnut","sea","plantern","cactus","blover","split","star","pumpkin","magnet","cabbage","pot","kernel","coffee","garlic","umbrella","marigold","melon","duck_walk","duck_eat","duck_death","snorkel_walk","snorkel_eat","snorkel_death","zamboni_walk","zamboni_eat","zamboni_death","bobsled_walk","bobsled_eat","bobsled_death","dolphin_walk","dolphin_eat","dolphin_death","jack_walk","jack_eat","jack_death","balloon_walk","balloon_eat","balloon_death","digger_walk","digger_eat","digger_death","pogo_walk","pogo_eat","pogo_death","yeti_walk","yeti_eat","yeti_death","bungee_walk","bungee_eat","bungee_death","ladder_walk","ladder_eat","ladder_death","catapult_walk","catapult_eat","catapult_death","garg_walk","garg_eat","garg_death","imp_walk","imp_eat","imp_death","boss_walk","boss_eat","boss_death","background_pool","background_fog","background_roof","background_boss","projectile_cabbage","projectile_kernel","projectile_butter","projectile_melon","projectile_star","projectile_cactus","vase","pool_cleaner","roof_cleaner","fog_cloud","boss_fireball","boss_iceball","reward_trophy","reward_silver_sunflower","reward_shovel","reward_almanac","reward_keys","reward_taco","reward_watering_can","reward_note","gatling","gatling_shoot","twin","gloom","gloom_shoot","cattail","cattail_shoot","winter","winter_shoot","goldmagnet","spikerock","cob","cob_unarmed","cob_shoot","imitater","imitater_morph","coin_silver","coin_gold","coin_diamond","projectile_winter","projectile_cob","projectile_cattail","store_background","almanac_background","rake","seed_cards","plant_awards","award_rays","mini_background","aquarium_background","slot_overlay","pea_body","pea_head","pea_fire","repeater_body","repeater_head","repeater_fire","snow_body","snow_head","snow_fire","portal_circle","portal_square","aquarium_swim","brain","bot_walk","bot_eat","heavy_tank","heavy_turret","garden_background","mushroom_garden","garden_needs","dialogue_cards","wisdom_cards","dave_idle","dave_talk","dave_enter","dave_leave","garden_sprout","snail_walk","tree_stages","disc_font","dave_grab","heavy_cattail","bobsled_seated","bobsled_onfoot","boss_head_enter","boss_head_leave","boss_rv","boss_bungee","boss_stomp1","boss_stomp2","boss_stomp3","boss_stomp4","boss_spawn1","boss_spawn2","boss_spawn3","boss_spawn4","boss_spawn5","boss_spit1","boss_spit2","boss_spit3","boss_spit4","boss_spit5","present","chocolate","sled","menu_main","menu_modes","menu_collection","ui_dialog","ui_button","ui_slider","ui_knob","challenge_frames","challenge_thumbs","book_pages","seed_chooser","shop_scene","shop_bubble","shop_icons","impact_dust","note_paper","squash_left","squash_right","squash_up","squash_down","award_glow","award_arrow","award_background","challenge_lock","challenge_open","garden_barrow","garden_fertilizer","garden_glove","garden_goldwater","garden_kelpaqua","garden_marigold_body","garden_marigold_full","garden_marigold_petals","garden_music","garden_pot","garden_seaaqua","garden_sleep_doomshroom","garden_sleep_fumeshroom","garden_sleep_gloomshroom","garden_sleep_hypnoshroom","garden_sleep_iceshroom","garden_sleep_magnetshroom","garden_sleep_puffshroom","garden_sleep_scaredyshroom","garden_sleep_seashroom","garden_sleep_sunshroom","garden_spray","garden_sprout_plain","garden_sunbig","garden_sunbigsleep","garden_water","garden_waterpot","garden_z","snail_idle","snail_in","snail_out","snail_turn","garg_throw","imp_thrown","imp_land","jack_pop","paper_gasp","dolphin_jump","zombie_charred","zombie_waterdeath","garg_walk_noimp","garg_eat_noimp","garg_death_noimp","garg_throw_noimp","classic_dancer_walk","classic_dancer_eat","classic_dancer_death","head_classic_dancer","menu_crow_idle","menu_crow_idle2","menu_crow_idle3","menu_crow_talk","menu_crow_fly","menu_crow_land","giga_walk","giga_walk_noimp","giga_eat","giga_eat_noimp","giga_death","giga_death_noimp","giga_throw","giga_throw_noimp","mower_tricked","reward_gold_sunflower","reward_moneybag","house_previews","secret_future","secret_mustache","secret_daisy","secret_pinata","zombie_dance","cone_dance","bucket_dance","pumpkin_back","pumpkin_front"};
static Animation assets[A_COUNT],damaged[A_COUNT][2],imitated_assets[A_COUNT][3];static Game game;static volatile int running=1;
static int screen,paused,help,gallery,ready,can_resume,debug_info;static char message[128];static float message_time;
static int review_ui_only;
static float shoot_alignment[3][80][2];
static Animation head_particles[5];
static const char *head_particle_names[5]={"head_normal","head_pole","head_football","head_dancer","head_backup"};
static int load_shoot_alignment(void){
    FILE *f=fopen("data/shoot-alignment.bin","rb");if(!f)return 0;
    uint32_t h[4];int ok=fread(h,sizeof(h),1,f)==1&&h[0]==0x31415050&&h[1]==3&&h[2]==80&&h[3]==60&&fread(shoot_alignment,sizeof(shoot_alignment),1,f)==1&&fgetc(f)==EOF;fclose(f);
    for(int k=0;ok&&k<3;k++)for(int n=0;n<80;n++)for(int j=0;j<2;j++)if(!isfinite(shoot_alignment[k][n][j])||fabsf(shoot_alignment[k][n][j])>64)ok=0;
    return ok;
}
static int live_benchmark,live_phase;
static int test_ok;static uint32_t previous_buttons;static float repeat_timer;static size_t texture_memory;
static RenderStats last_render;
static int menu_volume_row,volume_dirty;static float volume_repeat;
static int more_cursor,mini_cursor,note_cursor;
static Platform hardware;static uint32_t menu_modes=APP_BUILT_MODES;
static const int more_modes[]={APP_MINIGAMES,APP_PUZZLES,APP_SURVIVAL,APP_COOP,APP_VERSUS,-1};
static const int more_ids[]={1,8,7,5,6,12};
static int more_count(void){int count=0;for(int i=0;i<6;i++)if(more_modes[i]<0||platform_mode_available(&hardware,more_modes[i],menu_modes))count++;return count;}
static int more_id(int index){for(int i=0;i<6;i++)if(more_modes[i]<0||platform_mode_available(&hardware,more_modes[i],menu_modes)){if(!index--)return more_ids[i];}return 12;}
static int more_position(int id){switch(id){case 8:return 154;case 7:return 196;case 1:return 247;case 5:return 286;case 6:return 324;default:return 365;}}
static void more_horizontal(int direction){
 int current=more_position(more_id(more_cursor)),best=-1,distance=10000,wrap=-1,edge=direction>0?10000:-10000;
 for(int i=0;i<more_count();i++){int x=more_position(more_id(i)),delta=(x-current)*direction;if(delta>0&&delta<distance){distance=delta;best=i;}if(direction>0?x<edge:x>edge){edge=x;wrap=i;}}
 if(best>=0)more_cursor=best;else if(wrap>=0)more_cursor=wrap;
}
static int house_cursor;
static Profile player;static int store_cursor,book_cursor,book_zombies,book_scroll,imit_cursor;static const char *profile_path="profile.dat";
static Progress progress;static Game adventure_backup;static int replaying,menu_selected,replay_cursor;
static const char *adventure_path="save.dat",*progress_path="progress.dat";
static float reward_elapsed,reward_delay,reward_collect_time,reward_x,reward_ground;static int reward_row,reward_col;
static float packet_x[MAX_PACKETS],meter_progress,sun_glow,meter_start_health,meter_start_timer;static int meter_wave=-1;
#include "presentation_settings.inc"
#include "performance_settings.inc"
static int squash_audio_phase[ROWS][COLS];static int packet_ui_count,last_sun;
static double profile_seconds,profile_cpu,profile_gpu;static unsigned int profile_frames;
static const int seed_icons[ALL_SEEDS]={A_PEA,A_SUNFLOWER,A_WALLNUT,A_CHERRY,A_MINE,A_SNOW,A_CHOMPER,A_REPEATER,A_PUFF,A_SUNSHROOM,A_FUME,A_GRAVEBUSTER,A_HYPNO,A_SCAREDY,A_ICESHROOM,A_DOOM,A_LILY,A_SQUASH,A_THREEPEATER,A_KELP,A_JALAPENO,A_SPIKEWEED,A_TORCHWOOD,A_TALLNUT,A_SEA,A_PLANTERN,A_CACTUS,A_BLOVER,A_SPLIT,A_STAR,A_PUMPKIN,A_MAGNET,A_CABBAGE,A_POT,A_KERNEL,A_COFFEE,A_GARLIC,A_UMBRELLA,A_MARIGOLD,A_MELON,A_GATLING,A_TWIN,A_GLOOM,A_CATTAIL,A_WINTER,A_GOLDMAGNET,A_SPIKEROCK,A_COB,A_IMITATER};
static const char *full_seed_names[ALL_SEEDS]={"PEASHOOTER","SUNFLOWER","WALL-NUT","CHERRY BOMB","POTATO MINE","SNOW PEA","CHOMPER","REPEATER","PUFF-SHROOM","SUN-SHROOM","FUME-SHROOM","GRAVE BUSTER","HYPNO-SHROOM","SCAREDY-SHROOM","ICE-SHROOM","DOOM-SHROOM","LILY","SQUASH","THREEPEATER","KELP","JALAPENO","SPIKEWEED","TORCHWOOD","TALLNUT","SEA-SHROOM","PLANTERN","CACTUS","BLOVER","SPLIT","STAR","PUMPKIN","MAGNET","CABBAGE-PULT","FLOWER POT","KERNEL-PULT","COFFEE","GARLIC","UMBRELLA","MARIGOLD","MELON-PULT","GATLING PEA","TWIN SUNFLOWER","GLOOM-SHROOM","CATTAIL","WINTER MELON","GOLD MAGNET","SPIKEROCK","COB CANNON","IMITATER"};
static const char *level_advice[CAMPAIGN_LEVELS]={
    "PLANT PEASHOOTERS TO DEFEND THE FIRST ROW.\nSQUARE COLLECTS THE FALLING SUN.",
    "THREE ROWS OF LAWN ARE NOW OPEN.\nPLANT THREE SUNFLOWERS TO BUILD SUN\nAND BRING ON THE FIRST ZOMBIES.",
    "CONEHEAD ZOMBIES JOIN THE ATTACK.\nCHERRY BOMBS CLEAR A GROUP OF ZOMBIES.",
    "ALL FIVE ROWS OF LAWN ARE OPEN.\nWALL-NUTS HOLD BACK THE ZOMBIES.",
    "CRAZY DAVE INTRODUCES WALL-NUT BOWLING.\nROLL NUTS FROM BEHIND THE RED LINE.\nRED NUTS EXPLODE. SEEDS ARRIVE ON A BELT.",
    "POLE VAULTERS CAN JUMP OVER A PLANT.\nPOTATO MINES NEED TIME TO ARM.",
    "TWO FLAGS OF ZOMBIES ARE COMING.\nSNOW PEAS SLOW THE ADVANCE.",
    "BUCKETHEAD ZOMBIES HAVE STRONG ARMOR.\nCHOOSE SIX PLANTS BEFORE DEFENDING.",
    "YOUR REPEATER IS NOW AVAILABLE.\nCHOOSE SIX PLANTS FOR TWO FLAGS.",
    "THE ZOMBIES LAUNCH THEIR DAYTIME ATTACK.\nUSE THE PLANTS DELIVERED BY THE BELT.\nSUN AND SEED RECHARGE ARE NOT USED.",
    "NIGHT HAS FALLEN. THERE IS NO FALLING SUN.\nPUFF-SHROOMS ARE FREE BUT SHORT-RANGED.\nNEWSPAPER ZOMBIES SPEED UP WITHOUT PAPER.",
    "SUN-SHROOMS MAKE SMALL SUN AT FIRST.\nTHEY GROW AFTER 120 SECONDS.",
    "FUME-SHROOMS HIT GROUPS AT SHORT RANGE.\nTHEIR FUMES PASS THROUGH SCREEN DOORS.",
    "GRAVES BLOCK PLANTING AND RAISE ZOMBIES.\nPLANT A GRAVE BUSTER ON A GRAVE TO EAT IT.",
    "WHACK A ZOMBIE! X STRIKES THE CHOSEN TILE.\nL/R SELECTS A TOOL. SQUARE PLANTS THE TOOL.",
    "FOOTBALL ZOMBIES HAVE STRONG ARMOR.\nAN EATEN HYPNO-SHROOM TURNS A ZOMBIE.",
    "SCAREDY-SHROOMS SHOOT ALONG THEIR ROW.\nTHEY HIDE WHEN A ZOMBIE GETS TOO CLOSE.",
    "DANCING ZOMBIES SUMMON BACKUP DANCERS.\nICE-SHROOM FREEZES THE WHOLE LAWN.",
    "DOOM-SHROOM BLASTS A LARGE AREA.\nITS CRATER TEMPORARILY BLOCKS PLANTING.",
    "THE NIGHTTIME CONVEYOR FINALE.\nUSE DELIVERED MUSHROOMS TO CLEAR THE LAWN.\nGRAVE BUSTERS REMOVE THE TOMBSTONES."};
#include "menu_widgets.inc"
static void notify(const char *text);
static int persist_profile(void);
#include "player_name.inc"
#include "customizer.inc"
#include "menu_logic013.inc"
static void write_profile(FILE *log,const char *reason){
    if(!log||!profile_frames||profile_seconds<=0)return;
    fprintf(log,"%s: %.2f FPS, draw %.3f ms, GPU wait %.3f ms, %u frames\n",reason,profile_frames/profile_seconds,
            profile_cpu/profile_frames,profile_gpu/profile_frames,profile_frames);fflush(log);
}

static int exit_callback(int a,int b,void *c){(void)a;(void)b;(void)c;running=0;return 0;}
static int callbacks(SceSize args,void *argp){(void)args;(void)argp;int id=sceKernelCreateCallback("exit",exit_callback,0);sceKernelRegisterExitCallback(id);sceKernelSleepThreadCB();return 0;}
static void setup_callbacks(void){int id=sceKernelCreateThread("callbacks",callbacks,0x11,4096,0,0);if(id>=0)sceKernelStartThread(id,0,0);}
static float baseline(int r){return game_ground_y(&game,r);}
static float plant_base(int r,int c){return baseline(r)+game_roof_offset(&game,GRID_X+(c+.5f)*CELL_W);}
static float center(int c){return GRID_X+(c+.5f)*CELL_W;}
static void notify(const char *text){snprintf(message,sizeof(message),"%s",text);message_time=2;}
static int persist_profile(void){int ok=profile_save(&player,profile_path);if(!ok)notify("COULD NOT SAVE. CHECK YOUR MEMORY CARD.");return ok;}
static int plant_asset(const Plant *p){
    switch(p->kind){
        case GATLING:return p->flash>0?A_GATLING_SHOOT:A_GATLING;case TWIN:return A_TWIN;
        case GLOOM:return p->flash>0?A_GLOOM_SHOOT:A_GLOOM;case CATTAIL:return p->flash>0?A_CATTAIL_SHOOT:A_CATTAIL;
        case WINTER:return p->flash>0?A_WINTER_SHOOT:A_WINTER;case GOLDMAGNET:return A_GOLDMAGNET;case SPIKEROCK:return A_SPIKEROCK;
        case COB:return p->flash>0?A_COB_SHOOT:p->attack>0?A_COB_UNARMED:A_COB;case IMITATER:return p->age<2?A_IMITATER:A_IMITATER_MORPH;
        case PUFF:return p->flash>0?A_PUFF_SHOOT:A_PUFF;
        case SUNSHROOM:return p->age>=120?A_SUNSHROOM_BIG:A_SUNSHROOM;
        case FUME:return p->flash>0?A_FUME_SHOOT:A_FUME;
        case GRAVEBUSTER:return A_GRAVEBUSTER;case HYPNO:return A_HYPNO;
        case SCAREDY:return p->digest>0?A_SCAREDY_HIDE:p->flash>0?A_SCAREDY_SHOOT:A_SCAREDY;
        case ICESHROOM:return A_ICESHROOM;case DOOM:return A_DOOM;
        case PEA:return p->flash>0?A_PEA_SHOOT:A_PEA;
        case REPEATER:return p->flash>0?A_REPEATER_SHOOT:A_REPEATER;
        case SUNFLOWER:return A_SUNFLOWER;case WALLNUT:return A_WALLNUT;case CHERRY:return A_CHERRY;
        case MINE:return p->age<MINE_ARM_SECONDS?A_MINE_UNARMED:A_MINE;
        case SNOW:return p->flash>0?A_SNOW_SHOOT:A_SNOW;
        case CHOMPER:return p->flash>0?A_CHOMPER_BITE:p->digest>0?A_CHOMPER_CHEW:A_CHOMPER;
    }return p->kind>=16&&p->kind<ALL_SEEDS?seed_icons[p->kind]:A_PEA;
}

static void read_costs(void){
    profile_apply(&player,&game);persist_profile();
    FILE *f=fopen("data/seeds.bin","rb");if(!f)return;char magic[4];uint32_t values[SEEDS*2];
    int ok=fread(magic,4,1,f)==1&&!memcmp(magic,"PVS1",4)&&fread(values,sizeof(values),1,f)==1;fclose(f);
    if(ok)for(int i=0;i<SEEDS;i++)if(values[i*2]<=1000&&values[i*2+1]<=10000){game.seed_cost[i]=values[i*2];game.seed_refresh[i]=values[i*2+1];}
}
static void reset_level_ui(void){reward_elapsed=0;reward_delay=-1;reward_collect_time=-1;packet_ui_count=game.packet_count;for(int i=0;i<MAX_PACKETS;i++)packet_x[i]=44+i*37;meter_progress=game.spawned&&game.num_waves>0?(game.wave-1)/(float)game.num_waves:0;meter_wave=-1;memset(squash_audio_phase,0,sizeof(squash_audio_phase));last_sun=game.sun;sun_glow=0;}
static int save_current(void){
    if(can_resume||replaying){profile_apply(&player,&game);profile_bank(&player,&game);}
    if(!persist_profile())return 0;int ok=1;
    if(replaying&&game.extra.category)ok=game_save(&game,"mode-save.dat");
    if(!replaying&&can_resume){ok=game_save(&game,adventure_path);adventure_backup=game;}
    if(!ok)notify("COULD NOT SAVE. CHECK YOUR MEMORY CARD.");return ok;
}
static void visit_current(void){progress_visit(&progress,game.level,0);progress_save(&progress,progress_path);}
static void open_adventure(void){
    if(!can_resume)return;
    if(replaying){game=adventure_backup;replaying=0;reset_level_ui();}
    screen=(game.level_flags&LEVEL_READY)?1:2;paused=0;help=0;
}
static void start_replay(int level){
    if(!progress_played(&progress,level))return;
    if(!replaying)save_current();replaying=1;game_start_level(&game,level,0x500000u+level);read_costs();reset_level_ui();screen=2;paused=0;help=0;
}
static void new_game(void){
    if(can_resume){const Game *old=replaying?&adventure_backup:&game;game_save(old,"adventure-backup.dat");}
    replaying=0;
    game_start_adventure(&game,1,0x51a7c0de,player.completion.adventure_runs);read_costs();screen=2;paused=0;help=0;can_resume=1;
    visit_current();reset_level_ui();int saved=save_current();
    profile_seconds=profile_cpu=profile_gpu=0;profile_frames=0;
    if(saved)notify("ADVENTURE 1-1");
}
static int level_background(int level){if(level==50)return A_BACKGROUND_BOSS;if(level==35)return A_NIGHT;static const int backgrounds[5]={A_BACKGROUND,A_NIGHT,A_BACKGROUND_POOL,A_BACKGROUND_FOG,A_BACKGROUND_ROOF};return backgrounds[(level-1)/10];}
static int board_background(void){if(game.completion.adventure_run)return level_background(game.level);return game.level==1?A_BACKGROUND_ONE:game.level<=3?A_BACKGROUND_THREE:level_background(game.level);}
static void warm_level(void){
    animation_warm(&assets[board_background()]);
    for(int k=0;k<ALL_SEEDS;k++)if(game.unlocked_mask&(UINT64_C(1)<<k))animation_warm(&assets[seed_icons[k]]);
    for(int k=10;k<Z_TYPES;k++)if(game_zombie_allowed(game.level,k))for(int p=0;p<3;p++)animation_warm(&assets[A_DUCK_WALK+(k-10)*3+p]);
}
static int damage_mask(int asset){
    switch(asset){
        case A_ZOMBIE_DANCE:case A_CONE_DANCE:case A_BUCKET_DANCE:case A_CLASSIC_DANCER:case A_CLASSIC_DANCER_EAT:case A_ZOMBIE:case A_ZOMBIE_EAT:case A_CONE:case A_CONE_EAT:case A_BUCKET:case A_BUCKET_EAT:case A_POLE_WALK:case A_WALLNUT:case A_TALLNUT:case A_PUMPKIN_FRONT:return 3;
        case A_POLE_RUN:case A_POLE_JUMP:case A_POLE_EAT:case A_FLAG_WALK:case A_FLAG_EAT:return 1;
        case A_CLASSIC_DANCER_DEATH:case A_ZOMBIE_DEATH:case A_POLE_DEATH:return 2;
    }return 0;
}
static int defender_stage(float hp,int maximum){return hp<maximum/3?2:hp<maximum*2/3?1:0;}
static void zombie_health(const Zombie *z,int row){
    int max_body=z->kind==Z_GARG?(game.completion.giga[z-game.zombies]?6000:3000):z->kind==Z_YETI||z->kind==Z_ZOMBONI||z->kind==Z_CATAPULT?1350:z->kind==Z_POLE||z->kind==Z_DANCER?500:270,max_armor=z->kind==Z_CONE?370:z->kind==Z_BUCKET||z->kind==Z_DOOR?1100:z->kind==Z_PAPER?150:z->kind==Z_FOOTBALL?1400:0;
    if(z->death>0||(z->hp>=max_body&&z->armor>=max_armor))return;
    int stage=z->hp<max_body/3.0f?2:z->hp<max_body*2/3.0f?1:0;uint32_t color=stage==2?RGBA(240,85,60,255):stage==1?RGBA(245,192,65,255):RGBA(131,220,65,255);
    float y=baseline(row)+2;render_rect(z->x-12,y,24,3,RGBA(35,29,22,255));
    render_rect(z->x-11,y+1,22*fmaxf(0,z->hp)/max_body,1,color);
    if(max_armor){render_rect(z->x-12,y+4,24,2,RGBA(35,29,22,255));render_rect(z->x-11,y+4,22*fmaxf(0,z->armor)/max_armor,1,RGBA(180,200,215,255));}
}

static const Animation *gray_asset(int id,int stage){
    const Animation *source=stage?&damaged[id][stage-1]:&assets[id];Animation *gray=&imitated_assets[id][stage];
    if(!gray->loaded){animation_load(gray,source->path);gray->grayscale=1;}return gray->loaded?gray:source;
}
#include "presentation012.inc"
#include "gallery014.inc"
#include "zombie_presentation0189.inc"
static int draw_extra_hud(float);
static void draw_extra_overlay(void);
static int draw_mini_custom_board(void);
static void draw_mini_overlay(void);
static void draw_board(void){
    if(draw_mini_custom_board())return;
    render_background(&assets[board_background()]);
    if(game.mode==MODE_BOWLING)render_rect(GRID_X+CELL_W*3-2,GRID_Y,2,CELL_H*ROWS,RGBA(210,38,26,255));
    if(game.rake_state==1||game.rake_state==2)render_anim(&assets[A_RAKE],center(game.rake_col),plant_base(game.rake_row,game.rake_col),game.rake_state==1?0:game.rake_timer,.68f,WHITE,0);
    for(int row=0;row<game_rows(&game);row++){
        if(game_cleaner_active(&game,row)&&game.mower_x[row]<510)render_anim(&assets[game_world(&game)==4?A_ROOF_CLEANER:game_water_row(&game,row)&&(game.store_flags&STORE_POOL)?A_POOL_CLEANER:(game.completion.cosmetics&COS_TRICKED)?A_MOWER_TRICKED:A_MOWER],game.mower_x[row],baseline(game.extra.mower_row[row]?game.extra.mower_row[row]-1:row)+game_roof_offset(&game,game.mower_x[row]),game.time,.68f,WHITE,1);
        for(int col=0;col<COLS;col++){
            if(game.graves[row][col])render_anim(&assets[A_GRAVE],center(col),baseline(row),0,1,WHITE,0);
            if(game.craters[row][col]>0)render_anim(&assets[A_CRATER],center(col),baseline(row),0,1,WHITE,0);
            float foot=plant_base(row,col)+game.mini.match_fall[row][col];
            if(game.support[row][col]){int id=seed_icons[game.support[row][col]-1];render_anim(game.imitated[row][col]&2?gray_asset(id,0):&assets[id],center(col),foot,game.time,.68f,WHITE,1);}
            if(game.ice_tiles[row][col]>0)render_rect(center(col)-18,foot-6,36,7,RGBA(160,220,250,180));
            if(game.vases[row][col]){int reveal=game.extra.vase_green[row][col];for(int r=0;r<5;r++)for(int c=0;c<9;c++)if(game.plants[r][c].kind==PLANTERN&&abs(r-row)<=1&&abs(c-col)<=1)reveal=1;render_anim(&assets[A_VASE],center(col),foot,0,1,game.extra.vase_green[row][col]?RGBA(110,255,130,255):WHITE,0);if(reveal){int content=game.extra.vase_content[row][col],id=content==1000?A_SUN:content<0?A_ZOMBIE:seed_icons[content==LEFT_PACKET?REPEATER:content];render_anim(&assets[id],center(col),foot-7,0,.36f,RGBA(255,255,255,200),1);}}
            if(game.pumpkin_hp[row][col]>0)render_anim(game.imitated[row][col]&4?gray_asset(A_PUMPKIN_BACK,0):&assets[A_PUMPKIN_BACK],center(col),foot,game.time,.68f,WHITE,1);
            if(game.morph_timer[row][col]>0)render_anim(&assets[game.morph_timer[row][col]>30.0f/26?A_IMITATER:A_IMITATER_MORPH],center(col),foot,2+30.0f/26-game.morph_timer[row][col],.68f,WHITE,0);
            Plant *p=&game.plants[row][col];if(p->kind<0)continue;
            if(p->kind==SQUASH&&game.burst_left[row][col]>=3){}
            else if(p->kind==SQUASH&&game.burst_left[row][col]>0){int phase=game.burst_left[row][col],id=phase==1?(game.pea_shoot_time[row][col]<center(col)?A_SQUASH_LEFT:A_SQUASH_RIGHT):A_SQUASH_UP;render_anim(&assets[id],center(col),foot,game.burst[row][col],.68f,WHITE,0);}
            else if(p->kind==SUNFLOWER&&!(game.imitated[row][col]&1)&&board_sunflower_glow(p)>0)render_anim_glow(&assets[A_SUNFLOWER],center(col),foot,p->age,.68f,board_sunflower_glow(p));
            else{
                int asset=plant_asset(p),stage=p->kind==WALLNUT?defender_stage(p->hp,4000):p->kind==TALLNUT?defender_stage(p->hp,8000):0;
                const Animation *a=stage?&damaged[asset][stage-1]:&assets[asset];
                if(game.imitated[row][col]&1)a=gray_asset(asset,stage);
                if((p->kind==PEA||p->kind==SNOW||p->kind==REPEATER)&&!(game.imitated[row][col]&1)&&game.extra.category!=EXTRA_IZOMBIE){
                    int body=p->kind==SNOW?A_SNOW_BODY:p->kind==REPEATER?A_REPEATER_BODY:A_PEA_BODY;float action=game.pea_shoot_time[row][col];float idle=p->age*(.84f+((row*7+col*13)%11)*.026f);
                    (game.extra.left_facing[row][col]?render_anim_mirrored:render_anim)(&assets[body],center(col),foot,idle,.68f,WHITE,1);
                    float end=24.0f/(p->kind==REPEATER?45:35),blend=action>0?fminf(1,fminf(action/.12f,(end+.12f-action)/.12f)):0;
                    if(blend<1)(game.extra.left_facing[row][col]?render_anim_mirrored:render_anim)(&assets[body+1],center(col),foot,idle,.68f,RGBA(255,255,255,(int)(255*(1-blend))),1);
                    if(blend>0){int family=p->kind==SNOW?2:p->kind==REPEATER?1:0,frame=(int)(idle*60)%80;float dx=shoot_alignment[family][frame][0]*.68f,dy=shoot_alignment[family][frame][1]*.68f;if(game.extra.left_facing[row][col])dx=-dx;
                        (game.extra.left_facing[row][col]?render_anim_mirrored:render_anim)(&assets[body+2],center(col)+dx,foot+dy,action,.68f,RGBA(255,255,255,(int)(255*blend)),0);}
                }else (game.extra.left_facing[row][col]?render_anim_mirrored:render_anim)(a,center(col),foot,game.extra.category==EXTRA_IZOMBIE?0:p->age,p->kind==MINE?(p->age<MINE_ARM_SECONDS?.51f:.45f):.68f,game.asleep[row][col]?RGBA(160,160,190,255):WHITE,1);
            }
            if(p->kind==MINE&&p->age<MINE_ARM_SECONDS){
                render_rect(center(col)-11,foot+2,22,2,RGBA(60,40,20,255));
                render_rect(center(col)-11,foot+2,22*p->age/MINE_ARM_SECONDS,2,RGBA(250,202,85,255));
            }
            int max_hp=p->kind==WALLNUT?4000:p->kind==TALLNUT?8000:p->kind==SPIKEROCK?450:300;if((health_bars&1)&&p->hp<max_hp*.7f){float health=p->hp/max_hp;render_rect(center(col)-11,foot+1,22,2,RGBA(64,25,12,255));render_rect(center(col)-11,foot+1,22*health,2,RGBA(130,220,70,255));}
        }
        for(int col=0;col<COLS;col++)if(game.pumpkin_hp[row][col]>0){
            int stage=defender_stage(game.pumpkin_hp[row][col],4000);const Animation *front=stage?&damaged[A_PUMPKIN_FRONT][stage-1]:&assets[A_PUMPKIN_FRONT];if(game.imitated[row][col]&4)front=gray_asset(A_PUMPKIN_FRONT,stage);
            render_anim(front,center(col),plant_base(row,col)+game.mini.match_fall[row][col],game.time,.68f,WHITE,1);
        }
        for(int i=0;i<MAX_ZOMBIES;i++){
            Zombie *z=&game.zombies[i];if(!z->used||z->row!=row||game_mini_active(&game,MINI_INVIS))continue;
            int body;const Animation *animation=zombie_visible_clip(z,i,&body);
            uint32_t tint=game.hypnotized[i]?RGBA(240,150,255,255):z->slow>0||game.frozen[i]>0?RGBA(150,215,255,255):WHITE;
            float foot=baseline(row)+game_roof_offset(&game,z->x),scale=(game.level==25||game_mini_active(&game,MINI_LITTLE))?.465f:.93f;
            int action=game.polish.action[i];
            if(action==ACTION_GARLIC&&game.polish.action_time[i]>=1.7f){float t=fminf(1,game.polish.action_time[i]-1.7f);int oldrow=game.polish.target[i]/ROWS;foot+=(baseline(oldrow)-baseline(z->row))*(1-t);}
            if(action==ACTION_IMP_FLIGHT){float duration=fmaxf(.1f,fabsf(z->vault_from-z->vault_to)/142.5f);foot-=54*sinf(fminf(1,game.polish.action_time[i]/duration)*3.14159f);}
            int head=game.mini.zombie_heads[i];
            if(z->kind==Z_BALLOON)foot-=21;
            if(game.mode==MODE_BUNGEE&&game.special[i]<0){foot+=game.special[i]*40;render_anim(&assets[A_BUNGEE_EAT],z->x,foot-35,game.time,.85f,WHITE,1);render_rect(z->x,43,1,fmaxf(0,foot-65),RGBA(170,150,100,255));}
            if(z->kind==Z_BUNGEE){foot-=fmaxf(0,80-game.special[i]*25);render_rect(z->x,43,1,fmaxf(0,foot-35),RGBA(170,150,100,255));}
            float anim=zombie_visible_time(z,animation);
            if(z->death>0)tint=(tint&0xffffffu)|((uint32_t)(255*game_death_alpha(z))<<24);
            else if(game.mode==MODE_WHACK&&game.special[i]>0)tint=RGBA(255,210,140,255);
            int custom=z->kind==Z_FLAG&&z->death<=0&&body<2&&avatar.enabled&&avatar_draw(&avatar,z->x,foot,anim,scale,tint,!!z->eating,!!game.hypnotized[i],body==1);
            if(custom){}
             else if(game.hypnotized[i]||(z->kind==Z_DIGGER&&z->vaulted)||game.completion.yeti_retreat[i])render_anim_mirrored(animation,z->x,foot,anim,scale,tint,z->death<=0&&!action&&z->vault<=0);
             else render_anim(animation,z->x,foot,anim,scale,tint,z->death<=0&&!action&&z->vault<=0);
            if(!custom&&z->death<=0&&body<2&&(z->kind==Z_NORMAL||z->kind==Z_CONE||z->kind==Z_BUCKET||z->kind==Z_FLAG||z->kind==Z_DOOR)&&(game.completion.cosmetics&(COS_FUTURE|COS_MUSTACHE))&&avatar_assets()){
                int act=!!z->eating,dance=(game.completion.cosmetics&COS_DANCE)&&z->kind<=Z_BUCKET&&!act,n=dance?assets[A_ZOMBIE_DANCE].frames:avatar_art[AV_BODY+act].frames,frame=(int)(anim*(dance?assets[A_ZOMBIE_DANCE].fps:avatar_art[AV_BODY+act].fps))%n,mirror=!!game.hypnotized[i];
                if(game.completion.cosmetics&COS_FUTURE)render_anim_frame(&avatar_art[dance?AV_FACE_DANCE:AV_FACE+act],z->x,foot,(1+i%4)*n+frame,scale,tint,mirror);
                if(game.completion.cosmetics&COS_MUSTACHE)render_anim_frame(&avatar_art[dance?AV_MUSTACHE_DANCE:AV_MUSTACHE+act],z->x,foot,(i%3)*n+frame,scale,tint,mirror);
            }
            if(!custom&&z->death<=0&&game.extra.hit_flash[i]>0)render_anim_glow(animation,z->x,foot,anim,scale,.5f);
            if(z->kind==Z_BOBSLED&&game.extra.bobsled_team[i]==i+1&&game.extra.bobsled_phase[i]>=0)render_anim(&assets[A_SLED],z->x+22,foot+4,0,1,WHITE,0);
            if(head&&z->death<=0){if(game.extra.zombotany_jump[i]>0)foot-=sinf((.95f-game.extra.zombotany_jump[i])/.95f*3.14159f)*45;int icon=head==1?A_PEA_HEAD:head==2?A_WALLNUT:head==3?A_JALAPENO:head==4?A_GATLING:head==5?A_SQUASH:A_TALLNUT;render_anim_mirrored(&assets[icon],z->x-3,foot-(head==1?9:22),game.time,head==1?.78f:.52f,tint,1);}
        }
        for(int c=0;c<COLS;c++)if(game.plants[row][c].kind==SQUASH&&game.burst_left[row][c]>=3)draw_squash_flight(row,c);
        for(int i=0;i<MAX_SHOTS;i++)if(game.shots[i].used&&game.shots[i].row==row){
            Shot *shot=&game.shots[i];static const int projectile[]={A_PROJECTILE_PEA,A_PROJECTILE_SNOW,A_SPORE,A_SPORE,A_PROJECTILE_CABBAGE,A_PROJECTILE_KERNEL,A_PROJECTILE_BUTTER,A_PROJECTILE_MELON,A_PROJECTILE_STAR,A_PROJECTILE_CACTUS,A_PROJECTILE_PEA,A_PROJECTILE_CATTAIL,A_PROJECTILE_WINTER,A_PROJECTILE_COB};
            render_anim(&assets[projectile[shot->ice]],shot->x,game.shot_y[i],0,1,shot->ice==10?RGBA(255,170,60,255):WHITE,0);
        }
    }
    // Keep damage indicators visible when the next lane's tall sprites overlap.
    for(int i=0;i<MAX_ZOMBIES;i++)if((health_bars&2)&&game.zombies[i].used&&!game_mini_active(&game,MINI_INVIS))zombie_health(&game.zombies[i],game.zombies[i].row);
    for(int i=0;i<MAX_NUTS;i++)if(game.nuts[i].used){BowlingNut *n=&game.nuts[i];render_anim_rotated(&assets[A_WALLNUT],n->x,n->y,n->age,n->explosive==2?1.45f:.68f,n->explosive==1?RGBA(255,100,90,255):WHITE,n->age*7);}
    for(int i=0;i<MAX_SUNS;i++)if(game.suns[i].used){Sun *s=&game.suns[i];float scale=s->used==2?.60f*(1-.5f*fminf(1,s->age/s->flight)):s->from_plant?fminf(.60f,.24f+s->flight*1.2f):.60f;render_anim(&assets[A_SUN],s->x,s->y,game.time,scale,WHITE,1);}
    for(int i=0;i<MAX_COINS;i++)if(game.money[i].used){Coin *c=&game.money[i];render_anim(&assets[c->value==-100?A_CHOCOLATE:c->value<0?A_PRESENT:c->value>=100?A_COIN_DIAMOND:c->value>=5?A_COIN_GOLD:A_COIN_SILVER],c->x,c->y,game.time,.75f,WHITE,1);}
    for(int i=0;i<32;i++)if(game.extra.effect_time[i]>0){HeadParticle head;if(game_head_particle(&game,i,&head)){render_anim_rotated(head.kind==3&&classic_dancer?&assets[A_CLASSIC_DANCER_HEAD]:&head_particles[head.kind],head.x,head.y,0,.8f*.93f,RGBA(255,255,255,(int)(255*head.alpha)),head.rotation);continue;}int explosion=game.extra.effect_kind[i];float t=1-game.extra.effect_time[i]/(explosion?.5f:.2f),x=game.extra.effect_x[i],y=game.extra.effect_y[i];if(explosion){for(int k=0;k<6;k++){float a=k*1.0471976f;render_anim(&assets[A_IMPACT_DUST],x+cosf(a)*t*24,y+sinf(a)*t*15,t*.25f,1+t,RGBA(255,235,195,(int)(230*(1-t))),0);}}else for(int k=0;k<5;k++){float a=k*1.256637f;render_anim(&assets[A_PROJECTILE_PEA],x+cosf(a)*t*10,y+sinf(a)*t*8,0,.18f,RGBA(230,255,170,(int)(255*(1-t))),0);}}
    for(int i=0;i<24;i++){CosmeticParticle *p=&game.completion.particles[i];if(p->life<=0)continue;float scale=p->type==1?fminf(1,p->age*2):.7f,alpha=fminf(1,p->life/.2f);render_anim_frame(&assets[p->type==1?A_SECRET_DAISY:A_SECRET_PINATA],p->x,p->y,p->type==2?p->frame:0,scale,RGBA(255,255,255,(int)(255*alpha)),0);}
    if(game.flash>0)render_rect(0,44,480,205,RGBA(255,240,180,(int)(game.flash*400)));
    if(game.mode==MODE_BOSS){
        if(game.status==WON)render_anim(&assets[A_BOSS_DEATH],442,244,reward_elapsed*18/assets[A_BOSS_DEATH].fps,.92f,WHITE,0);else{static const int spawn[]={A_BOSS_SPAWN1,A_BOSS_SPAWN2,A_BOSS_SPAWN3,A_BOSS_SPAWN4,A_BOSS_SPAWN5},spit[]={A_BOSS_SPIT1,A_BOSS_SPIT2,A_BOSS_SPIT3,A_BOSS_SPIT4,A_BOSS_SPIT5},stomp[]={A_BOSS_STOMP1,A_BOSS_STOMP2,A_BOSS_STOMP3,A_BOSS_STOMP4};int action=game.extra.boss_action,id=action==B_SPAWN?spawn[game.boss_row]:action==B_HEAD_ENTER?A_BOSS_HEAD_ENTER:action==B_HEAD_SPIT?spit[game.boss_row]:action==B_HEAD_WAIT||action==B_HEAD_AFTER?A_BOSS_EAT:action==B_HEAD_LEAVE?A_BOSS_HEAD_LEAVE:action==B_STOMP?stomp[game.boss_row%4]:action==B_BUNGEE?A_BOSS_BUNGEE:action==B_RV?A_BOSS_RV:A_BOSS_WALK;render_anim(&assets[id],442,244,game.extra.boss_action_time*(action==B_RV?16:12)/assets[id].fps,.92f,WHITE,action==B_IDLE||action==B_HEAD_WAIT||action==B_HEAD_AFTER);}
        if(health_bars&2){render_rect(330,48,141,7,RGBA(30,20,15,255));render_rect(331,49,139*fmaxf(0,game.boss_hp)/(game.challenge?60000:40000),5,RGBA(180,40,35,255));}
        if(game.boss_ball_kind){float y=baseline(game.boss_ball_row)+game_roof_offset(&game,game.boss_ball_x);render_anim_rotated(&assets[game.boss_ball_kind==1?A_BOSS_FIREBALL:A_BOSS_ICEBALL],game.boss_ball_x,y,game.time,1,WHITE,-game.time*2);}
    }
    if(game_world(&game)==3&&game.level!=40&&game.fog_clear<=0){
        for(int col=4;col<COLS;col++)for(int row=0;row<6;row++){int lit=0;for(int r=0;r<6;r++)for(int c=0;c<COLS;c++)if(game.plants[r][c].kind==PLANTERN&&abs(r-row)<=1&&abs(c-col)<=1)lit=1;
            if(!lit)render_anim(&assets[A_FOG_CLOUD],GRID_X+(col+.5f)*CELL_W+3*sinf(game.time*.4f+row),GRID_Y+(row+1)*31+12,game.time+row+col,.98f,RGBA(255,255,255,245),1);}
    }
    if(game.level==40){float lightning=fmodf(game.time,8);int darkness=lightning<.4f?40:lightning<1?120:205;render_rect(0,45,480,203,RGBA(0,0,20,darkness));for(int i=0;i<45;i++){float rx=fmodf(i*79-game.time*48+10000,480),ry=45+fmodf(i*41+game.time*170,200);render_rect(rx,ry,1,7,RGBA(165,185,220,130));}}
    float x=GRID_X+game.cursor_col*CELL_W,y=GRID_Y+game.cursor_row*game_row_height(&game)+game_roof_offset(&game,center(game.cursor_col));
    uint32_t cursor=game.shovel?FOCUS_ACTION:FOCUS_SAGE;
    if((game.status==PLAYING&&!(game.extra.pending==2&&game.cursor_row==2&&game.cursor_col==4))||(game.status==WON&&!(game.level_flags&LEVEL_REWARD_COLLECTED)&&(game.cursor_row!=reward_row||game.cursor_col!=reward_col))){render_focus(x,y,CELL_W,game_row_height(&game),cursor);}
    draw_mini_overlay();
    if(game.huge_wave_timer>0){float scale=1+.015f*sinf(game.time*5);render_anim(&assets[A_HUGE_WAVE],240,151,0,scale,WHITE,0);}
}

static void update_level_ui(float dt){
    sun_glow=fmaxf(0,sun_glow-dt);if(game.sun>last_sun)sun_glow=.45f;last_sun=game.sun;
    float target=wave_meter_target();
    if(target>meter_progress)meter_progress=fminf(target,meter_progress+dt*.12f);
    for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++){int phase=game.plants[r][c].kind==SQUASH?game.burst_left[r][c]:0;if(phase==1&&!squash_audio_phase[r][c])audio_play(CUE_SQUASH_HMM);if(phase==5&&squash_audio_phase[r][c]!=5)audio_play(CUE_SQUASH_THUMP);squash_audio_phase[r][c]=phase;}
    if(game.mode==MODE_VASE)layout_vase_packets();
    else if(game.mode!=MODE_LAWN&&game.mode!=MODE_WHACK){
        for(int i=packet_ui_count;i<game.packet_count;i++)packet_x[i]=398+(i-packet_ui_count)*37;
        packet_ui_count=game.packet_count;
        for(int i=0;i<game.packet_count;i++){float goal=44+i*37;if(packet_x[i]>goal)packet_x[i]=fmaxf(goal,packet_x[i]-19*dt);if(i&&packet_x[i]<packet_x[i-1]+37)packet_x[i]=packet_x[i-1]+37;}
    }
}
static void remove_packet_ui(int index){for(int i=index;i<MAX_PACKETS-1;i++)packet_x[i]=packet_x[i+1];packet_ui_count=game.packet_count;}
static void draw_progress(void){
    float x=324,y=252,w=148,h=17,p=fminf(1,fmaxf(0,meter_progress));
    render_image(&assets[A_METER_EMPTY],x,y,w,h,0,WHITE);
    if(p>0)render_part(&assets[A_METER_FULL],0,158*(1-p),0,158*p,27,x+w*(1-p),y,w*p,h,WHITE);
    for(int wave=0;wave<game.num_waves;wave++)if(game_flag_wave(&game,wave)){
        float fx=x+w*(1-(wave+1)/(float)game.num_waves);int passed=game.spawned&&game.wave>=wave+1;
        render_image(&assets[A_METER_FLAG],fx-6,y-(passed?13:8),13,13,0,passed?WHITE:RGBA(185,185,185,255));
    }
    render_image(&assets[A_METER_HEAD],x+w*(1-p)-8,y-2,18,18,0,WHITE);
}
#include "challenge018.inc"
#include "audio_scene018.inc"
#include "mini_ui.inc"
static void draw_hud(float fps){
    if(draw_extra_hud(fps)){draw_extra_overlay();return;}
    if(draw_mini_hud(fps))return;
    char text[120];int cards=game.packet_count;
    if(game.mode==MODE_LAWN||game.mode==MODE_WHACK){
        cards=0;for(int i=0;i<ALL_SEEDS;i++)cards+=!!game_seed_available(&game,i);
        render_part(&assets[A_SEEDBANK],0,0,0,88,87,4,1,44,44,WHITE);
        render_part(&assets[A_SEEDBANK],0,88,0,358,87,48,1,cards*(cards>=10?36:42)+7,44,WHITE);
        // The label has only 26 by 9 usable pixels after scaling the bank.
        // Fit the visible 5x7 glyphs inside it, including unusually large saves.
        snprintf(text,sizeof(text),"%d",game.sun);float width=strlen(text)*6-1,scale=fminf(1,26/width);
        render_text(24.5f-width*scale*.5f,34+(7-7*scale)*.5f,text,sun_glow>0?RGBA(100,55,12,255):RGBA(40,33,15,255),scale);
    }else if(game.mode==MODE_VASE){render_part(&assets[A_SEEDBANK],0,88,0,358,87,4,1,430,44,WHITE);disc_text(10,12,36,"Plants",UI_INK,.5f);
    }else{
        render_image(&assets[A_CONVEYOR_FRAME],38,1,398,43,0,WHITE);
        render_image(&assets[A_CONVEYOR_STRIP],44,34,386,7,(int)(game.time*25)%6,WHITE);
    }
    int slot=0,spacing=cards>=10?36:42;
    for(int index=0;index<((game.mode==MODE_LAWN||game.mode==MODE_WHACK)?cards:game.packet_count);index++){
        int kind=(game.mode==MODE_LAWN||game.mode==MODE_WHACK)?game_seed_slot(&game,index):game.packets[index];if((game.mode==MODE_LAWN||game.mode==MODE_WHACK)&&!game_seed_available(&game,kind))continue;
        float x=(game.mode==MODE_LAWN||game.mode==MODE_WHACK)?54+slot++*spacing:packet_x[index];
        float visible=fminf(26,430-(x+3));if(visible<=0)continue;
        render_part(&assets[A_SEED_CARDS],kind==LEFT_PACKET?REPEATER:kind>=EXPLODE_NUT?WALLNUT:kind,0,0,assets[A_SEED_CARDS].width*visible/26,assets[A_SEED_CARDS].height,x+3,5,visible,35,kind==EXPLODE_NUT?RGBA(255,100,80,255):WHITE);
        if(game.mode==MODE_LAWN||game.mode==MODE_WHACK){
            if(*game_seed_cooldown(&game,kind)>0){float ratio=*game_seed_cooldown(&game,kind)/(game_seed_refresh(&game,kind)/100.0f);render_rect(x+3,5,26,35*ratio,RGBA(20,26,20,155));}
            if(game.sun<game_seed_cost(&game,kind))render_rect(x+3,5,26,35,RGBA(28,30,27,85));
        }
        if(((game.mode==MODE_LAWN||game.mode==MODE_WHACK)?kind==game.selected:index==game.selected)&&visible==26){render_focus(x+1,3,30,39,FOCUS_SAGE);}
    }
    if(game.level>=5||game.completion.adventure_run){render_rect(438,2,34,39,RGBA(102,56,27,220));render_image(&assets[A_SHOVEL],441,4,28,32,0,game.shovel?RGBA(255,240,140,255):WHITE);}
    const char *controls=game.status==WON?"":game.shovel_tutorial?"Triangle, then X: Dig up the three nuts":message_time>0?message:game.level==2&&!game.total_spawned&&!(game.level_flags&LEVEL_TUTORIAL_COMPLETE)?"Plant three sunflowers to prepare":game.mode==MODE_WHACK?"X: Whack   L/R: Tool   Square: Use   Start: Pause":"L/R: Seed   X: Plant   Square: Sun   Start: Pause";
    if(game.challenge&&!game.extra.category)snprintf(text,sizeof(text),"%s",mini_names[game.mini.id-1]);else snprintf(text,sizeof(text),"%s %d-%d",replaying?"Replay":"Level",(game.level-1)/10+1,(game.level-1)%10+1);
    ui_hud_footer(text,game_mini_active(&game,MINI_LASTSTAND)&&!game.mini.phase?"Start: Begin onslaught":controls,game.extra.category!=EXTRA_VASE&&game.extra.category!=EXTRA_IZOMBIE);
    if(game.extra.category!=EXTRA_VASE&&game.extra.category!=EXTRA_IZOMBIE)draw_progress();
    if(game.extra.category)draw_extra_overlay();
    if(debug_info){snprintf(text,sizeof(text),"%.0f FPS  DRAW %.1fMS  GPU %.1fMS  Z:%d",fps,last_render.cpu_ms,last_render.gpu_wait_ms,game_alive(&game));render_rect(0,235,480,13,RGBA(0,0,0,190));render_text(6,238,text,WHITE,1);}
}

static void menu_background(int page,int frame);
static void panel(const char *title){ui_dialog(36,49,408,183,title);}
static void draw_menu(void){menu_background(0,(can_resume?0:5)+menu_selected);char label[48];snprintf(label,sizeof(label),"Player: %s",player_name);render_image(&assets[A_UI_BUTTON],348,8,123,25,menu_selected==4,menu_selected==4?UI_ACTIVE_TINT:WHITE);ui_center(354,12,111,label,menu_selected==4?UI_FOCUS_TEXT:WHITE,.63f);if(menu_selected==3)ui_center(232,226,68,"Options",UI_GOLD,.65f);}
static void draw_options(void){
 render_image(&assets[A_MENU_MAIN],0,0,480,272,(can_resume?0:5)+3,WHITE);ui_dialog(66,24,348,220,"Options");static const char *bars[]={"Off","Plants","Zombies","Both"};
 int rows=options_count(),top=rows==9?55:58,step=rows==9?20:22;
 for(int i=0;i<rows;i++){int y=top+i*step;if(i<2){int value=i?audio_effects_volume():audio_music_volume();disc_text(88,y,92,i?"Sound effects":"Music",WHITE,.67f);render_image(&assets[A_UI_SLIDER],191,y+5,146,8,0,WHITE);render_image(&assets[A_UI_KNOB],185+146*value/100.0f,y,15,19,0,WHITE);char text[12];snprintf(text,sizeof(text),"%d",value);ui_center(351,y,38,text,i==menu_volume_row?UI_FOCUS_TEXT:WHITE,.72f);}
  else if(i==2){char label[48];snprintf(label,sizeof(label),"Player name: %s",player_name);render_image(&assets[A_UI_BUTTON],90,y-2,300,22,menu_volume_row==2,menu_volume_row==2?UI_ACTIVE_TINT:WHITE);ui_center(96,y,288,label,menu_volume_row==2?UI_FOCUS_TEXT:WHITE,.68f);}
  else if(i==3){disc_text(88,y,135,"Health bars",WHITE,.67f);ui_center(236,y,147,bars[health_bars],UI_GOLD,.7f);}
  else if(i==6){disc_text(88,y,135,"Dancing zombie",WHITE,.67f);ui_center(225,y,158,classic_dancer?"Michael Jackson":"Disco",UI_GOLD,.65f);}
  else if(i==7&&performance_option()){disc_text(88,y,135,"Frame rate",WHITE,.67f);ui_center(236,y,147,psp1000_fps==60?"60 FPS (test)":"30 FPS",UI_GOLD,.7f);}
  else{render_image(&assets[A_UI_BUTTON],90,y-2,300,22,menu_volume_row==i,menu_volume_row==i?UI_ACTIVE_TINT:WHITE);ui_center(96,y,288,i==4?"How to play":i==5?"Credits and extras":"Back",menu_volume_row==i?UI_FOCUS_TEXT:WHITE,.68f);}
  if(i==menu_volume_row&&(i<2||i==3||i==6||(i==7&&performance_option())))render_focus(82,y-3,313,22,FOCUS_SAGE);
 }ui_hint("Up/Down: Select   Left/Right: Adjust   X: Toggle/open   O: Back");
}
static void draw_replay(void){
 render_background(&assets[A_MINI_BACKGROUND]);ui_header("Replay Adventure");int first=replay_cursor/10*10;
 for(int i=first;i<first+10;i++){int x=12+(i%5)*94,y=49+(i%10/5)*96;char label[40];snprintf(label,sizeof(label),"Level %d-%d",i/10+1,i%10+1);ui_card(x,y,82,88,-1-i/10,label,i==replay_cursor,!!(progress.completed&(UINT64_C(1)<<i)),progress_played(&progress,i+1));}
 char title[96];snprintf(title,sizeof(title),"Chapter %d/5   D-pad: Select   X: Replay   O: Back",replay_cursor/10+1);ui_hint(message_time>0?message:title);
}
static void draw_help(void){ui_dialog(71,38,338,199,"How to play");disc_text(92,78,295,"D-pad / Analog: Move cursor\nL / R: Choose a seed\nX: Plant or use the shovel\nSquare: Collect sun\nTriangle: Shovel\nStart: Pause and save\nSelect: Performance display\nO: Back",WHITE,.7f);}
static int seed_count(uint64_t mask){int n=0;for(int k=0;k<ALL_SEEDS;k++)n+=(mask>>k)&1;return n;}
static void draw_intro(void){
 render_background(&assets[board_background()]);char title[100];const char *advice=0;
 if(game.extra.category){if(game.extra.category==EXTRA_SURVIVAL)snprintf(title,sizeof(title),"Survival: %s",game.extra.selection==0?"Day":game.extra.selection==1?"Night":game.extra.selection==2?"Pool":game.extra.selection==3?"Fog":"Roof");else ui_title_case(title,sizeof(title),puzzle_names[game.extra.selection+(game.extra.category==EXTRA_IZOMBIE?10:0)]);advice=game.extra.category==EXTRA_SURVIVAL?"Keep your lawn and sun between rounds.\nChoose new plants for each onslaught.\nNormal: 5 flags   Hard: 10 flags.":game.extra.category==EXTRA_IZOMBIE?"Place zombies to eat all five brains.\nUse sun to buy zombies.\nEating sunflowers releases more sun.":"Break vases to reveal plants and zombies.\nUse packets before they disappear.\nDefeat every zombie to clear the board.";}
 else if(game.challenge){ui_title_case(title,sizeof(title),mini_names[game.mini.id-1]);advice=mini_advice[game.mini.id-1];}
 else{snprintf(title,sizeof(title),"Adventure %d-%d",(game.level-1)/10+1,(game.level-1)%10+1);advice=game.completion.adventure_run&&game.mode==MODE_LAWN?"Crazy Dave has chosen three plants for you.\nFill the remaining seed slots to begin.":level_advice[game.level-1];if(!advice)advice=game.mode==MODE_VASE?"Break vases with X. L/R chooses a seed.\nDefeat the zombies hidden inside.\nClear three rounds.":game.mode==MODE_BOSS?"Defeat Dr. Zomboss.\nIce-shrooms stop fireballs.\nJalapenos stop iceballs.":game_world(&game)>=2&&game_world(&game)<=3?"Water plants need a lily pad.\nPrepare your defense for all six lanes.":"Flower pots support plants on the roof.\nUse cabbage, kernel and melon-pults.";}
 ui_dialog(66,63,348,150,title);disc_text(85,103,310,advice,WHITE,.64f);ui_hint("X: Continue   O: Main menu");
}
static void draw_seed_choice(void){
    normalize_choice();
    render_background(&assets[board_background()]);render_image(&assets[A_SEED_CHOOSER],6,47,278,201,0,WHITE);
    render_part(&assets[A_SEEDBANK],0,0,0,88,87,4,1,44,44,WHITE);render_part(&assets[A_SEEDBANK],0,88,0,358,87,48,1,game.seed_slots*36+7,44,WHITE);char sun[20];snprintf(sun,sizeof(sun),"%d",game.sun);float sun_width=strlen(sun)*6-1,sun_scale=fminf(1,26/sun_width);render_text(24.5f-sun_width*sun_scale*.5f,34+(7-7*sun_scale)*.5f,sun,UI_INK,sun_scale);for(int slot=0;slot<game.seed_slots;slot++){int k=game_seed_slot(&game,slot);if(k<0)break;render_image(&assets[A_SEED_CARDS],57+slot*36,5,26,35,k,WHITE);if(game_seed_locked(&game,k))render_image(&assets[A_CHALLENGE_LOCK],75+slot*36,6,8,10,0,WHITE);}
    for(int k=0;k<ALL_SEEDS;k++){
        int x=20+(k%9)*28,y=70+(k/9)*28,owned=!!(game.unlocked_mask&(UINT64_C(1)<<k)),chosen=!!(game.seed_mask&(UINT64_C(1)<<k));
        if(owned){render_image(&assets[A_SEED_CARDS],x,y,20,27,k,chosen?RGBA(130,130,130,255):WHITE);if(chosen)render_rect(x,y,20,27,RGBA(20,35,20,90));}
        if(k==game.selected)render_focus(x-1,y-1,22,29,FOCUS_SAGE);
    }
    ui_dialog(294,62,177,177,"");int owned=!!(game.unlocked_mask&(UINT64_C(1)<<game.selected));
    if(owned){render_image(&assets[A_SEED_CARDS],353,75,46,63,game.selected,WHITE);char name[64];ui_title_case(name,sizeof(name),full_seed_names[game.selected]);ui_center(301,147,163,name,UI_GOLD,.8f);char cost[48];snprintf(cost,sizeof(cost),"Sun: %d",game_seed_cost(&game,game.selected));ui_center(307,172,151,cost,WHITE,.64f);snprintf(cost,sizeof(cost),"Recharge: %.1fs",game_seed_refresh(&game,game.selected)/100.0f);ui_center(307,190,151,cost,WHITE,.59f);}
    char title[40];snprintf(title,sizeof(title),"Selected %d/%d",seed_count(game.seed_mask),game.seed_slots);ui_center(65,51,158,title,UI_GOLD,.55f);
}

#include "reward_ui.inc"

static void draw_result(void){
    if(game.status==WON&&reward_collect_time>=0&&reward_collect_time<.85f&&game_reward_type(&game)==REWARD_TROPHY){draw_reward_flight(REWARD_TROPHY,-1,reward_collect_time,reward_x,reward_ground);return;}
    if(game.status==WON&&game_reward_type(&game)==REWARD_TROPHY){draw_trophy_award(game.challenge?mini_names[game.mini.id-1]:"Challenge complete");return;}
    if(game.extra.category){ui_dialog(96,65,288,145,game.status==WON?"Challenge complete":"Try again");ui_button(122,109,236,"X: Mode selection",1);ui_button(122,148,236,"O: Main menu",0);return;}
    if(game.status==WON&&game_reward_type(&game)==REWARD_SEED&&game_reward_seed(&game)>=0){draw_plant_award(game_reward_seed(&game));return;}
    if(replaying&&game.level!=50){ui_dialog(80,56,320,170,game.status==WON?(game.challenge?"Mini-game complete":"Replay complete"):"The zombies got through!");ui_center(98,97,284,"Your Adventure save is preserved.",WHITE,.7f);ui_button(105,130,270,game.challenge?"X: Mini-games":"X: Choose another level",1);ui_button(105,170,270,"O: Main menu",0);return;}
    if(game.status==LOST){ui_dialog(89,65,302,145,"The zombies got through!");ui_button(114,109,252,"X: Retry this level",1);ui_button(114,148,252,"O: Main menu",0);return;}
    if(game.level%10==9){
        ui_note(game.level/10);
        char next[48];snprintf(next,sizeof(next),"X: ADVENTURE %d-10",(game.level-1)/10+1);ui_hint(next);return;
    }
    ui_dialog(74,54,332,174,game.level==CAMPAIGN_LEVELS?"Adventure complete!":"Level complete!");
    char result[180],found[80];ui_title_case(found,sizeof(found),reward_name());
    snprintf(result,sizeof(result),"You found: %s\n%d zombies defeated",found,game.kills);disc_text(98,97,284,result,WHITE,.77f);
    ui_button(100,154,280,game.level==50?((game.level_flags&32)?"X: Level replay":"X: Watch the ending"):"X: Next level",1);ui_center(96,195,288,"O: Main menu",WHITE,.65f);
}

static void move_cursor(SceCtrlData *pad,float dt){
    uint32_t direction=pad->Buttons&(PSP_CTRL_UP|PSP_CTRL_DOWN|PSP_CTRL_LEFT|PSP_CTRL_RIGHT);
    if(pad->Lx<65)direction|=PSP_CTRL_LEFT;if(pad->Lx>190)direction|=PSP_CTRL_RIGHT;
    if(pad->Ly<65)direction|=PSP_CTRL_UP;if(pad->Ly>190)direction|=PSP_CTRL_DOWN;
    if(!direction)repeat_timer=0;
    else {repeat_timer-=dt;if(repeat_timer<=0){
        if(direction&PSP_CTRL_LEFT)game.cursor_col=(game.cursor_col+COLS-1)%COLS;
        if(direction&PSP_CTRL_RIGHT)game.cursor_col=(game.cursor_col+1)%COLS;
        if(direction&PSP_CTRL_UP)do{game.cursor_row=(game.cursor_row+ROWS-1)%ROWS;}while(!game_row_active(&game,game.cursor_row));
        if(direction&PSP_CTRL_DOWN)do{game.cursor_row=(game.cursor_row+1)%ROWS;}while(!game_row_active(&game,game.cursor_row));
        repeat_timer=.16f;
    }}
}

#include "collection_ui.inc"
#include "extra_ui.inc"
#include "story_ui.inc"
#include "menu_hubs.inc"
#include "menu_motion020.inc"
#include "completion_ui.inc"
static int adventure_ending_pending(void){return game.level==50&&!game.challenge&&!game.extra.category&&game.status==WON&&(game.level_flags&LEVEL_REWARD_COLLECTED)&&!(game.level_flags&32);}
static void start_adventure_ending(void){if(ending_begin(5)){game.level_flags|=32;save_current();}}
static void input(SceCtrlData *pad,float dt){
    uint32_t pressed=pad->Buttons&~previous_buttons;previous_buttons=pad->Buttons;if(screen!=1&&(pressed&PSP_CTRL_CROSS))foley_sample(FX_BUTTONCLICK);else if(screen!=1&&(pressed&(PSP_CTRL_UP|PSP_CTRL_DOWN|PSP_CTRL_LEFT|PSP_CTRL_RIGHT|PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER)))foley_play(FOLEY_BLEEP);
    if(pressed&PSP_CTRL_SELECT)debug_info=!debug_info;
    if(help){if(pressed&(PSP_CTRL_CIRCLE|PSP_CTRL_START|PSP_CTRL_CROSS))help=0;return;}
    if(gallery){gallery_input(pressed);return;}
    if(!ready)return;
    if(screen==24){if(pressed&(PSP_CTRL_CROSS|PSP_CTRL_CIRCLE)){screen=1;challenge_notice=-1;}return;}
    if(completion_ui_input(pressed))return;
    if(avatar_input(pressed))return;
    if(hub_input(pressed))return;
    if(story_input(pressed))return;
    if(extra_menu_input(pressed,pad,dt))return;
    if(screen==0){
        if(pressed&(PSP_CTRL_UP|PSP_CTRL_LTRIGGER))menu_selected=(menu_selected+4)%5;
        if(pressed&PSP_CTRL_LEFT)menu_selected=(int[]){4,0,3,1,2}[menu_selected];
        if(pressed&(PSP_CTRL_DOWN|PSP_CTRL_RTRIGGER))menu_selected=(menu_selected+1)%5;
        if(pressed&PSP_CTRL_RIGHT)menu_selected=(int[]){3,3,4,2,0}[menu_selected];
        if(pressed&PSP_CTRL_CROSS){
            audio_play(CUE_SEED_LIFT);
            if(menu_selected==0){if(can_resume){screen=21;adventure_cursor=0;}else new_game();}
            else if(menu_selected==1){screen=6;more_cursor=0;}
            else if(menu_selected==2){screen=11;collection_cursor=0;}
            else if(menu_selected==3){options_return=0;screen=4;menu_volume_row=0;}
            else {player_hub_row=0;screen=25;}
        }
        if(pressed&PSP_CTRL_START){options_return=0;screen=4;menu_volume_row=0;}return;
    }
    if(screen==23){name_input(pressed);return;}
    if(screen==4){
        if(pressed&PSP_CTRL_UP)menu_volume_row=(menu_volume_row+options_count()-1)%options_count();
        if(pressed&PSP_CTRL_DOWN)menu_volume_row=(menu_volume_row+1)%options_count();
        uint32_t horizontal=pad->Buttons&(PSP_CTRL_LEFT|PSP_CTRL_RIGHT);
        if(!horizontal){volume_repeat=0;if(volume_dirty){audio_queue_settings();volume_dirty=0;}if(ui_settings_dirty)ui_settings_save();}
        else if(menu_volume_row<2||menu_volume_row==3||menu_volume_row==6||(menu_volume_row==7&&performance_option())){volume_repeat-=dt;if(volume_repeat<=0){
            int direction=(horizontal&PSP_CTRL_RIGHT)?1:-1;
            if(menu_volume_row==7&&performance_option())performance_select(direction>0?60:30);
            else if(menu_volume_row==6){classic_dancer=!classic_dancer;ui_settings_dirty=1;}
            else if(menu_volume_row==3){health_bars=(health_bars+direction+4)%4;ui_settings_dirty=1;}
            else{int music=audio_music_volume(),effects=audio_effects_volume();if(menu_volume_row)effects+=direction*5;else music+=direction*5;audio_set_volumes(music,effects);if(menu_volume_row)audio_play(SFX_PLANT);}
            if(menu_volume_row<2)volume_dirty=1;volume_repeat=.16f;
        }}
        if(pressed&PSP_CTRL_CROSS){if(menu_volume_row==2)name_begin(4);
            else if(menu_volume_row==3){health_bars=(health_bars+1)%4;ui_settings_save();}else if(menu_volume_row==6){classic_dancer=!classic_dancer;ui_settings_save();}else if(menu_volume_row==4)help=1;else if(menu_volume_row==5){screen=20;credits_cursor=0;}else if(menu_volume_row==7&&performance_option())performance_select(psp1000_fps==60?30:60);else if(menu_volume_row==options_back())screen=options_return;}
        if(pressed&PSP_CTRL_CIRCLE)screen=options_return;
        if(screen!=4&&volume_dirty){audio_queue_settings();volume_dirty=0;}if(screen!=4&&ui_settings_dirty)ui_settings_save();return;
    }
    if(screen==6){
        int count=more_count();if(more_cursor>=count)more_cursor=count-1;
        if(pressed&PSP_CTRL_UP)more_cursor=(more_cursor+count-1)%count;
        if(pressed&PSP_CTRL_DOWN)more_cursor=(more_cursor+1)%count;
        if(pressed&PSP_CTRL_LEFT)more_horizontal(-1);if(pressed&PSP_CTRL_RIGHT)more_horizontal(1);
        if(pressed&PSP_CTRL_CROSS){int id=more_id(more_cursor);if(id==1&&challenge_enter(0)){screen=7;mini_cursor=0;}else if(id==8&&challenge_enter(2)){screen=15;puzzle_cursor=puzzle_family=0;}else if(id==7&&challenge_enter(1)){screen=14;survival_cursor=0;}else if(id==12)screen=0;}

        if(pressed&PSP_CTRL_CIRCLE)screen=0;return;
    }
    if(screen==9){
        if(pressed&PSP_CTRL_LEFT)store_cursor=(store_cursor+SHOP_VISIBLE_COUNT-1)%SHOP_VISIBLE_COUNT;if(pressed&PSP_CTRL_RIGHT)store_cursor=(store_cursor+1)%SHOP_VISIBLE_COUNT;
        if(pressed&(PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER)){int page=store_cursor<8?0:store_cursor<14?1:store_cursor<22?2:3;static const int starts[]={0,8,14,22};page=(page+((pressed&PSP_CTRL_LTRIGGER)?3:1))%4;store_cursor=starts[page];}
        if(pressed&PSP_CTRL_SQUARE){static const int info[30]={2000,2001,2002,2003,2004,2005,2006,2007,2008,2009,2010,2010,2010,2019,2020,2022,2021,2023,2032,2024,2025,2011,2026,2027,2028,2029,0,2030,2031,2033};int item=shop_visible_items[store_cursor],id=item<30?info[item]:0;if(item==SHOP_SLOTS)id=2011+(player.purchases[item]<4?player.purchases[item]:3);if(id)story_begin(id,9,id,0);return;}
        if(pressed&PSP_CTRL_UP)store_cursor=(store_cursor+SHOP_VISIBLE_COUNT-4)%SHOP_VISIBLE_COUNT;
        if(pressed&PSP_CTRL_DOWN)store_cursor=(store_cursor+4)%SHOP_VISIBLE_COUNT;
        if(pressed&PSP_CTRL_CROSS){int item=shop_visible_items[store_cursor];if(profile_purchase(&player,item)){if(can_resume){profile_apply(&player,&game);profile_apply(&player,&adventure_backup);}if(save_current()){foley_play(FOLEY_COIN);notify("PURCHASED");}}else notify(profile_item_available(&player,item)?"NOT ENOUGH MONEY":"ITEM NOT AVAILABLE");}
        if(pressed&PSP_CTRL_CIRCLE){screen=store_return;store_return=11;}return;
    }
    if(screen==10){
        int count=book_zombies?Z_TYPES:ALL_SEEDS;
        if(pressed&(PSP_CTRL_LEFT|PSP_CTRL_LTRIGGER)){book_cursor=(book_cursor+count-1)%count;book_scroll=0;}
        if(pressed&(PSP_CTRL_RIGHT|PSP_CTRL_RTRIGGER)){book_cursor=(book_cursor+1)%count;book_scroll=0;}
        if(pressed&PSP_CTRL_UP)book_scroll=book_scroll>0?book_scroll-1:0;
        if(pressed&PSP_CTRL_DOWN)book_scroll++;
        if(pressed&PSP_CTRL_TRIANGLE){book_zombies=!book_zombies;book_cursor=book_scroll=0;}
        if(pressed&PSP_CTRL_CIRCLE)screen=11;return;
    }
    if(screen==13){
        if(pressed&(PSP_CTRL_LEFT|PSP_CTRL_LTRIGGER))do{imit_cursor=(imit_cursor+BASE_SEEDS-1)%BASE_SEEDS;}while(!(game.unlocked_mask&(UINT64_C(1)<<imit_cursor)));
        if(pressed&(PSP_CTRL_RIGHT|PSP_CTRL_RTRIGGER))do{imit_cursor=(imit_cursor+1)%BASE_SEEDS;}while(!(game.unlocked_mask&(UINT64_C(1)<<imit_cursor)));
        if(pressed&PSP_CTRL_CROSS){if((game.seed_mask&(UINT64_C(1)<<IMITATER))||seed_count(game.seed_mask)<game.seed_slots){game.imitater_seed=imit_cursor;game.seed_mask|=UINT64_C(1)<<IMITATER;screen=3;}else notify("REMOVE A PLANT BEFORE ADDING ANOTHER");}
        if(pressed&PSP_CTRL_CIRCLE)screen=3;return;
    }
    if(screen==7){
        if(pressed&PSP_CTRL_LTRIGGER)mini_cursor=mini_cursor>=8?mini_cursor-8:16+mini_cursor%4;if(pressed&PSP_CTRL_RTRIGGER)mini_cursor=mini_cursor<12?mini_cursor+8:mini_cursor%8;
        if(pressed&PSP_CTRL_LEFT)mini_cursor=(mini_cursor+19)%20;
        if(pressed&PSP_CTRL_RIGHT)mini_cursor=(mini_cursor+1)%20;
        if(pressed&PSP_CTRL_UP)mini_cursor=(mini_cursor+16)%20;
        if(pressed&PSP_CTRL_DOWN)mini_cursor=(mini_cursor+4)%20;
        if((pressed&PSP_CTRL_CROSS)&&challenge_launch(0,mini_cursor)){if(!replaying)save_current();replaying=1;game_start_mini(&game,mini_cursor,0x600+mini_cursor);read_costs();reset_level_ui();screen=2;}
        if(pressed&PSP_CTRL_CIRCLE)screen=6;return;
    }
    if(screen==8){
        if(pressed&(PSP_CTRL_RIGHT|PSP_CTRL_RTRIGGER))note_cursor=(note_cursor+1)%5;
        if(pressed&(PSP_CTRL_LEFT|PSP_CTRL_LTRIGGER))note_cursor=(note_cursor+4)%5;
        if(pressed&PSP_CTRL_CIRCLE)screen=11;return;
    }
    if(screen==5){
        if(pressed&PSP_CTRL_LEFT)replay_cursor=(replay_cursor+CAMPAIGN_LEVELS-1)%CAMPAIGN_LEVELS;
        if(pressed&PSP_CTRL_RIGHT)replay_cursor=(replay_cursor+1)%CAMPAIGN_LEVELS;
        if(pressed&PSP_CTRL_UP)replay_cursor=(replay_cursor+CAMPAIGN_LEVELS-5)%CAMPAIGN_LEVELS;
        if(pressed&PSP_CTRL_DOWN)replay_cursor=(replay_cursor+5)%CAMPAIGN_LEVELS;
        if(pressed&PSP_CTRL_CROSS){if(progress_played(&progress,replay_cursor+1))start_replay(replay_cursor+1);else notify("PLAY THIS LEVEL IN ADVENTURE FIRST");}
        if(pressed&PSP_CTRL_CIRCLE)screen=21;return;
    }
    if(screen==20){
        if(pressed&(PSP_CTRL_UP|PSP_CTRL_LEFT|PSP_CTRL_DOWN|PSP_CTRL_RIGHT))credits_cursor=(credits_cursor+1)%2;
        if(pressed&PSP_CTRL_CROSS){if(credits_cursor==0)ending_begin(20);else screen=4;}
        if(pressed&PSP_CTRL_CIRCLE)screen=4;return;
    }
    if(screen==2){
        if(pressed&PSP_CTRL_CIRCLE){save_current();screen=0;}
        if(pressed&PSP_CTRL_CROSS){screen=game.mode==MODE_LAWN&&seed_count(game.unlocked_mask)>game.seed_slots?3:1;game.selected=game.mode==MODE_WHACK?GRAVEBUSTER:0;if(screen==1){game.level_flags|=LEVEL_READY;warm_level();}if(!(game.level_flags&8)){int id=story_level_id();game.level_flags|=8;if(id)story_begin(id,screen,game.level==5?2406:0,0);}}
        return;
    }
    if(screen==3){
        normalize_choice();
        if(pressed&(PSP_CTRL_LEFT|PSP_CTRL_LTRIGGER))choose_owned(-1,0);
        if(pressed&(PSP_CTRL_RIGHT|PSP_CTRL_RTRIGGER))choose_owned(1,0);
        if(pressed&PSP_CTRL_UP)choose_owned(-1,1);
        if(pressed&PSP_CTRL_DOWN)choose_owned(1,1);
        if((pressed&PSP_CTRL_CROSS)&&!(game.unlocked_mask&(UINT64_C(1)<<game.selected))){notify("THIS PLANT HAS NOT BEEN UNLOCKED");return;}
        if((pressed&PSP_CTRL_CROSS)&&game.selected==IMITATER){if(game.seed_mask&(UINT64_C(1)<<IMITATER))game.seed_mask&=~(UINT64_C(1)<<IMITATER);else{screen=13;imit_cursor=0;}return;}
        if((pressed&PSP_CTRL_CROSS)&&game_seed_locked(&game,game.selected)){notify("CRAZY DAVE WANTS YOU TO USE THIS PLANT");return;}
        if(pressed&PSP_CTRL_CROSS){if(game.seed_mask&(UINT64_C(1)<<game.selected))game.seed_mask&=~(UINT64_C(1)<<game.selected);else if(seed_count(game.seed_mask)<game.seed_slots)game.seed_mask|=UINT64_C(1)<<game.selected;else notify("REMOVE A PLANT BEFORE ADDING ANOTHER");}
        if((pressed&PSP_CTRL_START)&&game_mini_active(&game,MINI_STARS)&&!(game.seed_mask&(UINT64_C(1)<<STAR))){notify("STARFRUIT IS REQUIRED FOR THIS CHALLENGE");return;}
        if(pressed&PSP_CTRL_START){if(seed_count(game.seed_mask)==game.seed_slots){screen=1;game.level_flags|=LEVEL_READY;warm_level();game.selected=0;if(!game_seed_available(&game,0))game_cycle_seed(&game,1);save_current();}else notify("FILL ALL AVAILABLE SEED SLOTS");}
        if(pressed&PSP_CTRL_CIRCLE)screen=2;return;
    }
    if(game.extra.pending&&game.status==PLAYING){move_cursor(pad,dt);if(pressed&PSP_CTRL_CROSS){if(game.extra.pending==2){if(!(game.level_flags&LEVEL_REWARD_COLLECTED)){if(game.cursor_row!=2||game.cursor_col!=4||reward_delay<0||reward_elapsed<reward_delay+.7f)return;game.level_flags|=LEVEL_REWARD_COLLECTED;reward_collect_time=0;audio_play(CUE_SEED_LIFT);save_current();return;}if(reward_collect_time>=0&&reward_collect_time<.85f)return;}int survival=game.extra.category==EXTRA_SURVIVAL;game_extra_continue(&game);reset_level_ui();if(survival){game.seed_mask=0;screen=3;}save_current();}if(pressed&PSP_CTRL_CIRCLE){save_current();screen=6;}return;}
    if(game.status==WON&&!(game.level_flags&LEVEL_REWARD_COLLECTED)){
        move_cursor(pad,dt);
        if((pressed&PSP_CTRL_CROSS)&&reward_delay>=0&&reward_elapsed>=reward_delay+.70f&&game.cursor_row==reward_row&&game.cursor_col==reward_col){
            if(game_collect_reward(&game)){reward_collect_time=0;audio_play(CUE_SEED_LIFT);save_current();}
        }
        if(pressed&PSP_CTRL_CIRCLE){screen=0;save_current();}return;
    }
    if(game.status!=PLAYING){
        if(game.status==WON&&reward_collect_time>=0&&reward_collect_time<.85f)return;
        if(pressed&PSP_CTRL_CROSS){
            if(adventure_ending_pending())start_adventure_ending();
            else if(replaying){screen=game.extra.category==EXTRA_SURVIVAL?14:game.extra.category?15:game.challenge?7:5;replay_cursor=game.level-1;}
            else if(game.status==LOST){game_restart_adventure(&game);read_costs();reset_level_ui();screen=2;save_current();}
            else if(game_next_level(&game)){read_costs();reset_level_ui();visit_current();screen=2;save_current();}
            else{screen=5;replay_cursor=game.level-1;}
        }
        if(pressed&PSP_CTRL_CIRCLE)screen=0;return;
    }
    if((pressed&PSP_CTRL_START)&&game_mini_action(&game,4)>0)return;
    if(pressed&PSP_CTRL_START){paused=!paused;foley_sample(FX_PAUSE);if(paused){pause_cursor=0;save_current();}}
    if(paused){pause_input(pressed);return;}
    if(game.mode==MODE_HEAVY){float move=(pad->Buttons&PSP_CTRL_LEFT)?-1:(pad->Buttons&PSP_CTRL_RIGHT)?1:0;game_heavy_control(&game,move,(pad->Lx-128)/128.0f,!!(pad->Buttons&PSP_CTRL_CROSS),dt);}
    if(game.mode==MODE_MATCH||game.mode==MODE_AQUARIUM||game.mode==MODE_HEAVY||game.mode==MODE_SLOT){
        if(pressed&PSP_CTRL_SQUARE)game_collect(&game);
        if((pressed&PSP_CTRL_LTRIGGER)&&game.mode!=MODE_SLOT)game_mini_action(&game,2);
        if((pressed&PSP_CTRL_RTRIGGER)&&game.mode!=MODE_SLOT)game_mini_action(&game,3);
        if(pressed&PSP_CTRL_TRIANGLE){if(game_mini_action(&game,1)<0)notify("NOT ENOUGH SUN OR ITEM IS NOT READY");}
        if(game.mode==MODE_MATCH||game.mode==MODE_AQUARIUM){move_cursor(pad,dt);if(pressed&PSP_CTRL_CROSS)game_mini_action(&game,0);return;}
        if(game.mode==MODE_HEAVY)return;
    }
    if(game.extra.category==EXTRA_IZOMBIE){if(pressed&PSP_CTRL_LTRIGGER)game_cycle_seed(&game,-1);if(pressed&PSP_CTRL_RTRIGGER)game_cycle_seed(&game,1);if(pressed&PSP_CTRL_SQUARE)game_collect(&game);move_cursor(pad,dt);if(pressed&PSP_CTRL_CROSS){int result=game_extra_action(&game,0);if(result==-3)notify("NOT ENOUGH SUN");if(result==-6)notify("PLACE THIS ZOMBIE ON THE CORRECT SIDE OF THE LINE");}return;}
    if(pressed&PSP_CTRL_LTRIGGER)game_cycle_seed(&game,-1);
    if(pressed&PSP_CTRL_RTRIGGER)game_cycle_seed(&game,1);
    if((pressed&PSP_CTRL_TRIANGLE)&&game.mode!=MODE_SLOT){if(game.level>=5||game.completion.adventure_run){game.shovel=!game.shovel;foley_play(game.shovel?FOLEY_SHOVEL:FOLEY_DROP);}else notify("SHOVEL UNLOCKS AFTER 1-4");}
    if(pressed&PSP_CTRL_CIRCLE){game.shovel=0;game.cob_aiming=0;}
    if(pressed&PSP_CTRL_SQUARE){if(game.mode==MODE_WHACK){game_place(&game,game.cursor_row,game.cursor_col,game.selected);}else game_collect(&game);}
    move_cursor(pad,dt);
    if(pressed&PSP_CTRL_CROSS){
        if(game.mode==MODE_VASE&&game.vases[game.cursor_row][game.cursor_col]){game_break_vase(&game,game.cursor_row,game.cursor_col);return;}
        if(game.mode==MODE_WHACK){
            int kind=-1;for(int i=0;i<MAX_ZOMBIES;i++){Zombie *z=&game.zombies[i];if(z->used&&z->death<=0&&z->row==game.cursor_row&&fabsf(z->x-center(game.cursor_col))<29){kind=z->armor>0?z->kind:-1;break;}}
            audio_play(CUE_WHACK_SWING);if(game_whack(&game,game.cursor_row,game.cursor_col))audio_play(kind==Z_BUCKET?CUE_WHACK_METAL:kind==Z_CONE?CUE_WHACK_PLASTIC:CUE_WHACK_BONK);return;}
        if(game.cob_aiming){game_cob_fire(&game,game.cursor_row,game.cursor_col);return;}
        if(!game.shovel&&game_cob_select(&game,game.cursor_row,game.cursor_col)){notify("SELECT THE TARGET TILE AND PRESS X");return;}
        if(game.shovel){game_shovel(&game,game.cursor_row,game.cursor_col);game.shovel=0;}
        else {int kind=game_selected_kind(&game),packet_index=game.selected,result=game_place(&game,game.cursor_row,game.cursor_col,kind);if(result==0&&game.mode!=MODE_LAWN)remove_packet_ui(packet_index);if(result<0)foley_sample(FX_BUZZER);if(result==-3)notify("NOT ENOUGH SUN");if(result==-4)notify("SEED IS RECHARGING");if(result==-2)notify("TILE IS OCCUPIED");if(result==-6)notify("BOWL FROM BEHIND THE RED LINE");if(result==-12)notify("PLANT THIS ON ITS REQUIRED BASE PLANT");if(result==-13)notify("CHOOSE WHAT IMITATER WILL COPY");if(result==-11)notify("THIS PLANT NEEDS WATER, A LILY PAD OR A POT");if(result==-9)notify("TILE IS BLOCKED");if(result==-8)notify("DIG UP THE THREE WALL-NUTS FIRST");if(result==0&&kind==MINE)notify("MINE ARMING - 15 SECONDS");}
    }
}
static int menu_volume_selftest(FILE *out){
    int old_music=audio_music_volume(),old_effects=audio_effects_volume(),old_row=menu_volume_row;
    int passed=0,total=0;screen=4;menu_volume_row=0;volume_repeat=0;previous_buttons=0;audio_set_volumes(40,50);
    SceCtrlData pad;memset(&pad,0,sizeof(pad));pad.Lx=pad.Ly=128;
    #define UI_CHECK(label,condition) do{int ok=(condition);passed+=ok;total++;fprintf(out,"menu %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    pad.Buttons=PSP_CTRL_RIGHT;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);
    UI_CHECK("right adjusts only selected music slider",audio_music_volume()==45&&audio_effects_volume()==50);
    pad.Buttons=PSP_CTRL_DOWN;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);pad.Buttons=PSP_CTRL_LEFT;input(&pad,.016f);
    UI_CHECK("down selects independent effects slider",menu_volume_row==1&&audio_music_volume()==45&&audio_effects_volume()==45);
    input(&pad,.16f);pad.Buttons=0;input(&pad,.016f);audio_set_volumes(0,0);
    UI_CHECK("release persists both edited slider values",audio_flush_settings()&&audio_load_settings("audio-settings.dat")&&audio_music_volume()==45&&audio_effects_volume()==40);
    audio_set_volumes(100,0);pad.Buttons=PSP_CTRL_UP;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);pad.Buttons=PSP_CTRL_RIGHT;input(&pad,.016f);
    UI_CHECK("menu slider respects maximum while other bus stays muted",audio_music_volume()==100&&audio_effects_volume()==0);
    audio_set_volumes(old_music,old_effects);audio_save_settings("audio-settings.dat");menu_volume_row=old_row;volume_dirty=0;volume_repeat=0;previous_buttons=0;
    screen=0;fprintf(out,"MENU RESULT %d/%d\n",passed,total);
    #undef UI_CHECK
    return passed==total;
}
static int replay_selftest(FILE *out){
    Game original=game,backup=adventure_backup,disk;Progress old_progress=progress;
    int old_resume=can_resume,old_replaying=replaying,old_screen=screen,passed=0,total=0;
    const char *old_path=adventure_path;adventure_path="selftest-adventure.dat";
    #define REPLAY_CHECK(label,test) do{int ok=(test);passed+=ok;total++;fprintf(out,"replay %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    game_start_level(&game,9,501);game.level_flags|=LEVEL_READY;game.sun=725;can_resume=1;replaying=0;
    progress=(Progress){.played=511,.completed=255};save_current();Game expected=game;
    start_replay(3);REPLAY_CHECK("played level starts an isolated session",replaying&&game.level==3&&screen==2&&adventure_backup.level==9);
    game.sun=999;game.status=WON;save_current();
    REPLAY_CHECK("replay autosave preserves Adventure on disk",game_load(&disk,adventure_path)&&!memcmp(&disk,&expected,sizeof(Game)));
    open_adventure();REPLAY_CHECK("Adventure restores exact level and board after replay",!replaying&&screen==1&&!memcmp(&game,&expected,sizeof(Game)));
    start_replay(10);REPLAY_CHECK("unvisited level cannot start a replay",!replaying&&game.level==9);
    remove(adventure_path);adventure_path=old_path;game=original;adventure_backup=backup;progress=old_progress;
    can_resume=old_resume;replaying=old_replaying;screen=old_screen;reset_level_ui();
    fprintf(out,"REPLAY RESULT %d/%d\n",passed,total);
    #undef REPLAY_CHECK
    return passed==total;
}

#include "award_ui_tests.inc"
#include "collection_ui_tests.inc"
#include "extra_ui_tests.inc"
#include "compact_ui_tests.inc"
#include "presentation_tests012.inc"
#include "menu_tests013.inc"
#include "menu_tests014.inc"
#include "gallery_tests0188.inc"
#include "extra_review.inc"
#include "features018_tests.inc"
#include "chewing_tests0189.inc"
#include "polish_presentation_tests0190.inc"
#include "menu020_tests.inc"
#include "performance_tests0181.inc"

static void review_chapter(int level){
    game_start_level(&game,level,720+level);screen=1;game.sun=100000;game.spawn_timer=100000;game.seed_mask=(UINT64_C(1)<<40)-1;reset_level_ui();
    for(int r=0;r<game_rows(&game);r++)for(int c=0;c<3;c++){
        int kind=game_world(&game)==4?(c==1?KERNEL:c==2?MELON:CABBAGE):game_world(&game)==3?(c==0?SEA:c==1?STAR:CACTUS):(c==0?THREEPEATER:c==1?TORCHWOOD:TALLNUT);
        if(game_water_row(&game,r)){game.support[r][c]=LILY+1;game.support_hp[r][c]=300;}
        game.plants[r][c]=(Plant){.kind=kind,.hp=kind==TALLNUT?8000:300};
    }
    int start=game_world(&game)==4?Z_BUNGEE:game_world(&game)==3?Z_JACK:Z_DUCK;
    for(int k=start;k<Z_BOSS;k++){if(k==Z_YETI)continue;int row=(k-start)%game_rows(&game);if(game_water_row(&game,row)&&k!=Z_DUCK&&k!=Z_SNORKEL&&k!=Z_DOLPHIN)row=0;int i=game_spawn_zombie(&game,row,k,270+(k%3)*45);if(k==Z_BUNGEE)game.zombies[i].x=210;}
    game.seed_mask=game_world(&game)==4?((UINT64_C(1)<<CABBAGE)|(UINT64_C(1)<<POT)|(UINT64_C(1)<<KERNEL)|(UINT64_C(1)<<MELON)|(UINT64_C(1)<<UMBRELLA)|(UINT64_C(1)<<JALAPENO)):((UINT64_C(1)<<LILY)|(UINT64_C(1)<<THREEPEATER)|(UINT64_C(1)<<TORCHWOOD)|(UINT64_C(1)<<CACTUS)|(UINT64_C(1)<<BLOVER)|(UINT64_C(1)<<PLANTERN));game.selected=game_world(&game)==4?CABBAGE:LILY;
}

static void review_deaths(float phase){
    game_start_level(&game,9,1120);screen=1;reset_level_ui();game.spawn_timer=100000;
    int kinds[]={Z_NORMAL,Z_POLE,Z_PAPER,Z_FOOTBALL,Z_JACK,Z_POGO,Z_GARG,Z_IMP};
    for(int i=0;i<8;i++){int id=game_spawn_zombie(&game,i<4?1:3,kinds[i],100+(i%4)*100);Zombie *z=&game.zombies[id];z->hp=0;z->vault_from=z->kind==Z_GARG?14:27;
        float duration=game_death_duration(z),fall=duration-.60f;z->age=phase<=1?fall*phase+.02f:duration-.15f;z->death=fmaxf(.01f,duration-z->age);}
}
static void review_win(int level,int mini,int claimed){
    if(mini)game_start_minigame(&game,1,1140);else game_start_level(&game,level,1140+level);screen=1;game.status=WON;reset_level_ui();
    if(claimed)game.level_flags|=LEVEL_REWARD_COLLECTED;
    update_reward(level==50?8:1);
}

/* Display pacing is independent of the original fixed-step game clock. */
static int advance_game_frame(Game *g,float elapsed,float *remainder){
    int updates=0;*remainder+=elapsed;
    while(*remainder>=1.0f/60){game_step(g,1.0f/60);*remainder-=1.0f/60;updates++;}
    return updates;
}
#include "performance_live0181.inc"
static int performance_selftest(FILE *out){
    Game a,b;game_start_level(&a,9,4517);a.level_flags|=LEVEL_READY;a.spawn_timer=1;
    a.sun=1000;game_place(&a,2,0,PEA);game_spawn_zombie(&a,2,Z_NORMAL,320);b=a;
    float ra=0,rb=0;int na=0,nb=0;
    for(int n=0;n<360;n++)na+=advance_game_frame(&a,1.0f/60,&ra);
    for(int n=0;n<180;n++)nb+=advance_game_frame(&b,1.0f/30,&rb);
    int ok=na==360&&nb==360&&a.time>5.99f&&a.time<6.01f&&a.total_spawned>1&&!memcmp(&a,&b,sizeof(a));
    fprintf(out,"performance 30 and 60 FPS preserve identical six-second combat and spawn state: %s\nPERFORMANCE RESULT %d/1\n",ok?"PASS":"FAIL",ok);return ok;
}

#include "completion_presentation_tests.inc"
int main(int argc,char *argv[]){
    uint64_t boot_start;sceRtcGetCurrentTick(&boot_start);int first_frame_logged=0;
    setup_callbacks();scePowerSetClockFrequency(333,333,166);sceCtrlSetSamplingCycle(0);sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    if(argc>0&&argv[0]){char directory[512];snprintf(directory,sizeof(directory),"%s",argv[0]);char *slash=strrchr(directory,'/');if(slash){*slash=0;chdir(directory);}}
    hardware=platform_detect();performance_load();performance_apply();animation_cache_configure(performance.low_memory);ui_settings_load();player_name_load();avatar_load();
    FILE *log=fopen("runtime.log","w");performance_log=log;if(log){fprintf(log,BUILD018_LABEL "\nModel: %s (id %d)\nWLAN hardware: %s\nWireless modes implemented: NO\nCWD: ",hardware.name,hardware.model,hardware.wlan?"YES":"NO");char cwd[512];if(getcwd(cwd,sizeof(cwd)))fprintf(log,"%s\n",cwd);}
    FILE *flag=fopen("benchmark.flag","rb");int benchmark=flag!=0;if(flag)fclose(flag);FILE *live_flag=fopen("live-benchmark.flag","rb");live_benchmark=live_flag!=0;if(live_flag)fclose(live_flag);FILE *ui_flag=fopen("review-ui.flag","rb");review_ui_only=ui_flag!=0;if(ui_flag)fclose(ui_flag);
    int motion_ok=game_load_motion("data/zombie_motion.bin");if(log){fprintf(log,"PS3 ground-motion profiles: %s\n",motion_ok?"OK":"MISSING");fflush(log);}
    test_ok=1;
    if(benchmark){
    FILE *tests=fopen("selftest.txt","w");test_ok=tests?game_selftest(tests):0;
    if(tests){test_ok&=game_mini_selftest(tests);test_ok&=game_extra_selftest(tests);test_ok&=garden_selftest(tests);test_ok&=platform_selftest(tests);test_ok&=performance_selftest(tests);test_ok&=audio_selftest(tests);test_ok&=foley_selftest(tests);test_ok&=game_audio018_selftest(tests);test_ok&=music_selftest(tests);test_ok&=progress_selftest(tests);test_ok&=profile_selftest(tests);int ok=render_selftest();test_ok&=ok;fprintf(tests,"texture swizzle pixel integrity: %s\n",ok?"PASS":"FAIL");fclose(tests);}
    }
    profile_init(&player);profile_load(&player,profile_path);challenge_initialize();
    game_init(&game,1);can_resume=game_load(&game,"save.dat");if(profile_load_backup_used()||game_load_backup_used())notify("RECOVERED YOUR LAST BACKUP SAVE");ready=motion_ok&&load_shoot_alignment();
    progress_load(&progress,progress_path);if(can_resume){progress_import(&progress,&game);profile_apply(&player,&game);adventure_backup=game;}for(int l=1;l<=50;l++)if(progress_played(&progress,l)&&(uint32_t)l>player.highest)player.highest=l;reset_level_ui();
    int catalog_ok=animation_catalog_load("data/assets.idx");if(log)fprintf(log,"Animation metadata index: %s\n",catalog_ok?"loaded":"individual-file fallback");
    for(int i=0;i<A_COUNT;i++){char path[128];snprintf(path,sizeof(path),"data/%s.pva",asset_names[i]);
        if(!animation_load(&assets[i],path)){ready=0;if(log)fprintf(log,"FAILED %s\n",path);}
        else texture_memory+=(size_t)assets[i].frames*assets[i].stride;
        for(int stage=1;stage<=2;stage++)if(damage_mask(i)&(1<<(stage-1))){
            snprintf(path,sizeof(path),"data/%s_damage%d.pva",asset_names[i],stage);
            if(!animation_load(&damaged[i][stage-1],path)){ready=0;if(log)fprintf(log,"FAILED %s\n",path);}
            else texture_memory+=(size_t)damaged[i][stage-1].frames*damaged[i][stage-1].stride;
        }
    }
    if(!menu_motion_load("data/menu-motion.bin")){ready=0;if(log)fprintf(log,"Menu motion metadata: FAILED\n");}
    for(int i=0;i<5;i++){char path[128];snprintf(path,sizeof(path),"data/%s.pva",head_particle_names[i]);if(!animation_load(&head_particles[i],path)){ready=0;if(log)fprintf(log,"FAILED %s\n",path);}}
    if(log){fprintf(log,"Assets: %s\nTexture bytes: %lu\nSelf tests: %s\nGame struct: %lu bytes\nUser partition free: %u bytes\n",ready?"OK":"MISSING",(unsigned long)texture_memory,benchmark?(test_ok?"PASS":"FAIL"):"skipped (normal startup)",(unsigned long)sizeof(Game),(unsigned)sceKernelTotalFreeMemSize());
        fprintf(log,"Memory configuration: automatic heap, 1 MiB system reserve / %d MiB frame cache / %d MiB banks / %d MiB static cap\nRender target: %d FPS; simulation: 60 updates/s\n",performance.low_memory?12:24,performance.low_memory?2:8,performance.low_memory?0:6,performance.target_fps);
        fflush(log);}
    ready=ready&&animation_warm(&assets[A_MENU_MAIN])&&animation_warm(&assets[A_DISC_FONT]);
    disc_font_init(&assets[A_DISC_FONT]);
    render_init();audio_init();if(audio_preferred_track()>=0){audio_set_preferred_track(-1);audio_queue_settings();}audio_write_status(log);
    if(benchmark&&ready){
        FILE *ui_tests=fopen("selftest.txt","a");if(ui_tests){test_ok&=animation_cache_layout_selftest(ui_tests);test_ok&=animation_catalog_selftest(ui_tests);test_ok&=foley_data_selftest(ui_tests);test_ok&=foley_runtime_selftest(ui_tests);test_ok&=features018_selftest(ui_tests);test_ok&=movie_selftest(ui_tests);test_ok&=performance_settings_selftest(ui_tests);test_ok&=game_campaign_selftest(ui_tests);test_ok&=award_ui_selftest(ui_tests);test_ok&=death_asset_selftest(ui_tests);test_ok&=menu_volume_selftest(ui_tests);test_ok&=audio_stream_data_selftest(ui_tests);test_ok&=replay_selftest(ui_tests);test_ok&=collection_ui_selftest(ui_tests);test_ok&=extra_ui_selftest(ui_tests);test_ok&=compact_ui_selftest(ui_tests);test_ok&=presentation_selftest(ui_tests);test_ok&=menu013_selftest(ui_tests);test_ok&=menu014_selftest(ui_tests);test_ok&=customizer_selftest(ui_tests);test_ok&=gallery_io_selftest(ui_tests);test_ok&=chewing_selftest(ui_tests);test_ok&=polish_presentation_selftest(ui_tests);test_ok&=menu020_selftest(ui_tests);test_ok&=completion_presentation_selftest(ui_tests);fclose(ui_tests);}
        new_game();game_start_level(&game,9,123);screen=1;debug_info=1;game.sun=100000;game.spawn_timer=100000;game.seed_mask=255;
        for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++){int kind=(r+c)%SEEDS;*game_seed_cooldown(&game,kind)=0;game_place(&game,r,c,kind);}
        for(int i=0;i<30;i++){
            int id=game_spawn_zombie(&game,i%ROWS,i%10,240+(i/ROWS)*40),stage=(i/5)%3;
            Zombie *z=&game.zombies[id];if(z->kind==Z_CONE||z->kind==Z_BUCKET){int max=z->kind==Z_CONE?370:1100;z->armor=max*(stage==1?.60f:stage==2?.25f:1);}
            else z->hp=(z->kind==Z_POLE?500:270)*(stage==1?.60f:stage==2?.25f:1);
        }
        for(int i=0;i<30;i++){game.shots[i]=(Shot){.used=1,.row=i%5,.ice=i%2,.x=120+(i/5)*43};game.shot_y[i]=baseline(i%5)-17;}
        for(int i=0;i<30;i++)game.suns[i]=(Sun){.used=1,.x=80+(i%10)*36,.y=100+(i/10)*50,.target_y=200};
        game.plants[0][1].kind=SUNFLOWER;game.plants[0][1].attack=.25f;
        memset(game.events,0,sizeof(game.events));
        if(log){fprintf(log,"Benchmark scene ready, loading daytime music\n");fflush(log);}
        audio_select_music(MUSIC_DAY);
        if(log){fprintf(log,"Benchmark music ready\n");fflush(log);}
    }
    game_sound_listener(native_sound);
    if(benchmark&&live_benchmark)live_combat_setup(0);
    #ifdef TEST_PSP1000_60FPS
    const float review_start=live_benchmark?133:67;
    if(log){fprintf(log,"60 FPS experiment: 65.5-second dense animated scene before UI review\n");fflush(log);}
    #else
    const float review_start=live_benchmark?133:7;
    #endif
    int review_phase=0,music_test=0,mini_review=-1,extra_review=-1,completion_review=-1;FILE *stream_tests=0;
    uint64_t previous; sceRtcGetCurrentTick(&previous);float clock=0,accumulator=0,fps=60,fps_seconds=0;unsigned int fps_frames=0;
    while(running){
        uint64_t now;sceRtcGetCurrentTick(&now);float dt=(now-previous)/(float)sceRtcGetTickResolution();previous=now;
        float elapsed=dt;dt=fminf(.1f,fmaxf(.001f,dt));clock+=dt;message_time=fmaxf(0,message_time-dt);
        // Frames / elapsed time stays accurate when frame intervals vary.
        // Averaging reciprocal intervals can misleadingly report more than 60 FPS.
        fps_seconds+=elapsed;fps_frames++;
        if(fps_seconds>=.5f){fps=fps_frames/fps_seconds;fps_seconds=0;fps_frames=0;}
        int was_paused=paused;SceCtrlData pad;sceCtrlPeekBufferPositive(&pad,1);if(!benchmark)input(&pad,dt);
        menu_motion_step(dt,!benchmark);
        if(!benchmark)game.completion.cosmetics=player.completion.cosmetics;
        int old_status=game.status,old_wave=game.wave,old_vase_stage=game.stage;float old_huge=game.huge_wave_timer;
        if(!benchmark&&screen==1&&!paused&&!help&&game.status==PLAYING)advance_game_frame(&game,dt,&accumulator);
        else if(!(benchmark&&live_benchmark&&review_phase==0))accumulator=0;
        if(!benchmark&&screen==1&&!game.challenge&&game.level==35&&old_vase_stage!=game.stage&&game.status==PLAYING)story_begin(game.stage==1?2700:2800,1,0,0);
        if(!benchmark&&screen==1&&game.status==WON){game_animate_pickups(&game,dt);if(old_status==WON){game_animate_deaths(&game,dt);game_animate_attacks(&game,dt);}}
        if(!benchmark&&screen==1){uint64_t gates=player.garden.story_seen;if(profile_bank(&player,&game)){profile_apply(&player,&game);persist_profile();uint64_t fresh=(player.garden.story_seen^gates)&(CHALLENGE_MINI|CHALLENGE_PUZZLE);if(fresh){challenge_notice=(fresh&CHALLENGE_MINI)?0:2;screen=24;foley_play(FOLEY_PRIZE);}}}
        if(!benchmark&&old_status!=game.status){if(game.status==WON&&!game.challenge){progress_visit(&progress,game.level,1);progress_save(&progress,progress_path);}save_current();}
        if(!benchmark&&old_huge<=0&&game.huge_wave_timer>0)audio_play(CUE_HUGE_WAVE);
        if(!benchmark&&old_wave!=game.wave&&game.wave==game.num_waves)audio_play(CUE_FINAL_WAVE);
        if(screen==1&&!paused&&!help){update_level_ui(dt);update_reward(dt);}
        if(!benchmark&&screen==1&&game.level==5&&!game.challenge&&!game.completion.adventure_run&&!game.shovel_tutorial&&(game.level_flags&8)&&!(game.level_flags&16)){game.level_flags|=16;story_begin(2408,1,0,0);}
        if(!benchmark&&screen==1&&!game.challenge&&!game.completion.adventure_run&&game.status==WON&&(game.level_flags&LEVEL_REWARD_COLLECTED)&&reward_collect_time>1.5f&&!(game.level_flags&64)&&(game.level==24||game.level==34||game.level==44)){game.level_flags|=64;if(game.level==44){garden_initialize(&player,(int64_t)time(NULL));garden_return=1;story_begin(2100,16,0,0);}else{store_return=1;store_cursor=0;if(game.level==34){game.money_collected+=100;profile_bank(&player,&game);}story_begin(game.level==24?301:601,9,0,0);}save_current();}
        if(!benchmark&&screen==1&&game.challenge&&game.status==WON&&reward_collect_time>1.5f&&player.completion.gold_sunflower==1){completion_return=1;screen=28;}
        if(!benchmark&&screen==1&&reward_collect_time>1.5f&&adventure_ending_pending())start_adventure_ending();
        if(screen==18)story_step(dt);if(screen==19)ending_step(dt);
        if(benchmark&&live_benchmark&&review_phase==0){
            if(!live_phase&&clock>=67){write_profile(log,"LIVE DAY BENCHMARK");live_combat_report(log,"DAY");render_debug_frame("live-day.ppm");profile_seconds=profile_cpu=profile_gpu=0;profile_frames=0;live_phase=1;live_combat_setup(1);}
            live_combat_step(dt,&accumulator);
        }
        if(benchmark&&!(live_benchmark&&review_phase==0)){game.time=clock;for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)game.plants[r][c].age=clock+16;for(int i=0;i<MAX_ZOMBIES;i++)if(game.zombies[i].used&&game.zombies[i].death<=0)game_zombie_walk(&game.zombies[i],dt);}
        if(screen==16){garden_clock+=dt;int64_t now=(int64_t)time(NULL);garden_step(&player,garden_view,dt,now);garden_present_step(dt);if(!garden_pending()&&now-garden_saved_time>=30){persist_profile();garden_saved_time=now;}}
        challenge_step(dt);
        int context=soundtrack_context();
        int preferred=audio_preferred_track();if(ready)audio_select_music(music_test&&preferred>=0?preferred:context);
        audio_music(ready&&!help&&!(screen==19&&ending_paused)&&(screen!=1||(!paused&&(game.status==PLAYING||(game.status==WON&&game.level==50))))&&!(screen==1&&game.level==40));
        audio_scene_step(dt);memset(game.events,0,sizeof(game.events));
        render_begin();
        if(!ready){panel("GAME DATA REQUIRED");render_text(56,106,"RUN SETUP.EXE ON YOUR COMPUTER\nWITH YOUR OWN SUPPORTED PS3 ISO.\n\nCOPY THE RESULTING PVZPSP FOLDER\nINTO PSP/GAME ON YOUR MEMORY CARD.\n\nSEE THE INCLUDED README.",WHITE,1);}
        else if(gallery)draw_gallery(clock);
        else if(screen==0)draw_menu();
        else if(screen==2)draw_intro();
        else if(screen==3)draw_seed_choice();
        else if(screen==4)draw_options();
        else if(screen==5)draw_replay();
        else if(screen==6)draw_more_ways();
        else if(screen==11)draw_collection_menu();
        else if(screen==21)draw_adventure_menu();
        else if(screen==22)draw_garden_picker();
        else if(screen==9)draw_store();
        else if(screen==10)draw_book();
        else if(screen==13)draw_imitater();
        else if(screen==14)draw_survival_menu();
        else if(screen==15)draw_puzzle_menu();
        else if(screen==16)draw_garden();
        else if(screen==17)draw_tree();
        else if(screen==18)draw_story();
        else if(screen==19){render_movie(movie_pixels());if(ending_paused){ui_dialog(120,88,240,101,"Paused");ui_center(136,135,208,"Start: Resume   O: Skip",WHITE,.7f);}}
        else if(screen==20)draw_credits_menu();
        else if(screen==23)draw_player_name();
        else if(screen==25)draw_player_hub();
        else if(screen==26)draw_customizer(clock);
        else if(screen==28)draw_completion_trophies();
        else if(screen==29)draw_house_choice();
        else if(screen==24)challenge_notice_draw();
        else if(screen==7)draw_mini_menu();
        else if(screen==8){render_background(&assets[A_MENU_BG]);ui_note(note_cursor);char caption[96];snprintf(caption,sizeof(caption),"Zombie note %d/5   L/R: Next   O: Back",note_cursor+1);ui_hint(caption);}
        else {draw_board();draw_hud(fps);
            if(paused)draw_pause_menu();
            if(game.status==WON&&!(game.level_flags&LEVEL_REWARD_COLLECTED))draw_reward();else if(game.status!=PLAYING)draw_result();
        }
        if(help)draw_help();if(message_time>0&&screen!=1&&screen!=19&&screen!=23&&screen!=25&&screen!=26){ui_dialog(65,207,350,44,"");disc_text(82,218,316,message,UI_GOLD,.62f);}render_end();last_render=render_stats();if(!animation_cache_ok())ready=0;
        if(!first_frame_logged){uint64_t boot_end;sceRtcGetCurrentTick(&boot_end);if(log){fprintf(log,"STARTUP FIRST FRAME: %.3f seconds; diagnostics %s\n",(boot_end-boot_start)/(double)sceRtcGetTickResolution(),benchmark?"enabled":"disabled");fflush(log);}first_frame_logged=1;}
        if(benchmark&&screen==19)ending_review_frame();
        if(screen==1&&!paused&&!help&&game.status==PLAYING&&(!benchmark||(review_phase==0&&clock>1.5f))){profile_seconds+=elapsed;profile_cpu+=last_render.cpu_ms;profile_gpu+=last_render.gpu_wait_ms;profile_frames++;}
        if(!was_paused&&paused)write_profile(log,"PAUSE");
        if(music_test&&audio_stream_test_step(stream_tests,elapsed)){if(stream_tests)fclose(stream_tests);stream_tests=0;running=0;}
        if(benchmark&&!music_test&&screen!=19&&clock>review_start+review_phase*.2f){
            switch(review_phase++){
                case 0:if(live_benchmark){write_profile(log,"LIVE POOL BENCHMARK");live_combat_report(log,"POOL");foley_loop(FOLEY_RAIN,0);live_benchmark=0;}write_profile(log,"BENCHMARK");render_debug_frame("benchmark.ppm");{
                    FILE *done=fopen("benchmark-complete.txt","w");if(done){fprintf(done,"FPS %.2f\nDRAW_MS %.3f\nGPU_WAIT_MS %.3f\n",profile_frames/profile_seconds,profile_cpu/profile_frames,profile_gpu/profile_frames);fclose(done);}
                    game_start_level(&game,2,123);reset_level_ui();screen=2;debug_info=0;break;}
                case 1:render_debug_frame("review-intro.ppm");game_start_level(&game,9,123);screen=3;break;
                case 2:render_debug_frame("review-seeds.ppm");game_start_level(&game,1,123);screen=1;break;
                case 3:render_debug_frame("review-first-lawn.ppm");game_start_level(&game,2,123);game.sun=500;game_place(&game,2,1,SUNFLOWER);game.plants[2][1].attack=.2f;reset_level_ui();screen=1;break;
                case 4:render_debug_frame("review-sunflower.ppm");game_start_level(&game,5,123);for(int c=0;c<3;c++)game_shovel(&game,2,c);game_place(&game,2,1,WALLNUT);game.nuts[0].x=250;game_spawn_zombie(&game,1,Z_CONE,370);game_spawn_zombie(&game,3,Z_NORMAL,410);break;
                case 5:render_debug_frame("review-bowling.ppm");game_start_level(&game,9,123);game.status=WON;game.level_flags|=LEVEL_REWARD_COLLECTED;reset_level_ui();break;
                case 6:render_debug_frame("review-note.ppm");game_start_level(&game,10,123);game.status=WON;game.level_flags|=LEVEL_REWARD_COLLECTED;reset_level_ui();break;
                case 7:render_debug_frame("review-chapter-complete.ppm");screen=0;break;
                case 8:render_debug_frame("review-menu.ppm");game_start_level(&game,9,123);screen=1;debug_info=0;game.sun=1000;
                    for(int r=0;r<4;r++)for(int stage=0;stage<3;stage++){
                        int kind=r==0?Z_NORMAL:r==1?Z_CONE:r==2?Z_BUCKET:Z_POLE,id=game_spawn_zombie(&game,r,kind,210+stage*83);Zombie *z=&game.zombies[id];
                        if(r==1||r==2){int max=r==1?370:1100;z->armor=max*(stage==1?.6f:stage==2?.25f:1);}
                        else{z->hp=(r==3?500:270)*(stage==1?.6f:stage==2?.25f:1);if(r==3){z->vaulted=1;z->speed=.3f;}}
                    }
                    game.seed_mask=255;game_place(&game,4,0,PEA);game_place(&game,4,1,SNOW);
                    for(int i=0;i<8;i++)game.shots[i]=(Shot){.used=1,.row=4,.ice=i%2,.x=160+i*32};break;
                case 9:render_debug_frame("review-damage.ppm");game_start_level(&game,10,123);screen=1;game.packet_count=10;game.selected=9;
                    {int seeds[10]={PEA,CHERRY,WALLNUT,REPEATER,SNOW,CHOMPER,MINE,PEA,CHERRY,REPEATER};memcpy(game.packets,seeds,sizeof(seeds));}reset_level_ui();break;
                case 10:render_debug_frame("review-conveyor-hud.ppm");game.huge_wave_timer=7;break;
                case 11:render_debug_frame("review-huge-wave.ppm");game.huge_wave_timer=0;screen=4;menu_volume_row=2;break;
                case 12:render_debug_frame("review-options.ppm");screen=5;progress.played=31;progress.completed=15;replay_cursor=4;break;
                case 13:render_debug_frame("review-replay.ppm");game_start_level(&game,9,124);screen=1;game.seed_mask=255;game.sun=1000;game.spawned=1;game.wave=12;reset_level_ui();meter_progress=.58f;
                    game_place(&game,3,1,PEA);game_place(&game,3,2,REPEATER);game_place(&game,3,3,MINE);game.plants[3][3].age=MINE_ARM_SECONDS;
                    game_spawn_zombie(&game,2,Z_FLAG,345);game_spawn_zombie(&game,3,Z_NORMAL,360);
                    game.suns[0]=(Sun){.used=1,.x=210,.y=185,.target_y=185};break;
                case 14:render_debug_frame("review-sprites.ppm");game_collect(&game);game_animate_pickups(&game,.2f);break;
                case 15:render_debug_frame("review-sun-collection.ppm");game_animate_pickups(&game,1);break;
                case 16:render_debug_frame("review-sun-collected.ppm");game.sun=0;break;
                case 17:render_debug_frame("review-counter-0.ppm");game.sun=25;break;
                case 18:render_debug_frame("review-counter-25.ppm");game.sun=150;break;
                case 19:render_debug_frame("review-counter-150.ppm");game.sun=9990;break;
                case 20:render_debug_frame("review-counter-9990.ppm");game.sun=1000000;break;
                case 21:render_debug_frame("review-counter-1000000.ppm");game_start_level(&game,19,612);screen=1;game.sun=1000;game.seed_mask=0xff00;game.selected=PUFF;game.spawned=1;game.wave=11;reset_level_ui();meter_progress=.55f;
                    for(int k=PUFF;k<16;k++){game.graves[k%5][(k-PUFF)/5]=0;game_place(&game,k%5,(k-PUFF)/5,k);}
                    for(int k=Z_PAPER;k<10;k++)game_spawn_zombie(&game,k-Z_PAPER,k,325+(k%2)*25);break;
                case 22:render_debug_frame("review-night.ppm");game_start_level(&game,12,613);screen=3;game.selected=SUNSHROOM;break;
                case 23:render_debug_frame("review-night-seeds.ppm");game_start_level(&game,19,613);game.status=WON;game.level_flags|=LEVEL_REWARD_COLLECTED;reset_level_ui();screen=1;break;
                case 24:render_debug_frame("review-note2.ppm");screen=5;progress.played=0xfffff;progress.completed=0x7ffff;replay_cursor=19;break;
                case 25:render_debug_frame("review-night-replay.ppm");screen=8;note_cursor=4;break;
                case 26:render_debug_frame("review-final-note-gallery.ppm");game_start_minigame(&game,1,614);screen=1;game.spawn_timer=0;game_step(&game,.05f);reset_level_ui();break;
                case 27:render_debug_frame("review-whack.ppm");screen=7;break;
                case 28:render_debug_frame("review-minigames.ppm");review_chapter(29);break;
                case 29:render_debug_frame("review-pool.ppm");review_chapter(39);break;
                case 30:render_debug_frame("review-fog.ppm");review_chapter(49);break;
                case 31:render_debug_frame("review-roof.ppm");game_start_level(&game,35,750);reset_level_ui();break;
                case 32:render_debug_frame("review-vasebreaker.ppm");review_chapter(45);break;
                case 33:render_debug_frame("review-bungee.ppm");review_chapter(50);break;
                case 34:render_debug_frame("review-boss.ppm");screen=5;replay_cursor=49;progress.played=(UINT64_C(1)<<50)-1;progress.completed=(UINT64_C(1)<<49)-1;break;
                case 35:render_debug_frame("review-all-levels.ppm");review_deaths(0);break;
                case 36:render_debug_frame("review-death-start.ppm");review_deaths(.5f);break;
                case 37:render_debug_frame("review-death-fall.ppm");review_deaths(1);break;
                case 38:render_debug_frame("review-death-ground.ppm");review_deaths(1.15f);break;
                case 39:render_debug_frame("review-death-fade.ppm");review_win(15,1,0);break;
                case 40:render_debug_frame("review-trophy-pickup.ppm");review_win(1,0,0);break;
                case 41:render_debug_frame("review-seed-pickup.ppm");review_win(9,0,0);break;
                case 42:render_debug_frame("review-note-pickup.ppm");review_win(50,0,0);break;
                case 43:render_debug_frame("review-silver-pickup.ppm");review_win(24,0,0);break;
                case 44:render_debug_frame("review-keys-pickup.ppm");review_win(34,0,0);break;
                case 45:render_debug_frame("review-taco-pickup.ppm");review_win(44,0,0);break;
                case 46:render_debug_frame("review-watering-pickup.ppm");review_win(14,0,0);break;
                case 47:render_debug_frame("review-almanac-pickup.ppm");review_win(4,0,0);break;
                case 48:render_debug_frame("review-shovel-pickup.ppm");profile_init(&player);player.highest=50;player.adventure_finished=1;player.money=12345;screen=9;store_cursor=0;break;
                case 49:render_debug_frame("review-store.ppm");store_cursor=11;break;
                case 50:render_debug_frame("review-store-upgrades.ppm");screen=10;book_zombies=0;book_cursor=0;book_scroll=0;player.seen_plants=(UINT64_C(1)<<ALL_SEEDS)-1;player.seen_zombies=(1u<<Z_TYPES)-1;break;
                case 51:render_debug_frame("review-almanac-plant.ppm");book_zombies=1;book_cursor=25;book_scroll=0;break;
                case 52:render_debug_frame("review-almanac-zombie.ppm");game_start_level(&game,49,1500);game.unlocked_mask=(UINT64_C(1)<<ALL_SEEDS)-1;game.seed_slots=10;game.selected=40;screen=3;break;
                case 53:render_debug_frame("review-upgrade-seeds.ppm");screen=13;imit_cursor=MINE;break;
                case 54:render_debug_frame("review-imitater-choice.ppm");game_start_level(&game,9,1501);game.seed_mask=game.unlocked_mask=(UINT64_C(1)<<ALL_SEEDS)-1;game.sun=100000;game.seed_slots=10;screen=1;
                    for(int k=40;k<48;k++){int r=(k-40)/4,c=((k-40)%4)*2;game.plants[r][c]=(Plant){.kind=k,.hp=300};if(k==COB)game.plants[r][c+1].kind=-2;}
                    game.plants[3][1]=(Plant){.kind=PEA,.hp=300};game.imitated[3][1]=1;game.plants[3][3]=(Plant){.kind=WALLNUT,.hp=1000};game.imitated[3][3]=1;
                    game.seed_mask=0;for(int k=39;k<49;k++)game.seed_mask|=UINT64_C(1)<<k;game.selected=GATLING;game.imitater_seed=MINE;
                    for(int i=0;i<MAX_COINS;i++)game.money[i].used=0;game_add_coin(&game,150,180,1);game_add_coin(&game,220,180,5);game_add_coin(&game,290,180,100);reset_level_ui();break;
                case 55:render_debug_frame("review-upgrades-and-imitater.ppm");game_collect_coins(&game);game_animate_pickups(&game,.2f);break;
                case 56:render_debug_frame("review-coin-collection.ppm");screen=6;more_cursor=3;break;
                case 57:render_debug_frame("review-expanded-menu.ppm");review_win(1,0,1);break;
                case 58:render_debug_frame("review-new-plant-award.ppm");review_win(1,0,1);reward_collect_time=.38f;break;
                case 59:render_debug_frame("review-plant-pickup-flight.ppm");mini_review=0;break;
                default:
                    if(mini_review>=0&&mini_review<20){
                        if(mini_review>0){char file[80];snprintf(file,sizeof(file),"review-mini-%02d.ppm",mini_review-1);render_debug_frame(file);}
                        game_start_mini(&game,mini_review,2200+mini_review);read_costs();reset_level_ui();game.sun=500;game.mini.phase=1;game.spawn_timer=0;game_step(&game,.05f);screen=1;mini_review++;
                    }else if(mini_review==20){render_debug_frame("review-mini-19.ppm");extra_review=0;mini_review++;}else if(extra_review>=0){if(review_expansion(extra_review++)){extra_review=-1;completion_review=0;}}else if(completion_review>=0){if(review_completion(completion_review++)){screen=4;menu_volume_row=2;stream_tests=fopen("selftest.txt","a");audio_stream_test_begin(performance.target_fps);music_test=1;}}break;
            }
        }
    }
    if(benchmark){int ok=animation_cache_ok();test_ok&=ok;FILE *checks=fopen("selftest.txt","a");if(checks){fprintf(checks,"all captured scenes and ending finish without cache allocation or decode errors: %s\nASSET CACHE RESULT %d/1\n",ok?"PASS":"FAIL",ok);fclose(checks);}}
    game_sound_listener(0);foley_report(log);
    if(!benchmark){garden_flush();save_current();progress_save(&progress,progress_path);}
    write_profile(log,"EXIT");
    if(!benchmark){audio_queue_settings();if(ui_settings_dirty)ui_settings_save();if(performance_dirty)performance_save();}
    movie_close();menu_motion_close();audio_shutdown();render_shutdown();for(int i=0;i<2;i++)animation_free(&gallery_thumbnails[i]);for(int i=0;i<AV_COUNT;i++)animation_free(&avatar_art[i]);for(int i=0;i<5;i++)animation_free(&head_particles[i]);for(int i=0;i<A_COUNT;i++){animation_free(&assets[i]);for(int stage=0;stage<3;stage++)animation_free(&imitated_assets[i][stage]);for(int stage=0;stage<2;stage++)animation_free(&damaged[i][stage]);}animation_catalog_close();animation_cache_report(log);if(log)fclose(log);sceKernelExitGame();return 0;
}
