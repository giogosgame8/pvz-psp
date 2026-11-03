#ifndef PVZ_PLATFORM_H
#define PVZ_PLATFORM_H
#include <stdint.h>
#include <stdio.h>
typedef struct {int model,known,wlan;const char *name;} Platform;
typedef struct {int low_memory,target_fps;} PerformanceProfile;
enum { APP_ADVENTURE,APP_REPLAY,APP_MINIGAMES,APP_NOTES,APP_SHOP,APP_ALMANAC,APP_COOP,APP_VERSUS,APP_SURVIVAL,APP_PUZZLES,APP_GARDEN,APP_TREE,APP_CREDITS,APP_MODE_COUNT };
#define APP_MODE_BIT(mode) (UINT32_C(1)<<(mode))
/* Add multiplayer bits only when their game and transport implementations ship. */
#define APP_BUILT_MODES (((UINT32_C(1)<<APP_COOP)-1)|APP_MODE_BIT(APP_SURVIVAL)|APP_MODE_BIT(APP_PUZZLES)|APP_MODE_BIT(APP_GARDEN)|APP_MODE_BIT(APP_TREE)|APP_MODE_BIT(APP_CREDITS))
Platform platform_from_model(int model);
Platform platform_detect(void);
PerformanceProfile platform_performance(const Platform *platform);
PerformanceProfile platform_frame_preference(const Platform *platform,int fps);
int platform_mode_available(const Platform *p,int mode,uint32_t built_modes);
int platform_selftest(FILE *out);
#endif
