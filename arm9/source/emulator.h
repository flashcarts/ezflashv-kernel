
#ifndef Emulator_h
#define Emulator_h

// for Dualis Release 10 2005-06-18
//#define EmulatorDebug

#ifdef EmulatorDebug

#define ExecuteShowDebug
//#define notuseMemDMA2
//#define notuseMemDMA3
#define notuseIPCKey
#define notuseSound
#define StartupIMFS
#define DisableStartupSound
#define DisableAutoDetect
#define WaitForConsolePrint
#define DPGTimeProfile
#define ShowPluginInfo

#else

//#define ExecuteShowDebug
//#define notuseMemDMA2
//#define notuseMemDMA3
//#define notuseIPCKey
//#define notuseSound
//#define StartupIMFS
//#define DisableStartupSound
//#define DisableAutoDetect
//#define WaitForConsolePrint
//#define DPGTimeProfile
//#define ShowPluginInfo

#endif

#endif

