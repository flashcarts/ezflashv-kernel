
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "_const.h"
#include "_console.h"

#include "memtool.h"

#include "resume.h"

#include "gba_nds_fat.h"
#include "filesys.h"

#include "directdisk.h"

#include "mwin.h"
#include "inifile.h"

#include "main_extforresume.h"

#include "strpcm.h"

#include "dll.h"

#include "plugin/plug_dpg.h"
#include "plugin/plug_mp2.h"
#include "plugin/plug_gmensf.h"
#include "plugin/plug_gmegbs.h"

static bool ResumeEnabled;

#define DummySize (256)

#define VID (0x02440244)

#define SID "MoonShellResumeData"
#define SIDlen (20+1)

typedef struct {
  char Dummy[DummySize];
  u32 vid;
  char sid[SIDlen];
  u32 SoundVolume;
  u32 MusicNext;
  u32 NDSLite_Brightness;
  EExecMode ExecMode;
  char PlayPathName[forResume_PlayPathNameLen];
  char PlayFileAliasName[forResume_PlayFileAliasNameLen];
  u32 TextY;
  float PicRatio;
  u32 PicX,PicY;
  u32 SoundPos;
  char Terminate[DD_SectorSize-DummySize];
} TResumeData;

static TResumeData ResumeData;

void Resume_Init(void)
{
  ResumeEnabled=false;
  if(DD_isEnabled()==false) return;
  
  switch(DD_GetSaveType()){
    case EDDST_None: return; break;
    case EDDST_FAT: {
      FAT_FILE *fp=FAT_fopen(DDFilename_resume,"r");
      if(fp==NULL) return;
      FAT_fclose(fp);
    } break;
    case EDDST_CART: {
    } break;
  }
  
  _consolePrintf("\n");
  _consolePrintf("Resume initialize.\n");
  
  if(DD_InitFile(EDDFile_resume)==false){
    _consolePrintf("\n");
    _consolePrintf("Critical error!! CPU HALT for safety.\n");
    ShowLogHalt();
    while(1);
  }
  
  DD_ReadFile(EDDFile_resume,&ResumeData,DD_SectorSize);
  
  _consolePrintf("Resume initialized.\n");
  
  ResumeEnabled=true;
}

static void Resume_Backup_Clear(void)
{
  MemSet32DMA3(0,&ResumeData.vid,DD_SectorSize-DummySize);
  
  ResumeData.vid=VID;
  strncpy(ResumeData.sid,SID,SIDlen);
  
  ResumeData.SoundVolume=strpcmGetVolume16();
  ResumeData.MusicNext=GlobalINI.System.MusicNext;
  ResumeData.NDSLite_Brightness=NDSLite_Brightness;
  
  ResumeData.ExecMode=ExecMode;
  
  strncpy(ResumeData.PlayPathName,forResume_PlayPathName,forResume_PlayPathNameLen);
  strncpy(ResumeData.PlayFileAliasName,forResume_PlayFileAliasName,forResume_PlayFileAliasNameLen);
}

static void Resume_Backup_Write(void)
{
  DD_WriteFile(EDDFile_resume,&ResumeData,DD_SectorSize);
}

#include "_consoleWriteLog.h"

void Resume_Backup(bool AlreadyTop)
{
  if(ResumeEnabled==false) return;
  
//  if(strncmp("//MPCF",forResume_PlayPathName,6)!=0) return;
  
  TResumeData ResumeDataBackup=ResumeData;
  
  EExecMode em=ExecMode;
  
//  PrfStart();
  
  if((em!=EM_Text)&&(em!=EM_MSPImage)&&(em!=EM_MSPSound)&&(em!=EM_GMENSF)&&(em!=EM_GMEGBS)&&(em!=EM_DPG)){
    if(ResumeData.ExecMode==EM_None) return;
    Resume_Backup_Clear();
    ResumeData.ExecMode=EM_None;
    }else{
    Resume_Backup_Clear();
    switch(ExecMode){
      case EM_None: case EM_NDSROM: case EM_FPK: case EM_MP3Boot: {
      } break;
      case EM_Text: {
        ResumeData.TextY=MWin_GetSBarVPos(WM_TextView);
      } break;
      case EM_MSPImage: {
        ResumeData.PicRatio=Pic_GetRatio();
        ResumeData.PicX=MWin_GetSBarHPos(WM_PicView);
        ResumeData.PicY=MWin_GetSBarVPos(WM_PicView);
      } break;
      case EM_MSPSound: {
        if(AlreadyTop==false) ResumeData.SoundPos=pPluginBody->pSL->GetPosOffset();
      } break;
      case EM_GMENSF: {
        if(AlreadyTop==false) ResumeData.SoundPos=GMENSF_GetPosOffset();
      } break;
      case EM_GMEGBS: {
        if(AlreadyTop==false) ResumeData.SoundPos=GMEGBS_GetPosOffset();
      } break;
/*
      case EM_GMEVGM: {
        if(AlreadyTop==false) ResumeData.SoundPos=GMEVGM_GetPosOffset();
      } break;
      case EM_GMEGYM: {
        if(AlreadyTop==false) ResumeData.SoundPos=GMEGYM_GetPosOffset();
      } break;
*/
      case EM_DPG: {
        if(AlreadyTop==false) ResumeData.SoundPos=DPG_GetCurrentFrameCount();
      } break;
    }
  }
  
  u32 *pchk0=(u32*)&ResumeData;
  u32 *pchk1=(u32*)&ResumeDataBackup;
  
  for(u32 idx=0;idx<DD_SectorSize/4;idx++){
    if(pchk0[idx]!=pchk1[idx]){
      Resume_Backup_Write();
      return;
    }
  }
}

bool Resume_Restore(void)
{
  if(ResumeEnabled==false) return(false);
  
  if(ResumeData.vid!=VID) return(false);
  
  if(ResumeData.ExecMode==EM_None) return(false);
  
  FS_ExecuteStop();
  
  TResumeData data=ResumeData;
  
  _consolePrintf("restore resume.\n");
  
  strpcmSetVolume16(data.SoundVolume);
  GlobalINI.System.MusicNext=data.MusicNext;
  if(data.NDSLite_Brightness!=4){
    NDSLite_Brightness=data.NDSLite_Brightness;
    NDSLite_SetBrightness(NDSLite_Brightness);
  }
  
  _consolePrintf("PathName=%s\n",data.PlayPathName);
  _consolePrintf("FileAliasName=%s\n",data.PlayFileAliasName);
  
  FS_ChangePath(data.PlayPathName);
  
  u32 fcnt=FileSys_GetFileCount();
  
  _consolePrintf("FileCount=%d\n",fcnt);
  
  u32 findidx=(u32)-1;
  
  for(u32 idx=0;idx<fcnt;idx++){
    if(strncmp(data.PlayFileAliasName,FileSys_GetAlias(idx),forResume_PlayFileAliasNameLen)==0) findidx=idx;
  }
  
  if(findidx==(u32)-1) return(false);
  
  FS_SetCursor(findidx);
  FS_StartFromIndex(findidx);
  MWin_SetSBarVPos(WM_FileSelect,findidx);
  MWin_DrawSBarVH(WM_FileSelect);
  MWin_DrawTitleBar(WM_FileSelect);
  MWin_DrawClient(WM_FileSelect);
  MWin_TransWindow(WM_FileSelect);
  
  if(ExecMode==data.ExecMode){
    switch(ExecMode){
      case EM_None: case EM_NDSROM: case EM_FPK: case EM_MP3Boot: {
      } break;
      case EM_Text: {
        if(data.TextY!=0){
          MWin_SetSBarVPos(WM_TextView,data.TextY);
          MWin_DrawSBarVH(WM_TextView);
          MWin_DrawClient(WM_TextView);
          MWin_TransWindow(WM_TextView);
        }
      } break;
      case EM_MSPImage: {
        float ratio=data.PicRatio;
        Pic_SetRatio(ratio);
        if(0.5<ratio){
          MWin_SetVideoOverlaySize(0,0,ratio);
          }else{
          MWin_SetVideoOverlaySize(0,0,ratio*2);
        }
        MWin_RefreshVideoFullScreen(true);
        Pic_SetSBar(ratio);
        
        MWin_SetSBarHPos(WM_PicView,data.PicX);
        MWin_SetSBarVPos(WM_PicView,data.PicY);
        
        MWin_DrawSBarVH(WM_PicView);
        MWin_DrawTitleBar(WM_PicView);
        MWin_DrawClient(WM_PicView);
        MWin_TransWindow(WM_PicView);
      } break;
      case EM_MSPSound: {
        if(data.SoundPos!=0) pPluginBody->pSL->SetPosOffset(data.SoundPos);
//        _consolePrintf("%d,%d\n",data.SoundPos,pPluginBody->pSL->GetPosOffset());
//        ShowLogHalt();
      } break;
      case EM_GMENSF: {
        if(data.SoundPos!=0) GMENSF_SetPosOffset(data.SoundPos);
      } break;
      case EM_GMEGBS: {
        if(data.SoundPos!=0) GMEGBS_SetPosOffset(data.SoundPos);
      } break;
/*
      case EM_GMEVGM: {
        if(data.SoundPos!=0) GMEVGM_SetPosOffset(data.SoundPos);
      } break;
      case EM_GMEGYM: {
        if(data.SoundPos!=0) GMEGYM_SetPosOffset(data.SoundPos);
      } break;
*/
      case EM_DPG: {
        if(data.SoundPos!=0){
          strpcmStop();
          DPG_SetFrameCount(data.SoundPos);
          DPG_RequestSyncStart=true;
        }
      } break;
    }
  }
  
  Resume_Backup(true);
  return(true);
}

bool Resume_isEnabled(void)
{
  return(ResumeEnabled);
}

