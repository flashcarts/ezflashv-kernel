
#ifndef _MP2_h
#define _MP2_h

#define MP2Title "MP2 decode from ARM7"

#include "../cstream.h"

extern bool StartMP2(CStream *_pCStream);
extern u32 UpdateMP2(s16 *lbuf,s16 *rbuf);
extern void FreeMP2(void);

extern u32 MP2_GetSamplePerFrame(void);

extern void MP2_SetPosition(double per,s64 smp);

extern void MP2_LoadReadBuffer(void);

extern void MP2_fread(void);
extern void MP2_fread_flash(void);

#endif
