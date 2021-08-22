
#ifndef _plug_dpg_h
#define _plug_dpg_h

#include "clibdpg.h"
#include "clibmpg.h"

#define DPGTitle0 libdpgTitle
#define DPGTitle1 libmpeg2Title
#define DPGTitle2 "libmad - MPEG audio decoder library"

extern bool DPG_RequestSyncStart;

extern bool StartDPG(int _FileHandleVideo,int _FileHandleAudio);
extern u32 UpdateDPG_Audio(s16 *lbuf,s16 *rbuf);
extern bool UpdateDPG_Video(u64 CurrentSamplesCount);
extern void FreeDPG(void);

extern u32 DPG_GetCurrentFrameCount(void);
extern u32 DPG_GetTotalFrameCount(void);
extern u32 DPG_GetFPS(void);
extern u32 DPG_GetSampleRate(void);
extern u32 DPG_GetChannelCount(void);
extern u32 DPG_GetSamplePerFrame(void);
extern void DPG_SetFrameCount(u32 Frame);

extern u32 DPG_GetWidth(void);
extern u32 DPG_GetHeight(void);

extern void DPG_fread(void);
extern void DPG_fread_flash(void);

#include "../unicode.h"

extern int DPG_GetInfoIndexCount(void);
extern bool DPG_GetInfoStrL(int idx,char *str,int len);
extern bool DPG_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool DPG_GetInfoStrUTF8(int idx,char *str,int len);

extern EDPGAudioFormat DPG_GetDPGAudioFormat(void);

#endif
