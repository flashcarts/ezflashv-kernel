
#ifndef dmalink_h
#define dmalink_h

#ifdef __cplusplus
extern "C" {
#endif

#include "../../IPC3.h"

typedef struct {
  bool Enabled;
  u32 srcadr;
  int srcWidth;
  u32 srctermadr;
  u32 dstadr;
  int dstWidth;
} TDMALink;

extern TDMALink DMALink;

extern void DMALinkInit(void);
extern void DMALinkWaitTerminate(void);
extern void DMALinkStart(void);
extern void DMALinkInterruptHandler_DMA1(void);

#ifdef __cplusplus
}
#endif

#endif

