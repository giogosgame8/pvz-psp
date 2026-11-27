#include "movie.h"
#include <pspkernel.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <zlib.h>
#include <math.h>
typedef struct {uint32_t offset,bytes,crc;} MovieIndex;
typedef struct {struct jpeg_error_mgr pub;jmp_buf jump;} DecodeError;
typedef struct {struct jpeg_decompress_struct jpeg;DecodeError error;} MovieDecoder;
enum { MOVIE_WIDTH=480,MOVIE_HEIGHT=272,MOVIE_TEXTURE_SIDE=512,
       MOVIE_TEXTURE_BYTES=MOVIE_TEXTURE_SIDE*MOVIE_TEXTURE_SIDE*2,
       MOVIE_VISIBLE_BYTES=MOVIE_TEXTURE_SIDE*MOVIE_HEIGHT*2 };
static FILE *file;static MovieIndex *index_table;static unsigned char *compressed;static uint16_t *frames[3];
static volatile int states[3],numbers[3],wanted,running,error_code;static SceUID worker=-1;static int front=-1;static uint32_t count,max_bytes;static float fps;
static uint32_t next_read;
static void decode_error(j_common_ptr d){DecodeError *e=(DecodeError*)d->err;longjmp(e->jump,1);}
static int init_decoder(MovieDecoder *decoder){
 memset(decoder,0,sizeof(*decoder));
 decoder->jpeg.err=jpeg_std_error(&decoder->error.pub);
 decoder->error.pub.error_exit=decode_error;
 if(setjmp(decoder->error.jump)){jpeg_destroy_decompress(&decoder->jpeg);return 0;}
 jpeg_create_decompress(&decoder->jpeg);
 return 1;
}
static int decode_jpeg(MovieDecoder *decoder,const unsigned char *data,int bytes,uint16_t *output){
 struct jpeg_decompress_struct *jpeg=&decoder->jpeg;
 if(setjmp(decoder->error.jump))return 0;
 jpeg_mem_src(jpeg,data,bytes);
 if(jpeg_read_header(jpeg,TRUE)!=JPEG_HEADER_OK||jpeg->image_width!=MOVIE_WIDTH||jpeg->image_height!=MOVIE_HEIGHT)return 0;
 jpeg->out_color_space=JCS_RGB;jpeg->dct_method=JDCT_IFAST;jpeg->do_fancy_upsampling=FALSE;
 if(!jpeg_start_decompress(jpeg)||jpeg->output_width!=MOVIE_WIDTH||jpeg->output_height!=MOVIE_HEIGHT||jpeg->output_components!=3)return 0;
 unsigned char row[MOVIE_WIDTH*3];JSAMPROW rows[1]={row};
 while(jpeg->output_scanline<jpeg->output_height){
  uint16_t *pixels=output+jpeg->output_scanline*MOVIE_TEXTURE_SIDE;
  if(jpeg_read_scanlines(jpeg,rows,1)!=1)return 0;
  for(int x=0;x<MOVIE_WIDTH;x++)pixels[x]=(row[x*3]>>3)|((row[x*3+1]>>2)<<5)|((row[x*3+2]>>3)<<11);
 }
 return jpeg_finish_decompress(jpeg);
}
static int read_frame(MovieDecoder *decoder,int n,uint16_t *out){
 if(n<0||(uint32_t)n>=count){error_code=2;return 0;}
 const MovieIndex *p=&index_table[n];
 if(p->offset!=next_read&&fseek(file,p->offset,SEEK_SET)){error_code=3;return 0;}
 if(fread(compressed,p->bytes,1,file)!=1){error_code=4;return 0;}
 next_read=p->offset+p->bytes;
 if(crc32(0,compressed,p->bytes)!=p->crc){error_code=5;return 0;}
 if(!decode_jpeg(decoder,compressed,p->bytes,out)){error_code=6;return 0;}
 return 1;
}
static int frame_available(int number){
 for(int i=0;i<3;i++)if(states[i]>=2&&numbers[i]==number)return 1;
 return 0;
}
static int decode_worker(SceSize args,void *arg){
 (void)args;(void)arg;
 MovieDecoder decoder;
 if(!init_decoder(&decoder)){error_code=6;running=0;return 0;}
 while(running){
  int current=wanted,target=current;
  while(target>=0&&target<current+3&&target<(int)count&&frame_available(target))target++;
  if(target<0||target>=current+3||target>=(int)count){sceKernelDelayThread(1000);continue;}
  int slot=-1;for(int i=0;i<3;i++)if(states[i]==0){slot=i;states[i]=1;break;}if(slot<0){sceKernelDelayThread(1000);continue;}
  if(!read_frame(&decoder,target,frames[slot])){
   FILE *diagnostic=fopen("movie-error.txt","a");
   if(diagnostic){fprintf(diagnostic,"frame %d, error %d, frame bytes %u, free memory %u\n",target,error_code,(unsigned)index_table[target].bytes,(unsigned)sceKernelTotalFreeMemSize());fclose(diagnostic);}
   states[slot]=0;running=0;break;
  }
  sceKernelDcacheWritebackRange(frames[slot],MOVIE_VISIBLE_BYTES);numbers[slot]=target;states[slot]=2;
 }
 jpeg_destroy_decompress(&decoder.jpeg);
 return 0;
}
void movie_close(void){running=0;if(worker>=0){sceKernelWaitThreadEnd(worker,0);sceKernelDeleteThread(worker);worker=-1;}if(file)fclose(file);file=0;free(index_table);index_table=0;free(compressed);compressed=0;for(int i=0;i<3;i++){free(frames[i]);frames[i]=0;states[i]=0;numbers[i]=-1;}front=-1;count=0;wanted=-1;}
int movie_open(const char *path){
 movie_close();error_code=0;file=fopen(path,"rb");if(!file)return 0;uint32_t h[8];int ok=fread(h,sizeof(h),1,file)==1&&h[0]==0x314a5650&&h[1]==MOVIE_WIDTH&&h[2]==MOVIE_HEIGHT&&h[3]>0&&h[3]<20000&&h[4]>=1&&h[4]<=60&&h[5]==1&&h[6]>0&&h[6]<=512*1024&&h[7]==1;
 if(!ok){movie_close();return 0;}count=h[3];fps=h[4];max_bytes=h[6];index_table=malloc(count*sizeof(*index_table));compressed=malloc(max_bytes);
 for(int i=0;i<3;i++)frames[i]=memalign(16,MOVIE_TEXTURE_BYTES);
 ok=index_table&&compressed&&frames[0]&&frames[1]&&frames[2]&&fread(index_table,sizeof(*index_table),count,file)==count;
 long total=-1;
 if(ok&&fseek(file,0,SEEK_END)==0)total=ftell(file);
 ok=ok&&total>=0;
 for(uint32_t i=0;i<count&&ok;i++){MovieIndex *p=&index_table[i];ok=p->bytes>0&&p->bytes<=max_bytes&&p->offset>=32+count*12&&(uint64_t)p->offset+p->bytes<=(uint64_t)total;}
 if(!ok){movie_close();return 0;}
 for(int i=0;i<3;i++){
  memset(frames[i],0,MOVIE_TEXTURE_BYTES);
  sceKernelDcacheWritebackRange(frames[i],MOVIE_TEXTURE_BYTES);
 }
 next_read=UINT32_MAX;wanted=0;running=1;
 worker=sceKernelCreateThread("pvzending",decode_worker,0x19,64*1024,THREAD_ATTR_USER,0);
 if(worker<0||sceKernelStartThread(worker,0,0)<0){movie_close();return 0;}
 return 1;
}
void movie_update(float seconds){if(!file||count==0)return;if(seconds>=0){int target=(int)(seconds*fps);if(target>=(int)count)target=count-1;if(target>wanted)wanted=target;}
 int shown=front>=0?numbers[front]:-1,best=-1;for(int i=0;i<3;i++)if(states[i]==2&&numbers[i]>shown&&numbers[i]<=wanted&&(best<0||numbers[i]>numbers[best]))best=i;
 if(best>=0){int old=front;states[best]=3;front=best;shown=numbers[best];if(old>=0)states[old]=0;}
 for(int i=0;i<3;i++)if(states[i]==2&&numbers[i]<=shown)states[i]=0;
}
const uint16_t *movie_pixels(void){return front>=0?frames[front]:0;}
float movie_duration(void){return fps>0?count/fps:0;}
int movie_at_end(void){return front>=0&&count&&numbers[front]==(int)count-1;}
int movie_active(void){return file!=0;}
int movie_frame(void){return front>=0?numbers[front]:-1;}
float movie_frame_lag(float audio_seconds){
 int displayed=movie_frame();
 if(displayed<0||!count)return 0;
 float frame_end=(displayed+1)/fps;
 return fmaxf(0,fminf(audio_seconds,movie_duration())-frame_end);
}
int movie_error(void){return error_code;}
int movie_selftest(FILE *out){
 int pass=0,total=0;
 #define T(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"ending %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 int opened=movie_open("data/ending.pvj");T("indexed original animation opens with bounded buffers",opened&&count>3000&&movie_duration()>150&&movie_duration()<170);
 if(opened){
  int targets[3]={0,(int)count/2,(int)count-1};
  for(int k=0;k<3;k++){
   wanted=targets[k];int received=0;
   for(int n=0;n<5000;n++){
    movie_update((targets[k]+.01f)/fps);
    if(movie_frame()==targets[k]){received=1;break;}
    if(error_code)break;
    sceKernelDelayThread(1000);
   }
   char label[80];snprintf(label,sizeof(label),"frame %d validates checksum and decodes",targets[k]);
   T(label,received&&!error_code);
   if(k==1){
    T("a frame is on time throughout its display interval",received&&movie_frame_lag((targets[k]+.5f)/fps)==0);
    T("the sync check detects a real video delay",received&&fabsf(movie_frame_lag((targets[k]+1)/fps+.125f)-.125f)<.001f);
   }
  }
  T("last frame is actually displayed",movie_at_end());
  T("holding the last frame while audio finishes does not report lag",movie_at_end()&&movie_frame_lag(movie_duration()+.5f)==0);
  movie_update(0);T("an earlier audio clock cannot rewind displayed video",movie_at_end());
 }
 movie_close();T("closing releases the decoder and all movie buffers",!movie_active()&&worker<0&&!frames[0]);
 #undef T
 fprintf(out,"ENDING RESULT %d/%d\n",pass,total);return pass==total;
}
