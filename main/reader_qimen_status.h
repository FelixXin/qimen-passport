#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum { QIMEN_WOOD, QIMEN_FIRE, QIMEN_EARTH, QIMEN_METAL, QIMEN_WATER, QIMEN_NO_ELEMENT } reader_qimen_element_t;
enum { QIMEN_TOMB=1, QIMEN_PUNISHMENT=2 };
typedef struct {
    uint8_t count, branch[2], growth[2], flags;
} reader_qimen_stem_status_t;
/* 阳顺阴逆十二长生; 中宫 has no branch and no growth/tomb/punishment status. */
bool reader_qimen_stem_status(unsigned stem, unsigned palace, reader_qimen_stem_status_t *status);
const char *reader_qimen_growth(unsigned stage);
reader_qimen_element_t reader_qimen_palace_element(unsigned palace);
reader_qimen_element_t reader_qimen_stem_element(unsigned stem);
const char *reader_qimen_element_name(reader_qimen_element_t element);
/* Solar-month branch element; 辰戌丑未 use earth for the whole month.
 * This convention does not subdivide earth's final eighteen days. */
reader_qimen_element_t reader_qimen_month_element(unsigned month_branch);
const char *reader_qimen_month_strength(unsigned stem, unsigned month_branch);
/* Same five-element comparison, with an explicit month/day/hour pillar branch.
 * A display reference, not a numeric energy score or a school-independent verdict. */
const char *reader_qimen_branch_strength(unsigned stem, unsigned branch);
/* 门迫 = 门五行克落宫五行. Center has no gate. Door uses its original palace ID. */
bool reader_qimen_door_forced(unsigned door, unsigned palace);
