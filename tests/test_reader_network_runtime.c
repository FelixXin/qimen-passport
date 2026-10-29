#include "reader_stubs/platform.h"
#include "../main/reader_network.c"
#include <assert.h>

const char *WIFI_EVENT="wifi";
const char *IP_EVENT="ip";
static unsigned step,fail_at,live_netifs,saved_credentials;
static char associated_ssid[33]="old";
static network_work_t queued;
static bool driver_live,running,http_live,handlers[2],fail_stop;
static esp_netif_t interfaces[2];
static esp_err_t next(void) {return ++step==fail_at?ESP_ERR_NO_MEM:ESP_OK;}
esp_err_t esp_netif_init(void) {return next();}
esp_err_t esp_event_loop_create_default(void) {return next();}
esp_netif_t *esp_netif_new(const esp_netif_config_t *cfg) {
    if(next()!=ESP_OK)return NULL;
    esp_netif_t *p=&interfaces[cfg->kind-1];assert(!p->kind);p->kind=cfg->kind;live_netifs++;return p;
}
void esp_netif_destroy_default_wifi(void *ptr) {esp_netif_t *p=ptr;assert(p->kind&&!driver_live);p->kind=0;live_netifs--;}
esp_err_t esp_netif_attach_wifi_ap(esp_netif_t *p) {(void)p;return next();}
esp_err_t esp_netif_attach_wifi_station(esp_netif_t *p) {(void)p;return next();}
esp_err_t esp_wifi_set_default_wifi_ap_handlers(void) {return next();}
esp_err_t esp_wifi_set_default_wifi_sta_handlers(void) {return next();}
esp_err_t esp_wifi_init(const wifi_init_config_t *cfg) {(void)cfg;assert(!driver_live);esp_err_t e=next();driver_live=e==ESP_OK;return e;}
esp_err_t esp_wifi_deinit(void) {assert(driver_live&&!running&&!handlers[0]&&!handlers[1]);driver_live=false;return ESP_OK;}
esp_err_t esp_event_handler_instance_register(esp_event_base_t base,int32_t id,event_callback_t callback,void *arg,esp_event_handler_instance_t *instance) {
    (void)id;(void)callback;(void)arg;unsigned slot=!strcmp(base,IP_EVENT);assert(!handlers[slot]);
    esp_err_t e=next();if(e==ESP_OK){handlers[slot]=true;*instance=&handlers[slot];}return e;
}
esp_err_t esp_event_handler_instance_unregister(esp_event_base_t base,int32_t id,esp_event_handler_instance_t instance) {
    (void)id;unsigned slot=!strcmp(base,IP_EVENT);assert(handlers[slot]&&instance==&handlers[slot]);handlers[slot]=false;return ESP_OK;
}
esp_err_t esp_wifi_set_storage(int value) {(void)value;return next();}
esp_err_t esp_wifi_set_mode(int value) {(void)value;return next();}
esp_err_t esp_wifi_set_config(int value,const wifi_config_t *cfg) {(void)value;(void)cfg;return next();}
esp_err_t esp_wifi_start(void) {assert(!running);esp_err_t e=next();running=e==ESP_OK;return e;}
esp_err_t esp_wifi_stop(void) {assert(running);if(fail_stop)return ESP_FAIL;running=false;return ESP_OK;}
esp_err_t esp_wifi_connect(void) {return ESP_OK;}
esp_err_t esp_wifi_disconnect(void) {return ESP_OK;}
esp_err_t esp_wifi_sta_get_ap_info(wifi_ap_record_t *p) {strcpy((char*)p->ssid,associated_ssid);return ESP_OK;}
esp_err_t httpd_start(httpd_handle_t *server_out,const httpd_config_t *cfg) {(void)cfg;assert(!http_live);esp_err_t e=next();if(e==ESP_OK){http_live=true;*server_out=&http_live;}return e;}
esp_err_t httpd_stop(httpd_handle_t h) {assert(h&&http_live);http_live=false;return ESP_OK;}
esp_err_t httpd_register_uri_handler(httpd_handle_t h,const httpd_uri_t *route) {(void)h;(void)route;return next();}
esp_err_t reader_storage_network_load(reader_network_credentials_t *out) {memset(out,0,sizeof(*out));return ESP_OK;}
esp_err_t reader_storage_network_save(const char *ssid,const char *password) {
    assert(!strcmp(ssid,"new-network")&&!strcmp(password,"new-password"));saved_credentials++;return ESP_OK;
}
esp_err_t reader_storage_network_forget(void) {return ESP_OK;}
static bool bluetooth_active;
void reader_ble_info(reader_ble_info_t *out) {memset(out,0,sizeof(*out));out->initialized=bluetooth_active;}
bool reader_ble_shutdown(void) {return true;}
bool reader_remote_submit(const char *line,char *reply_out,size_t capacity) {(void)line;(void)reply_out;(void)capacity;return false;}
BaseType_t xQueueSend(QueueHandle_t q,const void *item,unsigned wait) {(void)q;(void)wait;queued=*(const network_work_t*)item;return pdTRUE;}
void vTaskDelay(unsigned delay) {(void)delay;}
esp_err_t httpd_resp_set_status(httpd_req_t *r,const char *s) {(void)r;(void)s;return ESP_OK;}
esp_err_t httpd_resp_set_type(httpd_req_t *r,const char *s) {(void)r;(void)s;return ESP_OK;}
esp_err_t httpd_resp_set_hdr(httpd_req_t *r,const char *a,const char *b) {(void)r;(void)a;(void)b;return ESP_OK;}
esp_err_t httpd_resp_sendstr(httpd_req_t *r,const char *s) {(void)r;(void)s;return ESP_OK;}
esp_err_t httpd_resp_send(httpd_req_t *r,const char *s,size_t n) {(void)r;(void)s;(void)n;return ESP_OK;}
esp_err_t httpd_req_get_hdr_value_str(httpd_req_t *r,const char *s,char *v,size_t n) {(void)r;(void)s;(void)v;(void)n;return ESP_FAIL;}
int httpd_req_recv(httpd_req_t *r,char *s,size_t n) {(void)r;(void)s;(void)n;return -1;}
const unsigned char web_start[] __asm__("_binary_device_html_start")="test";
const unsigned char web_end[] __asm__("_binary_device_html_end")="";

static void reset(void) {
    assert(!live_netifs&&!driver_live&&!running&&!http_live&&!handlers[0]&&!handlers[1]);
    assert(!initialized&&!started&&!ap_netif&&!sta_netif&&!cleanup_pending&&!reconfiguring);
    memset(&info,0,sizeof(info));strcpy(info.ap,"test-hotspot");strcpy(info.password,"password123");step=0;
}
int main(void) {
    reset();fail_at=0;assert(enable_wifi(true)==ESP_OK);unsigned stages=step;
    assert(info.enabled&&info.portal&&running&&http_live);assert(release_wifi()==ESP_OK);
    for(unsigned i=1;i<=stages;i++) {
        reset();fail_at=i;assert(enable_wifi(true)!=ESP_OK);reset();
        fail_at=0;assert(enable_wifi(true)==ESP_OK);assert(release_wifi()==ESP_OK);
    }
    reset();fail_at=0;assert(enable_wifi(true)==ESP_OK);fail_stop=true;
    assert(release_wifi()!=ESP_OK&&cleanup_pending&&driver_live&&live_netifs==2);
    fail_stop=false;assert(release_wifi()==ESP_OK);reset();
    assert(enable_wifi(true)==ESP_OK);
    network_work_t close={.kind=NET_DISABLE};process_work(&close);reset();
    assert(enable_wifi(true)==ESP_OK);
    network_work_t prepare={.kind=NET_PREPARE_BLE};process_work(&prepare);
    assert(info.ble_ready&&!info.ble_prepare_failed&&!live_netifs&&!driver_live);
    network_work_t resume={.kind=NET_RESUME_WEB};process_work(&resume);
    assert(info.portal&&info.enabled);assert(release_wifi()==ESP_OK);reset();
    assert(enable_wifi(false)==ESP_OK);
    strcpy((char*)station.sta.ssid,"new-network");strcpy((char*)station.sta.password,"new-password");
    strcpy(expected_ssid,"new-network");strcpy(pending_ssid,"new-network");strcpy(pending_password,"new-password");
    pending_credentials=true;operation_id=info.request_id=9;info.connecting=true;
    process_work(&resume);
    assert(!strcmp((char*)station.sta.ssid,"new-network")&&!strcmp(expected_ssid,"new-network"));
    assert(pending_credentials&&!saved_credentials&&info.request_id==9&&info.portal);
    commands=(void*)1;ip_event_got_ip_t address={0};
    wifi_event(NULL,IP_EVENT,IP_EVENT_STA_GOT_IP,&address);assert(!info.connected);
    strcpy(associated_ssid,"new-network");wifi_event(NULL,IP_EVENT,IP_EVENT_STA_GOT_IP,&address);
    assert(info.connected&&queued.kind==NET_CONNECTED);process_work(&queued);
    assert(saved_credentials==1&&!pending_credentials&&info.provisioned&&info.request_done);
    assert(release_wifi()==ESP_OK);reset();
    bluetooth_active=true;unsigned allocations=step;
    network_work_t enable={.kind=NET_ENABLE};process_work(&enable);assert(step==allocations&&!info.enabled);
    bluetooth_active=false;process_work(&enable);assert(info.enabled&&info.portal);
    assert(release_wifi()==ESP_OK);reset();
    printf("Reader Wi-Fi: %u allocation/register/start/route fault points, cleanup retry and BLE/web cycle PASS\n",stages);
}
