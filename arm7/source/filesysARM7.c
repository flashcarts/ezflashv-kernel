
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "filesysARM7.h"

#include "../../ipc3.h"

int FileSys_GetFileSize(void)
{
  return(IPC3->IR_filesize);
}

/*
void FileSys_fread(void *buf,int size)
{
  IPC3->IR_readsize=size;
  IPC3->IR=IR_FileSys_fread;
  
  REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;
  
  while(IPC3->IR!=IR_NULL){
    swiDelay(1);
  }
  
  u8 *src=(u8*)IPC3->IR_readbuf;
  u8 *dst=(u8*)buf;
  
  memcpy(dst,src,size);
  return;
  
  for(int i=size;i!=0;i--){
    *dst++=*src++;
  }
}
*/

int FileSys_fread(void *buf,int size)
{
  while(IPC3->IR!=IR_NULL){
    swiDelay(1);
  }
  
  if(IPC3->IR_readbufsize<size) size=IPC3->IR_readbufsize;
  
  if(size!=0){
    DMA2_SRC = (u32)IPC3->IR_readbuf;
    DMA2_DEST = (u32)buf;
    DMA2_CR=(DMA_16_BIT | DMA_ENABLE | DMA_START_NOW | DMA_SRC_INC | DMA_DST_INC)+(size/2);
  }
  
  IPC3->IR_readsize=size;
  IPC3->IR=IR_FileSys_fread;
  
  REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;
  
  return(size);
}

int FileSys_fread_flash(void *buf,int size)
{
  IPC3->IR_readsize=0;
  IPC3->IR=IR_FileSys_fread_flash;
  
  REG_IPC_SYNC|=IPC_SYNC_IRQ_REQUEST;
  
  return(FileSys_fread(buf,size));
}

