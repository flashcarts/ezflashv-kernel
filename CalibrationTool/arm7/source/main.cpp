//////////////////////////////////////////////////////////////////////
// Simple ARM7 stub (sends RTC, TSC, and X/Y data to the ARM 9)
// -- joat
// -- modified by Darkain and others
//////////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <stdio.h>

#include <NDS.h>

#include "../../ipc4.h"

int main(int argc, char ** argv)
{
  IPC4->ready=0;
  
  // Reset the clock if needed
  rtcReset();
  
  REG_IME = 0;
  
  IPC4->xscale = ((PersonalData->calX2px - PersonalData->calX1px) << 19) / ((PersonalData->calX2) - (PersonalData->calX1));
  IPC4->yscale = ((PersonalData->calY2px - PersonalData->calY1px) << 19) / ((PersonalData->calY2) - (PersonalData->calY1));

  IPC4->xoffset = ((PersonalData->calX1 + PersonalData->calX2) * IPC4->xscale  - ((PersonalData->calX1px + PersonalData->calX2px) << 19) ) / 2;
  IPC4->yoffset = ((PersonalData->calY1 + PersonalData->calY2) * IPC4->yscale  - ((PersonalData->calY1px + PersonalData->calY2px) << 19) ) / 2;
  
  IPC4->ready=1;
  
  // Keep the ARM7 out of main RAM
  while (1){
    swiWaitForVBlank();
  }
  return 0;
}

