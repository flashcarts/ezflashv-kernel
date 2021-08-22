/*******************************************************
	by Rudolph (çcíÈ)
		2006/11/24 First release


 Hardware Routines for microSD card using a DSLink.

Redistribution and use in source and binary forms,
with or without modification, are permitted provided 
that the following conditions are met:

 Only the Homebrew application can be used. 
 It is not possible to use it by a business purpose. 

 This software is made based on information obtained 
 by Reverse engineering. 

 Please use that at once when a source code that is 
 more formal than the official is open to the public.

<File composition>
	io_DLMS.c
	io_DLMS.h
	io_DLMS_asm.s

********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "io_DLMS.h"


extern	bool	DLMS_Init_s(void);
extern	void	DLMS_Read_s(u32, void* buffer, u32 size);
extern	void	DLMS_Write_s(u32, void* buffer, u32 size);
extern	void	DLMS_stmd_s(u32 md);

/*******
#define	CARD_COMMAND_W1   (*(vu32*)0x040001A8)
#define	CARD_COMMAND_W2   (*(vu32*)0x040001AC)

void _dlms_stmd(u32 md)
{

	CARD_CR1H = CARD_CR1_ENABLE | CARD_CR1_IRQ;
	CARD_COMMAND_W1 = 0xEDEDDEDE;
	CARD_COMMAND_W2 = md << 24;
	CARD_CR2 = CARD_ACTIVATE | CARD_nRESET;

	while (!(CARD_CR2 & CARD_BUSY)) ;
}
*******/

/*-----------------------------------------------------------------
	 io interface
-------------------------------------------------------------------*/
bool DLMS_IsInserted(void) 
{
	return true;
}

bool DLMS_ClearStatus(void) 
{
	bool	ret;

//	WAIT_CR &= 0xF7FF;		// Card access ARM9
	swiDelay(0x200000);

	DLMS_stmd_s(1);

	ret = DLMS_Init_s();

	DLMS_stmd_s(0);

	return ret;
}

bool DLMS_ReadSectors (u32 sector, u8 numSecs, void* buffer)
{
	u32	size;

//	WAIT_CR &= 0xF7FF;		// Card access ARM9
	DLMS_stmd_s(1);

	if(!numSecs)	numSecs = 1;
	size = numSecs * 512;
	DC_FlushRange(buffer, size);

	DLMS_Read_s(sector * 512, buffer, size);

	DLMS_stmd_s(0);

	return true;
}


bool DLMS_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{
	u32	size;

//	WAIT_CR &= 0xF7FF;		// Card access ARM9
	DLMS_stmd_s(1);

	if(!numSecs)	numSecs = 1;
	size = numSecs * 512;
	DC_FlushRange(buffer, size);

	DLMS_Write_s(sector * 512, buffer, size);

	DLMS_stmd_s(0);

	return true;
}


bool DLMS_Shutdown(void)
{
//	WAIT_CR &= 0xF7FF;		// Card access ARM9
	DLMS_stmd_s(0);

	return true;
}

bool DLMS_StartUp(void)
{
	return DLMS_ClearStatus();
}


/*-----------------------------------------------------------------
	the actual interface structure
-------------------------------------------------------------------*/
IO_INTERFACE io_DLMS = {
	DEVICE_TYPE_DLMS,	// 'DLMS'
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_NDS,
	(FN_MEDIUM_STARTUP)&DLMS_StartUp,
	(FN_MEDIUM_ISINSERTED)&DLMS_IsInserted,
	(FN_MEDIUM_READSECTORS)&DLMS_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&DLMS_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&DLMS_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&DLMS_Shutdown
} ;


/*-----------------------------------------------------------------
DLMS_GetInterface
	returns the interface structure to host
-------------------------------------------------------------------*/
LPIO_INTERFACE DLMS_GetInterface(void) {
	return &io_DLMS;
} ;

