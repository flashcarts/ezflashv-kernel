//////////////////////////////////////////////////////////////////////
//
// ipc.h -- Inter-processor communication
//
// version 0.1, February 14, 2005
//
//  Copyright (C) 2005 Michael Noland (joat) and Jason Rogers (dovoto)
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any
//  damages arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any
//  purpose, including commercial applications, and to alter it and
//  redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source
//     distribution.
//
// Changelog:
//   0.1: First version
//
//////////////////////////////////////////////////////////////////////

#ifndef NDS_IPC4_INCLUDE
#define NDS_IPC4_INCLUDE

//////////////////////////////////////////////////////////////////////

#include <nds.h>

//////////////////////////////////////////////////////////////////////

typedef struct sTransferRegion4 {
  int ready;
  s32 xscale, yscale;
  s32 xoffset, yoffset;
} TransferRegion4, * pTransferRegion4;

//////////////////////////////////////////////////////////////////////

#define IPC4 ((TransferRegion4 volatile *)(0x027FF000))

#endif

//////////////////////////////////////////////////////////////////////

