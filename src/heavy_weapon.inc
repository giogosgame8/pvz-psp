typedef enum { HEAVY_REPEATER,HEAVY_THREEPEATER,HEAVY_CATTAIL,HEAVY_SNOW,HEAVY_TORCH,HEAVY_DOOM,HEAVY_UPGRADES } HeavyUpgrade;
static const int heavy_costs[HEAVY_UPGRADES]={200,400,200,100,125,200};
static const int heavy_seeds[HEAVY_UPGRADES]={REPEATER,THREEPEATER,CATTAIL,SNOW,TORCHWOOD,DOOM};

static float heavy_spike_interval(const Game *g){return 1.25f-.25f*g->extra.heavy_cattail;}

int game_heavy_upgrade_open(const Game *g,int tool){
 if(g->mode!=MODE_HEAVY||tool<0||tool>=HEAVY_UPGRADES)return 0;
 int level=g->extra.heavy_level;
 if(tool==HEAVY_REPEATER)return level==0;
 if(!level)return 0;
 if(tool==HEAVY_THREEPEATER)return level==1;
 if(tool==HEAVY_CATTAIL)return g->extra.heavy_cattail<3;
 if(tool==HEAVY_SNOW)return g->extra.heavy_ice_time<=0;
 if(tool==HEAVY_TORCH)return g->extra.heavy_fire_time<=0;
 return 1;
}
int game_heavy_upgrade_cost(int tool){return tool>=0&&tool<HEAVY_UPGRADES?heavy_costs[tool]:0;}
int game_heavy_upgrade_seed(int tool){return tool>=0&&tool<HEAVY_UPGRADES?heavy_seeds[tool]:PEA;}

static void heavy_doom(Game *g){
 float x=plant_x(4),y=game_ground_y(g,2)-17;
 for(int i=0;i<MAX_ZOMBIES;i++){
  Zombie *z=&g->zombies[i];
  if(!z->used||z->death>0||g->hypnotized[i])continue;
  float dx=z->x-x,dy=game_ground_y(g,z->row)-17-y;
  if(dx*dx+dy*dy<118.75f*118.75f)hurt_charred(g,z,1800);
 }
 g->craters[2][4]=180;g->flash=.25f;sound(g,SFX_EXPLODE);
}
static int heavy_buy(Game *g){
 int tool=g->selected,cost=game_heavy_upgrade_cost(tool);
 if(!game_heavy_upgrade_open(g,tool)||g->sun<cost)return -1;
 switch(tool){
 case HEAVY_REPEATER:g->extra.heavy_level=1;break;
 case HEAVY_THREEPEATER:g->extra.heavy_level=2;break;
 case HEAVY_CATTAIL:g->extra.heavy_cattail++;g->mini.secondary=heavy_spike_interval(g);break;
 case HEAVY_SNOW:g->extra.heavy_ice_time=6;g->extra.heavy_fire_time=0;break;
 case HEAVY_TORCH:g->extra.heavy_fire_time=6;g->extra.heavy_ice_time=0;break;
 case HEAVY_DOOM:heavy_doom(g);break;
 }
 g->sun-=cost;return 1;
}

static int heavy_homing_target(const Game *g,float x){
 int result=-1;
 for(int i=0;i<MAX_ZOMBIES;i++){
  const Zombie *z=&g->zombies[i];
  if(!z->used||z->death>0||g->hypnotized[i]||z->x>439||z->x<0||z->kind==Z_BUNGEE||
     (z->kind==Z_DIGGER&&!z->vaulted))continue;
  if(result<0||z->row>g->zombies[result].row||
     (z->row==g->zombies[result].row&&fabsf(z->x-x)<fabsf(g->zombies[result].x-x)))result=i;
 }
 return result;
}
static int heavy_shot(Game *g,int type,float aim,int target){
 float dx=sinf(aim),dy=-cosf(aim);
 for(int i=0;i<MAX_SHOTS;i++)if(!g->shots[i].used){
  g->shots[i]=(Shot){1,4,type,g->mini.heavy_x+(type==11?14:12*dx)};
  g->shot_y[i]=222+12*dy;g->shot_vx[i]=158.175f*dx;g->shot_vy[i]=158.175f*dy;
  g->shot_damage[i]=type==10?40:20;g->shot_remaining[i]=0;
  g->shot_target[i]=target;g->polish024.shot_actor[i]=target<0?0:g->fidelity.actor_id[target];
  return 1;
 }
 return 0;
}
static float heavy_countdown(float timer,float dt,int fire){
 return fire&&timer>0?timer-dt:fmaxf(0,timer-dt);
}
void game_heavy_control(Game *g,float move,float aim,int fire,float dt){
 if(g->mode!=MODE_HEAVY||g->status!=PLAYING)return;
 g->mini.heavy_x=fminf(440,fmaxf(45,g->mini.heavy_x+move*150*dt));
 g->mini.heavy_y=245;g->mini.heavy_power=fminf(1.5708f,fmaxf(-1.5708f,aim));
 g->mini.heavy_fire=heavy_countdown(g->mini.heavy_fire,dt,fire);
 g->mini.secondary=heavy_countdown(g->mini.secondary,dt,fire);
 if(!fire)return;
 if(g->mini.heavy_fire<=.00001f){
  int count=g->extra.heavy_level==0?1:g->extra.heavy_level==1?2:3;
  int type=g->extra.heavy_fire_time>0?10:g->extra.heavy_ice_time>0?1:0;
  int fired=0;
  for(int barrel=0;barrel<count;barrel++){
   float spacing=count==2?20:15;
   float angle=g->mini.heavy_power+(barrel-(count-1)*.5f)*spacing*.017453343f;
   fired|=heavy_shot(g,type,angle,-1);
  }
  if(fired){g->mini.heavy_fire+=.25f;sound(g,SFX_SHOOT);}
 }
 if(g->extra.heavy_cattail>0&&g->mini.secondary<=.00001f&&
    heavy_shot(g,11,0,heavy_homing_target(g,g->mini.heavy_x)))g->mini.secondary+=heavy_spike_interval(g);
 g->mini.heavy_fire=fmaxf(0,g->mini.heavy_fire);g->mini.secondary=fmaxf(0,g->mini.secondary);
}
