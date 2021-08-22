
#ifndef inifile_h
#define inifile_h

#include "rect.h"

enum EClosedSholderButton {ECSB_Disabled=0,ECSB_Flexible=1,ECSB_AlwaysDisabled=2,ECSB_Enabled=3};
enum EScreenFlip {ESF_Normal=0,ESF_Flip=1,ESF_VFlip=2,WFS_HFlip=3};
enum EiniSystemWhenPanelClose {EISWPC_BacklightOff=0,EISWPC_DSPowerOff=1,EISWPC_PlayShutdownSound=2};

typedef struct {
  char StartPath[256];
  bool FileSelectSubScreen;
  EClosedSholderButton ClosedSholderButton;
  u32 MusicNext;
  int SoundVolume;
  bool FullScreenOverlaySubScreen;
  u32 FileMaxCount;
  u32 NDSLiteDefaultBrightness;
  EiniSystemWhenPanelClose WhenPanelClose;
  EScreenFlip TopScreenFlip;
  bool ResumeUsingWhileMusicPlaying;
  bool NDSROMBootFunctionWithEZ4SD;
  bool PrivateFunction;
} TiniSystem;

typedef struct {
    u32 ROM1stAccessCycleControl;
    u32 ROM2stAccessCycleControl;
} TiniForSuperCard;

typedef struct {
  bool Attribute_Archive;
  bool Attribute_Hidden;
  bool Attribute_System;
  bool Attribute_Readonly;
  bool Path_Shell;
  bool File_Thumbnail;
} TiniHiddenItem;

typedef struct {
  u32 DelayCount;
  u32 RateCount;
} TiniKeyRepeat;

typedef struct {
  int WhenStandby;
  int WhenPicture;
  int WhenText;
  int WhenSound;
} TiniBacklightTimeout;

#define MusicNext_Stop (0)
#define MusicNext_Repeat (1)
#define MusicNext_NormalLoop (2)
#define MusicNext_NormalPOff (3)
#define MusicNext_ShuffleLoop (4)
#define MusicNext_ShufflePOff (5)
#define MusicNext_PowerOff (6)
#define MusicNext_Count (7)

enum EiniBootStartupSound {EIBSS_Off=0,EIBSS_Startupmp3=1,EIBSS_AutoPlay=2};

typedef struct {
  bool hiddenAboutWindow;
  bool hiddenHelpWindow;
  bool hiddenDateTimeWindow;
  u32 StartupSound;
} TiniBoot;

enum EiniThumbnailMode {EITM_Off=0,EITM_ThumbSelector=1,EITM_FullscreenPreview=2,EITM_Both=3};

typedef struct {
  EiniThumbnailMode Mode;
  u32 SelectItemAlpha;
  u32 UnselectItemAlpha;
} TiniThumbnail;

enum EiniTextPluginScrollType {EITPST_Normal=0,EITPST_FullPage=1};
enum EiniTextPluginSelectDisplay {EITPSD_Bottom=0,EITPSD_Top=1};

typedef struct {
  u32 SpacePixel;
  EiniTextPluginScrollType ScrollType;
  EiniTextPluginSelectDisplay SelectDisplay;
} TiniTextPlugin;

typedef struct {
  u32 MaxVoiceCount;
  bool Flag_Interpolate;
  bool Flag_Surround;
  bool Flag_HQMixer;
  u32 Channels,Frequency;
} TiniMikModPlugin;

typedef struct {
  u32 DefaultSongLength;
} TiniSPCPlugin;

typedef struct {
  bool CrashMeProtection;
} TiniNDSROMPlugin;

enum EDPGDithering {EDPGD_NoCnt=0,EDPGD_HalfCnt=1,EDPGD_QuatCnt=2,EDPGD_HeavyCnt=3};

typedef struct {
  EDPGDithering Dithering;
  bool GOPSkip;
  bool AdaptiveDelayControl;
  bool BlockGSMAudio;
  u32 DelayFrames;
} TiniDPGPlugin;

#include "imgcalc.h"

typedef struct {
  u32 GUITimeOutSec;
  EImageMode ImageMode;
  bool DrawFileInfo;
  int VerticalPadding;
  u32 Interpolation;
} TiniImagePlugin;

typedef struct {
  bool Enabled;
  bool RTC24Hour;
  char FormatDate[32+1];
} TiniClockPlugin;

typedef struct {
  s32 x,y,w,h;
  bool DisabledCloseButton;
} TiniOverrideWindowRectBody;

typedef struct {
  TiniOverrideWindowRectBody About,Help,PlayControl,FileSelect,DateTime,PicView,TextView,Progress,SetNext,FileInfo;
} TiniOverrideWindowRect;

typedef struct {
  bool EnabledWrite;
  bool SC_EnabledDRAM;
  u32 EZ4_PSRAMSizeMByte;
} TiniAdapterConfig;

typedef struct {
  TiniSystem System;
  TiniForSuperCard ForSuperCard;
  TiniHiddenItem HiddenItem;
  TiniKeyRepeat KeyRepeat;
  TiniBacklightTimeout BacklightTimeout;
  TiniBoot Boot;
  TiniThumbnail Thumbnail;
  TiniTextPlugin TextPlugin;
  TiniNDSROMPlugin NDSROMPlugin;
  TiniDPGPlugin DPGPlugin;
  TiniImagePlugin ImagePlugin;
  TiniClockPlugin ClockPlugin;
  TiniOverrideWindowRect OverrideWindowRect;
  TiniAdapterConfig AdapterConfig;
} TGlobalINI;

extern TGlobalINI GlobalINI;

extern void InitINI(void);
extern void LoadINI(char *inifn);

#endif


