#include "reader_qimen_status.h"
#include <string.h>

static const char *growth_names[] = {"长生","沐浴","冠带","临官","帝旺","衰","病","死","墓","绝","胎","养"};
static const uint8_t birth[] = {11,6,2,9,2,9,5,0,8,3};
static const uint8_t punishment_palace[] = {0,0,0,0,3,2,8,9,4,4};
static const uint8_t palace_branches[10][2] = {{0,0},{0,0},{7,8},{3,3},{4,5},{0,0},{10,11},{9,9},{1,2},{6,6}};
static const reader_qimen_element_t palace_elements[] = {QIMEN_NO_ELEMENT,QIMEN_WATER,QIMEN_EARTH,QIMEN_WOOD,QIMEN_WOOD,QIMEN_EARTH,QIMEN_METAL,QIMEN_METAL,QIMEN_EARTH,QIMEN_FIRE};

const char *reader_qimen_growth(unsigned stage) { return stage < 12 ? growth_names[stage] : ""; }
reader_qimen_element_t reader_qimen_palace_element(unsigned palace) { return palace < 10 ? palace_elements[palace] : QIMEN_NO_ELEMENT; }
reader_qimen_element_t reader_qimen_stem_element(unsigned stem) { return stem < 10 ? (reader_qimen_element_t)(stem/2) : QIMEN_NO_ELEMENT; }
const char *reader_qimen_element_name(reader_qimen_element_t element) {
    static const char *names[] = {"木","火","土","金","水",""};
    return (unsigned)element <= QIMEN_NO_ELEMENT ? names[element] : "";
}
reader_qimen_element_t reader_qimen_month_element(unsigned month_branch) {
    static const reader_qimen_element_t elements[] = {
        QIMEN_WATER,QIMEN_EARTH,QIMEN_WOOD,QIMEN_WOOD,QIMEN_EARTH,QIMEN_FIRE,
        QIMEN_FIRE,QIMEN_EARTH,QIMEN_METAL,QIMEN_METAL,QIMEN_EARTH,QIMEN_WATER
    };
    return month_branch < 12 ? elements[month_branch] : QIMEN_NO_ELEMENT;
}
const char *reader_qimen_month_strength(unsigned stem, unsigned month_branch) {
    return reader_qimen_branch_strength(stem,month_branch);
}
const char *reader_qimen_branch_strength(unsigned stem, unsigned month_branch) {
    /* Relative to the ruling element: same, child, controlled, controller, parent. */
    static const char *strength[] = {"旺","相","死","囚","休"};
    if (stem >= 10 || month_branch >= 12) return "";
    return strength[(reader_qimen_stem_element(stem)+5-reader_qimen_month_element(month_branch))%5];
}
bool reader_qimen_stem_status(unsigned stem, unsigned palace, reader_qimen_stem_status_t *s) {
    if (!s) return false;
    memset(s, 0, sizeof(*s));
    if (stem >= 10 || palace < 1 || palace > 9) return false;
    if (palace == 5) return true;
    s->count = palace == 2 || palace == 4 || palace == 6 || palace == 8 ? 2 : 1;
    for (unsigned i=0;i<s->count;i++) {
        s->branch[i] = palace_branches[palace][i];
        int delta = stem%2 ? (int)birth[stem]-s->branch[i] : (int)s->branch[i]-birth[stem];
        s->growth[i] = (uint8_t)((delta+12)%12);
        if (s->growth[i] == 8) s->flags |= QIMEN_TOMB;
    }
    /* Keep adjacent stages in lifecycle order, including 养→长生 across 11→0.
       Keep the associated branches attached when reversing the two labels. */
    if (s->count == 2 && (s->growth[1]+1)%12 == s->growth[0]) {
        uint8_t g=s->growth[0], b=s->branch[0];
        s->growth[0]=s->growth[1];s->branch[0]=s->branch[1];s->growth[1]=g;s->branch[1]=b;
    }
    if (punishment_palace[stem] == palace) s->flags |= QIMEN_PUNISHMENT;
    return true;
}
bool reader_qimen_door_forced(unsigned door, unsigned palace) {
    if (door < 1 || door > 9 || door == 5 || palace < 1 || palace > 9 || palace == 5) return false;
    return (palace_elements[door]+2)%5 == palace_elements[palace];
}
