
#ifndef plug_text_h
#define plug_text_h

#define TextTitle "TextView ver0.1"

#include "../mwin.h"

extern bool StartText(int _FileHandle,u32 _ClientWidth,u32 _ClientHeight);
extern void FreeText(void);

extern void Text_RefreshScreen(u16 *pBuf,u32 BufWidth,TRect ClientRect);

extern void Text_SetLine(s32 Line,TRect ClientRect);
extern s32 Text_GetLine(void);

extern s32 Text_GetLineCount(void);
extern s32 Text_GetLineHeight(void);
extern s32 Text_GetDispLineCount(void);

enum EBookmarkDialogMode {BDM_Exit,BDM_LoadMenu,BDM_SaveMenu};

extern EBookmarkDialogMode Text_BookmarkDialog(EBookmarkDialogMode bdm);

#endif

