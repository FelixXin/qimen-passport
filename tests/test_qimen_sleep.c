#include "qimen_sleep.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 reader_clock_t c,out;reader_clock_init(&c);qimen_navigation_t n,restored;qimen_navigation_init(&n,0);
 n.page=Q_SYMBOL;n.palace=8;n.symbol=6;n.selected=8;n.brightness=3;n.repeat_at=123;n.pressed=7;
 reader_qimen_t q,chart;assert(reader_qimen_cast(-2209017600LL,&q));
 assert(reader_clock_local(&c,1709135999LL,0));qimen_sleep_snapshot_t s;
 assert(qimen_sleep_save(&s,&c,&q,&n,750000,10000000));
 assert(qimen_sleep_restore(&s,&out,&chart,&restored,50000,10350000));
 assert(chart.epoch_utc==q.epoch_utc&&restored.page==Q_SYMBOL&&restored.palace==8&&restored.symbol==6);
 assert(!restored.sleep&&!restored.intro&&!restored.pressed&&!restored.repeat_at&&restored.wake_guard);
 reader_clock_value_t v;reader_clock_read(&out,50000,&v);assert(v.year==2024&&v.month==2&&v.day==29&&v.hour==0&&v.minute==0&&v.second==0);
 reader_clock_read(&out,950000,&v);assert(v.second==1);
 /* A wake can happen before one second of new uptime: negative anchors are valid. */
 assert(qimen_sleep_save(&s,&out,&chart,&restored,50000,10350000));
 assert(qimen_sleep_restore(&s,&c,&q,&n,10000,11350000));reader_clock_read(&c,10000,&v);assert(v.second==1);
 qimen_sleep_snapshot_t original=s;
 for(size_t i=0;i<sizeof(s);i++){s=original;((unsigned char*)&s)[i]^=1;if(i<sizeof(s)-sizeof(s.checksum))assert(!qimen_sleep_restore(&s,&c,&q,&n,0,12000000));}
 s=original;assert(!qimen_sleep_restore(&s,&c,&q,&n,0,s.rtc_us-1));
 reader_clock_init(&c);memset(&q,0,sizeof(q));assert(qimen_sleep_save(&s,&c,&q,&n,0,0));assert(qimen_sleep_restore(&s,&out,&chart,&restored,0,86400000000LL));assert(out.source==READER_CLOCK_UNSET&&!chart.valid);
 assert(reader_clock_manual(&c,23,59,0));assert(qimen_sleep_save(&s,&c,&q,&n,0,0));assert(qimen_sleep_restore(&s,&out,&chart,&restored,0,61000000));reader_clock_read(&out,0,&v);assert(!v.has_date&&v.hour==0&&v.minute==0&&v.second==1);
 assert(reader_clock_local(&c,1735660799LL,0));assert(qimen_sleep_save(&s,&c,&q,&n,0,0));assert(qimen_sleep_restore(&s,&out,&chart,&restored,0,2000000));reader_clock_read(&out,0,&v);assert(v.year==2025&&v.month==1&&v.day==1&&v.second==1);
 puts("Sleep snapshot: CRC corruption, subsecond continuity, leap/year boundaries, historical chart, unset/manual clock and repeat sleep PASS");
}
