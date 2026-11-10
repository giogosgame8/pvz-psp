static int features018_selftest(FILE *out){
 Profile old=player;Game oldg=game;int olds=screen;int pass=0,total=0;int64_t now=(int64_t)time(NULL);
 #define F(label,test) do{int ok=(test);total++;pass+=ok;fprintf(out,"features018 %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 profile_init(&player);player.garden.story_seen=CHALLENGE_MIGRATED;
 F("fresh profiles have no challenge access before the presents",!challenge_gate(0)&&!challenge_gate(1)&&!challenge_gate(2)&&challenge_mask(0)==0&&challenge_mask(2)==0);
 player.garden.story_seen|=CHALLENGE_MINI;F("mini present opens exactly the first three",challenge_mask(0)==7);
 player.mini_trophies=7;F("early trophies cannot bypass Adventure requirement",challenge_mask(0)==7);
 player.adventure_finished=1;F("three distinct trophies unlock six mini-games",challenge_mask(0)==63);
 player.mini_trophies=1;F("one trophy cannot be counted again by replaying",challenge_mask(0)==15);
 player.mini_trophies=0x1ffff;F("all twenty mini-games unlock after seventeen wins",challenge_mask(0)==0xfffff);
 player.adventure_finished=0;player.mini_trophies=1u<<19;player.garden.story_seen=CHALLENGE_MIGRATED;F("earned out-of-order legacy trophies remain playable",challenge_available(0,19)&&!challenge_available(0,18));
 player.garden.story_seen|=CHALLENGE_PUZZLE;player.garden.puzzle_wins=0;F("each puzzle family starts at its first challenge",challenge_mask(2)==1&&challenge_mask(3)==1);
 player.garden.puzzle_wins=1;F("Vasebreaker progress does not unlock I Zombie",challenge_mask(2)==3&&challenge_mask(3)==1);
 player.garden.puzzle_wins=7;F("first three puzzle wins wait for Adventure completion",challenge_mask(2)==7);
 player.adventure_finished=1;F("Adventure unlocks the fourth puzzle after three wins",challenge_mask(2)==15);
 player.garden.puzzle_wins=511;F("endless puzzle needs the nine preceding wins",challenge_mask(2)==1023&&challenge_mask(3)==1);
 player.garden.survival_wins=0;F("Survival initially opens three terrains",challenge_mask(1)==7);
 player.garden.survival_wins=511;F("nine Survival trophies do not unlock Pool Endless",!challenge_available(1,10));player.garden.survival_wins=1023;F("all ten Survival trophies unlock the original Pool Endless",challenge_mask(1)==2047);
 player.garden.story_seen|=CHALLENGE_MINI|CHALLENGE_PUZZLE|CHALLENGE_SURVIVAL;Profile loaded;F("challenge gates and trophies survive v3 profile round trip",profile_save(&player,"test018-profile.dat")&&profile_load(&loaded,"test018-profile.dat")&&loaded.garden.story_seen==player.garden.story_seen&&loaded.garden.survival_wins==1023);remove("test018-profile.dat");
 profile_init(&player);player.highest=50;garden_initialize(&player,now);int index=0;GardenPlant *v=&player.garden.plants[index];
 F("marigolds receive only original palette and facing values",(v->variation&15)>=1&&(v->variation&15)<=11&&!(v->variation&~31));
 garden_add(&player,PEA,now);F("ordinary plants use facing without marigold colour bits",!(player.garden.plants[2].variation&15));
 v->age=3;F("mushroom sleep is limited to greenhouse presentation",!garden_sleeping(v));v->kind=PUFF;F("greenhouse mushroom uses its sleeping clip",garden_sleeping(v));v->garden=GARDEN_MUSHROOM;F("mushroom garden wakes its plants",!garden_sleeping(v));v->garden=GARDEN_MAIN;v->kind=MARIGOLD;v->age=0;v->waterings=v->required;v->watered=now-100;v->fertilized=0;
 int supply=player.purchases[SHOP_FERTILIZER];F("fertilizer reserves a supply until its animation completes",garden_action_start(index,TOOL_FERTILIZER)&&v->age==0&&player.purchases[SHOP_FERTILIZER]==(uint32_t)supply-1);
 F("repeated input cannot double-charge the same tool action",!garden_action_start(index,TOOL_FERTILIZER));garden_flush();F("tool completion applies growth once and consumes one supply",v->age==1&&player.purchases[SHOP_FERTILIZER]==(uint32_t)supply-1&&!garden_pending());garden_flush();F("flushing completed tools is idempotent",v->age==1&&player.purchases[SHOP_FERTILIZER]==(uint32_t)supply-1);
 v->age=0;v->watered=now-100;v->fertilized=0;v->waterings=v->required;int reserved=player.purchases[SHOP_FERTILIZER];int queued=garden_action_start(index,TOOL_FERTILIZER);v->waterings=0;garden_flush();F("care changes during tool animation refund an unusable supply",queued&&v->age==0&&player.purchases[SHOP_FERTILIZER]==(uint32_t)reserved);
 F("garden palette and direction retain the complete profile v3 prefix",profile_save(&player,"test018-garden.dat")&&profile_load(&loaded,"test018-garden.dat")&&loaded.garden.plants[index].variation==v->variation&&offsetof(Profile,completion)==17320);remove("test018-garden.dat");
 F("marigold body and petals share identical anchor and frame geometry",assets[A_GARDEN_MARIGOLD_BODY].width==assets[A_GARDEN_MARIGOLD_PETALS].width&&assets[A_GARDEN_MARIGOLD_BODY].height==assets[A_GARDEN_MARIGOLD_PETALS].height&&assets[A_GARDEN_MARIGOLD_BODY].frames==assets[A_GARDEN_MARIGOLD_PETALS].frames);
 F("all garden tools and aquatic clips were imported",assets[A_GARDEN_WATER].frames==17&&assets[A_GARDEN_GOLDWATER].frames==17&&assets[A_GARDEN_FERTILIZER].frames==31&&assets[A_GARDEN_SPRAY].frames==26&&assets[A_GARDEN_MUSIC].frames==37&&assets[A_GARDEN_KELPAQUA].frames>1&&assets[A_GARDEN_SEAAQUA].frames>1);
 profile_init(&player);game_start_level(&game,22,600);profile_apply(&player,&game);game_add_coin(&game,180,120,-200);game_collect_coins(&game);game_animate_pickups(&game,.5f);profile_bank(&player,&game);F("collected Adventure present unlocks Mini-games once",(player.garden.story_seen&CHALLENGE_MINI)&&challenge_gate(0)&&challenge_mask(0)==7);uint64_t seen=player.garden.story_seen;profile_bank(&player,&game);F("reloading gift journal cannot duplicate the unlock",player.garden.story_seen==seen);
 game_start_level(&game,36,601);profile_apply(&player,&game);game_add_coin(&game,180,120,-201);game_collect_coins(&game);game_animate_pickups(&game,.5f);profile_bank(&player,&game);F("Puzzle present unlocks both independent families",challenge_gate(2)&&challenge_gate(3)&&challenge_mask(2)==1&&challenge_mask(3)==1);
 player=old;game=oldg;screen=olds;memset(garden_actions,0,sizeof(garden_actions));memset(garden_grow,0,sizeof(garden_grow));
 #undef F
 fprintf(out,"FEATURES018 RESULT %d/%d\n",pass,total);return pass==total;
}
