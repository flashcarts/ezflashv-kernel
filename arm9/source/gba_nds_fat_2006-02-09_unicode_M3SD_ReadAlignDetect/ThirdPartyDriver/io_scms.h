/*
	io_sccf.h 

	Hardware Routines for reading a compact flash card
	using the GBA Movie Player

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#ifndef IO_SCMS_H
#define IO_SCMS_H

#include "disc_io.h"

// export interface
extern LPIO_INTERFACE SCMS_GetInterface(void) ;

#endif	// define IO_SCMS_H
