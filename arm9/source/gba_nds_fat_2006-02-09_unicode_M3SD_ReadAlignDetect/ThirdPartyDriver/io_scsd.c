/*
	io_scsd.c based on

	compact_flash.c
	By chishm (Michael Chisholm)

	Hardware Routines for reading a compact flash card
	using the Super Card CF

	CF routines modified with help from Darkfader

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#include "io_scsd.h"

//---------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------

extern void InitSCMode(void);
extern void ReadSector(u16 *buff,u32 sector,u8 ReadNumber);
extern void WriteSector(u16 *buff,u32 sector,u8 writeNumber);
#ifdef __cplusplus
}	   // extern "C"
#endif

extern bool MPCF_ClearStatus (void);

#define SC_SD_COMMAND 0x09800000

/*-----------------------------------------------------------------
SCSD_Unlock
Returns true if SuperCard was unlocked, false if failed
Added by MightyMax
Modified by Chishm
-----------------------------------------------------------------*/
extern bool MemoryCard_IsInserted(void);

bool SCSD_IsInserted(void)
{
	return MemoryCard_IsInserted();

//    return ((SC_SD_COMMAND & 0x0300) == 0);
}

bool SCSD_Unlock(void)
{
//    *(vu16*)0x4000204 &= ~(0x0880); //set WAIT_CR to access
    volatile short *unlockAddress = (volatile short *)0x09FFFFFE;
    *unlockAddress = 0xA55A ;
    *unlockAddress = 0xA55A ;
    *unlockAddress = 0x3 ;
    *unlockAddress = 0x3 ;
    return (SCSD_IsInserted());
}

bool SCSD_ReadSectors (u32 sector, u8 numSecs, void* buffer)
{
    ReadSector(buffer,sector,numSecs);
//    ReadSector((u16 *)buffer,sector,numSecs);
    return true;
}

bool SCSD_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{
    WriteSector((u16 *)buffer, sector, numSecs);
//    WriteSector((u16 *)buffer, sector, numSecs);
    return true;
}

bool SCSD_Shutdown(void) {
	return MPCF_ClearStatus() ;
} ;

bool SCSD_StartUp(void) {
	return SCSD_Unlock() ;
} ;


IO_INTERFACE io_scsd = {
	0x44534353,	// 'SCSD'
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&SCSD_StartUp,
	(FN_MEDIUM_ISINSERTED)&SCSD_IsInserted,
	(FN_MEDIUM_READSECTORS)&SCSD_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&SCSD_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&MPCF_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&SCSD_Shutdown
} ;


LPIO_INTERFACE SCSD_GetInterface(void) {
	return &io_scsd ;
} ;
