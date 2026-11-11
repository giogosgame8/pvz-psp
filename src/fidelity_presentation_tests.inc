static int fidelity_presentation_selftest(FILE *out){
 Game saved=game;char raw[2048],text[2048];memcpy(raw,story_raw,sizeof(raw));memcpy(text,story_text,sizeof(text));int old_offset=story_offset,old_end=story_end;
 int pass=0,total=0;
 #define FP(label,condition) do{int ok=(condition);pass+=ok;total++;fprintf(out,"fidelity UI %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 int ready=1;for(int i=A_CHOMPER_SWALLOW;i<A_COUNT;i++)ready&=assets[i].loaded;
 FP("every new transition, projectile and story bank is present",ready);
 game_start_level(&game,9,2305);game.plants[2][3]=(Plant){.kind=CHOMPER,.hp=300};int a,loop;float t;
 game.burst_left[2][3]=5;game.burst[2][3]=.8f;a=A_CHOMPER;loop=1;t=0;fidelity_plant_view(2,3,&a,&t,&loop);FP("a swallowed target finishes the bite before digestion",a==A_CHOMPER_BITE&&!loop);
 game.burst_left[2][3]=3;a=A_CHOMPER;loop=1;fidelity_plant_view(2,3,&a,&t,&loop);FP("digestion recovery uses the separate swallow bank",a==A_CHOMPER_SWALLOW&&!loop);
 game.plants[2][3].kind=KERNEL;game.burst_left[2][3]=1;game.burst_target[2][3]=6;a=A_KERNEL;fidelity_plant_view(2,3,&a,&t,&loop);FP("Kernel butter windup carries butter rather than a kernel",a==A_KERNEL_BUTTER&&!loop);
 int id=game_spawn_zombie(&game,2,Z_LADDER,350);game.fidelity.equipment_lost[id]=1;game.zombies[id].armor=0;game.zombies[id].hp=100;
 FP("a disarmed headless Ladder keeps both its missing ladder and missing head",zombie_visible_clip(&game.zombies[id],id,0)==&damaged[A_LADDER_BARE_WALK][1]);
 game.zombies[id].kind=Z_JACK;game.zombies[id].hp=150;FP("a disarmed Jack keeps its damaged bare-body bank",zombie_visible_clip(&game.zombies[id],id,0)==&damaged[A_JACK_BARE_WALK][1]);
 int bodies=1;const int body_assets[]={A_DUCK_WALK,A_SNORKEL_EAT,A_DOLPHIN_WALK,A_JACK_WALK,A_BALLOON_GROUND,A_DIGGER_GROUND,A_POGO_GROUND,A_LADDER_BARE_WALK,A_IMP_WALK,A_YETI_WALK};for(int i=0;i<10;i++)bodies&=damaged[body_assets[i]][0].loaded&&damaged[body_assets[i]][1].loaded;
 FP("advanced bodies have both progressive damage banks",bodies);
 for(int i=0;i<5;i++){int start=A_DAVE_WALLNUT_ENTER+i*3;ready&=assets[start].frames==18&&assets[start+1].frames==21&&assets[start+2].frames==9;}FP("all five held props retain enter, idle and talk poses",ready);
 memset(story_text,0,sizeof(story_text));for(int n=0;n<60;n++)strcat(story_text,"This is a long dialogue passage. ");story_offset=0;story_layout();FP("long dialogue splits at a word boundary inside its original speech bubble",story_end>0&&story_text[story_end]&&disc_text_height(story_page,290,.8f)<=130);
 int end=story_end;story_offset=end;story_layout();FP("the next dialogue page continues without dropping text",story_end>end&&!strncmp(story_page,story_text+end,strlen(story_page)));
 snprintf(story_raw,sizeof(story_raw),"{DELAY_150}{TIME_275}Hello");FP("disc dialogue timing cues use hundredths of a second",fabsf(story_token_time("{DELAY_",0)-1.5f)<.001f&&fabsf(story_token_time("{TIME_",0)-2.75f)<.001f);
 game=saved;memcpy(story_raw,raw,sizeof(raw));memcpy(story_text,text,sizeof(text));story_offset=old_offset;story_end=old_end;
 #undef FP
 fprintf(out,"FIDELITY UI RESULT %d/%d\n",pass,total);return pass==total;
}
