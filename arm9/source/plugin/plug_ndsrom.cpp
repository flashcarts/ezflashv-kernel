
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "../../../ipc3.h"

#ifndef USENDSROM

bool BootNDSROM(u32 FileIndex)
{
  return(false);
}

#else

#include "../_console.h"
#include <NDS/ARM9/CP15.h>

#include "../_const.h"

#include "../filesys.h"
#include "../memtool.h"

#include "plug_ndsrom.h"
#include "../inifile.h"

#include "../md5_global.h"
#include "../md5.h"

extern void videoSetModeSub_SetShowLog(bool e); // from main.cpp

void chkmd5(u32 FileIndex)
{
  int fhandle;
  u8 *inbuf;
  u32 fsize;
  MD5_CTX MD5CTX;
  unsigned char MD5Data[16];
  
#define bufsize (64*1024)

  u32 FileSize_DSBrickA=151361;
  unsigned char MD5Data_DSBrickA[16*3]={0xa9,0x00,0x00,0x59,0x00,0x00,0xcf,0x00,0x00,0xa5,0x00,0x00,0x14,0x00,0x00,0xf4,0x00,0x00,0xc7,0x00,0x00,0x16,0x00,0x00,
                                        0x2a,0x00,0x00,0x81,0x00,0x00,0x42,0x00,0x00,0x1e,0x00,0x00,0xe9,0x00,0x00,0x9d,0x00,0x00,0x33,0x00,0x00,0x56,0x00,0x00};
  u32 FileSize_DSBrickB=548673;
  unsigned char MD5Data_DSBrickB[16*3]={0x8e,0x00,0x00,0x7a,0x00,0x00,0x37,0x00,0x00,0x28,0x00,0x00,0x75,0x00,0x00,0x9d,0x00,0x00,0xf2,0x00,0x00,0x65,0x00,0x00,
                                        0xca,0x00,0x00,0x3a,0x00,0x00,0x78,0x00,0x00,0x55,0x00,0x00,0x3c,0x00,0x00,0xf2,0x00,0x00,0x7b,0x00,0x00,0xb8,0x00,0x00};
  
  fsize=FileSys_GetFileDataSize(FileIndex);
  
  {
    bool SkipFlag=true;
    
    if(fsize==FileSize_DSBrickA) SkipFlag=false;
    if(fsize==FileSize_DSBrickB) SkipFlag=false;
    
    if(SkipFlag==true) return;
  }
  
  inbuf=(u8*)malloc(bufsize);
  
  fhandle=FileSys_fopen(FileIndex);
  MD5Init(&MD5CTX);
  
  while(fsize!=0){
    u32 readsize=fsize;
    if(bufsize<readsize) readsize=bufsize;
    FileSys_fread(inbuf,readsize,1,fhandle);
    MD5Update(&MD5CTX,inbuf,readsize);
    fsize-=readsize;
    _consolePrintf("CreateMD5 lastbyte:%d\n",fsize);
  }
  
  MD5Final(MD5Data,&MD5CTX);
  FileSys_fclose(fhandle);
  
  free(inbuf); inbuf=NULL;
  
#undef bufsize

  for(u32 idx=0;idx<16;idx++){
    _consolePrintf("%02x",MD5Data[idx]);
  }
  
  fsize=FileSys_GetFileDataSize(FileIndex);
  
  {
    bool comp=true;
    
    for(int i=0;i<16;i++){
      if(MD5Data[i]!=MD5Data_DSBrickA[i*3]) comp=false;
    }
    if(comp==true){
      _consolePrintf("This NDS is 'Trojan.DSBrick.A'. Halt.\n");
      ShowLogHalt();
      while(1);
    }
  }
  
  {
    bool comp=true;
    
    for(int i=0;i<16;i++){
      if(MD5Data[i]!=MD5Data_DSBrickB[i*3]) comp=false;
    }
    if(comp==true){
      _consolePrintf("This NDS is 'Trojan.DSBrick.B'. Halt.\n");
      ShowLogHalt();
      while(1);
    }
  }
}

extern "C" {
  bool FAT_FreeFiles (void);
}

#include "mediatype.h"
#include "plug_ndsrom_gbamp.h"
#include "plug_ndsrom_resetmem.h"
#include "plug_ndsrom_m3.h"
#include "plug_ndsrom_sc.h"
#include "plug_ndsrom_ez4.h"

bool BootNDSROM(u32 FileIndex)
{
  videoSetModeSub_SetShowLog(true);
  
  if(GlobalINI.NDSROMPlugin.CrashMeProtection==true) chkmd5(FileIndex);
  
  if(FileSys_GetVolumeType()!=VT_MPCF){
    _consolePrintf("It is not possible to start excluding FAT.\n");
    return(false);
  }
  
  switch(DIMediaType){
    case DIMT_NONE: break;
    case DIMT_M3CF: lp_boot_M3(FileIndex); break;
    case DIMT_M3SD: lp_boot_M3(FileIndex); break;
    case DIMT_MPCF: lp_boot_GBAMP(FileIndex); break;
    case DIMT_MPSD: break;
    case DIMT_SCCF: lp_boot_SC(FileIndex); break;
    case DIMT_SCSD: lp_boot_SC(FileIndex); break; // seems to work fine with SCSD as well
    case DIMT_FCSR: break;
    case DIMT_NMMC: break;
//    case DIMT_MPMD: lp_boot_GBAMP(FileIndex); break;
    case DIMT_EZSD: {
      if(GlobalINI.System.NDSROMBootFunctionWithEZ4SD==true) lp_boot_EZ4(FileIndex);
    } break;
    case DIMT_MMCF: break;
    case DIMT_SCMS: lp_boot_SC(FileIndex); break; // switch test for MicroSD
    case DIMT_EWSD: break;
    case DIMT_NJSD: break;
    case DIMT_DLMS: break;
    case DIMT_G6FC: break;
  }
  
  videoSetModeSub_SetShowLog(true);
  
  _consolePrintf("not support adapter type.\n");
  
  {
    extern void ScanIPC3(bool useStack);
    extern void WaitKeyClear(bool SoundUpdateFlag);
    extern u16 KEYS_CUR;
    
    ScanIPC3(true);
    while(KEYS_CUR==0){
      swiWaitForVBlank();
      ScanIPC3(true);
    }
    
    WaitKeyClear(false);
  }
  
  videoSetModeSub_SetShowLog(false);
  
  return(false);
}

#endif
