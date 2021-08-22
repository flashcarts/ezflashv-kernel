
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NDS.h>

#include "cstream_fs.h"

#include "filesys.h"

#include "_console.h"

CStreamFS::CStreamFS(const int _file):CStream(NULL,0),file(_file)
{
  FileSys_fseek(file,0,SEEK_END);
  size=FileSys_ftell(file);
  FileSys_fseek(file,0,SEEK_SET);
  ofs=0;
}

CStreamFS::~CStreamFS(void)
{
}

int CStreamFS::GetOffset(void) const
{
  return(ofs);
}

void CStreamFS::SetOffset(int _ofs)
{
  ofs=_ofs;
  if(size<ofs) ofs=size;
  FileSys_fseek(file,ofs,SEEK_SET);
}

int CStreamFS::GetSize(void) const
{
  return(size);
}

void CStreamFS::OverrideSize(int _size)
{
  size=_size;
}

bool CStreamFS::eof(void) const
{
  if(ofs==size){
    return(true);
    }else{
    return(false);
  }
}

u8 CStreamFS::Readu8(void)
{
  if(eof()==true) return(0);
  
  u8 data=0;
  
  FileSys_fread(&data,1,1,file);
  ofs++;
  
  return(data);
}

u16 CStreamFS::Readu16(void)
{
  if(eof()==true) return(0);
  
  u16 data;
  
  FileSys_fread(&data,2,1,file);
  ofs+=2;
  
  return(data);
  
  data=(u16)Readu8();
  data=data | ((u16)Readu8() << 8);
  
  return(data);
}

u32 CStreamFS::Readu32(void)
{
  if(eof()==true) return(0);
  
  u32 data;
  
  FileSys_fread(&data,4,1,file);
  ofs+=4;
  
  return(data);
  
  data=(u32)Readu8();
  data=data | ((u32)Readu8() << 8);
  data=data | ((u32)Readu8() << 16);
  data=data | ((u32)Readu8() << 24);
  
  return(data);
}

int CStreamFS::ReadBuffer(void *_dstbuf,const int _readsize)
{
  if(eof()==true) return(0);
  
  int readsize;
  
  if((ofs+_readsize)<=size){
    readsize=_readsize;
    }else{
    readsize=size-ofs;
    if(readsize<0) readsize=0;
  }
  
  if(readsize!=0){
    readsize=FileSys_fread(_dstbuf,1,readsize,file);
  }
  
  ofs+=readsize;
  
  return(readsize);
}

int CStreamFS::ReadBuffer32bit(void *_dstbuf,const int _readsize)
{
  if(eof()==true) return(0);
  
  int readsize;
  
  if((ofs+_readsize)<=size){
    readsize=_readsize;
    }else{
    readsize=size-ofs;
    if(readsize<0) readsize=0;
  }
  
  if(readsize!=0){
    readsize=FileSys_fread_fast(_dstbuf,1,readsize,file);
  }
  
  ofs+=readsize;
  
  return(readsize);
}

