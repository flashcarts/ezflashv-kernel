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

#include "io_scms.h"

//---------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------

extern void scms_InitSCMode(void);
extern void scms_ReadSector(u16 *buff,u32 sector,u8 ReadNumber);
extern void scms_WriteSector(u16 *buff,u32 sector,u8 writeNumber);
#ifdef __cplusplus
}	   // extern "C"
#endif

extern bool MPCF_ClearStatus (void);

#define SC_SD_COMMAND 0x09800000

/*-----------------------------------------------------------------
SCMS_Unlock
Returns true if SuperCard was unlocked, false if failed
Added by MightyMax
Modified by Chishm
-----------------------------------------------------------------*/
extern bool scms_MemoryCard_IsInserted(void);

bool SCMS_IsInserted(void)
{
	return scms_MemoryCard_IsInserted();

//    return ((SC_SD_COMMAND & 0x0300) == 0);
}

bool SCMS_Unlock(void)
{
//    *(vu16*)0x4000204 &= ~(0x0880); //set WAIT_CR to access
    volatile short *unlockAddress = (volatile short *)0x09FFFFFE;
    *unlockAddress = 0xA55A ;
    *unlockAddress = 0xA55A ;
    *unlockAddress = 0x3 ;
    *unlockAddress = 0x3 ;
    return (SCMS_IsInserted());
}

bool SCMS_ReadSectors (u32 sector, u8 numSecs, void* buffer)
{
    scms_ReadSector(buffer,sector,numSecs);
//    ReadSector((u16 *)buffer,sector,numSecs);
    return true;
}

bool SCMS_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{
    scms_WriteSector((u16 *)buffer, sector, numSecs);
//    WriteSector((u16 *)buffer, sector, numSecs);
    return true;
}

bool SCMS_Shutdown(void) {
	return MPCF_ClearStatus() ;
} ;

bool SCMS_StartUp(void) {
	return SCMS_Unlock() ;
} ;


IO_INTERFACE io_scms = {
	0x534D4353,	// 'SCMS'
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&SCMS_StartUp,
	(FN_MEDIUM_ISINSERTED)&SCMS_IsInserted,
	(FN_MEDIUM_READSECTORS)&SCMS_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&SCMS_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&MPCF_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&SCMS_Shutdown
} ;


LPIO_INTERFACE SCMS_GetInterface(void) {
	return &io_scms ;
} ;
