#include "render.h"
#include <pspkernel.h>
#include <pspgu.h>
#include <zlib.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

enum { PAGE_BYTES=32768,PAGES=768 };
static int low_memory_cache;
static int page_limit=PAGES,bank_limit=8*1024*1024,static_limit=6*1024*1024;
typedef struct { const Animation *owner;int frame,start,pages;uint32_t used,pinned; } CachedFrame;
static CachedFrame entries[PAGES];static unsigned char page_used[PAGES];
static uint16_t *frame_memory,*scratch;static Animation *animations;
static uint32_t generation=1,serial,bank_bytes,bank_peak,frame_peak,static_bytes,static_peak,decode_count,cache_errors;
static uint32_t error_kinds[5];
static void cache_error(const Animation *a,int frame,int reason){
    cache_errors++;error_kinds[reason]++;
    if(cache_errors<=12){FILE *f=fopen("asset-cache-errors.txt","a");if(f){fprintf(f,"reason %d, asset %s, frame %d, banks %u, pages %d\n",reason,a?a->path:"none",frame,(unsigned)bank_bytes,page_limit);fclose(f);}}
}
void animation_cache_configure(int low){
    animation_cache_suspend();free(frame_memory);frame_memory=0;low_memory_cache=!!low;page_limit=low?384:PAGES;bank_limit=(low?2:8)*1024*1024;static_limit=low?0:6*1024*1024;
}
int animation_cache_ok(void){return cache_errors==0;}
extern int animation_load_legacy(Animation*,const char*);
void animation_cache_next_frame(void){generation++;}
static void release_frame(int i){
    CachedFrame *e=&entries[i];if(!e->owner)return;
    if(e->owner->cached_slots)((Animation*)e->owner)->cached_slots[e->frame]=-1;
    for(int p=e->start;p<e->start+e->pages;p++)page_used[p]=0;memset(e,0,sizeof(*e));
}
int animation_warm(const Animation *source){
    Animation *a=(Animation*)source;if(!a->loaded)return 0;if(!a->compressed||a->packed)return 1;
    while(bank_bytes+a->packed_bytes>(uint32_t)bank_limit){
        Animation *old=0;for(Animation *p=animations;p;p=p->next)if(p!=a&&p->packed&&(!old||p->touched<old->touched))old=p;
        if(!old)return 0;free(old->packed);old->packed=0;bank_bytes-=old->packed_bytes;
    }
    FILE *f=fopen(a->path,"rb");if(!f)return 0;
    a->packed=malloc(a->packed_bytes);int ok=a->packed&&!fseek(f,a->payload_offset,SEEK_SET)&&fread(a->packed,a->packed_bytes,1,f)==1;fclose(f);
    if(!ok){free(a->packed);a->packed=0;cache_error(a,-1,0);return 0;}
    bank_bytes+=a->packed_bytes;if(bank_bytes>bank_peak)bank_peak=bank_bytes;a->touched=++serial;return 1;
}
uint16_t *animation_frame(const Animation *source,int frame){
    Animation *a=(Animation*)source;if(!a->loaded||frame<0||frame>=(int)a->frames)return 0;
    if(!a->compressed)return a->pixels+frame*(a->stride/2);
    if(a->frames==1&&a->pixels)return a->pixels;
    int cached=a->cached_slots[frame];if(cached>=0){CachedFrame *e=&entries[cached];e->used=++serial;e->pinned=generation;return frame_memory+e->start*(PAGE_BYTES/2);}
    if(!animation_warm(a)){cache_error(a,frame,0);return 0;}a->touched=++serial;
    if(!a->gpu_ready&&!scratch)scratch=memalign(16,512*512*2);if(!a->gpu_ready&&!scratch){cache_error(a,frame,1);return 0;}
    uint16_t *target=0;int entry=-1,start=0,pages=(a->stride+PAGE_BYTES-1)/PAGE_BYTES;
    int use_static=0;
    if(a->frames==1&&static_bytes+a->stride<=(uint32_t)static_limit){a->pixels=memalign(16,a->stride);target=a->pixels;if(target){use_static=1;static_bytes+=a->stride;if(static_bytes>static_peak)static_peak=static_bytes;}}
    if(!target){
        if(!frame_memory)frame_memory=memalign(16,page_limit*PAGE_BYTES);if(!frame_memory){cache_error(a,frame,2);return 0;}
        for(;;){
            int run=0;start=-1;for(int p=0;p<page_limit;p++){run=page_used[p]?0:run+1;if(run==pages){start=p-pages+1;break;}}
            if(start>=0){for(int i=0;i<page_limit;i++)if(!entries[i].owner){entry=i;break;}if(entry>=0)break;}
            int oldest=-1;for(int i=0;i<page_limit;i++)if(entries[i].owner&&entries[i].pinned!=generation&&(oldest<0||entries[i].used<entries[oldest].used))oldest=i;
            if(oldest<0){cache_error(a,frame,3);return 0;}release_frame(oldest);
        }
        target=frame_memory+start*(PAGE_BYTES/2);
    }
    if(!target){cache_error(a,frame,2);return 0;}uLongf decoded=a->stride;
    uint16_t *decoded_pixels=a->gpu_ready?target:scratch;
    int ok=uncompress((Bytef*)decoded_pixels,&decoded,a->packed+a->index[frame*2],a->index[frame*2+1])==Z_OK&&decoded==a->stride;
    if(!ok){if(use_static){static_bytes-=a->stride;free(a->pixels);a->pixels=0;}cache_error(a,frame,4);return 0;}
    if(a->grayscale)for(unsigned int i=0;i<a->stride/2;i++){uint16_t p=decoded_pixels[i];int gray=((p&15)*30+((p>>4)&15)*59+((p>>8)&15)*11)/100;decoded_pixels[i]=(p&0xf000)|gray|(gray<<4)|(gray<<8);}
    unsigned char *out=(unsigned char*)target,*in=(unsigned char*)scratch;int row_bytes=a->tw*2;
    if(!a->gpu_ready)for(unsigned int y=0;y<a->th;y+=8)for(int x=0;x<row_bytes;x+=16)for(int row=0;row<8;row++){memcpy(out,in+(y+row)*row_bytes+x,16);out+=16;}
    if(entry>=0){a->cached_slots[frame]=entry;entries[entry]=(CachedFrame){a,frame,start,pages,++serial,generation};for(int p=start;p<start+pages;p++)page_used[p]=1;
        unsigned int used=0;for(int p=0;p<page_limit;p++)used+=page_used[p];if(used*PAGE_BYTES>frame_peak)frame_peak=used*PAGE_BYTES;
    }
    decode_count++;sceKernelDcacheWritebackRange(target,a->stride);sceGuTexFlush();return target;
}
int animation_load(Animation *a,const char *path){
    memset(a,0,sizeof(*a));FILE *f=fopen(path,"rb");if(!f)return 0;char magic[4];uint32_t words[6]={0};float fps,anchor[2];
    if(fread(magic,4,1,f)!=1){fclose(f);return 0;}
    if(memcmp(magic,"PVA3",4)&&memcmp(magic,"PVA4",4)){fclose(f);return animation_load_legacy(a,path);}
    a->gpu_ready=!memcmp(magic,"PVA4",4);
    int ok=fread(words,sizeof(words),1,f)==1&&fread(&fps,4,1,f)==1&&fread(anchor,8,1,f)==1;
    if(ok)ok=words[0]>0&&words[1]>0&&words[0]<=words[2]&&words[1]<=words[3]&&words[2]>=8&&words[3]>=8&&words[2]<=512&&words[3]<=512&&!(words[2]&(words[2]-1))&&!(words[3]&(words[3]-1))&&words[4]>0&&words[4]<=256&&words[5]==words[2]*words[3]*2&&isfinite(fps)&&fps>0&&fps<=120&&isfinite(anchor[0])&&isfinite(anchor[1]);
    long length=0;if(ok){fseek(f,0,SEEK_END);length=ftell(f);ok=length>40+(long)words[4]*8&&length<8*1024*1024;fseek(f,40,SEEK_SET);}
    if(ok){a->index=malloc(words[4]*8);ok=a->index&&fread(a->index,words[4]*8,1,f)==1;}
    if(ok)for(uint32_t i=0;i<words[4];i++)if(a->index[i*2]>(uint32_t)length-40-words[4]*8||!a->index[i*2+1]||a->index[i*2+1]>(uint32_t)length-40-words[4]*8-a->index[i*2])ok=0;
    fclose(f);if(!ok){free(a->index);memset(a,0,sizeof(*a));return 0;}
    a->width=words[0];a->height=words[1];a->tw=words[2];a->th=words[3];a->frames=words[4];a->stride=words[5];a->fps=fps;a->anchor_x=anchor[0];a->anchor_y=anchor[1];
    a->cached_slots=malloc(a->frames*sizeof(int16_t));if(!a->cached_slots){free(a->index);memset(a,0,sizeof(*a));return 0;}for(unsigned i=0;i<a->frames;i++)a->cached_slots[i]=-1;
    a->payload_offset=40+a->frames*8;a->packed_bytes=length-a->payload_offset;a->compressed=a->loaded=1;snprintf(a->path,sizeof(a->path),"%s",path);a->next=animations;animations=a;return 1;
}
void animation_free(Animation *a){
    for(int i=0;i<PAGES;i++)if(entries[i].owner==a)release_frame(i);
    Animation **p=&animations;while(*p&&*p!=a)p=&(*p)->next;if(*p)*p=a->next;
    if(a->packed)bank_bytes-=a->packed_bytes;if(a->compressed&&a->pixels)static_bytes-=a->stride;free(a->packed);free(a->pixels);free(a->index);free(a->cached_slots);memset(a,0,sizeof(*a));
}
void animation_cache_report(FILE *out){if(out){struct mallinfo memory=mallinfo();fprintf(out,"Heap: %u allocated, %u free, %u arena bytes\n",(unsigned)memory.uordblks,(unsigned)memory.fordblks,(unsigned)memory.arena);fprintf(out,"Animation cache: bank peak %u, frame peak %u, static peak %u bytes, %u decoded frames, %u errors\n",(unsigned)bank_peak,(unsigned)frame_peak,(unsigned)static_peak,(unsigned)decode_count,(unsigned)cache_errors);fprintf(out,"Cache error kinds: bank %u, scratch %u, pool %u, pinned %u, decode %u\n",(unsigned)error_kinds[0],(unsigned)error_kinds[1],(unsigned)error_kinds[2],(unsigned)error_kinds[3],(unsigned)error_kinds[4]);fflush(out);}}
void animation_cache_suspend(void){
    for(int i=0;i<PAGES;i++)if(entries[i].owner)release_frame(i);
    for(Animation *a=animations;a;a=a->next)if(a->compressed){free(a->packed);a->packed=0;free(a->pixels);a->pixels=0;}
    /* A music-file allocation during the ending can split the freed small-cache
       block. Keep the small profile's arena reserved so returning to a menu
       never depends on finding another contiguous 8 MiB heap block. Movie
       buffers fit alongside it after banks, static art and scratch are freed. */
    bank_bytes=static_bytes=0;if(!low_memory_cache){free(frame_memory);frame_memory=0;}free(scratch);scratch=0;
}
int animation_cache_layout_selftest(FILE *out){
 uint16_t linear[32*16],tiled[32*16];for(int y=0;y<16;y++)for(int x=0;x<32;x++){uint16_t value=(uint16_t)(x*31+y*123+0x8000);linear[y*32+x]=value;int offset=((y/8)*4+x/8)*64+(y%8)*8+x%8;tiled[offset]=value;}
 unsigned char packed[2048];int pass=0;const char *file="selftest-texture-layout.pva";
 for(int mode=0;mode<3;mode++){
  uLongf bytes=sizeof(packed);int ok=compress2(packed,&bytes,(Bytef*)(mode? tiled:linear),sizeof(linear),9)==Z_OK;
  FILE *f=fopen(file,"wb");uint32_t words[6]={32,16,32,16,1,sizeof(linear)},index[2]={0,bytes};float tail[3]={1,16,16};
  ok=ok&&f&&fwrite(mode?"PVA4":"PVA3",4,1,f)==1&&fwrite(words,sizeof(words),1,f)==1&&fwrite(tail,sizeof(tail),1,f)==1&&fwrite(index,sizeof(index),1,f)==1&&fwrite(packed,bytes,1,f)==1;if(f)fclose(f);
  Animation a={0};ok=ok&&animation_load(&a,file);if(ok){a.grayscale=mode==2;uint16_t *pixels=animation_frame(&a,0);ok=pixels!=0;
   for(int y=0;y<16&&ok;y++)for(int x=0;x<32;x++){uint16_t expected=linear[y*32+x];if(mode==2){int gray=((expected&15)*30+((expected>>4)&15)*59+((expected>>8)&15)*11)/100;expected=(expected&0xf000)|gray|(gray<<4)|(gray<<8);}int at=((y/8)*4+x/8)*64+(y%8)*8+x%8;if(pixels[at]!=expected){ok=0;break;}}
  }animation_free(&a);pass+=ok;fprintf(out,"texture layout %s preserves every pixel: %s\n",mode==0?"legacy PVA3":mode==1?"GPU-ready PVA4":"PVA4 Imitater grayscale",ok?"PASS":"FAIL");
 }
 remove(file);fprintf(out,"TEXTURE LAYOUT RESULT %d/3\n",pass);return pass==3;
}
