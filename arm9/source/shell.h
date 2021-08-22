
#ifndef shell_h
#define shell_h

extern char FATShellPath[256+1];
extern char FATShellPluginPath[256+1];

extern u32 Shell_VolumeType;

extern void Shell_AutoDetect(void);
extern void Shell_AutoDetect_EXFS(void);

extern bool Shell_FindShellPath(void);

extern void Shell_ReadFile(const char *fn,void **pbuf,int *psize);

extern char **Shell_EnumMSP(void);
extern bool Shell_ReadHeadMSP(char *fn,void *buf,int size);
extern void Shell_ReadMSP(const char *fn,void **pbuf,int *psize);
extern void Shell_ReadMSPINI(const char *fn,char **pbuf,int *psize);
extern int Shell_OpenMSPBIN(const char *fn);
extern int Shell_OpenMSPData(const char *fn);

#endif

