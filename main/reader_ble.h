#pragma once
#include "reader_local.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct { char line[READER_LOCAL_MAX]; uint32_t generation; } reader_ble_message_t;
typedef struct {
    bool initialized, window, connected, authenticated, stopping;
    char name[24]; unsigned code, seconds; int error;
} reader_ble_info_t;
bool reader_ble_open(int64_t now);
void reader_ble_close(void);
void reader_ble_info(reader_ble_info_t *info);
void reader_ble_tick(int64_t now);
bool reader_ble_next(reader_ble_message_t *message);
bool reader_ble_gate(const reader_ble_message_t *message, const reader_local_command_t *command);
void reader_ble_reply(const char *reply, uint32_t generation);

/* Conservative preflight for this one-connection C3 configuration. */
enum { READER_BLE_MIN_DMA_FREE=72*1024, READER_BLE_MIN_DMA_BLOCK=24*1024 };
static inline bool reader_ble_memory_ready(uint32_t free_bytes,uint32_t largest_block) {
    return free_bytes>=READER_BLE_MIN_DMA_FREE && largest_block>=READER_BLE_MIN_DMA_BLOCK;
}

/* Called only by the network worker after the application marks teardown pending. */
bool reader_ble_shutdown(void);
