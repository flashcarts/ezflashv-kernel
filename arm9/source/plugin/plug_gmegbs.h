
#ifndef plug_gmegbs_h
#define plug_gmegbs_h

#define GMEGBSTitle "GMEGBS"

#include "../_const.h"

extern bool StartGMEGBS(int FileHandle,int TrackNum);
extern void FreeGMEGBS(void);
extern u32 UpdateGMEGBS(s16 *pbufl,s16 *pbufr);

extern s32 GMEGBS_GetPosMax(void);
extern s32 GMEGBS_GetPosOffset(void);
extern void GMEGBS_SetPosOffset(s32 ofs);

extern u32 GMEGBS_GetSampleRate(void);
extern u32 GMEGBS_GetChannelCount(void);
extern u32 GMEGBS_GetSamplePerFrame(void);

#include "../unicode.h"

extern int GMEGBS_GetInfoIndexCount(void);
extern bool GMEGBS_GetInfoStrL(int idx,char *str,int len);
extern bool GMEGBS_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool GMEGBS_GetInfoStrUTF8(int idx,char *str,int len);

#endif

