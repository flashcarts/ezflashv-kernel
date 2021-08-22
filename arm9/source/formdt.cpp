
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "_const.h"

#include "formdt.h"

static char FormatDateStr[32+1]={0,};

void formdt_SetFormatDateStr(char *format)
{
  snprintf(FormatDateStr,32,format);
  FormatDateStr[32]=(char)0;
}

static char *psrcstr;
static char dstchar;

enum EDateType {EDT_End,EDT_Char,EDT_YY,EDT_YYYY,EDT_M,EDT_MM,EDT_D,EDT_DD};

EDateType fd_GetNextDateType(void)
{
  if(psrcstr==NULL) return(EDT_End);
  if(psrcstr[0]==(char)0) return(EDT_End);
  
  if((psrcstr[0]=='Y')&&(psrcstr[1]=='Y')&&(psrcstr[2]=='Y')&&(psrcstr[3]=='Y')){
    psrcstr+=4;
    return(EDT_YYYY);
  }
  if((psrcstr[0]=='Y')&&(psrcstr[1]=='Y')){
    psrcstr+=2;
    return(EDT_YY);
  }
  
  if((psrcstr[0]=='M')&&(psrcstr[1]=='M')){
    psrcstr+=2;
    return(EDT_MM);
  }
  if(psrcstr[0]=='M'){
    psrcstr+=1;
    return(EDT_M);
  }
  
  if((psrcstr[0]=='D')&&(psrcstr[1]=='D')){
    psrcstr+=2;
    return(EDT_DD);
  }
  if(psrcstr[0]=='D'){
    psrcstr+=1;
    return(EDT_D);
  }
  
  dstchar=*psrcstr++;
  return(EDT_Char);
  
}

u32 formdt_FormatDate(char *str, u32 size, const u32 year, const u32 month, const u32 day)
{
  if(str==NULL) return(0);
  if(size==0) return(0);
  
  *str=(char)0;
  
  u32 dstsizebackup=size;
  
  psrcstr=FormatDateStr;
  dstchar=(char)0;
  
  while(1){
    EDateType edt=fd_GetNextDateType();
    if(edt==EDT_End) break;
    
    u32 len=0;
    
    switch(edt){
      case EDT_End: break;
      case EDT_Char: {
        *str=dstchar;
        len=1;
      } break;
      case EDT_YY:   len=snprintf(str,size,"%02d",year%100); break;
      case EDT_YYYY: len=snprintf(str,size,"%04d",year); break;
      case EDT_M:    len=snprintf(str,size,"%d",month); break;
      case EDT_MM:   len=snprintf(str,size,"%02d",month); break;
      case EDT_D:    len=snprintf(str,size,"%d",day); break;
      case EDT_DD:   len=snprintf(str,size,"%02d",day); break;
    }
    
    str+=len;
    size-=len;
    if(size==0) break;
  }
  
  *str=(char)0;
  return(dstsizebackup-size);
}

