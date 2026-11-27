#ifndef PVZ_RECORD_FILE_H
#define PVZ_RECORD_FILE_H
#include <errno.h>
#include <stdio.h>

typedef int (*RecordValidator)(const char *);
typedef int (*RecordReader)(FILE *,void *);

/* A damaged or absent record can be repaired. A read error leaves its
   contents unknown, so it must abort a replacement. */
static inline int record_inspect(const char *path,void *record,RecordReader read_record){
    FILE *file=fopen(path,"rb");
    if(!file)return errno==ENOENT?0:-1;
    int valid=read_record(file,record);
    if(ferror(file))valid=-1;
    if(fclose(file))valid=-1;
    return valid;
}

static inline int record_load(const char *path,void *record,RecordReader read_record){
    if(record_inspect(path,record,read_record)>0)return 1;
    char backup[160];
    int length=snprintf(backup,sizeof(backup),"%s.bak",path);
    return length>=0&&length<(int)sizeof(backup)&&record_inspect(backup,record,read_record)>0;
}

/* Both callers have already written and verified the temporary file. */
static inline int commit_verified_save(const char *path,const char *temp,const char *backup,int primary){
    if(primary>0){
        if(remove(backup)&&errno!=ENOENT)return 0;
        if(rename(path,backup))return 0;
    }else if(remove(path)&&errno!=ENOENT)return 0;
    if(!rename(temp,path))return 1;
    if(primary>0)rename(backup,path);
    return 0;
}

static inline int record_replace(const char *path,const void *data,size_t bytes,RecordValidator valid){
    char temp[160],backup[160];
    if(snprintf(temp,sizeof(temp),"%s.tmp",path)>=(int)sizeof(temp)||
       snprintf(backup,sizeof(backup),"%s.bak",path)>=(int)sizeof(backup))return 0;
    FILE *file=fopen(temp,"wb");
    if(!file)return 0;
    int ok=fwrite(data,1,bytes,file)==bytes;
    if(fclose(file))ok=0;
    if(ok&&valid(temp)>0){
        int primary=valid(path);
        if(primary>=0&&(primary==0||valid(backup)>=0)&&
           commit_verified_save(path,temp,backup,primary))return 1;
    }
    remove(temp);
    return 0;
}
#endif
