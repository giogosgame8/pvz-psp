#ifndef PVZ_FOLEY_H
#define PVZ_FOLEY_H
#include <stdint.h>
#include <stdio.h>
#include "foley_defs.h"
int foley_init(void);
void foley_shutdown(void);
void foley_play(int group);
void foley_sample(int sample);
void foley_loop(int group,int enabled);
void foley_stop(int group);
void foley_pause(int paused);
void foley_mix(int16_t *stereo,unsigned frames,int volume,int music_volume);
void foley_report(FILE *out);
int foley_selftest(FILE *out);
int foley_data_selftest(FILE *out);
int foley_ok(void);
int foley_runtime_selftest(FILE*);
#endif
