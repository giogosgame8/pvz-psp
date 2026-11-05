static int polish_presentation_selftest(FILE *out){Game saved=game;int pass=0,total=0;
 #define PP(label,test) do{int ok=(test);total++;pass+=ok;fprintf(out,"0190 presentation %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
 const int clips[]={A_GARG_THROW,A_IMP_THROWN,A_IMP_LAND,A_JACK_POP,A_PAPER_GASP,A_DOLPHIN_JUMP,A_ZOMBIE_CHARRED,A_ZOMBIE_WATERDEATH,A_GARG_WALK_NOIMP,A_GARG_EAT_NOIMP,A_GARG_DEATH_NOIMP,A_GARG_THROW_NOIMP};
 const int counts[]={34,16,6,16,13,23,43,25,49,33,59,34};int ready=1;for(int i=0;i<12;i++)ready&=assets[clips[i]].loaded&&assets[clips[i]].frames==counts[i];
 PP("all twelve original transition banks have their expected frame counts",ready);
 game_start_level(&game,9,1919);int id=game_spawn_zombie(&game,2,Z_GARG,330);Zombie *z=&game.zombies[id];game.polish.action[id]=ACTION_GARG_THROW;game.polish.action_time[id]=.7f;
 PP("a Gargantuar selects its one-shot throw",zombie_visible_clip(z,id,0)==&assets[A_GARG_THROW]);
 z->vaulted=1;PP("the passenger disappears from the throw immediately after release",zombie_visible_clip(z,id,0)==&assets[A_GARG_THROW_NOIMP]);
 game.polish.action[id]=0;PP("a recovering Gargantuar keeps its empty back",zombie_visible_clip(z,id,0)==&assets[A_GARG_WALK_NOIMP]);
 z->kind=Z_PAPER;game.polish.action[id]=ACTION_PAPER;PP("Newspaper gasp holds its final pose",zombie_visible_clip(z,id,0)==&assets[A_PAPER_GASP]&&zombie_visible_time(z,&assets[A_PAPER_GASP])<=1.001f);
 z->kind=Z_JACK;game.polish.action[id]=ACTION_JACK;PP("Jack uses pop rather than a walk or eat loop",zombie_visible_clip(z,id,0)==&assets[A_JACK_POP]);
 z->kind=Z_IMP;game.polish.action[id]=ACTION_IMP_FLIGHT;PP("an airborne Imp uses the authored thrown pose",zombie_visible_clip(z,id,0)==&assets[A_IMP_THROWN]);
 game.polish.action[id]=ACTION_IMP_LAND;PP("an Imp has a distinct landing pose",zombie_visible_clip(z,id,0)==&assets[A_IMP_LAND]);
 z->death=1;game.polish.death_style[id]=DEATH_CHARRED;PP("death overrides an interrupted action",zombie_visible_clip(z,id,0)==&assets[A_ZOMBIE_CHARRED]);
 game.polish.death_style[id]=DEATH_WATER;PP("drowning selects the water-death bank",zombie_visible_clip(z,id,0)==&assets[A_ZOMBIE_WATERDEATH]);
 z->death=0;z->kind=Z_DOLPHIN;z->vault=1;z->age=0;game.polish.action[id]=0;game.polish.pose[id]=100;
 PP("a Dolphin starts its jump at frame zero regardless of prior walking",zombie_visible_clip(z,id,0)==&assets[A_DOLPHIN_JUMP]&&zombie_visible_time(z,&assets[A_DOLPHIN_JUMP])==0);
 z->age=43/48.0f;PP("a Dolphin reaches the middle of its own vault clip",fabsf(zombie_visible_time(z,&assets[A_DOLPHIN_JUMP])*assets[A_DOLPHIN_JUMP].fps-assets[A_DOLPHIN_JUMP].frames*.5f)<.001f);
 game.wave=3;game.num_waves=10;game.spawned=0;game.polish.wave_pending=1;PP("a queued wave retains the completed portion of the progress meter",fabsf(wave_meter_target()-.2f)<.001f);
 game=saved;
 #undef PP
 return pass==total;
}
