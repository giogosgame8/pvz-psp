static int award_ui_selftest(FILE *out){
    Game old_game=game;Progress old_progress=progress;int old_screen=screen,old_replaying=replaying,old_resume=can_resume,old_ready=ready;
    const char *old_path=adventure_path,*old_progress_path=progress_path;adventure_path="selftest-award-ui.dat";progress_path="selftest-award-history.dat";
    int passed=0,total=0;SceCtrlData pad;memset(&pad,0,sizeof(pad));pad.Lx=pad.Ly=128;ready=1;can_resume=1;replaying=0;screen=1;help=gallery=0;previous_buttons=0;repeat_timer=0;
    #define AWARD_CHECK(label,test) do{int ok=(test);passed+=ok;total++;fprintf(out,"award UI %s: %s\n",label,ok?"PASS":"FAIL");}while(0)
    game_start_level(&game,1,1101);game.status=WON;reset_level_ui();update_reward(.01f);pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);
    AWARD_CHECK("X cannot skip the reward drop",game.level==1&&!(game.level_flags&LEVEL_REWARD_COLLECTED)&&screen==1);
    pad.Buttons=0;input(&pad,.016f);update_reward(1);game.cursor_col=(reward_col+1)%COLS;pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);
    AWARD_CHECK("X away from the pickup cannot collect it",!(game.level_flags&LEVEL_REWARD_COLLECTED));
    pad.Buttons=0;input(&pad,.016f);game.cursor_col=reward_col;game.cursor_row=reward_row;pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);
    AWARD_CHECK("X on the reward opens its result screen",game.level==1&&(game.level_flags&LEVEL_REWARD_COLLECTED)&&screen==1);
    input(&pad,.016f);AWARD_CHECK("holding X cannot both collect and advance",game.level==1&&screen==1);
    pad.Buttons=0;input(&pad,.016f);pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);
    AWARD_CHECK("pickup flight cannot be skipped by a second quick press",game.level==1&&reward_collect_time<.85f);
    pad.Buttons=0;input(&pad,.016f);update_reward(1);pad.Buttons=PSP_CTRL_CROSS;input(&pad,.016f);
    AWARD_CHECK("a separate press advances after collection",game.level==2&&screen==2&&!(game.level_flags&LEVEL_REWARD_COLLECTED));
    Game resumed;AWARD_CHECK("the next level is saved after pickup",game_load(&resumed,adventure_path)&&resumed.level==2);
    remove(adventure_path);remove(progress_path);adventure_path=old_path;progress_path=old_progress_path;game=old_game;progress=old_progress;screen=old_screen;replaying=old_replaying;can_resume=old_resume;ready=old_ready;previous_buttons=0;reset_level_ui();
    fprintf(out,"AWARD UI RESULT %d/%d\n",passed,total);
    #undef AWARD_CHECK
    return passed==total;
}
static int death_asset_selftest(FILE *out){
    int passed=0;for(int kind=0;kind<Z_TYPES;kind++){
        int id=kind>=Z_DUCK?A_DUCK_DEATH+(kind-Z_DUCK)*3:kind==Z_POLE?A_POLE_DEATH:kind==Z_PAPER?A_PAPER_DEATH:kind==Z_FOOTBALL?A_FOOTBALL_DEATH:kind==Z_DANCER?A_DANCER_DEATH:kind==Z_BACKUP?A_BACKUP_DEATH:A_ZOMBIE_DEATH;
        Zombie z={.kind=kind,.death=1,.vault_from=27};z.age=(assets[id].frames-1)/27.0f;
        int ok=assets[id].loaded&&fabsf(game_zombie_animation_time(&z)*assets[id].fps-(assets[id].frames-1))<.01f;passed+=ok;fprintf(out,"death asset kind %d final-frame timing: %s\n",kind,ok?"PASS":"FAIL");
    }
    fprintf(out,"DEATH ASSET RESULT %d/26\n",passed);return passed==26;
}
