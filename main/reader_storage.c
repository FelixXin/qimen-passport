#include "reader_storage.h"
#include "reader_radio.h"
#include <stdio.h>
#include <string.h>

static bool valid_config(const reader_network_credentials_t *config) {
    return config->version == 1 && config->configured <= 1 &&
           memchr(config->ssid, 0, sizeof(config->ssid)) &&
           memchr(config->password, 0, sizeof(config->password)) &&
           (!config->configured || reader_radio_credentials(config->ssid, config->password, false));
}

esp_err_t reader_storage_network_load(reader_network_credentials_t *out) {
    if (!out) return ESP_ERR_INVALID_ARG;
    memset(out, 0, sizeof(*out));
    nvs_handle_t handle;
    esp_err_t error = nvs_open("qimen_net", NVS_READONLY, &handle);
    if (error == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if (error != ESP_OK) return error;
    reader_network_credentials_t config = {0};
    size_t size = sizeof(config);
    error = nvs_get_blob(handle, "station", &config, &size);
    if (error == ESP_ERR_NVS_NOT_FOUND) {
        config.version = 1;
        size_t ssid_size = sizeof(config.ssid), password_size = sizeof(config.password);
        error = nvs_get_str(handle, "ssid", config.ssid, &ssid_size);
        if (error == ESP_ERR_NVS_NOT_FOUND) error = ESP_OK;
        else if (error == ESP_OK) {
            error = nvs_get_str(handle, "password", config.password, &password_size);
            if (error == ESP_ERR_NVS_NOT_FOUND) { config.password[0] = 0; error = ESP_OK; }
            if (error == ESP_OK) config.configured = config.ssid[0] != 0;
        }
    } else if (error == ESP_OK && size != sizeof(config)) error = ESP_ERR_INVALID_SIZE;
    if (error == ESP_OK && !valid_config(&config)) error = ESP_ERR_INVALID_ARG;
    if (error == ESP_OK) *out = config;
    memset(&config, 0, sizeof(config));
    nvs_close(handle);
    return error;
}

static esp_err_t write_config(const reader_network_credentials_t *config) {
    nvs_handle_t handle;
    esp_err_t error = nvs_open("qimen_net", NVS_READWRITE, &handle);
    if (error == ESP_OK) {
        error = nvs_set_blob(handle, "station", config, sizeof(*config));
        if (error == ESP_OK) error = nvs_commit(handle);
        nvs_close(handle);
    }
    return error;
}

esp_err_t reader_storage_network_save(const char *ssid, const char *password) {
    if (!reader_radio_credentials(ssid, password, false)) return ESP_ERR_INVALID_ARG;
    reader_network_credentials_t config = {.version = 1, .configured = 1};
    strcpy(config.ssid, ssid); strcpy(config.password, password);
    esp_err_t error = write_config(&config);
    memset(&config, 0, sizeof(config));
    return error;
}

esp_err_t reader_storage_network_forget(void) {
    reader_network_credentials_t empty = {.version = 1};
    /* A committed tombstone must hide legacy keys, including after power loss. */
    return write_config(&empty);
}
