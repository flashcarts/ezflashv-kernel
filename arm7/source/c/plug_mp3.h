
#ifndef _MP3_h
#define _MP3_h

#define MP3Title "libmad - MPEG audio decoder library"

extern "C" {

void MP3_SetFunc_consolePrintf(u32 adr);

bool StartMP3(int _FileHandle);
u32 UpdateMP3(s16 *lbuf,s16 *rbuf);
void FreeMP3(void);

s32 MP3_GetFileSize(void);
s32 MP3_GetFileOffset(void);
void MP3_SetFileOffset(s32 ofs);

u32 MP3_GetBitRate(void);
u32 MP3_GetChannelCount(void);
u32 MP3_GetSampleRate(void);
u32 MP3_GetSamplePerFrame(void);

int MP3_GetInfoIndexCount(void);
bool MP3_GetInfoStrL(int idx,char *str,int len);
bool MP3_GetInfoStrW(int idx,UnicodeChar *str,int len);
bool MP3_GetInfoStrUTF8(int idx,char *str,int len);

}

#endif
