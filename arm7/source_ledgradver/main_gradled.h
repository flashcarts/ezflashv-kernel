
static volatile bool EnabledLEDTimer;
static volatile bool EnabledGradLED;

typedef struct {
  u32 NextCount;
  bool VectorPlus;
  int CurrentTimerCount;
  u16 TimerData;
} TGradLED;

static TGradLED GradLED;

// --------------

__attribute__((noinline)) static void setled(u32 f)
{
  SerialWaitBusy();
  REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER |SPI_BAUD_4MHz | SPI_CONTINUOUS;
  REG_SPIDATA = 0;
  u32 control = LastPowerReg & ~(PM_LED_CONTROL(3) | BIT(7));
  control|=(u16)f;
  SerialWaitBusy();
  REG_SPICNT = SPI_ENABLE | SPI_DEVICE_POWER |SPI_BAUD_4MHz;
  REG_SPIDATA = control;
}

static void InterruptHandler_Timer2(void)
{
  if(EnabledLEDTimer==false) return;
  
  REG_IE&=~IRQ_VBLANK;
  
  setled(PM_LED_CONTROL(0)); // on
  
  TIMER3_CR=0;
  TIMER3_DATA=GradLED.TimerData;
  TIMER3_CR=TIMER_IRQ_REQ | TIMER_ENABLE;
}

static void InterruptHandler_Timer3(void)
{
  TIMER3_CR=0;
  
  setled(PM_LED_CONTROL(1)); // off
  
  REG_IE|=IRQ_VBLANK;
}

// ----------

#define gtbl_max (128)
static u16 gtbl_td[gtbl_max];

static inline void main_Init_GradLEDTable(void)
{
  for(u32 i=0;i<gtbl_max;i++){
    u32 v;
    
    v=(0x400*i*i*i)/((gtbl_max-1)*(gtbl_max-1)*(gtbl_max-1));
    v=i*0x7800/0x400;
    v+=0x100;
    gtbl_td[i]=(u16)(0x10000-v);
  }
}

__attribute__((noinline)) static void main_Init_GradLED(bool e)
{
  if(EnabledGradLED==e) return;
  
  u16 ime=REG_IME;
  REG_IME=0;
  
  EnabledGradLED=false;
  
  TGradLED *pGradLED=&GradLED;
  
  pGradLED->NextCount=0;
  pGradLED->VectorPlus=false;
  pGradLED->CurrentTimerCount=gtbl_max;
  pGradLED->TimerData=0;
  
  TIMER2_CR = 0;
  TIMER2_DATA = 0;
  TIMER3_CR = 0;
  TIMER3_DATA = 0;
  
  if(e==true){
    EnabledGradLED=true;
    TIMER2_DATA = 0x8000;
    TIMER2_CR = TIMER_IRQ_REQ | TIMER_ENABLE;
  }
  
  REG_IME=ime;
}

__attribute__((noinline)) static void main_Proc_GradLED(void)
{
  TGradLED *pGradLED=&GradLED;
  
  if(pGradLED->NextCount!=0){
    pGradLED->NextCount--;
    return;
  }
  pGradLED->NextCount=1;
  
  bool VectorPlus=pGradLED->VectorPlus;
  int CurrentTimerCount=pGradLED->CurrentTimerCount;
  
  if(VectorPlus==true){
    CurrentTimerCount+=2;
    if(gtbl_max<=CurrentTimerCount){
      VectorPlus=false;
      CurrentTimerCount=gtbl_max-1;
    }
    }else{
    CurrentTimerCount-=1;
    if(CurrentTimerCount==-24) VectorPlus=true;
  }
  
  pGradLED->VectorPlus=VectorPlus;
  pGradLED->CurrentTimerCount=CurrentTimerCount;
  
  int ctc=CurrentTimerCount;
  
  if(ctc<0) ctc=0;
  pGradLED->TimerData=gtbl_td[ctc];
}

