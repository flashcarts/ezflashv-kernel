
u32 VideoOverlayWidth=ScreenWidth;
u32 VideoOverlayHeight=ScreenHeight;
float VideoOverlayRatio=0;

void MWin_InitVideoOverlay(void)
{ cwl();
}

void MWin_SetVideoWideFlag(bool w)
{
  pScreenMain->SetWideFlag(w);
}

void MWin_SetVideoOverlay(bool e)
{ cwl();
  VideoOverlay=e;
  
  if((VideoOverlay==true)&&(VideoFullScreen==true)&&(VideoOverlayRatio==0)){ cwl();
#ifndef DPGTimeProfile
    IPC3->LCDPowerControl=LCDPC_ON_TOP_LEDON;
#else
    IPC3->LCDPowerControl=LCDPC_ON_BOTH;
#endif
    }else{ cwl();
    IPC3->LCDPowerControl=LCDPC_ON_BOTH;
  }
}

void MWin_ClearVideoOverlay(void)
{ cwl();
  MemSet16DMA3(RGB15(0,0,0)|BIT(15),pScreenMain->pCanvas->GetVRAMBuf(),384*1024);
  return;
  
  MemSet16DMA3(RGB15(0,0,0)|BIT(15),pScreenMain->pCanvas->GetVRAMBuf(),pScreenMain->pCanvas->GetWidth()*pScreenMain->pCanvas->GetHeight()*2);
  
  if(pScreenMain->GetWideFlag()==false){
    pScreenMain->Flip(true);
    MemSet16DMA3(RGB15(0,0,0)|BIT(15),pScreenMain->pCanvas->GetVRAMBuf(),pScreenMain->pCanvas->GetWidth()*pScreenMain->pCanvas->GetHeight()*2);
    pScreenMain->Flip(true);
  }
}

void MWin_SetVideoOverlaySize(u32 w,u32 h,float ratio)
{ cwl();
  VideoOverlayWidth=w;
  VideoOverlayHeight=h;
  VideoOverlayRatio=ratio;
}

void MWin_SetVideoFullScreen(bool e)
{ cwl();
  VideoFullScreen=e;
  
  if((VideoOverlay==true)&&(VideoFullScreen==true)&&(VideoOverlayRatio==0)){ cwl();
    IPC3->LCDPowerControl=LCDPC_ON_TOP_LEDON;
    }else{ cwl();
    IPC3->LCDPowerControl=LCDPC_ON_BOTH;
  }
}

bool MWin_GetVideoFullScreen(void)
{ cwl();
  return(VideoFullScreen);
}

void MWin_RefreshVideoFullScreen(bool IgnoreRect)
{ cwl();
  // FullScreenPicture
  TRect Rect;
  TRect ClientRect;
  
  if(IgnoreRect==false){
    TWinBody *pwb=&WinBody[WM_PicView];
    Rect=pwb->Rect;
    ClientRect=pwb->ClientRect;
    }else{
    Rect.x=0;
    Rect.y=0;
    Rect.w=256;
    Rect.h=192;
    ClientRect=Rect;
  }
  
  TRect r;
  bool TopMost;
  
  bool _VideoFullScreen;
  
  if(VideoOverlayRatio==0){
    _VideoFullScreen=VideoFullScreen;
    }else{
    _VideoFullScreen=false;
  }
  
  if(_VideoFullScreen==false){ cwl();
    r.x=Rect.x+ClientRect.x;
    r.y=Rect.y+ClientRect.y;
    r.w=ClientRect.w;
    r.h=ClientRect.h;
    
    TopMost=false;
    }else{ cwl();
    r.x=0;
    r.y=0;
    r.w=ScreenWidth;
    r.h=ScreenHeight;
    
    TopMost=true;
  }
  
  if(VideoOverlay==false) TopMost=false;
  
  float ratio;
  
  if(VideoOverlayRatio!=0){
    ratio=VideoOverlayRatio;
    }else{
    if((VideoOverlayWidth==0)||(VideoOverlayHeight==0)){
      _consolePrintf("VideoOverlaySize setting error.\n");
      ShowLogHalt();
      return;
    }
    
    float ratiox=(float)r.w/VideoOverlayWidth;
    float ratioy=(float)r.h/VideoOverlayHeight;
    
    if(ratiox<ratioy){ cwl();
      ratio=ratiox;
      }else{ cwl();
      ratio=ratioy;
    }
  }
  
  if(_VideoFullScreen==true){ cwl();
    r.x=(int)((ScreenWidth-(VideoOverlayWidth*ratio))/2);
    r.y=(int)((ScreenHeight-(VideoOverlayHeight*ratio))/2);
  }
  
  _consolePrintf("ScreenRatio x%f\n",ratio);
  
  pScreenMainOverlay->SetVisible(!TopMost);
  
  pScreenMain->SetViewport(r.x,r.y,ratio,ratio);
  
  if(GlobalINI.System.FullScreenOverlaySubScreen==true){ cwl();
    if((VideoOverlay==true)&&(_VideoFullScreen==true)){ cwl();
      POWER_CR |= POWER_SWAP_LCDS;
      }else{ cwl();
      POWER_CR &= ~POWER_SWAP_LCDS;
    }
  }
}

