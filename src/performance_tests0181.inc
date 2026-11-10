static int performance_settings_selftest(FILE *out){
 Platform old_hardware=hardware;int old_fps=psp1000_fps,old_screen=screen,old_row=menu_volume_row,old_dirty=performance_dirty;const char *old_path=performance_path;FILE *old_log=performance_log;Game original=game;int pass=0,total=0;performance_path="selftest-performance.dat";performance_log=0;
 #define P(label,test) do{int ok=(test);total++;pass+=ok;fprintf(out,"pacing %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 #define KEY(button) do{pad.Buttons=0;input(&pad,.016f);pad.Buttons=button;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);}while(0)
 Platform fat=platform_from_model(0),unknown=platform_from_model(-1),street=platform_from_model(10);PerformanceProfile fp=platform_frame_preference(&fat,60),up=platform_frame_preference(&unknown,60),sp=platform_frame_preference(&street,30);
 #ifndef LOW_MEMORY
 P("60 FPS on model 0 retains the small memory profile",fp.low_memory&&fp.target_fps==60);
 P("Street ignores a saved 1000-only 30 FPS preference",!sp.low_memory&&sp.target_fps==60);
 #else
 P("forced low-memory diagnostics retain 30 FPS",fp.low_memory&&fp.target_fps==30&&sp.low_memory&&sp.target_fps==30);
 #endif
 P("unknown hardware never enables the experimental target",up.low_memory&&up.target_fps==30);
 psp1000_fps=60;int wrote=performance_save();psp1000_fps=30;P("60 FPS preference survives reopening",wrote&&performance_load()&&psp1000_fps==60);
 FILE *f=fopen(performance_path,"r+b");if(f){fseek(f,12,SEEK_SET);fputc(0,f);fclose(f);}psp1000_fps=30;P("damaged preference cannot enable 60 FPS",!performance_load()&&psp1000_fps==30);
 f=fopen(performance_path,"wb");if(f){fputs("short",f);fclose(f);}P("truncated settings preserve the safe setting",!performance_load()&&psp1000_fps==30);
 #ifndef LOW_MEMORY
 SceCtrlData pad;memset(&pad,0,sizeof(pad));pad.Lx=pad.Ly=128;hardware=fat;psp1000_fps=30;performance_apply();screen=4;menu_volume_row=7;previous_buttons=0;help=gallery=0;
 P("1000 Options exposes a separate frame-rate row",options_count()==9&&options_back()==8);
 KEY(PSP_CTRL_RIGHT);P("Right enables 60 FPS immediately",psp1000_fps==60&&performance.target_fps==60&&screen==4);
 KEY(PSP_CTRL_CROSS);P("X restores the 30 FPS fallback immediately",psp1000_fps==30&&performance.target_fps==30);
 KEY(PSP_CTRL_LEFT);P("Left cannot cycle below 30 FPS",psp1000_fps==30);
 KEY(PSP_CTRL_RIGHT);psp1000_fps=30;P("the controller-selected 60 FPS setting is saved",performance_load()&&psp1000_fps==60);
 P("changing render targets never resets the current level",!memcmp(&game,&original,sizeof(game)));
 KEY(PSP_CTRL_DOWN);P("Down reaches Back after the new option",menu_volume_row==8);options_return=0;KEY(PSP_CTRL_CROSS);P("Back still closes Options",screen==0);
 hardware=street;P("other PSP models keep the existing compact Options",options_count()==8&&options_back()==7);
 #endif
 remove(performance_path);performance_path=old_path;performance_log=old_log;hardware=old_hardware;psp1000_fps=old_fps;performance_dirty=old_dirty;performance_apply();screen=old_screen;menu_volume_row=old_row;previous_buttons=0;volume_repeat=0;
 #undef KEY
 #undef P
 fprintf(out,"PACING RESULT %d/%d\n",pass,total);return pass==total;
}
