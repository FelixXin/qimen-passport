#pragma once
#include <stdbool.h>
#include <stdint.h>
bool reader_radio_credentials(const char *ssid, const char *password, bool hotspot);
const char *reader_radio_phase(uint32_t request, bool done, bool connected, bool saved);

bool reader_radio_accept_address(bool changing,const char *expected,const uint8_t *associated);
