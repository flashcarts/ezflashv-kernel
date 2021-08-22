
#ifndef cstream_fs_h
#define cstream_fs_h

#include <stdlib.h>
#include <NDS.h>

#include "cstream.h"

class CStreamFS: public CStream
{
  const int file;
  CStreamFS(const CStreamFS&);
  CStreamFS& operator=(const CStreamFS&);
public:
  CStreamFS(const int _file);
  ~CStreamFS(void);
  int GetOffset(void) const;
  void SetOffset(int _ofs);
  int GetSize(void) const;
  void OverrideSize(int _size);
  bool eof(void) const;
  u8 Readu8(void);
  u16 Readu16(void);
  u32 Readu32(void);
  int ReadBuffer(void *_dstbuf,const int _readsize);
  // fast request 16bit aligned file position and write buffer
  int ReadBuffer32bit(void *_dstbuf,const int _readsize);
};

#endif

