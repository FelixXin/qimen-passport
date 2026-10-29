#pragma once
#include "qimen_navigation.h"
#include "reader_clock.h"
#include "reader_qimen.h"
#include "reader_network.h"
#include "reader_ble.h"
typedef struct {
 const qimen_navigation_t *navigation;const reader_qimen_t *chart;
 reader_clock_value_t clock;reader_network_info_t network;reader_ble_info_t bluetooth;
 int battery;const char *notice;
} qimen_view_t;
/* Caller holds BSP LVGL lock. */
void qimen_ui_create(void);
void qimen_ui_render(const qimen_view_t *view);
bool qimen_ui_scroll(int direction);
