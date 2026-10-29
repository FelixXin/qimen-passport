#include "qimen_ui.h"
#include "reader_qimen_text.h"
#include "lvgl.h"
#include <stdio.h>
#include <string.h>
LV_FONT_DECLARE(reader_font);
LV_FONT_DECLARE(reader_chart_font);
#define INK 0x252621
#define PAPER 0xf6f4ed
#define MUTED 0x74746c
static const unsigned grid[]={4,9,2,3,5,7,8,1,6};
static lv_obj_t *screen,*title,*battery,*subtitle,*date,*names[4],*pillars[4],*body,*heading,*article,*card,*parts[QIMEN_CARD_COUNT],*cells[9],*summary,*footer,*buttons[2],*markers[3],*rule,*tabs[QIMEN_SYMBOL_COUNT];
static qimen_page_t last_page=Q_HELP;static unsigned last_symbol,last_palace;static int64_t last_epoch;static bool was_sleep;
static void show(lv_obj_t *o,bool yes){if(yes)lv_obj_remove_flag(o,LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(o,LV_OBJ_FLAG_HIDDEN);}
static lv_obj_t *label(lv_obj_t *parent,int x,int y,int w,bool small){lv_obj_t *o=lv_label_create(parent);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);lv_obj_set_style_text_font(o,small?&reader_chart_font:&reader_font,0);lv_obj_set_style_text_color(o,lv_color_hex(INK),0);lv_label_set_long_mode(o,LV_LABEL_LONG_WRAP);return o;}
void qimen_ui_create(void){
 screen=lv_obj_create(NULL);lv_obj_remove_style_all(screen);lv_obj_set_style_bg_color(screen,lv_color_hex(PAPER),0);lv_obj_set_style_bg_opa(screen,LV_OPA_COVER,0);lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);
 title=label(screen,20,18,174,false);battery=label(screen,199,10,33,true);date=label(screen,199,32,38,true);
 subtitle=label(screen,20,83,200,true);rule=lv_obj_create(screen);lv_obj_remove_style_all(rule);lv_obj_set_pos(rule,20,104);lv_obj_set_size(rule,200,1);lv_obj_set_style_bg_color(rule,lv_color_hex(0xd4d2c9),0);lv_obj_set_style_bg_opa(rule,LV_OPA_COVER,0);
 for(unsigned i=0;i<4;i++){names[i]=label(screen,12+45*i,5,45,true);lv_label_set_text(names[i],(const char*[]){"年柱","月柱","日柱","时柱"}[i]);pillars[i]=label(screen,12+45*i,22,45,false);lv_label_set_recolor(pillars[i],true);lv_obj_set_style_text_line_space(pillars[i],-3,0);lv_obj_set_style_text_align(pillars[i],LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_align(names[i],LV_TEXT_ALIGN_CENTER,0);}
 body=lv_obj_create(screen);lv_obj_remove_style_all(body);lv_obj_set_pos(body,20,110);lv_obj_set_size(body,200,174);lv_obj_set_scroll_dir(body,LV_DIR_VER);lv_obj_set_scrollbar_mode(body,LV_SCROLLBAR_MODE_AUTO);lv_obj_set_style_pad_right(body,3,0);
 heading=label(body,0,0,197,false);article=label(body,0,34,197,false);lv_label_set_recolor(article,true);
 card=lv_obj_create(body);lv_obj_remove_style_all(card);lv_obj_set_size(card,197,153);lv_obj_remove_flag(card,LV_OBJ_FLAG_SCROLLABLE);lv_obj_set_style_border_width(card,1,0);lv_obj_set_style_border_color(card,lv_color_hex(0xd4d2c9),0);
 static const int layout[QIMEN_CARD_COUNT][4]={{8,6,110,27},{36,46,90,27},{8,110,114,27},{8,49,24,18},{136,46,52,27},{136,110,52,27},{100,28,90,18},{100,92,90,18},{8,73,180,18},{8,134,180,18},{114,8,76,18},{74,8,42,18}};
 for(unsigned i=0;i<QIMEN_CARD_COUNT;i++){const int *p=layout[i];parts[i]=label(card,p[0],p[1],p[2],i>=QIMEN_CARD_HEAVEN_GROWTH||i==QIMEN_CARD_HIDDEN);lv_obj_set_height(parts[i],p[3]);lv_label_set_long_mode(parts[i],LV_LABEL_LONG_CLIP);lv_label_set_recolor(parts[i],true);if(i==QIMEN_CARD_HEAVEN_STRENGTH||i==QIMEN_CARD_EARTH_STRENGTH)lv_obj_set_style_text_align(parts[i],LV_TEXT_ALIGN_RIGHT,0);}
 for(unsigned i=0;i<9;i++){cells[i]=label(screen,16+(i%3)*70,110+(i/3)*53,68,true);lv_obj_set_height(cells[i],51);lv_label_set_recolor(cells[i],true);lv_obj_set_style_text_align(cells[i],LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_border_width(cells[i],1,0);lv_obj_set_style_border_color(cells[i],lv_color_hex(0xd4d2c9),0);lv_obj_set_style_bg_opa(cells[i],LV_OPA_COVER,0);}
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
 lv_screen_load(screen);
}
bool qimen_ui_scroll(int direction){
 if(direction!=1&&direction!=-1)return false;
 lv_obj_update_layout(body);
 int remaining=direction>0?lv_obj_get_scroll_bottom(body):lv_obj_get_scroll_top(body);if(remaining<=0)return true;
 int step=reader_font.line_height+lv_obj_get_style_text_line_space(article,0);if(step<1)step=1;if(step>remaining)step=remaining;
 lv_obj_scroll_by(body,0,direction>0?-step:step,LV_ANIM_OFF);return false;
}
void qimen_ui_render(const qimen_view_t *v){
 const qimen_navigation_t *n=v->navigation;const reader_qimen_t *q=v->chart;bool chart=n->page<=Q_SYMBOL&&q->valid&&!n->sleep;bool overview=chart&&n->page==Q_GRID,detail=chart&&n->page==Q_DETAIL,symbol=chart&&n->page==Q_SYMBOL;
 bool changed=last_page!=n->page||last_symbol!=n->symbol||last_palace!=n->palace||last_epoch!=q->epoch_utc||was_sleep!=n->sleep;
 show(title,!chart||symbol);show(date,chart&&!symbol);show(subtitle,!symbol);
 lv_obj_set_pos(title,20,symbol?5:18);lv_obj_set_style_text_font(title,&reader_font,0);
 lv_obj_set_y(rule,symbol?96:104);lv_obj_set_pos(body,20,symbol?104:110);
 for(unsigned i=0;i<QIMEN_SYMBOL_COUNT;i++){
  show(tabs[i],symbol);bool active=n->symbol==i;
  lv_obj_set_style_bg_color(tabs[i],lv_color_hex(active?INK:PAPER),0);
  lv_obj_set_style_text_color(tabs[i],lv_color_hex(active?PAPER:INK),0);
  lv_obj_set_style_border_color(tabs[i],lv_color_hex(active&&n->symbol_tabs?0xb87928:0xd4d2c9),0);
  lv_obj_set_style_border_width(tabs[i],active&&n->symbol_tabs?2:1,0);
 }
show(summary,chart&&!symbol);show(body,!overview);show(card,detail);show(heading,!detail);show(article,!detail);
 for(unsigned i=0;i<4;i++){show(names[i],chart&&!symbol);show(pillars[i],chart&&!symbol);}
 for(unsigned i=0;i<9;i++)show(cells[i],overview);
 for(unsigned i=0;i<3;i++)show(markers[i],false);
 for(unsigned i=0;i<2;i++)show(buttons[i],overview);
 lv_label_set_text_fmt(battery,v->battery<0?"--":"%d%%",v->battery);lv_label_set_text(footer,v->notice);lv_obj_set_width(footer,overview?142:200);lv_obj_set_width(subtitle,overview?142:200);lv_obj_set_height(body,symbol?180:156);lv_label_set_recolor(article,chart);lv_obj_set_pos(article,0,34);
 if(chart){
  char text[READER_QIMEN_SYMBOL_CAPACITY];for(unsigned i=0;i<4;i++){reader_qimen_pillar_column_text(q,i,text,sizeof(text));lv_label_set_text(pillars[i],text);}
  lv_label_set_text_fmt(date,"%04u\n%02u-%02u\n%02u:%02u",q->year,q->month,q->day,q->hour,q->minute);
  lv_label_set_text_fmt(subtitle,"%s遁%u局 · %s · %s",q->yin?"阴":"阳",q->ju,(const char*[]){"上元","中元","下元"}[q->yuan],reader_qimen_term(q->term));
  unsigned p=grid[n->palace%9];if(!symbol){reader_qimen_summary_text(q,p,text,sizeof(text));lv_label_set_text(summary,text);}
  if(overview){
   for(unsigned i=0;i<9;i++){bool selected=n->palace==i;reader_qimen_cell_text(q,grid[i],selected,text,sizeof(text));lv_label_set_text(cells[i],text);lv_obj_set_style_bg_color(cells[i],lv_color_hex(selected?INK:PAPER),0);lv_obj_set_style_text_color(cells[i],lv_color_hex(selected?PAPER:INK),0);}
   for(unsigned i=0;i<2;i++){bool focused=n->selected==(i?9:10);lv_obj_set_style_text_color(buttons[i],lv_color_hex(focused?0xc45b00:INK),0);}
   for(unsigned i=0;i<3;i++){unsigned mark=i<2?q->void_palace[i]:q->horse_palace;if(i==1&&mark==q->void_palace[0])continue;for(unsigned j=0;j<9;j++)if(grid[j]==mark){lv_obj_set_pos(markers[i],16+(j%3)*70+(i<2?3:53),110+(j/3)*53+(i<2?7:0));bool selected=n->palace==j;
    if(i<2)lv_obj_set_style_border_color(markers[i],lv_color_hex(selected?0xbac4ff:0x5e6aa0),0);
    else {lv_obj_set_style_text_color(markers[i],lv_color_hex(selected?0x42d6cb:0x007f79),0);}
    show(markers[i],true);}}
   if(!v->notice[0]){if(v->clock.has_date)lv_label_set_text_fmt(footer,"现在%04u-%02u-%02u %02u:%02u",v->clock.year,v->clock.month,v->clock.day,v->clock.hour,v->clock.minute);else lv_label_set_text(footer,"长按确定：设置");}
  }else if(detail){
   for(unsigned i=0;i<QIMEN_CARD_COUNT;i++){reader_qimen_card_text(q,p,(reader_qimen_card_field_t)i,text,sizeof(text));lv_label_set_text(parts[i],text);}lv_label_set_text(footer,"下键或确定：符号象义");
  }else{
   reader_qimen_symbol_title(q,p,(reader_qimen_symbol_t)n->symbol,text,sizeof(text));lv_label_set_text(heading,text);
   if(!reader_qimen_symbol_text(q,p,(reader_qimen_symbol_t)n->symbol,text,sizeof(text)))strcpy(text,"象义生成失败");
   lv_label_set_text(article,text);
   lv_label_set_text_fmt(title,"符号象义 · %u/7",n->symbol+1);
   lv_label_set_text(footer,n->symbol_tabs?"上/下选类 确定查看":"确定选类 长按确定返回");
  }
 }else{
  lv_label_set_text(title,"奇门遁甲");lv_label_set_text(subtitle,"长按确定返回");lv_label_set_text(heading,"");lv_label_set_text(article,"");
  if(n->sleep){lv_label_set_text(title,"时钟");if(v->clock.valid)lv_label_set_text_fmt(heading,"%02u:%02u",v->clock.hour,v->clock.minute);else lv_label_set_text(heading,"--:--");if(v->clock.has_date)lv_label_set_text_fmt(article,"%04u-%02u-%02u",v->clock.year,v->clock.month,v->clock.day);lv_label_set_text(footer,"按右侧键返回");}
  else if(n->page<=Q_SYMBOL){lv_label_set_text(heading,"请先校时");lv_label_set_text(article,"长按确定打开设置\n本地设置完整日期\n或用 Wi-Fi 网页校时\n校时后确定起盘");}
  else if(n->page==Q_MENU){lv_label_set_text(title,"设置");static const char *items[]={"当前起盘","Wi-Fi 设置","蓝牙校时","本地校时","屏幕亮度","时钟待机","按键说明","返回"};char text[512]={0};size_t used=0;unsigned first=n->selected>4?n->selected-4:0;for(unsigned i=first;i<8&&i<first+5;i++)used+=(size_t)snprintf(text+used,sizeof(text)-used,"%s%s\n",n->selected==i?"> ":"  ",items[i]);lv_obj_set_y(article,0);lv_label_set_text(article,text);lv_label_set_text_fmt(subtitle,"亮度 %u%%",(n->brightness+1)*25);}
  else if(n->page==Q_WIFI){lv_label_set_text(title,"Wi-Fi 设置");lv_label_set_text_fmt(subtitle,"确定%s",v->network.enabled?"关闭":"开启");if(v->network.enabled)lv_label_set_text_fmt(article,"热点 %s\n密码 %s\n管理码 %s\n192.168.4.1\n设备 IP %s\n%s",v->network.ap,v->network.password,v->network.key,v->network.ip,v->network.message);else lv_label_set_text(article,"开启后连接设备热点\n用浏览器校时和配网");lv_obj_set_y(article,0);}
  else if(n->page==Q_BLE){lv_label_set_text(title,"蓝牙校时");lv_obj_set_y(article,0);if(v->bluetooth.window)lv_label_set_text_fmt(article,"设备 %s\n连接码 %06u\n剩余 %u 秒\n%s",v->bluetooth.name,v->bluetooth.code,v->bluetooth.seconds,v->bluetooth.authenticated?"已验证":"等待连接");else lv_label_set_text(article,"确定开启三分钟连接\n使用支持自定义 BLE\n的客户端发送当前时间\n系统蓝牙不会自动校时");}
  else if(n->page==Q_CLOCK){lv_label_set_text(title,"本地校时");lv_label_set_text_fmt(heading,"%04u-%02u-%02u",n->year,n->month,n->day);lv_label_set_text_fmt(article,"%02u:%02u\n正在调整：%s\n上加 / 下减\n确定切换，最后保存",n->hour,n->minute,(const char*[]){"年","月","日","小时","分钟"}[n->field%5]);}
  else if(n->page==Q_HELP){lv_label_set_text(title,"按键说明");lv_obj_set_y(article,0);lv_label_set_text(article,"九宫上/下选择\n确定查看宫位\n长按上/下切前后局\n双击确定回当前盘\n详情下键进入象义\n象义上/下每次一行\n读完再按下到下一篇\n确定返回详情\n长按确定逐级返回\n长按确定打开设置\n双击上键时钟待机\n待机按右侧键唤醒\n断电后需要重新校时");}
 }
 if(changed){lv_obj_update_layout(body);lv_obj_scroll_to_y(body,symbol&&n->read_end?LV_COORD_MAX:0,LV_ANIM_OFF);}
 last_page=n->page;last_symbol=n->symbol;last_palace=n->palace;last_epoch=q->epoch_utc;was_sleep=n->sleep;
}
