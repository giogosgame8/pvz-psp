/* Only owned packets participate in chooser navigation. */
static void choose_owned(int direction,int vertical){
 int start=game.selected;if(start<0||start>=ALL_SEEDS)start=0;
 if(!game.unlocked_mask)return;
 if(vertical){int col=start%9,row=start/9;for(int n=1;n<=6;n++){int r=(row+direction*n+12)%6,k=r*9+col;if(k<ALL_SEEDS&&(game.unlocked_mask&(UINT64_C(1)<<k))){game.selected=k;return;}}}
 for(int n=1;n<=ALL_SEEDS;n++){int k=(start+direction*n+ALL_SEEDS*2)%ALL_SEEDS;if(game.unlocked_mask&(UINT64_C(1)<<k)){game.selected=k;return;}}
}
static void normalize_choice(void){if(game.selected<0||game.selected>=ALL_SEEDS||!(game.unlocked_mask&(UINT64_C(1)<<game.selected))){game.selected=ALL_SEEDS-1;choose_owned(1,0);}}
static int soundtrack_context(void){return screen==19?MUSIC_CREDITS:screen==18?MUSIC_MENU:screen==16||screen==17?MUSIC_ZEN:screen==2||screen==3?MUSIC_SEEDS:screen==1?(game.status==WON&&!game.challenge&&game.level==50&&(game.level_flags&LEVEL_REWARD_COLLECTED)?MUSIC_CREDITS:game_music_theme(&game)):MUSIC_MENU;}
