#include "reader_stubs/platform.h"
#include "../main/reader_network.c"
#include <assert.h>

const char *WIFI_EVENT="wifi";
const char *IP_EVENT="ip";
static unsigned step,fail_at,live_netifs,saved_credentials;
static int64_t test_now;
static unsigned clients;
static int signal_dbm=-52;
static bool status_failure;
static reader_network_credentials_t saved_station;
int64_t esp_timer_get_time(void){return test_now;}
esp_err_t esp_wifi_ap_get_sta_list(wifi_sta_list_t *out){out->num=(int)clients;return status_failure?ESP_FAIL:ESP_OK;}
static bool cover_allow,cover_finish_ok=true,cover_aborted;
static size_t cover_received,body_received;
static int cover_fail_read;
static char request_key[16]="",request_crc[16]="00000000",response_status[64];
static unsigned char response_body[128];
static bool cover_get_active;
static unsigned char cover_read_pixels[QIMEN_COVER_BYTES];
static char associated_ssid[33]="old";
static network_work_t queued;
static bool driver_live,running,http_live,handlers[2],fail_stop;
static esp_netif_t interfaces[2];
static int registered_event_id[2];
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
    esp_err_t e=next();if(e==ESP_OK){handlers[slot]=true;registered_event_id[slot]=id;*instance=&handlers[slot];}return e;
}
esp_err_t esp_event_handler_instance_unregister(esp_event_base_t base,int32_t id,esp_event_handler_instance_t instance) {
    (void)id;unsigned slot=!strcmp(base,IP_EVENT);assert(handlers[slot]&&instance==&handlers[slot]&&registered_event_id[slot]==id);handlers[slot]=false;return ESP_OK;
}
esp_err_t esp_wifi_set_storage(int value) {(void)value;return next();}
esp_err_t esp_wifi_set_mode(int value) {(void)value;return next();}
esp_err_t esp_wifi_set_config(int value,const wifi_config_t *cfg) {(void)value;(void)cfg;return next();}
esp_err_t esp_wifi_start(void) {assert(!running);esp_err_t e=next();running=e==ESP_OK;return e;}
esp_err_t esp_wifi_stop(void) {assert(running);if(fail_stop)return ESP_FAIL;running=false;return ESP_OK;}
esp_err_t esp_wifi_connect(void) {return ESP_OK;}
esp_err_t esp_wifi_disconnect(void) {return ESP_OK;}
esp_err_t esp_wifi_sta_get_ap_info(wifi_ap_record_t *p) {strcpy((char*)p->ssid,associated_ssid);p->rssi=(int8_t)signal_dbm;return status_failure?ESP_FAIL:ESP_OK;}
esp_err_t httpd_start(httpd_handle_t *server_out,const httpd_config_t *cfg) {(void)cfg;assert(!http_live);esp_err_t e=next();if(e==ESP_OK){http_live=true;*server_out=&http_live;}return e;}
esp_err_t httpd_stop(httpd_handle_t h) {assert(h&&http_live);http_live=false;return ESP_OK;}
esp_err_t httpd_register_uri_handler(httpd_handle_t h,const httpd_uri_t *route) {(void)h;(void)route;return next();}
esp_err_t reader_storage_network_load(reader_network_credentials_t *out) {*out=saved_station;return ESP_OK;}
esp_err_t reader_storage_network_save(const char *ssid,const char *password) {
    assert(!strcmp(ssid,"new-network")&&!strcmp(password,"new-password"));saved_credentials++;saved_station.configured=true;strcpy(saved_station.ssid,ssid);strcpy(saved_station.password,password);return ESP_OK;
}
esp_err_t reader_storage_network_forget(void) {return ESP_OK;}
bool qimen_cover_store_acquire(qimen_cover_view_t *v){memset(v,0,sizeof(*v));v->pixels=cover_get_active?cover_read_pixels:NULL;return true;}
void qimen_cover_store_release(bool rendered){(void)rendered;}
bool qimen_cover_store_begin(size_t size,uint32_t crc){(void)crc;assert(size==QIMEN_COVER_BYTES);cover_received=0;cover_aborted=false;return cover_allow;}
bool qimen_cover_store_chunk(size_t offset,const void *bytes,size_t size){assert(bytes&&offset==cover_received&&size<=1024);cover_received+=size;return true;}
bool qimen_cover_store_finish(void){assert(cover_received==QIMEN_COVER_BYTES);return cover_finish_ok;}
void qimen_cover_store_abort(void){cover_aborted=true;}
static bool bluetooth_active;
void reader_ble_info(reader_ble_info_t *out) {memset(out,0,sizeof(*out));out->initialized=bluetooth_active;}
static bool sleep_ble_failure;
void reader_ble_close(void){bluetooth_active=false;}
bool reader_ble_shutdown(void) {return !sleep_ble_failure;}
bool reader_remote_submit(const char *line,char *reply_out,size_t capacity) {(void)line;(void)reply_out;(void)capacity;return false;}
BaseType_t xQueueSend(QueueHandle_t q,const void *item,unsigned wait) {(void)q;(void)wait;queued=*(const network_work_t*)item;return pdTRUE;}
void vTaskDelay(unsigned delay) {(void)delay;}
esp_err_t httpd_resp_set_status(httpd_req_t *r,const char *s) {(void)r;snprintf(response_status,sizeof(response_status),"%s",s);return ESP_OK;}
esp_err_t httpd_resp_set_type(httpd_req_t *r,const char *s) {(void)r;(void)s;return ESP_OK;}
esp_err_t httpd_resp_set_hdr(httpd_req_t *r,const char *a,const char *b) {(void)r;(void)a;(void)b;return ESP_OK;}
esp_err_t httpd_resp_sendstr(httpd_req_t *r,const char *s) {(void)r;snprintf((char*)response_body,sizeof(response_body),"%s",s);return ESP_OK;}
esp_err_t httpd_resp_send(httpd_req_t *r,const char *s,size_t n) {(void)r;assert(s&&n==QIMEN_COVER_BYTES);return ESP_OK;}
esp_err_t httpd_req_get_hdr_value_str(httpd_req_t *r,const char *s,char *v,size_t n) {(void)r;const char *value=!strcmp(s,"X-Qimen-Key")?request_key:request_crc;if(strlen(value)>=n)return ESP_FAIL;snprintf(v,n,"%s",value);return ESP_OK;}
int httpd_req_recv(httpd_req_t *r,char *s,size_t n) {(void)r;if(cover_fail_read&&body_received>=1024)return -1;memset(s,0,n);body_received+=n;return (int)n;}
const unsigned char web_start[] __asm__("_binary_device_html_start")="test";
const unsigned char web_end[] __asm__("_binary_device_html_end")="";

static void reset(void) {
    assert(!live_netifs&&!driver_live&&!running&&!http_live&&!handlers[0]&&!handlers[1]);
    assert(!initialized&&!started&&!ap_netif&&!sta_netif&&!cleanup_pending&&!reconfiguring);
    memset(&info,0,sizeof(info));strcpy(info.ap,"test-hotspot");strcpy(info.password,"password123");step=0;
}
static hotspot_config_t saved_ap,staged_ap;
static bool have_ap,ap_commit_failure;
esp_err_t nvs_open(const char *ns,int mode,nvs_handle_t *out){(void)mode;assert(!strcmp(ns,"qimen_net"));*out=1;return ESP_OK;}
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *size){(void)h;assert(!strcmp(key,"ap_config"));if(!have_ap)return ESP_FAIL;assert(*size>=sizeof(saved_ap));memcpy(out,&saved_ap,sizeof(saved_ap));*size=sizeof(saved_ap);return ESP_OK;}
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *data,size_t size){(void)h;assert(!strcmp(key,"ap_config")&&size==sizeof(staged_ap));memcpy(&staged_ap,data,size);return ESP_OK;}
esp_err_t nvs_commit(nvs_handle_t h){(void)h;if(ap_commit_failure)return ESP_FAIL;saved_ap=staged_ap;have_ap=true;return ESP_OK;}
void nvs_close(nvs_handle_t h){(void)h;}
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
    pending_credentials=true;operation_id=info.request_id=9;info.connecting=true;retry_allowed=true;strcpy(info.station_ssid,"new-network");
    process_work(&resume);
    assert(!strcmp((char*)station.sta.ssid,"new-network")&&!strcmp(expected_ssid,"new-network"));
    assert(pending_credentials&&!saved_credentials&&info.request_id==9&&info.portal);
    commands=(void*)1;ip_event_got_ip_t address={0};
    wifi_event(NULL,IP_EVENT,IP_EVENT_STA_GOT_IP,&address);assert(!info.connected);
    strcpy(associated_ssid,"new-network");wifi_event(NULL,IP_EVENT,IP_EVENT_STA_GOT_IP,&address);
    assert(info.connected&&queued.kind==NET_CONNECTED);process_work(&queued);
    assert(saved_credentials==1&&!pending_credentials&&info.provisioned&&info.request_done);memset(&saved_station,0,sizeof(saved_station));
    assert(release_wifi()==ESP_OK);reset();
    bluetooth_active=true;unsigned allocations=step;
    network_work_t enable={.kind=NET_ENABLE};process_work(&enable);assert(step==allocations&&!info.enabled);
    bluetooth_active=false;process_work(&enable);assert(info.enabled&&info.portal);
    assert(release_wifi()==ESP_OK);reset();
    strcpy(info.key,"1234ABCD");httpd_req_t request={.content_len=QIMEN_COVER_BYTES};
    assert(cover_page(&request)==ESP_OK&&!strcmp(response_status,"403 Forbidden"));
    strcpy(request_key,"1234ABCD");request.content_len=1;assert(cover_page(&request)==ESP_OK&&!strcmp(response_status,"400 Bad Request"));request.content_len=QIMEN_COVER_BYTES;
    strcpy(request_crc,"nothex00");assert(cover_page(&request)==ESP_OK&&!strcmp(response_status,"400 Bad Request"));strcpy(request_crc,"00000000");
    assert(cover_page(&request)==ESP_OK&&!strcmp(response_status,"409 Conflict"));cover_allow=true;cover_fail_read=1;body_received=0;
    assert(cover_page(&request)==ESP_OK&&cover_aborted&&!strcmp(response_status,"408 Request Timeout"));
    cover_fail_read=0;body_received=0;cover_finish_ok=false;assert(cover_page(&request)==ESP_OK&&!strcmp(response_status,"422 Unprocessable Content"));
    body_received=0;cover_finish_ok=true;assert(cover_page(&request)==ESP_OK&&!strcmp(response_status,"200 OK")&&!strcmp((char*)response_body,"QIMEN\tOK\tCOVER"));
    assert(cover_received==QIMEN_COVER_BYTES);assert(cover_image(&request)==ESP_OK&&!strcmp(response_status,"204 No Content"));cover_get_active=true;assert(cover_image(&request)==ESP_OK);
    /* Completion must mean both radios and HTTP are gone, not just enabled=false. */
    assert(release_wifi()==ESP_OK);reset();commands=(QueueHandle_t)1;
    assert(enable_wifi(true)==ESP_OK);assert(reader_network_sleep_request());assert(!reader_network_enable(true));
    assert(reader_network_sleep_result()==0);reader_network_sleep_cancel();assert(sleep_requested);
    network_work_t old={.kind=NET_ENABLE};process_work(&old);assert(running);
    network_work_t sleeping={.kind=NET_SLEEP};process_work(&sleeping);
    assert(reader_network_sleep_result()==1&&!driver_live&&!running&&!http_live&&!live_netifs);
    reader_network_sleep_cancel();assert(!sleep_requested);
    assert(enable_wifi(true)==ESP_OK);fail_stop=true;assert(reader_network_sleep_request());process_work(&sleeping);
    assert(reader_network_sleep_result()==-1&&driver_live);reader_network_sleep_cancel();fail_stop=false;assert(release_wifi()==ESP_OK);
    sleep_ble_failure=true;assert(reader_network_sleep_request());process_work(&sleeping);assert(reader_network_sleep_result()==-1);reader_network_sleep_cancel();sleep_ble_failure=false;
    load_hotspot_config();assert(!strcmp(info.password,"12345678")&&!strcmp(info.ap,"Qimen-Passport"));
    assert(reader_network_set_ap("custom-hotspot","new-pass123"));assert(!strcmp(info.ap,"custom-hotspot"));
    assert(reader_network_set_ap("","12345678"));assert(!strcmp(info.ap,"custom-hotspot"));
    memset(&info,0,sizeof(info));load_hotspot_config();assert(!strcmp(info.ap,"custom-hotspot")&&!strcmp(info.password,"12345678"));
    ap_commit_failure=true;assert(!reader_network_set_ap("","rejected-pass"));assert(!strcmp(info.password,"12345678"));ap_commit_failure=false;
    assert(reader_network_set_ap("","persistent-pass"));memset(&info,0,sizeof(info));load_hotspot_config();assert(!strcmp(info.password,"persistent-pass"));
    assert(!reader_network_set_ap("","short")&&!strcmp(info.ap,"custom-hotspot"));
    assert(reader_network_set_ap("renamed-hotspot",""));assert(!strcmp(info.password,"persistent-pass")&&!strcmp(info.ap,"renamed-hotspot"));
    memset(&info,0,sizeof(info));load_hotspot_config();assert(!strcmp(info.password,"persistent-pass")&&!strcmp(info.ap,"renamed-hotspot"));
    assert(enable_wifi(true)==ESP_OK);clients=2;network_work_t status={.kind=NET_STATUS};process_work(&status);assert(info.ap_clients==2);
    clients=1;wifi_event(NULL,WIFI_EVENT,WIFI_EVENT_AP_STADISCONNECTED,NULL);assert(queued.kind==NET_STATUS);process_work(&queued);assert(info.ap_clients==1);
    status_failure=true;process_work(&status);assert(info.ap_clients==-1);status_failure=false;
    strcpy(saved_station.ssid,"new-network");strcpy(saved_station.password,"new-password");saved_station.configured=true;
    network_work_t reconnect={.kind=NET_RECONNECT,.request=42};unsigned old_saves=saved_credentials;process_work(&reconnect);
    assert(info.request_id==42&&info.connecting&&!pending_credentials&&!strcmp(info.station_ssid,"new-network"));
    wifi_event(NULL,IP_EVENT,IP_EVENT_STA_GOT_IP,&address);process_work(&queued);assert(info.connected&&info.provisioned&&saved_credentials==old_saves);
    wifi_event(NULL,IP_EVENT,IP_EVENT_STA_LOST_IP,NULL);assert(!info.connected&&info.connecting&&info.last_error==NET_ERROR_NO_IP);
    wifi_event(NULL,IP_EVENT,IP_EVENT_STA_GOT_IP,&address);process_work(&queued);assert(info.connected&&info.last_error==NET_ERROR_NONE);
    signal_dbm=-80;process_work(&status);assert(info.rssi==-80&&!strcmp(reader_network_signal_text(&info),"弱"));
    wifi_event_sta_disconnected_t lost={.reason=202};wifi_event(NULL,WIFI_EVENT,WIFI_EVENT_STA_DISCONNECTED,&lost);assert(info.connecting&&!info.connected&&info.disconnect_reason==202);
    process_work(&queued);assert(info.retry_count==1&&reader_network_state(&info)==3);assert(!strcmp(reader_network_error_text(&info),"认证失败，请检查密码"));
    test_now=connect_deadline+1;process_work(&status);assert(reader_network_state(&info)==5&&info.last_error==NET_ERROR_TIMEOUT&&!retry_allowed);
    wifi_event(NULL,IP_EVENT,IP_EVENT_STA_GOT_IP,&address);assert(!info.connected);
        wifi_event(NULL,IP_EVENT,IP_EVENT_STA_LOST_IP,NULL);assert(!info.connected);
    assert(release_wifi()==ESP_OK);memset(&saved_station,0,sizeof(saved_station));reset();assert(!reader_network_reconnect());
    printf("Reader Wi-Fi: %u allocation/register/start/route fault points, cleanup retry and BLE/web cycle PASS\n",stages);
}
