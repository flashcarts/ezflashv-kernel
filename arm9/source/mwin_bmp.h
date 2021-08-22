
#define BI_RGB (0)
#define BI_RLE8 (1)
#define BI_RLE4 (2)
#define BI_Bitfields (3)

typedef struct {
  u8 bfType[2];
  u32 bfSize;
  u16 bfReserved1;
  u16 bfReserved2;
  u32 bfOffset;
  u32 biSize;
  u32 biWidth;
  u32 biHeight;
  u16 biPlanes;
  u16 biBitCount;
  u32 biCopmression;
  u32 biSizeImage;
  u32 biXPixPerMeter;
  u32 biYPixPerMeter;
  u32 biClrUsed;
  u32 biCirImportant;
  u8 *pPalette;
  u8 *pBitmap;
  
  u32 DataWidth;
} TBMPHeader;

static u16 GetVariable16bit(void *pb)
{
  u16 res;
  u8 *pb8=(u8*)pb;
  
  res=(u32)pb8[0] << 0;
  res+=(u32)pb8[1] << 8;
  
  return(res);
}

static u32 GetVariable32bit(void *pb)
{
  u32 res;
  u8 *pb8=(u8*)pb;
  
  res=(u32)pb8[0] << 0;
  res+=(u32)pb8[1] << 8;
  res+=(u32)pb8[2] << 16;
  res+=(u32)pb8[3] << 24;
  
  return(res);
}

static char *BMP_LoadErrorStr;

static bool GetBMPHeader(u8 *pb,TBMPHeader *pBMPHeader)
{
  if(pb==NULL){
    BMP_LoadErrorStr="SourceData Null.";
    return(false);
  }
  if(pBMPHeader==NULL){
    BMP_LoadErrorStr="pBMPHeader Null.";
    return(false);
  }
  
  pBMPHeader->bfType[0]=pb[0];
  pBMPHeader->bfType[1]=pb[1];
  pBMPHeader->bfSize=GetVariable32bit(&pb[2]);
  pBMPHeader->bfReserved1=GetVariable16bit(&pb[6]);
  pBMPHeader->bfReserved2=GetVariable16bit(&pb[8]);
  pBMPHeader->bfOffset=GetVariable32bit(&pb[10]);
  pBMPHeader->biSize=GetVariable32bit(&pb[14+0]);
  pBMPHeader->biWidth=GetVariable32bit(&pb[14+4]);
  pBMPHeader->biHeight=GetVariable32bit(&pb[14+8]);
  pBMPHeader->biPlanes=GetVariable16bit(&pb[14+12]);
  pBMPHeader->biBitCount=GetVariable16bit(&pb[14+14]);
  pBMPHeader->biCopmression=GetVariable32bit(&pb[14+16]);
  pBMPHeader->biSizeImage=GetVariable32bit(&pb[14+20]);
  pBMPHeader->biXPixPerMeter=GetVariable32bit(&pb[14+24]);
  pBMPHeader->biYPixPerMeter=GetVariable32bit(&pb[14+28]);
  pBMPHeader->biClrUsed=GetVariable32bit(&pb[14+32]);
  pBMPHeader->biCirImportant=GetVariable32bit(&pb[14+36]);
  
  pBMPHeader->pPalette=&pb[14+40];
  pBMPHeader->pBitmap=&pb[pBMPHeader->bfOffset];
  
  pBMPHeader->DataWidth=0;
  
  if((pBMPHeader->bfType[0]!='B')||(pBMPHeader->bfType[1]!='M')){
    BMP_LoadErrorStr="Error MagicID!=BM";
    return(false);
  }
  
  if(pBMPHeader->biCopmression!=BI_RGB){
    BMP_LoadErrorStr="Error notsupport Compression";
    return(false);
  }
  
  if(pBMPHeader->biHeight>=0x80000000){
    BMP_LoadErrorStr="Error notsupport OS/2 format";
    return(false);
  }
  
  if(pBMPHeader->biPlanes!=1){
    BMP_LoadErrorStr="Error notsupport Planes!=1";
    return(false);
  }
  
  switch(pBMPHeader->biBitCount){
    case 1:
      BMP_LoadErrorStr="Error notsupport 1bitcolor.";
      return(false);
    case 4:
      BMP_LoadErrorStr="Error notsupport 4bitcolor.";
      return(false);
    case 8:
      pBMPHeader->DataWidth=pBMPHeader->biWidth*1;
      break;
    case 16:
      BMP_LoadErrorStr="Error notsupport 16bitcolor.";
      return(false);
    case 24:
      pBMPHeader->DataWidth=pBMPHeader->biWidth*3;
      break;
    case 32:
      pBMPHeader->DataWidth=pBMPHeader->biWidth*4;
      break;
    default:
      BMP_LoadErrorStr="Error Unknown xxBitColor.";
      return(false);
  }
  
  if((pBMPHeader->DataWidth&3)!=0){
    pBMPHeader->DataWidth+=4-(pBMPHeader->DataWidth&3);
  }
  
  BMP_LoadErrorStr="";
  return(true);
}

static char bmerrstr1[256],bmerrstr2[256];

static bool intLoadBM(const char *bmpfn,u16 *pbm,const u32 bmw,const u32 bmh)
{
  bmerrstr1[0]=0;
  bmerrstr2[0]=0;
  
  if(pbm==NULL){
    snprintf(bmerrstr1,256,"BitmapMemory is NULL.");
    snprintf(bmerrstr2,256,"The memory is insufficient?");
    _consolePrintf("%s\n",bmerrstr1);
    _consolePrintf("%s\n",bmerrstr2);
    return(false);
  }
  
  u8 *bmdata;
  u32 bmsize;
  
  Shell_ReadFile(bmpfn,(void**)&bmdata,(int*)&bmsize);
  if(bmdata==NULL){
    return(false);
    }else{
//    _consolePrintf("loadskin /shell/%s\n",bmpfn);
  }
  
  TBMPHeader BMPHeader;
  
  if(GetBMPHeader(bmdata,&BMPHeader)==false){ cwl();
    snprintf(bmerrstr1,256,"Request /shell/%s WindowsBitmapFormat",bmpfn);
    snprintf(bmerrstr2,256,"%s",BMP_LoadErrorStr);
    _consolePrintf("%s\n",bmerrstr1);
    _consolePrintf("%s\n",bmerrstr2);
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  if((BMPHeader.biWidth==1)&&(BMPHeader.biHeight==1)){ cwl();
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  if(BMPHeader.biBitCount==32){
    _consolePrintf("Error. not support 32bit color.");
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  if((BMPHeader.biWidth<bmw)||(BMPHeader.biHeight<bmh)){ cwl();
    snprintf(bmerrstr1,256,"Request /shell/%s WindowsBitmapFormat",bmpfn);
    snprintf(bmerrstr2,256,"%d x %dpixel 8 or 24bitcolor NoCompression.",bmw,bmh);
    _consolePrintf("%s\n",bmerrstr1);
    _consolePrintf("%s\n",bmerrstr2);
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  u32 gr=0,gg=0,gb=0;
  
#define Gravity(c,cg) { \
  c+=cg; \
  cg=c&7; \
  c=c>>3; \
  if((c&(~0x1f))!=0) c=(c<0) ? 0x00 : 0x1f; \
}

  for(u32 y=0;y<bmh;y++){
    u8 *pSrcBM=&BMPHeader.pBitmap[(BMPHeader.biHeight-1-y)*BMPHeader.DataWidth];
    u16 *pDstBM=&pbm[y*bmw];
    
    switch(BMPHeader.biBitCount){
      case 8: {
        u8 *PaletteTable=BMPHeader.pPalette;
        for(u32 x=0;x<bmw;x++){
          u8 *pal;
          u32 r,g,b;
          
          pal=&PaletteTable[*pSrcBM*4];
          pSrcBM+=1;
          
          b=pal[0];
          g=pal[1];
          r=pal[2];
          
          Gravity(b,gb);
          Gravity(g,gg);
          Gravity(r,gr);
          
          pDstBM[x]=RGB15(r,g,b) | BIT(15);
        }
        break;
      }
      case 24: {
        for(u32 x=0;x<bmw;x++){
          u32 r,g,b;
          
          b=pSrcBM[0];
          g=pSrcBM[1];
          r=pSrcBM[2];
          pSrcBM+=3;
          
          Gravity(b,gb);
          Gravity(g,gg);
          Gravity(r,gr);
          
          pDstBM[x]=RGB15(r,g,b) | BIT(15);
        }
        break;
      }
    }
    
  }
  
#undef Gravity

  free(bmdata); bmdata=NULL;
  
  return(true);
}

static bool intLoadBM_TransGreen(const char *bmpfn,u16 *pbm,const u32 bmw,const u32 bmh)
{
  bmerrstr1[0]=0;
  bmerrstr2[0]=0;
  
  if(pbm==NULL){
    snprintf(bmerrstr1,256,"BitmapMemory is NULL.");
    snprintf(bmerrstr2,256,"The memory is insufficient?");
    _consolePrintf("%s\n",bmerrstr1);
    _consolePrintf("%s\n",bmerrstr2);
    return(false);
  }
  
  u8 *bmdata;
  u32 bmsize;
  
  Shell_ReadFile(bmpfn,(void**)&bmdata,(int*)&bmsize);
  if(bmdata==NULL){
    return(false);
    }else{
//    _consolePrintf("loadskin /shell/%s\n",bmpfn);
  }
  
  TBMPHeader BMPHeader;
  
  if(GetBMPHeader(bmdata,&BMPHeader)==false){ cwl();
    snprintf(bmerrstr1,256,"Request /shell/%s WindowsBitmapFormat",bmpfn);
    snprintf(bmerrstr2,256,"%s",BMP_LoadErrorStr);
    _consolePrintf("%s\n",bmerrstr1);
    _consolePrintf("%s\n",bmerrstr2);
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  if((BMPHeader.biWidth==1)&&(BMPHeader.biHeight==1)){ cwl();
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  if(BMPHeader.biBitCount==32){
    _consolePrintf("Error. not support 32bit color.");
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  if((BMPHeader.biWidth<bmw)||(BMPHeader.biHeight<bmh)){ cwl();
    snprintf(bmerrstr1,256,"Request /shell/%s WindowsBitmapFormat",bmpfn);
    snprintf(bmerrstr2,256,"%d x %dpixel 8 or 24bitcolor NoCompression.",bmw,bmh);
    _consolePrintf("%s\n",bmerrstr1);
    _consolePrintf("%s\n",bmerrstr2);
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  u32 gr=0,gg=0,gb=0;
  
#define Gravity(c,cg) { \
  c+=cg; \
  cg=c&7; \
  c=c>>3; \
  if((c&(~0x1f))!=0) c=(c<0) ? 0x00 : 0x1f; \
}

  for(u32 y=0;y<bmh;y++){
    u8 *pSrcBM=&BMPHeader.pBitmap[(BMPHeader.biHeight-1-y)*BMPHeader.DataWidth];
    u16 *pDstBM=&pbm[y*bmw];
    
    switch(BMPHeader.biBitCount){
      case 8: {
        u8 *PaletteTable=BMPHeader.pPalette;
        for(u32 x=0;x<bmw;x++){
          u8 *pal;
          u32 r,g,b;
          
          pal=&PaletteTable[*pSrcBM*4];
          pSrcBM+=1;
          
          b=pal[0];
          g=pal[1];
          r=pal[2];
          
          if((b==0x00)&&(g==0xff)&&(r==0x00)){
            pDstBM[x]=0;
            }else{
            Gravity(b,gb);
            Gravity(g,gg);
            Gravity(r,gr);
            
            pDstBM[x]=RGB15(r,g,b) | BIT(15);
          }
        }
        break;
      }
      case 24: {
        for(u32 x=0;x<bmw;x++){
          u32 r,g,b;
          
          b=pSrcBM[0];
          g=pSrcBM[1];
          r=pSrcBM[2];
          pSrcBM+=3;
          
          if((b==0x00)&&(g==0xff)&&(r==0x00)){
            pDstBM[x]=0;
            }else{
            Gravity(b,gb);
            Gravity(g,gg);
            Gravity(r,gr);
            
            pDstBM[x]=RGB15(r,g,b) | BIT(15);
          }
        }
        break;
      }
    }
    
  }
  
#undef Gravity

  free(bmdata); bmdata=NULL;
  
  return(true);
}

static bool intLoadBM32(const char *bmpfn,u32 *pbm32,const u32 bmw,const u32 bmh)
{
  bmerrstr1[0]=0;
  bmerrstr2[0]=0;
  
  if(pbm32==NULL){
    snprintf(bmerrstr1,256,"BitmapMemory is NULL.");
    snprintf(bmerrstr2,256,"The memory is insufficient?");
    _consolePrintf("%s\n",bmerrstr1);
    _consolePrintf("%s\n",bmerrstr2);
    return(false);
  }
  
  u8 *bmdata;
  u32 bmsize;
  
  Shell_ReadFile(bmpfn,(void**)&bmdata,(int*)&bmsize);
  if(bmdata==NULL){
    return(false);
    }else{
//    _consolePrintf("loadskin /shell/%s\n",bmpfn);
  }
  
  TBMPHeader BMPHeader;
  
  if(GetBMPHeader(bmdata,&BMPHeader)==false){ cwl();
    snprintf(bmerrstr1,256,"Request /shell/%s WindowsBitmapFormat",bmpfn);
    snprintf(bmerrstr2,256,"%s",BMP_LoadErrorStr);
    _consolePrintf("%s\n",bmerrstr1);
    _consolePrintf("%s\n",bmerrstr2);
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  if((BMPHeader.biWidth==1)&&(BMPHeader.biHeight==1)){ cwl();
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  if((BMPHeader.biBitCount!=24)&&(BMPHeader.biBitCount!=32)){
    _consolePrintf("Error. supported format 32bit bmp only.");
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  if((BMPHeader.biWidth<bmw)||(BMPHeader.biHeight<bmh)){ cwl();
    snprintf(bmerrstr1,256,"Request /shell/%s WindowsBitmapFormat",bmpfn);
    snprintf(bmerrstr2,256,"%d x %dpixel 8 or 24bitcolor NoCompression.",bmw,bmh);
    _consolePrintf("%s\n",bmerrstr1);
    _consolePrintf("%s\n",bmerrstr2);
    free(bmdata); bmdata=NULL;
    return(false);
  }
  
  for(u32 y=0;y<bmh;y++){
    u8 *pSrcBM=&BMPHeader.pBitmap[(BMPHeader.biHeight-1-y)*BMPHeader.DataWidth];
    u32 *pDstBM=&pbm32[y*bmw];
    
    switch(BMPHeader.biBitCount){
      case 24: {
        for(u32 x=0;x<bmw;x++){
          u32 r,g,b;
          
          b=pSrcBM[0];
          g=pSrcBM[1];
          r=pSrcBM[2];
          pSrcBM+=3;
          
          if((b==0x00)&&(g==0xff)&&(r==0x00)){
            pDstBM[x]=0;
            }else{
            u32 a=0xff>>3;
            b>>=3;
            g>>=3;
            r>>=3;
            pDstBM[x]=(a<<24) | (r<<16) | (g<<8) | (b<<0);
          }
        }
        break;
      }
      case 32: {
        for(u32 x=0;x<bmw;x++){
          u32 col;
          
          col=*((u32*)pSrcBM);
          pSrcBM+=4;
          
          col=(col & ~0x07070707) >> 3;
          
          pDstBM[x]=col;
        }
        break;
      }
    }
  }
  
  free(bmdata); bmdata=NULL;
  
  return(true);
}

void LoadSkinBM(void)
{ cwl();
  MemSet8DMA3(0,&SkinBM,sizeof(TSkinBM));
  
  TSkinBM *psbm=&SkinBM;
  
  if(psbm->pDesktopBM!=NULL){ cwl();
    free(psbm->pDesktopBM); psbm->pDesktopBM=NULL;
  }
  psbm->DesktopBM_W=ScreenWidth;
  psbm->DesktopBM_H=ScreenHeight;
  psbm->pDesktopBM=(u16*)safemalloc(psbm->DesktopBM_W*psbm->DesktopBM_H*2);
  
  if(intLoadBM("desktop.bmp",psbm->pDesktopBM,psbm->DesktopBM_W,psbm->DesktopBM_H)==false){
    MemSet16DMA3(MWC_DesktopBG,psbm->pDesktopBM,psbm->DesktopBM_W*psbm->DesktopBM_H*2);
    VRAM_WriteStringRect(psbm->pDesktopBM,psbm->DesktopBM_W,psbm->DesktopBM_W,1+0,1+1*FontProHeight,bmerrstr1,RGB15( 0, 0, 0) | BIT(15));
    VRAM_WriteStringRect(psbm->pDesktopBM,psbm->DesktopBM_W,psbm->DesktopBM_W,0+0,0+1*FontProHeight,bmerrstr1,RGB15(31,31,31) | BIT(15));
    VRAM_WriteStringRect(psbm->pDesktopBM,psbm->DesktopBM_W,psbm->DesktopBM_W,1+0,1+2*FontProHeight,bmerrstr2,RGB15( 0, 0, 0) | BIT(15));
    VRAM_WriteStringRect(psbm->pDesktopBM,psbm->DesktopBM_W,psbm->DesktopBM_W,0+0,0+2*FontProHeight,bmerrstr2,RGB15(31,31,31) | BIT(15));
  }
  /**********************************************************************************************/
   //增加desktop
  if(psbm->ppDesktopBnBM!=NULL){cwl();
	  free(psbm->ppDesktopBnBM);psbm->ppDesktopBnBM=NULL;
  }
  psbm->DesktopBnBM_W=256;
  psbm->DesktopBnBM_H=80;	
  psbm->ppDesktopBnBM = (u16*)safemalloc(256*80*2);
  if(intLoadBM("desktopbn.bmp",psbm->ppDesktopBnBM,psbm->DesktopBnBM_W,psbm->DesktopBnBM_H)==false){
  }
  else
  {
  }
  //增加设置位图
  if(psbm->pConfigBM!=NULL){cwl();
  	free(psbm->pConfigBM);psbm->pConfigBM=NULL;
  }
  psbm->pConfigBM = (u16*)safemalloc(189*136*2);
  psbm->ConfigBM_W=189;
  psbm->ConfigBM_H=136;
  if(intLoadBM("setting.bmp",psbm->pConfigBM,psbm->ConfigBM_W,psbm->ConfigBM_H)==false){
	 MemSet16DMA3(MWC_DesktopBG,psbm->pConfigBM,psbm->ConfigBM_W*psbm->ConfigBM_H*2);  	
  } 
  //增加选择选择中ICOM
  if(psbm->pCheckedBM!=NULL){cwl();
  	free(psbm->pCheckedBM);psbm->pCheckedBM=NULL;
  }
  psbm->pCheckedBM = (u16*)safemalloc(12*12*2);
  psbm->CheckedBM_W = 12;
  psbm->CheckedBM_H = 12;
  if(intLoadBM("checked.bmp",psbm->pCheckedBM,psbm->CheckedBM_W,psbm->CheckedBM_H)==false){
  } 
  //增加选择ICOM
  if(psbm->pCheckBM!=NULL){cwl();
  	free(psbm->pCheckBM);psbm->pCheckBM=NULL;
  }
  psbm->pCheckBM = (u16*)safemalloc(12*12*2);
  psbm->CheckBM_W = 12;
  psbm->CheckBM_H = 12;
  if(intLoadBM("uncheck.bmp",psbm->pCheckBM,psbm->CheckBM_W,psbm->CheckBM_H)==false){
  } 
  //增加帮助对话框
  if(psbm->pHelpBM!=NULL){cwl();
  	free(psbm->pHelpBM);psbm->pHelpBM=NULL;
  }
  psbm->HelpBM_W = 220;
  psbm->HelpBM_H = 160;
  psbm->pHelpBM = (u16*)safemalloc(220*160*2);
  if(intLoadBM("help.bmp",psbm->pHelpBM,psbm->HelpBM_W,psbm->HelpBM_H)==false){
  } 
   //增加MP3ICON图
  if(psbm->pMp3Icon!=NULL){ cwl();
  	free(psbm->pMp3Icon);psbm->pMp3Icon=NULL;
  }
  psbm->mp3BM_W=32;
  psbm->mp3BM_H=32;
  psbm->pMp3Icon=(u16*)safemalloc(psbm->mp3BM_W*psbm->mp3BM_H*2);
  if(intLoadBM("ez_mp3.bmp",psbm->pMp3Icon,32,32)==false){
    if(psbm->pMp3Icon!=NULL){ cwl();
      free(psbm->pMp3Icon); psbm->pMp3Icon=NULL;
    }
  }
  //增加进度条
  if(psbm->pProgressBM!=NULL){cwl();
 	free(psbm->pProgressBM); psbm->pProgressBM=NULL;
  }
  psbm->ProgressBM_W = 168;
  psbm->ProgressBM_H = 60;
  psbm->pProgressBM = (u16*)safemalloc(168*60*2);
  if(intLoadBM("progress.bmp",psbm->pProgressBM,psbm->ProgressBM_W,psbm->ProgressBM_H)==false){
  } 
  //
  if(psbm->pProgressBarBM1!=NULL){cwl();
  	free(psbm->pProgressBarBM1); psbm->pProgressBarBM1=NULL;
  }
  psbm->ProgressBarBM1_W=116;
  psbm->ProgressBarBM1_H=10;
  	psbm->pProgressBarBM1 = (u16*)safemalloc(116*10*2);
   if(intLoadBM("progbar2.bmp",psbm->pProgressBarBM1,psbm->ProgressBarBM1_W,psbm->ProgressBarBM1_H)==false){
  } 	
   
  //
  if(psbm->pProgressBarBM2!=NULL){cwl();
  	free(psbm->pProgressBarBM2); psbm->pProgressBarBM2=NULL;
  }
  psbm->ProgressBarBM2_W=10;
  psbm->ProgressBarBM2_H=10;
  psbm->pProgressBarBM2 = (u16*)safemalloc(10*10*2);
   if(intLoadBM("progbar1.bmp",psbm->pProgressBarBM2,psbm->ProgressBarBM2_W,psbm->ProgressBarBM2_H)==false){
  } 	
  
  /**********************************************************************************************/
  psbm->TitleBM_W=ScreenWidth;
  psbm->TitleBM_H=TitleBarHeight;
  
  if(psbm->pTitleABM!=NULL){ cwl();
    free(psbm->pTitleABM); psbm->pTitleABM=NULL;
  }
  psbm->pTitleABM=(u16*)safemalloc(psbm->TitleBM_W*psbm->TitleBM_H*2);
  
  if(intLoadBM("titlea.bmp",psbm->pTitleABM,psbm->TitleBM_W,psbm->TitleBM_H)==false){
    if(psbm->pTitleABM!=NULL){ cwl();
      free(psbm->pTitleABM); psbm->pTitleABM=NULL;
    }
  }
  
  if(psbm->pTitleDBM!=NULL){ cwl();
    free(psbm->pTitleDBM); psbm->pTitleDBM=NULL;
  }
  psbm->pTitleDBM=(u16*)safemalloc(psbm->TitleBM_W*psbm->TitleBM_H*2);
  
  if(intLoadBM("titled.bmp",psbm->pTitleDBM,psbm->TitleBM_W,psbm->TitleBM_H)==false){
    if(psbm->pTitleDBM!=NULL){ cwl();
      free(psbm->pTitleDBM); psbm->pTitleDBM=NULL;
    }
  }
  
  if(psbm->pCloseBtnBM!=NULL){ cwl();
    free(psbm->pCloseBtnBM); psbm->pCloseBtnBM=NULL;
  }
  psbm->CloseBtnBM_W=TitleBarHeight;
  psbm->CloseBtnBM_H=TitleBarHeight;
  psbm->pCloseBtnBM=(u16*)safemalloc(psbm->CloseBtnBM_W*psbm->CloseBtnBM_H*2);
  
  if(intLoadBM_TransGreen("closebtn.bmp",psbm->pCloseBtnBM,psbm->CloseBtnBM_W,psbm->CloseBtnBM_H)==false){
    if(psbm->pCloseBtnBM!=NULL){ cwl();
      free(psbm->pCloseBtnBM); psbm->pCloseBtnBM=NULL;
    }
  }
  
  if(psbm->pClientBM!=NULL){ cwl();
    free(psbm->pClientBM); psbm->pClientBM=NULL;
  }
  psbm->ClientBM_W=ScreenWidth;
  psbm->ClientBM_H=ScreenHeight;
  psbm->pClientBM=(u16*)safemalloc(psbm->ClientBM_W*psbm->ClientBM_H*2);
  
  if(intLoadBM("client.bmp",psbm->pClientBM,psbm->ClientBM_W,psbm->ClientBM_H)==false){
    if(psbm->pClientBM!=NULL){ cwl();
      free(psbm->pClientBM); psbm->pClientBM=NULL;
    }
  }
  
  if(psbm->pFileBM!=NULL){ cwl();
    free(psbm->pFileBM); psbm->pFileBM=NULL;
  }
  psbm->FileBM_W=ScreenWidth;
  psbm->FileBM_H=ScreenHeight;
  psbm->pFileBM=(u16*)safemalloc(psbm->FileBM_W*psbm->FileBM_H*2);
  
  if(intLoadBM("c_file.bmp",psbm->pFileBM,psbm->FileBM_W,psbm->FileBM_H)==false){
    if(psbm->pFileBM!=NULL){ cwl();
      free(psbm->pFileBM); psbm->pFileBM=NULL;
    }
  }
  
  if(psbm->pTextBM!=NULL){ cwl();
    free(psbm->pTextBM); psbm->pTextBM=NULL;
  }
  psbm->TextBM_W=ScreenWidth;
  psbm->TextBM_H=ScreenHeight;
  psbm->pTextBM=(u16*)safemalloc(psbm->TextBM_W*psbm->TextBM_H*2);
  
  if(intLoadBM("c_text.bmp",psbm->pTextBM,psbm->TextBM_W,psbm->TextBM_H)==false){
    if(psbm->pTextBM!=NULL){ cwl();
      free(psbm->pTextBM); psbm->pTextBM=NULL;
    }
  }
  
  psbm->PrgBarBM_W=ScreenWidth;
  psbm->PrgBarBM_H=PrgBarHeight;
  
  if(psbm->pPrgBarABM!=NULL){ cwl();
    free(psbm->pPrgBarABM); psbm->pPrgBarABM=NULL;
  }
  psbm->pPrgBarABM=(u16*)safemalloc(psbm->PrgBarBM_W*psbm->PrgBarBM_H*2);
  
  if(intLoadBM("prgbara.bmp",psbm->pPrgBarABM,psbm->PrgBarBM_W,psbm->PrgBarBM_H)==false){
    if(psbm->pPrgBarABM!=NULL){ cwl();
      free(psbm->pPrgBarABM); psbm->pPrgBarABM=NULL;
    }
  }
  
  if(psbm->pPrgBarDBM!=NULL){ cwl();
    free(psbm->pPrgBarDBM); psbm->pPrgBarDBM=NULL;
  }
  psbm->pPrgBarDBM=(u16*)safemalloc(psbm->PrgBarBM_W*psbm->PrgBarBM_H*2);
  
  if(intLoadBM("prgbard.bmp",psbm->pPrgBarDBM,psbm->PrgBarBM_W,psbm->PrgBarBM_H)==false){
    if(psbm->pPrgBarDBM!=NULL){ cwl();
      free(psbm->pPrgBarDBM); psbm->pPrgBarDBM=NULL;
    }
  }
  
  if(psbm->pSB_BGBM!=NULL){ cwl();
    free(psbm->pSB_BGBM); psbm->pSB_BGBM=NULL;
  }
  psbm->pSB_BGBM=(u16*)safemalloc(8*8*2);
  
  if(intLoadBM("sb_bg.bmp",psbm->pSB_BGBM,8,8)==false){
    if(psbm->pSB_BGBM!=NULL){ cwl();
      free(psbm->pSB_BGBM); psbm->pSB_BGBM=NULL;
    }
  }
  
  if(psbm->pSB_BodyBM!=NULL){ cwl();
    free(psbm->pSB_BodyBM); psbm->pSB_BodyBM=NULL;
  }
  psbm->pSB_BodyBM=(u16*)safemalloc(8*8*2);
  
  if(intLoadBM("sb_body.bmp",psbm->pSB_BodyBM,8,8)==false){
    if(psbm->pSB_BodyBM!=NULL){ cwl();
      free(psbm->pSB_BodyBM); psbm->pSB_BodyBM=NULL;
    }
  }
  
  if(psbm->pSB_PartBM!=NULL){ cwl();
    free(psbm->pSB_PartBM); psbm->pSB_PartBM=NULL;
  }
  psbm->pSB_PartBM=(u16*)safemalloc(8*8*2);
  
  if(intLoadBM("sb_part.bmp",psbm->pSB_PartBM,8,8)==false){
    if(psbm->pSB_PartBM!=NULL){ cwl();
      free(psbm->pSB_PartBM); psbm->pSB_PartBM=NULL;
    }
  }
  
  psbm->FileBarBM32_W=ScreenWidth;
  psbm->FileBarBM32_H=FontProHeight;
  
  if(psbm->pFileBarCurrentBM32!=NULL){ cwl();
    free(psbm->pFileBarCurrentBM32); psbm->pFileBarCurrentBM32=NULL;
  }
  psbm->pFileBarCurrentBM32=(u32*)safemalloc(psbm->FileBarBM32_W*psbm->FileBarBM32_H*4);
  
  if(intLoadBM32("fbarcur.bmp",psbm->pFileBarCurrentBM32,psbm->FileBarBM32_W,psbm->FileBarBM32_H)==false){
    if(psbm->pFileBarCurrentBM32!=NULL){ cwl();
      free(psbm->pFileBarCurrentBM32); psbm->pFileBarCurrentBM32=NULL;
    }
  }
  
  if(psbm->pFileBarPlayBM32!=NULL){ cwl();
    free(psbm->pFileBarPlayBM32); psbm->pFileBarPlayBM32=NULL;
  }
  psbm->pFileBarPlayBM32=(u32*)safemalloc(psbm->FileBarBM32_W*psbm->FileBarBM32_H*4);
  
  if(intLoadBM32("fbarplay.bmp",psbm->pFileBarPlayBM32,psbm->FileBarBM32_W,psbm->FileBarBM32_H)==false){
    if(psbm->pFileBarPlayBM32!=NULL){ cwl();
      free(psbm->pFileBarPlayBM32); psbm->pFileBarPlayBM32=NULL;
    }
  }
  
}

