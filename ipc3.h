//////////////////////////////////////////////////////////////////////
//
// ipc.h -- Inter-processor communication
//
// version 0.1, February 14, 2005
//
//  Copyright (C) 2005 Michael Noland (joat) and Jason Rogers (dovoto)
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any
//  damages arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any
//  purpose, including commercial applications, and to alter it and
//  redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source
//     distribution.
//
// Changelog:
//   0.1: First version
//
//////////////////////////////////////////////////////////////////////

#ifndef NDS_IPC3_INCLUDE
#define NDS_IPC3_INCLUDE

//////////////////////////////////////////////////////////////////////

#include <nds.h>

//////////////////////////////////////////////////////////////////////

typedef u32 EstrpcmFormat;
#define SPF_PCMx1 1
#define SPF_PCMx2 2
#define SPF_PCMx4 3
#define SPF_GSM 4
#define SPF_MP2 5

typedef u32 EIPCREQ;
#define IR_NULL 0
#define IR_NextSoundData 1
#define IR_FileSys_fread 2
#define IR_FileSys_fread_flash 3

typedef u32 ER2YREQ;
#define R2YREQ_NULL 0
#define R2YREQ_Convert 1
#define R2YREQ_Transfer 2

typedef u32 EstrpcmControl;
#define strpcmControl_NOP (0)
#define strpcmControl_Play (1)
#define strpcmControl_Stop (2)

typedef u32 ELCDPC;
#define LCDPC_NOP (0)
#define LCDPC_OFF_BOTH (1)
#define LCDPC_ON_BOTTOM (2)
#define LCDPC_ON_TOP_LEDON (3)
#define LCDPC_ON_TOP_LEDBLINK (4)
#define LCDPC_ON_BOTH (5)
#define LCDPC_SOFT_POWEROFF (0xff)

typedef u32 ERESET;
#define RESET_NULL (0)
#define RESET_GBAMP (1)
#define RESET_GBAROM (2)
#define RESET_MENU_DSLink (3)
#define RESET_MENU_MPCF (4)
#define RESET_MENU_M3CF (5)
#define RESET_MENU_M3SD (6)
#define RESET_MENU_SCCF (7)
#define RESET_MENU_SCSD (8)
#define RESET_MENU_EZSD (9)

typedef u32 cbool;

typedef struct sTransferRegion3 {
  uint32 heartbeat;          // counts frames
  uint32 UserLanguage;       // from BIOS
 
//   int16 touchX,   touchY;   // TSC X, Y
   int16 touchXpx, touchYpx; // TSC X, Y pixel values
//   int16 touchZ1,  touchZ2;  // TSC x-panel measurements
  uint16 tdiode1,  tdiode2;  // TSC temperature diodes
  uint32 temperature;        // TSC computed temperature
 
  uint16 buttons;            // X, Y, /PENIRQ buttons
 
  union {
    uint8 curtime[8];        // current time response from RTC
 
    struct {
      u8 rtc_command;
      u8 rtc_year;           //add 2000 to get 4 digit year
      u8 rtc_month;          //1 to 12
      u8 rtc_day;            //1 to (days in month)
 
      u8 rtc_incr;
      u8 rtc_hours;          //0 to 11 for AM, 52 to 63 for PM
      u8 rtc_minutes;        //0 to 59
      u8 rtc_seconds;        //0 to 59
    };
  };
  bool curtimeFlag;
  
  uint16 battery;            // battery life ??  hopefully.  :)
  uint16 aux;                // i have no idea...
  
  EIPCREQ IR;
  int IR_filesize;
  int IR_readsize;
  void *IR_readbuf;
  int IR_readbufsize;
  cbool IR_flash;
  s64 IR_samples;
  
  ELCDPC LCDPowerControl;
  
  u32 ReqVsyncUpdate;
  
  EstrpcmControl strpcmControl;
  u32 strpcmFreq,strpcmSamples,strpcmChannels;
  u32 strpcmVolume16;
  u32 strpcmWriteRequest;
  EstrpcmFormat strpcmFormat;
  s16 *strpcmLBuf,*strpcmRBuf;
  
  cbool isNDSLite;
  u32 Brightness,DefaultBrightness; // 0=darkness 1=dark 2=light 3=lightness
  
  cbool WhenPanelClose;
  cbool RequestShotDown;
  
  ER2YREQ R2YREQ;
  u8 *R2Y_pYBuf;
  u8 *R2Y_pUBuf;
  u8 *R2Y_pVBuf;
  u16 *R2Y_pFrameBuf;
  u16 *R2Y_pVRAM;
  
  int PlugMP3_Ptr_rq_table;
  int PlugMP3_Ptr_D;
  
  ERESET RESET;
} TransferRegion3, * pTransferRegion3;

//////////////////////////////////////////////////////////////////////

#define IPC3 ((TransferRegion3 volatile *)(0x027FF000))

#endif

//////////////////////////////////////////////////////////////////////

