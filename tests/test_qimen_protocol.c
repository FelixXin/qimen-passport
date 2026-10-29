#include "reader_local.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){reader_local_command_t c;reader_local_frame_t f={0};
 assert(reader_local_parse("TIME\t1792645200",&c)&&c.epoch==1792645200LL);
 assert(reader_local_parse("TIME\t1704038400",&c));assert(reader_local_parse("TIME\t4102415999",&c));assert(!reader_local_parse("TIME\t4102416000",&c));assert(!reader_local_parse("TIME\t1704038399",&c));
 assert(!reader_local_parse("TIME\t4102444800",&c));assert(!reader_local_parse("TIME\t1792645200\textra",&c));
 assert(reader_local_parse("WIFI\t74657374\t70617373776f7264313233",&c)&&!strcmp(c.ssid,"test")&&!strcmp(c.password,"password123"));
 assert(!reader_local_parse("WIFI\t00\t00",&c));assert(!reader_local_parse("AP\t74657374\t313233",&c));
 assert(!reader_local_parse("PROFILE\tname\tnote\t0",&c));assert(!reader_local_parse("AVATAR_BEGIN\t8192\t00000000",&c));
 assert(reader_local_parse("AUTH\t001234",&c)&&c.number==1234);assert(!reader_local_parse("AUTH\t1234",&c));
 for(unsigned i=0;i<READER_LOCAL_MAX;i++)assert(!reader_local_feed(&f,'a'));assert(reader_local_feed(&f,'\n')==-1);assert(!reader_local_feed(&f,'H'));assert(reader_local_feed(&f,'\n')==1&&!strcmp(f.line,"H"));
 assert(reader_local_parse("NAME\te5ada6e69893",&c)&&c.op==LOCAL_NAME&&!strcmp(c.ssid,"学易"));
 assert(reader_local_parse("NAME\t",&c)&&c.op==LOCAL_NAME);
 assert(!reader_local_parse("NAME\t00",&c));assert(!reader_local_parse("NAME\tc0af",&c));
 assert(reader_local_parse("COVERINFO",&c)&&c.op==LOCAL_COVER_INFO);assert(reader_local_parse("COVERRESET",&c)&&c.op==LOCAL_COVER_RESET);
 assert(reader_local_parse("CASTAT\t-2209017600",&c)&&c.op==LOCAL_CAST_AT&&c.epoch==-2209017600LL);
 assert(reader_local_parse("CASTAT\t0",&c)&&c.epoch==0);assert(reader_local_parse("CASTAT\t4102415999",&c));
 assert(!reader_local_parse("CASTAT\t-2209017601",&c));assert(!reader_local_parse("CASTAT\t4102416000",&c));assert(!reader_local_parse("CASTAT\t--1",&c));assert(!reader_local_parse("CASTAT\t+1",&c));
 assert(reader_local_parse("STYLE\t4",&c)&&c.op==LOCAL_STYLE);assert(!reader_local_parse("STYLE\t5",&c));assert(reader_local_parse("STYLEINFO",&c));
 puts("Protocol: time/numeric/credential/framing bounds, removed private operations PASS");}
