#include "qimen_ui.h"
#include "qimen_power.h"
#include "bsp_audio.h"
#include "qimen_cover_store.h"
#include "reader_local.h"
#include "reader_remote.h"
#include "reader_radio.h"
#include "reader_usb.h"
#include "bsp_display.h"
#include "bsp_button.h"
#include "bsp_battery.h"
#include "esp_app_desc.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
static qimen_navigation_t navigation;
static reader_clock_t clock_model;
static reader_qimen_t chart;
static reader_local_frame_t usb_frame;
static QueueHandle_t keys;static nvs_handle_t settings;
static bool have_settings,wifi_wanted,ble_pending,radio_restore,sntp_started;
static int64_t ble_deadline;
static char notice[96];
static int cached_battery=-1;static int64_t next_battery;
typedef struct {bsp_btn_t button;bsp_btn_ev_t event;} key_message_t;
static void render(void){
 int64_t now_us=esp_timer_get_time();if(now_us>=next_battery){cached_battery=bsp_battery_soc();next_battery=now_us+30000000;}
 qimen_view_t v={.navigation=&navigation,.chart=&chart,.notice=notice,.battery=cached_battery};
 reader_clock_read(&clock_model,esp_timer_get_time(),&v.clock);reader_network_info(&v.network);reader_ble_info(&v.bluetooth);
 qimen_cover_view_t cover_view;
 if(qimen_cover_store_acquire(&cover_view)){v.cover=cover_view;bool rendered=false;
  if(bsp_lvgl_lock(200)){qimen_ui_render(&v);bsp_lvgl_unlock();rendered=true;}
  qimen_cover_store_release(rendered);
 }
}
static bool cast_epoch(int64_t epoch){reader_qimen_t next;if(!reader_qimen_cast(epoch,&next))return false;chart=next;return true;}
static void cast(void){reader_clock_value_t now;reader_clock_read(&clock_model,esp_timer_get_time(),&now);if(!now.has_date||!cast_epoch(now.epoch_utc))strcpy(notice,"请先设置完整日期和时间");}
static void reply(const reader_ble_message_t *ble,uint32_t token,const char *line){if(ble)reader_ble_reply(line,ble->generation);else if(token)reader_remote_reply(token,line);else printf("%s\n",line);}
static void local(const char *line,const reader_ble_message_t *ble,uint32_t token){
 reader_local_command_t c;char out[READER_LOCAL_MAX];
 if(!reader_local_parse(line,&c)){reply(ble,token,"QIMEN\tERR\tCOMMAND");return;}
 if(ble&&!reader_ble_gate(ble,&c))return;
 bool ok=true,redraw=true;
 switch(c.op){
 case LOCAL_CAST_AT:ok=cast_epoch(c.epoch);if(ok){navigation.page=Q_GRID;navigation.sleep=false;navigation.intro=false;navigation.last_input=esp_timer_get_time();bsp_display_backlight((navigation.brightness+1)*25);reader_ble_close();ble_pending=false;}snprintf(out,sizeof(out),"QIMEN\t%s\tCASTAT",ok?"OK":"ERR");break;
 case LOCAL_STYLE:ok=qimen_cover_store_style(c.number);snprintf(out,sizeof(out),"QIMEN\t%s\tSTYLE",ok?"OK":"ERR");break;
 case LOCAL_STYLE_INFO:{redraw=false;qimen_cover_view_t v;if(qimen_cover_store_acquire(&v)){snprintf(out,sizeof(out),"QIMEN\tOK\tSTYLEINFO\t%lu\t%u",(unsigned long)v.style,v.custom);qimen_cover_store_release(false);}else snprintf(out,sizeof(out),"QIMEN\tERR\tBUSY");break;}
 case LOCAL_NAME:{bool supported=false;if(bsp_lvgl_lock(100)){supported=qimen_ui_name_supported(c.ssid);bsp_lvgl_unlock();}
  ok=supported&&qimen_cover_store_name(c.ssid);snprintf(out,sizeof(out),supported?"QIMEN\t%s\tNAME":"QIMEN\tERR\tNAME_GLYPH",ok?"OK":"ERR");break;}
 case LOCAL_COVER_RESET:ok=qimen_cover_store_reset();if(ok)ok=qimen_cover_store_style(0);snprintf(out,sizeof(out),"QIMEN\t%s\tCOVERRESET",ok?"OK":"ERR");break;
 case LOCAL_COVER_INFO:{redraw=false;qimen_cover_view_t v;
  if(qimen_cover_store_acquire(&v)){char encoded[QIMEN_NAME_BYTES*2]={0};for(size_t i=0;i<strlen(v.name);i++)snprintf(encoded+2*i,3,"%02x",(unsigned char)v.name[i]);
   snprintf(out,sizeof(out),"QIMEN\tOK\tCOVERINFO\t%s\t%u\t%lu",encoded,v.custom,(unsigned long)v.generation);qimen_cover_store_release(false);
  }else snprintf(out,sizeof(out),"QIMEN\tERR\tBUSY");break;}
 case LOCAL_HELLO:redraw=false;snprintf(out,sizeof(out),"QIMEN\tOK\tHELLO\t%s",esp_app_get_description()->version);break;
 case LOCAL_GET:{redraw=false;reader_clock_value_t now;reader_clock_read(&clock_model,esp_timer_get_time(),&now);snprintf(out,sizeof(out),"QIMEN\tOK\tGET\t%lld",(long long)(now.has_date?now.epoch_utc:0));break;}
 case LOCAL_TIME:ok=reader_clock_local(&clock_model,c.epoch,esp_timer_get_time());snprintf(out,sizeof(out),"QIMEN\t%s\tTIME",ok?"OK":"ERR");if(ok&&!chart.valid)cast();break;
 case LOCAL_CAST:cast();ok=chart.valid;snprintf(out,sizeof(out),"QIMEN\t%s\tCAST",ok?"OK":"ERR");break;
 case LOCAL_NET:{redraw=false;reader_network_info_t n;reader_network_info(&n);snprintf(out,sizeof(out),"QIMEN\tOK\tNET\t%u\t%u\t%u\t%u\t%lu\t%s\t%s",n.enabled,n.connected,n.connecting,n.provisioned,(unsigned long)n.request_id,reader_radio_phase(n.request_id,n.request_done,n.connected,n.provisioned),n.ip);break;}
 case LOCAL_WIFI:{uint32_t id=reader_network_configure(c.ssid,c.password,ble?ble->generation:0);if(id){wifi_wanted=true;if(have_settings){esp_err_t err=nvs_set_u8(settings,"wifi",1);if(err==ESP_OK)err=nvs_commit(settings);if(err!=ESP_OK)strcpy(notice,"网络偏好保存失败");}}snprintf(out,sizeof(out),"QIMEN\t%s\tWIFI\t%lu",id?"OK":"ERR",(unsigned long)id);break;}
 case LOCAL_AP:ok=reader_network_set_ap(c.ssid,c.password);snprintf(out,sizeof(out),"QIMEN\t%s\tAP",ok?"OK":"ERR");break;
 case LOCAL_RADIO:
  /* HTTP must return before stopping its own server; radio setting is a physical-key action. */
  snprintf(out,sizeof(out),"QIMEN\tERR\tUSE_DEVICE_KEYS");break;
 case LOCAL_STATS:redraw=false;snprintf(out,sizeof(out),"QIMEN\tOK\tSTATS\t%u\t%u\t%u\t%u",(unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),(unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),(unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA),(unsigned)uxTaskGetStackHighWaterMark(NULL));break;
 default:snprintf(out,sizeof(out),"QIMEN\tERR\tAUTH");break;
 }
 memset(&c,0,sizeof(c));reply(ble,token,out);memset(out,0,sizeof(out));if(redraw)render();
}
static void on_key(bsp_btn_t button,bsp_btn_ev_t event,void *arg){(void)arg;key_message_t k={button,event};if(keys)(void)xQueueSend(keys,&k,0);}
static void begin_ble(void){
 reader_ble_info_t ble;reader_ble_info(&ble);if(ble.window){reader_ble_close();ble_pending=false;return;}
 ble_pending=reader_network_prepare_ble();radio_restore=ble_pending;ble_deadline=esp_timer_get_time()+5000000;
 strcpy(notice,ble_pending?"正在准备蓝牙":"网络任务忙，请重试");
}
static void input(key_message_t k){
 if(qimen_power_pending())return;
 qimen_key_t key;switch(k.button){case BSP_BTN_UP:key=K_UP;break;case BSP_BTN_DOWN:key=K_DOWN;break;case BSP_BTN_OK:key=K_OK;break;default:return;}
 qimen_event_t event;switch(k.event){case BSP_BTN_PRESS:event=E_PRESS;break;case BSP_BTN_RELEASE:event=E_RELEASE;break;case BSP_BTN_CLICK:event=E_CLICK;break;case BSP_BTN_DOUBLE:event=E_DOUBLE;break;case BSP_BTN_LONG:event=E_LONG;break;default:return;}
 qimen_page_t previous=navigation.page;
 qimen_action_t action=qimen_navigation_key(&navigation,key,event,esp_timer_get_time());
 if(action==A_NONE)return;
 if(previous==Q_BLE&&navigation.page!=Q_BLE){reader_ble_close();ble_pending=false;}
 if(previous!=Q_COVER&&navigation.page==Q_COVER){qimen_cover_view_t cover;if(qimen_cover_store_acquire(&cover)){navigation.cover_cursor=cover.style;qimen_cover_store_release(false);}}
 if(previous!=Q_CAST_TIME&&navigation.page==Q_CAST_TIME&&chart.valid){navigation.year=chart.year;navigation.month=chart.month;navigation.day=chart.day;navigation.hour=chart.hour;navigation.minute=chart.minute;}
 if(previous!=Q_CLOCK&&navigation.page==Q_CLOCK){reader_clock_value_t c;reader_clock_read(&clock_model,esp_timer_get_time(),&c);if(c.has_date){navigation.year=c.year;navigation.month=c.month;navigation.day=c.day;navigation.hour=c.hour;navigation.minute=c.minute;}}
 notice[0]=0;
 switch(action){
 case A_DEEP_SLEEP:
  ble_pending=false;radio_restore=false;reader_ble_close();
  if(sntp_started){esp_netif_sntp_deinit();sntp_started=false;}
  if(qimen_power_request())strcpy(notice,"正在关闭，请稍候");else strcpy(notice,"操作失败，请重试");
  break;
 case A_CAST:cast();break;
 case A_CAST_TIME:{int64_t epoch;if(qimen_navigation_epoch(&navigation,&epoch)&&cast_epoch(epoch))navigation.page=Q_GRID;else strcpy(notice,"日期无效，请调整");break;}
 case A_STYLE:if(qimen_cover_store_style(navigation.cover_cursor))navigation.sleep=true;else strcpy(notice,"请先上传自定义封面");break;
 case A_PREV:case A_NEXT:if(!reader_qimen_step(&chart,action==A_PREV?-1:1))strcpy(notice,"请校时或已到日期边界");break;
 case A_SCROLL_UP:case A_SCROLL_DOWN:{int dir=action==A_SCROLL_DOWN?1:-1;bool edge=false;if(bsp_lvgl_lock(100)){edge=qimen_ui_scroll(dir);bsp_lvgl_unlock();}if(edge&&qimen_navigation_turn(&navigation,dir))render();return;}
 case A_WIFI:{bool wanted=!wifi_wanted;if(reader_network_enable(wanted)){wifi_wanted=wanted;if(have_settings){esp_err_t err=nvs_set_u8(settings,"wifi",wanted);if(err==ESP_OK)err=nvs_commit(settings);if(err!=ESP_OK)strcpy(notice,"网络偏好保存失败");}}else strcpy(notice,"网络任务忙，请重试");break;}
 case A_BLE:begin_ble();break;
 case A_TIME:{int64_t epoch;if(qimen_navigation_epoch(&navigation,&epoch)&&reader_clock_local(&clock_model,epoch,esp_timer_get_time())){navigation.page=Q_GRID;cast();}else strcpy(notice,"日期无效，请调整");break;}
 case A_PREF:if(have_settings){esp_err_t err=nvs_set_u8(settings,"brightness",navigation.brightness);if(err==ESP_OK)err=nvs_commit(settings);if(err!=ESP_OK)strcpy(notice,"亮度保存失败");}break;
 default:break;
 }
 bsp_display_backlight(navigation.sleep?10:(navigation.brightness+1)*25);render();
}
void app_main(void){
 ESP_LOGI("qimen","Qimen Passport %s starting",esp_app_get_description()->version);
 esp_err_t error=nvs_flash_init();if(error!=ESP_OK){ESP_LOGE("qimen","NVS unavailable: %s; no automatic erase",esp_err_to_name(error));}
 have_settings=error==ESP_OK&&nvs_open("qimen_app",NVS_READWRITE,&settings)==ESP_OK;
 qimen_cover_store_init();qimen_navigation_init(&navigation,esp_timer_get_time());navigation.intro=true;reader_clock_init(&clock_model);
 bool resumed=qimen_power_restore(&clock_model,&chart,&navigation);
 uint8_t value;if(have_settings&&nvs_get_u8(settings,"brightness",&value)==ESP_OK&&value<4)navigation.brightness=value;
 if(bsp_display_init()!=ESP_OK||!bsp_lvgl_init()){ESP_LOGE("qimen","Display unavailable");return;}
 (void)bsp_battery_init();
 if(bsp_audio_idle_sleep()!=ESP_OK)ESP_LOGW("qimen_power","Unused codec sleep failed; will retry before deep sleep");
 if(!bsp_lvgl_lock(1000))return;
 qimen_ui_create();bsp_lvgl_unlock();
 keys=xQueueCreate(16,sizeof(key_message_t));if(!keys||bsp_button_init(on_key,NULL)!=ESP_OK){ESP_LOGE("qimen","Buttons unavailable");return;}
 reader_remote_init();reader_network_init();bool usb=reader_usb_init();
 if(have_settings&&nvs_get_u8(settings,"wifi",&value)==ESP_OK&&value)wifi_wanted=reader_network_enable(true);
 bsp_display_backlight((navigation.brightness+1)*25);render();
 if(resumed){navigation.last_input=esp_timer_get_time();navigation.quiet_until=navigation.last_input+300000;navigation.wake_guard=true;}
 int64_t next_render=0,intro_until=esp_timer_get_time()+2000000;bool connected_before=false;
 for(;;){
  key_message_t key;while(xQueueReceive(keys,&key,0)==pdTRUE)input(key);
  if(qimen_navigation_tick(&navigation,esp_timer_get_time())!=A_NONE)render();
  reader_remote_message_t remote;if(reader_remote_next(&remote)){local(remote.line,NULL,remote.token);memset(&remote,0,sizeof(remote));}
  reader_ble_message_t ble_message;while(reader_ble_next(&ble_message)){local(ble_message.line,&ble_message,0);memset(&ble_message,0,sizeof(ble_message));}
  if(usb){uint8_t bytes[64];int count=read(STDIN_FILENO,bytes,sizeof(bytes));for(int i=0;i<count;i++){int status=reader_local_feed(&usb_frame,bytes[i]);if(status==1)local(usb_frame.line,NULL,0);else if(status<0)printf("QIMEN\tERR\tLENGTH\n");}memset(bytes,0,sizeof(bytes));}
  int64_t now=esp_timer_get_time();if(navigation.intro&&now>=intro_until){navigation.intro=false;render();}reader_ble_tick(now);reader_network_info_t net;reader_network_info(&net);reader_ble_info_t ble;reader_ble_info(&ble);
  if(!qimen_power_pending()&&ble_pending&&(net.ble_ready||now>=ble_deadline)){ble_pending=false;if(!net.ble_ready||net.ble_prepare_failed||!reader_ble_open(now))strcpy(notice,"蓝牙启动失败，请重试");else notice[0]=0;render();}
  if(!qimen_power_pending()&&radio_restore&&!ble_pending&&!ble.initialized&&!ble.stopping&&!ble.window){if(!wifi_wanted||reader_network_resume_web())radio_restore=false;}
  if(!qimen_power_pending()&&net.connected&&!sntp_started){esp_sntp_config_t config=ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");config.start=true;sntp_started=esp_netif_sntp_init(&config)==ESP_OK;}
  if(!qimen_power_pending()&&net.connected&&sntp_started&&esp_netif_sntp_sync_wait(0)==ESP_OK){time_t utc=time(NULL);if(utc>=1704038400&&reader_clock_sync(&clock_model,utc,now)&&!chart.valid){cast();render();}}
  if(!qimen_power_pending()&&navigation.page!=Q_BLE&&!navigation.sleep&&!navigation.pressed&&now-navigation.last_input>60000000){navigation.sleep=true;bsp_display_backlight(10);render();}
  if(now>=next_render||connected_before!=net.connected){next_render=now+1000000;render();connected_before=net.connected;}
  if(qimen_power_poll(&clock_model,&chart,&navigation)<0){
   strcpy(notice,"操作失败，请重试");if(wifi_wanted&&!qimen_power_pending())reader_network_enable(true);render();
  }
  vTaskDelay(pdMS_TO_TICKS(10));
 }
}
