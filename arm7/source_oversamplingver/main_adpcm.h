
#define ADPCMBufSize (strpcmSamplesMax/2*2)
static u8 strpcmADPCMBuf[ADPCMBufSize];

static const s32 adpcm_estimstep[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

static const s32 adpcm_estimindex[16] = {
	 2,  6,  10,  14,  18,  22,  26,  30,
	-2, -6, -10, -14, -18, -22, -26, -30
};

static const s32 adpcm_estim[49] = {
	 16,  17,  19,  21,  23,  25,  28,  31,  34,  37,
	 41,  45,  50,  55,  60,  66,  73,  80,  88,  97,
	107, 118, 130, 143, 157, 173, 190, 209, 230, 253,
	279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
	724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552
};

static s32 lmc_amp=0,lmc_estim=0;
static s32 rmc_amp=0,rmc_estim=0;

static void sndbuf_Handler_PCMProc_ADPCM4bit(u8 *readbuf,s16 *lbuf,u32 SamplePerFrame);
static void sndbuf_Handler_PCMProc_ADPCM4bit2ch(u8 *readbuf,s16 *lbuf,s16 *rbuf,u32 SamplePerFrame);
static void sndbuf_Handler_PCMProc_ADPCM4bitOverSampling(u8 *readbuf,s16 *lbuf,u32 SamplePerFrame);
static void sndbuf_Handler_PCMProc_ADPCM4bit2chOverSampling(u8 *readbuf,s16 *lbuf,s16 *rbuf,u32 SamplePerFrame);

void DecodeADPCM(u8 *ADPCMbuf,s16 *lbuf,s16 *rbuf,u32 Samples,u32 strpcmChannels)
{
  switch(strpcmChannels){
    case 0: break;
    case 1: {
      sndbuf_Handler_PCMProc_ADPCM4bitOverSampling(ADPCMbuf,lbuf,Samples);
    } break;
    case 2: {
      sndbuf_Handler_PCMProc_ADPCM4bit2chOverSampling(ADPCMbuf,lbuf,rbuf,Samples);
    } break;
  }
}

#define MAX( x, y ) ( ( x > y ) ? x : y )
#define MIN( x, y ) ( ( x < y ) ? x : y )

#define sndbuf_Handler_PCMProc_ADPCM4bit_Decode(_adpcmdata) {\
  u8 adpcmdata=_adpcmdata;\
  \
  tmc_amp+=adpcm_estim[tmc_estim]*adpcm_estimindex[adpcmdata];\
  if((u32)tmc_amp>0x7fff){\
    tmc_amp=MIN(MAX(tmc_amp,-0x8000),0x7fff);\
  }\
  \
  tmc_estim+=adpcm_estimstep[adpcmdata];\
  if((u32)tmc_estim>48){\
    tmc_estim=MIN(MAX(tmc_estim,0),48);\
  }\
  \
  *buf=(s16)(tmc_amp);\
  buf++;\
}

static void sndbuf_Handler_PCMProc_ADPCM4bit(u8 *readbuf,s16 *buf,u32 SamplePerFrame)
{
  s32 tmc_amp=lmc_amp;
  s32 tmc_estim=lmc_estim;
  
  for(s32 cnt=(SamplePerFrame/2*1)-1;cnt>=0;cnt--){
    sndbuf_Handler_PCMProc_ADPCM4bit_Decode(*readbuf&0x0f);
    sndbuf_Handler_PCMProc_ADPCM4bit_Decode((*readbuf>>4)&0x0f);
    readbuf++;
  }
  
  lmc_amp=tmc_amp;
  lmc_estim=tmc_estim;
}

#define sndbuf_Handler_PCMProc_ADPCM4bit2ch_Decode(tmc_amp,tmc_estim,buf,_adpcmdata) {\
  u8 adpcmdata=_adpcmdata;\
  \
  tmc_amp+=adpcm_estim[tmc_estim]*adpcm_estimindex[adpcmdata];\
  if((u32)tmc_amp>0x7fff){\
    tmc_amp=MIN(MAX(tmc_amp,-0x8000),0x7fff);\
  }\
  \
  tmc_estim+=adpcm_estimstep[adpcmdata];\
  if((u32)tmc_estim>48){\
    tmc_estim=MIN(MAX(tmc_estim,0),48);\
  }\
  \
  *buf=(s16)(tmc_amp);\
  buf++;\
}

static void sndbuf_Handler_PCMProc_ADPCM4bit2ch(u8 *readbuf,s16 *lbuf,s16 *rbuf,u32 SamplePerFrame)
{
  s32 tlmc_amp=lmc_amp;
  s32 tlmc_estim=lmc_estim;
  s32 trmc_amp=rmc_amp;
  s32 trmc_estim=rmc_estim;
  
  for(s32 cnt=(SamplePerFrame/2*2)-1;cnt>=0;cnt--){
    sndbuf_Handler_PCMProc_ADPCM4bit2ch_Decode(tlmc_amp,tlmc_estim,lbuf,*readbuf&0x0f);
    sndbuf_Handler_PCMProc_ADPCM4bit2ch_Decode(trmc_amp,trmc_estim,rbuf,(*readbuf>>4)&0x0f);
    readbuf++;
  }
  
  lmc_amp=tlmc_amp;
  lmc_estim=tlmc_estim;
  rmc_amp=trmc_amp;
  rmc_estim=trmc_estim;
}

#define sndbuf_Handler_PCMProc_ADPCM4bitOverSampling_Decode(_adpcmdata) {\
  u8 adpcmdata=_adpcmdata;\
  s32 lastamp=tmc_amp;\
  \
  tmc_amp+=adpcm_estim[tmc_estim]*adpcm_estimindex[adpcmdata];\
  if((u32)tmc_amp>0x7fff){\
    tmc_amp=MIN(MAX(tmc_amp,-0x8000),0x7fff);\
  }\
  \
  tmc_estim+=adpcm_estimstep[adpcmdata];\
  if((u32)tmc_estim>48){\
    tmc_estim=MIN(MAX(tmc_estim,0),48);\
  }\
  \
  *buf=(s16)((lastamp+tmc_amp)>>1);\
  buf++;\
  *buf=(s16)(tmc_amp);\
  buf++;\
}

static void sndbuf_Handler_PCMProc_ADPCM4bitOverSampling(u8 *readbuf,s16 *buf,u32 SamplePerFrame)
{
  s32 tmc_amp=lmc_amp;
  s32 tmc_estim=lmc_estim;
  
  for(s32 cnt=(SamplePerFrame/2*1/2)-1;cnt>=0;cnt--){
    sndbuf_Handler_PCMProc_ADPCM4bitOverSampling_Decode(*readbuf&0x0f);
    sndbuf_Handler_PCMProc_ADPCM4bitOverSampling_Decode((*readbuf>>4)&0x0f);
    readbuf++;
  }
  
  lmc_amp=tmc_amp;
  lmc_estim=tmc_estim;
}

#define sndbuf_Handler_PCMProc_ADPCM4bit2chOverSampling_Decode(tmc_amp,tmc_estim,buf,_adpcmdata) {\
  u8 adpcmdata=_adpcmdata;\
  s32 lastamp=tmc_amp;\
  \
  tmc_amp+=adpcm_estim[tmc_estim]*adpcm_estimindex[adpcmdata];\
  if((u32)tmc_amp>0x7fff){\
    tmc_amp=MIN(MAX(tmc_amp,-0x8000),0x7fff);\
  }\
  \
  tmc_estim+=adpcm_estimstep[adpcmdata];\
  if((u32)tmc_estim>48){\
    tmc_estim=MIN(MAX(tmc_estim,0),48);\
  }\
  \
  *buf=(s16)((lastamp+tmc_amp)>>1);\
  buf++;\
  *buf=(s16)(tmc_amp);\
  buf++;\
}

static void _sndbuf_Handler_PCMProc_ADPCM4bit2chOverSampling(u8 *readbuf,s16 *lbuf,s16 *rbuf,u32 SamplePerFrame)
{
  s32 tlmc_amp=lmc_amp;
  s32 tlmc_estim=lmc_estim;
  s32 trmc_amp=rmc_amp;
  s32 trmc_estim=rmc_estim;
  
  for(s32 cnt=(SamplePerFrame/2*2/2)-1;cnt>=0;cnt--){
    sndbuf_Handler_PCMProc_ADPCM4bit2chOverSampling_Decode(tlmc_amp,tlmc_estim,lbuf,*readbuf&0x0f);
    sndbuf_Handler_PCMProc_ADPCM4bit2chOverSampling_Decode(trmc_amp,trmc_estim,rbuf,(*readbuf>>4)&0x0f);
    readbuf++;
  }
  
  lmc_amp=tlmc_amp;
  lmc_estim=tlmc_estim;
  rmc_amp=trmc_amp;
  rmc_estim=trmc_estim;
}

static void sndbuf_Handler_PCMProc_ADPCM4bit2chOverSampling(u8 *_readbuf,s16 *lbuf,s16 *rbuf,u32 SamplePerFrame)
{
  
  {
    s32 tlmc_amp=lmc_amp;
    s32 tlmc_estim=lmc_estim;
    
    u8 *readbuf=_readbuf;
    
    for(s32 cnt=(SamplePerFrame/2*2/2)-1;cnt>=0;cnt--){
      sndbuf_Handler_PCMProc_ADPCM4bit2chOverSampling_Decode(tlmc_amp,tlmc_estim,lbuf,*readbuf&0x0f);
      readbuf++;
    }
    
    lmc_amp=tlmc_amp;
    lmc_estim=tlmc_estim;
  }
  
  {
    s32 trmc_amp=rmc_amp;
    s32 trmc_estim=rmc_estim;
    
    u8 *readbuf=_readbuf;
    
    for(s32 cnt=(SamplePerFrame/2*2/2)-1;cnt>=0;cnt--){
      sndbuf_Handler_PCMProc_ADPCM4bit2chOverSampling_Decode(trmc_amp,trmc_estim,rbuf,*readbuf&0x0f);
      readbuf++;
    }
    
    rmc_amp=trmc_amp;
    rmc_estim=trmc_estim;
  }
}


