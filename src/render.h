#ifndef PVZ_RENDER_H
#define PVZ_RENDER_H
#include <stdint.h>
#include <stdio.h>
typedef struct Animation { uint32_t width,height,tw,th,frames,stride;float fps;uint16_t *pixels;float anchor_x,anchor_y;
    int16_t *cached_slots;uint32_t *index,packed_bytes,payload_offset;unsigned char *packed;uint32_t touched;
    char path[128];struct Animation *next;int compressed,loaded,grayscale,gpu_ready; } Animation;
typedef struct { float cpu_ms,gpu_wait_ms,vblank_wait_ms;uint32_t draw_calls,texture_binds; } RenderStats;
void render_init(void);
void render_set_fps(int target_fps);
void render_begin(void);
void render_end(void);
void render_shutdown(void);
void render_rect(float x,float y,float w,float h,uint32_t color);
void render_outline(float x,float y,float w,float h,uint32_t color);
void render_focus(float x,float y,float w,float h,uint32_t tone);
void render_clip(int x,int y,int w,int h);
void render_text(float x,float y,const char *text,uint32_t color,float scale);
void render_anim(const Animation *animation,float x,float baseline,float seconds,float scale,uint32_t tint,int loop);
void render_anim_frame(const Animation *animation,float x,float baseline,int frame,float scale,uint32_t tint,int mirrored);
void render_anim_mirrored(const Animation *animation,float x,float baseline,float seconds,float scale,uint32_t tint,int loop);
void render_anim_glow(const Animation *animation,float x,float baseline,float seconds,float scale,float glow);
void render_anim_rotated(const Animation *animation,float x,float baseline,float seconds,float scale,uint32_t tint,float angle);
void render_background(const Animation *animation);
void render_movie(const uint16_t *pixels);
void render_image(const Animation *a,float x,float y,float w,float h,int frame,uint32_t tint);
void render_part(const Animation *a,int frame,float sx,float sy,float sw,float sh,float x,float y,float w,float h,uint32_t tint);
int animation_load(Animation *animation,const char *filename);
int animation_catalog_load(const char *filename);
int animation_catalog_selftest(FILE *out);
void animation_catalog_close(void);
void animation_free(Animation *animation);
RenderStats render_stats(void);
int render_selftest(void);
int render_debug_frame(const char *filename);
uint16_t *animation_frame(const Animation *animation,int frame);
int animation_warm(const Animation *animation);
int animation_bank_ready(const Animation *animation);
int animation_prefetch(const Animation *animation);
void animation_prefetch_poll(void);
typedef struct { uint32_t sync_reads,async_reads,waits,pending,bytes,limit; } AnimationIOStats;
AnimationIOStats animation_io_stats(void);
void animation_prefetch_test_delay(unsigned int milliseconds);
void animation_cache_report(FILE *out);
void animation_cache_suspend(void);
int animation_cache_ok(void);
void animation_cache_configure(int low_memory);
int animation_cache_layout_selftest(FILE*);
#define RGBA(r,g,b,a) ((uint32_t)(r)|((uint32_t)(g)<<8)|((uint32_t)(b)<<16)|((uint32_t)(a)<<24))
#define WHITE 0xffffffffu
#define FOCUS_SAGE RGBA(155,187,139,255)
#define FOCUS_ACTION RGBA(205,163,116,255)
#endif
