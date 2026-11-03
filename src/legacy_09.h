#ifndef PVZ_LEGACY_09_H
#define PVZ_LEGACY_09_H
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
} Legacy09;
#endif
