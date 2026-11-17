static int scene0241_selftest(FILE *out){
 Game saved=game;int pass=0,total=0;
#define SC(label,test) do{int ok=(test);total++;pass+=ok;fprintf(out,"scene0241 %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 SC("boss upper body survives the source canvas",assets[A_BOSS_BUNGEE_LEAVE].height>220&&assets[A_BOSS_HEAD_ENTER].height>280&&assets[A_BOSS_WALK].height>160);
 int valid=1;for(unsigned i=0;i<sizeof(boss_banks)/sizeof(*boss_banks);i++){const Animation *a=&assets[boss_banks[i]];valid&=a->loaded&&a->width>0&&a->height>0&&isfinite(a->anchor_x)&&isfinite(a->anchor_y);}
 SC("all boss banks share the towering upright-camera origin",valid&&boss_draw_scale==1.8f&&fabsf(boss_draw_x+(assets[A_BOSS_WALK].width-assets[A_BOSS_WALK].anchor_x)*boss_draw_scale-478)<.01f&&boss_draw_y==207&&boss_draw_y-assets[A_BOSS_WALK].anchor_y*boss_draw_scale<0);
 SC("ordinary swimmers use the 42-frame source swimming sequence",assets[A_DUCK_WALK].frames==42&&assets[A_DUCK_CONE_WALK].frames==42&&assets[A_DUCK_BUCKET_WALK].frames==42);
 SC("eating and swimming keep the same waterline anchor",fabsf(assets[A_DUCK_WALK].height-assets[A_DUCK_WALK].anchor_y)<1&&assets[A_DUCK_EAT].anchor_y>0);
 game_start_level(&game,49,24101);game.support[2][2]=POT+1;game.plants[2][2]=(Plant){.kind=CABBAGE,.hp=300};
 SC("potted pults retain their original placement offset",fabsf(potted_offset(2,2,CABBAGE,0))>1&&potted_offset(2,2,POT,1)>0&&potted_offset(2,2,CABBAGE,1)<0);
 float dx=potted_offset(2,2,CABBAGE,0),dy=potted_offset(2,2,CABBAGE,1);game.support[2][2]=0;
 SC("support alignment leaves ordinary lawn placement alone",potted_offset(2,2,CABBAGE,0)==0&&potted_offset(2,2,CABBAGE,1)==0&&isfinite(dx)&&isfinite(dy));
 game.support[2][2]=POT+1;game.plants[2][2].kind=MINE;game.plants[2][2].age=0;float small=potted_offset(2,2,MINE,1);game.plants[2][2].age=20;
 SC("unarmed mines use their own crop and drawing scale",isfinite(small)&&isfinite(potted_offset(2,2,MINE,1))&&small!=potted_offset(2,2,MINE,1));
 SC("every support placement entry is loaded",support_offsets[POT][1]>0&&support_offsets[CHOMPER][1]<0&&isfinite(support_offsets[ALL_SEEDS][0]));
 game_start_level(&game,49,24103);int kinds[]={Z_JACK,Z_LADDER,Z_DIGGER,Z_YETI},clips[]={A_JACK_WALK,A_LADDER_WALK,A_DIGGER_GROUND,A_YETI_WALK};
 for(int n=0;n<4;n++){int i=game_spawn_zombie(&game,n,kinds[n],350);Zombie *z=&game.zombies[i];z->vaulted=1;SC("new advanced zombies retain their full-health heads and intact body bank",game_zombie_has_head(&game,z)&&!game_zombie_body_stage_for(&game,z)&&zombie_visible_clip(z,i,0)==&assets[clips[n]]);z->hp=(int[]){100,100,80,400}[n];SC("the marked damage scene intentionally crosses the head-loss threshold",!game_zombie_has_head(&game,z));}
 game.zombies[0].used=0;int replacement=game_spawn_zombie(&game,0,Z_JACK,350);SC("reusing a headless actor slot restores the new zombie's head",replacement==0&&game_zombie_has_head(&game,&game.zombies[replacement])&&zombie_visible_clip(&game.zombies[replacement],replacement,0)==&assets[A_JACK_WALK]);
 game=saved;
#undef SC
 fprintf(out,"SCENE0241 RESULT %d/%d\n",pass,total);return pass==total;
}
