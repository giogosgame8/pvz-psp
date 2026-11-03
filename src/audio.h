#ifndef PVZ_AUDIO_H
#define PVZ_AUDIO_H
#include <stdio.h>
#include "foley.h"
enum { MUSIC_MENU,MUSIC_DAY,MUSIC_SEEDS,MUSIC_BOWLING,MUSIC_CONVEYOR,MUSIC_NIGHT,MUSIC_POOL,MUSIC_FOG,MUSIC_ROOF,MUSIC_PUZZLE,MUSIC_BOSS,MUSIC_ZEN,MUSIC_CREDITS,MUSIC_COUNT };
enum { CUE_HUGE_WAVE=8,CUE_FINAL_WAVE,CUE_SEED_LIFT,CUE_RAIN,CUE_THUNDER,CUE_WHACK_SWING,CUE_WHACK_BONK,CUE_WHACK_PLASTIC,CUE_WHACK_METAL,CUE_DAVE,CUE_DAVE_LONG,CUE_DAVE_SCREAM,CUE_DAVE_SCREAM2,CUE_SQUASH_HMM,CUE_SQUASH_THUMP,AUDIO_EFFECTS };
int audio_init(void);
void audio_restart_music(int track);
float audio_music_seconds(int track);
void audio_play(int effect);
void audio_music(int enabled);
void audio_shutdown(void);
int audio_music_volume(void);
int audio_effects_volume(void);
void audio_set_volumes(int music,int effects);
int audio_save_settings(const char *filename);
int audio_load_settings(const char *filename);
int audio_selftest(FILE *out);
void audio_write_status(FILE *out);
int audio_select_music(int track);
void audio_queue_settings(void);
int audio_flush_settings(void);
int audio_stream_data_selftest(FILE *out);
void audio_stream_test_begin(int target_fps);
int audio_stream_test_step(FILE *out,float elapsed);
int audio_preferred_track(void);
void audio_set_preferred_track(int track);
const char *audio_track_name(int track);
#endif
