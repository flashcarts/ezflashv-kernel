
#ifndef strpcm_h
#define strpcm_h

#include <NDS.h>
#include "../../IPC3.h"

#define strpcmRingBufCount (8)
#define strpcmRingBufBitMask (strpcmRingBufCount-1)

extern volatile bool VsyncPassedCount;

extern volatile bool strpcmRequestStop;

extern volatile bool strpcmRingEmptyFlag;
extern volatile u32 strpcmRingBufReadIndex;
extern volatile u32 strpcmRingBufWriteIndex;

extern s16 *strpcmRingLBuf;
extern s16 *strpcmRingRBuf;

extern void InterruptHandler(void);
extern void InitInterrupts(void);

extern void strpcmStart(bool FastStart,u32 SampleRate,u32 SamplePerBuf,u32 ChannelCount,EstrpcmFormat strpcmFormat);
extern void strpcmStop(void);

extern void strpcmSetVolume16(int v);
extern int strpcmGetVolume16(void);
extern void strpcmSetPause(bool v);
extern bool strpcmGetPause(void);

#endif
