#include "textdb.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
static uint32_t hash(const char *p){uint32_t n=2166136261u;while(*p)n=(n^(unsigned char)*p++)*16777619u;return n;}
int textdb_get(const char *key,char *text,size_t capacity){
 if(!capacity)return 0;text[0]=0;FILE *f=fopen("data/strings.bin","rb");if(!f)return 0;uint32_t h[2],row[2],sizes[2],wanted=hash(key);int ok=fread(h,sizeof(h),1,f)==1&&h[0]==0x32535650&&h[1]<10000;int lo=0,hi=ok?(int)h[1]:0;
 while(lo<hi){int mid=lo+(hi-lo)/2;if(fseek(f,8+mid*8,SEEK_SET)||fread(row,8,1,f)!=1){ok=0;break;}if(row[0]<wanted)lo=mid+1;else hi=mid;}
 char name[256];ok=ok&&lo<(int)h[1]&&!fseek(f,8+lo*8,SEEK_SET)&&fread(row,8,1,f)==1&&row[0]==wanted&&row[1]>=8+h[1]*8&&row[1]<4*1024*1024&&!fseek(f,row[1],SEEK_SET)&&fread(sizes,8,1,f)==1&&sizes[0]<sizeof(name)&&sizes[1]<65536;
 if(ok){ok=fread(name,sizes[0],1,f)==1;name[sizes[0]]=0;ok=ok&&!strcmp(name,key);if(ok){size_t n=sizes[1]<capacity-1?sizes[1]:capacity-1;ok=fread(text,1,n,f)==n;text[n]=0;}}
 fclose(f);if(!ok)text[0]=0;return ok;
}
int textdb_frame(const char *key){char full[256],value[32];snprintf(full,sizeof(full),"FRAME_%s",key);return textdb_get(full,value,sizeof(value))?atoi(value):-1;}
