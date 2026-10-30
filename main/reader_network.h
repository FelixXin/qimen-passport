#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
enum {NET_ERROR_NONE,NET_ERROR_LINK,NET_ERROR_NO_IP,NET_ERROR_TIMEOUT,NET_ERROR_SAVE,NET_ERROR_QUEUE,NET_ERROR_START,NET_ERROR_NO_SAVED,NET_ERROR_BUSY};
typedef struct {
    bool enabled,connected,connecting,portal,provisioned,ble_ready,ble_prepare_failed,request_done;
    uint32_t request_id;
    bool ap_pending;int ap_clients,rssi;unsigned last_error,disconnect_reason,retry_count;
    char station_ssid[33],saved_ssid[33];
    char ap[33],password[64],key[9],ip[16],message[96];
} reader_network_info_t;
void reader_network_init(void);
bool reader_network_enable(bool enabled);
void reader_network_info(reader_network_info_t *out);
void reader_network_provision_session(uint32_t token);
uint32_t reader_network_configure(const char *ssid,const char *password,uint32_t token);
bool reader_network_set_ap(const char *ssid,const char *password);
bool reader_network_forget(void);
bool reader_network_prepare_ble(void);

bool reader_network_resume_web(void);
uint32_t reader_network_stack_free(void);

bool reader_network_release_ble(void);

/* Exclusive worker-owned shutdown handshake. No NVS preferences are changed. */
bool reader_network_sleep_request(void);
int reader_network_sleep_result(void);
void reader_network_sleep_cancel(void);

void reader_network_tick(int64_t now_us);
uint32_t reader_network_reconnect(void);
unsigned reader_network_state(const reader_network_info_t *info);
const char *reader_network_state_text(const reader_network_info_t *info);
const char *reader_network_error_text(const reader_network_info_t *info);
const char *reader_network_signal_text(const reader_network_info_t *info);

bool reader_network_status_line(const reader_network_info_t *info,char *out,size_t size);
