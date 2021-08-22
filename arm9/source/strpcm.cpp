
#include <NDS.h>
#include <NDS/ARM9/CP15.h>

#include "_console.h"

#include <stdio.h>
#include <stdlib.h>

#include "memtool.h"
#include "dmalink.h"

#include "Emulator.h"

#include "../../ipc3.h"

#include "filesys.h"

#include "arm9tcm.h"

#include "strpcm.h"

#include "plugin/plug_dpg.h"

static volatile bool strpcmPause;

volatile bool VsyncPassedCount=0;

volatile bool strpcmRequestStop;

volatile bool strpcmRingEmptyFlag;
volatile u32 strpcmRingBufReadIndex;
volatile u32 strpcmRingBufWriteIndex;

s16 *strpcmRingLBuf=NULL;
s16 *strpcmRingRBuf=NULL;

static int strpcmVolume16;

static void strpcmUpdate(void);

CODE_IN_ITCM void InterruptHandler_DMA1(void)
{return;
  REG_IME=0;
  DMALinkInterruptHandler_DMA1();
  REG_IME=1;
}

CODE_IN_ITCM void InterruptHandler_VBlank(void)
{
#ifdef notuseSound
//  strpcmUpdate();
#endif
  
/*
  extern void InterruptHandler_Vsync(void);
  static void (*lp_InterruptHandler_Vsync)(void)=(VoidFunctionPointer)InterruptHandler_Vsync;
  lp_InterruptHandler_Vsync();
*/
  extern CODE_IN_ITCM void InterruptHandler_Vsync(void);
  InterruptHandler_Vsync();
  
  VsyncPassedCount++;
}

// ------------------------------------------

#define CACHE_LINE_SIZE (32)

CODE_IN_ITCM void ins_DC_FlushRangeOverrun(void *v,u32 size)
{
  static void (*lp_DC_FlushRange)(void *base, u32 size)=DC_FlushRange;
  static void (*lp_DC_InvalidateRange)(void *base, u32 size)=DC_InvalidateRange;
  
  size&=~(CACHE_LINE_SIZE-1);
  size+=CACHE_LINE_SIZE;
  
//  if(v==NULL) return;
//  if(size==0) return;
  
  lp_DC_FlushRange(v,size);
  lp_DC_InvalidateRange(v,size);
}

CODE_IN_ITCM void InterruptHandler_IPC_SYNC(void)
{
  switch(IPC3->IR){
    case IR_NULL: {
    } break;
    case IR_NextSoundData: {
      {
        static void (*lp_DC_FlushAll)(void)=DC_FlushAll;
        lp_DC_FlushAll();
      }
      
      strpcmUpdate();
      
      const u32 Samples=IPC3->strpcmSamples;
      const u32 Channels=IPC3->strpcmChannels;
      
      ins_DC_FlushRangeOverrun(IPC3->strpcmLBuf,Samples*2);
      if(Channels==2) ins_DC_FlushRangeOverrun(IPC3->strpcmRBuf,Samples*2);
      
      IPC3->strpcmWriteRequest=0;
    } break;
    case IR_FileSys_fread: {
      static void (*lp_DPG_fread)(void)=DPG_fread;
      lp_DPG_fread();
    } break;
    case IR_FileSys_fread_flash: {
      static void (*lp_DPG_fread_flash)(void)=DPG_fread_flash;
      lp_DPG_fread_flash();
    } break;
  }
  
  IPC3->IR=IR_NULL;
  
}

void InitInterrupts(void)
{
  REG_IME = 0;
  irqInit();
  irqSet(IRQ_IPC_SYNC,(VoidFunctionPointer)InterruptHandler_IPC_SYNC);
  irqSet(IRQ_VBLANK,(VoidFunctionPointer)InterruptHandler_VBlank);
  irqSet(IRQ_DMA1,(VoidFunctionPointer)InterruptHandler_DMA1);
  REG_IPC_SYNC=IPC_SYNC_IRQ_ENABLE;
  REG_IME = 1;
}

void strpcmStart(bool FastStart,u32 SampleRate,u32 SamplePerBuf,u32 ChannelCount,EstrpcmFormat strpcmFormat)
{
#ifdef notuseSound
  return;
#endif
  
  DC_FlushAll();
  while(IPC3->strpcmControl!=strpcmControl_NOP){
//    DC_FlushAll();
//    swiWaitForVBlank();
  }
  
  strpcmRequestStop=false;
  strpcmPause=false;
  
  u32 Samples=SamplePerBuf;
  u32 RingSamples=Samples*strpcmRingBufCount;
  
  strpcmRingEmptyFlag=false;
  strpcmRingBufReadIndex=0;
  if(FastStart==false){
    strpcmRingBufWriteIndex=strpcmRingBufCount-1;
    }else{
    strpcmRingBufWriteIndex=1;
  }
  
  strpcmRingLBuf=(s16*)safemalloc(RingSamples*2);
  strpcmRingRBuf=(s16*)safemalloc(RingSamples*2);
  
  MemSet16DMA3(0,strpcmRingLBuf,RingSamples*2);
  MemSet16DMA3(0,strpcmRingRBuf,RingSamples*2);
  
  IPC3->strpcmFreq=SampleRate;
  IPC3->strpcmSamples=Samples;
  IPC3->strpcmChannels=ChannelCount;
  IPC3->strpcmFormat=strpcmFormat;
  
  // ------
  
  IPC3->strpcmLBuf=(s16*)safemalloc(Samples*2);
  IPC3->strpcmRBuf=(s16*)safemalloc(Samples*2);
/*
  IPC3->strpcmLBuf=(s16*)IPC3->strpcmLBufArr;
  IPC3->strpcmRBuf=(s16*)IPC3->strpcmRBufArr;
*/
  
  MemSet16DMA3(0,IPC3->strpcmLBuf,Samples*2);
  MemSet16DMA3(0,IPC3->strpcmRBuf,Samples*2);
  
  // ------
  
  DC_FlushAll();
  IPC3->strpcmControl=strpcmControl_Play;
  
  DC_FlushAll();
  while(IPC3->strpcmControl!=strpcmControl_NOP){
//    DC_FlushAll();
//    swiWaitForVBlank();
  }
}

void strpcmStop(void)
{
#ifdef notuseSound
  return;
#endif
  
  strpcmRequestStop=true;
  
  DC_FlushAll();
  while(IPC3->strpcmControl!=strpcmControl_NOP){
    DC_FlushAll();
  }
  
  IPC3->strpcmControl=strpcmControl_Stop;
  
  DC_FlushAll();
  while(IPC3->strpcmControl!=strpcmControl_NOP){
    DC_FlushAll();
  }
  
  strpcmRequestStop=false;
  strpcmPause=false;
  
  strpcmRingEmptyFlag=false;
  strpcmRingBufReadIndex=0;
  strpcmRingBufWriteIndex=0;
  
  if(strpcmRingLBuf!=NULL){
    safefree((void*)strpcmRingLBuf); strpcmRingLBuf=NULL;
  }
  if(strpcmRingRBuf!=NULL){
    safefree((void*)strpcmRingRBuf); strpcmRingRBuf=NULL;
  }
  
  IPC3->strpcmFreq=0;
  IPC3->strpcmSamples=0;
  IPC3->strpcmChannels=0;
  
  if(IPC3->strpcmLBuf!=NULL){
    safefree(IPC3->strpcmLBuf); IPC3->strpcmLBuf=NULL;
  }
  if(IPC3->strpcmRBuf!=NULL){
    safefree(IPC3->strpcmRBuf); IPC3->strpcmRBuf=NULL;
  }
}

// ----------------------------------------------

static u32 DMAFIXSRC;

CODE_IN_ITCM void ins_MemSet16DMA2(u16 v,void *dst,u32 len)
{
#ifdef notuseMemDMA2
  static void (*lp_MemSet16CPU)(u16 v,void *dst,u32 len)=MemSet16CPU;
  lp_MemSet16CPU(v,dst,len);
  return;
#endif

  DMAFIXSRC=(vu32)v+((vu32)v<<16);
  
  ins_DC_FlushRangeOverrun(&DMAFIXSRC,4);
  ins_DC_FlushRangeOverrun(dst,len);
  
  u8 *_dst=(u8*)dst;
  DMA2_SRC = (uint32)&DMAFIXSRC;
  
  DMA2_DEST = (uint32)_dst;
  DMA2_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_16_BIT | (len>>1);
  while(DMA2_CR & DMA_BUSY);
  return;
}

CODE_IN_ITCM void ins_MemCopy16DMA2(void *src,void *dst,u32 len)
{
#ifdef notuseMemDMA2
  static void (*lp_MemCopy16CPU)(void *src,void *dst,u32 len)=MemCopy16CPU;
  lp_MemCopy16CPU(src,dst,len);
  return;
#endif

  ins_DC_FlushRangeOverrun(src,len);
  ins_DC_FlushRangeOverrun(dst,len);
  
  u8 *_src=(u8*)src;
  u8 *_dst=(u8*)dst;
  
  DMA2_SRC = (uint32)_src;
  DMA2_DEST = (uint32)_dst;
  DMA2_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | (len>>1);
  while(DMA2_CR & DMA_BUSY);
  return;
}

CODE_IN_ITCM void ins_MemCopy32swi256bit(void *src,void *dst,u32 len)
{
  swiFastCopy(src,dst,COPY_MODE_COPY | (len/4));
}

void strpcmUpdate(void)
{
#ifdef notuseSound
  strpcmRingBufReadIndex=(strpcmRingBufReadIndex+1) & strpcmRingBufBitMask;
  return;
#endif

//  while(DMA2_CR & DMA_BUSY);
//  while(DMA3_CR & DMA_BUSY);
  
  u32 Samples=IPC3->strpcmSamples;
  const u32 Channels=IPC3->strpcmChannels;
  
  s16 *ldst=IPC3->strpcmLBuf;
  s16 *rdst=IPC3->strpcmRBuf;
  
  if((ldst==NULL)||(rdst==NULL)) return;
//  _consolePrintf("%x,%x\n",ldst,rdst);
  
  if((strpcmRingLBuf==NULL)||(strpcmRingRBuf==NULL)){
    ins_MemSet16DMA2(0,ldst,Samples*2);
    if(Channels==2) ins_MemSet16DMA2(0,rdst,Samples*2);
    return;
  }
  
  bool IgnoreFlag=false;
  
  u32 CurIndex=(strpcmRingBufReadIndex+1) & strpcmRingBufBitMask;
  
  s16 *lsrc=&strpcmRingLBuf[Samples*CurIndex];
  s16 *rsrc=&strpcmRingRBuf[Samples*CurIndex];
  
  if(strpcmPause==true) IgnoreFlag=true;
//  if(strpcmRequestStop==true) IgnoreFlag=true;
  
  if(CurIndex==strpcmRingBufWriteIndex){
    strpcmRingEmptyFlag=true;
    IgnoreFlag=true;
  }
  
  if(IPC3->strpcmFormat==SPF_GSM){
    Samples=Samples/320*65/2;
  }
  
  if(IgnoreFlag==true){
    ins_MemSet16DMA2(0,ldst,Samples*2);
    if(Channels==2) ins_MemSet16DMA2(0,rdst,Samples*2);
    return;
  }
  
  ins_MemCopy16DMA2(lsrc,ldst,Samples*2);
  if(Channels==2) ins_MemCopy16DMA2(rsrc,rdst,Samples*2);
  
/*
  if(((Samples*2)&31)!=0){
    ins_MemCopy16DMA2(lsrc,ldst,Samples*2);
    if(Channels==2) ins_MemCopy16DMA2(rsrc,rdst,Samples*2);
    }else{
    ins_MemCopy32swi256bit(lsrc,ldst,Samples*2);
    if(Channels==2) ins_MemCopy32swi256bit(rsrc,rdst,Samples*2);
  }
*/
  
  strpcmRingBufReadIndex=CurIndex;
}

void strpcmSetVolume16(int v)
{
  if(v<0) v=0;
  if(64<v) v=64;
  
  strpcmVolume16=v;
  
  IPC3->strpcmVolume16=strpcmVolume16;
}

int strpcmGetVolume16(void)
{
  return(strpcmVolume16);
}

void strpcmSetPause(bool v)
{
  strpcmPause=v;
}

bool strpcmGetPause(void)
{
  return(strpcmPause);
}

