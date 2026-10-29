#include "reader_qimen.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv){
 reader_qimen_t q;
 assert(!reader_qimen_cast(INT64_MIN,&q));assert(!reader_qimen_cast(INT64_MAX,&q));
 assert(!reader_qimen_cast(-2209017601LL,&q));assert(!reader_qimen_cast(4102416000LL,&q));
 for(int64_t t=-2209017600LL;t<4102416000LL;t+=86400){assert(reader_qimen_cast(t,&q));assert(q.year>=1900&&q.year<=2099&&q.month>=1&&q.month<=12&&q.day>=1&&q.day<=31&&q.hour==0);}
 assert(reader_qimen_cast(-2209017600LL,&q)&&q.year==1900&&q.month==1&&q.day==1&&q.hour==0);reader_qimen_t before=q;assert(!reader_qimen_step(&q,-1)&&q.epoch_utc==before.epoch_utc);
 assert(reader_qimen_cast(4102415999LL,&q));assert(!reader_qimen_step(&q,1)&&q.year==2099);
 for(int i=1;i<argc;i++){assert(reader_qimen_cast(strtoll(argv[i],NULL,10),&q));char p[4][7];for(unsigned j=0;j<4;j++)reader_qimen_pillar(q.pillars[j],p[j]);
  printf("{\"civil\":[%u,%u,%u,%u,%u,%u],\"siZhu\":{\"year\":\"%s\",\"month\":\"%s\",\"day\":\"%s\",\"time\":\"%s\"},\"term\":\"%s\"}\n",q.year,q.month,q.day,q.hour,q.minute,q.second,p[0],p[1],p[2],p[3],reader_qimen_term(q.term));
 }return 0;
}
