
#ifndef unicode_h
#define unicode_h

typedef u16 UnicodeChar;

#define CP932 (932)

extern int DefaultCodePage;
extern u8 *isAnkTable;
extern u32 Local2UnicodeTableSize;
extern u16 *Local2UnicodeTable;
extern bool isUTF8;

extern void Unicode_StrCopy(const UnicodeChar *src,UnicodeChar *dst);
extern bool Unicode_isEqual(const UnicodeChar *s1,const UnicodeChar *s2);
extern void Unicode_StrAppend(UnicodeChar *s,const UnicodeChar *add);

extern void Unicode_Init(u8 *pank,u8 *pl2u);
extern void Unicode_Free(void);

extern void StrConvert_Ank2Unicode(const char *srcstr,UnicodeChar *dststr);
extern void StrConvert_Local2Unicode(const char *srcstr,UnicodeChar *dststr);
extern void StrConvert_UTF82Unicode(const char *srcstr,UnicodeChar *dststr);

#endif
