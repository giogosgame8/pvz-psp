/* Standalone challenge rules. Content IDs follow the supplied console disc.
   Rendering and controller mappings live in mini_ui.inc. */
const char *mini_names[20]={"ZOMBOTANY","WALL-NUT BOWLING","SLOT MACHINE","HEAVY WEAPON","BEGHOULED","INVISI-GHOUL","SEEING STARS","ZOMBIQUARIUM","BEGHOULED TWIST","BIG TROUBLE LITTLE ZOMBIE","PORTAL COMBAT","COLUMN LIKE YOU SEE 'EM","BOBSLED BONANZA","ZOMBIES ON SPEED","WHACK A ZOMBIE","LAST STAND","ZOMBOTANY 2","BOWLING 2","POGO PARTY","DR. ZOMBOSS'S REVENGE"};
int game_mini_active(const Game *g,int id){return g->challenge&&g->mini.id==id+1;}
static int mini_allowed(const Game *g,int k){
    switch(g->mini.id-1){
    case MINI_BOBSLED:return k==Z_ZOMBONI||k==Z_BOBSLED;
    case MINI_POGO:return k==Z_POGO;
    case MINI_HEAVY:return k==Z_NORMAL||k==Z_POLE||k==Z_POGO||k==Z_FOOTBALL||k==Z_GARG;
    case MINI_SPEED:return k==Z_NORMAL||k==Z_CONE||k==Z_DOLPHIN||k==Z_POLE;
    case MINI_PORTAL:return k==Z_NORMAL||k==Z_BUCKET||k==Z_FOOTBALL||k==Z_BALLOON;
    case MINI_LITTLE:return k==Z_NORMAL||k==Z_CONE||k==Z_FOOTBALL||k==Z_SNORKEL;
    case MINI_INVIS:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET||k==Z_DOLPHIN||k==Z_ZOMBONI||k==Z_JACK;
    case MINI_COLUMN:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET||k==Z_FOOTBALL;
    case MINI_BEGHOULED:case MINI_TWIST:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET||k==Z_DOOR||k==Z_FOOTBALL||k==Z_PAPER;
    case MINI_LASTSTAND:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET||k==Z_DOOR||k==Z_FOOTBALL||k==Z_PAPER||k==Z_JACK||k==Z_POLE||k==Z_DOLPHIN||k==Z_LADDER;
    case MINI_BOWLING:case MINI_BOWLING2:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET||k==Z_POLE||k==Z_PAPER||(game_mini_active(g,MINI_BOWLING2)&&(k==Z_DOOR||k==Z_DANCER));
    case MINI_ZOMBOTANY:case MINI_ZOMBOTANY2:return k==Z_NORMAL||k==Z_BUCKET;
    default:return k==Z_NORMAL||k==Z_CONE||k==Z_BUCKET;
    }
}
static int heavy_cost(const Game *g,int selection){(void)g;return selection==0?100:selection==1?200:selection==2?200:selection==3?400:selection==4?125:200;}
static int heavy_buy(Game *g){int tool=g->selected%6,cost=heavy_cost(g,tool);if(g->sun<cost)return -1;
 if(tool==1&&g->extra.heavy_level!=0)return -1;if(tool==3&&g->extra.heavy_level!=1)return -1;if(tool==5&&(g->extra.heavy_level<2||g->extra.heavy_cattail>=3))return -1;
 if(tool==0)g->extra.heavy_ice_time=20;else if(tool==4)g->extra.heavy_fire_time=20;else if(tool==2){for(int i=0;i<MAX_ZOMBIES;i++)if(g->zombies[i].used)hurt_zombie(g,&g->zombies[i],1800,0);g->craters[2][4]=180;g->flash=.25f;sound(g,SFX_EXPLODE);}else if(tool==5){g->extra.heavy_level=3;g->extra.heavy_cattail++;}else g->extra.heavy_level++;g->sun-=cost;return 1;
}
void game_heavy_control(Game *g,float move,float aim,int fire,float dt){
 if(g->mode!=MODE_HEAVY||g->status!=PLAYING)return;
 g->mini.heavy_x=fminf(440,fmaxf(45,g->mini.heavy_x+move*150*dt));g->mini.heavy_y=245;g->mini.heavy_power=aim*.6f;g->mini.heavy_fire=fmaxf(0,g->mini.heavy_fire-dt);
 if(fire&&g->mini.heavy_fire<=0){int level=g->extra.heavy_level,count=level==0?1:level==1?2:level==2?3:1;g->mini.heavy_fire=level==3?.17f/(1+g->extra.heavy_cattail*.5f):.17f;
  for(int n=0;n<count;n++)for(int i=0;i<MAX_SHOTS;i++)if(!g->shots[i].used){int ice=level==3?11:g->extra.heavy_fire_time>0?10:g->extra.heavy_ice_time>0?1:8;g->shots[i]=(Shot){1,4,ice,g->mini.heavy_x};g->shot_y[i]=232;float angle=aim*.6f+(n-(count-1)*.5f)*.25f;g->shot_vx[i]=sinf(angle)*250;g->shot_vy[i]=-cosf(angle)*250;g->shot_damage[i]=ice==10?40:20;g->shot_remaining[i]=10;g->shot_target[i]=-1;sound(g,SFX_SHOOT);break;}
 }
}
static int mini_zombie(Game *g,int i,float dt){
    if(!g->challenge)return 0;Zombie *z=&g->zombies[i];int head=g->mini.zombie_heads[i];if(!head||z->death>0||g->frozen[i]>0)return 0;
    if((head==2||head==6)&&z->armor<=0)g->mini.zombie_heads[i]=0;
    if(head==1||head==4){g->mini.zombie_attack[i]-=dt;if(g->mini.zombie_attack[i]<=0){
       if(head==4){if(!g->extra.zombotany_burst[i])g->extra.zombotany_burst[i]=4;g->extra.zombotany_burst[i]--;g->mini.zombie_attack[i]=g->extra.zombotany_burst[i]?.17f:.99f;}else g->mini.zombie_attack[i]=1.5f;
       for(int j=0;j<96;j++)if(!g->mini.enemy_used[j]){g->mini.enemy_used[j]=1;g->mini.enemy_x[j]=z->x-18;g->mini.enemy_y[j]=game_ground_y(g,z->row)-18;break;}}}
    if(head==3){g->mini.zombie_attack[i]-=dt;if(g->mini.zombie_attack[i]<=-10){for(int c=0;c<COLS;c++){g->plants[z->row][c].kind=-1;g->pumpkin_hp[z->row][c]=0;g->support[z->row][c]=0;}kill_zombie(g,z);g->flash=.25f;sound(g,SFX_EXPLODE);return 1;}}
    if(head==5){if(g->extra.zombotany_jump[i]>0){float old=g->extra.zombotany_jump[i];g->extra.zombotany_jump[i]-=dt;if(old>.2f&&g->extra.zombotany_jump[i]<=.2f){int c=g->extra.squash_col[i];g->plants[z->row][c].kind=-1;g->pumpkin_hp[z->row][c]=0;g->support[z->row][c]=0;sound(g,SFX_CHOMP);}if(g->extra.zombotany_jump[i]<=0)kill_zombie(g,z);return 1;}
     for(int c=0;c<COLS;c++)if(g->plants[z->row][c].kind>=0&&fabsf(z->x-plant_x(c))<30){g->extra.squash_col[i]=c;g->extra.zombotany_jump[i]=.95f;return 1;}}
    return 0;
}
static int mini_match_kind(const Game *g,int k){
    (void)g;
    if(k==REPEATER)k=PEA;if(k==FUME)k=PUFF;if(k==TALLNUT)k=WALLNUT;return k;
}
static int mini_match_seed(Game *g){
    static const int types[]={PUFF,STAR,MAGNET,SNOW,WALLNUT,PEA};
    int k=types[random_next(g)%6];if(k==PEA&&(g->mini.upgrade&1))k=REPEATER;if(k==PUFF&&(g->mini.upgrade&2))k=FUME;if(k==WALLNUT&&(g->mini.upgrade&4))k=TALLNUT;return k;
}
static void mini_fill_matches(Game *g){
    for(int c=0;c<8;c++)for(int r=4;r>=0;r--)if(g->plants[r][c].kind<0&&g->craters[r][c]<=0){
        int source=r-1;while(source>=0&&g->plants[source][c].kind<0)source--;
        if(source>=0){g->plants[r][c]=g->plants[source][c];g->plants[source][c].kind=-1;g->mini.match_fall[r][c]=-(r-source)*game_row_height(g);}
        else {put_plant(g,r,c,mini_match_seed(g));g->mini.match_fall[r][c]=-game_row_height(g)*(r+1);}
    }
}
static int mini_match_scan(Game *g,int apply){
    unsigned char remove[5][8]={{0}};int groups=0;
    for(int r=0;r<5;r++)for(int c=0;c<8;c++){
        int k=mini_match_kind(g,g->plants[r][c].kind);if(k<0)continue;
        if(c==0||mini_match_kind(g,g->plants[r][c-1].kind)!=k){int end=c+1;while(end<8&&mini_match_kind(g,g->plants[r][end].kind)==k)end++;if(end-c>=3){groups++;for(int j=c;j<end;j++)remove[r][j]=1;if(apply)for(int j=0;j<end-c-2&&j<5;j++)plant_sun(g,plant_x(c)+j*6,game_ground_y(g,r)-15);}}
        if(r==0||mini_match_kind(g,g->plants[r-1][c].kind)!=k){int end=r+1;while(end<5&&mini_match_kind(g,g->plants[end][c].kind)==k)end++;if(end-r>=3){groups++;for(int j=r;j<end;j++)remove[j][c]=1;if(apply)plant_sun(g,plant_x(c),game_ground_y(g,r)-15);}}
    }
    if(apply&&groups){g->mini.score+=groups;for(int r=0;r<5;r++)for(int c=0;c<8;c++)if(remove[r][c])g->plants[r][c].kind=-1;mini_fill_matches(g);g->mini.timer=.30f;sound_as(g,SFX_PLANT,FOLEY_CHIME);}
    return groups;
}
static void mini_shuffle(Game *g){
    for(int r=0;r<5;r++)for(int c=0;c<8;c++)if(g->craters[r][c]<=0){
        int tries=0,k;do{k=mini_match_seed(g);tries++;}while(tries<40&&((c>=2&&mini_match_kind(g,k)==mini_match_kind(g,g->plants[r][c-1].kind)&&mini_match_kind(g,k)==mini_match_kind(g,g->plants[r][c-2].kind))||(r>=2&&mini_match_kind(g,k)==mini_match_kind(g,g->plants[r-1][c].kind)&&mini_match_kind(g,k)==mini_match_kind(g,g->plants[r-2][c].kind))));put_plant(g,r,c,k);g->mini.match_fall[r][c]=-20;
    }
}
static int mini_has_move(Game *g){
    for(int r=0;r<5;r++)for(int c=0;c<8;c++){
        if(game_mini_active(g,MINI_TWIST)){
            if(r>=4||c>=7)continue;Plant a=g->plants[r][c],b=g->plants[r][c+1],d=g->plants[r+1][c],e=g->plants[r+1][c+1];
            if(a.kind<0||b.kind<0||d.kind<0||e.kind<0)continue;g->plants[r][c]=d;g->plants[r][c+1]=a;g->plants[r+1][c]=e;g->plants[r+1][c+1]=b;int yes=mini_match_scan(g,0);g->plants[r][c]=a;g->plants[r][c+1]=b;g->plants[r+1][c]=d;g->plants[r+1][c+1]=e;if(yes)return 1;
        }else for(int axis=0;axis<2;axis++){
            int rr=r+(axis==0),cc=c+(axis==1);if(rr>=5||cc>=8||g->plants[r][c].kind<0||g->plants[rr][cc].kind<0)continue;Plant old=g->plants[r][c];g->plants[r][c]=g->plants[rr][cc];g->plants[rr][cc]=old;int yes=mini_match_scan(g,0);old=g->plants[r][c];g->plants[r][c]=g->plants[rr][cc];g->plants[rr][cc]=old;if(yes)return 1;
        }
    }return 0;
}
static int mini_fish(Game *g){for(int i=0;i<100;i++)if(!g->mini.fish[i].used){AquariumFish *f=&g->mini.fish[i];*f=(AquariumFish){.used=1,.x=70+random_unit(g)*330,.y=90+random_unit(g)*130,.hunger=27,.produce=10+random_unit(g)*5,.dx=15,.dy=5};return 1;}return 0;}
static void mini_replan(Game *g){memset(g->wave_count,0,sizeof(g->wave_count));memset(g->wave_types,0,sizeof(g->wave_types));plan_waves(g);}
void game_start_mini(Game *g,int id,uint32_t seed){
    static const int levels[20]={9,5,9,9,19,29,9,29,19,29,19,49,29,29,15,29,29,9,49,50};
    static const int waves[20]={20,20,40,20,40,20,40,1,40,30,20,30,40,40,12,10,30,30,30,40};
    if(id<0||id>=MINI_COUNT)id=0;game_start_level(g,levels[id],seed);g->challenge=id==MINI_BOWLING?1:id==MINI_WHACK?2:id+3;g->mini.id=id+1;
    g->mode=MODE_LAWN;g->num_waves=waves[id];g->row_mask=game_rows(g)==6?63:31;g->sun=50;g->spawn_timer=18;g->shovel_tutorial=0;g->packet_count=0;g->mini.selected_row=g->mini.selected_col=-1;g->level_flags=LEVEL_TUTORIAL_COMPLETE;
    memset(g->graves,0,sizeof(g->graves));memset(g->support,0,sizeof(g->support));memset(g->support_hp,0,sizeof(g->support_hp));
    for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)g->plants[r][c].kind=-1;
    g->unlocked_mask=(UINT64_C(1)<<40)-1;g->seed_mask=(UINT64_C(1)<<PEA)|(UINT64_C(1)<<SUNFLOWER)|(UINT64_C(1)<<WALLNUT)|(UINT64_C(1)<<CHERRY)|(UINT64_C(1)<<MINE)|(UINT64_C(1)<<SNOW);g->selected=PEA;
    if(id==MINI_STARS){g->seed_mask&=~(UINT64_C(1)<<MINE);g->seed_mask|=UINT64_C(1)<<STAR;}
    if(id==MINI_BOWLING||id==MINI_BOWLING2){g->mode=MODE_BOWLING;g->sun=0;g->packets[g->packet_count++]=WALLNUT;g->spawn_timer=2;}
    if(id==MINI_SLOT){g->mode=MODE_SLOT;g->seed_mask=0;}
    if(id==MINI_HEAVY){g->mode=MODE_HEAVY;g->sun=0;g->mini.heavy_x=100;g->mini.heavy_y=150;g->spawn_timer=3;}
    if(id==MINI_BEGHOULED||id==MINI_TWIST){g->mode=MODE_MATCH;g->sun=0;mini_shuffle(g);g->mini.timer=0;}
    if(id==MINI_AQUARIUM){g->mode=MODE_AQUARIUM;g->sun=50;mini_fish(g);mini_fish(g);}
    if(id==MINI_INVIS||id==MINI_LITTLE||id==MINI_PORTAL||id==MINI_COLUMN){g->mode=MODE_CONVEYOR;g->sun=0;g->conveyor_timer=0;}
    if(id==MINI_PORTAL){g->mini.portals[0]=2;g->mini.portals[1]=19;g->mini.portals[2]=39;g->mini.portals[3]=42;g->mini.portal_timer=90;}
    if(id==MINI_COLUMN||id==MINI_POGO)for(int r=0;r<5;r++)for(int c=0;c<3;c++){g->support[r][c]=POT+1;g->support_hp[r][c]=300;}
    if(id==MINI_BOBSLED)for(int r=0;r<6;r++)if(!game_water_row(g,r))for(int c=4;c<9;c++)g->ice_tiles[r][c]=100000;
    if(id==MINI_WHACK){g->mode=MODE_WHACK;g->sun=0;g->seed_mask=(UINT64_C(1)<<GRAVEBUSTER)|(UINT64_C(1)<<ICESHROOM)|(UINT64_C(1)<<CHERRY);g->selected=GRAVEBUSTER;for(int i=0;i<9;i++)g->graves[random_next(g)%5][4+random_next(g)%5]=1;}
    if(id==MINI_LASTSTAND){g->sun=5000;g->mini.phase=0;g->seed_mask&=~(UINT64_C(1)<<SUNFLOWER);g->unlocked_mask&=~((UINT64_C(1)<<SUNFLOWER)|(UINT64_C(1)<<SUNSHROOM)|(UINT64_C(1)<<TWIN));}
    if(id==MINI_BOSS){g->mode=MODE_BOSS;g->sun=0;g->boss_hp=60000;g->boss_timer=8;for(int r=0;r<5;r++)for(int c=0;c<3;c++){g->support[r][c]=POT+1;g->support_hp[r][c]=300;}}
    mini_replan(g);
}
/* action: 0 tile, 1 special, 2 previous tool, 3 next tool, 4 onslaught. */
int game_mini_action(Game *g,int action){
    if(!g->challenge||g->status!=PLAYING)return 0;
    if(game_mini_active(g,MINI_LASTSTAND)&&action==4&&g->mini.phase==0){g->mini.phase=1;g->spawn_timer=2;return 1;}
    if(g->mode==MODE_SLOT&&action==1){if(g->mini.spinning||g->sun<25||g->packet_count>7)return -1;g->sun-=25;g->mini.spinning=1;g->mini.spin_time=3.0f;cue(GAME_SAMPLE(FX_SLOT_MACHINE));return 1;}
    if(g->mode==MODE_HEAVY){if(action==2||action==3){g->selected=(g->selected+(action==2?5:1))%6;return 1;}if(action==1)return heavy_buy(g);}
    if(g->mode==MODE_AQUARIUM){
        if(action==2||action==3){g->mini.upgrade^=1;return 1;}
        if(action==1){int price=g->mini.upgrade?1000:100;if(g->sun<price)return -1;if(g->mini.upgrade){g->sun-=price;g->status=WON;sound(g,SFX_WIN);}else if(mini_fish(g))g->sun-=price;return 1;}
        if(action==0){if(g->sun<5)return -1;for(int i=0;i<3;i++)if(!g->mini.brains[i].used){g->mini.brains[i]=(AquariumBrain){1,plant_x(g->cursor_col),game_ground_y(g,g->cursor_row)-15,0};g->sun-=5;return 1;}return -1;}
    }
    if(g->mode==MODE_MATCH){
        if(action==2||action==3){g->selected=(g->selected+(action==2?4:1))%5;return 1;}
        if(action==1){static const int costs[]={1000,500,250,100,200};int tool=g->selected%5;if(g->sun<costs[tool])return -1;
            if(tool<3){int bit=1<<tool;if(g->mini.upgrade&bit)return -1;g->mini.upgrade|=bit;for(int r=0;r<5;r++)for(int c=0;c<8;c++){Plant *p=&g->plants[r][c];if(p->kind==(int[]){PEA,PUFF,WALLNUT}[tool])put_plant(g,r,c,(int[]){REPEATER,FUME,TALLNUT}[tool]);}}
            if(tool==3)mini_shuffle(g);if(tool==4){memset(g->craters,0,sizeof(g->craters));mini_fill_matches(g);}g->sun-=costs[tool];return 1;}
        if(action==0){int r=g->cursor_row,c=g->cursor_col;if(r>=5||c>=8||g->mini.timer>0)return -1;
            if(game_mini_active(g,MINI_TWIST)){if(r>=4||c>=7)return -1;Plant old=g->plants[r][c];g->plants[r][c]=g->plants[r+1][c];g->plants[r+1][c]=g->plants[r+1][c+1];g->plants[r+1][c+1]=g->plants[r][c+1];g->plants[r][c+1]=old;
                if(!mini_match_scan(g,1)){old=g->plants[r][c];g->plants[r][c]=g->plants[r][c+1];g->plants[r][c+1]=g->plants[r+1][c+1];g->plants[r+1][c+1]=g->plants[r+1][c];g->plants[r+1][c]=old;return -1;}return 1;}
            int sr=g->mini.selected_row,sc=g->mini.selected_col;if(sr<0){g->mini.selected_row=r;g->mini.selected_col=c;return 1;}g->mini.selected_row=g->mini.selected_col=-1;
            if(abs(r-sr)+abs(c-sc)!=1)return -1;Plant old=g->plants[r][c];g->plants[r][c]=g->plants[sr][sc];g->plants[sr][sc]=old;
            if(!mini_match_scan(g,1)){old=g->plants[r][c];g->plants[r][c]=g->plants[sr][sc];g->plants[sr][sc]=old;return -1;}return 1;
        }
    }
    return 0;
}
static void mini_spawned(Game *g,int i){
    if(i<0||!g->challenge)return;Zombie *z=&g->zombies[i];g->extra.bobsled_team[i]=0;g->extra.brain_eating[i]=0;g->extra.zombotany_burst[i]=0;g->extra.zombotany_jump[i]=0;g->extra.last_portal_zombie[i]=0;g->mini.zombie_heads[i]=0;g->mini.zombie_attack[i]=1.5f;g->mini.zombie_portal[i]=0;
    if(game_mini_active(g,MINI_ZOMBOTANY)||game_mini_active(g,MINI_ZOMBOTANY2)){
        int head=game_mini_active(g,MINI_ZOMBOTANY2)?1+random_next(g)%6:z->kind==Z_BUCKET?2:1;g->mini.zombie_heads[i]=head;z->kind=Z_NORMAL;z->armor=head==2?1100:head==6?2200:0;
    }
    if(game_mini_active(g,MINI_LITTLE)){z->hp*=.25f;z->armor*=.25f;}
    if(g->mode==MODE_HEAVY)g->mini.hostile_y[i]=58+random_unit(g)*175;
}
static void mini_slot_resolve(Game *g){
    int peas=0;for(int r=0;r<5;r++)for(int c=0;c<9;c++)peas+=g->plants[r][c].kind==PEA;
    const int symbols[]={SUNFLOWER,PEA,SNOW,WALLNUT,50,51};
    for(int reel=0;reel<3;reel++){int w[6],sum=0;for(int i=0;i<6;i++){w[i]=i==1?200-20*(peas>5?5:peas):i==5?30:100;if(reel==2&&i!=5&&(symbols[i]==g->mini.reels[0]||symbols[i]==g->mini.reels[1]))w[i]+=w[i]/2;sum+=w[i];}int n=random_next(g)%sum,k=0;while(k<5&&n>=w[k])n-=w[k++];g->mini.reels[reel]=symbols[k];}
    int a=g->mini.reels[0],b=g->mini.reels[1],c=g->mini.reels[2],kind=a==b||a==c?a:b==c?b:-1,count=a==b&&b==c?3:1;if(kind<0){cue(GAME_SAMPLE(FX_BUZZER));return;}cue(FOLEY_CHIME);
    if(kind==50){int suns=count==3?20:4;for(int i=0;i<suns;i++)plant_sun(g,200+i*4,55);}
    else if(kind==51){for(int i=0;i<(count==3?5:1);i++)game_add_coin(g,200+i*8,60,100);}
    else for(int i=0;i<count&&g->packet_count<MAX_PACKETS;i++)g->packets[g->packet_count++]=kind;
}
static void mini_portals(Game *g,float dt){
 g->mini.portal_timer-=dt;if(g->mini.portal_timer<=0){g->mini.portal_timer=60;g->mini.portal_generation++;cue(FOLEY_PORTAL);int at=random_next(g)%4,other=g->mini.portals[at^1],candidates[50],count=0;for(int row=0;row<5;row++)for(int col=0;col<10;col++){int cell=row*10+col,free=col!=other%10&&row!=other/10;for(int p=0;p<4;p++)if(g->mini.portals[p]==cell)free=0;if(free)candidates[count++]=cell;}if(count)g->mini.portals[at]=candidates[random_next(g)%count];memset(g->extra.last_portal_zombie,0,sizeof(g->extra.last_portal_zombie));memset(g->extra.last_portal_shot,0,sizeof(g->extra.last_portal_shot));}
 for(int i=0;i<MAX_ZOMBIES;i++){Zombie *z=&g->zombies[i];if(!z->used||z->death>0)continue;int last=g->extra.last_portal_zombie[i];if(last&&fabsf(z->x-plant_x((last-1)%10))>26)g->extra.last_portal_zombie[i]=0;
  for(int p=0;p<4;p++){int cell=g->mini.portals[p];if(z->row==cell/10&&cell+1!=g->extra.last_portal_zombie[i]&&fabsf(z->x-plant_x(cell%10))<21){int to=g->mini.portals[p^1];z->row=to/10;z->x=plant_x(to%10)-12;g->extra.last_portal_zombie[i]=to+1;g->mini.zombie_portal[i]=1;cue(FOLEY_PORTAL);break;}}}
 for(int i=0;i<MAX_SHOTS;i++){Shot *s=&g->shots[i];if(!s->used||g->shot_vy[i]!=0)continue;int last=g->extra.last_portal_shot[i];if(last&&fabsf(s->x-plant_x((last-1)%10))>26)g->extra.last_portal_shot[i]=0;
  for(int p=0;p<4;p++){int cell=g->mini.portals[p];if(s->row==cell/10&&cell+1!=g->extra.last_portal_shot[i]&&fabsf(s->x-plant_x(cell%10))<19){int to=g->mini.portals[p^1];float delta=game_ground_y(g,to/10)-game_ground_y(g,s->row);s->row=to/10;s->x=plant_x(to%10)+(g->shot_vx[i]<0?-16:16);g->shot_y[i]+=delta;g->extra.last_portal_shot[i]=to+1;g->mini.shot_portal[i]=1;cue(FOLEY_PORTAL);break;}}}
 for(int i=0;i<ROWS;i++)if(g->mower_used[i]&&g->mower_x[i]<520){int row=g->extra.mower_row[i]?g->extra.mower_row[i]-1:i,last=g->extra.last_portal_mower[i];if(last&&fabsf(g->mower_x[i]-plant_x((last-1)%10))>26)g->extra.last_portal_mower[i]=0;
  for(int p=0;p<4;p++){int cell=g->mini.portals[p];if(row==cell/10&&cell+1!=g->extra.last_portal_mower[i]&&fabsf(g->mower_x[i]-plant_x(cell%10))<17){int to=g->mini.portals[p^1];g->mower_x[i]=plant_x(to%10)+12;g->extra.mower_row[i]=to/10+1;g->extra.last_portal_mower[i]=to+1;cue(FOLEY_PORTAL);break;}}}
}
static int mini_pre_step(Game *g,float dt){
    if(!g->challenge)return 0;
    if(game_mini_active(g,MINI_LASTSTAND)&&g->mini.phase==0)return 1;
    if(g->mode==MODE_HEAVY){g->extra.heavy_fire_time=fmaxf(0,g->extra.heavy_fire_time-dt);g->extra.heavy_ice_time=fmaxf(0,g->extra.heavy_ice_time-dt);
 for(int i=0;i<MAX_SUNS;i++)if(g->suns[i].used==1&&fabsf(g->suns[i].x-g->mini.heavy_x)<22&&g->suns[i].y>215){Sun *v=&g->suns[i];v->used=2;v->age=0;v->vx=v->x;v->vy=v->y;v->flight=.35f;}
 for(int i=0;i<MAX_COINS;i++)if(g->money[i].used==1&&fabsf(g->money[i].x-g->mini.heavy_x)<22){Coin *v=&g->money[i];v->used=2;v->age=0;v->vx=v->x;v->vy=v->y;}}

    for(int i=0;i<96;i++)if(g->mini.enemy_used[i]){g->mini.enemy_x[i]-=150*dt;int r=(int)((g->mini.enemy_y[i]+20.01f-GRID_Y)/game_row_height(g))-1;if(r<0)r=0;if(r>=game_rows(g))r=game_rows(g)-1;for(int c=0;c<COLS;c++)if(g->plants[r][c].kind>=0&&fabsf(g->mini.enemy_x[i]-plant_x(c))<12){g->plants[r][c].hp-=20;if(g->plants[r][c].hp<=0)g->plants[r][c].kind=-1;g->mini.enemy_used[i]=0;break;}if(g->mini.enemy_x[i]<0)g->mini.enemy_used[i]=0;}
    if(g->mode==MODE_AQUARIUM){int alive=0;for(int j=0;j<3;j++)if(g->mini.brains[j].used){AquariumBrain *b=&g->mini.brains[j];b->age+=dt;b->y+=5.7f*dt;if(b->y>240)b->used=0;}
        for(int i=0;i<100;i++)if(g->mini.fish[i].used){AquariumFish *f=&g->mini.fish[i];f->hunger-=dt;f->produce-=dt;if(f->hunger<=0){cue(GAME_SAMPLE(FX_ZOMBAQUARIUM_DIE));f->used=0;continue;}alive++;int target=-1;float best=10000;for(int j=0;j<3;j++)if(g->mini.brains[j].used){float d=hypotf(g->mini.brains[j].x-f->x,g->mini.brains[j].y-f->y);if(d<best){best=d;target=j;}}
            if(target>=0&&f->hunger<=15){AquariumBrain *b=&g->mini.brains[target];if(best<19&&b->age>=.15f){b->used=0;f->hunger=fminf(27,f->hunger+20);}else{f->dx=(b->x-f->x)/best*19;f->dy=(b->y-f->y)/best*19;}}
            else if(random_unit(g)<dt*.3f){float angle=random_unit(g)*6.283185f;f->dx=cosf(angle)*15;f->dy=sinf(angle)*12;}
            f->x+=f->dx*dt;f->y+=f->dy*dt;if(f->x<65||f->x>425){f->dx=-f->dx;f->x=fminf(425,fmaxf(65,f->x));}if(f->y<80||f->y>225){f->dy=-f->dy;f->y=fminf(225,fmaxf(80,f->y));}if(f->produce<=0){f->produce=10+random_unit(g)*5;plant_sun(g,f->x,f->y);}}
        if(!alive)g->status=LOST;return 1;
    }
    if(g->mode==MODE_SLOT&&g->mini.spinning){g->mini.spin_time-=dt;if(g->mini.spin_time<=0){g->mini.spinning=0;mini_slot_resolve(g);}}
    if(g->mode==MODE_MATCH){g->mini.timer=fmaxf(0,g->mini.timer-dt);for(int r=0;r<5;r++)for(int c=0;c<8;c++){g->mini.match_fall[r][c]=fminf(0,g->mini.match_fall[r][c]+180*dt);if(g->plants[r][c].kind<0&&g->craters[r][c]<=0)g->craters[r][c]=100000;}
        if(g->mini.timer<=0){g->mini.secondary-=dt;if(mini_match_scan(g,1)){g->mini.phase=0;g->mini.secondary=.5f;}else if(g->mini.secondary<=0){if(g->mini.phase==1){mini_shuffle(g);g->mini.phase=0;g->mini.secondary=.5f;}else if(!mini_has_move(g)){g->mini.phase=1;g->mini.secondary=5;}else g->mini.secondary=.5f;}}}
    if(game_mini_active(g,MINI_PORTAL))mini_portals(g,dt);
    return 0;
}
static int mini_progress(Game *g){
    if(!g->challenge)return 0;
    if(g->mode==MODE_MATCH||g->mode==MODE_SLOT||game_mini_active(g,MINI_STARS)){
        int win=g->mode==MODE_MATCH?g->mini.score>=75:g->mode==MODE_SLOT?g->sun>=2000:g->mini.star_count>=14;
        if(win){g->status=WON;sound(g,SFX_WIN);return 1;}
        if(g->wave==g->num_waves&&g->spawned>0&&!game_alive(g)){g->wave=g->num_waves-1;g->spawned=0;g->spawn_timer=5;}
    }
    if(game_mini_active(g,MINI_STARS)){static const unsigned short star[5]={0x008,0x018,0x07e,0x038,0x048};g->mini.star_count=0;for(int r=0;r<5;r++)for(int c=0;c<9;c++)if((star[r]&(1<<c))&&g->plants[r][c].kind==STAR)g->mini.star_count++;}
    if(game_mini_active(g,MINI_LASTSTAND)&&g->wave==g->num_waves&&g->spawned>0&&!game_alive(g)&&g->mini.stage<4){g->mini.stage++;g->mini.phase=0;g->sun+=250;g->wave=1;g->spawned=0;g->huge_wave_announced=0;g->huge_wave_timer=0;mini_replan(g);return 1;}
    return 0;
}
int game_mini_total(const Game *g){return g->mode==MODE_MATCH?75:g->mode==MODE_SLOT?2000:g->mode==MODE_AQUARIUM?1000:game_mini_active(g,MINI_STARS)?14:g->num_waves;}
