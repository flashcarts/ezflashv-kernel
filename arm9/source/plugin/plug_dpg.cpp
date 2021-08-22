
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NDS.h>

#include "../filesys.h"
#include "../cstream_fs.h"

#include "plug_dpg.h"
#include "plug_mp2.h"

#include "clibdpg.h"

bool DPG_RequestSyncStart;

#ifndef USEDPG

bool StartDPG(int _FileHandleVideo,int _FileHandleAudio)
{
  return(false);
}

u32 UpdateDPG_Audio(s16 *lbuf,s16 *rbuf)
{
  return(0);
}

bool UpdateDPG_Video(u64 CurrentSamplesCount)
{
  return(false);
}

void FreeDPG(void)
{
}

u32 DPG_GetCurrentFrameCount(void)
{
  return(0);
}

u32 DPG_GetTotalFrameCount(void)
{
  return(0);
}

u32 DPG_GetFPS(void)
{
  return(0);
}

u32 DPG_GetSampleRate(void)
{
  return(0);
}

u32 DPG_GetChannelCount(void)
{
  return(0);
}

u32 DPG_GetSamplePerFrame(void)
{
  return(0);
}

void DPG_SetFrameCount(u32 Frame)
{
}

u32 DPG_GetWidth(void)
{
  return(0);
}

u32 DPG_GetHeight(void)
{
  return(0);
}

void DPG_SetVideoDelayms(int ms)
{
}

void DPG_fread(void)
{
}

void DPG_fread_flash(void)
{
}

int DPG_GetInfoIndexCount(void)
{
  return(0);
}

bool DPG_GetInfoStrL(int idx,char *str,int len)
{
  return(false);
}

bool DPG_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool DPG_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

EDPGAudioFormat DPG_GetDPGAudioFormat(void)
{
  return(DPGAF_GSM);
}

#else

static bool Initialized=false;

static Clibdpg *pClibdpg=NULL;

static CStreamFS *pCStreamFS_DPGM=NULL;
static CStreamFS *pCStreamFS_DPGA=NULL;

bool StartDPG(int _FileHandleVideo,int _FileHandleAudio)
{
  if(Initialized==true) FreeDPG();
  Initialized=true;
  
  pCStreamFS_DPGM=new CStreamFS(_FileHandleVideo);
  pCStreamFS_DPGA=new CStreamFS(_FileHandleAudio);
  
  pClibdpg=new Clibdpg(pCStreamFS_DPGM,pCStreamFS_DPGA);
  
  if(pClibdpg->Initialized==false) return(false);
  
  pClibdpg->SetFrame(0);
  
  return(true);
}

void FreeDPG(void)
{
  if(Initialized==false) return;
  Initialized=false;
  
  DPG_RequestSyncStart=false;
  
  if(pCStreamFS_DPGM!=NULL){
    delete pCStreamFS_DPGM; pCStreamFS_DPGM=NULL;
  }
  if(pCStreamFS_DPGA!=NULL){
    delete pCStreamFS_DPGA; pCStreamFS_DPGA=NULL;
  }
  if(pClibdpg!=NULL){
    delete pClibdpg; pClibdpg=NULL;
  }
}

u32 DPG_GetCurrentFrameCount(void)
{
  return(pClibdpg->GetFrameNum());
}

u32 DPG_GetTotalFrameCount(void)
{
  return(pClibdpg->DPGINFO.TotalFrame);
}

u32 DPG_GetFPS(void)
{
  return(pClibdpg->DPGINFO.FPS);
}

u32 DPG_GetSampleRate(void)
{
  return(pClibdpg->DPGINFO.SndFreq);
}

u32 DPG_GetChannelCount(void)
{
  return(pClibdpg->DPGINFO.SndCh);
}

u32 DPG_GetSamplePerFrame(void)
{
  switch(pClibdpg->GetDPGAudioFormat()){
    case DPGAF_GSM: return(GSM_GetSamplePerFrame()); break;
    case DPGAF_MP2: return(MP2_GetSamplePerFrame()); break;
  }
  
  return(0);
}

bool UpdateDPG_Video(u64 CurrentSamplesCount)
{
  if(Initialized==false) return(false);
  
  switch(pClibdpg->GetDPGAudioFormat()){
    case DPGAF_GSM: GSM_LoadReadBuffer(); break;
    case DPGAF_MP2: MP2_LoadReadBuffer(); break;
  }
  
  return(pClibdpg->MovieProcDecode(CurrentSamplesCount));
}

u32 UpdateDPG_Audio(s16 *lbuf,s16 *rbuf)
{
  if(Initialized==false) return(0);
  
  return(pClibdpg->AudioDecode(lbuf,rbuf));
}

void DPG_SetFrameCount(u32 Frame)
{
  pClibdpg->SetFrame(Frame);
}

u32 DPG_GetWidth(void)
{
  return(pClibdpg->GetWidth());
}

u32 DPG_GetHeight(void)
{
  return(pClibdpg->GetHeight());
}

void DPG_fread(void)
{
  switch(pClibdpg->GetDPGAudioFormat()){
    case DPGAF_GSM: GSM_fread(); break;
    case DPGAF_MP2: MP2_fread(); break;
  }
  
  return;
}

void DPG_fread_flash(void)
{
  switch(pClibdpg->GetDPGAudioFormat()){
    case DPGAF_GSM: GSM_fread_flash(); break;
    case DPGAF_MP2: MP2_fread_flash(); break;
  }
  
  return;
}

int DPG_GetInfoIndexCount(void)
{
  return(8);
}

bool DPG_GetInfoStrL(int idx,char *str,int len)
{
  TDPGINFO *pDPGINFO=&pClibdpg->DPGINFO;
  
  switch(idx){
    case 0: snprintf(str,len,"Video TotalFrame=%d fps=%f",pDPGINFO->TotalFrame,(float)pDPGINFO->FPS/0x100); return(true); break;
    case 1: snprintf(str,len,"ScreenPixels=%dx%d",pClibdpg->GetWidth(),pClibdpg->GetHeight()); return(true); break;
    case 2: snprintf(str,len,"Audio %dHz %dChannels",pDPGINFO->SndFreq,pDPGINFO->SndCh); return(true); break;
    case 3: {
      int sec=pDPGINFO->TotalFrame*0x100/pDPGINFO->FPS;
      snprintf(str,len,"Length=%d:%2d",sec/60,sec%60);
      return(true);
    } break;
    case 4: snprintf(str,len,"Movie %d->%dbytes.",pDPGINFO->MoviePos,pDPGINFO->MovieSize); return(true); break;
    case 5: snprintf(str,len,"Audio %d->%dbytes.",pDPGINFO->AudioPos,pDPGINFO->AudioSize); return(true); break;
    case 6: snprintf(str,len,"GOPList %d->%dbytes.",pDPGINFO->GOPListPos,pDPGINFO->GOPListSize); return(true); break;
    case 7: {
      const char pfstr[][6]={"RGB15","RGB18","RGB21","RGB24"};
      snprintf(str,len,"PixelFormat %d %s",pDPGINFO->PixelFormat,pfstr[(int)pDPGINFO->PixelFormat]);
      return(true);
    } break;
  }
  return(false);
}

bool DPG_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool DPG_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

EDPGAudioFormat DPG_GetDPGAudioFormat(void)
{
  return(pClibdpg->GetDPGAudioFormat());
}

#endif

