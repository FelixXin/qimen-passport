#include "qimen_cover_store.h"
#include "esp_partition.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdio.h>
static SemaphoreHandle_t mutex;
static const esp_partition_t *partition;
static const uint8_t *mapped;
static esp_partition_mmap_handle_t mapping;
static qimen_cover_t cover;
static char display_name[QIMEN_NAME_BYTES];
static uint32_t rendered_generation;
static bool lock(void){return mutex&&xSemaphoreTake(mutex,pdMS_TO_TICKS(100))==pdTRUE;}
static void unlock(void){xSemaphoreGive(mutex);}
static bool read_data(void *p,size_t offset,void *out,size_t size){return esp_partition_read(p,offset,out,size)==ESP_OK;}
static bool write_data(void *p,size_t offset,const void *in,size_t size){return esp_partition_write(p,offset,in,size)==ESP_OK;}
static bool erase_data(void *p,size_t offset,size_t size){return esp_partition_erase_range(p,offset,size)==ESP_OK;}
void qimen_cover_store_init(void){
 mutex=xSemaphoreCreateMutex();if(!mutex)return;
 nvs_handle_t nvs;if(nvs_open("qimen_cover",NVS_READONLY,&nvs)==ESP_OK){size_t n=sizeof(display_name);if(nvs_get_str(nvs,"name",display_name,&n)!=ESP_OK||!qimen_cover_name_valid(display_name))display_name[0]=0;nvs_close(nvs);}
 partition=esp_partition_find_first(ESP_PARTITION_TYPE_DATA,0x40,"cover");
 /* The legacy wordbook begins at 0x400000. Never borrow or scan that area. */
 if(!partition||partition->address!=0x380000||partition->size!=QIMEN_COVER_BANK*2){partition=NULL;return;}
 const void *address=NULL;
 if(esp_partition_mmap(partition,0,partition->size,ESP_PARTITION_MMAP_DATA,&address,&mapping)!=ESP_OK){partition=NULL;return;}
 mapped=address;qimen_cover_io_t io={(void*)partition,read_data,write_data,erase_data};
 if(!qimen_cover_mount(&cover,&io)){esp_partition_munmap(mapping);partition=NULL;mapped=NULL;return;}
 rendered_generation=cover.generation;
}
bool qimen_cover_store_acquire(qimen_cover_view_t *v){
 if(!v||!lock())return false;
 memset(v,0,sizeof(*v));snprintf(v->name,sizeof(v->name),"%s",display_name);
 v->available=partition!=NULL;v->busy=cover.uploading;v->generation=cover.generation;
 size_t offset=qimen_cover_offset(&cover);v->pixels=offset?mapped+offset:NULL;v->custom=v->pixels!=NULL;return true;
}
void qimen_cover_store_release(bool rendered){if(rendered)rendered_generation=cover.generation;unlock();}
bool qimen_cover_store_name(const char *name){
 if(!qimen_cover_name_valid(name)||!lock())return false;
 nvs_handle_t nvs;bool ok=false;
 if(nvs_open("qimen_cover",NVS_READWRITE,&nvs)==ESP_OK){ok=nvs_set_str(nvs,"name",name)==ESP_OK&&nvs_commit(nvs)==ESP_OK;nvs_close(nvs);}
 if(ok)snprintf(display_name,sizeof(display_name),"%s",name);
 unlock();return ok;
}
bool qimen_cover_store_begin(size_t size,uint32_t crc){
 if(!lock())return false;
 /* UI must have released the previous bank before it can be erased again. */
 bool ok=partition&&rendered_generation==cover.generation&&qimen_cover_begin(&cover,size,crc);unlock();return ok;
}
bool qimen_cover_store_chunk(size_t offset,const void *bytes,size_t size){if(!lock())return false;bool ok=qimen_cover_write(&cover,offset,bytes,size);unlock();return ok;}
bool qimen_cover_store_finish(void){if(!lock())return false;bool ok=qimen_cover_finish(&cover);unlock();return ok;}
void qimen_cover_store_abort(void){if(lock()){qimen_cover_abort(&cover);unlock();}}
bool qimen_cover_store_reset(void){if(!lock())return false;bool ok=partition&&rendered_generation==cover.generation&&qimen_cover_reset(&cover);unlock();return ok;}
