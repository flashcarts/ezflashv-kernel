
#include <stdio.h>
#include <stdlib.h>

#include <NDS.h>
#include <NDS/ARM9/CP15.h>

#include "_console.h"
#include "_const.h"

#include "string.h"

#include "memtool.h"

#include "Emulator.h"

static u32 DMAFIXSRC;

#define CACHE_LINE_SIZE (32)
#define prcdiv (0x40)

#define memchk if(false)

void DC_FlushRangeOverrun(const void *v,u32 size)
{memchk{ if(v==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  static void (*lp_DC_FlushRange)(void *base, u32 size)=DC_FlushRange;
  static void (*lp_DC_InvalidateRange)(void *base, u32 size)=DC_InvalidateRange;
  
  size&=~(CACHE_LINE_SIZE-1);
  size+=CACHE_LINE_SIZE;
  
  if(v==NULL) return;
  if(size==0) return;
  
  lp_DC_FlushRange((void*)v,size);
  lp_DC_InvalidateRange((void*)v,size);
}

void MemCopy8CPU(const void *src,void *dst,u32 len)
{memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  if(len==0) return;
  
  u8 *_src=(u8*)src;
  u8 *_dst=(u8*)dst;
  
  memcpy(_dst,_src,len);
}

void MemCopy16CPU(const void *src,void *dst,u32 len)
{memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  if(len<2) return;
  
  u8 *_src=(u8*)src;
  u8 *_dst=(u8*)dst;
  
  memcpy(_dst,_src,len);
  return;
  
  while(prcdiv<=len){
    memcpy(_dst,_src,prcdiv);
    CP15_DrainWriteBuffer();
    len-=prcdiv;
    _src+=prcdiv;
    _dst+=prcdiv;
  }
  
  if(2<=len){
    memcpy(_dst,_src,len);
    CP15_DrainWriteBuffer();
  }
}

void MemCopy32CPU(const void *src,void *dst,u32 len)
{memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  MemCopy16CPU(src,dst,len);
}

void MemSet16CPU(const vu16 v,void *dst,u32 len)
{memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  if(len<2) return;
  
  u16 *_dst=(u16*)dst;
  
  for(u32 cnt=0;cnt<(len/2);cnt++){
    _dst[cnt]=v;
  }
  return;
  
  while(prcdiv<=len){
    for(u32 cnt=0;cnt<(prcdiv/2);cnt++){
      _dst[cnt]=v;
    }
    CP15_DrainWriteBuffer();
    len-=prcdiv;
    _dst+=prcdiv/2;
  }
  
  if(2<=len){
    for(u32 cnt=0;cnt<(len/2);cnt++){
      _dst[cnt]=v;
    }
    CP15_DrainWriteBuffer();
  }
}

void MemSet32CPU(const u32 v,void *dst,u32 len)
{memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  if(len<4) return;
  
  u32 *_dst=(u32*)dst;
  
  for(u32 cnt=0;cnt<(len/4);cnt++){
    _dst[cnt]=v;
  }
  return;
  
  while(prcdiv<=len){
    for(u32 cnt=0;cnt<(prcdiv/4);cnt++){
      _dst[cnt]=v;
    }
    CP15_DrainWriteBuffer();
    len-=prcdiv;
    _dst+=prcdiv/4;
  }
  
  if(4<=len){
    for(u32 cnt=0;cnt<(len/4);cnt++){
      _dst[cnt]=v;
    }
    CP15_DrainWriteBuffer();
  }
}

void MemSet8CPU(const u8 v,void *dst,u32 len)
{memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  if(len<1) return;
  
  u8 *_dst=(u8*)dst;
  
  for(u32 cnt=0;cnt<len;cnt++){
    _dst[cnt]=v;
  }
  return;
  
  while(prcdiv<=len){
    for(u32 cnt=0;cnt<prcdiv;cnt++){
      _dst[cnt]=v;
    }
    CP15_DrainWriteBuffer();
    len-=prcdiv;
    _dst+=prcdiv;
  }
  
  if(1<=len){
    for(u32 cnt=0;cnt<len;cnt++){
      _dst[cnt]=v;
    }
    CP15_DrainWriteBuffer();
  }
}

void MemCopy16DMA3(const void *src,void *dst,u32 len)
{memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA3
  {
    u16 *_src=(u16*)src;
    u16 *_dst=(u16*)dst;
    
    len/=2;
    
    for(u32 idx=0;idx<len;idx++){
      _dst[idx]=_src[idx];
    }
  }
  return;
#endif
  
  if(len<2) return;
  
  DC_FlushRangeOverrun((void*)src,len);
  DC_FlushRangeOverrun((void*)dst,len);
  
  u8 *_src=(u8*)src;
  u8 *_dst=(u8*)dst;
  
  DMA3_SRC = (uint32)_src;
  DMA3_DEST = (uint32)_dst;
  DMA3_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | (len>>1);
  while(DMA3_CR & DMA_BUSY);
  return;
  
  while(prcdiv<=len){
    DMA3_SRC = (uint32)_src;
    DMA3_DEST = (uint32)_dst;
    DMA3_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | (prcdiv>>1);
    while(DMA3_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
    len-=prcdiv;
    _src+=prcdiv;
    _dst+=prcdiv;
  }
  
  if(2<=len){
    DMA3_SRC = (uint32)_src;
    DMA3_DEST = (uint32)_dst;
    DMA3_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | (len>>1);
    while(DMA3_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
  }
}

void MemCopy32DMA3(const void *src,void *dst,u32 len)
{memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA3
  {
    u32 *_src=(u32*)src;
    u32 *_dst=(u32*)dst;
    
    len/=4;
    
    for(u32 idx=0;idx<len;idx++){
      _dst[idx]=_src[idx];
    }
  }
  return;
#endif
  
  if(len<4) return;
  
  DC_FlushRangeOverrun(src,len);
  DC_FlushRangeOverrun(dst,len);
  
  u8 *_src=(u8*)src;
  u8 *_dst=(u8*)dst;
  
  DMA3_SRC = (uint32)_src;
  DMA3_DEST = (uint32)_dst;
  DMA3_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_32_BIT | (len>>2);
  while(DMA3_CR & DMA_BUSY);
  return;
  
  while(prcdiv<=len){
    DMA3_SRC = (uint32)_src;
    DMA3_DEST = (uint32)_dst;
    DMA3_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_32_BIT | (prcdiv>>2);
    while(DMA3_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
    len-=prcdiv;
    _src+=prcdiv;
    _dst+=prcdiv;
  }
  
  if(4<=len){
    DMA3_SRC = (uint32)_src;
    DMA3_DEST = (uint32)_dst;
    DMA3_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_32_BIT | (len>>2);
    while(DMA3_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
  }
}

void MemSet16DMA3(const u16 v,void *dst,u32 len)
{memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA3
  static void (*lp_MemSet16CPU)(u16 v,void *dst,u32 len)=MemSet16CPU;
  lp_MemSet16CPU(v,dst,len);
  return;
#endif
  
  if(len<2) return;
  
  DMAFIXSRC=(vu32)v+((vu32)v<<16);
  
  DC_FlushRangeOverrun(&DMAFIXSRC,4);
  DC_FlushRangeOverrun(dst,len);
  
  u8 *_dst=(u8*)dst;
  DMA3_SRC = (uint32)&DMAFIXSRC;
  
  DMA3_DEST = (uint32)_dst;
  DMA3_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_16_BIT | (len>>1);
  while(DMA3_CR & DMA_BUSY);
  return;
  
  while(prcdiv<=len){
    DMA3_DEST = (uint32)_dst;
    DMA3_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_16_BIT | (prcdiv>>1);
    while(DMA3_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
    len-=prcdiv;
    _dst+=prcdiv;
  }
  
  if(2<=len){
    DMA3_DEST = (uint32)_dst;
    DMA3_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_16_BIT | (len>>1);
    while(DMA3_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
  }
}

void MemSet32DMA3(const u32 v,void *dst,u32 len)
{memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA3
  static void (*lp_MemSet32CPU)(u32 v,void *dst,u32 len)=MemSet32CPU;
  lp_MemSet32CPU(v,dst,len);
  return;
#endif
  
  if(len<4) return;
  
  DMAFIXSRC=(vu32)v;
  
  DC_FlushRangeOverrun(&DMAFIXSRC,4);
  DC_FlushRangeOverrun(dst,len);
  
  u8 *_dst=(u8*)dst;
  DMA3_SRC = (uint32)&DMAFIXSRC;
  
  DMA3_DEST = (uint32)_dst;
  DMA3_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_32_BIT | (len>>2);
  while(DMA3_CR & DMA_BUSY);
  return;
  
  while(prcdiv<=len){
    DMA3_DEST = (uint32)_dst;
    DMA3_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_32_BIT | (prcdiv>>2);
    while(DMA3_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
    len-=prcdiv;
    _dst+=prcdiv;
  }
  
  if(4<=len){
    DMA3_DEST = (uint32)_dst;
    DMA3_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_32_BIT | (len>>2);
    while(DMA3_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
  }
}

void MemSet8DMA3(const u8 v,void *dst,u32 len)
{memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA3
  static void (*lp_MemSet8CPU)(u8 v,void *dst,u32 len)=MemSet8CPU;
  lp_MemSet8CPU(v,dst,len);
  return;
#endif
  
  if(len<1) return;
  
  u32 v32=v;
  v32=v32 | (v32<<8) | (v32<<16) | (v32<<24);
  
  u8 *pb=(u8*)dst;
  
  if((((u32)pb)&BIT(0))!=0){
    pb[0]=v32;
    pb+=1;
    len-=1;
  }
  
  if((((u32)pb)&BIT(1))!=0){
    u16 *pb16=(u16*)pb;
    pb16[0]=v32;
    pb+=2;
    len-=2;
  }
  
  MemSet32DMA3(v32,pb,len);
  
  if((len&3)==0) return;
  
  pb+=len&(~3);
  len-=len&(~3);
  
  if((len&BIT(1))!=0){
    u16 *pb16=(u16*)pb;
    pb16[0]=v32;
    pb+=2;
    len-=2;
  }
  
  if((len&BIT(0))!=0){
    pb[0]=v32;
  }
  
}

void MemCopy16DMA2(const void *src,void *dst,u32 len)
{memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA2
  static void (*lp_MemCopy16CPU)(void *src,void *dst,u32 len)=MemCopy16CPU;
  lp_MemCopy16CPU(src,dst,len);
  return;
#endif

  DC_FlushRangeOverrun(src,len);
  DC_FlushRangeOverrun(dst,len);
  
  u8 *_src=(u8*)src;
  u8 *_dst=(u8*)dst;
  
  DMA2_SRC = (uint32)_src;
  DMA2_DEST = (uint32)_dst;
  DMA2_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | (len>>1);
  while(DMA2_CR & DMA_BUSY);
  return;
  
  while(prcdiv<=len){
    DMA2_SRC = (uint32)_src;
    DMA2_DEST = (uint32)_dst;
    DMA2_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | (prcdiv>>1);
    while(DMA2_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
    len-=prcdiv;
    _src+=prcdiv;
    _dst+=prcdiv;
  }
  
  if(len!=0){
    DMA2_SRC = (uint32)_src;
    DMA2_DEST = (uint32)_dst;
    DMA2_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | (len>>1);
    while(DMA2_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
  }
}

void MemSet16DMA2(const u16 v,void *dst,u32 len)
{memchk{ if(dst==NULL){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
#ifdef notuseMemDMA2
  static void (*lp_MemSet16CPU)(u16 v,void *dst,u32 len)=MemSet16CPU;
  lp_MemSet16CPU(v,dst,len);
  return;
#endif

  DMAFIXSRC=(vu32)v+((vu32)v<<16);
  
  DC_FlushRangeOverrun(&DMAFIXSRC,4);
  DC_FlushRangeOverrun(dst,len);
  
  u8 *_dst=(u8*)dst;
  DMA2_SRC = (uint32)&DMAFIXSRC;
  
  DMA2_DEST = (uint32)_dst;
  DMA2_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_16_BIT | (len>>1);
  while(DMA2_CR & DMA_BUSY);
  return;
  
  while(prcdiv<=len){
    DMA2_DEST = (uint32)_dst;
    DMA2_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_16_BIT | (prcdiv>>1);
    while(DMA2_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
    len-=prcdiv;
    _dst+=prcdiv;
  }
  
  if(len!=0){
    DMA2_DEST = (uint32)_dst;
    DMA2_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_16_BIT | (len>>1);
    while(DMA2_CR & DMA_BUSY);
    static void (*lp_CP15_DrainWriteBuffer)(void)=CP15_DrainWriteBuffer;
    lp_CP15_DrainWriteBuffer();
  }
}

void MemCopy32swi256bit(const void *src,void *dst,u32 len)
{memchk{ if((src==NULL)||(dst==NULL)){ _consolePrintf("Hooked memory address error.%s%d\n",__FILE__,__LINE__); while(1); } }
  swiFastCopy((void*)src,dst,COPY_MODE_COPY | (len/4));
}

void safemalloc_halt(void)
{
  _consolePrintf("Please start with the START button if you use M3.\n");
  ShowLogHalt();
}

void *safemalloc(const int size)
{
//  return(malloc(size));
  
  if(size<=0) return(NULL);
  
  void *ptr;
  u32 adr;
  
  ptr=malloc(size+(64*1024)); // 64kbは必ず残す
  
  if(ptr==NULL){
//    _consolePrintf("malloc(%d) fail NULL\n",size);
    return(NULL);
  }
  
  adr=(u32)ptr;
  
  if((adr&3)!=0){ // 4byteアライメントされていなかったら
    _consolePrintf("malloc(%d) fail not 4bytealian\n",size);
    safemalloc_halt();
    return(NULL);
  }
  
  if(adr<0x02000000){ // 先頭アドレスがメインメモリより前だったら
    _consolePrintf("malloc(%d) fail adr<MemArea\n",size);
    safemalloc_halt();
    return(NULL);
  }
  
  if((0x02000000+(4*1024*1024))<=(adr+size)){ // 終端アドレスがメインメモリより後だったら
    _consolePrintf("malloc(%d) fail MemArea<adr+size\n",size);
    safemalloc_halt();
    return(NULL);
  }
  
  // 64kb余剰確保したから取り直す
  
  void *checkedptr=ptr;
  
  free(ptr);
  ptr=malloc(size);
//  ptr=realloc(checkedptr,size);
  
  adr=(u32)ptr;
  
/*
  if(checkedptr!=ptr){ // チェック済みの取得ポインタと違ったら
    free(ptr);
    _consolePrintf("malloc(%d) fail reallocerror.%x!=%x\n",size,(u32)checkedptr,(u32)ptr);
    safemalloc_halt();
    return(NULL);
  }
*/
  
  if((adr&3)!=0){ // 4byteアライメントされていなかったら
    free(ptr);
    _consolePrintf("malloc(%d) fail reallocerror.%x!=%x\n",size,(u32)checkedptr,(u32)ptr);
    safemalloc_halt();
    return(NULL);
  }
  
  if(adr<0x02000000){ // 先頭アドレスがメインメモリより前だったら
    free(ptr);
    _consolePrintf("malloc(%d) fail reallocerror.%x!=%x\n",size,(u32)checkedptr,(u32)ptr);
    safemalloc_halt();
    return(NULL);
  }
  
  if((0x02000000+(4*1024*1024))<=(adr+size)){ // 終端アドレスがメインメモリより後だったら
    free(ptr);
    _consolePrintf("malloc(%d) fail reallocerror.%x!=%x\n",size,(u32)checkedptr,(u32)ptr);
    safemalloc_halt();
    return(NULL);
  }
  
/*
  {
    u8 *pbuf=(u8*)ptr;
    for(s32 cnt=0;cnt<size;cnt++){
      pbuf[cnt]=0;
    }
  }
*/
  
//  _consolePrintf("[%x]",(u32)ptr);
  return(ptr);

}

void safefree(const void *ptr)
{
//  _consolePrintf("_%x_",(u32)ptr);
  if(ptr==NULL){
    _consolePrintf("safefree Request NullPointer.\n");
    ShowLogHalt();
    return;
  }
  
  free((void*)ptr);
}

bool testmalloc(int size)
{
  if(size<=0) return(false);
  
  void *ptr;
  u32 adr;
  
  ptr=malloc(size+(64*1024)); // 64kbは必ず残す
  
  if(ptr==NULL) return(false);
  
  adr=(u32)ptr;
  free(ptr);
  
  if((adr&3)!=0){ // 4byteアライメントされていなかったら
    return(false);
  }
  
  if((adr+size)<0x02000000){ // 先頭アドレスがメインメモリより前だったら
    return(false);
  }
  
  if((0x02000000+(4*1024*1024))<=adr){ // 終端アドレスがメインメモリより後だったら
    return(false);
  }
  
  return(true);
}

#define PrintFreeMem_Seg (128*16)

void PrintFreeMem(void)
{
  s32 i;
  u32 FreeMemSize=0;
  
  for(i=1*PrintFreeMem_Seg;i<4096*1024;i+=PrintFreeMem_Seg){
    if(testmalloc(i)==false) break;
    FreeMemSize=i;
  }
  
  _consolePrintf("FreeMem=%dbyte    \n",FreeMemSize);
}

