#include "reader_clock.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    reader_clock_t c; reader_clock_value_t v;
    reader_clock_init(&c); reader_clock_read(&c, 0, &v); assert(!v.valid && !v.has_date);
    assert(!reader_clock_manual(&c, 24, 0, 0)); assert(!reader_clock_manual(&c, 0, 60, 0));
    assert(!reader_clock_manual(&c, 0, 0, -1)); assert(c.source == READER_CLOCK_UNSET);
    assert(reader_clock_manual(&c, 23, 59, 1000000));
    reader_clock_read(&c, 61000000, &v); assert(v.valid && !v.has_date && v.hour == 0 && v.minute == 0 && v.second == 0);
    reader_clock_read(&c, 62000000, &v); assert(v.second == 1);
    reader_clock_read(&c, 0, &v); assert(v.hour == 23 && v.minute == 59);
    assert(!reader_clock_sync(&c, 0, 0)); assert(!reader_clock_sync(&c, INT64_C(4102444800), 0));
    assert(c.source == READER_CLOCK_MANUAL);
    /* 2024-02-28 23:59:59 Beijing -> leap day. */
    assert(reader_clock_sync(&c, INT64_C(1709135999), 1000000));
    reader_clock_read(&c, 1000000, &v);
    assert(v.has_date && v.year == 2024 && v.month == 2 && v.day == 28 && v.hour == 23 && v.minute == 59 && v.second == 59);
    reader_clock_read(&c, 2000000, &v); assert(v.day == 29 && v.hour == 0 && v.minute == 0 && v.second == 0 && v.weekday == 4);
    assert(reader_clock_sync(&c, INT64_C(1735660799), 0));
    reader_clock_read(&c, 1000000, &v); assert(v.year == 2025 && v.month == 1 && v.day == 1 && v.hour == 0);
    assert(reader_clock_manual(&c, 9, 30, 2000000));
    reader_clock_read(&c, 62000000, &v); assert(v.source == READER_CLOCK_MANUAL && !v.has_date && v.hour == 9 && v.minute == 31);
    assert(reader_clock_local(&c, INT64_C(1792740000), 0));
    reader_clock_read(&c, 2000000, &v); assert(v.source == READER_CLOCK_LOCAL && v.has_date && v.epoch_utc == INT64_C(1792740002));
    assert(reader_clock_local(&c, INT64_C(1704038400), 0));
    reader_clock_read(&c, 0, &v); assert(v.year==2024 && v.month==1 && v.day==1 && v.hour==0);
    assert(reader_clock_local(&c, INT64_C(4102415999), 0));
    assert(!reader_clock_local(&c, INT64_C(4102416000), 0));
    assert(!reader_clock_local(&c, INT64_C(1704038399), 0));
    reader_clock_init(&c); reader_clock_read(&c, 1000000, &v); assert(!v.valid); /* Reboot never revives a stale saved clock. */
    puts("Clock: manual/UTC+8, midnight, leap/year rollover, invalid dates and restart PASS");
}
