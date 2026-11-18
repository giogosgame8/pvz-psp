#ifndef PVZ_RECORD_FILE_H
#define PVZ_RECORD_FILE_H
#include <stdio.h>

typedef int (*RecordValidator)(const char *);

static inline int record_replace(const char *path,const void *data,size_t bytes,RecordValidator valid){
    char temp[160],backup[160];
    if(snprintf(temp,sizeof(temp),"%s.tmp",path)>=(int)sizeof(temp)||
       snprintf(backup,sizeof(backup),"%s.bak",path)>=(int)sizeof(backup))return 0;
    FILE *file=fopen(temp,"wb");
    if(!file)return 0;
    int ok=fwrite(data,1,bytes,file)==bytes;
    if(fclose(file))ok=0;
    if(!ok||!valid(temp)){remove(temp);return 0;}
    int rotate=valid(path);
    if(rotate){
        file=fopen(backup,"rb");
        if(file){fclose(file);if(remove(backup)){remove(temp);return 0;}}
        if(rename(path,backup)){remove(temp);return 0;}
    }else{
        file=fopen(path,"rb");
        if(file){fclose(file);if(remove(path)){remove(temp);return 0;}}
    }
    if(!rename(temp,path))return 1;
    if(rotate)rename(backup,path);
    remove(temp);
    return 0;
}
#endif
