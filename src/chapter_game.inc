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
    for(int i=0;i<MAX_SHOTS;i++)if(!g->shots[i].used){
        float t=.85f,tx=target>=0?g->zombies[target].x-4:428;
        float ty=game_ground_y(g,row)+game_roof_offset(g,tx)-20,y=game_ground_y(g,row)+game_roof_offset(g,x)-25;
        g->shots[i]=(Shot){1,row,kind,x};g->shot_y[i]=y;g->shot_vx[i]=(tx-x)/t;g->shot_vy[i]=(ty-y-150*t*t)/t;g->shot_remaining[i]=t;g->shot_target[i]=target;
        g->shot_damage[i]=kind==7||kind==12?80:kind==5?20:40;sound_as(g,SFX_SHOOT,kind==6?FOLEY_BUTTER:FOLEY_THROW);return;
    }
}
#include "upgrade_game.inc"

static void chapter_plant(Game *g,int r,int c,float dt){
    Plant *p=&g->plants[r][c];int k=p->kind;float x=plant_x(c);
    if(k>=GATLING){upgrade_plant(g,r,c,dt);return;}
    if(k==SEA&&p->attack<=0&&target_in_lane(g,r,x)){shoot(g,r,x+8,2);p->attack=1.5f;p->flash=.3f;}
    if(k==THREEPEATER&&p->attack<=0){int target=0;for(int row=r-1;row<=r+1;row++)if(game_row_active(g,row)&&target_in_lane(g,row,x))target=1;if(target){for(int row=r-1;row<=r+1;row++)if(game_row_active(g,row))shoot(g,row,x+12,0);p->attack=1.5f;p->flash=.3f;}}
    if(k==SPLIT&&p->attack<=0){int target=target_in_lane(g,r,x),back=0;for(int z=0;z<MAX_ZOMBIES;z++)if(g->zombies[z].used&&g->zombies[z].death<=0&&g->zombies[z].row==r&&g->zombies[z].x<x-10)back=1;
        if(target)shoot(g,r,x+12,0);if(back)for(int n=0;n<2;n++){shoot(g,r,x-12-n*6,0);for(int i=0;i<MAX_SHOTS;i++)if(g->shots[i].used&&g->shots[i].x==x-12-n*6)g->shot_vx[i]=-150;}if(target||back){p->attack=1.5f;p->flash=.3f;}}
    if((k==CABBAGE||k==KERNEL||k==MELON)&&p->attack<=0){int target=-1;float nearest=500;
        for(int z=0;z<MAX_ZOMBIES;z++)if(g->zombies[z].used&&!g->hypnotized[z]&&g->zombies[z].death<=0&&g->zombies[z].kind!=Z_BALLOON&&g->zombies[z].row==r&&g->zombies[z].x>=x&&g->zombies[z].x<nearest){target=z;nearest=g->zombies[z].x;}
        if(target>=0||(g->mode==MODE_BOSS&&boss_exposed(g))){lob(g,r,x,k==CABBAGE?4:k==MELON?7:random_next(g)%4==0?6:5,target);p->attack=2.85f;p->flash=.4f;}}
    if(k==CACTUS&&p->attack<=0){int target=target_in_lane(g,r,x);for(int z=0;z<MAX_ZOMBIES;z++)if(g->zombies[z].used&&g->zombies[z].death<=0&&g->zombies[z].row==r&&g->zombies[z].kind==Z_BALLOON)target=1;
        if(target){shoot(g,r,x+12,9);p->attack=1.5f;p->flash=.3f;}}
    if(k==STAR&&p->attack<=0&&game_alive(g)){
        static const float vx[5]={-150,0,0,106,106},vy[5]={0,-150,150,-106,106};
        for(int n=0;n<5;n++){int slot=-1;for(int i=0;i<MAX_SHOTS;i++)if(!g->shots[i].used){slot=i;break;}if(slot>=0){shoot(g,r,x,8);g->shot_vx[slot]=vx[n];g->shot_vy[slot]=vy[n];}}p->attack=1.5f;p->flash=.3f;
    }
    if(k==SQUASH){
        int *phase=&g->burst_left[r][c];float *time=&g->burst[r][c];
        if(!*phase){int target=-1;float nearest=45;for(int z=0;z<MAX_ZOMBIES;z++){Zombie *enemy=&g->zombies[z];float distance=fabsf(enemy->x-x);if(enemy->used&&enemy->death<=0&&!g->hypnotized[z]&&enemy->row==r&&enemy->kind!=Z_BALLOON&&enemy->kind!=Z_BUNGEE&&enemy->vault<=0&&!(enemy->kind==Z_DIGGER&&!enemy->vaulted)&&distance<nearest){target=z;nearest=distance;}}
            if(target>=0){*phase=1;*time=0;g->burst_target[r][c]=target;g->pea_shoot_time[r][c]=g->zombies[target].x;}return;}
        float previous=*time;*time+=dt;
        if(*phase==1&&*time>=.8f){*phase=2;*time=0;}
        else if(*phase==2&&*time>=.45f){int z=g->burst_target[r][c];if(z>=0&&g->zombies[z].used&&g->zombies[z].death<=0)g->pea_shoot_time[r][c]=g->zombies[z].x-4;*phase=3;*time=0;}
        else if(*phase==3&&*time>=.5f){*phase=4;*time=0;}
        else if(*phase==4){if(previous<.05f&&*time>=.05f){float target=g->pea_shoot_time[r][c];for(int j=0;j<MAX_ZOMBIES;j++)if(g->zombies[j].used&&g->zombies[j].death<=0&&!g->hypnotized[j]&&g->zombies[j].kind!=Z_BALLOON&&g->zombies[j].row==r&&fabsf(g->zombies[j].x-target)<30)hurt_zombie(g,&g->zombies[j],1800,0);}
            if(*time>=.1f){*phase=5;*time=0;}}
        else if(*phase==5&&*time>=1){p->kind=-1;*phase=0;}return;
    }
    if(k==KELP){for(int z=0;z<MAX_ZOMBIES;z++){Zombie *enemy=&g->zombies[z];if(enemy->used&&!g->hypnotized[z]&&enemy->death<=0&&enemy->row==r&&fabsf(enemy->x-x)<28&&enemy->vault<=0&&enemy->kind!=Z_BALLOON&&enemy->kind!=Z_BUNGEE&&!(enemy->kind==Z_DIGGER&&!enemy->vaulted)){cue(FOLEY_FLOOP);if(enemy->kind<=Z_DUCK)g->polish.death_style[z]=DEATH_WATER;kill_zombie(g,enemy);p->kind=-1;sound_as(g,SFX_CHOMP,FOLEY_SLURP);cue(FOLEY_ZOMBIESPLASH);return;}}}
    if(k==JALAPENO&&p->age>=1){
        for(int z=0;z<MAX_ZOMBIES;z++)if(g->zombies[z].used&&!g->hypnotized[z]&&g->zombies[z].death<=0&&g->zombies[z].row==r)hurt_charred(g,&g->zombies[z],1800);
        for(int col=0;col<COLS;col++)g->ice_tiles[r][col]=0;
        if(g->mode==MODE_BOSS){if(g->boss_ball_kind==2&&r==g->boss_ball_row)g->boss_ball_kind=0;if(boss_exposed(g))g->boss_hp-=1800;}
        g->flash=.25f;p->kind=-1;sound_as(g,SFX_EXPLODE,FOLEY_JALAPENO_IGNITE);cue(FOLEY_JUICY);return;
    }
    if(k==BLOVER&&p->age>=1){for(int z=0;z<MAX_ZOMBIES;z++)if(g->zombies[z].used&&g->zombies[z].kind==Z_BALLOON)kill_zombie(g,&g->zombies[z]);g->fog_clear=24;p->kind=-1;cue(GAME_SAMPLE(FX_BLOVER));}
    if(k==SPIKEWEED&&p->attack<=0){for(int z=0;z<MAX_ZOMBIES;z++){Zombie *enemy=&g->zombies[z];if(enemy->used&&enemy->death<=0&&enemy->row==r&&fabsf(enemy->x-x)<22&&enemy->kind!=Z_BALLOON){if(enemy->kind==Z_ZOMBONI||enemy->kind==Z_CATAPULT){cue(FOLEY_TIRE_POP);kill_zombie(g,enemy);p->kind=-1;}else hurt_zombie(g,enemy,20,0);}}p->attack=1;}
    if(k==MAGNET&&p->attack<=0){for(int z=0;z<MAX_ZOMBIES;z++){Zombie *enemy=&g->zombies[z];if(enemy->used&&enemy->death<=0&&abs(enemy->row-r)<=2&&fabsf(enemy->x-x)<CELL_W*3&&((enemy->armor>0&&(enemy->kind==Z_BUCKET||enemy->kind==Z_DOOR||enemy->kind==Z_FOOTBALL||enemy->kind==Z_LADDER))||enemy->kind==Z_POGO||enemy->kind==Z_DIGGER||enemy->kind==Z_JACK)){
        enemy->armor=0;enemy->vaulted=1;if(enemy->kind==Z_POGO||enemy->kind==Z_DIGGER||enemy->kind==Z_JACK)enemy->kind=Z_NORMAL;p->attack=15;p->flash=.5f;cue(FOLEY_MAGNETSHROOM);break;
    }}}
    if(k==MARIGOLD&&p->attack<=0){game_add_coin(g,x,game_ground_y(g,r)-15,random_next(g)%10?1:5);p->attack=24;p->flash=1;}
    if(k==ICESHROOM&&p->age>=1&&g->mode==MODE_BOSS){if(g->boss_ball_kind==1)g->boss_ball_kind=0;g->boss_phase=10;g->boss_hp-=20;}
    (void)dt;
}
static void chapter_shots(Game *g,float dt){
    for(int i=0;i<MAX_SHOTS;i++)if(g->shots[i].used){
        Shot *s=&g->shots[i];float old=s->x,oldy=g->shot_y[i],vx=g->shot_vx[i];s->x+=vx*dt;g->shot_y[i]+=g->shot_vy[i]*dt;
        if(s->ice==11){int target=g->shot_target[i];if(target<0||!g->zombies[target].used||g->zombies[target].death>0){target=closest_enemy(g,s->row,s->x,1);g->shot_target[i]=target;}if(target<0){s->used=0;continue;}Zombie *enemy=&g->zombies[target];float ty=game_ground_y(g,enemy->row)+game_roof_offset(g,enemy->x)-18,dx=enemy->x-s->x,dy=ty-g->shot_y[i],distance=hypotf(dx,dy);if(distance<12){if(enemy->kind==Z_BALLOON)enemy->kind=Z_NORMAL;hurt_zombie(g,enemy,20,0);s->used=0;}else{float speed=fminf(285,hypotf(g->shot_vx[i],g->shot_vy[i])+190*dt);g->shot_vx[i]=dx/distance*speed;g->shot_vy[i]=dy/distance*speed;}continue;}
        int lobbed=(s->ice>=4&&s->ice<=7)||s->ice==12;
        if(s->ice==13){g->shot_remaining[i]-=dt;if(g->shot_remaining[i]>0)continue;
            if(g->shot_target[i]==-2){s->x=plant_x((int)g->shot_damage[i]);g->shot_y[i]=game_ground_y(g,s->row)+game_roof_offset(g,s->x)-334;g->shot_vy[i]=380;g->shot_remaining[i]=.88f;g->shot_target[i]=-3;continue;}
            for(int j=0;j<MAX_ZOMBIES;j++)if(g->zombies[j].used&&g->zombies[j].death<=0&&!g->hypnotized[j]&&abs(g->zombies[j].row-s->row)<=1&&fabsf(g->zombies[j].x-s->x)<CELL_W*1.5f)hurt_zombie(g,&g->zombies[j],1800,0);if(g->mode==MODE_BOSS&&boss_exposed(g))g->boss_hp-=1800;g->flash=.25f;sound(g,SFX_EXPLODE);s->used=0;continue;}
        if(lobbed){g->shot_vy[i]+=300*dt;g->shot_remaining[i]-=dt;
            if(g->shot_remaining[i]>0)continue;
            int target=-1;float nearest=25;
            for(int n=0;n<MAX_ZOMBIES;n++){Zombie *candidate=&g->zombies[n];float distance=fabsf(candidate->x-s->x);if(candidate->used&&candidate->death<=0&&!g->hypnotized[n]&&candidate->row==s->row&&candidate->kind!=Z_BALLOON&&candidate->kind!=Z_BUNGEE&&!(candidate->kind==Z_DIGGER&&!candidate->vaulted)&&distance<nearest){target=n;nearest=distance;}}
            if(target>=0){Zombie *z=&g->zombies[target];float armor=z->armor;if(z->kind==Z_DOOR||z->kind==Z_PAPER)z->armor=0;hurt_zombie(g,z,g->shot_damage[i],0);if(z->kind==Z_DOOR||z->kind==Z_PAPER)z->armor=armor;if(s->ice==6&&can_freeze(g,z))g->frozen[target]=4;cue(s->ice==6?FOLEY_BUTTER:s->ice==5?FOLEY_KERNEL_SPLAT:s->ice==7||s->ice==12?FOLEY_MELONIMPACT:FOLEY_SPLAT);
                if(s->ice==12&&can_chill(g,z))z->slow=8;
                if(s->ice==7||s->ice==12)for(int j=0;j<MAX_ZOMBIES;j++)if(j!=target&&g->zombies[j].used&&g->zombies[j].death<=0&&!g->hypnotized[j]&&abs(g->zombies[j].row-z->row)<=1&&fabsf(g->zombies[j].x-z->x)<CELL_W)hurt_zombie(g,&g->zombies[j],26,s->ice==12);
            }else if(g->mode==MODE_BOSS&&boss_exposed(g))g->boss_hp-=g->shot_damage[i];s->used=0;continue;
        }
        if(s->ice==2){g->shot_remaining[i]-=fabsf(vx)*dt;if(g->shot_remaining[i]<=0){s->used=0;continue;}}
        for(int c=0;c<COLS;c++)if(g->plants[s->row][c].kind==TORCHWOOD&&fabsf(s->x-plant_x(c))<8){if(s->ice==0){s->ice=10;g->shot_damage[i]=40;cue(FOLEY_FIREPEA);cue(FOLEY_IGNITE);}else if(s->ice==1)s->ice=0;}
        int target=-1;float closest=10000;
        for(int j=0;j<MAX_ZOMBIES;j++){Zombie *z=&g->zombies[j];
            if(!z->used||g->hypnotized[j]||z->death>0||z->kind==Z_BUNGEE||(z->kind==Z_BALLOON&&s->ice!=9)||(z->kind==Z_SNORKEL&&!z->eating)||(z->kind==Z_DIGGER&&!z->vaulted))continue;
            float zy=game_ground_y(g,z->row)+game_roof_offset(g,z->x)-17;
            if(g->mode!=MODE_HEAVY&&s->ice!=8&&z->row!=s->row)continue;
            
            float dx=s->x-old,dy=g->shot_y[i]-oldy,l=dx*dx+dy*dy,t=l?fmaxf(0,fminf(1,((z->x-8-old)*dx+(zy-oldy)*dy)/l)):0;
            if(hypotf(old+dx*t-(z->x-8),oldy+dy*t-zy)<13&&fabsf(z->x-old)<closest){target=j;closest=fabsf(z->x-old);}
        }
        if(target>=0){Zombie *z=&g->zombies[target];if(z->kind==Z_BALLOON&&s->ice==9){z->kind=Z_NORMAL;z->age=0;cue(GAME_SAMPLE(FX_BALLOON_POP));}hurt_zombie(g,z,g->shot_damage[i],s->ice==1);
            s->used=0;}
        if(game_world(g)==4&&s->ice!=8&&g->shot_y[i]>game_ground_y(g,s->row)+game_roof_offset(g,s->x)-7)s->used=0;
        if(s->x>490||s->x<0||g->shot_y[i]<0||g->shot_y[i]>270)s->used=0;
    }
}
static int chapter_zombie(Game *g,int i,float dt){
    Zombie *z=&g->zombies[i];if(z->death>0)return 0;
    if(g->mode==MODE_BUNGEE&&g->special[i]<0&&z->kind!=Z_BUNGEE){g->special[i]=fminf(0,g->special[i]+dt);z->age+=dt;return 1;}
    if(z->kind==Z_BUNGEE){
        g->special[i]+=dt;z->age+=dt;int col=(int)((z->x-GRID_X)/CELL_W);if(col<0)col=0;if(col>=COLS)col=COLS-1;
        if(g->special[i]>4){if(nearby_umbrella(g,z->row,col))cue(FOLEY_UMBRELLA);if(!nearby_umbrella(g,z->row,col)){destroy_square(g,z->row,col,0);g->pumpkin_hp[z->row][col]=0;}z->used=0;}return 1;
    }
    if(g->frozen[i]>0)return 0;
    if(z->kind==Z_BALLOON){z->x-=game_zombie_walk(z,dt);if(z->x<12)g->status=LOST;return 1;}
    if(z->kind==Z_DIGGER&&!z->vaulted){z->x-=16*dt;z->age+=dt;if(z->x<GRID_X){z->vaulted=1;g->special[i]=2;}return 1;}
    if(z->kind==Z_DIGGER&&z->vaulted){if(g->special[i]>0){g->special[i]-=dt;return 1;}int eating=0;for(int c=0;c<COLS;c++)if(g->plants[z->row][c].kind>=0&&fabsf(z->x-plant_x(c))<18){Plant *p=&g->plants[z->row][c];p->hp-=100*dt*(z->slow>0?.5f:1);if(p->hp<=0)p->kind=-1;eating=1;break;}if(eating!=z->eating){z->age=0;g->polish.pose[i]=0;}z->eating=eating;if(!eating)z->x+=game_zombie_walk(z,dt);else z->age+=dt*(z->slow>0?.5f:1);if(z->x>510)z->used=0;return 1;}
    if(z->kind==Z_JACK){g->special[i]+=dt;if(g->special[i]>12){g->polish.action[i]=ACTION_JACK;g->polish.action_time[i]=0;z->eating=0;cue(GAME_SAMPLE(FX_BOING));return 1;}}
    if(z->kind==Z_ZOMBONI){for(int c=0;c<COLS;c++)if(plant_x(c)>z->x)g->ice_tiles[z->row][c]=30;}
    if(z->kind==Z_CATAPULT&&z->x>360){g->special[i]-=dt;z->age+=dt;if(g->special[i]<=0){g->special[i]=3;cue(FOLEY_BASKETBALL);for(int c=0;c<COLS;c++)if(g->plants[z->row][c].kind>=0){if(nearby_umbrella(g,z->row,c))cue(FOLEY_UMBRELLA);if(!nearby_umbrella(g,z->row,c)){g->plants[z->row][c].hp-=75;if(g->plants[z->row][c].hp<=0)g->plants[z->row][c].kind=-1;}break;}}return 1;}
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
