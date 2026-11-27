/* Native handheld rules for the pool, fog, roof and chapter challenges. */
static void setup_vases(Game *g){
    adventure_vases(g);
}
static void chapter_start(Game *g){
    if(g->mode==MODE_VASE)setup_vases(g);
    if(g->mode==MODE_BOSS){g->boss_hp=40000;g->boss_timer=8;g->boss_row=2;g->spawn_timer=100000;}
    if(game_world(g)==4){
        for(int r=0;r<5;r++)for(int c=0;c<(g->level==41?5:g->level==42?4:3);c++){g->support[r][c]=POT+1;g->support_hp[r][c]=300;}
        if(g->mode==MODE_BOSS)for(int r=0;r<5;r++){put_plant(g,r,0,CABBAGE);put_plant(g,r,1,KERNEL);}
    }
}
int game_break_vase(Game *g,int row,int col){
    if(g->mode==MODE_VASE)return extra_break_vase(g,row,col);return 0;
}

static int nearby_umbrella(Game *g,int row,int col){
    for(int r=row-1;r<=row+1;r++)for(int c=col-1;c<=col+1;c++)if(r>=0&&r<ROWS&&c>=0&&c<COLS&&g->plants[r][c].kind==UMBRELLA&&!g->asleep[r][c])return 1;
    return 0;
}
static void lob(Game *g,int row,float x,int kind,int target){
    int i=allocate_shot(g,row,x,kind);
    if(i<0)return;
    float t=.85f,tx=target>=0?g->zombies[target].x-4:428;
    float ty=game_ground_y(g,row)+game_roof_offset(g,tx)-20,y=game_ground_y(g,row)+game_roof_offset(g,x)-25;
    g->shot_y[i]=y;g->shot_vx[i]=(tx-x)/t;g->shot_vy[i]=(ty-y-150*t*t)/t;g->shot_remaining[i]=t;g->shot_target[i]=target;
    g->shot_damage[i]=kind==7||kind==12?80:kind==5?20:40;sound_as(g,SFX_SHOOT,kind==6?FOLEY_BUTTER:FOLEY_THROW);
}
#include "upgrade_game.inc"

static void chapter_plant(Game *g,int r,int c,float dt,ZombieRows *rows){
    Plant *p=&g->plants[r][c];int k=p->kind;float x=plant_x(c);
    if(k>=GATLING){upgrade_plant(g,r,c,dt,rows);return;}
    if(k==SEA&&p->attack<=0&&spore_attack_area(g,rows,r,r,x-6,fminf(480,x+CELL_W*3),0)&&shoot(g,r,x+8,2)>=0){p->attack=1.5f;p->flash=.3f;}
    if(k==THREEPEATER&&p->attack<=0){int target=0;for(int row=r-1;row<=r+1&&!target;row++)if(game_row_active(g,row)&&target_in_lane(g,row,x,rows))target=1;if(target){for(int row=r-1;row<=r+1;row++)if(game_row_active(g,row))shoot(g,row,x+12,0);p->attack=1.5f;p->flash=.3f;}}
    if(k==SPLIT&&p->attack<=0){
        int fired=target_in_lane(g,r,x,rows)&&shoot(g,r,x+12,0)>=0;
        int back=lane_has_projectile_target(g,r,0,x-10,SHOT_HITS_GROUND,rows);
        if(back)for(int n=0;n<2;n++){
            int slot=shoot(g,r,x-12-n*6,0);
            if(slot<0)break;
            g->shot_vx[slot]=-150;fired=1;
        }
        if(fired){p->attack=1.5f;p->flash=.3f;}
    }
    if((k==CABBAGE||k==KERNEL||k==MELON)&&p->attack<=0){int target=-1;float nearest=500;
        for(int z=0;z<MAX_ZOMBIES;z++)if(g->zombies[z].used&&!g->hypnotized[z]&&g->zombies[z].death<=0&&g->zombies[z].kind!=Z_BALLOON&&g->zombies[z].row==r&&g->zombies[z].x>=x&&g->zombies[z].x<nearest){target=z;nearest=g->zombies[z].x;}
        if(target>=0||(g->mode==MODE_BOSS&&boss_exposed(g))){lob(g,r,x,k==CABBAGE?4:k==MELON?7:random_next(g)%4==0?6:5,target);p->attack=2.85f;p->flash=.4f;}}
    if(k==CACTUS&&p->attack<=0){
        int high=lane_has_projectile_target(g,r,x-6,480,SHOT_HITS_BALLOON,rows);
        if(high||target_in_lane(g,r,x,rows)){
            int slot=shoot(g,r,x+12,9);
            if(slot>=0){
                // The projectile record keeps its height class through saves and portals.
                g->shot_target[slot]=high?SHOT_HITS_BALLOON:SHOT_HITS_GROUND;
                if(high)g->shot_y[slot]-=BALLOON_LIFT;
                p->attack=1.5f;p->flash=.3f;
            }
        }
    }
    if(k==STAR&&p->attack<=0&&game_alive(g)){
        static const float vx[5]={-150,0,0,106,106},vy[5]={0,-150,150,-106,106};
        for(int n=0;n<5;n++){
            int slot=shoot(g,r,x,8);
            if(slot<0)break;
            g->shot_vx[slot]=vx[n];g->shot_vy[slot]=vy[n];
        }
        p->attack=1.5f;p->flash=.3f;
    }
    if(k==SQUASH){
        int *phase=&g->burst_left[r][c];float *time=&g->burst[r][c];
        if(!*phase){int target=-1;float nearest=45;for(int z=0;z<MAX_ZOMBIES;z++){Zombie *enemy=&g->zombies[z];float distance=fabsf(enemy->x-x);if(enemy->used&&enemy->death<=0&&!g->hypnotized[z]&&enemy->row==r&&!zombie_airborne(g,enemy)&&enemy->kind!=Z_BUNGEE&&enemy->vault<=0&&!(enemy->kind==Z_DIGGER&&!enemy->vaulted)&&distance<nearest){target=z;nearest=distance;}}
            if(target>=0){*phase=1;*time=0;g->burst_target[r][c]=target;g->pea_shoot_time[r][c]=g->zombies[target].x;}return;}
        float previous=*time;*time+=dt;
        if(*phase==1&&*time>=.8f){*phase=2;*time=0;}
        else if(*phase==2&&*time>=.45f){int z=-1;float best=45;for(int j=0;j<MAX_ZOMBIES;j++){Zombie *candidate=&g->zombies[j];float distance=fabsf(candidate->x-x);if(candidate->used&&candidate->death<=0&&!g->hypnotized[j]&&candidate->row==r&&!zombie_airborne(g,candidate)&&candidate->kind!=Z_BUNGEE&&candidate->vault<=0&&distance<best){best=distance;z=j;}}if(z>=0){Zombie predicted=g->zombies[z];g->pea_shoot_time[r][c]=predicted.x-(predicted.eating||g->frozen[z]>0?0:game_zombie_walk(&predicted,.3f));}*phase=3;*time=0;}
        else if(*phase==3&&*time>=.5f){*phase=4;*time=0;}
        else if(*phase==4){if(previous<.05f&&*time>=.05f){float target=g->pea_shoot_time[r][c];for(int j=0;j<MAX_ZOMBIES;j++)if(g->zombies[j].used&&g->zombies[j].death<=0&&!g->hypnotized[j]&&g->zombies[j].kind!=Z_BALLOON&&g->zombies[j].row==r&&fabsf(g->zombies[j].x-target)<30)hurt_zombie(g,&g->zombies[j],1800,0);}
            if(*time>=.1f){*phase=5;*time=0;}}
        else if(*phase==5&&*time>=1){p->kind=-1;*phase=0;}return;
    }
    if(k==KELP){for(int z=0;z<MAX_ZOMBIES;z++){Zombie *enemy=&g->zombies[z];if(enemy->used&&!g->hypnotized[z]&&enemy->death<=0&&enemy->row==r&&fabsf(enemy->x-x)<28&&enemy->vault<=0&&!zombie_airborne(g,enemy)&&enemy->kind!=Z_BUNGEE&&!(enemy->kind==Z_DIGGER&&!enemy->vaulted)){cue(FOLEY_FLOOP);if(enemy->kind<=Z_DUCK)g->polish.death_style[z]=DEATH_WATER;kill_zombie(g,enemy);p->kind=-1;sound_as(g,SFX_CHOMP,FOLEY_SLURP);cue(FOLEY_ZOMBIESPLASH);return;}}}
    if(k==JALAPENO&&p->age>=1){
        for(int z=0;z<MAX_ZOMBIES;z++)if(g->zombies[z].used&&!g->hypnotized[z]&&g->zombies[z].death<=0&&g->zombies[z].row==r)hurt_charred(g,&g->zombies[z],1800);
        for(int col=0;col<COLS;col++)g->ice_tiles[r][col]=0;
        if(g->mode==MODE_BOSS){if(g->boss_ball_kind==2&&r==g->boss_ball_row)g->boss_ball_kind=0;if(boss_exposed(g))g->boss_hp-=1800;}
        g->flash=.25f;p->kind=-1;sound_as(g,SFX_EXPLODE,FOLEY_JALAPENO_IGNITE);cue(FOLEY_JUICY);return;
    }
    if(k==BLOVER&&p->age>=1){for(int z=0;z<MAX_ZOMBIES;z++)if(g->zombies[z].used&&g->zombies[z].kind==Z_BALLOON&&!g->fidelity.equipment_lost[z])kill_zombie(g,&g->zombies[z]);g->fog_clear=24;p->kind=-1;cue(GAME_SAMPLE(FX_BLOVER));}
    if(k==SPIKEWEED&&p->attack<=0)spike_contact(g,r,c,rows);
    if(k==MAGNET&&p->attack<=0){for(int z=0;z<MAX_ZOMBIES;z++){Zombie *enemy=&g->zombies[z];if(enemy->used&&enemy->death<=0&&abs(enemy->row-r)<=2&&fabsf(enemy->x-x)<CELL_W*3&&((enemy->armor>0&&(enemy->kind==Z_BUCKET||enemy->kind==Z_DOOR||enemy->kind==Z_FOOTBALL||enemy->kind==Z_LADDER))||enemy->kind==Z_POGO||enemy->kind==Z_DIGGER||enemy->kind==Z_JACK)){
        enemy->armor=0;enemy->vaulted=1;if(enemy->kind==Z_POGO||enemy->kind==Z_DIGGER||enemy->kind==Z_JACK)enemy->kind=Z_NORMAL;p->attack=15;p->flash=.5f;cue(FOLEY_MAGNETSHROOM);break;
    }}}
    if(k==MARIGOLD&&p->attack<=0){game_add_coin(g,x,game_ground_y(g,r)-15,random_next(g)%10?1:5);p->attack=24;p->flash=1;}
    if(k==ICESHROOM&&p->age>=1&&g->mode==MODE_BOSS){if(g->boss_ball_kind==1)g->boss_ball_kind=0;if(boss_exposed(g)){g->boss_phase=10;g->boss_hp-=20;}}
    (void)dt;
}
static int projectile_portal_entry(Game *g,int i,float from,float to,float *contact){
    int *last=&g->extra.last_portal_shot[i];
    if(*last&&fabsf(from-plant_x((*last-1)%10))>26)*last=0;
    float low=fminf(from,to),high=fmaxf(from,to),nearest=INFINITY;
    int entry=-1;
    for(int p=0;p<4;p++){
        int cell=g->mini.portals[p];
        if(cell/10!=g->shots[i].row||cell+1==*last)continue;
        float center=plant_x(cell%10),left=center-19,right=center+19;
        if(low>=right||high<=left)continue;
        float at=fmaxf(left,fminf(right,from)),distance=fabsf(at-from);
        if(distance<nearest){nearest=distance;entry=p;*contact=at;}
    }
    return entry;
}
static void transfer_projectile(Game *g,int i,int entry){
    Shot *s=&g->shots[i];
    int destination=g->mini.portals[entry^1],row=destination/10;
    g->shot_y[i]+=game_ground_y(g,row)-game_ground_y(g,s->row);
    s->row=row;
    s->x=plant_x(destination%10)+(g->shot_vx[i]<0?-16:16);
    g->extra.last_portal_shot[i]=destination+1;
    g->mini.shot_portal[i]=1;
    cue(FOLEY_PORTAL);
}
static void torchwood_pass(Game *g,int i,float from,float to){
    Shot *s=&g->shots[i];
    float low=fminf(from,to),high=fmaxf(from,to);
    int direction=to<from?-1:1;
    for(int c=direction<0?COLS-1:0;(unsigned)c<COLS;c+=direction){
        if(g->plants[s->row][c].kind!=TORCHWOOD)continue;
        float x=plant_x(c),left=x-8,right=x+8;
        if(low>=right||high<=left)continue;
        int cell=s->row*COLS+c;
        // Peas use this saved slot for the Torchwood they are passing.
        if(g->shot_target[i]==cell&&from>left&&from<right)continue;
        g->shot_target[i]=cell;
        if(s->ice==1){s->ice=0;continue;}
        s->ice=10;g->shot_damage[i]=40;
        cue(FOLEY_FIREPEA);cue(FOLEY_IGNITE);
        return;
    }
}
static void direct_shot_hit(Game *g,int i,float from,float from_y,ZombieRows *targets,int heavy,int roof){
    Shot *s=&g->shots[i];
    collect_zombie_rows(g,targets);
    // Impact changes health and status, so test those live as shots resolve.
    int cross_lane=heavy||s->ice==8,count=cross_lane?targets->count:targets->lane_count[s->row];
    const uint8_t *slots=cross_lane?targets->all:targets->lane[s->row];
    int mask=SHOT_HITS_GROUND;
    if(s->ice==9){
        mask=g->shot_target[i];
        // Older snapshots gave cactus projectiles no height class.
        if(mask!=SHOT_HITS_BALLOON&&mask!=SHOT_HITS_GROUND)mask=SHOT_HITS_GROUND|SHOT_HITS_BALLOON;
    }
    float lift=mask==SHOT_HITS_BALLOON?BALLOON_LIFT:0;
    float dx=s->x-from,dy=g->shot_y[i]-from_y,length_squared=dx*dx+dy*dy;
    float end_x=from+dx,min_x=fminf(from,end_x)-13,max_x=fmaxf(from,end_x)+13;
    int target=-1;float closest=10000;
    float target_x=0,target_y=0;
    for(int k=0;k<count;k++){
        int j=slots[k];Zombie *z=&g->zombies[j];float zx=z->x-8;
        if(zx<min_x||zx>max_x||!(projectile_target_kind(g,j)&mask))continue;
        float zy=game_ground_y(g,z->row)+(roof?game_roof_offset(g,z->x):0)-17-lift;
        float t=length_squared?fmaxf(0,fminf(1,((zx-from)*dx+(zy-from_y)*dy)/length_squared)):0;
        float order=heavy?t:fabsf(z->x-from);
        float distance_x=from+dx*t-zx,distance_y=from_y+dy*t-zy;
        if(order<closest&&fabsf(distance_x)<13&&fabsf(distance_y)<13&&hypotf(distance_x,distance_y)<13){
            target=j;closest=order;target_x=zx;target_y=zy;
        }
    }
    if(s->ice==0||s->ice==1){
        float until=end_x;
        if(target>=0){
            // Limit Torchwood checks to the shot's path before first contact.
            until=from;
            if(length_squared>0){
                float along=((target_x-from)*dx+(target_y-from_y)*dy)/length_squared;
                float cross=(target_x-from)*dy-(target_y-from_y)*dx;
                float span=sqrtf(fmaxf(0,(13*13-cross*cross/length_squared)/length_squared));
                until+=dx*fmaxf(0,along-span);
            }
        }
        torchwood_pass(g,i,from,until);
    }
    if(target>=0){
        Zombie *z=&g->zombies[target];
        if(z->kind==Z_BALLOON&&s->ice==9&&!g->fidelity.equipment_lost[target]){
            g->fidelity.equipment_lost[target]=1;g->polish.action[target]=ACTION_BALLOON_FALL;g->polish.action_time[target]=0;z->age=0;cue(GAME_SAMPLE(FX_BALLOON_POP));
        }
        hurt_zombie(g,z,g->shot_damage[i],s->ice==1);
        s->used=0;
    }
}
static void chapter_shots_step(Game *g,float dt,ZombieRows *targets){
    int heavy=g->mode==MODE_HEAVY,roof=game_world(g)==4,portals=game_mini_active(g,MINI_PORTAL);
    for(int i=0;i<MAX_SHOTS;i++)if(g->shots[i].used){
        Shot *s=&g->shots[i];float old=s->x,oldy=g->shot_y[i],vx=g->shot_vx[i];s->x+=vx*dt;g->shot_y[i]+=g->shot_vy[i]*dt;
        if(s->ice==14){g->shot_vy[i]+=300*dt;g->shot_remaining[i]-=dt;if(g->shot_remaining[i]<=0){int cell=g->shot_target[i],r=cell/COLS,c=cell%COLS;if(r>=0&&r<ROWS&&c>=0&&c<COLS){if(nearby_umbrella(g,r,c))cue(FOLEY_UMBRELLA);else if(g->plants[r][c].kind>=0){Plant *p=&g->plants[r][c];extra_plant_damage(g,r,c,p->hp,p->hp-g->shot_damage[i]);p->hp-=g->shot_damage[i];if(p->hp<=0)destroy_square(g,r,c,0);}}s->used=0;}continue;}
        if(s->ice==11){
         int target=g->shot_target[i];
         if(target<0||target>=MAX_ZOMBIES||!g->zombies[target].used||g->zombies[target].death>0||g->hypnotized[target]||bungee_away(g,&g->zombies[target])||(g->polish024.shot_actor[i]&&g->polish024.shot_actor[i]!=g->fidelity.actor_id[target])){
          target=-1;float best=1e9f;
          if(g->mode==MODE_HEAVY)target=heavy_homing_target(g,g->mini.heavy_x);
          else for(int n=0;n<MAX_ZOMBIES;n++){Zombie *z=&g->zombies[n];if(!z->used||z->death>0||g->hypnotized[n]||bungee_away(g,z)||(z->kind==Z_DIGGER&&!z->vaulted))continue;
           float y=game_ground_y(g,z->row)+game_roof_offset(g,z->x)-18,d=hypotf(z->x-s->x,y-g->shot_y[i]);if(d<best){best=d;target=n;}}
          g->shot_target[i]=target;g->polish024.shot_actor[i]=target>=0?g->fidelity.actor_id[target]:0;
         }
         if(target<0){if(g->mode!=MODE_HEAVY||s->x>490||s->x<0||g->shot_y[i]<0||g->shot_y[i]>270)s->used=0;continue;}Zombie *enemy=&g->zombies[target];float ty=game_ground_y(g,enemy->row)+game_roof_offset(g,enemy->x)-18,dx=enemy->x-s->x,dy=ty-g->shot_y[i],distance=hypotf(dx,dy);
         if(distance<12){if(enemy->kind==Z_BALLOON&&!g->fidelity.equipment_lost[target]){g->fidelity.equipment_lost[target]=1;g->polish.action[target]=ACTION_BALLOON_FALL;g->polish.action_time[target]=0;cue(GAME_SAMPLE(FX_BALLOON_POP));}hurt_zombie(g,enemy,20,0);s->used=0;}
         else{float speed=fminf(285,hypotf(g->shot_vx[i],g->shot_vy[i])+190*dt);g->shot_vx[i]=dx/distance*speed;g->shot_vy[i]=dy/distance*speed;}continue;
        }
        int lobbed=(s->ice>=4&&s->ice<=7)||s->ice==12;
        if(s->ice==13){g->shot_remaining[i]-=dt;if(g->shot_remaining[i]>0)continue;
            if(g->shot_target[i]==-2){s->x=plant_x((int)g->shot_damage[i]);g->shot_y[i]=game_ground_y(g,s->row)+game_roof_offset(g,s->x)-334;g->shot_vy[i]=380;g->shot_remaining[i]=.88f;g->shot_target[i]=-3;continue;}
            for(int j=0;j<MAX_ZOMBIES;j++)if(g->zombies[j].used&&g->zombies[j].death<=0&&!g->hypnotized[j]&&abs(g->zombies[j].row-s->row)<=1&&fabsf(g->zombies[j].x-s->x)<CELL_W*1.5f)hurt_zombie(g,&g->zombies[j],1800,0);if(g->mode==MODE_BOSS&&boss_exposed(g))g->boss_hp-=1800;g->flash=.25f;sound(g,SFX_EXPLODE);s->used=0;continue;}
        if(lobbed){g->shot_vy[i]+=300*dt;g->shot_remaining[i]-=dt;
            if(g->shot_remaining[i]>0)continue;
            collect_zombie_rows(g,targets);
            int target=-1;float nearest=25;
            int count=(unsigned)s->row<ROWS?targets->lane_count[s->row]:0;
            for(int k=0;k<count;k++){
                int n=targets->lane[s->row][k];Zombie *candidate=&g->zombies[n];
                float distance=fabsf(candidate->x-s->x);
                if(distance<nearest&&candidate->used&&candidate->death<=0&&!zombie_airborne(g,candidate)&&!bungee_away(g,candidate)&&!(candidate->kind==Z_DIGGER&&!candidate->vaulted)){target=n;nearest=distance;}
            }
            if(target>=0){Zombie *z=&g->zombies[target];float armor=z->armor;if(z->kind==Z_DOOR||z->kind==Z_PAPER)z->armor=0;hurt_zombie(g,z,g->shot_damage[i],0);if(z->kind==Z_DOOR||z->kind==Z_PAPER)z->armor=armor;if(s->ice==6&&can_freeze(g,z))g->frozen[target]=4;cue(s->ice==6?FOLEY_BUTTER:s->ice==5?FOLEY_KERNEL_SPLAT:s->ice==7||s->ice==12?FOLEY_MELONIMPACT:FOLEY_SPLAT);
                if(s->ice==12&&can_chill(g,z))z->slow=8;
                if(s->ice==7||s->ice==12)for(int j=0;j<MAX_ZOMBIES;j++)if(j!=target&&g->zombies[j].used&&g->zombies[j].death<=0&&!g->hypnotized[j]&&abs(g->zombies[j].row-z->row)<=1&&fabsf(g->zombies[j].x-z->x)<CELL_W)hurt_zombie(g,&g->zombies[j],26,s->ice==12);
            }else if(g->mode==MODE_BOSS&&boss_exposed(g))g->boss_hp-=g->shot_damage[i];s->used=0;continue;
        }
        if(s->ice==2){g->shot_remaining[i]-=fabsf(vx)*dt;if(g->shot_remaining[i]<=0){s->used=0;continue;}}
        if((unsigned)s->row>=ROWS){s->used=0;continue;}
        float contact;
        int entry=portals&&g->shot_vy[i]==0?projectile_portal_entry(g,i,old,s->x,&contact):-1;
        if(entry>=0){
            float remaining=s->x-contact;
            s->x=contact;
            if(contact!=old)direct_shot_hit(g,i,old,oldy,targets,heavy,roof);
            if(!s->used)continue;
            transfer_projectile(g,i,entry);
            old=s->x;oldy=g->shot_y[i];
            s->x+=remaining;
        }
        direct_shot_hit(g,i,old,oldy,targets,heavy,roof);
        if(roof&&s->ice!=8&&g->shot_y[i]>game_ground_y(g,s->row)+game_roof_offset(g,s->x)-7)s->used=0;
        if(s->x>490||s->x<0||g->shot_y[i]<0||g->shot_y[i]>270)s->used=0;
    }
}
static int chapter_zombie(Game *g,int i,float dt){
    Zombie *z=&g->zombies[i];if(z->death>0)return 0;
    if(g->mode==MODE_BUNGEE&&g->special[i]<0&&z->kind!=Z_BUNGEE){g->special[i]=fminf(0,g->special[i]+dt);z->age+=dt;return 1;}
    if(z->kind==Z_BUNGEE){
        int phase=g->fidelity.bungee_phase[i],r=z->row,c=(int)((z->x-GRID_X)/CELL_W);if(c<0)c=0;if(c>=COLS)c=COLS-1;
        g->special[i]+=dt*(phase==2&&z->slow>0?.5f:1);z->age+=dt;
        if(phase==0&&g->special[i]>=fmaxf(.01f,g->fidelity.actor_aux[i])){g->special[i]=0;z->age=0;int child=actor_by_id024(g,g->polish024.bungee_child_id[i]);if(nearby_umbrella(g,r,c)){if(child>=0)g->zombies[child].used=0;g->fidelity.bungee_phase[i]=4;cue(FOLEY_UMBRELLA);cue(GAME_SAMPLE(FX_BOING));}else if(child>=0){g->polish.action[child]=0;g->polish024.altitude[child]=0;g->polish024.leader_id[child]=0;g->fidelity.bungee_phase[i]=3;}else g->fidelity.bungee_phase[i]=1;}
        else if(phase==1&&g->special[i]>=3){
            int layer=g->plants[r][c].kind>=0?0:g->pumpkin_hp[r][c]>0?1:2;
            int kind=layer==0?g->plants[r][c].kind:layer==1?PUMPKIN:g->support[r][c]-1;
            g->fidelity.actor_target[i]=r*COLS+c+layer*ROWS*COLS;g->fidelity.grabbed_id[i]=g->fidelity.plant_id[layer][r][c];
            g->fidelity.carried_kind[i]=(kind>=0&&kind!=COB&&kind!=GRAVEBUSTER&&!(kind==SQUASH&&g->burst_left[r][c]>=3))?kind+1:0;
            g->fidelity.carried_age[i]=g->plants[r][c].age;g->fidelity.bungee_phase[i]=2;g->special[i]=0;z->age=0;
        }else if(phase==2&&g->special[i]>=14/24.0f){
            int layer=g->fidelity.actor_target[i]/(ROWS*COLS),kind=g->fidelity.carried_kind[i]-1;
            int current=layer==0?g->plants[r][c].kind:layer==1?(g->pumpkin_hp[r][c]>0?PUMPKIN:-1):g->support[r][c]-1;
            if(kind>=0&&kind==current&&g->fidelity.grabbed_id[i]==g->fidelity.plant_id[layer][r][c]){
                if(layer==0)destroy_square(g,r,c,0);else if(layer==1)remove_pumpkin(g,r,c);else{g->support[r][c]=0;g->support_hp[r][c]=0;}cue(FOLEY_FLOOP);
            }else g->fidelity.carried_kind[i]=0;
            g->fidelity.bungee_phase[i]=3;g->special[i]=0;z->age=0;
        }else if(phase>=3&&g->special[i]>=.75f)z->used=0;
        return 1;
    }
    if(g->frozen[i]>0)return 0;
    if(z->kind==Z_BALLOON&&!g->fidelity.equipment_lost[i]){z->x-=game_zombie_walk(z,dt);if(z->x<12){if(game_zombie_has_head(g,z))g->status=LOST;else kill_zombie(g,z);}return 1;}
    if(z->kind==Z_DIGGER&&!z->vaulted){z->x-=16*dt;z->age+=dt;if(z->x<GRID_X){z->vaulted=1;g->special[i]=0;g->polish.action[i]=ACTION_DIGGER_RISE;g->polish.action_time[i]=0;cue(FOLEY_DIRT_RISE);}return 1;}
    if(z->kind==Z_DIGGER&&z->vaulted&&!g->hypnotized[i]){
        if(g->special[i]>0){g->special[i]-=dt;return 1;}
        int was_eating=z->eating,row=z->row;
        z->eating=0;
        if(game_zombie_has_head(g,z))for(int c=0;c<COLS;c++){
            if(fabsf(z->x-plant_x(c))>=18)continue;
            int anchor=bite_plant_col(g,row,c);
            int plant=squash_in_flight(g,row,anchor)?-1:g->plants[row][anchor].kind;
            if(tile_is_bite_target(g,row,c,plant,z->kind)){
                chew_tile(g,i,c,anchor,plant,dt);
                break;
            }
        }
        if(z->eating!=was_eating){z->age=0;g->polish.pose[i]=0;}
        if(!z->eating)z->x+=(g->fidelity.equipment_lost[i]?-1:1)*game_zombie_walk(z,dt);
        else z->age+=dt*(z->slow>0?.5f:1);
        if(z->x>510)z->used=0;
        return 1;
    }
    if(z->kind==Z_JACK&&!g->fidelity.equipment_lost[i]&&game_zombie_has_head(g,z)){g->special[i]+=dt*(z->slow>0?.5f:1);if(g->special[i]>(g->fidelity.actor_aux[i]>0?g->fidelity.actor_aux[i]:12)){g->polish.action[i]=ACTION_JACK;g->polish.action_time[i]=0;z->eating=0;cue(GAME_SAMPLE(FX_BOING));return 1;}}
    if(z->kind==Z_ZOMBONI){for(int c=0;c<COLS;c++)if(plant_x(c)>z->x)g->ice_tiles[z->row][c]=30;}
    if(z->kind==Z_CATAPULT&&z->x>360){
        int target=-1;for(int c=0;c<COLS;c++)if(g->plants[z->row][c].kind>=0){target=c;break;}
        if(target>=0&&g->fidelity.actor_aux[i]>0){g->special[i]-=dt;if(g->special[i]<=0){g->special[i]=3;g->polish.action[i]=ACTION_CATAPULT;g->polish.action_time[i]=0;g->fidelity.actor_target[i]=z->row*COLS+target;}return 1;}
    }
    if(z->kind==Z_GARG&&z->hp<(g->completion.giga[i]?3000:1500)&&!z->vaulted){g->polish.action[i]=ACTION_GARG_THROW;g->polish.action_time[i]=0;g->special[i]=0;z->eating=0;return 1;}
    return 0;
}
#include "boss_rules.inc"
static int chapter_progress(Game *g,float dt){
    if(g->mode==MODE_VASE){int left=0;for(int r=0;r<5;r++)for(int c=0;c<COLS;c++)left+=!!g->vases[r][c];
        if(!left&&!game_alive(g)){if(++g->stage==3){g->status=WON;sound(g,SFX_WIN);}else{for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)g->plants[r][c].kind=-1;setup_vases(g);}}return 1;
    }
    if(g->mode!=MODE_BOSS)return 0;
    return boss_step(g,dt);
}
