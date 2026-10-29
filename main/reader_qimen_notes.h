#pragma once
/* Short, curated lesson notes. Indices match reader_qimen's arrays, not display order. */
typedef enum { QIMEN_NOTE_STEM, QIMEN_NOTE_STAR, QIMEN_NOTE_DOOR, QIMEN_NOTE_GOD, QIMEN_NOTE_PALACE } reader_qimen_note_kind_t;
const char *reader_qimen_note(reader_qimen_note_kind_t kind, unsigned index);
