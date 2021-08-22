
#include <stdio.h>
#include <stdlib.h>

#include <NDS.h>

#include "_const.h"
#include "_console.h"
#include "memtool.h"
#include "extmem.h"
#include "inifile.h"

#include "mediatype.h"

typedef struct {
  u32 StartAdr,EndAdr,CurAdr;
  u32 *pSlot;
  u32 SlotCount;
} TBody_M3;

typedef struct {
  u32 StartAdr,EndAdr,CurAdr;
  u32 *pSlot;
  u32 SlotCount;
} TBody_SC;

typedef struct {
  u32 StartAdr,EndAdr,CurAdr;
  u32 *pSlot;
  u32 SlotCount;
} TBody_EZ4;

typedef struct {
  u32 StartAdr,EndAdr,CurAdr;
  u32 *pSlot;
  u32 SlotCount;
} TBody_DSBM;

typedef struct {
  EextmemType Type;
  void *pBody;
} TInfo;

static TInfo *pInfo;

#define PacketSize_M3 (2)
#define PacketSize_SC (2)
#define PacketSize_EZ4 (2)
#define PacketSize_DSBM (2)

// --- M3 stuff

static void SetM3_EnableCard(void) 
{
	// run unlock sequence
	volatile unsigned short tmp ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08E00002 ;
	tmp = *(volatile unsigned short *)0x0800000E ;
	tmp = *(volatile unsigned short *)0x08801FFC ;
	tmp = *(volatile unsigned short *)0x0800104A ;
	tmp = *(volatile unsigned short *)0x08800612 ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08801B66 ;
	tmp = *(volatile unsigned short *)0x08800006 ;
	tmp = *(volatile unsigned short *)0x08000000 ;
}

static void SetM3_EnablePSRAM(void) 
{
	// run unlock sequence
	volatile unsigned short tmp ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08E00002 ;
	tmp = *(volatile unsigned short *)0x0800000E ;
	tmp = *(volatile unsigned short *)0x08801FFC ;
	tmp = *(volatile unsigned short *)0x0800104A ;
	tmp = *(volatile unsigned short *)0x08800612 ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08801B66 ;
	tmp = *(volatile unsigned short *)0x08800004 ; // 0=bios?, 4 or c=8MByte?
	tmp = *(volatile unsigned short *)0x08000000 ;

        *(volatile u16*)0x09FFEFFE=0xAA55; // PepsiMan vote: The RAM on the M3 is made writable by writing 0xaa55 to 0x09ffeffe.

    // a=4; err=(n=0-7),(0x088nxxxx,0x089nxxxx,0x08anxxxx,0x08bnxxxx,0x08cnxxxx,0x08dnxxxx,0x08enxxxx,0x08fnxxxx)
}

#define M3_PSRAM_MaxSize (16*1024*1024)

static u32 _M3_GetMemorySize(void)
{
  u32 start=0x08000000;
  u32 end=start+M3_PSRAM_MaxSize;
  
  const u32 chksize=4*16;
  
  for(u32 adr=end-chksize;adr>=start;adr-=chksize){
    vu32 *pbuf=(u32*)adr;
    *pbuf=adr;
  }
  
  while(start<end){
    vu32 *pstart=(u32*)start;
    if(*pstart!=start) return(start-0x08000000);
    start+=chksize;
  }
  
  return(M3_PSRAM_MaxSize);
}

static u32 M3_GetMemorySize(void)
{
  u32 memsize;
  
  SetM3_EnablePSRAM();
  memsize=_M3_GetMemorySize();
  SetM3_EnableCard();
  
  _consolePrintf("PSRAM memory size %dMByte.\n",memsize/1024/1024);
  
  if(M3_PSRAM_MaxSize<memsize) memsize=M3_PSRAM_MaxSize;
  
  return(memsize);
}

// --- SC stuff

#define SSC_Disabled (0)
#define SSC_SDRAM (1)
#define SSC_CF (2)

#define SC_REG_UNLOCK	*(vu16*)(0x09FFFFFE)
static void SetSC_UNLOCK(int SSC)
{
  switch(SSC){
    case SSC_Disabled:
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0x0001;
      SC_REG_UNLOCK = 0x0001;
      break;
    case SSC_SDRAM:
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0x0005;
      SC_REG_UNLOCK = 0x0005;
      break;
    case SSC_CF:
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0xA55A;
      SC_REG_UNLOCK = 0x0003;
      SC_REG_UNLOCK = 0x0003;
  }
}
#undef SC_REG_UNLOCK

// --- EZ4 stuff

static inline void SetEZ4_open(void)
{return;
}

static inline void SetEZ4_close(void)
{return;
}

static u32 EZ4_GetMemorySize(void)
{
  u32 mbit=GlobalINI.AdapterConfig.EZ4_PSRAMSizeMByte*8;
  if(255<mbit) mbit=255;
  return(mbit*1024*1024/8);
}

// ---

// --- DS Browser Memory stuff

#define DSBM_Header ((vu16*)(0x080000B0))
#define DSBM_HeaderSize	(16)
#define DSBM_REG_UNLOCK	(*(vu16*)(0x8240000))

static inline bool ExistsDSBM(void)
{
  u16 *pHeaderData;
  {
    static const u8 Data[DSBM_HeaderSize]={0xFF,0xFF,0x00,0x00,0x00,0x24,0x24,0x24,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F};
    pHeaderData=(u16*)&Data[0];
  }
  
  for(u32 idx=0;idx<DSBM_HeaderSize/2;idx++){
    if(DSBM_Header[idx]!=pHeaderData[idx]) return(false);
  }
  
  return(true);
}

static inline void SetDSBM_MemoryOpen(void)
{
  DSBM_REG_UNLOCK=0x0001;
}

static inline void SetDSBM_MemoryClose(void)
{
  DSBM_REG_UNLOCK=0x0000;
}

// ---

void extmem_Init(void)
{
  if(pInfo!=NULL) extmem_Free();
  
  pInfo=(TInfo*)safemalloc(sizeof(TInfo));
  MemSet32DMA3(0,pInfo,sizeof(TInfo));
  
  pInfo->Type=EEMT_Unknown;
  
  switch(DIMediaType){
    case DIMT_NONE: break;
    case DIMT_M3CF: pInfo->Type=EEMT_M3; break;
    case DIMT_M3SD: pInfo->Type=EEMT_M3; break;
    case DIMT_MPCF: break;
    case DIMT_MPSD: break;
    case DIMT_SCCF: pInfo->Type=EEMT_SC; break;
    case DIMT_SCSD: pInfo->Type=EEMT_SC; break;
    case DIMT_NMMC: break;
    case DIMT_FCSR: break;
    case DIMT_EZSD: pInfo->Type=EEMT_EZ4; break;
    case DIMT_MMCF: break;
    case DIMT_SCMS: pInfo->Type=EEMT_SC; break;
    case DIMT_EWSD: break;
    case DIMT_NJSD: break;
    case DIMT_DLMS: break;
    case DIMT_G6FC: break;
  }
  
  if(pInfo->Type==EEMT_Unknown){
    if(ExistsDSBM()==true) pInfo->Type=EEMT_DSBM;
  }
  
  u32 EZ4MemorySize=0;
  if(pInfo->Type==EEMT_EZ4){
    EZ4MemorySize=EZ4_GetMemorySize();
    if(EZ4MemorySize==0) pInfo->Type=EEMT_Unknown;
  }
  
  if(pInfo->Type==EEMT_SC){
    if(GlobalINI.AdapterConfig.SC_EnabledDRAM==false) pInfo->Type=EEMT_Unknown;
  }
  
  switch(pInfo->Type){
    case EEMT_Unknown: {
      _consolePrintf("extmem:unknown cartridge type.\n");
    } break;
    case EEMT_M3: {
      _consolePrintf("extmem:set cartridge type to M3.\n");
      pInfo->pBody=(TBody_M3*)safemalloc(sizeof(TBody_M3));
      MemSet32DMA3(0,pInfo->pBody,sizeof(TBody_M3));
      
      u32 memsize=M3_GetMemorySize();
      
      TBody_M3 *pBody=(TBody_M3*)pInfo->pBody;
      
      pBody->StartAdr=0x08000000;
      pBody->EndAdr=pBody->StartAdr+memsize;
      pBody->CurAdr=pBody->StartAdr;
      pBody->pSlot=NULL;
      pBody->SlotCount=0;
    } break;
    case EEMT_SC: {
      _consolePrintf("extmem:set cartridge type to SC.\n");
      pInfo->pBody=(TBody_SC*)safemalloc(sizeof(TBody_SC));
      MemSet32DMA3(0,pInfo->pBody,sizeof(TBody_SC));
      
      TBody_SC *pBody=(TBody_SC*)pInfo->pBody;
      
      pBody->StartAdr=0x08000000;
      pBody->EndAdr=pBody->StartAdr+(32*1024*1024);
      pBody->CurAdr=pBody->StartAdr;
      pBody->pSlot=NULL;
      pBody->SlotCount=0;
    } break;
    case EEMT_EZ4: {
      _consolePrintf("extmem:set cartridge type to EZ4.\n");
      pInfo->pBody=(TBody_SC*)safemalloc(sizeof(TBody_EZ4));
      MemSet32DMA3(0,pInfo->pBody,sizeof(TBody_EZ4));
      
      TBody_EZ4 *pBody=(TBody_EZ4*)pInfo->pBody;
      
      pBody->StartAdr=0x08000000;
      pBody->EndAdr=pBody->StartAdr+EZ4MemorySize;
      pBody->CurAdr=pBody->StartAdr;
      pBody->pSlot=NULL;
      pBody->SlotCount=0;
    } break;
    case EEMT_DSBM: {
      _consolePrintf("extmem:set cartridge type to DSBM.\n");
      pInfo->pBody=(TBody_SC*)safemalloc(sizeof(TBody_DSBM));
      MemSet32DMA3(0,pInfo->pBody,sizeof(TBody_DSBM));
      
      TBody_DSBM *pBody=(TBody_DSBM*)pInfo->pBody;
      
      pBody->StartAdr=0x9000000;
      pBody->EndAdr=pBody->StartAdr+(8*1024*1024);
      pBody->CurAdr=pBody->StartAdr;
      pBody->pSlot=NULL;
      pBody->SlotCount=0;
    } break;
  }
}

void extmem_Free(void)
{
  if(pInfo==NULL) return;
  
  switch(pInfo->Type){
    case EEMT_Unknown: {
    } break;
    case EEMT_M3: {
      TBody_M3 *pBody=(TBody_M3*)pInfo->pBody;
      
      if(pBody->pSlot!=NULL){
        safefree(pBody->pSlot); pBody->pSlot=NULL;
      }
      
      safefree(pInfo->pBody); pInfo->pBody=NULL;
    } break;
    case EEMT_SC: {
      TBody_SC *pBody=(TBody_SC*)pInfo->pBody;
      
      if(pBody->pSlot!=NULL){
        safefree(pBody->pSlot); pBody->pSlot=NULL;
      }
      
      safefree(pInfo->pBody); pInfo->pBody=NULL;
    } break;
    case EEMT_EZ4: {
      TBody_EZ4 *pBody=(TBody_EZ4*)pInfo->pBody;
      
      if(pBody->pSlot!=NULL){
        safefree(pBody->pSlot); pBody->pSlot=NULL;
      }
      
      safefree(pInfo->pBody); pInfo->pBody=NULL;
    } break;
    case EEMT_DSBM: {
      TBody_DSBM *pBody=(TBody_DSBM*)pInfo->pBody;
      
      if(pBody->pSlot!=NULL){
        safefree(pBody->pSlot); pBody->pSlot=NULL;
      }
      
      safefree(pInfo->pBody); pInfo->pBody=NULL;
    } break;
  }
  
  safefree(pInfo); pInfo=NULL;
}

bool extmem_ExistMemory(void)
{
  if(pInfo==NULL) return(false);
  if(pInfo->pBody==NULL) return(false);
  
  return(true);
}

extern EextmemType extmem_GetMemType(void)
{
  if(extmem_ExistMemory()==false) return(EEMT_Unknown);
  
  return(pInfo->Type);
}

extern u32 extmem_GetMemSize(void)
{
  if(extmem_ExistMemory()==false) return(0);
  
  switch(pInfo->Type){
    case EEMT_Unknown: {
    } break;
    case EEMT_M3: {
      TBody_M3 *pBody=(TBody_M3*)pInfo->pBody;
      return(pBody->EndAdr-pBody->StartAdr);
    } break;
    case EEMT_SC: {
      TBody_SC *pBody=(TBody_SC*)pInfo->pBody;
      return(pBody->EndAdr-pBody->StartAdr);
    } break;
    case EEMT_EZ4: {
      TBody_EZ4 *pBody=(TBody_EZ4*)pInfo->pBody;
      return(pBody->EndAdr-pBody->StartAdr);
    } break;
    case EEMT_DSBM: {
      TBody_DSBM *pBody=(TBody_DSBM*)pInfo->pBody;
      return(pBody->EndAdr-pBody->StartAdr);
    } break;
  }
  
  return(0);
}

void extmem_SetCount(u32 Count)
{
  if(Count==0) return;
  if(pInfo==NULL) return;
  if(pInfo->pBody==NULL) return;
  
  switch(pInfo->Type){
    case EEMT_Unknown: {
    } break;
    case EEMT_M3: {
      TBody_M3 *pBody=(TBody_M3*)pInfo->pBody;
      
      if(pBody->pSlot!=NULL){
        _consolePrintf("extmem_SetSlotCount: Already allocated pSlot.\n");
        ShowLogHalt();
      }
      
      pBody->pSlot=(u32*)safemalloc(sizeof(u32)*Count);
      MemSet32DMA3(0,pBody->pSlot,sizeof(u32)*Count);
      pBody->SlotCount=Count;
    } break;
    case EEMT_SC: {
      TBody_SC *pBody=(TBody_SC*)pInfo->pBody;
      
      if(pBody->pSlot!=NULL){
        _consolePrintf("extmem_SetSlotCount: Already allocated pSlot.\n");
        ShowLogHalt();
      }
      
      pBody->pSlot=(u32*)safemalloc(sizeof(u32)*Count);
      MemSet32DMA3(0,pBody->pSlot,sizeof(u32)*Count);
      pBody->SlotCount=Count;
    } break;
    case EEMT_EZ4: {
      TBody_EZ4 *pBody=(TBody_EZ4*)pInfo->pBody;
      
      if(pBody->pSlot!=NULL){
        _consolePrintf("extmem_SetSlotCount: Already allocated pSlot.\n");
        ShowLogHalt();
      }
      
      pBody->pSlot=(u32*)safemalloc(sizeof(u32)*Count);
      MemSet32DMA3(0,pBody->pSlot,sizeof(u32)*Count);
      pBody->SlotCount=Count;
    } break;
    case EEMT_DSBM: {
      TBody_DSBM *pBody=(TBody_DSBM*)pInfo->pBody;
      
      if(pBody->pSlot!=NULL){
        _consolePrintf("extmem_SetSlotCount: Already allocated pSlot.\n");
        ShowLogHalt();
      }
      
      pBody->pSlot=(u32*)safemalloc(sizeof(u32)*Count);
      MemSet32DMA3(0,pBody->pSlot,sizeof(u32)*Count);
      pBody->SlotCount=Count;
    } break;
  }
}

bool extmem_Exists(u32 SlotIndex)
{
  if(pInfo==NULL) return(false);
  if(pInfo->pBody==NULL) return(false);
  
  switch(pInfo->Type){
    case EEMT_Unknown: {
      return(false);
    } break;
    case EEMT_M3: {
      TBody_M3 *pBody=(TBody_M3*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Exists(u32 SlotIndex=%d); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      if(pBody->pSlot[SlotIndex]==(u32)NULL) return(false);
    } break;
    case EEMT_SC: {
      TBody_SC *pBody=(TBody_SC*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Exists(u32 SlotIndex=%d); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      if(pBody->pSlot[SlotIndex]==(u32)NULL) return(false);
    } break;
    case EEMT_EZ4: {
      TBody_EZ4 *pBody=(TBody_EZ4*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Exists(u32 SlotIndex=%d); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      if(pBody->pSlot[SlotIndex]==(u32)NULL) return(false);
    } break;
    case EEMT_DSBM: {
      TBody_DSBM *pBody=(TBody_DSBM*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Exists(u32 SlotIndex=%d); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      if(pBody->pSlot[SlotIndex]==(u32)NULL) return(false);
    } break;
  }
  
  return(true);
}

bool extmem_Alloc(u32 SlotIndex,u32 Size)
{
  if(pInfo==NULL) return(false);
  if(pInfo->pBody==NULL) return(false);
  
  switch(pInfo->Type){
    case EEMT_Unknown: {
      return(false);
    } break;
    case EEMT_M3: {
      TBody_M3 *pBody=(TBody_M3*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Alloc(u32 SlotIndex=%d,u32 Size=%d); SlotCount==%d limit error.\n",SlotIndex,Size,pBody->SlotCount);
        return(false);
      }
      
      Size=(Size+(PacketSize_M3-1)) & (~(PacketSize_M3-1));
      
      u32 TermAdr=pBody->CurAdr+Size;
      
      if(pBody->EndAdr<TermAdr){
//        _consolePrintf("extmem_Alloc(u32 SlotIndex=%d,u32 Size=%d); MemAlloc limit error.\n",SlotIndex,Size);
        return(false);
      }
      
      pBody->pSlot[SlotIndex]=pBody->CurAdr;
      pBody->CurAdr=TermAdr;
    } break;
    case EEMT_SC: {
      TBody_SC *pBody=(TBody_SC*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Alloc(u32 SlotIndex=%d,u32 Size=%d); SlotCount==%d limit error.\n",SlotIndex,Size,pBody->SlotCount);
        return(false);
      }
      
      Size=(Size+(PacketSize_SC-1)) & (~(PacketSize_SC-1));
      
      u32 TermAdr=pBody->CurAdr+Size;
      
      if(pBody->EndAdr<TermAdr){
//        _consolePrintf("extmem_Alloc(u32 SlotIndex=%d,u32 Size=%d); MemAlloc limit error.\n",SlotIndex,Size);
        return(false);
      }
      
      pBody->pSlot[SlotIndex]=pBody->CurAdr;
      pBody->CurAdr=TermAdr;
    } break;
    case EEMT_EZ4: {
      TBody_EZ4 *pBody=(TBody_EZ4*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Alloc(u32 SlotIndex=%d,u32 Size=%d); SlotCount==%d limit error.\n",SlotIndex,Size,pBody->SlotCount);
        return(false);
      }
      
      Size=(Size+(PacketSize_EZ4-1)) & (~(PacketSize_EZ4-1));
      
      u32 TermAdr=pBody->CurAdr+Size;
      
      if(pBody->EndAdr<TermAdr){
//        _consolePrintf("extmem_Alloc(u32 SlotIndex=%d,u32 Size=%d); MemAlloc limit error.\n",SlotIndex,Size);
        return(false);
      }
      
      pBody->pSlot[SlotIndex]=pBody->CurAdr;
      pBody->CurAdr=TermAdr;
    } break;
    case EEMT_DSBM: {
      TBody_DSBM *pBody=(TBody_DSBM*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Alloc(u32 SlotIndex=%d,u32 Size=%d); SlotCount==%d limit error.\n",SlotIndex,Size,pBody->SlotCount);
        return(false);
      }
      
      Size=(Size+(PacketSize_DSBM-1)) & (~(PacketSize_DSBM-1));
      
      u32 TermAdr=pBody->CurAdr+Size;
      
      if(pBody->EndAdr<TermAdr){
//        _consolePrintf("extmem_Alloc(u32 SlotIndex=%d,u32 Size=%d); MemAlloc limit error.\n",SlotIndex,Size);
        return(false);
      }
      
      pBody->pSlot[SlotIndex]=pBody->CurAdr;
      pBody->CurAdr=TermAdr;
    } break;
  }
  
  return(true);
}

bool extmem_Write(u32 SlotIndex,void *pData,u32 DataSize)
{
  if(pInfo==NULL) return(false);
  if(pInfo->pBody==NULL) return(false);
  
  switch(pInfo->Type){
    case EEMT_Unknown: {
      return(false);
    } break;
    case EEMT_M3: {
      TBody_M3 *pBody=(TBody_M3*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Write(u32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      u16 *pSrcData=(u16*)pData;
      u32 SrcSize=DataSize;
      u16 *pDstData=(u16*)pBody->pSlot[SlotIndex];
      u32 DstSize=(SrcSize+(PacketSize_M3-1)) & (~(PacketSize_M3-1));
      
      if(pDstData==NULL){
        _consolePrintf("extmem_Write(u32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
        return(false);
      }
      
      SetM3_EnablePSRAM();
      MemCopy16DMA3(pSrcData,pDstData,DstSize);
      SetM3_EnableCard();
    } break;
    case EEMT_SC: {
      TBody_SC *pBody=(TBody_SC*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Write(u32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      u16 *pSrcData=(u16*)pData;
      u32 SrcSize=DataSize;
      u16 *pDstData=(u16*)pBody->pSlot[SlotIndex];
      u32 DstSize=(SrcSize+(PacketSize_SC-1)) & (~(PacketSize_SC-1));
      
      if(pDstData==NULL){
        _consolePrintf("extmem_Write(u32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
        return(false);
      }
      
      SetSC_UNLOCK(SSC_SDRAM);
      MemCopy16DMA3(pSrcData,pDstData,DstSize);
      SetSC_UNLOCK(SSC_CF);
    } break;
    case EEMT_EZ4: {
      TBody_EZ4 *pBody=(TBody_EZ4*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Write(u32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      u16 *pSrcData=(u16*)pData;
      u32 SrcSize=DataSize;
      u16 *pDstData=(u16*)pBody->pSlot[SlotIndex];
      u32 DstSize=(SrcSize+(PacketSize_EZ4-1)) & (~(PacketSize_EZ4-1));
      
      if(pDstData==NULL){
        _consolePrintf("extmem_Write(u32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
        return(false);
      }
      
      SetEZ4_close();
      MemCopy16DMA3(pSrcData,pDstData,DstSize);
      SetEZ4_open();
    } break;
    case EEMT_DSBM: {
      TBody_DSBM *pBody=(TBody_DSBM*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Write(u32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      u16 *pSrcData=(u16*)pData;
      u32 SrcSize=DataSize;
      u16 *pDstData=(u16*)pBody->pSlot[SlotIndex];
      u32 DstSize=(SrcSize+(PacketSize_DSBM-1)) & (~(PacketSize_DSBM-1));
      
      if(pDstData==NULL){
        _consolePrintf("extmem_Write(u32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
        return(false);
      }
      
      SetDSBM_MemoryOpen();
      MemCopy16DMA3(pSrcData,pDstData,DstSize);
      SetDSBM_MemoryClose();
    } break;
  }
  
  return(true);
}

bool extmem_Read(u32 SlotIndex,void *pData,u32 DataSize)
{
  if(pInfo==NULL) return(false);
  if(pInfo->pBody==NULL) return(false);
  
  switch(pInfo->Type){
    case EEMT_Unknown: {
      return(false);
    } break;
    case EEMT_M3: {
      TBody_M3 *pBody=(TBody_M3*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Read(u32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      u16 *pSrcData=(u16*)pBody->pSlot[SlotIndex];
//      u32 SrcSize=(Size+(PacketSize_M3-1)) & (~(PacketSize_M3-1));
      u16 *pDstData=(u16*)pData;
      u32 DstSize=DataSize;
      
      if(pSrcData==NULL){
        _consolePrintf("extmem_Read(u32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
        return(false);
      }
      
      SetM3_EnablePSRAM();
      MemCopy16DMA3(pSrcData,pDstData,DstSize);
      SetM3_EnableCard();
    } break;
    case EEMT_SC: {
      TBody_SC *pBody=(TBody_SC*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Read(u32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      u16 *pSrcData=(u16*)pBody->pSlot[SlotIndex];
//      u32 SrcSize=(Size+(PacketSize_SC-1)) & (~(PacketSize_SC-1));
      u16 *pDstData=(u16*)pData;
      u32 DstSize=DataSize;
      
      if(pSrcData==NULL){
        _consolePrintf("extmem_Read(u32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
        return(false);
      }
      
      SetSC_UNLOCK(SSC_SDRAM);
      MemCopy16DMA3(pSrcData,pDstData,DstSize);
      SetSC_UNLOCK(SSC_CF);
    } break;
    case EEMT_EZ4: {
      TBody_EZ4 *pBody=(TBody_EZ4*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Read(u32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      u16 *pSrcData=(u16*)pBody->pSlot[SlotIndex];
//      u32 SrcSize=(Size+(PacketSize_EZ4-1)) & (~(PacketSize_EZ4-1));
      u16 *pDstData=(u16*)pData;
      u32 DstSize=DataSize;
      
      if(pSrcData==NULL){
        _consolePrintf("extmem_Read(u32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
        return(false);
      }
      
      SetEZ4_close();
      MemCopy16DMA3(pSrcData,pDstData,DstSize);
      SetEZ4_open();
    } break;
    case EEMT_DSBM: {
      TBody_DSBM *pBody=(TBody_DSBM*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Read(u32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      u16 *pSrcData=(u16*)pBody->pSlot[SlotIndex];
//      u32 SrcSize=(Size+(PacketSize_DSBM-1)) & (~(PacketSize_DSBM-1));
      u16 *pDstData=(u16*)pData;
      u32 DstSize=DataSize;
      
      if(pSrcData==NULL){
        _consolePrintf("extmem_Read(u32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
        return(false);
      }
      
      SetDSBM_MemoryOpen();
      MemCopy16DMA3(pSrcData,pDstData,DstSize);
      SetDSBM_MemoryClose();
    } break;
  }
  
  return(true);
}

bool extmem_WriteByte(u32 SlotIndex,void *pData,u32 Pos,u32 Size)
{
  if(pInfo==NULL) return(false);
  if(pInfo->pBody==NULL) return(false);
  
  switch(pInfo->Type){
    case EEMT_Unknown: {
      return(false);
    } break;
    case EEMT_M3: {
      TBody_M3 *pBody=(TBody_M3*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Write(u32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      u8 *pSrcData=(u8*)pData;
      u8 *pDstData=(u8*)pBody->pSlot[SlotIndex];
      
      if(pDstData==NULL){
        _consolePrintf("extmem_Write(u32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
        return(false);
      }
      
      pDstData+=Pos;
      
      SetM3_EnablePSRAM();
      for(u32 idx=0;idx<Size;idx++){
        pDstData[idx]=pSrcData[idx];
      }
      SetM3_EnableCard();
    } break;
    case EEMT_SC: {
    } break;
    case EEMT_EZ4: {
    } break;
    case EEMT_DSBM: {
    } break;
  }
  
  return(true);
}

bool extmem_ReadByte(u32 SlotIndex,void *pData,u32 Pos,u32 Size)
{
  if(pInfo==NULL) return(false);
  if(pInfo->pBody==NULL) return(false);
  
  switch(pInfo->Type){
    case EEMT_Unknown: {
      return(false);
    } break;
    case EEMT_M3: {
      TBody_M3 *pBody=(TBody_M3*)pInfo->pBody;
      
      if(pBody->SlotCount<=SlotIndex){
        _consolePrintf("extmem_Read(u32 SlotIndex=%d,...); SlotCount==%d limit error.\n",SlotIndex,pBody->SlotCount);
        return(false);
      }
      
      u8 *pSrcData=(u8*)pBody->pSlot[SlotIndex];
      u8 *pDstData=(u8*)pData;
      
      if(pSrcData==NULL){
        _consolePrintf("extmem_Read(u32 SlotIndex=%d,...); pSlot[SlotIndex]==NULL not allocated error.\n",SlotIndex);
        return(false);
      }
      
      pSrcData+=Pos;
      
      SetM3_EnablePSRAM();
      for(u32 idx=0;idx<Size;idx++){
        pDstData[idx]=pSrcData[idx];
      }
      SetM3_EnableCard();
    } break;
    case EEMT_SC: {
    } break;
    case EEMT_EZ4: {
    } break;
    case EEMT_DSBM: {
    } break;
  }
  
  return(true);
}

