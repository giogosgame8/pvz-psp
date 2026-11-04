/* An optional PSP customizer composed from the owner's PS3 zombie parts. */
typedef struct {uint32_t face,hat,wear,mustache,enabled;} Avatar;
static Avatar avatar={0,1,0,0,0},avatar_draft;
static int avatar_row,avatar_loaded,player_hub_row;
static const char *avatar_path="custom-zombie.dat";
static char avatar_error[80];
enum { AV_BODY,AV_WOUNDED=2,AV_FACE=4,AV_HAIR=6,AV_CONE=8,AV_BUCKET=10,AV_HELMET=12,AV_MUSTACHE=14,AV_COUNT=16 };
static Animation avatar_art[AV_COUNT];
static const char *avatar_names[AV_COUNT]={"avatar_body_walk","avatar_body_eat","avatar_wounded_walk","avatar_wounded_eat",
 "avatar_face_walk","avatar_face_eat","avatar_hair_walk","avatar_hair_eat","avatar_cone_walk","avatar_cone_eat",
 "avatar_bucket_walk","avatar_bucket_eat","avatar_helmet_walk","avatar_helmet_eat","avatar_mustache_walk","avatar_mustache_eat"};
static uint32_t avatar_checksum(const uint32_t *words){uint32_t value=2166136261u;const unsigned char *bytes=(const unsigned char*)words;for(int i=0;i<28;i++)value=(value^bytes[i])*16777619u;return value;}
static int avatar_valid(const Avatar *a){return a->face<5&&a->hat<5&&a->wear<3&&a->mustache<4&&a->enabled<2;}
static int avatar_load(void){
 uint32_t record[8]={0};FILE *f=fopen(avatar_path,"rb");if(!f)return 0;
 int ok=fread(record,sizeof(record),1,f)==1&&fgetc(f)==EOF;fclose(f);
 Avatar value={record[2],record[3],record[4],record[5],record[6]};
 if(!ok||record[0]!=0x435a5650||record[1]!=1||record[7]!=avatar_checksum(record)||!avatar_valid(&value))return 0;avatar=value;return 1;
}
static int avatar_save(const Avatar *value){
 if(!avatar_valid(value))return 0;uint32_t record[8]={0x435a5650,1,value->face,value->hat,value->wear,value->mustache,value->enabled,0};record[7]=avatar_checksum(record);
 char temporary[160],backup[160];snprintf(temporary,sizeof(temporary),"%s.tmp",avatar_path);snprintf(backup,sizeof(backup),"%s.bak",avatar_path);
 FILE *f=fopen(temporary,"wb");if(!f)return 0;int ok=fwrite(record,sizeof(record),1,f)==1;if(fclose(f))ok=0;if(!ok)return 0;
 FILE *old=fopen(avatar_path,"rb");int existed=old!=0;if(old)fclose(old);
 if(existed){remove(backup);if(rename(avatar_path,backup))return 0;}
 if(rename(temporary,avatar_path)){if(existed)rename(backup,avatar_path);return 0;}
 avatar=*value;return 1;
}
static int avatar_assets(void){
 if(avatar_loaded)return avatar_loaded>0;int ok=1;
 for(int i=0;i<AV_COUNT;i++){char path[128];snprintf(path,sizeof(path),"data/%s.pva",avatar_names[i]);if(!animation_load(&avatar_art[i],path))ok=0;}
 for(int action=0;ok&&action<2;action++){unsigned n=avatar_art[AV_BODY+action].frames;ok=n>0&&avatar_art[AV_WOUNDED+action].frames==n&&avatar_art[AV_FACE+action].frames==n*5&&avatar_art[AV_HAIR+action].frames==n&&avatar_art[AV_CONE+action].frames==n*3&&avatar_art[AV_BUCKET+action].frames==n*3&&avatar_art[AV_HELMET+action].frames==n*3&&avatar_art[AV_MUSTACHE+action].frames==n*3;}
 if(!ok)for(int i=0;i<AV_COUNT;i++)animation_free(&avatar_art[i]);avatar_loaded=ok?1:-1;return ok;
}
static int avatar_draw(const Avatar *value,float x,float foot,float seconds,float scale,uint32_t tint,int eating,int mirrored,int wounded){
 if(!avatar_assets())return 0;int n=avatar_art[AV_BODY+eating].frames,frame=(int)fmaxf(0,seconds*avatar_art[AV_BODY+eating].fps)%n;
 render_anim_frame(&avatar_art[(wounded?AV_WOUNDED:AV_BODY)+eating],x,foot,frame,scale,tint,mirrored);
 render_anim_frame(&avatar_art[AV_FACE+eating],x,foot,value->face*n+frame,scale,tint,mirrored);
 if(value->hat){int bank=AV_HAIR+(value->hat-1)*2;render_anim_frame(&avatar_art[bank+eating],x,foot,(value->hat>1?value->wear*n:0)+frame,scale,tint,mirrored);}
 if(value->mustache)render_anim_frame(&avatar_art[AV_MUSTACHE+eating],x,foot,(value->mustache-1)*n+frame,scale,tint,mirrored);
 return 1;
}
static void avatar_begin(void){avatar_draft=avatar;if(!avatar.enabled)avatar_draft.enabled=1;avatar_row=0;avatar_error[0]=0;screen=26;}
static void draw_player_hub(void){
 render_image(&assets[A_MENU_MAIN],0,0,480,272,(can_resume?0:5)+4,WHITE);ui_dialog(110,62,260,151,"Player");
 const char *labels[]={"Player name","Zombie customizer","Back"};
 for(int i=0;i<3;i++){render_image(&assets[A_UI_BUTTON],138,105+i*30,204,24,i==player_hub_row,i==player_hub_row?UI_ACTIVE_TINT:WHITE);ui_center(144,109+i*30,192,labels[i],i==player_hub_row?UI_FOCUS_TEXT:WHITE,.75f);}
}
static void draw_customizer(float clock){
 render_image(&assets[A_MENU_MAIN],0,0,480,272,(can_resume?0:5)+4,WHITE);ui_dialog(22,18,436,236,"Zombie customizer");
 render_image(&assets[A_BACKGROUND],45,65,164,170,0,WHITE);
 int available=avatar_assets();
 if(available){float scale=fminf(154.0f/avatar_art[AV_BODY].width,160.0f/avatar_art[AV_BODY].height),x=127+(avatar_art[AV_BODY].anchor_x-avatar_art[AV_BODY].width*.5f)*scale;render_clip(45,65,164,170);avatar_draw(&avatar_draft,x,224,clock,scale,WHITE,0,0,0);render_clip(0,0,480,272);}
 else disc_text(51,100,150,"Prepare the latest game data on your PC.",WHITE,.7f);
 static const char *faces[]={"Normal","Red glasses","Green glasses","Blue glasses","Purple glasses"},*hats[]={"None","Hair","Cone","Bucket","Helmet"},*wear[]={"Intact","Dented","Battered"},*hair[]={"None","Curled","Short","Long"};
 char labels[7][64];snprintf(labels[0],64,"Face: %s",faces[avatar_draft.face]);snprintf(labels[1],64,"Headwear: %s",hats[avatar_draft.hat]);snprintf(labels[2],64,"Condition: %s",avatar_draft.hat>1?wear[avatar_draft.wear]:"--");snprintf(labels[3],64,"Mustache: %s",hair[avatar_draft.mustache]);snprintf(labels[4],64,"Use in game: %s",avatar_draft.enabled?"Yes":"No");snprintf(labels[5],64,"Save zombie");snprintf(labels[6],64,"Back");
 for(int i=0;i<7;i++){int y=65+i*24;render_image(&assets[A_UI_BUTTON],229,y,211,22,i==avatar_row,i==avatar_row?UI_ACTIVE_TINT:WHITE);ui_center(235,y+4,199,labels[i],i==avatar_row?UI_FOCUS_TEXT:WHITE,.60f);}
 if(avatar_error[0])ui_center(42,237,396,avatar_error,UI_GOLD,.55f);
}
static int avatar_input(uint32_t pressed){
 if(screen==25){if(pressed&PSP_CTRL_UP)player_hub_row=(player_hub_row+2)%3;if(pressed&PSP_CTRL_DOWN)player_hub_row=(player_hub_row+1)%3;
  if(pressed&PSP_CTRL_CROSS){if(!player_hub_row)name_begin(25);else if(player_hub_row==1)avatar_begin();else screen=0;}if(pressed&PSP_CTRL_CIRCLE)screen=0;return 1;}
 if(screen!=26)return 0;
 if(pressed&PSP_CTRL_UP)avatar_row=(avatar_row+6)%7;if(pressed&PSP_CTRL_DOWN)avatar_row=(avatar_row+1)%7;
 int direction=(pressed&PSP_CTRL_RIGHT)?1:(pressed&PSP_CTRL_LEFT)?-1:0;
 if((pressed&PSP_CTRL_CROSS)&&avatar_row<5)direction=1;
 if(direction){uint32_t *values[]={&avatar_draft.face,&avatar_draft.hat,&avatar_draft.wear,&avatar_draft.mustache,&avatar_draft.enabled};const int counts[]={5,5,3,4,2};if(avatar_row<5&&(avatar_row!=2||avatar_draft.hat>1))*values[avatar_row]=(*values[avatar_row]+counts[avatar_row]+direction)%counts[avatar_row];}
 if((pressed&PSP_CTRL_CROSS)&&avatar_row==5){if(!avatar_assets())snprintf(avatar_error,sizeof(avatar_error),"Latest game data is required");else if(avatar_save(&avatar_draft))screen=25;else snprintf(avatar_error,sizeof(avatar_error),"Could not save the custom zombie");}
 if((pressed&PSP_CTRL_CIRCLE)||((pressed&PSP_CTRL_CROSS)&&avatar_row==6))screen=25;return 1;
}
static int customizer_selftest(FILE *out){
 Avatar old=avatar,draft=avatar_draft;int old_screen=screen,old_row=avatar_row;const char *old_path=avatar_path;avatar_path="selftest-custom-zombie.dat";int pass=0,total=0;
 #define AV_CHECK(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"customizer %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 AV_CHECK("all PS3 layer banks have matching animation lengths",avatar_assets());
 avatar_begin();avatar_row=0;avatar_draft.face=0;avatar_input(PSP_CTRL_LEFT);AV_CHECK("face choices wrap left",avatar_draft.face==4);
 avatar_row=1;avatar_draft.hat=4;avatar_input(PSP_CTRL_RIGHT);AV_CHECK("headwear choices wrap right",avatar_draft.hat==0);
 avatar_draft=(Avatar){4,3,2,3,1};avatar_row=5;avatar_input(PSP_CTRL_CROSS);AV_CHECK("Save commits the preset and returns to Player",screen==25&&avatar.face==4&&avatar.hat==3&&avatar.wear==2&&avatar.mustache==3&&avatar.enabled);
 avatar=(Avatar){0,0,0,0,0};AV_CHECK("custom appearance survives reopening",avatar_load()&&avatar.face==4&&avatar.mustache==3);
 FILE *bad=fopen(avatar_path,"wb");if(bad){fputs("truncated",bad);fclose(bad);}AV_CHECK("damaged presets preserve the last valid appearance",!avatar_load()&&avatar.face==4);
 avatar_begin();avatar_draft.face=1;avatar_input(PSP_CTRL_CIRCLE);AV_CHECK("Cancel preserves the saved appearance",screen==25&&avatar.face==4);
 remove(avatar_path);char backup[160];snprintf(backup,sizeof(backup),"%s.bak",avatar_path);remove(backup);snprintf(backup,sizeof(backup),"%s.tmp",avatar_path);remove(backup);
 avatar=old;avatar_draft=draft;avatar_path=old_path;screen=old_screen;avatar_row=old_row;
 fprintf(out,"CUSTOMIZER RESULT %d/%d\n",pass,total);return pass==total;
 #undef AV_CHECK
}
