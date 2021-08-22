
#ifndef plug_gmevgm_h
#define plug_gmevgm_h

#define GMEVGMTitle "GMEVGM"

#include "../_const.h"

extern bool StartGMEVGM(int FileHandle,int TrackNum);
extern void FreeGMEVGM(void);
extern u32 UpdateGMEVGM(s16 *pbufl,s16 *pbufr);

extern s32 GMEVGM_GetPosMax(void);
extern s32 GMEVGM_GetPosOffset(void);
extern void GMEVGM_SetPosOffset(s32 ofs);

extern u32 GMEVGM_GetSampleRate(void);
extern u32 GMEVGM_GetChannelCount(void);
extern u32 GMEVGM_GetSamplePerFrame(void);

#include "../unicode.h"

extern int GMEVGM_GetInfoIndexCount(void);
extern bool GMEVGM_GetInfoStrL(int idx,char *str,int len);
extern bool GMEVGM_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool GMEVGM_GetInfoStrUTF8(int idx,char *str,int len);

#endif

