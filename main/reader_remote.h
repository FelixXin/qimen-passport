#pragma once
#include "reader_local.h"
typedef struct { uint32_t token; char line[READER_LOCAL_MAX]; } reader_remote_message_t;
void reader_remote_init(void);
bool reader_remote_submit(const char *line,char *reply,size_t capacity);
bool reader_remote_next(reader_remote_message_t *message);
void reader_remote_reply(uint32_t token,const char *reply);
