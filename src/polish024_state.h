#ifndef PVZ_POLISH024_STATE_H
#define PVZ_POLISH024_STATE_H
enum { DANCE_ENTER,DANCE_POINT,DANCE_HOLD,DANCE_GROUP,DANCE_RISE,DANCE_ORPHAN };
/* v14 appends state; no previously published Game offset moves. */
typedef struct {
 float altitude[96],velocity_y[96],actor_clock[96],summon_delay[96];
 uint32_t leader_id[96],bungee_child_id[96];uint8_t dancer_state[96],dancer_slot[96],pool_gear[96];
 float rise_delay;int rise_state,rise_cell,pool_arrivals;uint32_t pool_used;
 float boss_idle,boss_ball_age;uint32_t boss_bungee_id[3];
 uint32_t shot_actor[256];
 float wave_voice_delay;int wave_voice_pending;
} Polish024State;
#endif
