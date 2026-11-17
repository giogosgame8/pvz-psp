/* Original wave budgets, kept separate from bounded native arrival storage. */
static int wave_budget024(const Game *g,int wave){
 if(game_mini_active(g,MINI_LASTSTAND))return (g->mini.stage*g->num_waves+wave+10)*2/5+1;
 if(g->extra.category==EXTRA_SURVIVAL&&g->extra.stage>0)return (g->extra.stage*g->num_waves+wave)*2/5+1;
 if(!g->challenge&&!g->extra.category&&g->completion.adventure_run&&g->level!=5)return wave*2/5+1;
 return wave/3+1;
}
static int wave_multiplier024(const Game *g){
 if(game_mini_active(g,MINI_COLUMN))return 6;
 if(g->mode==MODE_BOWLING||g->level==25||game_mini_active(g,MINI_LITTLE))return 4;
 if(!g->challenge&&!g->extra.category&&g->level%10==0)return 3;
 if(g->mode==MODE_BUNGEE||game_mini_active(g,MINI_PORTAL)||game_mini_active(g,MINI_INVIS))return 2;
 return 1;
}
static int introduced_zombie024(const Game *g){
 if(g->challenge||g->extra.category)return -1;
 static const int first[Z_TYPES]={1,3,8,6,1,11,13,16,18,18,21,23,26,26,28,31,33,36,38,40,41,43,46,48,48,50};
 for(int k=Z_CONE;k<Z_BOSS;k++)if(k!=Z_FLAG&&k!=Z_BACKUP&&k!=Z_DUCK&&k!=Z_YETI&&k!=Z_IMP&&first[k]==g->level)return k;
 return -1;
}
static int intro_wave024(const Game *g,int kind,int wave){
 return kind>=0&&(wave==g->num_waves-1||((kind==Z_BALLOON||kind==Z_DIGGER)?wave==6:wave==g->num_waves/2));
}
static float wave_health024(const Game *g,const Zombie *z){
 int i=(int)(z-g->zombies);
 if(!z->used||z->death>0||g->hypnotized[i]||z->kind==Z_BUNGEE||g->polish.action[i]==ACTION_SKY_DROP||
    (g->mode==MODE_BUNGEE&&g->special[i]<0))return 0;
 int shield=z->kind==Z_DOOR||z->kind==Z_PAPER||z->kind==Z_LADDER;
 return fmaxf(0,z->hp)+fmaxf(0,z->armor)*(shield?.2f:1);
}
static int actor_by_id024(const Game *g,uint32_t id){
 if(id)for(int i=0;i<MAX_ZOMBIES;i++)if(g->zombies[i].used&&g->fidelity.actor_id[i]==id)return i;
 return -1;
}
