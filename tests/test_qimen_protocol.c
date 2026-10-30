#include "reader_local.h"
#include "reader_network.h"
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
 assert(reader_local_parse("AP\t\t3132333435363738",&c)&&c.op==LOCAL_AP&&!c.ssid[0]&&!strcmp(c.password,"12345678"));
 assert(!reader_local_parse("WIFI\t\t3132333435363738",&c));
 assert(!reader_local_parse("AP\t\t31323334353637",&c));
 assert(!reader_local_parse("NETINFO\textra",&c)&&!reader_local_parse("RECONNECT\textra",&c));
 assert(reader_local_parse("AP\t72656e616d6564\t",&c)&&c.op==LOCAL_AP&&!strcmp(c.ssid,"renamed")&&!c.password[0]);
 assert(!reader_local_parse("AP\t\t",&c));assert(reader_local_parse("NETINFO",&c)&&c.op==LOCAL_NET_INFO);assert(reader_local_parse("RECONNECT",&c)&&c.op==LOCAL_RECONNECT);
 reader_network_info_t net={.enabled=true,.portal=true,.connected=true,.ap_clients=2,.rssi=-52};strcpy(net.station_ssid,"router");strcpy(net.saved_ssid,"router");strcpy(net.ap,"Qimen-Passport");strcpy(net.password,"do-not-leak");strcpy(net.ip,"192.168.1.9");char reply[READER_LOCAL_MAX];assert(reader_network_status_line(&net,reply,sizeof(reply))&&!strstr(reply,"do-not-leak"));unsigned tabs=0;for(const char *p=reply;*p;p++)tabs+=*p=='\t';assert(tabs==15);assert(!reader_network_status_line(&net,reply,10));
 assert(reader_network_state(&net)==4);net.connected=false;net.connecting=true;assert(reader_network_state(&net)==2);net.retry_count=1;assert(reader_network_state(&net)==3);net.connecting=false;net.last_error=NET_ERROR_LINK;net.disconnect_reason=201;assert(reader_network_state(&net)==5&&!strcmp(reader_network_error_text(&net),"未找到热点"));
 puts("Protocol: time/numeric/credential/framing bounds, removed private operations PASS");}
