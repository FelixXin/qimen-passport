#include "reader_ble.h"
#include "reader_pairing.h"
#include "reader_network.h"
#include "esp_mac.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "os/os_mbuf.h"
#include <stdio.h>
#include <string.h>

static const ble_uuid128_t service_uuid=BLE_UUID128_INIT(0x00,0x41,0x6e,0xd1,0x3b,0x56,0xc8,0x98,0x23,0x4a,0x91,0x2b,0xe0,0xc6,0x17,0xfa);
static const ble_uuid128_t rx_uuid=BLE_UUID128_INIT(0x01,0x41,0x6e,0xd1,0x3b,0x56,0xc8,0x98,0x23,0x4a,0x91,0x2b,0xe0,0xc6,0x17,0xfa);
static const ble_uuid128_t tx_uuid=BLE_UUID128_INIT(0x02,0x41,0x6e,0xd1,0x3b,0x56,0xc8,0x98,0x23,0x4a,0x91,0x2b,0xe0,0xc6,0x17,0xfa);
static QueueHandle_t receive_queue, send_queue;
static portMUX_TYPE lock=portMUX_INITIALIZER_UNLOCKED;
static reader_pairing_t pairing;
static reader_local_frame_t receive_frame;
static uint16_t connection=BLE_HS_CONN_HANDLE_NONE, tx_handle;
static bool initialized, synced, subscribed, stopping, host_exited, cleanup_needed;
static int64_t stop_retry_at;
static uint8_t address_type;
static int last_error;
static char device_name[24];
static reader_ble_message_t pending_tx;
static size_t tx_offset;
static int gap_event(struct ble_gap_event *event,void *arg);
static void record_error(int error) {portENTER_CRITICAL(&lock);last_error=error;portEXIT_CRITICAL(&lock);}

static int advertise(void) {
    portENTER_CRITICAL(&lock);bool active=reader_pairing_active(&pairing,esp_timer_get_time());portEXIT_CRITICAL(&lock);
    if(!active||ble_gap_adv_active())return 0;
    struct ble_hs_adv_fields fields={0};fields.flags=BLE_HS_ADV_F_DISC_GEN|BLE_HS_ADV_F_BREDR_UNSUP;
    fields.uuids128=(ble_uuid128_t *)&service_uuid;fields.num_uuids128=1;fields.uuids128_is_complete=1;
    int rc=ble_gap_adv_set_fields(&fields);if(rc)return rc;
    struct ble_hs_adv_fields response={0};response.name=(uint8_t *)device_name;
    response.name_len=strlen(device_name);response.name_is_complete=1;
    rc=ble_gap_adv_rsp_set_fields(&response);if(rc)return rc;
    struct ble_gap_adv_params params={0};params.conn_mode=BLE_GAP_CONN_MODE_UND;params.disc_mode=BLE_GAP_DISC_MODE_GEN;
    params.itvl_min=160;params.itvl_max=240;
    return ble_gap_adv_start(address_type,NULL,BLE_HS_FOREVER,&params,gap_event,NULL);
}
static int access_characteristic(uint16_t conn,uint16_t attr,struct ble_gatt_access_ctxt *ctxt,void *arg) {
    (void)conn;(void)attr;(void)arg;
    if(ctxt->op==BLE_GATT_ACCESS_OP_READ_CHR) {
        static const char hello[]="QIMEN local configuration";
        return os_mbuf_append(ctxt->om,hello,sizeof(hello)-1)==0?0:BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    if(ctxt->op!=BLE_GATT_ACCESS_OP_WRITE_CHR)return BLE_ATT_ERR_WRITE_NOT_PERMITTED;
    portENTER_CRITICAL(&lock);bool allowed=reader_pairing_active(&pairing,esp_timer_get_time());portEXIT_CRITICAL(&lock);
    if(!allowed||!receive_queue)return BLE_ATT_ERR_INSUFFICIENT_AUTHOR;
    uint16_t size=OS_MBUF_PKTLEN(ctxt->om);if(size>180)return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    uint8_t bytes[180];if(os_mbuf_copydata(ctxt->om,0,size,bytes))return BLE_ATT_ERR_UNLIKELY;
    for(unsigned i=0;i<size;i++) {
        int complete=reader_local_feed(&receive_frame,bytes[i]);
        if(complete>0) {
            reader_ble_message_t message;
            memcpy(message.line,receive_frame.line,strlen(receive_frame.line)+1);
            portENTER_CRITICAL(&lock);message.generation=pairing.generation;portEXIT_CRITICAL(&lock);
            if(xQueueSend(receive_queue,&message,0)!=pdTRUE)return BLE_ATT_ERR_INSUFFICIENT_RES;
        } else if(complete<0)return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }
    return 0;
}
static const struct ble_gatt_svc_def services[]={
    {.type=BLE_GATT_SVC_TYPE_PRIMARY,.uuid=&service_uuid.u,.characteristics=(struct ble_gatt_chr_def[]){
        {.uuid=&rx_uuid.u,.access_cb=access_characteristic,.flags=BLE_GATT_CHR_F_WRITE},
        {.uuid=&tx_uuid.u,.access_cb=access_characteristic,.val_handle=&tx_handle,.flags=BLE_GATT_CHR_F_READ|BLE_GATT_CHR_F_NOTIFY},
        {0}}},
    {0}};
static int gap_event(struct ble_gap_event *event,void *arg) {
    (void)arg;
    switch(event->type) {
      case BLE_GAP_EVENT_CONNECT:
        portENTER_CRITICAL(&lock);reader_pairing_peer(&pairing);connection=event->connect.status==0?event->connect.conn_handle:BLE_HS_CONN_HANDLE_NONE;subscribed=false;
        uint32_t connect_generation=pairing.generation;bool connect_active=reader_pairing_active(&pairing,esp_timer_get_time());portEXIT_CRITICAL(&lock);
        reader_network_provision_session(connect_active?connect_generation:0);
        memset(&receive_frame,0,sizeof(receive_frame));if(event->connect.status)(void)advertise();break;
      case BLE_GAP_EVENT_DISCONNECT:
        portENTER_CRITICAL(&lock);reader_pairing_peer(&pairing);connection=BLE_HS_CONN_HANDLE_NONE;subscribed=false;
        uint32_t disconnect_generation=pairing.generation;bool disconnect_active=reader_pairing_active(&pairing,esp_timer_get_time());portEXIT_CRITICAL(&lock);
        reader_network_provision_session(disconnect_active?disconnect_generation:0);
        memset(&receive_frame,0,sizeof(receive_frame));(void)advertise();break;
      case BLE_GAP_EVENT_SUBSCRIBE:
        portENTER_CRITICAL(&lock);
        if(event->subscribe.attr_handle==tx_handle)subscribed=event->subscribe.cur_notify;
        portEXIT_CRITICAL(&lock);break;
      case BLE_GAP_EVENT_ADV_COMPLETE:(void)advertise();break;
      default:break;
    }
    return 0;
}
static void on_sync(void) {
    int rc=ble_hs_util_ensure_addr(0);if(!rc)rc=ble_hs_id_infer_auto(0,&address_type);
    portENTER_CRITICAL(&lock);synced=rc==0;last_error=rc;portEXIT_CRITICAL(&lock);
    if(!rc)(void)advertise();
}
static void on_reset(int reason) {portENTER_CRITICAL(&lock);last_error=reason;synced=false;reader_pairing_close(&pairing);portEXIT_CRITICAL(&lock);reader_network_provision_session(0);}
static void host_task(void *arg) {
    (void)arg;nimble_port_run();
    portENTER_CRITICAL(&lock);host_exited=true;portEXIT_CRITICAL(&lock);
    vTaskDelete(NULL);
}
static void rollback_initialization(int original_error) {
    esp_err_t cleanup=nimble_port_deinit();
    portENTER_CRITICAL(&lock);
    if(cleanup!=ESP_OK){initialized=true;host_exited=true;synced=false;}
    last_error=cleanup==ESP_OK?original_error:cleanup;
    portEXIT_CRITICAL(&lock);
}
static bool initialize(void) {
    if(!receive_queue)receive_queue=xQueueCreate(4,sizeof(reader_ble_message_t));
    if(!send_queue)send_queue=xQueueCreate(4,sizeof(reader_ble_message_t));
    if(!receive_queue||!send_queue)return false;
    portENTER_CRITICAL(&lock);bool busy=stopping||host_exited||cleanup_needed;bool ready=initialized;portEXIT_CRITICAL(&lock);
    if(busy){record_error(ESP_ERR_INVALID_STATE);return false;}
    if(ready)return true;
    size_t available=heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA);
    size_t largest=heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA);
    ESP_LOGI("reader_ble","Before init DMA free=%u largest=%u",(unsigned)available,(unsigned)largest);
    if(!reader_ble_memory_ready((uint32_t)available,(uint32_t)largest)) {record_error(ESP_ERR_NO_MEM);return false;}
    uint8_t mac[6];if(esp_read_mac(mac,ESP_MAC_WIFI_STA)!=ESP_OK)return false;
    snprintf(device_name,sizeof(device_name),"Qimen-%02X%02X%02X",mac[3],mac[4],mac[5]);
    esp_err_t error=nimble_port_init();if(error!=ESP_OK){record_error(error);return false;}
    ble_svc_gap_init();ble_svc_gatt_init();
    int rc=ble_svc_gap_device_name_set(device_name);
    if(!rc)rc=ble_gatts_count_cfg(services);
    if(!rc)rc=ble_gatts_add_svcs(services);
    if(rc){rollback_initialization(rc);return false;}
    ble_hs_cfg.sync_cb=on_sync;ble_hs_cfg.reset_cb=on_reset;
    portENTER_CRITICAL(&lock);host_exited=false;portEXIT_CRITICAL(&lock);
    if(xTaskCreate(host_task,"reader_ble",4096,NULL,5,NULL)!=pdPASS){rollback_initialization(ESP_ERR_NO_MEM);return false;}
    portENTER_CRITICAL(&lock);initialized=true;portEXIT_CRITICAL(&lock);
    ESP_LOGI("reader_ble","Initialized DMA free=%u largest=%u",(unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA),(unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA));
    return true;
}
bool reader_ble_open(int64_t now) {
    if(!initialize())return false;
    if(ble_gap_adv_active())(void)ble_gap_adv_stop();
    uint32_t code=esp_random()%1000000;
    portENTER_CRITICAL(&lock);reader_pairing_open(&pairing,code,now);bool ready=synced;uint16_t conn=connection;uint32_t generation=pairing.generation;portEXIT_CRITICAL(&lock);
    reader_network_provision_session(generation);
    if(conn!=BLE_HS_CONN_HANDLE_NONE){(void)ble_gap_terminate(conn,BLE_ERR_REM_USER_CONN_TERM);return true;}
    if(ready){int rc=advertise();if(rc){record_error(rc);reader_ble_close();return false;}}
    return true;
}
void reader_ble_close(void) {
    portENTER_CRITICAL(&lock);reader_pairing_close(&pairing);portEXIT_CRITICAL(&lock);
    reader_network_provision_session(0);
}
void reader_ble_info(reader_ble_info_t *info) {
    portENTER_CRITICAL(&lock);
    bool active=reader_pairing_active(&pairing,esp_timer_get_time());
    *info=(reader_ble_info_t){.initialized=initialized,.stopping=stopping||cleanup_needed,.window=active,.connected=connection!=BLE_HS_CONN_HANDLE_NONE,.authenticated=pairing.authorized,.code=pairing.code,.seconds=active?(unsigned)((pairing.expires_us-esp_timer_get_time()+999999)/1000000):0,.error=last_error};
    snprintf(info->name,sizeof(info->name),"%s",device_name);portEXIT_CRITICAL(&lock);
}
bool reader_ble_next(reader_ble_message_t *message) {
    if(!receive_queue)return false;
    for(unsigned i=0;i<4;i++)if(xQueueReceive(receive_queue,message,0)==pdTRUE){
        portENTER_CRITICAL(&lock);bool valid=reader_pairing_active(&pairing,esp_timer_get_time())&&message->generation==pairing.generation;portEXIT_CRITICAL(&lock);
        if(valid)return true;
    }else break;
    return false;
}
void reader_ble_reply(const char *reply,uint32_t generation) {
    if(!send_queue)return;
    reader_ble_message_t message={.generation=generation};snprintf(message.line,sizeof(message.line),"%s\n",reply);
    (void)xQueueSend(send_queue,&message,0);
}
bool reader_ble_gate(const reader_ble_message_t *message,const reader_local_command_t *command) {
    portENTER_CRITICAL(&lock);
    bool valid=reader_pairing_active(&pairing,esp_timer_get_time())&&message->generation==pairing.generation;
    bool auth=false;
    if(valid&&command->op==LOCAL_AUTH)auth=reader_pairing_auth(&pairing,command->number,esp_timer_get_time());
    bool allowed=valid&&(pairing.authorized||command->op==LOCAL_HELLO);
    portEXIT_CRITICAL(&lock);
    if(command->op==LOCAL_AUTH){reader_ble_reply(auth?"QIMEN\tOK\tAUTH":"QIMEN\tERR\tAUTH",message->generation);return false;}
    if(!allowed)reader_ble_reply("QIMEN\tERR\tAUTH",message->generation);
    return allowed;
}
void reader_ble_tick(int64_t now) {
    portENTER_CRITICAL(&lock);bool active=reader_pairing_active(&pairing,now);uint16_t conn=connection;uint32_t generation=pairing.generation;bool notify=subscribed,ready=synced,closing=stopping,live=initialized;portEXIT_CRITICAL(&lock);
    if(closing)return;
    if(!active){
        reader_network_provision_session(0);
        if(live&&ready){if(ble_gap_adv_active())(void)ble_gap_adv_stop();if(conn!=BLE_HS_CONN_HANDLE_NONE)(void)ble_gap_terminate(conn,BLE_ERR_REM_USER_CONN_TERM);}
        tx_offset=0;pending_tx.line[0]=0;
        if(live && now>=stop_retry_at) {
            portENTER_CRITICAL(&lock);stopping=true;cleanup_needed=true;portEXIT_CRITICAL(&lock);
            if(!reader_network_release_ble()) {
                portENTER_CRITICAL(&lock);stopping=false;portEXIT_CRITICAL(&lock);
            }
            stop_retry_at=now+1000000;
        }
        return;
    }
    if(!notify||conn==BLE_HS_CONN_HANDLE_NONE)return;
    if(!pending_tx.line[0]){if(xQueueReceive(send_queue,&pending_tx,0)!=pdTRUE)return;tx_offset=0;}
    if(pending_tx.generation!=generation){pending_tx.line[0]=0;return;}
    size_t remaining=strlen(pending_tx.line)-tx_offset,length=remaining>20?20:remaining;
    struct os_mbuf *buffer=ble_hs_mbuf_from_flat(pending_tx.line+tx_offset,length);if(!buffer)return;
    int rc=ble_gatts_notify_custom(conn,tx_handle,buffer);
    if(!rc){tx_offset+=length;if(tx_offset==strlen(pending_tx.line))pending_tx.line[0]=0;}
}

/* SDK stop waits on host semaphores. Run it in the existing network worker,
 * never in a key callback or while holding the LVGL lock. */
bool reader_ble_shutdown(void) {
    portENTER_CRITICAL(&lock);bool live=initialized,exited=host_exited;portEXIT_CRITICAL(&lock);
    esp_err_t error=ESP_OK;
    if(live && !exited)error=nimble_port_stop();
    if(error==ESP_OK && live) {
        int64_t deadline=esp_timer_get_time()+1000000;
        do {
            portENTER_CRITICAL(&lock);exited=host_exited;portEXIT_CRITICAL(&lock);
            if(exited)break;
            vTaskDelay(pdMS_TO_TICKS(10));
        }while(esp_timer_get_time()<deadline);
        if(!exited)error=ESP_ERR_TIMEOUT;
        else error=nimble_port_deinit();
    }
    if(error==ESP_OK) {
        if(receive_queue)xQueueReset(receive_queue);
        if(send_queue)xQueueReset(send_queue);
        portENTER_CRITICAL(&lock);
        initialized=false;synced=false;subscribed=false;host_exited=false;
        connection=BLE_HS_CONN_HANDLE_NONE;last_error=0;stopping=false;cleanup_needed=false;
        portEXIT_CRITICAL(&lock);
        ESP_LOGI("reader_ble","Released DMA free=%u largest=%u",(unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA),(unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA));
    } else {
        portENTER_CRITICAL(&lock);last_error=error;stopping=false;synced=false;portEXIT_CRITICAL(&lock);
    }
    return error==ESP_OK;
}
