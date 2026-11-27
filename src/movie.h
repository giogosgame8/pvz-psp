#ifndef PVZ_MOVIE_H
#define PVZ_MOVIE_H
#include <stdint.h>
#include <stdio.h>
int movie_open(const char *path);
void movie_close(void);
void movie_update(float seconds);
const uint16_t *movie_pixels(void);
float movie_duration(void);
int movie_at_end(void);
int movie_active(void);
int movie_frame(void);
float movie_frame_lag(float audio_seconds);
int movie_error(void);
int movie_selftest(FILE *out);
#endif
