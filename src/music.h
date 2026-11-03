#ifndef PVZ_MUSIC_H
#define PVZ_MUSIC_H
#include <stdint.h>
#include <stdio.h>
/* Prepared mono 22050 Hz IMA WAV, with independently decodable 256-byte blocks. */
typedef struct {
    FILE *file;
    uint32_t data_start,data_bytes,total_samples,position,block_bytes,loops;
    int decoded_at,decoded_count,error;
    int16_t decoded[505];
    unsigned char io_buffer[4096];
} MusicReader;
int music_decode_block(const unsigned char *input,int bytes,int16_t *output);
int music_reader_open(MusicReader *reader,const char *path);
int music_reader_read(MusicReader *reader,int16_t *output,int samples);
void music_reader_close(MusicReader *reader);
uint32_t music_fingerprint(const int16_t *samples,int count);
int music_selftest(FILE *out);
#endif
