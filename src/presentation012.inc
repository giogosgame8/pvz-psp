static void draw_squash_flight(int row,int col){
 int phase=game.burst_left[row][col];float elapsed=game.burst[row][col],x=center(col),target=game.pea_shoot_time[row][col],height=0,anim;
 if(phase==3){float t=fminf(1,elapsed/.3f),ease=t*t*(3-2*t);x+=(target-x)*ease;height=54*ease;anim=.45f+elapsed;}
 else{x=target;if(phase==4)height=54*(1-fminf(1,elapsed/.1f));anim=phase==4?elapsed:.2f;}
 float foot=baseline(row)+game_roof_offset(&game,x);render_anim(&assets[phase==3?A_SQUASH_UP:A_SQUASH_DOWN],x,foot-height,anim,.68f,WHITE,0);
}
static float current_wave_health(void){float health=0;for(int i=0;i<MAX_ZOMBIES;i++){const Zombie *z=&game.zombies[i];if(z->used&&z->death<=0&&!game.hypnotized[i]&&z->wave==game.wave)health+=fmaxf(0,z->hp)+fmaxf(0,z->armor);}return health;}
static float wave_meter_target(void){
 if(game.status==WON)return 1;if(game.mode==MODE_BOSS)return fminf(1,fmaxf(0,1-game.boss_hp/(game.challenge?60000:40000)));
 if(game.num_waves<1||!game.spawned)return 0;
 float remaining=current_wave_health();if(meter_wave!=game.wave){meter_wave=game.wave;meter_start_timer=fmaxf(.01f,game.spawn_timer);meter_start_health=fmaxf(1,remaining);}
 float damage=(meter_start_health-remaining)/fmaxf(1,meter_start_health-(game.wave==game.num_waves?0:game.wave_health_threshold));
 float clock=game.wave==game.num_waves?0:1-fmaxf(0,game.spawn_timer)/meter_start_timer;
 float fraction=fminf(.99f,fmaxf(0,fmaxf(damage,clock)));return fminf(1,(game.wave-1+fraction)/game.num_waves);
}
static void layout_vase_packets(void){for(int i=0;i<game.packet_count;i++)packet_x[i]=54+i*36;packet_ui_count=game.packet_count;}
