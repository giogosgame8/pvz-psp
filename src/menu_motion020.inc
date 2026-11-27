/* Original menu transforms interpolate on the GU; no full-screen movie cache. */
#include <zlib.h>
#include "crow_motion0245.inc"
typedef struct {Animation image;uint32_t page,group,mask,count;float fps;const float *keys;} MenuLayer;
enum { MENU_MOTION_PAGES=3, MENU_LAYER_LIMIT=192 };
static MenuLayer menu_layers[MENU_LAYER_LIMIT];static unsigned menu_layer_count;static unsigned char *menu_motion_data;
static uint8_t menu_page_layers[MENU_MOTION_PAGES][MENU_LAYER_LIMIT];
static unsigned menu_page_layer_count[MENU_MOTION_PAGES];
static int menu_motion_ready,menu_crow_page=-1;static float menu_clock,crow_clock;
static float crow_x=515,crow_y=25,crow_from_x=515,crow_from_y=25,crow_to_x=75,crow_to_y=125;
static void crow_destination(int page,float *x,float *y){static const float perches[3][2]={{75,125},{235,114},{112,128}};*x=perches[page][0];*y=perches[page][1];}
static void menu_motion_close(void){for(unsigned i=0;i<menu_layer_count;i++)animation_free(&menu_layers[i].image);memset(menu_layers,0,sizeof(menu_layers));memset(menu_page_layer_count,0,sizeof(menu_page_layer_count));menu_layer_count=0;free(menu_motion_data);menu_motion_data=0;menu_motion_ready=0;}
static int menu_motion_load(const char *path){
 menu_motion_close();FILE *f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long bytes=ftell(f);rewind(f);int ok=bytes>=16&&bytes<=524288;
 if(ok){menu_motion_data=malloc(bytes);ok=menu_motion_data&&fread(menu_motion_data,bytes,1,f)==1;}fclose(f);if(!ok){menu_motion_close();return 0;}
 uint32_t *h=(uint32_t*)menu_motion_data;ok=h[0]==0x554d5650&&h[1]==1&&h[2]>0&&h[2]<=MENU_LAYER_LIMIT&&h[3]==crc32(0,menu_motion_data+16,bytes-16);
 unsigned char *at=menu_motion_data+16,*end=menu_motion_data+bytes;if(ok)menu_layer_count=h[2];
 for(unsigned i=0;i<menu_layer_count&&ok;i++){
  MenuLayer *layer=&menu_layers[i];if(end-at<20){ok=0;break;}memcpy(&layer->page,at,16);memcpy(&layer->fps,at+16,4);at+=20;
  ok=layer->page<MENU_MOTION_PAGES&&layer->group<=4&&layer->mask>0&&layer->mask<1024&&layer->count>0&&layer->count<=256&&isfinite(layer->fps)&&layer->fps>0&&layer->fps<=60&&end-at>=(long)(layer->count*28);
  if(!ok)break;layer->keys=(const float*)at;for(unsigned k=0;k<layer->count*7&&ok;k++)ok=isfinite(layer->keys[k])&&fabsf(layer->keys[k])<=8192;
  for(unsigned k=0;k<layer->count&&ok;k++)ok=layer->keys[k*7+6]>=0&&layer->keys[k*7+6]<=1;
  at+=layer->count*28;char name[96];snprintf(name,sizeof(name),"data/menu_object%03u.pva",i);if(ok)ok=animation_load(&layer->image,name)&&layer->image.frames==1;
  if(ok)menu_page_layers[layer->page][menu_page_layer_count[layer->page]++]=i;
 }
 ok=ok&&at==end;if(!ok){menu_motion_close();return 0;}menu_motion_ready=1;return 1;
}
static int menu_layer_visible(const MenuLayer *layer,int page,int frame){return layer->page==(unsigned)page&&(layer->mask&(1u<<frame))&&(!layer->group||collection_available(layer->group-1));}
static void menu_motion_step(float dt,int sounds){
 menu_clock+=dt;int page=screen==0?0:screen==6?1:screen==11?2:-1;
 if(page!=menu_crow_page){
  int previous=menu_crow_page;menu_crow_page=page;crow_clock=0;
  if(page>=0){
   crow_route=crow_route_for(previous,page);crow_flight=crow_landing=1.1f;
   crow_from_x=previous<0?515:crow_x;crow_from_y=previous<0?25:crow_y;
   crow_destination(page,&crow_to_x,&crow_to_y);
   if(crow_route>=0){
    const CrowRoute *route=&crow_routes[crow_route];
    crow_flight=(route->frames-1)/(float)crow_rate;crow_landing=route->landing/(float)crow_rate;
    if(previous<0){crow_from_x=crow_position(crow_route,0,0);crow_from_y=crow_position(crow_route,1,0);}
   }
  }
 }
 if(page<0)return;
 float before=crow_clock;crow_clock+=dt;
 float f=fminf(1,crow_clock/crow_flight);
 if(crow_route>=0){
  float key=crow_clock*crow_rate,last=crow_routes[crow_route].frames-1;
  crow_x=crow_position(crow_route,0,key)+(1-f)*(crow_from_x-crow_position(crow_route,0,0))+f*(crow_to_x-crow_position(crow_route,0,last));
  crow_y=crow_position(crow_route,1,key)+(1-f)*(crow_from_y-crow_position(crow_route,1,0))+f*(crow_to_y-crow_position(crow_route,1,last));
 }else{
  float ease=f*f*(3-2*f);crow_x=crow_from_x+(crow_to_x-crow_from_x)*ease;
  crow_y=crow_from_y+(crow_to_y-crow_from_y)*ease-24*sinf(3.14159f*f);
 }
 if(sounds){for(int k=0;k<3;k++){float cue=(.15f+k*.29f)*crow_flight;if(before<cue&&crow_clock>=cue)foley_sample((int[]){FX_CROW_FLAP1,FX_CROW_FLAP2,FX_CROW_FLAP3}[k]);}
  float a=fmaxf(0,before-crow_landing-1),b=fmaxf(0,crow_clock-crow_landing-1);if((int)floorf((a+2.8f)/12)!=(int)floorf((b+2.8f)/12))foley_sample((int[]){FX_CROW_CAW1,FX_CROW_CAW2,FX_CROW_CAW3}[(int)(b/12)%3]);
 }
}
static void menu_crow_draw(void){
 int id=A_CROW_IDLE,loop=1;float t=crow_clock,x=crow_x,y=crow_y;
 if(t<crow_landing){id=A_CROW_FLY;}
 else if(t<crow_landing+1){id=A_CROW_LAND;t-=crow_landing;loop=0;}
 else{float cycle=fmodf(t-crow_landing-1,12);id=(int[]){A_CROW_IDLE,A_CROW_IDLE2,A_CROW_IDLE3,A_CROW_TALK}[(int)(cycle/3)];t=fmodf(cycle,3);}
 if(id==A_CROW_FLY&&(crow_route>=0?!crow_routes[crow_route].right:crow_to_x<crow_from_x))render_anim(&assets[id],x,y,t,1.65f,WHITE,loop);
 else render_anim_mirrored(&assets[id],x,y,t,1.65f,WHITE,loop);
}
typedef struct { unsigned count,first,next;float fps,fraction; } MenuPhase;

static const float *menu_layer_pose(const MenuLayer *layer,float clock,MenuPhase *phase,float pose[7]){
 if(layer->count==1)return layer->keys;
 if(phase->count!=layer->count||phase->fps!=layer->fps){
  float position=fmodf(clock*layer->fps,layer->count);
  phase->count=layer->count;
  phase->fps=layer->fps;
  phase->first=(unsigned)position;
  phase->next=phase->first+1==phase->count?0:phase->first+1;
  phase->fraction=position-(float)phase->first;
 }
 const float *first=layer->keys+phase->first*7,*next=layer->keys+phase->next*7;
 for(int i=0;i<7;i++)pose[i]=first[i]+(next[i]-first[i])*phase->fraction;
 return pose;
}

static void menu_background(int page,int frame){
 if(!menu_motion_ready){render_image(&assets[page==0?A_MENU_MAIN:page==1?A_MENU_MODES:A_MENU_COLLECTION],0,0,480,272,frame,WHITE);return;}
 render_rect(0,0,480,272,RGBA(122,181,225,255));
 MenuPhase phase={0};
 for(unsigned i=0;i<menu_page_layer_count[page];i++){
  MenuLayer *layer=&menu_layers[menu_page_layers[page][i]];
  if(!menu_layer_visible(layer,page,frame))continue;
  float storage[7];
  const float *pose=menu_layer_pose(layer,menu_clock,&phase,storage);
  if(pose[6]>.001f)render_image_affine(&layer->image,pose,RGBA(255,255,255,(int)(255*fminf(1,pose[6]))));
 }
 if(page==menu_crow_page)menu_crow_draw();
}
