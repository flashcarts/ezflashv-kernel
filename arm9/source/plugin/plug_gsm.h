
#ifndef _plug_gsm_h
#define _plug_gsm_h

#define GSMTitle "libgsm - GSM 06.10 13 kbit/s RPE/LTP speech codec"

#include "../cstream.h"

#ifdef __cplusplus
extern "C" {
#endif

bool StartGSM(CStream *pCStream);
u32 UpdateGSM(s16 *lbuf,s16 *rbuf);
void FreeGSM(void);

u32 GSM_GetChannelCount(void);
u32 GSM_GetSampleRate(void);
u32 GSM_GetSamplePerFrame(void);

void GSM_SetPosition(double per,s64 smp);

void GSM_LoadReadBuffer(void);

void GSM_fread(void);
void GSM_fread_flash(void);

#ifdef __cplusplus
}
#endif

#endif
