
#include <NDS.h>

#include "../_console.h"

#include <stdio.h>
#include <stdlib.h>

#include "../_const.h"
#include "../memtool.h"
#include "../mwin.h"
#include "../mwin_color.h"
#include "../FontPro.h"
#include "../VRAMTool.h"
#include "../filesys.h"

#include "../inifile.h"

#include "plug_text.h"
#include "../unicode.h"
#include "../formdt.h"

#include "plug_text_bookmark.h"

static int FileHandle;

static s32 BaseLine;
static s32 LineCount;

static u32 *LinesCache=NULL;

#define LinesCacheMax (0x40000)

//static u32 ClientWidth,ClientHeight;

#define MarginX (2)

static s32 LineHeight;
static s32 MarginY;
static s32 DispLineCount;

// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------

static bool LoadText(u32 ClientWidth,u32 ClientHeight)
{
  
  if(FileHandle==0){
    _consolePrintf("file not opened.\n");
    return(false);
  }
  if((ClientWidth==0)||(ClientHeight==0)){
    _consolePrintf("ClientSize is 0.\n");
    return(false);
  }
  
  u32 FileSize;
  
  FileSys_fseek(FileHandle,0,SEEK_END);
  FileSize=FileSys_ftell(FileHandle);
  FileSys_fseek(FileHandle,0,SEEK_SET);
  
  LineHeight=FontProHeight+GlobalINI.TextPlugin.SpacePixel;
  MarginY=(ClientHeight % LineHeight)/2;
  DispLineCount=ClientHeight / LineHeight;
  
  BaseLine=0;
  LineCount=0;
  
  LinesCache=(u32*)safemalloc(LinesCacheMax*4);
  if(LinesCache==NULL){
    _consolePrintf("malloc LinesCache out of memory.\n");
    return(false);
  }
  
  MemSet32DMA3((u32)-1,LinesCache,LinesCacheMax*4);
  
#define FileBufMaxSize (1024)
  
  char FileBuf[FileBufMaxSize+4];
  u32 FilePos=0;
  u32 FileBufPos;
  u32 FileBufSize;
  
  MWin_ProgressShow("Processing Unicode.",FileSize);
  
#define ReadFileBuf() { \
  FileBufPos=0; \
  FileBufSize=FileSys_fread(&FileBuf[0],1,FileBufMaxSize,FileHandle); \
  for(int i=FileBufSize;i<(FileBufMaxSize+4);i++) FileBuf[i]=0; \
  MWin_ProgressSetPos(FilePos); \
}

#define ReadByte() FileBuf[FileBufPos]; { \
  FilePos++; \
  FileBufPos++; \
  if(FileBufPos==FileBufSize){ \
    ReadFileBuf(); \
    if(FileBufSize==0) EndFlag=true; \
  } \
}

#define ReadByteDummy() { \
  FilePos++; \
  FileBufPos++; \
  if(FileBufPos==FileBufSize){ \
    ReadFileBuf(); \
    if(FileBufSize==0) EndFlag=true; \
  } \
}
  
  ReadFileBuf();
  
  u32 LineWidth=0;
  u8 LastByte=0x00;
  u32 LastBytePos=0x00;
  bool reqIgnoreNewLine=false;
  
  bool EndFlag=false;
  
  u32 wLimit=ClientWidth-(MarginX*2);
  
  while(EndFlag==false){
    UnicodeChar uc;
    
    u32 LastPos=FilePos;
    
    if(isUTF8==false){
      u16 lc;
      
      u16 b1=ReadByte();
      if(isAnkTable[b1]==0){
        u16 b2=ReadByte();
        lc=(b1 << 8)+(b2);
        }else{
        lc=b1;
      }
      
      if(lc<Local2UnicodeTableSize){
        uc=Local2UnicodeTable[lc];
        }else{
        uc=(UnicodeChar)'?';
      }
      
      }else{ // UTF-8
      u16 b0=ReadByte();
      
      if(b0<0x80){
        uc=b0;
        }else{
        if((b0&0xe0)==0xc0){ // 0b 110. ....
          u16 b1=ReadByte();
          uc=((b0&~0xe0)<<6)+((b1&~0xc0)<<0);
          }else{
          if((b0&0xf0)==0xe0){ // 0b 1110 ....
            u16 b1=ReadByte();
            u16 b2=ReadByte();
            uc=((b0&~0xf0)<<12)+((b1&~0xc0)<<6)+((b2&~0xc0)<<0);
            }else{
            ReadByteDummy(); ReadByteDummy(); ReadByteDummy();
            uc=(u16)'?';
          }
        }
      }
    }
    
    if((uc==0x0d)||(uc==0x0a)){
      if(LastByte==0x00){
        LastByte=(byte)uc;
        LastBytePos=LastPos;
        }else{
        if(reqIgnoreNewLine==true){
          reqIgnoreNewLine=false;
          }else{
          LinesCache[LineCount]=LastBytePos;
          LineCount++;
          LineWidth=0;
        }
        if(LastByte!=uc){
          LastByte=0x00;
          }else{
          LastBytePos=LastPos;
        }
      }
      uc=0x00;
    }
    
    if(uc!=0x00){
      if(LastByte!=0x00){
        LastByte=0x00;
        if(reqIgnoreNewLine==true){
          reqIgnoreNewLine=false;
          }else{
          LinesCache[LineCount]=LastBytePos;
          LineCount++;
          LineWidth=0;
        }
      }
      if(reqIgnoreNewLine==false){
        LinesCache[LineCount]=LastPos;
        LineCount++;
        LineWidth=0;
      }
      reqIgnoreNewLine=true;
      
      u32 FontProWidth;
      
      if(uc==0x09){
        FontProWidth=16;
        }else{
        FontProWidth=FontPro_GetCharWidth(uc);
        if(0x100<uc) FontProWidth++;
      }
      
      if((LineWidth+FontProWidth)<=wLimit){
        LineWidth+=FontProWidth;
        }else{
        LinesCache[LineCount]=LastPos;
        LineCount++;
        LineWidth=FontProWidth;
      }
      
    }
  }
  
#undef ReadFileBuf
#undef ReadByte
#undef ReadByteDummy

  MWin_ProgressHide();
  
  _consolePrintf("Initialized.\n");
  
  _consolePrintf("ClientSize=%d,%d\n",ClientWidth,ClientHeight);
  _consolePrintf("LineCount=%d\n",LineCount);
  
  return(true);
}

bool StartText(int _FileHandle,u32 _ClientWidth,u32 _ClientHeight)
{
  FreeText();
  
  FileHandle=_FileHandle;
  
  if(LoadText(_ClientWidth,_ClientHeight)==false){
    _consolePrintf("Text LoadError.\n");
    FreeText();
    return(false);
  }
  
  return(true);
}

void FreeText(void)
{
  BaseLine=0;
  LineCount=0;
  
  if(LinesCache!=NULL){
    free(LinesCache); LinesCache=NULL;
  }
}

static void drawlineW(u16 *pBuf,u32 w,u32 wLimit,u32 x,u32 y,char *str,u16 col)
{
  while(1){
    UnicodeChar uc;
    
    if(isUTF8==false){
      u16 lc;
      
      u16 b1=(u16)*str++;
      if(isAnkTable[b1]==0){
        u16 b2=(u16)*str++;
        lc=(b1 << 8)+(b2);
        }else{
        lc=b1;
      }
      
      if(lc==0){
        uc=0;
        }else{
        if(lc<Local2UnicodeTableSize){
          uc=Local2UnicodeTable[lc];
          }else{
          uc=(UnicodeChar)'?';
        }
      }
      
      }else{ // UTF-8
      u16 b0=(u16)*str++;
      
      if(b0<0x80){
        uc=b0;
        }else{
        if((b0&0xe0)==0xc0){ // 0b 110. ....
          u16 b1=(u16)*str++;
          uc=((b0&~0xe0)<<6)+((b1&~0xc0)<<0);
          }else{
          if((b0&0xf0)==0xe0){ // 0b 1110 ....
            u16 b1=(u16)*str++;
            u16 b2=(u16)*str++;
            uc=((b0&~0xf0)<<12)+((b1&~0xc0)<<6)+((b2&~0xc0)<<0);
            }else{
            str+=3;
            uc=(u16)'?';
          }
        }
      }
    }
    
    if((uc==0x00)||(uc==0x0d)||(uc==0x0a)) return;
    
    u32 FontProWidth;
    
    if(uc==0x09){
      FontProWidth=16;
      }else{
      FontProWidth=FontPro_GetCharWidth(uc);
      if(0x100<uc) FontProWidth++;
    }
    
    if((x+FontProWidth)<=wLimit){
      if(0x20<=uc) FontPro_WriteCharRect(pBuf,w,wLimit,x,y,uc,col);
      x+=FontProWidth;
      }else{
      return;
    }
  }
}

#define ReadBufSize (128*16*4)

void Text_RefreshScreen(u16 *pBuf,u32 BufWidth,TRect ClientRect)
{
  u16 colText=MWC_TextViewText;
  
  u32 BaseOfs=(u32)-1;
  u32 MaxOfs=0;
  char buf[ReadBufSize+4];
  
  for(s32 y=0;y<DispLineCount+1;y++){
    if((0<=(BaseLine+y))&&((BaseLine+y)<LinesCacheMax)){
      u32 ofs=LinesCache[BaseLine+y];
      
      if(ofs!=(u32)-1){
        if(BaseOfs==(u32)-1) BaseOfs=ofs;
        if(MaxOfs<ofs) MaxOfs=ofs;
      }
    }
  }
  
  if(BaseOfs==(u32)-1) return;
  
  int readsize;
  
  if(LineCount<=(BaseLine+DispLineCount)){
    readsize=ReadBufSize;
    }else{
    readsize=MaxOfs-BaseOfs;
    if(ReadBufSize<=readsize) readsize=ReadBufSize;
  }
  
  FileSys_fseek(FileHandle,BaseOfs,SEEK_SET);
  readsize=FileSys_fread(&buf[0],1,readsize,FileHandle);
  for(;readsize<(ReadBufSize+4);readsize++) buf[readsize]=0;
  
  for(s32 y=0;y<DispLineCount;y++){
    if((0<=(BaseLine+y))&&((BaseLine+y)<LinesCacheMax)){
      u32 wLimit=ClientRect.x+ClientRect.w-MarginX;
      u32 tx=ClientRect.x+MarginX;
      u32 ty=ClientRect.y+MarginY+(y*LineHeight);
      
      u16 *pDstBM=&pBuf[0];
      u32 ofs=LinesCache[BaseLine+y];
      
      if(ofs!=(u32)-1){
        drawlineW(pDstBM,BufWidth,wLimit,tx,ty,&buf[ofs-BaseOfs],colText);
      }
    }
  }
  
}

#undef ReadBufSize

void Text_SetLine(s32 Line,TRect ClientRect)
{
  BaseLine=Line;
  if((LineCount-DispLineCount)<BaseLine) BaseLine=LineCount-DispLineCount;
  if(BaseLine<0) BaseLine=0;
}

s32 Text_GetLine(void)
{
  return(BaseLine);
}

s32 Text_GetLineCount(void)
{
  return(LineCount);
}

s32 Text_GetLineHeight(void)
{
  return(LineHeight);
}

s32 Text_GetDispLineCount(void)
{
  return(DispLineCount);
}

// ---

#include "../../../ipc3.h"

static void DrawDisabledDialog(void)
{
  TWinBody *pwb=MWin_GetWinBody(WM_TextView);
  if(pwb==NULL) return;
  
  u16 colText=MWC_TextViewText;
  u16 colBG=MWC_TextViewBG;
  
  u32 BufWidth=pwb->Rect.w;
  
  u16 *pBuf=pwb->WinBuf;
  if(pBuf==NULL) return;
  pBuf+=pwb->ClientRect.x+(pwb->ClientRect.y*BufWidth);
  
  for(int y=0;y<pwb->ClientRect.h;y++){
    MemSet16DMA3(colBG,&pBuf[y*BufWidth],pwb->ClientRect.w*2);
  }
  
  u32 x=8;
  u32 y=32;
  
  const char *ps0=NULL,*ps1=NULL,*ps2=NULL;
  
  if(DefaultCodePage!=CP932){
    ps0="Disabled bookmark function.";
    ps1="Please refer 'misc/Bookmark/Bookmark.txt'";
    ps2=NULL;
    }else{
    ps0="ブックマーク機能が無効になっています。";
    ps1="'misc/Bookmark/Bookmark.txt'";
    ps2="を参照してください。";
  }
  
  if(ps0!=NULL) VRAM_WriteStringRectUTF8(pBuf,BufWidth,pwb->ClientRect.w,x,y+(14*0),ps0,colText);
  if(ps1!=NULL) VRAM_WriteStringRectUTF8(pBuf,BufWidth,pwb->ClientRect.w,x,y+(14*2),ps1,colText);
  if(ps2!=NULL) VRAM_WriteStringRectUTF8(pBuf,BufWidth,pwb->ClientRect.w,x,y+(14*3),ps2,colText);
  
  MWin_TransWindow(WM_TextView);
}

#define ItemTop (40)
#define ItemHeight (32)
#define ItemLineTop (2)
#define ItemLineHeight (14)

static void DrawDialog(EBookmarkDialogMode bdm,TBookmark *pThis,u32 CurrentIndex)
{
  TWinBody *pwb=MWin_GetWinBody(WM_TextView);
  if(pwb==NULL) return;
  
  u16 colText=MWC_TextViewText;
  u16 colBG=MWC_TextViewBG;
  
  u32 BufWidth=pwb->Rect.w;
  
  u16 *pBuf=pwb->WinBuf;
  if(pBuf==NULL) return;
  pBuf+=pwb->ClientRect.x+(pwb->ClientRect.y*BufWidth);
  
  for(int y=0;y<pwb->ClientRect.h;y++){
    MemSet16DMA3(colBG,&pBuf[y*BufWidth],pwb->ClientRect.w*2);
  }
  
  {
    u32 x=8;
    u32 y=5;
    
    char *pstrbuf=NULL;
    
    if(DefaultCodePage!=CP932){
      switch(bdm){
        case BDM_Exit: pstrbuf="Exit (program error?)"; break;
        case BDM_LoadMenu: pstrbuf="Load from bookmark"; break;
        case BDM_SaveMenu: pstrbuf="Save to bookmark"; break;
      }
      }else{
      switch(bdm){
        case BDM_Exit: pstrbuf="終了 (異常動作)"; break;
        case BDM_LoadMenu: pstrbuf="ブックマークからロードする。"; break;
        case BDM_SaveMenu: pstrbuf="ブックマークにセーブする。"; break;
      }
    }
    
    VRAM_WriteStringRectUTF8(pBuf,BufWidth,pwb->ClientRect.w,x,y,pstrbuf,colText);
    
    if(DefaultCodePage!=CP932){
      pstrbuf="(L=Load/R=Save)";
      }else{
      pstrbuf="(L=Load/R=Save)";
    }
    VRAM_WriteStringRectUTF8(pBuf,BufWidth,pwb->ClientRect.w,pwb->ClientRect.w-96,y,pstrbuf,colText);
    
    UnicodeChar ustr[512];
    StrConvert_Ank2Unicode(pThis->PathName,ustr);
    UnicodeChar upathstr[2];
    upathstr[0]=(UnicodeChar)'/';
    upathstr[1]=(UnicodeChar)0;
    Unicode_StrAppend(ustr,upathstr);
    Unicode_StrAppend(ustr,pThis->FileUnicodeName);
    
    VRAM_WriteStringRectW(pBuf,BufWidth,pwb->ClientRect.w,x,y+16,ustr,colText);
  }
  
  char *pItemTopStr=NULL;
  
  if(DefaultCodePage!=CP932){
    switch(bdm){
      case BDM_Exit: pItemTopStr="Exit"; break;
      case BDM_LoadMenu: pItemTopStr="Load"; break;
      case BDM_SaveMenu: pItemTopStr="Save"; break;
    }
    }else{
    switch(bdm){
      case BDM_Exit: pItemTopStr="終了"; break;
      case BDM_LoadMenu: pItemTopStr="ロード"; break;
      case BDM_SaveMenu: pItemTopStr="セーブ"; break;
    }
  }
  
  for(u32 idx=0;idx<BookmarkSlotCount;idx++){
    u16 colText,colBG;
    
    if(idx!=CurrentIndex){
      colText=MWC_TextViewText;
      colBG=MWC_TextViewBG;
      }else{
      colText=MWC_TextViewBG;
      colBG=MWC_TextViewText;
      for(u32 y=ItemTop+(ItemHeight*idx);y<ItemTop+(ItemHeight*(idx+1));y++){
        MemSet16DMA3(colBG,&pBuf[y*BufWidth],pwb->ClientRect.w*2);
      }
    }
    
    TBookmarkItem *pSlot=&pThis->Slot[idx];
    u32 LineNum=pSlot->LineNum;
    
    u32 x=2;
    u32 y=ItemTop+(ItemHeight*idx)+ItemLineTop;
    
    char *pformatstr;
    char strbuf[256];
    
    if(pSlot->DateTime==0){
      if(DefaultCodePage!=CP932){
        pformatstr="%sSlot.%d Line.%d empty.";
        }else{
        pformatstr="%sスロット.%d %d行 (空)";
      }
      snprintf(strbuf,256,pformatstr,pItemTopStr,1+idx,LineNum);
      VRAM_WriteStringRectUTF8(pBuf,BufWidth,pwb->ClientRect.w,x,y+(ItemLineHeight*0),strbuf,colText);
      }else{
      TBookmarkDateTime dts=Bookmark_GetDateTimeStructFrom32(pSlot->DateTime);
      if(DefaultCodePage!=CP932){
        pformatstr="%sSlot.%d Line.%d %s %d:%02d:%02d";
        }else{
        pformatstr="%sスロット.%d %d.行 %s %d:%02d:%02d";
      }
      char datestr[128];
      formdt_FormatDate(datestr,128,dts.year,dts.month,dts.day);
      snprintf(strbuf,256,pformatstr,pItemTopStr,1+idx,LineNum,datestr,dts.hours,dts.minutes,dts.seconds);
      VRAM_WriteStringRectUTF8(pBuf,BufWidth,pwb->ClientRect.w,x,y+(ItemLineHeight*0),strbuf,colText);
      
      if((u32)LineCount<=LineNum){
        VRAM_WriteStringRect(pBuf,BufWidth,pwb->ClientRect.w,x,y+(ItemLineHeight*1),"out of text",colText);
        }else{
        u32 readsize;
        if((LineNum+1)<(u32)LineCount){
          readsize=LinesCache[LineNum+1]-LinesCache[LineNum];
          if((256-4)<readsize) readsize=256-4;
          }else{
          readsize=256-4;
        }
        
        FileSys_fseek(FileHandle,LinesCache[LineNum],SEEK_SET);
        readsize=FileSys_fread(&strbuf[0],1,readsize,FileHandle);
        for(;readsize<256;readsize++) strbuf[readsize]=0;
        
        drawlineW(pBuf,BufWidth,pwb->ClientRect.w,x,y+(ItemLineHeight*1),&strbuf[0],colText);
      }
    }
  }
  
  MWin_TransWindow(WM_TextView);
}

static EBookmarkDialogMode ProcDialog(EBookmarkDialogMode bdm,TBookmark *pThis,u32 CurrentIndex)
{
  TBookmarkItem *pSlot=&pThis->Slot[CurrentIndex];
  
  switch(bdm){
    case BDM_Exit: {
    } break;
    case BDM_LoadMenu: {
      MWin_SetSBarVPos(WM_TextView,pSlot->LineNum);
      bdm=BDM_Exit;
    } break;
    case BDM_SaveMenu: {
      pSlot->LineNum=MWin_GetSBarVPos(WM_TextView);
      pSlot->DateTime=Bookmark_GetDateTime32FromIPC();
    } break;
  }
  
  return(bdm);
}

EBookmarkDialogMode Text_BookmarkDialog(EBookmarkDialogMode bdm)
{
  if(bdm==BDM_Exit) return(BDM_Exit);
  
  if(Bookmark_GetEnabled()==false){
    DrawDisabledDialog();
    while(KEYS_CUR!=0){
      swiWaitForVBlank();
      ScanIPC3(true);
    }
    while(KEYS_CUR==0){
      swiWaitForVBlank();
      ScanIPC3(true);
    }
    MWin_DrawSBarVH(WM_TextView);
    MWin_DrawClient(WM_TextView);
    MWin_TransWindow(WM_TextView);
    while(KEYS_CUR!=0){
      swiWaitForVBlank();
      ScanIPC3(true);
    }
    bdm=BDM_Exit;
    return(bdm);
  }
  
  TBookmark *pThis=Bookmark_GetCurrent();
  
  if(pThis==NULL){
    _consolePrintf("error: can not get current bookmark.\n");
    ShowLogHalt();
    while(1);
  }
  
  u32 SlotIndex=pThis->SlotIndex;
  
  while(bdm!=BDM_Exit){
    DrawDialog(bdm,pThis,SlotIndex);
    
    while(KEYS_CUR!=0){
      swiWaitForVBlank();
      ScanIPC3(true);
    }
    while(KEYS_CUR==0){
      swiWaitForVBlank();
      ScanIPC3(true);
    }
    
    u16 loadkey=KEYS_CUR;
    
    if(loadkey==KEY_UP){
      if(0<SlotIndex) SlotIndex--;
    }
    if(loadkey==KEY_DOWN){
      if(SlotIndex<(BookmarkSlotCount-1)) SlotIndex++;
    }
    if(loadkey==KEY_L) bdm=BDM_LoadMenu;
    if(loadkey==KEY_R) bdm=BDM_SaveMenu;
    if(loadkey==KEY_B) bdm=BDM_Exit;
    
    if(loadkey==KEY_A){
      bdm=ProcDialog(bdm,pThis,SlotIndex);
      Bookmark_Save();
    }
  }
  
  pThis->SlotIndex=SlotIndex;
  Bookmark_Save();
  
  MWin_DrawSBarVH(WM_TextView);
  MWin_DrawClient(WM_TextView);
  MWin_TransWindow(WM_TextView);
  
  while(KEYS_CUR!=0){
    swiWaitForVBlank();
    ScanIPC3(true);
  }
  
  return(bdm);
}

