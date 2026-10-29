#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
typedef int esp_err_t;
typedef int esp_reset_reason_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_STATE 1
#define ESP_RST_POWERON 1
#define ESP_RST_SW 3
#define ESP_RST_DEEPSLEEP 8
#define ESP_SLEEP_WAKEUP_ALL 0
#define ESP_SLEEP_WAKEUP_GPIO 7
#define ESP_GPIO_WAKEUP_GPIO_LOW 0
#define BSP_BTN_GPIO 0
#define BSP_BTN_MV_RELEASED_MIN 1900
#define RTC_DATA_ATTR
#define ESP_LOGI(tag,fmt,...) do{(void)(tag);if(0)printf(fmt,##__VA_ARGS__);}while(0)
#define ESP_LOGE ESP_LOGI
static inline const char *esp_err_to_name(int e){(void)e;return "stub";}
int64_t esp_timer_get_time(void);
esp_reset_reason_t esp_reset_reason(void);
void esp_restart(void);
void esp_deep_sleep_start(void);
int esp_sleep_disable_wakeup_source(int source);
int esp_deep_sleep_enable_gpio_wakeup(uint64_t pins,int mode);
int bsp_button_read_mv(void);
int bsp_button_prepare_deep_sleep(int *level);
bool bsp_lvgl_lock(int timeout);
void bsp_lvgl_unlock(void);
int bsp_battery_sleep(void);
int bsp_audio_idle_sleep(void);
int bsp_audio_prepare_deep_sleep(void);
int bsp_i2c_prepare_deep_sleep(void);
int bsp_display_prepare_deep_sleep(void);

#define CONFIG_ESP_TIME_FUNCS_USE_RTC_TIMER 1
