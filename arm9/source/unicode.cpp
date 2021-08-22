
#include <stdio.h>

#include <NDS.h>

#include "_const.h"

#include "memtool.h"
#include "_console.h"

#include "unicode.h"

#include "FontPro.h"

static u8 *pankbuf=NULL;
static u8 *pl2ubuf=NULL;

int DefaultCodePage;
u8 *isAnkTable;
u32 Local2UnicodeTableSize;
u16 *Local2UnicodeTable;
bool isUTF8;

extern void Unicode_StrCopy(const UnicodeChar *src,UnicodeChar *dst)
{
  if(dst==0) return;
  if(src==0){
    dst[0]=0;
    return;
  }
  
  while(*src!=0){
    *dst=*src;
    src++;
    dst++;
  }
  
  *dst=0;
}

bool Unicode_isEqual(const UnicodeChar *s1,const UnicodeChar *s2)
{
  if((s1==0)&&(s2==0)) return(true);
  if((s1==0)||(s2==0)) return(false);
  
  while(*s1==*s2){
    if((*s1==0)||(*s2==0)){
      if((*s1==0)&&(*s2==0)){
        return(true);
        }else{
        return(false);
      }
    }
    s1++;
    s2++;
  }
  return(false);
}

void Unicode_StrAppend(UnicodeChar *s,const UnicodeChar *add)
{
  if((s==0)||(add==0)) return;
  
  while(*s!=0){
    s++;
  }
  
  while(*add!=0){
    *s=*add;
    s++;
    add++;
  }
  
  *s=0;
}

void Unicode_Init(u8 *pank,u8 *pl2u)
{
  Unicode_Free();
  
  pankbuf=pank;
  pl2ubuf=pl2u;
  
  DefaultCodePage=*(u16*)&pl2u[0];
  Local2UnicodeTableSize=*(u16*)&pl2u[2];
  pl2u+=4;
  
  _consolePrintf("set DefaultCodePage is %d.\n",DefaultCodePage);
  
  if(Local2UnicodeTableSize==0xffff){
    isUTF8=true;
    Local2UnicodeTableSize=0;
    _consolePrintf("Enable UTF-8 Translation.\n");
    }else{
    isUTF8=false;
    _consolePrintf("Local2UnicodeTable size=%d.\n",Local2UnicodeTableSize);
  }
  
  isAnkTable=(u8*)pank;
  Local2UnicodeTable=(u16*)pl2u;
}

void Unicode_Free(void)
{
  if(pankbuf!=NULL){
    safefree(pankbuf); pankbuf=NULL;
  }
  if(pl2ubuf!=NULL){
    safefree(pl2ubuf); pl2ubuf=NULL;
  }
  
  DefaultCodePage=0;
  isAnkTable=NULL;
  Local2UnicodeTableSize=0;
  Local2UnicodeTable=NULL;
  isUTF8=false;
}

void StrConvert_Ank2Unicode(const char *srcstr,UnicodeChar *dststr)
{
  while(*srcstr!=0){
    *dststr=(UnicodeChar)*srcstr;
    dststr++;
    srcstr++;
  }
  
  *dststr=(UnicodeChar)0;
}

void StrConvert_Local2Unicode(const char *srcstr,UnicodeChar *dststr)
{
  if(isUTF8==true){
    StrConvert_UTF82Unicode(srcstr,dststr);
    return;
  }
  
  while(*srcstr!=0){
    u16 lc;
    
    lc=(u16)*srcstr;
    srcstr++;
    
    if(isAnkTable[lc]==0){
      lc=(lc << 8)+((u16)*srcstr);
      srcstr++;
    }
    
    if(lc<Local2UnicodeTableSize){
      *dststr=Local2UnicodeTable[lc];
      if(*dststr==(UnicodeChar)0) *dststr=(UnicodeChar)'?';
      }else{
      *dststr=(UnicodeChar)'?';
    }
    dststr++;
  }
  
  *dststr=(UnicodeChar)0;
}

void StrConvert_UTF82Unicode(const char *srcstr,UnicodeChar *dststr)
{
  while(*srcstr!=0){
    u16 b0=(byte)srcstr[0],b1=(byte)srcstr[1],b2=(byte)srcstr[2];
    u16 uc;
    
    if(b0<0x80){
      uc=b0;
      srcstr++;
      }else{
      if((b0&0xe0)==0xc0){ // 0b 110. ....
        uc=((b0&~0xe0)<<6)+((b1&~0xc0)<<0);
        srcstr+=2;
        }else{
        if((b0&0xf0)==0xe0){ // 0b 1110 ....
          uc=((b0&~0xf0)<<12)+((b1&~0xc0)<<6)+((b2&~0xc0)<<0);
          srcstr+=3;
          }else{
          uc=(u16)'?';
          srcstr+=4;
        }
      }
    }
    
    if(FontPro_Exists(uc)==false) uc=(u16)'?';
    
    *dststr=uc;
    dststr++;
  }
  
  *dststr=(UnicodeChar)0;
}

