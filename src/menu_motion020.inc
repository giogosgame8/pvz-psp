/* Original menu transforms interpolate on the GU; no full-screen movie cache. */
#include <zlib.h>
typedef struct {Animation image;uint32_t page,group,mask,count;float fps;const float *keys;} MenuLayer;
static MenuLayer menu_layers[192];static unsigned menu_layer_count;static unsigned char *menu_motion_data;
static int menu_motion_ready,menu_crow_page=-1;static float menu_clock,crow_clock;
static void menu_motion_close(void){for(unsigned i=0;i<menu_layer_count;i++)animation_free(&menu_layers[i].image);memset(menu_layers,0,sizeof(menu_layers));menu_layer_count=0;free(menu_motion_data);menu_motion_data=0;menu_motion_ready=0;}
static int menu_motion_load(const char *path){
 menu_motion_close();FILE *f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long bytes=ftell(f);rewind(f);int ok=bytes>=16&&bytes<=524288;
 if(ok){menu_motion_data=malloc(bytes);ok=menu_motion_data&&fread(menu_motion_data,bytes,1,f)==1;}fclose(f);if(!ok){menu_motion_close();return 0;}
 uint32_t *h=(uint32_t*)menu_motion_data;ok=h[0]==0x554d5650&&h[1]==1&&h[2]>0&&h[2]<=192&&h[3]==crc32(0,menu_motion_data+16,bytes-16);
 unsigned char *at=menu_motion_data+16,*end=menu_motion_data+bytes;if(ok)menu_layer_count=h[2];
 for(unsigned i=0;i<menu_layer_count&&ok;i++){
  MenuLayer *layer=&menu_layers[i];if(end-at<20){ok=0;break;}memcpy(&layer->page,at,16);memcpy(&layer->fps,at+16,4);at+=20;
  ok=layer->page<=2&&layer->group<=4&&layer->mask>0&&layer->mask<1024&&layer->count>0&&layer->count<=256&&isfinite(layer->fps)&&layer->fps>0&&layer->fps<=60&&end-at>=(long)(layer->count*28);
  if(!ok)break;layer->keys=(const float*)at;for(unsigned k=0;k<layer->count*7&&ok;k++)ok=isfinite(layer->keys[k])&&fabsf(layer->keys[k])<=8192;
  for(unsigned k=0;k<layer->count&&ok;k++)ok=layer->keys[k*7+6]>=0&&layer->keys[k*7+6]<=1;
  at+=layer->count*28;char name[96];snprintf(name,sizeof(name),"data/menu_object%03u.pva",i);if(ok)ok=animation_load(&layer->image,name)&&layer->image.frames==1;
 }
 ok=ok&&at==end;if(!ok){menu_motion_close();return 0;}menu_motion_ready=1;return 1;
}
static int menu_layer_visible(const MenuLayer *layer,int page,int frame){return layer->page==(unsigned)page&&(layer->mask&(1u<<frame))&&(!layer->group||collection_available(layer->group-1));}
static void menu_motion_step(float dt,int sounds){
 menu_clock+=dt;int page=screen==0?0:screen==6?1:screen==11?2:-1;
 if(page!=menu_crow_page){menu_crow_page=page;crow_clock=0;}if(page!=0)return;
 float before=crow_clock;crow_clock+=dt;
 if(sounds){for(int k=0;k<3;k++){float cue=.15f+k*.35f;if(before<cue&&crow_clock>=cue)foley_sample((int[]){FX_CROW_FLAP1,FX_CROW_FLAP2,FX_CROW_FLAP3}[k]);}
  float a=fmaxf(0,before-2.1f),b=fmaxf(0,crow_clock-2.1f);if((int)floorf((a+2.8f)/12)!=(int)floorf((b+2.8f)/12))foley_sample((int[]){FX_CROW_CAW1,FX_CROW_CAW2,FX_CROW_CAW3}[(int)(b/12)%3]);
 }
}
static void menu_crow_draw(void){
 int id=A_CROW_IDLE,loop=1;float t=crow_clock,x=391,y=218;
 if(t<1.1f){float f=fminf(1,t/1.1f),ease=f*f*(3-2*f);id=A_CROW_FLY;x=515+(391-515)*ease;y=25+(218-25)*ease;loop=0;}
 else if(t<2.1f){id=A_CROW_LAND;t-=1.1f;loop=0;}
 else{float cycle=fmodf(t-2.1f,12);id=(int[]){A_CROW_IDLE,A_CROW_IDLE2,A_CROW_IDLE3,A_CROW_TALK}[(int)(cycle/3)];t=fmodf(cycle,3);}
 render_anim(&assets[id],x,y,t,1,WHITE,loop);
}
static void menu_background(int page,int frame){
 if(!menu_motion_ready){render_image(&assets[page==0?A_MENU_MAIN:page==1?A_MENU_MODES:A_MENU_COLLECTION],0,0,480,272,frame,WHITE);return;}
 render_rect(0,0,480,272,RGBA(122,181,225,255));
 for(unsigned i=0;i<menu_layer_count;i++){
  MenuLayer *layer=&menu_layers[i];if(!menu_layer_visible(layer,page,frame))continue;
  float position=fmodf(menu_clock*layer->fps,layer->count),fraction=position-floorf(position);unsigned n=(unsigned)position,next=(n+1)%layer->count;float pose[7];
  for(int k=0;k<7;k++)pose[k]=layer->keys[n*7+k]+(layer->keys[next*7+k]-layer->keys[n*7+k])*fraction;
  if(pose[6]>.001f)render_image_affine(&layer->image,pose,RGBA(255,255,255,(int)(255*fminf(1,pose[6]))));
 }
 if(page==0)menu_crow_draw();
}
