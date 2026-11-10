/* A controller-operated name tab; identity does not alter the game save layout. */
enum { PLAYER_NAME_MAX=16 };
static char player_name[PLAYER_NAME_MAX+1]="Player",name_draft[PLAYER_NAME_MAX+1],name_error[64];
static const char *player_name_path="player-name.dat";
static int player_name_set,name_key,name_lower,name_return,name_mode;
static const char name_letters[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .'-";
static int normalize_name(char out[PLAYER_NAME_MAX+1],const char *value){
 size_t n=strlen(value);if(n>PLAYER_NAME_MAX)return 0;while(n&&value[n-1]==' ')n--;while(n&&*value==' '){value++;n--;}
 if(!n)return 0;for(size_t i=0;i<n;i++)if(!((value[i]>='a'&&value[i]<='z')||(value[i]>='A'&&value[i]<='Z')||(value[i]>='0'&&value[i]<='9')||strchr(" .'-",value[i])))return 0;
 memcpy(out,value,n);out[n]=0;return 1;
}
static int player_name_save(const char *value){
 char normalized[PLAYER_NAME_MAX+1];if(!normalize_name(normalized,value))return 0;
 uint32_t header[2]={0x4e505650,1};char record[PLAYER_NAME_MAX+1]={0},temporary[256];snprintf(record,sizeof(record),"%s",normalized);snprintf(temporary,sizeof(temporary),"%s.tmp",player_name_path);
 FILE *f=fopen(temporary,"wb");if(!f)return 0;int ok=fwrite(header,sizeof(header),1,f)==1&&fwrite(record,sizeof(record),1,f)==1;if(fclose(f))ok=0;
 if(ok){remove(player_name_path);ok=rename(temporary,player_name_path)==0;}if(ok){snprintf(player_name,sizeof(player_name),"%s",normalized);player_name_set=1;}return ok;
}
static int player_name_load(void){
 uint32_t header[2];char record[PLAYER_NAME_MAX+1],normalized[PLAYER_NAME_MAX+1];FILE *f=fopen(player_name_path,"rb");if(!f)return 0;
 int ok=fread(header,sizeof(header),1,f)==1&&header[0]==0x4e505650&&header[1]==1&&fread(record,sizeof(record),1,f)==1&&record[PLAYER_NAME_MAX]==0&&fgetc(f)==EOF;fclose(f);
 if(ok)ok=normalize_name(normalized,record);if(ok){snprintf(player_name,sizeof(player_name),"%s",normalized);player_name_set=1;}return ok;
}
static void name_begin(int destination){name_mode=0;snprintf(name_draft,sizeof(name_draft),"%s",player_name_set?player_name:"");name_error[0]=0;name_return=destination;name_key=0;name_lower=0;screen=23;}
static void secret_begin(void){name_begin(25);name_mode=1;name_draft[0]=0;}
static void name_delete(void){size_t n=strlen(name_draft);if(n)name_draft[n-1]=0;name_error[0]=0;}
static void name_append(char ch){size_t n=strlen(name_draft);if(n<PLAYER_NAME_MAX){name_draft[n]=ch;name_draft[n+1]=0;name_error[0]=0;}else snprintf(name_error,sizeof(name_error),"Maximum %d characters",PLAYER_NAME_MAX);}
static void name_accept(void){if(name_mode){uint32_t old=player.completion.cosmetics;int result=profile_apply_code(&player,name_draft);if(result<=0){snprintf(name_error,sizeof(name_error),"%s",result<0?"Grow the Tree of Wisdom further first.":"Code not recognized.");return;}if(!persist_profile()){player.completion.cosmetics=old;snprintf(name_error,sizeof(name_error),"Could not save the setting.");return;}screen=name_return;notify(result==1?"SECRET EFFECT ENABLED":"SECRET EFFECT DISABLED");return;}char normalized[PLAYER_NAME_MAX+1];if(!normalize_name(normalized,name_draft)){snprintf(name_error,sizeof(name_error),"Please enter your name");return;}if(player_name_save(normalized))screen=name_return;else snprintf(name_error,sizeof(name_error),"Could not save. Please try again.");}
static void name_input(uint32_t pressed){
 if(pressed&PSP_CTRL_CIRCLE){screen=name_return;return;}if(pressed&PSP_CTRL_START){name_accept();return;}if(pressed&PSP_CTRL_SQUARE)name_delete();if(pressed&PSP_CTRL_TRIANGLE)name_lower=!name_lower;
 if(pressed&PSP_CTRL_LEFT)name_key=name_key<40?name_key/8*8+(name_key%8+7)%8:40+(name_key-40+5)%6;
 if(pressed&PSP_CTRL_RIGHT)name_key=name_key<40?name_key/8*8+(name_key%8+1)%8:40+(name_key-40+1)%6;
 if(pressed&PSP_CTRL_UP)name_key=name_key>=40?32+(name_key-40)*8/6:name_key<8?40+name_key*6/8:name_key-8;
 if(pressed&PSP_CTRL_DOWN)name_key=name_key>=40?(name_key-40)*8/6:name_key>=32?40+(name_key%8)*6/8:name_key+8;
 if(pressed&PSP_CTRL_CROSS){if(name_key<40){char ch=name_letters[name_key];if(name_lower&&ch>='A'&&ch<='Z')ch+='a'-'A';name_append(ch);}else if(name_key==40)name_lower=!name_lower;else if(name_key==41)name_append(' ');else if(name_key==42)name_delete();else if(name_key==43){name_draft[0]=0;name_error[0]=0;}else if(name_key==44)name_accept();else screen=name_return;}
}
static void draw_player_name(void){
 render_image(&assets[A_MENU_MAIN],0,0,480,272,can_resume?4:9,WHITE);ui_dialog(46,14,388,244,"");render_image(&assets[A_UI_BUTTON],135,24,210,27,1,WHITE);ui_center(145,28,190,name_mode?"Secret code":"Player name",UI_GOLD,.85f);
 render_rect(80,64,320,26,RGBA(242,230,192,255));render_focus(80,64,320,26,FOCUS_SAGE);char display[24];snprintf(display,sizeof(display),"%s_",name_draft);ui_center(86,67,308,display,UI_INK,.95f);if(name_error[0])ui_center(60,91,360,name_error,UI_GOLD,.51f);
 for(int i=0;i<40;i++){int x=80+i%8*40,y=104+i/8*22;char key[2]={name_letters[i],0};if(name_lower&&key[0]>='A'&&key[0]<='Z')key[0]+='a'-'A';if(key[0]==' ')key[0]='_';render_image(&assets[A_UI_BUTTON],x,y,36,21,i==name_key,i==name_key?UI_ACTIVE_TINT:WHITE);ui_center(x+2,y+1,32,key,i==name_key?UI_FOCUS_TEXT:WHITE,.7f);}
 const char *labels[6]={name_lower?"ABC":"abc","Space","Delete","Clear",name_mode?"Apply":"Save","Cancel"};for(int i=0;i<6;i++){int x=60+i*60;render_image(&assets[A_UI_BUTTON],x,222,58,23,name_key==40+i,name_key==40+i?UI_ACTIVE_TINT:WHITE);ui_center(x+2,225,54,labels[i],name_key==40+i?UI_FOCUS_TEXT:WHITE,.58f);}
}
