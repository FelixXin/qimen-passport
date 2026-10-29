#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define READER_LOCAL_MAX 384
typedef enum { LOCAL_HELLO,LOCAL_GET,LOCAL_TIME,LOCAL_AUTH,LOCAL_WIFI,LOCAL_NET,LOCAL_AP,LOCAL_RADIO,LOCAL_CAST,LOCAL_STATS,LOCAL_NAME,LOCAL_COVER_INFO,LOCAL_COVER_RESET } reader_local_op_t;
typedef struct {reader_local_op_t op;int64_t epoch;uint32_t number;char ssid[33],password[65];} reader_local_command_t;
typedef struct {char line[READER_LOCAL_MAX];size_t length;bool dropping;} reader_local_frame_t;
int reader_local_feed(reader_local_frame_t *frame,uint8_t byte);
bool reader_local_parse(const char *line,reader_local_command_t *command);
