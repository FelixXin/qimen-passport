#pragma once
#include "qimen_cover.h"
typedef struct {char name[QIMEN_NAME_BYTES];const uint8_t *pixels;bool custom,available,busy;uint32_t generation,style;} qimen_cover_view_t;
void qimen_cover_store_init(void);
bool qimen_cover_store_acquire(qimen_cover_view_t *view);
void qimen_cover_store_release(bool rendered);
bool qimen_cover_store_name(const char *name);
bool qimen_cover_store_reset(void);
bool qimen_cover_store_begin(size_t size,uint32_t crc);
bool qimen_cover_store_chunk(size_t offset,const void *bytes,size_t size);
bool qimen_cover_store_finish(void);
void qimen_cover_store_abort(void);
bool qimen_cover_store_style(unsigned style);
