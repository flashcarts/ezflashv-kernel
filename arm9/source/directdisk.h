
#ifndef directdisk_h
#define directdisk_h

#include <NDS.h>

#include "gba_nds_fat.h"

#define DD_SectorSize (512)

enum EDDFile {EDDFile_resume=0,EDDFile_bookmrk0=1,EDDFile_bookmrk1=2,EDDFile_bookmrk2=3,EDDFile_bookmrk3=4,EDDFile_MaxCount=5};

extern char DDFilename_resume[];
extern char DDFilename_bookmrk0[];
extern char DDFilename_bookmrk1[];
extern char DDFilename_bookmrk2[];
extern char DDFilename_bookmrk3[];

enum EDDSaveType {EDDST_None,EDDST_FAT,EDDST_CART};

extern bool DD_isEnabled(void);
extern void DD_Init(EDDSaveType st);
extern EDDSaveType DD_GetSaveType(void);
extern bool DD_InitFile(EDDFile DDFile);
extern void DD_ReadFile(EDDFile DDFile,void *pbuf,u32 bufsize);
extern void DD_WriteFile(EDDFile DDFile,void *pbuf,u32 bufsize);

#endif

