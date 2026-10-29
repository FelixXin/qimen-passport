#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct { bool open, authorized; uint32_t code, generation; unsigned failures; int64_t expires_us; } reader_pairing_t;
void reader_pairing_open(reader_pairing_t *pairing, uint32_t code, int64_t now);
void reader_pairing_close(reader_pairing_t *pairing);
void reader_pairing_peer(reader_pairing_t *pairing);
bool reader_pairing_active(reader_pairing_t *pairing, int64_t now);
bool reader_pairing_auth(reader_pairing_t *pairing, uint32_t code, int64_t now);
