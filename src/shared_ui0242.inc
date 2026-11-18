/* Decode the four pause-menu frames once, before accepting normal input.
   Other small reward/packet frames arrive one per frame after async reads. */
static const int shared_art[]={A_DISC_FONT,A_UI_DIALOG,A_UI_BUTTON,A_UI_BUTTON,A_AWARD_GLOW,A_AWARD_RAYS,A_AWARD_ARROW,A_REWARD_TROPHY,A_REWARD_SHOVEL,A_REWARD_ALMANAC,A_REWARD_KEYS,A_REWARD_TACO,A_REWARD_WATERING_CAN,A_REWARD_NOTE,A_REWARD_SILVER_SUNFLOWER,A_REWARD_MONEYBAG};
static int shared_cursor;
static int shared_frame(int i){return i==3?1:i>=(int)(sizeof(shared_art)/sizeof(*shared_art))?i-sizeof(shared_art)/sizeof(*shared_art):0;}
static int shared_asset(int i){return i<(int)(sizeof(shared_art)/sizeof(*shared_art))?shared_art[i]:A_SEED_CARDS;}
static int shared_count(void){return sizeof(shared_art)/sizeof(*shared_art)+ALL_SEEDS;}
static void shared_ui_step(int immediate){
 if(!ready||screen==19)return;
 if(shared_cursor>=shared_count())shared_cursor=0;
 int budget=immediate?4:1;
 for(int n=0;n<shared_count()&&budget>0;n++){
  int at=shared_cursor,id=shared_asset(at),frame=shared_frame(at);Animation *a=&assets[id];
  if(animation_frame_retained(a,frame)){shared_cursor=(shared_cursor+1)%shared_count();continue;}
  if(!immediate&&!animation_bank_ready(a)){animation_prefetch(a);return;}
  if(!animation_retain_frame(a,frame))return;
  shared_cursor=(shared_cursor+1)%shared_count();budget--;
 }
}
