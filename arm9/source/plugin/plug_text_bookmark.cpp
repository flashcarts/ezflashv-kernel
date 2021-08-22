
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "../_const.h"
#include "../_console.h"

#include "../memtool.h"

#include "plug_text_bookmark.h"

#include "../zlibhelp.h"

#include "gba_nds_fat.h"
#include "disc_io.h"
#include "mediatype.h"
#include "../filesys.h"
#include "../directdisk.h"

#include "../mwin.h"

#include "../main_extforresume.h"

#include "../strpcm.h"

#include "../dll.h"

#define SectorCount (4)
#define AreaSize (DD_SectorSize*SectorCount)

static bool BookmarkEnabled;

extern LPIO_INTERFACE active_interface;

#define BookmarkMax (128)

u32 BookmarkCount;
TBookmark *pBookmark;

bool exists(char *pfn)
{
  FAT_FILE *fp=FAT_fopen(pfn,"r");
  if(fp==NULL){
    _consolePrintf("not found %s.\n",pfn);
    return(false);
    }else{
    FAT_fclose(fp);
    return(true);
  }
}

void Bookmark_Init(void)
{
  BookmarkEnabled=false;
  if(DD_isEnabled()==false) return;
  
  switch(DD_GetSaveType()){
    case EDDST_None: return; break;
    case EDDST_FAT: {
      u32 fcnt=0;
      if(exists(DDFilename_bookmrk0)==true) fcnt++;
      if(exists(DDFilename_bookmrk1)==true) fcnt++;
      if(exists(DDFilename_bookmrk2)==true) fcnt++;
      if(exists(DDFilename_bookmrk3)==true) fcnt++;
      if(fcnt==0) return;
      if(fcnt!=SectorCount){
        _consolePrintf("Critical error!! CPU HALT for safety.\n");
        ShowLogHalt();
        while(1);
      }
    } break;
    case EDDST_CART: {
    } break;
  }
  
  _consolePrintf("\n");
  _consolePrintf("Bookmark initialize.\n");
  
  if(DD_InitFile(EDDFile_bookmrk0)==false){
    _consolePrintf("\n");
    _consolePrintf("Critical error!! CPU HALT for safety.\n");
    ShowLogHalt();
    while(1);
  }
  if(DD_InitFile(EDDFile_bookmrk1)==false){
    _consolePrintf("\n");
    _consolePrintf("Critical error!! CPU HALT for safety.\n");
    ShowLogHalt();
    while(1);
  }
  if(DD_InitFile(EDDFile_bookmrk2)==false){
    _consolePrintf("\n");
    _consolePrintf("Critical error!! CPU HALT for safety.\n");
    ShowLogHalt();
    while(1);
  }
  if(DD_InitFile(EDDFile_bookmrk3)==false){
    _consolePrintf("\n");
    _consolePrintf("Critical error!! CPU HALT for safety.\n");
    ShowLogHalt();
    while(1);
  }
  
  _consolePrintf("Bookmark initialized.\n");
  
  BookmarkCount=0;
  pBookmark=NULL;
  
  BookmarkEnabled=true;
}

static void LoadFromSector(void)
{
  u8 *pfbuf=(u8*)safemalloc(AreaSize);
  if(pfbuf==NULL){
    BookmarkCount=0;
    return;
  }
  
  DD_ReadFile(EDDFile_bookmrk0,&pfbuf[0*DD_SectorSize],DD_SectorSize);
  DD_ReadFile(EDDFile_bookmrk1,&pfbuf[1*DD_SectorSize],DD_SectorSize);
  DD_ReadFile(EDDFile_bookmrk2,&pfbuf[2*DD_SectorSize],DD_SectorSize);
  DD_ReadFile(EDDFile_bookmrk3,&pfbuf[3*DD_SectorSize],DD_SectorSize);
  
  BookmarkCount=*(u32*)&pfbuf[0];
  
  if(BookmarkMax<BookmarkCount){
    _consolePrintf("BookmarkReadError. BookmarkCount=%d.\n",BookmarkCount);
    BookmarkCount=0;
  }
  
  if(BookmarkCount!=0){
    TZLIBData zs;
    zs.SrcSize=*(u32*)&pfbuf[4];
    zs.pSrcBuf=(u8*)&pfbuf[8];
    zs.DstSize=sizeof(TBookmark)*BookmarkCount;
    zs.pDstBuf=(u8*)pBookmark;
    
    if(zlibdecompress(&zs)==false){
      BookmarkCount=0;
      _consolePrintf("Bookmark DecompressError.\n");
    }
  }
  
  safefree(pfbuf); pfbuf=NULL;
}

static void DeleteItem(u32 idx)
{
  for(u32 delidx=idx+1;idx<BookmarkCount;idx++){
    pBookmark[delidx-1]=pBookmark[delidx];
  }
  BookmarkCount--;
}

static void DeleteOldestItem(void)
{
  u32 lastidx=(u32)-1;
  
  u32 dt=(u32)-1;
  for(u32 idx=0;idx<BookmarkCount;idx++){
    TBookmark *pThis=&pBookmark[idx];
    if(pThis->Resume.DateTime<dt){
      dt=pThis->Resume.DateTime;
      lastidx=idx;
    }
  }
  
  if(lastidx==(u32)-1){
    _consolePrintf("error DeleteOldestItem. can not detect oldest.\n");
    ShowLogHalt();
    while(1);
  }
  
  DeleteItem(lastidx);
}

static void SaveToSector(void)
{
  u8 *pfbuf=(u8*)safemalloc(AreaSize);
  if(pfbuf==NULL){
    BookmarkCount=0;
    return;
  }
  
  MemSet32CPU(0,pfbuf,AreaSize);
  
  {
    u32 *pu=(u32*)&pfbuf[0];
    *pu=BookmarkCount;
  }
  
  if(BookmarkCount!=0){
//    MemCopy8CPU(pBookmark,&pfbuf[8],512-8);
    while(1){
      TZLIBData zs;
      zs.SrcSize=sizeof(TBookmark)*BookmarkCount;
      zs.pSrcBuf=(u8*)pBookmark;
      zs.DstSize=AreaSize-8;
      zs.pDstBuf=NULL;
      
      if(zlibcompress(&zs,zs.DstSize)==true){
        if(zs.pDstBuf!=NULL){
          MemCopy8CPU(zs.pDstBuf,&pfbuf[8],zs.DstSize);
          safefree(zs.pDstBuf); zs.pDstBuf=NULL;
        }
        u32 *pu=(u32*)&pfbuf[4];
        *pu=zs.DstSize;
        _consolePrintf("%d->%dbyte compressed.\n",zs.SrcSize,zs.DstSize);
        break;
      }
      
      DeleteOldestItem();
    }
  }
  
  DD_WriteFile(EDDFile_bookmrk0,&pfbuf[0*DD_SectorSize],DD_SectorSize);
  DD_WriteFile(EDDFile_bookmrk1,&pfbuf[1*DD_SectorSize],DD_SectorSize);
  DD_WriteFile(EDDFile_bookmrk2,&pfbuf[2*DD_SectorSize],DD_SectorSize);
  DD_WriteFile(EDDFile_bookmrk3,&pfbuf[3*DD_SectorSize],DD_SectorSize);
  
  safefree(pfbuf); pfbuf=NULL;
}

static u32 FindThis(void)
{
  for(u32 idx=0;idx<BookmarkCount;idx++){
    TBookmark *pThis=&pBookmark[idx];
    if((strncmp(pThis->PathName,forResume_PlayPathName,forResume_PlayPathNameLen)==0)&&
       (Unicode_isEqual(pThis->FileUnicodeName,forResume_PlayFileUnicodeName)==true)){
      return(idx);
    }
  }
  return((u32)-1);
}

static void CreateThis(void)
{
  if(BookmarkCount==BookmarkMax) DeleteOldestItem();
  
  TBookmark *pThis=&pBookmark[BookmarkCount];
  
  MemSet32CPU(0,pThis,sizeof(TBookmark));
  
  strncpy(pThis->PathName,forResume_PlayPathName,forResume_PlayPathNameLen);
  Unicode_StrCopy(forResume_PlayFileUnicodeName,pThis->FileUnicodeName);
  
  pThis->Resume.LineNum=0;
  pThis->Resume.DateTime=Bookmark_GetDateTime32FromIPC();
  
  BookmarkCount++;
}

bool Bookmark_GetEnabled(void)
{
  return(BookmarkEnabled);
}

void Bookmark_Start(void)
{
  if(BookmarkEnabled==false) return;
  
  BookmarkCount=0;
  if(pBookmark!=NULL){
    safefree(pBookmark); pBookmark=NULL;
  }
  
  BookmarkCount=0;
  pBookmark=(TBookmark*)safemalloc(sizeof(TBookmark)*BookmarkMax);
  
  LoadFromSector();
  
  if(FindThis()==(u32)-1) CreateThis();
}

void Bookmark_End(void)
{
  if(BookmarkEnabled==false) return;
  
  BookmarkCount=0;
  if(pBookmark!=NULL){
    safefree(pBookmark); pBookmark=NULL;
  }
}

void Bookmark_Save(void)
{
  if(BookmarkEnabled==false) return;
  
  SaveToSector();
}

TBookmark *Bookmark_GetCurrent(void)
{
  if(BookmarkEnabled==false) return(NULL);
  
  u32 idx=FindThis();
  
  if(idx==(u32)-1) return(NULL);
  
  return(&pBookmark[idx]);
}

void Bookmark_CurrentResumeBackup(void)
{
  if(BookmarkEnabled==false) return;
  
  TBookmark *pThis=Bookmark_GetCurrent();
  
  pThis->Resume.LineNum=MWin_GetSBarVPos(WM_TextView);
  pThis->Resume.DateTime=Bookmark_GetDateTime32FromIPC();
}

void Bookmark_CurrentResumeRestore(void)
{
  if(BookmarkEnabled==false) return;
  
  TBookmark *pThis=Bookmark_GetCurrent();
  
  if(pThis->Resume.LineNum==0) return;
  if(pThis->Resume.DateTime==0) return;
  
  MWin_SetSBarVPos(WM_TextView,pThis->Resume.LineNum);
  MWin_DrawSBarVH(WM_TextView);
  MWin_DrawClient(WM_TextView);
  MWin_TransWindow(WM_TextView);
}

u32 Bookmark_GetDateTime32FromIPC(void)
{
  IPC3->curtimeFlag=true;
  IPC3->ReqVsyncUpdate=1;
  while(IPC3->ReqVsyncUpdate!=0){
    swiDelay(1);
  }
  
  u32 dt32;
  
  dt32=IPC3->curtime[1];
  dt32=(dt32*12)+IPC3->curtime[2];
  dt32=(dt32*31)+IPC3->curtime[3];
  u32 hours=IPC3->curtime[5];
  if(52<=hours) hours-=(52-12);
  dt32=(dt32*24)+hours;
  dt32=(dt32*60)+IPC3->curtime[6];
  dt32=(dt32*60)+IPC3->curtime[7];
  
  _consolePrintf("%d,%d,%d,%d,%d,%d,%d\n",dt32,IPC3->curtime[1],IPC3->curtime[2],IPC3->curtime[3],IPC3->curtime[5],IPC3->curtime[6],IPC3->curtime[7]);
  return(dt32);
}

TBookmarkDateTime Bookmark_GetDateTimeStructFrom32(u32 dt32)
{
  TBookmarkDateTime dts;
  
  _consolePrintf("%d,",dt32);
  
  dts.seconds=dt32 % 60;
  dt32/=60;
  dts.minutes=dt32 % 60;
  dt32/=60;
  dts.hours=dt32 % 24;
  dt32/=24;
  dts.day=dt32 % 31;
  dt32/=31;
  dts.month=dt32 % 12;
  dt32/=12;
  dts.year=2000+dt32;
  
  _consolePrintf("%d,%d,%d,%d,%d,%d\n",dts.year,dts.month,dts.day,dts.hours,dts.minutes,dts.seconds);
  return(dts);
}

