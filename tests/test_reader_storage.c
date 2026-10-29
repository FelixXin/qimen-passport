#include "reader_storage.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static reader_network_credentials_t stored;
static bool has_record, fail_write, fail_commit;
static unsigned blob_writes;
esp_err_t nvs_open(const char *name,int mode,nvs_handle_t *handle) {assert(!strcmp(name,"qimen_net"));(void)mode;*handle=1;return ESP_OK;}
void nvs_close(nvs_handle_t handle) {(void)handle;}
esp_err_t nvs_get_blob(nvs_handle_t handle,const char *key,void *out,size_t *size) {
    (void)handle;assert(!strcmp(key,"station"));
    if(!has_record)return ESP_ERR_NVS_NOT_FOUND;
    assert(*size>=sizeof(stored));memcpy(out,&stored,sizeof(stored));*size=sizeof(stored);return ESP_OK;
}
esp_err_t nvs_get_str(nvs_handle_t handle,const char *key,char *out,size_t *size) {
    (void)handle;const char *value=!strcmp(key,"ssid")?"old-network":"old-password";
    assert(*size>strlen(value));strcpy(out,value);return ESP_OK;
}
esp_err_t nvs_set_blob(nvs_handle_t handle,const char *key,const void *data,size_t size) {
    (void)handle;blob_writes++;if(fail_write)return ESP_ERR_NVS_NOT_ENOUGH_SPACE;
    if(!strcmp(key,"station")){assert(size==sizeof(stored));memcpy(&stored,data,size);has_record=true;}
    else assert(false);
    return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t h) {(void)h;return fail_commit?ESP_FAIL:ESP_OK;}
int main(void) {
    reader_network_credentials_t value;
    assert(reader_storage_network_load(&value)==ESP_OK && value.configured && !strcmp(value.ssid,"old-network"));
    assert(!blob_writes); /* Legacy reads must not silently migrate or rewrite user data. */
    assert(reader_storage_network_save("new-network","new-password")==ESP_OK);
    assert(blob_writes==1); /* Exactly one persistent record owns the pair. */
    assert(reader_storage_network_load(&value)==ESP_OK && !strcmp(value.ssid,"new-network")&&!strcmp(value.password,"new-password"));
    fail_write=true;
    assert(reader_storage_network_save("failed-network","failed-password")==ESP_ERR_NVS_NOT_ENOUGH_SPACE);
    assert(reader_storage_network_load(&value)==ESP_OK && !strcmp(value.ssid,"new-network")&&!strcmp(value.password,"new-password"));
    assert(reader_storage_network_forget()!=ESP_OK);
    fail_write=false;assert(reader_storage_network_forget()==ESP_OK);
    assert(reader_storage_network_load(&value)==ESP_OK&&!value.configured); /* No legacy resurrection. */
    stored.version=99;assert(reader_storage_network_load(&value)!=ESP_OK&&!value.configured);
    fail_commit=true;assert(reader_storage_network_save("valid-network","password123")==ESP_FAIL);
    puts("Storage: independent namespace, atomic network pair, failed write/commit, forget tombstone PASS");
}
