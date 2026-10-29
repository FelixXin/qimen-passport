#pragma once
#include "qimen_sleep.h"
bool qimen_power_restore(reader_clock_t *clock,reader_qimen_t *chart,qimen_navigation_t *navigation);
bool qimen_power_request(void);
bool qimen_power_pending(void);
/* -1: preparation failed and normal operation remains available; 0: no change.
 * Success enters deep sleep and never returns. */
int qimen_power_poll(const reader_clock_t *clock,const reader_qimen_t *chart,const qimen_navigation_t *navigation);
