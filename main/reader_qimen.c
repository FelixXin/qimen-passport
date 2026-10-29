#include "reader_qimen.h"
#include <stdio.h>
#include <string.h>
#include "reader_qimen_terms.inc"
static const uint8_t ring[] = {1,8,3,4,9,2,7,6};
static const uint8_t sequence[] = {4,5,6,7,8,9,3,2,1};
static const uint8_t branch_palace[] = {1,8,8,3,4,4,9,2,2,7,6,6};
static const uint8_t ju[24][3] = {
    {2,8,5},{3,9,6},{8,5,2},{9,6,3},{1,7,4},{3,9,6},
    {4,1,7},{5,2,8},{4,1,7},{5,2,8},{6,3,9},{9,3,6},
    {8,2,5},{7,1,4},{2,5,8},{1,4,7},{9,3,6},{7,1,4},
    {6,9,3},{5,8,2},{6,9,3},{5,8,2},{4,7,1},{1,7,4}
};
static const char *stems[] = {"甲","乙","丙","丁","戊","己","庚","辛","壬","癸"};
static const char *branches[] = {"子","丑","寅","卯","辰","巳","午","未","申","酉","戌","亥"};
static const char *terms[] = {"小寒","大寒","立春","雨水","惊蛰","春分","清明","谷雨","立夏","小满","芒种","夏至","小暑","大暑","立秋","处暑","白露","秋分","寒露","霜降","立冬","小雪","大雪","冬至"};
static const char *stars[] = {"","天蓬","禽芮","天冲","天辅","天禽","天心","天柱","天任","天英"};
static const char *doors[] = {"","休门","死门","伤门","杜门","","开门","惊门","生门","景门"};
static const char *gods[] = {"值符","腾蛇","太阴","六合","白虎","玄武","九地","九天",""};
static const char *palaces[] = {"","坎一","坤二","震三","巽四","中五","乾六","兑七","艮八","离九"};
const char *reader_qimen_stem(unsigned i) { return i < 10 ? stems[i] : ""; }
const char *reader_qimen_branch(unsigned i) { return i < 12 ? branches[i] : ""; }
const char *reader_qimen_term(unsigned i) { return i < 24 ? terms[i] : ""; }
const char *reader_qimen_star(unsigned i) { return i < 10 ? stars[i] : ""; }
const char *reader_qimen_door(unsigned i) { return i < 10 ? doors[i] : ""; }
const char *reader_qimen_god(unsigned i) { return i < 9 ? gods[i] : ""; }
const char *reader_qimen_palace(unsigned i) { return i < 10 ? palaces[i] : ""; }
void reader_qimen_pillar(unsigned i, char text[7]) { snprintf(text, 7, "%s%s", stems[i%10], branches[i%12]); }
static int mod(int value, int size) { int r = value % size; return r < 0 ? r + size : r; }
static bool leap(unsigned y) { return y%4 == 0 && (y%100 != 0 || y%400 == 0); }
static void civil(reader_qimen_t *c, int64_t days, unsigned seconds) {
    c->year = 1970;
    while (days >= (leap(c->year) ? 366 : 365)) { days -= leap(c->year) ? 366 : 365; c->year++; }
    static const unsigned lengths[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    c->month = 1;
    while (days >= (int64_t)(lengths[c->month-1] + (c->month == 2 && leap(c->year)))) {
        days -= lengths[c->month-1] + (c->month == 2 && leap(c->year)); c->month++;
    }
    c->day = (unsigned)days + 1; c->hour = seconds/3600; c->minute = seconds/60%60; c->second = seconds%60;
}
static uint8_t pillar(int stem, int branch) {
    for (unsigned i = 0; i < 60; i++) if (i%10 == (unsigned)stem && i%12 == (unsigned)branch) return (uint8_t)i;
    return 0;
}
bool reader_qimen_step(reader_qimen_t *chart, int direction) {
    if (!chart || !chart->valid || (direction!=-1 && direction!=1)) return false;
    int64_t delta=(int64_t)direction*7200;
    if ((delta>0 && chart->epoch_utc>INT64_MAX-delta) || (delta<0 && chart->epoch_utc<INT64_MIN-delta)) return false;
    reader_qimen_t next;
    if (!reader_qimen_cast(chart->epoch_utc+delta,&next)) return false;
    *chart=next;return true;
}
static unsigned locate(const uint8_t *values, unsigned stem) {
    for (unsigned p = 1; p <= 9; p++) if (values[p] == stem) return p;
    return 0;
}
static int ring_index(unsigned palace) {
    for (int i = 0; i < 8; i++) if (ring[i] == palace) return i;
    return 0;
}
bool reader_qimen_cast(int64_t epoch, reader_qimen_t *c) {
    if (!c) return false;
    memset(c, 0, sizeof(*c));
    /* Civil date range in UTC+8. Guard before addition to prevent overflow. */
    if (epoch < 1704038400LL || epoch >= 4102416000LL) return false;
    int64_t local = epoch + 28800, days = local/86400;
    civil(c, days, (unsigned)(local%86400));
    unsigned y = c->year - 2023;
    int term = 23;
    for (unsigned i = 0; i < 24; i++) if (epoch >= solar_terms[y][i]) term = (int)i; else break;
    c->term = (uint8_t)term;
    unsigned year = c->year;
    if (days < ((int64_t)solar_terms[y][2]+28800)/86400) year--;
    c->pillars[0] = (uint8_t)((year-4)%60);
    unsigned exact_year = epoch < solar_terms[y][2] ? c->year-1 : c->year;
    int jie = -1;
    for (unsigned i = 0; i < 24; i += 2) if (epoch >= solar_terms[y][i]) jie = (int)i; else break;
    if (jie < 0) jie = 22;
    unsigned ordinal = (unsigned)mod(jie/2-1, 12);
    c->pillars[1] = pillar(((exact_year-4)%10%5*2+2+ordinal)%10, (ordinal+2)%12);
    c->pillars[2] = (uint8_t)mod((int)(days-10963), 60); /* 2000-01-07 = 甲子 */
    unsigned hour_branch = (c->hour+1)/2%12;
    unsigned effective_day = (c->pillars[2]+(c->hour == 23))%60;
    c->pillars[3] = pillar((effective_day%10%5*2+hour_branch)%10, hour_branch);
    unsigned hour = c->pillars[3]; c->yuan = c->pillars[2]%15/5;
    c->ju = ju[term][c->yuan]; c->yin = term >= 11 && term <= 22;
    int step = c->yin ? -1 : 1;
    for (int i = 0; i < 9; i++) c->ground[mod(c->ju-1+step*i,9)+1] = sequence[i];
    c->xun_stem = 4+hour/10;
    unsigned root = locate(c->ground,c->xun_stem), root2 = root == 5 ? 2 : root;
    unsigned time_stem = hour%10 == 0 ? c->xun_stem : hour%10;
    unsigned target = locate(c->ground,time_stem); if (target == 5) target = 2;
    int rotation = mod(ring_index(target)-ring_index(root2),8);
    for (int i = 0; i < 8; i++) {
        unsigned p = ring[mod(i+rotation,8)]; c->star[p] = ring[i]; c->heaven[p] = c->ground[ring[i]];
    }
    c->heaven[5] = c->ground[5]; c->god[5] = 8;
    c->chief_star = (uint8_t)root; c->chief_star_palace = (uint8_t)target;
    unsigned door_target = (unsigned)mod((int)root-1+step*(int)(hour%10),9)+1;
    if (door_target == 5) door_target = 2;
    int shift = mod(ring_index(door_target)-ring_index(root2),8);
    for (int i = 0; i < 8; i++) c->door[ring[mod(i+shift,8)]] = ring[i];
    c->chief_door = (uint8_t)root2; c->chief_door_palace = (uint8_t)door_target;
    for (int i = 0; i < 8; i++) c->god[ring[mod(ring_index(target)+step*i,8)]] = (uint8_t)i;
    unsigned start = c->ground[door_target] == time_stem ? 5 : door_target;
    int sequence_index = 0;
    while (sequence[sequence_index] != time_stem) sequence_index++;
    for (int i = 0; i < 9; i++) c->hidden[mod((int)start-1+step*i,9)+1] = sequence[(sequence_index+i)%9];
    for (unsigned i = 0; i < 2; i++) { c->void_branch[i] = (hour/10*10+10+i)%12; c->void_palace[i] = branch_palace[c->void_branch[i]]; }
    static const uint8_t horse[] = {2,11,8,5};
    c->horse_branch = horse[hour_branch%4]; c->horse_palace = branch_palace[c->horse_branch];
    c->epoch_utc = epoch; c->valid = true; return true;
}
