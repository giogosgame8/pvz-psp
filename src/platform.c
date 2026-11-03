#include "platform.h"
#include <stddef.h>
extern int kuKernelGetModel(void);
Platform platform_from_model(int model){
    switch(model){
        case 0:return (Platform){model,1,1,"PSP-1000"};
        case 1:return (Platform){model,1,1,"PSP-2000"};
        case 2:case 3:case 6:case 8:return (Platform){model,1,1,"PSP-3000"};
        case 4:return (Platform){model,1,1,"PSP GO"};
        case 10:return (Platform){model,1,0,"PSP STREET"};
        default:return (Platform){model,0,0,"PSP (MODEL UNKNOWN)"};
    }
}
Platform platform_detect(void){
    /* The weak import stays jr ra / nop when a CFW does not expose KUBridge.
       Never call an unresolved stub or import a kernel-only function directly. */
    const volatile uint32_t *stub=(const volatile uint32_t *)(uintptr_t)&kuKernelGetModel;
    if(stub[0]==UINT32_C(0x03e00008)&&stub[1]==0)return platform_from_model(-1);
    return platform_from_model(kuKernelGetModel());
}
PerformanceProfile platform_performance(const Platform *p){
    /* Unknown CFW/model queries take the profile that is safe on 32 MiB. */
    int low=!p->known||p->model==0;
    #ifdef LOW_MEMORY
    low=1;
    #endif
    #ifdef TEST_PSP1000_60FPS
    return (PerformanceProfile){low,p->known&&p->model==0?60:low?30:60};
    #else
    return (PerformanceProfile){low,low?30:60};
    #endif
}
int platform_mode_available(const Platform *p,int mode,uint32_t built_modes){
    if(mode<0||mode>=APP_MODE_COUNT||!(built_modes&APP_MODE_BIT(mode)))return 0;
    if(mode==APP_COOP||mode==APP_VERSUS)return p->known&&p->wlan;
    return 1;
}
PerformanceProfile platform_frame_preference(const Platform *p,int fps){
    PerformanceProfile result=platform_performance(p);
    #ifndef LOW_MEMORY
    if(p->known&&p->model==0&&(fps==30||fps==60))result.target_fps=fps;
    #else
    (void)fps;
    #endif
    return result;
}
int platform_selftest(FILE *out){
    int passed=0,total=0;Platform street=platform_from_model(10),slim=platform_from_model(1),unknown=platform_from_model(-1);
    uint32_t all=(UINT32_C(1)<<APP_MODE_COUNT)-1;
    #define T(label,test) do{int ok=(test);passed+=ok;total++;fprintf(out,"platform %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    T("Street is identified without WLAN",street.known&&!street.wlan&&street.model==10);
    T("2000 is identified with WLAN",slim.known&&slim.wlan);
    int variants=1;int models[]={2,3,6,8};for(unsigned i=0;i<sizeof(models)/sizeof(*models);i++){Platform p=platform_from_model(models[i]);variants&=p.known&&p.wlan;}
    T("all four 3000 generations have WLAN",variants);
    T("Street hides co-op even when compiled",!platform_mode_available(&street,APP_COOP,all));
    T("Street hides versus even when compiled",!platform_mode_available(&street,APP_VERSUS,all));
    T("compatible models allow implemented wireless modes",platform_mode_available(&slim,APP_COOP,all)&&platform_mode_available(&slim,APP_VERSUS,all));
    T("unfinished wireless modes stay hidden on Wi-Fi models",!platform_mode_available(&slim,APP_COOP,APP_BUILT_MODES)&&!platform_mode_available(&slim,APP_VERSUS,APP_BUILT_MODES));
    T("unknown models keep offline play available",!unknown.known&&platform_mode_available(&unknown,APP_ADVENTURE,all));
    T("unknown models do not assume radio support",!platform_mode_available(&unknown,APP_COOP,all)&&!platform_mode_available(&unknown,APP_VERSUS,all));
    T("invalid mode identifiers are rejected",!platform_mode_available(&slim,-1,all)&&!platform_mode_available(&slim,APP_MODE_COUNT,all));
    Platform fat=platform_from_model(0);PerformanceProfile fp=platform_performance(&fat),up=platform_performance(&unknown);
    #ifdef TEST_PSP1000_60FPS
    T("experimental 1000 uses 60 FPS with the smaller caches",fp.low_memory&&fp.target_fps==60);
    #else
    T("1000 chooses smaller caches and 30 FPS",fp.low_memory&&fp.target_fps==30);
    #endif
    T("unknown model safely falls back to the 32 MiB profile",up.low_memory&&up.target_fps==30);
    #ifndef LOW_MEMORY
    PerformanceProfile sp=platform_performance(&street),pp=platform_performance(&slim);
    T("Street and 2000 retain larger caches and 60 FPS",!sp.low_memory&&sp.target_fps==60&&!pp.low_memory&&pp.target_fps==60);
    #endif
    fprintf(out,"PLATFORM RESULT %d/%d\n",passed,total);return passed==total;
    #undef T
}
