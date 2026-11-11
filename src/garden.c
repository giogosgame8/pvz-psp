#include "garden.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
static uint32_t next(Profile *p){uint32_t x=p->garden.rng?p->garden.rng:0x6a720ab3;x^=x<<13;x^=x>>17;x^=x<<5;return p->garden.rng=x;}
static int aquatic(int k){return k==LILY||k==KELP||k==SEA||k==CATTAIL;}
static int nocturnal(int k){return (k>=PUFF&&k<=DOOM&&k!=GRAVEBUSTER)||k==SEA||k==MAGNET||k==GLOOM;}
void garden_position(int garden,int slot,float *x,float *y){
 static const float main_x[4][8]={{132,162,194,226,258,289,321,353},{129,160,191,223,260,292,324,356},{124,155,187,219,263,296,328,360},{118,151,184,217,265,299,332,366}};
 static const float mushroom[8][2]={{143,212},{193,184},{216,221},{236,160},{250,125},{274,191},{286,226},{313,155}},aqua[8][2]={{90,125},{177,95},{210,150},{350,95},{376,150},{95,190},{220,222},{303,205}};
 slot=slot<0?0:slot>31?31:slot;if(garden==GARDEN_MAIN){*x=main_x[slot/8][slot%8];*y=88+(slot/8)*38;}else if(garden==GARDEN_MUSHROOM){*x=mushroom[slot%8][0];*y=mushroom[slot%8][1];}else if(garden==GARDEN_AQUARIUM){*x=aqua[slot%8][0];*y=aqua[slot%8][1];}else{*x=240;*y=190;}
}
int garden_count(const Profile *p,int garden){int n=0;for(int i=0;i<GARDEN_PLANTS;i++)n+=p->garden.plants[i].used&&p->garden.plants[i].garden==garden;return n;}
int garden_find(const Profile *p,int garden,int slot){for(int i=0;i<GARDEN_PLANTS;i++)if(p->garden.plants[i].used&&p->garden.plants[i].garden==garden&&p->garden.plants[i].slot==slot)return i;return -1;}
static int free_slot(const Profile *p,int garden){int cap=garden==GARDEN_MAIN?32:garden==GARDEN_BARROW?1:8;for(int i=0;i<cap;i++)if(garden_find(p,garden,i)<0)return i;return -1;}
int garden_add(Profile *p,int kind,int64_t now){int slot=free_slot(p,GARDEN_MAIN);if(slot<0||kind<0||kind>=ALL_SEEDS)return 0;for(int i=0;i<GARDEN_PLANTS;i++)if(!p->garden.plants[i].used){p->garden.plants[i]=(GardenPlant){.used=1,.kind=kind,.slot=slot,.required=3+next(p)%3,.variation=(kind==MARIGOLD?1+next(p)%11:0)|(next(p)%2?16:0),.watered=now-20,.coin_timer=20};return 1;}return 0;}
void garden_initialize(Profile *p,int64_t now){if(p->garden.initialized||p->highest<45)return;p->garden.initialized=1;p->garden.rng=(uint32_t)now^0x6a720ab3;garden_add(p,MARIGOLD,now);garden_add(p,MARIGOLD,now);for(int i=0;i<2;i++)p->garden.plants[i].required=3;p->purchases[SHOP_FERTILIZER]+=5;p->garden.snail_x=50;p->garden.snail_y=230;}
static int same_day(int64_t a,int64_t b){return a>0&&b>=a&&a/86400==b/86400;}
int garden_need(const Profile *p,int index,int64_t now){
    if(index<0||index>=GARDEN_PLANTS)return NEED_NONE;const GardenPlant *v=&p->garden.plants[index];if(!v->used||v->garden==GARDEN_BARROW)return NEED_NONE;
    if(v->age&&nocturnal(v->kind)&&v->garden==GARDEN_MAIN)return NEED_NONE;
    if((v->fertilized>0&&now-v->fertilized<3600)||(v->cared>0&&(now-v->cared<3600||same_day(v->cared,now))))return NEED_NONE;
    if(v->age&&aquatic(v->kind)){if(v->garden!=GARDEN_AQUARIUM)return NEED_NONE;if(v->age<3)return NEED_FERTILIZER;return v->need?v->need:NEED_WATER;}
    if(now-v->watered<15)return NEED_NONE;if(v->waterings<v->required)return NEED_WATER;
    return v->age<3?NEED_FERTILIZER:v->need?v->need:NEED_WATER;
}
static void reward(Profile *p,const GardenPlant *v,int value,int n){float x,y;garden_position(v->garden,v->slot,&x,&y);for(int j=0;j<n;j++)for(int i=0;i<32;i++)if(!p->garden.coins[i].used){p->garden.coins[i]=(GardenCoin){1,value,x+j*8,y,0};break;}}
static int care(Profile *p,int index,int tool,int64_t now){
    GardenPlant *v=&p->garden.plants[index];int need=garden_need(p,index,now);
    if(tool==TOOL_WATER&&need==NEED_WATER){v->waterings++;v->watered=now-(int)(next(p)%9);if(v->age==3&&v->need==NEED_NONE)v->need=next(p)%2?NEED_BUGSPRAY:NEED_MUSIC;reward(p,v,1,1);return 1;}
    if(tool==TOOL_FERTILIZER&&need==NEED_FERTILIZER&&p->purchases[SHOP_FERTILIZER]){p->purchases[SHOP_FERTILIZER]--;v->age++;v->waterings=0;v->need=NEED_NONE;v->fertilized=now;reward(p,v,v->age==3?100:5,v->age==1?1:v->age==2?2:v->kind==MARIGOLD?1:2);return 1;}
    if((tool==TOOL_BUGSPRAY&&need==NEED_BUGSPRAY&&p->purchases[SHOP_SPRAY])||(tool==TOOL_MUSIC&&need==NEED_MUSIC&&p->purchases[SHOP_PHONOGRAPH])){if(tool==TOOL_BUGSPRAY)p->purchases[SHOP_SPRAY]--;v->cared=now;v->need=NEED_NONE;v->waterings=0;v->coin_timer=18+next(p)%13;reward(p,v,5,nocturnal(v->kind)||aquatic(v->kind)?3:1);return 1;}
    if(tool==TOOL_CHOCOLATE&&v->age==3&&v->cared&&same_day(v->cared,now)&&now-v->chocolate>=300&&p->purchases[SHOP_CHOCOLATE]){v->chocolate=now;p->purchases[SHOP_CHOCOLATE]--;v->coin_timer=.6f;return 1;}
    return 0;
}
int garden_use(Profile *p,int index,int tool,int64_t now){
    if(tool==TOOL_SNAIL){if(!p->purchases[SHOP_SNAIL])return 0;p->garden.snail_awake=now;return 1;}
    if(tool==TOOL_CHOCOLATE&&index<0&&p->purchases[SHOP_SNAIL]&&p->purchases[SHOP_CHOCOLATE]){p->purchases[SHOP_CHOCOLATE]--;p->garden.snail_chocolate=now;return 1;}
    if(index<0||index>=GARDEN_PLANTS||!p->garden.plants[index].used)return 0;
    int count=care(p,index,tool,now);if(tool==TOOL_WATER&&p->purchases[SHOP_GOLD_CAN]){GardenPlant *v=&p->garden.plants[index];for(int i=0;i<GARDEN_PLANTS;i++)if(i!=index&&p->garden.plants[i].used&&p->garden.plants[i].garden==v->garden&&abs(p->garden.plants[i].slot%8-v->slot%8)<=1&&abs(p->garden.plants[i].slot/8-v->slot/8)<=1)count+=care(p,i,tool,now);}return count;
}
int garden_move(Profile *p,int index,int garden,int slot){
    if(index<0||index>=GARDEN_PLANTS||!p->garden.plants[index].used||garden<0||garden>3)return 0;GardenPlant *v=&p->garden.plants[index];int cap=garden==GARDEN_MAIN?32:garden==GARDEN_BARROW?1:8;
    if(slot<0||slot>=cap||garden_find(p,garden,slot)>=0||(garden==GARDEN_AQUARIUM&&!aquatic(v->kind)))return 0;
    if(garden==v->garden){if(!p->purchases[SHOP_GLOVE])return 0;}else{if(!p->purchases[SHOP_BARROW]||(garden==GARDEN_MUSHROOM&&!p->purchases[SHOP_MUSHROOM_GARDEN])||(garden==GARDEN_AQUARIUM&&!p->purchases[SHOP_AQUARIUM]))return 0;if(garden!=GARDEN_BARROW&&v->garden!=GARDEN_BARROW)return 0;}
    v->garden=garden;v->slot=slot;return 1;
}
int garden_sell_price(const Profile *p,int index){if(index<0||index>=GARDEN_PLANTS||!p->garden.plants[index].used)return 0;const GardenPlant *v=&p->garden.plants[index];static const int marigold[]={150,200,250,300},normal[]={150,300,500,800};return v->kind==MARIGOLD?marigold[v->age]:v->age==3&&(aquatic(v->kind)||nocturnal(v->kind))?1000:normal[v->age];}
int garden_sell(Profile *p,int index){int value=garden_sell_price(p,index);if(!value)return 0;p->garden.plants[index].used=0;p->money=p->money+value>99999?99999:p->money+value;return value;}
static int collect_one(Profile *p,int i){GardenCoin *c=&p->garden.coins[i];if(!c->used)return 0;p->money=p->money+(uint32_t)c->value>99999?99999:p->money+c->value;c->used=0;return 1;}
int garden_collect(Profile *p){int count=0;for(int i=0;i<32;i++)count+=collect_one(p,i);return count;}
void garden_step(Profile *p,int garden,float dt,int64_t now){
    for(int i=0;i<GARDEN_PLANTS;i++){GardenPlant *v=&p->garden.plants[i];if(!v->used||v->garden!=garden)continue;
        if(v->age==3&&v->cared&&same_day(v->cared,now)&&garden_need(p,i,now)==NEED_NONE){v->coin_timer-=dt;if(v->coin_timer<=0){float minutes=(now-v->cared)/60.0f,max=30+(fminf(30,fmaxf(5,minutes))-5)*4.8f;v->coin_timer=v->chocolate>0&&now-v->chocolate<300?6:18+(next(p)%1000)/1000.0f*(max-18);reward(p,v,next(p)%5?1:5,1);}}
        else if(v->age==3&&v->cared&&!same_day(v->cared,now)&&now-v->cared>=3600){v->need=next(p)%2?NEED_BUGSPRAY:NEED_MUSIC;v->cared=0;}
    }
    int awake=p->purchases[SHOP_SNAIL]&&((p->garden.snail_awake>0&&now-p->garden.snail_awake<180)||(p->garden.snail_chocolate>0&&now-p->garden.snail_chocolate<3600));
    int target=-1;float best=10000;for(int i=0;i<32;i++)if(p->garden.coins[i].used){GardenCoin *c=&p->garden.coins[i];c->age+=dt;if(c->age>60)c->used=0;else if(awake){float d=hypotf(c->x-p->garden.snail_x,c->y-p->garden.snail_y);if(d<best){best=d;target=i;}}}
    if(target>=0&&garden==GARDEN_MAIN){GardenCoin *c=&p->garden.coins[target];if(best<7)collect_one(p,target);else{float speed=p->garden.snail_chocolate>0&&now-p->garden.snail_chocolate<3600?80:35;float step=fminf(best,speed*dt);p->garden.snail_x+=(c->x-p->garden.snail_x)/best*step;p->garden.snail_y+=(c->y-p->garden.snail_y)/best*step;}}
}
int garden_tree_feed(Profile *p){if(!p->purchases[SHOP_TREE]||!p->purchases[SHOP_TREE_FOOD])return 0;p->purchases[SHOP_TREE_FOOD]--;if(p->garden.tree_height<1000000)p->garden.tree_height++;uint32_t h=p->garden.tree_height;p->garden.tree_tip=h==100?800:h==500?900:h==1000?1000:h>1000?1100:h>1?(h-1<49?h-1:49):1;return 1;}
#include "garden_tests.inc"
