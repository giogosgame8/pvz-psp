/* Original disc text is read on selection, so the full book stays off the heap. */
static char book_title[128],book_body[8192];static int book_loaded=-1;
static int load_book_entry(int entry){
    if(book_loaded==entry)return 1;book_loaded=-1;FILE *f=fopen("data/almanac.bin","rb");if(!f)return 0;
    uint32_t h[2],offset,sizes[2];int ok=fread(h,sizeof(h),1,f)==1&&h[0]==0x31425650&&h[1]==75&&entry>=0&&entry<75;
    if(ok)ok=!fseek(f,8+entry*4,SEEK_SET)&&fread(&offset,4,1,f)==1&&offset>=308&&offset<1024*1024;
    if(ok)ok=!fseek(f,offset,SEEK_SET)&&fread(sizes,sizeof(sizes),1,f)==1&&sizes[0]<sizeof(book_title)&&sizes[1]<sizeof(book_body);
    if(ok)ok=fread(book_title,sizes[0],1,f)==1&&fread(book_body,sizes[1],1,f)==1;
    fclose(f);if(ok){book_title[sizes[0]]=0;book_body[sizes[1]]=0;book_loaded=entry;}return ok;
}
static int wrap_book(char lines[160][59]){
    int row=0,col=0;memset(lines,0,160*59);const unsigned char *p=(const unsigned char*)book_body;
    while(*p&&row<159){
        if(*p=='\n'){row++;col=0;p++;continue;}
        if(*p==' '){if(col&&col<42)lines[row][col++]=' ';p++;continue;}
        const unsigned char *end=p;while(*end&&*end!=' '&&*end!='\n')end++;int length=end-p;
        if(col&&col+length>42){row++;col=0;if(row>=159)break;}
        while(p<end&&row<159){unsigned char ch=*p++;if(ch>=128)ch='?';if(col==42){row++;col=0;}lines[row][col++]=ch;}
    }return row+1;
}
static int book_known(void){return book_zombies?!!(player.seen_zombies&(1u<<book_cursor)):!!(player.seen_plants&(UINT64_C(1)<<book_cursor));}
static void draw_book(void){
 render_image(&assets[A_BOOK_PAGES],0,0,480,272,book_zombies,WHITE);int known=book_known(),entry=book_cursor+(book_zombies?49:0);char label[128];ui_center(114,19,260,"Suburban Almanac",UI_GOLD,1.0f);
 if(known&&load_book_entry(entry)){ui_center(13,62,160,book_title,UI_INK,.85f);int icon=book_zombies?(book_cursor>=10?A_DUCK_WALK+(book_cursor-10)*3:book_cursor==0?A_ZOMBIE:book_cursor==1?A_CONE:book_cursor==2?A_BUCKET:book_cursor==3?A_POLE_WALK:book_cursor==4?A_FLAG_WALK:book_cursor==5?A_PAPER:book_cursor==6?A_DOOR:book_cursor==7?A_FOOTBALL:book_cursor==8?A_DANCER:A_BACKUP):seed_icons[book_cursor];render_anim(&assets[icon],91,189,game.time,book_cursor==Z_BOSS&&book_zombies?.7f:1.1f,WHITE,1);
  char lines[160][59];int count=wrap_book(lines);if(book_scroll>count-1)book_scroll=count-1;for(int i=0;i<11&&book_scroll+i<count;i++)disc_text(186,72+i*14,279,lines[book_scroll+i],UI_INK,.64f);
 }else disc_text(186,105,260,known?"Almanac data missing":"You have not found this entry yet.",UI_INK,.75f);
 snprintf(label,sizeof(label),"%d/%d   L/R: Entry   Triangle: %s   Up/Down: Scroll   O: Back",book_cursor+1,book_zombies?Z_TYPES:ALL_SEEDS,book_zombies?"Plants":"Zombies");ui_hint(label);
}
static int store_page_start(int index){return index<8?0:index<14?8:index<22?14:index<28?22:28;}
static void draw_store_icon(int item,float x,float y){if(item>=SHOP_HOUSE_NORMAL&&item<=SHOP_HOUSE_FUN){int choice=item==SHOP_HOUSE_NORMAL?0:item==SHOP_HOUSE_SCARY?1:item==SHOP_HOUSE_TRUCK?2:item==SHOP_HOUSE_SPACE?3:4;int tier=choice?player.purchases[item]:2+player.purchases[item];if(tier>4)tier=4;render_image(&assets[A_HOUSE_PREVIEWS],x-11,y-24,22,20,choice*5+tier,WHITE);}else if(item<9)render_image(&assets[A_SEED_CARDS],x-10,y-27,20,27,40+item,WHITE);else if(item>=SHOP_MARIGOLD1&&item<=SHOP_MARIGOLD3)render_anim(&assets[A_MARIGOLD],x,y,0,.38f,WHITE,0);else if(item==SHOP_SNAIL)render_anim(&assets[A_SNAIL_WALK],x,y,game.time,.35f,WHITE,1);else if(item==SHOP_POOL_CLEANER||item==SHOP_ROOF_CLEANER||item==SHOP_RAKE)render_anim(&assets[item==SHOP_POOL_CLEANER?A_POOL_CLEANER:item==SHOP_ROOF_CLEANER?A_ROOF_CLEANER:A_RAKE],x,y,0,.36f,WHITE,0);else render_image(&assets[A_SHOP_ICONS],x-12,y-24,24,24,item,WHITE);}
static void draw_store(void){
 render_image(&assets[A_SHOP_SCENE],0,0,480,272,0,WHITE);render_anim(&assets[A_DAVE_IDLE],66,272,game.time,.88f,WHITE,1);int first=store_page_start(store_cursor),end=first==0?8:first==8?14:first==14?22:first==22?28:33,page=first==0?0:first==8?1:first==14?2:first==22?3:4;char label[160];snprintf(label,sizeof(label),"Crazy Dave's Twiddydinkies   $%lu",player.money*10);render_image(&assets[A_UI_BUTTON],10,0,460,25,0,WHITE);ui_center(18,3,444,label,UI_GOLD,.85f);
 for(int i=first;i<end;i++){int slot=i-first,item=shop_visible_items[i],x=(slot<4?236:204)+slot%4*28,y=slot<4?144:184;render_rect(x-12,y-28,24,38,RGBA(44,28,16,140));draw_store_icon(item,x,y);int available=profile_item_available(&player,item);if(i==store_cursor)render_focus(x-13,y-29,26,40,FOCUS_SAGE);if(!available)snprintf(label,sizeof(label),"%s",player.purchases[item]?"Owned":"Locked");else snprintf(label,sizeof(label),"$%d",profile_item_cost(&player,item)*10);ui_center(x-11,y+1,22,label,available?UI_GOLD:RGBA(210,210,190,255),.53f);}
 int item=shop_visible_items[store_cursor];char name[96];ui_title_case(name,sizeof(name),shop_item_names[item]);render_image(&assets[A_SHOP_BUBBLE],9,39,157,76,0,WHITE);disc_text(22,48,130,name,UI_INK,.76f);snprintf(label,sizeof(label),"$%d",profile_item_cost(&player,item)*10);disc_text(22,91,130,label,UI_INK,.7f);render_image(&assets[A_UI_BUTTON],173,205,209,25,0,WHITE);ui_center(181,208,193,name,UI_GOLD,.7f);snprintf(label,sizeof(label),"Page %d/5",page+1);ui_center(219,234,135,label,UI_GOLD,.62f);ui_hint(message_time>0?message:"D-pad: Item   L/R: Page   X: Buy   Square: Info   O: Back");
}
static void draw_imitater(void){render_background(&assets[board_background()]);ui_dialog(90,57,300,163,"Imitater");render_anim(&assets[seed_icons[imit_cursor]],240,160,game.time,.8f,WHITE,1);char title[64];ui_title_case(title,sizeof(title),full_seed_names[imit_cursor]);ui_center(112,175,256,title,WHITE,.82f);ui_hint("L/R: Choose a plant   X: Copy   O: Back");}
