
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filesys.h"
#include "inifile.h"

#include "strtool.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "_const.h"

#include "cimfs.h"

#include "gba_nds_fat.h"

#include <gbfs.h>
//#include "gbfsdata.h"

#include "unicode.h"

#define FAT_FT_END (0)
#define FAT_FT_FILE (1)
#define FAT_FT_DIR (2)

#define VolumeName_IMFS "IMFS"
#define VolumeName_EXFS "EXFS"
#define VolumeName_MPCF "EZSD"
#define VolumeName_GBFS "GBFS"

u32 FileSys_VolumeType=VT_NULL;
u32 CurrentVolumeType=VT_NULL;
char FileSys_PathName[MAX_PATH_LENGTH]="//";

const GBFS_FILE *gbfsfile=NULL;

typedef struct {
  u32 FileType;
  u32 FileSize;
  char Alias[IMFS_FilenameLengthMax];
  UnicodeChar Filename[IMFS_FilenameLengthMax];
  int TrackNum;
} TFile;

static int FileMaxCount;

TFile *pFile=NULL;
s32 FileCount=0;

char *FileSys_InitInterface_ErrorStr=NULL;

static char LastPathItemName[256]={0};

static int ShuffleCount=-1;
static int *pShuffleList=NULL;

extern "C" void _fatchk(char *file,u32 line);
#define fatchk() { _fatchk(__FILE__,__LINE__); }

void FileSys_Init(int _FileMaxCount)
{ cwl();
  FileSys_Free();
  
  FileMaxCount=_FileMaxCount;
  pFile=(TFile*)malloc(FileMaxCount*sizeof(TFile));
  pShuffleList=(int*)malloc(FileMaxCount*4);
  
  _consolePrintf("FileMaxCount=%d\n",FileMaxCount);
}

void FileSys_Free(void)
{ cwl();
  if(pFile!=NULL){
    free(pFile); pFile=NULL;
  }
  
  if(pShuffleList!=NULL){
    free(pShuffleList); pShuffleList=NULL;
  }
  
  FileCount=0;
  FileSys_VolumeType=VT_NULL;
  CurrentVolumeType=VT_NULL;
  strcpy(FileSys_PathName,"//");
}

u32 FileSys_GetVolumeType(void)
{ cwl();
  return(FileSys_VolumeType);
}

void FileSys_RefreshVolume(void)
{ cwl();
  FileSys_VolumeType=VT_NULL;
  
  if((FileSys_PathName[0]==0)||(FileSys_PathName[1]==0)) return;
  if((FileSys_PathName[0]!='/')||(FileSys_PathName[1]!='/')) return;
  
  if(strncmp(&FileSys_PathName[2],VolumeName_IMFS,4)==0) FileSys_VolumeType=VT_IMFS;
  if(strncmp(&FileSys_PathName[2],VolumeName_EXFS,4)==0) FileSys_VolumeType=VT_EXFS;
  if(strncmp(&FileSys_PathName[2],VolumeName_MPCF,4)==0) FileSys_VolumeType=VT_MPCF;
  if(strncmp(&FileSys_PathName[2],VolumeName_GBFS,4)==0) FileSys_VolumeType=VT_GBFS;
  
}

#include "setarm9_reg_waitcr.h"

static bool InitFlag_IMFS=false,InitFlag_EXFS=false,InitFlag_MPCF=false,InitFlag_GBFS=false;

bool FileSys_InitInterface(void)
{ cwl();
  FileSys_InitInterface_ErrorStr="";
  
  if(FileSys_VolumeType==CurrentVolumeType) return(true);
  CurrentVolumeType=FileSys_VolumeType;
  
  bool res=false;
  
  SetARM9_REG_WaitCR();
  
  switch(FileSys_VolumeType){ cwl();
    case VT_NULL: res=true; break;
    case VT_IMFS: {
      if(InitFlag_IMFS==true){
        res=true;
        }else{
        if(pIMFS==NULL){
          res=false;
          }else{
          res=true;
        }
        InitFlag_IMFS=res;
      }
    } break;
    case VT_EXFS: {
      if(InitFlag_EXFS==true){
        res=true;
        }else{
        if(pEXFS==NULL){
          res=false;
          }else{
          res=true;
        }
        InitFlag_EXFS=res;
      }
    } break;
    case VT_MPCF: {
      if(InitFlag_MPCF==true){
        res=true;
        }else{
        res=InitFlag_MPCF=FAT_InitFiles();
      }
    } break;
    case VT_GBFS: {
      if(InitFlag_GBFS==true){
        res=true;
        }else{
        gbfsfile=find_first_gbfs_file((void*)0x08000000);
        if(gbfsfile!=NULL){
          res=InitFlag_GBFS=true;
        }
      }
    } break;
    default: {
      _consolePrintf("FileSys_VolumeType==VT_??? FileSys_InitInterface(); halt.\n");
      ShowLogHalt();
    } break;
  }
  
  SetARM9_REG_WaitCR();
  
  if(res==false){
    FileSys_InitInterface_ErrorStr="media not insert or not found error.";
  }
  
  return(res);
}

void FileSys_FreeInterface(void)
{ cwl();
  SetARM9_REG_WaitCR();
  
  switch(FileSys_VolumeType){ cwl();
    case VT_NULL: break;
    case VT_IMFS: {
      if(InitFlag_IMFS==true){
//        IMFS_Free();
        InitFlag_IMFS=false;
      }
    } break;
    case VT_EXFS: {
      if(InitFlag_EXFS==true){
//        IMFS_Free();
        InitFlag_EXFS=false;
      }
    } break;
    case VT_MPCF: {
      if(InitFlag_MPCF==true){
//        FAT_FreeFiles();
        InitFlag_MPCF=false;
      }
    } break;
    case VT_GBFS: {
      if(InitFlag_GBFS==true){
//        gbfsfile=NULL;
        InitFlag_GBFS=false;
      }
    } break;
    default: {
      _consolePrintf("FileSys_VolumeType==VT_??? FileSys_FreeInterface(); halt.\n");
      ShowLogHalt();
    } break;
  }
  
  SetARM9_REG_WaitCR();
  
  CurrentVolumeType=VT_NULL;
}

void FileSys_RefreshPath_DuplicateTracks(int tracks)
{
  if(FileCount==FileMaxCount) return;
  
  u32 SrcFileIndex=FileCount-1;
  
  pFile[SrcFileIndex].TrackNum=0;
  
  for(int idx=1;idx<tracks;idx++){
    pFile[FileCount].FileType=pFile[SrcFileIndex].FileType;
    pFile[FileCount].FileSize=pFile[SrcFileIndex].FileSize;
    {
      char *src=pFile[SrcFileIndex].Alias;
      char *dst=pFile[FileCount].Alias;
      while(*src!=0){
        *dst++=*src++;
      }
      *dst=0;
    }
    {
      UnicodeChar *src=pFile[SrcFileIndex].Filename;
      UnicodeChar *dst=pFile[FileCount].Filename;
      while(*src!=0){
        *dst++=*src++;
      }
      *dst=0;
    }
    pFile[FileCount].TrackNum=idx;
    FileCount++;
    if(FileCount==FileMaxCount) break;
  }
}

void FileSys_RefreshPath_GMENSF_DuplicateTracks(int FileHandle)
{
  int tracks=-1;
  
  u8 buf[7];
  
  if(FAT_fread(&buf[0],1,7,(FAT_FILE*)FileHandle)==7){
    if((buf[0]=='N')&&(buf[1]=='E')&&(buf[2]=='S')&&(buf[3]=='M')&&(buf[4]==0x1a)){
      if(buf[5]==1){
        tracks=buf[6];
      }
    }
  }
  
  if(tracks!=-1) FileSys_RefreshPath_DuplicateTracks(tracks);
}

void FileSys_RefreshPath_GMEGBS_DuplicateTracks(int FileHandle)
{
  int tracks=-1;
  
  u8 buf[5];
  
  if(FAT_fread(&buf[0],1,5,(FAT_FILE*)FileHandle)==5){
    if((buf[0]=='G')&&(buf[1]=='B')&&(buf[2]=='S')){
      if(buf[3]==1){
        tracks=buf[4];
      }
    }
  }
  
  if(tracks!=-1) FileSys_RefreshPath_DuplicateTracks(tracks);
}

extern char *DIMediaName;

void FileSys_RefreshPath(void)
{ cwl();
//  _consolePrintf("FileSys_PathName=%s\n",FileSys_PathName);
  
  FileSys_RefreshVolume();
  
  if(FileSys_VolumeType==VT_NULL) strcpy(FileSys_PathName,"//");
  
  char tmpstr[MAX_FILENAME_LENGTH];
  
  for(int idx=0;idx<FileMaxCount;idx++){
    TFile *_pFile=&pFile[idx];
    _pFile->FileType=0;
    _pFile->FileSize=0;
    _pFile->Alias[0]=0;
    _pFile->Filename[0]=0;
    _pFile->TrackNum=-1;
  }
  
  if(FileSys_InitInterface()==false){ cwl();
    FileCount=0;
    
    pFile[FileCount].FileType=FT_Path;
    pFile[FileCount].FileSize=0;
    strcpy(pFile[FileCount].Alias,"//");
    sprintf(tmpstr,"%s %s",pFile[FileCount].Alias,FileSys_InitInterface_ErrorStr);
    StrConvert_Local2Unicode(tmpstr,pFile[FileCount].Filename);
    FileCount++;
    return;
  }
  
  switch(FileSys_VolumeType){ cwl();
    case VT_NULL:
      { cwl();
        FileCount=0;
        
       // pFile[FileCount].FileType=FT_Path;
       // pFile[FileCount].FileSize=0;
       // strcpy(pFile[FileCount].Alias,VolumeName_IMFS);
       // sprintf(tmpstr,"%s/ %s",pFile[FileCount].Alias,IMFSTitle);
       // StrConvert_Local2Unicode(tmpstr,pFile[FileCount].Filename);
        //FileCount++;
        
        //pFile[FileCount].FileType=FT_Path;
        //pFile[FileCount].FileSize=0;
        //strcpy(pFile[FileCount].Alias,VolumeName_EXFS);
        //sprintf(tmpstr,"%s/ %s",pFile[FileCount].Alias,EXFSTitle);
        //StrConvert_Local2Unicode(tmpstr,pFile[FileCount].Filename);
        //FileCount++;
        
        pFile[FileCount].FileType=FT_Path;
        pFile[FileCount].FileSize=0;
        strcpy(pFile[FileCount].Alias,VolumeName_MPCF);
        sprintf(tmpstr,"%s/ %s",pFile[FileCount].Alias,DIMediaName);//MPCFIOTitle);
        StrConvert_Local2Unicode(tmpstr,pFile[FileCount].Filename);
        FileCount++;
        
        //pFile[FileCount].FileType=FT_Path;
        //pFile[FileCount].FileSize=0;
        //strcpy(pFile[FileCount].Alias,VolumeName_GBFS);
        //sprintf(tmpstr,"%s/ %s",pFile[FileCount].Alias,GBFSTitle);
        //StrConvert_Local2Unicode(tmpstr,pFile[FileCount].Filename);
        //FileCount++;
      } break;
    case VT_IMFS:
    case VT_EXFS:
      { cwl();
        CIMFS *pFS;
        if(FileSys_VolumeType==VT_IMFS){
          pFS=pIMFS;
          }else{
          pFS=pEXFS;
        }
        
        s32 PathIndex=pFS->GetPathIndex(&FileSys_PathName[6]);
        FileCount=pFS->GetFileCount(PathIndex);
        
        if((PathIndex==-1)||(FileCount==-1)){ cwl();
          FileCount=0;
          
          pFile[FileCount].FileType=FT_Path;
          pFile[FileCount].FileSize=0;
          strcpy(pFile[FileCount].Alias,"//");
          StrConvert_Local2Unicode("// error. return root",pFile[FileCount].Filename);
          FileCount++;
          return;
        }
        
        for(s32 idx=0;idx<FileCount;idx++){ cwl();
          char *PathChar;
          if(pFS->GetFileTypeFromIndex(PathIndex,idx)==FT_Path){ cwl();
            PathChar="/";
            pFile[idx].FileType=FT_Path;
            pFile[idx].FileSize=0;
            }else{ cwl();
            PathChar="";
            pFile[idx].FileType=FT_File;
            pFile[idx].FileSize=pFS->GetFileDataSizeFromIndex(PathIndex,idx);
          }
          strcpy(pFile[idx].Alias,pFS->GetFilenameFromIndex(PathIndex,idx));
          
          {
            UnicodeChar *srcus=pFS->GetLongFilenameFromIndex(PathIndex,idx);
            UnicodeChar *dstus=pFile[idx].Filename;
            
            int i=0;
            
            while(srcus[i]!=0){
              dstus[i]=srcus[i];
              i++;
            }
            
            while(*PathChar!=0){
              dstus[i]=*PathChar;
              i++; PathChar++;
            }
            dstus[i]=0;
          }
        }
      } break;
    case VT_MPCF:
      { cwl();
        bool InsertDoubleDotPath;
        
        if(strlen(FileSys_PathName)<=6){ cwl();
          FAT_CWD("/");
          InsertDoubleDotPath=true;
          }else{ cwl();
          FAT_CWD(&FileSys_PathName[6]);
          InsertDoubleDotPath=true;
        }
        
        char fn[MAX_FILENAME_LENGTH];
        u32 FAT_FileType;
        
        FileCount=0;
        
        if(InsertDoubleDotPath==true){ cwl();
          pFile[FileCount].FileType=FT_Path;
          pFile[FileCount].FileSize=0;
          strcpy(pFile[FileCount].Alias,"..");
          StrConvert_Local2Unicode("../",pFile[FileCount].Filename);
          FileCount++;
        }
        
        FAT_FileType=FAT_FindFirstFile(fn);
        
        TiniHiddenItem *pHiddenItem=&GlobalINI.HiddenItem;
        
        u8 ReqHiddenAttrib=0;
        
        {
          const u8 ATTRIB_ARCH=0x20;
          const u8 ATTRIB_HID=0x02;
          const u8 ATTRIB_SYS=0x04;
          const u8 ATTRIB_RO=0x01;
          
          if(pHiddenItem->Attribute_Archive==true) ReqHiddenAttrib|=ATTRIB_ARCH;
          if(pHiddenItem->Attribute_Hidden==true) ReqHiddenAttrib|=ATTRIB_HID;
          if(pHiddenItem->Attribute_System==true) ReqHiddenAttrib|=ATTRIB_SYS;
          if(pHiddenItem->Attribute_Readonly==true) ReqHiddenAttrib|=ATTRIB_RO;
        }
        
        while(FAT_FileType!=FAT_FT_END){ cwl();
          bool useflag=true;
          
          if(FAT_FileType==FAT_FT_DIR){
            if(strcmp(fn,".")==0) useflag=false;
            if(strcmp(fn,"..")==0) useflag=false;
            if((pHiddenItem->Path_Shell==true)&&(strcmp(fn,"SHELL")==0)) useflag=false;
          }
          if(FAT_FileType==FAT_FT_FILE){
            if((pHiddenItem->File_Thumbnail==true)&&(strcmp(fn,"_THUMBNL.MSL")==0)) useflag=false;
          }
          
          if(ReqHiddenAttrib!=0){
            if((FAT_GetAttrib()&ReqHiddenAttrib)!=0) useflag=false;
          }
          
          if(useflag==true){ cwl();
            strcpy(pFile[FileCount].Alias,fn);
            
            char *PathChar="";
            
            switch(FAT_FileType){ cwl();
              case FAT_FT_DIR:
                pFile[FileCount].FileType=FT_Path;
                pFile[FileCount].FileSize=0;
                PathChar="/";
                break;
              case FAT_FT_FILE:
                pFile[FileCount].FileType=FT_File;
                pFile[FileCount].FileSize=FAT_GetFileSize();
                PathChar=""; // 0x00
                break;
            }
            
            UnicodeChar *lfn=pFile[FileCount].Filename;
            
            if(FAT_GetLongFilenameUnicode(lfn,IMFS_FilenameLengthMax-4)==false){ cwl();
              StrConvert_Local2Unicode(fn,lfn);
            }
            
            u32 idx=0;
            while(1){ cwl();
              if(lfn[idx]==0){ cwl();
                StrConvert_Local2Unicode(PathChar,&lfn[idx]);
                break;
              }
              idx++;
            }
            
            FileCount++;
            if(FileCount==FileMaxCount) break;
            
            { // DuplicateTracks
              char *Alias=pFile[FileCount-1].Alias;
              int AliasLen=strlen(Alias);
              int exf=0;
              
              if((Alias[AliasLen-4]=='.')&&(Alias[AliasLen-3]=='N')&&(Alias[AliasLen-2]=='S')&&(Alias[AliasLen-1]=='F')) exf=1;
              if((Alias[AliasLen-4]=='.')&&(Alias[AliasLen-3]=='G')&&(Alias[AliasLen-2]=='B')&&(Alias[AliasLen-1]=='S')) exf=2;
              
              switch(exf){
                case 0: { // none
                } break;
                case 1: { // NSF
                  int FileHandle=(int)FAT_fopen(Alias,"rb");
                  
                  if(FileHandle!=0){
                    FileSys_RefreshPath_GMENSF_DuplicateTracks(FileHandle);
                    FAT_fclose((FAT_FILE*)FileHandle);
                  }
                } break;
                case 2: { // GBS
                  int FileHandle=(int)FAT_fopen(Alias,"rb");
                  
                  if(FileHandle!=0){
                    FileSys_RefreshPath_GMEGBS_DuplicateTracks(FileHandle);
                    FAT_fclose((FAT_FILE*)FileHandle);
                  }
                }
              }
              
              if(FileCount==FileMaxCount) break;
            }
          }
          
          FAT_FileType=FAT_FindNextFile(fn);
        }
      } break;
    case VT_GBFS:
      { cwl();
        FileCount=0;
        
        pFile[FileCount].FileType=FT_Path;
        pFile[FileCount].FileSize=0;
        strcpy(pFile[FileCount].Alias,"..");
        strcpy(tmpstr,"../");
        StrConvert_Local2Unicode(tmpstr,pFile[FileCount].Filename);
        FileCount++;
        
        size_t idxcount;
        
        idxcount=gbfs_count_objs(gbfsfile);
        
        for(size_t idx=0;idx<idxcount;idx++){ cwl();
          char fn[MAX_FILENAME_LENGTH];
          u32 fsize;
          
          gbfs_get_nth_obj(gbfsfile,idx,fn,&fsize);
          
          pFile[FileCount].FileType=FT_File;
          pFile[FileCount].FileSize=fsize;
          strcpy(pFile[FileCount].Alias,fn);
          StrConvert_Local2Unicode(fn,pFile[FileCount].Filename);
          FileCount++;
        }
      } break;
  }
  
  if(FileSys_VolumeType!=VT_NULL){
    extern void FileListSort(void);
    FileListSort();
  }
}

void FileSys_ChangePath(char *TargetPathName)
{ cwl();
//  _consolePrintf("ChangePath(%s);\n",TargetPathName);
  
  LastPathItemName[0]=0;
  
  if((TargetPathName[0]=='.')&&(TargetPathName[1]=='.')){ cwl();
    u32 FlashPos=0;
    
    u32 cnt=0;
    while(FileSys_PathName[cnt]!=0){ cwl();
      if(FileSys_PathName[cnt]=='/') FlashPos=cnt;
      cnt++;
    }
    
    if(FlashPos!=0) strcpy(LastPathItemName,(char*)&FileSys_PathName[FlashPos+1]);
  }
  
  if(TargetPathName==NULL){ cwl();
    strcpy(FileSys_PathName,"//");
    FileSys_RefreshPath();
    return;
  }
  
  if(TargetPathName[0]==0){ cwl();
    strcpy(FileSys_PathName,"//");
    FileSys_RefreshPath();
    return;
  }
  
  // Rootから
  if((TargetPathName[0]=='/')&&(TargetPathName[1]=='/')){ cwl();
    strcpy(FileSys_PathName,TargetPathName);
    FileSys_RefreshPath();
    return;
  }
  
  // VolumeRootから
  if((TargetPathName[0]=='/')&&(TargetPathName[1]!='/')){ cwl();
    if(strlen(FileSys_PathName)<6){ cwl();
      strcpy(FileSys_PathName,"//");
      FileSys_RefreshPath();
      return;
    }
    
    FileSys_PathName[6]=0;
    strcat(FileSys_PathName,TargetPathName);
    FileSys_RefreshPath();
    return;
  }
  
  // UpPath
  if((TargetPathName[0]=='.')&&(TargetPathName[1]=='.')){ cwl();
    if(strlen(FileSys_PathName)<6){ cwl();
      strcpy(FileSys_PathName,"//");
      FileSys_RefreshPath();
      return;
    }
    
    { cwl();
      u32 FlashPos=0;
      u32 cnt=0;
      while(FileSys_PathName[cnt]!=0){ cwl();
        if(FileSys_PathName[cnt]=='/') FlashPos=cnt;
        cnt++;
      }
      FileSys_PathName[FlashPos]=0;
      if(strlen(FileSys_PathName)<6) strcpy(FileSys_PathName,"//");
      FileSys_RefreshPath();
      return;
    }
  }
  
  if(strlen(FileSys_PathName)<6){ cwl();
    }else{ cwl();
    strcat(FileSys_PathName,"/");
  }
  
  strcat(FileSys_PathName,TargetPathName);
  
  FileSys_RefreshPath();
  
  return;
}

s32 FileSys_GetLastPathItemIndex(void)
{ cwl();
  for(s32 idx=0;idx<FileCount;idx++){ cwl();
    if(pFile[idx].FileType==FT_Path){ cwl();
      if(strcmp(pFile[idx].Alias,LastPathItemName)==0) return(idx);
    }
  }
  
  return(-1);
}

s32 FileSys_GetFileCount(void)
{ cwl();
  return(FileCount);
}

s32 FileSys_GetPureFileCount(void)
{
  s32 cnt=0;
  
  for(s32 idx=0;idx<FileCount;idx++){ cwl();
    if(pFile[idx].FileType==FT_File) cnt++;
  }
  
  return(cnt);
}


char* FileSys_GetPathName(void)
{ cwl();
  return(FileSys_PathName);
}

char* FileSys_GetAlias(s32 FileIndex)
{ cwl();
  return(pFile[FileIndex].Alias);
}

UnicodeChar* FileSys_GetFilename(s32 FileIndex)
{ cwl();
  return(pFile[FileIndex].Filename);
}

u32 FileSys_GetFileType(s32 FileIndex)
{ cwl();
  return(pFile[FileIndex].FileType);
}

int FileSys_GetFileTrackNum(s32 FileIndex)
{ cwl();
  return(pFile[FileIndex].TrackNum);
}

void FileSys_GetFileExt(s32 FileIndex,char *ext)
{ cwl();
  char *Alias=FileSys_GetAlias(FileIndex);
  
  u32 cnt=0;
  
  u32 DotPos=0;
  cnt=0;
  while(Alias[cnt]!=0){ cwl();
    if(Alias[cnt]=='.') DotPos=cnt;
    cnt++;
  }
  
  if(DotPos==0){ cwl();
    ext[0]=0;
    }else{ cwl();
    strcpy(ext,&Alias[DotPos]);
  }
  
  cnt=0;
  while(ext[cnt]!=0){ cwl();
    char c=ext[cnt];
    
    if ((c>0x60)&&(c<0x7B)){ cwl();
      c=c-0x20;
      ext[cnt]=c;
    }
    cnt++;
  }
}

u32 FileSys_GetFileDataSize(u32 FileIndex)
{ cwl();
  return(pFile[FileIndex].FileSize);
}

bool FileSys_GetFileData(u32 FileIndex,u8 *dstbuf)
{ cwl();
  int fh;
  
  fh=FileSys_fopen(FileIndex);
  if(fh==0) return(false);
  
  FileSys_fread(dstbuf,1,FileSys_GetFileDataSize(FileIndex),fh);
  FileSys_fclose(fh);
  
  return(true);
}

#define MemFileCount (8)

typedef struct {
  bool Enabled;
  u32 VolumeType;
  bool reqfree;
  u8 *data;
  u32 datasize;
  u32 offset;
  FAT_FILE *file;
} TMemFile;

static TMemFile MemFile[MemFileCount];

static inline int GetFreeMemFileIndex(void)
{
  for(int idx=1;idx<MemFileCount;idx++){
    if(MemFile[idx].Enabled==false) return(idx);
  }
  
  return(0);
}

int FileSys_fopen_DirectMapping(u32 VolumeType,bool reqfree,u8 *data,u32 datasize,u32 offset,int file)
{ cwl();
  int hFile=GetFreeMemFileIndex();
  
  if(hFile==0){ cwl();
    _consolePrintf("fopen Error:Already Opened.\n");
    return(0);
  }
  
  TMemFile *pMemFile=&MemFile[hFile];
  
  pMemFile->VolumeType=VolumeType;
  pMemFile->reqfree=reqfree;
  pMemFile->data=data;
  pMemFile->datasize=datasize;
  pMemFile->offset=offset;
  pMemFile->file=(FAT_FILE*)file;
  
  pMemFile->Enabled=true;
  
  return(hFile);
}
void FileSys_fGetLongFileName(u32 FileIndex,char *cLongFilename)
{
 	char *p;
	FileSys_fopen(FileIndex);
	FAT_GetLongFilename(cLongFilename);
	FileSys_fclose(FileIndex);
}
int FileSys_fopen(u32 FileIndex)
{ cwl();
  int hFile=GetFreeMemFileIndex();
  if(hFile==0){ cwl();
    _consolePrintf("fopen Error:Already Opened.\n");
    return(0);
  }
  
  const char *pFilename=NULL;
  
  switch(FileIndex){
    case SystemFileID_Thumbnail1: pFilename=&SystemFilename_Thumbnail1[0]; break;
    default: pFilename=pFile[FileIndex].Alias; break;
  }
  
  TMemFile *pMemFile=&MemFile[hFile];
  
  pMemFile->VolumeType=FileSys_VolumeType;
  pMemFile->reqfree=false;
  pMemFile->data=NULL;
  pMemFile->datasize=0;
  pMemFile->offset=0;
  pMemFile->file=NULL;
  
  switch(pMemFile->VolumeType){ cwl();
    case VT_NULL: { cwl();
      return(0);
    } break;
    case VT_IMFS: case VT_EXFS: { cwl();
      CIMFS *pFS;
      if(pMemFile->VolumeType==VT_IMFS){
        pFS=pIMFS;
        }else{
        pFS=pEXFS;
      }
      
      u32 fsize;
      u8 *data;
      
      fsize=pFile[FileIndex].FileSize;
      
      s32 PathIndex=pFS->GetPathIndex(&FileSys_PathName[6]);
      if(PathIndex==-1) return(0);
      
      FileIndex=pFS->GetIndexFromFilename(PathIndex,pFilename);
      if(FileIndex==(u32)-1) return(0);
      
      if(pFS->GetFileTypeFromIndex(PathIndex,FileIndex)==FT_FileFlat){ cwl();
        pMemFile->reqfree=false;
        data=pFS->GetFileDirectDataFromIndex(PathIndex,FileIndex);
        if(data==NULL) return(0);
        }else{ cwl();
        pMemFile->reqfree=true;
        data=(u8*)malloc(fsize+32);
        pFS->GetFileDataFromIndex(PathIndex,FileIndex,data);
      }
      
      pMemFile->data=data;
      pMemFile->datasize=fsize;
      pMemFile->offset=0;
      
      pMemFile->Enabled=true;
      return(hFile);
    } break;
    case VT_MPCF: { cwl();
      FAT_FILE *file=FAT_fopen(pFilename,"r");
      
      // req NULL==0.
      if(file==NULL){ cwl();
//        _consolePrintf("FAT_fopen error.\n");
        return(0);
      }
      
      pMemFile->file=file;
      
      pMemFile->Enabled=true;
      return(hFile);
    } break;
    case VT_GBFS: { cwl();
      u32 fsize;
      const void *data;
      
      data=gbfs_get_obj(gbfsfile,pFilename,&fsize);
      
      if(data==NULL){ cwl();
        _consolePrintf("GBFS Error:File not found.\n");
        return(0);
      }
      
      pMemFile->reqfree=false;
      pMemFile->data=(u8*)data;
      pMemFile->datasize=fsize;
      pMemFile->offset=0;
      
      pMemFile->Enabled=true;
      return(hFile);
    } break;
  }
  
  return(0);
}

bool FileSys_fclose (int hFile)
{ cwl();
  TMemFile *pMemFile=&MemFile[hFile];
  if(pMemFile->Enabled==false) return(false);
  
  pMemFile->Enabled=false;
  
  switch(pMemFile->VolumeType){ cwl();
    case VT_NULL: { cwl();
    } break;
    case VT_IMFS: case VT_EXFS: case VT_GBFS: { cwl();
      if(pMemFile->reqfree==true) free(pMemFile->data);
      return(true);
    } break;
    case VT_MPCF: { cwl();
      if(pMemFile->file!=NULL) return(FAT_fclose(pMemFile->file));
    } break;
  }
  
  return(false);
}

long int FileSys_ftell (int hFile)
{ cwl();
  TMemFile *pMemFile=&MemFile[hFile];
  if(pMemFile->Enabled==false) return(0);
  
  switch(pMemFile->VolumeType){ cwl();
    case VT_NULL: { cwl();
    } break;
    case VT_IMFS: case VT_EXFS: case VT_GBFS: { cwl();
      return(pMemFile->offset);
    } break;
    case VT_MPCF: { cwl();
      return(FAT_ftell(pMemFile->file));
    } break;
  }
  
  return(0);
}

int FileSys_fseek(int hFile, u32 offset, int origin)
{ cwl();
  TMemFile *pMemFile=&MemFile[hFile];
  if(pMemFile->Enabled==false) return(0);
  
  switch(pMemFile->VolumeType){ cwl();
    case VT_NULL: { cwl();
    } break;
    case VT_IMFS: case VT_EXFS: case VT_GBFS: { cwl();
      int moveoffset=0;
      u32 lastoffset=pMemFile->offset;
      
      switch(origin){ cwl();
        case SEEK_SET:
          pMemFile->offset=offset;
          break;
        case SEEK_CUR:
          pMemFile->offset+=offset;
          break;
        case SEEK_END:
          pMemFile->offset=pMemFile->datasize-offset;
          break;
        default:
          return(0);
      }
      
      if(lastoffset<pMemFile->offset){ cwl();
        moveoffset=pMemFile->offset-lastoffset;
        }else{ cwl();
        moveoffset=lastoffset-pMemFile->offset;
        moveoffset=-moveoffset;
      }
      
      return(moveoffset);
    } break;
    case VT_MPCF: { cwl();
      return(FAT_fseek(pMemFile->file,offset,origin));
    } break;
  }
  
  return(0);
}

u32 FileSys_fread (void* buffer, u32 size, u32 count, int hFile)
{ cwl();
  TMemFile *pMemFile=&MemFile[hFile];
  if(pMemFile->Enabled==false) return(0);
  
  switch(pMemFile->VolumeType){ cwl();
    case VT_NULL: { cwl();
    } break;
    case VT_IMFS: case VT_EXFS: case VT_GBFS: { cwl();
      u32 readsize=size*count;
      
      if((pMemFile->offset+readsize)>pMemFile->datasize){ cwl();
        readsize=pMemFile->datasize-pMemFile->offset;
      }
      
      if(readsize==0) return(0);
      
      u8 *src=pMemFile->data;
      
      memcpy(buffer,&src[pMemFile->offset],readsize);
      
      pMemFile->offset+=readsize;
      
      return(readsize);
      
    } break;
    case VT_MPCF: { cwl();
      return(FAT_fread(buffer,size,count,pMemFile->file));
    } break;
  }
  
  return(0);
}

u32 FileSys_fread_fast (void* buffer, u32 size, u32 count, int hFile)
{ cwl();
  TMemFile *pMemFile=&MemFile[hFile];
  if(pMemFile->Enabled==false) return(0);
  
  if(pMemFile->VolumeType!=VT_MPCF){
    return(FileSys_fread(buffer,size,count,hFile));
    }else{
    return(FAT_fread_fast(buffer,size,count,pMemFile->file));
  }
}

u32 FileSys_GetFileCluster(u32 FileIndex)
{ cwl();
  if(FileSys_VolumeType!=VT_MPCF) return(0);
  
  char *Alias=pFile[FileIndex].Alias;
  int hFile=(int)FAT_fopen(Alias,"r");
  
  if(hFile==0){ cwl();
    _consolePrintf("FAT_fopen error.\n");
    return(0);
  }
  
  u32 FileCluster=FAT_GetFileCluster();
  
  FAT_fclose((FAT_FILE*)hFile);
  
  return(FileCluster);
}

// ---------------------------------------------------

inline static bool isSwapFile(TFile *pf0,TFile *pf1)
{
  UnicodeChar *puc0=&pf0->Filename[0];
  UnicodeChar *puc1=&pf1->Filename[0];
  
  while(1){
    u32 uc0=*puc0;
    u32 uc1=*puc1;
    if(((u32)'A'<=uc0)&&(uc0<=(u32)'Z')) uc0+=0x20;
    if(((u32)'A'<=uc1)&&(uc1<=(u32)'Z')) uc1+=0x20;
    
    if(uc0==uc1){
      // 同一ファイル名はないはず…だったのにね。
      if((uc0==0)&&(uc1==0)){
        if(pf0->TrackNum<=pf1->TrackNum){
          return(false);
          }else{
          return(true);
        }
      }
      }else{
      // ファイル名長さチェック
      if(uc0==0) return(false);
      if(uc1==0) return(true);
      // 文字比較
      if(uc0<uc1) return(false);
      if(uc0>uc1) return(true);
    }
    
    puc0++; puc1++;
  }
  return(false);
}

void FileListSort(void)
{
  if(FileCount<2) return;
  
  u32 topdata[FileMaxCount];
  
  for(int idx=0;idx<FileCount;idx++){
    u32 uc0=(u32)pFile[idx].Filename[0];
    u32 uc1=(u32)pFile[idx].Filename[1];
    if(((u32)'A'<=uc0)&&(uc0<=(u32)'Z')) uc0+=0x20;
    if(((u32)'A'<=uc1)&&(uc1<=(u32)'Z')) uc1+=0x20;
    topdata[idx]=(uc0<<16) | uc1;
  }
  
  for(int idx0=0;idx0<FileCount-1;idx0++){
    for(int idx1=idx0+1;idx1<FileCount;idx1++){
      TFile *pf0=&pFile[idx0];
      TFile *pf1=&pFile[idx1];
      bool SwapFlag=false;
      
      if(pf0->FileType!=pf1->FileType){
        if(pf0->FileType==FT_File) SwapFlag=true;
        }else{
        if(topdata[idx0]>=topdata[idx1]){
          if(isSwapFile(pf0,pf1)==true) SwapFlag=true;
        }
      }
      
      if(SwapFlag==true){
        TFile ftemp=*pf0;
        *pf0=*pf1;
        *pf1=ftemp;
        
        u32 tmp=topdata[idx0];
        topdata[idx0]=topdata[idx1];
        topdata[idx1]=tmp;
      }
    }
  }
  
}

// ------------------------------------------------------------------------

static void Shuffle_Refresh(int TopIndex)
{
  ShuffleCount=FileCount;
  
  pShuffleList[0]=TopIndex;
  for(int idx=1;idx<ShuffleCount;idx++){
    pShuffleList[idx]=-1;
  }
  
  for(int idx=0;idx<ShuffleCount;idx++){
    if(idx!=TopIndex){
      int r=(rand()%ShuffleCount)+1;
      int fidx=0;
      while(r!=0){
        fidx=(fidx+1)%ShuffleCount;
        if(pShuffleList[fidx]==-1) r--;
      }
//      _consolePrintf("ref%d,%d\n",fidx,idx);
      pShuffleList[fidx]=idx;
    }
  }
  
}

void Shuffle_Clear(void)
{
  ShuffleCount=-1;
}

int Shuffle_GetNextIndex(int LastIndex)
{
  if(ShuffleCount!=FileCount) Shuffle_Refresh(LastIndex);
  
  for(int idx=0;idx<ShuffleCount;idx++){
    if(pShuffleList[idx]==LastIndex){
      if(idx==(ShuffleCount-1)){
        ShuffleCount=-1;
        return(-1);
        }else{
        return(pShuffleList[idx+1]);
      }
    }
  }
  
  _consolePrintf("Shuffle_GetNextIndex(%d):not found next index?\n",LastIndex);
  for(int idx=0;idx<ShuffleCount;idx++){
    _consolePrintf("%d,",pShuffleList[idx]);
  }
  
  ShowLogHalt();
  return(-1);
}

int Shuffle_GetPrevIndex(int LastIndex)
{
  if(ShuffleCount!=FileCount) Shuffle_Refresh(LastIndex);
  
  for(int idx=0;idx<ShuffleCount;idx++){
    if(pShuffleList[idx]==LastIndex){
      if(idx==0){
        ShuffleCount=-1;
        return(-1);
        }else{
        return(pShuffleList[idx-1]);
      }
    }
  }
  
  _consolePrintf("Shuffle_GetNextIndex(%d):not found next index?\n",LastIndex);
  for(int idx=0;idx<ShuffleCount;idx++){
    _consolePrintf("%d,",pShuffleList[idx]);
  }
  
  ShowLogHalt();
  return(-1);
}

int Normal_GetNextIndex(int LastIndex)
{
  LastIndex++;
  if(LastIndex==FileCount) return(-1);
  
  return(LastIndex);
}

int Normal_GetPrevIndex(int LastIndex)
{
  if(LastIndex==0) return(-1);
  
  LastIndex--;
  
  return(LastIndex);
}

