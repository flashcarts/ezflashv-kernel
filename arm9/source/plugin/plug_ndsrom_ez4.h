
// --- EZ4 stuff

static void EZ4_OpenWrite()
{
//	  WAIT_CR &= ~0x80;
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9C40000 = 0x1500;
	*(vuint16 *)0x9fc0000 = 0x1500;
}

static void EZ4_CloseWrite()
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;  
	*(vuint16 *)0x9C40000 = 0xd200;
	*(vuint16 *)0x9fc0000 = 0x1500;
//	WAIT_CR |= 0x80;

}

#define EZ4_NandControl_Flash (0)
#define EZ4_NandControl_SD (1)

static void EZ4_SetNandControl(uint16  control)
{
	*(vuint16 *)0x9fe0000 = 0xd200;
	*(vuint16 *)0x8000000 = 0x1500;
	*(vuint16 *)0x8020000 = 0xd200;
	*(vuint16 *)0x8040000 = 0x1500;
	*(vuint16 *)0x9400000 = control;
	*(vuint16 *)0x9fc0000 = 0x1500;
}

static inline void SetEZ4_open(void)
{return;
  EZ4_OpenWrite();
  EZ4_SetNandControl(EZ4_NandControl_SD);
}

static inline void SetEZ4_close(void)
{return;
  EZ4_OpenWrite();
  EZ4_SetNandControl(EZ4_NandControl_SD);
}

#include "BootStrap_M3_pepsiman_m3powerloader_20060117_bin.h"

#define ReadBufCount (16*512)

static void boot_EZ4(u32 FileIndex)
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
      
      u16 *pBootStrap=(u16*)BootStrap_M3_pepsiman_m3powerloader_20060117_bin;
      u32 BootStrapSize=BootStrap_M3_pepsiman_m3powerloader_20060117_bin_size;
      
      SetEZ4_close();
      MemCopy16DMA3(pBootStrap,pROM16,BootStrapSize);
      SetEZ4_open();
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
      
      SetEZ4_close();
      MemCopy16DMA3(pReadBuf,pROM16,rs+1);
      SetEZ4_open();
      pROM16+=(rs+1)/2;
      
      if(rs!=ReadBufCount) break;
    }
    
    free(pReadBuf); pReadBuf=NULL;
  }
  
  _consolePrintf("FAT_fclose\n");
  FileSys_fclose(fh);
  _consolePrintf("FAT_FreeFiles\n");
  FAT_FreeFiles();
  
  _consolePrintf("Lock EZ4.\n");
  SetEZ4_close();
  
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

static void (*lp_boot_EZ4)(u32 FileIndex)=boot_EZ4;

#undef _REG_WAIT_CR
#undef ReadBufCount

