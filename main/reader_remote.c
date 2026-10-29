#include "reader_remote.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include <string.h>
#include <stdio.h>
static QueueHandle_t requests,replies;
static SemaphoreHandle_t access;
static uint32_t sequence;
void reader_remote_init(void) {
    requests=xQueueCreate(1,sizeof(reader_remote_message_t));
    replies=xQueueCreate(1,sizeof(reader_remote_message_t));access=xSemaphoreCreateMutex();
}
bool reader_remote_submit(const char *line,char *reply,size_t capacity) {
    if(!requests||!replies||!access||!line||strlen(line)>=READER_LOCAL_MAX||!reply||!capacity)return false;
    if(xSemaphoreTake(access,0)!=pdTRUE)return false;
    reader_remote_message_t message={0};
    if(!++sequence)++sequence;
    message.token=sequence;strcpy(message.line,line);bool ok=xQueueSend(requests,&message,0)==pdTRUE;
    memset(&message,0,sizeof(message));int64_t deadline=esp_timer_get_time()+5000000;
    bool answered=false;
    while(ok && esp_timer_get_time()<deadline) {
        if(xQueueReceive(replies,&message,pdMS_TO_TICKS(100))==pdTRUE && message.token==sequence) {
            answered=strlen(message.line)<capacity;
            if(answered)strcpy(reply,message.line);
            break;
        }
    }
    memset(&message,0,sizeof(message));xSemaphoreGive(access);return ok&&answered;
}
bool reader_remote_next(reader_remote_message_t *message) {
    return requests && xQueueReceive(requests,message,0)==pdTRUE;
}
void reader_remote_reply(uint32_t token,const char *reply) {
    reader_remote_message_t message={.token=token};snprintf(message.line,sizeof(message.line),"%s",reply);
    if(replies)xQueueOverwrite(replies,&message);
    memset(&message,0,sizeof(message));
}
