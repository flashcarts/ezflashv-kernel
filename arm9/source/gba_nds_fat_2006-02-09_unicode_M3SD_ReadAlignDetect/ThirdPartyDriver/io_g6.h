/*
	io_g6.h 
	
	Hardware Routines for accessing G6 flash card

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
			
	
*/

#ifndef IO_G6_H
#define IO_G6_H

	#include "disc_io.h"

	#define DEVICE_TYPE_G6 0x4C4C3647 // 'G6LL'

	u16 G6_SetChipReg(u16 Data);
	bool G6_SelectSaver(u32 Bank);
	extern LPIO_INTERFACE G6_GetInterface(void);
	
#endif	

