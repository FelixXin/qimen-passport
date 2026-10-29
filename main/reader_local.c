#include "reader_local.h"
#include "reader_radio.h"
#include <string.h>

int reader_local_feed(reader_local_frame_t *f, uint8_t byte) {
    if (byte == '\r') return 0;
    if (byte == '\n') {
        bool bad = f->dropping; f->line[f->length] = 0; f->length = 0; f->dropping = false;
        return bad ? -1 : 1;
    }
    if (f->dropping) return 0;
    if (!byte || f->length + 1 >= sizeof(f->line)) { f->dropping = true; return 0; }
    f->line[f->length++] = (char)byte; return 0;
}
static bool number(const char *s, uint64_t max, uint64_t *out) {
    if (!*s) return false;
    uint64_t value = 0;
    for (; *s; s++) {
        if (*s < '0' || *s > '9') return false;
        unsigned digit = (unsigned)(*s - '0');
        if (digit > max || value > (max - digit) / 10) return false;
        value = value * 10 + digit;
    }
    *out = value; return true;
}
static int hex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
static bool hex_text(const char *s,char *out,size_t capacity) {
    size_t n=strlen(s);if(n%2 || n/2>=capacity)return false;
    for(size_t i=0;i<n;i+=2) {
        int a=hex(s[i]),b=hex(s[i+1]);if(a<0||b<0||!(a||b))return false;
        out[i/2]=(char)((a<<4)|b);
    }
    out[n/2]=0;return true;
}
bool reader_local_parse(const char *line, reader_local_command_t *c) {
    size_t size = strlen(line); if (size >= READER_LOCAL_MAX) return false;
    char copy[READER_LOCAL_MAX]; memcpy(copy, line, size + 1);
    char *fields[6] = {copy}; unsigned count = 1;
    for (char *p = copy; *p; p++) if (*p == '\t') {
        if (count == 6) return false;
        *p = 0; fields[count++] = p + 1;
    }
    memset(c, 0, sizeof(*c)); uint64_t n;
    if (!strcmp(fields[0], "HELLO") && count == 1) { c->op = LOCAL_HELLO; return true; }
    if (!strcmp(fields[0], "STATS") && count == 1) {c->op=LOCAL_STATS;return true;}
    if (!strcmp(fields[0], "GET") && count == 1) { c->op = LOCAL_GET; return true; }
    if (!strcmp(fields[0], "NET") && count==1) {c->op=LOCAL_NET;return true;}
    if (!strcmp(fields[0], "CAST") && count==1) {c->op=LOCAL_CAST;return true;}
    if (!strcmp(fields[0], "RADIO") && count==2) {
        if(!number(fields[1],1,&n))return false;
        c->op=LOCAL_RADIO;c->number=(uint32_t)n;return true;
    }
    if ((!strcmp(fields[0],"WIFI") || !strcmp(fields[0],"AP")) && count==3) {
        bool ap=!strcmp(fields[0],"AP");
        if(!hex_text(fields[1],c->ssid,sizeof(c->ssid)) || !hex_text(fields[2],c->password,sizeof(c->password)) ||
           !reader_radio_credentials(c->ssid,c->password,ap))return false;
        c->op=ap?LOCAL_AP:LOCAL_WIFI;return true;
    }
    if (!strcmp(fields[0], "TIME") && count == 2) {
        if (!number(fields[1], UINT64_C(4102415999), &n) || n < UINT64_C(1704038400)) return false;
        c->op = LOCAL_TIME; c->epoch = (int64_t)n; return true;
    }
    if (!strcmp(fields[0], "AUTH") && count == 2) {
        if (strlen(fields[1]) != 6 || !number(fields[1], 999999, &n)) return false;
        c->op = LOCAL_AUTH; c->number = (uint32_t)n; return true;
    }
    return false;
}
