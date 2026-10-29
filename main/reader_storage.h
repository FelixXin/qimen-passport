#pragma once
#include <stdint.h>
#include "nvs.h"

typedef struct {
    uint32_t version;
    uint32_t configured;
    char ssid[33];
    char password[65];
} reader_network_credentials_t;

/* The single configuration blob is also a tombstone after forgetting a network.
 * Legacy separate keys are read only if no new-format record exists. */
esp_err_t reader_storage_network_load(reader_network_credentials_t *out);
esp_err_t reader_storage_network_save(const char *ssid, const char *password);
esp_err_t reader_storage_network_forget(void);
