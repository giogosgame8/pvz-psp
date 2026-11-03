#include "music.h"
#include <string.h>

static const int steps[89]={7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767};
static const int index_change[8]={-1,-1,-1,-1,2,4,6,8};
static uint16_t u16(const unsigned char *p){return p[0]|((uint16_t)p[1]<<8);}
static uint32_t u32(const unsigned char *p){return u16(p)|((uint32_t)u16(p+2)<<16);}
int music_decode_block(const unsigned char *input,int bytes,int16_t *output){
    if(bytes!=256||input[2]>88||input[3])return 0;
    int predictor=(int16_t)u16(input),index=input[2],count=1;output[0]=predictor;
    for(int byte=4;byte<bytes;byte++)for(int half=0;half<2;half++){
        int code=(input[byte]>>(half*4))&15;
        int step=steps[index],difference=step>>3;
        if(code&4)difference+=step;if(code&2)difference+=step>>1;if(code&1)difference+=step>>2;
        predictor+=(code&8)?-difference:difference;
        if(predictor>32767)predictor=32767;if(predictor< -32768)predictor=-32768;
        index+=index_change[code&7];if(index<0)index=0;if(index>88)index=88;
        output[count++]=(int16_t)predictor;
    }return count;
}
void music_reader_close(MusicReader *r){if(r->file)fclose(r->file);memset(r,0,sizeof(*r));}
int music_reader_open(MusicReader *r,const char *path){
    memset(r,0,sizeof(*r));r->file=fopen(path,"rb");if(!r->file)return 0;
    setvbuf(r->file,(char*)r->io_buffer,_IOFBF,sizeof(r->io_buffer));
    unsigned char header[20];int format_ok=0,reason=1;
    if(fseek(r->file,0,SEEK_END))goto bad;long length=ftell(r->file);
    if(length<60||length>16*1024*1024||fseek(r->file,0,SEEK_SET))goto bad;
    reason=2;if(fread(header,12,1,r->file)!=1||memcmp(header,"RIFF",4)||memcmp(header+8,"WAVE",4)||u32(header+4)!=(uint32_t)length-8)goto bad;
    for(uint32_t at=12;at+8<=(uint32_t)length;){
        reason=3;if(fseek(r->file,at,SEEK_SET)||fread(header,8,1,r->file)!=1)goto bad;
        uint32_t size=u32(header+4),start=at+8;if(size>(uint32_t)length-start)goto bad;
        if(!memcmp(header,"fmt ",4)){
            if(size<20||fread(header,20,1,r->file)!=1)goto bad;
            format_ok=u16(header)==17&&u16(header+2)==1&&u32(header+4)==22050&&u16(header+12)==256&&u16(header+14)==4&&u16(header+16)>=2&&u16(header+18)==505;
        }else if(!memcmp(header,"fact",4)){
            if(size<4||fread(header,4,1,r->file)!=1)goto bad;r->total_samples=u32(header);
        }else if(!memcmp(header,"data",4)){r->data_start=start;r->data_bytes=size;}
        at=start+size+(size&1);
    }
    reason=4;if(!format_ok||!r->data_start||!r->data_bytes||r->data_bytes%256||!r->total_samples||r->total_samples>(r->data_bytes/256)*505||r->total_samples<=((r->data_bytes/256)-1)*505)goto bad;
    if(fseek(r->file,r->data_start,SEEK_SET))goto bad;return 1;
bad:music_reader_close(r);r->error=reason;return 0;
}
int music_reader_read(MusicReader *r,int16_t *output,int samples){
    int written=0;if(!r->file||r->error||samples<=0)return 0;
    while(written<samples){
        if(r->position==r->total_samples){
            if(fseek(r->file,r->data_start,SEEK_SET)){r->error=1;break;}
            r->position=0;r->block_bytes=0;r->decoded_at=r->decoded_count=0;r->loops++;
        }
        if(r->decoded_at==r->decoded_count){
            unsigned char block[256];
            if(r->block_bytes+256>r->data_bytes||fread(block,256,1,r->file)!=1||(r->decoded_count=music_decode_block(block,256,r->decoded))!=505){r->error=1;break;}
            r->block_bytes+=256;r->decoded_at=0;
        }
        int count=r->decoded_count-r->decoded_at;if(count>samples-written)count=samples-written;
        if((uint32_t)count>r->total_samples-r->position)count=r->total_samples-r->position;
        memcpy(output+written,r->decoded+r->decoded_at,count*sizeof(*output));
        written+=count;r->decoded_at+=count;r->position+=count;
    }return written;
}
uint32_t music_fingerprint(const int16_t *samples,int count){
    uint32_t hash=2166136261u;for(int i=0;i<count;i++){uint16_t s=samples[i];hash=(hash^(s&255))*16777619u;hash=(hash^(s>>8))*16777619u;}return hash;
}
static void put32(FILE *f,uint32_t value){unsigned char p[4]={value,value>>8,value>>16,value>>24};fwrite(p,4,1,f);}
static int fixture(const char *path,int samples,int corrupt){
    FILE *f=fopen(path,"wb");if(!f)return 0;
    fwrite("RIFF",4,1,f);put32(f,320);fwrite("WAVEJUNK",8,1,f);put32(f,3);fwrite("abc\0",4,1,f);
    fwrite("fmt ",4,1,f);put32(f,20);unsigned char format[20]={17,0,1,0,34,86,0,0,0,0,0,0,0,1,4,0,2,0,249,1};fwrite(format,20,1,f);
    fwrite("fact",4,1,f);put32(f,4);put32(f,samples);fwrite("data",4,1,f);put32(f,256);
    unsigned char block[256];memset(block,0x11,sizeof(block));block[0]=232;block[1]=3;block[2]=corrupt?89:0;block[3]=0;fwrite(block,256,1,f);fclose(f);return 1;
}
int music_selftest(FILE *out){
    int pass=0,total=0;unsigned char block[256]={0};int16_t decoded[505],looped[1008];MusicReader r={0};
    #define CHECK(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"music %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    memset(block+4,0x11,252);block[0]=232;block[1]=3;
    CHECK("IMA positive nibbles decode in low/high order",music_decode_block(block,256,decoded)==505&&decoded[0]==1000&&decoded[1]==1001&&decoded[504]==1504);
    memset(block+4,0x99,252);CHECK("IMA negative nibbles preserve sign",music_decode_block(block,256,decoded)==505&&decoded[1]==999&&decoded[504]==496);
    memset(block+4,0x77,252);CHECK("IMA predictor saturates instead of wrapping",music_decode_block(block,256,decoded)==505&&decoded[504]==32767);
    block[2]=89;CHECK("invalid ADPCM step index is rejected",music_decode_block(block,256,decoded)==0);
    int made=fixture("selftest-music.wav",501,0),opened=made&&music_reader_open(&r,"selftest-music.wav");
    CHECK("WAV reader accepts odd-sized ancillary chunk",opened&&r.total_samples==501);
    int read=opened?music_reader_read(&r,looped,1008):0;
    CHECK("stream loops at fact length without padded samples",read==1008&&r.loops==2&&looped[500]==1500&&looped[501]==1000&&looped[1002]==1000);
    music_reader_close(&r);made=fixture("selftest-music.wav",501,1);opened=made&&music_reader_open(&r,"selftest-music.wav");
    CHECK("bad compressed block stops bounded streaming safely",opened&&music_reader_read(&r,looped,10)==0&&r.error);music_reader_close(&r);
    FILE *f=fopen("selftest-music.wav","wb");if(f){fwrite("RIFF",4,1,f);fclose(f);}
    CHECK("truncated WAV is rejected",!music_reader_open(&r,"selftest-music.wav"));remove("selftest-music.wav");
    fprintf(out,"MUSIC RESULT %d/%d\n",pass,total);
    #undef CHECK
    return pass==total;
}
