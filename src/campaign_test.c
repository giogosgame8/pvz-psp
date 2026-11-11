#include "game.h"
#include "profile.h"
#include <string.h>
#include <math.h>
/* Combat fixtures exercise every wave using real projectiles, zombie updates,
   terrain and win conditions. They are deliberately well defended fixtures,
   not a claim that a human has balanced or completed all 50 levels. */
static void fixture_defense(Game *g){
    /* These late roof defenses include the cleaner purchasable from 5-2. */
    if(game_world(g)==4&&g->level>=42)g->store_flags|=STORE_ROOF;
    for(int r=0;r<game_rows(g);r++)if(game_row_active(g,r))for(int c=0;c<COLS;c++){
        if(g->vases[r][c])continue;
        int k=game_world(g)==4?(g->level==50?MELON:CABBAGE):g->level>=9?REPEATER:PEA;
        if(c==0&&g->level>=36&&game_world(g)!=4)k=SPLIT;
        if(c==6&&g->level>=33&&game_world(g)!=4)k=CACTUS;
        if(game_world(g)==3&&g->level>=39&&(c==1||c==4))k=MAGNET;
        if(c==8)k=g->level>=29?TALLNUT:WALLNUT;
        if(game_world(g)==4&&(c==1||c==4||c==7)&&g->level>=44)k=UMBRELLA;
        if(game_world(g)==4&&c==5&&g->level>=49)k=MELON;
        if(g->level>=37)g->pumpkin_hp[r][c]=4000;
        g->plants[r][c]=(Plant){.kind=k,.hp=k==TALLNUT?8000:k==WALLNUT?4000:300,.attack=.3f};g->asleep[r][c]=0;
        if(game_water_row(g,r)||game_world(g)==4){g->support[r][c]=(game_water_row(g,r)?LILY:POT)+1;g->support_hp[r][c]=300;}
    }
}
int game_campaign_selftest(FILE *out){
    Game g;int passed=0;unsigned int ticks=0;
    fprintf(out,"CAMPAIGN COMBAT: preplanted defense fixtures, 100 ms simulation steps; conveyor packet actions; no forced zombie deaths or win states\n");fflush(out);
    for(int level=1;level<=50;level++){
        game_start_level(&g,level,0x70000u+level);
        if(g.shovel_tutorial)for(int c=0;c<3;c++)game_shovel(&g,2,c);
        if(g.mode!=MODE_BOWLING&&g.mode!=MODE_WHACK)fixture_defense(&g);
        int limit=18000,last_stage=0;
        for(int n=0;n<limit&&g.status==PLAYING;n++,ticks++){
            if(g.mode==MODE_WHACK){for(int z=0;z<MAX_ZOMBIES;z++)if(g.zombies[z].used&&g.zombies[z].death<=0){int col=(int)((g.zombies[z].x-GRID_X)/CELL_W);game_whack(&g,g.zombies[z].row,col);}}
            if(g.mode==MODE_BOWLING&&g.packet_count){int row=2;float closest=1000;for(int z=0;z<MAX_ZOMBIES;z++)if(g.zombies[z].used&&g.zombies[z].death<=0&&g.zombies[z].x<closest){closest=g.zombies[z].x;row=g.zombies[z].row;}game_place(&g,row,2,game_selected_kind(&g));}
            if(g.mode==MODE_VASE){
                if(g.stage!=last_stage){fixture_defense(&g);last_stage=g.stage;}
                if(g.packet_count>=8){int r=(n/3)%5,c=0;game_shovel(&g,r,c);game_place(&g,r,c,game_selected_kind(&g));}
                if(n%8==0){int done=0;for(int c=8;c>=0&&!done;c--)for(int r=0;r<5&&!done;r++)if(g.vases[r][c])done=game_break_vase(&g,r,c);}
            }
            if((g.mode==MODE_BUNGEE||g.mode==MODE_CONVEYOR||g.mode==MODE_BOSS)&&g.packet_count){
                /* Spend actual boss conveyor attacks during the vulnerable
                   head window instead of leaving them in the tray forever. */
                if(g.mode==MODE_BOSS&&g.extra.boss_action>=B_HEAD_WAIT&&g.extra.boss_action<=B_HEAD_AFTER){
                    for(int p=0;p<g.packet_count;p++)if(g.packets[p]==JALAPENO){int row=g.boss_row,col=8;g.selected=p;game_shovel(&g,row,col);game_place(&g,row,col,JALAPENO);break;}
                }
                int row=-1;float closest=300;for(int z=0;z<MAX_ZOMBIES;z++)if(g.zombies[z].used&&g.zombies[z].death<=0&&g.zombies[z].kind!=Z_BUNGEE&&g.zombies[z].x<closest){row=g.zombies[z].row;closest=g.zombies[z].x;}
                if(row>=0)for(int p=0;p<g.packet_count;p++)if(g.packets[p]==JALAPENO||g.packets[p]==CHERRY){int kind=g.packets[p],col=kind==JALAPENO?2:(int)((closest-GRID_X)/CELL_W);if(col<0)col=0;if(col>=COLS)col=COLS-1;g.selected=p;game_shovel(&g,row,col);if(game_place(&g,row,col,kind)==0)break;}
                for(int p=0;p<g.packet_count;p++){
                    int kind=g.packets[p];if(kind==JALAPENO||kind==CHERRY||kind==DOOM||kind==ICESHROOM)continue;
                    int placed=0;for(int c=0;c<COLS&&!placed;c++)for(int r=0;r<game_rows(&g)&&!placed;r++)if(g.plants[r][c].kind<0){g.selected=p;placed=game_place(&g,r,c,kind)==0;}
                    if(placed)break;
                }
                if(g.selected>=g.packet_count)g.selected=g.packet_count?g.packet_count-1:0;
            }
            if(g.mode==MODE_BOSS&&g.boss_ball_kind&&g.boss_ball_x>250){
                int r=g.boss_ball_row,c=6;g.support[r][c]=POT+1;g.support_hp[r][c]=300;
                if(g.plants[r][c].kind<0)g.plants[r][c]=(Plant){.kind=g.boss_ball_kind==1?ICESHROOM:JALAPENO,.hp=300};
            }
            /* Skip empty waiting time, leaving all live-enemy countdowns intact. */
            if(!game_alive(&g)&&g.mode!=MODE_VASE&&g.mode!=MODE_BOSS){g.spawn_timer=fminf(g.spawn_timer,.1f);if(g.huge_wave_timer>0)g.huge_wave_timer=fminf(g.huge_wave_timer,.1f);}
            game_collect(&g);game_step(&g,.1f);
        }
        int ok=g.status==WON;passed+=ok;
        if(!ok){for(int z=0;z<MAX_ZOMBIES;z++)if(g.zombies[z].used&&g.zombies[z].death<=0)fprintf(out,"  threat kind %d row %d x %.0f hp %.0f\n",g.zombies[z].kind,g.zombies[z].row,g.zombies[z].x,g.zombies[z].hp);}
        fprintf(out,"combat %d-%d: %s (wave %d/%d, kills %d, simulated %.1f s, boss %.0f)\n",(level-1)/10+1,(level-1)%10+1,ok?"PASS":"FAIL",g.wave,g.num_waves,g.kills,g.time,g.boss_hp);fflush(out);
    }
    fprintf(out,"CAMPAIGN COMBAT RESULT %d/50 (%u simulation steps)\n",passed,ticks);fflush(out);return passed==50;
}
