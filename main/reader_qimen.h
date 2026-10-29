#pragma once
#include <stdbool.h>
#include <stdint.h>
/* OpenFate's observed default: 四柱 + 时家, Beijing time, 1900–2099.
 * A cast is a snapshot. No network or LVGL dependency. Arrays use palace 1..9. */
typedef struct {
    bool valid, yin;
    int64_t epoch_utc;
    unsigned year, month, day, hour, minute, second;
    uint8_t pillars[4], term, yuan, ju, xun_stem;
    uint8_t chief_star, chief_star_palace, chief_door, chief_door_palace;
    uint8_t ground[10], heaven[10], star[10], door[10], god[10], hidden[10];
    uint8_t void_branch[2], void_palace[2], horse_branch, horse_palace;
} reader_qimen_t;
bool reader_qimen_cast(int64_t epoch_utc, reader_qimen_t *chart);
/* Previous/next hour chart: direction is -1/+1, preserving minute/second.
 * Failure (including supported-date boundaries) leaves the snapshot unchanged. */
bool reader_qimen_step(reader_qimen_t *chart, int direction);
const char *reader_qimen_stem(unsigned index);
const char *reader_qimen_branch(unsigned index);
const char *reader_qimen_term(unsigned index);
const char *reader_qimen_star(unsigned palace); /* 2 = 禽芮, 5 = 天禽 */
const char *reader_qimen_door(unsigned palace);
const char *reader_qimen_god(unsigned index); /* 8 = empty */
const char *reader_qimen_palace(unsigned palace);
void reader_qimen_pillar(unsigned index, char text[7]);
