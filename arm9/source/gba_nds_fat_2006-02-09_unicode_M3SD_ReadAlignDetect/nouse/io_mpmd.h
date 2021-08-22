/*
	io_mpmd.h based io_mpmd.h

	Hardware Routines for reading a compact flash card
	using the GBA Movie Player

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#ifndef IO_MPMD_H
#define IO_MPMD_H

// 'MPMD'
#define DEVICE_TYPE_MPMD 0x444D504D

#include "disc_io.h"

// export interface
extern void MPMD_SetAccuracy(bool flag) ;
extern LPIO_INTERFACE MPMD_GetInterface(void) ;

#endif	// define IO_MPMD_H
