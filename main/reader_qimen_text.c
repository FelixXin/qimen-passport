#include "reader_qimen_text.h"
#include "reader_qimen_status.h"
#include "reader_qimen_notes.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct { char *text; size_t length, capacity; bool ok; } text_buffer_t;
static void append(text_buffer_t *b, const char *format, ...) {
    if (!b->ok) return;
    va_list args;va_start(args,format);
    int length=vsnprintf(b->text+b->length,b->capacity-b->length,format,args);va_end(args);
    if (length < 0 || (size_t)length >= b->capacity-b->length) { b->ok=false;b->text[0]=0;return; }
    b->length += (size_t)length;
}
static text_buffer_t buffer(char *out, size_t capacity) {
    if (out && capacity) out[0]=0;
    return (text_buffer_t){.text=out,.capacity=capacity,.ok=out && capacity};
}
/* Normal/符使/入墓/击刑/刑墓. A brighter variant preserves contrast on selection. */
static const char *color(unsigned flags, bool selected) {
    if (flags == (QIMEN_TOMB|QIMEN_PUNISHMENT)) return selected ? "67cfff" : "007ba5";
    if (flags & QIMEN_PUNISHMENT) return selected ? "e4a0ff" : "9c36bf";
    if (flags & QIMEN_TOMB) return selected ? "ffac5a" : "c45b00";
    return selected ? "f6f4ed" : "252621";
}
static const char *chief_color(bool selected) { return selected ? "42d6cb" : "007f79"; }
static const char *forced_color(bool selected) { return selected ? "ff8c83" : "c93030"; }
static const char *flags_name(unsigned flags) {
    if (flags == (QIMEN_TOMB|QIMEN_PUNISHMENT)) return "刑墓";
    if (flags & QIMEN_PUNISHMENT) return "击刑";
    if (flags & QIMEN_TOMB) return "入墓";
    return "无刑墓";
}
static bool valid(const reader_qimen_t *q, unsigned palace) { return q && q->valid && palace >= 1 && palace <= 9; }
static unsigned stem_flags(unsigned stem, unsigned palace) {
    reader_qimen_stem_status_t s;
    return reader_qimen_stem_status(stem,palace,&s) ? s.flags : 0;
}
static void colored_stem(text_buffer_t *b, unsigned stem, unsigned palace, bool selected) {
    append(b,"#%s %s#",color(stem_flags(stem,palace),selected),reader_qimen_stem(stem));
}
static void strengths(text_buffer_t *b, const reader_qimen_t *q, unsigned stem, bool compact) {
    append(b,compact?"月%s日%s时%s":"月%s 日%s 时%s",
        reader_qimen_branch_strength(stem,q->pillars[1]%12),
        reader_qimen_branch_strength(stem,q->pillars[2]%12),
        reader_qimen_branch_strength(stem,q->pillars[3]%12));
}
static void stem_row(text_buffer_t *b, const reader_qimen_t *q, const char *name, unsigned stem, unsigned palace) {
    reader_qimen_stem_status_t s;
    if (!reader_qimen_stem_status(stem,palace,&s)) { b->ok=false;b->text[0]=0;return; }
    append(b,"#%s %s %s%s [%s]#\n",color(s.flags,false),name,reader_qimen_stem(stem),reader_qimen_element_name(reader_qimen_stem_element(stem)),s.count ? flags_name(s.flags) : "无支");
    if (!s.count) append(b,"中宫不判长生与刑墓\n");
    else {
        append(b,"长生：");
        for (unsigned i=0;i<s.count;i++) append(b,"%s%s%s",i?" / ":"",reader_qimen_branch(s.branch[i]),reader_qimen_growth(s.growth[i]));
        append(b,"\n");
    }
    strengths(b,q,stem,false);
    append(b,"\n参照支：%s/%s/%s\n",reader_qimen_branch(q->pillars[1]%12),reader_qimen_branch(q->pillars[2]%12),reader_qimen_branch(q->pillars[3]%12));
    append(b,"%s\n\n",reader_qimen_note(QIMEN_NOTE_STEM,stem));
}
const char *reader_qimen_symbol_name(reader_qimen_symbol_t symbol) {
    static const char *names[]={"宫","天盘干","地盘干","暗干","门","星","神"};
    return (unsigned)symbol<QIMEN_SYMBOL_COUNT?names[symbol]:"";
}
bool reader_qimen_symbol_title(const reader_qimen_t *q,unsigned p,reader_qimen_symbol_t symbol,char *out,size_t capacity) {
    text_buffer_t b=buffer(out,capacity);if(!b.ok || !valid(q,p) || (unsigned)symbol>=QIMEN_SYMBOL_COUNT)return false;
    const char *value="";
    switch(symbol) {
        case QIMEN_SYMBOL_PALACE:value=reader_qimen_palace(p);break;
        case QIMEN_SYMBOL_HEAVEN:value=reader_qimen_stem(q->heaven[p]);break;
        case QIMEN_SYMBOL_EARTH:value=reader_qimen_stem(q->ground[p]);break;
        case QIMEN_SYMBOL_HIDDEN:value=reader_qimen_stem(q->hidden[p]);break;
        case QIMEN_SYMBOL_DOOR:value=p==5?"无门":reader_qimen_door(q->door[p]);break;
        case QIMEN_SYMBOL_STAR:value=p==5?"天禽":reader_qimen_star(q->star[p]);break;
        case QIMEN_SYMBOL_GOD:value=p==5?"无神":reader_qimen_god(q->god[p]);break;
        default:return false;
    }
    append(&b,"%s · %s%s",reader_qimen_symbol_name(symbol),value,symbol==QIMEN_SYMBOL_PALACE?"宫":"");return b.ok;
}
static void palace_markers(text_buffer_t *b,const reader_qimen_t *q,unsigned p) {
    append(b,"%s\n%s\n",p==q->void_palace[0]||p==q->void_palace[1]?"#5e6aa0 本宫空亡#":"本宫非空亡",p==q->horse_palace?"#007f79 本宫有驿马#":"本宫无驿马");
}
bool reader_qimen_symbol_text(const reader_qimen_t *q,unsigned p,reader_qimen_symbol_t symbol,char *out,size_t capacity) {
    text_buffer_t b=buffer(out,capacity);if(!b.ok || !valid(q,p) || (unsigned)symbol>=QIMEN_SYMBOL_COUNT)return false;
    bool forced=reader_qimen_door_forced(q->door[p],p);
    append(&b,"落宫 %s宫 · %s\n",reader_qimen_palace(p),reader_qimen_element_name(reader_qimen_palace_element(p)));
    switch(symbol) {
        case QIMEN_SYMBOL_PALACE:
            append(&b,"%s\n\n",reader_qimen_note(QIMEN_NOTE_PALACE,p));palace_markers(&b,q,p);
            append(&b,"\n状态含义\n#c45b00 入墓#：天干落入墓支，取收藏、收敛之象。\n#9c36bf 击刑#：六仪落刑位，取冲突、破坏之象。\n#c93030 门迫#：门五行克宫五行，取阻碍、难推进之象。\n#5e6aa0 空亡#：本盘时旬空支落宫。\n#007f79 驿马#：按时支取马支落宫，取动态、外出、变化之象。\n空亡与驿马分别显示，不自动取消空亡。\n\n颜色说明\n#007f79 符使# / #c45b00 入墓#\n#9c36bf 击刑# / #c93030 门迫#\n#007ba5 刑+墓#\n#5e6aa0 空亡# / #007f79 驿马#");
            break;
        case QIMEN_SYMBOL_HEAVEN:case QIMEN_SYMBOL_EARTH:case QIMEN_SYMBOL_HIDDEN: {
            bool heaven=symbol==QIMEN_SYMBOL_HEAVEN,hidden=symbol==QIMEN_SYMBOL_HIDDEN;
            stem_row(&b,q,hidden?"暗干":(heaven?"天盘":"地盘"),hidden?q->hidden[p]:(heaven?q->heaven[p]:q->ground[p]),p);
            if(heaven && p!=5 && q->star[p]==2)stem_row(&b,q,"天寄",q->ground[5],p);
            if(!heaven && !hidden && p==2)stem_row(&b,q,"地寄",q->ground[5],p);
            append(&b,"能量参照\n月日时各取本盘柱支，按五行作旺相休囚死。\n辰戌丑未以土为参照，不细分土旺十八日。\n十二长生按所落宫支计算，中宫不判长生与刑墓。");
            break;
        }
        case QIMEN_SYMBOL_DOOR:
            append(&b,"#%s %s%s#\n",forced?forced_color(false):(q->chief_door_palace==p?chief_color(false):color(0,false)),p==5?"无门":reader_qimen_door(q->door[p]),q->chief_door_palace==p?" [值使]":"");
            append(&b,p==5?"中宫不判门迫\n":(forced?"#c93030 门迫：有#\n":"#c93030 无门迫#\n"));
            if(forced)append(&b,"门%s克宫%s\n",reader_qimen_element_name(reader_qimen_palace_element(q->door[p])),reader_qimen_element_name(reader_qimen_palace_element(p)));
            append(&b,"\n%s",reader_qimen_note(QIMEN_NOTE_DOOR,p==5?5:q->door[p]));break;
        case QIMEN_SYMBOL_STAR:
            append(&b,"#%s %s%s#\n\n%s",q->chief_star_palace==p?chief_color(false):color(0,false),p==5?"天禽":reader_qimen_star(q->star[p]),q->chief_star_palace==p?" [值符]":"",reader_qimen_note(QIMEN_NOTE_STAR,p==5?5:q->star[p]));
            if(p!=5 && q->star[p]==2) {
                append(&b,"\n\n%s",reader_qimen_note(QIMEN_NOTE_STAR,5));
            }
            break;
        case QIMEN_SYMBOL_GOD:
            append(&b,"%s\n\n%s",p==5?"无神":reader_qimen_god(q->god[p]),reader_qimen_note(QIMEN_NOTE_GOD,p==5?8:q->god[p]));break;
        default:return false;
    }
    return b.ok;
}
bool reader_qimen_cell_text(const reader_qimen_t *q, unsigned p, bool selected, char *out, size_t capacity) {
    text_buffer_t b=buffer(out,capacity);if (!b.ok || !valid(q,p)) return false;
    const char *normal=color(0,selected);
    append(&b,"%s\n#%s %s# ",reader_qimen_god(q->god[p]),q->chief_star_palace==p?chief_color(selected):normal,reader_qimen_star(q->star[p]));
    if (p!=5 && q->star[p]==2) colored_stem(&b,q->ground[5],p,selected);
    colored_stem(&b,q->heaven[p],p,selected);
    append(&b,"\n#%s %s# ",reader_qimen_door_forced(q->door[p],p)?forced_color(selected):(q->chief_door_palace==p?chief_color(selected):normal),reader_qimen_door(q->door[p]));
    if (p==2) colored_stem(&b,q->ground[5],p,selected);
    colored_stem(&b,q->ground[p],p,selected);
    return b.ok;
}
bool reader_qimen_summary_text(const reader_qimen_t *q, unsigned p, char *out, size_t capacity) {
    text_buffer_t b=buffer(out,capacity);if (!b.ok || !valid(q,p)) return false;
    unsigned flags=stem_flags(q->heaven[p],p)|stem_flags(q->ground[p],p)|stem_flags(q->hidden[p],p);
    if ((p!=5 && q->star[p]==2)||p==2) flags|=stem_flags(q->ground[5],p);
    /* Fixed category colors; explicit negatives distinguish absent states. */
    append(&b,"#%s %s# #%s %s# #%s %s# #%s %s# #%s %s#",
        p==5?"74746c":"c45b00",p==5?"无支":(flags&QIMEN_TOMB?"入墓":"无墓"),
        "9c36bf",flags&QIMEN_PUNISHMENT?"击刑":"无刑",
        "c93030",reader_qimen_door_forced(q->door[p],p)?"门迫":"无迫",
        p==q->void_palace[0]||p==q->void_palace[1]?"5e6aa0":"74746c",p==q->void_palace[0]||p==q->void_palace[1]?"空亡":"非空",
        p==q->horse_palace?"007f79":"74746c",p==q->horse_palace?"驿马":"无马");
    return b.ok;
}
bool reader_qimen_pillars_text(const reader_qimen_t *q, char *out, size_t capacity) {
    text_buffer_t b=buffer(out,capacity);if (!b.ok || !q || !q->valid) return false;
    char pillars[4][7];for(unsigned i=0;i<4;i++) reader_qimen_pillar(q->pillars[i],pillars[i]);
    append(&b,"本盘四柱 · %s遁%u局\n年%s 月%s 日%s 时%s",q->yin?"阴":"阳",q->ju,pillars[0],pillars[1],pillars[2],pillars[3]);
    return b.ok;
}
static const char *element_color(reader_qimen_element_t element) {
    static const char *colors[]={"22864c","c93030","95632f","a47b00","246eac","74746c"};
    return colors[(unsigned)element<=QIMEN_NO_ELEMENT?element:QIMEN_NO_ELEMENT];
}
bool reader_qimen_pillar_column_text(const reader_qimen_t *q, unsigned column, char *out, size_t capacity) {
    text_buffer_t b=buffer(out,capacity);if (!b.ok || !q || !q->valid || column>=4) return false;
    unsigned stem=q->pillars[column]%10,branch=q->pillars[column]%12;
    append(&b,"#%s %s#\n#%s %s#",element_color(reader_qimen_stem_element(stem)),reader_qimen_stem(stem),element_color(reader_qimen_month_element(branch)),reader_qimen_branch(branch));
    return b.ok;
}
static void growth_short(text_buffer_t *b, unsigned stem, unsigned p) {
    reader_qimen_stem_status_t s;reader_qimen_stem_status(stem,p,&s);
    if (!s.count) { append(b,"无宫支");return; }
    for(unsigned i=0;i<s.count;i++) append(b,"%s%s%s",i?"/":"",reader_qimen_branch(s.branch[i]),reader_qimen_growth(s.growth[i]));
}
bool reader_qimen_card_text(const reader_qimen_t *q, unsigned p, reader_qimen_card_field_t field, char *out, size_t capacity) {
    text_buffer_t b=buffer(out,capacity);if (!b.ok || !valid(q,p) || (unsigned)field>=QIMEN_CARD_COUNT) return false;
    bool carry_heaven=p!=5 && q->star[p]==2;
    switch(field) {
        case QIMEN_CARD_GOD:append(&b,"%s",p==5?"无神":reader_qimen_god(q->god[p]));break;
        case QIMEN_CARD_STAR:append(&b,"#%s %s#",q->chief_star_palace==p?chief_color(false):color(0,false),p==5?"天禽":reader_qimen_star(q->star[p]));break;
        case QIMEN_CARD_DOOR:append(&b,"#%s %s#",reader_qimen_door_forced(q->door[p],p)?forced_color(false):(q->chief_door_palace==p?chief_color(false):color(0,false)),p==5?"无门":reader_qimen_door(q->door[p]));break;
        case QIMEN_CARD_HIDDEN:append(&b,"暗%s",reader_qimen_stem(q->hidden[p]));break;
        case QIMEN_CARD_HEAVEN:if(carry_heaven)colored_stem(&b,q->ground[5],p,false);colored_stem(&b,q->heaven[p],p,false);break;
        case QIMEN_CARD_EARTH:if(p==2)colored_stem(&b,q->ground[5],p,false);colored_stem(&b,q->ground[p],p,false);break;
        case QIMEN_CARD_HEAVEN_GROWTH:if(carry_heaven)append(&b,"%s",reader_qimen_stem(q->heaven[p]));growth_short(&b,q->heaven[p],p);break;
        case QIMEN_CARD_EARTH_GROWTH:if(p==2)append(&b,"%s",reader_qimen_stem(q->ground[p]));growth_short(&b,q->ground[p],p);break;
        case QIMEN_CARD_HEAVEN_STRENGTH:case QIMEN_CARD_EARTH_STRENGTH: {
            unsigned stem=field==QIMEN_CARD_HEAVEN_STRENGTH?q->heaven[p]:q->ground[p];
            bool carried=field==QIMEN_CARD_HEAVEN_STRENGTH?carry_heaven:p==2;
            if(carried) {
                append(&b,"%s",reader_qimen_stem(q->ground[5]));strengths(&b,q,q->ground[5],true);
                append(&b," %s",reader_qimen_stem(stem));strengths(&b,q,stem,true);
            } else strengths(&b,q,stem,false);
            break;
        }
        case QIMEN_CARD_PALACE:append(&b,"%s宫",reader_qimen_palace(p));break;
        case QIMEN_CARD_MARKERS:
            if(p==q->void_palace[0]||p==q->void_palace[1])append(&b,"#5e6aa0 空亡#");
            if(p==q->horse_palace)append(&b," #007f79 驿马#");
            break;
        default:break;
    }
    return b.ok;
}
