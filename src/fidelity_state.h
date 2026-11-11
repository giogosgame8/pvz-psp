#ifndef PVZ_FIDELITY_STATE_H
#define PVZ_FIDELITY_STATE_H
/* v13 appends state; the complete v12 prefix remains readable. */
typedef struct {
    uint32_t next_actor,actor_id[96],plant_target_id[6][9];
    uint8_t equipment_lost[96];
    float support_safe[6][9],wake[6][9];
    float actor_aux[96];
    int actor_target[96];
    uint32_t next_plant,plant_id[3][6][9],grabbed_id[96];
    uint8_t bungee_phase[96],carried_kind[96];
    float carried_age[96];
} FidelityState;
#endif
