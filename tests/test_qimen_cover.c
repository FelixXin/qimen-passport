#include "qimen_cover.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned char flash[QIMEN_COVER_BANK*2],image[QIMEN_COVER_BYTES];
static unsigned operation,fail_at;
static bool rd(void *x,size_t at,void *out,size_t n){(void)x;assert(at+n<=sizeof(flash));memcpy(out,flash+at,n);return true;}
static bool wr(void *x,size_t at,const void *in,size_t n){(void)x;assert(at+n<=sizeof(flash));if(++operation==fail_at)return false;const unsigned char *p=in;for(size_t i=0;i<n;i++){assert((flash[at+i]&p[i])==p[i]);flash[at+i]&=p[i];}return true;}
static bool er(void *x,size_t at,size_t n){(void)x;assert(at+n<=sizeof(flash));if(++operation==fail_at)return false;memset(flash+at,255,n);return true;}
static qimen_cover_io_t io={NULL,rd,wr,er};
static bool upload(qimen_cover_t *c){if(!qimen_cover_begin(c,sizeof(image),qimen_cover_crc(0,image,sizeof(image))))return false;for(size_t i=0;i<sizeof(image);i+=1024){size_t n=sizeof(image)-i;if(n>1024)n=1024;if(!qimen_cover_write(c,i,image+i,n))return false;}return qimen_cover_finish(c);}
int main(void){
 memset(flash,255,sizeof(flash));for(size_t i=0;i<sizeof(image);i++)image[i]=(unsigned char)i;qimen_cover_t c,reboot;assert(qimen_cover_mount(&c,&io)&&!qimen_cover_offset(&c));
 assert(!qimen_cover_begin(&c,7,0));assert(upload(&c));assert(c.generation==1&&qimen_cover_offset(&c)==QIMEN_COVER_DATA);unsigned steps=operation;
 assert(qimen_cover_mount(&reboot,&io)&&reboot.generation==1);unsigned char baseline[sizeof(flash)];memcpy(baseline,flash,sizeof(flash));
 for(unsigned failure=1;failure<=steps;failure++){memcpy(flash,baseline,sizeof(flash));assert(qimen_cover_mount(&c,&io));operation=0;fail_at=failure;assert(!upload(&c));assert(qimen_cover_mount(&reboot,&io)&&reboot.generation==1);}
 fail_at=0;memcpy(flash,baseline,sizeof(flash));assert(qimen_cover_mount(&c,&io));
 assert(qimen_cover_begin(&c,sizeof(image),0));assert(!qimen_cover_begin(&c,sizeof(image),0));assert(!qimen_cover_write(&c,1,image,1));assert(!qimen_cover_finish(&c));assert(qimen_cover_mount(&reboot,&io)&&reboot.generation==1);
 assert(upload(&c));assert(c.generation==2);flash[qimen_cover_offset(&c)+123]^=1;assert(qimen_cover_mount(&reboot,&io)&&reboot.generation==1);
 c=reboot;assert(qimen_cover_reset(&c));assert(qimen_cover_mount(&reboot,&io)&&reboot.length==0&&!qimen_cover_offset(&reboot));
 assert(qimen_cover_name_valid("奇门随身盘"));assert(qimen_cover_name_valid(""));assert(qimen_cover_name_valid("QIMEN"));assert(!qimen_cover_name_valid("123456789"));assert(!qimen_cover_name_valid("bad\nname"));assert(!qimen_cover_name_valid("\xc0\xaf"));assert(!qimen_cover_name_valid("\xed\xa0\x80"));
 puts("Cover: power-loss/fault at every erase/write/commit, CRC fallback, tombstone reset, bounds and UTF-8 names PASS");
}
