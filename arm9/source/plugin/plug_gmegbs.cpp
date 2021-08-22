
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "../_console.h"
#include <NDS/ARM9/CP15.h>

#include "../memtool.h"
#include "../mwin.h"
#include "../filesys.h"

#include "plug_gmegbs.h"

#ifndef USEGME

bool StartGMEGBS(int _FileHandle,int _TrackNum)
{
  return(false);
}

void FreeGMEGBS(void)
{
}

u32 UpdateGMEGBS(s16 *pbufl,s16 *pbufr)
{
  return(0);
}

s32 GMEGBS_GetPosMax(void)
{
  return(0);
}

s32 GMEGBS_GetPosOffset(void)
{
  return(0);
}

void GMEGBS_SetPosOffset(s32 ofs)
{
}

u32 GMEGBS_GetSampleRate(void)
{
  return(0);
}

u32 GMEGBS_GetChannelCount(void)
{
  return(0);
}

u32 GMEGBS_GetSamplePerFrame(void)
{
  return(0);
}

int GMEGBS_GetInfoIndexCount(void)
{
  return(0);
}

bool GMEGBS_GetInfoStrL(int idx,char *str,int len)
{
  return(false);
}

bool GMEGBS_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool GMEGBS_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#else

// -----------------

#include "libgme/abstract_file.h"
#include "libgme/Gbs_Emu.h"
#include "libgme/Effects_Buffer.h"

#pragma pack(1)
typedef struct {
  char tag [3];
  byte vers;
  byte track_count;
  byte first_track;
  byte load_addr [2];
  byte init_addr [2];
  byte play_addr [2];
  byte stack_ptr [2];
  byte timer_modulo;
  byte timer_mode;
  char game [32];
  char author [32];
  char copyright [32];
} TGBSInfo;
#pragma pack()

static int FileHandle;
static int TrackNum;

static u8 *DeflateBuf=NULL;
static u32 DeflateSize;

static Music_Emu *pemu=NULL;
static Effects_Buffer *peffbuf=NULL;
static TGBSInfo GBSInfo;

static s16 *pGenBuf=NULL;

static int SampleRate=32768;
static int SamplePerFrame=2560/4;
static int BlankFrameSize=(SampleRate/SamplePerFrame)*3;

static int PosMax;
static int PosOffset;

// -----------------

static bool LoadGBSInfo(u8 *pb,u32 size)
{
  if(size<sizeof(TGBSInfo)){
    _consolePrintf("DataSize:%d<%d\n",size,sizeof(TGBSInfo));
    return(false);
  }
  
  MemCopy8CPU(pb,&GBSInfo,sizeof(TGBSInfo));
  
  if((GBSInfo.tag[0]!='G')||(GBSInfo.tag[1]!='B')||(GBSInfo.tag[2]!='S')){
    _consolePrintf("Illigal GBS format.\n");
    _consolePrintf("tag[]=");
    for(u32 i=0;i<8;i++){
      _consolePrintf("%02x,",pb[i]);
    }
    _consolePrintf("\n");
    return(false);
  }
  
  return(true);
}

static void ShowGBSInfo(void)
{
#define astr(ttl,data) _consolePrintf("%s:%.32s\n",ttl,data);
#define au8(ttl,data) _consolePrintf("%s:$%02x\n",ttl,data);
#define au16(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
#define au32(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
  
  _consolePrintf("ID:%.3s\n",GBSInfo.tag);
  
  au8("VersionNumber",GBSInfo.vers);
  au8("track_count",GBSInfo.track_count);
  au8("first_track",GBSInfo.first_track);
  au16("load_addr",(GBSInfo.load_addr[0]<<8)|GBSInfo.load_addr[1]);
  au16("init_addr",(GBSInfo.init_addr[0]<<8)|GBSInfo.init_addr[1]);
  au16("play_addr",(GBSInfo.play_addr[0]<<8)|GBSInfo.play_addr[1]);
  au16("stack_ptr",(GBSInfo.stack_ptr[0]<<8)|GBSInfo.stack_ptr[1]);
  au8("timer_modulo",GBSInfo.timer_modulo);
  au8("timer_mode",GBSInfo.timer_mode);
  astr("strGame",GBSInfo.game);
  astr("strAuthor",GBSInfo.author);
  astr("strCopyright",GBSInfo.copyright);

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

static void _free(void)
{
  if(peffbuf!=NULL){
    delete peffbuf; peffbuf=NULL;
  }
  if(pemu!=NULL){
    delete pemu; pemu=NULL;
  }
}

static bool _start(void)
{
  pemu=new Gbs_Emu;
  
  peffbuf=new Effects_Buffer;
  
  pemu->set_buffer(peffbuf);
  
  const char* error;
  
  error=pemu->set_sample_rate(SampleRate);
  if(error!=NULL){
    _consolePrintf("gme error:%s\n",error);
    _free();
    return(false);
  }
  
  {
    Mem_File_Reader *pin;
    
    pin=new Mem_File_Reader(DeflateBuf,DeflateSize);
    error = pemu->load(*pin);
    delete pin; pin=NULL;
    
    if(error!=NULL){
      _consolePrintf("gme error:%s\n",error);
      _free();
      return(false);
    }
  }
  
  SetEffect(true);
  
  pemu->start_track(TrackNum);
  
  PosMax=90*SampleRate;
  PosOffset=0;
  
  return(true);
}

// -----------------

bool StartGMEGBS(int _FileHandle,int _TrackNum)
{
  FileHandle=_FileHandle;
  TrackNum=_TrackNum;
  
  FileSys_fseek(FileHandle,0,SEEK_END);
  DeflateSize=FileSys_ftell(FileHandle);
  FileSys_fseek(FileHandle,0,SEEK_SET);
  
  DeflateBuf=(u8*)safemalloc(DeflateSize);
  if(DeflateBuf==NULL) return(false);
  FileSys_fread(DeflateBuf,1,DeflateSize,FileHandle);
  
  if(LoadGBSInfo(DeflateBuf,DeflateSize)==false){
    _consolePrintf("GBS Header error.\n");
    FreeGMEGBS();
    return(false);
  }
  ShowGBSInfo();
  
  pGenBuf=(s16*)safemalloc(SamplePerFrame*2*2);
  if(pGenBuf==NULL){
    _consolePrintf("out of memory.\n");
    FreeGMEGBS();
    return(false);
  }
  
  if(_start()==false){
    FreeGMEGBS();
    return(false);
  }
  
  {
    int strcnt=pemu->voice_count();
    const char **strs=pemu->voice_names();
    
    for(int idx=0;idx<strcnt;idx++){
      _consolePrintf("%d:%s\n",idx,strs[idx]);
    }
  }
  
  return(true);
}

void FreeGMEGBS(void)
{
  _free();
  
  if(pGenBuf!=NULL){
    safefree(pGenBuf); pGenBuf=NULL;
  }
  
  if(DeflateBuf!=NULL){
    safefree(DeflateBuf); DeflateBuf=NULL;
  }
}

u32 UpdateGMEGBS(s16 *lbuf,s16 *rbuf)
{
  if(pemu==NULL) return(0);
  
  PosOffset+=SamplePerFrame;
  
  if((lbuf==NULL)||(rbuf==NULL)){
    SetEffect(false);
    pemu->skip(SamplePerFrame*2);
    SetEffect(true);
    return(SamplePerFrame);
  }
  
  s16 *chkdata=lbuf;
  
  s16 *srcbuf=pGenBuf;
  
  pemu->play(SamplePerFrame*2,srcbuf);
  
  for(u32 idx=SamplePerFrame;idx!=0;idx--){
    *lbuf++=*srcbuf++;
    *rbuf++=*srcbuf++;
  }
  
  {
    bool exists=false;
    
    s16 chksrc=chkdata[0];
    for(u32 idx=8;idx<(u32)(SamplePerFrame-8);idx+=8){
      if(0x10<abs(chksrc-chkdata[idx])) exists=true;
    }
    
    static int BlankCount=0;
    if(PosOffset==0) BlankCount=0;
    
    if(exists==true){
      BlankCount=BlankFrameSize;
      }else{
      if(BlankCount!=0){
        BlankCount--;
        if(BlankCount==0) return(0);
      }
    }
  }
  
  return(SamplePerFrame);
}

s32 GMEGBS_GetPosMax(void)
{
  return(PosMax);
}

s32 GMEGBS_GetPosOffset(void)
{
  return(PosOffset);
}

void GMEGBS_SetPosOffset(s32 ofs)
{
  if(pemu==NULL) return;
  
  if(ofs==PosOffset) return;
  
  if(ofs<PosOffset){
    _free();
    _start();
  }
  
  u32 baseofs=PosOffset;
  
  MWin_ProgressShow("Seek...",ofs-baseofs);
  MWin_ProgressSetPos(PosOffset-baseofs);
  
  while(PosOffset<ofs){
    if((PosOffset&0xf)==0) MWin_ProgressSetPos(PosOffset-baseofs);
    if(UpdateGMEGBS(NULL,NULL)==0) break;
  }
  MWin_ProgressHide();
}

u32 GMEGBS_GetSampleRate(void)
{
  return(SampleRate);
}

u32 GMEGBS_GetChannelCount(void)
{
  return(2);
}

u32 GMEGBS_GetSamplePerFrame(void)
{
  return(SamplePerFrame);
}

int GMEGBS_GetInfoIndexCount(void)
{
  return(5);
}

bool GMEGBS_GetInfoStrL(int idx,char *str,int len)
{
  switch(idx){
    case 0: snprintf(str,len,"%.32s",GBSInfo.game); return(true); break;
    case 1: snprintf(str,len,"%.32s",GBSInfo.author); return(true); break;
    case 2: snprintf(str,len,"%.32s",GBSInfo.copyright); return(true); break;
    case 3: snprintf(str,len,"track_count:%d/%d",TrackNum,GBSInfo.track_count); return(true); break;
    case 4: snprintf(str,len,"first_track:%d",GBSInfo.first_track); return(true); break;
  }
  return(false);
}

bool GMEGBS_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool GMEGBS_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#endif
