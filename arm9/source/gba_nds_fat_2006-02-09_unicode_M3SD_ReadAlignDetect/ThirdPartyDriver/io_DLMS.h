/********************************************************
	by Rudolph (çcíÈ)
		2006/11/21 First release


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

#ifndef IO_DLMS_H
#define IO_DLMS_H

// 'DLMS'
#define DEVICE_TYPE_DLMS 0x534D4C44

#include "disc_io.h"

// export interface
extern LPIO_INTERFACE DLMS_GetInterface(void) ;

#endif
