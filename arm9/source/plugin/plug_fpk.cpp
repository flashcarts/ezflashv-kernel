
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NDS.h>

#include "../_console.h"
#include "../memtool.h"
#include "../filesys.h"
#include "../cstream_fs.h"

#include "../FontPro.h"
#include "../unicode.h"

static CStreamFS *pCStreamFS_FPK=NULL;

bool StartFPK(int FileHandle)
{
  pCStreamFS_FPK=new CStreamFS(FileHandle);
  
  {
    u32 ID;
    ID=pCStreamFS_FPK->Readu32();
    
    if(ID!=0x006b7066){ // fpk\0
      _consolePrintf("error MagicID != fpk\n");
      return(false);
    }
  }
  
  u32 ankofs=pCStreamFS_FPK->Readu32();
  u32 anksize=pCStreamFS_FPK->Readu32();
  u8 *pankbuf=NULL;
  u32 l2uofs=pCStreamFS_FPK->Readu32();
  u32 l2usize=pCStreamFS_FPK->Readu32();
  u8 *pl2ubuf=NULL;
  u32 fonofs=pCStreamFS_FPK->Readu32();
  u32 fonsize=pCStreamFS_FPK->Readu32();
  u8 *pfonbuf=NULL;
  
  Unicode_Free();
  FontPro_Free();
  
  pCStreamFS_FPK->SetOffset(ankofs);
  pankbuf=(u8*)safemalloc(anksize);
  pCStreamFS_FPK->ReadBuffer(pankbuf,anksize);
  _consolePrintf("ank 0x%x(%dbyte)\n",(u32)pankbuf,anksize);
  
  pCStreamFS_FPK->SetOffset(l2uofs);
  pl2ubuf=(u8*)safemalloc(l2usize);
  pCStreamFS_FPK->ReadBuffer(pl2ubuf,l2usize);
  _consolePrintf("l2u 0x%x(%dbyte)\n",(u32)pl2ubuf,l2usize);
  
  pCStreamFS_FPK->SetOffset(fonofs);
  pfonbuf=(u8*)safemalloc(fonsize);
  pCStreamFS_FPK->ReadBuffer(pfonbuf,fonsize);
  _consolePrintf("fon 0x%x(%dbyte)\n",(u32)pfonbuf,fonsize);
  
  Unicode_Init(pankbuf,pl2ubuf);
  FontPro_Init(pfonbuf);
  
  return(true);
}

void FreeFPK(void)
{
  if(pCStreamFS_FPK!=NULL){
    delete pCStreamFS_FPK; pCStreamFS_FPK=NULL;
  }
}

int FPK_GetInfoIndexCount(void)
{
  return(1);
}

bool FPK_GetInfoStrL(int idx,char *str,int len)
{
  if(idx==0){
    snprintf(str,len,"Portable FontPackage reloaded.");
    return(true);
  }
  
  return(false);
}

bool FPK_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool FPK_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

