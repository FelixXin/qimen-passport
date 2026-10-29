#include "qimen_cover.h"
#include <string.h>
#include <limits.h>
#define MAGIC UINT32_C(0x51434f56)
typedef struct {uint32_t magic,version,generation,length,crc,check;} header_t;
uint32_t qimen_cover_crc(uint32_t crc,const void *data,size_t size){
 const uint8_t *p=data;crc=~crc;
 for(size_t i=0;i<size;i++){crc^=p[i];for(unsigned j=0;j<8;j++)crc=(crc>>1)^((crc&1)?UINT32_C(0xedb88320):0);}
 return ~crc;
}
static bool inspect(qimen_cover_t *c,int bank,header_t *h){
 size_t base=(size_t)bank*QIMEN_COVER_BANK;
 if(!c->io.read(c->io.context,base,h,sizeof(*h))||h->magic!=MAGIC||h->version!=1||!h->generation||
    (h->length!=0&&h->length!=QIMEN_COVER_BYTES)||h->check!=qimen_cover_crc(0,h,offsetof(header_t,check)))return false;
 uint8_t block[256];uint32_t crc=0;
 for(size_t i=0;i<h->length;i+=sizeof(block)){size_t n=h->length-i;if(n>sizeof(block))n=sizeof(block);if(!c->io.read(c->io.context,base+QIMEN_COVER_DATA+i,block,n))return false;crc=qimen_cover_crc(crc,block,n);}
 return crc==h->crc;
}
bool qimen_cover_mount(qimen_cover_t *c,const qimen_cover_io_t *io){
 if(!c||!io||!io->read||!io->write||!io->erase)return false;
 memset(c,0,sizeof(*c));c->io=*io;c->active=c->target=-1;
 for(int i=0;i<2;i++){header_t h;if(inspect(c,i,&h)&&h.generation>c->generation){c->active=i;c->generation=h.generation;c->length=h.length;c->crc=h.crc;}}
 return true;
}
static bool prepare(qimen_cover_t *c){
 if(!c||c->uploading||c->generation==UINT32_MAX)return false;
 c->target=c->active==0?1:0;
 if(!c->io.erase(c->io.context,(size_t)c->target*QIMEN_COVER_BANK,QIMEN_COVER_BANK))return false;
 c->received=0;c->running_crc=0;c->uploading=true;return true;
}
bool qimen_cover_begin(qimen_cover_t *c,size_t size,uint32_t crc){
 if(size!=QIMEN_COVER_BYTES||!prepare(c))return false;
 c->expected_crc=crc;return true;
}
bool qimen_cover_write(qimen_cover_t *c,size_t offset,const void *bytes,size_t size){
 if(!c||!c->uploading||!bytes||!size||offset!=c->received||size>QIMEN_COVER_BYTES-c->received)return false;
 if(!c->io.write(c->io.context,(size_t)c->target*QIMEN_COVER_BANK+QIMEN_COVER_DATA+offset,bytes,size)){qimen_cover_abort(c);return false;}
 c->running_crc=qimen_cover_crc(c->running_crc,bytes,size);c->received+=size;return true;
}
void qimen_cover_abort(qimen_cover_t *c){if(c){c->uploading=false;c->target=-1;c->received=0;}}
static bool commit(qimen_cover_t *c,uint32_t length,uint32_t crc){
 header_t h={MAGIC,1,c->generation+1,length,crc,0};h.check=qimen_cover_crc(0,&h,offsetof(header_t,check));
 int target=c->target;
 if(!c->io.write(c->io.context,(size_t)target*QIMEN_COVER_BANK,&h,sizeof(h))){qimen_cover_abort(c);return false;}
 header_t actual;if(!inspect(c,target,&actual)){qimen_cover_abort(c);return false;}
 c->active=target;c->generation=h.generation;c->length=length;c->crc=crc;qimen_cover_abort(c);return true;
}
bool qimen_cover_finish(qimen_cover_t *c){
 if(!c||!c->uploading)return false;
 if(c->received!=QIMEN_COVER_BYTES||c->running_crc!=c->expected_crc){qimen_cover_abort(c);return false;}
 return commit(c,QIMEN_COVER_BYTES,c->running_crc);
}
bool qimen_cover_reset(qimen_cover_t *c){if(!prepare(c))return false;return commit(c,0,0);}
size_t qimen_cover_offset(const qimen_cover_t *c){return c&&c->active>=0&&c->length==QIMEN_COVER_BYTES?(size_t)c->active*QIMEN_COVER_BANK+QIMEN_COVER_DATA:0;}
bool qimen_cover_name_valid(const char *name){
 if(!name||strlen(name)>=QIMEN_NAME_BYTES)return false;
 const unsigned char *p=(const unsigned char*)name;unsigned count=0;
 while(*p){uint32_t cp=*p++;unsigned more=0;uint32_t min=0;
  if(cp>=0x80){if(cp>=0xc2&&cp<=0xdf){more=1;cp&=31;min=0x80;}else if(cp>=0xe0&&cp<=0xef){more=2;cp&=15;min=0x800;}else if(cp>=0xf0&&cp<=0xf4){more=3;cp&=7;min=0x10000;}else return false;
   while(more--){if((*p&0xc0)!=0x80)return false;cp=(cp<<6)|(*p++&63);}if(cp<min||cp>0x10ffff||(cp>=0xd800&&cp<=0xdfff))return false;}
  if(cp<32||cp==127||(cp>=0x80&&cp<=0x9f)||++count>8)return false;
 }
 return true;
}
