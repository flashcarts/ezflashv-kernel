
#ifndef clibmpg_h
#define clibmpg_h

#define libmpeg2Title "libmpeg2 is a free library for decoding mpeg-2 and mpeg-1 video streams."

#include <NDS.h>
#include "../cstream.h"

#include "libmpeg2/inttypes.h"
#include "libmpeg2/mpeg2.h"
#include "libmpeg2/mpeg2convert.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int width,height;
  int Xdiv2,Ydiv2;
/*
  u16 table_rbuf[256+197*2];
  u16 table_bbuf[256+232*2];
  u16 table_gbuf[256+132*2];
  u16 *table_rV[256];
  u16 *table_gU[256];
  int table_gV[256];
  u16 *table_bU[256];
*/
} TYUV420toBGR15_DATA;

enum EMPGPixelFormat {PF_RGB15=0,PF_RGB18=1,PF_RGB21=2,PF_RGB24=3,PF_ENUMCOUNT=4};

class Clibmpg
{
  CStream *pCStream;
  int DataTopPosition;
  const u32 TotalFrameCount,FPS;
  const u32 SndFreq;
  const EMPGPixelFormat PixelFormat;
  u32 GlobalDelaySamples,LastDelaySample;
  u8 *ReadBuf;
  mpeg2dec_t *decoder;
  const mpeg2_info_t *info;
  int Width,Height;
  u32 framenum;
  u16 DrawBuf[ScreenHeight*ScreenWidth];
  TYUV420toBGR15_DATA YUV420toBGR15_DATA;
  Clibmpg(const Clibmpg&);
  Clibmpg& operator=(const Clibmpg&);
  int ProcReadBuffer(void);
  bool ProcSequence(void);
  void YUV420toBGR15_Init(int Width,int Height);
  void YUV420toBGR15_CopyFull(u8 *_YBuf,u8 *_UBuf,u8 *_VBuf,u16 *_FrameBuf);
  void YUV420toBGR15Fix_CopyFull(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf);
  void YUV420toBGR15Fix_CopyHalf(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf);
  void YUV420toBGR15Fix_CopyCross0(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf);
  void YUV420toBGR15Fix_CopyCross1(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf);
  void YUV420toBGR15Fix_CopyLU(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf);
  void YUV420toBGR15Fix_CopyRU(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf);
  void YUV420toBGR15Fix_CopyLD(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf);
  void YUV420toBGR15Fix_CopyRD(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf);
  void debug_tableout(void);
public:
  bool Initialized;
  Clibmpg(CStream *_pCStream,const u32 _TotalFrameCount,const u32 _FPS,const u32 _SndFreq,const EMPGPixelFormat _PixelFormat);
  ~Clibmpg(void);
  int GetWidth(void) const;
  int GetHeight(void) const;
  void Reopen(u32 StartFrame,u32 StartOffset);
  bool ProcMoveFrame(u32 TargetFrame,u64 TargetSamplesCount);
  bool ProcMoveFrameGOP(u32 TargetFrame,u64 TargetSamplesCount,u32 TargetGOPFrame,u32 TargetGOPOffset);
  void SetVideoDelayms(int ms);
  bool ProcDecode(u64 CurrentSamplesCount);
  int GetFrameNum(void);
};

#ifdef __cplusplus
}
#endif

#endif

