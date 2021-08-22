
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NDS.h>

#include "_console.h"
#include "memtoolarm7.h"

#include "plug_gsm.h"

#ifndef USE_GSM

bool StartGSM(int Samples)
{
  return(false);
}

u32 UpdateGSM(s16 *lbuf,bool flash)
{
  return(0);
}

void FreeGSM(void)
{
}

#else

#include "../../ipc3.h"

#include "filesys.h"

#define MAX( x, y ) ( ( x > y ) ? x : y )
#define MIN( x, y ) ( ( x < y ) ? x : y )

// --------------------------------------------------------------------

typedef struct gsm_state {
	u16			dp0[ 280 ] ;

//	u16			z1;			/* preprocessing.c, Offset_com. */
//	longword		L_z2;		/*                  Offset_com. */
//	int				mp;			/*                  Preemphasis	*/

	u16			u[8] ;			/* short_term_aly_filter.c	*/
	u16			LARpp[2][8] ; 	/*                              */
	u16			j;				/*                              */

//	u16	        ltp_cut;        /* long_term.c, LTP crosscorr.  */
	u16			nrp; 			/* 40 */	/* long_term.c, synthesis	*/
	u16			v[9] ;			/* short_term.c, synthesis	*/
	u16			msr;			/* decoder.c,	Postprocessing	*/

//	char			verbose;		/* only used if !NDEBUG		*/
//	char			fast;			/* only used if FAST		*/

//	char			wav_fmt;		/* only used if WAV49 defined	*/
	u8	frame_index;	/*            odd/even chaining	*/
	u8	frame_chain;	/*   half-byte to carry forward	*/

	/* Moved here from code.c where it was defined as static */
//	u16 e[50] ;
} GSM_STATE;

static GSM_STATE *gsmstate=NULL;

#define GSM_BlockSamples (320)
#define GSM_BlockBytes (65)

// --------------------------------------------------------------------

extern "C" int gsm_decode(GSM_STATE *, u8 *, s16 *);

static bool WaveFile_Decode(u8 *pblock,s16 *psamples)
{
  gsm_decode (gsmstate, pblock, psamples);
  
  pblock+=(GSM_BlockBytes + 1) / 2;
  psamples+=GSM_BlockSamples / 2;
  gsm_decode (gsmstate, pblock, psamples);
  
  return(true);
}

// --------------------------------------------------------------------

// --------------------------------------------------------------------

static bool inited=false;

static int GSM_BlockCount;
static u8 *GSM_ReadBuf=NULL;

bool StartGSM(int Samples)
{
  inited=true;
  
  gsmstate=(GSM_STATE*)safemalloc(sizeof(GSM_STATE));
  
  gsmstate->nrp=40;
  
  GSM_BlockCount=Samples/GSM_BlockSamples;
  GSM_ReadBuf=(u8*)safemalloc(GSM_BlockBytes*GSM_BlockCount);
  
  return(true);
}

u32 UpdateGSM(s16 *lbuf,bool flash)
{
  if(inited==false) return(0);
  
  u8 *databuf=GSM_ReadBuf;
  
  u32 BlockCount;
  
  if(flash==true){
    BlockCount=FileSys_fread_flash(databuf,GSM_BlockBytes*GSM_BlockCount);
    }else{
    BlockCount=FileSys_fread(databuf,GSM_BlockBytes*GSM_BlockCount);
  }
  
  BlockCount/=GSM_BlockBytes;
  
  u32 samples=0;
  
  for(u32 cnt=0;cnt<BlockCount;cnt++){
    if(WaveFile_Decode(databuf,lbuf)==false) return(samples);
    
    databuf+=GSM_BlockBytes;
    lbuf+=GSM_BlockSamples;
    samples+=GSM_BlockSamples;
  }
  
  return(samples);
}

void FreeGSM(void)
{
  if(inited==false) return;
  inited=false;
  
  GSM_BlockCount=0;
  
  if(GSM_ReadBuf!=NULL){
    safefree(GSM_ReadBuf); GSM_ReadBuf=NULL;
  }
  
  if(gsmstate!=NULL){
    safefree(gsmstate); gsmstate=NULL;
  }
}

#endif

