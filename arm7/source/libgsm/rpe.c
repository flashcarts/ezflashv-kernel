/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

//#include <stdio.h>
#include <assert.h>

#include "gsm.h"
#include "gsm610_priv.h"

/*  4.2.13 .. 4.2.17  RPE ENCODING SECTION
 */

/* 4.2.13 */

/* 4.2.14 */

/* 4.12.15 */

attrinline void APCM_quantization_xmaxc_to_exp_mant (
	word		xmaxc,		/* IN 	*/
	word		* expon_out,	/* OUT	*/
	word		* mant_out )	/* OUT  */
{
	word	expon, mant;

	/* Compute expononent and mantissa of the decoded version of xmaxc
	 */

	expon = 0;
	if (xmaxc > 15) expon = SASR_W(xmaxc, 3) - 1;
	mant = xmaxc - (expon << 3);

	if (mant == 0) {
		expon  = -4;
		mant = 7;
	}
	else {
		while (mant <= 7) {
			mant = mant << 1 | 1;
			expon--;
		}
		mant -= 8;
	}

	assert( expon  >= -4 && expon <= 6 );
	assert( mant >= 0 && mant <= 7 );

	*expon_out  = expon;
	*mant_out = mant;
}

/* 4.2.16 */

attrinline void APCM_inverse_quantization (
	register word	* xMc,	/* [0..12]			IN 	*/
	word		mant,
	word		expon,
	register word	* xMp)	/* [0..12]			OUT 	*/
/* 
 *  This part is for decoding the RPE sequence of coded xMc[0..12]
 *  samples to obtain the xMp[0..12] array.  Table 4.6 is used to get
 *  the mantissa of xmaxc (FAC[0..7]).
 */
{
	int	i;
	word	temp, temp1, temp2, temp3;

	assert( mant >= 0 && mant <= 7 ); 

	temp1 = gsm_FAC[ mant ];	/* see 4.2-15 for mant */
	temp2 = gsm_sub( 6, expon );	/* see 4.2-15 for exp  */
	temp3 = gsm_asl( 1, gsm_sub( temp2, 1 ));

	for (i = 13; i--;) {

		assert( *xMc <= 7 && *xMc >= 0 ); 	/* 3 bit unsigned */

		/* temp = gsm_sub( *xMc++ << 1, 7 ); */
		temp = (*xMc++ << 1) - 7;	        /* restore sign   */
		assert( temp <= 7 && temp >= -7 ); 	/* 4 bit signed   */

		temp <<= 12;				/* 16 bit signed  */
		temp = GSM_MULT_R( temp1, temp );
		temp = GSM_ADD( temp, temp3 );
		*xMp++ = gsm_asr( temp, temp2 );
	}
}

/* 4.2.17 */

attrinline void RPE_grid_positioning (
	word		Mc,		/* grid position	IN	*/
	register word	* xMp,		/* [0..12]		IN	*/
	register word	* ep		/* [0..39]		OUT	*/
)
/*
 *  This procedure computes the reconstructed long term residual signal
 *  ep[0..39] for the LTP analysis filter.  The inputs are the Mc
 *  which is the grid position selection and the xMp[0..12] decoded
 *  RPE samples which are upsampled by a factor of 3 by inserting zero
 *  values.
 */
{
	int	i = 13;

	assert(0 <= Mc && Mc <= 3);

        switch (Mc) {
                case 3: *ep++ = 0;
                case 2:  do {
                                *ep++ = 0;
                case 1:         *ep++ = 0;
                case 0:         *ep++ = *xMp++;
                         } while (--i);
        }
        while (++Mc < 4) *ep++ = 0;

	/*

	int i, k;
	for (k = 0; k <= 39; k++) ep[k] = 0;
	for (i = 0; i <= 12; i++) {
		ep[ Mc + (3*i) ] = xMp[i];
	}
	*/
}

/* 4.2.18 */

/*  This procedure adds the reconstructed long term residual signal
 *  ep[0..39] to the estimated signal dpp[0..39] from the long term
 *  analysis filter to compute the reconstructed short term residual
 *  signal dp[-40..-1]; also the reconstructed short term residual
 *  array dp[-120..-41] is updated.
 */

void Gsm_RPE_Decoding (
	/*-struct gsm_state	* S,-*/

	word 		xmaxcr,
	word		Mcr,
	word		* xMcr,  /* [0..12], 3 bits 		IN	*/
	word		* erp	 /* [0..39]			OUT 	*/
)
{
	word	expon=0, mant=0;
//	static word	xMp[ 13 ];
	word	xMp[ 13 ];

	APCM_quantization_xmaxc_to_exp_mant( xmaxcr, &expon, &mant );
	APCM_inverse_quantization( xMcr, mant, expon, xMp );
	RPE_grid_positioning( Mcr, xMp, erp );

}
/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 82005b9e-1560-4e94-9ddb-00cb14867295
*/

