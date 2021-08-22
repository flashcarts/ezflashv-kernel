
#include <NDS.h>

#include "_console.h"
#include "_consoleWriteLog.h"

void PrfStart(void)
{
  TIMER0_CR=0;
  TIMER0_DATA=0;
//  TIMER0_CR=TIMER_ENABLE | TIMER_DIV_1024;
  TIMER0_CR=TIMER_ENABLE | TIMER_DIV_64;
}

u32 PrfEnd(int data)
{
  double us=TIMER0_DATA;
  
//  us=us*1024/33.34;
  us=us*64/33.34;
  
  u32 ius=(u32)us;
  
  if(ius<1000){
//    _consolePrint(".");
    }else{
    if(data!=-1){
      _consolePrintf("prf data=%d %6dus\n",data,ius);
      }else{
      _consolePrintf("prf %6dus\n",ius);
    }
  }
  
  return(ius);
}
