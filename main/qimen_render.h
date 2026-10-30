#pragma once
#include "qimen_ui.h"
/* Zero-filled semantic key: no pointers, timer ticks or struct padding as input. */
typedef struct {
 unsigned nav[17],clock[7],net_flags,ble[4],cover[4];
 int64_t epoch,last_network_sync;unsigned network_sync_state;int battery,network[5];
 char station_ssid[33],saved_ssid[33];
 char name[QIMEN_NAME_BYTES],notice[96],ap[33],password[64],key[9],ip[16],ble_name[33];
} qimen_render_key_t;
void qimen_render_key(const qimen_view_t *view,qimen_render_key_t *key);
