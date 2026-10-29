#include "reader_qimen.h"
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void mapping(const char *name, const uint8_t *values, unsigned kind) {
    printf(",\"%s\":{",name);
    for (unsigned p=1;p<=9;p++) {
        const char *text=kind==0?reader_qimen_stem(values[p]):kind==1?reader_qimen_star(values[p]):kind==2?reader_qimen_door(values[p]):reader_qimen_god(values[p]);
        printf("%s\"%u\":\"%s\"",p==1?"":",",p,text);
    }
    printf("}");
}
int main(int argc, char **argv) {
    reader_qimen_t c;
    assert(!reader_qimen_cast(INT64_MAX,&c) && !c.valid);
    assert(!reader_qimen_cast(INT64_MIN,&c) && !c.valid);
    assert(!reader_qimen_cast(-2209017601LL,&c));
    assert(reader_qimen_cast(1704038400LL,&c));
    assert(reader_qimen_cast(4102415999LL,&c));
    assert(!reader_qimen_cast(4102416000LL,&c));
    assert(!reader_qimen_cast(1704038400LL,NULL));
    /* Exercise every hour across four years including leap day and all terms. */
    for (int64_t t=1704038400LL;t<1830268800LL;t+=3600) {
        assert(reader_qimen_cast(t,&c));
        unsigned seen=0;
        for(unsigned p=1;p<=9;p++){assert(c.ground[p]>=1 && c.ground[p]<=9);seen|=1u<<c.ground[p];}
        assert(seen==0x3fe && c.chief_star_palace!=5 && c.chief_door_palace!=5);
        assert(c.pillars[0]<60 && c.pillars[1]<60 && c.pillars[2]<60 && c.pillars[3]<60);
    }
    for(int arg=1;arg<argc;arg++) {
        assert(reader_qimen_cast(strtoll(argv[arg],NULL,10),&c));
        char pillars[4][7];for(unsigned i=0;i<4;i++)reader_qimen_pillar(c.pillars[i],pillars[i]);
        printf("{\"siZhu\":{\"year\":\"%s\",\"month\":\"%s\",\"day\":\"%s\",\"time\":\"%s\"}",pillars[0],pillars[1],pillars[2],pillars[3]);
        printf(",\"juShu\":{\"jieQiName\":\"%s\",\"type\":\"%s\",\"num\":%u,\"yuan\":\"%s\"}",reader_qimen_term(c.term),c.yin?"yin":"yang",c.ju,(const char*[]){"上元","中元","下元"}[c.yuan]);
        printf(",\"xunShou\":\"%s\",\"zhiFuGong\":\"%u\",\"zhiFuXing\":\"%s\",\"zhiShiGong\":\"%u\",\"zhiShiMen\":\"%s\"",reader_qimen_stem(c.xun_stem),c.chief_star_palace,reader_qimen_star(c.chief_star),c.chief_door_palace,reader_qimen_door(c.chief_door));
        mapping("diPanGan",c.ground,0);mapping("tianPanGan",c.heaven,0);mapping("jiuXing",c.star,1);mapping("baMen",c.door,2);mapping("baShen",c.god,3);mapping("anGan",c.hidden,0);
        printf(",\"kongWangGan\":[\"%s\",\"%s\"],\"kongWangGong\":[\"%u\",\"%u\"],\"yiMa\":{\"gan\":\"%s\",\"gong\":\"%u\"}}\n",reader_qimen_branch(c.void_branch[0]),reader_qimen_branch(c.void_branch[1]),c.void_palace[0],c.void_palace[1],reader_qimen_branch(c.horse_branch),c.horse_palace);
    }
    return 0;
}
