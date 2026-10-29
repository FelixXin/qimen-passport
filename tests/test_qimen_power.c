#include "power_stubs.h"
#include <setjmp.h>
#include <assert.h>
#define gettimeofday test_gettimeofday
static int64_t mono=1000,wall=10000000;
int test_gettimeofday(struct timeval *t,void *zone){(void)zone;t->tv_sec=wall/1000000;t->tv_usec=wall%1000000;return 0;}
#include "../main/qimen_power.c"
static jmp_buf reboot;
static int reason=ESP_RST_POWERON,radio,mv=3300,stage,fail_stage;
static bool request_ok=true,lock_ok=true,arm_ok=true,locked,armed;
int64_t esp_timer_get_time(void){return mono;}
esp_reset_reason_t esp_reset_reason(void){return reason;}
void esp_restart(void){longjmp(reboot,2);}
void esp_deep_sleep_start(void){assert(locked&&armed&&stage==6);longjmp(reboot,1);}
int esp_sleep_disable_wakeup_source(int source){(void)source;armed=false;return ESP_OK;}
int esp_deep_sleep_enable_gpio_wakeup(uint64_t pins,int mode){assert(pins==1&&mode==0);armed=arm_ok;return arm_ok?ESP_OK:ESP_FAIL;}
bool reader_network_sleep_request(void){return request_ok;}
int reader_network_sleep_result(void){return radio;}
void reader_network_sleep_cancel(void){radio=0;}
int bsp_button_read_mv(void){return mv;}
bool bsp_lvgl_lock(int timeout){assert(timeout==1000);locked=lock_ok;return locked;}
void bsp_lvgl_unlock(void){assert(locked);locked=false;}
static int next(void){return ++stage==fail_stage?ESP_FAIL:ESP_OK;}
int bsp_button_prepare_deep_sleep(int *level){*level=1;return next();}
int bsp_battery_sleep(void){assert(locked&&stage==1);return next();}
int bsp_audio_idle_sleep(void){assert(stage==2);return next();}
int bsp_audio_prepare_deep_sleep(void){assert(stage==3);return next();}
int bsp_i2c_prepare_deep_sleep(void){assert(stage==4);return next();}
int bsp_display_prepare_deep_sleep(void){assert(stage==5);return next();}
static void reset(void){pending=aborted=locked=armed=false;radio=stage=fail_stage=0;mono=1000;mv=3300;request_ok=lock_ok=arm_ok=true;reason=ESP_RST_POWERON;}
int main(void){
 reader_clock_t clock;reader_clock_init(&clock);assert(reader_clock_local(&clock,1792645200LL,0));
 reader_qimen_t chart;assert(reader_qimen_cast(-2209017600LL,&chart));qimen_navigation_t n;qimen_navigation_init(&n,0);n.page=Q_DETAIL;
 reset();request_ok=false;assert(!qimen_power_request());request_ok=true;assert(qimen_power_request());assert(!qimen_power_request());assert(qimen_power_poll(&clock,&chart,&n)==0&&stage==0);
 radio=-1;assert(qimen_power_poll(&clock,&chart,&n)==-1&&!pending&&stage==0);
 reset();assert(qimen_power_request());mono+=11000000;assert(qimen_power_poll(&clock,&chart,&n)==-1&&!qimen_power_pending()&&pending);radio=1;assert(qimen_power_poll(&clock,&chart,&n)==-1&&!pending&&stage==0);
 reset();assert(qimen_power_request());radio=1;mv=0;assert(qimen_power_poll(&clock,&chart,&n)==0&&stage==0);mv=-1;assert(qimen_power_poll(&clock,&chart,&n)==-1&&!pending);
 reset();assert(qimen_power_request());radio=1;arm_ok=false;assert(qimen_power_poll(&clock,&chart,&n)==-1&&!pending&&!locked&&!armed&&stage==0);
 reset();assert(qimen_power_request());radio=1;lock_ok=false;assert(qimen_power_poll(&clock,&chart,&n)==-1&&!pending&&!locked&&stage==0);
 for(int fault=0;fault<=6;fault++){
  reset();fail_stage=fault;assert(qimen_power_request());radio=1;
  int jumped=setjmp(reboot);if(!jumped){qimen_power_poll(&clock,&chart,&n);assert(0);}
  assert(jumped==(fault?2:1));assert(stage==(fault?fault:6));
  reason=fault?ESP_RST_SW:ESP_RST_DEEPSLEEP;wall+=60000000;mono=50000;
  reader_clock_t recovered;reader_clock_init(&recovered);reader_qimen_t snapshot={0};qimen_navigation_t navigation;qimen_navigation_init(&navigation,0);
  assert(qimen_power_restore(&recovered,&snapshot,&navigation));assert(snapshot.epoch_utc==chart.epoch_utc&&navigation.page==Q_DETAIL);
  reader_clock_value_t t;reader_clock_read(&recovered,mono,&t);assert(t.epoch_utc==1792645260LL);
  assert(!qimen_power_restore(&recovered,&snapshot,&navigation));
 }
 /* Cold boot must ignore RTC remnants, even if their CRC is valid. */
 reset();assert(qimen_sleep_save(&retained,&clock,&chart,&n,mono,wall));assert(!qimen_power_restore(&clock,&chart,&n));
 puts("Power adapter: radio handshake/failure/timeout, held/invalid key, display lock, ordered stop, six terminal faults, cold boot and RTC restart recovery PASS");
}
