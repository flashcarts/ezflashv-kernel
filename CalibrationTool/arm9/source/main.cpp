
#include <NDS.h>
#include <NDS/ARM9/CP15.h>

#include "_console.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "memtool.h"
//#include "strtool.h"

#include "maindef.h"

#include "../../ipc4.h"
//#include "arm9tcm.h"

#include "_const.h"
#include "boolean.h"

#include "Emulator.h"

#include "setarm9_reg_waitcr.h"

static b8 ShowLog;

extern void videoSetModeSub_SetShowLog(b8 e);

// --------------------------------------------------------

// ------------------------------------------------------------------

void videoSetModeSub_SetShowLog(b8 e)
{
  ShowLog=e;
  
  if(e==False){
    videoSetModeSub(MODE_0_2D | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D | DISPLAY_SPR_2D_BMP_256);
    }else{
    videoSetModeSub(MODE_0_2D | DISPLAY_BG3_ACTIVE);
  }
}

void ShowLogHalt(void)
{
  videoSetModeSub_SetShowLog(True);
  while(1){
    swiWaitForVBlank();
  }
}

void glDefaultMemorySetting(void)
{
  videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D | DISPLAY_SPR_2D_BMP_256);
  videoSetModeSub(MODE_0_2D | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D | DISPLAY_SPR_2D_BMP_256);
  
  vramSetMainBanks(VRAM_A_MAIN_BG_0x6000000, VRAM_B_MAIN_SPRITE, VRAM_C_MAIN_BG_0x6020000, VRAM_D_SUB_SPRITE);
  vramSetBankH(VRAM_H_SUB_BG);
  vramSetBankI(VRAM_I_LCD);
}

int main(void)
{
  REG_IME=0;
  
  POWER_CR = POWER_ALL_2D;
  
  SetARM9_REG_WaitCR();
  
  videoSetModeSub_SetShowLog(True);
  
  glDefaultMemorySetting();
  
  {
    SUB_BG3_CR = BG_32x32 | BG_MAP_BASE(4) | BG_TILE_BASE(0) | BG_PRIORITY_0;
    BG_PALETTE_SUB[255] = RGB15(31,31,31);//by default font will be rendered with color 255
    
    //consoleInit() is a lot more flexible but this gets you up and running quick
    _consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(4), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);
    _consoleClear();
    _consolePrintSet(0,0);
  }
  
#ifdef EmulatorDebug
  _consolePrintf("#define EmulatorDebug\n\n");
#endif
  
  _consolePrintf("boot %s\n%s\n%s\n\n",ROMTITLE,ROMVERSION,ROMDATE);
  
//  mainloop();
  
  _consolePrintf("ARM7 Processing... ");
  
  int timeout=0x100000;
  
  while(IPC4->ready==0){
    timeout--;
    if(timeout==0){
      _consolePrintf("Timeout!!\n");
      IPC4->xscale=0;
      break;
    }
  }
  
  if((IPC4->xscale<128)||(IPC4->yscale<128)||(IPC4->xoffset<128)||(IPC4->yoffset<128)){
    _consolePrintf("Calibration error.\n");
    _consolePrintf("Please start from GBAMP.\n");
    _consolePrintf("(or other adapter/FlashCart)\n");
    
    }else{
    
    _consolePrintf("Calibrated.\n");
    _consolePrintf("\n");
    
    _consolePrintf("[TouchPadCalibration]\n");
    _consolePrintf("\n");
    _consolePrintf("enabled=1\n");
    _consolePrintf("xscale=%d\n",IPC4->xscale);
    _consolePrintf("yscale=%d\n",IPC4->yscale);
    _consolePrintf("xoffset=%d\n",IPC4->xoffset);
    _consolePrintf("yoffset=%d\n",IPC4->yoffset);
    
    _consolePrintf("\n");
    _consolePrintf("Please change your\n");
    _consolePrintf("shell/global.ini setting.\n");
  }
  
  ShowLogHalt();
}

