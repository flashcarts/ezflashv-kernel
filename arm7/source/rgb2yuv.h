
#ifndef _rgb2yuv_h
#define _rgb2yuv_h

#define MP2Title "libmad - MPEG audio decoder library"

extern void YUV420toBGR15_Init(void);
extern void YUV420toBGR15_CopyFull(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf);
/*
extern void YUV420toBGR15_CopyCross0(u8 *_YBuf,u8 *_UBuf,u8 *_VBuf,u16 *_FrameBuf);
extern void YUV420toBGR15_CopyCross1(u8 *_YBuf,u8 *_UBuf,u8 *_VBuf,u16 *_FrameBuf);
extern void YUV420toBGR15_CopyLU(u8 *_YBuf,u8 *_UBuf,u8 *_VBuf,u16 *_FrameBuf);
extern void YUV420toBGR15_CopyRU(u8 *_YBuf,u8 *_UBuf,u8 *_VBuf,u16 *_FrameBuf);
extern void YUV420toBGR15_CopyLD(u8 *_YBuf,u8 *_UBuf,u8 *_VBuf,u16 *_FrameBuf);
extern void YUV420toBGR15_CopyRD(u8 *_YBuf,u8 *_UBuf,u8 *_VBuf,u16 *_FrameBuf);
*/

extern void YUV420toBGR15_CopyFull_fix(const u8 *_YBuf,const u8 *_UBuf,const u8 *_VBuf,u16 *_FrameBuf);

#endif
