
#ifndef memtool_h
#define memtool_h

#ifdef __cplusplus
extern "C" {
#endif

void DC_FlushRangeOverrun(const void *v,u32 size);

void MemCopy8CPU(const void *src,void *dst,u32 len);
void MemCopy16CPU(const void *src,void *dst,u32 len);
void MemCopy32CPU(const void *src,void *dst,u32 len);
void MemSet16CPU(const vu16 v,void *dst,u32 len);
void MemSet32CPU(const u32 v,void *dst,u32 len);
void MemCopy16DMA3(const void *src,void *dst,u32 len);
void MemCopy32DMA3(const void *src,void *dst,u32 len);
void MemSet16DMA3(const vu16 v,void *dst,u32 len);
void MemSet32DMA3(const u32 v,void *dst,u32 len);
void MemSet8DMA3(const u8 v,void *dst,u32 len);

void MemCopy16DMA2(const void *src,void *dst,u32 len);
void MemSet16DMA2(const u16 v,void *dst,u32 len);

void MemCopy32swi256bit(const void *src,void *dst,u32 len);

void *safemalloc(const int size);
void safefree(const void *ptr);
bool testmalloc(int size);
void PrintFreeMem(void);

#ifdef __cplusplus
}
#endif

#endif

