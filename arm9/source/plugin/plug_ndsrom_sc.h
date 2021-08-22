
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

#define _REG_WAIT_CR (*(vuint16*)0x04000204)

#include "BootStrap_SC_bin.h"

#define ReadBufCount (16*512)

static void boot_SC(u32 FileIndex)
{
  int fh=FileSys_fopen(FileIndex);
  
  u32 FileSize=FileSys_GetFileDataSize(FileIndex);
  
  u16 *pROM16=(u16*)0x08000000;
  
  {
    u16 *prb=(u16*)malloc(512);
    
    FileSys_fseek(fh,0,SEEK_SET);
    FileSys_fread_fast(prb,1,512,fh);
    FileSys_fseek(fh,0,SEEK_SET);
    
    if((prb[2]==0)&&(prb[3]==0)&&(prb[0x10]!=0x4e20)&&(prb[0x11]!=0x5344)&&(prb[0x12]!=0x6c20)){
      _consolePrintf("Transfer BootStrap.\n");
      
      u16 *pBootStrap=(u16*)BootStrap_SC_bin;
      u32 BootStrapSize=BootStrap_SC_bin_size;
      
      SetSC_UNLOCK(SSC_SDRAM);
      MemCopy16DMA3(pBootStrap,pROM16,BootStrapSize);
      SetSC_UNLOCK(SSC_CF);
      pROM16+=BootStrapSize/2;
    }
    
    free(prb); prb=NULL;
  }
  
  {
    _consolePrintf("Read file and write ROM.\n");
    
    u16 *pReadBuf=(u16*)malloc(ReadBufCount);
    u32 ReadCount=0;
    
    int ReadPosY=_consoleGetPrintSetY();
    
    FileSys_fseek(fh,0,SEEK_SET);
    
    while(1){
      _consolePrintSet(0,ReadPosY);
      _consolePrintf("%d/%dkb\n",ReadCount/1024,FileSize/1024);
      
      u32 rs=FileSys_fread_fast(pReadBuf,1,ReadBufCount,fh);
      ReadCount+=rs;
      
      SetSC_UNLOCK(SSC_SDRAM);
      MemCopy16DMA3(pReadBuf,pROM16,rs+1);
      SetSC_UNLOCK(SSC_CF);
      pROM16+=(rs+1)/2;
      
      if(rs!=ReadBufCount) break;
    }
    
    free(pReadBuf); pReadBuf=NULL;
  }
  
  _consolePrintf("FAT_fclose\n");
  FileSys_fclose(fh);
  _consolePrintf("FAT_FreeFiles\n");
  FAT_FreeFiles();
  
  _consolePrintf("Lock SuperCard.\n");
  SetSC_UNLOCK(SSC_Disabled);
  
  {
    u16 KEYS_CUR;
    
    KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
    while(KEYS_CUR!=0){
      KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
    }
  }
  
  _consolePrintf("resetMemory1_ARM9\n");
  lp_resetMemory1_ARM9();
  _consolePrintf("resetMemory2_ARM9\n");
  lp_resetMemory2_ARM9();
  
  {
//    _consolePrintf("reboot...\n");
    
    REG_IME = IME_DISABLE;	// Disable interrupts
    WAIT_CR |= (0x8080);  // ARM7 has access to GBA cart
    *((vu32*)0x027FFE08) = (u32)0xE59FF014;  // ldr pc, 0x027FFE24
    *((vu32*)0x027FFE24) = (u32)0x027FFE08;  // Set ARM9 Loop address (M3/SC)
    IPC3->RESET=RESET_GBAROM;
    swiSoftReset();  // Reset
  }
  
  while(1);
}

static void (*lp_boot_SC)(u32 FileIndex)=boot_SC;

#undef _REG_WAIT_CR
#undef ReadBufCount

