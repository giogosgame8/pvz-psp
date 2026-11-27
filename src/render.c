#include "render.h"
void animation_cache_next_frame(void);
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <psprtc.h>
#include <math.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int __attribute__((aligned(16))) commands[262144];
static uint16_t __attribute__((aligned(16))) font_pixels[128*64];
static uint16_t __attribute__((aligned(16))) font_swizzled[128*64];
static uint16_t *const font_vram=(uint16_t*)(0x04000000+512*272*2*2+512*512*2);
static uint16_t *const background_vram=(uint16_t*)(0x04000000+512*272*2*2);
static const Animation *background_source;
static int clip_left,clip_top,clip_right=480,clip_bottom=272;
static const uint16_t *bound_pixels;
static int bound_tw,bound_th,bound_filter,texturing;
static RenderStats stats;
static uint64_t frame_start;
static int target_fps=60;
static uint32_t last_present;
void render_set_fps(int fps){target_fps=fps<=30?30:60;last_present=0;}
typedef struct { float u,v;uint32_t color;float x,y,z; } TexturedVertex;
typedef struct { uint32_t color;float x,y,z; } ColorVertex;

// Original 5x7 bitmap lettering for the prototype UI; no external font asset.
static const char *alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:.-/+!?()$',\"_";
static const uint8_t glyphs[][7]={
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
 {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
 {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
 {7,2,2,2,2,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
 {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
 {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
 {17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
 {30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
 {14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
 {14,17,17,15,1,1,14},{0,4,4,0,4,4,0},{0,0,0,0,0,6,6},
 {0,0,0,31,0,0,0},{1,2,2,4,8,8,16},{0,4,4,31,4,4,0},
 {4,4,4,4,4,0,4},{14,17,1,2,4,0,4},{2,4,8,8,8,4,2},{8,4,2,2,2,4,8},
 {4,15,20,14,5,30,4},{4,4,8,0,0,0,0},{0,0,0,0,0,4,8},{10,10,20,0,0,0,0},
 {0,0,0,0,0,0,31}
};

static uint64_t tick(void){uint64_t value;sceRtcGetCurrentTick(&value);return value;}
static float milliseconds(uint64_t duration){return duration*(1000.0f/sceRtcGetTickResolution());}
static void swizzle_16(const uint16_t *source,uint16_t *destination,int width,int height){
    const uint8_t *in=(const uint8_t*)source;uint8_t *out=(uint8_t*)destination;int row_bytes=width*2;
    for(int y=0;y<height;y+=8)for(int x=0;x<row_bytes;x+=16)
        for(int row=0;row<8;row++){memcpy(out,in+(y+row)*row_bytes+x,16);out+=16;}
}
int render_selftest(void){
    uint16_t source[32*16],swizzled[32*16];
    for(int i=0;i<32*16;i++)source[i]=(uint16_t)(i*73+11);
    swizzle_16(source,swizzled,32,16);
    for(int y=0;y<16;y++)for(int x=0;x<32;x++){
        int byte_x=x*2,offset=((y/8)*4+byte_x/16)*128+(y%8)*16+byte_x%16;
        if(swizzled[offset/2]!=source[y*32+x])return 0;
    }return 1;
}
static void bind_texture(uint16_t *pixels,int tw,int th,int filter){
    if(!texturing){sceGuEnable(GU_TEXTURE_2D);texturing=1;}
    if(bound_pixels!=pixels||bound_tw!=tw||bound_th!=th){
        sceGuTexMode(GU_PSM_4444,0,0,1);sceGuTexImage(0,tw,th,tw,pixels);
        bound_pixels=pixels;bound_tw=tw;bound_th=th;stats.texture_binds++;
    }
    if(bound_filter!=filter){sceGuTexFilter(filter,filter);bound_filter=filter;}
}
static void quad(uint16_t *pixels,int tw,int th,float u,float v,float uw,float vh,
                 float x,float y,float w,float h,uint32_t color){
    if(!pixels)return;TexturedVertex *p=sceGuGetMemory(sizeof(*p)*2);
    p[0]=(TexturedVertex){u,v,color,x,y,0};p[1]=(TexturedVertex){u+uw,v+vh,color,x+w,y+h,0};
    bind_texture(pixels,tw,th,GU_LINEAR);
    sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,p);
    stats.draw_calls++;
}

static int outside_clip(float x,float y,float w,float h){
    return x+w<=clip_left||y+h<=clip_top||x>=clip_right||y>=clip_bottom;
}
static void animation_quad(const Animation *a,int frame,float u,float v,float uw,float vh,
                           float x,float y,float w,float h,uint32_t color){
    if(w>=0&&h>=0&&outside_clip(x,y,w,h))return;
    quad(animation_frame(a,frame),a->tw,a->th,u,v,uw,vh,x,y,w,h,color);
}

void render_init(void){
    background_source=0;
    for(size_t i=0;i<strlen(alphabet);i++){
        int code=(unsigned char)alphabet[i],ox=(code%16)*8,oy=(code/16)*8;
        for(int y=0;y<7;y++)for(int x=0;x<5;x++)if(glyphs[i][y]&(1<<(4-x)))font_pixels[(oy+y)*128+ox+x]=0xffff;
    }
    swizzle_16(font_pixels,font_swizzled,128,64);
    memcpy((void*)((uintptr_t)font_vram|0x40000000),font_swizzled,sizeof(font_swizzled));
    sceKernelDcacheWritebackAll();sceGuInit();sceGuStart(GU_DIRECT,commands);
    // RGB565 scanout halves framebuffer traffic; textures retain RGBA4444 alpha.
    sceGuDrawBuffer(GU_PSM_5650,(void*)0,512);sceGuDispBuffer(480,272,(void*)(512*272*2),512);
    sceGuOffset(2048-240,2048-136);sceGuViewport(2048,2048,480,272);
    render_clip(0,0,480,272);sceGuEnable(GU_SCISSOR_TEST);sceGuDisable(GU_DEPTH_TEST);
    sceGuEnable(GU_BLEND);sceGuBlendFunc(GU_ADD,GU_SRC_ALPHA,GU_ONE_MINUS_SRC_ALPHA,0,0);
    sceGuShadeModel(GU_SMOOTH);
    sceGuAlphaFunc(GU_GREATER,0,0xff);sceGuEnable(GU_ALPHA_TEST);
    sceGuTexWrap(GU_CLAMP,GU_CLAMP);sceGuTexScale(1,1);sceGuTexOffset(0,0);
    sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
    sceGuFinish();sceGuSync(0,0);sceDisplayWaitVblankStart();sceGuDisplay(GU_TRUE);
}
void render_begin(void){
    animation_cache_next_frame();memset(&stats,0,sizeof(stats));frame_start=tick();bound_pixels=0;bound_filter=-1;
    sceGuStart(GU_DIRECT,commands);sceGuDisable(GU_TEXTURE_2D);texturing=0;
    sceGuClearColor(RGBA(23,35,27,255));sceGuClear(GU_COLOR_BUFFER_BIT);
}
void render_end(void){
    sceGuFinish();uint64_t submitted=tick();sceGuSync(0,0);uint64_t finished=tick();
    sceDisplayWaitVblankStart();
    uint32_t vblank=(uint32_t)sceDisplayGetVcount();
    if(target_fps==30&&last_present&&(uint32_t)(vblank-last_present)<2){sceDisplayWaitVblankStart();vblank=(uint32_t)sceDisplayGetVcount();}
    last_present=vblank;uint64_t presented=tick();sceGuSwapBuffers();
    stats.cpu_ms=milliseconds(submitted-frame_start);stats.gpu_wait_ms=milliseconds(finished-submitted);
    stats.vblank_wait_ms=milliseconds(presented-finished);
}
RenderStats render_stats(void){return stats;}
int render_debug_frame(const char *filename){
    void *address=0;int width=0,format=0;
    if(sceDisplayGetFrameBuf(&address,&width,&format,0)<0||!address||format!=PSP_DISPLAY_PIXEL_FORMAT_565)return 0;
    // A GE transfer requests a fresh readback from the emulator's host GPU.
    uint16_t *capture=memalign(16,512*272*2);if(!capture)return 0;
    sceKernelDcacheWritebackInvalidateRange(capture,512*272*2);
    sceGuStart(GU_DIRECT,commands);
    sceGuCopyImage(GU_PSM_5650,0,0,480,272,width,address,0,0,512,capture);
    sceGuFinish();sceGuSync(0,0);sceKernelDcacheInvalidateRange(capture,512*272*2);
    FILE *f=fopen(filename,"wb");if(!f){free(capture);return 0;}fprintf(f,"P6\n480 272\n255\n");
    const uint16_t *pixels=capture;width=512;uint8_t row[480*3];
    int ok=1;
    for(int y=0;y<272;y++){
        for(int x=0;x<480;x++){uint16_t p=pixels[y*width+x];row[x*3]=(p&31)*255/31;row[x*3+1]=((p>>5)&63)*255/63;row[x*3+2]=(p>>11)*255/31;}
        if(fwrite(row,sizeof(row),1,f)!=1){ok=0;break;}
    }fclose(f);free(capture);return ok;
}
void render_shutdown(void){sceGuTerm();}
void render_rect(float x,float y,float w,float h,uint32_t color){
    ColorVertex *p=sceGuGetMemory(2*sizeof(*p));p[0]=(ColorVertex){color,x,y,0};p[1]=(ColorVertex){color,x+w,y+h,0};
    if(texturing){sceGuDisable(GU_TEXTURE_2D);texturing=0;}
    sceGuDrawArray(GU_SPRITES,GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,p);stats.draw_calls++;
}
void render_outline(float x,float y,float w,float h,uint32_t color){render_rect(x,y,w,1,color);render_rect(x,y+h-1,w,1,color);render_rect(x,y,1,h,color);render_rect(x+w-1,y,1,h,color);}
static void focus_point(float x,float y,float w,float h,float radius,int corner,int segment,float *px,float *py){
    static const float c[6]={1,.9510565f,.809017f,.5877853f,.309017f,0},s[6]={0,.309017f,.5877853f,.809017f,.9510565f,1};
    float dx=corner==0?-c[segment]:corner==1?s[segment]:corner==2?c[segment]:-s[segment];
    float dy=corner==0?-s[segment]:corner==1?-c[segment]:corner==2?s[segment]:c[segment];
    *px=x+(corner==0||corner==3?radius:w-radius)+radius*dx;
    *py=y+(corner<2?radius:h-radius)+radius*dy;
}
enum { FOCUS_POINTS=25, FOCUS_EDGES=4, FOCUS_STRIP=FOCUS_POINTS*2 };
static void focus_edges(ColorVertex edges[FOCUS_EDGES][FOCUS_POINTS],float x,float y,float w,float h,uint32_t tone){
    static const float offsets[FOCUS_EDGES]={-2.5f,0,1.2f,2.5f};
    static const uint32_t alphas[FOCUS_EDGES]={0,190u<<24,125u<<24,0};
    float radius=fminf(6,fminf(w,h)*.18f);
    uint32_t rgb=tone&0xffffffu;
    for(int edge=0;edge<FOCUS_EDGES;edge++){
        float offset=offsets[edge],r=fmaxf(0,radius-offset);
        for(int k=0;k<FOCUS_POINTS-1;k++){
            float px,py;
            focus_point(x+offset,y+offset,w-2*offset,h-2*offset,r,k/6,k%6,&px,&py);
            edges[edge][k]=(ColorVertex){rgb|alphas[edge],px,py,0};
        }
        edges[edge][FOCUS_POINTS-1]=edges[edge][0];
    }
}
static void fill_focus_strip(ColorVertex *p,const ColorVertex *outside,const ColorVertex *inside){
    for(int k=0;k<FOCUS_POINTS;k++){p[k*2]=outside[k];p[k*2+1]=inside[k];}
}
static void submit_focus_strip(ColorVertex *p,int count){
    if(texturing){sceGuDisable(GU_TEXTURE_2D);texturing=0;}
    sceGuDrawArray(GU_TRIANGLE_STRIP,GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,count,0,p);
    stats.draw_calls++;
}
void render_focus(float x,float y,float w,float h,uint32_t tone){
    if(w<8||h<8)return;
    ColorVertex edges[FOCUS_EDGES][FOCUS_POINTS];
    focus_edges(edges,x,y,w,h,tone);
    ColorVertex *p=sceGuGetMemory((3*FOCUS_STRIP+4)*sizeof(*p));
    int used=0;
    for(int band=0;band<FOCUS_EDGES-1;band++){
        if(band){
            /* Repeated endpoints join the strips without adding a visible triangle. */
            p[used]=p[used-1];used++;
            p[used++]=edges[band][0];
        }
        fill_focus_strip(p+used,edges[band],edges[band+1]);
        used+=FOCUS_STRIP;
    }
    submit_focus_strip(p,used);
}

int render_focus_selftest(void){
    static const float frames[][4]={{30,42,37,34},{63.5f,61.25f,33,40},{-3,15,45,29}};
    const char *paths[]={"review-focus-separate.ppm","review-focus-batched.ppm"};
    int ok=1;
    for(int grouped=0;grouped<2;grouped++){
        render_begin();render_clip(4,18,470,240);
        for(int k=0;k<3;k++){
            const float *f=frames[k];uint32_t color=k==1?FOCUS_ACTION:FOCUS_SAGE;
            if(grouped)render_focus(f[0],f[1],f[2],f[3],color);
            else{
                ColorVertex edges[FOCUS_EDGES][FOCUS_POINTS];
                focus_edges(edges,f[0],f[1],f[2],f[3],color);
                for(int band=0;band<FOCUS_EDGES-1;band++){
                    ColorVertex *p=sceGuGetMemory(FOCUS_STRIP*sizeof(*p));
                    fill_focus_strip(p,edges[band],edges[band+1]);
                    submit_focus_strip(p,FOCUS_STRIP);
                }
            }
        }
        render_clip(0,0,480,272);render_end();
        ok&=render_stats().draw_calls==(grouped?3u:9u)&&render_stats().texture_binds==0;
        ok&=render_debug_frame(paths[grouped]);
    }
    FILE *a=fopen(paths[0],"rb"),*b=fopen(paths[1],"rb");
    if(!a||!b)ok=0;
    while(ok){
        unsigned char first[1024],second[1024];
        size_t n=fread(first,1,sizeof(first),a),m=fread(second,1,sizeof(second),b);
        ok=n==m&&!memcmp(first,second,n)&&!ferror(a)&&!ferror(b);
        if(!n)break;
    }
    if(a)fclose(a);if(b)fclose(b);
    return ok;
}
void render_text(float x,float y,const char *text,uint32_t color,float scale){
    float start=x;int count=0;
    for(const char *p=text;*p;p++)if(*p!=' '&&*p!='\n')count++;
    if(!count)return;
    TexturedVertex *vertices=sceGuGetMemory(count*2*sizeof(*vertices));int used=0;
    for(;*text;text++){
        unsigned char ch=*text;if(ch>='a'&&ch<='z')ch-=32;
        if(ch=='\n'){y+=9*scale;x=start;continue;}
        if(ch!=' '){
            float u=(ch%16)*8,v=(ch/16)*8;
            vertices[used++]=(TexturedVertex){u,v,color,x,y,0};
            vertices[used++]=(TexturedVertex){u+6,v+8,color,x+6*scale,y+8*scale,0};
        }
        x+=6*scale;
    }
    bind_texture(font_vram,128,64,scale<1?GU_LINEAR:GU_NEAREST);
    sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,used,0,vertices);stats.draw_calls++;
}
void render_anim_frame(const Animation *a,float x,float baseline,int frame,float scale,uint32_t tint,int mirrored){
    if(!a->loaded||frame<0||frame>=(int)a->frames)return;
    animation_quad(a,frame,mirrored?a->width:0,0,mirrored?-(float)a->width:a->width,a->height,
         x-(mirrored?a->width-a->anchor_x:a->anchor_x)*scale,baseline-a->anchor_y*scale,a->width*scale,a->height*scale,tint);
}
void render_anim(const Animation *a,float x,float baseline,float seconds,float scale,uint32_t tint,int loop){
    if(!a->loaded||a->frames==0)return;
    int frame=(int)fmaxf(0,seconds*a->fps);if(loop)frame%=a->frames;else if(frame>=(int)a->frames)frame=a->frames-1;
    float width=a->width*scale,height=a->height*scale;
    animation_quad(a,frame,0,0,a->width,a->height,x-a->anchor_x*scale,baseline-a->anchor_y*scale,width,height,tint);
}
void render_part(const Animation *a,int frame,float sx,float sy,float sw,float sh,float x,float y,float w,float h,uint32_t tint){
    if(!a->loaded||!a->frames||sw<=0||sh<=0||w<=0||h<=0)return;
    frame=frame<0?0:frame%(int)a->frames;
    animation_quad(a,frame,sx,sy,sw,sh,x,y,w,h,tint);
}
void render_parts(const Animation *a,int frame,const TexturePart *parts,unsigned count,uint32_t tint){
    if(!a->loaded||!a->frames)return;
    TexturedVertex *vertices=NULL;
    unsigned used=0;
    for(unsigned i=0;i<count;i++){
        const TexturePart *part=&parts[i];
        if(part->sw<=0||part->sh<=0||part->w<=0||part->h<=0||
           outside_clip(part->x,part->y,part->w,part->h))continue;
        if(!vertices)vertices=sceGuGetMemory(count*2*sizeof(*vertices));
        vertices[used++]=(TexturedVertex){part->sx,part->sy,tint,part->x,part->y,0};
        vertices[used++]=(TexturedVertex){part->sx+part->sw,part->sy+part->sh,tint,part->x+part->w,part->y+part->h,0};
    }
    if(!used)return;
    frame=frame<0?0:frame%(int)a->frames;
    uint16_t *pixels=animation_frame(a,frame);
    if(!pixels)return;
    bind_texture(pixels,a->tw,a->th,GU_LINEAR);
    sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,used,0,vertices);
    stats.draw_calls++;
}
void render_anim_mirrored(const Animation *a,float x,float baseline,float seconds,float scale,uint32_t tint,int loop){
    if(!a->loaded||!a->frames)return;int frame=(int)fmaxf(0,seconds*a->fps);
    if(loop)frame%=a->frames;else if(frame>=(int)a->frames)frame=a->frames-1;
    animation_quad(a,frame,a->width,0,-(float)a->width,a->height,
         x-(a->width-a->anchor_x)*scale,baseline-a->anchor_y*scale,a->width*scale,a->height*scale,tint);
}
void render_image(const Animation *a,float x,float y,float w,float h,int frame,uint32_t tint){render_part(a,frame,0,0,a->width,a->height,x,y,w,h,tint);}
void render_background(const Animation *a){
    if(!a->loaded||!a->frames)return;
    if(background_source!=a){
        if(a->tw!=512||a->th!=512)return;
        uint16_t *pixels=animation_frame(a,0);if(!pixels)return;
        memcpy((void*)((uintptr_t)background_vram|0x40000000),pixels,a->stride);background_source=a;
    }
    // Narrow strips preserve texture-cache locality on the real PSP GE.
    TexturedVertex *p=sceGuGetMemory(16*sizeof(*p));int used=0;
    for(int x=0;x<480;x+=64){int right=x+64<480?x+64:480;
        p[used++]=(TexturedVertex){x,0,WHITE,x,0,0};p[used++]=(TexturedVertex){right,272,WHITE,right,272,0};
    }
    bind_texture(background_vram,a->tw,a->th,GU_LINEAR);
    sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,used,0,p);stats.draw_calls++;
}
void render_anim_glow(const Animation *a,float x,float baseline,float seconds,float scale,float glow){
    int add=(int)(fminf(1,fmaxf(0,glow))*180);
    sceGuTexFunc(GU_TFX_ADD,GU_TCC_RGBA);
    render_anim(a,x,baseline,seconds,scale,RGBA(add,add,add,255),1);
    sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
}
void render_clip(int x,int y,int w,int h){
    int right=x+w,bottom=y+h;
    clip_left=x>0?x:0;clip_top=y>0?y:0;
    clip_right=right<480?right:480;clip_bottom=bottom<272?bottom:272;
    sceGuScissor(x,y,right,bottom);
}
void render_anim_rotated(const Animation *a,float x,float baseline,float seconds,float scale,uint32_t tint,float angle){
    if(!a->loaded||!a->frames)return;
    int frame=(int)fmaxf(0,seconds*a->fps)%a->frames;
    float w=a->width*scale,h=a->height*scale,c=cosf(angle),s=sinf(angle),cy=baseline-h*.5f;
    TexturedVertex *v=sceGuGetMemory(4*sizeof(*v));
    for(int i=0;i<4;i++){
        float dx=(i%2?w:-w)*.5f,dy=(i/2?h:-h)*.5f;
        v[i]=(TexturedVertex){i%2?a->width:0,i/2?a->height:0,tint,x+dx*c-dy*s,cy+dx*s+dy*c,0};
    }
    uint16_t *pixels=animation_frame(a,frame);if(!pixels)return;bind_texture(pixels,a->tw,a->th,GU_LINEAR);
    sceGuDrawArray(GU_TRIANGLE_STRIP,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,4,0,v);stats.draw_calls++;
}
void render_image_affine(const Animation *a,const float m[6],uint32_t tint){
    if(!a->loaded||!a->frames)return;uint16_t *pixels=animation_frame(a,0);if(!pixels)return;
    TexturedVertex *v=sceGuGetMemory(4*sizeof(*v));
    for(int i=0;i<4;i++){float x=i%2?a->width:0,y=i/2?a->height:0;v[i]=(TexturedVertex){x,y,tint,m[0]*x+m[1]*y+m[4],m[2]*x+m[3]*y+m[5],0};}
    bind_texture(pixels,a->tw,a->th,GU_LINEAR);
    sceGuDrawArray(GU_TRIANGLE_STRIP,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,4,0,v);stats.draw_calls++;
}
int animation_load_legacy(Animation *a,const char *filename){
    memset(a,0,sizeof(*a));FILE *f=fopen(filename,"rb");if(!f)return 0;
    char magic[4];uint32_t words[6]={0};float fps;
    int ok=fread(magic,4,1,f)==1&&(!memcmp(magic,"PVA1",4)||!memcmp(magic,"PVA2",4))&&fread(words,sizeof(words),1,f)==1&&fread(&fps,4,1,f)==1;
    float anchor[2]={words[0]*.5f,words[1]};if(ok&&!memcmp(magic,"PVA2",4))ok=fread(anchor,sizeof(anchor),1,f)==1;
    if(ok)ok=words[0]>0&&words[1]>0&&words[0]<=words[2]&&words[1]<=words[3]&&words[2]>=8&&words[3]>=8&&words[2]<=512&&words[3]<=512&&words[4]>0&&words[4]<=256&&words[5]==words[2]*words[3]*2&&fps>0&&fps<=120;
    if(ok)ok=(words[2]&(words[2]-1))==0&&(words[3]&(words[3]-1))==0;
    if(ok)ok=isfinite(anchor[0])&&isfinite(anchor[1])&&fabsf(anchor[0])<=1024&&fabsf(anchor[1])<=1024;
    size_t bytes=ok?(size_t)words[4]*words[5]:0;if(bytes>8*1024*1024)ok=0;
    if(ok){a->pixels=memalign(16,bytes);ok=a->pixels&&fread(a->pixels,bytes,1,f)==1;}
    fclose(f);
    if(!ok){animation_free(a);return 0;}
    uint16_t *scratch=memalign(16,words[5]);
    if(!scratch){animation_free(a);return 0;}
    for(uint32_t frame=0;frame<words[4];frame++){
        uint16_t *pixels=a->pixels+frame*(words[5]/2);swizzle_16(pixels,scratch,words[2],words[3]);memcpy(pixels,scratch,words[5]);
    }free(scratch);
    a->width=words[0];a->height=words[1];a->tw=words[2];a->th=words[3];a->frames=words[4];a->stride=words[5];a->fps=fps;
    a->anchor_x=anchor[0];a->anchor_y=anchor[1];
    a->loaded=1;sceKernelDcacheWritebackRange(a->pixels,bytes);return 1;
}


void render_movie(const uint16_t *pixels){
    if(!pixels)return;if(!texturing){sceGuEnable(GU_TEXTURE_2D);texturing=1;}
    sceGuTexMode(GU_PSM_5650,0,0,0);sceGuTexImage(0,512,512,512,pixels);sceGuTexFilter(GU_LINEAR,GU_LINEAR);TexturedVertex *v=sceGuGetMemory(2*sizeof(*v));
    v[0]=(TexturedVertex){0,0,WHITE,0,0,0};v[1]=(TexturedVertex){480,272,WHITE,480,272,0};sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,v);stats.draw_calls++;stats.texture_binds++;bound_pixels=0;bound_filter=-1;
}
