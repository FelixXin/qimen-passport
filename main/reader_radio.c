#include "reader_radio.h"
#include "reader_network.h"
#include <string.h>
#include <stdio.h>
bool reader_radio_credentials(const char *ssid,const char *password,bool hotspot) {
    if(!ssid || !password)return false;
    size_t sn=strlen(ssid),pn=strlen(password);
    if(!sn || sn>32 || pn>64 || (hotspot && (pn<8 || pn>63)) || (pn && pn<8))return false;
    if(pn==64)for(size_t i=0;i<pn;i++) {
        unsigned char c=(unsigned char)password[i];
        if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F')))return false;
    }
    return true;
}
const char *reader_radio_phase(uint32_t request,bool done,bool connected,bool saved) {
    if(!request)return connected?"ONLINE":"IDLE";
    if(!done)return connected?"SAVING":"CONNECTING";
    return connected&&saved?"CONNECTED":"FAILED";
}

bool reader_radio_accept_address(bool changing,const char *expected,const uint8_t *associated) {
    return !changing && expected && expected[0] && associated && !strncmp((const char*)associated,expected,32);
}

unsigned reader_network_state(const reader_network_info_t *n){
 if(!n->enabled)return 0;
 if(n->connected)return 4;
 if(n->connecting)return n->retry_count?3:2;
 if(n->last_error)return 5;
 return n->station_ssid[0]?6:1;
}
const char *reader_network_state_text(const reader_network_info_t *n){
 static const char *names[]={"已关闭","尚未配置","连接中","重连中","路由器已连接","连接失败","等待连接"};return names[reader_network_state(n)];
}
const char *reader_network_error_text(const reader_network_info_t *n){
 if(n->last_error==NET_ERROR_NONE)return "无";
 if(n->last_error==NET_ERROR_LINK){
  if(n->disconnect_reason==201)return "未找到热点";
  if(n->disconnect_reason==202)return "认证失败，请检查密码";
  if(n->disconnect_reason==204||n->disconnect_reason==15)return "认证超时";
  if(n->disconnect_reason==200)return "信号中断";
  if(n->disconnect_reason==203)return "关联失败";
  if(n->disconnect_reason>=210&&n->disconnect_reason<=212)return "热点不符合连接条件";
  return "连接已断开";
 }
 static const char *names[]={"无","连接已断开","网络地址已失效","连接超时","资料保存失败","网络任务繁忙","网络启动失败","尚无已保存网络","请先关闭蓝牙窗口"};
 return n->last_error<=NET_ERROR_BUSY?names[n->last_error]:"网络操作失败";
}
const char *reader_network_signal_text(const reader_network_info_t *n){
 if(!n->connected||n->rssi>0||n->rssi < -120)return "未知";
 return n->rssi>=-60?"强":n->rssi>=-75?"一般":"弱";
}

static void hex_ssid(const char *value,char out[65]){
 size_t i;for(i=0;i<32&&value[i];i++)snprintf(out+i*2,3,"%02x",(unsigned char)value[i]);out[i*2]=0;
}
bool reader_network_status_line(const reader_network_info_t *n,char *out,size_t size){
 char station[65],saved[65],ap[65];hex_ssid(n->station_ssid,station);hex_ssid(n->saved_ssid,saved);hex_ssid(n->ap,ap);
 int written=snprintf(out,size,"QIMEN\tOK\tNETINFO\t%u\t%u\t%d\t%s\t%s\t%u\t%s\t%d\t%u\t%u\t%s\t%u\t%u",n->enabled,n->portal,n->ap_clients,station,saved,reader_network_state(n),n->ip,n->rssi,n->disconnect_reason,n->retry_count,ap,n->ap_pending,n->last_error);
 return written>=0&&(size_t)written<size;
}
