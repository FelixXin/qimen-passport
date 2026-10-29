#include "reader_radio.h"
#include <string.h>
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
