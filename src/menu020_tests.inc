static int menu020_selftest(FILE *out){
 Game saved_game=game;Profile saved_player=player;int saved_screen=screen,saved_cursor=collection_cursor,saved_row=menu_volume_row,saved_style=classic_dancer,saved_bars=health_bars,saved_dirty=ui_settings_dirty;const char *saved_path=ui_settings_path;ui_settings_path="selftest-menu020.dat";int pass=0,total=0;
 #define N(label,test) do{int ok=(test);total++;pass+=ok;fprintf(out,"menu020 %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 SceCtrlData pad;memset(&pad,0,sizeof(pad));pad.Lx=pad.Ly=128;
 #define NK(button) do{pad.Buttons=0;input(&pad,.016f);pad.Buttons=button;input(&pad,.016f);pad.Buttons=0;input(&pad,.016f);}while(0)
 for(int style=0;style<2;style++){classic_dancer=style;health_bars=3;int wrote=ui_settings_save();classic_dancer=!style;health_bars=0;N("both dancer choices persist with health-bar preferences",wrote&&ui_settings_load()&&classic_dancer==style&&health_bars==3);}
 FILE *f=fopen(ui_settings_path,"wb");uint32_t old[3]={0x49555650,1,2};int wrote=f&&fwrite(old,sizeof(old),1,f)==1;if(f)fclose(f);classic_dancer=1;N("old preferences import with Disco as the default",wrote&&ui_settings_load()&&health_bars==2&&!classic_dancer);
 f=fopen(ui_settings_path,"wb");uint32_t bad[4]={0x49555650,2,1,2};if(f){fwrite(bad,sizeof(bad),1,f);fclose(f);}classic_dancer=1;N("invalid dancer choices cannot replace a valid preference",!ui_settings_load()&&classic_dancer==1);
 classic_dancer=0;screen=4;menu_volume_row=6;help=gallery=0;previous_buttons=0;NK(PSP_CTRL_CROSS);N("X selects Michael Jackson in Options",classic_dancer==1&&screen==4);NK(PSP_CTRL_LEFT);N("D-pad can return to Disco",!classic_dancer);NK(PSP_CTRL_RIGHT);N("D-pad can select Michael Jackson",classic_dancer==1);
 N("changing the dancer does not reset Adventure",!memcmp(&game,&saved_game,sizeof(game)));
 game_start_level(&game,9,2020);int id=game_spawn_zombie(&game,2,Z_DANCER,330);Zombie *z=&game.zombies[id];
 N("Michael Jackson uses his original walking bank",zombie_visible_clip(z,id,0)==&assets[A_CLASSIC_DANCER]);z->eating=1;
 N("Michael Jackson uses his original eating bank",zombie_visible_clip(z,id,0)==&assets[A_CLASSIC_DANCER_EAT]);z->hp=100;
 N("head loss retains the classic body instead of replacing its outfit",zombie_visible_clip(z,id,0)==&damaged[A_CLASSIC_DANCER_EAT][1]);z->hp=500;z->eating=0;z->age=.5f;
 N("walking spans the whole classic clip with its own source FPS",fabsf(zombie_visible_time(z,&assets[A_CLASSIC_DANCER])-assets[A_CLASSIC_DANCER].frames/(2*assets[A_CLASSIC_DANCER].fps))<.001f);
 N("the animation gallery follows the saved dancer choice",gallery_zombie(Z_DANCER,2)==&assets[A_CLASSIC_DANCER_DEATH]);classic_dancer=0;
 N("Disco remains available without modifying zombie behavior",zombie_visible_clip(z,id,0)==&assets[A_DANCER]&&z->kind==Z_DANCER);
 profile_init(&player);player.highest=1;screen=11;collection_cursor=0;collection_normalize();N("fresh Extras selects Back and has no locked selection",collection_cursor==4&&!collection_available(0)&&!collection_available(1)&&!collection_available(2)&&!collection_available(3));
 int stays_back=1;for(int n=0;n<20;n++){collection_move(n%2?1:-1);stays_back&=collection_cursor==4;}N("D-pad cannot hover locked Extras entries",stays_back);
 player.highest=10;collection_cursor=4;collection_move(-1);N("the first zombie note unlocks its Extras entry",collection_cursor==3&&collection_available(3)&&!collection_available(1));collection_move(1);N("Extras wraps between only available entries",collection_cursor==4);
 player.highest=15;N("the Almanac appears after its Adventure reward",collection_available(1)&&!collection_available(0));player.highest=25;N("the shop appears after earning the car keys",collection_available(0)&&!collection_available(2));player.highest=45;int all=1;for(int n=0;n<5;n++)all&=collection_available(n);N("the watering can completes the five-entry Extras menu",all);
 N("original menu motion and painter-order data loaded",menu_motion_ready&&menu_layer_count>0&&menu_layer_count<=192);
 int moving[3]={0},gated[4]={0};for(unsigned i=0;i<menu_layer_count;i++){MenuLayer *layer=&menu_layers[i];if(layer->count>1)for(unsigned k=7;k<layer->count*7;k++)if(fabsf(layer->keys[k]-layer->keys[k%7])>.001f)moving[layer->page]=1;if(layer->group)gated[layer->group-1]=1;}
 N("all three menu pages contain original moving layers",moving[0]&&moving[1]&&moving[2]);N("all four unlock-dependent artworks have separate gates",gated[0]&&gated[1]&&gated[2]&&gated[3]);
 player.highest=1;int hidden=1;for(unsigned i=0;i<menu_layer_count;i++)if(menu_layers[i].group)hidden&=!menu_layer_visible(&menu_layers[i],2,0);N("locked artwork is omitted from the actual drawing path",hidden);
 N("the crow has flight, landing, speech and three idle clips",assets[A_CROW_FLY].loaded&&assets[A_CROW_LAND].loaded&&assets[A_CROW_TALK].loaded&&assets[A_CROW_IDLE].loaded&&assets[A_CROW_IDLE2].loaded&&assets[A_CROW_IDLE3].loaded);
 unsigned original_layers=menu_layer_count;f=fopen("selftest-menu020-motion.bin","wb");if(f){fwrite("PVMU",4,1,f);fclose(f);}N("truncated menu motion fails cleanly",!menu_motion_load("selftest-menu020-motion.bin")&&!menu_motion_data&&!menu_layer_count);N("menu motion can reload after a rejected file",menu_motion_load("data/menu-motion.bin")&&menu_layer_count==original_layers);remove("selftest-menu020-motion.bin");
 remove(ui_settings_path);ui_settings_path=saved_path;classic_dancer=saved_style;health_bars=saved_bars;ui_settings_dirty=saved_dirty;game=saved_game;player=saved_player;screen=saved_screen;collection_cursor=saved_cursor;menu_volume_row=saved_row;previous_buttons=0;volume_repeat=0;
 #undef NK
 #undef N
 fprintf(out,"MENU020 RESULT %d/%d\n",pass,total);return pass==total;
}
