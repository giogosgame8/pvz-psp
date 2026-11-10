static int ending_review_captured,ending_review_frames,ending_review_last=-1,ending_review_ticks;static float ending_review_max_lag;
static void ending_review_frame(void){int frame=movie_frame();if(frame!=ending_review_last){ending_review_frames++;ending_review_last=frame;}if(frame>=0){float lag=ending_clock-frame/24.0f;if(lag>ending_review_max_lag)ending_review_max_lag=lag;}
 if((ending_review_ticks++%150)==0){FILE *f=fopen("ending-progress.txt","a");if(f){fprintf(f,"tick %d, audio %.3f, frame %d, error %d, free %u\n",ending_review_ticks,ending_clock,frame,movie_error(),(unsigned)sceKernelTotalFreeMemSize());fclose(f);}}
 int id=ending_clock>150?3:ending_clock>80?2:ending_clock>20?1:0;if(id>ending_review_captured){char file[80];snprintf(file,sizeof(file),"review-ending-%d.ppm",id);render_debug_frame(file);ending_review_captured=id;}
}
static int review_expansion(int n){
 switch(n){
 case 0:screen=14;break;
 case 1:render_debug_frame("review-survival-menu.ppm");game_start_survival(&game,2,SURVIVAL_HARD,3000);screen=1;reset_level_ui();break;
 case 2:render_debug_frame("review-survival.ppm");game.extra.pending=1;game.extra.flags=2;break;
 case 3:render_debug_frame("review-survival-round.ppm");screen=15;puzzle_family=0;break;
 case 4:render_debug_frame("review-vase-menu.ppm");puzzle_family=1;break;
 case 5:render_debug_frame("review-izombie-menu.ppm");game_start_puzzle(&game,0,9,3001);screen=1;reset_level_ui();break;
 case 6:render_debug_frame("review-vase-endless.ppm");game_start_puzzle(&game,1,8,3002);screen=1;reset_level_ui();break;
 case 7:render_debug_frame("review-izombie.ppm");profile_init(&player);player.highest=50;garden_initialize(&player,(int64_t)time(NULL));for(int i=0;i<8;i++){garden_add(&player,i,(int64_t)time(NULL));player.garden.plants[i].age=i%4;}garden_view=0;screen=16;break;
 case 8:render_debug_frame("review-zen-garden.ppm");player.purchases[SHOP_TREE]=1;player.garden.tree_height=30;player.garden.tree_tip=29;screen=17;break;
 case 9:render_debug_frame("review-tree.ppm");game_start_level(&game,11,3003);story_begin(201,1,0,0);story_phase=1;break;
 case 10:render_debug_frame("review-dave.ppm");game_start_level(&game,50,3004);game.extra.boss_action=B_HEAD_ENTER;game.extra.boss_action_time=4.5f;screen=1;reset_level_ui();break;
 case 11:render_debug_frame("review-boss-head-enter.ppm");game.extra.boss_action=B_STOMP;game.extra.boss_action_time=1.4f;break;
 case 12:render_debug_frame("review-boss-stomp.ppm");game_start_mini(&game,MINI_HEAVY,3005);game.extra.heavy_level=3;game.extra.heavy_cattail=1;reset_level_ui();break;
 case 13:render_debug_frame("review-heavy-cattail.ppm");screen=6;more_cursor=0;break;
 case 14:render_debug_frame("review-more-ways.ppm");screen=11;collection_cursor=0;break;
 case 15:render_debug_frame("review-extras.ppm");screen=21;adventure_cursor=1;break;
 case 16:render_debug_frame("review-adventure-menu.ppm");screen=4;menu_volume_row=0;break;
 case 17:render_debug_frame("review-options-compact.ppm");screen=22;garden_picker_cursor=4;break;
 case 18:render_debug_frame("review-garden-picker.ppm");screen=20;break;
 case 19:render_debug_frame("review-credits-menu.ppm");game_start_level(&game,5,3401);game.shovel_tutorial=0;game.spawn_timer=100000;for(int c=0;c<9;c++)game.plants[2][c].kind=-1;game.nuts[0]=(BowlingNut){.used=1,.row=2,.target_row=2,.last_hit=-1,.x=230,.y=game_ground_y(&game,2)};game_spawn_zombie(&game,2,Z_NORMAL,240);game_step(&game,.01f);screen=1;reset_level_ui();break;
 case 20:render_debug_frame("review-bowling-regular-hit.ppm");game_start_level(&game,5,3402);game.shovel_tutorial=0;game.spawn_timer=100000;game.nuts[0]=(BowlingNut){.used=1,.explosive=1,.row=2,.target_row=2,.last_hit=-1,.x=230,.y=game_ground_y(&game,2)};game_spawn_zombie(&game,2,Z_NORMAL,240);game_step(&game,.01f);reset_level_ui();break;
 case 21:render_debug_frame("review-bowling-red-hit.ppm");game_start_level(&game,50,3403);game.status=WON;game.level_flags|=LEVEL_REWARD_COLLECTED;reset_level_ui();reward_collect_time=1;screen=1;break;
 case 22:render_debug_frame("review-final-result.ppm");game_start_level(&game,9,3404);screen=1;paused=1;reset_level_ui();break;
 case 23:render_debug_frame("review-pause-compact.ppm");paused=0;game_start_level(&game,49,3800);game.seed_mask=(UINT64_C(1)<<10)-1;game.seed_slots=10;screen=1;reset_level_ui();break;
 case 24:render_debug_frame("review012-seedbank-ten.ppm");game_start_puzzle(&game,0,0,3801);game.packet_count=10;for(int i=0;i<10;i++)game.packets[i]=i;reset_level_ui();break;
 case 25:render_debug_frame("review012-vase-packets.ppm");game_start_puzzle(&game,0,0,3802);game.plants[2][3]=(Plant){.kind=SQUASH,.hp=300};game.vases[2][3]=0;game.burst_left[2][3]=1;game.burst[2][3]=.2f;game.pea_shoot_time[2][3]=center(4);game_spawn_zombie(&game,2,Z_CONE,center(4));reset_level_ui();break;
 case 26:render_debug_frame("review012-squash-look.ppm");game.burst_left[2][3]=3;game.burst[2][3]=.2f;break;
 case 27:render_debug_frame("review012-squash-jump.ppm");game.burst_left[2][3]=4;game.burst[2][3]=.075f;break;
 case 28:render_debug_frame("review012-squash-landing.ppm");game_start_level(&game,1,3803);game.status=WON;reset_level_ui();update_reward(.01f);reward_x=311;reward_ground=baseline(2);reward_elapsed=reward_delay+1;break;
 case 29:render_debug_frame("review012-seed-pickup.ppm");game.level_flags|=LEVEL_REWARD_COLLECTED;reward_collect_time=.4f;break;
 case 30:render_debug_frame("review012-seed-flight.ppm");reward_collect_time=1;break;
 case 31:render_debug_frame("review012-seed-award.ppm");game_start_mini(&game,MINI_BOWLING,3804);game.status=WON;screen=1;reset_level_ui();update_reward(.01f);reward_x=311;reward_ground=baseline(2);reward_elapsed=reward_delay+1;break;
 case 32:render_debug_frame("review012-trophy-pickup.ppm");game.level_flags|=LEVEL_REWARD_COLLECTED;reward_collect_time=.4f;break;
 case 33:render_debug_frame("review012-trophy-flight.ppm");reward_collect_time=1;break;
 case 34:render_debug_frame("review012-trophy-award.ppm");screen=4;menu_volume_row=3;health_bars=3;break;
 case 35:render_debug_frame("review012-health-options.ppm");game_start_level(&game,9,3805);screen=1;reset_level_ui();meter_progress=.35f;break;
 case 36:render_debug_frame("review012-progress.ppm");game_start_puzzle(&game,0,9,3806);game.extra.stage=9;game.extra.pending=2;game.cursor_row=2;game.cursor_col=4;reset_level_ui();update_reward(1);break;
 case 37:render_debug_frame("review012-endless-trophy.ppm");screen=0;menu_selected=3;break;
 case 38:render_debug_frame("review013-gear.ppm");menu_selected=4;snprintf(player_name,sizeof(player_name),"Alaco");player_name_set=1;break;
 case 39:render_debug_frame("review013-player-tab.ppm");name_begin(0);name_key=44;break;
 case 40:render_debug_frame("review013-keyboard.ppm");screen=4;menu_volume_row=2;break;
 case 41:render_debug_frame("review013-options.ppm");screen=20;break;
 case 42:render_debug_frame("review013-credits.ppm");game_start_level(&game,8,4000);game.unlocked_mask=(UINT64_C(1)<<5)-1;game.seed_mask=7;game.selected=4;screen=3;break;
 case 43:render_debug_frame("review013-owned-seeds.ppm");game.unlocked_mask=(UINT64_C(1)<<49)-1;game.seed_mask=1023;game.seed_slots=10;game.selected=48;break;
 case 44:render_debug_frame("review013-full-seed-grid.ppm");screen=20;credits_cursor=1;break;
 case 45:render_debug_frame("review014-credits.ppm");game_start_puzzle(&game,1,0,4200);for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++)game.plants[r][c].attack=0;screen=1;reset_level_ui();break;
 case 46:render_debug_frame("review014-izombie-colors.ppm");paused=1;pause_cursor=1;break;
 case 47:render_debug_frame("review014-pause.ppm");paused=0;screen=20;gallery_open();gallery_focus=1;break;
 case 48:case 49:case 50:case 51:case 52:case 53:case 54:{char file[80];snprintf(file,sizeof(file),"review014-plants-%d.ppm",n-48);render_debug_frame(file);if(n<54)gallery_cursor[0]+=8;else{gallery_tab=1;gallery_cursor[1]=0;}break;}
 case 55:case 56:case 57:case 58:{char file[80];snprintf(file,sizeof(file),"review014-zombies-%d.ppm",n-55);render_debug_frame(file);if(n<58)gallery_cursor[1]+=8;else{gallery_cursor[1]=Z_BOSS;gallery_pose=2;}break;}
 case 59:render_debug_frame("review014-zombie-death.ppm");gallery_tab=0;gallery_cursor[0]=SQUASH;gallery_pose=1;break;
 case 60:render_debug_frame("review014-plant-action.ppm");gallery_focus=0;gallery_tab=1;gallery_pose=0;break;
 case 61:render_debug_frame("review014-gallery-tabs.ppm");gallery=0;profile_init(&player);player.highest=50;garden_initialize(&player,(int64_t)time(NULL));for(int i=0;i<4;i++){if(i>=2)garden_add(&player,PEA,(int64_t)time(NULL));GardenPlant *v=&player.garden.plants[i];v->kind=PEA;v->age=i==1?1:3;v->waterings=i==0?0:3;v->required=3;v->watered=(int64_t)time(NULL)-60;v->fertilized=v->cared=0;v->need=i==2?NEED_BUGSPRAY:i==3?NEED_MUSIC:NEED_NONE;}screen=16;garden_view=garden_slot=0;break;
 case 62:render_debug_frame("review016-garden-needs.ppm");profile_init(&player);player.highest=50;garden_initialize(&player,(int64_t)time(NULL));for(int i=0;i<11;i++){if(i>=2)garden_add(&player,MARIGOLD,(int64_t)time(NULL));GardenPlant *v=&player.garden.plants[i];v->age=3;v->variation=(i+1)|(i%2?16:0);}screen=16;garden_view=0;garden_slot=0;break;
 case 63:render_debug_frame("review018-marigold-palette.ppm");for(int i=0;i<4;i++){GardenPlant *v=&player.garden.plants[i];v->age=0;v->watered=(int64_t)time(NULL)-100;v->waterings=0;}garden_action_start(0,TOOL_WATER);break;
 case 64:render_debug_frame("review018-watering.ppm");garden_flush();player.garden.plants[0].watered=(int64_t)time(NULL)-100;player.garden.plants[0].waterings=player.garden.plants[0].required;garden_action_start(0,TOOL_FERTILIZER);break;
 case 65:render_debug_frame("review018-fertilizer.ppm");garden_flush();for(int i=0;i<4;i++){GardenPlant *v=&player.garden.plants[i];v->age=3;v->kind=(int[]){PUFF,SUNSHROOM,MAGNET,SEA}[i];}break;
 case 66:render_debug_frame("review018-sleeping-garden.ppm");garden_view=GARDEN_AQUARIUM;for(int i=0;i<4;i++){GardenPlant *v=&player.garden.plants[i];v->garden=GARDEN_AQUARIUM;v->slot=i;v->kind=i%2?KELP:SEA;}break;
 case 67:render_debug_frame("review018-aquarium-variants.ppm");garden_view=GARDEN_BARROW;player.garden.plants[0].garden=GARDEN_BARROW;player.garden.plants[0].slot=0;break;
 case 68:render_debug_frame("review018-wheelbarrow.ppm");profile_init(&player);player.garden.story_seen=CHALLENGE_MIGRATED|CHALLENGE_MINI;screen=7;mini_cursor=3;break;
 case 69:render_debug_frame("review018-locked-minigames.ppm");player.adventure_finished=1;player.mini_trophies=1;screen=14;survival_cursor=12;break;
 case 70:render_debug_frame("review018-locked-endless.ppm");player.garden.survival_wins=1023;break;
 case 71:render_debug_frame("review018-unlocked-endless.ppm");screen=15;puzzle_family=0;player.garden.puzzle_wins=1;puzzle_cursor=2;break;
 case 72:render_debug_frame("review018-puzzle-progression.ppm");screen=24;challenge_notice=0;break;
 case 73:render_debug_frame("review018-challenge-present.ppm");challenge_notice=-1;message_time=0;screen=25;player_hub_row=1;break;
 case 74:render_debug_frame("review0187-player.ppm");avatar_begin();avatar_draft=(Avatar){1,2,0,1,1};break;
 case 75:render_debug_frame("review0187-customizer-cone.ppm");avatar_draft=(Avatar){4,3,1,2,1};avatar_row=5;break;
 case 76:render_debug_frame("review0187-customizer-bucket.ppm");avatar=avatar_draft;game_start_level(&game,9,8765);screen=1;game.spawn_timer=1000;game.sun=1000;for(int row=0;row<5;row++)game_spawn_zombie(&game,row,Z_FLAG,260+row*18);break;
 case 77:render_debug_frame("review0187-custom-flags.ppm");avatar.enabled=0;game_start_level(&game,9,19077);screen=1;game.spawn_timer=10000;reset_level_ui();
  for(int r=0;r<5;r++){int i=game_spawn_zombie(&game,r,(int[]){Z_PAPER,Z_JACK,Z_GARG,Z_IMP,Z_DOLPHIN}[r],300);game.frozen[i]=100;game.polish.action[i]=(int[]){ACTION_PAPER,ACTION_JACK,ACTION_GARG_THROW,ACTION_IMP_FLIGHT,0}[r];game.polish.action_time[i]=.6f;game.zombies[i].vault_from=330;game.zombies[i].vault_to=190;if(r==4)game.zombies[i].vault=1;}break;
 case 78:render_debug_frame("review0190-action-windups.ppm");for(int i=0;i<5;i++)game.polish.action_time[i]=1.2f;game.zombies[2].vaulted=1;game.polish.action[3]=ACTION_IMP_LAND;break;
 case 79:render_debug_frame("review0190-action-recovery.ppm");game_start_level(&game,9,19079);screen=1;game.spawn_timer=10000;reset_level_ui();for(int r=0;r<3;r++){int i=game_spawn_zombie(&game,r,Z_NORMAL,300);game.polish.death_style[i]=r;game.zombies[i].vault_from=27;game.zombies[i].death=2;game.zombies[i].age=.3f;}break;
 case 80:render_debug_frame("review0190-death-styles.ppm");profile_init(&player);player.highest=1;screen=11;collection_cursor=4;menu_clock=0;break;
 case 81:render_debug_frame("review0200-extras-fresh.ppm");player.highest=15;collection_cursor=1;menu_clock=1;break;
 case 82:render_debug_frame("review0200-extras-almanac.ppm");player.highest=25;collection_cursor=0;menu_clock=2;break;
 case 83:render_debug_frame("review0200-extras-shop.ppm");player.highest=45;collection_cursor=2;menu_clock=3;break;
 case 84:render_debug_frame("review0200-extras-garden.ppm");screen=0;menu_selected=0;menu_clock=0;crow_clock=.5f;break;
 case 85:render_debug_frame("review0200-home-flight.ppm");menu_clock=2.5f;crow_clock=2.5f;break;
 case 86:render_debug_frame("review0200-home-perched.ppm");screen=6;more_cursor=0;menu_clock=2;break;
 case 87:render_debug_frame("review0200-modes-motion.ppm");screen=4;menu_volume_row=6;classic_dancer=1;break;
 case 88:render_debug_frame("review0200-dancer-options.ppm");game_start_level(&game,9,20088);screen=1;game.spawn_timer=10000;reset_level_ui();for(int r=0;r<3;r++){int i=game_spawn_zombie(&game,r*2,Z_DANCER,260+r*65);game.zombies[i].age=.4f;game.zombies[i].speed=0;if(r==1){game.zombies[i].eating=1;game.zombies[i].age=.5f;}if(r==2)game.zombies[i].hp=100;}break;
 case 89:render_debug_frame("review0200-classic-dancer.ppm");classic_dancer=0;break;
 case 90:render_debug_frame("review0200-disco-dancer.ppm");
if(review_ui_only){FILE *f=fopen("selftest.txt","a");if(f){fprintf(f,"ENDING PLAYBACK: UI-only model run; initial first/middle/final decoder checks performed; full playback rerun on Street.\n");fclose(f);}return 1;}ending_review_captured=ending_review_frames=0;ending_review_max_lag=0;if(!ending_begin(20)){FILE *f=fopen("selftest.txt","a");if(f){fprintf(f,"ending full synchronized playback opens after all game assets: FAIL\n");fclose(f);}test_ok=0;return 1;}break;
 default:{FILE *f=fopen("selftest.txt","a");if(f){int ok=ending_review_frames>100&&ending_review_captured==3&&ending_clock>=158;fprintf(f,"ending full synchronized playback reaches final credits: %s\nENDING PLAYBACK: %d displayed frames, maximum lag %.3f s, audio position %.3f s\n",ok?"PASS":"FAIL",ending_review_frames,ending_review_max_lag,ending_clock);fclose(f);test_ok&=ok;}return 1;}
 }return 0;
}
