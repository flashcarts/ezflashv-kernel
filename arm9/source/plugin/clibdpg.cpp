
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>
#include <NDS/ARM9/CP15.h>

#include "../_const.h"
#include "../_console.h"
#include "../_consoleWriteLog.h"
#include "../inifile.h"
#include "../memtool.h"

#include "clibdpg.h"

extern void ShowLogHalt(void);

#define DPG0ID (0x30475044)
#define DPG1ID (0x31475044)
#define DPG2ID (0x32475044)

#ifndef USEDPG

Clibdpg::Clibdpg(CStream *_pCStreamMovie,CStream *_pCStreamAudio)
{
}

Clibdpg::~Clibdpg(void)
{
}

bool Clibdpg::LoadDPGINFO(CStream *pCStream)
{
  return(false);
}

bool Clibdpg::MovieProcDecode(u64 CurrentSamplesCount)
{
  return(false);
}

int Clibdpg::AudioDecode(s16 *lbuf,s16 *rbuf)
{
  return(0);
}

void Clibdpg::SetFrame(int Frame)
{
}

int Clibdpg::GetFrameNum(void)
{
  return(0);
}

int Clibdpg::GetWidth(void)
{
  return(0);
}

int Clibdpg::GetHeight(void)
{
  return(0);
}

EDPGAudioFormat Clibdpg::GetDPGAudioFormat(void)
{
  return(DPGAF_GSM);
}

#else

Clibdpg::Clibdpg(CStream *_pCStreamMovie,CStream *_pCStreamAudio)
{
  Initialized=false;
  
  DelayFrames=0;
  
  GOPListCount=0;
  pGOPList=NULL;
  
  if(LoadDPGINFO(_pCStreamMovie)==false) return;
  
  if(GlobalINI.DPGPlugin.BlockGSMAudio==true){
    if(GetDPGAudioFormat()==DPGAF_GSM) return;
  }
  
  pCStreamMovie=_pCStreamMovie;
  pCStreamAudio=_pCStreamAudio;
  
  pCStreamMovie->SetOffset(DPGINFO.MoviePos);
  pCStreamMovie->OverrideSize(DPGINFO.MoviePos+DPGINFO.MovieSize);
  pClibmpg=new Clibmpg(pCStreamMovie,DPGINFO.TotalFrame,DPGINFO.FPS,DPGINFO.SndFreq,DPGINFO.PixelFormat);
  
  pClibmpg->SetVideoDelayms(1000*0x100/DPGINFO.FPS*DelayFrames);
  
  pCStreamAudio->SetOffset(DPGINFO.AudioPos);
  pCStreamAudio->OverrideSize(DPGINFO.AudioPos+DPGINFO.AudioSize);
  
  switch(GetDPGAudioFormat()){
    case DPGAF_GSM: StartGSM(pCStreamAudio); break;
    case DPGAF_MP2: StartMP2(pCStreamAudio); break;
  }
  
  Initialized=true;
}

Clibdpg::~Clibdpg(void)
{
  if(pClibmpg!=NULL){
    delete pClibmpg; pClibmpg=NULL;
  }
  
  switch(GetDPGAudioFormat()){
    case DPGAF_GSM: FreeGSM(); break;
    case DPGAF_MP2: FreeMP2(); break;
  }
  
  GOPListCount=0;
  if(pGOPList!=NULL){
    safefree(pGOPList); pGOPList=NULL;
  }
  
  pCStreamMovie=NULL;
  pCStreamAudio=NULL;
}

bool Clibdpg::LoadDPGINFO(CStream *pCStream)
{
  pCStream->SetOffset(0);
  
  u32 id=pCStream->Readu32();
  if((id!=DPG0ID)&&(id!=DPG1ID)&&(id!=DPG2ID)){
    pCStream->SetOffset(0);
    _consolePrintf("Unknown DPG Format.ID=%08x\n",pCStream->Readu32());
    return(false);
  }
  
  DPGINFO.TotalFrame=pCStream->Readu32();
  DPGINFO.FPS=pCStream->Readu32();
  DPGINFO.SndFreq=pCStream->Readu32();
  DPGINFO.SndCh=pCStream->Readu32();
  DPGINFO.AudioPos=pCStream->Readu32();
  DPGINFO.AudioSize=pCStream->Readu32();
  DPGINFO.MoviePos=pCStream->Readu32();
  DPGINFO.MovieSize=pCStream->Readu32();
  
  DPGINFO.GOPListPos=0;
  DPGINFO.GOPListSize=0;
  GOPListCount=0;
  pGOPList=NULL;
  
  if(id==DPG2ID){
    DPGINFO.GOPListPos=pCStream->Readu32();
    DPGINFO.GOPListSize=pCStream->Readu32();
  }
  
  if(id==DPG0ID) DPGINFO.PixelFormat=PF_RGB24;
  if((id==DPG1ID)||(id==DPG2ID)) DPGINFO.PixelFormat=(EMPGPixelFormat)(pCStream->Readu32());
  
  _consolePrintf("Movie %dframes %ffps\n",DPGINFO.TotalFrame,(float)DPGINFO.FPS/0x100);
  _consolePrintf("Sound %dHz %dChs\n",DPGINFO.SndFreq,DPGINFO.SndCh);
  _consolePrintf("Audio %d->%dbytes.\n",DPGINFO.AudioPos,DPGINFO.AudioSize);
  _consolePrintf("Movie %d->%dbytes.\n",DPGINFO.MoviePos,DPGINFO.MovieSize);
  _consolePrintf("GOPList %d->%dbytes.\n",DPGINFO.GOPListPos,DPGINFO.GOPListSize);
  
  const char pfstr[][6]={"RGB15","RGB18","RGB21","RGB24"};
  _consolePrintf("PixelFormat %d %s\n",DPGINFO.PixelFormat,pfstr[(int)DPGINFO.PixelFormat]);
  
  if((DPGINFO.GOPListPos!=0)&&(DPGINFO.GOPListSize!=0)){
    GOPListCount=DPGINFO.GOPListSize/8;
    pGOPList=(TGOPList*)safemalloc(GOPListCount*8);
    _consolePrintf("GOPListCount=%d,pGOPList=0x%x\n",GOPListCount,pGOPList);
    if(pGOPList==NULL){
      GOPListCount=0;
      }else{
      pCStream->SetOffset(DPGINFO.GOPListPos);
      for(u32 idx=0;idx<GOPListCount;idx++){
        pGOPList[idx].FrameIndex=pCStream->Readu32();
        pGOPList[idx].Offset=pCStream->Readu32();
      }
    }
  }
  
  if(id==DPG2ID){
    DelayFrames=3;
    }else{
    DelayFrames=0;
  }
  
  return(true);
}

bool Clibdpg::MovieProcDecode(u64 CurrentSamplesCount)
{
  return(pClibmpg->ProcDecode(CurrentSamplesCount));
}

int Clibdpg::AudioDecode(s16 *lbuf,s16 *rbuf)
{
  switch(GetDPGAudioFormat()){
    case DPGAF_GSM: return(UpdateGSM(lbuf,rbuf)); break;
    case DPGAF_MP2: return(UpdateMP2(lbuf,rbuf)); break;
  }
  
  return(0);
}

void Clibdpg::SetFrame(int Frame)
{
  u64 smp=(u64)Frame*DPGINFO.SndFreq*0x100/DPGINFO.FPS;
  
  if((GOPListCount==0)||(pGOPList==NULL)){
    pClibmpg->ProcMoveFrame(Frame,smp);
    }else{
    u32 GOPFrame=0,GOPOffset=0;
    for(u32 idx=0;idx<GOPListCount;idx++){
      if(pGOPList[idx].FrameIndex<=Frame){
        GOPFrame=pGOPList[idx].FrameIndex;
        GOPOffset=pGOPList[idx].Offset;
      }
    }
    _consolePrintf("GOPFrame=%d, GOPOffset=0x%x\n",GOPFrame,GOPOffset);
    pClibmpg->ProcMoveFrameGOP(Frame,smp,GOPFrame,GOPOffset);
  }
  
  Frame-=DelayFrames;
  
  double per=(double)Frame/((double)DPGINFO.TotalFrame);
  s64 ssmp=(s64)Frame*DPGINFO.SndFreq*0x100/DPGINFO.FPS;
  
  switch(GetDPGAudioFormat()){
    case DPGAF_GSM: {
      GSM_SetPosition(per,ssmp);
    } break;
    case DPGAF_MP2: {
      MP2_SetPosition(per,ssmp);
    } break;
  }
  
}

int Clibdpg::GetFrameNum(void)
{
  return(pClibmpg->GetFrameNum());
}

int Clibdpg::GetWidth(void)
{
  return(pClibmpg->GetWidth());
}

int Clibdpg::GetHeight(void)
{
  return(pClibmpg->GetHeight());
}

EDPGAudioFormat Clibdpg::GetDPGAudioFormat(void)
{
  if(DPGINFO.SndCh!=0){
    return(DPGAF_GSM);
    }else{
    return(DPGAF_MP2);
  }
  
}

#endif

