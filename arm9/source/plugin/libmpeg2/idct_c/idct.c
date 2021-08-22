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

/*
 * In legal streams, the IDCT output should be between -384 and +384.
 * In corrupted streams, it is possible to force the IDCT output to go
 * to +-3826 - this is the worst case for a column IDCT where the
 * column inputs are 16-bit values.
 */
#define ClipMargen (256)
static DATA_IN_DTCM uint8_t mpeg2_clip[ClipMargen * 2 + 256];
static const uint8_t *pmpeg2_clip=mpeg2_clip + ClipMargen;
//#define CLIP(i) (pmpeg2_clip[i])

//#define CLIP(i) ((i) < 0) ? 0 : (((i) > 255) ? 255 : (i))
//#define CLIP(i) ( (((i)&~0xff)==0) ? (i) : pmpeg2_clip[i])
#define CLIP(i) i
//#define CLIP(i) (((i) < 0) ? 0 : (i))

/*
static inline int CLIP(int i)
{
  if((i)<0) return(0);
  return(i);
}
*/

#if 0
#define BUTTERFLY(t0,t1,W0,W1,d0,d1)	\
do {					\
    t0 = W0 * d0 + W1 * d1;		\
    t1 = W0 * d1 - W1 * d0;		\
} while (0)
#else
#define BUTTERFLY(t0,t1,W0,W1,d0,d1)	\
do {					\
    int tmp = W0 * (d0 + d1);		\
    t0 = tmp + (W1 - W0) * d1;		\
    t1 = tmp - (W1 + W0) * d0;		\
} while (0)
#endif

static inline void idct_row (int16_t * const block)
{
    int d0, d1, d2, d3;
    int a0, a1, a2, a3, b0, b1, b2, b3;
    int t0, t1, t2, t3;

    /* shortcut */
    if (likely (!(block[1] | ((int32_t *)block)[1] | ((int32_t *)block)[2] |
		  ((int32_t *)block)[3]))) {
	uint32_t tmp = (uint16_t) (block[0] >> 1);
	tmp |= tmp << 16;
	((int32_t *)block)[0] = tmp;
	((int32_t *)block)[1] = tmp;
	((int32_t *)block)[2] = tmp;
	((int32_t *)block)[3] = tmp;
	return;
    }

    d0 = (block[0] << 11) + 2048;
    d1 = block[1];
    d2 = block[2] << 11;
    d3 = block[3];
    t0 = d0 + d2;
    t1 = d0 - d2;
    BUTTERFLY (t2, t3, W6, W2, d3, d1);
    a0 = t0 + t2;
    a1 = t1 + t3;
    a2 = t1 - t3;
    a3 = t0 - t2;

    d0 = block[4];
    d1 = block[5];
    d2 = block[6];
    d3 = block[7];
    BUTTERFLY (t0, t1, W7, W1, d3, d0);
    BUTTERFLY (t2, t3, W3, W5, d1, d2);
    b0 = t0 + t2;
    b3 = t1 + t3;
    t0 -= t2;
    t1 -= t3;
    b1 = ((t0 + t1) >> 8) * 181;
    b2 = ((t0 - t1) >> 8) * 181;

    block[0] = (a0 + b0) >> 12;
    block[1] = (a1 + b1) >> 12;
    block[2] = (a2 + b2) >> 12;
    block[3] = (a3 + b3) >> 12;
    block[4] = (a3 - b3) >> 12;
    block[5] = (a2 - b2) >> 12;
    block[6] = (a1 - b1) >> 12;
    block[7] = (a0 - b0) >> 12;
}

static inline void idct_col (int16_t * const block)
{
    int d0, d1, d2, d3;
    int a0, a1, a2, a3, b0, b1, b2, b3;
    int t0, t1, t2, t3;

    d0 = (block[8*0] << 11) + 65536;
    d1 = block[8*1];
    d2 = block[8*2] << 11;
    d3 = block[8*3];
    t0 = d0 + d2;
    t1 = d0 - d2;
    BUTTERFLY (t2, t3, W6, W2, d3, d1);
    a0 = t0 + t2;
    a1 = t1 + t3;
    a2 = t1 - t3;
    a3 = t0 - t2;

    d0 = block[8*4];
    d1 = block[8*5];
    d2 = block[8*6];
    d3 = block[8*7];
    BUTTERFLY (t0, t1, W7, W1, d3, d0);
    BUTTERFLY (t2, t3, W3, W5, d1, d2);
    b0 = t0 + t2;
    b3 = t1 + t3;
    t0 -= t2;
    t1 -= t3;
    b1 = ((t0 + t1) >> 8) * 181;
    b2 = ((t0 - t1) >> 8) * 181;

    block[8*0] = (a0 + b0) >> 17;
    block[8*1] = (a1 + b1) >> 17;
    block[8*2] = (a2 + b2) >> 17;
    block[8*3] = (a3 + b3) >> 17;
    block[8*4] = (a3 - b3) >> 17;
    block[8*5] = (a2 - b2) >> 17;
    block[8*6] = (a1 - b1) >> 17;
    block[8*7] = (a0 - b0) >> 17;
}

static inline void idct_row_copy (int16_t * const block, uint8_t * dest)
{
    int d0, d1, d2, d3;
    int a0, a1, a2, a3, b0, b1, b2, b3;
    int t0, t1, t2, t3;

    /* shortcut */
    if (likely (!(block[1] | ((int32_t *)block)[1] | ((int32_t *)block)[2] |
		  ((int32_t *)block)[3]))) {
	uint32_t tmp = CLIP((uint16_t) (block[0] >> 1));
	tmp |= tmp << 8;
	tmp |= tmp << 16;
	uint32_t *dest32=(uint32_t*)&dest[0];
	dest32[0] = tmp;
	dest32[1] = tmp;
	return;
    }

    d0 = (block[0] << 11) + 2048;
    d1 = block[1];
    d2 = block[2] << 11;
    d3 = block[3];
    t0 = d0 + d2;
    t1 = d0 - d2;
    BUTTERFLY (t2, t3, W6, W2, d3, d1);
    a0 = t0 + t2;
    a1 = t1 + t3;
    a2 = t1 - t3;
    a3 = t0 - t2;

    d0 = block[4];
    d1 = block[5];
    d2 = block[6];
    d3 = block[7];
    
    BUTTERFLY (t0, t1, W7, W1, d3, d0);
    BUTTERFLY (t2, t3, W3, W5, d1, d2);
    b0 = t0 + t2;
    b3 = t1 + t3;
    t0 -= t2;
    t1 -= t3;
    b1 = ((t0 + t1) >> 8) * 181;
    b2 = ((t0 - t1) >> 8) * 181;
    
    dest[0] = CLIP ((a0 + b0) >> 12);
    dest[1] = CLIP ((a1 + b1) >> 12);
    dest[2] = CLIP ((a2 + b2) >> 12);
    dest[3] = CLIP ((a3 + b3) >> 12);
    dest[4] = CLIP ((a3 - b3) >> 12);
    dest[5] = CLIP ((a2 - b2) >> 12);
    dest[6] = CLIP ((a1 - b1) >> 12);
    dest[7] = CLIP ((a0 - b0) >> 12);
}

static inline void idct_row_add (int16_t * const block, uint8_t * dest)
{
    int d0, d1, d2, d3;
    int a0, a1, a2, a3, b0, b1, b2, b3;
    int t0, t1, t2, t3;

    /* shortcut */
    if (likely (!(block[1] | ((int32_t *)block)[1] | ((int32_t *)block)[2] |
		  ((int32_t *)block)[3]))) {
	uint32_t tmp = (uint32_t)(block[0] >> 1);
	block[0] = 0;
	dest[0] = CLIP(dest[0]+tmp);
	dest[1] = CLIP(dest[1]+tmp);
	dest[2] = CLIP(dest[2]+tmp);
	dest[3] = CLIP(dest[3]+tmp);
	dest[4] = CLIP(dest[4]+tmp);
	dest[5] = CLIP(dest[5]+tmp);
	dest[6] = CLIP(dest[6]+tmp);
	dest[7] = CLIP(dest[7]+tmp);
	return;
    }

    d0 = (block[0] << 11) + 2048;
    d1 = block[1];
    d2 = block[2] << 11;
    d3 = block[3];
    t0 = d0 + d2;
    t1 = d0 - d2;
    BUTTERFLY (t2, t3, W6, W2, d3, d1);
    a0 = t0 + t2;
    a1 = t1 + t3;
    a2 = t1 - t3;
    a3 = t0 - t2;

    d0 = block[4];
    d1 = block[5];
    d2 = block[6];
    d3 = block[7];
    
    {
      uint32_t *block32=(uint32_t*)&block[0];
      block32[0]=0; block32[1]=0; block32[2]=0; block32[3]=0;
    }
    
    BUTTERFLY (t0, t1, W7, W1, d3, d0);
    BUTTERFLY (t2, t3, W3, W5, d1, d2);
    b0 = t0 + t2;
    b3 = t1 + t3;
    t0 -= t2;
    t1 -= t3;
    b1 = ((t0 + t1) >> 8) * 181;
    b2 = ((t0 - t1) >> 8) * 181;

    dest[0] = CLIP (dest[0] + ((a0 + b0) >> 12));
    dest[1] = CLIP (dest[1] + ((a1 + b1) >> 12));
    dest[2] = CLIP (dest[2] + ((a2 + b2) >> 12));
    dest[3] = CLIP (dest[3] + ((a3 + b3) >> 12));
    dest[4] = CLIP (dest[4] + ((a3 - b3) >> 12));
    dest[5] = CLIP (dest[5] + ((a2 - b2) >> 12));
    dest[6] = CLIP (dest[6] + ((a1 - b1) >> 12));
    dest[7] = CLIP (dest[7] + ((a0 - b0) >> 12));
}

CODE_IN_ITCM static void mpeg2_idct_copy_cfast(int16_t * block, uint8_t * dest, const int stride)
{
    int idx;
    
    int16_t *_block=block;
    
//    for(idx=0;idx<8;idx++){
    for(idx=7;idx!=-1;idx--){
      idct_col(block+idx);
    }
    
    for(idx=8;idx!=0;idx--){
      idct_row_copy(block,dest);
      dest+=stride;
      block+=8;
    }
    
    int val=0;
    swiFastCopy(&val,_block,COPY_MODE_FILL | (64*2/4));
}

CODE_IN_ITCM static void mpeg2_idct_add_c(const int last, int16_t * block, uint8_t * dest, const int stride)
{
    int i;

    if (last != 129 || (block[0] & (7 << 4)) == (4 << 4)) {
	for (i = 0; i < 8; i++)
	    idct_row (block + 8 * i);
	for (i = 0; i < 8; i++)
	    idct_col (block + i);
	do {
	    dest[0] = CLIP (block[0] + dest[0]);
	    dest[1] = CLIP (block[1] + dest[1]);
	    dest[2] = CLIP (block[2] + dest[2]);
	    dest[3] = CLIP (block[3] + dest[3]);
	    dest[4] = CLIP (block[4] + dest[4]);
	    dest[5] = CLIP (block[5] + dest[5]);
	    dest[6] = CLIP (block[6] + dest[6]);
	    dest[7] = CLIP (block[7] + dest[7]);

	    ((int32_t *)block)[0] = 0;	((int32_t *)block)[1] = 0;
	    ((int32_t *)block)[2] = 0;	((int32_t *)block)[3] = 0;

	    dest += stride;
	    block += 8;
	} while (--i);
    } else {
	int DC;

	DC = (block[0] + 64) >> 7;
	block[0] = block[63] = 0;
	i = 8;
	do {
	    dest[0] = CLIP (DC + dest[0]);
	    dest[1] = CLIP (DC + dest[1]);
	    dest[2] = CLIP (DC + dest[2]);
	    dest[3] = CLIP (DC + dest[3]);
	    dest[4] = CLIP (DC + dest[4]);
	    dest[5] = CLIP (DC + dest[5]);
	    dest[6] = CLIP (DC + dest[6]);
	    dest[7] = CLIP (DC + dest[7]);
	    dest += stride;
	} while (--i);
    }
}

void mpeg2_idct_init (uint32_t accel)
{
//	extern uint8_t mpeg2_scan_norm[64];
//	extern uint8_t mpeg2_scan_alt[64];
	int i, j;

	mpeg2_idct_copy = mpeg2_idct_copy_cfast;
	mpeg2_idct_add = mpeg2_idct_add_c;
	for (i = -ClipMargen; i < ClipMargen+256; i++)
	    mpeg2_clip[ClipMargen+i] = (i < 0) ? 0 : ((i > 255) ? 255 : i);
/*
	for (i = 0; i < 64; i++) {
	    j = mpeg2_scan_norm[i];
	    mpeg2_scan_norm[i] = ((j & 0x36) >> 1) | ((j & 0x09) << 2);
	    j = mpeg2_scan_alt[i];
	    mpeg2_scan_alt[i] = ((j & 0x36) >> 1) | ((j & 0x09) << 2);
	}
*/
}
