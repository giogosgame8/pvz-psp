#ifndef PVZ_GARDEN_H
#define PVZ_GARDEN_H
#include "profile.h"
void garden_initialize(Profile*,int64_t now);
int garden_add(Profile*,int kind,int64_t now);
int garden_count(const Profile*,int garden);
int garden_find(const Profile*,int garden,int slot);
int garden_need(const Profile*,int index,int64_t now);
int garden_use(Profile*,int index,int tool,int64_t now);
int garden_move(Profile*,int index,int garden,int slot);
int garden_sell(Profile*,int index);
int garden_sell_price(const Profile*,int index);
void garden_step(Profile*,int garden,float dt,int64_t now);
int garden_collect(Profile*);
int garden_tree_feed(Profile*);
int garden_selftest(FILE*);
void garden_position(int garden,int slot,float *x,float *y);
#endif
