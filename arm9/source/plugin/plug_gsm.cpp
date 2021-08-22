
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NDS.h>

#include "../_console.h"
#include "../_consoleWriteLog.h"
#include "../memtool.h"

#include "../../../ipc3.h"

#include "plug_gsm.h"

#define MAX( x, y ) ( ( x > y ) ? x : y )
#define MIN( x, y ) ( ( x < y ) ? x : y )

#define GSM_BlockSamples (320)
#define GSM_BlockBytes (65)
#define GSM_BlockCount (4)

#define ReadBufSize (GSM_BlockBytes*GSM_BlockCount*4)

#define SamplePerFrame (GSM_BlockSamples*GSM_BlockCount)

static u32 MovePosition;
static s64 MoveSample;

static u8 *pReadTempBuffer;

// --------------------------------------------------------------------

static bool Initialized=false;

typedef struct {
  u16 wFormatTag;
  u16 nChannels;
  u32 nSamplesPerSec;
  u32 nAvgBytesPerSec;
  u16 nBlockAlign;
  u16 wBitsPerSample;
  u16 cbSize;
} WAVEFORMATEX;

static int StreamTopOffset;
static CStream *pCStream;
static WAVEFORMATEX wfex;
static int DataTopOffset;
static int DataSize;

// --------------------------------------------------------------------

static void WaveFile_ReadWaveChunk(void)
{
  pCStream->SetOffset(StreamTopOffset+0x14);
  pCStream->ReadBuffer((void*)&wfex,sizeof(wfex));
  
  if(wfex.wFormatTag!=0x31){
    _consolePrintf("Illigal CompressFormat Error.");
    while(1);
  }
  
  _consolePrintf("fmt:0x%x(GSM06.10) chns:%d\n",wfex.wFormatTag,wfex.nChannels);
  _consolePrintf("Smpls/Sec:%d AvgBPS:%d\n",wfex.nSamplesPerSec,wfex.nAvgBytesPerSec);
  _consolePrintf("BlockAlign:%d Bits/Smpl:%d\n",wfex.nBlockAlign,wfex.wBitsPerSample);
}

static void WaveFile_SeekDataChunk(void)
{
  pCStream->SetOffset(StreamTopOffset+0);
  
  while(pCStream->Readu32()!=0x61746164){ // find "data"
    if(pCStream->eof()==true){
      _consolePrintf("can not find data chunk.\n");
      return;
    }
  }
  
  DataSize=pCStream->Readu32();
  
  DataTopOffset=pCStream->GetOffset()-StreamTopOffset;
  
  _consolePrintf("DataTop:%d DataSize:%dbyte\n",DataTopOffset,DataSize);
}

static void WaveFile_OpenGSMDecode(CStream *_pCStream)
{
  pCStream=_pCStream;
  
  u32 RIFFID=pCStream->Readu32();
  
  if(RIFFID!=0x46464952){ // check "RIFF"
    _consolePrintf("error.not RIFFWAVEFILE");
    _consolePrintf("topdata:0x%04x\n",RIFFID);
    while(1);
  }
  
  WaveFile_ReadWaveChunk();
  WaveFile_SeekDataChunk();
}

static void WaveFile_CloseGSMDecode(void)
{
  pCStream=NULL;
}

// --------------------------------------------------------------------

// --------------------------------------------------------------------

bool StartGSM(CStream *pCStream)
{
  FreeGSM();
  Initialized=true;
  
  StreamTopOffset=pCStream->GetOffset();
  
  WaveFile_OpenGSMDecode(pCStream);
  
  _consolePrintf("Request ARM7 GSM decode.\n");
  
  IPC3->IR=IR_NULL;
  IPC3->IR_filesize=DataSize;
  IPC3->IR_readsize=0;
  IPC3->IR_readbuf=(u8*)malloc(ReadBufSize);
  IPC3->IR_readbufsize=0;
  IPC3->IR_flash=false;
  
  IPC3->IR_samples=0;
  
  MovePosition=0;
  MoveSample=0;
  
  IPC3->IR_readbufsize=pCStream->ReadBuffer(IPC3->IR_readbuf,ReadBufSize);
  
  pReadTempBuffer=(u8*)malloc(ReadBufSize);
  
  return(true);
}

u32 UpdateGSM(s16 *lbuf,s16 *rbuf)
{
  return(SamplePerFrame);
}

void FreeGSM(void)
{
  if(Initialized==true){
    WaveFile_CloseGSMDecode();
    
    pCStream=NULL;
    
    IPC3->IR_filesize=0;
    IPC3->IR_readsize=0;
    if(IPC3->IR_readbuf!=NULL){
      free(IPC3->IR_readbuf); IPC3->IR_readbuf=NULL;
    }
    if(pReadTempBuffer!=NULL){
      free(pReadTempBuffer); pReadTempBuffer=NULL;
    }
  }
  Initialized=false;
}

u32 GSM_GetChannelCount(void)
{
  return(wfex.nChannels);
}

u32 GSM_GetSampleRate(void)
{
  return(wfex.nSamplesPerSec);
}

u32 GSM_GetSamplePerFrame(void)
{
  return(SamplePerFrame);
}

void GSM_SetPosition(double per,s64 smp)
{
  int ofs=StreamTopOffset+DataTopOffset;
  
  MovePosition=(u32)(DataSize*per);
  MovePosition/=GSM_BlockBytes;
  MovePosition*=GSM_BlockBytes;
  MovePosition+=ofs;
  
  if(smp<0) smp=0;
  MoveSample=smp;
  IPC3->IR_flash=true;
}

void GSM_LoadReadBuffer(void)
{
  if(IPC3->IR_flash==true) return;
  
  u8 *buf=(u8*)IPC3->IR_readbuf;
  int ReadSize=ReadBufSize-IPC3->IR_readbufsize;
  
  ReadSize=pCStream->ReadBuffer(pReadTempBuffer,ReadSize);
  
  REG_IME=0;
  
  memmove(&buf[IPC3->IR_readbufsize],pReadTempBuffer,ReadSize);
  IPC3->IR_readbufsize+=ReadSize;
  
  REG_IME=1;
}

void GSM_fread(void)
{
  u8 *buf=(u8*)IPC3->IR_readbuf;
  int size=IPC3->IR_readsize;
  int remain=IPC3->IR_readbufsize-size;
  
  if(remain<0){
    remain=0;
    }else{
    memmove(&buf[0],&buf[size],remain);
  }
  
  IPC3->IR_readbufsize=remain;
}

void GSM_fread_flash(void)
{
  pCStream->SetOffset(MovePosition);
  MovePosition=0;
  
  IPC3->IR_readbufsize=pCStream->ReadBuffer(IPC3->IR_readbuf,ReadBufSize);
  
  IPC3->IR_samples=MoveSample;
  MoveSample=0;
}

