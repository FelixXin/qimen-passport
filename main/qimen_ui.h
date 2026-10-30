#pragma once
#include "qimen_navigation.h"
#include "reader_clock.h"
#include "reader_qimen.h"
#include "reader_network.h"
#include "reader_ble.h"
#include "qimen_cover_store.h"
enum {Q_SYNC_IDLE,Q_SYNC_WAIT,Q_SYNC_OK,Q_SYNC_FAILED,Q_SYNC_OFFLINE};
typedef struct {
 const qimen_navigation_t *navigation;const reader_qimen_t *chart;
 reader_clock_value_t clock;reader_network_info_t network;reader_ble_info_t bluetooth;
 qimen_cover_view_t cover;
 unsigned network_sync_state;int64_t last_network_sync;
 int battery;const char *notice;
} qimen_view_t;
/* Caller holds BSP LVGL lock. */
void qimen_ui_create(void);
void qimen_ui_render(const qimen_view_t *view);
bool qimen_ui_scroll(int direction);
bool qimen_ui_name_supported(const char *name);
unsigned qimen_ui_updates(void);
