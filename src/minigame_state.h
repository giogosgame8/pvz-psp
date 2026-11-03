#ifndef PVZ_MINIGAME_STATE_H
#define PVZ_MINIGAME_STATE_H
enum { MINI_ZOMBOTANY,MINI_BOWLING,MINI_SLOT,MINI_HEAVY,MINI_BEGHOULED,MINI_INVIS,MINI_STARS,MINI_AQUARIUM,MINI_TWIST,MINI_LITTLE,MINI_PORTAL,MINI_COLUMN,MINI_BOBSLED,MINI_SPEED,MINI_WHACK,MINI_LASTSTAND,MINI_ZOMBOTANY2,MINI_BOWLING2,MINI_POGO,MINI_BOSS,MINI_COUNT };
typedef struct {int used;float x,y,hunger,produce,dx,dy;} AquariumFish;
typedef struct {int used;float x,y,age;} AquariumBrain;
typedef struct {
 int id,phase,score,stage,selected_row,selected_col,spinning,reels[3],star_count,upgrade,shot_kind;
 float timer,secondary,spin_time,heavy_x,heavy_y,heavy_fire,heavy_power;
 int portals[4],portal_generation;float portal_timer;
 unsigned char zombie_heads[96];float zombie_attack[96],zombie_portal[96],shot_portal[256];
 AquariumFish fish[100];AquariumBrain brains[3];float hostile_y[96],match_fall[6][9];
 float enemy_x[96],enemy_y[96];unsigned char enemy_used[96];
} MiniState;
#endif
