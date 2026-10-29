#include "reader_stubs/platform.h"
#include "../main/reader_ble.c"
#include <assert.h>

ble_hs_cfg_t ble_hs_cfg;
static bool port_live,advertising,fail_stop,fail_deinit;
static unsigned init_calls,stop_calls,deinit_calls,stop_requests,queue_creates,queue_resets;
static int64_t now_us;
static uint32_t active_token;
size_t heap_caps_get_free_size(unsigned caps) {(void)caps;return 180000;}
size_t heap_caps_get_largest_free_block(unsigned caps) {(void)caps;return 100000;}
esp_err_t esp_read_mac(uint8_t *mac,int type) {(void)type;memset(mac,1,6);return ESP_OK;}
uint32_t esp_random(void) {return 123456;}
int64_t esp_timer_get_time(void) {return now_us;}
void vTaskDelay(unsigned ticks) {now_us+=(int64_t)ticks*1000;}
QueueHandle_t xQueueCreate(unsigned count,size_t size) {(void)count;(void)size;queue_creates++;return (void*)(uintptr_t)queue_creates;}
BaseType_t xQueueReset(QueueHandle_t q) {assert(q);queue_resets++;return pdTRUE;}
BaseType_t xQueueSend(QueueHandle_t q,const void *item,unsigned wait) {(void)q;(void)item;(void)wait;return pdTRUE;}
BaseType_t xQueueReceive(QueueHandle_t q,void *item,unsigned wait) {(void)q;(void)item;(void)wait;return 0;}
BaseType_t xTaskCreate(void (*entry)(void *),const char *name,unsigned stack,void *arg,unsigned priority,TaskHandle_t *task) {(void)entry;(void)name;(void)stack;(void)arg;(void)priority;(void)task;return pdPASS;}
void vTaskDelete(TaskHandle_t task) {(void)task;}
esp_err_t nimble_port_init(void) {assert(!port_live);port_live=true;init_calls++;return ESP_OK;}
esp_err_t nimble_port_deinit(void) {assert(port_live&&host_exited);deinit_calls++;if(fail_deinit)return ESP_FAIL;port_live=false;return ESP_OK;}
int nimble_port_stop(void) {assert(port_live&&!host_exited);stop_calls++;if(fail_stop)return ESP_FAIL;host_exited=true;advertising=false;return ESP_OK;}
void nimble_port_run(void) {}
bool ble_gap_adv_active(void) {return advertising;}
int ble_gap_adv_stop(void) {advertising=false;return 0;}
int ble_gap_adv_set_fields(const struct ble_hs_adv_fields *f) {(void)f;return 0;}
int ble_gap_adv_rsp_set_fields(const struct ble_hs_adv_fields *f) {(void)f;return 0;}
int ble_gap_adv_start(uint8_t type,void *address,int expiry,const struct ble_gap_adv_params *params,int (*callback)(struct ble_gap_event *,void *),void *arg) {(void)type;(void)address;(void)expiry;(void)params;(void)callback;(void)arg;advertising=true;return 0;}
int ble_gap_terminate(uint16_t handle,int reason) {(void)handle;(void)reason;return 0;}
int ble_hs_util_ensure_addr(int type) {(void)type;return 0;}
int ble_hs_id_infer_auto(int type,uint8_t *out) {(void)type;*out=0;return 0;}
void ble_svc_gap_init(void) {}
void ble_svc_gatt_init(void) {}
int ble_svc_gap_device_name_set(const char *name) {(void)name;return 0;}
int ble_gatts_count_cfg(const struct ble_gatt_svc_def *defs) {(void)defs;return 0;}
int ble_gatts_add_svcs(const struct ble_gatt_svc_def *defs) {(void)defs;return 0;}
int os_mbuf_append(struct os_mbuf *m,const void *data,size_t size) {(void)m;(void)data;(void)size;return 0;}
int os_mbuf_copydata(struct os_mbuf *m,unsigned offset,unsigned size,void *data) {(void)m;(void)offset;(void)size;(void)data;return 0;}
struct os_mbuf *ble_hs_mbuf_from_flat(const void *data,size_t size) {(void)data;(void)size;return NULL;}
int ble_gatts_notify_custom(uint16_t handle,uint16_t attr,struct os_mbuf *m) {(void)handle;(void)attr;(void)m;return 0;}
void reader_network_provision_session(uint32_t token) {active_token=token;}
bool reader_network_release_ble(void) {stop_requests++;return true;}

int main(void) {
    assert(reader_ble_open(0));on_sync();assert(initialized&&advertising&&active_token);
    reader_ble_close();reader_ble_tick(1);assert(stopping&&stop_requests==1&&!active_token);
    assert(!reader_ble_open(2)); /* A new window must not race teardown. */
    fail_stop=true;assert(!reader_ble_shutdown());assert(initialized&&!stopping&&!host_exited);assert(!reader_ble_open(3));
    fail_stop=false;reader_ble_tick(1000001);assert(stopping);
    fail_deinit=true;assert(!reader_ble_shutdown());assert(host_exited&&initialized&&!stopping);
    unsigned stopped=stop_calls;
    fail_deinit=false;reader_ble_tick(2000002);assert(reader_ble_shutdown());
    assert(stop_calls==stopped&&!initialized&&!stopping&&!host_exited&&!port_live);
    assert(queue_resets==2);
    now_us=3000000;
    for(unsigned i=0;i<20;i++) {
        now_us+=3000000;assert(reader_ble_open(now_us));on_sync();reader_ble_close();
        reader_ble_tick(now_us+1);assert(stopping);assert(reader_ble_shutdown());
    }
    assert(init_calls==21&&queue_creates==2&&!initialized&&!port_live);
    printf("Reader BLE: stop/deinit failures, safe retry, reopen isolation and 20 resource cycles PASS (%u stops, %u deinits)\n",stop_calls,deinit_calls);
}
