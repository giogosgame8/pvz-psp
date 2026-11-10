#include "save_file.h"
#include "profile.h"
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <math.h>
#include "garden.h"
#include "legacy_profile02.h"
_Static_assert(sizeof(LegacyProfile02)==312,"0.9 profile layout");
const int shop_visible_items[SHOP_VISIBLE_COUNT]={SHOP_SLOTS,SHOP_POOL_CLEANER,SHOP_RAKE,SHOP_ROOF_CLEANER,SHOP_GATLING,SHOP_TWIN,SHOP_GLOOM,SHOP_CATTAIL,SHOP_SPIKEROCK,SHOP_GOLDMAGNET,SHOP_WINTER,SHOP_COB,SHOP_IMITATER,SHOP_FIRST_AID,SHOP_MARIGOLD1,SHOP_MARIGOLD2,SHOP_MARIGOLD3,SHOP_GOLD_CAN,SHOP_FERTILIZER,SHOP_SPRAY,SHOP_PHONOGRAPH,SHOP_GLOVE,SHOP_MUSHROOM_GARDEN,SHOP_BARROW,SHOP_SNAIL,SHOP_AQUARIUM,SHOP_TREE,SHOP_TREE_FOOD};
const char *shop_item_names[SHOP_ITEMS]={"GATLING PEA","TWIN SUNFLOWER","GLOOM-SHROOM","CATTAIL","WINTER MELON","GOLD MAGNET","SPIKEROCK","COB CANNON","IMITATER","BONUS MOWER","MARIGOLD","MARIGOLD","MARIGOLD","GOLD WATERING CAN","FERTILIZER","BUG SPRAY","PHONOGRAPH","GARDENING GLOVE","MUSHROOM GARDEN","WHEELBARROW","STINKY THE SNAIL","EXTRA SEED SLOT","POOL CLEANER","ROOF CLEANER","GARDEN RAKE","AQUARIUM GARDEN","CHOCOLATE","TREE OF WISDOM","TREE FOOD","WALL-NUT FIRST AID"};
/* Ten-dollar units, recovered from PS3 GetItemCost at VA 0x2a0bf4. */
static const int prices[SHOP_ITEMS]={500,500,750,1000,1000,300,750,2000,3000,200,250,250,250,1000,75,100,1500,100,3000,20,300,75,100,300,20,3000,0,1000,250,200};
void profile_init(Profile *p){memset(p,0,sizeof(*p));p->highest=1;p->next_session=1;p->seen_plants=1;p->completion.house_owned=31;}
int profile_item_cost(const Profile *p,int item){
    if(item<0||item>=SHOP_ITEMS)return -1;
    if(item==SHOP_SLOTS){static const int cost[4]={75,500,2000,8000};return p->purchases[item]<4?cost[p->purchases[item]]:-1;}
    if(item==SHOP_EXTRA_MOWER)return p->purchases[item]?500:200;return prices[item];
}
int profile_item_available(const Profile *p,int item){
    if(item<0||item>=SHOP_ITEMS)return 0;
    if(item==SHOP_SLOTS)return p->highest>=13&&p->purchases[item]<4;
    if(p->highest<25)return 0;
    if(item==SHOP_RAKE)return !p->purchases[item];
    if(item==SHOP_MARIGOLD1||item==SHOP_MARIGOLD2||item==SHOP_MARIGOLD3)return p->adventure_finished&&garden_count(p,GARDEN_MAIN)<32&&p->garden.shop_days[item-SHOP_MARIGOLD1]!=(int64_t)time(NULL)/86400;
    if(item==SHOP_FERTILIZER||item==SHOP_SPRAY)return p->highest>=45&&p->purchases[item]<=15;
    if(item==SHOP_TREE_FOOD)return p->purchases[SHOP_TREE]&&p->purchases[item]<10;
    if(item==SHOP_EXTRA_MOWER)return p->purchases[item]<2;
    if(item>=SHOP_GOLD_CAN&&item<=SHOP_SNAIL&&p->highest<45)return 0;
    if((item==SHOP_AQUARIUM||item==SHOP_TREE)&&p->highest<45)return 0;
    if(item==SHOP_BARROW&&!p->purchases[SHOP_MUSHROOM_GARDEN]&&!p->purchases[SHOP_AQUARIUM])return 0;
    if(p->purchases[item])return 0;
    if(!p->adventure_finished){
        if(item==SHOP_GLOOM||item==SHOP_CATTAIL)return p->highest>=35;
        if(item==SHOP_SPIKEROCK||item==SHOP_GOLDMAGNET)return p->highest>=41;
        if(item==SHOP_ROOF_CLEANER)return p->highest>=42;
        if(item==SHOP_WINTER||item==SHOP_COB||item==SHOP_IMITATER||item==SHOP_FIRST_AID)return 0;
    }
    return 1;
}
int profile_purchase(Profile *p,int item){
    if(!profile_item_available(p,item))return 0;int cost=profile_item_cost(p,item);if(cost<0||p->money<(uint32_t)cost)return 0;
    p->money-=cost;
    if(item>=SHOP_MARIGOLD1&&item<=SHOP_MARIGOLD3){if(!garden_add(p,MARIGOLD,(int64_t)time(NULL))){p->money+=cost;return 0;}p->garden.shop_days[item-SHOP_MARIGOLD1]=(int64_t)time(NULL)/86400;p->purchases[item]=1;return 1;}
    if(item==SHOP_FERTILIZER||item==SHOP_SPRAY){p->purchases[item]+=5;return 1;}
    if(item==SHOP_TREE){p->purchases[item]=1;p->garden.tree_height=1;if(!p->garden.tree_gift){p->garden.tree_gift=1;p->purchases[SHOP_TREE_FOOD]+=5;}return 1;}
    if(item==SHOP_SNAIL)p->garden.snail_awake=(int64_t)time(NULL);
    if(item==SHOP_RAKE)p->purchases[item]=3;else p->purchases[item]++;return 1;
}
void profile_apply(Profile *p,Game *g){
    int fresh=!g->session_id;if(fresh)g->session_id=p->next_session++;
    uint32_t reached=g->level+((g->status==WON&&(g->level_flags&LEVEL_REWARD_COLLECTED)&&g->level<50)?1:0);if(!g->challenge&&reached>p->highest)p->highest=reached;
    if(!g->challenge&&g->level==50&&g->status==WON&&(g->level_flags&LEVEL_REWARD_COLLECTED)){p->adventure_finished=1;if(!g->completion.completion_credited&&g->session_id!=p->completion.credited_session){uint32_t runs=g->completion.adventure_run+1;if(runs>p->completion.adventure_runs)p->completion.adventure_runs=runs;p->completion.credited_session=g->session_id;g->completion.completion_credited=1;}}
    if(g->challenge&&g->mini.id>=1&&g->mini.id<=20&&g->status==WON&&(g->level_flags&LEVEL_REWARD_COLLECTED))p->mini_trophies|=1u<<(g->mini.id-1);
    if(g->extra.category==EXTRA_SURVIVAL){int id=g->extra.selection+g->extra.difficulty*5;if(id>=0&&id<15){if((uint32_t)g->extra.flags>p->garden.survival_best[id])p->garden.survival_best[id]=g->extra.flags;if(g->status==WON&&(g->level_flags&LEVEL_REWARD_COLLECTED))p->garden.survival_wins|=1u<<id;}}
    if(g->extra.category==EXTRA_VASE||g->extra.category==EXTRA_IZOMBIE){int id=g->extra.selection+(g->extra.category==EXTRA_IZOMBIE?10:0);uint32_t score=g->extra.stage+(g->extra.pending||g->status==WON);if(score>p->garden.puzzle_best[id])p->garden.puzzle_best[id]=score;if((g->status==WON||g->extra.pending==2)&&(g->level_flags&LEVEL_REWARD_COLLECTED))p->garden.puzzle_wins|=1u<<id;}
    if(!p->completion.gold_sunflower&&profile_gold_eligible(p))p->completion.gold_sunflower=1;
    if(p->adventure_finished)p->garden.story_seen|=CHALLENGE_MINI|CHALLENGE_PUZZLE|CHALLENGE_SURVIVAL;
    if(g->completion.adventure_run&&!g->challenge)g->unlocked_mask|=(UINT64_C(1)<<BASE_SEEDS)-1;
    g->completion.cosmetics=p->completion.cosmetics;
    g->purchased_mask=0;for(int k=0;k<9;k++)if(p->purchases[k])g->purchased_mask|=UINT64_C(1)<<(40+k);
    g->unlocked_mask=(g->unlocked_mask&((UINT64_C(1)<<40)-1))|g->purchased_mask;
    if(game_mini_active(g,MINI_LASTSTAND)){uint64_t banned=(UINT64_C(1)<<SUNFLOWER)|(UINT64_C(1)<<SUNSHROOM)|(UINT64_C(1)<<TWIN);g->unlocked_mask&=~banned;g->seed_mask&=~banned;}
    g->seed_slots=6+p->purchases[SHOP_SLOTS];if(g->seed_slots>10)g->seed_slots=10;
    int kept_rake=g->store_flags&STORE_RAKE;g->store_flags=kept_rake|(p->purchases[SHOP_POOL_CLEANER]?STORE_POOL:0)|(p->purchases[SHOP_ROOF_CLEANER]?STORE_ROOF:0)|(p->purchases[SHOP_FIRST_AID]?STORE_FIRST_AID:0)|(p->adventure_finished&&garden_count(p,GARDEN_MAIN)<32?STORE_GARDEN:0)|(p->purchases[SHOP_SNAIL]&&p->purchases[SHOP_CHOCOLATE]<10?STORE_CHOCOLATE:0)|((p->garden.story_seen&CHALLENGE_MINI)?STORE_MINI:0)|((p->garden.story_seen&CHALLENGE_PUZZLE)?STORE_PUZZLE:0)|((p->garden.story_seen&CHALLENGE_SURVIVAL)?STORE_SURVIVAL:0);
    if(fresh&&p->purchases[SHOP_RAKE]&&g->mode!=MODE_WHACK&&g->mode!=MODE_BOSS){g->store_flags|=STORE_RAKE;p->purchases[SHOP_RAKE]--;}
    p->seen_plants|=g->unlocked_mask;p->seen_zombies|=g->seen_zombies;for(int i=0;i<MAX_ZOMBIES;i++)if(g->zombies[i].used)p->seen_zombies|=1u<<g->zombies[i].kind;
}
int profile_bank(Profile *p,const Game *g){
    if(!g->session_id)return 0;CoinCheckpoint *c=0;
    for(int i=0;i<16;i++)if(p->credited[i].id==g->session_id){c=&p->credited[i];break;}
    if(!c){c=&p->credited[p->journal_next++%16];*c=(CoinCheckpoint){g->session_id,0};}
    int changed=0;if(g->money_collected>c->total){uint32_t amount=g->money_collected-c->total;c->total=g->money_collected;if(amount>99999-p->money)p->money=99999;else p->money+=amount;changed=1;}
    int slot=-1;for(int i=0;i<16;i++)if(p->garden.gift_ids[i]==g->session_id){slot=i;break;}
    if(slot<0){slot=p->garden.gift_journal++%16;p->garden.gift_ids[slot]=g->session_id;p->garden.gift_counts[slot]=0;}
    uint32_t count=g->extra.garden_drop_count,start=p->garden.gift_counts[slot];if(start+32<count)start=count-32;
    for(uint32_t i=start;i<count;i++){int gift=g->extra.garden_drops[i%32];if(gift==-200||gift==-201){p->garden.story_seen|=gift==-200?CHALLENGE_MINI:CHALLENGE_PUZZLE;}else if(gift==-100){if(p->purchases[SHOP_CHOCOLATE]<10)p->purchases[SHOP_CHOCOLATE]++;}else if(gift<0&&gift>=-ALL_SEEDS)garden_add(p,-1-gift,(int64_t)time(NULL));changed=1;}
    if(count>p->garden.gift_counts[slot])p->garden.gift_counts[slot]=count;return changed;

}
static int bit_count(uint32_t bits){int count=0;for(;bits;bits&=bits-1)count++;return count;}
int profile_trophy_count(const Profile *p){return bit_count(p->mini_trophies&0xfffffu)+bit_count(p->garden.puzzle_wins&0x7fdffu)+bit_count(p->garden.survival_wins&1023u);}
int profile_gold_eligible(const Profile *p){return p->adventure_finished&&(p->mini_trophies&0xfffffu)==0xfffffu&&(p->garden.puzzle_wins&0x7fdffu)==0x7fdffu&&(p->garden.survival_wins&1023u)==1023u;}
int profile_collect_gold(Profile *p){if(p->completion.gold_sunflower!=1||!profile_gold_eligible(p))return 0;p->completion.gold_sunflower=2;p->money=p->money>99499?99999:p->money+500;return 1;}
int profile_choose_house(Profile *p,int choice){if(choice<0||choice>=5||!(p->completion.house_owned&(1u<<choice)))return 0;p->completion.house_selected=choice;return 1;}
int profile_house_tier(const Profile *p){int tier=profile_trophy_count(p)/12;return tier>4?4:tier;}
int profile_apply_code(Profile *p,const char *code){
 char lower[24];size_t count=strlen(code);if(count>=sizeof(lower))return 0;for(size_t n=0;n<=count;n++)lower[n]=code[n]>='A'&&code[n]<='Z'?code[n]+32:code[n];
 static const struct {const char *name;uint32_t bit,height;} codes[]={{"future",COS_FUTURE,0},{"mustache",COS_MUSTACHE,0},{"trickedout",COS_TRICKED,0},{"daisies",COS_DAISIES,100},{"pinata",COS_PINATA,1000},{"dance",COS_DANCE,500},{"sukhbir",COS_SUKHBIR,0}};
 for(unsigned n=0;n<sizeof(codes)/sizeof(codes[0]);n++)if(!strcmp(lower,codes[n].name)){if(p->garden.tree_height<codes[n].height)return -1;p->completion.cosmetics^=codes[n].bit;return p->completion.cosmetics&codes[n].bit?1:2;}return 0;
}
static uint32_t check(const void *data,size_t bytes){const unsigned char *b=data;uint32_t v=2166136261u;for(size_t i=0;i<bytes;i++)v=(v^b[i])*16777619u;return v;}
int profile_save(const Profile *p,const char *path){uint32_t h[4]={0x38504650,4,sizeof(*p),check(p,sizeof(*p))};return save_file_replace(path,h,p);}
static int profile_load_one(Profile *p,const char *path){
    Profile q;memset(&q,0,sizeof(q));uint32_t h[4];FILE *f=fopen(path,"rb");if(!f)return 0;
    typedef struct {uint32_t money,purchases[SHOP_ITEMS],highest,adventure_finished,next_session,seen_zombies;uint64_t seen_plants;CoinCheckpoint credited[16];uint32_t journal_next;} PreviousProfile;
    int ok=fread(h,sizeof(h),1,f)==1&&h[0]==0x38504650;
    if(ok&&h[1]==4&&h[2]==sizeof(q))ok=fread(&q,sizeof(q),1,f)==1&&h[3]==check(&q,sizeof(q));
    else if(ok&&h[1]==3&&h[2]==offsetof(Profile,completion))ok=fread(&q,h[2],1,f)==1&&h[3]==check(&q,h[2]);
    else if(ok&&h[1]==2&&h[2]==sizeof(LegacyProfile02)){LegacyProfile02 old;ok=fread(&old,sizeof(old),1,f)==1&&h[3]==check(&old,sizeof(old));if(ok)memcpy(&q,&old,sizeof(old));}
    else if(ok&&h[1]==1&&h[2]==sizeof(PreviousProfile)){PreviousProfile old;ok=fread(&old,sizeof(old),1,f)==1&&h[3]==check(&old,sizeof(old));if(ok)memcpy(&q,&old,offsetof(Profile,mini_trophies));}
    else ok=0;
    ok=ok&&fgetc(f)==EOF&&q.money<=99999&&q.highest>=1&&q.highest<=50&&q.purchases[SHOP_SLOTS]<=4;fclose(f);if(ok){int occupied[4][32]={{0}};for(int i=0;i<GARDEN_PLANTS&&ok;i++){GardenPlant *v=&q.garden.plants[i];if(!v->used)continue;int cap=v->garden==GARDEN_MAIN?32:v->garden==GARDEN_BARROW?1:8;ok=v->kind>=0&&v->kind<ALL_SEEDS&&v->garden>=0&&v->garden<4&&v->slot>=0&&v->slot<cap&&v->age>=0&&v->age<=3&&v->required>=3&&v->required<=5&&v->waterings>=0&&v->waterings<=5&&v->need>=0&&v->need<=NEED_MUSIC&&isfinite(v->coin_timer);if(ok){ok=!occupied[v->garden][v->slot];occupied[v->garden][v->slot]=1;}}ok=ok&&q.garden.tree_height<=1000000&&isfinite(q.garden.snail_x)&&isfinite(q.garden.snail_y);}
    if(ok&&h[1]<4){q.completion.adventure_runs=q.adventure_finished?1:0;q.completion.house_owned=31;}
    if(ok)ok=q.completion.adventure_runs<=1000000&&q.completion.gold_sunflower<=2&&q.completion.house_owned<=31&&q.completion.house_selected<5&&(q.completion.house_owned&(1u<<q.completion.house_selected))&&q.completion.cosmetics<=127;
    if(ok&&!q.completion.gold_sunflower&&profile_gold_eligible(&q))q.completion.gold_sunflower=1;
    if(ok)*p=q;return ok;
}
static int profile_recovered;
int profile_load_backup_used(void){return profile_recovered;}
int profile_load(Profile *p,const char *path){
 profile_recovered=0;if(profile_load_one(p,path))return 1;
 char backup[512];if(snprintf(backup,sizeof(backup),"%s.bak",path)>=(int)sizeof(backup))return 0;
 profile_recovered=profile_load_one(p,backup);return profile_recovered;
}
int profile_selftest(FILE *out){
    Profile p,q;Game g;int pass=0,total=0;profile_init(&p);p.money=99999;
    #define T(label,x) do{int ok=(x);pass+=ok;total++;fprintf(out,"profile %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    T("shop remains locked before the car keys",!profile_purchase(&p,SHOP_GATLING));p.highest=25;
    {Profile early;profile_init(&early);early.highest=13;early.money=75;T("Dave can sell an early seed slot before the car keys",profile_purchase(&early,SHOP_SLOTS)&&early.money==0&&early.purchases[SHOP_SLOTS]==1&&!profile_item_available(&early,SHOP_GATLING));}
    T("Gatling costs 500 ten-dollar units",profile_item_cost(&p,SHOP_GATLING)==500&&profile_purchase(&p,SHOP_GATLING)&&p.money==99499);
    T("owned upgrade cannot be purchased twice",!profile_purchase(&p,SHOP_GATLING));
    T("late upgrades remain gated by story progress",!profile_purchase(&p,SHOP_GLOOM)&&!profile_purchase(&p,SHOP_COB));p.highest=35;
    T("gloom becomes purchasable at its original unlock",profile_purchase(&p,SHOP_GLOOM));
    int slots=1;for(int i=0;i<4;i++)slots&=profile_purchase(&p,SHOP_SLOTS);T("seed bank upgrades stop at ten slots",slots&&!profile_purchase(&p,SHOP_SLOTS));
    game_start_level(&g,35,1);profile_apply(&p,&g);T("owned plants and slots are applied without unlocking other upgrades",g.seed_slots==10&&(g.unlocked_mask&(UINT64_C(1)<<GATLING))&&!(g.unlocked_mask&(UINT64_C(1)<<COB)));
    g.money_collected=5;uint32_t money=p.money;T("collected coins are banked once",profile_bank(&p,&g)&&p.money==money+5&&!profile_bank(&p,&g));
    int saved=profile_save(&p,"selftest-profile.dat");T("wallet and purchases survive restart",saved&&profile_load(&q,"selftest-profile.dat")&&!memcmp(&p,&q,sizeof(p)));T("resuming cannot deposit the same pickup twice",!profile_bank(&q,&g));remove("selftest-profile.dat");
    LegacyProfile02 legacy;memcpy(&legacy,&p,sizeof(legacy));legacy.mini_trophies=7;uint32_t h[4]={0x38504650,2,sizeof(legacy),check(&legacy,sizeof(legacy))};FILE *f=fopen("selftest-profile02.dat","wb");int wrote=f&&fwrite(h,sizeof(h),1,f)==1&&fwrite(&legacy,sizeof(legacy),1,f)==1;if(f)fclose(f);
    T("0.9 profile imports its wallet and mini trophies",wrote&&profile_load(&q,"selftest-profile02.dat")&&q.money==p.money&&q.mini_trophies==7&&!q.garden.initialized);
    h[1]=1;h[3]=check(&legacy,sizeof(legacy));f=fopen("selftest-profile02.dat","wb");wrote=f&&fwrite(h,sizeof(h),1,f)==1&&fwrite(&legacy,sizeof(legacy),1,f)==1;if(f)fclose(f);
    T("0.8 profile ignores its former trailing padding",wrote&&profile_load(&q,"selftest-profile02.dat")&&q.money==p.money&&!q.mini_trophies&&!q.garden.initialized);remove("selftest-profile02.dat");
    profile_init(&p);p.adventure_finished=1;int before=profile_trophy_count(&p);p.mini_trophies=0xfffffu;p.garden.puzzle_wins=0x7fdffu;p.garden.survival_wins=1023;
    T("completion Gold Sunflower requires all forty-eight finite challenge trophies",before==0&&profile_trophy_count(&p)==48&&profile_gold_eligible(&p));
    p.garden.puzzle_wins&=~1u;T("completion Endless records cannot replace a missing finite Puzzle trophy",!profile_gold_eligible(&p));p.garden.puzzle_wins=0xfffffu;p.garden.survival_wins=32767;
    T("completion Endless records do not inflate the trophy total",profile_trophy_count(&p)==48);game_start_level(&g,9,2110);profile_apply(&p,&g);T("completion Gold Sunflower is awarded when the combined requirements are met",p.completion.gold_sunflower==1);
    profile_init(&p);game_start_adventure(&g,50,2111,0);g.status=WON;g.level_flags|=LEVEL_REWARD_COLLECTED;profile_apply(&p,&g);uint32_t runs=p.completion.adventure_runs;profile_apply(&p,&g);
    T("completion Adventure completion is credited once per final-level save",runs==1&&p.completion.adventure_runs==1&&g.completion.completion_credited);
    game_start_adventure(&g,50,2112,1);g.status=WON;g.level_flags|=LEVEL_REWARD_COLLECTED;profile_apply(&p,&g);T("completion the second completed Adventure increments the run counter",p.completion.adventure_runs==2);
    p.completion.adventure_runs=0;uint32_t previous_header[4]={0x38504650,3,offsetof(Profile,completion),check(&p,offsetof(Profile,completion))};f=fopen("selftest-profile03.dat","wb");wrote=f&&fwrite(previous_header,sizeof(previous_header),1,f)==1&&fwrite(&p,previous_header[2],1,f)==1;if(f)fclose(f);
    T("completion old finished profiles import as one completed Adventure",wrote&&profile_load(&q,"selftest-profile03.dat")&&q.adventure_finished&&q.completion.adventure_runs==1);remove("selftest-profile03.dat");
    profile_init(&p);p.adventure_finished=1;p.mini_trophies=0xfffffu;p.garden.puzzle_wins=0x7fdffu;p.garden.survival_wins=1023;profile_apply(&p,&g);
    p.money=99700;T("completion Gold pickup caps the wallet and cannot be repeated",profile_collect_gold(&p)&&p.money==99999&&!profile_collect_gold(&p));profile_apply(&p,&g);
    T("completion applying progress never rearms a collected Gold trophy",p.completion.gold_sunflower==2&&!profile_collect_gold(&p));
    p.completion.gold_sunflower=0;int pending_saved=profile_save(&p,"selftest-profile-gold.dat");T("completion reopening an already-qualified profile restores its pending Gold award",pending_saved&&profile_load(&q,"selftest-profile-gold.dat")&&q.completion.gold_sunflower==1&&q.money==p.money);remove("selftest-profile-gold.dat");remove("selftest-profile-gold.dat.bak");
    profile_init(&p);T("completion codes accept case and toggle only their own effect",profile_apply_code(&p,"FuTuRe")==1&&p.completion.cosmetics==COS_FUTURE&&profile_apply_code(&p,"future")==2&&!p.completion.cosmetics);
    T("completion Tree gates reject locked cosmetics without mutation",profile_apply_code(&p,"daisies")==-1&&profile_apply_code(&p,"dance")==-1&&profile_apply_code(&p,"pinata")==-1&&!p.completion.cosmetics);
    p.garden.tree_height=100;int daisy=profile_apply_code(&p,"daisies");p.garden.tree_height=500;int dance=profile_apply_code(&p,"dance");p.garden.tree_height=1000;
    T("completion each Tree cosmetic unlocks at its own height",daisy==1&&dance==1&&profile_apply_code(&p,"pinata")==1&&p.completion.cosmetics==(COS_DAISIES|COS_DANCE|COS_PINATA));
    T("completion unknown secret codes preserve existing effects",!profile_apply_code(&p,"not-a-code")&&p.completion.cosmetics==(COS_DAISIES|COS_DANCE|COS_PINATA));
    p.completion.house_owned=1;T("completion house selection rejects unowned and invalid styles",!profile_choose_house(&p,1)&&!profile_choose_house(&p,5)&&profile_choose_house(&p,0));
    const char *recovery="selftest-profile-recovery.dat";remove(recovery);remove("selftest-profile-recovery.dat.bak");profile_init(&p);p.money=10;int first=profile_save(&p,recovery);p.money=20;int second=profile_save(&p,recovery);
    f=fopen(recovery,"wb");if(f){fwrite("broken",6,1,f);fclose(f);}
    T("save corrupt profile primary recovers its previous committed wallet",first&&second&&profile_load(&q,recovery)&&profile_load_backup_used()&&q.money==10);
    q.money=30;int repaired=profile_save(&q,recovery);f=fopen(recovery,"wb");if(f){fwrite("broken",6,1,f);fclose(f);}
    T("save rewriting a damaged primary preserves the good backup",repaired&&profile_load(&q,recovery)&&profile_load_backup_used()&&q.money==10);
    remove(recovery);f=fopen("selftest-profile-recovery.dat.tmp","wb");if(f){fwrite("unfinished",10,1,f);fclose(f);}
    T("save interrupted replacement recovers backup and ignores unfinished temp",profile_load(&q,recovery)&&profile_load_backup_used()&&q.money==10);
    remove("selftest-profile-recovery.dat.bak");q.money=77;T("save failed load preserves the caller's existing profile",!profile_load(&q,recovery)&&q.money==77);
    remove("selftest-profile-recovery.dat.tmp");
    uint32_t future[4]={0x38504650,5,sizeof(p),check(&p,sizeof(p))};f=fopen(recovery,"wb");int future_written=f&&fwrite(future,sizeof(future),1,f)==1&&fwrite(&p,sizeof(p),1,f)==1;if(f)fclose(f);
    T("save this build refuses to overwrite a newer profile format",future_written&&!profile_save(&p,recovery)&&save_container_valid(recovery,0x38504650,5));
    int moved=!rename(recovery,"selftest-profile-recovery.dat.bak");
    T("save this build also preserves a newer backup format",moved&&!profile_save(&p,recovery)&&save_container_valid("selftest-profile-recovery.dat.bak",0x38504650,5));remove("selftest-profile-recovery.dat.bak");remove(recovery);
    #undef T
    fprintf(out,"PROFILE RESULT %d/%d\n",pass,total);return pass==total;
}
