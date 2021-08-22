
#include <stdio.h>
#include <stdlib.h>

#include <NDS.h>

#include "emulator.h"
#include "_console.h"
#include "_consoleWriteLog.h"

#include "imgcalc.h"

#include "glib/glib.h"
#include "memtool.h"

#include "_const.h"
#include "inifile.h"

#include "VRAMTool.h"

typedef struct {
  float ratio;
  bool keywait;
} TRatioTable;

static TRatioTable RatioTable[32];
static int RatioTableCount;
static EImageMode ImageMode;
static int VerticalPadding;

static void imgcalc_InitAdd(float ratio,bool keywait)
{
  if(RatioTableCount==32){
    _consolePrintf("RatioTable overflow.\n");
    ShowLogHalt();
  }
  
  RatioTable[RatioTableCount].ratio=ratio;
  RatioTable[RatioTableCount].keywait=keywait;
  RatioTableCount++;
}

void imgcalc_SetImageMode(EImageMode EIM,int _VerticalPadding)
{
  ImageMode=EIM;
  VerticalPadding=_VerticalPadding;
  
  RatioTableCount=0;
  
  switch(EIM){
    case EIM_Single: {
      for(int idx=0;idx<5;idx++){
        float base;
        switch(idx){
          case 0: base=0.25; break;
          case 1: base=0.5; break;
          case 2: base=1.0; break;
          case 3: base=2.0; break;
          case 4: base=4.0; break;
          default: base=0; break;
        }
        if(base!=0){
          imgcalc_InitAdd(base+((base/4)*0),true);
          imgcalc_InitAdd(base+((base/4)*1),false);
          imgcalc_InitAdd(base+((base/4)*2),false);
          imgcalc_InitAdd(base+((base/4)*3),false);
        }
      }
      imgcalc_InitAdd(8.0,true);
    } break;
    case EIM_Double: {
/*
      for(int idx=0;idx<5;idx++){
        float base;
        switch(idx){
          case 0: {
            base=0;
            imgcalc_InitAdd(0.75/2,true);
            imgcalc_InitAdd(0.875/2,false);
          } break;
          case 1: {
            base=0;
            imgcalc_InitAdd(0.5/1,true);
            imgcalc_InitAdd(0.75/1,false);
            imgcalc_InitAdd(0.875/1,false);
          } break;
          case 2: base=1.0; break;
          case 3: base=2.0; break;
          case 4: base=4.0; break;
          default: base=0; break;
        }
        if(base!=0){
          imgcalc_InitAdd(base+((base/4)*0),true);
          imgcalc_InitAdd(base+((base/4)*1),false);
          imgcalc_InitAdd(base+((base/4)*2),false);
          imgcalc_InitAdd(base+((base/4)*3),false);
        }
      }
      imgcalc_InitAdd(8.0,true);
*/
      imgcalc_InitAdd(0.5,true);
      imgcalc_InitAdd(1.0,true);
    } break;
    default: {
      _consolePrintf("undefine EImageMode error.\n");
      ShowLogHalt();
    } break;
  }
}

EImageMode imgcalc_GetImageMode(void)
{
  return(ImageMode);
}

int imgcalc_GetNextIndex(float CurrentRatio,int Vector)
{
  int CurrentIndex=-1;
  
  for(int idx=0;idx<RatioTableCount;idx++){
    if(CurrentRatio==RatioTable[idx].ratio) CurrentIndex=idx;
  }
  
  if(CurrentIndex==-1){
    _consolePrintf("Change ratio error:CurrentRatio=%f Vector=%d\n",CurrentRatio,Vector);
    ShowLogHalt();
  }
  
  CurrentIndex+=Vector;
  
  if(CurrentIndex<0) return(0);
  if(RatioTableCount<=CurrentIndex) return(RatioTableCount-1);
  return(CurrentIndex);
}

float imgcalc_GetRatio(int Index)
{
  return(RatioTable[Index].ratio);
}

bool imgcalc_GetKeyWait(int Index)
{
  return(RatioTable[Index].keywait);
}

bool imgcalc_GetWideFlag(void)
{
  switch(ImageMode){
    case EIM_Single: return(true); break;
    case EIM_Double: return(false); break;
  }
  
  return(false);
}

int imgcalc_GetVerticalPadding(void)
{
  return(VerticalPadding);
}

