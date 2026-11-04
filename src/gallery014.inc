enum { GALLERY_ZOMBIES=Z_TYPES+6,GALLERY_PAGE=8 };
static int gallery_tab,gallery_focus,gallery_cursor[2],gallery_pose;
static Animation gallery_thumbnails[2];
static const char *gallery_zombie_names[GALLERY_ZOMBIES]={"Zombie","Conehead","Buckethead","Pole Vaulter","Flag Zombie","Newspaper","Screen Door","Football","Dancing Zombie","Backup Dancer","Ducky Tube","Snorkel","Zomboni","Bobsled Team","Dolphin Rider","Jack-in-the-Box","Balloon","Digger","Pogo","Yeti","Bungee","Ladder","Catapult","Gargantuar","Imp","Dr. Zomboss","Peashooter Zombie","Wall-nut Zombie","Jalapeno Zombie","Gatling Zombie","Squash Zombie","Tall-nut Zombie"};
static int gallery_count(void){return gallery_tab?GALLERY_ZOMBIES:ALL_SEEDS;}
static void gallery_open(void){
 if(!gallery_thumbnails[0].loaded)animation_load(&gallery_thumbnails[0],"data/gallery_plants.pva");
 if(!gallery_thumbnails[1].loaded)animation_load(&gallery_thumbnails[1],"data/gallery_zombies.pva");
 gallery=1;gallery_tab=gallery_focus=gallery_cursor[0]=gallery_cursor[1]=gallery_pose=0;
}
static const Animation *gallery_plant(int kind,int pose,const char **label){
 *label="Idle";int id=seed_icons[kind];if(!pose)return &assets[id];*label="Action";
 switch(kind){
 case PEA:id=A_PEA_SHOOT;break;case SNOW:id=A_SNOW_SHOOT;break;case REPEATER:id=A_REPEATER_SHOOT;break;
 case SUNFLOWER:*label="Idle";break;case WALLNUT:*label=pose==1?"Damaged":"Very damaged";return &damaged[A_WALLNUT][pose-1];
 case CHERRY:id=A_CHERRY_EXPLODE;break;case MINE:id=A_MINE_UNARMED;*label="Unarmed";break;
 case CHOMPER:id=pose==1?A_CHOMPER_BITE:A_CHOMPER_CHEW;*label=pose==1?"Biting":"Chewing";break;
 case PUFF:id=A_PUFF_SHOOT;break;case SUNSHROOM:id=A_SUNSHROOM_BIG;*label="Grown";break;case FUME:id=A_FUME_SHOOT;break;
 case SCAREDY:id=pose==1?A_SCAREDY_SHOOT:A_SCAREDY_HIDE;*label=pose==1?"Shooting":"Hiding";break;
 case SQUASH:id=pose==1?A_SQUASH_UP:A_SQUASH_DOWN;*label=pose==1?"Jumping":"Landing";break;
 case GATLING:id=A_GATLING_SHOOT;break;case GLOOM:id=A_GLOOM_SHOOT;break;case CATTAIL:id=A_CATTAIL_SHOOT;break;case WINTER:id=A_WINTER_SHOOT;break;
 case COB:id=pose==1?A_COB_SHOOT:A_COB_UNARMED;*label=pose==1?"Firing":"Recharging";break;case IMITATER:id=A_IMITATER_MORPH;*label="Transforming";break;
 default:*label="Idle";break;
 }return &assets[id];
}
static int gallery_poses(int tab,int index){
 if(tab)return 3;
 switch(index){case WALLNUT:case CHOMPER:case SCAREDY:case SQUASH:case COB:return 3;
 case PEA:case SNOW:case REPEATER:case CHERRY:case MINE:case PUFF:case SUNSHROOM:case FUME:case GATLING:case GLOOM:case CATTAIL:case WINTER:case IMITATER:return 2;default:return 1;}
}
static const Animation *gallery_zombie(int kind,int pose){
 if(kind>=Z_TYPES)return &assets[pose==2?A_ZOMBIE_DEATH:pose?A_BOT_EAT:A_BOT_WALK];
 if(kind>=Z_DUCK)return &assets[A_DUCK_WALK+(kind-Z_DUCK)*3+pose];
 if(kind==Z_NORMAL||kind==Z_CONE||kind==Z_BUCKET)return &assets[pose==2?A_ZOMBIE_DEATH:kind==Z_CONE?(pose?A_CONE_EAT:A_CONE):kind==Z_BUCKET?(pose?A_BUCKET_EAT:A_BUCKET):(pose?A_ZOMBIE_EAT:A_ZOMBIE)];
 switch(kind){case Z_POLE:return &assets[pose==2?A_POLE_DEATH:pose?A_POLE_EAT:A_POLE_RUN];case Z_FLAG:return &assets[pose==2?A_ZOMBIE_DEATH:pose?A_FLAG_EAT:A_FLAG_WALK];case Z_PAPER:return &assets[A_PAPER+pose];case Z_DOOR:return &assets[pose==2?A_ZOMBIE_DEATH:pose?A_DOOR_EAT:A_DOOR];case Z_FOOTBALL:return &assets[A_FOOTBALL+pose];case Z_DANCER:return &assets[A_DANCER+pose];default:return &assets[A_BACKUP+pose];}
}
static const Animation *gallery_head(int index){return &assets[(int[]){A_PEA_HEAD,A_WALLNUT,A_JALAPENO,A_GATLING,A_SQUASH,A_TALLNUT}[index-Z_TYPES]];}
static void gallery_schedule(void){
 const Animation *thumb=&gallery_thumbnails[gallery_tab];
 if(!animation_prefetch(thumb))return;
 int selected=gallery_cursor[gallery_tab],first=selected/GALLERY_PAGE*GALLERY_PAGE,count=gallery_count();
 for(int slot=-1;slot<GALLERY_PAGE;slot++){
  int index=slot<0?selected:first+slot;if(index>=count||(slot>=0&&index==selected))continue;
  int pose=index==selected?gallery_pose:0;const char *label;
  animation_prefetch(gallery_tab?gallery_zombie(index,pose):gallery_plant(index,pose,&label));
  if(gallery_tab&&index>=Z_TYPES&&pose!=2)animation_prefetch(gallery_head(index));
 }
}
static void gallery_sprite(int tab,int index,int pose,float x,float y,float width,float height,float clock){
 const char *label;const Animation *a=tab?gallery_zombie(index,pose):gallery_plant(index,pose,&label);const Animation *head=0;float hs=1,offset=0;
 if(tab&&index>=Z_TYPES&&pose!=2)head=gallery_head(index);
 if(!animation_bank_ready(a)||(head&&!animation_bank_ready(head))){
  if(animation_bank_ready(&gallery_thumbnails[tab]))render_image(&gallery_thumbnails[tab],x,y,width,height,index,WHITE);
  return;
 }
 float left=-a->anchor_x,top=-a->anchor_y,right=a->width-a->anchor_x,bottom=a->height-a->anchor_y;
 if(head){int h=index-Z_TYPES;hs=h==0?.84f:.56f;offset=h==0?9.7f:23.7f;
  left=fminf(left,-3.2f-(head->width-head->anchor_x)*hs);right=fmaxf(right,-3.2f+head->anchor_x*hs);top=fminf(top,-offset-head->anchor_y*hs);bottom=fmaxf(bottom,-offset+(head->height-head->anchor_y)*hs);
 }
 float scale=fminf(.93f,fminf(width/fmaxf(1,right-left),height/fmaxf(1,bottom-top))),cx=x+width*.5f-(left+right)*scale*.5f,foot=y+height*.5f-(top+bottom)*scale*.5f;
 float duration=a->frames/fmaxf(1,a->fps),seconds=clock;if((tab&&pose==2)||(!tab&&pose)){float period=duration+.65f;seconds=fminf(duration-.001f,fmodf(clock,period));}
 render_anim(a,cx,foot,seconds,scale,WHITE,!(tab&&pose==2));if(head)render_anim_mirrored(head,cx-3.2f*scale,foot-offset*scale,clock,hs*scale,WHITE,1);
}
static void gallery_input(uint32_t pressed){
 if(pressed&PSP_CTRL_CIRCLE){gallery=0;return;}
 int before=gallery_cursor[gallery_tab];
 if(pressed&(PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER)){gallery_tab^=1;gallery_pose=0;}
 if(!gallery_focus){if(pressed&(PSP_CTRL_LEFT|PSP_CTRL_RIGHT)){gallery_tab^=1;gallery_pose=0;}if(pressed&(PSP_CTRL_DOWN|PSP_CTRL_CROSS))gallery_focus=1;return;}
 int *cursor=&gallery_cursor[gallery_tab],count=gallery_count();
 if(pressed&PSP_CTRL_LEFT)*cursor=(*cursor+count-1)%count;
 if(pressed&PSP_CTRL_RIGHT)*cursor=(*cursor+1)%count;
 if(pressed&PSP_CTRL_UP){if(*cursor%GALLERY_PAGE<4)gallery_focus=0;else *cursor-=4;}
 if(pressed&PSP_CTRL_DOWN)*cursor=(*cursor+4)%count;
 if(*cursor!=before)gallery_pose=0;
 if(pressed&PSP_CTRL_CROSS)gallery_pose=(gallery_pose+1)%gallery_poses(gallery_tab,*cursor);
}
static void gallery_label(int x,int y,const char *name){
 render_clip(x+12,y+55,79,24);
 if(disc_text_width(name,.54f)<=79||!strchr(name,' '))ui_center(x+12,y+60,79,name,UI_INK,.54f);
 else{char first[96],second[96];size_t split=0;float best=10000;for(size_t i=0;name[i];i++)if(name[i]==' '){memcpy(first,name,i);first[i]=0;float width=fmaxf(disc_text_width(first,.42f),disc_text_width(name+i+1,.42f));if(width<best){best=width;split=i;}}
  memcpy(first,name,split);first[split]=0;snprintf(second,sizeof(second),"%s",name+split+1);float line=disc_text_height("A",79,.42f);ui_center(x+12,y+56,79,first,UI_INK,.42f);ui_center(x+12,y+56+line,79,second,UI_INK,.42f);
 }
 render_clip(0,0,480,272);
}
static void draw_gallery(float clock){
 gallery_schedule();
 render_background(&assets[A_MINI_BACKGROUND]);ui_button(82,8,151,"Plants",gallery_tab==0);ui_button(247,8,151,"Zombies",gallery_tab==1);if(!gallery_focus)render_focus(gallery_tab?246:81,7,153,27,FOCUS_SAGE);
 int count=gallery_count(),selected=gallery_cursor[gallery_tab],first=selected/GALLERY_PAGE*GALLERY_PAGE;char caption[128];const char *action=gallery_tab?(gallery_pose==0?"Walking":gallery_pose==1?"Eating":"Death"):"Idle";if(!gallery_tab)gallery_plant(selected,gallery_pose,&action);
 snprintf(caption,sizeof(caption),"%s %d-%d/%d   %s",gallery_tab?"Zombies":"Plants",first+1,first+8<count?first+8:count,count,action);ui_center(120,37,240,caption,UI_GOLD,.48f);
 for(int i=first;i<first+8&&i<count;i++){int slot=i-first,x=13+slot%4*117,y=64+slot/4*92;render_image(&assets[A_CHALLENGE_FRAMES],x,y,103,86,gallery_focus&&selected==i,gallery_focus&&selected==i?UI_ACTIVE_TINT:WHITE);render_clip(x+5,y+4,93,77);gallery_sprite(gallery_tab,i,selected==i?gallery_pose:0,x+8,y+5,87,54,clock);
  char name[96];if(gallery_tab)snprintf(name,sizeof(name),"%s",gallery_zombie_names[i]);else ui_title_case(name,sizeof(name),full_seed_names[i]);gallery_label(x,y,name);if(gallery_focus&&selected==i)render_focus(x-1,y-1,105,88,FOCUS_SAGE);
 }
}
static float board_sunflower_glow(const Plant *p){return game.mode==MODE_IZOMBIE||game.extra.category==EXTRA_IZOMBIE?0:game_sunflower_glow(p);}
static int credits_cursor,pause_cursor;
static void draw_pause_menu(void){ui_dialog(104,59,272,162,"Paused");ui_button(129,97,222,"Continue",pause_cursor==0);ui_button(129,132,222,"Main menu",pause_cursor==1);ui_button(129,167,222,"How to play",pause_cursor==2);}
static void pause_input(uint32_t pressed){
 if(pressed&(PSP_CTRL_UP|PSP_CTRL_LEFT))pause_cursor=(pause_cursor+2)%3;if(pressed&(PSP_CTRL_DOWN|PSP_CTRL_RIGHT))pause_cursor=(pause_cursor+1)%3;
 if(pressed&PSP_CTRL_TRIANGLE){screen=0;paused=0;}if(pressed&PSP_CTRL_SQUARE)help=1;
 if(pressed&PSP_CTRL_CROSS){if(pause_cursor==0)paused=0;else if(pause_cursor==1){screen=0;paused=0;}else help=1;}
}
