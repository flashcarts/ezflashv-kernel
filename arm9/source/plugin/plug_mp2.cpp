
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NDS.h>

#include "../filesys.h"
#include "../memtool.h"

#include "../_console.h"

#include "../../../ipc3.h"

#include "plug_mp2.h"

#define SamplePerFrame (1152)

#define ReadBufSize (4*1024)

static bool Initialized=false;

void FreeMP2(void);

static CStream *pCStream;

static u32 MovePosition;
static s64 MoveSample;

static u8 *pReadTempBuffer;

bool StartMP2(CStream *_pCStream)
{
  if(Initialized==true){
    FreeMP2();
  }
  Initialized=true;
  
  pCStream=_pCStream;
  
  _consolePrintf("Request ARM7 mp2 decode.\n");
  
  IPC3->IR=IR_NULL;
  IPC3->IR_filesize=pCStream->GetSize();
  IPC3->IR_readsize=0;
  IPC3->IR_readbuf=(u8*)malloc(ReadBufSize);
  IPC3->IR_readbufsize=0;
  IPC3->IR_flash=false;
  
  IPC3->IR_samples=0;
  
  MovePosition=0;
  MoveSample=0;
  
  IPC3->IR_readbufsize=pCStream->ReadBuffer(IPC3->IR_readbuf,ReadBufSize);
  
  pReadTempBuffer=(u8*)malloc(ReadBufSize);
  
  return(true);
}

u32 UpdateMP2(s16 *lbuf,s16 *rbuf)
{
  return(SamplePerFrame);
}

void FreeMP2(void)
{
  if(Initialized==true){
    pCStream=NULL;
    IPC3->IR_filesize=0;
    IPC3->IR_readsize=0;
    if(IPC3->IR_readbuf!=NULL){
      free(IPC3->IR_readbuf); IPC3->IR_readbuf=NULL;
    }
    if(pReadTempBuffer!=NULL){
      free(pReadTempBuffer); pReadTempBuffer=NULL;
    }
  }
  Initialized=false;
}

u32 MP2_GetSamplePerFrame(void)
{
  return(SamplePerFrame);
}

void MP2_SetPosition(double per,s64 smp)
{
  MovePosition=(int)(pCStream->GetSize()*per);
  if(MovePosition<0) MovePosition=0;
  MovePosition&=~1;
  MoveSample=smp;
  IPC3->IR_flash=true;
}

void MP2_LoadReadBuffer(void)
{
  if(IPC3->IR_flash==true) return;
  
  u8 *buf=(u8*)IPC3->IR_readbuf;
  int ReadSize=ReadBufSize-IPC3->IR_readbufsize;
  
  ReadSize&=~1;
  if(ReadSize==0) return;
  
  REG_IME=0;
  ReadSize=pCStream->ReadBuffer(&buf[IPC3->IR_readbufsize],ReadSize);
  IPC3->IR_readbufsize+=ReadSize;
  REG_IME=1;
}

void MP2_fread(void)
{
//  _consolePrintf("fr%d,%d",IPC3->IR_readsize,IPC3->IR_readbufsize);
  
  u8 *buf=(u8*)IPC3->IR_readbuf;
  int size=IPC3->IR_readsize;
  int remain=IPC3->IR_readbufsize-size;
  
  if(remain<=0){
    remain=0;
    }else{
    DC_FlushRangeOverrun((void*)buf,ReadBufSize);
    DMA0_SRC = (uint32)&buf[size];
    DMA0_DEST = (uint32)&buf[0];
    DMA0_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | (remain/2);
//    while(DMA0_CR & DMA_BUSY);
  }
  
  IPC3->IR_readbufsize=remain;
}

void MP2_fread_flash(void)
{
//  _consolePrint("!");
  
  pCStream->SetOffset(MovePosition);
  MovePosition=0;
  
  IPC3->IR_readbufsize=pCStream->ReadBuffer(IPC3->IR_readbuf,ReadBufSize-36); // fetch sector size.
  
  IPC3->IR_samples=MoveSample;
  MoveSample=0;
}

