
#ifndef memtoolARM7_h
#define memtoolARM7_h

#include <NDS.h>

#ifdef __cplusplus
extern "C" {
#endif

void* safemalloc(int size);
//void safefree(void *ptr);

#define safefree(ptr) { if(ptr!=NULL) free(ptr); }

//void MemSet16CPU(vu16 v,void *dst,u32 len);
//void MemSet32CPU(u32 v,void *dst,u32 len);
void MemCopy16DMA3(void *src,void *dst,u32 len);
//void MemCopy32DMA3(void *src,void *dst,u32 len);
void MemSet16DMA3(u16 v,void *dst,u32 len);
//void MemSet32DMA3(u32 v,void *dst,u32 len);

#ifdef __cplusplus
}
#endif

#endif

