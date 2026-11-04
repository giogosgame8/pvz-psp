static int gallery_page_ready(void){
 if(!animation_bank_ready(&gallery_thumbnails[gallery_tab]))return 0;
 int first=gallery_cursor[gallery_tab]/8*8;const char *label;
 for(int i=first;i<first+8&&i<gallery_count();i++){
  int pose=i==gallery_cursor[gallery_tab]?gallery_pose:0;
  if(!animation_bank_ready(gallery_tab?gallery_zombie(i,pose):gallery_plant(i,pose,&label)))return 0;
  if(gallery_tab&&i>=Z_TYPES&&pose!=2&&!animation_bank_ready(gallery_head(i)))return 0;
 }return 1;
}
static void gallery_test_ui_warm(void){
 animation_warm(&assets[A_MINI_BACKGROUND]);animation_warm(&assets[A_UI_BUTTON]);animation_warm(&assets[A_CHALLENGE_FRAMES]);animation_warm(&assets[A_DISC_FONT]);
}
static int gallery_io_selftest(FILE *out){
 int old_gallery=gallery,old_tab=gallery_tab,old_focus=gallery_focus,old_pose=gallery_pose,old_cursor[2]={gallery_cursor[0],gallery_cursor[1]},pass=0,total=0;float worst=0;
 #define GIO_CHECK(label,test) do{int ok=(test);pass+=ok;total++;fprintf(out,"gallery IO %s: %s\n",label,ok?"PASS":"FAIL");fflush(out);}while(0)
 gallery_open();gallery_focus=1;
 GIO_CHECK("all 81 fallback portraits are indexed",gallery_thumbnails[0].frames==49&&gallery_thumbnails[1].frames==32);
 animation_prefetch_test_delay(25);
 for(int tab=0;tab<2;tab++)for(int page=0;page<(tab?4:7);page++){
  animation_cache_suspend();gallery_test_ui_warm();gallery_tab=tab;gallery_cursor[tab]=page*8;gallery_pose=0;
  AnimationIOStats before=animation_io_stats();uint64_t start=sceKernelGetSystemTimeWide();
  do{render_begin();uint64_t t=sceKernelGetSystemTimeWide();draw_gallery((sceKernelGetSystemTimeWide()-start)/1000000.0f);float ms=(sceKernelGetSystemTimeWide()-t)/1000.0f;if(ms>worst)worst=ms;render_end();}
  while(!gallery_page_ready()&&sceKernelGetSystemTimeWide()-start<5000000);
  AnimationIOStats after=animation_io_stats();char label[96];snprintf(label,sizeof(label),"%s page %d animates fully without synchronous reads or waits",tab?"zombie":"plant",page+1);
  GIO_CHECK(label,gallery_page_ready()&&after.sync_reads==before.sync_reads&&after.waits==before.waits&&after.bytes<=after.limit);
 }
 animation_cache_suspend();gallery_test_ui_warm();animation_prefetch_test_delay(250);gallery_tab=gallery_pose=0;gallery_cursor[0]=gallery_cursor[1]=0;
 AnimationIOStats before=animation_io_stats();uint64_t start=sceKernelGetSystemTimeWide();
 for(int n=0;n<40;n++){
  if(n%3==0)gallery_input(PSP_CTRL_RTRIGGER);
  for(int i=0;i<8;i++)gallery_input(PSP_CTRL_RIGHT);
  if(n%5==0)gallery_input(PSP_CTRL_CROSS);
  render_begin();uint64_t t=sceKernelGetSystemTimeWide();draw_gallery(n/60.0f);float ms=(sceKernelGetSystemTimeWide()-t)/1000.0f;if(ms>worst)worst=ms;render_end();
 }
 AnimationIOStats after=animation_io_stats();
 GIO_CHECK("rapid page, tab and pose changes stay nonblocking with 250 ms delayed reads",after.async_reads>before.async_reads&&after.sync_reads==before.sync_reads&&after.waits==before.waits&&worst<100&&sceKernelGetSystemTimeWide()-start<5000000);
 fprintf(out,"GALLERY TIMING: worst drawing pass %.3f ms; 250 ms delay injected per asynchronous read; bank budget %u bytes\n",worst,(unsigned)after.limit);
 animation_cache_suspend();animation_prefetch_test_delay(1000);
 Animation normal={0},background={0};int equal=animation_load(&normal,"data/gallery_plants.pva")&&animation_load(&background,"data/gallery_plants.pva")&&animation_warm(&normal);
 animation_prefetch(&background);animation_warm(&background);
 equal=equal&&normal.packed&&background.packed&&normal.packed_bytes==background.packed_bytes&&!memcmp(normal.packed,background.packed,normal.packed_bytes);
 GIO_CHECK("asynchronous bytes match a synchronous read exactly",equal);animation_free(&normal);animation_free(&background);
 Animation temporary={0};animation_load(&temporary,"data/gallery_zombies.pva");animation_prefetch(&temporary);animation_free(&temporary);
 GIO_CHECK("freeing an in-flight owner drains safely",!animation_io_stats().pending&&animation_cache_ok());
 animation_prefetch(&gallery_thumbnails[0]);animation_cache_suspend();
 GIO_CHECK("suspending the cache drains in-flight reads within its memory budget",!animation_io_stats().pending&&animation_io_stats().bytes==0&&animation_cache_ok());
 animation_prefetch_test_delay(0);gallery=old_gallery;gallery_tab=old_tab;gallery_focus=old_focus;gallery_pose=old_pose;gallery_cursor[0]=old_cursor[0];gallery_cursor[1]=old_cursor[1];
 fprintf(out,"GALLERY IO RESULT %d/%d\n",pass,total);return pass==total;
 #undef GIO_CHECK
}
