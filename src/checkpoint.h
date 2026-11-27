#ifndef PVZ_CHECKPOINT_H
#define PVZ_CHECKPOINT_H
#include "profile.h"
#include "progress.h"
int checkpoint_init(void);
int checkpoint_queue(const Profile*,const char*,const Game*,const char*,const Progress*,const char*);
typedef enum { CHECKPOINT_IDLE, CHECKPOINT_WRITING, CHECKPOINT_SAVED, CHECKPOINT_FAILED } CheckpointResult;
/* Returns 1 for an accepted save, 0 while the worker is busy, or -1 if unavailable. */
int checkpoint_queue_observed(const Profile*,const char*,const Game*,const char*,const Progress*,const char*);
CheckpointResult checkpoint_observed_result(void);
/* Like a save, a load runs after earlier queued work. Only one can be held. */
int checkpoint_queue_load(const char *path);
/* The returned board belongs to the worker slot until checkpoint_release_load. */
const Game *checkpoint_loaded_game(int *pending,int *recovered);
void checkpoint_release_load(void);
int checkpoint_busy(void);
int checkpoint_flush(void);
int checkpoint_take_error(void);
void checkpoint_shutdown(void);
int checkpoint_selftest(FILE*);
void checkpoint_test_delay(unsigned milliseconds);
#endif
