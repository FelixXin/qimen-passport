#define _POSIX_C_SOURCE 200809L
#include "reader_clock.h"
#include <string.h>
#include <time.h>

void reader_clock_init(reader_clock_t *clock) { memset(clock, 0, sizeof(*clock)); }
bool reader_clock_manual(reader_clock_t *c, unsigned hour, unsigned minute, int64_t now) {
    if (hour > 23 || minute > 59 || now < 0) return false;
    c->source = READER_CLOCK_MANUAL; c->base_seconds = hour * 3600 + minute * 60; c->anchor_us = now;
    return true;
}
bool reader_clock_sync(reader_clock_t *c, int64_t utc, int64_t now) {
    /* Reject unset clocks and implausible network dates (2024 through 2099). */
    if (utc < INT64_C(1704038400) || utc >= INT64_C(4102416000) || now < 0) return false;
    c->source = READER_CLOCK_NETWORK; c->base_seconds = utc; c->anchor_us = now;
    return true;
}
bool reader_clock_local(reader_clock_t *c, int64_t utc, int64_t now) {
    if (!reader_clock_sync(c, utc, now)) return false;
    c->source = READER_CLOCK_LOCAL; return true;
}
void reader_clock_read(const reader_clock_t *c, int64_t now, reader_clock_value_t *out) {
    memset(out, 0, sizeof(*out)); out->source = c->source;
    if (c->source == READER_CLOCK_UNSET) return;
    int64_t elapsed = now >= c->anchor_us ? (now - c->anchor_us) / 1000000 : 0;
    int64_t seconds = c->base_seconds + elapsed;
    if (c->source == READER_CLOCK_NETWORK || c->source == READER_CLOCK_LOCAL) {
        out->epoch_utc = seconds;
        time_t local = (time_t)(seconds + 8 * 3600); /* Beijing UTC+8, independent of process TZ. */
        struct tm date;
        if (!gmtime_r(&local, &date)) return;
        out->year = (unsigned)date.tm_year + 1900; out->month = (unsigned)date.tm_mon + 1;
        out->day = (unsigned)date.tm_mday; out->weekday = (unsigned)date.tm_wday; out->has_date = true;
        out->hour = (unsigned)date.tm_hour; out->minute = (unsigned)date.tm_min; out->second = (unsigned)date.tm_sec;
    } else {
        seconds %= 86400;
        out->hour = (unsigned)(seconds / 3600); out->minute = (unsigned)(seconds / 60 % 60); out->second = (unsigned)(seconds % 60);
    }
    out->valid = true;
}
