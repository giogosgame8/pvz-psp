#ifndef PVZ_PROFILE_H
#define PVZ_PROFILE_H
#include "game.h"
#include "garden_state.h"
enum { SHOP_GATLING,SHOP_TWIN,SHOP_GLOOM,SHOP_CATTAIL,SHOP_WINTER,SHOP_GOLDMAGNET,SHOP_SPIKEROCK,SHOP_COB,SHOP_IMITATER,
    SHOP_EXTRA_MOWER,SHOP_MARIGOLD1,SHOP_MARIGOLD2,SHOP_MARIGOLD3,SHOP_GOLD_CAN,SHOP_FERTILIZER,SHOP_SPRAY,SHOP_PHONOGRAPH,SHOP_GLOVE,SHOP_MUSHROOM_GARDEN,SHOP_BARROW,SHOP_SNAIL,SHOP_SLOTS,SHOP_POOL_CLEANER,SHOP_ROOF_CLEANER,SHOP_RAKE,SHOP_AQUARIUM,SHOP_CHOCOLATE,SHOP_TREE,SHOP_TREE_FOOD,SHOP_FIRST_AID,SHOP_BLUEPRINT_SWITCH,SHOP_HOUSE_NORMAL,SHOP_HOUSE_SCARY,SHOP_HOUSE_SPACE,SHOP_HOUSE_TRUCK,SHOP_HOUSE_FUN,SHOP_ITEMS=37 };
#define CHALLENGE_MINI (UINT64_C(1)<<56)
#define CHALLENGE_PUZZLE (UINT64_C(1)<<57)
#define CHALLENGE_SURVIVAL (UINT64_C(1)<<58)
#define CHALLENGE_MIGRATED (UINT64_C(1)<<59)
enum { STORE_MINI=64,STORE_PUZZLE=128,STORE_SURVIVAL=256 };
enum { STORE_POOL=1,STORE_ROOF=2,STORE_FIRST_AID=4,STORE_RAKE=8,STORE_GARDEN=16,STORE_CHOCOLATE=32 };
enum { COS_FUTURE=1,COS_MUSTACHE=2,COS_TRICKED=4,COS_DAISIES=8,COS_PINATA=16,COS_DANCE=32,COS_SUKHBIR=64 };
typedef struct { uint32_t id,total; } CoinCheckpoint;
typedef struct {uint32_t money,purchases[SHOP_ITEMS],highest,adventure_finished,next_session,seen_zombies;uint64_t seen_plants;CoinCheckpoint credited[16];uint32_t journal_next;uint32_t mini_trophies;GardenState garden;CompletionProfile completion;} Profile;
enum { SHOP_VISIBLE_COUNT=33 };
extern const int shop_visible_items[SHOP_VISIBLE_COUNT];
extern const char *shop_item_names[SHOP_ITEMS];
void profile_init(Profile *p);
void profile_apply(Profile *p,Game *g);
int profile_bank(Profile *p,const Game *g);
int profile_item_cost(const Profile *p,int item);
int profile_item_available(const Profile *p,int item);
int profile_purchase(Profile *p,int item);
int profile_save(const Profile *p,const char *path);
int profile_load(Profile *p,const char *path);
int profile_load_backup_used(void);
int profile_selftest(FILE *out);
int profile_trophy_count(const Profile *p);
int profile_gold_eligible(const Profile *p);
int profile_apply_code(Profile *p,const char *code);
int profile_collect_gold(Profile *p);
int profile_choose_house(Profile *p,int choice);
int profile_house_tier(const Profile *p);
int profile_house_item(int choice);
int profile_house_owned(const Profile *p,int choice);
#endif
