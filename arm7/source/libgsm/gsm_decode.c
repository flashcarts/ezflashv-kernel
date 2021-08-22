/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

#include "gsm.h"
#include "gsm610_priv.h"

int gsm_decode (gsm s, gsm_byte * c, gsm_signal * target)
{
  
//	static word  	LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13*4];
	word  	LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13*4];

#ifdef WAV49
//	if (s->wav_fmt) {
if(1){
		uword sr = 0;

		s->frame_index = !s->frame_index;
		if (s->frame_index) {

			sr = *c++;
			LARc[0] = sr & 0x3f;  sr >>= 6;
			sr |= (uword)*c++ << 2;
			LARc[1] = sr & 0x3f;  sr >>= 6;
			sr |= (uword)*c++ << 4;
			LARc[2] = sr & 0x1f;  sr >>= 5;
			LARc[3] = sr & 0x1f;  sr >>= 5;
			sr |= (uword)*c++ << 2;
			LARc[4] = sr & 0xf;  sr >>= 4;
			LARc[5] = sr & 0xf;  sr >>= 4;
			sr |= (uword)*c++ << 2;			/* 5 */
			LARc[6] = sr & 0x7;  sr >>= 3;
			LARc[7] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 4;
			Nc[0] = sr & 0x7f;  sr >>= 7;
			bc[0] = sr & 0x3;  sr >>= 2;
			Mc[0] = sr & 0x3;  sr >>= 2;
			sr |= (uword)*c++ << 1;
			xmaxc[0] = sr & 0x3f;  sr >>= 6;
			xmc[0] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			xmc[1] = sr & 0x7;  sr >>= 3;
			xmc[2] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;
			xmc[3] = sr & 0x7;  sr >>= 3;
			xmc[4] = sr & 0x7;  sr >>= 3;
			xmc[5] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;			/* 10 */
			xmc[6] = sr & 0x7;  sr >>= 3;
			xmc[7] = sr & 0x7;  sr >>= 3;
			xmc[8] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			xmc[9] = sr & 0x7;  sr >>= 3;
			xmc[10] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;
			xmc[11] = sr & 0x7;  sr >>= 3;
			xmc[12] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 4;
			Nc[1] = sr & 0x7f;  sr >>= 7;
			bc[1] = sr & 0x3;  sr >>= 2;
			Mc[1] = sr & 0x3;  sr >>= 2;
			sr |= (uword)*c++ << 1;
			xmaxc[1] = sr & 0x3f;  sr >>= 6;
			xmc[13] = sr & 0x7;  sr >>= 3;
			sr = *c++;				/* 15 */
			xmc[14] = sr & 0x7;  sr >>= 3;
			xmc[15] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;
			xmc[16] = sr & 0x7;  sr >>= 3;
			xmc[17] = sr & 0x7;  sr >>= 3;
			xmc[18] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;
			xmc[19] = sr & 0x7;  sr >>= 3;
			xmc[20] = sr & 0x7;  sr >>= 3;
			xmc[21] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			xmc[22] = sr & 0x7;  sr >>= 3;
			xmc[23] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;
			xmc[24] = sr & 0x7;  sr >>= 3;
			xmc[25] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 4;			/* 20 */
			Nc[2] = sr & 0x7f;  sr >>= 7;
			bc[2] = sr & 0x3;  sr >>= 2;
			Mc[2] = sr & 0x3;  sr >>= 2;
			sr |= (uword)*c++ << 1;
			xmaxc[2] = sr & 0x3f;  sr >>= 6;
			xmc[26] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			xmc[27] = sr & 0x7;  sr >>= 3;
			xmc[28] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;
			xmc[29] = sr & 0x7;  sr >>= 3;
			xmc[30] = sr & 0x7;  sr >>= 3;
			xmc[31] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;
			xmc[32] = sr & 0x7;  sr >>= 3;
			xmc[33] = sr & 0x7;  sr >>= 3;
			xmc[34] = sr & 0x7;  sr >>= 3;
			sr = *c++;				/* 25 */
			xmc[35] = sr & 0x7;  sr >>= 3;
			xmc[36] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;
			xmc[37] = sr & 0x7;  sr >>= 3;
			xmc[38] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 4;
			Nc[3] = sr & 0x7f;  sr >>= 7;
			bc[3] = sr & 0x3;  sr >>= 2;
			Mc[3] = sr & 0x3;  sr >>= 2;
			sr |= (uword)*c++ << 1;
			xmaxc[3] = sr & 0x3f;  sr >>= 6;
			xmc[39] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			xmc[40] = sr & 0x7;  sr >>= 3;
			xmc[41] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;			/* 30 */
			xmc[42] = sr & 0x7;  sr >>= 3;
			xmc[43] = sr & 0x7;  sr >>= 3;
			xmc[44] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;
			xmc[45] = sr & 0x7;  sr >>= 3;
			xmc[46] = sr & 0x7;  sr >>= 3;
			xmc[47] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			xmc[48] = sr & 0x7;  sr >>= 3;
			xmc[49] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;
			xmc[50] = sr & 0x7;  sr >>= 3;
			xmc[51] = sr & 0x7;  sr >>= 3;

			s->frame_chain = sr & 0xf;
		}
		else {
			sr = s->frame_chain;
			sr |= (uword)*c++ << 4;			/* 1 */
			LARc[0] = sr & 0x3f;  sr >>= 6;
			LARc[1] = sr & 0x3f;  sr >>= 6;
			sr = *c++;
			LARc[2] = sr & 0x1f;  sr >>= 5;
			sr |= (uword)*c++ << 3;
			LARc[3] = sr & 0x1f;  sr >>= 5;
			LARc[4] = sr & 0xf;  sr >>= 4;
			sr |= (uword)*c++ << 2;
			LARc[5] = sr & 0xf;  sr >>= 4;
			LARc[6] = sr & 0x7;  sr >>= 3;
			LARc[7] = sr & 0x7;  sr >>= 3;
			sr = *c++;				/* 5 */
			Nc[0] = sr & 0x7f;  sr >>= 7;
			sr |= (uword)*c++ << 1;
			bc[0] = sr & 0x3;  sr >>= 2;
			Mc[0] = sr & 0x3;  sr >>= 2;
			sr |= (uword)*c++ << 5;
			xmaxc[0] = sr & 0x3f;  sr >>= 6;
			xmc[0] = sr & 0x7;  sr >>= 3;
			xmc[1] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;
			xmc[2] = sr & 0x7;  sr >>= 3;
			xmc[3] = sr & 0x7;  sr >>= 3;
			xmc[4] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			xmc[5] = sr & 0x7;  sr >>= 3;
			xmc[6] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;			/* 10 */
			xmc[7] = sr & 0x7;  sr >>= 3;
			xmc[8] = sr & 0x7;  sr >>= 3;
			xmc[9] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;
			xmc[10] = sr & 0x7;  sr >>= 3;
			xmc[11] = sr & 0x7;  sr >>= 3;
			xmc[12] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			Nc[1] = sr & 0x7f;  sr >>= 7;
			sr |= (uword)*c++ << 1;
			bc[1] = sr & 0x3;  sr >>= 2;
			Mc[1] = sr & 0x3;  sr >>= 2;
			sr |= (uword)*c++ << 5;
			xmaxc[1] = sr & 0x3f;  sr >>= 6;
			xmc[13] = sr & 0x7;  sr >>= 3;
			xmc[14] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;			/* 15 */
			xmc[15] = sr & 0x7;  sr >>= 3;
			xmc[16] = sr & 0x7;  sr >>= 3;
			xmc[17] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			xmc[18] = sr & 0x7;  sr >>= 3;
			xmc[19] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;
			xmc[20] = sr & 0x7;  sr >>= 3;
			xmc[21] = sr & 0x7;  sr >>= 3;
			xmc[22] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;
			xmc[23] = sr & 0x7;  sr >>= 3;
			xmc[24] = sr & 0x7;  sr >>= 3;
			xmc[25] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			Nc[2] = sr & 0x7f;  sr >>= 7;
			sr |= (uword)*c++ << 1;			/* 20 */
			bc[2] = sr & 0x3;  sr >>= 2;
			Mc[2] = sr & 0x3;  sr >>= 2;
			sr |= (uword)*c++ << 5;
			xmaxc[2] = sr & 0x3f;  sr >>= 6;
			xmc[26] = sr & 0x7;  sr >>= 3;
			xmc[27] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;	
			xmc[28] = sr & 0x7;  sr >>= 3;
			xmc[29] = sr & 0x7;  sr >>= 3;
			xmc[30] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			xmc[31] = sr & 0x7;  sr >>= 3;
			xmc[32] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;
			xmc[33] = sr & 0x7;  sr >>= 3;
			xmc[34] = sr & 0x7;  sr >>= 3;
			xmc[35] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;			/* 25 */
			xmc[36] = sr & 0x7;  sr >>= 3;
			xmc[37] = sr & 0x7;  sr >>= 3;
			xmc[38] = sr & 0x7;  sr >>= 3;
			sr = *c++;
			Nc[3] = sr & 0x7f;  sr >>= 7;
			sr |= (uword)*c++ << 1;		
			bc[3] = sr & 0x3;  sr >>= 2;
			Mc[3] = sr & 0x3;  sr >>= 2;
			sr |= (uword)*c++ << 5;
			xmaxc[3] = sr & 0x3f;  sr >>= 6;
			xmc[39] = sr & 0x7;  sr >>= 3;
			xmc[40] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;
			xmc[41] = sr & 0x7;  sr >>= 3;
			xmc[42] = sr & 0x7;  sr >>= 3;
			xmc[43] = sr & 0x7;  sr >>= 3;
			sr = *c++;				/* 30 */
			xmc[44] = sr & 0x7;  sr >>= 3;
			xmc[45] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 2;
			xmc[46] = sr & 0x7;  sr >>= 3;
			xmc[47] = sr & 0x7;  sr >>= 3;
			xmc[48] = sr & 0x7;  sr >>= 3;
			sr |= (uword)*c++ << 1;
			xmc[49] = sr & 0x7;  sr >>= 3;
			xmc[50] = sr & 0x7;  sr >>= 3;
			xmc[51] = sr & 0x7;  sr >>= 3;
		}
	}
	else
#endif
	{
	}
	
	Gsm_Decoder(s, LARc, Nc, bc, Mc, xmaxc, xmc, target);
  
	return 0;
}
/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 6a9b6628-821c-4a96-84c1-485ebd35f170
*/

