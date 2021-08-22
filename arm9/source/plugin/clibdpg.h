
#ifndef clibdpg_h
#define clibdpg_h

#define libdpgTitle "libdpg mpeg-1 and GSM06.10 mux"

#include <NDS.h>
#include "../cstream.h"
#include "../glib/glib.h"

#include "clibmpg.h"
#include "plug_gsm.h"
#include "plug_mp2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int TotalFrame;
  int FPS; // fix8.8
  int SndFreq,SndCh;
  int AudioPos,AudioSize;
  int MoviePos,MovieSize;
  int GOPListPos,GOPListSize;
  EMPGPixelFormat PixelFormat;
} TDPGINFO;

typedef struct {
  u32 FrameIndex;
  u32 Offset;
} TGOPList;

enum EDPGAudioFormat {DPGAF_GSM,DPGAF_MP2};

class Clibdpg
{
  CStream *pCStreamMovie,*pCStreamAudio;
  Clibmpg *pClibmpg;
  Clibdpg(const Clibdpg&);
  Clibdpg& operator=(const Clibdpg&);
  bool LoadDPGINFO(CStream *pCStream);
  u32 GOPListCount;
  TGOPList *pGOPList;
  u32 DelayFrames;
public:
  Clibdpg(CStream *_pCStreamMovie,CStream *_pCStreamAudio);
  ~Clibdpg(void);
  bool Initialized;
  TDPGINFO DPGINFO;
  bool MovieProcDecode(u64 CurrentSamplesCount);
  int AudioDecode(s16 *lbuf,s16 *rbuf);
  void SetFrame(int Frame);
  int GetFrameNum(void);
  int GetWidth(void);
  int GetHeight(void);
  EDPGAudioFormat GetDPGAudioFormat(void);
};

#ifdef __cplusplus
}
#endif

#endif

