/* Presentation preferences are independent of Adventure/profile saves. */
static int health_bars,classic_dancer,ui_settings_dirty;
static const char *ui_settings_path="ui-settings.dat";
static int ui_settings_read(FILE *f,void *buffer){
 uint32_t *record=buffer;
 size_t count=fread(record,1,4*sizeof(*record),f);
 int ok=(count==3*sizeof(*record)||count==4*sizeof(*record))&&record[0]==0x49555650&&record[2]<=3&&
    fgetc(f)==EOF&&!ferror(f)&&((record[1]==1&&count==3*sizeof(*record))||(record[1]==2&&count==4*sizeof(*record)&&record[3]<=1));
 return ok;
}
static int ui_settings_valid(const char *path){uint32_t record[4];return record_inspect(path,record,ui_settings_read);}
static int ui_settings_load(void){uint32_t record[4];if(!record_load(ui_settings_path,record,ui_settings_read))return 0;health_bars=record[2];classic_dancer=record[1]==2?record[3]:0;return 1;}
static int ui_settings_save(void){uint32_t record[4]={0x49555650,2,health_bars,classic_dancer};if(!record_replace(ui_settings_path,record,sizeof(record),ui_settings_valid))return 0;ui_settings_dirty=0;return 1;}
