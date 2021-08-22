
#include <NDS.h>

#if 1
void YUV420toBGR15_Init(void)
{
}

void YUV420toBGR15_CopyFull(const u8 *YBuf,const u8 *UBuf,const u8 *VBuf,u16 *_FrameBuf)
{
}

#else

// all format
//#define trofs (197)
//#define tgofs (132)
//#define tbofs (232)

// SMPTE 170M format optimized.
//#define trofs (175)
//#define tgofs (132)
//#define tbofs (221)

// no limitter
#define trofs (0)
#define tgofs (0)
#define tbofs (0)

static u16 itable_rbuf[256+trofs*2];
static u16 itable_gbuf[256+tgofs*2];
static u16 itable_bbuf[256+tbofs*2];
static u16 *itable_rV[256];
static u16 *itable_gU[256];
static u8 itable_gV[256];
static u16 *itable_bU[256];

#define ScreenWidth (256)
#define ScreenHeight (192)

#define FrameWidth (256)
#define FrameHeight (192)
#define MCUXCount (FrameWidth/8)
#define Ydiv2 (FrameHeight/2/8)

//static u16 DrawBuf[ScreenHeight*ScreenWidth];

enum EMPGPixelFormat {PF_RGB15=0,PF_RGB18=1,PF_RGB21=2,PF_RGB24=3,PF_ENUMCOUNT=4};
const EMPGPixelFormat PixelFormat=PF_RGB21;

static int div_round (int dividend, int divisor)
{
    if (dividend > 0)
        return (dividend + (divisor>>1)) / divisor;
    else
        return -((-dividend + (divisor>>1)) / divisor);
}

void YUV420toBGR15_Init(void)
{
  u8 table_Y[1024];
  
  u32 bs;
  switch(PixelFormat){
    case PF_RGB15: bs=0; break;
    case PF_RGB18: bs=1; break;
    case PF_RGB21: bs=2; break;
    case PF_RGB24: bs=3; break;
    default: bs=3; break;
  }
  
  for (int i=0;i<1024;i++){
    int j=((76309*(i-384-16))+32768) >> 16;
    table_Y[i]=(j<0) ? 0 : ((j>255) ? 255 : j);
    table_Y[i]=table_Y[i]>>bs;
    if(0x1f<=table_Y[i]) table_Y[i]=0x1f;
  }

  for (int i=-trofs;i<256+trofs;i++){
    itable_rbuf[i+trofs]=table_Y[i+384];
  }
  
  for (int i=-tgofs;i<256+tgofs;i++){
    itable_gbuf[i+tgofs]=table_Y[i+384] << 5;
  }
  
  for (int i=-tbofs;i<256+tbofs;i++){
    itable_bbuf[i+tbofs]=(table_Y[i+384] << 10) | (1 << 15); // transmask
  }
  
  // for SMPTE 170M only
  const int crv=104597; // Inverse_Table_6_9[matrix_coefficients][0];
  const int cbu=132201; // Inverse_Table_6_9[matrix_coefficients][1];
  const int cgu=-25675; // -Inverse_Table_6_9[matrix_coefficients][2];
  const int cgv=-53279; // -Inverse_Table_6_9[matrix_coefficients][3];
  
  for (int i=0;i<256;i++) {
    itable_rV[i]=&itable_rbuf[trofs+div_round(crv*(i-128), 76309)];
    itable_gU[i]=&itable_gbuf[tgofs+div_round(cgu*(i-128), 76309)-89];
    itable_gV[i]=                   div_round(cgv*(i-128), 76309)+89;
    itable_bU[i]=&itable_bbuf[tbofs+div_round(cbu*(i-128), 76309)];
  }
}

#define RGB(i,aofs) { \
  const u32 U=pubuf[i],V=pvbuf[i]; \
  const u16 *r,*g,*b; \
  r=itable_rV[V]; \
  g=itable_gU[U]+itable_gV[V]; \
  b=itable_bU[U]; \
    { \
      u32 c=0; \
      { const u32 Y=pybuf[ofs+aofs+0]; c=r[Y] | g[Y] | b[Y]; } \
      { const u32 Y=pybuf[ofs+aofs+1]; c|=(r[Y] | g[Y] | b[Y])<<16; } \
      *(u32*)&pdst[ofs+aofs]=c; \
    } \
    ofs+=ScreenWidth; \
    { \
      u32 c=0; \
      { const u32 Y=pybuf[ofs+aofs+0]; c=r[Y] | g[Y] | b[Y]; } \
      { const u32 Y=pybuf[ofs+aofs+1]; c|=(r[Y] | g[Y] | b[Y])<<16; } \
      *(u32*)&pdst[ofs+aofs]=c; \
    } \
    ofs-=ScreenWidth; \
}

void YUV420toBGR15_CopyFull(const u8 *YBuf,const u8 *UBuf,const u8 *VBuf,u16 *_FrameBuf)
{
  u16 *pdstto=&_FrameBuf[0];
  
  int dy=Ydiv2;
  
  do {
    u16 dstbuf[ScreenWidth*2];
    u16 *pdst=&dstbuf[ScreenWidth*0];
    
    u8 ybuf[FrameWidth*2];
    DMA2_SRC = (u32)YBuf;
    DMA2_DEST = (u32)ybuf;
    DMA2_CR=(DMA_32_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_INC | DMA_DST_INC)+(FrameWidth*2/4);
//    swiFastCopy((void*)YBuf,ybuf,(FrameWidth*2/4) | COPY_MODE_COPY);
    
    const u8 *pybuf=&YBuf[FrameWidth*0];
    
    u8 ubuf[FrameWidth/2];
    DMA1_SRC = (u32)UBuf;
    DMA1_DEST = (u32)ubuf;
    DMA1_CR=(DMA_32_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_INC | DMA_DST_INC)+(FrameWidth/2/4);
//    swiFastCopy((void*)UBuf,ubuf,(FrameWidth/2/4) | COPY_MODE_COPY);
    const u8 *pubuf=ubuf;
    
    u8 vbuf[FrameWidth/2];
    DMA0_SRC = (u32)VBuf;
    DMA0_DEST = (u32)vbuf;
    DMA0_CR=(DMA_32_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_INC | DMA_DST_INC)+(FrameWidth/2/4);
//    swiFastCopy((void*)VBuf,vbuf,(FrameWidth/2/4) | COPY_MODE_COPY);
    const u8 *pvbuf=vbuf;
    
    int dx=MCUXCount;
    
    u32 ofs=0;
    
    do {
      RGB(0,0);
      RGB(1,2);
      RGB(2,4);
      RGB(3,6);
      ofs+=8;
    } while (--dx);
    
    DMA3_SRC = (u32)dstbuf;
    DMA3_DEST = (u32)pdstto;
    DMA3_CR=(DMA_32_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_INC | DMA_DST_INC)+(ScreenWidth*2*2/4);
//    swiFastCopy(dstbuf,pdstto,(ScreenWidth*2*2/4) | COPY_MODE_COPY);
    
    YBuf+=FrameWidth*2;
    UBuf+=FrameWidth/2;
    VBuf+=FrameWidth/2;
    pdstto+=(ScreenWidth+(ScreenWidth-FrameWidth))*2;
  } while (--dy);
}

#endif
