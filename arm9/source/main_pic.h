
static float Pic_Ratio;
static u32 Pic_BaseX,Pic_BaseY;

typedef struct {
  bool Enabled;
  ufix8 fix8Step;
  u32 CurLine;
  u32 gr,gg,gb;
} TImageSmooth;

static TImageSmooth ImageSmooth;

TRect ImageRect;

static int Pic_GetWidth(void)
{
  switch(ExecMode){ cwl();
    case EM_MSPImage: return(pPluginBody->pIL->GetWidth()); break;
    default: return(0); break;
  }
  return(0);
}

static int Pic_GetHeight(void)
{
  switch(ExecMode){ cwl();
    case EM_MSPImage: return(pPluginBody->pIL->GetHeight()); break;
    default: return(0); break;
  }
  return(0);
}

static void Pic_InitPos(void)
{
  ImageRect.x=0;
  ImageRect.y=0;
  
  switch(imgcalc_GetImageMode()){
    case EIM_Single: {
      ImageRect.w=ScreenWidth;
      ImageRect.h=ScreenHeight;
    } break;
    case EIM_Double: {
      ImageRect.w=ScreenWidth;
      ImageRect.h=ScreenHeight+imgcalc_GetVerticalPadding()+ScreenHeight;
    } break;
  }
  
  Pic_BaseX=0;
  Pic_BaseY=0;
}

static void Pic_SetPos(int x,int y)
{
  // FullScreenPicture
  TRect ClientRect=ImageRect;
  
  int pw=Pic_GetWidth();
  int ph=Pic_GetHeight();
  
  int cw=(int)(ClientRect.w/Pic_Ratio)+1;
  int ch=(int)(ClientRect.h/Pic_Ratio)+1;
  
  if(pw<=(x+cw)) x=pw-cw-1;
  if(x<0) x=0;
  if(ph<=(y+ch)) y=ph-ch-1;
  if(y<0) y=0;
  
  switch(ExecMode){ cwl();
    case EM_MSPImage:{
      Pic_BaseX=x;
      Pic_BaseY=y;
    } break;
    default: break;
  }
}

static int Pic_GetPosX(void)
{
  switch(ExecMode){ cwl();
    case EM_MSPImage: return(Pic_BaseX); break;
    default: return(0); break;
  }
  return(0);
}

static int Pic_GetPosY(void)
{
  switch(ExecMode){ cwl();
    case EM_MSPImage: return(Pic_BaseY); break;
    default: return(0); break;
  }
  return(0);
}

void Pic_SetSBar(float lastratio)
{
  int w=Pic_GetWidth();
  int h=Pic_GetHeight();
  
  if((w==0)||(h==0)) return;
  
  int x=Pic_GetPosX();
  int y=Pic_GetPosY();
  int cw=ImageRect.w;
  int ch=ImageRect.h;
  
  if(lastratio!=Pic_Ratio){
    x+=((int)(((cw+(lastratio/2))/lastratio)-((cw+(Pic_Ratio/2))/Pic_Ratio)+0.5)+1)/2;
    y+=((int)(((ch+(lastratio/2))/lastratio)-((ch+(Pic_Ratio/2))/Pic_Ratio)+0.5)+1)/2;
  }
  
  MWin_SetSBarH(WM_PicView,w,(int)(cw/Pic_Ratio)+1,x);
  MWin_SetSBarV(WM_PicView,h,(int)(ch/Pic_Ratio)+1,y);
}

void Pic_SetRatio(float ratio)
{
  Pic_Ratio=ratio;
}

float Pic_GetRatio(void)
{
  return(Pic_Ratio);
}

static void Pic_RefreshScreen(void)
{
  if(ExecMode!=EM_MSPImage) return;
  
  TRect ClientRect=ImageRect;
  EImageMode EIM=imgcalc_GetImageMode();
  
  u16 *pUpBuf=pScreenSub->GetVRAMBuf();
  u16 *pDownBuf=pScreenMain->GetVRAMBuf(ScrMainID_View); // flip‚ÍŽg‚í‚È‚¢
  u32 BufWidth=pScreenMain->pCanvas->GetWidth();
  
  bool HalfMode;
  
  if(0.5<Pic_Ratio){
    HalfMode=false;
    }else{
    HalfMode=true;
  }
  
  // Clear padding area
  
  int PicW=Pic_GetWidth();
  int PicH=Pic_GetHeight();
  int ScrW=ImageRect.w;
  int ScrH=ImageRect.h;
  int vps=imgcalc_GetVerticalPadding();
  
  if(HalfMode==true){
    PicW/=2;
    PicH/=2;
    ScrW=(int)(ScrW/(Pic_Ratio*2));
    ScrH=(int)(ScrH/(Pic_Ratio*2));
    }else{
    ScrW=(int)(ScrW/(Pic_Ratio*1));
    ScrH=(int)(ScrH/(Pic_Ratio*1));
  }
  
  if(ScrH<PicH) PicH=ScrH;
  if(ScrW<PicW) PicW=ScrW;
  
  switch(EIM){
    case EIM_Single: {
      if(PicW<ScrW){
        for(int y=0;y<PicH;y++){
          MemSet16DMA3(RGB15(0,0,0)|BIT15,&pDownBuf[PicW+(y*BufWidth)],(ScrW-PicW)*2);
        }
      }
      for(int y=PicH;y<ScrH;y++){
        MemSet16DMA3(RGB15(0,0,0)|BIT15,&pDownBuf[0+(y*BufWidth)],(ScrW-0)*2);
      }
    } break;
    case EIM_Double: {
      if(PicW<ScrW){
        for(int y=0;y<PicH;y++){
          if(y<ScreenHeight){
            MemSet16DMA3(RGB15(0,0,0)|BIT15,&pUpBuf[PicW+(y*BufWidth)],(ScrW-PicW)*2);
            }else{
            if(y<(ScreenHeight+vps)){
              }else{
              MemSet16DMA3(RGB15(0,0,0)|BIT15,&pDownBuf[PicW+((y-(ScreenHeight+vps))*BufWidth)],(ScrW-PicW)*2);
            }
          }
        }
      }
      for(int y=PicH;y<ScrH;y++){
        if(y<ScreenHeight){
          MemSet16DMA3(RGB15(0,0,0)|BIT15,&pUpBuf[0+(y*BufWidth)],(ScrW-0)*2);
          }else{
          if(y<(ScreenHeight+vps)){
            }else{
            MemSet16CPU(RGB15(0,0,0)|BIT15,&pDownBuf[0+((y-(ScreenHeight+vps))*BufWidth)],(ScrW-0)*2);
          }
        }
      }
    } break;
  }
  
  ClientRect.x=0;
  ClientRect.y=0;
  ClientRect.w=(int)(ClientRect.w/Pic_Ratio);
  ClientRect.h=(int)(ClientRect.h/Pic_Ratio);
  
  if((ClientRect.w!=0)&&(ClientRect.h!=0)){
    switch(ExecMode){ cwl();
      case EM_MSPImage: {
        switch(EIM){
          case EIM_Single: {
            pPluginBody->pIL->RefreshScreen(Pic_BaseX,Pic_BaseY,pDownBuf,BufWidth,ClientRect.w,ClientRect.h,HalfMode);
          } break;
          case EIM_Double: {
            TRect tmprect=ClientRect;
            int vpsr=(int)(vps/Pic_Ratio);
            tmprect.h=(tmprect.h-vpsr)/2;
            pPluginBody->pIL->RefreshScreen(Pic_BaseX,Pic_BaseY,pUpBuf,BufWidth,tmprect.w,tmprect.h,HalfMode);
            {
              u16 tmpc[256];
              pPluginBody->pIL->RefreshScreen(Pic_BaseX,Pic_BaseY+tmprect.h,&tmpc[0],1,1,vpsr,HalfMode);
            }
            pPluginBody->pIL->RefreshScreen(Pic_BaseX,Pic_BaseY+tmprect.h+vpsr,pDownBuf,BufWidth,tmprect.w,tmprect.h,HalfMode);
          } break;
        }
      } break;
      default: break;
    }
  }
  
  ImageControlTimeOut_ProcReset();
}

void ImageControlTimeOut_ProcReset_callback(void)
{
  TImageSmooth *pim=&ImageSmooth;
  
  pim->Enabled=false;
  
  if(Pic_Ratio<=0.5) return;
  if(Pic_Ratio==1.0) return;
  if(imgcalc_GetImageMode()!=EIM_Single) return;
  
  pim->Enabled=true;
  pim->fix8Step=(ufix8)((1/Pic_Ratio)*SFIX8VALUE);
  pim->CurLine=0;
  
  pim->gr=0;
  pim->gg=0;
  pim->gb=0;
}

#define Gravity(c,cg) { \
  c+=cg; \
  cg=c&0xffff; \
  c=c>>16; \
  if((c&(~0x1f))!=0) c=(c<0) ? 0x00 : 0x1f; \
}

bool ImageControlTimeOut_ProcVSync_callback(u32 VsyncCount)
{
  TImageSmooth *pim=&ImageSmooth;
  if(pim->Enabled==false) return(false);
  
  if(pim->CurLine==ScreenHeight) return(true);
  
  u32 gr=pim->gr,gg=pim->gg,gb=pim->gb;
  
  ufix8 fix8Step=pim->fix8Step;
  
  u32 SrcX,SrcY;
  ufix8 fix8SrcModX,fix8SrcModY;
  
  fix8SrcModX=0*fix8Step;
  fix8SrcModY=pim->CurLine*fix8Step;
  SrcX=fix8SrcModX >> 8;
  SrcY=fix8SrcModY >> 8;
  fix8SrcModX&=0xff;
  fix8SrcModY&=0xff;
  
  u32 DstX,DstY;
  
  DstX=0;
  DstY=pim->CurLine;
  
  u16 *pSrcBuf=pScreenMain->GetVRAMBuf(ScrMainID_View);
  u32 SrcBufWidth=pScreenMain->pCanvas->GetWidth();
  u16 *pSrcBuf0=&pSrcBuf[(SrcY+0)*SrcBufWidth];
  u16 *pSrcBuf1=&pSrcBuf[(SrcY+1)*SrcBufWidth];
  
  u16 *pDstBuf=pScreenMainOverlay->GetVRAMBuf();
  u32 DstBufWidth=pScreenMainOverlay->pCanvas->GetWidth();
  pDstBuf=&pDstBuf[DstY*DstBufWidth];
  
  u32 sp0r,sp0g,sp0b;
  u32 sp1r,sp1g,sp1b;
  
  {
    u32 spr,spg,spb;
    
    ufix8 mod;
    u32 sp;
    
    mod=SFIX8VALUE-fix8SrcModY;
    sp=pSrcBuf0[SrcX];
    spr=((sp>>0)&0x1f)*mod;
    spg=((sp>>5)&0x1f)*mod;
    spb=((sp>>10)&0x1f)*mod;
    
    mod=fix8SrcModY;
    sp=pSrcBuf1[SrcX];
    spr+=((sp>>0)&0x1f)*mod;
    spg+=((sp>>5)&0x1f)*mod;
    spb+=((sp>>10)&0x1f)*mod;
    
    sp0r=spr;
    sp0g=spg;
    sp0b=spb;
  }
  {
    u32 spr,spg,spb;
    
    ufix8 mod;
    u32 sp;
    
    mod=SFIX8VALUE-fix8SrcModY;
    sp=pSrcBuf0[SrcX+1];
    spr=((sp>>0)&0x1f)*mod;
    spg=((sp>>5)&0x1f)*mod;
    spb=((sp>>10)&0x1f)*mod;
    
    mod=fix8SrcModY;
    sp=pSrcBuf1[SrcX+1];
    spr+=((sp>>0)&0x1f)*mod;
    spg+=((sp>>5)&0x1f)*mod;
    spb+=((sp>>10)&0x1f)*mod;
    
    sp1r=spr;
    sp1g=spg;
    sp1b=spb;
  }
  
  while(1){
    {
      u32 spr,spg,spb;
      ufix8 imod;
      
      imod=SFIX8VALUE-fix8SrcModX;
      spr=(sp0r*imod)+(sp1r*fix8SrcModX);
      Gravity(spr,gr);
      spg=(sp0g*imod)+(sp1g*fix8SrcModX);
      Gravity(spg,gg);
      spb=(sp0b*imod)+(sp1b*fix8SrcModX);
      Gravity(spb,gb);
      
      *pDstBuf=RGB15(spr,spg,spb) | BIT15;
    }
    
    fix8SrcModX+=fix8Step;
    while(0x100<=fix8SrcModX){
      fix8SrcModX-=0x100;
      SrcX++;
      
      sp0r=sp1r;
      sp0g=sp1g;
      sp0b=sp1b;
      
      {
        u32 spr,spg,spb;
        
        ufix8 mod;
        u32 sp;
        
        mod=SFIX8VALUE-fix8SrcModY;
        sp=pSrcBuf0[SrcX+1];
        spr=((sp>>0)&0x1f)*mod;
        spg=((sp>>5)&0x1f)*mod;
        spb=((sp>>10)&0x1f)*mod;
        
        mod=fix8SrcModY;
        sp=pSrcBuf1[SrcX+1];
        spr+=((sp>>0)&0x1f)*mod;
        spg+=((sp>>5)&0x1f)*mod;
        spb+=((sp>>10)&0x1f)*mod;
        
        sp1r=spr;
        sp1g=spg;
        sp1b=spb;
      }
    }
    
    DstX++;
    if(DstX==ScreenWidth) break;
    pDstBuf++;
  }
  
  pim->CurLine++;
  
  pim->gr=gr;
  pim->gg=gg;
  pim->gb=gb;
  
  return(true);
}

