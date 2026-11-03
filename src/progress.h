#ifndef PVZ_PROGRESS_H
#define PVZ_PROGRESS_H
#include "game.h"
typedef struct { uint64_t played,completed; } Progress;
void progress_import(Progress *p,const Game *adventure);
void progress_visit(Progress *p,int level,int won);
int progress_played(const Progress *p,int level);
int progress_save(const Progress *p,const char *filename);
int progress_load(Progress *p,const char *filename);
int progress_selftest(FILE *out);
#endif
