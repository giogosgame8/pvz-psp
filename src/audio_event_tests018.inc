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
 z=game_spawn_zombie(&g,2,Z_PAPER,400);hurt_zombie(&g,&g.zombies[z],150,0);E("newspaper destruction emits rip and angry variants",event018[FOLEY_NEWSPAPER_RIP]&&event018[FOLEY_NEWSPAPER_RARRGH]);
 memset(event018,0,sizeof(event018));g.suns[0]=(Sun){.used=1,.x=100,.y=100};game_collect(&g);E("sun pickup uses original points Foley",event018[FOLEY_SUN]==1);
 memset(event018,0,sizeof(event018));game_add_coin(&g,100,100,5);game_collect(&g);E("coin-only pickup does not play the sun effect",event018[FOLEY_COIN]==1&&!event018[FOLEY_SUN]);
 memset(event018,0,sizeof(event018));shoot(&g,2,100,2);shoot(&g,2,100,3);E("mushrooms use puff and fume instead of pea sounds",event018[FOLEY_PUFF]==1&&event018[FOLEY_FUME]==1);
 memset(event018,0,sizeof(event018));game_start_puzzle(&g,0,0,123);int broken=0;for(int r=0;r<5&&!broken;r++)for(int c=0;c<9&&!broken;c++)if(g.vases[r][c])broken=game_break_vase(&g,r,c);E("breaking a vase uses the glass-breaking effect",broken&&event018[FOLEY_VASE_BREAKING]==1);
 sound_listener=prior;
 #undef E
 fprintf(out,"EVENT018 RESULT %d/%d\n",pass,total);return pass==total;
}
