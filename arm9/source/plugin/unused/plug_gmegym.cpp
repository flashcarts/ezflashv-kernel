
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "../_console.h"
#include <NDS/ARM9/CP15.h>

#include "../memtool.h"
#include "../mwin.h"
#include "../filesys.h"

#include "plug_gmegym.h"

#ifndef USEGME_GYM

bool StartGMEGYM(int _FileHandle,int _TrackNum)
{
  return(false);
}

void FreeGMEGYM(void)
{
}

u32 UpdateGMEGYM(s16 *pbufl,s16 *pbufr)
{
  return(0);
}

s32 GMEGYM_GetPosMax(void)
{
  return(0);
}

s32 GMEGYM_GetPosOffset(void)
{
  return(0);
}

void GMEGYM_SetPosOffset(s32 ofs)
{
}

u32 GMEGYM_GetSampleRate(void)
{
  return(0);
}

u32 GMEGYM_GetChannelCount(void)
{
  return(0);
}

u32 GMEGYM_GetSamplePerFrame(void)
{
  return(0);
}

int GMEGYM_GetInfoIndexCount(void)
{
  return(0);
}

bool GMEGYM_GetInfoStrL(int idx,char *str,int len)
{
  return(false);
}

bool GMEGYM_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool GMEGYM_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#else

// -----------------

#include "libgme/abstract_file.h"
#include "libgme/Gym_Emu.h"
#include "libgme/Effects_Buffer.h"

typedef struct {
  char tag [4];
  char song [32];
  char game [32];
  char copyright [32];
  char emulator [32];
  char dumper [32];
  char comment [256];
  byte loop_start [4]; // in 1/60 seconds, 0 if not looped
  byte packed [4];
} TGYMInfo;

static int FileHandle;

static u8 *DeflateBuf=NULL;
static u32 DeflateSize;

static Music_Emu *pemu=NULL;
static Effects_Buffer *peffbuf=NULL;
static TGYMInfo GYMInfo;

static s16 *pGenBuf=NULL;

static int SampleRate=8000;
static int SamplePerFrame=2560/4;
static bool EffectFlag=false;

static int PosMax;
static int PosOffset;

// -----------------

static bool LoadGYMInfo(u8 *pb,u32 size)
{
  if(size<sizeof(TGYMInfo)){
    _consolePrintf("DataSize:%d<%d\n",size,sizeof(TGYMInfo));
    return(false);
  }
  
  MemCopy8CPU(pb,&GYMInfo,sizeof(TGYMInfo));
  
  if((GYMInfo.tag[0]!='G')||(GYMInfo.tag[1]!='Y')||(GYMInfo.tag[2]!='M')||(GYMInfo.tag[3]!='X')){
    _consolePrintf("Illigal GYM format.\n");
    _consolePrintf("tag[]=");
    for(u32 i=0;i<8;i++){
      _consolePrintf("%02x,",pb[i]);
    }
    _consolePrintf("\n");
    return(false);
  }
  
  if((GYMInfo.packed[0]!=0)||(GYMInfo.packed[1]!=0)||(GYMInfo.packed[2]!=0)||(GYMInfo.packed[3]!=0)){
    _consolePrintf("Packed GYM file not supported.\n");
    return(false);
  }
  
  return(true);
}

static void ShowGYMInfo(void)
{
#define astr32(ttl,data) _consolePrintf("%s:%.32s\n",ttl,data);
#define astr256(ttl,data) _consolePrintf("%s:%.256s\n",ttl,data);
  
  astr32("song",GYMInfo.song);
  astr32("game",GYMInfo.game);
  astr32("copyright",GYMInfo.copyright);
  astr32("emulator",GYMInfo.emulator);
  astr32("dumper",GYMInfo.dumper);
  astr256("comment",GYMInfo.comment);

#undef astr32
#undef astr256
}

static void SetEffect(bool flag)
{
  // Configure effects buffer
  Effects_Buffer::config_t cfg;
  cfg.pan_1 = -0.12;          // put first two channels slightly off-center
  cfg.pan_2 = 0.12;
  cfg.reverb_delay = 88;      // delays are in milliseconds
  cfg.reverb_level = 0.20;    // significant reverb
  cfg.echo_delay = 61;        // echo applies to noise and percussion channels
  cfg.echo_level = 0.15;
  cfg.delay_variance = 18;    // left/right delays must differ for stereo effect
  cfg.effects_enabled = flag;
  peffbuf->config(cfg);
}

// -----------------

bool StartGMEGYM(int _FileHandle,int TrackNum)
{
  FileHandle=_FileHandle;
  
  FileSys_fseek(FileHandle,0,SEEK_END);
  DeflateSize=FileSys_ftell(FileHandle);
  FileSys_fseek(FileHandle,0,SEEK_SET);
  
  DeflateBuf=(u8*)safemalloc(DeflateSize);
  if(DeflateBuf==NULL) return(false);
  FileSys_fread(DeflateBuf,1,DeflateSize,FileHandle);
  
  if(LoadGYMInfo(DeflateBuf,DeflateSize)==false){
    _consolePrintf("GYM Header error.\n");
    FreeGMEGYM();
    return(false);
  }
  ShowGYMInfo();
  
  pGenBuf=(s16*)safemalloc(SamplePerFrame*2*2);
  if(pGenBuf==NULL){
    _consolePrintf("out of memory.\n");
    FreeGMEGYM();
    return(false);
  }
  
  pemu=new Gym_Emu;
  
  peffbuf=new Effects_Buffer;
  
  pemu->set_buffer(peffbuf);
  
  const char* error;
  
  error=pemu->set_sample_rate(SampleRate);
  if(error!=NULL){
    _consolePrintf("gme error:%s\n",error);
    FreeGMEGYM();
    return(false);
  }
  
  {
    Mem_File_Reader *pin;
    
    pin=new Mem_File_Reader(DeflateBuf,DeflateSize);
    error = pemu->load(*pin);
    delete pin; pin=NULL;
    
    if(error!=NULL){
      _consolePrintf("gme error:%s\n",error);
      FreeGMEGYM();
      return(false);
    }
  }
  
  SetEffect(EffectFlag);
  
  pemu->start_track(TrackNum);
  
  PosMax=90*SampleRate;
  PosOffset=0;
  
  return(true);
}

void FreeGMEGYM(void)
{
  if(peffbuf!=NULL){
    delete peffbuf; peffbuf=NULL;
  }
  if(pemu!=NULL){
    delete pemu; pemu=NULL;
  }
  
  if(pGenBuf!=NULL){
    safefree(pGenBuf); pGenBuf=NULL;
  }
  
  if(DeflateBuf!=NULL){
    safefree(DeflateBuf); DeflateBuf=NULL;
  }
}

u32 UpdateGMEGYM(s16 *lbuf,s16 *rbuf)
{
  if(pemu==NULL) return(0);
  
  PosOffset+=SamplePerFrame;
  
  s16 *srcbuf=pGenBuf;
  
  if((lbuf==NULL)||(rbuf==NULL)){
    SetEffect(false);
    pemu->play(SamplePerFrame*2,srcbuf);
    SetEffect(true);
    return(SamplePerFrame);
  }
  
  pemu->play(SamplePerFrame*2,srcbuf);
  
  for(u32 idx=SamplePerFrame;idx!=0;idx--){
    *lbuf++=*srcbuf++;
    *rbuf++=*srcbuf++;
  }
  
  return(SamplePerFrame);
}

s32 GMEGYM_GetPosMax(void)
{
  return(PosMax);
}

s32 GMEGYM_GetPosOffset(void)
{
  return(PosOffset);
}

void GMEGYM_SetPosOffset(s32 ofs)
{
}

u32 GMEGYM_GetSampleRate(void)
{
  return(SampleRate);
}

u32 GMEGYM_GetChannelCount(void)
{
  return(2);
}

u32 GMEGYM_GetSamplePerFrame(void)
{
  return(SamplePerFrame);
}

int GMEGYM_GetInfoIndexCount(void)
{
  return(6);
}

bool GMEGYM_GetInfoStrL(int idx,char *str,int len)
{
  switch(idx){
    case 0: snprintf(str,len,"song:%.32s",GYMInfo.song); return(true); break;
    case 1: snprintf(str,len,"game:%.32s",GYMInfo.game); return(true); break;
    case 2: snprintf(str,len,"copyright:%.32s",GYMInfo.copyright); return(true); break;
    case 3: snprintf(str,len,"emulator:%.32s",GYMInfo.emulator); return(true); break;
    case 4: snprintf(str,len,"dumper:%.32s",GYMInfo.dumper); return(true); break;
    case 5: snprintf(str,len,"comment:%.256s",GYMInfo.comment); return(true); break;
  }
  return(false);
}

bool GMEGYM_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool GMEGYM_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#endif
