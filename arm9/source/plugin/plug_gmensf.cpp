
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "../_console.h"
#include <NDS/ARM9/CP15.h>

#include "../memtool.h"
#include "../mwin.h"
#include "../filesys.h"

#include "plug_gmensf.h"

#ifndef USEGME

bool StartGMENSF(int _FileHandle,int _TrackNum)
{
  return(false);
}

void FreeGMENSF(void)
{
}

u32 UpdateGMENSF(s16 *pbufl,s16 *pbufr)
{
  return(0);
}

s32 GMENSF_GetPosMax(void)
{
  return(0);
}

s32 GMENSF_GetPosOffset(void)
{
  return(0);
}

void GMENSF_SetPosOffset(s32 ofs)
{
}

u32 GMENSF_GetSampleRate(void)
{
  return(0);
}

u32 GMENSF_GetChannelCount(void)
{
  return(0);
}

u32 GMENSF_GetSamplePerFrame(void)
{
  return(0);
}

int GMENSF_GetInfoIndexCount(void)
{
  return(0);
}

bool GMENSF_GetInfoStrL(int idx,char *str,int len)
{
  return(false);
}

bool GMENSF_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool GMENSF_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#else

// -----------------

#include "libgme/abstract_file.h"
#include "libgme/Nsf_Emu.h"
#include "libgme/Effects_Buffer.h"

#define ESCF_VRCVI (BIT0)
#define ESCF_VRCVII (BIT1)
#define ESCF_FDS (BIT2)
#define ESCF_MMC5 (BIT3)
#define ESCF_Namco106 (BIT4)
#define ESCF_SunsoftFME07 (BIT5)
#define ESCF_Future6 (BIT6)
#define ESCF_Future7 (BIT7)

typedef struct {
  u8 ID[5];
  u8 VersionNumber;
  u8 TotalSongs;
  u8 StartingSong;
  u16 LoadAddress;
  u16 InitAddress;
  u16 PlayAddress;
  u8 *strName;
  u8 *strArtist;
  u8 *strCopyright;
  u16 NTSCTicksCount;
  u8 BankswitchInitValues[8];
  bool UseBankswitch;
  u16 PALTicksCount;
  u8 VSyncMode;
  u8 ExtraSoundChipFlags;
  u8 *Data;
  u32 DataSize;
  u32 PageCount;
} TNSFInfo;

static int FileHandle;
static int TrackNum;

static u8 *DeflateBuf=NULL;
static u32 DeflateSize;

static Music_Emu *pemu=NULL;
static Effects_Buffer *peffbuf=NULL;
static TNSFInfo NSFInfo;

static s16 *pGenBuf=NULL;

static int SampleRate=32768;
static int SamplePerFrame=2560/4;
static int BlankFrameSize=(SampleRate/SamplePerFrame)*3;

static int PosMax;
static int PosOffset;

// -----------------

static bool LoadNSFInfo(u8 *pb,u32 size)
{
#define writebyte(pos,data) (pb[pos]=data)
#define readbyte(pos) (pb[pos])
#define readword(pos) (readbyte(pos)+((u16)readbyte(pos+1) << 8))
#define readstr(pos) (&pb[pos])
  
  if(size<sizeof(TNSFInfo)){
    _consolePrintf("DataSize:%d<%d\n",size,sizeof(TNSFInfo));
    return(false);
  }
  
  NSFInfo.DataSize=size-sizeof(TNSFInfo);
  NSFInfo.Data=&pb[sizeof(TNSFInfo)];
  NSFInfo.PageCount=NSFInfo.DataSize/0x1000;
  
  for(u32 i=0;i<5;i++){
    NSFInfo.ID[i]=readbyte(0x00+i);
  }
  
  if((NSFInfo.ID[0]!='N')||(NSFInfo.ID[1]!='E')||(NSFInfo.ID[2]!='S')||(NSFInfo.ID[3]!='M')||(NSFInfo.ID[4]!=0x1a)){
    _consolePrintf("Illigal NSF format.\n");
    _consolePrintf("ID[]=");
    for(u32 i=0;i<8;i++){
      _consolePrintf("%02x,",pb[i]);
    }
    _consolePrintf("\n");
    return(false);
  }
  
  NSFInfo.TotalSongs=readbyte(0x06);
  NSFInfo.StartingSong=readbyte(0x07)-1;
  
  if(NSFInfo.TotalSongs==0){
    _consolePrintf("TotalSongs 0x%2x==0x00\n",NSFInfo.TotalSongs);
    return(false);
  }
  
  if(NSFInfo.StartingSong<=NSFInfo.TotalSongs){
    NSFInfo.StartingSong=NSFInfo.TotalSongs-1;
  }
  
  NSFInfo.LoadAddress=readword(0x08);
  
  if(NSFInfo.LoadAddress<0x8000){
    _consolePrintf("LoadAddress 0x%4x<0x8000\n",NSFInfo.LoadAddress);
    return(false);
  }
  
  NSFInfo.InitAddress=readword(0x0a);
  NSFInfo.PlayAddress=readword(0x0c);
  NSFInfo.strName=readstr(0x0e);
  NSFInfo.strArtist=readstr(0x2e);
  NSFInfo.strCopyright=readstr(0x4e);
  NSFInfo.NTSCTicksCount=readword(0x6e);
  
  NSFInfo.UseBankswitch=false;
  for(u32 i=0;i<8;i++){
    NSFInfo.BankswitchInitValues[i]=readbyte(0x70+i);
    if(NSFInfo.BankswitchInitValues[i]!=0x00) NSFInfo.UseBankswitch=true;
  }
  
  NSFInfo.PALTicksCount=readword(0x78);
  
  NSFInfo.VSyncMode=readbyte(0x7a) & BIT0;
  
  NSFInfo.ExtraSoundChipFlags=readbyte(0x7b);
  
  u8 excf=NSFInfo.ExtraSoundChipFlags;
  
  if((excf&ESCF_VRCVI)!=0) _consolePrintf("VRCVI,");
  if((excf&ESCF_VRCVII)!=0) _consolePrintf("VRCVII,");
  if((excf&ESCF_FDS)!=0) _consolePrintf("FDS,");
  if((excf&ESCF_MMC5)!=0) _consolePrintf("MMC5,");
  if((excf&ESCF_Namco106)!=0) _consolePrintf("Namco106,");
  if((excf&ESCF_SunsoftFME07)!=0) _consolePrintf("SunsoftFME07,");
  if((excf&ESCF_Future6)!=0) _consolePrintf("Future6,");
  if((excf&ESCF_Future7)!=0) _consolePrintf("Future7,");
  
  excf&=~(ESCF_VRCVI | ESCF_Namco106 | ESCF_SunsoftFME07);
  
  if(excf!=0x00){
    _consolePrintf("notsupport ExtraSoundChip\n");
  }
  
  // overwrite
  
  writebyte(0x7b,NSFInfo.ExtraSoundChipFlags & (ESCF_VRCVI | ESCF_Namco106 | ESCF_SunsoftFME07));
  
  return(true);

#undef writebyte
#undef readbyte
#undef readword
#undef readstr
}

static void ShowNSFInfo(void)
{
#define astr(ttl,data) _consolePrintf("%s:%s\n",ttl,data);
#define au8(ttl,data) _consolePrintf("%s:$%02x\n",ttl,data);
#define au16(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
#define au32(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
  
  _consolePrintf("ID:%4s $%02x\n",(u32*)NSFInfo.ID,NSFInfo.ID[4]);

  au8("VersionNumber",NSFInfo.VersionNumber);

  au8("TotalSongs",NSFInfo.TotalSongs);
  au8("StartingSong",NSFInfo.StartingSong);
  au16("LoadAddress",NSFInfo.LoadAddress);
  au16("InitAddress",NSFInfo.InitAddress);
  au16("PlayAddress",NSFInfo.PlayAddress);
  astr("strName",NSFInfo.strName);
  astr("strArtist",NSFInfo.strArtist);
  astr("strCopyright",NSFInfo.strCopyright);
  au16("NTSCTicksCount",NSFInfo.NTSCTicksCount);
  
  astr("BankswitchInitValues[8]","...");
  if(NSFInfo.UseBankswitch==false){
    _consolePrintf("notused.\n");
    }else{
    for(u32 cnt=0;cnt<8;cnt++){
      _consolePrintf("%02x,",NSFInfo.BankswitchInitValues[cnt]);
    }
    _consolePrintf("\n");
  }
  
  au16("PALTicksCount",NSFInfo.PALTicksCount);
  au8("VSyncMode",NSFInfo.VSyncMode);
  au8("ExtraSoundChipFlags",NSFInfo.ExtraSoundChipFlags);

  au32("DataPtr",(u32)&NSFInfo.Data[0]);
  au32("DataSize",NSFInfo.DataSize);
  au32("PageCount",NSFInfo.PageCount);

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
  pemu=new Nsf_Emu;
  
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

bool StartGMENSF(int _FileHandle,int _TrackNum)
{
  FileHandle=_FileHandle;
  TrackNum=_TrackNum;
  
  FileSys_fseek(FileHandle,0,SEEK_END);
  DeflateSize=FileSys_ftell(FileHandle);
  FileSys_fseek(FileHandle,0,SEEK_SET);
  
  DeflateBuf=(u8*)safemalloc(DeflateSize);
  if(DeflateBuf==NULL) return(false);
  FileSys_fread(DeflateBuf,1,DeflateSize,FileHandle);
  
  if(LoadNSFInfo(DeflateBuf,DeflateSize)==false){
    _consolePrintf("NSF Header error.\n");
    FreeGMENSF();
    return(false);
  }
  ShowNSFInfo();
  
  pGenBuf=(s16*)safemalloc(SamplePerFrame*2*2);
  if(pGenBuf==NULL){
    _consolePrintf("out of memory.\n");
    FreeGMENSF();
    return(false);
  }
  
  if(_start()==false){
    FreeGMENSF();
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

void FreeGMENSF(void)
{
  _free();
  
  if(pGenBuf!=NULL){
    safefree(pGenBuf); pGenBuf=NULL;
  }
  
  if(DeflateBuf!=NULL){
    safefree(DeflateBuf); DeflateBuf=NULL;
  }
}

u32 UpdateGMENSF(s16 *lbuf,s16 *rbuf)
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

s32 GMENSF_GetPosMax(void)
{
  return(PosMax);
}

s32 GMENSF_GetPosOffset(void)
{
  return(PosOffset);
}

void GMENSF_SetPosOffset(s32 ofs)
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
    if(UpdateGMENSF(NULL,NULL)==0) break;
  }
  MWin_ProgressHide();
}

u32 GMENSF_GetSampleRate(void)
{
  return(SampleRate);
}

u32 GMENSF_GetChannelCount(void)
{
  return(2);
}

u32 GMENSF_GetSamplePerFrame(void)
{
  return(SamplePerFrame);
}

int GMENSF_GetInfoIndexCount(void)
{
  return(7);
}

bool GMENSF_GetInfoStrL(int idx,char *str,int len)
{
  switch(idx){
    case 0: snprintf(str,len,"Name=%s",NSFInfo.strName); return(true); break;
    case 1: snprintf(str,len,"Artist=%s",NSFInfo.strArtist); return(true); break;
    case 2: snprintf(str,len,"Copyright=%s",NSFInfo.strCopyright); return(true); break;
    case 3: {
      u8 excf=NSFInfo.ExtraSoundChipFlags;
      if(excf==0) strcat(str,"not use extra sound chips.");
      if((excf&ESCF_VRCVI)!=0) strcat(str,"VRCVI,");
      if((excf&ESCF_VRCVII)!=0) strcat(str,"VRCVII,");
      if((excf&ESCF_FDS)!=0) strcat(str,"FDS,");
      if((excf&ESCF_MMC5)!=0) strcat(str,"MMC5,");
      if((excf&ESCF_Namco106)!=0) strcat(str,"Namco106,");
      if((excf&ESCF_SunsoftFME07)!=0) strcat(str,"SunsoftFME07,");
      if((excf&ESCF_Future6)!=0) strcat(str,"Future6,");
      if((excf&ESCF_Future7)!=0) strcat(str,"Future7,");
      return(true);
    } break;
    case 4: snprintf(str,len,"track_count:%d/%d",TrackNum,NSFInfo.TotalSongs); return(true); break;
    case 5: snprintf(str,len,"Load0x%x Init0x%x Play0x%x",NSFInfo.LoadAddress,NSFInfo.InitAddress,NSFInfo.PlayAddress); return(true); break;
    case 6: snprintf(str,len,"NTSCTicks=%d PALTicks=%d",NSFInfo.NTSCTicksCount,NSFInfo.PALTicksCount); return(true); break;
  }
  return(false);
}

bool GMENSF_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool GMENSF_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#endif
