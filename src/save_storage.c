#include "save_storage.h"
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>

static const char *const names[SAVE_COUNT]={
    "save.dat", "profile.dat", "progress.dat", "mode-save.dat", "adventure-backup.dat",
    "audio-settings.dat", "ui-settings.dat", "performance.dat", "player-name.dat",
    "custom-zombie.dat", "challenge-view.dat", "mini-save.dat"
};
static char paths[SAVE_COUNT][64];
static int active;

const char *save_storage_path(SaveSlot slot){
    return active?paths[slot]:names[slot];
}

static int is_missing(int code){
    return (unsigned)code==0x80010002u;
}

static int copy_legacy(const char *name,const char *stage,int *copied){
    SceIoStat stat;
    int status=sceIoGetstat(name,&stat);
    if(status<0)return is_missing(status);
    if(!FIO_S_ISREG(stat.st_mode)||stat.st_size<0||stat.st_size>1024*1024+16)return 0;
    char path[96];
    snprintf(path,sizeof(path),"%s/%s",stage,name);
    FILE *source=fopen(name,"rb");
    if(!source)return 0;
    FILE *destination=fopen(path,"wb");
    if(!destination){fclose(source);return 0;}
    unsigned char buffer[4096];
    size_t size,total=0;
    int ok=1;
    while((size=fread(buffer,1,sizeof(buffer),source))>0){
        if(fwrite(buffer,1,size,destination)!=size){ok=0;break;}
        total+=size;
    }
    if(ferror(source)||total!=(size_t)stat.st_size)ok=0;
    fclose(source);
    if(fclose(destination))ok=0;
    if(ok)(*copied)++;
    return ok;
}

int save_storage_init(const char *device,int *copied){
    if(active)return 1;
    *copied=0;
    char root[32],target[48],stage[56];
    snprintf(root,sizeof(root),"%s:/PSP/SAVEDATA",device);
    snprintf(target,sizeof(target),"%s/" SAVE_DIRECTORY,root);
    snprintf(stage,sizeof(stage),"%s-migrate",target);
    SceIoStat stat;
    int status=sceIoGetstat(target,&stat);
    if(status<0){
        if(!is_missing(status))return 0;
        if(sceIoMkdir(root,0777)<0&&(sceIoGetstat(root,&stat)<0||!FIO_S_ISDIR(stat.st_mode)))return 0;
        if(sceIoMkdir(stage,0777)<0&&(sceIoGetstat(stage,&stat)<0||!FIO_S_ISDIR(stat.st_mode)))return 0;
        for(int slot=0;slot<SAVE_COUNT;slot++)for(int backup=0;backup<2;backup++){
            char name[40],path[96];
            snprintf(name,sizeof(name),"%s%s",names[slot],backup?".bak":"");
            snprintf(path,sizeof(path),"%s/%s",stage,name);
            status=sceIoRemove(path);
            if(status<0&&!is_missing(status))return 0;
            if(!copy_legacy(name,stage,copied))return 0;
        }
        if(sceIoRename(stage,target)<0)return 0;
    }else if(!FIO_S_ISDIR(stat.st_mode))return 0;
    for(int slot=0;slot<SAVE_COUNT;slot++)snprintf(paths[slot],sizeof(paths[slot]),"%s/%s",target,names[slot]);
    active=1;
    return 1;
}
