#include "qimen_render.h"
#include <stdio.h>
#include <string.h>
void qimen_render_key(const qimen_view_t *v,qimen_render_key_t *k){
 memset(k,0,sizeof(*k));const qimen_navigation_t *n=v->navigation;
 unsigned fields[]={n->page,n->selected,n->palace,n->symbol,n->field,n->brightness,n->year,n->month,n->day,n->hour,n->minute,n->cover_cursor,n->sleep,n->intro,n->symbol_tabs,n->read_end,v->chart->valid};
 memcpy(k->nav,fields,sizeof(fields));k->epoch=v->chart->valid?v->chart->epoch_utc:0;k->battery=v->battery;
 if(n->sleep||n->intro||n->page==Q_GRID){unsigned clock[]={v->clock.valid,v->clock.has_date,v->clock.year,v->clock.month,v->clock.day,v->clock.hour,v->clock.minute};memcpy(k->clock,clock,sizeof(clock));}
 else if(n->page==Q_MENU)k->clock[1]=v->clock.has_date;
 k->cover[0]=v->cover.generation;k->cover[1]=v->cover.style;k->cover[2]=v->cover.custom;k->cover[3]=v->cover.available;
 snprintf(k->name,sizeof(k->name),"%s",v->cover.name);snprintf(k->notice,sizeof(k->notice),"%s",v->notice?v->notice:"");
 if(n->page==Q_MENU||n->page==Q_WIFI)k->net_flags=v->network.enabled|(v->network.connected<<1);
 if(n->page==Q_WIFI||n->page==Q_WIFI_DETAIL){k->network_sync_state=v->network_sync_state;k->last_network_sync=v->last_network_sync;k->net_flags=v->network.enabled|(v->network.connected<<1)|(v->network.connecting<<2)|(v->network.portal<<3)|(v->network.ap_pending<<4)|(v->network.request_done<<5)|(v->network.provisioned<<6);
  int values[]={v->network.ap_clients,v->network.rssi,(int)v->network.last_error,(int)v->network.disconnect_reason,(int)v->network.retry_count};memcpy(k->network,values,sizeof(values));
  snprintf(k->station_ssid,sizeof(k->station_ssid),"%s",v->network.station_ssid);snprintf(k->saved_ssid,sizeof(k->saved_ssid),"%s",v->network.saved_ssid);
  snprintf(k->ap,sizeof(k->ap),"%s",v->network.ap);snprintf(k->password,sizeof(k->password),"%s",v->network.password);snprintf(k->key,sizeof(k->key),"%s",v->network.key);snprintf(k->ip,sizeof(k->ip),"%s",v->network.ip);}
 if(n->page==Q_MENU||n->page==Q_BLE)k->ble[0]=v->bluetooth.window;
 if(n->page==Q_BLE){k->ble[1]=v->bluetooth.code;k->ble[2]=v->bluetooth.seconds;k->ble[3]=v->bluetooth.authenticated;snprintf(k->ble_name,sizeof(k->ble_name),"%s",v->bluetooth.name);}
}
