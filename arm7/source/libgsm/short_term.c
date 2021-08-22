/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

//#include <stdio.h>
#include <assert.h>

#include "gsm.h"
#include "gsm610_priv.h"

/*
 *  SHORT TERM ANALYSIS FILTERING SECTION
 */

/* 4.2.8 */

attrinline void Decoding_of_the_coded_Log_Area_Ratios (
	word 	* LARc,		/* coded log area ratio	[0..7] 	IN	*/
	word	* LARpp)	/* out: decoded ..			*/
{
	register word	temp1 /* , temp2 */;

	/*  This procedure requires for efficient implementation
	 *  two tables.
 	 *
	 *  INVA[1..8] = integer( (32768 * 8) / real_A[1..8])
	 *  MIC[1..8]  = minimum value of the LARc[1..8]
	 */

	/*  Compute the LARpp[1..8]
	 */

	/* 	for (i = 1; i <= 8; i++, B++, MIC++, INVA++, LARc++, LARpp++) {
	 *
	 *		temp1  = GSM_ADD( *LARc, *MIC ) << 10;
	 *		temp2  = *B << 1;
	 *		temp1  = GSM_SUB( temp1, temp2 );
	 *
	 *		assert(*INVA != MIN_WORD);
	 *
	 *		temp1  = GSM_MULT_R( *INVA, temp1 );
	 *		*LARpp = GSM_ADD( temp1, temp1 );
	 *	}
	 */

#undef	STEP
#define	STEP( B, MIC, INVA )	\
		temp1    = GSM_ADD( *LARc++, MIC ) << 10;	\
		temp1    = GSM_SUB( temp1, B << 1 );		\
		temp1    = GSM_MULT_R( INVA, temp1 );		\
		*LARpp++ = GSM_ADD( temp1, temp1 );

	STEP(      0,  -32,  13107 );
	STEP(      0,  -32,  13107 );
	STEP(   2048,  -16,  13107 );
	STEP(  -2560,  -16,  13107 );

	STEP(     94,   -8,  19223 );
	STEP(  -1792,   -8,  17476 );
	STEP(   -341,   -4,  31454 );
	STEP(  -1144,   -4,  29708 );

	/* NOTE: the addition of *MIC is used to restore
	 * 	 the sign of *LARc.
	 */
}

/* 4.2.9 */
/* Computation of the quantized reflection coefficients 
 */

/* 4.2.9.1  Interpolation of the LARpp[1..8] to get the LARp[1..8]
 */

/*
 *  Within each frame of 160 analyzed speech samples the short term
 *  analysis and synthesis filters operate with four different sets of
 *  coefficients, derived from the previous set of decoded LARs(LARpp(j-1))
 *  and the actual set of decoded LARs (LARpp(j))
 *
 * (Initial value: LARpp(j-1)[1..8] = 0.)
 */

attrinline void Coefficients_0_12 (
	register word * LARpp_j_1,
	register word * LARpp_j,
	register word * LARp)
{
	register int 	i;

	for (i = 1; i <= 8; i++, LARp++, LARpp_j_1++, LARpp_j++) {
		*LARp = GSM_ADD( SASR_W( *LARpp_j_1, 2 ), SASR_W( *LARpp_j, 2 ));
		*LARp = GSM_ADD( *LARp,  SASR_W( *LARpp_j_1, 1));
	}
}

attrinline void Coefficients_13_26 (
	register word * LARpp_j_1,
	register word * LARpp_j,
	register word * LARp)
{
	register int i;
	
	for (i = 1; i <= 8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
		*LARp = GSM_ADD( SASR_W( *LARpp_j_1, 1), SASR_W( *LARpp_j, 1 ));
	}
}

attrinline void Coefficients_27_39 (
	register word * LARpp_j_1,
	register word * LARpp_j,
	register word * LARp)
{
	register int i;

	for (i = 1; i <= 8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
		*LARp = GSM_ADD( SASR_W( *LARpp_j_1, 2 ), SASR_W( *LARpp_j, 2 ));
		*LARp = GSM_ADD( *LARp, SASR_W( *LARpp_j, 1 ));
	}
}


attrinline void Coefficients_40_159 (
	register word * LARpp_j,
	register word * LARp)
{
	register int i;

	for (i = 1; i <= 8; i++, LARp++, LARpp_j++)
		*LARp = *LARpp_j;
}

/* 4.2.9.2 */

attriwram static void LARp_to_rp (
	register word * LARp)	/* [0..7] IN/OUT  */
/*
 *  The input of this procedure is the interpolated LARp[0..7] array.
 *  The reflection coefficients, rp[i], are used in the analysis
 *  filter and in the synthesis filter.
 */
{
	register int 		i;
	register word		temp;

	for (i = 1; i <= 8; i++, LARp++) {

		/* temp = GSM_ABS( *LARp );
	         *
		 * if (temp < 11059) temp <<= 1;
		 * else if (temp < 20070) temp += 11059;
		 * else temp = GSM_ADD( temp >> 2, 26112 );
		 *
		 * *LARp = *LARp < 0 ? -temp : temp;
		 */

		if (*LARp < 0) {
			temp = *LARp == MIN_WORD ? MAX_WORD : -(*LARp);
			*LARp = - ((temp < 11059) ? temp << 1
				: ((temp < 20070) ? temp + 11059
				:  GSM_ADD( (word) (temp >> 2), (word) 26112 )));
		} else {
			temp  = *LARp;
			*LARp =    (temp < 11059) ? temp << 1
				: ((temp < 20070) ? temp + 11059
				:  GSM_ADD( (word) (temp >> 2), (word) 26112 ));
		}
	}
}


/* 4.2.10 */

attriwram static void Short_term_synthesis_filtering (
	struct gsm_state * S,
	word	* rrp,	/* [0..7]	IN	*/
	int	k,	/* k_end - k_start	*/
	word	* wt,	/* [0..k-1]	IN	*/
	word	* sr	/* [0..k-1]	OUT	*/
)
{
	word		* v = S->v;
	int		i;
	word		sri, tmp1, tmp2;

	while (k--) {
		sri = *wt++;
		for (i = 8; i--;) {
			/* sri = GSM_SUB( sri, gsm_mult_r( rrp[i], v[i] ) );
			 */
			tmp1 = rrp[i];
			tmp2 = v[i];
			tmp2 =  ( tmp1 == MIN_WORD && tmp2 == MIN_WORD
				? MAX_WORD
				: 0x0FFFF & (( (longword)tmp1 * (longword)tmp2
					     + 16384) >> 15)) ;

			sri  = GSM_SUB( sri, tmp2 );

			/* v[i+1] = GSM_ADD( v[i], gsm_mult_r( rrp[i], sri ) );
			 */
			tmp1  = ( tmp1 == MIN_WORD && sri == MIN_WORD
				? MAX_WORD
				: 0x0FFFF & (( (longword)tmp1 * (longword)sri
					     + 16384) >> 15)) ;

			v[i+1] = GSM_ADD( v[i], tmp1);
		}
		*sr++ = v[0] = sri;
	}
}

void Gsm_Short_Term_Synthesis_Filter (
	struct gsm_state * S,

	word	* LARcr,	/* received log area ratios [0..7] IN  */
	word	* wt,		/* received d [0..159]		   IN  */

	word	* s		/* signal   s [0..159]		  OUT  */
)
{
	word		* LARpp_j	= S->LARpp[ S->j     ];
	word		* LARpp_j_1	= S->LARpp[ S->j ^=1 ];

//	static word		LARp[8];
	word		LARp[8];

#undef	FILTER
#if 	defined(FAST) && defined(USE_FLOAT_MUL)

# 	define	FILTER 	(* (S->fast			\
			   ? Fast_Short_term_synthesis_filtering	\
		    	   : Short_term_synthesis_filtering	))
#else
#	define	FILTER	Short_term_synthesis_filtering
#endif

	Decoding_of_the_coded_Log_Area_Ratios( LARcr, LARpp_j ); // 1ms

  // 47ms
	Coefficients_0_12( LARpp_j_1, LARpp_j, LARp );
	LARp_to_rp( LARp );
	FILTER( S, LARp, 13, wt, s );

  // 50ms
	Coefficients_13_26( LARpp_j_1, LARpp_j, LARp);
	LARp_to_rp( LARp );
	FILTER( S, LARp, 14, wt + 13, s + 13 );

  // 47ms
	Coefficients_27_39( LARpp_j_1, LARpp_j, LARp);
	LARp_to_rp( LARp );
	FILTER( S, LARp, 13, wt + 27, s + 27 );

  // 384ms
	Coefficients_40_159( LARpp_j, LARp ); // 1ms
	LARp_to_rp( LARp ); // 2ms
	FILTER(S, LARp, 120, wt + 40, s + 40); // 380ms
}
/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 019ac7ba-c6dd-4540-abf0-8644b6c4a633
*/

