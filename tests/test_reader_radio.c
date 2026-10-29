#include "reader_radio.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void){
    assert(reader_radio_credentials("home","password123",false));
    assert(reader_radio_credentials("home","",false));assert(!reader_radio_credentials("home","",true));
    assert(!reader_radio_credentials("","password123",false));assert(!reader_radio_credentials("home","1234567",false));
    assert(reader_radio_credentials("home","12345678",true));
    char ssid[34],password[66];memset(ssid,'s',33);ssid[33]=0;assert(!reader_radio_credentials(ssid,"password123",false));ssid[32]=0;
    memset(password,'a',64);password[64]=0;assert(reader_radio_credentials(ssid,password,false));assert(!reader_radio_credentials(ssid,password,true));
    password[10]='z';assert(!reader_radio_credentials(ssid,password,false));password[10]='a';password[64]='a';password[65]=0;assert(!reader_radio_credentials(ssid,password,false));
    assert(!strcmp(reader_radio_phase(0,false,false,false),"IDLE"));assert(!strcmp(reader_radio_phase(0,false,true,false),"ONLINE"));
    assert(!strcmp(reader_radio_phase(1,false,false,false),"CONNECTING"));assert(!strcmp(reader_radio_phase(1,false,true,false),"SAVING"));
    assert(!strcmp(reader_radio_phase(1,true,true,true),"CONNECTED"));assert(!strcmp(reader_radio_phase(1,true,true,false),"FAILED"));
    uint8_t associated[33];memset(associated,'x',sizeof(associated));memcpy(associated,"new",4);
    assert(reader_radio_accept_address(false,"new",associated)); /* Ignore bytes after the SSID terminator. */
    assert(!reader_radio_accept_address(false,"old",associated));
    assert(!reader_radio_accept_address(true,"new",associated));
    assert(!reader_radio_accept_address(false,"",associated));
    assert(!reader_radio_accept_address(false,"new",NULL));
    memset(associated,'s',32);associated[32]=0;assert(reader_radio_accept_address(false,ssid,associated));
    associated[31]='t';assert(!reader_radio_accept_address(false,ssid,associated));
    puts("Own radio credentials and asynchronous state boundaries PASS");
}
