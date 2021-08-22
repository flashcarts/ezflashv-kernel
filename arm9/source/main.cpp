
#include <NDS.h>
#include <NDS/ARM9/CP15.h>

#include "_console.h"
#include "_consoleWriteLog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memtool.h"
#include "strtool.h"

#include "maindef.h"
#include "dmalink.h"

#include "glib/glib.h"

#include "../../ipc3.h"
#include "arm9tcm.h"

#include "filesys.h"
#include "shell.h"
#include "cstream_fs.h"
#include "thumb.h"

#include "gba_nds_fat.h"
#include "mediatype.h"
#include "directdisk.h"

#include "cimfs.h"

#include "_const.h"
#include "inifile.h"

#include "plugin/plug_ndsrom.h"
#include "plugin/plug_fpk.h"
#include "plugin/plug_text.h"
#include "plugin/plug_dpg.h"
#include "plugin/plug_mp2.h"
#include "plugin/plug_gmensf.h"
#include "plugin/plug_gmegbs.h"

#include "VRAMTool.h"
#include "FontPro.h"
#include "mwin.h"
#include "mwin_color.h"
#include "imgcalc.h"

#include "strpcm.h"

#include "Emulator.h"

#include "dll.h"

#include "formdt.h"

#include "resume.h"
#include "plugin/plug_text_bookmark.h"

#include "main_extforresume.h"

#include "zlibhelp.h"

#include "linkreset_arm9.h"
#include "dsCard.h"
#include "ez5_language.h"
#include "cardsaver.h"

static volatile bool strpcmDoubleSpeedFlag=false;

static EExecMode LastExecMode=EM_None;
EExecMode ExecMode=EM_None;

static bool ShowLog;

u32 NDSLite_Brightness;

TPluginBody *pPluginBody=NULL;

static TPluginBody *pPluginBodyClock;
static TPlugin_ClockLib_Time CurrentTime;
static bool PluginBodyClock_RequestRefresh;
static bool PluginBodyClock_Execute;

static int BacklightOffVsyncCount=0;

//*********aladdin add **************************/
#ifdef __cplusplus
extern "C" {
#endif
extern void DS_Read(uint32 add,uint8* buf);
#ifdef __cplusplus
}
#endif
// -----------------------------------------------

extern volatile u32 reqflip; // set from dpg-plugin

volatile u32 reqflip=false;

void InterruptHandler_Vsync_setfb(bool flip,int bl)
{
  if(flip==true) pScreenMain->Flip(false);
  pScreenMain->SetBlendLevel(bl);
}

CODE_IN_ITCM void InterruptHandler_Vsync(void)
{
  extern u16 KEYS_Stack;
  KEYS_Stack|=(~REG_KEYINPUT)&0x3ff;
  
  static void (*_InterruptHandler_Vsync_setfb)(bool flip,int bl)=InterruptHandler_Vsync_setfb;
  
/*
  if(IPC3->R2YREQ==R2YREQ_Transfer){
    DMA1_SRC = (u32)IPC3->R2Y_pFrameBuf;
    DMA1_DEST = (u32)IPC3->R2Y_pVRAM;
    DMA1_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | ((256*192*2)>>1);
    reqflip=true;
    IPC3->R2YREQ=R2YREQ_NULL;
  }
*/
  
  if(reqflip!=0){
    if(reqflip==3){
      _InterruptHandler_Vsync_setfb(true,6);
      }else{
      if(reqflip==2){
        _InterruptHandler_Vsync_setfb(false,11);
        }else{
        _InterruptHandler_Vsync_setfb(false,16);
      }
    }
    reqflip--;
  }
}

bool strpcmUpdate_mainloop(void)
{ cwl();
#ifdef notuseSound
  return(false);
#endif

  if(ExecMode==EM_DPG) return(false);
  
  u32 BaseSamples=IPC3->strpcmSamples;
  u32 Samples=0;
  
  REG_IME=0;
  
  u32 CurIndex=(strpcmRingBufWriteIndex+1) & strpcmRingBufBitMask;
  u32 PlayIndex=strpcmRingBufReadIndex;
  bool EmptyFlag;
  
  EmptyFlag=strpcmRingEmptyFlag;
  strpcmRingEmptyFlag=false;
  
  REG_IME=1;
  
  if(CurIndex==PlayIndex) return(false);
  
  if(EmptyFlag==true){ cwl();
    _consolePrintf("strpcm:CPU overflow.\n");
  }
  
  if((strpcmRingLBuf==NULL)||(strpcmRingRBuf==NULL)) return(false);
  
  s16 *ldst=&strpcmRingLBuf[BaseSamples*CurIndex];
  s16 *rdst=&strpcmRingRBuf[BaseSamples*CurIndex];
  
  if(strpcmRequestStop==true){ cwl();
    Samples=0;
    }else{ cwl();
    
    switch(ExecMode){ cwl();
      case EM_MSPSound: case EM_MP3Boot: { cwl();
        if(strpcmDoubleSpeedFlag==true){ cwl();
          pPluginBody->pSL->Update(NULL,NULL);
        }
        Samples=pPluginBody->pSL->Update(ldst,rdst);
      } break;
      case EM_GMENSF: {
        if(strpcmDoubleSpeedFlag==true){ cwl();
          UpdateGMENSF(NULL,NULL);
        }
        Samples=UpdateGMENSF(ldst,rdst);
      } break;
      case EM_GMEGBS: {
        if(strpcmDoubleSpeedFlag==true){ cwl();
          UpdateGMEGBS(NULL,NULL);
        }
        Samples=UpdateGMEGBS(ldst,rdst);
      } break;
/*
      case EM_GMEVGM: {
        if(strpcmDoubleSpeedFlag==true){ cwl();
          UpdateGMEVGM(NULL,NULL);
        }
        Samples=UpdateGMEVGM(ldst,rdst);
      } break;
      case EM_GMEGYM: {
        if(strpcmDoubleSpeedFlag==true){ cwl();
          UpdateGMEGYM(NULL,NULL);
        }
        Samples=UpdateGMEGYM(ldst,rdst);
      } break;
*/
      case EM_DPG: { cwl();
        Samples=UpdateDPG_Audio(&ldst[0],&rdst[0]);
      } break;
      default: {
        Samples=0;
      } break;
    }
    if(Samples!=BaseSamples) strpcmRequestStop=true;
  }
  
  if(Samples<BaseSamples){ cwl();
    for(u32 idx=Samples;idx<BaseSamples;idx++){ cwl();
      ldst[idx]=0;
      rdst[idx]=0;
    }
  }
  
  REG_IME=0;
  strpcmRingBufWriteIndex=CurIndex;
  REG_IME=1;
  
  if(Samples==0) return(false);
  
  return(true);
}

// ------------------------------------------------------------------

bool isExecMode_DPG(void)
{
  if(ExecMode==EM_DPG){
    return(true);
    }else{
    return(false);
  }
}

#include "main_pluginfo.h"

#include "main_pic.h"

#include "main_fs.h"

// ------------------------------------------------------------------

static bool MP3Boot_PlayFileFromShell(char *mp3fn)
{ cwl();
  u32 DeflateSize=0;
  u8 *DeflateBuf=NULL;
  
  Shell_ReadFile(mp3fn,(void**)&DeflateBuf,(int*)&DeflateSize);
  if((DeflateBuf==NULL)||(DeflateSize==0)){
    _consolePrintf("shell/%s file not found.\n",mp3fn);
    return(false);
  }
  
  FileHandle=FileSys_fopen_DirectMapping(VT_IMFS,true,DeflateBuf,DeflateSize,0,0);
  
  if(FileHandle==0){ cwl();
    _consolePrintf("fopen_DirectMapping error.\n");
    return(false);
  }
  
  {
    {
      char fn[PluginFilenameMax];
      if(DLLList_GetPluginFilename(".mp3",fn)!=EPT_Sound){
        _consolePrintf("not found mp3 plugin.\n");
        return(false);
      }
      pPluginBody=DLLList_LoadPlugin(fn);
    }
    if(pPluginBody->pSL->Start(FileHandle)==false){ cwl();
      _consolePrintf("mp3 plugin start error.\n");
      return(false);
    }
    ExecMode=EM_MP3Boot;
    strpcmStart(false,pPluginBody->pSL->GetSampleRate(),pPluginBody->pSL->GetSamplePerFrame(),pPluginBody->pSL->GetChannelCount(),SPF_PCMx4);
  }
  
  while(strpcmUpdate_mainloop()==true){ cwl();
  }
  
  return(true);
}

// ------------------------------------------------------------------

bool RefreshBacklightOffCount(void)
{
  TiniBacklightTimeout *BacklightTimeout=&GlobalINI.BacklightTimeout;
  int vsec=0;
  bool eclk=false;
  
  switch(ExecMode){
    case EM_DPG: {
      vsec=0;
      eclk=false;
    } break;
    case EM_None: case EM_MP3Boot: case EM_NDSROM: case EM_FPK: {
      vsec=BacklightTimeout->WhenStandby;
      eclk=true;
    } break;
    case EM_MSPImage: {
      vsec=BacklightTimeout->WhenPicture;
      eclk=false;
    } break;
    case EM_Text: {
      vsec=BacklightTimeout->WhenText;
      eclk=true;
    } break;
    case EM_MSPSound: case EM_GMENSF: case EM_GMEGBS: { // case EM_GMEVGM: case EM_GMEGYM: {
      vsec=BacklightTimeout->WhenSound;
      eclk=true;
    } break;
/*
    default: {
      vsec=0;
    }
*/
  }
  
  bool res=false;
  if(vsec!=0){
    if(BacklightOffVsyncCount==0){
      if(PluginBodyClock_Execute==true) MWin_TransWindow(WM_FileSelect);
      if(ExecMode==EM_Text){
        switch(GlobalINI.TextPlugin.SelectDisplay){
          case EITPSD_Bottom: IPC3->LCDPowerControl=LCDPC_ON_BOTTOM; break;
          case EITPSD_Top: IPC3->LCDPowerControl=LCDPC_ON_TOP_LEDON; break;
          default: break;
        }
        }else{
        IPC3->LCDPowerControl=LCDPC_ON_BOTH;
      }
      if(GlobalINI.System.FileSelectSubScreen==true){
        PluginBodyClock_Execute=false;
        }else{
        if(pPluginBodyClock==NULL){
          PluginBodyClock_Execute=false;
          }else{
          PluginBodyClock_Execute=eclk;
        }
      }
      res=true;
    }
  }
  
  if(vsec==0){
    BacklightOffVsyncCount=0;
    }else{
    BacklightOffVsyncCount=vsec*60;
  }
  return(res);
}

void ProcBacklightOffCount(u32 VsyncCount)
{
  if(BacklightOffVsyncCount==0) return;
  
  bool eclk=false;
  
  switch(ExecMode){
    case EM_DPG: {
      eclk=false;
    } break;
    case EM_None: case EM_MP3Boot: case EM_NDSROM: case EM_FPK: {
      eclk=true;
    } break;
    case EM_MSPImage: {
      eclk=false;
    } break;
    case EM_Text: {
      eclk=true;
    } break;
    case EM_MSPSound: case EM_GMENSF: case EM_GMEGBS: { // case EM_GMEVGM: case EM_GMEGYM: {
      eclk=true;
    } break;
/*
    default: {
      vsec=0;
    }
*/
  }
  
  if(BacklightOffVsyncCount<=(s32)VsyncCount){
    BacklightOffVsyncCount=0;
    }else{
    BacklightOffVsyncCount-=(s32)VsyncCount;
  }
  
  if(BacklightOffVsyncCount==0){
    if((eclk==false)||(pPluginBodyClock==NULL)){
      IPC3->LCDPowerControl=LCDPC_OFF_BOTH;
      PluginBodyClock_Execute=false;
      }else{
      IPC3->LCDPowerControl=LCDPC_ON_TOP_LEDBLINK;
      PluginBodyClock_Execute=true;
      PluginBodyClock_RequestRefresh=true;
    }
  }
}

void videoSetModeSub_SetShowLog(bool e)
{
  ShowLog=e;
  
  bool ebg=false;
  
  if(e==false){
    if(GlobalINI.System.FileSelectSubScreen==true) ebg=true;
    if(pPluginBodyClock!=NULL) ebg=true;
  }
  
  if(ebg==false){
    videoSetModeSub(MODE_2_2D | DISPLAY_BG2_ACTIVE);
    }else{
    videoSetModeSub(MODE_2_2D | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D | DISPLAY_SPR_2D_BMP_256);
  }
}

void ShowLogHalt(void)
{ cwl();
  REG_IME=0;
  
  videoSetModeSub_SetShowLog(true);
  IPC3->LCDPowerControl=LCDPC_ON_BOTH;
  _consolePrintf("\n");
  while(1){ cwl();
    swiWaitForVBlank();
  }
}

extern u32 __dtcm_lma;
extern u32 __dtcm_start;
extern u32 __dtcm_end;

extern u32 __itcm_lma;
extern u32 __itcm_start;
extern u32 __itcm_end;

#define	REG_WRAMCNT	(*(vu8*) 0x4000247)

static void ResetMemory(void)
{
  // 0x04000247 - REG_WRAMCNT - (Shared) WRAM Control Register (R/W)
  // 3=Bank0 and Bank1 to ARM7
  REG_WRAMCNT=3;
  
/*
  // FlashMeV6 NDS-V3
  CP15_SetID          (0x41059461);
  CP15_SetCacheType   (0x0f0d2112);
  CP15_SetTCMSize     (0x00140180);
  CP15_SetControl     (0x0005307d);
  CP15_SetDataCachable(0x00000042);
  CP15_SetInstructionCachable(0x00000042);
  CP15_SetDataBufferable(0x00000006);
  CP15_SetDataPermissions(0x36333333);
  CP15_SetInstructionPermissions(0x36636333);
  CP15_SetRegion0(0x04000033);
  CP15_SetRegion1(0x0200002b);
  CP15_SetRegion2(0x037f801d);
  CP15_SetRegion3(0x08000035);
  CP15_SetRegion4(0x0080001b);
  CP15_SetRegion5(0x0000001d);
  CP15_SetRegion6(0xffff001d);
  CP15_SetRegion7(0x0240002b);
  CP15_SetDCacheLockdown(0x00000000);
  CP15_SetICacheLockdown(0x00000000);
  CP15_SetDTCM(0x0080000a);
  CP15_SetITCM(0x0000000c);
  CPSR=6000001f
  
  // NoFlashMe NDS-V2
  CP15_SetID          (0x41059461);
  CP15_SetCacheType   (0x0f0d2112);
  CP15_SetTCMSize     (0x00140180);
  CP15_SetControl     (0x0005307d);
  CP15_SetDataCachable(0x00000042);
  CP15_SetInstructionCachable(0x00000042);
  CP15_SetDataBufferable(0x00000006);
  CP15_SetDataPermissions(0x36333333);
  CP15_SetInstructionPermissions(0x36636333);
  CP15_SetRegion0(0x04000033);
  CP15_SetRegion1(0x0200002b);
  CP15_SetRegion2(0x037f801d);
  CP15_SetRegion3(0x08000035);
  CP15_SetRegion4(0x0080001b);
  CP15_SetRegion5(0x0000001d);
  CP15_SetRegion6(0xffff001d);
  CP15_SetRegion7(0x0240002b);
  CP15_SetDCacheLockdown(0x00000000);
  CP15_SetICacheLockdown(0x00000000);
  CP15_SetDTCM(0x0080000a);
  CP15_SetITCM(0x00000020);
  
  // FlashMe M3SD-J15b
  CP15_SetID          (0x41059461);
  CP15_SetCacheType   (0x0f0d2112);
  CP15_SetTCMSize     (0x00140180);
  CP15_SetControl     (0x0005307d);
  CP15_SetDataCachable(0x00000042);
  CP15_SetInstructionCachable(0x00000042);
  CP15_SetDataBufferable(0x00000006);
  CP15_SetDataPermissions(0x36333333);
  CP15_SetInstructionPermissions(0x36636333);
  CP15_SetRegion0(0x04000033);
  CP15_SetRegion1(0x0200002b);
  CP15_SetRegion2(0x037f801d);
  CP15_SetRegion3(0x08000035);
  CP15_SetRegion4(0x0080001b);
  CP15_SetRegion5(0x0000001d);
  CP15_SetRegion6(0xffff001d);
  CP15_SetRegion7(0x0240002b);
  CP15_SetDCacheLockdown(0x00000000);
  CP15_SetICacheLockdown(0x00000000);
  CP15_SetDTCM(0x0080000a);
  CP15_SetITCM(0x0000000c);
  CPSR=6000001f
*/
  
  CP15_SetITCM(0x0000000c);
  {
    u32 itcmsize=(u32)&__itcm_end-(u32)&__itcm_start;
    
    volatile u32 *srcptr=(vu32*)((u32)&__itcm_lma);
    volatile u32 *dstptr=(vu32*)(0x00000000);
    
    for(u32 i=0;i<itcmsize;i+=4){
      *dstptr++=*srcptr++;
    }
  }
  
  {
    u32 dtcmsize=(u32)&__dtcm_end-(u32)&__dtcm_start;
    
    volatile u32 *srcptr=(vu32*)((u32)&__dtcm_lma);
    volatile u32 *dstptr=(vu32*)(0x00800000);
    
    for(u32 i=0;i<dtcmsize;i+=4){
      *dstptr++=*srcptr++;
    }
  }
}

bool chkITCM(int line)
{return(true);
  u32 itcmsize=(u32)&__itcm_end-(u32)&__itcm_start;
  
  volatile u32 *srcptr=(vu32*)((u32)&__itcm_lma);
  volatile u32 *dstptr=(vu32*)(0x00000000);
  
  for(u32 i=0;i<itcmsize/4;i++){
    if(*srcptr!=*dstptr){
      _consolePrintf("%d:ITCM image error at 0x%x(%x)->0x%x(%x)\n",line,(u32)srcptr,*srcptr,(u32)dstptr,*dstptr);
      ShowLogHalt();
      return(false);
    }
    dstptr++; srcptr++;
  }
  
  return(true);
}

bool chkDTCM(int line)
{
  u32 dtcmsize=(u32)&__dtcm_end-(u32)&__dtcm_start;
  
  volatile u32 *srcptr=(vu32*)((u32)&__dtcm_lma);
  volatile u32 *dstptr=(vu32*)(0x00800000);
  
  for(u32 i=0;i<dtcmsize/4;i++){
    if(*srcptr!=*dstptr){
      _consolePrintf("%d:DTCM image error at 0x%x->0x%x\n",line,(u32)srcptr,(u32)dstptr);
      ShowLogHalt();
      return(false);
    }
    dstptr++; srcptr++;
  }
  
  return(true);
}

extern "C" {
  void CheckTCM(int line);
}

void CheckTCM(int line)
{
  if(false){
    int sp;
    asm("mov %0, sp":"=r"(sp));
    _consolePrintf("sp%8x\n",sp);
  }
  
  chkITCM(line);
//  chkDTCM(line);
}

static void _test_ShowCP15(void)
{
  _consolePrintf("%8x\n",CP15_GetID());
  _consolePrintf("%8x\n",CP15_GetCacheType());
  _consolePrintf("%8x\n",CP15_GetTCMSize());
  _consolePrintf("%8x\n",CP15_GetControl());
  _consolePrintf("%8x\n",CP15_GetDataCachable());
  _consolePrintf("%8x\n",CP15_GetInstructionCachable());
  _consolePrintf("%8x\n",CP15_GetDataBufferable());
  _consolePrintf("%8x\n",CP15_GetDataPermissions());
  _consolePrintf("%8x\n",CP15_GetInstructionPermissions());
  _consolePrintf("%8x\n",CP15_GetRegion0());
  _consolePrintf("%8x\n",CP15_GetRegion1());
  _consolePrintf("%8x\n",CP15_GetRegion2());
  _consolePrintf("%8x\n",CP15_GetRegion3());
  _consolePrintf("%8x\n",CP15_GetRegion4());
  _consolePrintf("%8x\n",CP15_GetRegion5());
  _consolePrintf("%8x\n",CP15_GetRegion6());
  _consolePrintf("%8x\n",CP15_GetRegion7());
  _consolePrintf("%8x\n",CP15_GetDCacheLockdown());
  _consolePrintf("%8x\n",CP15_GetICacheLockdown());
  _consolePrintf("%8x\n",CP15_GetDTCM());
  _consolePrintf("%8x\n",CP15_GetITCM());
  
  int a;
  
  asm("mrs %0, cpsr":"=r"(a));
  
  _consolePrintf("cpsr=%8x\n",a);
  
  while(1);
}

void NDSLite_SetBrightness(u32 bright)
{
  if(IPC3->isNDSLite==false) return;
  
  IPC3->Brightness=bright;
}

// ------------------------------------------------------

#include "setarm9_reg_waitcr.h"
#include "gba_nds_fat.h"

u8 OpenSaveTypeFile()
{
	FAT_FILE *fh=NULL;
	int size;
	fh = FAT_fopen("/shell/ezsave.lst","r");
	if(fh==NULL)
	{
		//_consolePrintf('here\n");
		return 0;
	}
	FAT_fseek(fh,0,SEEK_END);
	gl_SizeTypeFileSize=FAT_ftell(fh);
	FAT_fseek(fh,0,SEEK_SET);
	gl_pSaveTypeBuf=(u8*)safemalloc(gl_SizeTypeFileSize);
	if(gl_pSaveTypeBuf==NULL)
	{
		FAT_fclose(fh);
		return 0;
	}
	FAT_fread(gl_pSaveTypeBuf,gl_SizeTypeFileSize,1,fh);
	FAT_fclose(fh);
	return 1;
}

u8 WriteSaveTypeFile()
{
	FAT_FILE *fh=NULL;
	int size;
	fh = FAT_fopen("/shell/ezsave.lst","w");
	if(fh==NULL)
		return 0;
	FAT_fwrite(gl_pSaveTypeBuf,gl_SizeTypeFileSize,1,fh);
	FAT_fclose(fh);
	return 1;
}

static void FATTest(void)
{
  SetARM9_REG_WaitCR();
  FAT_InitFiles();
  
  FAT_CWD("/");
  
  char fn[MAX_FILENAME_LENGTH];
  u32 FAT_FileType;
  
  FAT_FileType=FAT_FindFirstFile(fn);
  
  while(FAT_FileType!=0){ cwl();
    if((strcmp(fn,".")!=0)&&(strcmp(fn,"..")!=0)){ cwl();
      _consolePrintf("fn(%d)=%s\n",FAT_FileType,fn);
      
      UnicodeChar lfn[256];
      
      if(FAT_GetLongFilenameUnicode(lfn,IMFS_FilenameLengthMax-4)==false){ cwl();
        StrConvert_Local2Unicode(fn,lfn);
      }
      
    }
    
    FAT_FileType=FAT_FindNextFile(fn);
  }
  
  {
    u16 KEYS_CUR;
    
    KEYS_CUR=0;
    while(KEYS_CUR==0){ cwl();
      KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
    }
  }
  
  FAT_FreeFiles();
}

// ------------------------------------------------------

u32 SetARM9_REG_ROM1stAccessCycleControl,SetARM9_REG_ROM2stAccessCycleControl;

void main_ShowWarn(void)
{
  u32 UserLanguage=(u32)-1;
  u32 Timeout=0x10000;
  
  while(UserLanguage==(u32)-1){
    UserLanguage=IPC3->UserLanguage;
    Timeout--;
    if(Timeout==0){
      _consolePrintf("NDS farmware language read error. ARM7CPU stopped...?\n");
      while(1);
    }
  }
  
  _consolePrintf("NDS farmware language ID : %d\n",UserLanguage);
  
  s32 PathIndex=pIMFS->GetPathIndex("/shell");
  if(PathIndex==-1){
    _consolePrintf("can not found IMFS /shell.\n");
    while(1);
  }
  
  char *pfn;
  
  switch(UserLanguage){
    case 1: pfn="warn_eng.b15"; break;
    case 2: pfn="warn_fre.b15"; break;
    case 4: pfn="warn_ita.b15"; break;
    case 3: pfn="warn_deu.b15"; break;
    case 5: pfn="warn_esp.b15"; break;
    case 0: pfn="warn_jpn.b15"; break;
    default: pfn="warn_eng.b15"; break;
  }
  
  _consolePrintf("warn image filename [%s]\n",pfn);
  
  s32 FileIndex=pIMFS->GetIndexFromFilename(PathIndex,pfn);
  
  if(FileIndex==-1){
    pfn="warn_eng.b15";
    _consolePrintf("change to [%s]\n",pfn);
    FileIndex=pIMFS->GetIndexFromFilename(PathIndex,pfn);
  }
  
  u32 FileSize;
  u8 *FileBuf;
  
  FileSize=pIMFS->GetFileDataSizeFromIndex(PathIndex,FileIndex);
  FileBuf=(u8*)safemalloc(FileSize);
  pIMFS->GetFileDataFromIndex(PathIndex,FileIndex,FileBuf);
  
  if((FileSize==0)||(FileBuf==NULL)){
    _consolePrintf("can not found image.\n");
    while(1);
  }
  
  {
    CglB15 glB15(FileBuf,FileSize);
//    glB15.BitBlt(pScreenMainOverlay->pCanvas,0,0,glB15.GetWidth(),glB15.GetHeight(),0,0);
    MemCopy16DMA3(glB15.pCanvas->GetScanLine(0),pScreenMainOverlay->pCanvas->GetScanLine(0),ScreenHeight*ScreenWidth*2);
  }
  
  safefree(FileBuf); FileBuf=NULL;
}

#ifdef ShowDebugMsg
#else
static void _consolePrint_dummy(const char* s)
{
}
#endif

int main(void)
{
  ResetMemory();
  
  REG_IME=0;
  
  POWER_CR = POWER_ALL_2D;
  
  videoSetModeSub_SetShowLog(false);
  
#ifdef ShowDebugMsg
  glSetFuncDebugPrint(_consolePrint);
#else
  glSetFuncDebugPrint(_consolePrint_dummy);
#endif
  glDefaultMemorySetting();
  
/*
  {
    SUB_BG2_CR = BG_32x32 | BG_MAP_BASE(8) | BG_TILE_BASE(0) | BG_PRIORITY_0;
    BG_PALETTE_SUB[255] = RGB15(31,31,31);//by default font will be rendered with color 255
    
    //consoleInit() is a lot more flexible but this gets you up and running quick
    _consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(8), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);
    _consoleClear();
    _consolePrintSet(0,0);
  }
*/
  {
    SUB_BG2_CR = BG_256_COLOR | BG_RS_64x64 | BG_MAP_BASE(8) | BG_TILE_BASE(0) | BG_PRIORITY_0; // Tile16kb Map2kb(64x32)
    
    BG_PALETTE_SUB[(0*16)+0] = RGB15(0,0,0); // unuse (transparent)
    BG_PALETTE_SUB[(0*16)+1] = RGB15(0,0,8) | BIT(15); // BG color
    BG_PALETTE_SUB[(0*16)+2] = RGB15(0,0,0) | BIT(15); // Shadow color
    BG_PALETTE_SUB[(0*16)+3] = RGB15(31,31,31) | BIT(15); // Text color
    
    u16 XDX=(u16)((8.0/6)*0x100);
    u16 YDY=(u16)((8.0/6)*0x100);
    
    SUB_BG2_XDX = XDX;
    SUB_BG2_XDY = 0;
    SUB_BG2_YDX = 0;
    SUB_BG2_YDY = YDY;
    
    SUB_BG2_CX=-1;
    SUB_BG2_CY=-1;
    
    //consoleInit() is a lot more flexible but this gets you up and running quick
    _consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(8), (u16*)CHAR_BASE_BLOCK_SUB(0), 256);
    _consoleClear();
    _consolePrintSet(0,0);
  }
  
  glDefaultClassCreate();
  
#ifdef EmulatorDebug
  _consolePrintf("#define EmulatorDebug\n\n");
#endif
  
  _consolePrintf("boot %s\n%s\n%s\n\n",ROMTITLE,ROMVERSION,ROMDATE);
  
  _consolePrintf("ITCM0x%x,0x%x,0x%x\n",(u32)&__itcm_lma,(u32)&__itcm_start,(u32)&__itcm_end-(u32)&__itcm_start);
  _consolePrintf("DTCM0x%x,0x%x,0x%x\n",(u32)&__dtcm_lma,(u32)&__dtcm_start,(u32)&__dtcm_end-(u32)&__dtcm_start);
  _consolePrintf("\n");
  CheckTCM(__LINE__);
 
//  FATTest();
  
//  _test_ShowCP15(); while(1);
  
  strpcmSetVolume16(16);
  
  DMALinkInit();
  
  IPC3->strpcmLBuf=NULL;
  IPC3->strpcmRBuf=NULL;
  
  pIMFS=new CIMFS();
  if(pIMFS->InitIMFS()==false) while(1);
  
  //main_ShowWarn();
  
  pEXFS=new CIMFS();
  if(pEXFS->InitEXFS()==false){
    delete pEXFS; pEXFS=NULL;
  }
  
  
  _consolePrintf("Init FileSystem.\n");
  FileSys_Init(256);
  
#ifdef DisableAutoDetect
  Shell_AutoDetect_EXFS();
#else
  Shell_AutoDetect();
#endif
  
  if(Shell_FindShellPath()==false) while(1);
  
  if(pEXFS!=NULL){
    DD_Init(EDDST_CART);
    }else{
    DD_Init(EDDST_FAT);
//    DD_Init(EDDST_CART);
  }
  Resume_Init();
  Bookmark_Init();
 
  { cwl();
    u8 *pank,*pl2u,*pfon;
    int dummysize;
    
    _consolePrintf("load [system.ank]\n");
    Shell_ReadFile("system.ank",(void**)&pank,&dummysize);
    _consolePrintf("load [system.l2u]\n");
    Shell_ReadFile("system.l2u",(void**)&pl2u,&dummysize);
    _consolePrintf("load [system.fon]\n");
    Shell_ReadFile("system.fon",(void**)&pfon,&dummysize);
    
    if((pank==NULL)||(pl2u==NULL)||(pfon==NULL)){ cwl();
      _consolePrintf("notfound '/shell/system.ank|.l2u|.fon'\n\n");
      _consolePrintf("'ChangeCodePage/cp(your codepage).bat' is executed. When GBFS is used, 'WriteFiles_gbfs.bat' is executed.\n\n");
      _consolePrintf("Please refer to 'ChangeCodePage/codepage.txt' for CodePage.\n\n");
      ShowLogHalt();
    }
    
    Unicode_Init(pank,pl2u);
    FontPro_Init(pfon);
  }
  if(OpenSaveTypeFile()==0)
  {

	 _consolePrintf("\\n");
  	_consolePrintf("Can't find 'Shell' folder on the MicroSD.\n");
  	_consolePrintf("Loader is running in NOSKIN mode.\n");
	_consolePrintf("Please visit http://www.ezflash.cn to download necessary software.\n");
	_consolePrintf("Press A to continue.....\n");
	while(1){cwl()
		scanKeys();
		if(keysUp()&KEY_A)
		{
			break;
		}
	}
  } 
  { cwl();
    static volatile signed char sc=-1;
    static volatile unsigned char uc=-1;
    static volatile char c=-1;
    
    if((s32)c==(s32)sc) _consolePrintf("Default char is signed char.\n");
    if((s32)c==(s32)uc) _consolePrintf("Default char is unsigned char.\n");
  }
  
  
  InitINI();
  LoadINI("global.ini");
  LoadINI("skin.ini");
  
  {
    TiniForSuperCard *ForSuperCard=&GlobalINI.ForSuperCard;
    SetARM9_REG_ROM1stAccessCycleControl=ForSuperCard->ROM1stAccessCycleControl;
    SetARM9_REG_ROM2stAccessCycleControl=ForSuperCard->ROM2stAccessCycleControl;
    SetARM9_REG_WaitCR();
  }
  
  DLLList_Init();
  
  strpcmSetVolume16(GlobalINI.System.SoundVolume);
  
  _consolePrintf("Resize FileSystem.\n");
  FileSys_Init(GlobalINI.System.FileMaxCount);
  
  u32 Brightness=GlobalINI.System.NDSLiteDefaultBrightness;
  if(Brightness==4){
    Brightness=IPC3->DefaultBrightness;
  }
  NDSLite_Brightness=Brightness;
  NDSLite_SetBrightness(NDSLite_Brightness);
  
  switch(GlobalINI.System.WhenPanelClose){
    case EISWPC_BacklightOff: IPC3->WhenPanelClose=false; break;
    case EISWPC_DSPowerOff: IPC3->WhenPanelClose=true; break;
    case EISWPC_PlayShutdownSound: IPC3->WhenPanelClose=true; break;
  }
  
  formdt_SetFormatDateStr(GlobalINI.ClockPlugin.FormatDate);
  
  switch(GlobalINI.System.TopScreenFlip){
    case ESF_Normal: {
      pScreenSub->SetFlipMode(0);
    } break;
    case ESF_Flip: {
      pScreenSub->SetFlipMode(1);
    } break;
    case ESF_VFlip: {
      pScreenSub->SetFlipMode(2);
    } break;
    case WFS_HFlip: {
      pScreenSub->SetFlipMode(3);
    } break;
  }
  
  {
    extern void mainloop(void);
    mainloop();
  }
  
  glDefaultClassFree();
  
  ShowLogHalt();
}

// ---------------------------------------

static bool tpDragFlag=false;
static bool tpIgnore=false;

static TransferRegion3 RIPC3;

static u16 KEYS_Stack=0;

u16 KEYS_CUR=0;

static void cartSetMenuMode(void)
{
  *(vu16*)(0x04000204) &= ~0x0880;    //sysSetBusOwners(true, true);

  *((vu32*)0x027FFFF8) = 0x080000C0; // ARM7 reset address
}

static void cartSetMenuMode_MPCF(void)
{
  cartSetMenuMode();
}

static void cartSetMenuMode_EZSD(void)
{
  cartSetMenuMode();
}

static void cartSetMenuMode_M3CFSD(void)
{
  cartSetMenuMode();

  u32 mode = 0x00400004;
  vu16 tmp;
  tmp = *(vu16*)(0x08E00002);
  tmp = *(vu16*)(0x0800000E);
  tmp = *(vu16*)(0x08801FFC);
  tmp = *(vu16*)(0x0800104A);
  tmp = *(vu16*)(0x08800612);
  tmp = *(vu16*)(0x08000000);
  tmp = *(vu16*)(0x08801B66);
  tmp = *(vu16*)(0x08000000 + (mode << 1)); 
  tmp = *(vu16*)(0x0800080E);
  tmp = *(vu16*)(0x08000000);

  tmp = *(vu16*)(0x080001E4);
  tmp = *(vu16*)(0x080001E4);
  tmp = *(vu16*)(0x08000188);
  tmp = *(vu16*)(0x08000188);
}

static void cartSetMenuMode_SCCFSD(void)
{
  cartSetMenuMode();

  *(vu16*)0x09FFFFFE = 0xA55A;
  *(vu16*)0x09FFFFFE = 0xA55A;
  *(vu16*)0x09FFFFFE = 0;
  *(vu16*)0x09FFFFFE = 0;
  *((vu32*)0x027FFFF8) = 0x08000000; // Special ARM7 reset address
}

void ScanIPC3(bool useStack)
{ cwl();
#ifdef notuseIPCKey
  RIPC3=*(TransferRegion3*)IPC3;
  
  RIPC3.touchXpx=IPC->touchXpx;
  RIPC3.touchYpx=IPC->touchYpx;
  RIPC3.buttons=IPC->buttons;
  
  KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
//  KEYS_CUR|=((~RIPC3.buttons)&3)<<10;
//  KEYS_CUR|=((~RIPC3.buttons)<<6) & KEY_TOUCH;
  
  swiWaitForVBlank();
  VsyncPassedCount=1;
  
  return;
#endif
  
  if(ExecMode==EM_DPG){ cwl();
    IPC3->ReqVsyncUpdate=2;
    }else{ cwl();
    IPC3->ReqVsyncUpdate=1;
    while(IPC3->ReqVsyncUpdate!=0){ cwl();
      swiDelay(1);
    }
  }
  
  MemCopy32DMA3((void*)IPC3,&RIPC3,sizeof(TransferRegion)+4); // 奼挘晹暘偼梫傜側偄
  
  KEYS_CUR=0;
  
  REG_IME=0;
  if(useStack==true) KEYS_CUR=KEYS_Stack;
  KEYS_Stack=0;
  REG_IME=1;
  
  u32 btns=~RIPC3.buttons;
  
  KEYS_CUR|=(~REG_KEYINPUT)&0x3ff;
  if((btns&IPC_PEN_DOWN)!=0) KEYS_CUR|=KEY_TOUCH;
  if((btns&IPC_X)!=0) KEYS_CUR|=KEY_X;
  if((btns&IPC_Y)!=0) KEYS_CUR|=KEY_Y;
  
  if(ExecMode!=EM_DPG){
    if(KEYS_CUR!=0){
      if(RefreshBacklightOffCount()==true) tpIgnore=true;
    }
  }
  
  if(KEYS_CUR==(KEY_L|KEY_R|KEY_A|KEY_B)){
    videoSetModeSub_SetShowLog(true);
    
    _consolePrintf("Resume save...\n");
    {
      EExecMode em=ExecMode;
      if(em!=EM_DPG){
        Resume_Backup(false);
        if(em==EM_Text){
          Bookmark_CurrentResumeBackup();
          Bookmark_Save();
        }
      }
    }
    
    _consolePrintf("FS_ExecuteStop...\n");
    FS_ExecuteStop();
    
    _consolePrintf("play shutdown.mp3 from shell\n");
    if(MP3Boot_PlayFileFromShell("shutdown.mp3")==false){
      IPC3->LCDPowerControl=LCDPC_SOFT_POWEROFF;
      while(1);
    }
    
    _consolePrintf("wait for sound terminate.\n");
    while(strpcmRequestStop==false){
      strpcmUpdate_mainloop();
//      swiWaitForVBlank();
    }
    
    _consolePrintf("wait for sound empty.\n");
    strpcmRingEmptyFlag=false;
    while(strpcmRingEmptyFlag==false){ cwl();
      swiWaitForIRQ();
    }
    
    _consolePrintf("Execute stop.\n");
    FS_ExecuteStop();
    
    _consolePrintf("go to farmware menu. [%s]\n",DIMediaName);
    
    IPC3->RESET=RESET_NULL;
    
    switch(DIMediaType){
      case DIMT_NONE: break;
      case DIMT_M3CF: cartSetMenuMode_M3CFSD(); IPC3->RESET=RESET_MENU_M3CF; break;
      case DIMT_M3SD: cartSetMenuMode_M3CFSD(); IPC3->RESET=RESET_MENU_M3SD; break;
      case DIMT_MPCF: cartSetMenuMode_MPCF(); IPC3->RESET=RESET_MENU_MPCF; break;
      case DIMT_MPSD: break;
      case DIMT_SCCF: cartSetMenuMode_SCCFSD(); IPC3->RESET=RESET_MENU_SCCF; break;
      case DIMT_SCSD: cartSetMenuMode_SCCFSD(); IPC3->RESET=RESET_MENU_SCSD; break;
      case DIMT_FCSR: break;
      case DIMT_NMMC: break;
      case DIMT_EZSD: cartSetMenuMode_EZSD(); IPC3->RESET=RESET_MENU_EZSD; break;
      case DIMT_MMCF: break;
      case DIMT_SCMS: break;
      case DIMT_EWSD: break;
      case DIMT_NJSD: break;
      case DIMT_DLMS: IPC3->RESET=RESET_MENU_DSLink; LinkReset_ARM9(); while(1); break;
      case DIMT_G6FC: break;
    }
    
    if(IPC3->RESET==RESET_NULL){
      _consolePrintf("not support adapter type.\n");
      while(1);
    }
    
    *(vu16*)(0x04000208) = 0;           //REG_IME = IME_DISABLE;
    *(vu16*)(0x04000204) |= 0x0880;     //sysSetBusOwners(false, false);
    *((vu32*)0x027FFFFC) = 0;
    *((vu32*)0x027FFE04) = (u32)0xE59FF018;
    *((vu32*)0x027FFE24) = (u32)0x027FFE04;
    asm("swi 0x00");                    //swiSoftReset();
    asm("bx lr");
    
    while(1);
  }
  
}

// -----------------------------------------

void WaitKeyClear(bool SoundUpdateFlag)
{ cwl();
  ScanIPC3(true);
  while(KEYS_CUR!=0){ cwl();
    if(SoundUpdateFlag==true){
      while(strpcmUpdate_mainloop()==true){ cwl();
      }
    }
    swiWaitForVBlank();
    ScanIPC3(true);
  }
}

#include "main_keyrepeat.h"

#include "main_keyinput.h"

// -----------------------------------------

static void mainloop_RandomInit(void)
{
#ifdef notuseIPCKey
  return;
#endif
  
  _consolePrintf("Initialize random seed.\n");
  
  u32 chk=0;
  u32 retry=0x100;
  
  while(chk==0){
    IPC3->curtimeFlag=true;
    while(IPC3->curtimeFlag==true){
      IPC3->ReqVsyncUpdate=1;
      while(IPC3->ReqVsyncUpdate!=0){ cwl();
        swiDelay(1);
      }
    }
    
    for(u32 cnt=0;cnt<8;cnt++){ cwl();
      chk=(chk*60)+IPC3->curtime[cnt];
    }
    
    retry--;
    if(retry==0) break;
  }
  
  srand(chk);
  for(u32 idx=0;idx<(chk&0xff);idx++) rand();
  
  _consolePrintf("Initialized. random seed = %d\n",chk);
}

static void mainloop_TimeInit(void)
{
  MemSet32CPU(0,&CurrentTime,sizeof(TPlugin_ClockLib_Time));
  
  TPlugin_ClockLib_Time *pCT=&CurrentTime;
  
  pCT->SystemRTC24Hour=GlobalINI.ClockPlugin.RTC24Hour;
  
/*
  _consolePrintf("clk=%x\n",pPluginBodyClock);
  if(pPluginBodyClock!=NULL){
    pPluginBodyClock->pCL->UpdateVsync(pScreenSub->GetVRAMBuf(),1,PluginBodyClock_RequestRefresh,CurrentTime);
    PluginBodyClock_RequestRefresh=false;
  }
*/
}

static void mainloop_TimeRead(void)
{
  TPlugin_ClockLib_Time *pCT=&CurrentTime;
  
  pCT->Year=2000+RIPC3.curtime[1];
  pCT->Month=RIPC3.curtime[2];
  pCT->Day=RIPC3.curtime[3];
  pCT->Hour=RIPC3.curtime[5];
  pCT->Minuts=RIPC3.curtime[6];
  pCT->Second=RIPC3.curtime[7];
  
  if(52<=pCT->Hour) pCT->Hour-=(52-12);
  
  {
    static u32 Temperature=0;
    u32 SrcTemperature=IPC3->temperature;
    if(Temperature==0){ cwl();
      Temperature=SrcTemperature;
      pCT->Temperature12=SrcTemperature;
      }else{ cwl();
      pCT->Temperature12=((Temperature*15)+SrcTemperature)/16;
    }
  }
}

static void mainloop_ProcVsync(u32 VsyncCount)
{ cwl();
  if(KeyRepeatFlag==true){ cwl();
    if(KeyRepeatCount<=VsyncCount){ cwl();
      KeyRepeatCount=0;
      }else{ cwl();
      KeyRepeatCount-=VsyncCount;
    }
    ScanIPC3(false); // 僆乕僩儕僺乕僩拞偼Vsync偱KEYS傪娔帇偟側偄丅
    }else{ cwl();
    ScanIPC3(true);
  }
  
  if(ExecMode!=EM_DPG){ cwl();
    if(RIPC3.curtimeFlag==false){ cwl();
      IPC3->curtimeFlag=true;
      
      u32 chk=0;
      for(u32 cnt=0;cnt<8;cnt++){ cwl();
        chk+=RIPC3.curtime[cnt];
      }
      
      if(chk==0){ cwl();
        MWin_SetWindowTitle(WM_DateTime,"NotImplement RTC.");
        MWin_TransWindow(WM_DateTime);
        }else{ cwl();
        static u32 sec=0xff;
        if(sec!=RIPC3.curtime[7]){ cwl();
          sec=RIPC3.curtime[7];
          
          mainloop_TimeRead();
          
          if(thumbGetDrawed()==false){
            char str[128];
            
            TPlugin_ClockLib_Time *pCT=&CurrentTime;
            
            char *pstr=str;
            
            pstr+=formdt_FormatDate(pstr,128,pCT->Year,pCT->Month,pCT->Day);
            
            if(GlobalINI.ClockPlugin.RTC24Hour==true){
              sprintf(pstr," %d:%02d:%02d %4.1fC",pCT->Hour,pCT->Minuts,pCT->Second,((float)pCT->Temperature12) / 0x1000);
              }else{
              if(pCT->Hour<12){
                sprintf(pstr," AM%d:%02d:%02d %4.1fC",pCT->Hour,pCT->Minuts,pCT->Second,((float)pCT->Temperature12) / 0x1000);
                }else{
                sprintf(pstr," PM%d:%02d:%02d %4.1fC",pCT->Hour-12,pCT->Minuts,pCT->Second,((float)pCT->Temperature12) / 0x1000);
              }
            }
            
            MWin_SetWindowTitle(WM_DateTime,str);
            MWin_TransWindow(WM_DateTime);
          }
          
        }
        
        if(PluginBodyClock_Execute==true){
          if(pPluginBodyClock!=NULL){
            bool RunningFile;
            if(ExecMode==EM_None){
              RunningFile=false;
              }else{
              RunningFile=true;
            }
            pPluginBodyClock->pCL->UpdateVsync(pScreenSub->GetVRAMBuf(),VsyncCount,RunningFile,PluginBodyClock_RequestRefresh,CurrentTime);
            PluginBodyClock_RequestRefresh=false;
            KEYS_CUR=pPluginBodyClock->pCL->ProcKeys(KEYS_CUR);
          }
        }
        
      }
    }
  }
  
  u32 NowKey=KEYS_CUR;
  
  switch(GlobalINI.System.ClosedSholderButton){ cwl();
    case ECSB_Disabled: { cwl();
      if(RIPC3.buttons == 0x00FF) NowKey&=~(KEY_L | KEY_R);
    } break;
    case ECSB_Flexible: { cwl();
      static u16 FirstPressBtn=0;
      if(RIPC3.buttons != 0x00FF){
        FirstPressBtn=0;
        }else{
        u16 sbtn=NowKey&(KEY_L | KEY_R);
        NowKey&=~(KEY_L | KEY_R);
        
        if(sbtn!=(KEY_L | KEY_R)){
          FirstPressBtn=sbtn;
          }else{
          if(FirstPressBtn==KEY_L) NowKey|=KEY_R;
          if(FirstPressBtn==KEY_R) NowKey|=KEY_L;
          FirstPressBtn=0;
        }
      }
    } break;
    case ECSB_AlwaysDisabled: { cwl();
      NowKey&=~(KEY_L | KEY_R);
    } break;
    case ECSB_Enabled: { cwl();
    } break;
  }
  
  if(NowKey & KEY_SELECT){ cwl();
    strpcmDoubleSpeedFlag=true;
    }else{ cwl();
    strpcmDoubleSpeedFlag=false;
  }
  
  NowKey=KeyRepeat_Proc(NowKey);
  
  if(RequestFileCloseFromMWin==true){
    RequestFileCloseFromMWin=false;
    NowKey=KEY_B;
  }
  
  ProcessKeys(NowKey);
  
  ProcessTouchPad();
}

static void ProcDPG(void)
{ cwl();
  if(DPG_RequestSyncStart==true){ cwl();
    _consolePrintf("DPG_RequestSyncStart\n");
    DPG_RequestSyncStart=false;
    
    switch(DPG_GetDPGAudioFormat()){
      case DPGAF_GSM: {
        strpcmStart(true,DPG_GetSampleRate(),DPG_GetSamplePerFrame(),DPG_GetChannelCount(),SPF_GSM);
      } break;
      case DPGAF_MP2: {
        strpcmStart(false,DPG_GetSampleRate(),8,0,SPF_MP2);
      } break;
    }
    
    while(IPC3->IR_flash==true){
    }
    
    REG_IME=0;
    VsyncPassedCount=0;
    REG_IME=1;
    
    while(strpcmUpdate_mainloop()==true){ cwl();
    }
    
    return;
  }
  
  if(strpcmGetPause()==true) return;
  
  s64 CurrentSamplesCount=IPC3->IR_samples;
  if(CurrentSamplesCount<0) CurrentSamplesCount=0;
  if(UpdateDPG_Video((u64)CurrentSamplesCount)==false){ cwl();
    strpcmRequestStop=true;
    return;
  }
}

void Proc_StartSound_AutoPlay(void)
{
  Shuffle_Clear();
  
  FS_PlayIndex=0;
  
  while(1){ cwl();
    FS_PlayIndex=Shuffle_GetNextIndex(FS_PlayIndex);
    if(FS_PlayIndex==-1) return;
    
    if(FileSys_GetFileType(FS_PlayIndex)==FT_File){ cwl();
      EExecMode EM=FS_GetExecMode(FS_PlayIndex);
      
      if((EM==EM_MSPSound)||(EM==EM_GMENSF)||(EM==EM_GMEGBS)){
        FS_StartFromIndex(FS_PlayIndex);
        WaitKeyClear(true);
        RefreshBacklightOffCount();
        break;
      }
    }
  }
}

void Proc_Shutdown(void)
{
  videoSetModeSub_SetShowLog(true);
  
  _consolePrintf("Resume save...\n");
  {
    EExecMode em=ExecMode;
    if(em!=EM_DPG){
      Resume_Backup(false);
      if(em==EM_Text){
        Bookmark_CurrentResumeBackup();
        Bookmark_Save();
      }
    }
  }
  
  _consolePrintf("FS_ExecuteStop...\n");
  FS_ExecuteStop();
  
  _consolePrintf("Shutdown...\n");
  switch(GlobalINI.System.WhenPanelClose){
    case EISWPC_BacklightOff: case EISWPC_DSPowerOff: {
      IPC3->LCDPowerControl=LCDPC_SOFT_POWEROFF;
      while(1);
    } break;
    case EISWPC_PlayShutdownSound: break; // next
  }
  
  _consolePrintf("play shutdown.mp3 from shell\n");
  if(MP3Boot_PlayFileFromShell("shutdown.mp3")==false){
    IPC3->LCDPowerControl=LCDPC_SOFT_POWEROFF;
    while(1);
  }
  
  _consolePrintf("wait for sound terminate.\n");
  while(strpcmRequestStop==false){
    strpcmUpdate_mainloop();
//    swiWaitForVBlank();
  }
  
  _consolePrintf("wait for sound empty.\n");
  strpcmRingEmptyFlag=false;
  while(strpcmRingEmptyFlag==false){ cwl();
    swiWaitForIRQ();
  }
  
  _consolePrintf("Execute stop.\n");
  FS_ExecuteStop();
  
  _consolePrintf("Power off.\n");
  IPC3->LCDPowerControl=LCDPC_SOFT_POWEROFF;
  
  while(1);
}

#include "mediatype.h"

static void _test_PrintMediaType(bool keywait)
{
  _consolePrintf("MediaType:");
  
  if(DIMediaType==DIMT_NONE){
    _consolePrintf("could not find a working IO Interface\n");
    }else{
    _consolePrintf("%s\n",DIMediaName);
  }
  
  if(keywait==false) return;
  
  ScanIPC3(true);
  while(KEYS_CUR==0){ cwl();
    swiWaitForVBlank();
    ScanIPC3(true);
  }
  WaitKeyClear(false);
}

void strpcmRequestStop_Proc(void)
{
  switch(ExecMode){
    case EM_DPG: {
      FS_ExecuteStop();
      FS_MusicNext(GlobalINI.System.MusicNext);
      RefreshBacklightOffCount();
    } break;
    case EM_None: case EM_MP3Boot: case EM_NDSROM: case EM_FPK: {
    } break;
    case EM_MSPImage: {
    } break;
    case EM_Text: {
    } break;
    case EM_MSPSound: case EM_GMENSF: case EM_GMEGBS: { // case EM_GMEVGM: case EM_GMEGYM: {
      strpcmRingEmptyFlag=false;
      while(strpcmRingEmptyFlag==false){ cwl();
        swiWaitForIRQ();
      }
      FS_ExecuteStop();
      FS_MusicNext(GlobalINI.System.MusicNext);
      if(ExecMode==EM_None) RefreshBacklightOffCount();
    } break;
  }
  
/*
  {
    FS_SetCursor(FS_PlayIndex);
    MWin_SetSBarVPos(WM_FileSelect,FS_TopIndex);
    MWin_DrawSBarVH(WM_FileSelect);
    MWin_DrawClient(WM_FileSelect);
    MWin_TransWindow(WM_FileSelect);
  }
*/
}

void mainloop(void)
{ cwl();
  InitInterrupts();
  IPC3->heartbeat=1;
  swiWaitForVBlank();
  _consolePrintf("VBlankPassed.\n");
  
  videoSetModeSub_SetShowLog(true);
  MWin_Init();
  
  MWin_InitVideoOverlay();
  MWin_SetVideoWideFlag(false);
  MWin_ClearVideoOverlay();
  MWin_SetVideoOverlaySize(0,0,0);
  MWin_SetVideoOverlay(false);
  MWin_SetVideoFullScreen(true);
  
  MWin_SetActive(WM_About);
  MWin_SetActive(WM_DateTime);
  MWin_SetActive(WM_FileSelect);
  MWin_RefreshScreenMask();
  
  MWin_SetSBarV(WM_About,11,5,0);
  MWin_SetSBarV(WM_Help,22+1,8,0);
  
  { cwl();
    char str[128];
    sprintf(str,"About... %s",ROMTITLE);
    MWin_SetWindowTitle(WM_About,str);
  }
  //加载语言库
  dsCardi_SetRompage(0);
  u8 *pFlash = (u8*)safemalloc(0x2000);
//  BL_ReadFlashBuffer(ROM_OFF,pFlash,0x2000);
  ResetFlash();
  for(s16 ii=0;ii<0x2000;ii+=0x200)
	  dsCardi_ReadCard(ROM_OFF+ii,&pFlash[ii]);	
  gl_language = pFlash[0];
  gl_speedAuto = pFlash[1];
  gl_SaveType=pFlash[4];
  LoadLanguage();
  if(gl_speedAuto!=0 && gl_speedAuto!=1)
  	gl_speedAuto=1;
  gl_speed = *((u16 *)(pFlash+2));  
 if(gl_speed==0xFFFF || gl_speed==0)
 	gl_speed=1000;
  for(u16 ii=0;ii<256;ii++)
  {

  	pSaveFileName[ii] = pFlash[256+ii];
  }
  if((pFlash[256]==0xFF) || (pFlash[256]==0x00))
  {
  	for(u16 ii=0;ii<256;ii++)
  	{

  		pSaveFileName[ii] ='\0';
  	}
  }
//   _consolePrintf("pSaveFileName=%s\n",(char *)pSaveFileName);
  free(pFlash);
  pFlash=NULL;
  
  /**创建save目录,为备份存档做准备 开始********/
   videoSetModeSub_SetShowLog(false);
  u8 pSave[512];
  char str[512];
  FAT_FILE *h=NULL;
  FAT_InitFiles();
  FAT_mkdir("/save");
  FAT_CWD("/save");  
  if(strlen((char *)pSaveFileName)>0)
  {
  	  h=FAT_fopen((char *)pSaveFileName,"w");
	  if(h==NULL)
	  	_consolePrintf("open file %s fail\n",(char *)pSaveFileName);
	  else
	  {
	  	_consolePrintf("back save file %s OK\n",(char *)pSaveFileName);
		if(GetSaveSize()>0)
		{
		 MWin_ShowProgressDialog(2,0,0);
		  for(s32 ii=0;ii<GetSaveSize();ii+=512)
		  {
		  	 //Enable_Arm9DS();
		  	cardmeReadEeprom(ii,pSave,512,3);
			//Enable_Arm7DS();
			FAT_fwrite(pSave,512,1,h);
			//if(ii%0x8000==0)
				//_consolePrintf("load save file %d%s\n",(ii*100)/GetSaveSize(),"%"); 
			if( (ii*10/GetSaveSize())>0)
					MWin_ShowProgressDialog(2,ii*10/GetSaveSize(),0);
		  }
		}
		FAT_fclose(h);
		_consolePrintf("back save file OK\n");
	  }
   }
  FAT_FreeFiles();
  /**创建save目录,为备份存档做准备 结束*********/

  MWin_AllRefresh();
  
  mainloop_RandomInit();
  mainloop_TimeInit();
  
  if(GlobalINI.ClockPlugin.Enabled==false){
    PluginBodyClock_RequestRefresh=false;
    PluginBodyClock_Execute=false;
    pPluginBodyClock=NULL;
    
    }else{
    PluginBodyClock_RequestRefresh=true;
    PluginBodyClock_Execute=false;
    
    char fn[PluginFilenameMax];
    
    DLLList_GetClockPluginFilename(0,fn);
    pPluginBodyClock=DLLList_LoadPlugin(fn);
    
    if(pPluginBodyClock!=NULL){
      if(pPluginBodyClock->pCL->Start(0)==false){ cwl();
        _consolePrintf("Clock plugin start error.\n");
        if(pPluginBodyClock!=NULL){
          DLLList_FreePlugin(pPluginBodyClock);
          safefree(pPluginBodyClock); pPluginBodyClock=NULL;
        }
      }
    }
    
  }
  
#ifndef DisableStartupSound
  if(GlobalINI.Boot.StartupSound==EIBSS_Startupmp3) MP3Boot_PlayFileFromShell("startup.mp3");
#endif
  
  _consolePrintf("read FileSystem.\n");
  
  FS_DispCount=MWin_GetClientHeight(WM_FileSelect)/FontProHeight;
  Shell_VolumeType = VT_MPCF;
#ifdef StartupIMFS
  FS_ChangePath("//EXFS");
#else
  if(strcmp(GlobalINI.System.StartPath,"//AUTO")!=0){
    FS_ChangePath(GlobalINI.System.StartPath);
    }else{
    switch(Shell_VolumeType){
      case VT_NULL: FS_ChangePath("//"); break;
      case VT_IMFS: FS_ChangePath("//IMFS"); break;
      case VT_EXFS: FS_ChangePath("//EXFS"); break;
      case VT_MPCF: FS_ChangePath("//EZSD"); break;
      case VT_GBFS: FS_ChangePath("//GBFS"); break;
      default: FS_ChangePath("//"); break;
    }
  }
  _test_PrintMediaType(false);
#endif
  
  PrintFreeMem();
  WaitKeyClear(false);
  
  KeyRepeatLastKey=0;
  KeyRepeatFlag=false;
  KeyRepeatCount=0;
  
  videoSetModeSub_SetShowLog(false);
  
  CheckTCM(__LINE__);
  
//  videoSetModeSub_SetShowLog(true); _consolePrintf("success boot sequence process. stop.\n"); while(1);
  
  RefreshBacklightOffCount();
  
  if(Resume_Restore()==false){
    if(GlobalINI.Boot.StartupSound==EIBSS_AutoPlay) Proc_StartSound_AutoPlay();
  }
  
  u32 ResumeTimer=0;
  
  while(1){ cwl();
    if(strpcmRequestStop==true) strpcmRequestStop_Proc();
    
    if(ExecMode==EM_DPG){
      strpcmUpdate_mainloop();
      PrfStart();
      ProcDPG();
      PrfEnd(DPG_GetCurrentFrameCount());
      
      if(VsyncPassedCount==0) swiWaitForVBlank();
      
      REG_IME=0;
      u32 VsyncCount=VsyncPassedCount;
      VsyncPassedCount=0;
      REG_IME=1;
//      if(VsyncCount!=0) _consolePrintf("[hb%d]",IPC3->heartbeat);
      
      if(VsyncCount!=0){
        mainloop_ProcVsync(VsyncCount);
        if(MWin_GetVideoFullScreen()==false){ cwl();
          if(MWin_GetVisible(WM_PlayControl)==true){ cwl();
            static u32 pc=0;
            if(pc<=VsyncCount){ cwl();
              pc=0;
              }else{ cwl();
              pc-=VsyncCount;
            }
            if(pc==0){ cwl();
              pc=30;
              MWin_DrawClient(WM_PlayControl);
              MWin_TransWindow(WM_PlayControl);
            }
          }
        }
      }
      continue;
    }
    
    while(strpcmUpdate_mainloop()==true){ cwl();
      if(GlobalINI.KeyRepeat.DelayCount<VsyncPassedCount) break;
    }
    
    while(VsyncPassedCount==0){ cwl();
      strpcmUpdate_mainloop();
      CheckTCM(__LINE__);
      swiWaitForIRQ();
    }
    
    if(IPC3->RequestShotDown==true){
      videoSetModeSub_SetShowLog(true);
      Proc_Shutdown();
      while(1);
    }
    
    u32 VsyncCount;
    
    REG_IME=0;
    VsyncCount=VsyncPassedCount;
    VsyncPassedCount=0;
    REG_IME=1;
    
    if(ExecMode!=EM_DPG) ProcBacklightOffCount(VsyncCount);
    
    if(ExecMode==EM_MSPImage) ImageControlTimeOut_ProcVSync(VsyncCount);
    
    mainloop_ProcVsync(VsyncCount);
    
    if((Resume_isEnabled()==true)&&(KEYS_CUR==0)){
      if(ResumeTimer==0){
        EExecMode em=ExecMode;
        bool ReqResume=true;
        if(em==EM_DPG) ReqResume=false;
        if((em==EM_MSPSound)||(em==EM_GMENSF)||(em==EM_GMEGBS)){ // ||(em==EM_GMEVGM)||(em==EM_GMEGYM)
          if(GlobalINI.System.ResumeUsingWhileMusicPlaying==false) ReqResume=false;
        }
        if(ReqResume==true){
          Resume_Backup(false);
          if(em==EM_Text){
            Bookmark_CurrentResumeBackup();
            Bookmark_Save();
          }
        }
        ResumeTimer=3*60;
        }else{
        if(ResumeTimer<VsyncCount){
          ResumeTimer=0;
          }else{
          ResumeTimer-=VsyncCount;
        }
      }
    }
    
    if(MWin_GetVisible(WM_PlayControl)==true){ cwl();
      static u32 pc=0;
      if(pc<=VsyncCount){ cwl();
        pc=0;
        }else{ cwl();
        pc-=VsyncCount;
      }
      if(pc==0){ cwl();
        pc=30;
        MWin_DrawClient(WM_PlayControl);
        MWin_TransWindow(WM_PlayControl);
      }
    }
    
  }
  
  FS_ExecuteStop();
  
  if(pPluginBodyClock!=NULL){
    DLLList_FreePlugin(pPluginBodyClock);
    safefree(pPluginBodyClock); pPluginBodyClock=NULL;
  }
  
  MWin_Free();
  FileSys_Free();
  
  ShowLogHalt();
}

#include "main_drawcb.h"
#include "main_drawcb_closebutton.h"

