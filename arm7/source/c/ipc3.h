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

enum EstrpcmFormat {SPF_PCMx1=1,SPF_PCMx2,SPF_PCMx4,SPF_GSM,SPF_MP2};

enum EIPCREQ {IR_NULL=0,IR_NextSoundData,IR_FileSys_fread,IR_FileSys_fread_flash};

enum ER2YREQ {R2YREQ_NULL=0,R2YREQ_Convert,R2YREQ_Transfer};

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
  bool IR_flash;
  s64 IR_samples;
  
  u32 LCDPowerControl;
  
  u32 ReqVsyncUpdate;
  
  u32 strpcmControl;
  u32 strpcmFreq,strpcmSamples,strpcmChannels;
  u32 strpcmVolume16;
  u32 strpcmWriteRequest;
  EstrpcmFormat strpcmFormat;
  s16 *strpcmLBuf,*strpcmRBuf;
  
  bool isNDSLite;
  u32 Brightness,DefaultBrightness; // 0=darkness 1=dark 2=light 3=lightness
  
  bool WhenPanelClose;
  bool RequestShotDown;
  
  ER2YREQ R2YREQ;
  u8 *R2Y_pYBuf;
  u8 *R2Y_pUBuf;
  u8 *R2Y_pVBuf;
  u16 *R2Y_pFrameBuf;
  u16 *R2Y_pVRAM;
} TransferRegion3, * pTransferRegion3;

//////////////////////////////////////////////////////////////////////

#define IPC3 ((TransferRegion3 volatile *)(0x027FF000))

#define strpcmControl_NOP (0)
#define strpcmControl_Play (1)
#define strpcmControl_Stop (2)

#define LCDPC_NOP (0)
#define LCDPC_OFF_BOTH (1)
#define LCDPC_ON_BOTTOM (2)
#define LCDPC_ON_TOP_LEDON (3)
#define LCDPC_ON_TOP_LEDBLINK (4)
#define LCDPC_ON_BOTH (5)
#define LCDPC_SOFT_POWEROFF (0xff)

#endif

//////////////////////////////////////////////////////////////////////

