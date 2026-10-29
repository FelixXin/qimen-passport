#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum { READER_CLOCK_UNSET, READER_CLOCK_MANUAL, READER_CLOCK_NETWORK, READER_CLOCK_LOCAL } reader_clock_source_t;
typedef struct {
    reader_clock_source_t source;
    int64_t anchor_us, base_seconds;
} reader_clock_t;
typedef struct {
    reader_clock_source_t source;
    bool valid, has_date;
    int64_t epoch_utc;
    unsigned hour, minute, second, year, month, day, weekday;
} reader_clock_value_t;
void reader_clock_init(reader_clock_t *clock);
bool reader_clock_manual(reader_clock_t *clock, unsigned hour, unsigned minute, int64_t now_us);
bool reader_clock_sync(reader_clock_t *clock, int64_t utc_epoch, int64_t now_us);
bool reader_clock_local(reader_clock_t *clock, int64_t utc_epoch, int64_t now_us);
void reader_clock_read(const reader_clock_t *clock, int64_t now_us, reader_clock_value_t *value);
