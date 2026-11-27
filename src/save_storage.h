#ifndef PVZ_SAVE_STORAGE_H
#define PVZ_SAVE_STORAGE_H

typedef enum {
    SAVE_ADVENTURE, SAVE_PROFILE, SAVE_PROGRESS, SAVE_CHALLENGE, SAVE_ADVENTURE_BACKUP,
    SAVE_AUDIO, SAVE_PRESENTATION, SAVE_PERFORMANCE, SAVE_NAME, SAVE_AVATAR, SAVE_CHALLENGE_VIEW,
    SAVE_MINIGAME, SAVE_COUNT
} SaveSlot;

#define SAVE_DIRECTORY "PVZPSP"

const char *save_storage_path(SaveSlot slot);
int save_storage_init(const char *device, int *copied);

#endif
