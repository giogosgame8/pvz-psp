#ifndef PVZ_DISC_FONT_H
#define PVZ_DISC_FONT_H
#include "render.h"
int disc_font_init(Animation *atlas);
void disc_text(float x,float y,float width,const char *text,uint32_t color,float scale);
float disc_text_width(const char *text,float scale);
float disc_text_height(const char *text,float width,float scale);
#endif
