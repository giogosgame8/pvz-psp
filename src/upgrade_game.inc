static int closest_enemy(Game *g,int row,float x,int any_lane){
    int target=-1;float best=100000;
    for(int i=0;i<MAX_ZOMBIES;i++){Zombie *z=&g->zombies[i];if(!z->used||z->death>0||g->hypnotized[i]||z->kind==Z_BUNGEE||(!any_lane&&z->row!=row))continue;
        float weight=fabsf(z->x-x)+(any_lane?abs(z->row-row)*CELL_W:0);if(any_lane&&z->kind==Z_BALLOON)weight-=1000;
        if(weight<best){best=weight;target=i;}}
    return target;
}
static void upgrade_damage_ring(Game *g,int row,float x){
    for(int i=0;i<MAX_ZOMBIES;i++){Zombie *z=&g->zombies[i];if(!z->used||z->death>0||g->hypnotized[i]||z->kind==Z_BALLOON||abs(z->row-row)>1||fabsf(z->x-x)>CELL_W*1.5f)continue;
        float shield=z->armor;if(z->kind==Z_DOOR||z->kind==Z_PAPER)z->armor=0;hurt_zombie(g,z,20,0);if(z->kind==Z_DOOR||z->kind==Z_PAPER)z->armor=shield;}
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
static void fire_cob(Game *g,int row,int col,int target_row,int target_col){cue(FOLEY_COB_LAUNCH);
    for(int i=0;i<MAX_SHOTS;i++)if(!g->shots[i].used){
        float x=plant_x(col);
        g->shots[i]=(Shot){1,target_row,13,x};g->shot_y[i]=game_ground_y(g,row)+game_roof_offset(g,x)-70;
        g->shot_vx[i]=0;g->shot_vy[i]=-380;g->shot_remaining[i]=.88f;g->shot_target[i]=-2;g->shot_damage[i]=target_col;return;
    }
}
static void store_step(Game *g,float dt){
    if(!(g->store_flags&STORE_RAKE))return;
    if(!g->rake_state){
        int rows[ROWS],count=0;for(int r=0;r<game_rows(g);r++)if(game_row_active(g,r)&&!game_water_row(g,r))rows[count++]=r;
        if(!count)return;g->rake_row=rows[random_next(g)%count];g->rake_col=g->mode==MODE_VASE?COLS-4-g->stage:7;g->rake_state=1;
    }
    if(g->rake_state==1){for(int i=0;i<MAX_ZOMBIES;i++){Zombie *z=&g->zombies[i];if(z->used&&z->death<=0&&!g->hypnotized[i]&&z->row==g->rake_row&&!zombie_airborne(g,z)&&z->kind!=Z_BUNGEE&&z->kind!=Z_BOBSLED&&fabsf(z->x-plant_x(g->rake_col))<25){g->rake_state=2;g->rake_timer=0;break;}}}
    else if(g->rake_state==2){float old=g->rake_timer;g->rake_timer+=dt;if(old<.28f&&g->rake_timer>=.28f){for(int i=0;i<MAX_ZOMBIES;i++){Zombie *z=&g->zombies[i];if(z->used&&z->death<=0&&z->row==g->rake_row&&fabsf(z->x-plant_x(g->rake_col))<25){hurt_zombie(g,z,1800,0);break;}}}if(g->rake_timer>=2)g->rake_state=3;}
}
static void upgrade_plant(Game *g,int r,int c,float dt){
    Plant *p=&g->plants[r][c];int kind=p->kind;float x=plant_x(c);
    if(kind==TWIN&&p->attack<=0){p->attack=23.5f+random_unit(g)*1.5f;p->flash=1;plant_sun(g,x-5,game_ground_y(g,r)-22);plant_sun(g,x+5,game_ground_y(g,r)-22);cue(FOLEY_SPAWN_SUN);return;}
    if(kind==GATLING||kind==GLOOM||kind==CATTAIL){
        int target=kind==GATLING?target_in_lane(g,r,x):closest_enemy(g,r,x,kind==CATTAIL)>=0;
        if(kind==GLOOM){target=0;for(int i=0;i<MAX_ZOMBIES;i++)if(g->zombies[i].used&&g->zombies[i].death<=0&&abs(g->zombies[i].row-r)<=1&&fabsf(g->zombies[i].x-x)<=CELL_W*1.5f)target=1;}
        if(p->attack<=0&&target){p->attack=kind==GLOOM?2:1.5f;g->burst_left[r][c]=kind==CATTAIL?2:4;g->burst[r][c]=kind==GATLING?.32f:kind==GLOOM?.74f:.31f;p->flash=kind==GLOOM?2:1;}
        if(g->burst_left[r][c]>0){g->burst[r][c]-=dt;if(g->burst[r][c]<=0){g->burst_left[r][c]--;g->burst[r][c]+=kind==GLOOM?.28f:kind==CATTAIL?.18f:.17f;
            if(kind==GLOOM){upgrade_damage_ring(g,r,x);cue(FOLEY_FUME);}
            else if(kind==GATLING)shoot(g,r,x+13,0);
            else{int target=closest_enemy(g,r,x,1);if(target>=0){int slot=-1;for(int i=0;i<MAX_SHOTS;i++)if(!g->shots[i].used){slot=i;break;}if(slot>=0){shoot(g,r,x+10,11);g->shot_target[slot]=target;g->shot_vx[slot]=95;}}}
        }}return;
    }
    if(kind==WINTER&&p->attack<=0){int target=closest_enemy(g,r,x,0);if(target>=0||g->mode==MODE_BOSS){lob(g,r,x,12,target);p->attack=2.85f;p->flash=.4f;}return;}
    if(kind==GOLDMAGNET&&p->attack<=0){int count=0;for(int i=0;i<MAX_COINS&&count<5;i++){Coin *coin=&g->money[i];if(coin->used==1&&coin->value>0&&hypotf(coin->x-x,coin->y-game_ground_y(g,r))<CELL_W*6){coin->used=2;coin->age=0;coin->vx=coin->x;coin->vy=coin->y;count++;}}
        if(count){p->attack=6;p->flash=.5f;}return;}
    if(kind==SPIKEROCK&&p->attack<=0){for(int i=0;i<MAX_ZOMBIES;i++){Zombie *z=&g->zombies[i];if(z->used&&z->death<=0&&z->row==r&&fabsf(z->x-x)<22&&!zombie_airborne(g,z)){if(z->kind==Z_ZOMBONI||z->kind==Z_CATAPULT){cue(FOLEY_TIRE_POP);kill_zombie(g,z);p->hp-=50;if(p->hp<=0){p->kind=-1;break;}}else hurt_zombie(g,z,40,0);}}p->attack=1;return;}
    if(kind==COB){if(p->attack<=0&&p->attack+dt>0)cue(FOLEY_SHOOP);
        if(c+1<COLS&&g->plants[r][c+1].kind!=-2)g->plants[r][c+1]=(Plant){.kind=-2};
        if(g->burst_left[r][c]){g->burst[r][c]-=dt;if(g->burst[r][c]<=0){int tile=g->burst_target[r][c];fire_cob(g,r,c,tile/COLS,tile%COLS);g->burst_left[r][c]=0;}}
    }
}
