
#ifndef resume_h
#define resume_h

#include <NDS.h>

extern void Resume_Init(void);
extern void Resume_Backup(bool AlreadyTop);
extern bool Resume_Restore(void);
extern bool Resume_isEnabled(void);

#endif

