#ifndef PVZ_LEVEL_RANDOM_H
#define PVZ_LEVEL_RANDOM_H
/* MT19937 for deterministic level-only choices, separate from combat RNG. */
typedef struct {uint32_t state[624];int at;} LevelRandom;
static void level_random_seed(LevelRandom *r,uint32_t seed){r->state[0]=seed;for(uint32_t n=1;n<624;n++)r->state[n]=1812433253u*(r->state[n-1]^(r->state[n-1]>>30))+n;r->at=624;}
static uint32_t level_random_next(LevelRandom *r){
 if(r->at==624){for(int n=0;n<624;n++){uint32_t bits=(r->state[n]&0x80000000u)|(r->state[(n+1)%624]&0x7fffffffu);r->state[n]=r->state[(n+397)%624]^(bits>>1)^((bits&1)?0x9908b0dfu:0);}r->at=0;}
 uint32_t x=r->state[r->at++];x^=x>>11;x^=(x<<7)&0x9d2c5680u;x^=(x<<15)&0xefc60000u;return x^(x>>18);
}
static unsigned level_random_below(LevelRandom *r,unsigned count){uint32_t limit=count-1,mask=limit;mask|=mask>>1;mask|=mask>>2;mask|=mask>>4;mask|=mask>>8;mask|=mask>>16;uint32_t x;do{x=level_random_next(r)&mask;}while(x>limit);return x;}
#endif
