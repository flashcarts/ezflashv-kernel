
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "../_console.h"
#include "../memtool.h"

#include "plug_mp3_plugin.h"

#include "libmp3/mad.h"

// --------------------------------------------------------------------

extern void ShowLogHalt(void);

//#extern "C" {
extern void (*C_consolePrint)(const char *str);
extern void (*C_consolePrintf)(const char* format, ...);
extern void (*C_consolePrintSet)(int x, int y);

extern void (*C_ShowLogHalt)(void);

extern void *(*C_calloc)(size_t nmemb, size_t size);
extern void *(*C_malloc)(size_t size);
extern void (*C_free)(void *ptr);

extern void *(*C_memcpy)(void *buf1, const void *buf2, size_t n);
extern void *(*C_memmove)(void *buf1, const void *buf2, size_t n);
//}

void (*C_consolePrint)(const char *str);
void (*C_consolePrintf)(const char* format, ...);
void (*C_consolePrintSet)(int x, int y);

void (*C_ShowLogHalt)(void);

void *(*C_calloc)(size_t nmemb, size_t size);
void *(*C_malloc)(size_t size);
void (*C_free)(void *ptr);

void *(*C_memcpy)(void *buf1, const void *buf2, size_t n);
void *(*C_memmove)(void *buf1, const void *buf2, size_t n);

void ShowLogHalt(void)
{
}

void _consolePrint(const char *str)
{
}

void _consolePrintSet(int x, int y)
{
}

void *_calloc(size_t nmemb, size_t size)
{
  return(safemalloc(nmemb*size));
}

void *_malloc(size_t size)
{
  return(safemalloc(size));
}

void _free(void *ptr)
{
}

void funcinit(void)
{
  C_consolePrint=_consolePrint;
  C_consolePrintf=_consolePrintf;
  C_consolePrintSet=_consolePrintSet;
  
  C_ShowLogHalt=ShowLogHalt;
  C_calloc=_calloc;
  C_malloc=_malloc;
  C_free=_free;
  
  C_memcpy=memcpy;
  C_memmove=memmove;
}

