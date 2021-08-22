
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>
#include <NDS/ARM9/CP15.h>

#include "_console.h"
#include "dmalink.h"
#include "_const.h"

TDMALink DMALink;

void DMALinkInit(void)
{
  DMALink.Enabled=false;
}

void DMALinkWaitTerminate(void)
{
  while(DMALink.Enabled==true){};
}

void DMALinkStart(void)
{
  if((DMA1_CR&DMA_ENABLE)!=0){
    _consolePrintf("Already used DMA1\n");
    ShowLogHalt();
    return;
  }
  
  TDMALink *pDMALink=&DMALink;
  
  pDMALink->Enabled=true;
  
//  DC_FlushRangeOverrun((void*)pDMALink->srcadr,pDMALink->srcWidth);
//  DC_FlushRangeOverrun((void*)pDMALink->dstadr,pDMALink->dstWidth);
  DMA1_SRC = pDMALink->srcadr;
  DMA1_DEST = pDMALink->dstadr;
  DMA1_CR = DMA_ENABLE | DMA_IRQ_REQ | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | ((pDMALink->srcWidth)>>1);
}

void DMALinkInterruptHandler_DMA1(void)
{
  TDMALink *pDMALink=&DMALink;
  
  if(pDMALink->Enabled==false) return;
  
  pDMALink->srcadr+=pDMALink->srcWidth;
  pDMALink->dstadr+=pDMALink->dstWidth;
  
  if(pDMALink->srcadr==pDMALink->srctermadr){
    DMA1_CR=0;
    pDMALink->Enabled=false;
    return;
  }
  
//  DC_FlushRangeOverrun((void*)pDMALink->srcadr,pDMALink->srcWidth);
//  DC_FlushRangeOverrun((void*)pDMALink->dstadr,pDMALink->dstWidth);
  DMA1_SRC = pDMALink->srcadr;
  DMA1_DEST = pDMALink->dstadr;
  DMA1_CR = DMA_ENABLE | DMA_IRQ_REQ | DMA_SRC_INC | DMA_DST_INC | DMA_16_BIT | ((pDMALink->srcWidth)>>1);
}

/*
            DMALinkWaitTerminate();
            
            TDMALink *pDMALink=&DMALink;
            
            pDMALink->srcadr=(u32)info->display_fbuf->buf[0];
            pDMALink->srcWidth=Width*2;
            pDMALink->srctermadr=pDMALink->srcadr+(Height*Width*2);
            pDMALink->dstadr=(u32)FrameBuf;
            pDMALink->dstWidth=ScreenWidth*2;
            
            DMALinkStart();
*/
