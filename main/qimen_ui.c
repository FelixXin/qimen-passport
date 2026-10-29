#include "qimen_ui.h"
#include "qimen_render.h"
#include "reader_qimen_text.h"
#include "lvgl.h"
#include "src/misc/cache/instance/lv_image_cache.h"
#include <stdio.h>
#include <string.h>
LV_FONT_DECLARE(reader_font);
LV_FONT_DECLARE(reader_chart_font);
LV_IMAGE_DECLARE(taiji_avatar);
LV_IMAGE_DECLARE(taiji_light);
static unsigned last_cover_style=99;
static lv_obj_t *cover_panel,*cover_image,*cover_name,*cover_time,*cover_date,*cover_hint,*cover_top,*cover_shade;
static lv_obj_t *setting_rows[4],*setting_titles[4],*setting_values[4];
static const uint8_t *last_cover_pixels;
static qimen_render_key_t previous_key;static bool key_valid;static unsigned update_count;
unsigned qimen_ui_updates(void){return update_count;}
static lv_image_dsc_t uploaded_image={.header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_RGB565,.w=240,.h=320,.stride=480},.data_size=240*320*2};
#define INK 0x252621
#define PAPER 0xf6f4ed
#define MUTED 0x74746c
static const unsigned grid[]={4,9,2,3,5,7,8,1,6};
static lv_obj_t *screen,*title,*battery,*subtitle,*date,*names[4],*pillars[4],*body,*heading,*article,*card,*parts[QIMEN_CARD_COUNT],*cells[9],*summary,*footer,*buttons[2],*markers[3],*rule,*tabs[QIMEN_SYMBOL_COUNT];
static qimen_page_t last_page=Q_HELP;static unsigned last_symbol,last_palace,last_selected;static int64_t last_epoch;static bool was_sleep,last_chart_valid;
static void show(lv_obj_t *o,bool yes){if(yes)lv_obj_remove_flag(o,LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(o,LV_OBJ_FLAG_HIDDEN);}
static lv_obj_t *label(lv_obj_t *parent,int x,int y,int w,bool small){lv_obj_t *o=lv_label_create(parent);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);lv_obj_set_style_text_font(o,small?&reader_chart_font:&reader_font,0);lv_obj_set_style_text_color(o,lv_color_hex(INK),0);lv_label_set_long_mode(o,LV_LABEL_LONG_WRAP);return o;}
void qimen_ui_create(void){
 key_valid=false;update_count=0;
 screen=lv_obj_create(NULL);lv_obj_remove_style_all(screen);lv_obj_set_style_bg_color(screen,lv_color_hex(PAPER),0);lv_obj_set_style_bg_opa(screen,LV_OPA_COVER,0);lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);
 title=label(screen,20,18,174,false);battery=label(screen,199,10,33,true);date=label(screen,199,32,38,true);
 subtitle=label(screen,20,83,200,true);rule=lv_obj_create(screen);lv_obj_remove_style_all(rule);lv_obj_set_pos(rule,20,104);lv_obj_set_size(rule,200,1);lv_obj_set_style_bg_color(rule,lv_color_hex(0xd4d2c9),0);lv_obj_set_style_bg_opa(rule,LV_OPA_COVER,0);
 for(unsigned i=0;i<4;i++){names[i]=label(screen,12+45*i,5,45,true);lv_label_set_text(names[i],(const char*[]){"年柱","月柱","日柱","时柱"}[i]);pillars[i]=label(screen,12+45*i,22,45,false);lv_label_set_recolor(pillars[i],true);lv_obj_set_style_text_line_space(pillars[i],-3,0);lv_obj_set_style_text_align(pillars[i],LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_align(names[i],LV_TEXT_ALIGN_CENTER,0);}
 body=lv_obj_create(screen);lv_obj_remove_style_all(body);lv_obj_set_pos(body,20,110);lv_obj_set_size(body,200,174);lv_obj_set_scroll_dir(body,LV_DIR_VER);lv_obj_set_scrollbar_mode(body,LV_SCROLLBAR_MODE_AUTO);lv_obj_set_style_pad_right(body,3,0);
 heading=label(body,0,0,197,false);article=label(body,0,34,197,false);lv_label_set_recolor(article,true);
 card=lv_obj_create(body);lv_obj_remove_style_all(card);lv_obj_set_size(card,197,153);lv_obj_remove_flag(card,LV_OBJ_FLAG_SCROLLABLE);lv_obj_set_style_border_width(card,1,0);lv_obj_set_style_border_color(card,lv_color_hex(0xd4d2c9),0);
 static const int layout[QIMEN_CARD_COUNT][4]={{8,6,110,27},{36,46,90,27},{8,110,114,27},{8,49,24,18},{136,46,52,27},{136,110,52,27},{100,28,90,18},{100,92,90,18},{8,73,180,18},{8,134,180,18},{114,8,76,18},{74,8,42,18}};
 for(unsigned i=0;i<QIMEN_CARD_COUNT;i++){const int *p=layout[i];parts[i]=label(card,p[0],p[1],p[2],i>=QIMEN_CARD_HEAVEN_GROWTH||i==QIMEN_CARD_HIDDEN);lv_obj_set_height(parts[i],p[3]);lv_label_set_long_mode(parts[i],LV_LABEL_LONG_CLIP);lv_label_set_recolor(parts[i],true);if(i==QIMEN_CARD_HEAVEN_STRENGTH||i==QIMEN_CARD_EARTH_STRENGTH)lv_obj_set_style_text_align(parts[i],LV_TEXT_ALIGN_RIGHT,0);}
 for(unsigned i=0;i<9;i++){cells[i]=label(screen,16+(i%3)*70,110+(i/3)*53,68,true);lv_obj_set_height(cells[i],51);lv_obj_set_style_text_line_space(cells[i],-3,0);lv_label_set_recolor(cells[i],true);lv_obj_set_style_text_align(cells[i],LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_border_width(cells[i],1,0);lv_obj_set_style_border_color(cells[i],lv_color_hex(0xd4d2c9),0);lv_obj_set_style_bg_opa(cells[i],LV_OPA_COVER,0);}
 for(unsigned i=0;i<3;i++){
  if(i<2){markers[i]=lv_obj_create(screen);lv_obj_remove_style_all(markers[i]);lv_obj_set_size(markers[i],7,7);lv_obj_set_style_radius(markers[i],LV_RADIUS_CIRCLE,0);lv_obj_set_style_border_width(markers[i],1,0);lv_obj_remove_flag(markers[i],LV_OBJ_FLAG_SCROLLABLE);}
  else {markers[i]=label(screen,0,0,12,true);lv_label_set_text(markers[i],"马");}
 }
 for(unsigned i=0;i<2;i++){buttons[i]=label(screen,170,i?298:83,52,true);lv_label_set_text(buttons[i],i?"下一局":"上一局");lv_obj_set_style_border_width(buttons[i],1,0);lv_obj_set_style_border_color(buttons[i],lv_color_hex(0xd4d2c9),0);}
 summary=label(screen,20,271,200,true);lv_obj_set_height(summary,18);lv_label_set_recolor(summary,true);
 footer=label(screen,20,298,200,true);lv_obj_set_height(footer,18);lv_label_set_long_mode(footer,LV_LABEL_LONG_CLIP);lv_obj_set_style_text_color(footer,lv_color_hex(MUTED),0);
 static const int tab_layout[QIMEN_SYMBOL_COUNT][3]={{16,35,82},{102,35,58},{164,35,60},{16,64,49},{69,64,49},{122,64,49},{175,64,49}};
 for(unsigned i=0;i<QIMEN_SYMBOL_COUNT;i++){
  const int *p=tab_layout[i];tabs[i]=label(screen,p[0],p[1],p[2],true);
  lv_obj_set_height(tabs[i],25);lv_label_set_text(tabs[i],i==0?"宫的象义":reader_qimen_symbol_name((reader_qimen_symbol_t)i));
  lv_obj_set_style_text_align(tabs[i],LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_pad_top(tabs[i],3,0);
  lv_obj_set_style_border_width(tabs[i],1,0);lv_obj_set_style_border_color(tabs[i],lv_color_hex(0xd4d2c9),0);
  lv_obj_set_style_bg_opa(tabs[i],LV_OPA_COVER,0);lv_obj_set_style_radius(tabs[i],3,0);
 }
 for(unsigned i=0;i<4;i++){
  setting_rows[i]=lv_obj_create(screen);lv_obj_remove_style_all(setting_rows[i]);lv_obj_set_pos(setting_rows[i],12,70+i*55);lv_obj_set_size(setting_rows[i],216,52);lv_obj_remove_flag(setting_rows[i],LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(setting_rows[i],LV_OPA_COVER,0);lv_obj_set_style_radius(setting_rows[i],5,0);lv_obj_set_style_border_width(setting_rows[i],1,0);
  setting_titles[i]=label(setting_rows[i],10,1,195,true);setting_values[i]=label(setting_rows[i],10,25,195,true);
  lv_obj_set_height(setting_titles[i],27);lv_obj_set_height(setting_values[i],27);
  lv_label_set_long_mode(setting_titles[i],LV_LABEL_LONG_CLIP);lv_label_set_long_mode(setting_values[i],LV_LABEL_LONG_CLIP);
 }
 cover_panel=lv_obj_create(screen);lv_obj_remove_style_all(cover_panel);lv_obj_set_size(cover_panel,240,320);lv_obj_remove_flag(cover_panel,LV_OBJ_FLAG_SCROLLABLE);lv_obj_set_style_bg_color(cover_panel,lv_color_hex(0x10110e),0);lv_obj_set_style_bg_opa(cover_panel,LV_OPA_COVER,0);
 cover_image=lv_image_create(cover_panel);lv_image_set_src(cover_image,&taiji_avatar);lv_obj_set_pos(cover_image,88,94);
 cover_shade=lv_obj_create(cover_panel);lv_obj_remove_style_all(cover_shade);lv_obj_set_pos(cover_shade,0,200);lv_obj_set_size(cover_shade,240,120);lv_obj_set_style_bg_color(cover_shade,lv_color_hex(0x10110e),0);
 cover_top=label(cover_panel,16,18,208,true);cover_name=label(cover_panel,16,46,208,false);cover_time=label(cover_panel,12,208,216,false);cover_date=label(cover_panel,16,267,208,true);cover_hint=label(cover_panel,16,297,208,true);
 lv_obj_t *cover_labels[]={cover_top,cover_name,cover_time,cover_date,cover_hint};
 for(unsigned i=0;i<5;i++){lv_obj_set_style_text_color(cover_labels[i],lv_color_hex(i==0||i==4?0x989c8c:0xf6f4ed),0);lv_obj_set_style_text_align(cover_labels[i],LV_TEXT_ALIGN_CENTER,0);lv_label_set_long_mode(cover_labels[i],LV_LABEL_LONG_CLIP);}
 lv_obj_set_style_text_font(cover_time,&lv_font_montserrat_48,0);lv_label_set_text(cover_top,"");show(cover_top,false);lv_label_set_text(cover_hint,"按右侧键返回");
 lv_screen_load(screen);
}
bool qimen_ui_scroll(int direction){
 if(direction!=1&&direction!=-1)return false;
 lv_obj_update_layout(body);
 int remaining=direction>0?lv_obj_get_scroll_bottom(body):lv_obj_get_scroll_top(body);if(remaining<=0)return true;
 int step=reader_font.line_height+lv_obj_get_style_text_line_space(article,0);if(step<1)step=1;if(step>remaining)step=remaining;
 lv_obj_scroll_by(body,0,direction>0?-step:step,LV_ANIM_OFF);return false;
}
bool qimen_ui_name_supported(const char *name){
 if(!qimen_cover_name_valid(name))return false;
 const unsigned char *p=(const unsigned char*)name;
 while(*p){uint32_t cp=*p++;unsigned more=cp<0x80?0:cp<0xe0?1:cp<0xf0?2:3;
  if(more){cp&=more==1?31:more==2?15:7;while(more--)cp=(cp<<6)|(*p++&63);}
  lv_font_glyph_dsc_t dsc;if(!lv_font_get_glyph_dsc(&reader_font,&dsc,cp,0)||dsc.is_placeholder)return false;
 }return true;
}
static void settings_row(unsigned i,const char *name,const char *value,bool selected,bool menu){
 show(setting_rows[i],true);lv_obj_set_style_bg_color(setting_rows[i],lv_color_hex(selected?INK:0xfbfaf5),0);lv_obj_set_style_border_color(setting_rows[i],lv_color_hex(selected?0x287e75:0xe0dfd6),0);
 lv_obj_set_style_text_font(setting_titles[i],menu?&reader_font:&reader_chart_font,0);lv_obj_set_style_text_font(setting_values[i],menu?&reader_chart_font:&reader_font,0);
 lv_obj_set_style_text_color(setting_titles[i],lv_color_hex(selected?PAPER:(menu?INK:MUTED)),0);lv_obj_set_style_text_color(setting_values[i],lv_color_hex(selected?0xc0d5ce:(menu?MUTED:INK)),0);
 lv_obj_set_y(setting_titles[i],menu?0:2);lv_obj_set_y(setting_values[i],menu?29:19);
 lv_label_set_text(setting_titles[i],name);lv_label_set_text(setting_values[i],value);
}
void qimen_ui_render(const qimen_view_t *v){
 qimen_render_key_t key;qimen_render_key(v,&key);
 if(key_valid&&!memcmp(&key,&previous_key,sizeof(key)))return;
 previous_key=key;key_valid=true;update_count++;

 bool cover_visible=v->navigation->sleep||v->navigation->intro;
 show(cover_panel,cover_visible);
 unsigned style=v->cover.style;if(style>4||(style==4&&!v->cover.pixels))style=0;
 const uint8_t *pixels=style==4?v->cover.pixels:NULL;
 if(last_cover_pixels!=pixels||last_cover_style!=style){
  lv_image_cache_drop(&uploaded_image);uploaded_image.data=pixels;lv_image_set_src(cover_image,pixels?&uploaded_image:style==1?&taiji_light:&taiji_avatar);last_cover_pixels=pixels;
  bool light=style==1||style==3;uint32_t background=light?PAPER:style==2?0x11251f:0x10110e;
  lv_obj_set_style_bg_color(cover_panel,lv_color_hex(background),0);show(cover_image,style<2||style==4);lv_obj_set_pos(cover_image,pixels?0:88,pixels?0:94);
  lv_obj_set_style_bg_opa(cover_shade,pixels?LV_OPA_80:LV_OPA_TRANSP,0);
  lv_obj_t *labels[]={cover_top,cover_name,cover_time,cover_date,cover_hint};
  for(unsigned i=0;i<5;i++){lv_obj_set_style_text_color(labels[i],lv_color_hex(light?(i==0||i==4?MUTED:INK):(i==0||i==4?0x9aa994:PAPER)),0);lv_obj_set_style_text_align(labels[i],style==3&&i!=4?LV_TEXT_ALIGN_LEFT:LV_TEXT_ALIGN_CENTER,0);}
  lv_obj_set_pos(cover_top,style==3?24:16,style==3?28:18);lv_label_set_text(cover_top,"");show(cover_top,false);
  lv_obj_set_pos(cover_name,style==3?24:16,style==3?104:46);lv_obj_set_width(cover_name,style==3?192:208);
  lv_obj_set_pos(cover_time,style==3?24:12,style==2?120:208);lv_obj_set_width(cover_time,style==3?192:216);lv_obj_set_style_text_font(cover_time,style==3?&lv_font_montserrat_28:&lv_font_montserrat_48,0);
  lv_obj_set_pos(cover_date,style==3?24:16,style==2?190:style==3?252:267);
  lv_obj_set_style_bg_color(cover_name,lv_color_hex(0x10110e),0);lv_obj_set_style_bg_opa(cover_name,pixels?LV_OPA_80:LV_OPA_TRANSP,0);
  lv_obj_set_style_bg_color(cover_top,lv_color_hex(0x10110e),0);lv_obj_set_style_bg_opa(cover_top,pixels?LV_OPA_80:LV_OPA_TRANSP,0);last_cover_style=style;
 }
 lv_label_set_text(cover_name,v->cover.name[0]?v->cover.name:"奇门随身盘");
 if(cover_visible){
  lv_label_set_text_fmt(cover_time,v->clock.valid?"%02u:%02u":"--:--",v->clock.hour,v->clock.minute);
  if(v->clock.has_date)lv_label_set_text_fmt(cover_date,"%04u / %02u / %02u",v->clock.year,v->clock.month,v->clock.day);else lv_label_set_text(cover_date,"设置日期与时间");
  was_sleep=true;return;
 }
 for(unsigned i=0;i<4;i++)show(setting_rows[i],false);
 const qimen_navigation_t *n=v->navigation;const reader_qimen_t *q=v->chart;bool chart=n->page<=Q_SYMBOL&&q->valid&&!n->sleep;bool overview=chart&&n->page==Q_GRID,detail=chart&&n->page==Q_DETAIL,symbol=chart&&n->page==Q_SYMBOL;
 bool changed=last_chart_valid!=q->valid||last_page!=n->page||last_symbol!=n->symbol||last_palace!=n->palace||last_epoch!=q->epoch_utc||was_sleep!=n->sleep;
 show(title,!chart||symbol);show(date,chart&&!symbol);show(subtitle,!symbol);
 lv_obj_set_pos(title,20,symbol?5:18);lv_obj_set_style_text_font(title,&reader_font,0);
 lv_obj_set_y(subtitle,83);
 lv_obj_set_y(rule,symbol?96:104);lv_obj_set_pos(body,20,symbol?104:110);
 for(unsigned i=0;i<QIMEN_SYMBOL_COUNT;i++){
  show(tabs[i],symbol);bool active=n->symbol==i;
  lv_obj_set_style_bg_color(tabs[i],lv_color_hex(active?INK:PAPER),0);
  lv_obj_set_style_text_color(tabs[i],lv_color_hex(active?PAPER:INK),0);
  lv_obj_set_style_border_color(tabs[i],lv_color_hex(active&&n->symbol_tabs?0xb87928:0xd4d2c9),0);
  lv_obj_set_style_border_width(tabs[i],active&&n->symbol_tabs?2:1,0);
 }
show(summary,chart&&!symbol&&(!overview||n->selected<9));show(body,!overview);show(card,detail);show(heading,!detail);show(article,!detail);
 for(unsigned i=0;i<4;i++){show(names[i],chart&&!symbol);show(pillars[i],chart&&!symbol);}
 for(unsigned i=0;i<9;i++)show(cells[i],overview);
 for(unsigned i=0;i<3;i++)show(markers[i],false);
 for(unsigned i=0;i<2;i++)show(buttons[i],overview);
 lv_label_set_text_fmt(battery,v->battery<0?"--":"%d%%",v->battery);lv_label_set_text(footer,v->notice);lv_obj_set_width(footer,overview?142:200);lv_obj_set_width(subtitle,overview?142:200);lv_obj_set_height(body,symbol?180:156);lv_label_set_recolor(article,chart);lv_obj_set_pos(article,0,34);
 if(chart){
  bool content_changed=last_chart_valid!=q->valid||last_epoch!=q->epoch_utc||last_page!=n->page||was_sleep;
  char text[READER_QIMEN_SYMBOL_CAPACITY];if(content_changed){for(unsigned i=0;i<4;i++){reader_qimen_pillar_column_text(q,i,text,sizeof(text));lv_label_set_text(pillars[i],text);}
  lv_label_set_text_fmt(date,"%04u\n%02u-%02u\n%02u:%02u",q->year,q->month,q->day,q->hour,q->minute);
  lv_label_set_text_fmt(subtitle,"%s遁%u局 · %s · %s",q->yin?"阴":"阳",q->ju,(const char*[]){"上元","中元","下元"}[q->yuan],reader_qimen_term(q->term));}
  unsigned p=grid[n->palace%9];if(!symbol&&(!overview||n->selected<9)&&(content_changed||last_palace!=n->palace)){reader_qimen_summary_text(q,p,text,sizeof(text));lv_label_set_text(summary,text);}
  if(overview){
   for(unsigned i=0;i<9;i++){bool selected=n->selected<9&&n->palace==i;bool old_selected=last_selected<9&&last_palace==i;if(!content_changed&&selected==old_selected)continue;reader_qimen_cell_text(q,grid[i],selected,text,sizeof(text));lv_label_set_text(cells[i],text);lv_obj_set_style_bg_color(cells[i],lv_color_hex(selected?INK:PAPER),0);lv_obj_set_style_text_color(cells[i],lv_color_hex(selected?PAPER:INK),0);}
   for(unsigned i=0;i<2;i++){bool focused=n->selected==(i?9:10);lv_obj_set_style_text_color(buttons[i],lv_color_hex(focused?0xc45b00:INK),0);}
   for(unsigned i=0;i<3;i++){unsigned mark=i<2?q->void_palace[i]:q->horse_palace;if(i==1&&mark==q->void_palace[0])continue;for(unsigned j=0;j<9;j++)if(grid[j]==mark){lv_obj_set_pos(markers[i],16+(j%3)*70+(i<2?3:53),110+(j/3)*53+(i<2?7:0));bool selected=n->selected<9&&n->palace==j;
    if(i<2)lv_obj_set_style_border_color(markers[i],lv_color_hex(selected?0xbac4ff:0x5e6aa0),0);
    else {lv_obj_set_style_text_color(markers[i],lv_color_hex(selected?0x42d6cb:0x007f79),0);}
    show(markers[i],true);}}
   if(!v->notice[0]){if(v->clock.has_date)lv_label_set_text_fmt(footer,"%04u-%02u-%02u %02u:%02u",v->clock.year,v->clock.month,v->clock.day,v->clock.hour,v->clock.minute);else lv_label_set_text(footer,"长按确定：设置");}
  }else if(detail){
   if(changed){for(unsigned i=0;i<QIMEN_CARD_COUNT;i++){reader_qimen_card_text(q,p,(reader_qimen_card_field_t)i,text,sizeof(text));lv_label_set_text(parts[i],text);}}
   lv_label_set_text(footer,"下键或确定：符号象义");
  }else{
   if(changed){reader_qimen_symbol_title(q,p,(reader_qimen_symbol_t)n->symbol,text,sizeof(text));lv_label_set_text(heading,text);
   if(!reader_qimen_symbol_text(q,p,(reader_qimen_symbol_t)n->symbol,text,sizeof(text)))strcpy(text,"象义生成失败");
   lv_label_set_text(article,text);}
   lv_label_set_text_fmt(title,"符号象义 · %u/7",n->symbol+1);
   lv_label_set_text(footer,n->symbol_tabs?"上/下选类 确定查看":"确定选类 长按确定返回");
  }
 }else{
  lv_label_set_text(title,"奇门遁甲");lv_label_set_text(subtitle,"长按确定返回");lv_label_set_text(heading,"");lv_label_set_text(article,"");
  if(n->sleep){lv_label_set_text(title,"时钟");if(v->clock.valid)lv_label_set_text_fmt(heading,"%02u:%02u",v->clock.hour,v->clock.minute);else lv_label_set_text(heading,"--:--");if(v->clock.has_date)lv_label_set_text_fmt(article,"%04u-%02u-%02u",v->clock.year,v->clock.month,v->clock.day);lv_label_set_text(footer,"按右侧键返回");}
  else if(n->page<=Q_SYMBOL){lv_label_set_text(heading,"请先校时");lv_label_set_text(article,"长按确定打开设置\n本地设置完整日期\n或用 Wi-Fi 网页校时\n校时后确定起盘");}
  else if(n->page==Q_HELP){lv_label_set_text(title,"按键说明");lv_obj_set_y(article,0);lv_obj_set_y(body,70);lv_obj_set_height(body,216);lv_label_set_text(article,"上下选择宫位\n确定查看详情\n长按上下切换前后局\n双击下键切下一局\n双击确定起当前盘\n详情进入七类象义\n正文上下逐行滚动\n读完再按下翻篇\n确定回分类选择\n长按确定逐级返回\n双击上键查看封面\n按键唤醒后返回\n断电后重新校时");lv_label_set_text(subtitle,"上/下滚动 · 长按确定返回");lv_obj_set_y(subtitle,42);lv_obj_set_y(rule,64);}
  else {
   show(body,false);lv_obj_set_y(subtitle,42);lv_obj_set_y(rule,64);lv_label_set_text(footer,v->notice[0]?v->notice:"长按确定返回");
   char text[96];
   if(n->page==Q_MENU){
    lv_label_set_text(title,"设置");lv_label_set_text_fmt(subtitle,"显示与连接 · %u/9",n->selected+1);
    const char *items[]={"当前起盘","封面与名称","日期与时间","Wi-Fi 网络","蓝牙校时","屏幕亮度","按键说明","指定时间起盘","返回排盘"};
    const char *values[]={"按现在时间重新起盘",v->cover.custom?"自定义图像":"太极封面",v->clock.has_date?"已校时":"等待校时",v->network.enabled?"已开启":"未开启",v->bluetooth.window?"等待连接":"按需开启","", "查看全部操作","1900-2099 年","继续查看当前盘"};
    unsigned first=n->selected>3?n->selected-3:0;
    for(unsigned i=0;i<4&&first+i<9;i++){unsigned at=first+i;snprintf(text,sizeof(text),"%u%%",(n->brightness+1)*25);settings_row(i,items[at],at==5?text:values[at],at==n->selected,true);}
    lv_label_set_text(footer,"上/下选择 · 确定进入");
   }else if(n->page==Q_WIFI){
    lv_label_set_text(title,"Wi-Fi 网络");lv_label_set_text(subtitle,v->network.enabled?"已开启 · 确定关闭":"未开启 · 确定开启");
    if(v->network.enabled){settings_row(0,"设备热点",v->network.ap,false,false);settings_row(1,"热点密码",v->network.password,false,false);settings_row(2,"网页管理码",v->network.key,false,false);settings_row(3,"浏览器打开","192.168.4.1",false,false);if(v->network.connected)lv_label_set_text_fmt(footer,"网络 IP %s",v->network.ip);}
    else {settings_row(0,"手机浏览器管理","连接热点即可使用",false,false);settings_row(1,"可设置","名称 / 图片 / 时间",false,false);settings_row(2,"外部 Wi-Fi","连接成功后保存",false,false);}
   }else if(n->page==Q_BLE){
    lv_label_set_text(title,"蓝牙校时");lv_label_set_text(subtitle,v->bluetooth.window?"连接窗口已开启":"确定开启连接窗口");
    if(v->bluetooth.window){settings_row(0,"设备名称",v->bluetooth.name,false,false);snprintf(text,sizeof(text),"%06u",v->bluetooth.code);settings_row(1,"连接码",text,true,false);snprintf(text,sizeof(text),"%u 秒",v->bluetooth.seconds);settings_row(2,"剩余时间",text,false,false);settings_row(3,"连接状态",v->bluetooth.authenticated?"已验证":"等待手机连接",false,false);}
    else{settings_row(0,"临时连接","三分钟后自动关闭",false,false);settings_row(1,"手机客户端","支持自定义 BLE 协议",false,false);settings_row(2,"也可以","通过 Wi-Fi 网页校时",false,false);}
   }else if(n->page==Q_CLOCK||n->page==Q_CAST_TIME){
    lv_label_set_text(title,n->page==Q_CAST_TIME?"指定时间起盘":"日期与时间");lv_label_set_text(subtitle,n->page==Q_CAST_TIME?"1900-2099 · 年份可长按":"上/下调整 · 确定下一项");
    unsigned values[]={n->year,n->month,n->day,n->hour,n->minute};const char *names[]={"年","月","日","小时","分钟"};
    unsigned first=n->field>3?n->field-3:0;for(unsigned i=0;i<4;i++){unsigned at=first+i;snprintf(text,sizeof(text),at?"%02u":"%04u",values[at]);settings_row(i,names[at],text,at==n->field,false);}
    lv_label_set_text(footer,n->field==4?(n->page==Q_CAST_TIME?"确定起盘 · 不修改时钟":"确定保存 · 长按取消"):"北京时间 · 长按取消");
   }else if(n->page==Q_COVER){
    lv_label_set_text(title,"封面与名称");lv_label_set_text(subtitle,"名称请在 Wi-Fi 网页设置");
    unsigned first=n->cover_cursor>3?n->cover_cursor-3:0;
    for(unsigned i=0;i<4;i++){unsigned at=first+i;settings_row(i,qimen_cover_style_name(at),at==v->cover.style?"正在使用":at==4&&!v->cover.custom?"请先在网页上传":"确定应用并预览",at==n->cover_cursor,true);}
    lv_label_set_text(footer,"上/下选样式 · 确定预览");
   }else if(n->page==Q_BRIGHT){
    lv_label_set_text(title,"屏幕亮度");lv_label_set_text(subtitle,"上/下调整 · 确定返回");
    const char *levels[]={"低亮","柔和","明亮","高亮"};for(unsigned i=0;i<4;i++){snprintf(text,sizeof(text),"%u%%",(i+1)*25);settings_row(i,levels[i],text,i==n->brightness,false);}
   }
  }
 }
 if(changed){lv_obj_update_layout(body);lv_obj_scroll_to_y(body,symbol&&n->read_end?LV_COORD_MAX:0,LV_ANIM_OFF);}
 last_page=n->page;last_symbol=n->symbol;last_palace=n->palace;last_selected=n->selected;last_epoch=q->epoch_utc;was_sleep=n->sleep;last_chart_valid=q->valid;
}
