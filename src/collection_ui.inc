enum {
    BOOK_LINE_MAX=160, BOOK_LINE_WIDTH=42, BOOK_ENTRIES=ALL_SEEDS+Z_TYPES,
    BOOK_INDEX_SIZE=8+4*BOOK_ENTRIES, BOOK_CACHE_LIMIT=64*1024,
    BOOK_OFFSET_LIMIT=1024*1024, BOOK_TITLE_MAX=128, BOOK_BODY_MAX=8192
};
static char book_title[BOOK_TITLE_MAX],book_lines[BOOK_LINE_MAX][BOOK_LINE_WIDTH+1];
static int book_loaded=-1,book_line_count;
static unsigned char *book_image;
static FILE *book_stream;
static size_t book_size;
static int book_attempted;

static void book_close(void){
    free(book_image);book_image=NULL;
    if(book_stream)fclose(book_stream);
    book_stream=NULL;book_size=0;book_attempted=0;
    book_loaded=-1;book_line_count=0;
}

static int book_read_at(size_t offset,void *output,size_t length){
    if(offset>book_size||length>book_size-offset)return 0;
    if(book_image){memcpy(output,book_image+offset,length);return 1;}
    return book_stream&&!fseek(book_stream,offset,SEEK_SET)&&
           fread(output,1,length,book_stream)==length;
}

static int book_open(const char *path){
    if(book_attempted)return book_image||book_stream;
    book_attempted=1;
    book_stream=fopen(path,"rb");
    if(!book_stream)return 0;
    long length=fseek(book_stream,0,SEEK_END)?-1:ftell(book_stream);
    if(length<BOOK_INDEX_SIZE||length>BOOK_OFFSET_LIMIT+8+BOOK_TITLE_MAX+BOOK_BODY_MAX)goto failed;
    book_size=length;
    /* Fall back to indexed reads if an older prepared book is larger or the heap is tight. */
    if(book_size<=BOOK_CACHE_LIMIT)book_image=malloc(book_size);
    if(book_image){
        if(fseek(book_stream,0,SEEK_SET)||fread(book_image,1,book_size,book_stream)!=book_size)goto failed;
        fclose(book_stream);book_stream=NULL;
    }
    uint32_t header[2];
    if(book_read_at(0,header,sizeof(header))&&header[0]==0x31425650&&header[1]==BOOK_ENTRIES)return 1;
failed:
    book_close();book_attempted=1;return 0;
}

static int wrap_book(const char *body){
    int row=0,col=0;
    memset(book_lines,0,sizeof(book_lines));
    const unsigned char *p=(const unsigned char*)body;
    while(*p&&row<BOOK_LINE_MAX-1){
        if(*p=='\n'){row++;col=0;p++;continue;}
        if(*p==' '){if(col&&col<BOOK_LINE_WIDTH)book_lines[row][col++]=' ';p++;continue;}
        const unsigned char *end=p;while(*end&&*end!=' '&&*end!='\n')end++;int length=end-p;
        if(col&&col+length>BOOK_LINE_WIDTH){row++;col=0;if(row>=BOOK_LINE_MAX-1)break;}
        while(p<end&&row<BOOK_LINE_MAX-1){
            unsigned char ch=*p++;if(ch>=128)ch='?';
            if(col==BOOK_LINE_WIDTH){row++;col=0;}
            book_lines[row][col++]=ch;
        }
    }
    return row+1;
}

static int load_book_entry(int entry){
    if(entry<0||entry>=BOOK_ENTRIES)return 0;
    if(book_loaded==entry)return book_line_count>0;
    book_loaded=entry;book_line_count=0;
    if(!book_open("data/almanac.bin"))return 0;
    char body[BOOK_BODY_MAX];
    uint32_t offset,sizes[2];
    if(!book_read_at(8+entry*4,&offset,sizeof(offset))||offset<BOOK_INDEX_SIZE||offset>=BOOK_OFFSET_LIMIT||
       !book_read_at(offset,sizes,sizeof(sizes))||sizes[0]>=sizeof(book_title)||sizes[1]>=sizeof(body))return 0;
    offset+=sizeof(sizes);
    if(!book_read_at(offset,book_title,sizes[0])||!book_read_at(offset+sizes[0],body,sizes[1]))return 0;
    book_title[sizes[0]]=0;body[sizes[1]]=0;
    book_line_count=wrap_book(body);
    return 1;
}
static int book_known(void){return book_zombies?!!(player.seen_zombies&(1u<<book_cursor)):!!(player.seen_plants&(UINT64_C(1)<<book_cursor));}
static const Animation *book_portrait(void){
    static const int ground_zombies[]={A_ZOMBIE,A_CONE,A_BUCKET,A_POLE_WALK,A_FLAG_WALK,A_PAPER,A_DOOR,A_FOOTBALL,A_DANCER,A_BACKUP};
    if(!book_zombies)return &assets[seed_icons[book_cursor]];
    if(book_cursor==Z_BOSS)return &assets[A_BOSS_EAT];
    if(book_cursor<Z_DUCK)return &assets[ground_zombies[book_cursor]];
    return &assets[A_DUCK_WALK+(book_cursor-Z_DUCK)*3];
}
static void draw_book_portrait(void){
    const Animation *portrait=book_portrait();
    if(book_zombies&&book_cursor==Z_BOSS){
        if(!portrait->loaded)return;
        float scale=fminf(150.0f/portrait->width,136.0f/portrait->height);
        float w=portrait->width*scale,h=portrait->height*scale;
        render_image(portrait,93-w*.5f,152-h*.5f,w,h,0,WHITE);
        return;
    }
    render_anim(portrait,91,189,game.time,1.1f,WHITE,1);
}
static void draw_book(void){
 render_image(&assets[A_BOOK_PAGES],0,0,480,272,book_zombies,WHITE);int known=book_known(),entry=book_cursor+(book_zombies?49:0);char label[128];ui_center(114,30,260,"Suburban Almanac",UI_GOLD,.84f);
 if(known&&load_book_entry(entry)){ui_center(13,62,160,book_title,UI_INK,.85f);draw_book_portrait();
  if(book_scroll>=book_line_count)book_scroll=book_line_count-1;
  for(int i=0;i<11&&book_scroll+i<book_line_count;i++)
   disc_text(186,72+i*14,279,book_lines[book_scroll+i],UI_INK,.64f);
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
