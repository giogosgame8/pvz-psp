/* Transient garden presentation; persistent plant and inventory records keep v3 layout. */
typedef struct {int used,index,tool,item,reserved;float elapsed,duration,x,y;} GardenAction;
static GardenAction garden_actions[8];static float garden_grow[GARDEN_PLANTS];
static float snail_pose_time,snail_previous_x;static int snail_pose,snail_facing,snail_was_awake;
static const uint32_t marigold_colors[11]={RGBA(255,255,255,255),RGBA(230,30,195,255),RGBA(250,125,5,255),RGBA(255,145,215,255),RGBA(160,255,245,255),RGBA(230,30,30,255),RGBA(5,130,255,255),RGBA(195,55,235,255),RGBA(235,210,255,255),RGBA(255,245,55,255),RGBA(180,255,105,255)};
static int garden_sleeping(const GardenPlant *v){int k=v->kind;return v->age&&v->garden==GARDEN_MAIN&&((k>=PUFF&&k<=DOOM&&k!=GRAVEBUSTER)||k==SEA||k==MAGNET||k==GLOOM);}
static int garden_busy(int index){for(int n=0;n<8;n++)if(garden_actions[n].used&&(garden_actions[n].index==index||garden_actions[n].tool==TOOL_WATER))return 1;return 0;}
static int garden_pending(void){for(int n=0;n<8;n++)if(garden_actions[n].used)return 1;return 0;}
static int garden_action_start(int index,int tool){
 if(garden_busy(index))return 0;int slot=-1;for(int n=0;n<8;n++)if(!garden_actions[n].used){slot=n;break;}if(slot<0)return 0;
 Profile probe=player;int64_t now=(int64_t)time(NULL);if(!garden_use(&probe,index,tool,now))return 0;
 int item=tool==TOOL_FERTILIZER?SHOP_FERTILIZER:tool==TOOL_BUGSPRAY?SHOP_SPRAY:tool==TOOL_CHOCOLATE?SHOP_CHOCOLATE:-1;
 if(item>=0)player.purchases[item]--;float x=player.garden.snail_x,y=player.garden.snail_y;if(index>=0)garden_position(player.garden.plants[index].garden,player.garden.plants[index].slot,&x,&y);
 float duration=tool==TOOL_WATER?(player.purchases[SHOP_GOLD_CAN]?2.125f:.85f):tool==TOOL_FERTILIZER?1.55f:tool==TOOL_BUGSPRAY?26/30.0f:tool==TOOL_MUSIC?3.7f:.6f;
 garden_actions[slot]=(GardenAction){1,index,tool,item,item>=0,0,duration,x,y};
 static const int cues[TOOL_COUNT]={FOLEY_WATERING,FOLEY_FERTILIZER,FOLEY_BUGSPRAY,FOLEY_PHONOGRAPH,FOLEY_CHOMP,FOLEY_PLANT,FOLEY_MONEYFALLS,FOLEY_PLANT,FOLEY_WAKEUP};foley_play(cues[tool]);return 1;
}
static void garden_action_finish(GardenAction *a){
 int age=a->index>=0?player.garden.plants[a->index].age:0;if(a->reserved)player.purchases[a->item]++;
 if(garden_use(&player,a->index,a->tool,(int64_t)time(NULL))&&a->index>=0&&player.garden.plants[a->index].age>age){garden_grow[a->index]=.8f;foley_play(FOLEY_PLANTGROW);}a->used=0;
}
static void garden_flush(void){for(int n=0;n<8;n++)if(garden_actions[n].used)garden_action_finish(&garden_actions[n]);}
static void garden_present_step(float dt){
 int had=garden_pending();for(int n=0;n<8;n++)if(garden_actions[n].used){garden_actions[n].elapsed+=dt;if(garden_actions[n].elapsed>=garden_actions[n].duration)garden_action_finish(&garden_actions[n]);}
 for(int n=0;n<GARDEN_PLANTS;n++)garden_grow[n]=fmaxf(0,garden_grow[n]-dt);
 if(had&&!garden_pending())profile_save(&player,profile_path);
 int64_t now=(int64_t)time(NULL);int awake=(player.garden.snail_awake>0&&now-player.garden.snail_awake<180)||(player.garden.snail_chocolate>0&&now-player.garden.snail_chocolate<3600);
 if(!awake&&!snail_was_awake&&snail_pose==0)snail_pose=4;
 snail_pose_time+=dt;
 if(awake!=snail_was_awake){snail_pose=awake?2:3;snail_pose_time=0;snail_was_awake=awake;}
 float dx=player.garden.snail_x-snail_previous_x;int facing=dx<-.01f?1:dx>.01f?0:snail_facing;
 if(snail_pose==1&&snail_pose_time>=8/12.0f)snail_pose=0;
 if(snail_pose==2&&snail_pose_time>=15/12.0f)snail_pose=0;
 if(snail_pose==3&&snail_pose_time>=19/12.0f)snail_pose=4;
 if(awake&&snail_pose==0&&facing!=snail_facing){snail_pose=1;snail_pose_time=0;}
 snail_facing=facing;snail_previous_x=player.garden.snail_x;
}
static void garden_anim(int asset,float x,float y,float t,float scale,uint32_t tint,int facing,int loop){if(facing)render_anim_mirrored(&assets[asset],x,y,t,scale,tint,loop);else render_anim(&assets[asset],x,y,t,scale,tint,loop);}
static void garden_draw_plant(int index,float x,float y){
 GardenPlant *v=&player.garden.plants[index];int aquatic=v->kind==LILY||v->kind==KELP||v->kind==SEA||v->kind==CATTAIL;float t=garden_clock+index*.17f;if(v->chocolate>0&&(int64_t)time(NULL)-v->chocolate<300)t*=2;
 int face=!!(v->variation&16);float size=v->age==1?.275f:v->age==2?.4125f:.55f;if(garden_grow[index]>0)size*=1+.12f*sinf((.8f-garden_grow[index])*3.14159f/.8f);
 if(v->garden!=GARDEN_AQUARIUM)render_anim(&assets[aquatic&&v->age?A_GARDEN_WATERPOT:A_GARDEN_POT],x,y,0,.55f,WHITE,1);
 int a=seed_icons[v->kind];if(!v->age)a=v->kind==MARIGOLD?A_GARDEN_SPROUT:A_GARDEN_SPROUT_PLAIN;
 else if(v->garden==GARDEN_AQUARIUM&&v->kind==KELP)a=A_GARDEN_KELPAQUA;else if(v->garden==GARDEN_AQUARIUM&&v->kind==SEA)a=A_GARDEN_SEAAQUA;
 else if(v->kind==SUNSHROOM&&v->age==3)a=garden_sleeping(v)?A_GARDEN_SUNBIGSLEEP:A_GARDEN_SUNBIG;
 else if(garden_sleeping(v)){static const int kinds[]={PUFF,SUNSHROOM,FUME,HYPNO,SCAREDY,ICESHROOM,DOOM,SEA,MAGNET,GLOOM};static const int sleep[]={A_GARDEN_SLEEP_PUFFSHROOM,A_GARDEN_SLEEP_SUNSHROOM,A_GARDEN_SLEEP_FUMESHROOM,A_GARDEN_SLEEP_HYPNOSHROOM,A_GARDEN_SLEEP_SCAREDYSHROOM,A_GARDEN_SLEEP_ICESHROOM,A_GARDEN_SLEEP_DOOMSHROOM,A_GARDEN_SLEEP_SEASHROOM,A_GARDEN_SLEEP_MAGNETSHROOM,A_GARDEN_SLEEP_GLOOMSHROOM};for(int k=0;k<10;k++)if(v->kind==kinds[k])a=sleep[k];}
 if(v->kind==MARIGOLD&&v->age){int color=(v->variation&15)-1;uint32_t tint=color>=0&&color<11?marigold_colors[color]:WHITE;garden_anim(A_GARDEN_MARIGOLD_PETALS,x,y-4,t,size,tint,face,1);garden_anim(A_GARDEN_MARIGOLD_BODY,x,y-4,t,size,WHITE,face,1);}
 else garden_anim(a,x,y-4,t,size,WHITE,face,1);
 if(garden_sleeping(v)){float z=fmodf(garden_clock+index*.2f,2.0f);render_image(&assets[A_GARDEN_Z],x+5+z*4,y-24-z*10,5+z*2,7+z*2,0,RGBA(255,255,255,(int)(200*(1-z/2))));}
}
static void garden_draw_actions(void){for(int n=0;n<8;n++)if(garden_actions[n].used){GardenAction *a=&garden_actions[n];int asset=a->tool==TOOL_WATER?(player.purchases[SHOP_GOLD_CAN]?A_GARDEN_GOLDWATER:A_GARDEN_WATER):a->tool==TOOL_FERTILIZER?A_GARDEN_FERTILIZER:a->tool==TOOL_BUGSPRAY?A_GARDEN_SPRAY:a->tool==TOOL_MUSIC?A_GARDEN_MUSIC:a->tool==TOOL_CHOCOLATE?A_CHOCOLATE:A_GARDEN_GLOVE;
 float t=a->elapsed;if(a->tool==TOOL_WATER&&player.purchases[SHOP_GOLD_CAN])t*=8/20.0f;if(a->tool==TOOL_MUSIC)t*=20/30.0f;
 render_anim(&assets[asset],a->x+12,a->y-8,t,a->tool==TOOL_MUSIC?.65f:.8f,WHITE,a->tool==TOOL_MUSIC);}}
