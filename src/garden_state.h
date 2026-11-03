#ifndef PVZ_GARDEN_STATE_H
#define PVZ_GARDEN_STATE_H
#define GARDEN_PLANTS 200
enum { GARDEN_MAIN,GARDEN_MUSHROOM,GARDEN_AQUARIUM,GARDEN_BARROW };
enum { NEED_NONE,NEED_WATER,NEED_FERTILIZER,NEED_BUGSPRAY,NEED_MUSIC };
enum { TOOL_WATER,TOOL_FERTILIZER,TOOL_BUGSPRAY,TOOL_MUSIC,TOOL_CHOCOLATE,TOOL_GLOVE,TOOL_SELL,TOOL_BARROW,TOOL_SNAIL,TOOL_COUNT };
typedef struct {int used,kind,garden,slot,age,waterings,required,need,variation;int64_t watered,fertilized,cared,chocolate;float coin_timer;} GardenPlant;
typedef struct {int used,value;float x,y,age;} GardenCoin;
typedef struct {uint32_t rng,initialized,tree_height,tree_tip,survival_wins,puzzle_wins;uint32_t survival_best[15],puzzle_best[20];uint64_t story_seen;int credits_seen,tree_gift;int64_t shop_days[3],snail_awake,snail_chocolate;float snail_x,snail_y;GardenPlant plants[GARDEN_PLANTS];GardenCoin coins[32];uint32_t gift_ids[16],gift_counts[16],gift_journal;} GardenState;
#endif
