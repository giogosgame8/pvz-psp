#ifndef PVZ_TEXTDB_H
#define PVZ_TEXTDB_H
#include <stddef.h>
int textdb_get(const char *key,char *text,size_t capacity);
int textdb_frame(const char *key);
#endif
