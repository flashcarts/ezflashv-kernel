
#ifndef plug_fpk_h
#define plug_fpk_h

bool StartFPK(int FileHandle);
void FreeFPK(void);

#include "../unicode.h"

int FPK_GetInfoIndexCount(void);
bool FPK_GetInfoStrL(int idx,char *str,int len);
bool FPK_GetInfoStrW(int idx,UnicodeChar *str,int len);
bool FPK_GetInfoStrUTF8(int idx,char *str,int len);

#endif
