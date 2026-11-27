static int closest_enemy(Game *g,int row,float x,int any_lane,ZombieRows *rows){
    collect_zombie_rows(g,rows);
    int count=any_lane?rows->count:rows->lane_count[row];
    const uint8_t *slots=any_lane?rows->all:rows->lane[row];
    int target=-1;float best=100000;
    for(int slot=0;slot<count;slot++){Zombie *z=&g->zombies[slots[slot]];if(!z->used||z->death>0||z->kind==Z_BUNGEE)continue;
        float weight=fabsf(z->x-x)+(any_lane?abs(z->row-row)*CELL_W:0);if(any_lane&&z->kind==Z_BALLOON)weight-=1000;
        if(weight<best){best=weight;target=slots[slot];}}
    return target;
}
int game_cob_select(Game *g,int row,int col){
    if(g->status!=PLAYING||!game_row_active(g,row)||col<0||col>=COLS)return 0;if(g->plants[row][col].kind==-2&&col>0)col--;
    Plant *p=&g->plants[row][col];if(p->kind!=COB||p->attack>0)return 0;g->cob_row=row;g->cob_col=col;g->cob_aiming=1;return 1;
}
int game_cob_fire(Game *g,int row,int col){
    if(!g->cob_aiming||!game_row_active(g,row)||col<0||col>=COLS||g->cob_row<0||g->cob_row>=ROWS||g->cob_col<0||g->cob_col>=COLS)return 0;
    Plant *p=&g->plants[g->cob_row][g->cob_col];if(p->kind!=COB||p->attack>0){g->cob_aiming=0;return 0;}
    p->flash=41.0f/12;p->attack=30+55.0f/12;g->burst_target[g->cob_row][g->cob_col]=row*COLS+col;g->burst[g->cob_row][g->cob_col]=2.05f;g->burst_left[g->cob_row][g->cob_col]=1;g->cob_aiming=0;return 1;
}
static void fire_cob(Game *g,int row,int col,int target_row,int target_col){
    cue(FOLEY_COB_LAUNCH);
    float x=plant_x(col);
    int i=allocate_shot(g,target_row,x,13);
    if(i<0)return;
    g->shot_y[i]=game_ground_y(g,row)+game_roof_offset(g,x)-70;
    g->shot_vx[i]=0;g->shot_vy[i]=-380;g->shot_remaining[i]=.88f;g->shot_target[i]=-2;g->shot_damage[i]=target_col;
}
static int rake_target(const Game *g){
    float x=plant_x(g->rake_col);
    for(int i=0;i<MAX_ZOMBIES;i++){
        const Zombie *z=&g->zombies[i];
        if(ground_trap_target(g,i,g->rake_row)&&z->kind!=Z_BOBSLED&&fabsf(z->x-x)<25)return i;
    }
    return -1;
}
static void store_step(Game *g,float dt){
    if(!(g->store_flags&STORE_RAKE))return;
    if(!g->rake_state){
        int rows[ROWS],count=0;for(int r=0;r<game_rows(g);r++)if(game_row_active(g,r)&&!game_water_row(g,r))rows[count++]=r;
        if(!count)return;g->rake_row=rows[random_next(g)%count];g->rake_col=g->mode==MODE_VASE?COLS-4-g->stage:7;g->rake_state=1;
    }
    if(g->rake_state==1&&rake_target(g)>=0){g->rake_state=2;g->rake_timer=0;}
    else if(g->rake_state==2){
        float old=g->rake_timer;
        g->rake_timer+=dt;
        if(old<.28f&&g->rake_timer>=.28f){
            int i=rake_target(g);
            if(i>=0)hurt_zombie(g,&g->zombies[i],1800,0);
        }
        if(g->rake_timer>=2)g->rake_state=3;
    }
}

static void spike_contact(Game *g,int row,int col,ZombieRows *targets){
    Plant *p=&g->plants[row][col];
    int spikerock=p->kind==SPIKEROCK;
    float x=plant_x(col);
    collect_zombie_rows(g,targets);
    for(int slot=0;slot<targets->lane_count[row];slot++){
        int i=targets->lane[row][slot];
        Zombie *z=&g->zombies[i];
        if(!ground_trap_target(g,i,row)||fabsf(z->x-x)>=22)continue;
        if(z->kind==Z_ZOMBONI||z->kind==Z_CATAPULT){
            cue(FOLEY_TIRE_POP);
            kill_zombie(g,z);
            if(spikerock)p->hp-=50;
            if(!spikerock||p->hp<=0){destroy_square(g,row,col,0);return;}
        }else hurt_zombie(g,z,spikerock?40:20,0);
    }
    p->attack=1;
}
static void upgrade_plant(Game *g,int r,int c,float dt,ZombieRows *rows){
    Plant *p=&g->plants[r][c];int kind=p->kind;float x=plant_x(c);
    if(kind==TWIN&&p->attack<=0){p->attack=23.5f+random_unit(g)*1.5f;p->flash=1;plant_sun(g,x-5,game_ground_y(g,r)-22);plant_sun(g,x+5,game_ground_y(g,r)-22);cue(FOLEY_SPAWN_SUN);return;}
    if(kind==GATLING||kind==GLOOM||kind==CATTAIL){
        float radius=CELL_W*1.5f;
        if(p->attack<=0){
            int target=0;
            if(kind==GATLING)target=target_in_lane(g,r,x,rows);
            else if(kind==CATTAIL)target=closest_enemy(g,r,x,1,rows)>=0;
            else target=spore_attack_area(g,rows,r-1,r+1,x-radius,x+radius,0);
            if(target){p->attack=kind==GLOOM?2:1.5f;g->burst_left[r][c]=kind==CATTAIL?2:4;g->burst[r][c]=kind==GATLING?.32f:kind==GLOOM?.74f:.31f;p->flash=kind==GLOOM?2:1;}
        }
        if(g->burst_left[r][c]>0){g->burst[r][c]-=dt;if(g->burst[r][c]<=0){g->burst_left[r][c]--;g->burst[r][c]+=kind==GLOOM?.28f:kind==CATTAIL?.18f:.17f;
            if(kind==GLOOM){spore_attack_area(g,rows,r-1,r+1,x-radius,x+radius,1);cue(FOLEY_FUME);}
            else if(kind==GATLING)shoot(g,r,x+13,0);
            else{
                int target=closest_enemy(g,r,x,1,rows);
                int slot=target>=0?shoot(g,r,x+10,11):-1;
                if(slot>=0){g->shot_target[slot]=target;g->polish024.shot_actor[slot]=g->fidelity.actor_id[target];g->shot_vx[slot]=95;}
            }
        }}return;
    }
    if(kind==WINTER&&p->attack<=0){int target=closest_enemy(g,r,x,0,rows);if(target>=0||g->mode==MODE_BOSS){lob(g,r,x,12,target);p->attack=2.85f;p->flash=.4f;}return;}
    if(kind==GOLDMAGNET&&p->attack<=0){int count=0;for(int i=0;i<MAX_COINS&&count<5;i++){Coin *coin=&g->money[i];if(coin->used==1&&coin->value>0&&hypotf(coin->x-x,coin->y-game_ground_y(g,r))<CELL_W*6){coin->used=2;coin->age=0;coin->vx=coin->x;coin->vy=coin->y;count++;}}
        if(count){p->attack=6;p->flash=.5f;}return;}
    if(kind==SPIKEROCK&&p->attack<=0){spike_contact(g,r,c,rows);return;}
    if(kind==COB){if(p->attack<=0&&p->attack+dt>0)cue(FOLEY_SHOOP);
        if(c+1<COLS&&g->plants[r][c+1].kind!=-2)g->plants[r][c+1]=(Plant){.kind=-2};
        if(g->burst_left[r][c]){g->burst[r][c]-=dt;if(g->burst[r][c]<=0){int tile=g->burst_target[r][c];fire_cob(g,r,c,tile/COLS,tile%COLS);g->burst_left[r][c]=0;}}
    }
}
