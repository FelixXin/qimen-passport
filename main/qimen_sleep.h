#pragma once
#include "qimen_navigation.h"
#include "reader_clock.h"
#include "reader_qimen.h"
/* Stored only in RTC RAM, never in flash. Consume only after a deep-sleep reset. */
typedef struct {
 uint32_t magic,version;
 int64_t rtc_us,clock_us,chart_epoch;
 reader_clock_source_t source;
 bool chart_valid;
 qimen_navigation_t navigation;
 uint32_t checksum;
} qimen_sleep_snapshot_t;
bool qimen_sleep_save(qimen_sleep_snapshot_t *s,const reader_clock_t *clock,const reader_qimen_t *chart,const qimen_navigation_t *navigation,int64_t monotonic_us,int64_t rtc_us);
bool qimen_sleep_restore(const qimen_sleep_snapshot_t *s,reader_clock_t *clock,reader_qimen_t *chart,qimen_navigation_t *navigation,int64_t monotonic_us,int64_t rtc_us);
