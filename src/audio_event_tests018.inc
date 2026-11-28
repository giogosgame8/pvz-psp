/* Verify choices at the gameplay event boundary, without opening audio devices. */
static unsigned event018[FOLEY_COUNT+FX_SAMPLE_COUNT];
static void capture018(int event){if(event>=0&&event<(int)(sizeof(event018)/sizeof(*event018)))event018[event]++;}
int game_audio018_selftest(FILE *out){
 void (*prior)(int)=sound_listener;sound_listener=capture018;Game g;int pass=0,total=0;memset(event018,0,sizeof(event018));
 #define E(label,test) do{int ok=(test);total++;pass+=ok;fprintf(out,"event018 %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 game_start_level(&g,9,12);game_spawn_zombie(&g,2,Z_NORMAL,350);E("ordinary spawning does not emit a groan on every zombie",!event018[FOLEY_GROAN]);
 game_spawn_zombie(&g,2,Z_DOLPHIN,400);game_spawn_zombie(&g,2,Z_BALLOON,420);E("dolphin and balloon have their specific appearance cues",event018[FOLEY_DOLPHIN_APPEARS]==1&&event018[FOLEY_BALLOONINFLATE]==1);
 int z=game_spawn_zombie(&g,2,Z_CONE,400);hurt_zombie(&g,&g.zombies[z],20,0);E("cone armor uses plastic impact variants",event018[FOLEY_PLASTIC_HIT]==1);
 z=game_spawn_zombie(&g,2,Z_BUCKET,400);hurt_zombie(&g,&g.zombies[z],20,0);E("metal armor uses shield impact variants",event018[FOLEY_SHIELD_HIT]==1);
 z=game_spawn_zombie(&g,2,Z_PAPER,400);hurt_zombie(&g,&g.zombies[z],150,0);E("newspaper destruction emits rip before its recovery",event018[FOLEY_NEWSPAPER_RIP]&&!event018[FOLEY_NEWSPAPER_RARRGH]);g.spawn_timer=10000;advance(&g,1.7f);E("newspaper anger emits once after gasp recovery",event018[FOLEY_NEWSPAPER_RARRGH]==1);
 memset(event018,0,sizeof(event018));g.suns[0]=(Sun){.used=1,.x=100,.y=100};game_collect(&g);E("sun pickup uses original points Foley",event018[FOLEY_SUN]==1);
 memset(event018,0,sizeof(event018));game_add_coin(&g,100,100,5);game_collect(&g);E("coin-only pickup does not play the sun effect",event018[FOLEY_COIN]==1&&!event018[FOLEY_SUN]);
 #define START_MUSHROOM_TEST(level) do{game_start_level(&g,level,1819);g.spawn_timer=g.sky_timer=10000;g.shovel_tutorial=0;g.level_flags|=LEVEL_READY;memset(event018,0,sizeof(event018));}while(0)
 START_MUSHROOM_TEST(19);
 float x=plant_x(2);
 Plant *p=&g.plants[2][2];
 *p=(Plant){.kind=FUME,.hp=300};
 int flying=game_spawn_zombie(&g,2,Z_IMP,x+35);
 int buried=game_spawn_zombie(&g,2,Z_DIGGER,x+55);
 g.polish.action[flying]=ACTION_IMP_FLIGHT;
 g.frozen[flying]=g.frozen[buried]=5;
 game_step(&g,1.0f/60);
 E("fume stays ready when its lane only has unreachable enemies",p->attack<=0&&!event018[FOLEY_FUME]&&
   g.zombies[flying].hp==270&&g.zombies[buried].hp==270);
 int door=game_spawn_zombie(&g,2,Z_DOOR,x+80);
 int walker=game_spawn_zombie(&g,2,Z_NORMAL,x+90);
 game_step(&g,1.0f/60);
 E("fume pierces a door and hits multiple ground targets with one fume cue",g.zombies[door].armor==1100&&
   g.zombies[door].hp==250&&g.zombies[walker].hp==250&&g.zombies[flying].hp==270&&
   g.zombies[buried].hp==270&&event018[FOLEY_FUME]==1&&!event018[FOLEY_THROW]);

 START_MUSHROOM_TEST(19);
 *p=(Plant){.kind=SCAREDY,.hp=300};
 walker=game_spawn_zombie(&g,2,Z_NORMAL,x+170);
 g.frozen[walker]=5;
 game_step(&g,1.0f/60);
 E("scaredy fires a long range spore with the puff cue",g.shots[0].ice==3&&g.shots[0].used&&
   event018[FOLEY_PUFF]==1&&!event018[FOLEY_FUME]);
 int threat=game_spawn_zombie(&g,1,Z_NORMAL,x+10);
 g.frozen[threat]=5;
 advance(&g,.4f);
 E("scaredy crouches immediately while its shot is recharging and stays crouched",p->attack>0&&p->digest>.19f);
 g.zombies[threat].used=0;p->attack=0;
 advance(&g,.1f);
 E("scaredy avoids firing while still crouched",p->digest>0&&event018[FOLEY_PUFF]==1);
 advance(&g,.15f);
 E("scaredy fires again after the threat leaves and its crouch ends",p->digest<=0&&event018[FOLEY_PUFF]==2);

 START_MUSHROOM_TEST(19);
 *p=(Plant){.kind=GLOOM,.hp=300};
 int ally=game_spawn_zombie(&g,2,Z_NORMAL,x+25);
 int balloon=game_spawn_zombie(&g,2,Z_BALLOON,x+35);
 g.frozen[ally]=g.frozen[balloon]=5;
 g.hypnotized[ally]=1;
 advance(&g,.85f);
 E("gloom does not start a burst for a friendly zombie or a balloon",p->attack<=0&&!event018[FOLEY_FUME]);
 walker=game_spawn_zombie(&g,1,Z_NORMAL,x+30);
 g.frozen[walker]=5;
 advance(&g,.85f);
 E("gloom pulses damage into adjacent lanes and leaves ineligible targets unharmed",
   g.zombies[walker].hp==250&&g.zombies[ally].hp==270&&g.zombies[balloon].hp==270&&event018[FOLEY_FUME]==1);

 START_MUSHROOM_TEST(29);
 *p=(Plant){.kind=SEA,.hp=300};
 walker=game_spawn_zombie(&g,2,Z_DUCK,x+CELL_W*4);
 g.frozen[walker]=5;
 game_step(&g,1.0f/60);
 E("sea-shroom ignores targets outside its spore range",p->attack<=0&&!event018[FOLEY_PUFF]);
 g.zombies[walker].x=x+CELL_W*2;
 game_step(&g,1.0f/60);
 E("sea-shroom fires once a target comes into range",p->attack>0&&event018[FOLEY_PUFF]==1);

 START_MUSHROOM_TEST(19);
 flying=game_spawn_zombie(&g,2,Z_IMP,x+40);
 walker=game_spawn_zombie(&g,2,Z_NORMAL,x+70);
 g.polish.action[flying]=ACTION_IMP_FLIGHT;
 g.frozen[flying]=g.frozen[walker]=5;
 shoot(&g,2,x,2);advance(&g,.55f);
 E("a spore passes a flying imp and hits a ground target behind it",g.zombies[flying].hp==270&&g.zombies[walker].hp==250);

 START_MUSHROOM_TEST(49);
 int digger=game_spawn_zombie(&g,2,Z_DIGGER,GRID_X+.1f);
 game_step(&g,1.0f/60);advance(&g,.3f);
 E("a naturally emerging Digger starts its rise with one dirt cue",
   g.zombies[digger].vaulted&&g.polish.action[digger]==ACTION_DIGGER_RISE&&
   event018[FOLEY_DIRT_RISE]==1&&!event018[FOLEY_DIGGER]);

 START_MUSHROOM_TEST(49);
 *p=(Plant){.kind=MAGNET,.hp=300};
 digger=game_spawn_zombie(&g,2,Z_DIGGER,x+35);
 game_step(&g,1.0f/60);advance(&g,.3f);
 E("Magnet triggers one dirt cue and one pickup cue without starting the digging loop",
   g.zombies[digger].vaulted&&g.fidelity.equipment_lost[digger]&&
   event018[FOLEY_DIRT_RISE]==1&&event018[FOLEY_MAGNETSHROOM]==1&&!event018[FOLEY_DIGGER]);
 #undef START_MUSHROOM_TEST
 memset(event018,0,sizeof(event018));game_start_puzzle(&g,0,0,123);int broken=0;for(int r=0;r<5&&!broken;r++)for(int c=0;c<9&&!broken;c++)if(g.vases[r][c])broken=game_break_vase(&g,r,c);E("breaking a vase uses the glass-breaking effect",broken&&event018[FOLEY_VASE_BREAKING]==1);
 sound_listener=prior;
 #undef E
 fprintf(out,"EVENT018 RESULT %d/%d\n",pass,total);return pass==total;
}
