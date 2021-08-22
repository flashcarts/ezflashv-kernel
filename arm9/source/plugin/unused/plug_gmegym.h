
#ifndef plug_gmegym_h
#define plug_gmegym_h

#define GMEGYMTitle "GMEGYM"

#include "../_const.h"

extern bool StartGMEGYM(int FileHandle,int TrackNum);
extern void FreeGMEGYM(void);
extern u32 UpdateGMEGYM(s16 *pbufl,s16 *pbufr);

extern s32 GMEGYM_GetPosMax(void);
extern s32 GMEGYM_GetPosOffset(void);
extern void GMEGYM_SetPosOffset(s32 ofs);

extern u32 GMEGYM_GetSampleRate(void);
extern u32 GMEGYM_GetChannelCount(void);
extern u32 GMEGYM_GetSamplePerFrame(void);

#include "../unicode.h"

extern int GMEGYM_GetInfoIndexCount(void);
extern bool GMEGYM_GetInfoStrL(int idx,char *str,int len);
extern bool GMEGYM_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool GMEGYM_GetInfoStrUTF8(int idx,char *str,int len);

#endif

