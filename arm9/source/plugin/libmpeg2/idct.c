/*
 * idct.c
 * Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
 * See http://libmpeg2.sourceforge.net/ for updates.
 *
 * mpeg2dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Modified for use with MPlayer, see libmpeg-0.4.0.diff for the exact changes.
 * detailed CVS changelog at http://www.mplayerhq.hu/cgi-bin/cvsweb.cgi/main/
 * $Id: idct.c,v 1.12 2005/02/19 02:32:12 diego Exp $
 */

#include "config.h"

#include <stdlib.h>
#include <inttypes.h>

#include "mpeg2.h"
#include "attributes.h"
#include "mpeg2_internal.h"

#include "../../arm9tcm.h"

#define W1 2841 /* 2048 * sqrt (2) * cos (1 * pi / 16) */
#define W2 2676 /* 2048 * sqrt (2) * cos (2 * pi / 16) */
#define W3 2408 /* 2048 * sqrt (2) * cos (3 * pi / 16) */
#define W5 1609 /* 2048 * sqrt (2) * cos (5 * pi / 16) */
#define W6 1108 /* 2048 * sqrt (2) * cos (6 * pi / 16) */
#define W7 565  /* 2048 * sqrt (2) * cos (7 * pi / 16) */

/* idct main entry point  */
void (*mpeg2_idct_copy)(int16_t * block, uint8_t * dest, int stride);
void (*mpeg2_idct_add)(int last, int16_t * block, uint8_t * dest, int stride);

extern void mpeg2_idct_init_cliptable(void);
extern void mpeg2_idct_copy_cfast_clip(int16_t * block, uint8_t * dest, const int stride);
extern void mpeg2_idct_add_c_clip(const int last, int16_t * block, uint8_t * dest, const int stride);
extern void mpeg2_idct_copy_cfast_noclip(int16_t * block, uint8_t * dest, const int stride);
extern void mpeg2_idct_add_c_noclip(const int last, int16_t * block, uint8_t * dest, const int stride);

void mpeg2_idct_init (uint32_t accel)
{
//	extern uint8_t mpeg2_scan_norm[64];
//	extern uint8_t mpeg2_scan_alt[64];
//	int i, j;

	mpeg2_idct_init_cliptable();
	
	mpeg2_idct_copy = mpeg2_idct_copy_cfast_clip;
	mpeg2_idct_add = mpeg2_idct_add_c_clip;
	
/*
	for (i = 0; i < 64; i++) {
	    j = mpeg2_scan_norm[i];
	    mpeg2_scan_norm[i] = ((j & 0x36) >> 1) | ((j & 0x09) << 2);
	    j = mpeg2_scan_alt[i];
	    mpeg2_scan_alt[i] = ((j & 0x36) >> 1) | ((j & 0x09) << 2);
	}
*/
}

void mpeg2_idct_SetClipMode(bool UseClip)
{
  if(UseClip==true){
    mpeg2_idct_copy = mpeg2_idct_copy_cfast_clip;
    mpeg2_idct_add = mpeg2_idct_add_c_clip;
    }else{
    mpeg2_idct_copy = mpeg2_idct_copy_cfast_noclip;
    mpeg2_idct_add = mpeg2_idct_add_c_noclip;
  }
}

