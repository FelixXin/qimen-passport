#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef enum {Q_GRID,Q_DETAIL,Q_SYMBOL,Q_MENU,Q_WIFI,Q_BLE,Q_CLOCK,Q_HELP,Q_COVER,Q_BRIGHT,Q_CAST_TIME} qimen_page_t;
typedef enum {K_UP,K_DOWN,K_OK} qimen_key_t;
typedef enum {E_PRESS,E_RELEASE,E_CLICK,E_DOUBLE,E_LONG} qimen_event_t;
typedef enum {A_NONE,A_DRAW,A_CAST,A_PREV,A_NEXT,A_SCROLL_UP,A_SCROLL_DOWN,A_WIFI,A_BLE,A_TIME,A_PREF,A_WAKE,A_CAST_TIME,A_STYLE,A_DEEP_SLEEP} qimen_action_t;
typedef struct {
 qimen_page_t page;unsigned selected,palace,symbol,field,brightness;
 unsigned year,month,day,hour,minute,cover_cursor;
 bool sleep,intro,wake_guard,read_end,symbol_tabs;uint8_t pressed;
 int64_t last_input,quiet_until,repeat_at;
} qimen_navigation_t;
void qimen_navigation_init(qimen_navigation_t *n,int64_t now);
qimen_action_t qimen_navigation_key(qimen_navigation_t *n,qimen_key_t key,qimen_event_t event,int64_t now);
bool qimen_navigation_turn(qimen_navigation_t *n,int direction);
bool qimen_navigation_epoch(const qimen_navigation_t *n,int64_t *epoch);
qimen_action_t qimen_navigation_tick(qimen_navigation_t *n,int64_t now);
