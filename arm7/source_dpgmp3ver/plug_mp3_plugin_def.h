
#ifndef plugin_def_h
#define plugin_def_h

#include <nds.h>

#include <stdio.h>

#define abort(x) 

extern void (*C_consolePrint)(const char *str);
extern void (*C_consolePrintf)(const char* format, ...);
extern void (*C_consolePrintSet)(int x, int y);

extern void (*C_ShowLogHalt)(void);

extern void *(*C_calloc)(size_t nmemb, size_t size);
extern void *(*C_malloc)(size_t size);
extern void (*C_free)(void *ptr);

extern void *(*C_memcpy)(void *buf1, const void *buf2, size_t n);
extern void *(*C_memmove)(void *buf1, const void *buf2, size_t n);

extern int (*C_sprintf)(char *str, const char *format, ...);

#define BLOCKFUNC BLOCK

#undef clearerr
#define clearerr (blockfunc)
#define fdopen (blockfunc)
#undef feof
#define feof (blockfunc)
#undef ferror
#define fopen (blockfunc)
#define fclose (blockfunc)
#define ferror (blockfunc)
#define fflush (blockfunc)
#define fgetc (blockfunc)
#define fgetpos (blockfunc)
#define fgets (blockfunc)
#define fileno (blockfunc)
#define fprintf (blockfunc)
#define fpurge (blockfunc)
#undef fputc
#define fputc (blockfunc)
#define fputs (blockfunc)
#define freopen (blockfunc)
#define fscanf (blockfunc)
#define fsetpos (blockfunc)
#define fwrite (blockfunc)
#undef getc
#define getc (blockfunc)
#undef getchar
#define getchar (blockfunc)
#define gets (blockfunc)
#define getw (blockfunc)
#define mktemp (blockfunc)
#define perror (blockfunc)
#define printf (blockfunc)
#undef putc
#define putc (blockfunc)
#undef putchar
#define putchar (blockfunc)
#define puts (blockfunc)
#define putw (blockfunc)
#define remove (blockfunc)
#define rewind (blockfunc)
#define scanf (blockfunc)
#define setbuf (blockfunc)
#define setbuffer (blockfunc)
#define setlinebuf (blockfunc)
#define setvbuf (blockfunc)
#define sscanf (blockfunc)
#define strerror (blockfunc)
#define sys_errlist (blockfunc)
#define sys_nerr (blockfunc)
#define tempnam (blockfunc)
#define tmpfile (blockfunc)
#define tmpnam (blockfunc)
#define ungetc (blockfunc)
#define vfprintf (blockfunc)
#define vfscanf (blockfunc)
#define vprintf (blockfunc)
#define vscanf (blockfunc)
#define vsprintf (blockfunc)
#define vsscanf (blockfunc)

#define _consolePrint (C_consolePrint)
#define _consolePrintf (C_consolePrintf)
#define _consolePrintSet (C_consolePrintSet)
#define ShowLogHalt (C_ShowLogHalt)
#define MWin_ProgressShow (BLOCKFUNC MWin_ProgressShow)
#define MWin_ProgressSetPos (BLOCKFUNC MWin_ProgressSetPos)
#define MWin_ProgressHide (BLOCKFUNC MWin_ProgressHide)
#define GetINIData (BLOCKFUNC GetINIData)
#define GetINISize (BLOCKFUNC GetINISize)
#define GetBINData (BLOCKFUNC GetBINData)
#define GetBINSize (BLOCKFUNC GetBINSize)

#define DC_FlushRangeOverrun (BLOCKFUNC DC_FlushRangeOverrun)
#define MemCopy8CPU (BLOCKFUNC MemCopy8CPU)
#define MemCopy16CPU (BLOCKFUNC MemCopy16CPU)
#define MemCopy32CPU (BLOCKFUNC MemCopy32CPU)
#define MemSet16CPU (BLOCKFUNC MemSet16CPU)
#define MemSet32CPU (BLOCKFUNC MemSet32CPU)
#define MemCopy16DMA3 (BLOCKFUNC MemCopy16DMA3)
#define MemCopy32DMA3 (BLOCKFUNC MemCopy32DMA3)
#define MemSet16DMA3 (BLOCKFUNC MemSet16DMA3)
#define MemSet32DMA3 (BLOCKFUNC MemSet32DMA3)
#define MemSet8DMA3 (BLOCKFUNC MemSet8DMA3)
#define MemCopy16DMA2 (BLOCKFUNC MemCopy16DMA2)
#define MemSet16DMA2 (BLOCKFUNC MemSet16DMA2)
#define MemCopy32swi256bit (BLOCKFUNC MemCopy32swi256bit)
#define safemalloc (BLOCKFUNC safemalloc)
#define safefree (BLOCKFUNC safefree)

#define calloc (C_calloc)
#define malloc (C_malloc)
#define free (C_free)
#define realloc (BLOCKFUNC realloc)

#define rand (BLOCKFUNC rand)

#define fread (BLOCKFUNC fread)
#define fseek (BLOCKFUNC fseek)
#define ftell (BLOCKFUNC ftell)

#define sprintf (C_sprintf)
#define snprintf (BLOCKFUNC snprintf)

#define memchr (BLOCKFUNC memchr)
#define memcmp (BLOCKFUNC memcmp)
#define memcpy (C_memcpy)
#define memmove (C_memmove)
#define memset (BLOCKFUNC memset)

#define abs (BLOCKFUNC abs)
#define labs (BLOCKFUNC labs)
#define llabs (BLOCKFUNC llabs)
#define fabs (BLOCKFUNC fabs)
#define fabsf (BLOCKFUNC fabsf)

#define atof (BLOCKFUNC atof)
#define atoi (BLOCKFUNC atoi)
#define atol (BLOCKFUNC atol)
#define atoll (BLOCKFUNC atoll)

#define strcat (BLOCKFUNC strcat)
#define strchr (BLOCKFUNC strchr)
#define strcmp (BLOCKFUNC strcmp)
#define strcoll (BLOCKFUNC strcoll)
#define strcpy (BLOCKFUNC strcpy)
#define strcspn (BLOCKFUNC strcspn)
#define strdup (BLOCKFUNC strdup)
#define strlen (BLOCKFUNC strlen)
#define strncat (BLOCKFUNC strncat)
#define strncmp (BLOCKFUNC strncmp)
#define strncpy (BLOCKFUNC strncpy)
#define strpbrk (BLOCKFUNC strpbrk)
#define strrchr (BLOCKFUNC strrchr)
#define strsep (BLOCKFUNC strsep)
#define strspn (BLOCKFUNC strspn)
#define strstr (BLOCKFUNC strstr)
#define strtok (BLOCKFUNC strtok)
#define strxfrm (BLOCKFUNC strxfrm)

#define GetBINFileHandle (BLOCKFUNC GetBINFileHandle)
#define msp_fopen (BLOCKFUNC msp_fopen)
#define msp_fclose (BLOCKFUNC msp_fclose)

#define extmem_SetCount (BLOCKFUNC extmem_SetCount)
#define extmem_Exists (BLOCKFUNC extmem_Exists)
#define extmem_Alloc (BLOCKFUNC extmem_Alloc)
#define extmem_Write (BLOCKFUNC extmem_Write)
#define extmem_Read (BLOCKFUNC extmem_Read)

#define formdt_FormatDate (BLOCKFUNC formdt_FormatDate)

#endif

