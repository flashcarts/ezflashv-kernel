
typedef   signed int mad_fixed_t;

# define MAD_F_FRACBITS		28

# if MAD_F_FRACBITS == 28
#  define MAD_F(x)		((mad_fixed_t) (x##L))
# else
#  if MAD_F_FRACBITS < 28
#   warning "MAD_F_FRACBITS < 28"
#   define MAD_F(x)		((mad_fixed_t)  \
				 (((x##L) +  \
				   (1L << (28 - MAD_F_FRACBITS - 1))) >>  \
				  (28 - MAD_F_FRACBITS)))
#  elif MAD_F_FRACBITS > 28
#   error "MAD_F_FRACBITS > 28 not currently supported"
#   define MAD_F(x)		((mad_fixed_t)  \
				 ((x##L) << (MAD_F_FRACBITS - 28)))
#  endif
# endif

struct fixedfloat {
//  unsigned long mantissa  : 27;
//  unsigned short exponent :  5;
  int mantissa;
  int exponent;
} rq_table[8207] = {
# include "mp3_rq_table.dat"
};

extern int PlugMP3_GetPtr_rq_table(void);

int PlugMP3_GetPtr_rq_table(void)
{
//  return((int)&rq_table[0]);
  
  static int a[8207];
  for(int idx=0;idx<8207;idx++){
    a[idx]=rq_table[idx].mantissa | (rq_table[idx].exponent << 27);
  }
  return((int)&a[0]);
}

// -----------------

#define OPT_SSO

# if defined(OPT_SSO)
#  if MAD_F_FRACBITS != 28
#   error "MAD_F_FRACBITS must be 28 to use OPT_SSO"
#  endif
#  define ML0(hi, lo, x, y)	((lo)  = (x) * (y))
#  define MLA(hi, lo, x, y)	((lo) += (x) * (y))
#  define MLN(hi, lo)		((lo)  = -(lo))
#  define MLZ(hi, lo)		((void) (hi), (mad_fixed_t) (lo))
#  define SHIFT(x)		((x) >> 2)
#  define PRESHIFT(x)		((MAD_F(x) + (1L << 13)) >> 14)
# else
#  define ML0(hi, lo, x, y)	MAD_F_ML0((hi), (lo), (x), (y))
#  define MLA(hi, lo, x, y)	MAD_F_MLA((hi), (lo), (x), (y))
#  define MLN(hi, lo)		MAD_F_MLN((hi), (lo))
#  define MLZ(hi, lo)		MAD_F_MLZ((hi), (lo))
#  define SHIFT(x)		(x)
#  if defined(MAD_F_SCALEBITS)
#   undef  MAD_F_SCALEBITS
#   define MAD_F_SCALEBITS	(MAD_F_FRACBITS - 12)
#   define PRESHIFT(x)		(MAD_F(x) >> 12)
#  else
#   define PRESHIFT(x)		MAD_F(x)
#  endif
# endif

static
mad_fixed_t const D[17][32] = {
# include "mp3_D.dat"
};

extern int PlugMP3_GetPtr_D(void);

int PlugMP3_GetPtr_D(void)
{
  return((int)&D);
}

void main(void)
{
  {
    extern int PlugMP3_GetPtr_rq_table(void);
    extern int PlugMP3_GetPtr_D(void);
    IPC3->PlugMP3_Ptr_rq_table=PlugMP3_GetPtr_rq_table();
    IPC3->PlugMP3_Ptr_D=PlugMP3_GetPtr_D();
//    _consolePrintf("PlugMP3_Ptr rq_table=0x%08x D=0x%08x\n",IPC3->PlugMP3_Ptr_rq_table,IPC3->PlugMP3_Ptr_D);
  }
}
