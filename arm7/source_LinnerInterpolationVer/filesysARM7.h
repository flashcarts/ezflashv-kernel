
#ifndef filesysARM7_h
#define filesysARM7_h

#include <NDS.h>

#ifdef __cplusplus
extern "C" {
#endif

int FileSys_GetFileSize(void);
int FileSys_fread(void *buf,int size);
int FileSys_fread_flash(void *buf,int size);

#ifdef __cplusplus
}
#endif

#endif

