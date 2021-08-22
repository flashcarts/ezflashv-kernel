
#ifndef imgcalc_h
#define imgcalc_h

#include <NDS.h>

#include "rect.h"

enum EImageMode {EIM_Single,EIM_Double};

extern void imgcalc_SetImageMode(EImageMode EIM,int _VerticalPadding);
extern EImageMode imgcalc_GetImageMode(void);
extern int imgcalc_GetNextIndex(float CurrentRatio,int Vector);
extern float imgcalc_GetRatio(int Index);
extern bool imgcalc_GetKeyWait(int Index);
extern bool imgcalc_GetWideFlag(void);
extern int imgcalc_GetVerticalPadding(void);

#endif

