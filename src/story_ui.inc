/* Dialogue text and Dave animation come from the player's imported resources. */
static int story_id,story_return,story_phase,story_library,story_index,story_limit;
static float story_time,story_line_time;static char story_key[64],story_raw[2048],story_text[2048];
static int ending_return,ending_paused;static float ending_clock;
static int story_offset,story_end,story_prop;static char story_page[2048];
static float story_token_time(const char *prefix,float fallback){const char *s=strstr(story_raw,prefix);return s?fminf(30,fmaxf(0,atoi(s+strlen(prefix))/100.0f)):fallback;}
static void story_layout(void){
 const char *s=story_text+story_offset;size_t count=strlen(s),lo=0,hi=count;
 while(lo<hi){size_t mid=(lo+hi+1)/2;memcpy(story_page,s,mid);story_page[mid]=0;if(disc_text_height(story_page,290,.8f)<=130)lo=mid;else hi=mid-1;}
 if(lo<count&&lo){size_t word=lo;while(word&&s[word]!=' '&&s[word]!='\n')word--;if(word)lo=word;}
 if(!lo&&count)lo=1;memcpy(story_page,s,lo);story_page[lo]=0;story_end=story_offset+lo;while(story_text[story_end]==' '||story_text[story_end]=='\n')story_end++;
}
#include "story_data.h"
static void story_load(void){
 if(story_library)snprintf(story_key,sizeof(story_key),"%s",story_keys[story_index]);else snprintf(story_key,sizeof(story_key),"CRAZY_DAVE_%d",story_id);
 story_raw[0]=0;textdb_get(story_key,story_raw,sizeof(story_raw));size_t n=0;story_offset=0;story_prop=-1;
 const char *props[]={"{SHOW_WALLNUT}","{SHOW_HAMMER}","{SHOW_FERTILIZER}","{SHOW_TREE_FOOD}","{SHOW_MONEYBAG}"};for(int i=0;i<5;i++)if(strstr(story_raw,props[i]))story_prop=i;
 for(const char *s=story_raw;*s&&n+1<sizeof(story_text);){if(*s=='{'){const char *e=strchr(s,'}');if(!e)break;char token[64],value[64]="";size_t len=e-s+1;if(len>=sizeof(token))len=sizeof(token)-1;memcpy(token,s,len);token[len]=0;
   if(!strcmp(token,"{MONEY}"))snprintf(value,sizeof(value),"$%lu",player.money*10);else if(!strcmp(token,"{UPGRADE_COST}"))snprintf(value,sizeof(value),"$%d",profile_item_cost(&player,SHOP_SLOTS)*10);else if(!strcmp(token,"{PLAYER_NAME}"))snprintf(value,sizeof(value),"%s",player_name);
   for(char *v=value;*v&&n+1<sizeof(story_text);v++)story_text[n++]=*v;s=e+1;continue;}
  if(*s=='<'&&strchr(s,'>')){const char *e=strchr(s,'>');if(e-s<10){if(n+3<sizeof(story_text)){story_text[n++]='(';story_text[n++]='X';story_text[n++]=')';}s=e+1;continue;}}
  if(*s!='\r')story_text[n++]=*s;s++;
 }story_text[n]=0;story_line_time=0;if(!strstr(story_raw,"{NO_SOUND}"))foley_play(strstr(story_raw,"{SCREAM2}")?FOLEY_CRAZY_DAVE_SCREAM_2:strstr(story_raw,"{SCREAM}")?FOLEY_CRAZY_DAVE_SCREAM:strstr(story_raw,"{SHAKE}")?FOLEY_CRAZY_DAVE_CRAZY:strstr(story_raw,"{SHORT_SOUND}")?FOLEY_CRAZY_DAVE_SHORT:strlen(story_text)>160?FOLEY_CRAZY_DAVE_EXTRA_LONG:strlen(story_text)>80?FOLEY_CRAZY_DAVE_LONG:FOLEY_CRAZY_DAVE_SHORT);
}
static int story_begin(int id,int destination,int limit,int library){
 char key[64],line[8];snprintf(key,sizeof(key),"CRAZY_DAVE_%d",id);if(!library&&!textdb_get(key,line,sizeof(line)))return 0;
 story_id=id;story_return=destination;story_limit=limit;story_library=library;story_phase=0;story_time=0;story_index=0;screen=18;story_load();return 1;
}
static void story_leave(void){story_phase=2;story_time=0;}
static void story_step(float dt){story_time+=dt;story_line_time+=dt;if(story_phase==0&&story_time>.75f){story_phase=1;story_time=0;}if(story_phase==2&&story_time>(story_return==1&&game.level==50?2.4f:.75f)){screen=story_return;if(screen==1){game.level_flags|=LEVEL_READY;warm_level();}story_phase=0;}}
static int story_level_id(void){
 if(game.extra.category==EXTRA_IZOMBIE)return game.extra.selection==0?2200:0;
 if(game.extra.category==EXTRA_VASE)return game.extra.selection==0?3000:0;
 if(game.challenge)return 0;
 if(game.completion.adventure_run){switch(game.level){case 1:return 1601;case 5:return 2411;case 15:return 401;case 25:return 701;case 35:return 2500;case 40:return 1101;case 45:return 1304;case 50:return 2300;default:return 0;}}
 switch(game.level){case 5:return game.shovel_tutorial?2400:2408;case 11:return 201;case 12:return 1401;case 15:return 401;case 21:return 501;case 25:return 701;case 31:return 801;case 35:return 2500;case 40:return 1101;case 41:return 1201;case 45:return 1301;case 50:return 2300;default:break;}
 if(game.level>=13&&game.level<=24&&player.purchases[SHOP_SLOTS]<2&&player.money>=(uint32_t)profile_item_cost(&player,SHOP_SLOTS))return player.purchases[SHOP_SLOTS]?1551:1501;return 0;
}
static void draw_story(void){
 render_background(&assets[story_return==16?A_GARDEN_BG:story_return==17?A_TREE_STAGES:story_return==9?A_STORE_BACKGROUND:board_background()]);
 int talk=strstr(story_raw,"{SHAKE}")?A_DAVE_CRAZY:strstr(story_raw,"{SCREAM}")||strlen(story_text)<23?A_DAVE_SMALLTALK:strlen(story_text)<52?A_DAVE_TALK:A_DAVE_BLAHBLAH;
 int talking=story_line_time<story_token_time("{TIME_",(assets[talk].frames-1)/12.0f);
 int anim=story_phase==0?A_DAVE_ENTER:story_phase==2?(story_return==1&&game.level==50?A_DAVE_GRAB:A_DAVE_LEAVE):talking?talk:A_DAVE_IDLE;
 if(story_prop>=0&&story_phase!=2)anim=A_DAVE_WALLNUT_ENTER+story_prop*3+(story_phase==0?0:talking?2:1);
 float rate=story_phase==1?12:story_phase==2&&game.level==50&&story_return==1?18:24;
 render_anim(&assets[anim],91,263,(story_phase==1?story_line_time:story_time)*rate/fmaxf(1,assets[anim].fps),1,WHITE,story_phase==1&&(story_prop>=0||!talking));
 if(story_phase!=2){render_image(&assets[A_SHOP_BUBBLE],158,43,312,181,0,WHITE);story_layout();disc_text(169,54,290,story_page,RGBA(42,33,22,255),.8f);}
 ui_hint(story_library?"L/R: Passage   X: Next   O: Back":story_id==1503||story_id==1553?"X: Buy seed slot   Square: Decline":"X: Continue   O: Skip scene");
}
static void ending_finish(void){movie_close();player.garden.credits_seen=1;persist_profile();screen=ending_return;ending_paused=0;}
static int ending_begin(int destination){animation_cache_suspend();if(!movie_open("data/ending.pvj")){notify("ENDING DATA COULD NOT BE OPENED");return 0;}ending_return=destination;ending_paused=0;ending_clock=0;screen=19;audio_restart_music(MUSIC_CREDITS);return 1;}
static void ending_step(float dt){if(ending_paused)return;float t=audio_music_seconds(MUSIC_CREDITS);if(t>=0)ending_clock=t;else ending_clock+=dt;movie_update(ending_clock);if(movie_error()){ending_finish();notify("ENDING FRAME COULD NOT BE READ");}else if(ending_clock>movie_duration()+.5f)ending_finish();}
static int story_input(uint32_t pressed){
 if(screen==19){if(pressed&PSP_CTRL_START)ending_paused=!ending_paused;if(pressed&PSP_CTRL_CIRCLE)ending_finish();return 1;}
 if(screen!=18)return 0;
 if(pressed&PSP_CTRL_CIRCLE){story_leave();return 1;}if(story_phase!=1||story_line_time<fmaxf(.25f,story_token_time("{DELAY_",.25f)))return 1;
 if(pressed&PSP_CTRL_CROSS){story_layout();if(story_text[story_end]){story_offset=story_end;return 1;}}
 if(story_library&&(pressed&(PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_CROSS))){story_index=(story_index+STORY_LINES+((pressed&PSP_CTRL_LTRIGGER)?-1:1))%STORY_LINES;story_load();return 1;}
 if(story_id==1503||story_id==1553){if(pressed&(PSP_CTRL_CROSS|PSP_CTRL_SQUARE)){int yes=(pressed&PSP_CTRL_CROSS)&&profile_purchase(&player,SHOP_SLOTS);story_id=(story_id==1503?1500:1550)+(yes?10:20);profile_apply(&player,&game);persist_profile();story_load();}return 1;}
 if(pressed&PSP_CTRL_CROSS){if(story_limit&&story_id>=story_limit){story_leave();return 1;}story_id++;char key[64],text[8];snprintf(key,sizeof(key),"CRAZY_DAVE_%d",story_id);if(!textdb_get(key,text,sizeof(text)))story_leave();else story_load();}return 1;
}
