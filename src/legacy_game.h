#ifndef PVZ_LEGACY_GAME_H
#define PVZ_LEGACY_GAME_H
typedef struct {
    uint32_t rng;
    int status, sun, wave, spawned, kills, total_spawned, selected, shovel;
    int cursor_row, cursor_col, mower_used[5], events[SFX_COUNT];
    float time, spawn_timer, sky_timer, wave_delay, flash, cooldown[8], mower_x[5];
    int seed_cost[8], seed_refresh[8];
    Plant plants[5][COLS];
    Zombie zombies[MAX_ZOMBIES];
    Shot shots[MAX_SHOTS];
    Sun suns[MAX_SUNS];
    float wave_health_threshold;
    int level,num_waves,row_mask,unlocked_mask,seed_mask,mode;
    int wave_count[20];uint8_t wave_types[20][WAVE_CAPACITY];
    int row_history[5],row_second[5],row_mowed_wave[5],packets[MAX_PACKETS],packet_count,last_packet,sky_count;
    float conveyor_timer,huge_wave_timer;
    int huge_wave_announced;
    BowlingNut nuts[MAX_NUTS];
    int shovel_tutorial,level_flags;
    /* The first 16528 bytes retain the exact 0.3--0.5.2 save layout. */
    float night_cooldown[8];int night_cost[8],night_refresh[8];
    uint8_t graves[5][COLS],hypnotized[MAX_ZOMBIES];
    float craters[5][COLS],frozen[MAX_ZOMBIES],special[MAX_ZOMBIES];
    int challenge,whack_goal;
    float shot_remaining[MAX_SHOTS];
} LegacyGame;
#endif
