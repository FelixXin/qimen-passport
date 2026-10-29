#include "reader_pairing.h"
void reader_pairing_open(reader_pairing_t *p, uint32_t code, int64_t now) {
    p->open = true; p->authorized = false; p->code = code % 1000000; p->expires_us = now + 180000000;
    p->failures = 0; p->generation++;
}
void reader_pairing_close(reader_pairing_t *p) { p->open = false; p->authorized = false; p->generation++; }
void reader_pairing_peer(reader_pairing_t *p) { p->authorized = false; p->generation++; }
bool reader_pairing_active(reader_pairing_t *p, int64_t now) {
    if (p->open && now >= p->expires_us) reader_pairing_close(p);
    return p->open;
}
bool reader_pairing_auth(reader_pairing_t *p, uint32_t code, int64_t now) {
    if (!reader_pairing_active(p,now)) return false;
    if (code == p->code) { p->authorized = true; return true; }
    p->authorized = false;
    if (++p->failures >= 5) reader_pairing_close(p);
    return false;
}
