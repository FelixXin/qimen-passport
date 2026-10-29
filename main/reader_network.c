#include "reader_network.h"
#include "reader_radio.h"
#include "reader_remote.h"
#include "reader_storage.h"
#include "reader_ble.h"
#include "qimen_cover_store.h"
#include "esp_wifi_default.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static reader_network_info_t info;
static portMUX_TYPE info_lock=portMUX_INITIALIZER_UNLOCKED;
static QueueHandle_t commands;
static httpd_handle_t server;
static bool initialized,started,reconfiguring,pending_credentials;
static esp_netif_t *ap_netif,*sta_netif;
static esp_event_handler_instance_t wifi_handler,ip_handler;
static bool wifi_registered,ip_registered,cleanup_pending;
static TaskHandle_t worker_task;
static unsigned retries;
static uint32_t session_token,operation_token,operation_id,next_request;
static wifi_config_t station;
static char pending_ssid[33],pending_password[65],expected_ssid[33];
enum {NET_DISABLE,NET_ENABLE,NET_RETRY,NET_CONFIGURE,NET_CONNECTED,NET_FORGET,NET_PREPARE_BLE,NET_RESUME_WEB,NET_RELEASE_BLE};
typedef struct {int kind;uint32_t token,request;char ssid[33],password[65];} network_work_t;
typedef struct {uint32_t version;char ssid[33],password[64];} hotspot_config_t;
extern const unsigned char web_start[] __asm__("_binary_device_html_start");
extern const unsigned char web_end[] __asm__("_binary_device_html_end");
static bool enqueue(network_work_t *work){return commands&&xQueueSend(commands,work,0)==pdTRUE;}
static bool valid_session(uint32_t token){portENTER_CRITICAL(&info_lock);bool ok=!token||token==session_token;portEXIT_CRITICAL(&info_lock);return ok;}
void reader_network_provision_session(uint32_t token){portENTER_CRITICAL(&info_lock);session_token=token;portEXIT_CRITICAL(&info_lock);}
void reader_network_info(reader_network_info_t *out){portENTER_CRITICAL(&info_lock);*out=info;portEXIT_CRITICAL(&info_lock);}
static void message(const char *text){portENTER_CRITICAL(&info_lock);snprintf(info.message,sizeof(info.message),"%s",text);portEXIT_CRITICAL(&info_lock);}
static esp_err_t reply(httpd_req_t *r,const char *status,const char *text){
    httpd_resp_set_status(r,status);httpd_resp_set_type(r,"text/plain; charset=utf-8");
    httpd_resp_set_hdr(r,"Cache-Control","no-store");httpd_resp_set_hdr(r,"X-Content-Type-Options","nosniff");
    return httpd_resp_sendstr(r,text);
}
static bool authorized(httpd_req_t *r){
    char key[10]={0};reader_network_info_t current;reader_network_info(&current);
    if(httpd_req_get_hdr_value_str(r,"X-Qimen-Key",key,sizeof(key))!=ESP_OK||strcmp(key,current.key)){
        reply(r,"403 Forbidden","管理码不正确，请查看设备 Wi-Fi 页。");return false;
    }return true;
}
static bool receive(httpd_req_t *r,char *buffer,size_t length){
    size_t done=0;unsigned timeouts=0;
    while(done<length){int got=httpd_req_recv(r,buffer+done,length-done);
        if(got==HTTPD_SOCK_ERR_TIMEOUT&&++timeouts<=3)continue;
        if(got<=0)return false;
        done+=(size_t)got;
    }return true;
}
static esp_err_t home(httpd_req_t *r){
    httpd_resp_set_type(r,"text/html; charset=utf-8");httpd_resp_set_hdr(r,"Cache-Control","no-store");
    httpd_resp_set_hdr(r,"Content-Security-Policy","default-src 'self'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; img-src 'self' blob: data:; connect-src 'self'; frame-ancestors 'none'");
    return httpd_resp_send(r,(const char*)web_start,(size_t)(web_end-web_start)-1);
}
static esp_err_t command_page(httpd_req_t *r){
    if(!authorized(r))return ESP_OK;
    if(r->content_len<1||r->content_len>=READER_LOCAL_MAX)return reply(r,"400 Bad Request","指令长度无效");
    char line[READER_LOCAL_MAX]={0},answer[READER_LOCAL_MAX]={0};
    if(!receive(r,line,r->content_len))return reply(r,"408 Request Timeout","接收超时");
    if(memchr(line,0,r->content_len)||memchr(line,'\n',r->content_len)||memchr(line,'\r',r->content_len)){
        memset(line,0,sizeof(line));return reply(r,"400 Bad Request","指令格式无效");
    }
    bool ok=reader_remote_submit(line,answer,sizeof(answer));memset(line,0,sizeof(line));
    esp_err_t err=reply(r,ok?"200 OK":"503 Service Unavailable",ok?answer:"设备忙，请稍后刷新状态");
    memset(answer,0,sizeof(answer));return err;
}
static esp_err_t cover_image(httpd_req_t *r){
    if(!authorized(r))return ESP_OK;
    qimen_cover_view_t v;if(!qimen_cover_store_acquire(&v))return reply(r,"503 Service Unavailable","设备忙");
    esp_err_t err;
    if(v.pixels){httpd_resp_set_type(r,"application/octet-stream");httpd_resp_set_hdr(r,"Cache-Control","no-store");err=httpd_resp_send(r,(const char*)v.pixels,QIMEN_COVER_BYTES);}
    else err=reply(r,"204 No Content","");
    qimen_cover_store_release(false);return err;
}
static esp_err_t cover_page(httpd_req_t *r){
    if(!authorized(r))return ESP_OK;
    if(r->content_len!=QIMEN_COVER_BYTES)return reply(r,"400 Bad Request","封面尺寸无效");
    char sum[9]={0};uint32_t crc=0;
    if(httpd_req_get_hdr_value_str(r,"X-Cover-CRC32",sum,sizeof(sum))!=ESP_OK||strlen(sum)!=8)return reply(r,"400 Bad Request","封面校验无效");
    for(unsigned i=0;i<8;i++){unsigned char ch=(unsigned char)sum[i];unsigned value=ch>='0'&&ch<='9'?ch-'0':ch>='a'&&ch<='f'?ch-'a'+10:ch>='A'&&ch<='F'?ch-'A'+10:99;if(value>15)return reply(r,"400 Bad Request","封面校验无效");crc=(crc<<4)|value;}
    if(!qimen_cover_store_begin(r->content_len,crc))return reply(r,"409 Conflict","设备忙或图片存储未就绪，请稍后重试");
    char buffer[1024];size_t offset=0;
    while(offset<r->content_len){size_t n=r->content_len-offset;if(n>sizeof(buffer))n=sizeof(buffer);
        if(!receive(r,buffer,n)){qimen_cover_store_abort();return reply(r,"408 Request Timeout","上传中断，原封面保留");}
        if(!qimen_cover_store_chunk(offset,buffer,n)){qimen_cover_store_abort();return reply(r,"503 Service Unavailable","保存失败，原封面保留");}offset+=n;
    }
    if(!qimen_cover_store_finish())return reply(r,"422 Unprocessable Content","封面校验失败，原封面保留");
    return reply(r,"200 OK","QIMEN\tOK\tCOVER");
}

static bool save_credentials(const char *ssid,const char *password) {
    return reader_storage_network_save(ssid,password)==ESP_OK;
}
static void clear_pending(void){pending_credentials=false;memset(pending_ssid,0,sizeof(pending_ssid));memset(pending_password,0,sizeof(pending_password));}
static void failure(void){
    portENTER_CRITICAL(&info_lock);info.connecting=false;info.provisioned=false;info.request_done=true;portEXIT_CRITICAL(&info_lock);
    clear_pending();message("Connection failed; previous credentials retained");
}
static void wifi_event(void *arg,esp_event_base_t base,int32_t id,void *data){
    (void)arg;
    if(base==IP_EVENT&&id==IP_EVENT_STA_GOT_IP){
        wifi_ap_record_t associated={0};char expected[33];
        portENTER_CRITICAL(&info_lock);bool changing=reconfiguring;memcpy(expected,expected_ssid,sizeof(expected));portEXIT_CRITICAL(&info_lock);
        if(esp_wifi_sta_get_ap_info(&associated)!=ESP_OK||!reader_radio_accept_address(changing,expected,associated.ssid))return;
        ip_event_got_ip_t *event=data;network_work_t work={.kind=NET_CONNECTED};
        portENTER_CRITICAL(&info_lock);info.connected=true;info.connecting=false;
        snprintf(info.ip,sizeof(info.ip),IPSTR,IP2STR(&event->ip_info.ip));work.token=operation_token;work.request=operation_id;portEXIT_CRITICAL(&info_lock);
        if(!enqueue(&work))message("Wi-Fi connected; save queue busy");
    }else if(base==WIFI_EVENT&&id==WIFI_EVENT_STA_DISCONNECTED){
        network_work_t work={.kind=NET_RETRY};
        portENTER_CRITICAL(&info_lock);info.connected=false;strcpy(info.ip,"0.0.0.0");
        bool retry=info.enabled&&!reconfiguring;work.token=operation_token;work.request=operation_id;portEXIT_CRITICAL(&info_lock);
        if(retry&&!enqueue(&work))message("Reconnect queue busy");
    }
}
static void load_station(void) {
    memset(&station,0,sizeof(station));
    reader_network_credentials_t config;
    if(reader_storage_network_load(&config)==ESP_OK && config.configured) {
        memcpy(station.sta.ssid,config.ssid,strlen(config.ssid));
        memcpy(station.sta.password,config.password,strlen(config.password));
    }
    memset(&config,0,sizeof(config));station.sta.pmf_cfg.capable=true;
    portENTER_CRITICAL(&info_lock);
    memset(expected_ssid,0,sizeof(expected_ssid));memcpy(expected_ssid,station.sta.ssid,32);
    portEXIT_CRITICAL(&info_lock);
}

/* Retain ownership after a failed cleanup; a later explicit retry can finish it.
 * Never destroy an interface while a registered handler or driver can use it. */
static esp_err_t release_wifi(void) {
    clear_pending();
    portENTER_CRITICAL(&info_lock);
    reconfiguring=true;info.enabled=false;info.connected=false;info.connecting=false;info.portal=false;
    strcpy(info.ip,"0.0.0.0");
    portEXIT_CRITICAL(&info_lock);
    esp_err_t error=ESP_OK;
    if(server) {
        error=httpd_stop(server);
        if(error!=ESP_OK)goto done;
        server=NULL;
    }
    if(started) {
        error=esp_wifi_stop();
        if(error!=ESP_OK && error!=ESP_ERR_WIFI_NOT_STARTED)goto done;
        error=ESP_OK;started=false;
    }
    if(ip_registered) {
        error=esp_event_handler_instance_unregister(IP_EVENT,IP_EVENT_STA_GOT_IP,ip_handler);
        if(error!=ESP_OK)goto done;
        ip_registered=false;
    }
    if(wifi_registered) {
        error=esp_event_handler_instance_unregister(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_handler);
        if(error!=ESP_OK)goto done;
        wifi_registered=false;
    }
    if(initialized) {
        error=esp_wifi_deinit();
        if(error!=ESP_OK)goto done;
        initialized=false;
    }
    if(sta_netif){esp_netif_destroy_default_wifi(sta_netif);sta_netif=NULL;}
    if(ap_netif){esp_netif_destroy_default_wifi(ap_netif);ap_netif=NULL;}
    memset(&station,0,sizeof(station));
 done:
    cleanup_pending=error!=ESP_OK;
    portENTER_CRITICAL(&info_lock);reconfiguring=cleanup_pending;portEXIT_CRITICAL(&info_lock);
    return error;
}

static esp_err_t initialize_wifi(void) {
    if(cleanup_pending) {
        esp_err_t cleanup=release_wifi();
        if(cleanup!=ESP_OK)return cleanup;
    }
    if(initialized)return ESP_OK;
    esp_err_t error=esp_netif_init();
    if(error!=ESP_OK)return error;
    error=esp_event_loop_create_default();
    if(error!=ESP_OK && error!=ESP_ERR_INVALID_STATE)return error;
    esp_netif_config_t ap_config=ESP_NETIF_DEFAULT_WIFI_AP(),sta_config=ESP_NETIF_DEFAULT_WIFI_STA();
    ap_netif=esp_netif_new(&ap_config);
    if(!ap_netif){error=ESP_ERR_NO_MEM;goto fail;}
    if((error=esp_netif_attach_wifi_ap(ap_netif))!=ESP_OK ||
       (error=esp_wifi_set_default_wifi_ap_handlers())!=ESP_OK)goto fail;
    sta_netif=esp_netif_new(&sta_config);
    if(!sta_netif){error=ESP_ERR_NO_MEM;goto fail;}
    if((error=esp_netif_attach_wifi_station(sta_netif))!=ESP_OK ||
       (error=esp_wifi_set_default_wifi_sta_handlers())!=ESP_OK)goto fail;
    wifi_init_config_t config=WIFI_INIT_CONFIG_DEFAULT();
    if((error=esp_wifi_init(&config))!=ESP_OK)goto fail;
    initialized=true;
    error=esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_event,NULL,&wifi_handler);
    if(error!=ESP_OK)goto fail;
    wifi_registered=true;
    error=esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP,wifi_event,NULL,&ip_handler);
    if(error!=ESP_OK)goto fail;
    ip_registered=true;
    return ESP_OK;
 fail:
    (void)release_wifi();return error;
}

static esp_err_t configure_hotspot(const reader_network_info_t *current) {
    wifi_config_t ap={0};memcpy(ap.ap.ssid,current->ap,strlen(current->ap));
    snprintf((char*)ap.ap.password,sizeof(ap.ap.password),"%s",current->password);
    ap.ap.ssid_len=strlen(current->ap);ap.ap.channel=1;ap.ap.max_connection=2;ap.ap.authmode=WIFI_AUTH_WPA2_PSK;
    esp_err_t error=esp_wifi_set_config(WIFI_IF_AP,&ap);
    memset(&ap,0,sizeof(ap));return error;
}
static esp_err_t start_web(void) {
    if(server)return ESP_OK;
    httpd_config_t config=HTTPD_DEFAULT_CONFIG();config.stack_size=9216;config.max_open_sockets=3;config.lru_purge_enable=true;
    esp_err_t error=httpd_start(&server,&config);
    if(error!=ESP_OK)return error;
    const httpd_uri_t routes[]={
        {.uri="/",.method=HTTP_GET,.handler=home},
        {.uri="/api/command",.method=HTTP_POST,.handler=command_page},
        {.uri="/api/cover",.method=HTTP_POST,.handler=cover_page},
        {.uri="/api/cover",.method=HTTP_GET,.handler=cover_image}};
    for(unsigned i=0;i<sizeof(routes)/sizeof(routes[0]);i++)
        if((error=httpd_register_uri_handler(server,&routes[i]))!=ESP_OK)return error;
    return ESP_OK;
}
static esp_err_t enable_wifi(bool portal) {
    reader_network_info_t current;reader_network_info(&current);
    if(current.enabled && (!portal || current.portal))return ESP_OK;
    esp_err_t error=initialize_wifi();
    if(error!=ESP_OK)return error;
    if(current.enabled && portal) {
        /* Add the portal to a live STA without reloading old NVS credentials,
         * stopping an in-flight join, or changing its request ownership. */
        if((error=esp_wifi_set_mode(WIFI_MODE_APSTA))!=ESP_OK ||
           (error=configure_hotspot(&current))!=ESP_OK ||
           (error=start_web())!=ESP_OK)goto fail;
        portENTER_CRITICAL(&info_lock);info.portal=true;portEXIT_CRITICAL(&info_lock);
        message("Web portal resumed");return ESP_OK;
    }
    portENTER_CRITICAL(&info_lock);reconfiguring=true;info.enabled=false;info.connected=false;portEXIT_CRITICAL(&info_lock);
    if(started) {
        if((error=esp_wifi_stop())!=ESP_OK)goto fail;
        started=false;
    }
    load_station();retries=0;
    /* Configure secured AP settings before starting even in STA-only mode,
     * so later hot portal activation never exposes an unconfigured AP. */
    if((error=esp_wifi_set_storage(WIFI_STORAGE_RAM))!=ESP_OK ||
       (error=esp_wifi_set_mode(WIFI_MODE_APSTA))!=ESP_OK ||
       (error=configure_hotspot(&current))!=ESP_OK ||
       (error=esp_wifi_set_config(WIFI_IF_STA,&station))!=ESP_OK ||
       (!portal && (error=esp_wifi_set_mode(WIFI_MODE_STA))!=ESP_OK) ||
       (error=esp_wifi_start())!=ESP_OK)goto fail;
    started=true;
    if(portal && (error=start_web())!=ESP_OK)goto fail;
    portENTER_CRITICAL(&info_lock);info.enabled=true;info.portal=portal;reconfiguring=false;portEXIT_CRITICAL(&info_lock);
    if(station.sta.ssid[0]){(void)esp_wifi_connect();message("Connecting Wi-Fi...");}
    else message(portal?"Open 192.168.4.1 in Safari":"Waiting for phone Wi-Fi settings");
    memset(&current,0,sizeof(current));return ESP_OK;
 fail:
    (void)release_wifi();failure();return error;
}
static void connect_provisioned(network_work_t *work){
    if(!valid_session(work->token))return;
    portENTER_CRITICAL(&info_lock);info.request_id=work->request;info.request_done=false;info.provisioned=false;info.connecting=true;portEXIT_CRITICAL(&info_lock);
    if(enable_wifi(false)!=ESP_OK){failure();return;}
    portENTER_CRITICAL(&info_lock);operation_token=work->token;operation_id=work->request;memset(expected_ssid,0,sizeof(expected_ssid));strcpy(expected_ssid,work->ssid);reconfiguring=true;info.connected=false;info.connecting=true;portEXIT_CRITICAL(&info_lock);
    (void)esp_wifi_disconnect();memset(&station,0,sizeof(station));
    memcpy(station.sta.ssid,work->ssid,strlen(work->ssid));memcpy(station.sta.password,work->password,strlen(work->password));station.sta.pmf_cfg.capable=true;
    retries=0;clear_pending();strcpy(pending_ssid,work->ssid);strcpy(pending_password,work->password);pending_credentials=true;
    esp_err_t err=esp_wifi_set_config(WIFI_IF_STA,&station);
    portENTER_CRITICAL(&info_lock);reconfiguring=false;portEXIT_CRITICAL(&info_lock);message("Connecting Wi-Fi...");
    if(err!=ESP_OK||esp_wifi_connect()!=ESP_OK)failure();
}
static void process_work(network_work_t *work) {
        if(work->kind==NET_ENABLE || work->kind==NET_RESUME_WEB) {
            reader_ble_info_t bluetooth;reader_ble_info(&bluetooth);
            if(bluetooth.window || bluetooth.initialized || bluetooth.stopping) {
                message("Web resumes after Bluetooth closes");return;
            }
            if(enable_wifi(true)!=ESP_OK)message("Wi-Fi start failed; toggle to retry");
        }
        else if(work->kind==NET_DISABLE){
            clear_pending();portENTER_CRITICAL(&info_lock);operation_token=operation_id=0;memset(expected_ssid,0,sizeof(expected_ssid));info.enabled=false;info.connected=false;info.connecting=false;info.portal=false;info.provisioned=false;info.request_id=0;portEXIT_CRITICAL(&info_lock);
            message(release_wifi()==ESP_OK?"Wi-Fi off; resources released":"Wi-Fi cleanup failed; retry disabling");
        }else if(work->kind==NET_RETRY){
            reader_network_info_t current;reader_network_info(&current);
            portENTER_CRITICAL(&info_lock);bool match=work->token==operation_token&&work->request==operation_id;portEXIT_CRITICAL(&info_lock);
            if(!match||!current.enabled||!station.sta.ssid[0])return;
            if(retries++>=4){failure();return;}
            vTaskDelay(pdMS_TO_TICKS(1000));if(esp_wifi_connect()!=ESP_OK)failure();
        }else if(work->kind==NET_CONFIGURE)connect_provisioned(work);
        else if(work->kind==NET_CONNECTED){
            reader_network_info_t current;reader_network_info(&current);
            portENTER_CRITICAL(&info_lock);bool match=work->token==operation_token&&work->request==operation_id;portEXIT_CRITICAL(&info_lock);
            if(!match||!current.connected)return;
            retries=0;
            if(pending_credentials){if(!save_credentials(pending_ssid,pending_password)){failure();return;}
                portENTER_CRITICAL(&info_lock);info.provisioned=true;info.request_done=true;portEXIT_CRITICAL(&info_lock);}
            clear_pending();message("Wi-Fi connected");
        }else if(work->kind==NET_PREPARE_BLE){
            reader_network_info_t current;reader_network_info(&current);esp_err_t err=ESP_OK;
            if(server){err=httpd_stop(server);if(err==ESP_OK)server=NULL;}
            if(err==ESP_OK && initialized && current.enabled) {
                if(!station.sta.ssid[0])err=release_wifi();
                else err=esp_wifi_set_mode(WIFI_MODE_STA);
            }
            portENTER_CRITICAL(&info_lock);if(err==ESP_OK)info.portal=false;info.ble_prepare_failed=err!=ESP_OK;info.ble_ready=true;portEXIT_CRITICAL(&info_lock);message(err==ESP_OK?"Web portal paused for Bluetooth":"Could not prepare radio");
        }else if(work->kind==NET_RELEASE_BLE){
            if(!reader_ble_shutdown())message("Bluetooth cleanup failed; retrying");
        }else if(work->kind==NET_FORGET){
            clear_pending();
            if(reader_storage_network_forget()!=ESP_OK){message("Could not clear Wi-Fi");return;}
            memset(&station,0,sizeof(station));portENTER_CRITICAL(&info_lock);operation_token=operation_id=0;memset(expected_ssid,0,sizeof(expected_ssid));info.provisioned=false;info.connecting=false;info.request_id=0;portEXIT_CRITICAL(&info_lock);
            if(initialized)(void)esp_wifi_disconnect();
            message("Saved station Wi-Fi cleared");
        }
}
static void worker(void *unused) {
    (void)unused;network_work_t work;
    for(;;)if(xQueueReceive(commands,&work,portMAX_DELAY)) {
        process_work(&work);memset(&work,0,sizeof(work));
    }
}
bool reader_network_set_ap(const char *ssid,const char *password){
    if(!reader_radio_credentials(ssid,password,true))return false;
    hotspot_config_t config={.version=1};strcpy(config.ssid,ssid);strcpy(config.password,password);
    nvs_handle_t nvs;esp_err_t err=nvs_open("qimen_net",NVS_READWRITE,&nvs);
    if(err==ESP_OK){err=nvs_set_blob(nvs,"ap_config",&config,sizeof(config));if(err==ESP_OK)err=nvs_commit(nvs);nvs_close(nvs);}
    if(err==ESP_OK){portENTER_CRITICAL(&info_lock);strcpy(info.ap,ssid);strcpy(info.password,password);portEXIT_CRITICAL(&info_lock);}
    memset(&config,0,sizeof(config));return err==ESP_OK;
}
void reader_network_init(void){
    strcpy(info.ap,"Qimen-Passport");snprintf(info.password,sizeof(info.password),"%08lX%04lX",(unsigned long)esp_random(),(unsigned long)(esp_random()&0xffff));
    hotspot_config_t config={0};size_t length=sizeof(config);nvs_handle_t nvs;
    if(nvs_open("qimen_net",NVS_READONLY,&nvs)==ESP_OK){
        if(nvs_get_blob(nvs,"ap_config",&config,&length)==ESP_OK&&length==sizeof(config)&&config.version==1&&
           memchr(config.ssid,0,sizeof(config.ssid))&&memchr(config.password,0,sizeof(config.password))&&reader_radio_credentials(config.ssid,config.password,true)){
            strcpy(info.ap,config.ssid);strcpy(info.password,config.password);}
        nvs_close(nvs);
    }
    memset(&config,0,sizeof(config));snprintf(info.key,sizeof(info.key),"%08lX",(unsigned long)esp_random());strcpy(info.ip,"0.0.0.0");strcpy(info.message,"Wi-Fi off");
    commands=xQueueCreate(6,sizeof(network_work_t));
    if(commands&&xTaskCreate(worker,"reader_net",6144,NULL,4,&worker_task)!=pdPASS){vQueueDelete(commands);commands=NULL;}
}
bool reader_network_enable(bool enabled){network_work_t work={.kind=enabled?NET_ENABLE:NET_DISABLE};return enqueue(&work);}
uint32_t reader_network_configure(const char *ssid,const char *password,uint32_t token){
    if(!valid_session(token)||!reader_radio_credentials(ssid,password,false))return 0;
    network_work_t work={.kind=NET_CONFIGURE,.token=token};
    portENTER_CRITICAL(&info_lock);if(!++next_request)++next_request;work.request=next_request;portEXIT_CRITICAL(&info_lock);
    strcpy(work.ssid,ssid);strcpy(work.password,password);bool ok=enqueue(&work);uint32_t request=work.request;memset(&work,0,sizeof(work));return ok?request:0;
}
bool reader_network_forget(void){network_work_t work={.kind=NET_FORGET};return enqueue(&work);}
bool reader_network_prepare_ble(void){portENTER_CRITICAL(&info_lock);info.ble_ready=false;info.ble_prepare_failed=false;portEXIT_CRITICAL(&info_lock);network_work_t work={.kind=NET_PREPARE_BLE};return enqueue(&work);}

bool reader_network_resume_web(void) {network_work_t work={.kind=NET_RESUME_WEB};return enqueue(&work);}
uint32_t reader_network_stack_free(void) {return worker_task?(uint32_t)uxTaskGetStackHighWaterMark(worker_task):0;}

bool reader_network_release_ble(void) {network_work_t work={.kind=NET_RELEASE_BLE};return enqueue(&work);}
