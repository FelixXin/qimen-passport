#include "qimen_navigation.h"
#include <assert.h>
#include <stdio.h>
static qimen_action_t click(qimen_navigation_t *n,qimen_key_t k){return qimen_navigation_key(n,k,E_CLICK,n->last_input+1000000);}
int main(void){
 qimen_navigation_t n;qimen_navigation_init(&n,0);int64_t epoch;
 n.year=2024;n.month=n.day=1;n.hour=n.minute=0;assert(qimen_navigation_epoch(&n,&epoch)&&epoch==1704038400LL);
 n.month=2;n.day=29;assert(qimen_navigation_epoch(&n,&epoch));n.year=2025;assert(!qimen_navigation_epoch(&n,&epoch));
 n.year=2099;n.month=12;n.day=31;n.hour=23;n.minute=59;assert(qimen_navigation_epoch(&n,&epoch)&&epoch==4102415940LL);
 n.page=Q_GRID;click(&n,K_UP);assert(n.selected==10&&n.palace==0);assert(click(&n,K_OK)==A_PREV);click(&n,K_DOWN);assert(n.selected==0);
 click(&n,K_OK);assert(n.page==Q_DETAIL);click(&n,K_DOWN);assert(n.page==Q_SYMBOL&&n.symbol==0&&n.symbol_tabs);
 for(unsigned i=1;i<7;i++){click(&n,K_DOWN);assert(n.symbol==i);}
 click(&n,K_DOWN);assert(n.symbol==0);click(&n,K_UP);assert(n.symbol==6);click(&n,K_DOWN);assert(n.symbol==0);
 click(&n,K_OK);assert(!n.symbol_tabs);
 for(unsigned symbol=0;symbol<7;symbol++){assert(n.symbol==symbol);for(int i=0;i<10;i++){assert(click(&n,K_DOWN)==A_SCROLL_DOWN&&n.symbol==symbol);}if(symbol<6)assert(qimen_navigation_turn(&n,1));}
 assert(!qimen_navigation_turn(&n,1));for(int i=6;i>0;i--){assert(qimen_navigation_turn(&n,-1));assert(n.symbol==(unsigned)i-1&&n.read_end);}assert(qimen_navigation_turn(&n,-1)&&n.page==Q_DETAIL);
 click(&n,K_UP);assert(n.page==Q_GRID&&n.palace==0);
 n.page=Q_CLOCK;n.year=2024;n.month=2;n.day=29;n.field=0;click(&n,K_UP);assert(n.year==2025&&n.day==28);
 n.field=1;n.month=1;n.day=31;click(&n,K_UP);assert(n.month==2&&n.day==28);
 n.field=4;assert(click(&n,K_OK)==A_TIME);
 n.page=Q_GRID;n.sleep=true;assert(qimen_navigation_key(&n,K_OK,E_PRESS,100000000)==A_WAKE);assert(qimen_navigation_key(&n,K_OK,E_RELEASE,100100000)==A_NONE);assert(qimen_navigation_key(&n,K_OK,E_CLICK,100280000)==A_NONE);assert(n.page==Q_GRID);assert(qimen_navigation_key(&n,K_OK,E_CLICK,100700000)==A_DRAW&&n.page==Q_DETAIL);
 puts("Navigation: full-date conversion/leaps, 11 targets, seven tabs/articles, boundary/reverse, wake gesture PASS");
}
