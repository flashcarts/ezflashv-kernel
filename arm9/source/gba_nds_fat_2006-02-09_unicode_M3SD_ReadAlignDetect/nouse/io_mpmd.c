/*
	io_mpmd.c based on

	compact_flash.c
	By chishm (Michael Chisholm)

	Hardware Routines for reading a compact flash card
	using the GBA Movie Player

	CF routines modified with help from Darkfader

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#include <stdio.h>
#include <stdlib.h>

#include "io_mpmd.h"

#include "../_console.h"

#ifdef SUPPORT_MPMD

//---------------------------------------------------------------
// DMA
#ifdef _CF_USE_DMA
 #ifndef NDS
  #include "gba_dma.h"
 #else
  #include <nds/dma.h>
  #ifdef ARM9
   #include <nds/arm9/cache.h>
  #endif
 #endif
#endif

//---------------------------------------------------------------
// CF Addresses & Commands

#define GAME_PAK		0x08000000			// Game pack start address

// GBAMP CF Addresses
#define MP_REG_STS		*(vu16*)(GAME_PAK + 0x018C0000)	// 6h Status of the CF Card / Device control
#define MP_REG_CMD		*(vu16*)(GAME_PAK + 0x010E0000)	// 7h Commands sent to control chip and status return
#define MP_REG_ERR		*(vu16*)(GAME_PAK + 0x01020000)	// 1h Errors / Features

#define MP_REG_SEC		*(vu16*)(GAME_PAK + 0x01040000)	// Number of sector to transfer
#define MP_REG_LBA1		*(vu16*)(GAME_PAK + 0x01060000)	// 1st byte of sector address
#define MP_REG_LBA2		*(vu16*)(GAME_PAK + 0x01080000)	// 2nd byte of sector address
#define MP_REG_LBA3		*(vu16*)(GAME_PAK + 0x010A0000)	// 3rd byte of sector address
#define MP_REG_LBA4		*(vu16*)(GAME_PAK + 0x010C0000)	// last nibble of sector address | 0xE0

#define MP_DATA			(vu16*)(GAME_PAK + 0x01000000)		// Pointer to buffer of CF data transered from card

// CF Card status
#define CF_STS_INSERTED		0x50
#define CF_STS_REMOVED		0x00
#define CF_STS_READY		0x58

#define CF_STS_DRQ			0x08
#define CF_STS_BUSY			0x80

// CF Card commands
#define CF_CMD_LBA			0xE0
#define CF_CMD_READ			0x20
#define CF_CMD_WRITE		0x30

//#define CARD_TIMEOUT	10000000		// Updated due to suggestion from SaTa, otherwise card will timeout sometimes on a write
#define CARD_TIMEOUT	(0x7fffffff)		// Updated due to suggestion from SaTa, otherwise card will timeout sometimes on a write


/*-----------------------------------------------------------------
MPMD_IsInserted
Is a compact flash card inserted?
bool return OUT:  true if a CF card is inserted
-----------------------------------------------------------------*/
bool MPMD_IsInserted (void) 
{
	// Change register, then check if value did change
	MP_REG_STS = CF_STS_INSERTED;
	return ((MP_REG_STS & 0xff) == CF_STS_INSERTED);
}


/*-----------------------------------------------------------------
MPMD_ClearStatus
Tries to make the CF card go back to idle mode
bool return OUT:  true if a CF card is idle
-----------------------------------------------------------------*/
bool MPMD_ClearStatus (void) 
{
	int i;
	
	// Wait until CF card is finished previous commands
	i=0;
	while ((MP_REG_CMD & CF_STS_BUSY) && (i < CARD_TIMEOUT))
	{
		i++;
	}
	
	// Wait until card is ready for commands
	i = 0;
	while ((!(MP_REG_STS & CF_STS_INSERTED)) && (i < CARD_TIMEOUT))
	{
		i++;
	}
	if (i >= CARD_TIMEOUT)
		return false;

	return true;
}


/*-----------------------------------------------------------------
MPMD_ReadSectors
Read 512 byte sector numbered "sector" into "buffer"
u32 sector IN: address of first 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer OUT: pointer to 512 byte buffer to store data in
bool return OUT: true if successful
-----------------------------------------------------------------*/

static bool MPMD_ReadSectorsSpeed (u32 sector, u8 numSecs, void* buffer)
{
	int j = (numSecs > 0 ? numSecs : 256);
	u16 *buff = (u16*)buffer;
#ifdef _CF_ALLOW_UNALIGNED
	u8 *buff_u8 = (u8*)buffer;
	int temp;
#endif

#if (defined _CF_USE_DMA) && (defined NDS) && (defined ARM9)
	void *dmabuf=buffer;
	int dmasize=j*BYTE_PER_READ;
	DC_FlushRange(dmabuf,dmasize);
	DC_InvalidateRange(dmabuf,dmasize);
#endif

	// Wait until CF card is finished previous commands
	while(MP_REG_CMD & CF_STS_BUSY);
	
	// Wait until card is ready for commands
	while (!(MP_REG_STS & CF_STS_INSERTED));
	
	// Set number of sectors to read
	MP_REG_SEC = numSecs;	
	
	// Set read sector
	MP_REG_LBA1 = sector & 0xFF;						// 1st byte of sector number
	MP_REG_LBA2 = (sector >> 8) & 0xFF;					// 2nd byte of sector number
	MP_REG_LBA3 = (sector >> 16) & 0xFF;				// 3rd byte of sector number
	MP_REG_LBA4 = ((sector >> 24) & 0x0F )| CF_CMD_LBA;	// last nibble of sector number
	
	// Set command to read
	MP_REG_CMD = CF_CMD_READ;
	
	while (j--)
	{
		// Wait until card is ready for reading
		while ((MP_REG_STS & 0xff)!= CF_STS_READY);
		
		// Read data
#ifdef _CF_USE_DMA
 #ifdef NDS
		DMA3_SRC = (u32)MP_DATA;
		DMA3_DEST = (u32)buff;
		DMA3_CR = 256 | DMA_COPY_HALFWORDS | DMA_SRC_FIX;
 #else
		DMA3COPY ( MP_DATA, buff, 256 | DMA16 | DMA_ENABLE | DMA_SRC_FIXED);
 #endif
		buff += BYTE_PER_READ / 2;
#elif defined _CF_ALLOW_UNALIGNED
		i=256;
		if ((u32)buff_u8 & 0x01) {
			while(i--){
				temp = *MP_DATA;
				*buff_u8++ = temp & 0xFF;
				*buff_u8++ = temp >> 8;
			}
		} else {
			while(i--){
				*buff++ = *MP_DATA;
			}
		}
#else
#error define error.
#endif
		
	}
	
	while ((MP_REG_STS & 0xff)!= CF_STS_INSERTED);
	
#if (defined _CF_USE_DMA) && (defined NDS)
	// Wait for end of transfer before returning
	while(DMA3_CR & DMA_BUSY);
	DC_FlushRange(dmabuf,dmasize);
	DC_InvalidateRange(dmabuf,dmasize);
#endif

	return true;
}

static void MPMD_ReadSectorsAccuracy_readonereal (u32 sector, void* buffer)
{
	u16 *buff = (u16*)buffer;
#ifdef _CF_ALLOW_UNALIGNED
	u8 *buff_u8 = (u8*)buffer;
	int temp;
#endif

#if (defined _CF_USE_DMA) && (defined NDS) && (defined ARM9)
	void *dmabuf=buffer;
	int dmasize=1*BYTE_PER_READ;
	DC_FlushRange(dmabuf,dmasize);
	DC_InvalidateRange(dmabuf,dmasize);
#endif

	// Wait until CF card is finished previous commands
	while(MP_REG_CMD & CF_STS_BUSY);
	
	// Wait until card is ready for commands
	while (!(MP_REG_STS & CF_STS_INSERTED));
	
	// Set number of sectors to read
	MP_REG_SEC = 1;
	
	// Set read sector
	MP_REG_LBA1 = sector & 0xFF;						// 1st byte of sector number
	MP_REG_LBA2 = (sector >> 8) & 0xFF;					// 2nd byte of sector number
	MP_REG_LBA3 = (sector >> 16) & 0xFF;				// 3rd byte of sector number
	MP_REG_LBA4 = ((sector >> 24) & 0x0F )| CF_CMD_LBA;	// last nibble of sector number
	
	// Set command to read
	MP_REG_CMD = CF_CMD_READ;
	
	// Wait until card is ready for reading
	while ((MP_REG_STS & 0xff)!= CF_STS_READY);
	
	// Read data
#ifdef _CF_USE_DMA
 #ifdef NDS
	DMA3_SRC = (u32)MP_DATA;
	DMA3_DEST = (u32)buff;
	DMA3_CR = 256 | DMA_COPY_HALFWORDS | DMA_SRC_FIX;
 #else
	DMA3COPY ( MP_DATA, buff, 256 | DMA16 | DMA_ENABLE | DMA_SRC_FIXED);
 #endif
	buff += BYTE_PER_READ / 2;
#elif defined _CF_ALLOW_UNALIGNED
	i=256;
	if ((u32)buff_u8 & 0x01) {
		while(i--){
			temp = *MP_DATA;
			*buff_u8++ = temp & 0xFF;
			*buff_u8++ = temp >> 8;
		}
	} else {
		while(i--){
			*buff++ = *MP_DATA;
		}
	}
#else
#error define error.
#endif
	
	while ((MP_REG_STS & 0xff)!= CF_STS_INSERTED);
	
#if (defined _CF_USE_DMA) && (defined NDS)
	// Wait for end of transfer before returning
	while(DMA3_CR & DMA_BUSY);
	DC_FlushRange(dmabuf,dmasize);
	DC_InvalidateRange(dmabuf,dmasize);
#endif
}

static u16 *pbuf0=NULL,*pbuf1=NULL,*pbuf2=NULL;

static void MPMD_ReadSectorsAccuracy_readone (u32 sector, void* buffer)
{
  bool alldone=false;
  bool doneseg[16]={false,};
  u16 *pbuf=(u16*)buffer;
  
  while(alldone==false){
    MPMD_ReadSectorsAccuracy_readonereal(sector,pbuf0);
    MPMD_ReadSectorsAccuracy_readonereal(sector,pbuf1);
    MPMD_ReadSectorsAccuracy_readonereal(sector,pbuf2);
    
    int idx;
    
    for(idx=0;idx<16;idx++){
      if(doneseg[idx]==false){
        bool chk=true;
        int ofs;
        
        ofs=idx*16;
        u16 *pb0=&pbuf0[ofs];
        u16 *pb1=&pbuf1[ofs];
        u16 *pb2=&pbuf2[ofs];
        
        u32 tmp;
        for(tmp=16;tmp!=0;tmp--){
          if((*pb0!=*pb1)||(*pb1!=*pb2)) chk=false;
          pb0++; pb1++; pb2++;
        }
        
        if(chk==true){
          u16 *psrc=&pbuf0[idx*16];
          u16 *pdst=&pbuf[idx*16];
          u32 tmp;
          for(tmp=16;tmp!=0;tmp--){
            *pdst++=*psrc++;
          }
          doneseg[idx]=true;
        }
        
      }
    }
    
    alldone=true;
    for(idx=0;idx<16;idx++){
      if(doneseg[idx]==false) alldone=false;
    }
    
  }
  
}

static bool MPMD_ReadSectorsAccuracy (u32 sector, u8 numSecs, void* buffer)
{
/*
  _consolePrintOne("ReadSector:0x%X,",sector);
  _consolePrintOne("%d,",numSecs);
  _consolePrintOne("%08X\n",(u32)buffer);
*/
  
  u16 *buf=(u16*)buffer;
  
  int j = (numSecs > 0 ? numSecs : 256);
  
  while (j--){
    MPMD_ReadSectorsAccuracy_readone(sector, buf);
    sector++;
    buf+=256;
  }
  
  return(true);
}

/*-----------------------------------------------------------------
MPMD_WriteSectors
Write 512 byte sector numbered "sector" from "buffer"
u32 sector IN: address of 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer IN: pointer to 512 byte buffer to read data from
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool MPMD_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{
	int i;
	int j = (numSecs > 0 ? numSecs : 256);
	u16 *buff = (u16*)buffer;
#ifdef _CF_ALLOW_UNALIGNED
	u8 *buff_u8 = (u8*)buffer;
	int temp;
#endif
	
#if defined _CF_USE_DMA && defined NDS && defined ARM9
	DC_FlushRange( buffer, j * BYTE_PER_READ);
#endif

	// Wait until CF card is finished previous commands
	i=0;
	while ((MP_REG_CMD & CF_STS_BUSY) && (i < CARD_TIMEOUT))
	{
		i++;
	}
	
	// Wait until card is ready for commands
	i = 0;
	while ((!(MP_REG_STS & CF_STS_INSERTED)) && (i < CARD_TIMEOUT))
	{
		i++;
	}
	if (i >= CARD_TIMEOUT)
		return false;
	
	// Set number of sectors to write
	MP_REG_SEC = numSecs;	
	
	// Set write sector
	MP_REG_LBA1 = sector & 0xFF;						// 1st byte of sector number
	MP_REG_LBA2 = (sector >> 8) & 0xFF;					// 2nd byte of sector number
	MP_REG_LBA3 = (sector >> 16) & 0xFF;				// 3rd byte of sector number
	MP_REG_LBA4 = ((sector >> 24) & 0x0F )| CF_CMD_LBA;	// last nibble of sector number
	
	// Set command to write
	MP_REG_CMD = CF_CMD_WRITE;
	
	while (j--)
	{
		// Wait until card is ready for writing
		i = 0;
		while (((MP_REG_STS & 0xff) != CF_STS_READY) && (i < CARD_TIMEOUT))
		{
			i++;
		}
		if (i >= CARD_TIMEOUT)
			return false;
		
		// Write data
#ifdef _CF_USE_DMA
 #ifdef NDS
		DMA3_SRC = (u32)buff;
		DMA3_DEST = (u32)MP_DATA;
		DMA3_CR = 256 | DMA_COPY_HALFWORDS | DMA_DST_FIX;
 #else
		DMA3COPY( buff, MP_DATA, 256 | DMA16 | DMA_ENABLE | DMA_DST_FIXED);
 #endif
		buff += BYTE_PER_READ / 2;
#elif defined _CF_ALLOW_UNALIGNED
		i=256;
		if ((u32)buff_u8 & 0x01) {
			while(i--)
			{
				temp = *buff_u8++;
				temp |= *buff_u8++ << 8;
				*MP_DATA = temp;
			}
		} else {
		while(i--)
			*MP_DATA = *buff++; 
		}
#else
		i=256;
		while(i--)
			*MP_DATA = *buff++; 
#endif
	}
#if defined _CF_USE_DMA && defined NDS
	// Wait for end of transfer before returning
	while(DMA3_CR & DMA_BUSY);
#endif
	
	return true;
}

/*-----------------------------------------------------------------
MPMD_Shutdown
unload the GBAMP CF interface
-----------------------------------------------------------------*/
bool MPMD_Shutdown(void) 
{
	return MPMD_ClearStatus() ;
}

/*-----------------------------------------------------------------
MPMD_StartUp
initializes the CF interface, returns true if successful,
otherwise returns false
-----------------------------------------------------------------*/
bool MPMD_StartUp(void)
{
	volatile int i;
	_consolePrintOne("\n",0);
while(1){
	for(i=0;i<0xfffff;i++);
	u8 chk0,chk1;
	MP_REG_LBA1=0x00;
	chk0=MP_REG_LBA1;
	MP_REG_LBA1=0xaa;
	chk1=MP_REG_LBA1;
	_consolePrintOne("Standby MicroDrive ready... f2 %4x,",chk0);
	_consolePrintOne("%4x\n",chk1);
	for(i=0;i<0xfffff;i++);
	if((chk0==0x00)&&(chk1==0xaa)) break;
};
return(true);

/*
	u8 temp = MP_REG_LBA1;
	MP_REG_LBA1 = (~temp & 0xFF);
	temp = (~temp & 0xFF);
	return (MP_REG_LBA1 == temp) ;
*/
}

/*-----------------------------------------------------------------
the actual interface structure
-----------------------------------------------------------------*/
IO_INTERFACE io_mpmd = {
	DEVICE_TYPE_MPMD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&MPMD_StartUp,
	(FN_MEDIUM_ISINSERTED)&MPMD_IsInserted,
	(FN_MEDIUM_READSECTORS)NULL,//&MPMD_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&MPMD_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&MPMD_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&MPMD_Shutdown
} ;

static bool Accuracy=true;

void MPMD_SetAccuracy(bool flag)
{
  Accuracy=flag;
}

/*-----------------------------------------------------------------
MPMD_GetInterface
returns the interface structure to host
-----------------------------------------------------------------*/
LPIO_INTERFACE MPMD_GetInterface(void) {
	if(Accuracy==true){
	  if(pbuf0==NULL) pbuf0=(u16*)malloc(512);
	  if(pbuf1==NULL) pbuf1=(u16*)malloc(512);
	  if(pbuf2==NULL) pbuf2=(u16*)malloc(512);
	  io_mpmd.fn_ReadSectors=(FN_MEDIUM_READSECTORS)&MPMD_ReadSectorsAccuracy;
	  }else{
	  io_mpmd.fn_ReadSectors=(FN_MEDIUM_READSECTORS)&MPMD_ReadSectorsSpeed;
	}
	
	return &io_mpmd ;
} ;

#endif // SUPPORT_MPMD
