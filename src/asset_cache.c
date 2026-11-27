#include "render.h"
#include <pspkernel.h>
#include <pspgu.h>
#include <pspiofilemgr.h>
#include <zlib.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

enum { PAGE_BYTES=4096,PAGES=6144,PAGES_PER_WORD=32 };
static int page_limit=PAGES,bank_limit=8*1024*1024,static_limit=6*1024*1024;
typedef struct { const Animation *owner;int frame,start,pages;int16_t older,newer;uint32_t pinned;int retained; } CachedFrame;
static CachedFrame entries[PAGES];static uint32_t page_used[PAGES/PAGES_PER_WORD];
static int oldest_frame=-1,newest_frame=-1;
static uint16_t *frame_memory,*scratch;static Animation *animations;
static Animation *oldest_bank,*newest_bank;
static uint32_t generation=1,bank_bytes,bank_peak,bank_evictions,frame_peak,static_bytes,static_peak,decode_count,cache_errors;
enum { BANK_OK=-1, BANK_IO, SCRATCH_MEMORY, FRAME_MEMORY, FRAME_PINNED, FRAME_DECODE, BANK_MEMORY, ERROR_KINDS };
static uint32_t error_kinds[ERROR_KINDS],retained_bytes,retained_peak;
static AnimationFault first_fault;
static char first_fault_path[128];
static unsigned used_pages;static int free_slot_hint;
/* Only the kernel writes the pending buffer. The drawing thread adopts it
   after completion, and it counts against the same bank budget throughout. */
static Animation *pending_owner;static unsigned char *pending_buffer;
static SceUID pending_fd=-1;static int pending_state;static uint64_t pending_after;
static uint32_t sync_reads,async_reads,async_waits,prefetch_delay,prefetch_displacements;
static void prefetch_finish(int wait);
static void cache_error(const Animation *a,int frame,int reason){
    cache_errors++;error_kinds[reason]++;
    if(!first_fault){
        first_fault=reason==BANK_IO||reason==FRAME_DECODE?ANIMATION_FAULT_DATA:ANIMATION_FAULT_MEMORY;
        if(a)snprintf(first_fault_path,sizeof(first_fault_path),"%s",a->path);
    }
    if(cache_errors<=12){FILE *f=fopen("asset-cache-errors.txt","a");if(f){fprintf(f,"reason %d, asset %s, frame %d, banks %u, pages %d\n",reason,a?a->path:"none",frame,(unsigned)bank_bytes,page_limit);fclose(f);}}
}
void animation_cache_configure(int low){
    animation_cache_suspend();free(frame_memory);frame_memory=0;page_limit=low?3072:PAGES;bank_limit=(low?2:8)*1024*1024;static_limit=low?0:6*1024*1024;
}
int animation_cache_ok(void){return cache_errors==0;}
AnimationFault animation_cache_fault(const char **path){*path=first_fault_path;return first_fault;}
extern int animation_load_legacy(Animation*,const char*);
void animation_cache_next_frame(void){animation_prefetch_poll();generation++;}
static void unlink_frame(int index){
    CachedFrame *entry=&entries[index];
    if(entry->older>=0)entries[entry->older].newer=entry->newer;
    else oldest_frame=entry->newer;
    if(entry->newer>=0)entries[entry->newer].older=entry->older;
    else newest_frame=entry->older;
}
static void append_frame(int index){
    CachedFrame *entry=&entries[index];
    entry->older=newest_frame;entry->newer=-1;
    if(newest_frame>=0)entries[newest_frame].newer=index;
    else oldest_frame=index;
    newest_frame=index;
}
static int find_free_pages(const uint32_t *occupied,int limit,int pages){
    int run=0;
    for(int base=0;base<limit;base+=PAGES_PER_WORD){
        uint32_t word=occupied[base/PAGES_PER_WORD];
        if(word==UINT32_MAX){run=0;continue;}
        if(!word){
            if(run+PAGES_PER_WORD>=pages)return base-run;
            run+=PAGES_PER_WORD;continue;
        }
        for(int bit=0;bit<PAGES_PER_WORD;bit++){
            run=(word&(1u<<bit))?0:run+1;
            if(run==pages)return base+bit-pages+1;
        }
    }
    return -1;
}
static void set_pages(uint32_t *bitmap,int start,int pages,int occupied){
    while(pages>0){
        int bit=start%PAGES_PER_WORD,chunk=PAGES_PER_WORD-bit;
        if(chunk>pages)chunk=pages;
        uint32_t mask=(UINT32_MAX>>(PAGES_PER_WORD-chunk))<<bit;
        if(occupied)bitmap[start/PAGES_PER_WORD]|=mask;
        else bitmap[start/PAGES_PER_WORD]&=~mask;
        start+=chunk;
        pages-=chunk;
    }
}
static void release_frame(int i){
    CachedFrame *e=&entries[i];if(!e->owner)return;
    if(e->retained)retained_bytes-=e->pages*PAGE_BYTES;
    if(e->owner->cached_slots)((Animation*)e->owner)->cached_slots[e->frame]=-1;
    unlink_frame(i);set_pages(page_used,e->start,e->pages,0);
    used_pages-=e->pages;free_slot_hint=i;memset(e,0,sizeof(*e));
}
static void unlink_bank(Animation *a){
    if(a->bank_older)a->bank_older->bank_newer=a->bank_newer;
    else oldest_bank=a->bank_newer;
    if(a->bank_newer)a->bank_newer->bank_older=a->bank_older;
    else newest_bank=a->bank_older;
    a->bank_older=a->bank_newer=0;
}
static void append_bank(Animation *a){
    a->bank_older=newest_bank;
    a->bank_newer=0;
    if(newest_bank)newest_bank->bank_newer=a;
    else oldest_bank=a;
    newest_bank=a;
}
static void touch_bank(Animation *a){
    if(!a->packed||a==newest_bank)return;
    unlink_bank(a);
    append_bank(a);
}
static void release_bank(Animation *a){
    if(!a->packed)return;
    unlink_bank(a);
    free(a->packed);
    a->packed=0;
    bank_bytes-=a->packed_bytes;
}
static int reserve_bank(const Animation *a){
    if(a->packed_bytes>(uint32_t)bank_limit)return 0;
    while(bank_bytes+a->packed_bytes>(uint32_t)bank_limit){
        if(!oldest_bank)return 0;
        release_bank(oldest_bank);
        bank_evictions++;
    }
    return 1;
}
int animation_bank_ready(const Animation *a){return a&&a->loaded&&(!a->compressed||a->packed);}
AnimationIOStats animation_io_stats(void){return (AnimationIOStats){sync_reads,async_reads,async_waits,pending_owner!=0,bank_bytes,bank_limit};}
void animation_prefetch_test_delay(unsigned int ms){prefetch_delay=ms;}
static void prefetch_finish(int wait){
    if(!pending_owner)return;
    if(pending_state==1){
        SceInt64 result=0;int status;
        if(wait){async_waits++;status=sceIoWaitAsync(pending_fd,&result);}
        else status=sceIoPollAsync(pending_fd,&result);
        if(status==1)return;
        if(status<0){SceInt64 ignored;sceIoWaitAsync(pending_fd,&ignored);}
        sceIoClose(pending_fd);pending_fd=-1;
        if(status<0||result!=(SceInt64)pending_owner->packed_bytes){
            // An optional prefetch can be retried synchronously when drawn.
            bank_bytes-=pending_owner->packed_bytes;free(pending_buffer);pending_buffer=0;pending_owner=0;pending_state=0;return;
        }
        pending_state=2;
    }
    if(!wait&&(uint64_t)sceKernelGetSystemTimeWide()<pending_after)return;
    pending_owner->packed=pending_buffer;
    append_bank(pending_owner);
    pending_buffer=0;pending_owner=0;pending_state=0;
}
void animation_prefetch_poll(void){prefetch_finish(0);}
int animation_prefetch(const Animation *source){
    Animation *a=(Animation*)source;animation_prefetch_poll();
    if(animation_bank_ready(a)){touch_bank(a);return 1;}
    if(!a||!a->loaded||pending_owner||!reserve_bank(a))return 0;
    unsigned char *buffer=memalign(64,a->packed_bytes);if(!buffer)return 0;
    SceUID fd=sceIoOpen(a->path,PSP_O_RDONLY,0);
    if(fd<0){free(buffer);return 0;}
    sceIoChangeAsyncPriority(fd,0x30);
    if(sceIoLseek32(fd,a->payload_offset,PSP_SEEK_SET)<0||sceIoReadAsync(fd,buffer,a->packed_bytes)<0){sceIoClose(fd);free(buffer);return 0;}
    pending_owner=a;pending_buffer=buffer;pending_fd=fd;pending_state=1;
    pending_after=sceKernelGetSystemTimeWide()+(uint64_t)prefetch_delay*1000;
    bank_bytes+=a->packed_bytes;if(bank_bytes>bank_peak)bank_peak=bank_bytes;async_reads++;return 0;
}
static int warm_bank(Animation *a){
    if(!a->loaded)return BANK_IO;
    if(!a->compressed||a->packed)return BANK_OK;
    if(a->packed_bytes>(uint32_t)bank_limit)return BANK_MEMORY;
    if(pending_owner==a){
        prefetch_finish(1);
        if(a->packed)return BANK_OK;
    }else if(pending_owner&&pending_owner->packed_bytes>(uint32_t)bank_limit-a->packed_bytes){
        // Keep the optional buffer alive until the kernel has stopped writing to it.
        Animation *optional=pending_owner;
        prefetch_finish(1);
        release_bank(optional);
        prefetch_displacements++;
    }
    if(!reserve_bank(a))return BANK_MEMORY;
    FILE *f=fopen(a->path,"rb");if(!f)return BANK_IO;
    a->packed=malloc(a->packed_bytes);
    if(!a->packed){fclose(f);return BANK_MEMORY;}
    sync_reads++;
    int ok=!fseek(f,a->payload_offset,SEEK_SET)&&fread(a->packed,a->packed_bytes,1,f)==1;fclose(f);
    if(!ok){free(a->packed);a->packed=0;return BANK_IO;}
    bank_bytes+=a->packed_bytes;if(bank_bytes>bank_peak)bank_peak=bank_bytes;
    append_bank(a);
    return BANK_OK;
}
int animation_warm(const Animation *source){return warm_bank((Animation*)source)==BANK_OK;}
int animation_check_bank_file(const Animation *a){
    int reason=BANK_IO;
    if(a->loaded){
        if(!a->compressed)return 1;
        if(a->packed_bytes>(uint32_t)bank_limit)reason=BANK_MEMORY;
        else{
            SceIoStat info;
            if(sceIoGetstat(a->path,&info)>=0&&FIO_S_ISREG(info.st_mode)&&
               info.st_size>=(SceOff)a->payload_offset+a->packed_bytes)return 1;
        }
    }
    cache_error(a,-1,reason);
    return 0;
}
int animation_require_bank(const Animation *source){
    int reason=warm_bank((Animation*)source);
    if(reason==BANK_OK)return 1;
    cache_error(source,-1,reason);
    return 0;
}
uint16_t *animation_frame(const Animation *source,int frame){
    Animation *a=(Animation*)source;if(!a->loaded||frame<0||frame>=(int)a->frames)return 0;
    if(!a->compressed)return a->pixels+frame*(a->stride/2);
    if(a->frames==1&&a->pixels)return a->pixels;
    int cached=a->cached_slots[frame];
    if(cached>=0){
        CachedFrame *e=&entries[cached];e->pinned=generation;
        if(cached!=newest_frame){unlink_frame(cached);append_frame(cached);}
        return frame_memory+e->start*(PAGE_BYTES/2);
    }
    int reason=warm_bank(a);
    if(reason>=0){cache_error(a,frame,reason);return 0;}
    touch_bank(a);
    if(!a->gpu_ready&&!scratch)scratch=memalign(16,512*512*2);if(!a->gpu_ready&&!scratch){cache_error(a,frame,SCRATCH_MEMORY);return 0;}
    uint16_t *target=0;int entry=-1,start=0,pages=(a->stride+PAGE_BYTES-1)/PAGE_BYTES;
    int use_static=0;
    if(a->frames==1&&static_bytes+a->stride<=(uint32_t)static_limit){a->pixels=memalign(16,a->stride);target=a->pixels;if(target){use_static=1;static_bytes+=a->stride;if(static_bytes>static_peak)static_peak=static_bytes;}}
    if(!target){
        if(!frame_memory)frame_memory=memalign(16,page_limit*PAGE_BYTES);if(!frame_memory){cache_error(a,frame,FRAME_MEMORY);return 0;}
        for(;;){
            start=(unsigned)pages<=page_limit-used_pages?find_free_pages(page_used,page_limit,pages):-1;
            if(start>=0){int i=free_slot_hint;for(int n=0;n<page_limit;n++){if(!entries[i].owner){entry=i;free_slot_hint=i+1==page_limit?0:i+1;break;}if(++i==page_limit)i=0;}if(entry>=0)break;}
            int oldest=oldest_frame;
            while(oldest>=0&&(entries[oldest].pinned==generation||entries[oldest].retained))oldest=entries[oldest].newer;
            if(oldest<0){cache_error(a,frame,FRAME_PINNED);return 0;}release_frame(oldest);
        }
        target=frame_memory+start*(PAGE_BYTES/2);
    }
    if(!target){cache_error(a,frame,FRAME_MEMORY);return 0;}uLongf decoded=a->stride;
    uint16_t *decoded_pixels=a->gpu_ready?target:scratch;
    int ok=uncompress((Bytef*)decoded_pixels,&decoded,a->packed+a->index[frame*2],a->index[frame*2+1])==Z_OK&&decoded==a->stride;
    if(!ok){if(use_static){static_bytes-=a->stride;free(a->pixels);a->pixels=0;}cache_error(a,frame,FRAME_DECODE);return 0;}
    if(a->grayscale)for(unsigned int i=0;i<a->stride/2;i++){uint16_t p=decoded_pixels[i];int gray=((p&15)*30+((p>>4)&15)*59+((p>>8)&15)*11)/100;decoded_pixels[i]=(p&0xf000)|gray|(gray<<4)|(gray<<8);}
    unsigned char *out=(unsigned char*)target,*in=(unsigned char*)scratch;int row_bytes=a->tw*2;
    if(!a->gpu_ready)for(unsigned int y=0;y<a->th;y+=8)for(int x=0;x<row_bytes;x+=16)for(int row=0;row<8;row++){memcpy(out,in+(y+row)*row_bytes+x,16);out+=16;}
    if(entry>=0){a->cached_slots[frame]=entry;entries[entry]=(CachedFrame){.owner=a,.frame=frame,.start=start,.pages=pages,.pinned=generation};
        append_frame(entry);set_pages(page_used,start,pages,1);
        used_pages+=pages;if(used_pages*PAGE_BYTES>frame_peak)frame_peak=used_pages*PAGE_BYTES;
    }
    decode_count++;sceKernelDcacheWritebackRange(target,a->stride);sceGuTexFlush();return target;
}
/* Small shared UI frames survive ordinary LRU churn. The fixed ceiling leaves
   most of the 12 MiB profile available for combat and never grows the cache. */
int animation_retain_frame(const Animation *source,int frame){
 Animation *a=(Animation*)source;if(!a||!a->loaded||frame<0||frame>=(int)a->frames)return 0;
 int slot=a->compressed?a->cached_slots[frame]:-1;
 if(!a->compressed||(a->frames==1&&a->pixels)||(slot>=0&&entries[slot].retained))return 1;
 unsigned bytes=((a->stride+PAGE_BYTES-1)/PAGE_BYTES)*PAGE_BYTES;
 if(retained_bytes+bytes>1536*1024)return 0;
 if(!animation_frame(a,frame))return 0;slot=a->cached_slots[frame];
 if(slot>=0&&!entries[slot].retained){entries[slot].retained=1;retained_bytes+=bytes;if(retained_bytes>retained_peak)retained_peak=retained_bytes;}
 return 1;
}
int animation_frame_retained(const Animation *a,int frame){
 if(!a||!a->loaded||frame<0||frame>=(int)a->frames)return 0;
 if(!a->compressed||(a->frames==1&&a->pixels))return 1;
 int slot=a->cached_slots[frame];return slot>=0&&entries[slot].retained;
}
static unsigned char *catalog;static uint32_t catalog_count,catalog_offsets[1024];
void animation_catalog_close(void){free(catalog);catalog=0;catalog_count=0;}
static int compare_catalog_offsets(const void *left,const void *right){
 uint32_t a=*(const uint32_t*)left,b=*(const uint32_t*)right;
 int names=strcmp((const char*)catalog+a,(const char*)catalog+b);
 return names?names:(a>b)-(a<b);
}
static const unsigned char *catalog_find(const char *name){
 uint32_t first=0,last=catalog_count;
 while(first<last){
  uint32_t middle=first+(last-first)/2;
  const char *candidate=(const char*)catalog+catalog_offsets[middle];
  if(strcmp(candidate,name)<0)first=middle+1;
  else last=middle;
 }
 if(first==catalog_count)return 0;
 const unsigned char *record=catalog+catalog_offsets[first];
 return strcmp((const char*)record,name)==0?record:0;
}
int animation_catalog_load(const char *path){
 animation_catalog_close();FILE *f=fopen(path,"rb");if(!f)return 0;uint32_t h[5];int ok=fread(h,sizeof(h),1,f)==1&&h[0]==0x49415650&&h[1]==1&&h[2]>0&&h[2]<=1024&&h[3]<=1048576&&h[3]>=120;
 if(ok){catalog=malloc(h[3]);ok=catalog&&fread(catalog,h[3],1,f)==1&&fgetc(f)==EOF&&crc32(0,catalog,h[3])==h[4];}fclose(f);
 uint32_t at=0;int needs_sort=0;
 for(uint32_t i=0;ok&&i<h[2];i++){
  if(at>h[3]||h[3]-at<120){ok=0;break;}uint32_t bytes,metadata;memcpy(&bytes,catalog+at+64,4);memcpy(&metadata,catalog+at+68,4);
  if(!memchr(catalog+at,0,64)||strchr((char*)catalog+at,'/')||strchr((char*)catalog+at,'\\')||metadata<48||metadata>2088||metadata>h[3]-at-72||bytes<=metadata||bytes>=8388608){ok=0;break;}
  catalog_offsets[i]=at;
  if(i&&compare_catalog_offsets(&catalog_offsets[i-1],&catalog_offsets[i])>0)needs_sort=1;
  at+=72+metadata;
 }
 if(!ok||at!=h[3]){animation_catalog_close();return 0;}
 catalog_count=h[2];
 if(needs_sort)qsort(catalog_offsets,catalog_count,sizeof(*catalog_offsets),compare_catalog_offsets);
 return 1;
}
static int animation_metadata(Animation *a,const char *path,const unsigned char *header,uint32_t metadata,uint32_t length){
 if(metadata<48||(memcmp(header,"PVA3",4)&&memcmp(header,"PVA4",4)))return 0;
 uint32_t words[6];float fps,anchor[2];memcpy(words,header+4,sizeof(words));memcpy(&fps,header+28,4);memcpy(anchor,header+32,8);
 int ok=words[0]>0&&words[1]>0&&words[0]<=words[2]&&words[1]<=words[3]&&words[2]>=8&&words[3]>=8&&words[2]<=512&&words[3]<=512&&!(words[2]&(words[2]-1))&&!(words[3]&(words[3]-1))&&words[4]>0&&words[4]<=256&&words[5]==words[2]*words[3]*2&&isfinite(fps)&&fps>0&&fps<=120&&isfinite(anchor[0])&&isfinite(anchor[1]);
 if(!ok||metadata!=40+words[4]*8||length<=metadata||length>=8388608)return 0;
 a->index=malloc(words[4]*8);if(!a->index)return 0;memcpy(a->index,header+40,words[4]*8);
 for(uint32_t i=0;i<words[4];i++)if(a->index[i*2]>length-metadata||!a->index[i*2+1]||a->index[i*2+1]>length-metadata-a->index[i*2])ok=0;
 a->cached_slots=malloc(words[4]*sizeof(int16_t));if(!a->cached_slots)ok=0;
 if(!ok){free(a->index);free(a->cached_slots);memset(a,0,sizeof(*a));return 0;}
 a->gpu_ready=!memcmp(header,"PVA4",4);a->width=words[0];a->height=words[1];a->tw=words[2];a->th=words[3];a->frames=words[4];a->stride=words[5];a->fps=fps;a->anchor_x=anchor[0];a->anchor_y=anchor[1];
 for(unsigned i=0;i<a->frames;i++)a->cached_slots[i]=-1;
 a->payload_offset=metadata;a->packed_bytes=length-metadata;a->compressed=a->loaded=1;snprintf(a->path,sizeof(a->path),"%s",path);a->next=animations;animations=a;return 1;
}
int animation_load(Animation *a,const char *path){
 memset(a,0,sizeof(*a));const char *name=strrchr(path,'/');name=name?name+1:path;
 if(!strncmp(path,"data/",5)){
  const unsigned char *record=catalog_find(name);
  if(record){
   uint32_t length,metadata;memcpy(&length,record+64,4);memcpy(&metadata,record+68,4);
   if(animation_metadata(a,path,record+72,metadata,length))return 1;
  }
 }
 FILE *f=fopen(path,"rb");if(!f)return 0;unsigned char header[2088]={0};int ok=fread(header,40,1,f)==1;
 if(ok&&memcmp(header,"PVA3",4)&&memcmp(header,"PVA4",4)){fclose(f);return animation_load_legacy(a,path);}
 uint32_t frames=0;memcpy(&frames,header+20,4);ok=ok&&frames>0&&frames<=256;
 uint32_t metadata=40+frames*8;long length=0;
 if(ok){ok=fread(header+40,frames*8,1,f)==1&&!fseek(f,0,SEEK_END);if(ok)length=ftell(f);}fclose(f);
 return ok&&length>0&&animation_metadata(a,path,header,metadata,(uint32_t)length);
}

void animation_free(Animation *a){
    if(pending_owner==a)prefetch_finish(1);
    if(a->cached_slots){
        for(unsigned frame=0;frame<a->frames;frame++){
            int slot=a->cached_slots[frame];
            if(slot>=0)release_frame(slot);
        }
    }
    Animation **p=&animations;while(*p&&*p!=a)p=&(*p)->next;if(*p)*p=a->next;
    release_bank(a);
    if(a->compressed&&a->pixels)static_bytes-=a->stride;free(a->pixels);free(a->index);free(a->cached_slots);memset(a,0,sizeof(*a));
}
int animation_catalog_selftest(FILE *out){
 int pass=0;Animation indexed={0},individual={0};int ok=animation_catalog_load("data/assets.idx")&&animation_load(&indexed,"data/pea_idle.pva");
 animation_catalog_close();ok=ok&&animation_load(&individual,"data/pea_idle.pva")&&indexed.frames==individual.frames&&indexed.stride==individual.stride&&indexed.fps==individual.fps&&indexed.anchor_x==individual.anchor_x&&indexed.anchor_y==individual.anchor_y&&!memcmp(indexed.index,individual.index,indexed.frames*8);
 pass+=ok;fprintf(out,"catalog metadata matches individual animation headers: %s\n",ok?"PASS":"FAIL");animation_free(&indexed);animation_free(&individual);
 const char *path="selftest-assets.idx";uint32_t header[5]={0x49415650,1,1,120,123};unsigned char payload[120]={0};FILE *f=fopen(path,"wb");if(f){fwrite(header,sizeof(header),1,f);fwrite(payload,sizeof(payload),1,f);fclose(f);}
 ok=!animation_catalog_load(path);pass+=ok;fprintf(out,"catalog rejects a damaged checksum: %s\n",ok?"PASS":"FAIL");header[3]=0xffffffffu;f=fopen(path,"wb");if(f){fwrite(header,sizeof(header),1,f);fclose(f);}ok=!animation_catalog_load(path);pass+=ok;fprintf(out,"catalog rejects oversized metadata before allocation: %s\n",ok?"PASS":"FAIL");
 static const char *const names[]={"z-last.pva","pea_idle.pva","a-first.pva","a-first.pva"};
 uint32_t metadata=48,length=49;unsigned char shuffled[sizeof(names)/sizeof(*names)*120]={0};
 for(unsigned i=0;i<sizeof(names)/sizeof(*names);i++){
  unsigned char *record=shuffled+i*120;
  strcpy((char*)record,names[i]);memcpy(record+64,&length,4);memcpy(record+68,&metadata,4);
 }
 header[2]=sizeof(names)/sizeof(*names);header[3]=sizeof(shuffled);header[4]=crc32(0,shuffled,sizeof(shuffled));
 f=fopen(path,"wb");int written=f&&fwrite(header,sizeof(header),1,f)==1&&fwrite(shuffled,sizeof(shuffled),1,f)==1;
 if(f&&fclose(f))written=0;
 ok=written&&animation_catalog_load(path)&&catalog_find("a-first.pva")==catalog+240&&
    catalog_find("z-last.pva")==catalog&&!catalog_find("missing.pva");
 pass+=ok;fprintf(out,"catalog searches unordered records and keeps the first duplicate: %s\n",ok?"PASS":"FAIL");
 ok=ok&&animation_load(&individual,"data/pea_idle.pva")&&individual.frames>0;
 pass+=ok;fprintf(out,"invalid indexed metadata falls back to the real animation header: %s\n",ok?"PASS":"FAIL");
 animation_free(&individual);remove(path);
 ok=animation_catalog_load("data/assets.idx");pass+=ok;fprintf(out,"catalog can reopen after invalid input: %s\nCATALOG RESULT %d/6\n",ok?"PASS":"FAIL",pass);return pass==6;
}
void animation_cache_report(FILE *out){
    if(!out)return;
    struct mallinfo memory=mallinfo();
    fprintf(out,"Heap: %u allocated, %u free, %u arena bytes\n",
            (unsigned)memory.uordblks,(unsigned)memory.fordblks,(unsigned)memory.arena);
    fprintf(out,"Animation cache: bank peak %u, frame peak %u, static peak %u bytes, %u decoded frames, %u errors\n",
            (unsigned)bank_peak,(unsigned)frame_peak,(unsigned)static_peak,(unsigned)decode_count,(unsigned)cache_errors);
    fprintf(out,"Cache error kinds: bank I/O %u, scratch %u, pool %u, pinned %u, decode %u, bank memory %u\n",
            (unsigned)error_kinds[BANK_IO],(unsigned)error_kinds[SCRATCH_MEMORY],(unsigned)error_kinds[FRAME_MEMORY],
            (unsigned)error_kinds[FRAME_PINNED],(unsigned)error_kinds[FRAME_DECODE],(unsigned)error_kinds[BANK_MEMORY]);
    fprintf(out,"Animation IO: %u synchronous reads, %u asynchronous reads, %u lifecycle waits\n",
            (unsigned)sync_reads,(unsigned)async_reads,(unsigned)async_waits);
    fprintf(out,"Animation bank evictions: %u; displaced prefetches: %u\n",
            (unsigned)bank_evictions,(unsigned)prefetch_displacements);
    fprintf(out,"Retained UI frames: %u bytes, peak %u bytes (1536 KiB ceiling)\n",
            (unsigned)retained_bytes,(unsigned)retained_peak);
    fflush(out);
}
void animation_cache_suspend(void){
    prefetch_finish(1);
    while(oldest_frame>=0)release_frame(oldest_frame);
    free_slot_hint=0;
    while(oldest_bank)release_bank(oldest_bank);
    for(Animation *a=animations;a;a=a->next)if(a->compressed){free(a->pixels);a->pixels=0;}
    /* Music and decoder allocations can split a freed frame arena during the
       ending. Reserve it through suspension so the menu can reuse it directly.
       The decoder uses room released by the banks, static art and scratch. */
    bank_bytes=static_bytes=0;free(scratch);scratch=0;
}
static int bank_order_selftest(FILE *out,const char *path){
 Animation a={0},b={0},c={0},pending={0};
 int previous_limit=bank_limit;
 animation_cache_suspend();
 int loaded=animation_load(&a,path)&&animation_load(&b,path)&&animation_load(&c,path)&&animation_load(&pending,path);
 int ordered=0,reserved=0,adopted=0,cleared=0;
 if(loaded){
  bank_limit=2*a.packed_bytes;
  ordered=animation_warm(&a)&&animation_warm(&b)&&animation_prefetch(&a)&&animation_warm(&c)&&
          !b.packed&&oldest_bank==&a&&newest_bank==&c&&bank_bytes==(uint32_t)bank_limit;
  if(ordered){
   animation_prefetch(&pending);
   reserved=pending_owner==&pending&&!pending.packed&&!a.packed&&oldest_bank==&c&&
            bank_bytes==(uint32_t)bank_limit&&animation_warm(&b)&&!c.packed&&pending_owner==&pending;
   adopted=reserved&&animation_warm(&pending)&&!pending_owner&&oldest_bank==&b&&newest_bank==&pending&&
           b.bank_newer==&pending&&pending.bank_older==&b&&bank_bytes==(uint32_t)bank_limit;
   animation_free(&b);
   int unlinked=oldest_bank==&pending&&newest_bank==&pending&&!pending.bank_older&&!pending.bank_newer&&
                bank_bytes==pending.packed_bytes;
   animation_cache_suspend();
   cleared=unlinked&&!oldest_bank&&!newest_bank&&!pending.packed&&!bank_bytes;
  }
 }
 animation_free(&a);animation_free(&b);animation_free(&c);animation_free(&pending);
 bank_limit=previous_limit;
 fprintf(out,"bank budget evicts the least recently used resident: %s\n",ordered?"PASS":"FAIL");
 fprintf(out,"pending prefetch consumes the budget before joining the list: %s\n",reserved?"PASS":"FAIL");
 fprintf(out,"completed prefetch joins the newest end of the bank list: %s\n",adopted?"PASS":"FAIL");
 fprintf(out,"freeing and suspending clear resident bank links: %s\n",cleared?"PASS":"FAIL");
 return ordered+reserved+adopted+cleared;
}

static int bank_priority_selftest(FILE *out,const char *large_path,const char *small_path){
 Animation optional={0},needed={0},resident={0};
 unsigned before=cache_errors,displacements=prefetch_displacements;
 int previous_limit=bank_limit;
 animation_cache_suspend();
 int loaded=animation_load(&optional,large_path)&&animation_load(&needed,large_path)&&
            animation_load(&resident,small_path)&&resident.packed_bytes<needed.packed_bytes;
 int oversize_ok=0,priority_ok=0;
 if(loaded){
  bank_limit=needed.packed_bytes+resident.packed_bytes;
  if(animation_warm(&resident)){
   animation_prefetch(&optional);
   Animation oversize={.loaded=1,.compressed=1,.packed_bytes=(uint32_t)bank_limit+1};
   oversize_ok=pending_owner==&optional&&warm_bank(&oversize)==BANK_MEMORY&&
               pending_owner==&optional&&prefetch_displacements==displacements;
   uint16_t *pixels=animation_frame(&needed,0);
   priority_ok=pixels&&pixels[0]==0x8000&&cache_errors==before&&!pending_owner&&
               !optional.packed&&resident.packed&&oldest_bank==&resident&&newest_bank==&needed&&
               bank_bytes==(uint32_t)bank_limit&&prefetch_displacements==displacements+1;
  }
 }
 animation_free(&optional);animation_free(&needed);animation_free(&resident);
 bank_limit=previous_limit;
 fprintf(out,"a genuinely oversized bank leaves a pending prefetch untouched: %s\n",oversize_ok?"PASS":"FAIL");
 fprintf(out,"a required frame can displace an optional prefetch without evicting a compatible bank: %s\n",priority_ok?"PASS":"FAIL");
 return oversize_ok+priority_ok;
}

static int page_range_selftest(void){
 static const uint32_t initial[]={0x7630ff10u,0xffffffffu,0x00000000u,0x31bf7e2du,0xa0f0003cu};
 static const int starts[]={0,1,31,32,33,63,64,95},sizes[]={0,1,2,31,32,33,64,65,128};
 for(unsigned i=0;i<sizeof(starts)/sizeof(*starts);i++)for(unsigned j=0;j<sizeof(sizes)/sizeof(*sizes);j++){
  int start=starts[i],count=sizes[j];
  if(start+count>(int)(sizeof(initial)*8))continue;
  for(int occupied=0;occupied<2;occupied++){
   uint32_t expected[5],batch[5];
   memcpy(expected,initial,sizeof(initial));memcpy(batch,initial,sizeof(initial));
   for(int p=start;p<start+count;p++){
    uint32_t bit=1u<<(p%PAGES_PER_WORD);
    if(occupied)expected[p/PAGES_PER_WORD]|=bit;
    else expected[p/PAGES_PER_WORD]&=~bit;
   }
   set_pages(batch,start,count,occupied);
   if(memcmp(batch,expected,sizeof(batch)))return 0;
  }
 }
 return 1;
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
 uint32_t layout[]={UINT32_MAX>>2,0,UINT32_MAX<<2};
 int spanning=find_free_pages(layout,96,1)==30&&find_free_pages(layout,96,36)==30&&
              find_free_pages(layout,96,37)<0&&find_free_pages(layout,32,3)<0;
 pass+=spanning;fprintf(out,"texture allocation finds the first free range across bitmap words: %s\n",spanning?"PASS":"FAIL");
 int page_ranges=page_range_selftest();
 pass+=page_ranges;fprintf(out,"texture page updates preserve adjacent bits at word boundaries: %s\n",page_ranges?"PASS":"FAIL");
 /* 512 tiny frames pinned in one draw used to waste 16 MiB in 32 KiB
    blocks. They must coexist in the same 12 MiB PSP-1000 cache. */
 Animation banks[2]={{0}};int dense=1;unsigned before=cache_errors;
 for(int bank=0;bank<2;bank++){
  char path[64];snprintf(path,sizeof(path),"selftest-dense-cache-%d.pva",bank);
  uint32_t words[6]={32,16,32,16,256,1024},index[512]={0};float tail[3]={12,16,16};
  FILE *f=fopen(path,"wb");int ok=f&&fwrite("PVA4",4,1,f)==1&&fwrite(words,sizeof(words),1,f)==1&&fwrite(tail,sizeof(tail),1,f)==1&&fwrite(index,sizeof(index),1,f)==1;uint32_t offset=0;
  for(int n=0;n<256&&ok;n++){
   for(int p=0;p<512;p++)linear[p]=(uint16_t)(0x8000+bank*256+n);
   uLongf size=sizeof(packed);ok=compress2(packed,&size,(Bytef*)linear,sizeof(linear),1)==Z_OK;
   index[n*2]=offset;index[n*2+1]=size;offset+=size;ok=ok&&fwrite(packed,size,1,f)==1;
  }
  if(ok)ok=!fseek(f,40,SEEK_SET)&&fwrite(index,sizeof(index),1,f)==1;if(f)fclose(f);
  ok=ok&&animation_load(&banks[bank],path);dense&=ok;
 }
 for(int bank=0;bank<2&&dense;bank++)for(int n=0;n<256&&dense;n++)dense=animation_frame(&banks[bank],n)!=0;
 for(int bank=0;bank<2&&dense;bank++)for(int n=0;n<256&&dense;n++){
  dense=banks[bank].cached_slots[n]>=0;uint16_t *pixels=dense?animation_frame(&banks[bank],n):0;
  for(int p=0;p<512&&dense;p++)dense=pixels&&pixels[p]==(uint16_t)(0x8000+bank*256+n);
 }
 dense&=cache_errors==before;pass+=dense;
 fprintf(out,"texture cache holds 512 simultaneous small frames with intact pixels: %s\n",dense?"PASS":"FAIL");
 int touch_ok=0;
 if(dense){
  int previous_newest=banks[1].cached_slots[255],reused=banks[0].cached_slots[0];
  touch_ok=animation_frame(&banks[0],0)!=0&&newest_frame==reused&&
           entries[reused].older==previous_newest&&entries[previous_newest].newer==reused;
 }
 pass+=touch_ok;fprintf(out,"texture cache moves a reused frame to the newest end: %s\n",touch_ok?"PASS":"FAIL");
 int released=0;
 if(dense&&animation_retain_frame(&banks[0],0)){
  unsigned pages_before=used_pages,retained_before=retained_bytes;
  animation_free(&banks[0]);
  released=used_pages+256==pages_before&&retained_bytes+PAGE_BYTES==retained_before;
  for(int n=0;n<256&&released;n++){
   int slot=banks[1].cached_slots[n];
   released=slot>=0&&entries[slot].owner==&banks[1]&&entries[slot].frame==n;
  }
 }
 pass+=released;fprintf(out,"releasing one animation preserves another animation's cached frames: %s\n",released?"PASS":"FAIL");
 uint16_t *reserved_frames=frame_memory;
 animation_cache_suspend();
 int suspended=dense&&!used_pages&&!retained_bytes&&oldest_frame<0&&newest_frame<0&&!free_slot_hint;
 for(int n=0;n<256&&suspended;n++)suspended=banks[1].cached_slots[n]<0;
 pass+=suspended;fprintf(out,"suspending the cache clears the resident frames and their reverse indexes: %s\n",suspended?"PASS":"FAIL");
 int reclaimed=suspended&&reserved_frames&&frame_memory==reserved_frames&&animation_frame(&banks[1],0)==reserved_frames;
 pass+=reclaimed;fprintf(out,"returning from suspension reuses the reserved frame arena: %s\n",reclaimed?"PASS":"FAIL");
 for(int bank=0;bank<2;bank++)animation_free(&banks[bank]);
 pass+=bank_order_selftest(out,"selftest-dense-cache-0.pva");
 pass+=bank_priority_selftest(out,"selftest-dense-cache-0.pva",file);
 remove(file);
 remove("selftest-dense-cache-0.pva");remove("selftest-dense-cache-1.pva");
 fprintf(out,"TEXTURE LAYOUT RESULT %d/16\n",pass);return pass==16;
}
