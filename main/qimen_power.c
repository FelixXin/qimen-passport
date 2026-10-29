#include "qimen_power.h"
#include "sdkconfig.h"
#if !CONFIG_ESP_TIME_FUNCS_USE_RTC_TIMER
#error "Deep-sleep time recovery requires the RTC time source"
#endif
#include "reader_network.h"
#include "bsp_audio.h"
#include "bsp_battery.h"
#include "bsp_button.h"
#include "bsp_display.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_timer.h"
#include <sys/time.h>
#include <string.h>
static RTC_DATA_ATTR qimen_sleep_snapshot_t retained;
static RTC_DATA_ATTR bool recover_failed_entry;
static bool pending,aborted;
static int64_t deadline;
static int64_t rtc_now(void){struct timeval t;gettimeofday(&t,NULL);return (int64_t)t.tv_sec*1000000+t.tv_usec;}
bool qimen_power_restore(reader_clock_t *clock,reader_qimen_t *chart,qimen_navigation_t *n){
 esp_reset_reason_t reason=esp_reset_reason();
 bool allowed=reason==ESP_RST_DEEPSLEEP||(reason==ESP_RST_SW&&recover_failed_entry);
 bool restored=allowed&&qimen_sleep_restore(&retained,clock,chart,n,esp_timer_get_time(),rtc_now());
 memset(&retained,0,sizeof(retained));recover_failed_entry=false;
 ESP_LOGI("qimen_power","Resume=%u reset=%d clock=%u chart=%u",restored,reason,clock->source,chart->valid);return restored;
}
bool qimen_power_request(void){
 if(pending||!reader_network_sleep_request())return false;
 pending=true;aborted=false;deadline=esp_timer_get_time()+10000000;return true;
}
bool qimen_power_pending(void){return pending&&!aborted;}
static int cancel(void){reader_network_sleep_cancel();pending=false;aborted=false;return -1;}
static void restart_after_failure(const char *step,esp_err_t error){
 ESP_LOGE("qimen_power","Sleep preparation failed at %s: %s; restarting",step,esp_err_to_name(error));
 recover_failed_entry=true;esp_restart();
}
int qimen_power_poll(const reader_clock_t *clock,const reader_qimen_t *chart,const qimen_navigation_t *n){
 if(!pending)return 0;
 int radio=reader_network_sleep_result();
 if(!aborted&&esp_timer_get_time()>=deadline){
  aborted=true;ESP_LOGE("qimen_power","Sleep preparation timeout; remaining awake");
  if(!radio)return -1;
 }
 if(!radio)return 0;
 if(aborted||radio<0)return cancel();
 int mv=bsp_button_read_mv();
 if(mv<0)return cancel();
 if(mv<BSP_BTN_MV_RELEASED_MIN)return 0;
 /* Arm GPIO only. No periodic wakeup, flash writes or network auto-start while asleep. */
 esp_err_t error=esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
 if(error==ESP_OK)error=esp_deep_sleep_enable_gpio_wakeup(1ULL<<BSP_BTN_GPIO,ESP_GPIO_WAKEUP_GPIO_LOW);
 if(error!=ESP_OK)return cancel();
 if(!bsp_lvgl_lock(1000)){esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);return cancel();}
 if(!qimen_sleep_save(&retained,clock,chart,n,esp_timer_get_time(),rtc_now())){bsp_lvgl_unlock();return cancel();}
 /* From button hand-off onward the peripherals are terminal: reboot on failure. */
 int level=0;error=bsp_button_prepare_deep_sleep(&level);
 if(error!=ESP_OK||level!=1){restart_after_failure("keys",error==ESP_OK?ESP_ERR_INVALID_STATE:error);return 0;}
 if((error=bsp_battery_sleep())!=ESP_OK){restart_after_failure("battery",error);return 0;}
 if((error=bsp_audio_idle_sleep())!=ESP_OK){restart_after_failure("codec",error);return 0;}
 if((error=bsp_audio_prepare_deep_sleep())!=ESP_OK){restart_after_failure("audio pins",error);return 0;}
 if((error=bsp_i2c_prepare_deep_sleep())!=ESP_OK){restart_after_failure("I2C pins",error);return 0;}
 if((error=bsp_display_prepare_deep_sleep())!=ESP_OK){restart_after_failure("display",error);return 0;}
 ESP_LOGI("qimen_power","Deep sleep ready: radios stopped, display off, RTC retained; GPIO wake only");
 esp_deep_sleep_start();restart_after_failure("deep sleep returned",ESP_FAIL);return 0;
}
