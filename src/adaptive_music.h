#ifndef PVZ_ADAPTIVE_MUSIC_H
#define PVZ_ADAPTIVE_MUSIC_H
#include <stdint.h>
#include <stdio.h>
enum { MUSIC_GAIN_ONE=4096 };
typedef struct {
    int scheme,burst,ticks,drums,drum_ticks,queued_phase;
    int main_gain,drum_gain,hat_gain,night_jump;
} AdaptiveMusic;
void adaptive_music_reset(AdaptiveMusic *s,int scheme);
void adaptive_music_start(AdaptiveMusic *s);
void adaptive_music_tick(AdaptiveMusic *s,int zombies,int phase);
int adaptive_music_selftest(FILE *out);
#endif
