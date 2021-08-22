
#ifndef thumb_h
#define thumb_h

extern void thumbChangePath(void);
extern void thumbReturnImage(void);
extern void thumbChangeCursorIndex(s32 idx);
extern u16* thumbGetImage64(s32 idx);
extern bool thumbExists(void);
extern bool thumbGetDrawed(void);

#endif
