
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <NDS.h>

#include "emulator.h"
#include "_const.h"
#include "_console.h"
#include "filesys.h"
#include "memtool.h"
#include "mwin.h"
#include "shell.h"
#include "extmem.h"
#include "formdt.h"

#include "dll.h"

extern char *Plugin_GetINIData(void);
extern int Plugin_GetINISize(void);
extern void *Plugin_GetBINData(void);
extern int Plugin_GetBINSize(void);
extern int Plugin_GetBINFileHandle(void);
extern int Plugin_msp_fopen(const char *fn);
extern bool Plugin_msp_fclose(int fh);

#ifdef ShowDebugMsg
#else
static void _consolePrint_dummy(const char* s)
{
}
static void _consolePrintf_dummy(const char* format, ...)
{
}
#endif

static inline const TPlugin_StdLib *Plugin_GetStdLib(void)
{
  static TPlugin_StdLib sPlugin_StdLib={
#ifdef ShowDebugMsg
    _consolePrint,
    _consolePrintf,
#else
    _consolePrint_dummy,
    _consolePrintf_dummy,
#endif
    _consolePrintSet,
    ShowLogHalt,
    MWin_ProgressShow,
    MWin_ProgressSetPos,
    MWin_ProgressHide,
    Plugin_GetINIData,
    Plugin_GetINISize,
    Plugin_GetBINData,
    Plugin_GetBINSize,
    
    DC_FlushRangeOverrun,
    MemCopy8CPU,
    MemCopy16CPU,
    MemCopy32CPU,
    MemSet16CPU,
    MemSet32CPU,
    MemCopy16DMA3,
    MemCopy32DMA3,
    MemSet16DMA3,
    MemSet32DMA3,
    MemSet8DMA3,
    MemCopy16DMA2,
    MemSet16DMA2,
    MemCopy32swi256bit,
    safemalloc,
    safefree,
    
    calloc,
    malloc,
    free,
    realloc,
    
    rand,
    
    FileSys_fread,
    FileSys_fseek,
    FileSys_ftell,
    
    sprintf,
    snprintf,
    
    memchr,
    memcmp,
    memcpy,
    memmove,
    memset,
    
    abs,
    labs,
    llabs,
    fabs,
    fabsf,
    
    atof,
    atoi,
    atol,
    atoll,
    
    strcat,
    strchr,
    strcmp,
    strcoll,
    strcpy,
    strcspn,
    strdup,
    strlen,
    strncat,
    strncmp,
    strncpy,
    strpbrk,
    strrchr,
    strsep,
    strspn,
    strstr,
    strtok,
    strxfrm,
    
    Plugin_GetBINFileHandle,
    Plugin_msp_fopen,
    Plugin_msp_fclose,
    
    extmem_SetCount,
    extmem_Exists,
    extmem_Alloc,
    extmem_Write,
    extmem_Read,
    
    formdt_FormatDate
  };
  
  return(&sPlugin_StdLib);
}

bool DLL_LoadLibrary(TPluginBody *pPB,void *pbin,int binsize)
{
  memset(pPB,0,sizeof(TPluginBody));
  
  TPluginHeader *pPH=&pPB->PluginHeader;
  
  memmove(pPH,pbin,sizeof(TPluginHeader));
  
#ifdef ShowPluginInfo
  _consolePrintf("MoonShellPlugin Header\n");
  _consolePrintf("ID=%x ver%d.%d\n",pPH->ID,pPH->VersionHigh,pPH->VersionLow);
  {
    char *pts;
    switch((EPluginType)(pPH->PluginType)){
      case EPT_None: pts="NULL"; break;
      case EPT_Image: pts="Image"; break;
      case EPT_Sound: pts="Sound"; break;
      case EPT_Clock: pts="Clock"; break;
      default: pts="undefined"; break;
    }
    _consolePrintf("PluginType=%x %s\n",pPH->PluginType,pts);
  }
  _consolePrintf("Data 0x%x-0x%x\n",pPH->DataStart,pPH->DataEnd);
  _consolePrintf("got  0x%x-0x%x\n",pPH->gotStart,pPH->gotEnd);
  _consolePrintf("bss  0x%x-0x%x\n",pPH->bssStart,pPH->bssEnd);
  {
    char *str=pPH->info;
    _consolePrintf("Name=%s\n",str);
    while(*str!=0){
      *str++;
    }
    *str++;
    _consolePrintf("Author=%s\n",str);
  }
  _consolePrintf("\n",0);
#endif
  
  pPB->DataSize=binsize;
  pPB->pData=pbin;
  
#ifdef ShowPluginInfo
  _consolePrintf("Plugin LoadAddress=0x%08x\n",(u32)pPB->pData);
#endif
  
  pPB->bssSize=pPH->bssEnd-pPH->bssStart;
  pPB->pbss=malloc(pPB->bssSize);
  
  if(pPB->pbss==NULL){
    _consolePrintf("LoadLibrary:bss malloc error.\n");
    DLL_FreeLibrary(pPB,false);
    return(false);
  }
  
  memset(pPB->pbss,0,pPB->bssSize);
  
#ifdef ShowPluginInfo
  u32 plug_got_bssbaseofs=pPH->bssStart;
#endif
  u32 plug_got_ofs=pPH->gotStart-pPH->DataStart;
  u32 plug_got_size=pPH->gotEnd-pPH->gotStart;
  
#ifdef ShowPluginInfo
  _consolePrintf("allocbss 0x%08x (0x%xbyte)\n",(u32)pPB->pbss,pPB->bssSize);
  _consolePrintf("got_bssbaseofs=0x%x\n",plug_got_bssbaseofs);
  _consolePrintf("got_ofs=0x%x got_size=0x%x\n",plug_got_ofs,plug_got_size);
#endif
  
  {
    u32 *padr=(u32*)pPB->pData;
    for(u32 idx=64/4;idx<plug_got_ofs/4;idx++){
      u32 adr=padr[idx];
      if((pPH->bssStart<=adr)&&(adr<pPH->bssEnd)){
#ifdef ShowPluginInfo
//        _consolePrintf("b%x:%x ",idx*4,adr);
#endif
        padr[idx]=(u32)pPB->pbss+(adr-pPH->bssStart);
        }else{
        if((pPH->DataStart<=adr)&&(adr<pPH->DataEnd)){
#ifdef ShowPluginInfo
//          _consolePrintf("d%x:%x ",idx*4,adr);
#endif
          padr[idx]=(u32)pPB->pData+(adr-pPH->DataStart);
        }
      }
    }
  }
  
  {
    u32 *padr=(u32*)pPB->pData;
    for(u32 idx=(plug_got_ofs+plug_got_size)/4;idx<((u32)pPH->DataEnd-(u32)pPH->DataStart)/4;idx++){
      u32 adr=padr[idx];
      if((pPH->bssStart<=adr)&&(adr<pPH->bssEnd)){
#ifdef ShowPluginInfo
//        _consolePrintf("b%x:%x ",idx*4,adr);
#endif
        padr[idx]=(u32)pPB->pbss+(adr-pPH->bssStart);
        }else{
        if((pPH->DataStart<=adr)&&(adr<pPH->DataEnd)){
#ifdef ShowPluginInfo
//          _consolePrintf("d%x:%x ",idx*4,adr);
#endif
          padr[idx]=(u32)pPB->pData+(adr-pPH->DataStart);
        }
      }
    }
  }
  
  {
    u32 *padr=(u32*)((u32)pPB->pData+plug_got_ofs);
    
    for(u32 idx=0;idx<plug_got_size/4;idx++){
      u32 adr=padr[idx];
      if((pPH->bssStart<=adr)&&(adr<pPH->bssEnd)){
#ifdef ShowPluginInfo
//        _consolePrintf("b%x:%x ",idx*4,adr);
#endif
        padr[idx]=(u32)pPB->pbss+(adr-pPH->bssStart);
        }else{
        if((pPH->DataStart<=adr)&&(adr<pPH->DataEnd)){
#ifdef ShowPluginInfo
//          _consolePrintf("d%x:%x ",idx*4,adr);
#endif
          padr[idx]=(u32)pPB->pData+(adr-pPH->DataStart);
        }
      }
    }
    
  }
  
  {
    u32 src;
    u32 *pdst;
    
    src=pPH->LoadLibrary;
    if(src==0){
      _consolePrintf("LoadLibrary:BootStrap function is NULL.\n");
      DLL_FreeLibrary(pPB,false);
      return(false);
    }
    pdst=(u32*)&pPB->LoadLibrary;
    *pdst=(u32)pPB->pData+(src-pPH->DataStart);
    
    src=pPH->FreeLibrary;
    if(src==0){
      _consolePrintf("LoadLibrary:BootStrap function is NULL.\n");
      DLL_FreeLibrary(pPB,false);
      return(false);
    }
    pdst=(u32*)&pPB->FreeLibrary;
    *pdst=(u32)pPB->pData+(src-pPH->DataStart);
    
    src=pPH->QueryInterfaceLibrary;
    if(src==0){
      _consolePrintf("LoadLibrary:BootStrap function is NULL.\n");
      DLL_FreeLibrary(pPB,false);
      return(false);
    }
    pdst=(u32*)&pPB->QueryInterfaceLibrary;
    *pdst=(u32)pPB->pData+(src-pPH->DataStart);
  }
  
  _consolePrintf("0x%08x LoadLibrary\n",(u32)pPB->LoadLibrary);
  _consolePrintf("0x%08x FreeLibrary\n",(u32)pPB->FreeLibrary);
  _consolePrintf("0x%08x QueryInterfaceLib.\n",(u32)pPB->QueryInterfaceLibrary);
  
  if(pPB->LoadLibrary==NULL){
    _consolePrintf("LoadLibrary:LoadLibrary() is NULL.\n");
    DLL_FreeLibrary(pPB,false);
    return(false);
  }
  
  bool res=pPB->LoadLibrary(Plugin_GetStdLib(),(u32)pPB->pData);
  
  if(res==false){
    _consolePrintf("LoadLibrary:LoadLibrary() false.\n");
    DLL_FreeLibrary(pPB,false);
    return(false);
  }
  
  pPB->pIL=NULL;
  pPB->pSL=NULL;
  pPB->pCL=NULL;
  
  switch((EPluginType)(pPH->PluginType)){
    case EPT_None: {
#ifdef ShowPluginInfo
      _consolePrintf("LoadLibrary:PluginType == None.\n");
#endif
      DLL_FreeLibrary(pPB,false);
      return(false);
    } break;
    case EPT_Image: {
      pPB->pIL=(TPlugin_ImageLib*)pPB->QueryInterfaceLibrary();
#ifdef ShowPluginInfo
      _consolePrintf("ImageInterfacePtr 0x%08x\n",(u32)pPB->pIL);
#endif
    } break;
    case EPT_Sound: {
      pPB->pSL=(TPlugin_SoundLib*)pPB->QueryInterfaceLibrary();
#ifdef ShowPluginInfo
      _consolePrintf("SoundInterfacePtr 0x%08x\n",(u32)pPB->pSL);
#endif
    } break;
    case EPT_Clock: {
      pPB->pCL=(TPlugin_ClockLib*)pPB->QueryInterfaceLibrary();
#ifdef ShowPluginInfo
      _consolePrintf("ClockInterfacePtr 0x%08x\n",(u32)pPB->pCL);
#endif
    } break;
  }
  
  if((pPB->pIL==NULL)&&(pPB->pSL==NULL)&&(pPB->pCL==NULL)){
    _consolePrintf("LoadLibrary:not found function list error.\n");
    DLL_FreeLibrary(pPB,false);
    return(false);
  }
  
#ifdef ShowPluginInfo
  _consolePrintf("LoadLibrary:Initialized.\n");
#endif
  
  return(true);
}

void DLL_FreeLibrary(TPluginBody *pPB,bool callfree)
{
  if(callfree==true){
    if(pPB!=NULL){
      if(pPB->FreeLibrary!=NULL) pPB->FreeLibrary();
    }
  }
  
  if(pPB->pData!=NULL){
    free(pPB->pData); pPB->pData=NULL;
  }
  if(pPB->pbss!=NULL){
    free(pPB->pbss); pPB->pbss=NULL;
  }
  
//  memset(pPB,0,sizeof(TPluginBody));
  
#ifdef ShowPluginInfo
  _consolePrintf("FreeLibrary:Destroied.\n");
#endif
}

// ------------------------------------------------------------

typedef struct {
  u32 ext;
  EPluginType PluginType;
  char fn[PluginFilenameMax];
} TDLLList;

static int DLLListCount;
TDLLList *DLLList=NULL;

void DLLList_Regist(char *fn)
{
  TPluginHeader PH;
  
  if(Shell_ReadHeadMSP(fn,&PH,sizeof(TPluginHeader))==false){
    _consolePrintf("%s file open error.\n",fn);
    return;
  }
  
  if(PH.ID!=0x0050534d){
    _consolePrintf("%s is illigal format.\n",fn);
    return;
  }
  if(0x04<PH.VersionHigh){
    _consolePrintf("%s check version error ver%d.%d\n",fn,PH.VersionHigh,PH.VersionLow);
    return;
  }
  
  switch((EPluginType)PH.PluginType){
    case EPT_None: {
      _consolePrintf("%s Plugin type is EPT_None(0)\n",fn);
    } break;
    case EPT_Image: case EPT_Sound: {
      for(int idx=0;idx<4;idx++){
        if(PH.ext[idx]!=0){
          DLLList[DLLListCount].ext=PH.ext[idx];
          DLLList[DLLListCount].PluginType=(EPluginType)PH.PluginType;
          strncpy(DLLList[DLLListCount].fn,fn,PluginFilenameMax);
#ifdef ShowPluginInfo
          _consolePrintf("regist:%2d .%s %s\n",DLLListCount,(char*)&DLLList[DLLListCount].ext,DLLList[DLLListCount].fn);
#endif
          DLLListCount++;
        }
      }
    } break;
    case EPT_Clock: {
      DLLList[DLLListCount].ext=0;
      DLLList[DLLListCount].PluginType=(EPluginType)PH.PluginType;
      strncpy(DLLList[DLLListCount].fn,fn,PluginFilenameMax);
#ifdef ShowPluginInfo
      _consolePrintf("regist(clk):%2d %s\n",DLLListCount,DLLList[DLLListCount].fn);
#endif
      DLLListCount++;
    }
  }
}

void DLLList_Init(void)
{
  DLLListCount=0;
  
  char **fnl=Shell_EnumMSP();
  
  if(fnl==NULL){
    _consolePrintf("not found plugin.\n");
    return;
  }
  if(fnl[0]==NULL){
    _consolePrintf("not found plugin.\n");
    return;
  }
  
  if(DLLList!=NULL){
    safefree(DLLList); DLLList=NULL;
  }
  
  {
    int cnt=0;
    while(fnl[cnt]!=NULL){
      cnt++;
    }
    if(cnt!=0){
      DLLList=(TDLLList*)safemalloc(cnt*4*sizeof(TDLLList));
    }
  }
  
  _consolePrintf("Plugin Read headers.\n");
  
  int idx=0;
  while(fnl[idx]!=NULL){
    DLLList_Regist(fnl[idx]);
    free(fnl[idx]); fnl[idx]=NULL;
    idx++;
  }
  
  free(fnl); fnl=NULL;
  
  _consolePrintf("Plugin Registed.\n");
}

EPluginType DLLList_GetPluginFilename(char *extstr,char *resfn)
{
  resfn[0]=0;
  
  if((extstr==NULL)||(extstr[0]==0)) return(EPT_None);
  
  u32 ext=0;
  
  u32 c;
  
  c=(u32)extstr[1];
  if(c!=0){
    if((0x41<=c)&&(c<0x5a)) c+=0x20;
    ext|=c << 0;
    c=(u32)extstr[2];
    if(c!=0){
      if((0x41<=c)&&(c<0x5a)) c+=0x20;
      ext|=c << 8;
      c=(u32)extstr[3];
      if(c!=0){
        if((0x41<=c)&&(c<0x5a)) c+=0x20;
        ext|=c << 16;
        c=(u32)extstr[4];
        if(c!=0){
          if((0x41<=c)&&(c<0x5a)) c+=0x20;
          ext|=c << 24;
        }
      }
    }
  }
  
  for(int idx=0;idx<DLLListCount;idx++){
    if(DLLList[idx].ext==ext){
      strncpy(resfn,DLLList[idx].fn,PluginFilenameMax);
      return(DLLList[idx].PluginType);
    }
  }
  
  return(EPT_None);
}

EPluginType DLLList_GetClockPluginFilename(int idx,char *resfn)
{
  resfn[0]=0;
  
  for(int findidx=0;findidx<DLLListCount;findidx++){
    if(DLLList[findidx].PluginType==EPT_Clock){
      if(idx==0){
        strncpy(resfn,DLLList[findidx].fn,PluginFilenameMax);
        return(DLLList[findidx].PluginType);
      }
      idx--;
    }
  }
  
  for(int findidx=0;findidx<DLLListCount;findidx++){
    if(DLLList[findidx].PluginType==EPT_Clock){
      strncpy(resfn,DLLList[findidx].fn,PluginFilenameMax);
      return(DLLList[findidx].PluginType);
    }
  }
  
  return(EPT_None);
}

// ----------------------------------

static TPluginBody *pCurrentPluginBody=NULL;

TPluginBody* DLLList_LoadPlugin(char *fn)
{
  void *buf;
  int size;
  
  Shell_ReadMSP(fn,&buf,&size);
  
  if((buf==NULL)||(size==0)){
    _consolePrintf("%s file read error.\n",fn);
    return(NULL);
  }
  
  TPluginBody *pPB=(TPluginBody*)safemalloc(sizeof(TPluginBody));
  
  if(pPB==NULL){
    _consolePrintf("Memory overflow.\n");
    return(NULL);
  }
  
  extmem_Init();
  
  if(DLL_LoadLibrary(pPB,buf,size)==false){
    extmem_Free();
    safefree(pPB); pPB=NULL;
    return(NULL);
  }
  
  pPB->INIData=NULL;
  pPB->INISize=0;
  pPB->BINFileHandle=0;
  pPB->BINData=NULL;
  pPB->BINSize=0;
  
  Shell_ReadMSPINI(fn,(char**)&pPB->INIData,&pPB->INISize);
#ifdef ShowPluginInfo
  _consolePrintf("LoadINI 0x%x(%d)\n",(u32)pPB->INIData,pPB->INISize);
  _consolePrintf("%s\n",pPB->INIData);
#endif
  
  pPB->BINFileHandle=Shell_OpenMSPBIN(fn);
  if(pPB->BINFileHandle==0){
    pPB->BINData=NULL;
    pPB->BINSize=0;
    }else{
    pPB->BINData=NULL;
    FileSys_fseek(pPB->BINFileHandle,0,SEEK_END);
    pPB->BINSize=FileSys_ftell(pPB->BINFileHandle);
    FileSys_fseek(pPB->BINFileHandle,0,SEEK_SET);
  }
#ifdef ShowPluginInfo
  _consolePrintf("LoadBIN FileHandle=%d size=%d\n",pPB->BINFileHandle,pPB->BINSize);
#endif
  
  pCurrentPluginBody=pPB;
  
  return(pPB);
}

void DLLList_FreePlugin(TPluginBody *pPB)
{
  pCurrentPluginBody=NULL;
  
  if(pPB==NULL) return;
  
  if(pPB->pIL!=NULL) pPB->pIL->Free();
  if(pPB->pSL!=NULL) pPB->pSL->Free();
  if(pPB->pCL!=NULL) pPB->pCL->Free();
  
  DLL_FreeLibrary(pPB,true);
  
  if(pPB->INIData!=NULL){
    safefree(pPB->INIData); pPB->INIData=NULL;
    pPB->INISize=0;
  }
  
  if(pPB->BINFileHandle!=0){
    FileSys_fclose(pPB->BINFileHandle);
    pPB->BINFileHandle=0;
  }
  
  if(pPB->BINData!=NULL){
    safefree(pPB->BINData); pPB->BINData=NULL;
    pPB->BINSize=0;
  }
  
  extmem_Free();
}

char *Plugin_GetINIData(void)
{
  TPluginBody *pPB=pCurrentPluginBody;
  
  return(pPB->INIData);
}

int Plugin_GetINISize(void)
{
  TPluginBody *pPB=pCurrentPluginBody;
  
  return(pPB->INISize);
}

void *Plugin_GetBINData(void)
{
  TPluginBody *pPB=pCurrentPluginBody;
  
  if(pPB->BINFileHandle==0) return(0);
  if(pPB->BINSize==0) return(0);
  
  if(pPB->BINData==NULL){
    pPB->BINData=safemalloc(pPB->BINSize);
    if(pPB->BINData!=NULL){
      FileSys_fread(pPB->BINData,1,pPB->BINSize,pPB->BINFileHandle);
    }
  }
  
  return(pPB->BINData);
}

int Plugin_GetBINSize(void)
{
  TPluginBody *pPB=pCurrentPluginBody;
  
  return(pPB->BINSize);
}

int Plugin_GetBINFileHandle(void)
{
  TPluginBody *pPB=pCurrentPluginBody;
  
  if(pPB->BINFileHandle==0) return(0);
  if(pPB->BINSize==0) return(0);
  
  return(pPB->BINFileHandle);
}

int Plugin_msp_fopen(const char *fn)
{
  return(Shell_OpenMSPData(fn));
}

bool Plugin_msp_fclose(int fh)
{
  return(FileSys_fclose(fh));
}

