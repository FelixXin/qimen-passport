#include "qimen_navigation.h"
#include <string.h>
static unsigned days(unsigned y,unsigned m) {static const unsigned d[]={31,28,31,30,31,30,31,31,30,31,30,31};return d[m-1]+(m==2&&y%4==0&&(y%100!=0||y%400==0));}
void qimen_navigation_init(qimen_navigation_t *n,int64_t now) {memset(n,0,sizeof(*n));n->last_input=now;n->brightness=2;n->year=2026;n->month=n->day=1;}
bool qimen_navigation_epoch(const qimen_navigation_t *n,int64_t *epoch) {
 if(!n||!epoch||n->year<1900||n->year>2099||n->month<1||n->month>12||n->day<1||n->day>days(n->year,n->month)||n->hour>23||n->minute>59)return false;
 int64_t count=0;if(n->year<1970)for(unsigned y=n->year;y<1970;y++)count-=365+(y%4==0&&(y%100!=0||y%400==0));
 for(unsigned y=1970;y<n->year;y++)count+=365+(y%4==0&&(y%100!=0||y%400==0));
 for(unsigned m=1;m<n->month;m++)count+=days(n->year,m);
 *epoch=(count+n->day-1)*86400+n->hour*3600+n->minute*60-28800;return true;
}
bool qimen_navigation_turn(qimen_navigation_t *n,int direction) {
 if(!n||n->page!=Q_SYMBOL||(direction!=1&&direction!=-1))return false;
 if(direction>0){if(n->symbol>=6)return false;n->symbol++;n->read_end=false;}
 else if(n->symbol){n->symbol--;n->read_end=true;}else{n->page=Q_DETAIL;n->read_end=false;}
 return true;
}
static qimen_action_t back(qimen_navigation_t *n) {
 if(n->page==Q_SYMBOL)n->page=Q_DETAIL;
 else if(n->page==Q_DETAIL||n->page==Q_MENU)n->page=Q_GRID;
 else {unsigned source=n->page;n->page=Q_MENU;n->selected=source==Q_COVER?1:source==Q_CLOCK?2:source==Q_WIFI?3:source==Q_BLE?4:source==Q_BRIGHT?5:source==Q_CAST_TIME?7:6;}
 return A_DRAW;
}
qimen_action_t qimen_navigation_key(qimen_navigation_t *n,qimen_key_t key,qimen_event_t event,int64_t now) {
 if(key>K_OK||event>E_LONG)return A_NONE;
 n->last_input=now;
 /* Expire the wake guard before marking a new press as held. Otherwise every
    later short press re-arms the guard and no physical gesture can escape. */
 if(n->wake_guard&&!n->pressed&&now>=n->quiet_until)n->wake_guard=false;
 if(event==E_PRESS)n->pressed|=1u<<key;
 if(event==E_RELEASE){n->pressed&=~(1u<<key);n->repeat_at=0;}
 if(n->intro&&event!=E_RELEASE){n->intro=false;n->sleep=true;}
 if(n->sleep&&event!=E_RELEASE){n->sleep=false;n->wake_guard=true;n->quiet_until=now+300000;return A_WAKE;}
 if(n->wake_guard){if(n->pressed||now<n->quiet_until){n->quiet_until=now+300000;return A_NONE;}n->wake_guard=false;}
 /* Cover selection responds to each physical press, not delayed click grouping. */
 if(n->page==Q_COVER&&event==E_PRESS&&key!=K_OK){n->cover_cursor=(n->cover_cursor+(key==K_UP?4:1))%5;return A_DRAW;}
 if(event==E_PRESS||event==E_RELEASE)return A_NONE;
 if(event==E_DOUBLE&&key==K_OK)return A_DEEP_SLEEP;
 if(event==E_DOUBLE&&key==K_UP&&n->page==Q_GRID){n->sleep=true;return A_DRAW;}
 if(event==E_LONG&&key==K_OK){if(n->page==Q_GRID){n->page=Q_MENU;n->selected=0;return A_DRAW;}return back(n);}
 if(n->page==Q_GRID){
  if(event==E_DOUBLE&&key==K_DOWN)return A_NEXT;
  if(event==E_LONG&&key!=K_OK)return key==K_UP?A_PREV:A_NEXT;
  if(event!=E_CLICK)return A_NONE;
  if(key==K_OK){if(n->selected>=9)return n->selected==10?A_PREV:A_NEXT;n->palace=n->selected;n->page=Q_DETAIL;return A_DRAW;}
  n->selected=(n->selected+(key==K_UP?10:1))%11;if(n->selected<9)n->palace=n->selected;return A_DRAW;
 }
 if(n->page==Q_SYMBOL){
  if(event==E_CLICK&&key==K_OK){n->symbol_tabs=!n->symbol_tabs;return A_DRAW;}
  if((event==E_CLICK||event==E_LONG||event==E_DOUBLE)&&key!=K_OK){
   if(n->symbol_tabs){n->symbol=(n->symbol+(key==K_UP?6:1))%7;n->read_end=false;return A_DRAW;}
   return key==K_UP?A_SCROLL_UP:A_SCROLL_DOWN;
  }
  return A_NONE;
 }
 if(n->page==Q_HELP){
  if(event==E_CLICK&&key==K_OK)return back(n);
  if((event==E_CLICK||event==E_LONG)&&key!=K_OK)return key==K_UP?A_SCROLL_UP:A_SCROLL_DOWN;
  return A_NONE;
 }
 if(n->page==Q_CAST_TIME&&event==E_LONG&&key!=K_OK&&n->field==0){
  n->year=1900+(n->year-1900+(key==K_UP?10:190))%200;n->repeat_at=now+180000;return A_DRAW;
 }
 if(event!=E_CLICK)return A_NONE;
 if(n->page==Q_DETAIL){if(key==K_UP)return back(n);n->page=Q_SYMBOL;n->symbol=0;n->read_end=false;n->symbol_tabs=true;return A_DRAW;}
 if(n->page==Q_COVER)return key==K_OK?A_STYLE:A_NONE;
 if(n->page==Q_BRIGHT){if(key==K_OK)return back(n);n->brightness=(n->brightness+(key==K_UP?1:3))%4;return A_PREF;}
 if(n->page==Q_WIFI)return key==K_OK?A_WIFI:A_NONE;
 if(n->page==Q_BLE)return key==K_OK?A_BLE:A_NONE;
 if(n->page==Q_CLOCK||n->page==Q_CAST_TIME){
  if(key==K_OK){if(n->field<4){n->field++;return A_DRAW;}return n->page==Q_CAST_TIME?A_CAST_TIME:A_TIME;}
  unsigned *values[]={&n->year,&n->month,&n->day,&n->hour,&n->minute};
  unsigned low[]={n->page==Q_CAST_TIME?1900:2024,1,1,0,0},high[]={2099,12,days(n->year,n->month),23,59};unsigned f=n->field%5;
  unsigned value=*values[f];value=key==K_UP?(value==high[f]?low[f]:value+1):(value==low[f]?high[f]:value-1);*values[f]=value;
  unsigned limit=days(n->year,n->month);if(n->day>limit)n->day=limit;return A_DRAW;
 }
 if(n->page==Q_MENU){
  if(key!=K_OK){n->selected=(n->selected+(key==K_UP?8:1))%9;return A_DRAW;}
  switch(n->selected){
   case 0:n->page=Q_GRID;return A_CAST;
   case 1:n->page=Q_COVER;return A_DRAW;
   case 2:n->page=Q_CLOCK;n->field=0;return A_DRAW;
   case 3:n->page=Q_WIFI;return A_DRAW;
   case 4:n->page=Q_BLE;return A_BLE;
   case 5:n->page=Q_BRIGHT;return A_DRAW;
   case 6:n->page=Q_HELP;return A_DRAW;
   case 7:n->page=Q_CAST_TIME;n->field=0;return A_DRAW;
   default:n->page=Q_GRID;return A_DRAW;
  }
 }
 return A_NONE;
}
qimen_action_t qimen_navigation_tick(qimen_navigation_t *n,int64_t now){
 if(n->page!=Q_CAST_TIME||n->field||!n->repeat_at||now<n->repeat_at||!(n->pressed&3))return A_NONE;
 qimen_key_t key=n->pressed&1?K_UP:K_DOWN;
 return qimen_navigation_key(n,key,E_LONG,now);
}
