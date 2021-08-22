//////////////////////////////////////////////////////////////////////
// Simple ARM7 stub (sends RTC, TSC, and X/Y data to the ARM 9)
// -- joat
// -- modified by Darkain and others
//////////////////////////////////////////////////////////////////////


#include <stdlib.h>

#include <_interrupts.h>
#include <_touch.h>
#include <NDS.h>

#include "../../ipc3.h"
#include "memtoolARM7.h"
#include "a7sleep.h"
#include "plug_gsm.h"
#include "plug_mp2.h"

#include "main_boot_gbamp.h"
#include "main_boot_m3sc.h"

static u16 LastPowerReg;

#include "main_gradled.h"

//////////////////////////////////////////////////////////////////////

#define MP2_Multiple (1)

static u32 LastLCDPowerControl=LCDPC_ON_BOTH;
static bool LCDPowerApplyFlag=true;

static u32 strpcmCursorFlag=0;

static u32 strpcmSamples,strpcmChannels;
static EstrpcmFormat strpcmFormat;
static s16 *strpcmLBuf=NULL,*strpcmRBuf=NULL;

static s16 *strpcmL0=NULL,*strpcmL1=NULL,*strpcmR0=NULL,*strpcmR1=NULL;
static s16 tmp0[1152*MP2_Multiple]={1,},tmp1[1152*MP2_Multiple]={1,};
static s16 tmp2[1152*MP2_Multiple]={1,},tmp3[1152*MP2_Multiple]={1,};

#undef SOUND_FREQ
#define SOUND_FREQ(n)	(0x10000 - (16756000 / (n)))

static inline void strpcmPlay()
{
  IPC3->IR=IR_NULL;
  
  strpcmCursorFlag=0;
  
  strpcmFormat=IPC3->strpcmFormat;
  
  strpcmLBuf=IPC3->strpcmLBuf;
  strpcmRBuf=IPC3->strpcmRBuf;
  
  int Multiple=0;
  
  switch(strpcmFormat){
    case SPF_PCMx1: Multiple=1; break;
    case SPF_PCMx2: Multiple=2; break;
    case SPF_PCMx4: Multiple=4; break;
    case SPF_GSM: Multiple=1; break;
    case SPF_MP2: Multiple=MP2_Multiple; break;
  }
  
  if(strpcmFormat==SPF_MP2){
    if(StartMP2()==false) while(1);
    IPC3->strpcmSamples=MP2_GetSamplePerFrame();
    IPC3->strpcmChannels=MP2_GetChannelCount();
    IPC3->strpcmFreq=MP2_GetSampleRate();
  }
  
  strpcmSamples=IPC3->strpcmSamples;
  strpcmChannels=IPC3->strpcmChannels;
  
  if(strpcmFormat!=SPF_MP2){
    strpcmL0=(s16*)safemalloc(strpcmSamples*Multiple*2);
    strpcmL1=(s16*)safemalloc(strpcmSamples*Multiple*2);
    strpcmR0=(s16*)safemalloc(strpcmSamples*Multiple*2);
    strpcmR1=(s16*)safemalloc(strpcmSamples*Multiple*2);
    }else{
    strpcmL0=tmp0;
    strpcmL1=tmp1;
    strpcmR0=tmp2;
    strpcmR1=tmp3;
    for(u32 idx=0;idx<strpcmSamples*Multiple;idx++){
      strpcmL0[idx]=strpcmL1[idx]=0;
      strpcmR0[idx]=strpcmR1[idx]=0;
    }
  }
  
  if(strpcmFormat==SPF_GSM){
#ifdef USE_GSM
    StartGSM(strpcmSamples);
#endif
  }
  
//  powerON(POWER_SOUND);
//  SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
  SCHANNEL_CR(0) = 0;
  SCHANNEL_CR(1) = 0;
  SCHANNEL_CR(2) = 0;
  SCHANNEL_CR(3) = 0;
  
  int f=IPC3->strpcmFreq*Multiple;
  
  TIMER0_DATA = SOUND_FREQ(f);
  TIMER0_CR = TIMER_DIV_1 | TIMER_ENABLE;
  
  TIMER1_DATA = 0x10000 - (strpcmSamples*Multiple*2);
  TIMER1_CR = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;
  
  for(u32 ch=0;ch<4;ch++){
    SCHANNEL_CR(ch) = 0;
    SCHANNEL_TIMER(ch) = SOUND_FREQ(f);
    SCHANNEL_LENGTH(ch) = (strpcmSamples*Multiple*2) >> 2;
    SCHANNEL_REPEAT_POINT(ch) = 0;
  }
  
  IPC3->strpcmWriteRequest=0;
}

__attribute__((noinline)) static void strpcmStop()
{
//  powerOFF(POWER_SOUND);
//  SOUND_CR = 0;
  TIMER0_CR = 0;
  TIMER1_CR = 0;
  
  for(u32 ch=0;ch<4;ch++){
    SCHANNEL_CR(ch) = 0;
  }
  
  if(strpcmFormat!=SPF_MP2){
    safefree(strpcmL0); strpcmL0=NULL;
    safefree(strpcmL1); strpcmL1=NULL;
    safefree(strpcmR0); strpcmR0=NULL;
    safefree(strpcmR1); strpcmR1=NULL;
    }else{
    strpcmL0=NULL;
    strpcmL1=NULL;
    strpcmR0=NULL;
    strpcmR1=NULL;
  }
  
  if(strpcmFormat==SPF_GSM){
    FreeGSM();
  }
  
  if(strpcmFormat==SPF_MP2){
    FreeMP2();
  }
  
  IPC3->IR=IR_NULL;
}

//////////////////////////////////////////////////////////////////////

//#include "main_adpcm.h"

//////////////////////////////////////////////////////////////////////

static volatile bool ReqStrPCMRefresh;

static void (*fpStrPCM_Refresh)(void);

static volatile bool VsyncPassed;

#define MAX( x, y ) ( ( x > y ) ? x : y )
#define MIN( x, y ) ( ( x < y ) ? x : y )

static volatile int TPA_Count=0,TPA_X=0,TPA_Y=0;

static void InterruptHandler_Timer1(void)
{
  if(ReqStrPCMRefresh==true) return;
  
  s16 *lbuf,*rbuf;
  
  if(strpcmCursorFlag==0){
    lbuf=strpcmL0;
    rbuf=strpcmR0;
    }else{
    lbuf=strpcmL1;
    rbuf=strpcmR1;
  }
  
  u32 channel=strpcmCursorFlag;
  
  // Left channel
  SCHANNEL_CR(channel) = 0;
  SCHANNEL_SOURCE(channel) = (uint32)lbuf;
  SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0) | SOUND_16BIT;
  
  channel+=2;
  
  // Right channel
  SCHANNEL_CR(channel) = 0;
  SCHANNEL_SOURCE(channel) = (uint32)rbuf;
  SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(0x7F) | SOUND_PAN(0x7F) | SOUND_16BIT;
  
  strpcmCursorFlag=1-strpcmCursorFlag;
  
  ReqStrPCMRefresh=true;
}

static void InterruptHandler_Timer1_Null(void)
{
}

static void InterruptHandler_Timer1_PCM(void)
{
  s16 *lbuf,*rbuf;
  
  if(strpcmCursorFlag==0){
    lbuf=strpcmL0;
    rbuf=strpcmR0;
    }else{
    lbuf=strpcmL1;
    rbuf=strpcmR1;
  }
  
  s16 *lsrc=strpcmLBuf;
  s16 *rsrc=strpcmRBuf;
  
  u32 Samples=strpcmSamples;
  
  if(IPC3->strpcmWriteRequest!=0){
    MemSet16DMA3(0,lbuf,Samples*2);
    MemSet16DMA3(0,rbuf,Samples*2);
    }else{
    s16 *ldst,*rdst;
    
    switch(strpcmFormat){
      case SPF_PCMx1: {
        ldst=&lbuf[Samples*0]; rdst=&rbuf[Samples*0];
      } break;
      case SPF_PCMx2: {
        ldst=&lbuf[Samples*1]; rdst=&rbuf[Samples*1];
      } break;
      case SPF_PCMx4: {
        ldst=&lbuf[Samples*3]; rdst=&rbuf[Samples*3];
      } break;
      default: {
        ldst=NULL; rdst=NULL;
      }
    }
    
    if(strpcmChannels==2){
      MemCopy16DMA3(lsrc,ldst,Samples*2);
      MemCopy16DMA3(rsrc,rdst,Samples*2);
      }else{
      MemCopy16DMA3(lsrc,ldst,Samples*2);
      MemCopy16DMA3(lsrc,rdst,Samples*2);
    }
    
    IPC3->IR=IR_NextSoundData;
    REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;
    IPC3->strpcmWriteRequest=1;
    
    {
      s32 Volume=(s32)IPC3->strpcmVolume16;
      for(u32 cnt=Samples;cnt!=0;cnt--){
        s32 SrcSample;
        s32 Sample;
        
        SrcSample=(s32)*ldst;
        Sample=MAX(MIN(32767,(SrcSample*Volume)/16),-32768);
        *ldst++=(s16)Sample;
        
        SrcSample=(s32)*rdst;
        Sample=MAX(MIN(32767,(SrcSample*Volume)/16),-32768);
        *rdst++=(s16)Sample;
      }
    }
    
    static s16 slastl=0,slastr=0;
    
    switch(strpcmFormat){
      case SPF_PCMx1: {
      } break;
      case SPF_PCMx2: {
        s16 *lsrc=&lbuf[Samples*1],*rsrc=&rbuf[Samples*1];
        s16 *ldst=lbuf,*rdst=rbuf;
        s16 lastl=slastl,lastr=slastr;
        for(u32 cnt=Samples;cnt!=0;cnt--){
          s16 sample;
          
          sample=*lsrc++;
          *ldst++=lastl;
          *ldst++=(lastl+sample)/2;
          lastl=sample;
          
          sample=*rsrc++;
          *rdst++=lastr;
          *rdst++=(lastr+sample)/2;
          lastr=sample;
        }
        slastl=lastl; slastr=lastr;
      } break;
      case SPF_PCMx4: {
        s16 *lsrc=&lbuf[Samples*3],*rsrc=&rbuf[Samples*3];
        s16 *ldst=lbuf,*rdst=rbuf;
        s16 lastl=slastl,lastr=slastr;
        for(u32 cnt=Samples;cnt!=0;cnt--){
          s16 sample,half;
          
          sample=*lsrc++;
          half=(lastl+sample)/2;
          *ldst++=lastl;
          *ldst++=(lastl+half)/2;
          *ldst++=half;
          *ldst++=(half+sample)/2;
          lastl=sample;
          
          sample=*rsrc++;
          half=(lastr+sample)/2;
          *rdst++=lastr;
          *rdst++=(lastr+half)/2;
          *rdst++=half;
          *rdst++=(half+sample)/2;
          lastr=sample;
        }
        slastl=lastl; slastr=lastr;
      } break;
      default: {
      }
    }
    
  }
}

#ifndef USE_GSM

static void InterruptHandler_Timer1_GSM(void)
{
}

#else

static void InterruptHandler_Timer1_GSM(void)
{
  s16 *lbuf,*rbuf;
  
  if(strpcmCursorFlag==0){
    lbuf=strpcmL0;
    rbuf=strpcmR0;
    }else{
    lbuf=strpcmL1;
    rbuf=strpcmR1;
  }
  
  u32 Samples=strpcmSamples;
  
  if(IPC3->strpcmWriteRequest!=0){
    MemSet16DMA3(0,lbuf,Samples*2);
    MemSet16DMA3(0,rbuf,Samples*2);
    }else{
    IPC3->strpcmWriteRequest=1;
    
    int OutputSamples;
    
    if(IPC3->IR_flash==true){
      IPC3->IR_flash=false;
      OutputSamples=UpdateGSM(lbuf,true);
      }else{
      OutputSamples=UpdateGSM(lbuf,false);
    }
    
    if(OutputSamples!=0){
      s32 Volume=(s32)IPC3->strpcmVolume16;
      for(u32 cnt=Samples;cnt!=0;cnt--){
        s32 SrcSample;
        s32 Sample;
        
        SrcSample=(s32)*lbuf;
        Sample=MAX(MIN(32767,(SrcSample*Volume)/16),-32768);
        *lbuf++=*rbuf++=(s16)Sample;
      }
      IPC3->IR_samples+=(u64)OutputSamples;
      }else{
      for(u32 cnt=Samples;cnt!=0;cnt--){
        *lbuf++=0; *rbuf++=0;
      }
      IPC3->IR_samples+=(u64)Samples;
    }
    
    IPC3->strpcmWriteRequest=0;
  }
}

#endif

static void InterruptHandler_Timer1_MP2(void)
{
  s16 *lbuf,*rbuf;
  
  if(strpcmCursorFlag==0){
    lbuf=strpcmL0;
    rbuf=strpcmR0;
    }else{
    lbuf=strpcmL1;
    rbuf=strpcmR1;
  }
  
  u32 Samples=strpcmSamples;
  
  if(IPC3->strpcmWriteRequest!=0){
    MemSet16DMA3(0,lbuf,Samples*MP2_Multiple*2);
    MemSet16DMA3(0,rbuf,Samples*MP2_Multiple*2);
    }else{
    IPC3->strpcmWriteRequest=1;
    
    int OutputSamples;
    
    if(IPC3->IR_flash==true){
      IPC3->IR_flash=false;
      OutputSamples=UpdateMP2(lbuf,rbuf,true);
      }else{
      OutputSamples=UpdateMP2(lbuf,rbuf,false);
    }
    
    if(OutputSamples!=0){
      s32 Volume=(s32)IPC3->strpcmVolume16;
      for(u32 cnt=Samples*MP2_Multiple;cnt!=0;cnt--){
        s32 SrcSample;
        s32 Sample;
        
        SrcSample=(s32)*lbuf;
        Sample=MAX(MIN(32767,(SrcSample*Volume)/16),-32768);
        *lbuf++=(s16)Sample;
        
        SrcSample=(s32)*rbuf;
        Sample=MAX(MIN(32767,(SrcSample*Volume)/16),-32768);
        *rbuf++=(s16)Sample;
      }
      IPC3->IR_samples+=(u64)OutputSamples;
      }else{
      for(u32 cnt=Samples*MP2_Multiple;cnt!=0;cnt--){
        *lbuf++=0;
        *rbuf++=0;
      }
      IPC3->IR_samples+=(u64)Samples;
    }
    
    IPC3->strpcmWriteRequest=0;
    
  }
}

static void InterruptHandler_VBlank(void)
{
  static bool LastCloseFlag=false;
  
  bool CurrentCloseFlag=(REG_KEYXY==0x00FF);
  if(LastCloseFlag!=CurrentCloseFlag){
    if(CurrentCloseFlag==true){
      if(IPC3->WhenPanelClose==true){
        IPC3->LCDPowerControl=LCDPC_SOFT_POWEROFF;
      }
    }
    
    LastCloseFlag=CurrentCloseFlag;
    LCDPowerApplyFlag=true;
  }
  
  IPC3->heartbeat++;
  
  {
    if(IPC3->TouchPadCalibration!=ETPC_NULL){
      switch(IPC3->TouchPadCalibration){
        case ETPC_NULL: break;
        case ETPC_Auto: _touchReadXY_AutoDetect(); break;
        case ETPC_Manual: _touchReadXY_Override(IPC3->Calibration_xscale, IPC3->Calibration_yscale, IPC3->Calibration_xoffset, IPC3->Calibration_yoffset); break;
      }
      IPC3->TouchPadCalibration=ETPC_NULL;
    }
    
    if (!(REG_KEYXY & 0x40)) {
      touchPosition touchPos=_touchReadXY();
      TPA_Count++;
      TPA_X+=touchPos.px;
      TPA_Y+=touchPos.py;
      }else{
      TPA_Count=0;
      TPA_X=0;
      TPA_Y=0;
    }
  }
  
  VsyncPassed=true;
}

//////////////////////////////////////////////////////////////////////

__attribute__((noinline)) static void main_Proc_VsyncUpdate(void)
{
  uint16 but=0, xpx=0, ypx=0, batt=0, aux=0;
  int t1=0, t2=0;
  uint32 temp=0;
  uint8 ct[sizeof(IPC3->curtime)];
  
  // Read the X/Y buttons and the /PENIRQ line
  but = REG_KEYXY;
  
/*
  if (!(but & 0x40)) {
    touchPosition touchPos=_touchReadXY();
    xpx=touchPos.px;
    ypx=touchPos.py;
  }
*/
  {
    int c=TPA_Count,x=TPA_X,y=TPA_Y;
    TPA_Count=0;
    TPA_X=0;
    TPA_Y=0;
    
    if(c!=0){
      but&=~0x40;
      x=x/c;
      y=y/c;
      xpx=x;
      ypx=y;
      }else{
//      but|=0x40;
      if (!(but & 0x40)) {
        touchPosition touchPos=_touchReadXY();
        xpx=touchPos.px;
        ypx=touchPos.py;
      }
    }
  }
  
  batt = _touchRead(TSC_MEASURE_BATTERY);
  aux  = _touchRead(TSC_MEASURE_AUX);
  
  // Read the time
  if(IPC3->curtimeFlag==true){
    rtcGetTime((uint8 *)ct);
    BCDToInteger((uint8 *)&(ct[1]), 7);
    
    for(u32 i=0; i<sizeof(ct); i++) {
      IPC3->curtime[i] = ct[i];
    }
    
    IPC3->curtimeFlag=false;
  }
  
  // Read the temperature
  temp = _touchReadTemperature(&t1, &t2);
  
  // Update the IPC struct
  IPC3->buttons   = but;
  IPC3->touchXpx  = xpx;
  IPC3->touchYpx  = ypx;
  IPC3->battery   = batt;
  IPC3->aux       = aux;

  IPC3->temperature = temp;
  IPC3->tdiode1 = t1;
  IPC3->tdiode2 = t2;
}

static inline void SoftPowerOff(void)
{
  // referrence from SaTa.'s document
  a7poff();
  while(1);
}

// read firmware.
// reference hbfirmware.zip/firmware/arm7/source/settings.c

typedef void(*call0)(void);
typedef void(*call3)(u32,void*,u32);

//size must be a multiple of 4
static inline void read_nvram(u32 src, void *dst, u32 size) {
	((call3)0x2437)(src,dst,size);
}

//read firmware settings
__attribute__ ((long_call)) void load_PersonalData() {
	u32 src, count0, count1;

	read_nvram(0x20, &src, 4);		//find settings area
	src=(src&0xffff)*8;

	read_nvram(src+0x70, &count0, 4);	//pick recent copy
	read_nvram(src+0x170, &count1, 4);
	if((u16)count0<(u16)count1){
		src+=0x100;
	}
	
	read_nvram(src, PersonalData, 0x80);
	if(swiCRC16(0xffff,PersonalData,0x70) != ((u16*)PersonalData)[0x72/2]){ 	//invalid / corrupt?
		read_nvram(src^0x100, PersonalData, 0x80);	//try the older copy
	}
}

static inline void main_InitIRQ(void)
{
  // Set up the interrupt handler
  REG_IME = 0;
  
  _irqInit();
  
  _irqSet(IRQ_TIMER1,InterruptHandler_Timer1); // for Streaming PCM
  _irqSet(IRQ_TIMER2,InterruptHandler_Timer2); // for LED
  _irqSet(IRQ_VBLANK,InterruptHandler_VBlank);
  _irqSet(IRQ_TIMER3,InterruptHandler_Timer3); // for LED
  
  DISP_SR |= DISP_VBLANK_IRQ ;
  REG_IE = IRQ_TIMER1 | IRQ_TIMER2 | IRQ_TIMER3 | IRQ_VBLANK;
  
  REG_IME = 1;
}

#define PM_NDSLITE_ADR (4)
#define PM_NDSLITE_ISLITE BIT(6)
#define PM_NDSLITE_BRIGHTNESS(x) ((x & 0x03)<<0)
#define PM_NDSLITE_BRIGHTNESS_MASK (PM_NDSLITE_BRIGHTNESS(3))

static inline void main_InitNDSL(void)
{
  IPC3->isNDSLite = ( (PM_GetRegister(PM_NDSLITE_ADR) & PM_NDSLITE_ISLITE) != 0) ? true : false;
  if(IPC3->isNDSLite==false){
    IPC3->DefaultBrightness=0;
    }else{
    u8 data;
    data=PM_GetRegister(PM_NDSLITE_ADR);
    data&=PM_NDSLITE_BRIGHTNESS_MASK;
    IPC3->DefaultBrightness=data;
  }
  IPC3->Brightness=0xff;
}

static inline void main_InitSoundDevice(void)
{
  powerON(POWER_SOUND);
  SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
  
  POWER_CR&=~POWER_UNKNOWN; // wifi power off
  
  swiChangeSoundBias(1,0x400);
  a7SetSoundAmplifier(true);
}

__attribute__((noinline)) static void main_InitAll(void)
{
  IPC3->heartbeat=0;
  
  REG_IME=0;
  REG_IE=0;
  
  // Clear DMA
  for(int i=0;i<0x30/4;i++){
    *((vu32*)(0x40000B0+i))=0;
  }
  
  // Reset the clock if needed
  rtcReset();
  
  REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
  load_PersonalData();
  REG_SPICNT = 0;
  
  IPC3->curtimeFlag=true;
  IPC3->strpcmControl=strpcmControl_NOP;
  IPC3->LCDPowerControl=LCDPC_ON_BOTH;
  IPC3->IR=IR_NULL;
  IPC3->TouchPadCalibration=ETPC_NULL;
  
  main_InitNDSL();
  
  main_InitIRQ();
  
  main_InitSoundDevice();
  
  main_Init_GradLEDTable();
}

__attribute__((noinline)) static void main_Proc_strpcmControl(void)
{
  REG_IME=0;
  ReqStrPCMRefresh=false;
  
  switch(IPC3->strpcmControl){
    case strpcmControl_Play: {
      strpcmPlay();
      switch(strpcmFormat){
        case SPF_PCMx1: case SPF_PCMx2: case SPF_PCMx4: {
          fpStrPCM_Refresh=InterruptHandler_Timer1_PCM;
        } break;
        case SPF_GSM: fpStrPCM_Refresh=InterruptHandler_Timer1_GSM; break;
        case SPF_MP2: fpStrPCM_Refresh=InterruptHandler_Timer1_MP2; break;
        default: fpStrPCM_Refresh=InterruptHandler_Timer1_Null; break;
      }
    } break;
    case strpcmControl_Stop: {
      strpcmStop();
      fpStrPCM_Refresh=InterruptHandler_Timer1_Null;
    }
    default: {
      strpcmStop();
      fpStrPCM_Refresh=InterruptHandler_Timer1_Null;
    }
  }
  IPC3->strpcmControl=strpcmControl_NOP;
  REG_IME=1;
}

__attribute__((noinline)) static void main_Proc_LCDPowerControl(void)
{
  LastLCDPowerControl=IPC3->LCDPowerControl;
  IPC3->LCDPowerControl=LCDPC_NOP;
  LCDPowerApplyFlag=true;
}

__attribute__((noinline)) static void main_Proc_Brightness(void)
{
  u8 data;
  
  data=PM_GetRegister(PM_NDSLITE_ADR);
  data&=~PM_NDSLITE_BRIGHTNESS_MASK;
  data|=PM_NDSLITE_BRIGHTNESS(IPC3->Brightness);
  
  PM_SetRegister(PM_NDSLITE_ADR,data);
  
  IPC3->Brightness=0xff;
}

__attribute__((noinline)) static void main_Proc_LCDPowerApplyFlag(void)
{
  REG_IME=0;
  
  if(LastLCDPowerControl==LCDPC_SOFT_POWEROFF){ // exclusive
    SoftPowerOff();
    while(1);
  }
  
  extern void main_Init_GradLED(bool e);
  
  bool led;
  u16 lcd;
  
  if(REG_KEYXY == 0x00FF){ // PanelClose
    led=false;
    lcd=0;
    }else{
    switch(LastLCDPowerControl){
      case LCDPC_OFF_BOTH: {
        led=false;
        lcd=0;
      } break;
      case LCDPC_ON_BOTTOM: {
        led=true;
        lcd=PM_BACKLIGHT_BOTTOM;
      } break;
      case LCDPC_ON_TOP_LEDON: {
        led=true;
        lcd=PM_BACKLIGHT_TOP;
      } break;
      case LCDPC_ON_TOP_LEDBLINK: {
        led=false;
        lcd=PM_BACKLIGHT_TOP;
      } break;
      case LCDPC_ON_BOTH: {
        led=true;
        lcd=PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP;
      } break;
      case LCDPC_SOFT_POWEROFF: {
        SoftPowerOff();
        while(1);
      }
      default: while(1); break; // this execute is bug.
    }
  }
  
  if(led==true){
    a7led(0);
    main_Init_GradLED(false);
    }else{
    a7led(1);
    main_Init_GradLED(true);
  }
  a7lcd_select(lcd);
  LastPowerReg=PM_GetRegister(0);
  
  REG_IME=1;
}

// ----------

int main(int argc, char ** argv)
{
  main_InitAll();
  
  EnabledLEDTimer=false;
  TIMER2_CR = 0;
  TIMER3_CR = 0;
  
  ReqStrPCMRefresh=false;
  fpStrPCM_Refresh=InterruptHandler_Timer1_Null;
  
  while(IPC3->heartbeat==0);
  
  main_Init_GradLED(false);
  
  VsyncPassed=false;
  
  LastPowerReg=PM_GetRegister(0);
  
  // Keep the ARM7 out of main RAM
  while (1){
    if(VsyncPassed==false){
      if(EnabledGradLED==true){
        EnabledLEDTimer=true;
        main_Proc_GradLED();
      }
      while(1){
        swiIntrWait(0, 0xffff);
        if(VsyncPassed==true) break;
        if(ReqStrPCMRefresh==true){
          fpStrPCM_Refresh();
          ReqStrPCMRefresh=false;
        }
      }
      while(TIMER3_CR!=0);
      EnabledLEDTimer=false;
    }
    VsyncPassed=false;
    
    if(fpStrPCM_Refresh==InterruptHandler_Timer1_MP2){
      if(ReqStrPCMRefresh==true){
        fpStrPCM_Refresh();
        ReqStrPCMRefresh=false;
      }
    }
    
    if(IPC3->ReqVsyncUpdate!=0){
      REG_IE&=~(IRQ_VBLANK|IRQ_TIMER2|IRQ_TIMER3);
      main_Proc_VsyncUpdate();
      if(IPC3->ReqVsyncUpdate==1) IPC3->ReqVsyncUpdate=0;
      REG_IE|=IRQ_VBLANK|IRQ_TIMER2|IRQ_TIMER3;
    }
    
    if(IPC3->strpcmControl!=strpcmControl_NOP) main_Proc_strpcmControl();
    
    if(IPC3->LCDPowerControl!=LCDPC_NOP) main_Proc_LCDPowerControl();
    
    if(IPC3->Brightness!=0xff) main_Proc_Brightness();
    
    if(LCDPowerApplyFlag==true){
      REG_IE&=~(IRQ_VBLANK|IRQ_TIMER2|IRQ_TIMER3);
      LCDPowerApplyFlag=false;
      main_Proc_LCDPowerApplyFlag();
      REG_IE|=IRQ_VBLANK|IRQ_TIMER2|IRQ_TIMER3;
    }
    
    {
      // base source writed by SaTa. (include 2005/09/27 Moonlight.)
      // 垂直同期割り込みとかで定期的にチェック
      // **********************************************************************
      // Put this code into your ARM7 interrupt handler
      u32 bootfunc=*((vu32*)0x027FFE24);
      if (bootfunc == (u32)0x027FFE04) boot_GBAMP(); // Check for ARM9 reset (GBAMP)
      if (bootfunc == (u32)0x027FFE08) boot_M3SC(); // Check for ARM9 reset (M3/SC)
      // ***********************************************************************
    }
    
  }
  
  return 0;
}

