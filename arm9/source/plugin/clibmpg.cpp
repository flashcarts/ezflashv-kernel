
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>
#include <NDS/ARM9/CP15.h>

#include "../../ipc3.h"

#include "../emulator.h"
#include "../_const.h"
#include "../_console.h"
#include "../_consoleWriteLog.h"
#include "../glib/glib.h"
#include "../memtool.h"
#include "../arm9tcm.h"
#include "../inifile.h"
#include "../mwin.h"

#include "clibdpg.h"

#ifndef USEDPG

Clibmpg::Clibmpg(CStream *_pCStream,const u32 _TotalFrameCount,const u32 _FPS,const u32 _SndFreq,const EMPGPixelFormat _PixelFormat):TotalFrameCount(_TotalFrameCount),FPS(_FPS),SndFreq(_SndFreq),PixelFormat(_PixelFormat)
{
}

Clibmpg::~Clibmpg(void)
{
}

void Clibmpg::Reopen(u32 StartFrame,u32 StartOffset)
{
}

int Clibmpg::ProcReadBuffer(void)
{
  return(0);
}

bool Clibmpg::ProcSequence(void)
{
  return(false);
}

int Clibmpg::GetWidth(void) const
{
  return(0);
}

int Clibmpg::GetHeight(void) const
{
  return(0);
}

bool Clibmpg::ProcMoveFrame(u32 TargetFrame,u64 TargetSamplesCount)
{
  return(false);
}

bool Clibmpg::ProcMoveFrameGOP(u32 TargetFrame,u64 TargetSamplesCount,u32 TargetGOPFrame,u32 TargetGOPOffset)
{
  return(false);
}

void Clibmpg::SetVideoDelayms(int ms)
{
}

bool Clibmpg::ProcDecode(u64 CurrentSamplesCount)
{
  return(false);
}

int Clibmpg::GetFrameNum(void)
{
  return(0);
}

void Clibmpg::debug_tableout(void)
{
}

void Clibmpg::YUV420toBGR15_Init(int Width,int Height)
{
}

void Clibmpg::YUV420toBGR15_CopyFull(u8 *_YBuf,u8 *_UBuf,u8 *_VBuf,u16 *_FrameBuf)
{
}

#define DECLARE_COPY(func,_LU,_RU,_LD,_RD) \
void func(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf) \
{ \
}

DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyFull,1,1,1,1)
DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyLU,1,0,0,0)
DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyRU,0,1,0,0)
DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyLD,0,0,1,0)
DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyRD,0,0,0,1)

#undef DECLARE_COPY

#define DECLARE_COPY(func) \
void func(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf) \
{ \
}

DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyHalf)

#undef DECLARE_COPY

#else

extern void ShowLogHalt(void);

extern volatile u32 reqflip;

typedef struct {
  bool AdaptiveDelayControl;
  u32 msGOPSkip;
  u32 msFull,msHalf,msQuater;
} TAjuster;

static TAjuster stAjuster;

#define ReadBufSize (8*1024)

Clibmpg::Clibmpg(CStream *_pCStream,const u32 _TotalFrameCount,const u32 _FPS,const u32 _SndFreq,const EMPGPixelFormat _PixelFormat):TotalFrameCount(_TotalFrameCount),FPS(_FPS),SndFreq(_SndFreq),PixelFormat(_PixelFormat)
{
  Initialized=false;
  
  {
    TAjuster Ajuster;
    
    Ajuster.AdaptiveDelayControl=GlobalINI.DPGPlugin.AdaptiveDelayControl;
    
    Ajuster.msGOPSkip=100000;
    Ajuster.msFull=100000;
    Ajuster.msHalf=100000;
    Ajuster.msQuater=100000;
    
    if(GlobalINI.DPGPlugin.GOPSkip==true) Ajuster.msGOPSkip=400;
    
    switch(GlobalINI.DPGPlugin.Dithering){
      case EDPGD_NoCnt: {
        // Full only
      } break;
      case EDPGD_HalfCnt: {
        Ajuster.msFull=200;
        Ajuster.msHalf=400;
        Ajuster.msQuater=400;
      } break;
      case EDPGD_QuatCnt: {
        Ajuster.msFull=200;
        Ajuster.msHalf=400;
      } break;
      case EDPGD_HeavyCnt: {
        Ajuster.msFull=100;
        Ajuster.msHalf=200;
        Ajuster.msQuater=300;
      } break;
    }
    
    stAjuster=Ajuster;
  }
  
  MemSet16DMA3(RGB15(0,0,0)|BIT(15),DrawBuf,ScreenWidth*ScreenHeight*2);
  MemSet16DMA3(RGB15(0,0,0)|BIT(15),pScreenMain->GetVRAMBuf(ScrMainID_Back),ScreenWidth*ScreenHeight*2);
  MemSet16DMA3(RGB15(0,0,0)|BIT(15),pScreenMain->GetVRAMBuf(ScrMainID_View),ScreenWidth*ScreenHeight*2);
  
  pCStream=_pCStream;
  DataTopPosition=pCStream->GetOffset();
  
  ReadBuf=(u8*)malloc(ReadBufSize);
  
  GlobalDelaySamples=0;
  LastDelaySample=0;
  
  framenum=0;
  
  decoder=mpeg2_init();
  if(decoder==NULL){
    _consolePrintf("Could not allocate a decoder object.\n");
    return;
  }
  info=mpeg2_info(decoder);
  
  if(ProcSequence()==false) return;
  
  Initialized=true;
  
  return;
}

Clibmpg::~Clibmpg(void)
{
  if(Initialized==true){
    Initialized=false;
    
    if(decoder!=NULL){
      mpeg2_close(decoder); decoder=NULL;
    }
    info=NULL;
    
    if(ReadBuf!=NULL){
      free(ReadBuf); ReadBuf=NULL;
    }
  }
}

void Clibmpg::Reopen(u32 StartFrame,u32 StartOffset)
{
  if(Initialized==true){
    Initialized=false;
    
    if(decoder!=NULL){
      mpeg2_close(decoder); decoder=NULL;
    }
    info=NULL;
  }
  
  framenum=StartFrame;
  
  pCStream->SetOffset(DataTopPosition+(StartOffset&~3));
  
  decoder=mpeg2_init();
  if(decoder==NULL){
    _consolePrintf("Could not allocate a decoder object.\n");
    return;
  }
  info=mpeg2_info(decoder);
  
  if(ProcSequence()==false) return;
  
  Initialized=true;
}

int Clibmpg::ProcReadBuffer(void)
{
  int readsize;
  
  readsize=pCStream->ReadBuffer32bit(ReadBuf,ReadBufSize);
  if(readsize!=0) mpeg2_buffer (decoder, ReadBuf,&ReadBuf[readsize]);
  
  return(readsize);
}

bool Clibmpg::ProcSequence(void)
{
  while(1){
    mpeg2_state_t state=mpeg2_parse(decoder);
    
    switch (state) {
      case STATE_BUFFER: {
        if(ProcReadBuffer()==0) return(false);
      } break;
      case STATE_SEQUENCE: {
        Width=info->sequence->width;
        Height=info->sequence->height;
        int ChromaWidth=info->sequence->chroma_width;
        int ChromaHeight=info->sequence->chroma_height;
        _consolePrintf("ProcSeq:Width=%d,Height=%d\n",Width,Height);
        _consolePrintf("ProcSeq:ChromaWidth=%d,Height=%d\n",ChromaWidth,ChromaHeight);
        YUV420toBGR15_Init(Width,Height);
        return(true);
      } break;
      case STATE_GOP: break;
      case STATE_SLICE: {
        _consolePrintf("ProcSeq:StateError found first.(Slice/End/InvalidEnd)\n");
        return(false);
      } break;
      default: break;
    }
  }
}

int Clibmpg::GetWidth(void) const
{
  return(Width);
}

int Clibmpg::GetHeight(void) const
{
  return(Height);
}

bool Clibmpg::ProcMoveFrame(u32 TargetFrame,u64 TargetSamplesCount)
{
  if(framenum==TargetFrame) return(true);
  
  bool prgbar;
  bool defull;
  
  if(TargetFrame<framenum){
    Reopen(0,0);
    prgbar=true;
    }else{
    if((FPS*5/0x100)<=(TargetFrame-framenum)){
      prgbar=true;
      }else{
      prgbar=false;
    }
  }
  
  if(TargetFrame==0) return(true);
  
  defull=false;
  
  if(prgbar==true){
    if(MWin_GetVideoFullScreen()==true){
      extern void ProcessTouchPad_DPG(void);
      ProcessTouchPad_DPG();
      defull=true;
    }
  }
  
  mpeg2_skip(decoder,true);
  
  int prgofs=framenum;
  
  if(prgbar==true) MWin_ProgressShow("seek frame...",TargetFrame-prgofs);
  
  while(1){
    mpeg2_state_t state=mpeg2_parse(decoder);
    
    switch (state) {
      case STATE_BUFFER: {
        if(ProcReadBuffer()==0) return(false);
      } break;
      case STATE_SEQUENCE: break;
      case STATE_GOP: break;
      case STATE_SLICE: {
        framenum++;
        if((framenum&63)==0){
          if(prgbar==true) MWin_ProgressSetPos(framenum-prgofs);
        }
        if(TargetFrame<=framenum){
          mpeg2_skip(decoder,false);
          if(prgbar==true) MWin_ProgressHide();
          if(defull==true){
            MWin_SetVideoFullScreen(true);
            MWin_RefreshVideoFullScreen(false);
          }
          return(true);
        }
      } break;
      default: break;
    }
  }
  
  return(false);
}

bool Clibmpg::ProcMoveFrameGOP(u32 TargetFrame,u64 TargetSamplesCount,u32 TargetGOPFrame,u32 TargetGOPOffset)
{
  _consolePrintf("framenum=%d TargetFrame=%d\n",framenum,TargetFrame);
  
  if(framenum==TargetFrame) return(true);
  
  if((TargetGOPFrame<=framenum)&&(framenum<=TargetFrame)){
    }else{
    Reopen(TargetGOPFrame,TargetGOPOffset&~3);
  }
  
  if(framenum==TargetFrame) return(true);
  
  bool prgbar;
  bool defull;
  
  if((FPS*2/0x100)<(TargetFrame-framenum)){
    prgbar=true;
    }else{
    prgbar=false;
  }
  
  defull=false;
  
  if(prgbar==true){
    if(MWin_GetVideoFullScreen()==true){
      extern void ProcessTouchPad_DPG(void);
      ProcessTouchPad_DPG();
      defull=true;
    }
  }
  
  int prgofs=framenum;
  
  if(prgbar==true) MWin_ProgressShow("seek frame...",TargetFrame-prgofs);
  
  while(1){
    mpeg2_state_t state=mpeg2_parse(decoder);
    
    switch (state) {
      case STATE_BUFFER: {
        if(ProcReadBuffer()==0) return(false);
      } break;
      case STATE_SEQUENCE: break;
      case STATE_GOP: break;
      case STATE_SLICE: {
        framenum++;
        if((framenum&3)==0){
          if(prgbar==true) MWin_ProgressSetPos(framenum-prgofs);
        }
        if(TargetFrame<=framenum){
          if(prgbar==true) MWin_ProgressHide();
          if(defull==true){
            MWin_SetVideoFullScreen(true);
            MWin_RefreshVideoFullScreen(false);
          }
          return(true);
        }
      } break;
      default: break;
    }
  }
  
  return(false);
}

void Clibmpg::SetVideoDelayms(int ms)
{
  GlobalDelaySamples=(u32)(ms*SndFreq/1000);
  LastDelaySample=0;
}

//#define DPGTimeProfile

extern "C" {
  void mpeg2_idct_SetClipMode(bool UseClip);
}

bool Clibmpg::ProcDecode(u64 CurrentSamplesCount)
{
  TAjuster Ajuster=stAjuster;
  
  u32 delayms;
  
  {
    u32 _LastDelaySample=LastDelaySample;
    
    u64 framesamples=(u64)framenum*SndFreq*0x100/FPS;
    
    if(framesamples<GlobalDelaySamples){
      framesamples=0;
      }else{
      framesamples-=GlobalDelaySamples;
    }
    
    if(Ajuster.AdaptiveDelayControl==true){
      if(framesamples<_LastDelaySample){
        framesamples=0;
        }else{
        framesamples-=_LastDelaySample;
      }
    }
    
    if(CurrentSamplesCount<framesamples) return(true);
    
    _LastDelaySample=(u32)(CurrentSamplesCount-framesamples);
    
    delayms=_LastDelaySample*1000/SndFreq;
    
    LastDelaySample=_LastDelaySample;
  }
  
/*
  static u32 a;
  if(CurrentSamplesCount==0) a=0;
//  a+=delayms;
  if((framenum%(FPS/0x100))==0) _consolePrintf("%d totaldelay=%4dms curdelay=%4dms\n",framenum,a,delayms);
  if(framenum==256) ShowLogHalt();
*/
  
#ifdef DPGTimeProfile
  bool enprf=false;
  
  if((framenum%(FPS/0x100))==0){
    _consolePrintf("ProcDec:%d,gap%4dms\n",framenum,delayms);
//    PrintFreeMem();
    enprf=true;
  }
#endif
  
  framenum++;
  if(TotalFrameCount<=framenum){
    _consolePrintf("End of video stream.\n");
    return(false);
  }
//  _consolePrintf("Delay=%5dms\n",delayms);
  
  static bool skipflag=false;
  
#ifdef DPGTimeProfile
  if(enprf==true) PrfStart();
#endif
  
  if((skipflag==false)&&(delayms<Ajuster.msQuater)) reqflip=3;
  
  while(1){
    if(delayms<Ajuster.msFull){
      mpeg2_idct_SetClipMode(true);
      }else{
      mpeg2_idct_SetClipMode(false);
    }
    
    mpeg2_state_t state=mpeg2_parse(decoder);
//    _consolePrintf("[st%d]",state);
    
    switch (state) {
      case STATE_BUFFER: {
        if(ProcReadBuffer()==0) return(false);
      } break;
      case STATE_SEQUENCE: break;
      case STATE_GOP: {
        if(Ajuster.msGOPSkip<delayms){
          if(skipflag==false){
            skipflag=true;
            mpeg2_skip(decoder,skipflag);
            _consolePrintf("DropGOP");
          }
          }else{
          if(skipflag==true){
            skipflag=false;
            mpeg2_skip(decoder,skipflag);
          }
        }
      } break;
      case STATE_SLICE: {
        if((skipflag==false)&&(info->display_fbuf)){
          // ARM9 processing converter
          bool reqtrans=true;
          
          if(Width!=ScreenWidth){
            YUV420toBGR15_CopyFull(info->display_fbuf->buf[0],info->display_fbuf->buf[1],info->display_fbuf->buf[2],DrawBuf);
            }else{
            if(delayms<Ajuster.msFull){
              YUV420toBGR15Fix_CopyFull(info->display_fbuf->buf[0],info->display_fbuf->buf[1],info->display_fbuf->buf[2],DrawBuf);
              }else{
              if(delayms<Ajuster.msHalf){
                YUV420toBGR15Fix_CopyHalf(info->display_fbuf->buf[0],info->display_fbuf->buf[1],info->display_fbuf->buf[2],DrawBuf);
                }else{
                if(delayms<Ajuster.msQuater){
                  static int skiptype=0;
                  skiptype++;
                  if(skiptype==4) skiptype=0;
                  switch(skiptype){
                    case 0: YUV420toBGR15Fix_CopyLU(info->display_fbuf->buf[0],info->display_fbuf->buf[1],info->display_fbuf->buf[2],DrawBuf); break;
                    case 1: YUV420toBGR15Fix_CopyRD(info->display_fbuf->buf[0],info->display_fbuf->buf[1],info->display_fbuf->buf[2],DrawBuf); break;
                    case 2: YUV420toBGR15Fix_CopyRU(info->display_fbuf->buf[0],info->display_fbuf->buf[1],info->display_fbuf->buf[2],DrawBuf); break;
                    case 3: YUV420toBGR15Fix_CopyLD(info->display_fbuf->buf[0],info->display_fbuf->buf[1],info->display_fbuf->buf[2],DrawBuf); break;
                  }
                  }else{
                  reqtrans=false;
                  _consolePrintf("!");
                }
              }
            }
          }
          
          //while(reqflip!=0) swiWaitForVBlank();
          
          REG_IME=0;
          if(reqflip!=0){
            if(reqflip==3) pScreenMain->Flip(false);
            pScreenMain->SetBlendLevel(16);
            reqflip=0;
          }
          REG_IME=1;
          
          if(reqtrans==true){
            DMA1_SRC = (u32)DrawBuf;
            DMA1_DEST = (u32)pScreenMain->GetVRAMBuf(ScrMainID_Back);
            DMA1_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | ((ScreenWidth*Height*2)>>1);
          }
          
/*
          // ARM7 processing converter
          while(IPC3->R2YREQ!=R2YREQ_NULL);
          IPC3->R2Y_pYBuf=info->display_fbuf->buf[0];
          IPC3->R2Y_pUBuf=info->display_fbuf->buf[1];
          IPC3->R2Y_pVBuf=info->display_fbuf->buf[2];
          IPC3->R2Y_pFrameBuf=DrawBuf;
          IPC3->R2Y_pVRAM=pScreenMain->GetVRAMBuf(ScrMainID_Back);
          IPC3->R2YREQ=R2YREQ_Convert;
          while(IPC3->R2YREQ==R2YREQ_Convert);
*/
          
        }
        
#ifdef DPGTimeProfile
        if(enprf==true) PrfEnd(0);
#endif
        return(true);
      } break;
      default: break;
    }
  }
  
  return(false);
}

int Clibmpg::GetFrameNum(void)
{
  return(framenum);
}


// -----------------------------------------------------------------

// all format
//#define trofs (197)
//#define tgofs (132)
//#define tbofs (232)

// SMPTE 170M format optimized.
#define trofs (175/4)
#define tgofs (132/4)
#define tbofs (221/4)

// no limitter
//#define trofs (0)
//#define tgofs (0)
//#define tbofs (0)

typedef u8 TColorR;
typedef u32 TColorG;
typedef u32 TColorB;
typedef u32 TColorRGB;

static DATA_IN_DTCM TColorR itable_rbuf[256+trofs*2];
static DATA_IN_DTCM TColorG itable_gbuf[256+tgofs*2];
static DATA_IN_DTCM TColorB itable_bbuf[256+tbofs*2];
static DATA_IN_DTCM TColorR *itable_rV[256];
static DATA_IN_DTCM TColorG *itable_gU[256];
static DATA_IN_DTCM u8 itable_gV[256];
static DATA_IN_DTCM TColorB *itable_bU[256];

//#define RGB15_UseTable

#ifdef RGB15_UseTable
static u8 RGB15_LimitTable[256];
static TColorRGB RGB15_RGB2YUV[1<<(5+5+5)];
#endif

static inline int div_round (int dividend, int divisor)
{
    if (dividend > 0)
        return (dividend + (divisor>>1)) / divisor;
    else
        return -((-dividend + (divisor>>1)) / divisor);
}

#include "../md5_global.h"
#include "../md5.h"

void Clibmpg::debug_tableout(void)
{
}

void Clibmpg::YUV420toBGR15_Init(int Width,int Height)
{
  TYUV420toBGR15_DATA *id=&YUV420toBGR15_DATA;
  
  u8 table_Y[1024];
  
  u32 bs;
  switch(PixelFormat){
    case PF_RGB15: bs=0; break;
    case PF_RGB18: bs=1; break;
    case PF_RGB21: bs=2; break;
    case PF_RGB24: bs=3; break;
    default: bs=3; break;
  }
  
  for (int i=0;i<1024;i++){
    int j=((76309*(i-384-16))+32768) >> 16;
    j=(j<0) ? 0 : ((j>255) ? 255 : j);
    j>>=bs;
    if(0x1f<j) j=0x1f;
    table_Y[i]=j;
  }

  for (int i=-trofs;i<256+trofs;i++){
    itable_rbuf[i+trofs]=table_Y[i+384];
  }
  
  for (int i=-tgofs;i<256+tgofs;i++){
    itable_gbuf[i+tgofs]=table_Y[i+384] << 5;
  }
  
  for (int i=-tbofs;i<256+tbofs;i++){
    itable_bbuf[i+tbofs]=(table_Y[i+384] << 10) | (1 << 15); // transmask
  }
  
#if 0
  int matrix_coefficients=6;
  int Inverse_Table_6_9[8][4] = {
    {117504, 138453, 13954, 34903}, /* no sequence_display_extension */
    {117504, 138453, 13954, 34903}, /* ITU-R Rec. 709 (1990) */
    {104597, 132201, 25675, 53279}, /* unspecified */
    {104597, 132201, 25675, 53279}, /* reserved */
    {104448, 132798, 24759, 53109}, /* FCC */
    {104597, 132201, 25675, 53279}, /* ITU-R Rec. 624-4 System B, G */
    {104597, 132201, 25675, 53279}, /* SMPTE 170M */
    {117579, 136230, 16907, 35559}  /* SMPTE 240M (1987) */
  };
#endif
  
  // for SMPTE 170M only
  const int crv=104597; // Inverse_Table_6_9[matrix_coefficients][0];
  const int cbu=132201; // Inverse_Table_6_9[matrix_coefficients][1];
  const int cgu=-25675; // -Inverse_Table_6_9[matrix_coefficients][2];
  const int cgv=-53279; // -Inverse_Table_6_9[matrix_coefficients][3];
  
  for (int i=0;i<256;i++) {
    itable_rV[i]=&itable_rbuf[trofs+div_round(crv*(i-128), 76309)];
    itable_gU[i]=&itable_gbuf[tgofs+div_round(cgu*(i-128), 76309)-89];
    itable_gV[i]=                   div_round(cgv*(i-128), 76309)+89;
    itable_bU[i]=&itable_bbuf[tbofs+div_round(cbu*(i-128), 76309)];
  }
  
//  debug_tableout();
  
  id->width=Width;
  id->height=Height;
  id->Xdiv2=Width/2;
  id->Ydiv2=Height/2;
  
#ifdef RGB15_UseTable
  for(u32 i=0;i<0x100;i++){
    s32 d=(s32)i;
    d-=0x80;
    if(d<-0x40) d=-0x40;
    if(0x3f<=d) d=0x3f;
    d+=0x40;
    RGB15_LimitTable[i]=d>>2;
  }
  
  for(u32 u=0;u<0x20;u++){
    for(u32 v=0;v<0x20;v++){
      u32 _u=u<<3,_v=v<<3;
      const TColorR *r=itable_rV[_v];
      const TColorG *g=itable_gU[_u]+itable_gV[_v];
      const TColorB *b=itable_bU[_u];
      for(u32 y=0;y<0x20;y++){
        u32 _y=y<<3;
        RGB15_RGB2YUV[(u<<(5+5))|(v<<5)|y]=(TColorRGB)(r[_y] | g[_y] | b[_y]);
      }
    }
  }
#endif
}

// --------------

#define RGB(i) { \
  const TColorR *r=itable_rV[pv[i]]; \
  const TColorG *g=itable_gU[pu[i]]+itable_gV[pv[i]]; \
  const TColorB *b=itable_bU[pu[i]]; \
  const u32 aofs=i*2; \
  { \
    u32 c; \
    { const u32 Y=py_1[ofs+aofs+0]; c=r[Y] | g[Y] | b[Y]; } \
    { const u32 Y=py_1[ofs+aofs+1]; c|=(r[Y] | g[Y] | b[Y])<<16; } \
    *(u32*)&dst[ofs+aofs]=c; \
  } \
  { \
    u32 c; \
    { const u32 Y=py_2[ofs+aofs+0]; c=r[Y] | g[Y] | b[Y]; } \
    { const u32 Y=py_2[ofs+aofs+1]; c|=(r[Y] | g[Y] | b[Y])<<16; } \
    *(u32*)&dst2[ofs+aofs]=c; \
  } \
}

void Clibmpg::YUV420toBGR15_CopyFull(u8 *_YBuf,u8 *_UBuf,u8 *_VBuf,u16 *_FrameBuf)
{
  const TYUV420toBGR15_DATA *id=&YUV420toBGR15_DATA;
  
  u16 *dst=&_FrameBuf[0];
  const u8 *py_1=_YBuf;
  const u8 *pu=_UBuf;
  const u8 *pv=_VBuf;
  u32 dy=id->Ydiv2;
  
  do {
    u16 *dst2=&dst[ScreenWidth];
    const u8 *py_2=&py_1[id->width];
    u32 dx=id->Xdiv2;
    u32 ofs=0;
    do {
      RGB(0);
      ofs+=2;
      pu+=1; pv+=1;
    } while (--dx);
    
    py_1+=id->width*2;
    dst+=ScreenWidth*2;
  } while (--dy);
}

#undef RGB

// ------

#ifdef RGB15_UseTable
#define RGB(i) { \
  const u32 aofs=i*2; \
  u32 u=RGB15_LimitTable[pu[i]]; \
  u32 v=RGB15_LimitTable[pv[i]]; \
  TColorRGB *YTable=&RGB15_RGB2YUV[(u<<(5+5))|(v<<5)|0]; \
   \
  *(u32*)&dst[ofs+aofs]=YTable[py[ofs+aofs+0]>>3] | (YTable[py[ofs+aofs+1]>>3]<<16); \
  ofs+=ScreenWidth; \
  *(u32*)&dst[ofs+aofs]=YTable[py[ofs+aofs+0]>>3] | (YTable[py[ofs+aofs+1]>>3]<<16); \
  ofs-=ScreenWidth; \
}
#endif

#define RGB(i) { \
  const TColorR *r=itable_rV[pv[i]]; \
  const TColorG *g=itable_gU[pu[i]]+itable_gV[pv[i]]; \
  const TColorB *b=itable_bU[pu[i]]; \
  const u32 aofs=i*2; \
  if((LU==1)||(RU==1)){ \
    if((LU==1)&&(RU==1)){ \
      u32 c; \
      { const u32 Y=py[ofs+aofs+0]; c=r[Y] | g[Y] | b[Y]; } \
      { const u32 Y=py[ofs+aofs+1]; c|=(r[Y] | g[Y] | b[Y])<<16; } \
      *(u32*)&dst[ofs+aofs]=c; \
      }else{ \
      if(LU==1){ const u32 Y=py[ofs+aofs+0]; dst[ofs+aofs+0]=r[Y] | g[Y] | b[Y]; } \
      if(RU==1){ const u32 Y=py[ofs+aofs+1]; dst[ofs+aofs+1]=r[Y] | g[Y] | b[Y]; } \
    } \
  } \
  if((LD==1)||(RD==1)){ \
    ofs+=ScreenWidth; \
    if((LD==1)&&(RD==1)){ \
      u32 c; \
      { const u32 Y=py[ofs+aofs+0]; c=r[Y] | g[Y] | b[Y]; } \
      { const u32 Y=py[ofs+aofs+1]; c|=(r[Y] | g[Y] | b[Y])<<16; } \
      *(u32*)&dst[ofs+aofs]=c; \
      }else{ \
      if(LD==1){ const u32 Y=py[ofs+aofs+0]; dst[ofs+aofs+0]=r[Y] | g[Y] | b[Y]; } \
      if(RD==1){ const u32 Y=py[ofs+aofs+1]; dst[ofs+aofs+1]=r[Y] | g[Y] | b[Y]; } \
    } \
    ofs-=ScreenWidth; \
  } \
}

#define DECLARE_COPY(func,_LU,_RU,_LD,_RD) \
void func(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf) \
{ \
  const int LU=_LU,RU=_RU,LD=_LD,RD=_RD; \
   \
  const TYUV420toBGR15_DATA *id=&YUV420toBGR15_DATA; \
   \
  u16 *dst=&_FrameBuf[0]; \
  const u8 *py=_YBuf; \
  const u8 *pu=_UBuf; \
  const u8 *pv=_VBuf; \
  u32 dy=id->Ydiv2; \
   \
  do { \
    u32 dx=ScreenWidth/16; \
    u32 ofs=0; \
    do { \
      RGB(0); RGB(1); RGB(2); RGB(3); RGB(4); RGB(5); RGB(6); RGB(7); \
      ofs+=16; \
      pu+=8; pv+=8; \
    } while (--dx); \
     \
    py+=ScreenWidth*2; \
    dst+=ScreenWidth*2; \
  } while (--dy); \
}

DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyFull,1,1,1,1)
DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyLU,1,0,0,0)
DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyRU,0,1,0,0)
DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyLD,0,0,1,0)
DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyRD,0,0,0,1)

#undef DECLARE_COPY
#undef RGB

#define RGB(i) { \
  const TColorR *r=itable_rV[pv[i]]; \
  const TColorG *g=itable_gU[pu[i]]+itable_gV[pv[i]]; \
  const TColorB *b=itable_bU[pu[i]]; \
  const u32 aofs=i*2; \
  { \
    u32 c; \
    { const u32 Y=py[ofs+aofs+0]; c=r[Y] | g[Y] | b[Y]; } \
    *(u32*)&dst[ofs+aofs]=c | (c<<16); \
  } \
  { \
    ofs+=ScreenWidth; \
    u32 c; \
    { const u32 Y=py[ofs+aofs+0]; c=r[Y] | g[Y] | b[Y]; } \
    *(u32*)&dst[ofs+aofs]=c | (c<<16); \
    ofs-=ScreenWidth; \
  } \
}

#define DECLARE_COPY(func) \
void func(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf) \
{ \
  const TYUV420toBGR15_DATA *id=&YUV420toBGR15_DATA; \
   \
  u16 *dst=&_FrameBuf[0]; \
  const u8 *py=_YBuf; \
  const u8 *pu=_UBuf; \
  const u8 *pv=_VBuf; \
  u32 dy=id->Ydiv2; \
   \
  do { \
    u32 dx=ScreenWidth/16; \
    u32 ofs=0; \
    do { \
      RGB(0); RGB(1); RGB(2); RGB(3); RGB(4); RGB(5); RGB(6); RGB(7); \
      ofs+=16; \
      pu+=8; pv+=8; \
    } while (--dx); \
     \
    py+=ScreenWidth*2; \
    dst+=ScreenWidth*2; \
  } while (--dy); \
}

DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyHalf)

#undef DECLARE_COPY
#undef RGB

#if 0

#define RGBU(i) { \
  const TColorR *r=itable_rV[pv[i]]; \
  const TColorG *g=itable_gU[pu[i]]+itable_gV[pv[i]]; \
  const TColorB *b=itable_bU[pu[i]]; \
  const u32 aofs=i*2; \
  u32 c; \
  { const u32 Y=py[ofs+aofs+0]; c=r[Y] | g[Y] | b[Y]; } \
  { const u32 Y=py[ofs+aofs+1]; c|=(r[Y] | g[Y] | b[Y])<<16; } \
  *(u32*)&dst[ofs+aofs]=c; \
}

#define RGBD(i) { \
  const TColorR *r=itable_rV[pv[i]]; \
  const TColorG *g=itable_gU[pu[i]]+itable_gV[pv[i]]; \
  const TColorB *b=itable_bU[pu[i]]; \
  const u32 aofs=i*2; \
  ofs+=ScreenWidth; \
  u32 c; \
  { const u32 Y=py[ofs+aofs+0]; c=r[Y] | g[Y] | b[Y]; } \
  { const u32 Y=py[ofs+aofs+1]; c|=(r[Y] | g[Y] | b[Y])<<16; } \
  *(u32*)&dst[ofs+aofs]=c; \
  ofs-=ScreenWidth; \
}

#define DECLARE_COPY(func,FastLine) \
void func(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf) \
{ \
  const TYUV420toBGR15_DATA *id=&YUV420toBGR15_DATA; \
   \
  u16 *dst=&_FrameBuf[0]; \
  const u8 *py=_YBuf; \
  const u8 *pu=_UBuf; \
  const u8 *pv=_VBuf; \
  u32 dy=id->Ydiv2; \
   \
  do { \
    u32 dx=ScreenWidth/16; \
    u32 ofs=0; \
    do { \
      if(FastLine==0){ \
        RGBU(0); RGBD(1); RGBU(2); RGBD(3); RGBU(4); RGBD(5); RGBU(6); RGBD(7); \
        }else{ \
        RGBD(0); RGBU(1); RGBD(2); RGBU(3); RGBD(4); RGBU(5); RGBD(6); RGBU(7); \
      } \
      ofs+=16; \
      pu+=8; pv+=8; \
    } while (--dx); \
     \
    py+=ScreenWidth*2; \
    dst+=ScreenWidth*2; \
  } while (--dy); \
}

DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyCross0,0)
DECLARE_COPY (Clibmpg::YUV420toBGR15Fix_CopyCross1,1)

#undef DECLARE_COPY
#undef RGBU
#undef RGBD

#endif

#endif

