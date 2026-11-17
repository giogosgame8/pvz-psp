static int polish_ui024_selftest(FILE *out){
 Game saved=game;int old_screen=screen,old_style=classic_dancer,phase=story_phase,prop=story_prop,destination=story_return;float clock=story_time,line=story_line_time;
 int pass=0,total=0;
#define P24(label,test) do{int ok=(test);total++;pass+=ok;fprintf(out,"polish024 UI %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 P24("new dancer clips contain interpolated source poses",assets[A_DANCER_POINT].loaded&&assets[A_DANCER_POINT].frames==18&&assets[A_DANCER_POINT].fps==48&&assets[A_DANCER_ARMRAISE].frames==44&&assets[A_CLASSIC_DANCER_ARMRAISE].frames==22);
 P24("boss has the original Bungee exit clip",assets[A_BOSS_BUNGEE_LEAVE].loaded&&assets[A_BOSS_BUNGEE_LEAVE].frames==36&&assets[A_BOSS_BUNGEE_LEAVE].fps==24);
 game_start_level(&game,18,2420);classic_dancer=0;int id=game_spawn_zombie(&game,2,Z_DANCER,350);Zombie *z=&game.zombies[id];
 P24("Disco uses a moonwalk during its entrance",zombie_visible_clip(z,id,0)==&assets[A_DANCER_MOONWALK]);game.polish024.dancer_state[id]=DANCE_POINT;game.polish024.actor_clock[id]=.1f;
 P24("summoning uses its pointing clip and its own clock",zombie_visible_clip(z,id,0)==&assets[A_DANCER_POINT]&&fabsf(zombie_visible_time(z,&assets[A_DANCER_POINT])-.1f)<.001f);
 game.polish024.dancer_state[id]=DANCE_HOLD;game.polish024.actor_clock[id]=.01f;
 P24("pointing holds its final pose without replaying the snap",fabsf(zombie_visible_time(z,&assets[A_DANCER_POINT])-17/48.0f)<.001f);
 game.polish024.dancer_state[id]=DANCE_GROUP;game.time=2.7f;P24("the group beat switches to the arm-raise clip",zombie_visible_clip(z,id,0)==&assets[A_DANCER_ARMRAISE]);classic_dancer=1;
 P24("the classic choice retains its own arm-raise artwork",zombie_visible_clip(z,id,0)==&assets[A_CLASSIC_DANCER_ARMRAISE]);
 game_start_level(&game,29,2421);id=game_spawn_zombie(&game,2,Z_DUCK,350);z=&game.zombies[id];game.polish024.pool_gear[id]=Z_BUCKET;z->armor=1100;
 P24("swimming Bucketheads retain visible armour",zombie_visible_clip(z,id,0)==&assets[A_DUCK_BUCKET_WALK]);z->armor=600;P24("swimming armour has its first damage stage",zombie_visible_clip(z,id,0)==&damaged[A_DUCK_BUCKET_WALK][0]);z->armor=300;z->eating=1;
 P24("swimming armour damage also applies while eating",zombie_visible_clip(z,id,0)==&damaged[A_DUCK_BUCKET_EAT][1]);z->armor=0;P24("losing swimming armour restores the unarmoured body",zombie_visible_clip(z,id,0)==&assets[A_DUCK_EAT]);
 story_phase=0;story_prop=-1;story_time=story_line_time=0;story_return=0;screen=18;
 story_step(story_duration(A_DAVE_ENTER,24)-.01f);P24("Dave cannot talk before his entrance finishes",story_phase==0&&story_line_time==0);story_step(.02f);
 P24("Dave begins talking from the start of the talk animation",story_phase==1&&story_line_time==0);story_step(.02f);P24("Dave's dialogue clock advances after entrance",fabsf(story_line_time-.02f)<.001f);
 story_phase=2;story_time=0;story_return=1;game.level=50;story_step(2.5f);P24("the final Dave grab is not cut off after 2.4 seconds",screen==18);story_step(1);P24("the authored grab can finish before gameplay resumes",screen==1);
 P24("reward growth uses the full source transition time",REWARD_FLIGHT_DURATION==4);
 game=saved;screen=old_screen;classic_dancer=old_style;story_phase=phase;story_prop=prop;story_return=destination;story_time=clock;story_line_time=line;
#undef P24
 fprintf(out,"POLISH024 UI RESULT %d/%d\n",pass,total);return pass==total;
}
