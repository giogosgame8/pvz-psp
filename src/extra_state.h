#ifndef PVZ_EXTRA_STATE_H
#define PVZ_EXTRA_STATE_H
enum { EXTRA_NONE,EXTRA_SURVIVAL,EXTRA_VASE,EXTRA_IZOMBIE };
enum { B_IDLE,B_SPAWN,B_HEAD_ENTER,B_HEAD_WAIT,B_HEAD_SPIT,B_HEAD_AFTER,B_HEAD_LEAVE,B_STOMP,B_BUNGEE,B_RV };
enum { SURVIVAL_NORMAL,SURVIVAL_HARD,SURVIVAL_ENDLESS };
typedef struct {
 int garden_drops[32],garden_drop_count,bonus_mowers;
 int category,selection,difficulty,stage,flags,goal,pending,repick,seed_selection;
 uint32_t allowed_zombies;int brains[6],zombie_cards[12],card_count;
 int vase_content[6][9],vase_green[6][9],left_facing[6][9];float packet_age[10];
 float transition,brain_eating[96];int bobsled_team[96],bobsled_position[96];
 float bobsled_phase[96],portal_mower_cooldown[6];int last_portal_zombie[96],last_portal_shot[256],last_portal_mower[6];
 int boss_action,boss_target_col,boss_stomps;float boss_action_time,boss_attack_delay,boss_head_time;
 float effect_time[32],effect_x[32],effect_y[32];int effect_kind[32];
 int heavy_level,heavy_cattail,mower_row[6],zombotany_burst[96],squash_col[96];
 float heavy_fire_time,heavy_ice_time,zombotany_jump[96],boss_stomp_delay,boss_bungee_delay;
 float hit_flash[96];
} ExtraState;
#endif
