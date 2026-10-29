#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct {
    bool enabled,connected,connecting,portal,provisioned,ble_ready,ble_prepare_failed,request_done;
    uint32_t request_id;
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
