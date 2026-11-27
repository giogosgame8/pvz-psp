#ifndef PVZ_SAVE_FILE_H
#define PVZ_SAVE_FILE_H
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include "record_file.h"

/* Write/close the new file before touching either committed copy. An invalid
   primary must never replace a good backup during recovery. */
typedef int (*SavePayloadCheck)(FILE *,const uint32_t *,void *);
static int save_checked_container_valid(const char *path,uint32_t magic,uint32_t newest,SavePayloadCheck payload_check,void *context){
    FILE *f=fopen(path,"rb");if(!f)return errno==ENOENT?0:-1;uint32_t h[4],value=2166136261u;
    int ok=fread(h,sizeof(h),1,f)==1&&h[0]==magic&&h[1]>0&&h[1]<=newest&&h[2]>0&&h[2]<=1024*1024;
    if(ok&&payload_check)ok=payload_check(f,h,context);
    else if(ok){
        unsigned char block[1024];uint32_t left=h[2];
        while(left&&ok){size_t count=left>sizeof(block)?sizeof(block):left;ok=fread(block,count,1,f)==1;
            if(ok){for(size_t n=0;n<count;n++)value=(value^block[n])*16777619u;left-=count;}}
        ok=ok&&value==h[3];
    }
    if(ok>0)ok=fgetc(f)==EOF;
    if(ferror(f))ok=-1;
    if(fclose(f))ok=-1;
    return ok;
}
static inline int save_container_valid(const char *path,uint32_t magic,uint32_t newest){
    return save_checked_container_valid(path,magic,newest,NULL,NULL)>0;
}
static int save_existing_header_safe(const char *path,const uint32_t header[4]){
    FILE *existing=fopen(path,"rb");
    if(!existing)return errno==ENOENT;
    uint32_t old[4];
    int newer=fread(old,sizeof(old),1,existing)==1&&old[0]==header[0]&&old[1]>header[1];
    int readable=!ferror(existing);
    return !fclose(existing)&&readable&&!newer;
}
static int save_file_replace(const char *path,const uint32_t header[4],const void *data,SavePayloadCheck payload_check,void *context){
    char temp[512],backup[512];
    if(snprintf(temp,sizeof(temp),"%s.tmp",path)>=(int)sizeof(temp)||snprintf(backup,sizeof(backup),"%s.bak",path)>=(int)sizeof(backup))return 0;
    if(!save_existing_header_safe(path,header)||!save_existing_header_safe(backup,header))return 0;
    FILE *f=fopen(temp,"wb");if(!f)return 0;
    int ok=fwrite(header,16,1,f)==1&&fwrite(data,header[2],1,f)==1;
    if(fflush(f))ok=0;if(fclose(f))ok=0;
    if(ok&&save_checked_container_valid(temp,header[0],header[1],payload_check,context)>0){
        int primary=save_checked_container_valid(path,header[0],header[1],payload_check,context);
        /* A failed inspection cannot establish whether rotation is safe. */
        if(primary>=0&&commit_verified_save(path,temp,backup,primary))return 1;
    }
    remove(temp);
    return 0;
}
#endif
