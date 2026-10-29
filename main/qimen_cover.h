#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define QIMEN_COVER_BYTES (240u*320u*2u)
#define QIMEN_COVER_BANK 0x40000u
#define QIMEN_COVER_DATA 0x1000u
#define QIMEN_NAME_BYTES 25
/* Fixed-size RGB565 data, two independently committed banks; no image decoder. */
typedef struct {
 void *context;
 bool (*read)(void *,size_t,void *,size_t);
 bool (*write)(void *,size_t,const void *,size_t);
 bool (*erase)(void *,size_t,size_t);
} qimen_cover_io_t;
typedef struct {
 qimen_cover_io_t io;int active,target;uint32_t generation,length,crc;
 uint32_t expected_crc,running_crc;size_t received;bool uploading;
} qimen_cover_t;
uint32_t qimen_cover_crc(uint32_t crc,const void *bytes,size_t size);
bool qimen_cover_mount(qimen_cover_t *cover,const qimen_cover_io_t *io);
bool qimen_cover_begin(qimen_cover_t *cover,size_t size,uint32_t crc);
bool qimen_cover_write(qimen_cover_t *cover,size_t offset,const void *bytes,size_t size);
bool qimen_cover_finish(qimen_cover_t *cover);
void qimen_cover_abort(qimen_cover_t *cover);
bool qimen_cover_reset(qimen_cover_t *cover);
size_t qimen_cover_offset(const qimen_cover_t *cover);
bool qimen_cover_name_valid(const char *name);

/* Built-in styles 0..3; 4 selects the retained uploaded image. */
const char *qimen_cover_style_name(unsigned style);
