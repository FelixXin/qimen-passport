#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_NO_MEM 0x101
#define ESP_ERR_INVALID_ARG 0x102
#define ESP_ERR_INVALID_STATE 0x103
#define ESP_ERR_INVALID_SIZE 0x104
#define ESP_ERR_TIMEOUT 0x107
#define ESP_ERR_WIFI_NOT_STARTED 0x3002
#define ESP_ERR_NVS_NOT_FOUND 0x1102
#define ESP_ERR_NVS_NOT_ENOUGH_SPACE 0x1105
typedef unsigned nvs_handle_t;
#define NVS_READONLY 0
#define NVS_READWRITE 1
esp_err_t nvs_open(const char *, int, nvs_handle_t *);
void nvs_close(nvs_handle_t);
esp_err_t nvs_get_blob(nvs_handle_t, const char *, void *, size_t *);
esp_err_t nvs_get_str(nvs_handle_t, const char *, char *, size_t *);
esp_err_t nvs_set_blob(nvs_handle_t, const char *, const void *, size_t);
esp_err_t nvs_set_u8(nvs_handle_t, const char *, uint8_t);
esp_err_t nvs_commit(nvs_handle_t);
typedef unsigned BaseType_t;
typedef void *QueueHandle_t;
typedef void *TaskHandle_t;
typedef unsigned portMUX_TYPE;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(lock) ((void)(lock))
#define portEXIT_CRITICAL(lock) ((void)(lock))
#define pdTRUE 1
#define pdPASS 1
#define portMAX_DELAY UINT32_MAX
#define pdMS_TO_TICKS(ms) (ms)
QueueHandle_t xQueueCreate(unsigned,size_t);
BaseType_t xQueueSend(QueueHandle_t,const void *,unsigned);
BaseType_t xQueueReceive(QueueHandle_t,void *,unsigned);
void vQueueDelete(QueueHandle_t);
BaseType_t xTaskCreate(void (*)(void *),const char *,unsigned,void *,unsigned,TaskHandle_t *);
unsigned uxTaskGetStackHighWaterMark(TaskHandle_t);
void vTaskDelay(unsigned);
uint32_t esp_random(void);

typedef struct {int kind;} esp_netif_t;
typedef struct {int kind;} esp_netif_config_t;
#define ESP_NETIF_DEFAULT_WIFI_AP() {1}
#define ESP_NETIF_DEFAULT_WIFI_STA() {2}
esp_err_t esp_netif_init(void);
esp_netif_t *esp_netif_new(const esp_netif_config_t *);
void esp_netif_destroy_default_wifi(void *);
esp_err_t esp_netif_attach_wifi_ap(esp_netif_t *);
esp_err_t esp_netif_attach_wifi_station(esp_netif_t *);
esp_err_t esp_wifi_set_default_wifi_ap_handlers(void);
esp_err_t esp_wifi_set_default_wifi_sta_handlers(void);
typedef const char *esp_event_base_t;
typedef void *esp_event_handler_instance_t;
extern const char *WIFI_EVENT;
extern const char *IP_EVENT;
#define ESP_EVENT_ANY_ID -1
#define IP_EVENT_STA_GOT_IP 2
#define WIFI_EVENT_STA_DISCONNECTED 3
#define IP_EVENT_STA_LOST_IP 4
#define WIFI_EVENT_AP_STACONNECTED 5
#define WIFI_EVENT_AP_STADISCONNECTED 6
typedef void (*event_callback_t)(void *,esp_event_base_t,int32_t,void *);
esp_err_t esp_event_loop_create_default(void);
esp_err_t esp_event_handler_instance_register(esp_event_base_t,int32_t,event_callback_t,void *,esp_event_handler_instance_t *);
esp_err_t esp_event_handler_instance_unregister(esp_event_base_t,int32_t,esp_event_handler_instance_t);
typedef struct {int unused;} wifi_init_config_t;
#define WIFI_INIT_CONFIG_DEFAULT() {0}
typedef union {
    struct {uint8_t ssid[32],password[64];struct {bool capable;} pmf_cfg;} sta;
    struct {uint8_t ssid[32],password[64];uint8_t ssid_len,channel,max_connection;int authmode;} ap;
} wifi_config_t;
typedef struct {uint8_t ssid[33];int8_t rssi;} wifi_ap_record_t;
typedef struct {uint8_t reason;} wifi_event_sta_disconnected_t;
typedef struct {int num;} wifi_sta_list_t;
esp_err_t esp_wifi_ap_get_sta_list(wifi_sta_list_t *);
typedef struct {struct {uint32_t ip;} ip_info;} ip_event_got_ip_t;
#define IPSTR "%u.%u.%u.%u"
#define IP2STR(ip) ((void)(ip),1u),2u,3u,4u
#define WIFI_STORAGE_RAM 0
#define WIFI_MODE_APSTA 1
#define WIFI_MODE_STA 2
#define WIFI_IF_AP 0
#define WIFI_IF_STA 1
#define WIFI_AUTH_WPA2_PSK 3
esp_err_t esp_wifi_init(const wifi_init_config_t *);
esp_err_t esp_wifi_deinit(void);
esp_err_t esp_wifi_set_storage(int);
esp_err_t esp_wifi_set_mode(int);
esp_err_t esp_wifi_set_config(int,const wifi_config_t *);
esp_err_t esp_wifi_start(void);
esp_err_t esp_wifi_stop(void);
esp_err_t esp_wifi_connect(void);
esp_err_t esp_wifi_disconnect(void);
esp_err_t esp_wifi_sta_get_ap_info(wifi_ap_record_t *);

typedef void *httpd_handle_t;
typedef struct {size_t content_len;} httpd_req_t;
typedef struct {unsigned stack_size,max_open_sockets;bool lru_purge_enable;} httpd_config_t;
typedef struct {const char *uri;int method;esp_err_t (*handler)(httpd_req_t *);} httpd_uri_t;
#define HTTPD_DEFAULT_CONFIG() {0}
#define HTTP_GET 0
#define HTTP_POST 1
#define HTTPD_SOCK_ERR_TIMEOUT -2
esp_err_t httpd_start(httpd_handle_t *,const httpd_config_t *);
esp_err_t httpd_stop(httpd_handle_t);
esp_err_t httpd_register_uri_handler(httpd_handle_t,const httpd_uri_t *);
esp_err_t httpd_resp_set_status(httpd_req_t *,const char *);
esp_err_t httpd_resp_set_type(httpd_req_t *,const char *);
esp_err_t httpd_resp_set_hdr(httpd_req_t *,const char *,const char *);
esp_err_t httpd_resp_sendstr(httpd_req_t *,const char *);
esp_err_t httpd_resp_send(httpd_req_t *,const char *,size_t);
esp_err_t httpd_req_get_hdr_value_str(httpd_req_t *,const char *,char *,size_t);
int httpd_req_recv(httpd_req_t *,char *,size_t);
#define ESP_LOGI(...) ((void)0)
#define ESP_LOGW(...) ((void)0)
#define ESP_LOGE(...) ((void)0)
#define MALLOC_CAP_INTERNAL 1
#define MALLOC_CAP_DMA 2
#define MALLOC_CAP_8BIT 4
#define ESP_MAC_WIFI_STA 0
size_t heap_caps_get_free_size(unsigned);
size_t heap_caps_get_largest_free_block(unsigned);
esp_err_t esp_read_mac(uint8_t *,int);
int64_t esp_timer_get_time(void);
BaseType_t xQueueReset(QueueHandle_t);
void vTaskDelete(TaskHandle_t);

typedef struct {uint8_t type;} ble_uuid_t;
typedef struct {ble_uuid_t u;uint8_t value[16];} ble_uuid128_t;
#define BLE_UUID128_INIT(...) {{0},{__VA_ARGS__}}
#define BLE_HS_CONN_HANDLE_NONE 0xffff
#define BLE_HS_ADV_F_DISC_GEN 1
#define BLE_HS_ADV_F_BREDR_UNSUP 2
#define BLE_GAP_CONN_MODE_UND 1
#define BLE_GAP_DISC_MODE_GEN 1
#define BLE_HS_FOREVER 0
#define BLE_GATT_ACCESS_OP_READ_CHR 1
#define BLE_GATT_ACCESS_OP_WRITE_CHR 2
#define BLE_ATT_ERR_INSUFFICIENT_RES 1
#define BLE_ATT_ERR_WRITE_NOT_PERMITTED 2
#define BLE_ATT_ERR_INSUFFICIENT_AUTHOR 3
#define BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN 4
#define BLE_ATT_ERR_UNLIKELY 5
#define BLE_GATT_SVC_TYPE_PRIMARY 1
#define BLE_GATT_CHR_F_WRITE 1
#define BLE_GATT_CHR_F_READ 2
#define BLE_GATT_CHR_F_NOTIFY 4
#define BLE_GAP_EVENT_CONNECT 1
#define BLE_GAP_EVENT_DISCONNECT 2
#define BLE_GAP_EVENT_SUBSCRIBE 3
#define BLE_GAP_EVENT_ADV_COMPLETE 4
#define BLE_ERR_REM_USER_CONN_TERM 0x13
struct os_mbuf {unsigned length;};
#define OS_MBUF_PKTLEN(m) ((m)->length)
struct ble_gatt_access_ctxt {int op;struct os_mbuf *om;};
struct ble_gatt_chr_def {
    const ble_uuid_t *uuid;
    int (*access_cb)(uint16_t,uint16_t,struct ble_gatt_access_ctxt *,void *);
    uint16_t *val_handle;unsigned flags;
};
struct ble_gatt_svc_def {int type;const ble_uuid_t *uuid;struct ble_gatt_chr_def *characteristics;};
struct ble_hs_adv_fields {unsigned flags;ble_uuid128_t *uuids128;unsigned num_uuids128,uuids128_is_complete;uint8_t *name;unsigned name_len,name_is_complete;};
struct ble_gap_adv_params {unsigned conn_mode,disc_mode,itvl_min,itvl_max;};
struct ble_gap_event {
    int type;
    struct {int status;uint16_t conn_handle;} connect;
    struct {uint16_t attr_handle;bool cur_notify;} subscribe;
};
typedef struct {void (*sync_cb)(void);void (*reset_cb)(int);} ble_hs_cfg_t;
extern ble_hs_cfg_t ble_hs_cfg;
bool ble_gap_adv_active(void);
int ble_gap_adv_stop(void);
int ble_gap_adv_set_fields(const struct ble_hs_adv_fields *);
int ble_gap_adv_rsp_set_fields(const struct ble_hs_adv_fields *);
int ble_gap_adv_start(uint8_t,void *,int,const struct ble_gap_adv_params *,int (*)(struct ble_gap_event *,void *),void *);
int ble_gap_terminate(uint16_t,int);
int ble_hs_util_ensure_addr(int);
int ble_hs_id_infer_auto(int,uint8_t *);
esp_err_t nimble_port_init(void);
esp_err_t nimble_port_deinit(void);
int nimble_port_stop(void);
void nimble_port_run(void);
void ble_svc_gap_init(void);
void ble_svc_gatt_init(void);
int ble_svc_gap_device_name_set(const char *);
int ble_gatts_count_cfg(const struct ble_gatt_svc_def *);
int ble_gatts_add_svcs(const struct ble_gatt_svc_def *);
int os_mbuf_append(struct os_mbuf *,const void *,size_t);
int os_mbuf_copydata(struct os_mbuf *,unsigned,unsigned,void *);
struct os_mbuf *ble_hs_mbuf_from_flat(const void *,size_t);
int ble_gatts_notify_custom(uint16_t,uint16_t,struct os_mbuf *);

/* Cover partition/mutex adapter, exercised without real flash. */
typedef struct {uint32_t address,size;} esp_partition_t;
typedef uint32_t esp_partition_mmap_handle_t;
#define ESP_PARTITION_TYPE_DATA 1
#define ESP_PARTITION_MMAP_DATA 0
const esp_partition_t *esp_partition_find_first(int,int,const char *);
esp_err_t esp_partition_read(const esp_partition_t *,size_t,void *,size_t);
esp_err_t esp_partition_write(const esp_partition_t *,size_t,const void *,size_t);
esp_err_t esp_partition_erase_range(const esp_partition_t *,size_t,size_t);
esp_err_t esp_partition_mmap(const esp_partition_t *,size_t,size_t,int,const void **,esp_partition_mmap_handle_t *);
void esp_partition_munmap(esp_partition_mmap_handle_t);
typedef void *SemaphoreHandle_t;
SemaphoreHandle_t xSemaphoreCreateMutex(void);
BaseType_t xSemaphoreTake(SemaphoreHandle_t,unsigned);
BaseType_t xSemaphoreGive(SemaphoreHandle_t);
esp_err_t nvs_set_str(nvs_handle_t,const char *,const char *);
