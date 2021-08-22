
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "../_console.h"
#include <NDS/ARM9/CP15.h>

#include "../memtool.h"
#include "../mwin.h"
#include "../filesys.h"

#include "plug_gmevgm.h"

#ifndef USEGME_VGM

bool StartGMEVGM(int _FileHandle,int _TrackNum)
{
  return(false);
}

void FreeGMEVGM(void)
{
}

u32 UpdateGMEVGM(s16 *pbufl,s16 *pbufr)
{
  return(0);
}

s32 GMEVGM_GetPosMax(void)
{
  return(0);
}

s32 GMEVGM_GetPosOffset(void)
{
  return(0);
}

void GMEVGM_SetPosOffset(s32 ofs)
{
}

u32 GMEVGM_GetSampleRate(void)
{
  return(0);
}

u32 GMEVGM_GetChannelCount(void)
{
  return(0);
}

u32 GMEVGM_GetSamplePerFrame(void)
{
  return(0);
}

int GMEVGM_GetInfoIndexCount(void)
{
  return(0);
}

bool GMEVGM_GetInfoStrL(int idx,char *str,int len)
{
  return(false);
}

bool GMEVGM_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool GMEVGM_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#else

// -----------------

#include "libgme/abstract_file.h"
#include "libgme/Vgm_Emu.h"
#include "libgme/Effects_Buffer.h"

#pragma pack(1)
typedef struct {
  char tag [4];
  byte data_size [4];
  byte version [4];
  byte psg_rate [4];
  byte ym2413_rate [4];
  byte gd3_offset [4];
  byte track_duration [4];
  byte loop_offset [4];
  byte loop_duration [4];
  byte frame_rate [4];
  byte noise_feedback [2];
  byte noise_width;
  byte unused1;
  byte ym2612_rate [4];
  byte ym2151_rate [4];
  byte data_offset [4];
  byte unused2 [8];
} TVGMInfo;
#pragma pack()

static int FileHandle;

static u8 *DeflateBuf=NULL;
static u32 DeflateSize;

static Music_Emu *pemu=NULL;
static Effects_Buffer *peffbuf=NULL;
static TVGMInfo VGMInfo;

static s16 *pGenBuf=NULL;

static int SampleRate=22050;
static int SamplePerFrame=2560/4;
static bool EffectFlag=true;

static int PosMax;
static int PosOffset;

static int CurrentTrackNum;

// -----------------

static bool LoadVGMInfo(u8 *pb,u32 size)
{
  if(size<sizeof(TVGMInfo)){
    _consolePrintf("DataSize:%d<%d\n",size,sizeof(TVGMInfo));
    return(false);
  }
  
  MemCopy8CPU(pb,&VGMInfo,sizeof(TVGMInfo));
  
  if((VGMInfo.tag[0]!='V')||(VGMInfo.tag[1]!='g')||(VGMInfo.tag[2]!='m')||(VGMInfo.tag[3]!=' ')){
    _consolePrintf("Illigal VGM format.\n");
    _consolePrintf("tag[]=");
    for(u32 i=0;i<8;i++){
      _consolePrintf("%02x,",pb[i]);
    }
    _consolePrintf("\n");
    return(false);
  }
  
  return(true);
}

static void ShowVGMInfo(void)
{
#define astr(ttl,data) _consolePrintf("%s:%.32s\n",ttl,data);
#define au8(ttl,data) _consolePrintf("%s:$%02x\n",ttl,data);
#define au16(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
#define au32(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
  
  _consolePrintf("ID:%.3s\n",VGMInfo.tag);

#undef astr
#undef au8
#undef au16
#undef au32
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

bool StartGMEVGM(int _FileHandle,int TrackNum)
{
  CurrentTrackNum=TrackNum;
  
  FileHandle=_FileHandle;
  
  FileSys_fseek(FileHandle,0,SEEK_END);
  DeflateSize=FileSys_ftell(FileHandle);
  FileSys_fseek(FileHandle,0,SEEK_SET);
  
  DeflateBuf=(u8*)safemalloc(DeflateSize);
  if(DeflateBuf==NULL) return(false);
  FileSys_fread(DeflateBuf,1,DeflateSize,FileHandle);
  
  if(LoadVGMInfo(DeflateBuf,DeflateSize)==false){
    _consolePrintf("VGM Header error.\n");
    FreeGMEVGM();
    return(false);
  }
  ShowVGMInfo();
  
  pGenBuf=(s16*)safemalloc(SamplePerFrame*2*2);
  if(pGenBuf==NULL){
    _consolePrintf("out of memory.\n");
    FreeGMEVGM();
    return(false);
  }
  
  pemu=new Vgm_Emu;
  
  peffbuf=new Effects_Buffer;
  
  pemu->set_buffer(peffbuf);
  
  const char* error;
  
  error=pemu->set_sample_rate(SampleRate);
  if(error!=NULL){
    _consolePrintf("gme error:%s\n",error);
    FreeGMEVGM();
    return(false);
  }
  
  {
    Mem_File_Reader *pin;
    
    pin=new Mem_File_Reader(DeflateBuf,DeflateSize);
    error = pemu->load(*pin);
    delete pin; pin=NULL;
    
    if(error!=NULL){
      _consolePrintf("gme error:%s\n",error);
      FreeGMEVGM();
      return(false);
    }
  }
  
  SetEffect(EffectFlag);
  
  pemu->start_track(CurrentTrackNum);
  
  PosMax=90*SampleRate;
  PosOffset=0;
  
  return(true);
}

void FreeGMEVGM(void)
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

u32 UpdateGMEVGM(s16 *lbuf,s16 *rbuf)
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

s32 GMEVGM_GetPosMax(void)
{
  return(PosMax);
}

s32 GMEVGM_GetPosOffset(void)
{
  return(PosOffset);
}

void GMEVGM_SetPosOffset(s32 ofs)
{
}

u32 GMEVGM_GetSampleRate(void)
{
  return(SampleRate);
}

u32 GMEVGM_GetChannelCount(void)
{
  return(2);
}

u32 GMEVGM_GetSamplePerFrame(void)
{
  return(SamplePerFrame);
}

int GMEVGM_GetInfoIndexCount(void)
{
  return(5);
}

bool GMEVGM_GetInfoStrL(int idx,char *str,int len)
{
  switch(idx){
/*
    case 0: snprintf(str,len,"%.32s",VGMInfo.game); return(true); break;
    case 1: snprintf(str,len,"%.32s",VGMInfo.author); return(true); break;
    case 2: snprintf(str,len,"%.32s",VGMInfo.copyright); return(true); break;
    case 3: snprintf(str,len,"track_count:%d/%d",CurrentTrackNum,VGMInfo.track_count); return(true); break;
    case 4: snprintf(str,len,"first_track:%d",VGMInfo.first_track); return(true); break;
*/
  }
  return(false);
}

bool GMEVGM_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool GMEVGM_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#endif
