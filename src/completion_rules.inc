/* Campaign state is independent of profile ownership and replay history. */
#include "level_random.h"
int game_seed_locked(const Game *g,int kind){return kind>=0&&kind<ALL_SEEDS&&!!(g->completion.locked_seeds&(UINT64_C(1)<<kind));}
int game_seed_slot(const Game *g,int slot){
    for(int pass=0;pass<2;pass++)for(int k=0;k<ALL_SEEDS;k++)if((g->seed_mask&(UINT64_C(1)<<k))&&game_seed_locked(g,k)==(pass==0)){if(slot--==0)return k;}
    return -1;
}
static int dave_seed_allowed(const Game *g,int kind){
    if(kind>=GATLING)return 0;
    if((kind==LILY||kind==KELP||kind==SEA)&&game_rows(g)!=6)return 0;
    if(kind==POT&&game_world(g)!=4)return 0;
    if(kind==GRAVEBUSTER&&game_world(g)!=1)return 0;
    if(kind==PLANTERN&&game_world(g)!=3)return 0;
    if(kind==COFFEE&&game_is_night(g))return 0;
    if(!game_is_night(g)&&((kind>=PUFF&&kind<=DOOM&&kind!=GRAVEBUSTER)||kind==SEA||kind==MAGNET))return 0;
    if((kind==TORCHWOOD||kind==SPIKEWEED)&&game_world(g)==4)return 0;
    if(kind==UMBRELLA&&!game_zombie_allowed(g->level,Z_BUNGEE)&&!game_zombie_allowed(g->level,Z_CATAPULT))return 0;
    if(kind==BLOVER&&game_world(g)!=3&&!game_zombie_allowed(g->level,Z_BALLOON))return 0;
    return 1;
}
void game_start_adventure(Game *g,int level,uint32_t seed,uint32_t run){
    if(!seed)seed=0x51a7c0de;game_start_level(g,level,seed);g->completion.adventure_run=run;g->completion.level_seed=seed;
    if(!run)return;
    g->unlocked_mask=(UINT64_C(1)<<BASE_SEEDS)-1;
    g->row_mask=game_rows(g)==6?63:31;g->level_flags|=LEVEL_TUTORIAL_COMPLETE;
    if(g->shovel_tutorial){g->shovel_tutorial=0;for(int c=0;c<3;c++)g->plants[2][c].kind=-1;g->level_flags|=16;}
    if(level%10&&g->mode!=MODE_WHACK&&g->mode!=MODE_VASE)g->num_waves=g->num_waves<10?20:g->num_waves+10;
    memset(g->wave_count,0,sizeof(g->wave_count));memset(g->wave_types,0,sizeof(g->wave_types));plan_waves(g);
    if(g->mode==MODE_LAWN){
        g->sun=50;g->spawn_timer=18;g->seed_mask=0;
        int choices[BASE_SEEDS],count=0;for(int original=0;original<BASE_SEEDS;original++){int k=original==2?CHERRY:original==3?WALLNUT:original;if(dave_seed_allowed(g,k))choices[count++]=k;}
        LevelRandom rng;level_random_seed(&rng,seed);
        for(int n=0;n<3&&count;n++){int at=level_random_below(&rng,count),kind=choices[at];g->completion.locked_seeds|=UINT64_C(1)<<kind;memmove(choices+at,choices+at+1,(--count-at)*sizeof(int));}
        g->seed_mask=g->completion.locked_seeds;
    }
    /* Yeti's introduction is one queued arrival halfway through 4-10. */
    if(level==40){int wave=g->num_waves/2,count=g->wave_count[wave];if(count<WAVE_CAPACITY)g->wave_types[wave][g->wave_count[wave]++]=Z_YETI;else g->wave_types[wave][count-1]=Z_YETI;}
}
void game_restart_adventure(Game *g){int level=g->level;uint32_t run=g->completion.adventure_run,seed=g->completion.level_seed?g->completion.level_seed:g->rng;game_start_adventure(g,level,seed,run);}
int game_spawn_giga(Game *g,int row,float x){int id=game_spawn_zombie(g,row,Z_GARG,x);if(id>=0){g->completion.giga[id]=1;g->zombies[id].hp=6000;}return id;}
static int completion_yeti_step(Game *g,int i,float dt){
    Zombie *z=&g->zombies[i];if(z->kind!=Z_YETI||g->hypnotized[i])return 0;
    if(!game_zombie_has_head(g,z))return 0;
    if(!g->completion.yeti_retreat[i]){
        g->completion.yeti_timer[i]=fmaxf(0,g->completion.yeti_timer[i]-dt);
        if(g->completion.yeti_timer[i]>0)return 0;
        g->completion.yeti_retreat[i]=1;z->speed=.8f;z->eating=0;z->age=0;g->polish.pose[i]=0;
    }
    z->eating=0;z->x+=game_zombie_walk(z,dt);if(z->x>530)z->used=0;return 1;
}
static void completion_death_effect(Game *g,const Zombie *z){
    uint32_t claimed=0;
    for(int type=1;type<=2;type++){
    if(!(g->completion.cosmetics&(type==2?COS_PINATA:COS_DAISIES)))continue;
    int count=type==2?10:3;
    for(int n=0;n<count;n++){int slot=-1;for(int i=0;i<24;i++)if(!(claimed&(1u<<i))&&g->completion.particles[i].life<=0){slot=i;break;}
        /* Long-lived daisies cannot permanently starve new candy bursts. Keep
           the bounded pool responsive by retiring its oldest decorations. */
        for(int pass=0;slot<0&&pass<2;pass++)for(int i=0;i<24;i++)if(!(claimed&(1u<<i))&&(pass||g->completion.particles[i].type==1)&&(slot<0||g->completion.particles[i].age>g->completion.particles[slot].age))slot=i;
        if(slot<0)break;claimed|=1u<<slot;
        float ground=game_ground_y(g,z->row)+game_roof_offset(g,z->x);uint32_t value=(g->kills*71+n*31+slot*17);
        g->completion.particles[slot]=(CosmeticParticle){.type=type,.frame=value%5,.x=z->x+(int)(value%25)-12,.y=type==2?ground-32:ground,.vx=type==2?(int)(value%111)-55:0,.vy=type==2?-65-(int)(value%40):0,.life=type==2?1.8f:180,.ground=ground};
    }
    }
}
static void completion_effects_step(Game *g,float dt){
    for(int i=0;i<24;i++){CosmeticParticle *p=&g->completion.particles[i];if(p->life<=0)continue;p->life=fmaxf(0,p->life-dt);p->age+=dt;
        if(p->type==2){p->x+=p->vx*dt;p->y+=p->vy*dt;p->vy+=150*dt;if(p->y>p->ground){p->y=p->ground;p->vy=-fabsf(p->vy)*.3f;p->vx*=.6f;}}
    }
}
static int completion_valid(const Game *g){
    const CompletionState *c=&g->completion;
    if(c->cosmetics>127||c->adventure_run>1000000||c->completion_credited>1||c->yeti_spawned<0||c->yeti_spawned>1||c->giga_allowed<0||c->giga_allowed>1||(c->locked_seeds&~g->unlocked_mask))return 0;
    int count=0;for(int k=0;k<ALL_SEEDS;k++)count+=game_seed_locked(g,k);
    if(count>3||(!c->adventure_run&&count)||(c->locked_seeds&~g->seed_mask))return 0;
    for(int i=0;i<MAX_ZOMBIES;i++)if(!isfinite(c->yeti_timer[i])||c->yeti_timer[i]<0||c->yeti_timer[i]>30||c->yeti_retreat[i]>1||c->giga[i]>1)return 0;
    for(int w=0;w<MAX_WAVES;w++)for(int n=0;n<WAVE_CAPACITY;n++)if(c->wave_giga[w][n]>1||(c->wave_giga[w][n]&&g->wave_types[w][n]!=Z_GARG))return 0;
    for(int i=0;i<24;i++){const CosmeticParticle *p=&c->particles[i];if(p->type<0||p->type>2||p->frame<0||p->frame>4||!isfinite(p->life)||p->life<0||p->life>180||!isfinite(p->age)||p->age<0||!isfinite(p->x)||!isfinite(p->y)||!isfinite(p->vx)||!isfinite(p->vy)||!isfinite(p->ground))return 0;}
    return 1;
}
