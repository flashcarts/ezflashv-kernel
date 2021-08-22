
#ifndef _plug_gsm_h
#define _plug_gsm_h

#define GSMTitle "libgsm - GSM 06.10 13 kbit/s RPE/LTP speech codec"

#ifdef __cplusplus
extern "C" {
#endif

bool StartGSM(int Samples);
u32 UpdateGSM(s16 *lbuf,bool flash);
void FreeGSM(void);

#ifdef __cplusplus
}
#endif

#endif
