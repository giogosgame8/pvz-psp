#ifndef PVZ_COMPLETION_STATE_H
#define PVZ_COMPLETION_STATE_H

typedef struct {int type,frame;float x,y,vx,vy,life,age,ground;} CosmeticParticle;

/* Save v12 appends this block; all v11 offsets stay unchanged. */
typedef struct {
    uint32_t adventure_run,level_seed,completion_credited,cosmetics;
    int yeti_spawned,giga_allowed;
    uint64_t locked_seeds;
    float yeti_timer[96];
    uint8_t yeti_retreat[96],giga[96],wave_giga[40][64];
    CosmeticParticle particles[24];
} CompletionState;

typedef struct {
    uint32_t adventure_runs,credited_session,gold_sunflower;
    uint32_t house_owned,house_selected,cosmetics,coop_wins;
} CompletionProfile;

#endif
