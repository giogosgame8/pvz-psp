#ifndef PVZ_POLISH_STATE_H
#define PVZ_POLISH_STATE_H
enum { ACTION_NONE,ACTION_GARLIC,ACTION_PAPER,ACTION_JACK,ACTION_GARG_THROW,ACTION_IMP_FLIGHT,ACTION_IMP_LAND,ACTION_KELP,ACTION_LADDER,ACTION_BALLOON_FALL,ACTION_DIGGER_RISE,ACTION_CATAPULT,ACTION_SNORKEL_RISE,ACTION_SNORKEL_DIVE,ACTION_COUNT };
enum { DEATH_NORMAL,DEATH_CHARRED,DEATH_WATER };
/* Version 11 appends this state; version 10's complete prefix is retained. */
typedef struct {
 float pose[96],action_time[96];int action[96],target[96],death_style[96];
 int wave_pending,wave_entry,grave_entry;
 float wave_health,wave_fraction;
} PolishState;
#endif
