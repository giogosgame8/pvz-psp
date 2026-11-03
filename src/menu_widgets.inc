/* Original disc art and type, fitted to the 480x272 viewport. */
#define UI_INK RGBA(64,42,25,255)
#define UI_GOLD RGBA(255,234,151,255)
#define UI_FOCUS_TEXT RGBA(216,230,204,255)
#define UI_ACTIVE_TINT RGBA(226,237,220,255)
static void ui_center(float x,float y,float width,const char *text,uint32_t color,float size){float measured=disc_text_width(text,size);if(measured>width){size*=width/measured;measured=width;}disc_text(x+(width-measured)*.5f,y,width,text,color,size);}
static void ui_title_case(char *out,size_t cap,const char *text){int word=1;size_t i=0;for(;text[i]&&i+1<cap;i++){char c=text[i];if(c>='A'&&c<='Z'&&!word)c+='a'-'A';out[i]=c;word=c==' '||c=='\n'||c=='-';}out[i]=0;}
static void ui_hint(const char *text){(void)text;}
static void ui_dialog(float x,float y,float width,float height,const char *title){render_image(&assets[A_UI_DIALOG],x,y,width,height,0,WHITE);if(title&&*title)ui_center(x+18,y+11,width-36,title,UI_GOLD,1.02f);}
static void ui_button(float x,float y,float width,const char *label,int selected){render_image(&assets[A_UI_BUTTON],x,y,width,25,selected?1:0,selected?UI_ACTIVE_TINT:WHITE);ui_center(x+6,y+3,width-12,label,selected?UI_FOCUS_TEXT:WHITE,.72f);}
static void ui_header(const char *title){ui_center(90,17,300,title,UI_INK,1.0f);}
static void ui_note(int index){render_rect(0,0,480,253,RGBA(0,0,0,155));render_image(&assets[A_NOTE_PAPER],54,12,372,236,0,WHITE);render_anim(&assets[A_NOTE1+index],240,228,0,1.05f,WHITE,0);}
static void ui_left(float x,float y,float width,const char *text,uint32_t color,float size){float measured=disc_text_width(text,size);if(measured>width-2)size*=(width-2)/measured;disc_text(x,y,width,text,color,size);}
static void ui_hud_footer(const char *title,const char *controls,int meter){int width=meter?316:480;render_rect(0,248,width,24,RGBA(28,40,25,235));ui_left(6,249,width-12,title,UI_GOLD,.5f);ui_left(6,260,width-12,controls,WHITE,.47f);}
static float ui_card_scale(const char *name,int width){float scale=.61f;while(scale>.30f&&disc_text_height(name,width-14,scale)>20)scale-=.02f;return scale;}
static void ui_card(int x,int y,int width,int height,int icon,const char *label,int selected,int complete,int available){
 render_image(&assets[A_CHALLENGE_FRAMES],x,y,width,height,selected?1:0,available?(selected?UI_ACTIVE_TINT:WHITE):RGBA(150,150,150,255));
 if(icon<0){static const int terrains[]={A_BACKGROUND,A_NIGHT,A_BACKGROUND_POOL,A_BACKGROUND_FOG,A_BACKGROUND_ROOF};render_image(&assets[terrains[-icon-1]],x+5,y+4,width-10,height-31,0,available?WHITE:RGBA(80,80,80,255));}
 else render_image(&assets[A_CHALLENGE_THUMBS],x+5,y+4,width-10,height-31,icon,available?WHITE:RGBA(80,80,80,255));
 char name[80];ui_title_case(name,sizeof(name),label);float scale=ui_card_scale(name,width);
 disc_text(x+7,y+height-27,width-14,name,UI_INK,scale);
 if(complete)render_image(&assets[A_REWARD_TROPHY],x+width-19,y+7,13,17,0,WHITE);
 if(selected)render_focus(x-1,y-1,width+2,height+2,FOCUS_SAGE);
}
