#ifndef PVZ_CHECKPOINT_H
#define PVZ_CHECKPOINT_H
#include "profile.h"
#include "progress.h"
int checkpoint_init(void);
int checkpoint_queue(const Profile*,const char*,const Game*,const char*,const Progress*,const char*);
int checkpoint_busy(void);
int checkpoint_flush(void);
int checkpoint_take_error(void);
void checkpoint_shutdown(void);
int checkpoint_selftest(FILE*);
void checkpoint_test_delay(unsigned milliseconds);
#endif
