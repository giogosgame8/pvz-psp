/* BLUS30852 challenge table and unlock arithmetic (ELF VA 0x1bc2b8). */
static int puzzle_family;
static uint32_t challenge_seen[4];static int unlock_family=-1,unlock_index=-1;static float unlock_time;
static int bits32(uint32_t n){int k=0;while(n){n&=n-1;k++;}return k;}
static int challenge_gate(int family){return player.adventure_finished||!!(player.garden.story_seen&(family==0?CHALLENGE_MINI:family==1?CHALLENGE_SURVIVAL:CHALLENGE_PUZZLE));}
static uint32_t challenge_wins(int family){return family==0?player.mini_trophies:family==1?player.garden.survival_wins&1023:(player.garden.puzzle_wins>>((family-2)*10))&1023;}
static int challenge_available(int family,int index){
 if(family<0||family>3||index<0||index>=(family==0?20:family==1?11:10))return 0;
 uint32_t wins=challenge_wins(family);int legacy_index=family==1&&index==10?12:index;
 if((family==1?player.garden.survival_wins:wins)&(1u<<legacy_index))return 1; /* retain earned legacy access */
 if(!challenge_gate(family))return 0;
 int count=bits32(wins);
 if(family>=2)return index<=count&&(index<3||player.adventure_finished);
 if(!player.adventure_finished)return index<3;
 return family==1&&index==10?count>=10:index<count+3;
}
static uint32_t challenge_mask(int family){uint32_t mask=0;for(int n=0;n<(family==0?20:family==1?11:10);n++)if(challenge_available(family,n))mask|=1u<<n;return mask;}
static void challenge_seen_save(void){uint32_t data[6]={0x184c4843,1,challenge_seen[0],challenge_seen[1],challenge_seen[2],challenge_seen[3]};FILE *f=fopen("challenge-view.dat.tmp","wb");int ok=f&&fwrite(data,sizeof(data),1,f)==1;if(f&&fclose(f))ok=0;if(ok){remove("challenge-view.dat");rename("challenge-view.dat.tmp","challenge-view.dat");}}
static void challenge_initialize(void){
 if(!(player.garden.story_seen&CHALLENGE_MIGRATED)){
  if(player.highest>=23||player.mini_trophies)player.garden.story_seen|=CHALLENGE_MINI;
  if(player.highest>=37||player.garden.puzzle_wins)player.garden.story_seen|=CHALLENGE_PUZZLE;
  if(player.adventure_finished||player.garden.survival_wins)player.garden.story_seen|=CHALLENGE_SURVIVAL;
  player.garden.story_seen|=CHALLENGE_MIGRATED;
 }
 uint32_t d[6];FILE *f=fopen("challenge-view.dat","rb");int ok=f&&fread(d,sizeof(d),1,f)==1&&d[0]==0x184c4843&&d[1]==1;if(f)fclose(f);
 for(int n=0;n<4;n++)challenge_seen[n]=ok?d[n+2]:challenge_mask(n);
}
static int challenge_enter(int family){if(challenge_gate(family)||challenge_wins(family))return 1;notify(family==0?"Find the Mini-games present in level 3-2.":family==1?"Finish Adventure to unlock Survival.":"Find the Puzzle present in level 4-6.");foley_sample(FX_BUZZER);return 0;}
static int challenge_launch(int family,int index){if(unlock_family==family&&unlock_index==index&&unlock_time<1.5f)return 0;if(challenge_available(family,index))return 1;notify(!player.adventure_finished&&index>=3?"Finish Adventure to unlock more challenges.":family==1&&index==10?"Earn all 10 Survival trophies first.":family>=2?"Complete the preceding puzzle to unlock this one.":"Earn more trophies to unlock this challenge.");foley_sample(FX_BUZZER);return 0;}
static void challenge_lock(int x,int y,int w,int h,int family,int index){
 int opening=family==unlock_family&&index==unlock_index;
 if(challenge_available(family,index)&&!opening)return;
 float shake=opening&&unlock_time<1?2*sinf(unlock_time*90):0;uint32_t tint=opening&&unlock_time>=1?RGBA(255,255,255,(int)(255*fmaxf(0,1-(unlock_time-1)*2))):WHITE;
 render_image(&assets[opening&&unlock_time>=1?A_CHALLENGE_OPEN:A_CHALLENGE_LOCK],x+w*.5f-11+shake,y+h*.35f-12,22,28,0,tint);
}
static void challenge_step(float dt){
 int family=screen==7?0:screen==14?1:screen==15?2+puzzle_family:-1;
 if(unlock_family>=0&&unlock_family!=family){unlock_family=unlock_index=-1;unlock_time=0;}
 if(family<0)return;
 if(unlock_family<0){uint32_t fresh=challenge_mask(family)&~challenge_seen[family];if(fresh){unlock_family=family;unlock_index=0;while(!(fresh&(1u<<unlock_index)))unlock_index++;unlock_time=0;foley_play(FOLEY_PAPER);}}
 if(unlock_family>=0){unlock_time+=dt;if(unlock_time>=1.5f){challenge_seen[family]|=1u<<unlock_index;challenge_seen_save();unlock_family=unlock_index=-1;}}
}

static int challenge_notice=-1;
static void draw_board(void);static void draw_hud(float);
static void challenge_notice_draw(void){draw_board();draw_hud(60);render_rect(0,0,480,272,RGBA(0,0,0,120));ui_dialog(82,58,316,166,"New mode unlocked!");render_anim(&assets[A_PRESENT],240,133,0,1,WHITE,0);ui_center(100,143,280,challenge_notice==0?"Mini-games unlocked!":"Puzzle mode unlocked!",UI_GOLD,.92f);ui_button(139,180,202,"Continue",1);}
