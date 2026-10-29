#include "reader_stubs/platform.h"
#include "../main/qimen_cover_store.c"
#include <assert.h>
static uint8_t flash[QIMEN_COVER_BANK*2],image[QIMEN_COVER_BYTES];
static esp_partition_t area={0x380000,sizeof(flash)};
static bool locked,fail_commit,style_exists;static uint32_t committed_style,pending_style;static char saved_name[25];
SemaphoreHandle_t xSemaphoreCreateMutex(void){return (void*)1;}
BaseType_t xSemaphoreTake(SemaphoreHandle_t m,unsigned wait){(void)m;(void)wait;assert(!locked);locked=true;return pdTRUE;}
BaseType_t xSemaphoreGive(SemaphoreHandle_t m){(void)m;assert(locked);locked=false;return pdTRUE;}
esp_err_t nvs_open(const char *name,int mode,nvs_handle_t *n){(void)mode;assert(!strcmp(name,"qimen_cover"));*n=1;return ESP_OK;}
void nvs_close(nvs_handle_t n){(void)n;}
esp_err_t nvs_get_str(nvs_handle_t n,const char *key,char *out,size_t *size){(void)n;assert(!strcmp(key,"name"));assert(*size>strlen(saved_name));strcpy(out,saved_name);*size=strlen(saved_name)+1;return ESP_OK;}
esp_err_t nvs_set_str(nvs_handle_t n,const char *key,const char *value){(void)n;assert(!strcmp(key,"name"));strcpy(saved_name,value);return ESP_OK;}
esp_err_t nvs_get_blob(nvs_handle_t n,const char *key,void *out,size_t *size){(void)n;assert(!strcmp(key,"style"));if(!style_exists)return ESP_ERR_NVS_NOT_FOUND;assert(*size==4);memcpy(out,&committed_style,4);return ESP_OK;}
esp_err_t nvs_set_blob(nvs_handle_t n,const char *key,const void *in,size_t size){(void)n;assert(!strcmp(key,"style")&&size==4);memcpy(&pending_style,in,4);return ESP_OK;}
esp_err_t nvs_commit(nvs_handle_t n){(void)n;if(fail_commit)return ESP_FAIL;committed_style=pending_style;style_exists=true;return ESP_OK;}
const esp_partition_t *esp_partition_find_first(int type,int sub,const char *name){assert(type==1&&sub==0x40&&!strcmp(name,"cover"));return &area;}
esp_err_t esp_partition_read(const esp_partition_t *p,size_t o,void *out,size_t n){assert(p==&area&&o+n<=sizeof(flash));memcpy(out,flash+o,n);return ESP_OK;}
esp_err_t esp_partition_write(const esp_partition_t *p,size_t o,const void *in,size_t n){assert(p==&area&&o+n<=sizeof(flash));const uint8_t *b=in;for(size_t i=0;i<n;i++){assert((flash[o+i]&b[i])==b[i]);flash[o+i]&=b[i];}return ESP_OK;}
esp_err_t esp_partition_erase_range(const esp_partition_t *p,size_t o,size_t n){assert(p==&area&&o+n<=sizeof(flash));memset(flash+o,255,n);return ESP_OK;}
esp_err_t esp_partition_mmap(const esp_partition_t *p,size_t o,size_t n,int kind,const void **out,esp_partition_mmap_handle_t *handle){assert(p==&area&&!o&&n==sizeof(flash)&&kind==0);*out=flash;*handle=1;return ESP_OK;}
void esp_partition_munmap(esp_partition_mmap_handle_t h){assert(h==1);}
static void reboot(void){assert(!locked);memset(&cover,0,sizeof(cover));mutex=NULL;partition=NULL;mapped=NULL;selected_style=rendered_generation=0;have_style=false;display_name[0]=0;qimen_cover_store_init();}
int main(void){memset(flash,255,sizeof(flash));qimen_cover_view_t v;reboot();assert(qimen_cover_store_acquire(&v));assert(v.available&&!v.custom&&v.style==0);qimen_cover_store_release(true);
 assert(!qimen_cover_store_style(4));assert(!qimen_cover_store_style(5));
 for(unsigned style=0;style<4;style++){assert(qimen_cover_store_style(style));reboot();assert(qimen_cover_store_acquire(&v));assert(v.style==style);qimen_cover_store_release(true);}
 assert(qimen_cover_store_style(0));fail_commit=true;assert(!qimen_cover_store_style(2));fail_commit=false;reboot();assert(qimen_cover_store_acquire(&v)&&v.style==0);qimen_cover_store_release(true);
 assert(qimen_cover_store_begin(sizeof(image),qimen_cover_crc(0,image,sizeof(image))));for(size_t o=0;o<sizeof(image);o+=1024){size_t n=sizeof(image)-o;if(n>1024)n=1024;assert(qimen_cover_store_chunk(o,image+o,n));}assert(qimen_cover_store_finish());
 assert(!qimen_cover_store_begin(sizeof(image),0));assert(qimen_cover_store_acquire(&v)&&v.custom&&v.style==0);const uint8_t *pixels=v.pixels;qimen_cover_store_release(true);
 assert(qimen_cover_store_style(4));assert(qimen_cover_store_style(1));reboot();assert(qimen_cover_store_acquire(&v)&&v.custom&&v.style==1&&v.pixels==pixels);qimen_cover_store_release(true);
 assert(qimen_cover_store_style(4));reboot();assert(qimen_cover_store_acquire(&v)&&v.style==4&&v.custom);qimen_cover_store_release(true);
 area.address=0x400000;reboot();assert(!qimen_cover_store_begin(sizeof(image),0));puts("Cover adapter: style persistence, commit failure, retained uploads, display acknowledgement and partition guard PASS");return 0;}
