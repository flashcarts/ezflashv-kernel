
#ifndef main_extforresume_h
#define main_extforresume_h

#include "dll.h"
#include "unicode.h"

extern TPluginBody *pPluginBody;

enum EExecMode {EM_None=1,EM_NDSROM,EM_FPK,EM_Text,EM_MSPImage,EM_MSPSound,EM_MP3Boot,EM_DPG,EM_GMENSF,EM_GMEGBS}; // ,EM_GMEVGM,EM_GMEGYM

extern u32 NDSLite_Brightness;
extern void NDSLite_SetBrightness(u32 bright);

extern EExecMode ExecMode;

#define forResume_PlayPathNameLen (128+1)
extern char forResume_PlayPathName[forResume_PlayPathNameLen];

#define forResume_PlayFileAliasNameLen (13+1)
extern char forResume_PlayFileAliasName[forResume_PlayFileAliasNameLen];

#define forResume_PlayFileUnicodeNameLen (128+1)
extern UnicodeChar forResume_PlayFileUnicodeName[forResume_PlayFileUnicodeNameLen];

extern void FS_ExecuteStop(void);
extern void FS_ChangePath(char *TargetPathName);
extern void FS_SetCursor(s32 idx);
extern void FS_StartFromIndex(s32 idx);

extern void Pic_SetSBar(float lastratio);
extern void Pic_SetRatio(float ratio);
extern float Pic_GetRatio(void);

extern u16 KEYS_CUR;

extern void ScanIPC3(bool useStack);

#endif

