#include <pspkernel.h>
#include <pspatrac3.h>
#include <psputility.h>
#include <psputility_avmodules.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
PSP_MODULE_INFO("PvZ launcher audio check",0,1,0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(8*1024);
static unsigned short pcm[4096] __attribute__((aligned(64)));
int main(int argc,char **argv){
 if(argc&&argv[0]){char path[512];snprintf(path,sizeof(path),"%s",argv[0]);char *slash=strrchr(path,'/');if(slash){*slash=0;chdir(path);}}
 FILE *report=fopen("codec-test.txt","w");if(!report){sceKernelExitGame();return 1;}
 sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);sceUtilityLoadAvModule(PSP_AV_MODULE_ATRAC3PLUS);
 FILE *f=fopen("SND0.AT3","rb");if(!f){fprintf(report,"Missing audio: FAIL\n");fclose(report);sceKernelExitGame();return 1;}
 fseek(f,0,SEEK_END);long bytes=ftell(f);rewind(f);void *data=memalign(64,bytes);int loaded=data&&fread(data,bytes,1,f)==1;fclose(f);
 int id=loaded?sceAtracSetDataAndGetID(data,bytes):-1;fprintf(report,"PSP ATRAC accepts full SND0: %s (id %d)\n",id>=0?"PASS":"FAIL",id);
 if(id>=0){int last=0,first=0,end=0;sceAtracGetSoundSample(id,&last,&first,&end);int length=end-first+1;fprintf(report,"PSP loop length: %s (%d samples; %d..%d)\n",length==882000?"PASS":"FAIL",length,first,end);
  int setup=sceAtracSetLoopNum(id,1),done=0,decoded=0,error=0,wraps=0;u32 previous=0;unsigned long energy=0;
  for(int frame=0;frame<4000&&!done;frame++){int n=0,remaining=0;int rc=sceAtracDecodeData(id,pcm,&n,&done,&remaining);if(rc<0){error=rc;break;}decoded+=n;for(int i=0;i<n*2;i++)energy+=abs((short)pcm[i]);u32 position=0;if(sceAtracGetNextDecodePosition(id,&position)>=0){if(position<previous)wraps++;previous=position;}}
  fprintf(report,"PSP decodes audio and one loop to completion: %s (setup %d, samples %d, wraps %d, error %08x, end %d)\n",setup>=0&&!error&&done&&decoded>=1761952&&wraps>=1?"PASS":"FAIL",setup,decoded,wraps,error,done);
  fprintf(report,"Decoded stereo audio contains sound: %s (energy %lu)\n",energy>100000?"PASS":"FAIL",energy);sceAtracReleaseAtracID(id);
 }
 free(data);fclose(report);sceKernelExitGame();return 0;
}
