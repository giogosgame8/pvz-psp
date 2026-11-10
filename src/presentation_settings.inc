/* Presentation preferences are independent of Adventure/profile saves. */
static int health_bars,classic_dancer,ui_settings_dirty;
static const char *ui_settings_path="ui-settings.dat";
static int ui_settings_load(void){uint32_t record[4]={0};FILE *f=fopen(ui_settings_path,"rb");if(!f)return 0;size_t n=fread(record,4,4,f);int ok=record[0]==0x49555650&&record[2]<=3&&fgetc(f)==EOF&&((record[1]==1&&n==3)||(record[1]==2&&n==4&&record[3]<=1));fclose(f);if(ok){health_bars=record[2];classic_dancer=record[1]==2?record[3]:0;}return ok;}
static int ui_settings_save(void){uint32_t record[4]={0x49555650,2,health_bars,classic_dancer};char temporary[256];snprintf(temporary,sizeof(temporary),"%s.tmp",ui_settings_path);FILE *f=fopen(temporary,"wb");if(!f)return 0;int ok=fwrite(record,sizeof(record),1,f)==1;if(fclose(f))ok=0;if(ok){remove(ui_settings_path);ok=rename(temporary,ui_settings_path)==0;}if(ok)ui_settings_dirty=0;return ok;}
