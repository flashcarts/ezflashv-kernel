
#include <stdio.h>
#include <stdlib.h>

#include <NDS.h>

#include "emulator.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "../../ipc3.h"

#include "glib/glib.h"

#include "mwin.h"
#include "strtool.h"
#include "memtool.h"
#include "FontPro.h"
#include "unicode.h"

#include "_const.h"
#include "inifile.h"

#include "VRAMTool.h"

#include "mwin_color.h"
#include "ez5_language.h"

#define PrgBarHeight (16)

static TWinBody WinBody[WinBodyCount];

static u32 PriorityLevel;

static u8 *ScreenMask;

TSkinBM SkinBM;

static UnicodeChar PlayControl_WindowTitleStrW[512];

static bool VideoOverlay;
static bool VideoFullScreen;

bool RequestFileCloseFromMWin=false;

// --------------------------------------------------------

static u32 ImageControlTimeOutStart;
static u32 ImageControlTimeOutLast;

void ImageControlTimeOut_SetDefault(u32 sec)
{
  ImageControlTimeOutStart=sec*60;
  ImageControlTimeOutLast=0;
}

void ImageControlTimeOut_ProcReset(void)
{
  ImageControlTimeOutLast=ImageControlTimeOutStart;
  
  {
    extern void ImageControlTimeOut_ProcReset_callback(void);
    ImageControlTimeOut_ProcReset_callback();
  }
}

void ImageControlTimeOut_ProcVSync(u32 VsyncCount)
{
  if(MWin_GetVisible(WM_PicView)==false) return;
  
  if(GlobalINI.ImagePlugin.Interpolation!=0){
    extern bool ImageControlTimeOut_ProcVSync_callback(u32 VsyncCount);
    if(ImageControlTimeOut_ProcVSync_callback(VsyncCount)==true) return;
  }
  
  if(ImageControlTimeOutLast==0) return;
  
  if(ImageControlTimeOutLast<=VsyncCount){
    ImageControlTimeOutLast=0;
    }else{
    ImageControlTimeOutLast-=VsyncCount;
  }
  
  if(ImageControlTimeOutLast==0){
    u16 *pVRAMBuf=pScreenMainOverlay->GetVRAMBuf();
    u32 VRAMSize=ScreenHeight*ScreenWidth;
    MemSet16DMA3(0,pVRAMBuf,VRAMSize*2);
  }
}

// --------------------------------------------------------

#include "shell.h"

#include "mwin_bmp.h"

void ApplyOverrideWindowRect(TRect *Rect,TRect *ClientRect,bool *CloseButton,TiniOverrideWindowRectBody *OverrideWindowRect)
{
  if(OverrideWindowRect->x!=-1) Rect->x=OverrideWindowRect->x;
  if(OverrideWindowRect->y!=-1) Rect->y=OverrideWindowRect->y;
  if(OverrideWindowRect->w!=-1) ClientRect->w=OverrideWindowRect->w;
  if(OverrideWindowRect->h!=-1) ClientRect->h=OverrideWindowRect->h;
  if(OverrideWindowRect->DisabledCloseButton==true) *CloseButton=false;
}

void MWin_Init(void)
{ cwl();
  PriorityLevel=0;
  
  ScreenMask=(u8*)safemalloc(ScreenWidth*ScreenHeight);
  MemSet8DMA3(0,ScreenMask,ScreenWidth*ScreenHeight);
  
  PlayControl_WindowTitleStrW[0]=(UnicodeChar)0;
  
  for(u32 cnt=0;cnt<WinBodyCount;cnt++){ cwl();
    TWinBody *pwb=&WinBody[cnt];
    
    pwb->WinMode=cnt;
    
    pwb->DynamicAllocMem=false;
    pwb->WinBuf=NULL;
    
    pwb->Visible=false;
    pwb->SubDisplay=false;
    pwb->TopFlag=false;
    pwb->Priority=PriorityLevel;
    PriorityLevel++;
    
    pwb->ClientBGColor=MWC_ClientBG;
    
    pwb->SBarV.Enabled=false;
    pwb->SBarV.isV=true;
    
    pwb->SBarH.Enabled=false;
    pwb->SBarH.isV=false;
    
    pwb->CloseButton=true;
    
    switch(pwb->WinMode){ cwl();
      case WM_About: {
        if(GlobalINI.Boot.hiddenAboutWindow==true) pwb->Visible=false;
        	pwb->Visible=false;
        sprintf(pwb->TitleStr,"About...");
        pwb->Rect.x=8;
        pwb->Rect.y=8;
        pwb->ClientRect.w=192;
        pwb->ClientRect.h=FontProHeight*5;
        pwb->SBarV.Enabled=true;
        ApplyOverrideWindowRect(&pwb->Rect,&pwb->ClientRect,&pwb->CloseButton,&GlobalINI.OverrideWindowRect.About);
      } break;
      case WM_Help: {
        if(GlobalINI.Boot.hiddenHelpWindow==true) pwb->Visible=false;
        sprintf(pwb->TitleStr,"OnlineHelp...");
        pwb->Rect.x=120;
        pwb->Rect.y=50;
        pwb->ClientRect.w=130-8;
        pwb->ClientRect.h=FontProHeight*8;
        pwb->SBarV.Enabled=true;
        ApplyOverrideWindowRect(&pwb->Rect,&pwb->ClientRect,&pwb->CloseButton,&GlobalINI.OverrideWindowRect.Help);
      } break;
      case WM_PlayControl: {
        pwb->DynamicAllocMem=true;
        pwb->Visible=false;
        sprintf(pwb->TitleStr,"PlayControl...");
        pwb->Rect.x=0;
        pwb->Rect.y=ScreenHeight-(TitleBarHeight+1+FontProHeight+16+1);
        pwb->ClientRect.w=ScreenWidth-2;
        pwb->ClientRect.h=FontProHeight+PrgBarHeight;
        ApplyOverrideWindowRect(&pwb->Rect,&pwb->ClientRect,&pwb->CloseButton,&GlobalINI.OverrideWindowRect.PlayControl);
      } break;
      case WM_FileSelect: {
        sprintf(pwb->TitleStr,"FileSelect...");
        pwb->Visible=true;
        pwb->Rect.x=0;
        pwb->Rect.y=0;
        pwb->ClientRect.w=ScreenWidth-8-1-1;
        pwb->ClientRect.h=ScreenHeight-1-TitleBarHeight-1;
        pwb->ClientBGColor=0;
        pwb->SBarV.Enabled=true;
        
        ApplyOverrideWindowRect(&pwb->Rect,&pwb->ClientRect,&pwb->CloseButton,&GlobalINI.OverrideWindowRect.FileSelect);
        
        pwb->SubDisplay=GlobalINI.System.FileSelectSubScreen;
        if(pwb->SubDisplay==true){
          pwb->Rect.x=0;
          pwb->Rect.y=0;
          pwb->ClientRect.w=ScreenWidth-8-1-1;
          pwb->ClientRect.h=ScreenHeight-1-TitleBarHeight-1;
          pwb->CloseButton=false;
        }
      } break;
      case WM_DateTime: {
        if(GlobalINI.Boot.hiddenDateTimeWindow==true) pwb->Visible=false;
        sprintf(pwb->TitleStr,"DateTimeWindow");
        pwb->Rect.x=8;
        pwb->Rect.y=130;
        pwb->ClientRect.w=168;
        pwb->ClientRect.h=0;
        ApplyOverrideWindowRect(&pwb->Rect,&pwb->ClientRect,&pwb->CloseButton,&GlobalINI.OverrideWindowRect.DateTime);
      } break;
      case WM_PicView: {
        pwb->DynamicAllocMem=true;
        pwb->Visible=false;
        sprintf(pwb->TitleStr,"PictureView...");
        pwb->Rect.x=8;
        pwb->Rect.y=16;
        pwb->ClientRect.w=ScreenWidth-pwb->Rect.x-16;
        pwb->ClientRect.h=ScreenHeight-pwb->Rect.y-TitleBarHeight-16;
        pwb->ClientBGColor=0;
        pwb->SBarV.Enabled=true;
        pwb->SBarH.Enabled=true;
        
        ApplyOverrideWindowRect(&pwb->Rect,&pwb->ClientRect,&pwb->CloseButton,&GlobalINI.OverrideWindowRect.PicView);
        
        { // FullScreen
          pwb->Rect.x=0;
          pwb->Rect.y=0;
          pwb->ClientRect.w=ScreenWidth-8-1-1;
          pwb->ClientRect.h=ScreenHeight-1-TitleBarHeight-8-1;
        }
      } break;
      case WM_TextView: {
        pwb->DynamicAllocMem=true;
        pwb->Visible=false;
        sprintf(pwb->TitleStr,"TextView...");
        pwb->Rect.x=8;
        pwb->Rect.y=16;
        pwb->ClientRect.w=ScreenWidth-pwb->Rect.x-16;
        pwb->ClientRect.h=ScreenHeight-pwb->Rect.y-TitleBarHeight-16;
        pwb->ClientBGColor=MWC_TextViewBG;
        pwb->SBarV.Enabled=true;
        pwb->SBarH.Enabled=false;
        
        switch(GlobalINI.TextPlugin.SelectDisplay){
          case EITPSD_Bottom: pwb->SubDisplay=false; break;
          case EITPSD_Top: pwb->SubDisplay=true; break;
          default: break;
        }
        
        { // FullScreen
          pwb->Rect.x=0;
          pwb->Rect.y=0;
          pwb->ClientRect.w=ScreenWidth-8-1-1;
          pwb->ClientRect.h=ScreenHeight-1-TitleBarHeight-1;
        }
        
        ApplyOverrideWindowRect(&pwb->Rect,&pwb->ClientRect,&pwb->CloseButton,&GlobalINI.OverrideWindowRect.TextView);
      } break;
      case WM_Progress: {
        pwb->Visible=false;
        sprintf(pwb->TitleStr,"Progress...");
        pwb->Rect.x=ScreenWidth-132;
        pwb->Rect.y=ScreenHeight-40;
        pwb->ClientRect.w=128;
        pwb->ClientRect.h=PrgBarHeight;
        pwb->ClientBGColor=0;
        ApplyOverrideWindowRect(&pwb->Rect,&pwb->ClientRect,&pwb->CloseButton,&GlobalINI.OverrideWindowRect.Progress);
        pwb->CloseButton=false;
      } break;
      case WM_SetNext: {
        pwb->Visible=false;
        sprintf(pwb->TitleStr,"SetNext...");
        pwb->Rect.x=(ScreenWidth-96)/2;
        pwb->Rect.y=32;
        pwb->ClientRect.w=96;
        pwb->ClientRect.h=MusicNext_Count*FontProHeight;
        ApplyOverrideWindowRect(&pwb->Rect,&pwb->ClientRect,&pwb->CloseButton,&GlobalINI.OverrideWindowRect.SetNext);
      } break;
      case WM_FileInfo: {
        pwb->Visible=false;
        sprintf(pwb->TitleStr,"File Infomation");
        pwb->Rect.x=32;
        pwb->Rect.y=48;
        pwb->ClientRect.w=192;
        pwb->ClientRect.h=(5*FontProHeight)+2+2;
        pwb->SBarV.Enabled=true;
        ApplyOverrideWindowRect(&pwb->Rect,&pwb->ClientRect,&pwb->CloseButton,&GlobalINI.OverrideWindowRect.FileInfo);
      } break;
      default: {
        _consolePrintf("FatalError. mwin illigal initialize.\n");
        ShowLogHalt();
        while(1);
      }
    }
    
    pwb->ClientRect.x=1;
    pwb->ClientRect.y=TitleBarHeight+1;
    pwb->Rect.w=pwb->ClientRect.x+pwb->ClientRect.w+1;
    if(pwb->ClientRect.h==0){ cwl();
      pwb->Rect.h=pwb->ClientRect.y-1;
      }else{ cwl();
      pwb->Rect.h=pwb->ClientRect.y+pwb->ClientRect.h+1;
    }
    
    { cwl();
      TSBar *psb=&pwb->SBarV;
      if(psb->Enabled==true){ cwl();
        TRect *pcr=&psb->ClientRect;
        
        pcr->x=pwb->ClientRect.x+pwb->ClientRect.w;
        pcr->y=pwb->ClientRect.y;
        pcr->w=8;
        pcr->h=pwb->ClientRect.h;
        
        pwb->Rect.w+=8;
      }
      
    }
    
    { cwl();
      TSBar *psb=&pwb->SBarH;
      if(psb->Enabled==true){ cwl();
        TRect *pcr=&psb->ClientRect;
        
        pcr->x=pwb->ClientRect.x;
        pcr->y=pwb->ClientRect.y+pwb->ClientRect.h;
        pcr->w=pwb->ClientRect.w;
        pcr->h=8;
        
        pwb->Rect.h+=8;
      }
      
    }
    
//    pwb->DynamicAllocMem=false; // disabled
    
    if(pwb->DynamicAllocMem==false){
      if(pwb->WinBuf==NULL) pwb->WinBuf=(u16*)safemalloc(pwb->Rect.w*pwb->Rect.h*2);
      }else{
      pwb->WinBuf=NULL;
    }
  }
  
  LoadSkinBM();
  
  VideoOverlay=false;
  VideoFullScreen=true;
}

void MWin_Free(void)
{ cwl();
}

void MWin_AllRefresh(void)
{ cwl();
  MWin_ClearDesktop();
  
  for(u32 cnt=0;cnt<WinBodyCount;cnt++){ cwl();
    MWin_RefreshWindow(cnt);
  }
}

void MWin_AllTrans(void)
{ cwl();
  MWin_ClearDesktop();
  
  for(u32 cnt=0;cnt<WinBodyCount;cnt++){ cwl();
    MWin_TransWindow(cnt);
  }
}

void MWin_AllocMem(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  if(pwb->DynamicAllocMem==false){
    if(pwb->WinBuf==NULL){
      _consolePrintf("MWinError:DynamicAllocMem==false and WinBuf==NULL\n");
    }
    return;
  }
  
  if(pwb->WinBuf==NULL){ cwl();
    pwb->WinBuf=(u16*)safemalloc(pwb->Rect.w*pwb->Rect.h*2);
    MemSet16DMA3(0,pwb->WinBuf,pwb->Rect.w*pwb->Rect.h*2);
  }
}

extern void MWin_FreeMem(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  if(pwb->DynamicAllocMem==false){
    if(pwb->WinBuf==NULL){
      _consolePrintf("MWinError:DynamicAllocMem==false and WinBuf==NULL\n");
    }
    return;
  }
  
  if(pwb->WinBuf!=NULL){ cwl();
    safefree(pwb->WinBuf); pwb->WinBuf=NULL;
  }
}

u32 MWin_SetActive(u32 WinBodyIndex)
{ cwl();
  u32 DeactiveWinIndex=(u32)-1;
  
  for(u32 cnt=0;cnt<WinBodyCount;cnt++){ cwl();
    TWinBody *pwb=&WinBody[cnt];
    
    if(cnt==WinBodyIndex){ cwl();
      pwb->TopFlag=true;
      pwb->Priority=PriorityLevel;
      PriorityLevel++;
      }else{ cwl();
      if(pwb->TopFlag==true){ cwl();
        DeactiveWinIndex=cnt;
        pwb->TopFlag=false;
      }
    }
  }
  
  return(DeactiveWinIndex);
}

u32 MWin_SetActiveTopMost(void)
{ cwl();
  u32 _Priority=0;
  u32 idx=(u32)-1;
  
  for(u32 cnt=0;cnt<WinBodyCount;cnt++){ cwl();
    TWinBody *pwb=&WinBody[cnt];
    
    if(pwb->Visible==true){ cwl();
      if(_Priority<=pwb->Priority){ cwl();
        _Priority=pwb->Priority;
        idx=cnt;
      }
    }
  }
  
  if(idx==(u32)-1) return(idx);
  
  MWin_SetActive(idx);
  return(idx);
}

void MWin_InsideScreen(TRect *Rect)
{ cwl();
  if(Rect->x<0){ cwl();
    Rect->w=Rect->w+Rect->x;
    Rect->x=0;
  }
  if(Rect->y<0){ cwl();
    Rect->h=Rect->h+Rect->y;
    Rect->y=0;
  }
  if((Rect->x+Rect->w)>ScreenWidth) Rect->w=ScreenWidth-Rect->x;
  if((Rect->y+Rect->h)>ScreenHeight) Rect->h=ScreenHeight-Rect->y;
}

void MWin_RefreshScreenMask(void)
{ cwl();
  u32 SortIndex[WinBodyCount];
  u32 SortPriority[WinBodyCount];
  
  for(u32 cnt=0;cnt<WinBodyCount;cnt++){ cwl();
    TWinBody *pwb=&WinBody[cnt];
    
    SortIndex[cnt]=cnt;
    SortPriority[cnt]=pwb->Priority;
  }
  
  for(u32 c0=0;c0<WinBodyCount-1;c0++){ cwl();
    for(u32 c1=c0+1;c1<WinBodyCount;c1++){ cwl();
      if(SortPriority[c0]>SortPriority[c1]){ cwl();
        u32 tmp;
        
        tmp=SortIndex[c0];
        SortIndex[c0]=SortIndex[c1];
        SortIndex[c1]=tmp;
        
        tmp=SortPriority[c0];
        SortPriority[c0]=SortPriority[c1];
        SortPriority[c1]=tmp;
      }
    }
  }
  
  u8 *pMask=&ScreenMask[0];
  
  for(s32 y=0;y<ScreenHeight;y++){ cwl();
    MemSet8DMA3(0xff,pMask,ScreenWidth);
    pMask+=ScreenWidth;
  }
  
  for(u32 sortcnt=0;sortcnt<WinBodyCount;sortcnt++){ cwl();
    u32 cnt=SortIndex[sortcnt];
    TWinBody *pwb=&WinBody[cnt];
    
    if((pwb->Visible==true)&&(pwb->SubDisplay==false)){ cwl();
      TRect Rect=pwb->Rect;
      
      MWin_InsideScreen(&Rect);
      
      u8 *pMask=&ScreenMask[Rect.x+(Rect.y*ScreenWidth)];
      
      for(s32 y=0;y<Rect.h;y++){ cwl();
        MemSet8DMA3(cnt,pMask,Rect.w);
        pMask+=ScreenWidth;
      }
    }
  }
}

void MWin_SetVisible(u32 WinIndex,bool Visible)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  pwb->Visible=Visible;
}

bool MWin_GetVisible(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  return(pwb->Visible);
}

void MWin_DrawTitleBar(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  if(pwb->WinBuf==NULL) return;
  
  u16 *pBuf;
  u16 col;
  
  s32 BufWidth=pwb->Rect.w;
  
  TRect Rect;
  
  Rect.x=0;
  Rect.y=0;
  Rect.w=pwb->Rect.w;
  Rect.h=TitleBarHeight;
  
  bool ActiveFlag;
  
  if(pwb->TopFlag==true){ cwl();
    ActiveFlag=true;
    }else{ cwl();
    ActiveFlag=false;
  }
  
  if(pwb->SubDisplay==true) ActiveFlag=true;
  
  u16 *pTitleBM;
  const TSkinBM *psbm=&SkinBM;
  
  if(ActiveFlag==true){ cwl();
    pTitleBM=psbm->pTitleABM;
    }else{
    pTitleBM=psbm->pTitleDBM;
  }
  
  if(pTitleBM!=NULL){ // BitmapTitleBar
    pBuf=&pwb->WinBuf[Rect.x+(Rect.y*BufWidth)];
    for(s32 y=0;y<TitleBarHeight;y++){ cwl();
      MemCopy16DMA3(&pTitleBM[0],&pBuf[0],BufWidth*2);
      pTitleBM+=psbm->TitleBM_W;
      pBuf+=BufWidth;
    }
    
    }else{ // default
    if(ActiveFlag==true){ cwl();
      col=MWC_TitleA_Bright;
      }else{ cwl();
      col=MWC_TitleD_Bright;
    }
    
    // TopLine
    pBuf=&pwb->WinBuf[Rect.x+(Rect.y*BufWidth)];
    MemSet16DMA3(col,pBuf,Rect.w*2);
    
    // LeftLine
    pBuf=&pwb->WinBuf[Rect.x+(Rect.y*BufWidth)];
    for(s32 y=0;y<Rect.h;y++){ cwl();
      pBuf[0]=col;
      pBuf+=BufWidth;
    }
    
    if(ActiveFlag==true){ cwl();
      col=MWC_TitleA_Dark;
      }else{ cwl();
      col=MWC_TitleD_Dark;
    }
    
    // BottomLine
    pBuf=&pwb->WinBuf[(Rect.y+Rect.h-1)*BufWidth];
    MemSet16DMA3(col,&pBuf[1],(Rect.w-1)*2);
    
    // RightLine
    pBuf=&pwb->WinBuf[Rect.x+Rect.w-1+(Rect.y*BufWidth)];
    pBuf+=BufWidth;
    for(s32 y=1;y<Rect.h;y++){ cwl();
      pBuf[0]=col;
      pBuf+=BufWidth;
    }
    
    if(ActiveFlag==true){ cwl();
      col=MWC_TitleA_BG;
      }else{ cwl();
      col=MWC_TitleD_BG;
    }
    
    // FillBG
    pBuf=&pwb->WinBuf[Rect.x+(Rect.y*BufWidth)];
    pBuf+=BufWidth;
    for(s32 y=1;y<TitleBarHeight-1;y++){ cwl();
      MemSet16DMA3(col,&pBuf[1],(BufWidth-1-1)*2);
      pBuf+=BufWidth;
    }
  }
  
  if(pwb->CloseButton==true){
    u16 *pBM=psbm->pCloseBtnBM;
    u32 BM_W=psbm->CloseBtnBM_W;
    u32 BM_H=psbm->CloseBtnBM_H;
    
    if(pBM!=NULL){ // Bitmap CloseButton
      pBuf=&pwb->WinBuf[(Rect.x+Rect.w-BM_W)+(Rect.y*BufWidth)];
      for(u32 y=0;y<BM_H;y++){
        for(u32 x=0;x<BM_W;x++){
          if(pBM[x]!=0) pBuf[x]=pBM[x];
        }
        pBM+=BM_H;
        pBuf+=BufWidth;
      }
    }
  }
  
  pBuf=&pwb->WinBuf[0];
  
  bool isDPG;
  {
    extern bool isExecMode_DPG(void);
    isDPG=isExecMode_DPG();
  }
  
  if((WinIndex==WM_PicView)&&(isDPG==false)){ cwl();
    char strbuf[256];
    
    {
      extern float Pic_GetRatio(void);
      sprintf(strbuf,"%d%%",(int)(Pic_GetRatio()*100));
    }
    
    if(ActiveFlag==true){ cwl();
      VRAM_WriteStringRect(pBuf,BufWidth,BufWidth,pwb->Rect.w-48+1,1+1,strbuf,MWC_TitleA_TextShadow);
      VRAM_WriteStringRect(pBuf,BufWidth,BufWidth,pwb->Rect.w-48+0,1+0,strbuf,MWC_TitleA_Text);
      }else{ cwl();
      VRAM_WriteStringRect(pBuf,BufWidth,BufWidth,pwb->Rect.w-48+0,1+0,strbuf,MWC_TitleD_Text);
    }
  }
  
  if((WinIndex==WM_PlayControl)||(WinIndex==WM_PicView)){ cwl();
    if(PlayControl_WindowTitleStrW[0]!=(UnicodeChar)0){ cwl();
      if(ActiveFlag==true){ cwl();
        VRAM_WriteStringRectW(pBuf,BufWidth,BufWidth,4+1,1+1,PlayControl_WindowTitleStrW,MWC_TitleA_TextShadow);
        VRAM_WriteStringRectW(pBuf,BufWidth,BufWidth,4+0,1+0,PlayControl_WindowTitleStrW,MWC_TitleA_Text);
        }else{ cwl();
        VRAM_WriteStringRectW(pBuf,BufWidth,BufWidth,4+0,1+0,PlayControl_WindowTitleStrW,MWC_TitleD_Text);
      }
      return;
    }
  }
  
  if(ActiveFlag==true){ cwl();
    VRAM_WriteStringRect(pBuf,BufWidth,BufWidth,4+1,1+1,pwb->TitleStr,MWC_TitleA_TextShadow);
    VRAM_WriteStringRect(pBuf,BufWidth,BufWidth,4+0,1+0,pwb->TitleStr,MWC_TitleA_Text);
    }else{ cwl();
    VRAM_WriteStringRect(pBuf,BufWidth,BufWidth,4+0,1+0,pwb->TitleStr,MWC_TitleD_Text);
  }
}

void MWin_DrawClientFrame(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  if(pwb->WinBuf==NULL) return;
  
  if(pwb->ClientRect.h==0) return;
  
  u32 BufWidth=pwb->Rect.w;
  
  TRect Rect;
  
  Rect.x=0;
  Rect.y=TitleBarHeight;
  Rect.w=pwb->Rect.w;
  Rect.h=pwb->Rect.h-Rect.y;
  
  u16 *pBuf;
  u16 col;
  
  col=MWC_FrameBright;
  
  pBuf=&pwb->WinBuf[Rect.x+(Rect.y*BufWidth)];
  MemSet16DMA3(col,pBuf,Rect.w*2);
  
  pBuf=&pwb->WinBuf[Rect.x+(Rect.y*BufWidth)];
  for(s32 y=0;y<Rect.h;y++){ cwl();
    pBuf[0]=col;
    pBuf+=BufWidth;
  }
  
  col=MWC_FrameDark;
  
  pBuf=&pwb->WinBuf[(Rect.y+Rect.h-1)*BufWidth];
  MemSet16DMA3(col,&pBuf[1],(Rect.w-1)*2);
  
  pBuf=&pwb->WinBuf[Rect.x+Rect.w-1+(Rect.y*BufWidth)];
  pBuf+=BufWidth;
  for(s32 y=1;y<Rect.h;y++){ cwl();
    pBuf[0]=col;
    pBuf+=BufWidth;
  }
  
}

void MWin_DrawClientBG(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  if(pwb->WinBuf==NULL) return;
  
  u16 col=pwb->ClientBGColor;
  
  if(pwb->Rect.h<=TitleBarHeight) return;
  if(col==0) return;
  
  u32 BufWidth=pwb->Rect.w;
  
  u16 *pBuf;
  
  u16 *pClientBM;
  u32 ClientBM_W;
  const TSkinBM *psbm=&SkinBM;
  
  switch(WinIndex){
    case WM_TextView: {
      pClientBM=psbm->pTextBM;
      ClientBM_W=psbm->TextBM_W;
    } break;
    default: {
      pClientBM=psbm->pClientBM;
      ClientBM_W=psbm->ClientBM_W;
    } break;
  }
  
  if(pClientBM!=NULL){ // BitmapBG
    pBuf=&pwb->WinBuf[pwb->ClientRect.x+(pwb->ClientRect.y)*BufWidth];
    for(s32 y=0;y<pwb->ClientRect.h;y++){ cwl();
      MemCopy16DMA3(pClientBM,pBuf,pwb->ClientRect.w*2);
      pClientBM+=ClientBM_W;
      pBuf+=BufWidth;
    }
    
    }else{ // default
    pBuf=&pwb->WinBuf[pwb->ClientRect.x+(pwb->ClientRect.y)*BufWidth];
    for(s32 y=0;y<pwb->ClientRect.h;y++){ cwl();
      MemSet16DMA3(col,pBuf,pwb->ClientRect.w*2);
      pBuf+=BufWidth;
    }
  }

}

void MWin_TransWindow(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  if(pwb->WinBuf==NULL) return;
  if(pwb->Visible==false) return;
  if(pwb->SubDisplay==true){
    u16 *pVRAMBuf=pScreenSub->GetVRAMBuf();
    u16 *pSrcBuf=&pwb->WinBuf[0];
    
    MemCopy32swi256bit(pSrcBuf,pVRAMBuf,ScreenHeight*ScreenWidth*2);
    return;
  }
  
  u32 BufWidth=pwb->Rect.w;
  
  TRect Rect=pwb->Rect;
  
  s32 ofsx,ofsy;
  
  ofsx=Rect.x;
  ofsy=Rect.y;
  
  MWin_InsideScreen(&Rect);
  
  ofsx=Rect.x-ofsx;
  ofsy=Rect.y-ofsy;
  
  u16 *pSrcBuf=&pwb->WinBuf[ofsx+(ofsy*BufWidth)];
  u16 *pVRAMBuf=pScreenMainOverlay->GetVRAMBuf();
  u16 *pDstBuf=&pVRAMBuf[Rect.x+(Rect.y*ScreenWidth)];
  
  if(pwb->TopFlag==true){ cwl();
    for(s32 y=0;y<Rect.h;y++){ cwl();
      MemCopy16DMA3(pSrcBuf,pDstBuf,Rect.w*2);
      pSrcBuf+=BufWidth;
      pDstBuf+=ScreenWidth;
    }
    return;
  }
  
  u8 *pMask=&ScreenMask[Rect.x+(Rect.y*ScreenWidth)];
  
  for(s32 y=0;y<Rect.h;y++){ cwl();
    for(s32 x=0;x<Rect.w;x++){ cwl();
      if(pMask[x]==WinIndex){ cwl();
        pDstBuf[x]=pSrcBuf[x];
      }
    }
    pSrcBuf+=BufWidth;
    pDstBuf+=ScreenWidth;
    pMask+=ScreenWidth;
  }
}

void MWin_ClearDesktop(void)
{ cwl();
  u16 *pDstBuf;
  u8 *pMask;
  u16 *pDesktopBuf;
  
  u16 *pVRAMBuf=pScreenMainOverlay->GetVRAMBuf();
  
  const TSkinBM *psbm=&SkinBM;
  
  pDstBuf=&pVRAMBuf[0];
  pMask=&ScreenMask[0];
  pDesktopBuf=&psbm->pDesktopBM[0];
  
  // 傎傫偲偼ScreenX偠傖側偔偰丄psbm->DesktopBM_X傪巊偆傋偒乧
  
  for(s32 y=0;y<ScreenHeight;y++){ cwl();
    for(s32 x=0;x<ScreenWidth;x++){ cwl();
      if(pMask[x]==0xff){ cwl();
        pDstBuf[x]=pDesktopBuf[x];
      }
    }
    pDstBuf+=ScreenWidth;
    pMask+=ScreenWidth;
    pDesktopBuf+=ScreenWidth;
  }
}

void MWin_DrawClient(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  if(pwb->WinBuf==NULL) return;
  if(pwb->Visible==false) return;
  
  MWin_DrawClientBG(WinIndex);
  
  switch(pwb->WinMode){ cwl();
    case WM_About: MWinCallAbout_Draw(pwb); break;
    case WM_Help: MWinCallHelp_Draw(pwb); break;
    case WM_PlayControl: MWinCallPlayControl_Draw(pwb,&SkinBM); break;
    case WM_FileSelect: MWinCallFileSelect_Draw(pwb,&SkinBM); break;
    case WM_DateTime: MWinCallDateTime_Draw(pwb); break;
    case WM_PicView: MWinCallPicView_Draw(pwb); break;
    case WM_TextView: MWinCallTextView_Draw(pwb); break;
    case WM_Progress: MWinCallProgress_Draw(pwb,&SkinBM); break;
    case WM_SetNext: MWinCallSetNext_Draw(pwb); break;
    case WM_FileInfo: MWinCallFileInfo_Draw(pwb); break;
  }
  
}

static bool InitSBImageFlag=false;
static u16 SBImageBG8[8*8]={
	0xE0,0x79,0x88,0x88,0x89,0x89,0x88,0xE0,
	0xE0,0x79,0x88,0x88,0x89,0x89,0x88,0xE0,
	0xE0,0x79,0x88,0x88,0x89,0x89,0x88,0xE0,
	0xE0,0x79,0x88,0x88,0x89,0x89,0x88,0xE0,
	0xE0,0x79,0x88,0x88,0x89,0x89,0x88,0xE0,
	0xE0,0x79,0x88,0x88,0x89,0x89,0x88,0xE0,
	0xE0,0x79,0x88,0x88,0x89,0x89,0x88,0xE0,
	0xE0,0x79,0x88,0x88,0x89,0x89,0x88,0xE0,
};
static u16 SBImageBody8[8*8]={
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
};
static u16 SBImagePart8[8*8]={
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x88,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0xB8,0xB8,0xB8,0xB8,0x79,0xE0,
	0xE0,0x79,0x89,0x79,0x79,0x79,0x79,0xE0,
	0xE0,0x79,0x88,0x89,0x79,0x79,0x79,0xE0,
};

void MWin_DrawSBar(u32 WinIndex,TSBar *psb)
{ cwl();
  if(InitSBImageFlag==false){
    InitSBImageFlag=true;
    
    u16 col;
    u32 r,g,b;
    
    col=MWC_ScrollBar;
    
    r=(col >> 0) & 0x1f;
    g=(col >> 5) & 0x1f;
    b=(col >> 10) & 0x1f;
    
    const TSkinBM *psbm=&SkinBM;
    
    if(psbm->pSB_BGBM!=NULL){
      MemCopy16DMA3(psbm->pSB_BGBM,SBImageBG8,8*8*2);
      }else{
      for(u32 cnt=0;cnt<8*8;cnt++){ cwl();
        u32 col=(((u32)SBImageBG8[cnt])*0x123/0x100)/8; // 0xe0 -> 0xff -> 0x1f
        SBImageBG8[cnt]=RGB15(col*r/0x20,col*g/0x20,col*b/0x20) | BIT(15);
      }
    }
    if(psbm->pSB_BodyBM!=NULL){
      MemCopy16DMA3(psbm->pSB_BodyBM,SBImageBody8,8*8*2);
      }else{
      for(u32 cnt=0;cnt<8*8;cnt++){ cwl();
        u32 col=(((u32)SBImageBody8[cnt])*0x123/0x100)/8; // 0xe0 -> 0xff -> 0x1f
        SBImageBody8[cnt]=RGB15(col*r/0x20,col*g/0x20,col*b/0x20) | BIT(15);
      }
    }
    if(psbm->pSB_PartBM!=NULL){
      MemCopy16DMA3(psbm->pSB_PartBM,SBImagePart8,8*8*2);
      }else{
      for(u32 cnt=0;cnt<8*8;cnt++){ cwl();
        u32 col=(((u32)SBImagePart8[cnt])*0x123/0x100)/8; // 0xe0 -> 0xff -> 0x1f
        SBImagePart8[cnt]=RGB15(col*r/0x20,col*g/0x20,col*b/0x20) | BIT(15);
      }
    }
  }
  
  TWinBody *pwb=&WinBody[WinIndex];
  
  if(pwb->WinBuf==NULL) return;
  
  TRect *pcr=&psb->ClientRect;
  
  if(psb->Enabled==false) return;
  
  u32 BufWidth=pwb->Rect.w;
  
  {
    u16 *pBuf=&pwb->WinBuf[pcr->x+(pcr->y*BufWidth)];
    u16 *pCol=&SBImageBG8[0];
    
    if(psb->isV==true){ cwl();
      for(s32 y=0;y<pcr->h;y++){ cwl();
        u16 *_pCol=&pCol[(y&7)*8];
        for(s32 x=0;x<8;x++){ cwl();
          pBuf[x]=_pCol[x];
        }
        pBuf+=BufWidth;
      }
      }else{ cwl();
      for(s32 y=0;y<8;y++){ cwl();
        u16 *_pCol=&pCol[y];
        for(s32 x=0;x<pcr->w;x++){ cwl();
          pBuf[x]=_pCol[(x&7)*8];
        }
        pBuf+=BufWidth;
      }
    }
  }
  
  if((psb->GripMax==0)||(psb->GripSize==0)) return;
  
  float fGripSize=(float)psb->GripSize/psb->GripMax;
  float fGripPos=(float)psb->GripPos/psb->GripMax;
  
  s32 iGripMax,iGripSize,iGripPos;
  
  if(psb->isV==true){ cwl();
    iGripMax=pcr->h;
    }else{ cwl();
    iGripMax=pcr->w;
  }
  iGripSize=(s32)(fGripSize*iGripMax);
  iGripPos=(s32)(fGripPos*iGripMax);
  
  if(iGripSize>iGripMax){ cwl();
    iGripPos=0;
    iGripSize=iGripMax;
  }
  
  if(iGripSize<8) iGripSize=8;
  if(iGripMax<(iGripPos+iGripSize)) iGripPos=iGripMax-iGripSize;
  
  u16 *pBuf=&pwb->WinBuf[pcr->x+(pcr->y*BufWidth)];
  
//  _consolePrintf("%f %f %d %d %d\n",fGripSize,fGripPos,iGripSize,iGripPos,iGripMax);
  
  if(psb->isV==true){ cwl();
    u16 *_pBuf;
    u16 *pCol;
    
    for(int y=0;y<4;y++){
      pCol=&SBImagePart8[y*8];
      _pBuf=&pBuf[(iGripPos+y)*BufWidth];
      for(s32 x=0;x<8;x++){ cwl();
        _pBuf[x]=pCol[x];
      }
    }
    for(int y=0;y<4;y++){
      pCol=&SBImagePart8[(4+y)*8];
      _pBuf=&pBuf[(iGripPos+iGripSize-4+y)*BufWidth];
      for(s32 x=0;x<8;x++){ cwl();
        _pBuf[x]=pCol[x];
      }
    }
    pCol=&SBImageBody8[0];
    _pBuf=&pBuf[(iGripPos+4)*BufWidth];
    for(s32 y=4;y<iGripSize-4;y++){ cwl();
      u16 *_pCol=&pCol[(y&7)*8];
      for(s32 x=0;x<8;x++){ cwl();
        _pBuf[x]=_pCol[x];
      }
      _pBuf+=BufWidth;
    }
    }else{ cwl();
    u16 *_pBuf;
    u16 *pCol;
    
    for(int y=0;y<4;y++){
      pCol=&SBImagePart8[y*8];
      _pBuf=&pBuf[(iGripPos+y)];
      for(s32 x=0;x<8;x++){ cwl();
        *_pBuf=pCol[x];
        _pBuf+=BufWidth;
      }
    }
    for(int y=0;y<4;y++){
      pCol=&SBImagePart8[(4+y)*8];
      _pBuf=&pBuf[(iGripPos+iGripSize-4+y)];
      for(s32 x=0;x<8;x++){ cwl();
        *_pBuf=pCol[x];
        _pBuf+=BufWidth;
      }
    }
    pCol=&SBImageBody8[0];
    _pBuf=&pBuf[(iGripPos+0)];
    for(s32 y=0;y<8;y++){ cwl();
      u16 *_pCol=&pCol[y];
      for(s32 x=2;x<(iGripSize-2);x++){
        _pBuf[x]=_pCol[(x&7)*8];
      }
      _pBuf+=BufWidth;
    }
  }
}

void MWin_DrawSBarVH(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  if(pwb->WinBuf==NULL) return;
  
  MWin_DrawSBar(WinIndex,&pwb->SBarV);
  MWin_DrawSBar(WinIndex,&pwb->SBarH);
  
  if((pwb->SBarV.Enabled==true)&&(pwb->SBarH.Enabled==true)){
    int ox,oy;
    
    ox=pwb->ClientRect.x+pwb->ClientRect.w;
    oy=pwb->ClientRect.y+pwb->ClientRect.h;
    
    for(int y=0;y<8;y++){
      u16 *pbuf=&pwb->WinBuf[ox+((oy+y)*pwb->Rect.w)];
      for(int x=0;x<8;x++){
        pbuf[x]=MWC_ClientBG;
      }
    }
  }
}

void MWin_RefreshWindow(u32 WinIndex)
{ cwl();
  MWin_DrawTitleBar(WinIndex);
  MWin_DrawClientFrame(WinIndex);
  MWin_DrawSBarVH(WinIndex);
  MWin_DrawClient(WinIndex);
  MWin_TransWindow(WinIndex);
}

void MWin_SetWindowTitle(u32 WinIndex,char *TitleStr)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
//  sprintf(pwb->TitleStr,"%s",TitleStr);
  
  for(u32 cnt=0;cnt<128;cnt++){ cwl();
    pwb->TitleStr[cnt]=TitleStr[cnt];
  }
  
  MWin_DrawTitleBar(WinIndex);
}

void MWin_PlayControl_SetWindowTitleW(UnicodeChar *TitleStrW)
{ cwl();
  if(TitleStrW==NULL){ cwl();
    PlayControl_WindowTitleStrW[0]=(UnicodeChar)0;
    return;
  }
  
  for(u32 cnt=0;cnt<512;cnt++){ cwl();
    PlayControl_WindowTitleStrW[cnt]=TitleStrW[cnt];
  }
}

TWinBody* MWin_GetWinBody(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  return(pwb);
}

void MWin_SetSBarV(u32 WinIndex,s32 GripMax,s32 GripSize,s32 GripPos)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  TSBar *psb=&pwb->SBarV;
  
  psb->GripMax=GripMax;
  psb->GripSize=GripSize;
  
  MWin_SetSBarVPos(WinIndex,GripPos);
}

void MWin_SetSBarH(u32 WinIndex,s32 GripMax,s32 GripSize,s32 GripPos)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  TSBar *psb=&pwb->SBarH;
  
  psb->GripMax=GripMax;
  psb->GripSize=GripSize;
  
  MWin_SetSBarHPos(WinIndex,GripPos);
}

void MWin_SetSBarVPos(u32 WinIndex,s32 GripPos)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  TSBar *psb=&pwb->SBarV;
  
  if(psb->GripMax<(GripPos+psb->GripSize)) GripPos=psb->GripMax-psb->GripSize;
  if(GripPos<0) GripPos=0;
  
  psb->GripPos=GripPos;
}

void MWin_SetSBarHPos(u32 WinIndex,s32 GripPos)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  TSBar *psb=&pwb->SBarH;
  
  if(psb->GripMax<(GripPos+psb->GripSize)) GripPos=psb->GripMax-psb->GripSize;
  if(GripPos<0) GripPos=0;
  
  psb->GripPos=GripPos;
}

s32 MWin_GetSBarVPos(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  if(pwb->SBarV.Enabled==false) return(-1);
  
  return(pwb->SBarV.GripPos);
}

s32 MWin_GetSBarHPos(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  if(pwb->SBarH.Enabled==false) return(-1);
  
  return(pwb->SBarH.GripPos);
}

void MWin_SetSBarVPosFromPixel(u32 WinIndex,s32 px)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  TSBar *psb=&pwb->SBarV;
  
  float pos;
  
  pos=px*psb->GripMax;
  pos/=psb->ClientRect.h;
  pos-=psb->GripSize/2;
  psb->GripPos=(u32)pos;
  
  if(psb->GripPos>(psb->GripMax-psb->GripSize)) psb->GripPos=psb->GripMax-psb->GripSize;
  if(psb->GripPos<0) psb->GripPos=0;
}

void MWin_SetSBarHPosFromPixel(u32 WinIndex,s32 px)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  TSBar *psb=&pwb->SBarH;
  
  float pos;
  
  pos=px*psb->GripMax;
  pos/=psb->ClientRect.w;
  pos-=psb->GripSize/2;
  psb->GripPos=(u32)pos;
  
  if(psb->GripPos>(psb->GripMax-psb->GripSize)) psb->GripPos=psb->GripMax-psb->GripSize;
  if(psb->GripPos<0) psb->GripPos=0;
}

u32 MWin_GetWindowIndexFromPos(s32 x,s32 y)
{ cwl();
  u32 SortIndex[WinBodyCount];
  u32 SortPriority[WinBodyCount];
  
  for(u32 cnt=0;cnt<WinBodyCount;cnt++){ cwl();
    TWinBody *pwb=&WinBody[cnt];
    
    SortIndex[cnt]=cnt;
    SortPriority[cnt]=pwb->Priority;
  }
  
  for(u32 c0=0;c0<WinBodyCount-1;c0++){ cwl();
    for(u32 c1=c0+1;c1<WinBodyCount;c1++){ cwl();
      if(SortPriority[c0]>SortPriority[c1]){ cwl();
        u32 tmp;
        
        tmp=SortIndex[c0];
        SortIndex[c0]=SortIndex[c1];
        SortIndex[c1]=tmp;
        
        tmp=SortPriority[c0];
        SortPriority[c0]=SortPriority[c1];
        SortPriority[c1]=tmp;
      }
    }
  }
  
  u32 WinIndex=0xff;
  
  for(u32 sortcnt=0;sortcnt<WinBodyCount;sortcnt++){ cwl();
    u32 cnt=SortIndex[sortcnt];
    TWinBody *pwb=&WinBody[cnt];
    
    if((pwb->Visible==true)&&(pwb->SubDisplay==false)){ cwl();
      if((pwb->Rect.x<=x)&&(x<(pwb->Rect.x+pwb->Rect.w))){ cwl();
        if((pwb->Rect.y<=y)&&(y<(pwb->Rect.y+pwb->Rect.h))){ cwl();
          WinIndex=cnt;
        }
      }
    }
  }
  
  return(WinIndex);
}

u32 MWin_GetTopWinIndex(void)
{ cwl();
  for(u32 cnt=0;cnt<WinBodyCount;cnt++){ cwl();
    TWinBody *pwb=&WinBody[cnt];
    if(pwb->Visible==true){ cwl();
      if(pwb->TopFlag==true){ cwl();
        return(cnt);
      }
    }
  }
  
  return(0xff);
}

s32 MWin_GetClientWidth(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  return(pwb->ClientRect.w);
}

s32 MWin_GetClientHeight(u32 WinIndex)
{ cwl();
  TWinBody *pwb=&WinBody[WinIndex];
  
  return(pwb->ClientRect.h);
}

u32 ActiveWinIndex=0xff;
s32 mcx,mcy,mlx,mly;
bool HoldTitleBar=false;
bool HoldCloseButton=false;
bool HoldSBarV=false;
bool HoldSBarH=false;
bool HoldClient=false;

bool CheckInsideRect(TRect Rect,s32 x,s32 y)
{ cwl();
  if((Rect.x<=x)&&(x<(Rect.x+Rect.w))){ cwl();
    if((Rect.y<=y)&&(y<(Rect.y+Rect.h))){ cwl();
      return(true);
    }
  }
  return(false);
}

void MWin_MouseDown(s32 x,s32 y)
{ cwl();
  ActiveWinIndex=MWin_GetWindowIndexFromPos(x,y);
  
  u32 DeactiveWinIndex=MWin_GetTopWinIndex();
  
  if(ActiveWinIndex!=DeactiveWinIndex){ cwl();
    if(DeactiveWinIndex!=0xff){ cwl();
      TWinBody *pwb=&WinBody[DeactiveWinIndex];
      
      pwb->TopFlag=false;
      MWin_DrawTitleBar(DeactiveWinIndex);
      MWin_TransWindow(DeactiveWinIndex);
    }
    
    if(ActiveWinIndex!=0xff){ cwl();
      MWin_SetActive(ActiveWinIndex);
      MWin_RefreshScreenMask();
      
      MWin_DrawTitleBar(ActiveWinIndex);
      MWin_TransWindow(ActiveWinIndex);
    }
  }
  
  if(ActiveWinIndex==0xff) return;
  
  TWinBody *pwb=&WinBody[ActiveWinIndex];
  
  mcx=x-pwb->Rect.x;
  mcy=y-pwb->Rect.y;
  
  HoldTitleBar=false;
  HoldCloseButton=false;
  HoldSBarV=false;
  HoldSBarH=false;
  HoldClient=false;
  
  if(mcy<TitleBarHeight){
    if(mcx<(pwb->Rect.w-TitleBarHeight)){
      if(pwb->WinMode==WM_PicView){ // Picture window locked position.
        }else{
        HoldTitleBar=true;
      }
      }else{
      HoldCloseButton=true;
    }
  }
  
  if(pwb->SBarV.Enabled==true){ cwl();
    if(CheckInsideRect(pwb->SBarV.ClientRect,mcx,mcy)==true) HoldSBarV=true;
  }
  
  if(pwb->SBarH.Enabled==true){ cwl();
    if(CheckInsideRect(pwb->SBarH.ClientRect,mcx,mcy)==true) HoldSBarH=true;
  }
  
  if(CheckInsideRect(pwb->ClientRect,mcx,mcy)==true) HoldClient=true;
  
  if(HoldTitleBar==true){};
  if(HoldCloseButton==true){};
  
  if(HoldSBarV==true){ cwl();
    TSBar *psb=&pwb->SBarV;
    
    s32 px=mcy-psb->ClientRect.y;
    MWin_SetSBarVPosFromPixel(ActiveWinIndex,px);
    MWin_DrawSBar(ActiveWinIndex,&pwb->SBarV);
    MWin_DrawClient(ActiveWinIndex);
    MWin_TransWindow(ActiveWinIndex);
  }
  
  if(HoldSBarH==true){ cwl();
    TSBar *psb=&pwb->SBarH;
    
    s32 px=mcx-psb->ClientRect.x;
    MWin_SetSBarHPosFromPixel(ActiveWinIndex,px);
    MWin_DrawSBar(ActiveWinIndex,&pwb->SBarH);
    MWin_DrawClient(ActiveWinIndex);
    MWin_TransWindow(ActiveWinIndex);
  }
  
  if(HoldClient==true){ cwl();
    s32 x=mcx-pwb->ClientRect.x;
    s32 y=mcy-pwb->ClientRect.y;
    
    switch(pwb->WinMode){ cwl();
      case WM_About: MWinCallAbout_MouseDown(ActiveWinIndex,x,y); break;
      case WM_Help: MWinCallHelp_MouseDown(ActiveWinIndex,x,y); break;
      case WM_PlayControl: MWinCallPlayControl_MouseDown(ActiveWinIndex,x,y); break;
      case WM_FileSelect: MWinCallFileSelect_MouseDown(ActiveWinIndex,x,y); break;
      case WM_DateTime: MWinCallDateTime_MouseDown(ActiveWinIndex,x,y); break;
      case WM_PicView: MWinCallPicView_MouseDown(ActiveWinIndex,x,y); break;
      case WM_TextView: MWinCallTextView_MouseDown(ActiveWinIndex,x,y); break;
      case WM_Progress: MWinCallProgress_MouseDown(ActiveWinIndex,x,y); break;
      case WM_SetNext: MWinCallSetNext_MouseDown(ActiveWinIndex,x,y); break;
      case WM_FileInfo: MWinCallFileInfo_MouseDown(ActiveWinIndex,x,y); break;
    }
  };
  
  mlx=mcx;
  mly=mcy;
  
}

void MWin_MouseMove(s32 x,s32 y)
{ cwl();
  if(ActiveWinIndex==0xff) return;
  
  TWinBody *pwb=&WinBody[ActiveWinIndex];
  
  mcx=x-pwb->Rect.x;
  mcy=y-pwb->Rect.y;
  
  if((mcx==mlx)&&(mcy==mly)) return;
  
  if(HoldTitleBar==true){ cwl();
    pwb->Rect.x+=mcx-mlx;
    pwb->Rect.y+=mcy-mly;
    MWin_RefreshScreenMask();
    MWin_TransWindow(ActiveWinIndex);
    MWin_ClearDesktop();
    for(u32 cnt=0;cnt<WinBodyCount;cnt++){ cwl();
      if(cnt!=ActiveWinIndex) MWin_TransWindow(cnt);
    }
    mcx=x-pwb->Rect.x;
    mcy=y-pwb->Rect.y;
    
    if(VideoOverlay==true){ cwl();
      if(ActiveWinIndex==WM_PicView){ cwl();
        MWin_RefreshVideoFullScreen(false);
      }
    }
  }
  
  if(HoldSBarV==true){ cwl();
    TSBar *psb=&pwb->SBarV;
    
    s32 px=mcy-psb->ClientRect.y;
    MWin_SetSBarVPosFromPixel(ActiveWinIndex,px);
    MWin_DrawSBar(ActiveWinIndex,&pwb->SBarV);
    MWin_DrawClient(ActiveWinIndex);
    MWin_TransWindow(ActiveWinIndex);
  }
  
  if(HoldSBarH==true){ cwl();
    TSBar *psb=&pwb->SBarH;
    
    s32 px=mcx-psb->ClientRect.x;
    MWin_SetSBarHPosFromPixel(ActiveWinIndex,px);
    MWin_DrawSBar(ActiveWinIndex,&pwb->SBarH);
    MWin_DrawClient(ActiveWinIndex);
    MWin_TransWindow(ActiveWinIndex);
  }
  
  if(HoldClient==true){ cwl();
    s32 x=mcx-pwb->ClientRect.x;
    s32 y=mcy-pwb->ClientRect.y;
    
    switch(pwb->WinMode){ cwl();
      case WM_About: MWinCallAbout_MouseMove(ActiveWinIndex,x,y); break;
      case WM_Help: MWinCallHelp_MouseMove(ActiveWinIndex,x,y); break;
      case WM_PlayControl: MWinCallPlayControl_MouseMove(ActiveWinIndex,x,y); break;
      case WM_FileSelect: MWinCallFileSelect_MouseMove(ActiveWinIndex,x,y); break;
      case WM_DateTime: MWinCallDateTime_MouseMove(ActiveWinIndex,x,y); break;
      case WM_PicView: MWinCallPicView_MouseMove(ActiveWinIndex,x,y); break;
      case WM_TextView: MWinCallTextView_MouseMove(ActiveWinIndex,x,y); break;
      case WM_Progress: MWinCallProgress_MouseMove(ActiveWinIndex,x,y); break;
      case WM_SetNext: MWinCallSetNext_MouseMove(ActiveWinIndex,x,y); break;
      case WM_FileInfo: MWinCallFileInfo_MouseMove(ActiveWinIndex,x,y); break;
    }
  };
  
  mlx=mcx;
  mly=mcy;
  
}

void MWin_MouseUp(void)
{ cwl();
  if(ActiveWinIndex==0xff) return;
  
  TWinBody *pwb=&WinBody[ActiveWinIndex];
  
  if(HoldCloseButton==true){
    switch(pwb->WinMode){ cwl();
      case WM_About: MWinCallAbout_CloseButton(ActiveWinIndex); break;
      case WM_Help: MWinCallHelp_CloseButton(ActiveWinIndex); break;
      case WM_PlayControl: MWinCallPlayControl_CloseButton(ActiveWinIndex); break;
      case WM_FileSelect: MWinCallFileSelect_CloseButton(ActiveWinIndex); break;
      case WM_DateTime: MWinCallDateTime_CloseButton(ActiveWinIndex); break;
      case WM_PicView: MWinCallPicView_CloseButton(ActiveWinIndex); break;
      case WM_TextView: MWinCallTextView_CloseButton(ActiveWinIndex); break;
      case WM_Progress: MWinCallProgress_CloseButton(ActiveWinIndex); break;
      case WM_SetNext: MWinCallSetNext_CloseButton(ActiveWinIndex); break;
      case WM_FileInfo: MWinCallFileInfo_CloseButton(ActiveWinIndex); break;
    }
  }
  
  if(HoldTitleBar==true){};
  if(HoldSBarV==true){};
  if(HoldSBarH==true){};
  
  if(HoldClient==true){ cwl();
    s32 x=mcx-pwb->ClientRect.x;
    s32 y=mcy-pwb->ClientRect.y;
    
    switch(pwb->WinMode){ cwl();
      case WM_About: MWinCallAbout_MouseUp(ActiveWinIndex,x,y); break;
      case WM_Help: MWinCallHelp_MouseUp(ActiveWinIndex,x,y); break;
      case WM_PlayControl: MWinCallPlayControl_MouseUp(ActiveWinIndex,x,y); break;
      case WM_FileSelect: MWinCallFileSelect_MouseUp(ActiveWinIndex,x,y); break;
      case WM_DateTime: MWinCallDateTime_MouseUp(ActiveWinIndex,x,y); break;
      case WM_PicView: MWinCallPicView_MouseUp(ActiveWinIndex,x,y); break;
      case WM_TextView: MWinCallTextView_MouseUp(ActiveWinIndex,x,y); break;
      case WM_Progress: MWinCallProgress_MouseUp(ActiveWinIndex,x,y); break;
      case WM_SetNext: MWinCallSetNext_MouseUp(ActiveWinIndex,x,y); break;
      case WM_FileInfo: MWinCallFileInfo_MouseUp(ActiveWinIndex,x,y); break;
    }
  }
  
  ActiveWinIndex=0xff;
  
  HoldTitleBar=false;
  HoldSBarV=false;
  HoldSBarH=false;
  HoldClient=false;
  
  mcx=0;
  mcy=0;
  mlx=mcx;
  mly=mcy;
}
void MWin_ShowHelpDialog()
{

#define HIGHT 18
	const TSkinBM *psbm=&SkinBM;
	char tmpstr[256];
	UnicodeChar Filename[128];
	u16 *pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
	u16 *pHelpBM=&psbm->pHelpBM[0];
	pDstBuf+=16*256+18;
	for(u32 y=0;y<psbm->HelpBM_H;y++)
	{
		//for(u32 x=0;x<psbm->ConfigBM_W;x++)
		{
			MemCopy16DMA3(pHelpBM,pDstBuf,psbm->HelpBM_W*2);
		}
		pDstBuf+=256;
		pHelpBM+=220;
	}
	pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
	sprintf(tmpstr,"%s",gl_strAboutMsg);
	StrConvert_Local2Unicode(tmpstr,Filename);
	VRAM_WriteStringRectW(pDstBuf,256,256,24,20,Filename,MWC_TitleD_Text);				

	printf(tmpstr,"%s","V 1.31");
	StrConvert_Local2Unicode("V1.31",Filename);
	VRAM_WriteStringRectW(pDstBuf,256,256,188,20,Filename,MWC_TitleD_Text);			
	
	sprintf(tmpstr,"%s",gl_strAMsg);
	StrConvert_Local2Unicode(tmpstr,Filename);
	VRAM_WriteStringRectW(pDstBuf,256,256,30,40,Filename,MWC_TitleD_Text);		
	
	sprintf(tmpstr,"%s",gl_strBMsg);
	StrConvert_Local2Unicode(tmpstr,Filename);
	VRAM_WriteStringRectW(pDstBuf,256,256,30,40+HIGHT,Filename,MWC_TitleD_Text);				

	sprintf(tmpstr,"%s",gl_strLRMsg);
	StrConvert_Local2Unicode(tmpstr,Filename);
	VRAM_WriteStringRectW(pDstBuf,256,256,30,40+2*HIGHT,Filename,MWC_TitleD_Text);				

	sprintf(tmpstr,"%s",gl_strLXMsg);
	StrConvert_Local2Unicode(tmpstr,Filename);
	VRAM_WriteStringRectW(pDstBuf,256,256,30,40+3*HIGHT,Filename,MWC_TitleD_Text);				


	sprintf(tmpstr,"%s",gl_strBaseMoonShell);
	StrConvert_Local2Unicode(tmpstr,Filename);
	VRAM_WriteStringRectW(pDstBuf,256,256,30,154-HIGHT,Filename,MWC_TitleD_Text);		
	
	sprintf(tmpstr,"%s",gl_strThanksMoonShell);
	StrConvert_Local2Unicode(tmpstr,Filename);
	VRAM_WriteStringRectW(pDstBuf,256,256,30,154,Filename,MWC_TitleD_Text);				

}

void MWin_ShowDesktop()
{

	const TSkinBM *psbm=&SkinBM;
	u16 *pDstBuf;
  	u16 *pDesktopBuf;
	pDstBuf=pScreenMainOverlay->GetVRAMBuf();  
	pDesktopBuf=&psbm->pDesktopBM[0];
	MemCopy16DMA3(pDesktopBuf,pDstBuf,93804);//93804 = 256*192*2
}
void MWin_ShowNdsRomIcon(uint32 *bm,uint16 *pal,uint32 xOff,uint32 yOff,const UnicodeChar *str)
{

	const TSkinBM *psbm=&SkinBM;
	 u16 *pDstBuf;
  	 u16 *pDesktopBuf;
  
  	 pDstBuf=pScreenMainOverlay->GetVRAMBuf();  
	 pDesktopBuf=&psbm->ppDesktopBnBM[0];
  
  	// 傎傫偲偼ScreenX偠傖側偔偰丄psbm->DesktopBM_X傪巊偆傋偒乧
// 	 pDesktopBuf+=28672;//57344=256*112*2
 	 pDstBuf+=28672;
 	 MemCopy16DMA3(pDesktopBuf,pDstBuf,40960);//40960 = 256*80*2
  
	pal[0]=0;
	for(u32 idx=1;idx<0x10;idx++){
      pal[idx]|=BIT15;
    }
	uint16 *pBM=pScreenMainOverlay->GetVRAMBuf();
	pBM +=256*yOff+xOff;
    u32 *psrc=bm;
	//画图标
    for(u32 y=0;y<4;y++)
	{
      for(u32 x=0;x<4;x++)
	  {
        u16 *pdst=&pBM[(y*8*256)+x*8];
        for(u32 yo=0;yo<8;yo++)
		{
          u32 data=*psrc++;
          for(u32 xo=0;xo<8;xo++){
            pdst[xo]=pal[data&0x0f]; data>>=4;
          }
          pdst+=256;
        }
      }
    }
    pBM=pScreenMainOverlay->GetVRAMBuf();
	//UnicodeChar Filename[0x80];
	//StrConvert_Local2Unicode(str,Filename);
	VRAM_WriteStringRectW(pBM,256,256,xOff+36,yOff,str,MWC_TitleD_Text);

	//写存档类型
	char tmpstr[256];
	switch(gl_SaveType)
	{
	case 0:
		
sprintf(tmpstr,"%s: %s",gl_strSaveType,"4K EEPROM");
		 break;
	case 1:
		
sprintf(tmpstr,"%s: %s",gl_strSaveType,"64K EEPROM");
		 break;
	case 2:
		
sprintf(tmpstr,"%s: %s",gl_strSaveType,"128K EEPROM");
		 break;
	case 3:
		
sprintf(tmpstr,"%s: %s",gl_strSaveType,"256K EEPROM");
		 break;
	case 4:
		
sprintf(tmpstr,"%s: %s",gl_strSaveType,"512K EEPROM");
		 break;
	case 5:
		
sprintf(tmpstr,"%s: %s",gl_strSaveType,"2M Flash");
		 break;
	case 6:
		sprintf(tmpstr,"%s: %s",gl_strSaveType,"4M Flash");
		 break;
	case 0xFF:
		sprintf(tmpstr,"%s: %s",gl_strSaveType,"None Save");
		break;
	case 0xFE:
		
sprintf(tmpstr,"%s: %s",gl_strSaveType,"64K EEPROM");
		 break;
	default:
		break;
	}
	UnicodeChar Filename[64];
  	StrConvert_Local2Unicode(tmpstr,Filename);
   	VRAM_WriteStringRectW(pBM,256,256,xOff+36,yOff+20,Filename,MWC_TitleD_Text);
	
}
void MWin_ShowConfigDialog()
{

	const TSkinBM *psbm=&SkinBM;
	//char tmpstr[256];
	//UnicodeChar Filename[128];
	u16 *pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
	u16 *pConfigBM=&psbm->pConfigBM[0];
	pDstBuf+=28*256+33;
	for(u32 y=0;y<psbm->ConfigBM_H;y++)
	{
		//for(u32 x=0;x<psbm->ConfigBM_W;x++)
		{
			MemCopy16DMA3(pConfigBM,pDstBuf,psbm->ConfigBM_W*2);
		}
		pDstBuf+=256;
		pConfigBM+=189;
	}
}
void MWIn_ShowSelcetItem(u8 language,u8 speedAuto,u8 focusIndex,u16 speed,u16 speedDefault)
{
	const TSkinBM *psbm=&SkinBM;
	char tmpstr[256];
	UnicodeChar Filename[128];
	u16 *pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
	u16 *pConfigBM=&psbm->pConfigBM[0];
	u16 *pCheckedBM=&psbm->pCheckedBM[0];
	u16 *pUnCheckBM=&psbm->pCheckBM[0];
	u16 *p=NULL;
	//画中文选项
		switch(language)
		{
		case 1:
			p=pCheckedBM;
			break;
		case 2:
			p=pUnCheckBM;
			break;
		default:
			p=pCheckedBM;
			break;
		}
		pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
		pDstBuf += 256*51+45;
		for(u32 y=0;y<12;y++)
		{
			for(u32 x=0;x<12;x++)
			{
				MemCopy16DMA3(p,pDstBuf,24);
			}
			pDstBuf+=256;
			p+=12;
		}
		sprintf(tmpstr,"%s","中文");
	    StrConvert_Local2Unicode(tmpstr,Filename);
		pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
		if(focusIndex==0)
		    VRAM_WriteStringRectW(pDstBuf,256,256,63,51,Filename,MWC_TitleD_Red);
		else 
			VRAM_WriteStringRectW(pDstBuf,256,256,63,51,Filename,MWC_TitleD_Text);

		switch(language)
		{
		case 1:
			p=pUnCheckBM;
			break;
		case 2:
			p=pCheckedBM;
			break;
		default:
			p=pUnCheckBM;
			break;
		}
		pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
		pDstBuf += 256*51+140;
		for(u32 y=0;y<12;y++)
		{
			for(u32 x=0;x<12;x++)
			{
				MemCopy16DMA3(p,pDstBuf,24);
			}
			pDstBuf+=256;
			p+=12;
		}
		sprintf(tmpstr,"%s","English");
	    StrConvert_Local2Unicode(tmpstr,Filename);
		pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
		if(focusIndex==0)
		    VRAM_WriteStringRectW(pDstBuf,256,256,152,51,Filename,MWC_TitleD_Red);	
		else
			 VRAM_WriteStringRectW(pDstBuf,256,256,152,51,Filename,MWC_TitleD_Text);				
	//画速度设置
		pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
		//pUnCheckBM=&psbm->pCheckBM[0];
		if(speedAuto==1)
			p = pUnCheckBM;
		else
			p = pCheckedBM;
		pDstBuf += 256*63+45;
		for(u32 y=0;y<12;y++)
		{
			for(u32 x=0;x<12;x++)
			{
				MemCopy16DMA3(p,pDstBuf,24);
			}
			pDstBuf+=256;
			p+=12;
		}
		sprintf(tmpstr,"%s   %d",gl_speedset,speed/100);
	    StrConvert_Local2Unicode(tmpstr,Filename);
		
		pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
		pDstBuf+=63*256+63;
		p = pConfigBM;
		p+= 35*189+30;
		for(u32 y=0;y<12;y++)
		{	
			//for(u32 x=0;x<126;x++)
			{
				MemCopy16DMA3(p,pDstBuf,159*2);
			}
			pDstBuf+=256;
			pConfigBM+=189;
		}
		
		pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
		if(focusIndex==1)
		    VRAM_WriteStringRectW(pDstBuf,256,256,63,63,Filename,MWC_TitleD_Red);	
		else
			VRAM_WriteStringRectW(pDstBuf,256,256,63,63,Filename,MWC_TitleD_Text);		
	//速度自动检测
		pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
		if(speedAuto==1)
			p = pCheckedBM;
		else
			p = pUnCheckBM;
		pDstBuf += 256*75+45;
		for(u32 y=0;y<12;y++)
		{
			for(u32 x=0;x<12;x++)
			{
				MemCopy16DMA3(p,pDstBuf,24);
			}
			pDstBuf+=256;
			p+=12;
		}
		sprintf(tmpstr,"%s",gl_speedAutoDetect);
	    StrConvert_Local2Unicode(tmpstr,Filename);
		pDstBuf = pScreenMainOverlay->GetVRAMBuf();  
		if(focusIndex==2)
		    VRAM_WriteStringRectW(pDstBuf,256,256,63,75,Filename,MWC_TitleD_Red);	
		else
			VRAM_WriteStringRectW(pDstBuf,256,256,63,75,Filename,MWC_TitleD_Text);		

		sprintf(tmpstr,"%s",gl_strSaveQuit);
	    StrConvert_Local2Unicode(tmpstr,Filename);
		VRAM_WriteStringRectW(pDstBuf,256,256,45,146,Filename,MWC_TitleD_Text);					

		sprintf(tmpstr,"%s",gl_strQuit);
	    StrConvert_Local2Unicode(tmpstr,Filename);
		VRAM_WriteStringRectW(pDstBuf,256,256,147,146,Filename,MWC_TitleD_Text);					
		
}
void MWin_ShowFileDetail(uint32 xOff,uint32 yOff)
{
	const TSkinBM *psbm=&SkinBM;
}
void MWin_ShowProgressDialog(uint type,uint16 pos,u16 range)
{
	const TSkinBM *psbm=&SkinBM;
	char tmpstr[256];
	UnicodeChar name[128];
	u16 *pDstBuf = pScreenSub->GetVRAMBuf();  
	u16 *pProgressBM=&psbm->pProgressBM[0];
	u16 *pProgressBar1BM=&psbm->pProgressBarBM1[0];
	if(pos==0)
	{
		pDstBuf+=66*256+44;
		for(u32 y=0;y<psbm->ProgressBM_H;y++)
		{
			//for(u32 x=0;x<psbm->ConfigBM_W;x++)
			{
				MemCopy16DMA3(pProgressBM,pDstBuf,psbm->ProgressBM_W*2);
			}
			pDstBuf+=256;
			pProgressBM+=168;
		}
		if(type!=3)
		{
			//画进度条
			pDstBuf = pScreenSub->GetVRAMBuf();  
			pDstBuf+=100*256+70;
			for(u32 y=0;y<psbm->ProgressBarBM1_H;y++)
			{
				MemCopy16DMA3(pProgressBar1BM,pDstBuf,psbm->ProgressBarBM1_W*2);
				pDstBuf+=256;
				pProgressBar1BM+=116;
			}
		}
		switch(type)
		{
		case 1:	// Load save
			sprintf(tmpstr,"%s",gl_strLoadSave);
			StrConvert_Local2Unicode(tmpstr,name);
			pDstBuf = pScreenSub->GetVRAMBuf();  
			VRAM_WriteStringRectW(pDstBuf,256,256,62,70,name,MWC_TitleD_Text);
			break;
		case 2:   //write save
			sprintf(tmpstr,"%s",gl_strBackSave);
			StrConvert_Local2Unicode(tmpstr,name);
			pDstBuf = pScreenSub->GetVRAMBuf();  
			VRAM_WriteStringRectW(pDstBuf,256,256,74,70,name,MWC_TitleD_Text);
			break;
		case 3:
			sprintf(tmpstr,"%s",gl_strLoadGame);
			StrConvert_Local2Unicode(tmpstr,name);
			pDstBuf = pScreenSub->GetVRAMBuf();  
			VRAM_WriteStringRectW(pDstBuf,256,256,98,100,name,MWC_TitleD_Text);
			break;
		default:
			break;
		}
	}
	if(type!=3)
	{
		//画进度
		u16 *pProgressBar2BM=&psbm->pProgressBarBM2[0];
		for(s32 i=0;i<pos;i++)
		{
			pDstBuf = pScreenSub->GetVRAMBuf();  
			pProgressBar2BM=&psbm->pProgressBarBM2[0];
			pDstBuf+=100*256+70+i*10;
			for(u32 y=0;y<psbm->ProgressBarBM2_H;y++)
			{
				MemCopy16DMA3(pProgressBar2BM,pDstBuf,psbm->ProgressBarBM2_W*2);
				pDstBuf+=256;
				pProgressBar2BM+=10;
			}
			sprintf(tmpstr,"%d%s",i*10,"%");
			StrConvert_Local2Unicode(tmpstr,name);
			pDstBuf = pScreenSub->GetVRAMBuf();  
			pProgressBM=&psbm->pProgressBM[0];
			pDstBuf+=112*256+100;
			pProgressBM+=46*168+56;
			for(u32 y=0;y<12;y++)
			{
				MemCopy16DMA3(pProgressBM,pDstBuf,96);
				pDstBuf+=256;
				pProgressBM+=168;
			}
			pDstBuf = pScreenSub->GetVRAMBuf();  
			VRAM_WriteStringRectW(pDstBuf,256,256,124,112,name,MWC_TitleD_Text);
		}
		
	}
}
#include "mwin_progress.h"
#include "mwin_overlay.h"

// ----------------------------------------------------

