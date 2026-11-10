#ifndef PVZ_SAVE_FILE_H
#define PVZ_SAVE_FILE_H
#include <stdio.h>
#include <stdint.h>

/* Write/close the new file before touching either committed copy. An invalid
   primary must never replace a good backup during recovery. */
static int save_container_valid(const char *path,uint32_t magic,uint32_t newest){
    FILE *f=fopen(path,"rb");if(!f)return 0;uint32_t h[4],value=2166136261u;
    int ok=fread(h,sizeof(h),1,f)==1&&h[0]==magic&&h[1]>0&&h[1]<=newest&&h[2]>0&&h[2]<=1024*1024;
    unsigned char block[1024];uint32_t left=ok?h[2]:0;
    while(left&&ok){size_t count=left>sizeof(block)?sizeof(block):left;ok=fread(block,count,1,f)==1;
        if(ok){for(size_t n=0;n<count;n++)value=(value^block[n])*16777619u;left-=count;}}
    ok=ok&&value==h[3]&&fgetc(f)==EOF;fclose(f);return ok;
}
static int save_file_replace(const char *path,const uint32_t header[4],const void *data){
    char temp[512],backup[512];
    if(snprintf(temp,sizeof(temp),"%s.tmp",path)>=(int)sizeof(temp)||snprintf(backup,sizeof(backup),"%s.bak",path)>=(int)sizeof(backup))return 0;
    for(int copy=0;copy<2;copy++){
        FILE *existing=fopen(copy?backup:path,"rb");uint32_t old_header[4];
        int newer=existing&&fread(old_header,sizeof(old_header),1,existing)==1&&old_header[0]==header[0]&&old_header[1]>header[1];
        if(existing)fclose(existing);if(newer)return 0;
    }
    FILE *f=fopen(temp,"wb");if(!f)return 0;
    int ok=fwrite(header,16,1,f)==1&&fwrite(data,header[2],1,f)==1;
    if(fflush(f))ok=0;if(fclose(f))ok=0;
    if(!ok||!save_container_valid(temp,header[0],header[1])){remove(temp);return 0;}
    int rotated=0;
    if(save_container_valid(path,header[0],header[1])){
        /* If backup removal fails (e.g. read-only), keep the primary intact. */
        FILE *b=fopen(backup,"rb");int exists=b!=0;if(b)fclose(b);
        if(exists&&remove(backup)){remove(temp);return 0;}
        if(rename(path,backup)){remove(temp);return 0;}rotated=1;
    }else{
        FILE *p=fopen(path,"rb");int exists=p!=0;if(p)fclose(p);
        if(exists&&remove(path)){remove(temp);return 0;}
    }
    if(rename(temp,path)){if(rotated)rename(backup,path);remove(temp);return 0;}
    return 1;
}
#endif
