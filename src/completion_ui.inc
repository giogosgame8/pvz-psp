static int completion_return=25;
static void draw_completion_trophies(void){
    int gold=player.completion.gold_sunflower;char text[128];
    render_image(&assets[A_AWARD_BACKGROUND],0,0,480,272,0,WHITE);
    ui_center(110,29,260,gold?"Gold Sunflower Trophy":"Your trophies",UI_GOLD,.85f);
    render_clip(202,66,72,63);
    if(player.adventure_finished)render_image(&assets[gold?A_REWARD_GOLD_SUNFLOWER:A_REWARD_SILVER_SUNFLOWER],217,76,46,48,0,WHITE);
    else render_image(&assets[A_REWARD_TROPHY],217,82,46,38,0,RGBA(145,145,145,255));
    render_clip(0,0,480,272);
    snprintf(text,sizeof(text),"%d / 48 trophies",profile_trophy_count(&player));ui_center(187,139,106,text,UI_GOLD,.54f);
    snprintf(text,sizeof(text),"Adventure completed\n%u time%s",(unsigned)player.completion.adventure_runs,player.completion.adventure_runs==1?"":"s");disc_text(188,163,104,text,UI_INK,.55f);
    ui_button(163,225,154,gold==1?"X: Collect award":"X: Back",1);
}
static void draw_house_choice(void){
    static const char *names[]={"Regular House","Scary House","Truck House","Space House","Funhouse"};
    render_background(&assets[A_BACKGROUND]);ui_dialog(54,18,372,235,"Your house");
    int tier=profile_house_tier(&player);render_image(&assets[A_HOUSE_PREVIEWS],114,66,252,139,house_cursor*5+tier,WHITE);
    char text[80];snprintf(text,sizeof(text),"%s%s",names[house_cursor],player.completion.house_selected==(unsigned)house_cursor?"  (Selected)":"");ui_center(70,52,340,text,UI_GOLD,.66f);
    snprintf(text,sizeof(text),"Local trophy tier %d/5",tier+1);ui_center(70,208,340,text,WHITE,.55f);
    ui_button(83,222,145,"X: Select house",1);ui_button(252,222,145,"O: Back",0);
}
static int completion_ui_input(uint32_t pressed){
    if(screen==28){
        if(pressed&PSP_CTRL_CROSS){
            if(player.completion.gold_sunflower==1){uint32_t old_money=player.money;if(profile_collect_gold(&player)){if(!persist_profile()){player.money=old_money;player.completion.gold_sunflower=1;notify("COULD NOT SAVE THE TROPHY");return 1;}foley_sample(FX_DIAMOND);}}
            screen=completion_return;completion_return=25;
        }
        if(pressed&PSP_CTRL_CIRCLE){screen=completion_return;completion_return=25;}return 1;
    }
    if(screen!=29)return 0;
    if(pressed&(PSP_CTRL_LEFT|PSP_CTRL_UP))house_cursor=(house_cursor+4)%5;
    if(pressed&(PSP_CTRL_RIGHT|PSP_CTRL_DOWN))house_cursor=(house_cursor+1)%5;
    if(pressed&PSP_CTRL_CROSS){uint32_t old=player.completion.house_selected;if(profile_choose_house(&player,house_cursor)){if(persist_profile()){foley_sample(FX_TAP);notify("HOUSE SELECTED");}else{player.completion.house_selected=old;notify("COULD NOT SAVE THE HOUSE");}}}
    if(pressed&PSP_CTRL_CIRCLE)screen=25;return 1;
}
