#pragma once
#include "reader_qimen.h"
#include <stdbool.h>
#include <stddef.h>
#define READER_QIMEN_SYMBOL_CAPACITY 2048
typedef enum { QIMEN_SYMBOL_PALACE, QIMEN_SYMBOL_HEAVEN, QIMEN_SYMBOL_EARTH,
               QIMEN_SYMBOL_HIDDEN, QIMEN_SYMBOL_DOOR, QIMEN_SYMBOL_STAR, QIMEN_SYMBOL_GOD,
               QIMEN_SYMBOL_COUNT } reader_qimen_symbol_t;
typedef enum {
    QIMEN_CARD_GOD, QIMEN_CARD_STAR, QIMEN_CARD_DOOR, QIMEN_CARD_HIDDEN,
    QIMEN_CARD_HEAVEN, QIMEN_CARD_EARTH, QIMEN_CARD_HEAVEN_GROWTH,
    QIMEN_CARD_EARTH_GROWTH, QIMEN_CARD_HEAVEN_STRENGTH, QIMEN_CARD_EARTH_STRENGTH,
    QIMEN_CARD_MARKERS, QIMEN_CARD_PALACE, QIMEN_CARD_COUNT
} reader_qimen_card_field_t;
/* Rich text uses LVGL's #RRGGBB text# syntax; output only trusted chart values. */
/* Each symbol is a separate article, using only this chart snapshot. */
const char *reader_qimen_symbol_name(reader_qimen_symbol_t symbol);
bool reader_qimen_symbol_title(const reader_qimen_t *chart, unsigned palace, reader_qimen_symbol_t symbol, char *out, size_t capacity);
bool reader_qimen_symbol_text(const reader_qimen_t *chart, unsigned palace, reader_qimen_symbol_t symbol, char *out, size_t capacity);
bool reader_qimen_cell_text(const reader_qimen_t *chart, unsigned palace, bool selected, char *out, size_t capacity);
bool reader_qimen_summary_text(const reader_qimen_t *chart, unsigned palace, char *out, size_t capacity);
/* Fixed header describes the cast snapshot, not a changing wall clock. */
bool reader_qimen_pillars_text(const reader_qimen_t *chart, char *out, size_t capacity);
bool reader_qimen_pillar_column_text(const reader_qimen_t *chart, unsigned column, char *out, size_t capacity);
bool reader_qimen_card_text(const reader_qimen_t *chart, unsigned palace, reader_qimen_card_field_t field, char *out, size_t capacity);
