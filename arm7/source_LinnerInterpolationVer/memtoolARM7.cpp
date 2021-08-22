
#include <stdio.h>
#include <stdlib.h>

#include <NDS.h>

#include "memtoolARM7.h"

void* safemalloc(int size)
{
  u32 *res=(u32*)malloc(size+4);
  
//  for(u32 i=0;i<(size+4)/4;i++) res[i]=0;
  
  vu32 v=0;
  
  while((DMA3_CR&DMA_BUSY)!=0);
  DMA3_SRC = (u32)&v;
  DMA3_DEST = (u32)res;
  DMA3_CR=(DMA_32_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_FIX | DMA_DST_INC)+((size+4)/4);
  while((DMA3_CR&DMA_BUSY)!=0);
  
  return(res);
}

/*
void safefree(void *ptr)
{
  if(ptr!=NULL) free(ptr);
}
*/

/*

void MemSet16CPU(vu16 v,void *dst,u32 len)
{
  if(len<2) return;
  
  u16 *_dst=(u16*)dst;
  
  for(u32 cnt=0;cnt<(len/2);cnt++){
    _dst[cnt]=v;
  }
}

*/

/*

void MemSet32CPU(u32 v,void *dst,u32 len)
{
  if(len<4) return;
  
  u32 *_dst=(u32*)dst;
  
  for(u32 cnt=0;cnt<(len/4);cnt++){
    _dst[cnt]=v;
  }
}

*/

void MemCopy16DMA3(void *src,void *dst,u32 len)
{
//  MemCopy16CPU(src,dst,len);return;
  while((DMA3_CR&DMA_BUSY)!=0);
  DMA3_SRC = (u32)src;
  DMA3_DEST = (u32)dst;
  DMA3_CR=(DMA_16_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_INC | DMA_DST_INC)+(len/2);
  while((DMA3_CR&DMA_BUSY)!=0);
}

/*

void MemCopy32DMA3(void *src,void *dst,u32 len)
{
//  MemCopy32CPU(src,dst,len);return;
  while((DMA3_CR&DMA_BUSY)!=0);
  DMA3_SRC = (u32)src;
  DMA3_DEST = (u32)dst;
  DMA3_CR=(DMA_32_BIT | DMA_ENABLE | DMA_START_NOW |  DMA_SRC_INC | DMA_DST_INC)+(len/4);
  while((DMA3_CR&DMA_BUSY)!=0);
}

*/

void MemSet16DMA3(u16 v,void *dst,u32 len)
{
//  MemSet16CPU(v,dst,len);return;
  while((DMA3_CR&DMA_BUSY)!=0);
  DMA3_SRC = (u32)&v;
  DMA3_DEST = (u32)dst;
  DMA3_CR=(DMA_16_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_FIX | DMA_DST_INC)+(len/2);
  while((DMA3_CR&DMA_BUSY)!=0);
}

/*

void MemSet32DMA3(u32 v,void *dst,u32 len)
{
//  MemSet32CPU(v,dst,len);return;
  while((DMA3_CR&DMA_BUSY)!=0);
  DMA3_SRC = (u32)&v;
  DMA3_DEST = (u32)dst;
  DMA3_CR=(DMA_32_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_FIX | DMA_DST_INC)+(len/4);
  while((DMA3_CR&DMA_BUSY)!=0);
}

*/

