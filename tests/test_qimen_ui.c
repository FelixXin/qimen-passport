#include "qimen_ui.h"
#include "reader_qimen_text.h"
#include "lvgl.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static const char *output_dir;
static uint16_t frame[240*320],buffer[240*20];
static void flush(lv_display_t *d,const lv_area_t *a,uint8_t *pixels){uint16_t *p=(uint16_t*)pixels;for(int y=a->y1;y<=a->y2;y++)for(int x=a->x1;x<=a->x2;x++)frame[y*240+x]=*p++;lv_display_flush_ready(d);}
static void capture(const char *name){lv_obj_invalidate(lv_screen_active());lv_refr_now(NULL);char path[256];snprintf(path,sizeof(path),"%s/%s.ppm",output_dir,name);FILE *f=fopen(path,"wb");assert(f);fprintf(f,"P6\n240 320\n255\n");for(unsigned i=0;i<240*320;i++){uint16_t c=frame[i];unsigned char rgb[]={(unsigned char)(((c>>11)&31)*255/31),(unsigned char)(((c>>5)&63)*255/63),(unsigned char)((c&31)*255/31)};fwrite(rgb,1,3,f);}fclose(f);}
static lv_obj_t *find(lv_obj_t *o,const char *text){if(lv_obj_check_type(o,&lv_label_class)&&!strcmp(lv_label_get_text(o),text))return o;for(unsigned i=0;i<lv_obj_get_child_count(o);i++){lv_obj_t *found=find(lv_obj_get_child(o,i),text);if(found)return found;}return NULL;}
int main(int argc,char **argv){assert(argc==2);output_dir=argv[1];setbuf(stdout,NULL);lv_init();lv_display_t *d=lv_display_create(240,320);lv_display_set_color_format(d,LV_COLOR_FORMAT_RGB565);lv_display_set_buffers(d,buffer,NULL,sizeof(buffer),LV_DISPLAY_RENDER_MODE_PARTIAL);lv_display_set_flush_cb(d,flush);qimen_ui_create();qimen_navigation_t n;qimen_navigation_init(&n,0);reader_qimen_t q;assert(reader_qimen_cast(1792645200LL,&q));qimen_view_t v={.navigation=&n,.chart=&q,.battery=86,.notice="",.clock={.valid=true,.has_date=true,.year=2026,.month=10,.day=22,.hour=13,.minute=2}};
 assert(qimen_ui_name_supported("学易")&&qimen_ui_name_supported("QIMEN")&&!qimen_ui_name_supported("😀"));
 qimen_ui_render(&v);capture("grid");n.page=Q_DETAIL;qimen_ui_render(&v);capture("detail");
 /* A page must restore its complete layout after visiting settings/help. */
 n.page=Q_GRID;qimen_ui_render(&v);lv_obj_update_layout(lv_screen_active());
 char caption[96];snprintf(caption,sizeof(caption),"%s遁%u局 · %s · %s",q.yin?"阴":"阳",q.ju,(const char*[]){"上元","中元","下元"}[q.yuan],reader_qimen_term(q.term));
 lv_obj_t *chart_caption=find(lv_screen_active(),caption);assert(chart_caption);int caption_y=lv_obj_get_y(chart_caption);
 const unsigned positions[]={4,9,2,3,5,7,8,1,6};
 for(unsigned i=0;i<9;i++){char cell_text[512];reader_qimen_cell_text(&q,positions[i],n.palace==i,cell_text,sizeof(cell_text));lv_obj_t *cell=find(lv_screen_active(),cell_text);assert(cell);
  const lv_font_t *font=lv_obj_get_style_text_font(cell,0);int lines=1;for(const char *t=cell_text;*t;t++)if(*t=='\n')lines++;
  assert(lines*font->line_height+(lines-1)*lv_obj_get_style_text_line_space(cell,0)<=lv_obj_get_content_height(cell));
 }
 uint16_t initial_frame[240*320];capture("grid-baseline");memcpy(initial_frame,frame,sizeof(frame));
 const qimen_page_t visits[]={Q_MENU,Q_WIFI,Q_BLE,Q_CLOCK,Q_HELP,Q_COVER,Q_BRIGHT};
 for(unsigned k=0;k<sizeof(visits)/sizeof(visits[0]);k++){
  n.page=visits[k];qimen_ui_render(&v);n.page=Q_GRID;qimen_ui_render(&v);lv_obj_update_layout(lv_screen_active());
  assert(lv_obj_get_y(chart_caption)==caption_y);capture("grid-return");assert(!memcmp(initial_frame,frame,sizeof(frame)));
 }
 n.page=Q_DETAIL;qimen_ui_render(&v);capture("detail");
 char status[128];reader_qimen_summary_text(&q,4,status,sizeof(status));lv_obj_t *bar=find(lv_screen_active(),status);assert(bar&&!lv_obj_has_flag(bar,LV_OBJ_FLAG_HIDDEN));
 const unsigned grid[]={4,9,2,3,5,7,8,1,6};unsigned lines=0;
 for(unsigned palace=0;palace<9;palace++)for(unsigned symbol=0;symbol<7;symbol++){
  n.page=Q_SYMBOL;n.palace=palace;n.symbol=symbol;n.read_end=false;qimen_ui_render(&v);lv_obj_update_layout(lv_screen_active());assert(lv_obj_has_flag(bar,LV_OBJ_FLAG_HIDDEN));
  assert(lv_obj_has_flag(find(lv_screen_active(),"年柱"),LV_OBJ_FLAG_HIDDEN));
  const char *tab_names[]={"宫的象义","天盘干","地盘干","暗干","门","星","神"};
  for(unsigned t=0;t<7;t++)assert(!lv_obj_has_flag(find(lv_screen_active(),tab_names[t]),LV_OBJ_FLAG_HIDDEN));
  char text[2048];assert(reader_qimen_symbol_text(&q,grid[palace],(reader_qimen_symbol_t)symbol,text,sizeof(text)));lv_obj_t *article=find(lv_screen_active(),text);assert(article);lv_obj_t *body=lv_obj_get_parent(article);assert(lv_obj_get_height(body)==180&&lv_obj_get_scroll_y(body)==0);
  if(palace==8){char name[32];snprintf(name,sizeof(name),"article-%u",symbol);capture(name);}
  unsigned steps=0;while(!qimen_ui_scroll(1)){int y=lv_obj_get_scroll_y(body);qimen_ui_render(&v);lv_obj_update_layout(body);assert(lv_obj_get_scroll_y(body)==y);assert(++steps<200);lines++;}
  assert(lv_obj_get_scroll_bottom(body)<=0);int end=lv_obj_get_scroll_y(body);assert(qimen_ui_scroll(1)&&lv_obj_get_scroll_y(body)==end);
 }
 for(unsigned page=Q_MENU;page<=Q_HELP;page++){n.page=(qimen_page_t)page;n.selected=0;qimen_ui_render(&v);capture((const char*[]){"menu","wifi","ble","clock","help"}[page-Q_MENU]);}
 n.page=Q_COVER;n.sleep=false;v.cover.name[0]=0;v.cover.available=true;qimen_ui_render(&v);capture("cover-settings");
 n.page=Q_BRIGHT;n.brightness=1;qimen_ui_render(&v);capture("brightness");
 n.page=Q_WIFI;v.network.enabled=true;strcpy(v.network.ap,"Qimen-Passport");strcpy(v.network.password,"sample123");strcpy(v.network.key,"12AB34CD");qimen_ui_render(&v);capture("wifi-active");
 n.page=Q_CLOCK;n.field=1;n.year=2026;n.month=10;n.day=29;n.hour=18;n.minute=40;qimen_ui_render(&v);capture("time-editor");
 n.page=Q_MENU;n.selected=1;qimen_ui_render(&v);capture("settings-focus");
 n.page=Q_GRID;n.sleep=true;qimen_ui_render(&v);capture("standby");
 static uint8_t custom[QIMEN_COVER_BYTES];for(unsigned i=0;i<sizeof(custom);i+=2){custom[i]=0xe0;custom[i+1]=0x07;}
 v.cover.pixels=custom;v.cover.custom=true;qimen_ui_render(&v);capture("custom-cover");v.cover.pixels=NULL;v.cover.custom=false;qimen_ui_render(&v);capture("default-restored");
 n.sleep=false;q.valid=false;qimen_ui_render(&v);capture("unset");
 assert(lv_mem_test()==LV_RESULT_OK);lv_mem_monitor_t m;lv_mem_monitor(&m);printf("Native LVGL: 63 articles, %u line scrolls, stable refresh position, end boundary, all settings/standby/unset pages and pool integrity PASS; free=%lu largest=%lu\n",lines,(unsigned long)m.free_size,(unsigned long)m.free_biggest_size);
}
