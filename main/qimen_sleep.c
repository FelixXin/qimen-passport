#include "qimen_sleep.h"
#include <stddef.h>
#include <string.h>
#include <limits.h>
#define SLEEP_MAGIC UINT32_C(0x514d5350)
static uint32_t crc(const qimen_sleep_snapshot_t *s){
 const uint8_t *p=(const uint8_t*)s;uint32_t c=UINT32_MAX;
 for(size_t i=0;i<offsetof(qimen_sleep_snapshot_t,checksum);i++){c^=p[i];for(unsigned b=0;b<8;b++)c=(c>>1)^(UINT32_C(0xedb88320)&(0u-(c&1)));}
 return ~c;
}
bool qimen_sleep_save(qimen_sleep_snapshot_t *s,const reader_clock_t *clock,const reader_qimen_t *chart,const qimen_navigation_t *n,int64_t now,int64_t rtc){
 if(!s||!clock||!chart||!n||now<0||rtc<0||(unsigned)clock->source>READER_CLOCK_LOCAL)return false;
 if(clock->source!=READER_CLOCK_UNSET&&(clock->base_seconds<0||clock->base_seconds>=INT64_C(4102416000)||clock->anchor_us < -999999||now<clock->anchor_us))return false;
 if(clock->source!=READER_CLOCK_UNSET&&clock->anchor_us<0&&now>INT64_MAX+clock->anchor_us)return false;
 memset(s,0,sizeof(*s));s->magic=SLEEP_MAGIC;s->version=1;s->rtc_us=rtc;s->source=clock->source;
 if(s->source!=READER_CLOCK_UNSET){
  int64_t elapsed=now-clock->anchor_us;
  if(elapsed>=INT64_C(4102416000000000)-clock->base_seconds*INT64_C(1000000)){memset(s,0,sizeof(*s));return false;}
  s->clock_us=clock->base_seconds*INT64_C(1000000)+elapsed;
 }
 s->chart_valid=chart->valid;s->chart_epoch=chart->epoch_utc;s->navigation=*n;s->checksum=crc(s);return true;
}
bool qimen_sleep_restore(const qimen_sleep_snapshot_t *s,reader_clock_t *clock,reader_qimen_t *chart,qimen_navigation_t *n,int64_t now,int64_t rtc){
 if(!s||!clock||!chart||!n||now<0||now>INT64_MAX-300000||s->rtc_us<0||rtc<s->rtc_us||s->magic!=SLEEP_MAGIC||s->version!=1||s->checksum!=crc(s))return false;
 if((unsigned)s->source>READER_CLOCK_LOCAL||s->clock_us<0||s->clock_us>INT64_MAX-(rtc-s->rtc_us))return false;
 const qimen_navigation_t *v=&s->navigation;
 if(v->page>Q_WIFI_DETAIL||v->selected>10||v->palace>8||v->symbol>6||v->brightness>3||v->field>4||v->cover_cursor>4)return false;
 int64_t total=s->clock_us+(rtc-s->rtc_us);
 if(v->page==Q_WIFI_DETAIL&&(v->selected<1||v->selected>6))return false;
 if(s->source>=READER_CLOCK_NETWORK&&(total<INT64_C(1704038400000000)||total>=INT64_C(4102416000000000)))return false;
 reader_qimen_t restored={0};if(s->chart_valid&&!reader_qimen_cast(s->chart_epoch,&restored))return false;
 reader_clock_t recovered={0};recovered.source=s->source;
 if(s->source!=READER_CLOCK_UNSET){recovered.base_seconds=total/1000000;recovered.anchor_us=now-total%1000000;}
 *chart=restored;*clock=recovered;*n=*v;n->sleep=false;n->intro=false;n->pressed=0;n->repeat_at=0;
 n->last_input=now;n->quiet_until=now+300000;n->wake_guard=true;return true;
}
