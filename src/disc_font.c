#include "disc_font.h"
#include <stdio.h>
#include <string.h>
typedef struct {int16_t advance,x,y,w,h;} Glyph;
static Glyph glyphs[256];static Animation *atlas;static int line_height=24;
int disc_font_init(Animation *image){FILE *f=fopen("data/disc_font.bin","rb");if(!f)return 0;uint32_t h[3];int ok=fread(h,sizeof(h),1,f)==1&&h[0]==0x46445650&&h[1]==256&&h[2]>=10&&h[2]<=64&&fread(glyphs,sizeof(glyphs),1,f)==1&&fgetc(f)==EOF;fclose(f);if(ok){atlas=image;line_height=h[2];}return ok;}
static unsigned int next_char(const unsigned char **p){unsigned int c=*(*p)++;if(c<128)return c;if((c&0xe0)==0xc0&&(**p&0xc0)==0x80){c=((c&31)<<6)|(**p&63);(*p)++;return c;}while((**p&0xc0)==0x80)(*p)++;return '?';}
float disc_text_width(const char *text,float scale){float width=0,max=0;const unsigned char *p=(const unsigned char*)text;while(*p){unsigned c=next_char(&p);if(c=='\n'){if(width>max)max=width;width=0;}else width+=(c==' '?8:atlas?glyphs[c<256?c:'?'].advance:6)*scale;}return width>max?width:max;}
float disc_text_height(const char *text,float width,float scale){
 float x=0;int lines=1;const unsigned char *p=(const unsigned char*)text;
 while(*p){if(*p=='\n'){p++;x=0;lines++;continue;}if(*p==' '){p++;x+=8*scale;continue;}const unsigned char *end=p;float length=0;while(*end&&*end!=' '&&*end!='\n'){unsigned c=next_char(&end);length+=(atlas?glyphs[c<256?c:'?'].advance:6)*scale;}if(x>0&&x+length>width){x=0;lines++;}x+=length;p=end;}
 return lines*line_height*scale;
}
void disc_text(float x,float y,float width,const char *text,uint32_t color,float scale){
 if(!atlas||!atlas->loaded){render_text(x,y,text,color,scale);return;}float start=x;const unsigned char *p=(const unsigned char*)text;
 while(*p){if(*p=='\n'){p++;x=start;y+=line_height*scale;continue;}if(*p==' '){p++;x+=8*scale;continue;}const unsigned char *end=p;float length=0;while(*end&&*end!=' '&&*end!='\n'){unsigned c=next_char(&end);length+=glyphs[c<256?c:'?'].advance*scale;}if(x>start&&x+length>start+width){x=start;y+=line_height*scale;}
  while(p<end){unsigned c=next_char(&p);Glyph *g=&glyphs[c<256?c:'?'];if(g->w&&g->h)render_part(atlas,0,g->x,g->y,g->w,g->h,x,y,g->w*scale,g->h*scale,color);x+=g->advance*scale;}
 }
}
