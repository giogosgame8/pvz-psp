/* Shared by drawing and chewing cues so both use the same clip and clock. */
static const Animation *zombie_visible_clip(const Zombie *z,int index,int *body_result){
 int asset=z->death>0?A_ZOMBIE_DEATH:z->armor>0&&z->kind==1?(z->eating?A_CONE_EAT:A_CONE):z->armor>0&&z->kind==2?(z->eating?A_BUCKET_EAT:A_BUCKET):(z->eating?A_ZOMBIE_EAT:A_ZOMBIE);
 if(z->kind==Z_POLE)asset=z->death>0?A_POLE_DEATH:z->vault>0?A_POLE_JUMP:z->eating?A_POLE_EAT:z->vaulted?A_POLE_WALK:A_POLE_RUN;
 if(z->kind==Z_PAPER)asset=z->death>0?A_PAPER_DEATH:z->armor>0?(z->eating?A_PAPER_EAT:A_PAPER):(z->eating?A_PAPER_ANGRY_EAT:A_PAPER_ANGRY);
 if(z->kind==Z_DOOR&&z->armor>0)asset=z->eating?A_DOOR_EAT:A_DOOR;
 if(z->kind==Z_FOOTBALL)asset=z->death>0?A_FOOTBALL_DEATH:z->eating?A_FOOTBALL_EAT:A_FOOTBALL;
 if(z->kind==Z_DANCER)asset=z->death>0?A_DANCER_DEATH:z->eating?A_DANCER_EAT:A_DANCER;
 if(z->kind==Z_BACKUP)asset=z->death>0?A_BACKUP_DEATH:z->eating?A_BACKUP_EAT:A_BACKUP;
 if(z->kind==Z_FLAG&&z->death<=0)asset=z->eating?A_FLAG_EAT:A_FLAG_WALK;
 if(z->kind>=Z_DUCK)asset=A_DUCK_WALK+(z->kind-Z_DUCK)*3+(z->death>0?2:z->eating?1:0);
 int body=(game.level==25||game_mini_active(&game,MINI_LITTLE))?0:game_zombie_body_stage_for(&game,z),stage=z->armor>0&&z->death<=0?game_zombie_armor_stage(z):body;
 if((game.completion.cosmetics&COS_DANCE)&&z->kind<=Z_BUCKET&&!z->eating&&z->death<=0&&body<2)asset=z->kind==Z_CONE&&z->armor>0?A_CONE_DANCE:z->kind==Z_BUCKET&&z->armor>0?A_BUCKET_DANCE:A_ZOMBIE_DANCE;
 if(body==2&&z->death<=0&&z->kind<Z_DUCK&&!(classic_dancer&&z->kind==Z_DANCER))asset=z->kind==Z_POLE?A_POLE_WALK:A_ZOMBIE;
 if(z->kind==Z_DANCER&&classic_dancer)asset=z->death>0?A_CLASSIC_DANCER_DEATH:z->eating?A_CLASSIC_DANCER_EAT:A_CLASSIC_DANCER;
 const Animation *clip=stage&&damaged[asset][stage-1].loaded?&damaged[asset][stage-1]:&assets[asset];
 if(game.mini.zombie_heads[index]&&z->death<=0)clip=&assets[z->eating?A_BOT_EAT:A_BOT_WALK];
 if(z->kind==Z_BOBSLED&&z->death<=0)clip=&assets[z->eating?A_BOBSLED_EAT:game.extra.bobsled_team[index]&&game.extra.bobsled_phase[index]>=0?A_BOBSLED_SEATED:A_BOBSLED_ONFOOT];
 if(z->kind==Z_GARG&&z->vaulted)clip=&assets[z->death>0?A_GARG_DEATH_NOIMP:z->eating?A_GARG_EAT_NOIMP:A_GARG_WALK_NOIMP];
 if(z->death<=0&&game.fidelity.equipment_lost[index]){if(z->kind==Z_LADDER)clip=&assets[z->eating?A_LADDER_BARE_EAT:A_LADDER_BARE_WALK];else if(z->kind==Z_POGO&&!z->eating)clip=&assets[A_POGO_GROUND];else if(z->kind==Z_JACK)clip=&assets[z->eating?A_JACK_BARE_EAT:A_JACK_BARE_WALK];else if(z->kind==Z_FOOTBALL)clip=&assets[z->eating?A_FOOTBALL_BARE_EAT:A_FOOTBALL_BARE_WALK];else if(z->kind==Z_BALLOON&&!z->eating)clip=&assets[A_BALLOON_GROUND];}
 if(z->kind==Z_DIGGER&&z->vaulted&&z->death<=0&&!z->eating)clip=&assets[A_DIGGER_GROUND];
 if(z->kind==Z_BUNGEE&&z->death<=0){int phase=game.fidelity.bungee_phase[index];clip=&assets[phase==0?A_BUNGEE_WALK:phase==1?A_BUNGEE_HOLD:phase==2?A_BUNGEE_EAT:A_BUNGEE_DEATH];}
 if(game.completion.giga[index])clip=&assets[z->death>0?(z->vaulted?A_GIGA_DEATH_NOIMP:A_GIGA_DEATH):z->eating?(z->vaulted?A_GIGA_EAT_NOIMP:A_GIGA_EAT):(z->vaulted?A_GIGA_WALK_NOIMP:A_GIGA_WALK)];
 int action=game.polish.action[index];
 if(z->death<=0&&(action==ACTION_SNORKEL_RISE||action==ACTION_SNORKEL_DIVE))clip=&assets[A_SNORKEL_RISE];
 if(z->death<=0){if(action==ACTION_PAPER)clip=&assets[A_PAPER_GASP];else if(action==ACTION_JACK)clip=&assets[A_JACK_POP];else if(action==ACTION_GARG_THROW)clip=&assets[game.completion.giga[index]?(z->vaulted?A_GIGA_THROW_NOIMP:A_GIGA_THROW):(z->vaulted?A_GARG_THROW_NOIMP:A_GARG_THROW)];else if(action==ACTION_IMP_FLIGHT)clip=&assets[A_IMP_THROWN];else if(action==ACTION_IMP_LAND)clip=&assets[A_IMP_LAND];else if(z->kind==Z_DOLPHIN&&z->vault>0)clip=&assets[A_DOLPHIN_JUMP];}
 if(z->death<=0){if(action==ACTION_LADDER)clip=&assets[A_LADDER_PLACE];else if(action==ACTION_BALLOON_FALL)clip=&assets[A_BALLOON_POP];else if(action==ACTION_DIGGER_RISE)clip=&assets[game.polish.action_time[index]<.6f?A_DIGGER_DRILL:A_DIGGER_LAND];else if(action==ACTION_CATAPULT)clip=&assets[A_CATAPULT_EAT];}
 if(z->death>0){if(game.polish.death_style[index]==DEATH_CHARRED)clip=&assets[A_ZOMBIE_CHARRED];else if(game.polish.death_style[index]==DEATH_WATER)clip=&assets[A_ZOMBIE_WATERDEATH];}
 if(z->death<=0&&body&&(uintptr_t)clip>=(uintptr_t)assets&&(uintptr_t)clip<(uintptr_t)(assets+A_COUNT)){int id=(int)(clip-assets);if(damaged[id][body-1].loaded)clip=&damaged[id][body-1];}
 if(body_result)*body_result=body;return clip;
}
/* PS3 UpdateAnimSpeed, 0x2bca18-0x2bcacc: eating overrides the
   reanimation file's default rate with 20 or 36 authored frames/second. */
static float zombie_eating_rate(int kind){
 switch(kind){case Z_POLE:case Z_BALLOON:case Z_IMP:case Z_DIGGER:case Z_JACK:case Z_SNORKEL:case Z_YETI:return 20;default:return 36;}
}
static int zombie_chewing_kind(int kind){return kind!=Z_GARG&&kind!=Z_ZOMBONI&&kind!=Z_CATAPULT&&kind!=Z_BUNGEE&&kind!=Z_BOSS;}
static float zombie_visible_time(const Zombie *z,const Animation *clip){
 if(z->death>0){int style=game.polish.death_style[z-game.zombies];return style?fminf((clip->frames-1)/fmaxf(1,clip->fps),z->age*(style==DEATH_CHARRED?15:12)/fmaxf(1,clip->fps)):classic_dancer&&z->kind==Z_DANCER?fminf(1,z->age/fmaxf(.01f,game_death_duration(z)-.6f))*(clip->frames-1)/fmaxf(1,clip->fps):game_zombie_animation_time(z);}
 int index=(int)(z-game.zombies),action=game.polish.action[index];
 if(z->kind==Z_BUNGEE){int phase=game.fidelity.bungee_phase[index];return game.special[index]*(phase>=3?36:24)/fmaxf(1,clip->fps);}
 if(action==ACTION_SNORKEL_RISE||action==ACTION_SNORKEL_DIVE){float t=fminf(11/24.0f,game.polish.action_time[index]);return (action==ACTION_SNORKEL_DIVE?11/24.0f-t:t)*24/fmaxf(1,clip->fps);}
 if(z->kind==Z_DOLPHIN&&z->vault>0)return z->age*clip->frames/(game_vault_duration(z)*fmaxf(1,clip->fps));
 if(z->kind==Z_POGO&&z->vault>0)return z->age*clip->frames/(game_vault_duration(z)*fmaxf(1,clip->fps));
 if(action==ACTION_PAPER||action==ACTION_JACK||action==ACTION_GARG_THROW||action==ACTION_IMP_FLIGHT||action==ACTION_IMP_LAND){float rate=action==ACTION_PAPER?8:action==ACTION_JACK?28:action==ACTION_IMP_FLIGHT?18:24;return fminf((clip->frames-1)/fmaxf(1,clip->fps),game.polish.action_time[index]*rate/fmaxf(1,clip->fps));}
 if(action==ACTION_LADDER||action==ACTION_BALLOON_FALL||action==ACTION_DIGGER_RISE||action==ACTION_CATAPULT){float t=game.polish.action_time[index],rate=action==ACTION_BALLOON_FALL?24:action==ACTION_DIGGER_RISE?16:24;if(action==ACTION_DIGGER_RISE&&t>=.6f)t-=.6f;return fminf((clip->frames-1)/fmaxf(1,clip->fps),t*rate/fmaxf(1,clip->fps));}
 if(z->eating&&z->vault<=0&&zombie_chewing_kind(z->kind))return z->age*zombie_eating_rate(z->kind)/fmaxf(1,clip->fps);
 if(z->kind==Z_GARG&&z->eating)return game.special[index]*16/fmaxf(1,clip->fps);
 if((game.completion.cosmetics&COS_DANCE)&&z->kind<=Z_BUCKET&&!z->eating&&game_zombie_body_stage_for(&game,z)<2)return z->age*clip->frames/fmaxf(1,clip->fps);
 if(classic_dancer&&z->kind==Z_DANCER&&!z->eating)return z->age*clip->frames/fmaxf(1,clip->fps);
 return z->kind<Z_DUCK?game_zombie_animation_time(z):game.polish.pose[index];
}
