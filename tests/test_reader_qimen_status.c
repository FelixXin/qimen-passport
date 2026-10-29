#include "reader_qimen.h"
#include "reader_qimen_status.h"
#include "reader_qimen_text.h"
#include "reader_qimen_notes.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "qimen_growth_reference.inc"
static void screenshot_examples(void) {
    /* The user reference screenshot has no date; compare its explicit individual states. */
    static const struct { unsigned stem,palace,g0,g1,flags; } cases[] = {
        {1,4,1,2,0}, {9,4,10,11,QIMEN_PUNISHMENT},
        {6,8,8,9,QIMEN_TOMB|QIMEN_PUNISHMENT}, {5,8,7,8,QIMEN_TOMB},
        {4,6,8,9,QIMEN_TOMB}, {3,2,1,2,0}, {8,2,11,0,0}, {2,2,5,6,0}
    };
    for (unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);i++) {
        reader_qimen_stem_status_t s;assert(reader_qimen_stem_status(cases[i].stem,cases[i].palace,&s));
        assert(s.count==2&&s.growth[0]==cases[i].g0&&s.growth[1]==cases[i].g1&&s.flags==cases[i].flags);
    }
}
static void reference_image_plate(void) {
    /* The image omits its timestamp. This compatible cast reproduces the shown
       chart, without claiming to recover the original capture date. */
    reader_qimen_t q;assert(reader_qimen_cast(1792645200LL,&q));
    static const uint8_t ground[10]={0,1,2,3,9,8,7,6,5,4};
    static const uint8_t heaven[10]={0,2,3,7,1,8,4,9,6,5};
    static const uint8_t stars[10]={0,2,3,6,1,0,9,4,7,8};
    static const uint8_t doors[10]={0,6,9,8,3,0,7,2,1,4};
    static const uint8_t gods[10]={0,7,2,5,4,8,0,1,6,3};
    static const uint8_t hidden[10]={0,8,7,6,5,4,1,2,3,9};
    for(unsigned p=1;p<=9;p++) {
        assert(q.ground[p]==ground[p] && q.heaven[p]==heaven[p]);
        assert(q.star[p]==stars[p] && q.door[p]==doors[p] && q.god[p]==gods[p] && q.hidden[p]==hidden[p]);
        assert(!reader_qimen_door_forced(q.door[p],p));
    }
    assert(q.yin && q.ju==9 && q.yuan==1 && q.xun_stem==4 && q.chief_star_palace==6 && q.chief_door_palace==2 && q.horse_branch==5);
}
static void exhaustive_status(void) {
    static const unsigned tomb[] = {2,6,6,8,6,8,8,4,4,2},punish[] = {0,0,0,0,3,2,8,9,4,4};
    for (unsigned stem=0;stem<10;stem++) for (unsigned palace=1;palace<=9;palace++) {
        reader_qimen_stem_status_t s;assert(reader_qimen_stem_status(stem,palace,&s));
        for(unsigned i=0;i<s.count;i++)assert(s.growth[i]==growth_reference[stem][s.branch[i]]);
        assert((bool)(s.flags&QIMEN_TOMB)==(palace==tomb[stem]));
        assert((bool)(s.flags&QIMEN_PUNISHMENT)==(palace==punish[stem]));
        if (palace==5) assert(s.count==0&&s.flags==0);
    }
    reader_qimen_stem_status_t s;
    assert(!reader_qimen_stem_status(10,1,&s));assert(!reader_qimen_stem_status(0,0,&s));
    assert(!reader_qimen_stem_status(1,10,&s));assert(!reader_qimen_stem_status(1,1,NULL));
    assert(!strcmp(reader_qimen_element_name((reader_qimen_element_t)-1),""));
    assert(!strcmp(reader_qimen_growth(12),""));
}
static void doors(void) {
    static const unsigned expected[10] = {0,1u<<9,1u<<1,(1u<<2)|(1u<<8),(1u<<2)|(1u<<8),0,(1u<<3)|(1u<<4),(1u<<3)|(1u<<4),1u<<1,(1u<<6)|(1u<<7)};
    unsigned count=0;
    for(unsigned door=0;door<11;door++)for(unsigned p=0;p<11;p++) {
        bool force=door<10&&(expected[door]&(1u<<p));
        assert(reader_qimen_door_forced(door,p)==force);count+=force;
    }
    assert(count==13);
    assert(!reader_qimen_door_forced(9,1)); /* Palace water restrains fire: not 门迫. */
}
static void month_strength(void) {
    /* Explicit traditional table in 木火土金水 order, 子 through 亥.
       Four earth months use the whole-month branch convention, not eighteen days. */
    static const char *expected[12][5] = {
        {"相","死","囚","休","旺"}, {"囚","休","旺","相","死"},
        {"旺","相","死","囚","休"}, {"旺","相","死","囚","休"},
        {"囚","休","旺","相","死"}, {"休","旺","相","死","囚"},
        {"休","旺","相","死","囚"}, {"囚","休","旺","相","死"},
        {"死","囚","休","旺","相"}, {"死","囚","休","旺","相"},
        {"囚","休","旺","相","死"}, {"相","死","囚","休","旺"}
    };
    for(unsigned month=0;month<12;month++) for(unsigned stem=0;stem<10;stem++) {
        assert(!strcmp(reader_qimen_month_strength(stem,month),expected[month][stem/2]));
        assert(!strcmp(reader_qimen_branch_strength(stem,month),expected[month][stem/2]));
    }
    assert(reader_qimen_month_element(12)==QIMEN_NO_ELEMENT);
    assert(reader_qimen_month_element((unsigned)-1)==QIMEN_NO_ELEMENT);
    assert(!strcmp(reader_qimen_month_strength(10,0),""));
    assert(!strcmp(reader_qimen_month_strength(0,12),""));
}
static void summary_and_pillars(void) {
    reader_qimen_t q;assert(reader_qimen_cast(1792645200LL,&q));
    char text[128];assert(reader_qimen_pillars_text(&q,text,sizeof(text)));
    char first[7],second[7],third[7],fourth[7];
    reader_qimen_pillar(q.pillars[0],first);reader_qimen_pillar(q.pillars[1],second);
    reader_qimen_pillar(q.pillars[2],third);reader_qimen_pillar(q.pillars[3],fourth);
    assert(strstr(text,"本盘四柱 · 阴遁9局\n"));
    assert(strstr(text,first)&&strstr(text,second)&&strstr(text,third)&&strstr(text,fourth));
    size_t needed=strlen(text)+1;
    for(size_t capacity=1;capacity<needed;capacity++) { assert(!reader_qimen_pillars_text(&q,text,capacity));assert(!text[0]); }
    assert(!reader_qimen_pillars_text(NULL,text,sizeof(text))&&!text[0]);
    assert(!reader_qimen_pillars_text(&q,NULL,0));
    q.heaven[8]=6;q.ground[8]=5;q.hidden[8]=1;q.door[8]=3;
    q.void_palace[0]=q.horse_palace=8;
    assert(reader_qimen_summary_text(&q,8,text,sizeof(text)));
    assert(!strcmp(text,"#c45b00 入墓# #9c36bf 击刑# #c93030 门迫# #5e6aa0 空亡# #007f79 驿马#"));
    needed=strlen(text)+1;
    for(size_t capacity=1;capacity<needed;capacity++) { assert(!reader_qimen_summary_text(&q,8,text,capacity));assert(!text[0]); }
    q.void_palace[0]=q.void_palace[1]=q.horse_palace=4;
    assert(reader_qimen_summary_text(&q,5,text,sizeof(text)));
    assert(!strcmp(text,"#74746c 无支# #9c36bf 无刑# #c93030 无迫# #74746c 非空# #74746c 无马#"));
    q.heaven[9]=q.ground[9]=q.hidden[9]=1;q.door[9]=4;
    assert(reader_qimen_summary_text(&q,9,text,sizeof(text)));
    assert(!strcmp(text,"#c45b00 无墓# #9c36bf 无刑# #c93030 无迫# #74746c 非空# #74746c 无马#"));
    /* Keep category colors on absence, without changing actual cell colors. */
    char cell[256];assert(reader_qimen_cell_text(&q,9,false,cell,sizeof(cell)));
    assert(!strstr(cell,"#c45b00 ")&&!strstr(cell,"#9c36bf ")&&!strstr(cell,"#c93030 "));
    /* Carried center stem can be the only tomb marker at the destination. */
    q.heaven[2]=q.ground[2]=q.hidden[2]=1;q.ground[5]=9;
    assert(reader_qimen_summary_text(&q,2,text,sizeof(text)));assert(strstr(text,"#c45b00 入墓#"));
    q.valid=false;assert(!reader_qimen_pillars_text(&q,text,sizeof(text))&&!text[0]);
}
static void rich_text(void) {
    reader_qimen_t q;assert(reader_qimen_cast(1792742100LL,&q));
    q.heaven[8]=6;q.ground[8]=5;q.hidden[8]=1;q.door[8]=3;q.star[8]=2;q.god[8]=6;
    q.pillars[1]=34;q.pillars[2]=41;q.pillars[3]=7;
    q.ground[5]=8;q.chief_star_palace=8;q.chief_door_palace=8;
    q.void_palace[0]=q.horse_palace=8;
    char text[READER_QIMEN_SYMBOL_CAPACITY];
    assert(reader_qimen_symbol_text(&q,8,QIMEN_SYMBOL_HEAVEN,text,sizeof(text)));
    assert(strstr(text,"落宫 艮八宫 · 土"));
    assert(strstr(text,"#007ba5 天盘 庚金 [刑墓]#"));
    assert(strstr(text,"长生：丑墓 / 寅绝\n月相 日死 时相\n参照支：戌/巳/未"));
    assert(strstr(text,"天寄 壬") && !strstr(text,"暗干 乙"));
    assert(reader_qimen_symbol_text(&q,8,QIMEN_SYMBOL_HIDDEN,text,sizeof(text)));
    assert(strstr(text,"暗干 乙") && strstr(text,"参照支：戌/巳/未") && !strstr(text,"天盘 庚"));
    assert(!strstr(text,"地盘 己") && !strstr(text,"三期"));
    assert(reader_qimen_symbol_text(&q,8,QIMEN_SYMBOL_EARTH,text,sizeof(text)));
    assert(strstr(text,"#c45b00 地盘 己土 [入墓]#"));
    assert(strstr(text,"长生：寅死 / 丑墓\n月旺 日相 时旺"));
    assert(!strstr(text,"天盘 庚"));
    assert(reader_qimen_symbol_text(&q,2,QIMEN_SYMBOL_EARTH,text,sizeof(text)) && strstr(text,"地寄 壬"));
    assert(reader_qimen_symbol_text(&q,8,QIMEN_SYMBOL_DOOR,text,sizeof(text)));
    assert(strstr(text,"#c93030 伤门 [值使]#") && strstr(text,"门木克宫土") && strstr(text,"伤门属木"));
    assert(reader_qimen_symbol_text(&q,8,QIMEN_SYMBOL_STAR,text,sizeof(text)));
    assert(strstr(text,"#007f79 禽芮 [值符]#") && strstr(text,"天芮属土") && strstr(text,"天禽为中宫星"));
    assert(reader_qimen_symbol_text(&q,8,QIMEN_SYMBOL_GOD,text,sizeof(text)) && strstr(text,"低、小、慢"));
    assert(reader_qimen_symbol_text(&q,8,QIMEN_SYMBOL_PALACE,text,sizeof(text)));
    assert(strstr(text,"艮八，东北") && strstr(text,"#5e6aa0 本宫空亡#") && strstr(text,"#007f79 本宫有驿马#"));
    assert(strstr(text,"#9c36bf 击刑#") && strstr(text,"#007ba5 刑+墓#"));
    q.month=2;assert(reader_qimen_symbol_text(&q,8,QIMEN_SYMBOL_HEAVEN,text,sizeof(text)) && strstr(text,"月相 日死 时相"));
    q.pillars[1]=2;q.pillars[2]=0;q.pillars[3]=8;
    assert(reader_qimen_symbol_text(&q,8,QIMEN_SYMBOL_HEAVEN,text,sizeof(text)));
    assert(strstr(text,"月囚 日休 时旺") && strstr(text,"参照支：寅/子/申"));
    assert(reader_qimen_cell_text(&q,8,false,text,sizeof(text)) && strstr(text,"#007ba5 庚#"));
    assert(reader_qimen_cell_text(&q,8,true,text,sizeof(text)) && strstr(text,"#67cfff 庚#"));
    assert(!strcmp(reader_qimen_symbol_name(QIMEN_SYMBOL_PALACE),"宫"));
    assert(!strcmp(reader_qimen_symbol_name((reader_qimen_symbol_t)-1),""));
    assert(!reader_qimen_symbol_text(&q,8,QIMEN_SYMBOL_COUNT,text,sizeof(text)) && !text[0]);
    assert(!reader_qimen_symbol_title(&q,8,QIMEN_SYMBOL_COUNT,text,sizeof(text)) && !text[0]);
    assert(!reader_qimen_symbol_text(NULL,8,QIMEN_SYMBOL_PALACE,text,sizeof(text)) && !text[0]);
    assert(!reader_qimen_symbol_text(&q,0,QIMEN_SYMBOL_PALACE,text,sizeof(text)) && !text[0]);
    for(unsigned symbol=0;symbol<QIMEN_SYMBOL_COUNT;symbol++) {
        assert(reader_qimen_symbol_text(&q,8,(reader_qimen_symbol_t)symbol,text,sizeof(text)));
        size_t needed=strlen(text)+1;
        for(size_t capacity=1;capacity<needed;capacity++) {
            assert(!reader_qimen_symbol_text(&q,8,(reader_qimen_symbol_t)symbol,text,capacity));assert(!text[0]);
        }
        assert(!reader_qimen_symbol_text(&q,8,(reader_qimen_symbol_t)symbol,NULL,0));
        assert(reader_qimen_symbol_title(&q,8,(reader_qimen_symbol_t)symbol,text,sizeof(text)));
        needed=strlen(text)+1;
        for(size_t capacity=1;capacity<needed;capacity++)assert(!reader_qimen_symbol_title(&q,8,(reader_qimen_symbol_t)symbol,text,capacity) && !text[0]);
    }
    for(int64_t epoch=1790800000LL;epoch<1793478400LL;epoch+=7200) {
        assert(reader_qimen_cast(epoch,&q));
        for(unsigned p=1;p<=9;p++) {
            for(unsigned symbol=0;symbol<QIMEN_SYMBOL_COUNT;symbol++) {
                assert(reader_qimen_symbol_text(&q,p,(reader_qimen_symbol_t)symbol,text,sizeof(text)));
                assert(!strstr(text,"三期") && !strstr(text,"笔记"));
                assert(reader_qimen_symbol_title(&q,p,(reader_qimen_symbol_t)symbol,text,96));
            }
            assert(reader_qimen_cell_text(&q,p,false,text,512));assert(reader_qimen_summary_text(&q,p,text,128));
            assert(reader_qimen_pillars_text(&q,text,128));
            for(unsigned column=0;column<4;column++)assert(reader_qimen_pillar_column_text(&q,column,text,96));
            for(unsigned field=0;field<QIMEN_CARD_COUNT;field++)assert(reader_qimen_card_text(&q,p,(reader_qimen_card_field_t)field,text,128));
        }
    }
    assert(reader_qimen_symbol_text(&q,5,QIMEN_SYMBOL_HEAVEN,text,sizeof(text)) && strstr(text,"中宫不判长生与刑墓"));
    assert(reader_qimen_symbol_text(&q,5,QIMEN_SYMBOL_DOOR,text,sizeof(text)) && strstr(text,"中宫不判门迫"));
    assert(reader_qimen_symbol_text(&q,5,QIMEN_SYMBOL_GOD,text,sizeof(text)) && strstr(text,"没有独立八神"));
    q.valid=false;assert(!reader_qimen_symbol_text(&q,8,QIMEN_SYMBOL_PALACE,text,sizeof(text)) && !text[0]);
}
static void palace_card_and_notes(void) {
    reader_qimen_t q;assert(reader_qimen_cast(1792645200LL,&q));char text[128];
    assert(reader_qimen_pillar_column_text(&q,0,text,sizeof(text)));
    assert(!strcmp(text,"#c93030 丙#\n#c93030 午#"));
    assert(!reader_qimen_pillar_column_text(&q,4,text,sizeof(text))&&!text[0]);
    assert(reader_qimen_card_text(&q,9,QIMEN_CARD_GOD,text,sizeof(text)));assert(!strcmp(text,"六合"));
    assert(reader_qimen_card_text(&q,9,QIMEN_CARD_STAR,text,sizeof(text)));assert(strstr(text,"天任"));
    assert(reader_qimen_card_text(&q,9,QIMEN_CARD_DOOR,text,sizeof(text)));assert(strstr(text,"杜门"));
    assert(reader_qimen_card_text(&q,9,QIMEN_CARD_HEAVEN,text,sizeof(text)));assert(strstr(text,"己"));
    assert(reader_qimen_card_text(&q,9,QIMEN_CARD_EARTH,text,sizeof(text)));assert(strstr(text,"戊"));
    assert(reader_qimen_card_text(&q,8,QIMEN_CARD_HEAVEN_GROWTH,text,sizeof(text)));assert(!strcmp(text,"丑墓/寅绝"));
    assert(reader_qimen_card_text(&q,8,QIMEN_CARD_HEAVEN_STRENGTH,text,sizeof(text)));assert(!strcmp(text,"月相 日死 时相"));
    assert(reader_qimen_card_text(&q,9,QIMEN_CARD_PALACE,text,sizeof(text)));assert(!strcmp(text,"离九宫"));
    assert(reader_qimen_cell_text(&q,4,false,text,sizeof(text)));assert(!strncmp(text,"白虎\n",7));assert(!strstr(text,"巽四"));
    assert(reader_qimen_card_text(&q,5,QIMEN_CARD_HEAVEN_GROWTH,text,sizeof(text)));assert(!strcmp(text,"无宫支"));
    assert(reader_qimen_card_text(&q,5,QIMEN_CARD_GOD,text,sizeof(text)));assert(!strcmp(text,"无神"));
    q.void_palace[0]=q.horse_palace=9;
    assert(reader_qimen_card_text(&q,9,QIMEN_CARD_MARKERS,text,sizeof(text)));
    assert(strstr(text,"#5e6aa0 空亡#")&&strstr(text,"#007f79 驿马#"));
    q.star[9]=2;q.ground[5]=8;
    assert(reader_qimen_card_text(&q,9,QIMEN_CARD_HEAVEN,text,sizeof(text)));assert(strstr(text,"壬#")&&strstr(text,"己#"));
    assert(reader_qimen_card_text(&q,9,QIMEN_CARD_HEAVEN_STRENGTH,text,sizeof(text)));assert(!strcmp(text,"壬月死日囚时死 己月旺日相时旺"));
    assert(!reader_qimen_card_text(&q,0,QIMEN_CARD_GOD,text,sizeof(text))&&!text[0]);
    assert(!reader_qimen_card_text(&q,1,QIMEN_CARD_COUNT,text,sizeof(text))&&!text[0]);
    assert(!reader_qimen_card_text(&q,1,QIMEN_CARD_GOD,text,1)&&!text[0]);
    for(unsigned i=0;i<10;i++)assert(strlen(reader_qimen_note(QIMEN_NOTE_STEM,i))>20);
    for(unsigned i=1;i<10;i++) {
        assert(strlen(reader_qimen_note(QIMEN_NOTE_STAR,i))>20);
        assert(strlen(reader_qimen_note(QIMEN_NOTE_DOOR,i))>20);
        assert(strlen(reader_qimen_note(QIMEN_NOTE_PALACE,i))>20);
    }
    for(unsigned i=0;i<9;i++)assert(strlen(reader_qimen_note(QIMEN_NOTE_GOD,i))>20);
    assert(!strcmp(reader_qimen_note(QIMEN_NOTE_STEM,10),""));
    assert(!strcmp(reader_qimen_note((reader_qimen_note_kind_t)-1,0),""));
    assert(!strcmp(reader_qimen_note(QIMEN_NOTE_STAR,0),""));
}
static void adjacent_charts(void) {
    reader_qimen_t q,original,expected;
    assert(reader_qimen_cast(1792645297LL,&q));original=q;
    assert(reader_qimen_step(&q,1)&&q.epoch_utc==1792652497LL&&q.minute==1&&q.second==37);
    assert(reader_qimen_cast(1792652497LL,&expected));assert(!memcmp(&q,&expected,sizeof(q)));
    assert(reader_qimen_step(&q,-1)&&!memcmp(&q,&original,sizeof(q)));
    assert(!reader_qimen_step(&q,0)&&!memcmp(&q,&original,sizeof(q)));
    assert(reader_qimen_cast(1792681297LL,&q));
    assert(reader_qimen_step(&q,1)&&q.day==23&&q.hour==1&&q.minute==1&&q.second==37);
    assert(reader_qimen_cast(-2209017600LL,&q));original=q;
    assert(!reader_qimen_step(&q,-1)&&!memcmp(&q,&original,sizeof(q)));
    assert(reader_qimen_step(&q,1));
    assert(reader_qimen_cast(4102415999LL,&q));original=q;
    assert(!reader_qimen_step(&q,1)&&!memcmp(&q,&original,sizeof(q)));
    assert(reader_qimen_step(&q,-1));
    q.valid=false;original=q;assert(!reader_qimen_step(&q,1)&&!memcmp(&q,&original,sizeof(q)));
    q.valid=true;q.epoch_utc=INT64_MAX;original=q;assert(!reader_qimen_step(&q,1)&&!memcmp(&q,&original,sizeof(q)));
    q.epoch_utc=INT64_MIN;original=q;assert(!reader_qimen_step(&q,-1)&&!memcmp(&q,&original,sizeof(q)));
    assert(!reader_qimen_step(NULL,1));
}
int main(void) { adjacent_charts();screenshot_examples();reference_image_plate();exhaustive_status();doors();month_strength();summary_and_pillars();rich_text();palace_card_and_notes();puts("Qimen: adjacent casts, date/overflow bounds, three-period strength, palace tags, colors and notes PASS"); }
