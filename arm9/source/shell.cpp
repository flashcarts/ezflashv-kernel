
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>
#include <NDS/ARM9/CP15.h>

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "shell.h"

#include "memtool.h"

#include "filesys.h"
#include "cimfs.h"
#include "gba_nds_fat.h"
#include "mediatype.h"
#include <gbfs.h>

u32 Shell_VolumeType=VT_NULL;

#define FAT_FT_END (0)
#define FAT_FT_FILE (1)
#define FAT_FT_DIR (2)

static bool LastEnable=false;
static u32 LastVolumeType;
//static char LastPathName[MAX_PATH_LENGTH];

char FATShellPath[256+1];
char FATShellPluginPath[256+1];

__attribute__((noinline)) bool Shell_FindShellPath_FindIns(char *pBasePath)
{
  char toppath[2]={0,0};
  if(pBasePath==NULL) pBasePath=toppath;
  
  _consolePrintf("find.%s\n",pBasePath);
  
  {
    char fn[256];
    snprintf(fn,256,"%s/global.ini",pBasePath);
    FAT_FILE *pf=FAT_fopen(fn,"r");
    if(pf!=NULL){
      FAT_fclose(pf);
      snprintf(FATShellPath,256,"%s",pBasePath);
      snprintf(FATShellPluginPath,256,"%s/plugin",pBasePath);
      return(true);
    }
  }
  
  if(FAT_CWD(pBasePath)==false) return(false);
  
  u32 PathCount=0;
  char *Paths[128];
  
#define FindPathMax (128)
  
  for(u32 idx=0;idx<FindPathMax;idx++){
    Paths[idx]=(char*)safemalloc(256);
  }
  
  {
    u32 FAT_FileType;
    char fn[256];
    FAT_FileType=FAT_FindFirstFile(fn);
    
    while(FAT_FileType!=FAT_FT_END){ cwl();
      switch(FAT_FileType){ cwl();
        case FAT_FT_DIR: {
          if((strcmp(fn,".")!=0)&&(strcmp(fn,"..")!=0)){
            if(PathCount<FindPathMax){
              strncpy(Paths[PathCount],fn,256);
              PathCount++;
            }
          }
        } break;
        case FAT_FT_FILE: {
        } break;
      }
      
      FAT_FileType=FAT_FindNextFile(fn);
    }
  }
  
  bool res=false;
  
  for(u32 idx=0;idx<PathCount;idx++){
    char fn[256];
    snprintf(fn,256,"%s/%s",pBasePath,Paths[idx]);
    if(Shell_FindShellPath_FindIns(fn)==true){
      res=true;
      break;
    }
  }
  
  for(u32 idx=0;idx<FindPathMax;idx++){
    safefree(Paths[idx]); Paths[idx]=NULL;
  }
  
#undef FindPathMax
  
  return(res);
}

bool Shell_FindShellPath(void)
{
  strncpy(FATShellPath,"/shell",256);
  strncpy(FATShellPluginPath,"/shell/plugin",256);
  
  if(Shell_VolumeType!=VT_MPCF) return(true);
  
  {
    if(FAT_CWD("/shell")==true){
      FAT_FILE *pf=FAT_fopen("/shell/global.ini","r");
      if(pf!=NULL){
        FAT_fclose(pf);
        _consolePrintf("found shell=%s\n",FATShellPath);
        return(true);
      }
    }
  }
  
  _consolePrintf("find path...\n");
  
  if(Shell_FindShellPath_FindIns(NULL)==true){
    _consolePrintf("found shell=%s\n",FATShellPath);
    return(true);
  }
  
  _consolePrintf("can not found shell folder.\n");
  
  return(false);
}

static inline void Shell_FSBackup(void)
{return;
  if(Shell_VolumeType==FileSys_VolumeType){
    LastEnable=false;
    return;
  }
  _consolePrintf("b%d,%d\n",Shell_VolumeType,FileSys_VolumeType);
  
  LastEnable=true;
  
  LastVolumeType=FileSys_VolumeType;
//  strcpy(LastPathName,FileSys_PathName);
  FileSys_FreeInterface();
  
  FileSys_VolumeType=Shell_VolumeType;
  FileSys_InitInterface();
}

static inline void Shell_FSRestore(void)
{return;
  if(LastEnable==false) return;
  _consolePrintf("r%d,%d\n",Shell_VolumeType,FileSys_VolumeType);
  
  FileSys_FreeInterface();
  
  FileSys_VolumeType=LastVolumeType;
  FileSys_InitInterface();
//  FileSys_ChangePath(LastPathName);
}

void Shell_AutoDetect(void)
{
  Shell_VolumeType=VT_NULL;
  
  FileSys_VolumeType=VT_NULL;
  FileSys_InitInterface();
  
  FileSys_VolumeType=VT_EXFS;
  if(FileSys_InitInterface()==true){
    Shell_VolumeType=FileSys_VolumeType;
    _consolePrintf("Detected adapter is EXFS.\n");
    return;
    }else{
    _consolePrintf("not found EXFS.\n");
  }
  
  FileSys_VolumeType=VT_MPCF;
  if(FileSys_InitInterface()==true){
    Shell_VolumeType=FileSys_VolumeType;
    _consolePrintf("Detected adapter is [%s]\n",DIMediaName);
//    while(1);
    return;
    }else{
    _consolePrintf("not found MPCF.\n");
  }
  
/*
  FileSys_VolumeType=VT_GBFS;
  if(FileSys_InitInterface()==true){
    Shell_VolumeType=FileSys_VolumeType;
    _consolePrintf("Detected adapter is GBFS.\n");
    return;
    }else{
    _consolePrintf("not found GBFS.\n");
  }
*/
  
/*
  FileSys_VolumeType=VT_IMFS;
  if(FileSys_InitInterface()==true){
    Shell_VolumeType=FileSys_VolumeType;
    _consolePrintf("Detected adapter is IMFS.\n");
    return;
    }else{
    _consolePrintf("not found IMFS.\n");
  }
*/
  
  _consolePrintf("Not found adapter.\n");
}

void Shell_AutoDetect_EXFS(void)
{
  FileSys_VolumeType=VT_EXFS;
  FileSys_InitInterface();
  Shell_VolumeType=FileSys_VolumeType;
  _consolePrintf("Detected adapter is EXFS.\n");
}

static void Shell_ReadFile_IMFS(const char *fn,void **pbuf,int *psize)
{
  s32 PathIndex=pIMFS->GetPathIndex("/shell");
  s32 FileIndex=pIMFS->GetIndexFromFilename(PathIndex,fn);
  
  if((PathIndex!=-1)&&(FileIndex!=-1)){
    *psize=pIMFS->GetFileDataSizeFromIndex(PathIndex,FileIndex);
    *pbuf=(void*)safemalloc(*psize);
    
    if(pIMFS->GetFileTypeFromIndex(PathIndex,FileIndex)==FT_FileFlat){ cwl();
      u8 *data=pIMFS->GetFileDirectDataFromIndex(PathIndex,FileIndex);
      if(data!=NULL){
        MemCopy8CPU(data,*pbuf,*psize);
        return;
      }
      }else{ cwl();
      pIMFS->GetFileDataFromIndex(PathIndex,FileIndex,(u8*)*pbuf);
      return;
    }
  }
  
  _consolePrintf("skip //IMFS/shell/%s\n",fn);
  
  *psize=0;
  *pbuf=NULL;
}

void Shell_ReadFile(const char *fn,void **pbuf,int *psize)
{
  Shell_FSBackup();
  
  switch(Shell_VolumeType){ cwl();
    case VT_NULL: {
    } break;
    case VT_IMFS: case VT_EXFS: { cwl();
      CIMFS *pFS;
      if(FileSys_VolumeType==VT_IMFS){
        pFS=pIMFS;
        }else{
        pFS=pEXFS;
      }
      
      s32 PathIndex=pFS->GetPathIndex("/shell");
      s32 FileIndex=pFS->GetIndexFromFilename(PathIndex,fn);
      
      if((PathIndex!=-1)&&(FileIndex!=-1)){
        *psize=pFS->GetFileDataSizeFromIndex(PathIndex,FileIndex);
        *pbuf=(void*)safemalloc(*psize);
        
        if(pFS->GetFileTypeFromIndex(PathIndex,FileIndex)==FT_FileFlat){ cwl();
          u8 *data=pFS->GetFileDirectDataFromIndex(PathIndex,FileIndex);
          if(data!=NULL){
            MemCopy8CPU(data,*pbuf,*psize);
            Shell_FSRestore();
            return;
          }
          }else{ cwl();
          pFS->GetFileDataFromIndex(PathIndex,FileIndex,(u8*)*pbuf);
          Shell_FSRestore();
          return;
        }
      }
    } break;
    case VT_MPCF: { cwl();
      FAT_FILE *fh;
      {
        char fullfn[256];
        snprintf(fullfn,256,"%s/%s",FATShellPath,fn);
        fh=FAT_fopen(fullfn,"r");
      }
      if(fh!=NULL){
        FAT_fseek(fh,0,SEEK_END);
        *psize=FAT_ftell(fh);
        FAT_fseek(fh,0,SEEK_SET);
        *pbuf=(void*)safemalloc(*psize);
        
        FAT_fread(*pbuf,1,*psize,fh);
        FAT_fclose(fh);
        Shell_FSRestore();
        return;
      }
    } break;
    case VT_GBFS: { cwl();
      void *data;
      {
        extern const GBFS_FILE *gbfsfile;
        data=(void*)gbfs_get_obj(gbfsfile,fn,(u32*)psize);
      }
      
      if(data!=NULL){
        *pbuf=(void*)safemalloc(*psize);
        MemCopy8CPU(data,*pbuf,*psize);
        Shell_FSRestore();
        return;
      }
    } break;
  }
  
  Shell_ReadFile_IMFS(fn,pbuf,psize);
  
  Shell_FSRestore();
}

static bool Shell_EnumMSP_CheckExt(char *fn)
{
  u32 cnt=0;
  
  u32 DotPos=0;
  cnt=0;
  while(fn[cnt]!=0){ cwl();
    if(fn[cnt]=='.') DotPos=cnt;
    cnt++;
  }
  
  if(DotPos==0) return(false);
  
  char c;
  
  c=fn[DotPos+1];
  if((0x41<=c)&&(c<0x5a)) c+=0x20;
  if(c!='m') return(false);
  
  c=fn[DotPos+2];
  if((0x41<=c)&&(c<0x5a)) c+=0x20;
  if(c!='s') return(false);
  
  c=fn[DotPos+3];
  if((0x41<=c)&&(c<0x5a)) c+=0x20;
  if(c!='p') return(false);
  
  return(true);
}

char **Shell_EnumMSP(void)
{
  Shell_FSBackup();
  
  switch(Shell_VolumeType){ cwl();
    case VT_NULL: {
    } break;
    case VT_IMFS:
    case VT_EXFS: { cwl();
      CIMFS *pFS;
      if(FileSys_VolumeType==VT_IMFS){
        pFS=pIMFS;
        }else{
        pFS=pEXFS;
      }
      
      s32 PathIndex=pFS->GetPathIndex("/shell/plugin");
      s32 FileCount=pFS->GetFileCount(PathIndex);
      
      if((PathIndex!=-1)&&(FileCount<0)){
        Shell_FSRestore();
        return(NULL);
      }
      
      char **FileList=(char**)malloc((FileCount+1)*4);
      int StoreIndex=0;
      
      for(s32 idx=0;idx<FileCount;idx++){ cwl();
        if(pFS->GetFileTypeFromIndex(PathIndex,idx)==FT_FileFlat){
          char *fn=pFS->GetFilenameFromIndex(PathIndex,idx);
          if(Shell_EnumMSP_CheckExt(fn)==true){
            FileList[StoreIndex]=(char*)malloc(strlen(fn)+1);
            strcpy(FileList[StoreIndex],fn);
            StoreIndex++;
          }
        }
      }
      FileList[StoreIndex]=NULL;
      Shell_FSRestore();
      return(FileList);
    } break;
    case VT_MPCF: { cwl();
      if(FAT_CWD(FATShellPluginPath)==false){
        Shell_FSRestore();
        return(NULL);
      }
      
      char fn[MAX_FILENAME_LENGTH];
      u32 FAT_FileType;
      
      char **FileList=(char**)malloc((128+1)*4);
      int StoreIndex=0;
      
      FAT_FileType=FAT_FindFirstFile(fn);
      
      while(FAT_FileType!=FAT_FT_END){ cwl();
        switch(FAT_FileType){ cwl();
          case FAT_FT_DIR: {
          } break;
          case FAT_FT_FILE: {
            if(Shell_EnumMSP_CheckExt(fn)==true){
              FileList[StoreIndex]=(char*)malloc(strlen(fn)+1);
              strcpy(FileList[StoreIndex],fn);
              _consolePrintf("%s\n",FileList[StoreIndex]);
              StoreIndex++;
            }
          } break;
        }
        
        FAT_FileType=FAT_FindNextFile(fn);
      }
      
      FileList[StoreIndex]=NULL;
      Shell_FSRestore();
      return(FileList);
    } break;
    case VT_GBFS: { cwl();
    } break;
  }
  
  Shell_FSRestore();
  return(NULL);
}

bool Shell_ReadHeadMSP(char *fn,void *buf,int size)
{
  Shell_FSBackup();
  
  switch(Shell_VolumeType){ cwl();
    case VT_NULL: {
    } break;
    case VT_IMFS: case VT_EXFS: { cwl();
      CIMFS *pFS;
      if(FileSys_VolumeType==VT_IMFS){
        pFS=pIMFS;
        }else{
        pFS=pEXFS;
      }
      
      s32 PathIndex=pFS->GetPathIndex(FATShellPluginPath);
      s32 FileIndex=pFS->GetIndexFromFilename(PathIndex,fn);
      
      if((PathIndex!=-1)&&(FileIndex!=-1)){
        if(pFS->GetFileTypeFromIndex(PathIndex,FileIndex)==FT_FileFlat){ cwl();
          u8 *data=pFS->GetFileDirectDataFromIndex(PathIndex,FileIndex);
          if(data!=NULL){
            MemCopy8CPU(data,buf,size);
            Shell_FSRestore();
            return(true);
          }
          }else{ cwl();
          u32 srcsize=pFS->GetFileDataSizeFromIndex(PathIndex,FileIndex);
          u8 *srcbuf=(u8*)safemalloc(srcsize);
          pFS->GetFileDataFromIndex(PathIndex,FileIndex,srcbuf);
          MemCopy8CPU(srcbuf,buf,size);
          safefree(srcbuf); srcbuf=NULL;
          Shell_FSRestore();
          return(true);
        }
      }
    } break;
    case VT_MPCF: { cwl();
      FAT_FILE *fh;
      
      {
        char fullfn[256];
        snprintf(fullfn,256,"%s/%s",FATShellPluginPath,fn);
        fh=FAT_fopen(fullfn,"r");
      }
      if(fh!=NULL){
        FAT_fread(buf,1,size,fh);
        FAT_fclose(fh);
        Shell_FSRestore();
        return(true);
      }
    } break;
    case VT_GBFS: { cwl();
    } break;
  }
  
  Shell_FSRestore();
  return(false);
}

void Shell_ReadMSP(const char *fn,void **pbuf,int *psize)
{
  *pbuf=NULL;
  *psize=0;
  
  Shell_FSBackup();
  
  switch(Shell_VolumeType){ cwl();
    case VT_NULL: {
    } break;
    case VT_IMFS: case VT_EXFS: { cwl();
      CIMFS *pFS;
      if(FileSys_VolumeType==VT_IMFS){
        pFS=pIMFS;
        }else{
        pFS=pEXFS;
      }
      
      s32 PathIndex=pFS->GetPathIndex("/shell/plugin");
      s32 FileIndex=pFS->GetIndexFromFilename(PathIndex,fn);
      
      if((PathIndex!=-1)&&(FileIndex!=-1)){
        *psize=pFS->GetFileDataSizeFromIndex(PathIndex,FileIndex);
        *pbuf=(void*)safemalloc(*psize+1);
        ((char*)*pbuf)[*psize]=0;
        
        if(pFS->GetFileTypeFromIndex(PathIndex,FileIndex)==FT_FileFlat){ cwl();
          u8 *data=pFS->GetFileDirectDataFromIndex(PathIndex,FileIndex);
          if(data!=NULL){
            MemCopy8CPU(data,*pbuf,*psize);
            Shell_FSRestore();
            return;
          }
          }else{ cwl();
          pFS->GetFileDataFromIndex(PathIndex,FileIndex,(u8*)*pbuf);
          Shell_FSRestore();
          return;
        }
      }
    } break;
    case VT_MPCF: { cwl();
      FAT_FILE *fh;
      
      {
        char fullfn[256];
        snprintf(fullfn,256,"%s/%s",FATShellPluginPath,fn);
        fh=FAT_fopen(fullfn,"r");
      }
      
      if(fh!=NULL){
        FAT_fseek(fh,0,SEEK_END);
        *psize=FAT_ftell(fh);
        FAT_fseek(fh,0,SEEK_SET);
        *pbuf=(void*)safemalloc(*psize+1);
        ((char*)*pbuf)[*psize]=0;
        
        FAT_fread(*pbuf,1,*psize,fh);
        FAT_fclose(fh);
        Shell_FSRestore();
        return;
      }
    } break;
    case VT_GBFS: { cwl();
    } break;
  }
  
  Shell_FSRestore();
}

int Shell_OpenMSP(const char *fn)
{
  Shell_FSBackup();
  
  switch(Shell_VolumeType){ cwl();
    case VT_NULL: {
    } break;
    case VT_IMFS: case VT_EXFS: { cwl();
      CIMFS *pFS;
      if(FileSys_VolumeType==VT_IMFS){
        pFS=pIMFS;
        }else{
        pFS=pEXFS;
      }
      
      s32 PathIndex=pFS->GetPathIndex("/shell/plugin");
      s32 FileIndex=pFS->GetIndexFromFilename(PathIndex,fn);
      
      if((PathIndex!=-1)&&(FileIndex!=-1)){
        u32 size=pFS->GetFileDataSizeFromIndex(PathIndex,FileIndex);
        
        if(pFS->GetFileTypeFromIndex(PathIndex,FileIndex)==FT_FileFlat){ cwl();
          u8 *pdata=pFS->GetFileDirectDataFromIndex(PathIndex,FileIndex);
          if(pdata!=NULL){
/*
            u8 *pbuf=(u8*)safemalloc(size);
            MemCopy8CPU(pdata,pbuf,size);
            int fh=FileSys_fopen_DirectMapping(Shell_VolumeType,true,pbuf,size,0,0);
*/
            int fh=FileSys_fopen_DirectMapping(Shell_VolumeType,false,pdata,size,0,0);
            Shell_FSRestore();
            return(fh);
          }
          }else{ cwl();
          u8 *pbuf=(u8*)safemalloc(size);
          pFS->GetFileDataFromIndex(PathIndex,FileIndex,pbuf);
          int fh=FileSys_fopen_DirectMapping(Shell_VolumeType,true,pbuf,size,0,0);
          Shell_FSRestore();
          return(fh);
        }
      }
    } break;
    case VT_MPCF: { cwl();
      FAT_FILE *fh;
      
      {
        char fullfn[256];
        snprintf(fullfn,256,"%s/%s",FATShellPluginPath,fn);
        fh=FAT_fopen(fullfn,"r");
      }
      
      if(fh!=NULL){
        int ifh=FileSys_fopen_DirectMapping(Shell_VolumeType,true,0,0,0,(int)fh);
        Shell_FSRestore();
        return(ifh);
      }
    } break;
    case VT_GBFS: { cwl();
    } break;
  }
  
  Shell_FSRestore();
  
  return(0);
}

void Shell_ReadMSPINI(const char *fn,char **pbuf,int *psize)
{
  *pbuf=NULL;
  *psize=0;
  
  int fnlen=strlen(fn);
  
  char *inifn=(char*)safemalloc(fnlen+1);
  
  strcpy(inifn,fn);
  
  inifn[fnlen-3]='i';
  inifn[fnlen-2]='n';
  inifn[fnlen-1]='i';
  
  Shell_ReadMSP(inifn,(void**)pbuf,psize);
  
  if(*pbuf!=NULL){
    safefree(inifn); inifn=NULL;
    return;
  }
  
  inifn[fnlen-3]='I';
  inifn[fnlen-2]='N';
  inifn[fnlen-1]='I';
  
  Shell_ReadMSP(inifn,(void**)pbuf,psize);
  
  if(*pbuf!=NULL){
    safefree(inifn); inifn=NULL;
    return;
  }
  
  return;
}

int Shell_OpenMSPBIN(const char *fn)
{
  int fnlen=strlen(fn);
  
  char *binfn=(char*)safemalloc(fnlen+1);
  
  int fh;
  
  strcpy(binfn,fn);
  
  binfn[fnlen-3]='b';
  binfn[fnlen-2]='i';
  binfn[fnlen-1]='n';
  
  fh=Shell_OpenMSP(binfn);
  
  if(fh!=0){
    safefree(binfn); binfn=NULL;
    return(fh);
  }
  
  binfn[fnlen-3]='B';
  binfn[fnlen-2]='I';
  binfn[fnlen-1]='N';
  
  fh=Shell_OpenMSP(binfn);
  
  if(fh!=0){
    safefree(binfn); binfn=NULL;
    return(fh);
  }
  
  return(0);
}

int Shell_OpenMSPData(const char *fn)
{
  return(Shell_OpenMSP(fn));
}

