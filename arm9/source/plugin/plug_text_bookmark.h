
#ifndef bookmark_h
#define bookmark_h

#include <NDS.h>

#include "../main_extforresume.h"
#include "../unicode.h"

typedef struct {
  u32 year;
  u32 month;
  u32 day;
  u32 hours;
  u32 minutes;
  u32 seconds;
} TBookmarkDateTime;

typedef struct {
  u32 LineNum;
  u32 DateTime;
} TBookmarkItem;

#define BookmarkSlotCount (4)

typedef struct {
  char PathName[forResume_PlayPathNameLen];
  UnicodeChar FileUnicodeName[forResume_PlayFileUnicodeNameLen];
  TBookmarkItem Resume;
  TBookmarkItem Slot[BookmarkSlotCount];
  u32 SlotIndex;
} TBookmark;

extern void Bookmark_Init(void);
extern bool Bookmark_GetEnabled(void);
extern void Bookmark_Start(void);
extern void Bookmark_Save(void);
extern void Bookmark_End(void);
extern TBookmark *Bookmark_GetCurrent(void);
extern void Bookmark_CurrentResumeBackup(void);
extern void Bookmark_CurrentResumeRestore(void);
extern u32 Bookmark_GetDateTime32FromIPC(void);
extern TBookmarkDateTime Bookmark_GetDateTimeStructFrom32(u32 dt32);

#endif

