#include "reader_pairing.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
 reader_pairing_t p={0};assert(!reader_pairing_auth(&p,123456,0));
 reader_pairing_open(&p,123456,0);uint32_t gen=p.generation;
 assert(!p.authorized&&reader_pairing_auth(&p,123456,1)&&p.authorized);
 reader_pairing_peer(&p);assert(!p.authorized&&p.generation!=gen);
 for(unsigned i=0;i<4;i++)assert(!reader_pairing_auth(&p,1,2)&&p.open);
 assert(!reader_pairing_auth(&p,1,3)&&!p.open&&!p.authorized);
 reader_pairing_open(&p,999999,0);assert(reader_pairing_active(&p,179999999));assert(!reader_pairing_auth(&p,999999,180000000));
 reader_pairing_open(&p,1234567,0);assert(p.code==234567);reader_pairing_close(&p);assert(!p.open&&!p.authorized);
 puts("Pairing: explicit window, expiry, five failures, peer/session isolation PASS");
}
