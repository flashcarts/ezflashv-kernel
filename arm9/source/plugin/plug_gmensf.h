
#ifndef plug_gmensf_h
#define plug_gmensf_h

#define LIBGMETitle "Game_Music_Emu 0.3.0 by Blargg"

#include "../_const.h"

extern bool StartGMENSF(int FileHandle,int TrackNum);
extern void FreeGMENSF(void);
extern u32 UpdateGMENSF(s16 *pbufl,s16 *pbufr);

extern s32 GMENSF_GetPosMax(void);
extern s32 GMENSF_GetPosOffset(void);
extern void GMENSF_SetPosOffset(s32 ofs);

extern u32 GMENSF_GetSampleRate(void);
extern u32 GMENSF_GetChannelCount(void);
extern u32 GMENSF_GetSamplePerFrame(void);

#include "../unicode.h"

extern int GMENSF_GetInfoIndexCount(void);
extern bool GMENSF_GetInfoStrL(int idx,char *str,int len);
extern bool GMENSF_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool GMENSF_GetInfoStrUTF8(int idx,char *str,int len);

#endif

