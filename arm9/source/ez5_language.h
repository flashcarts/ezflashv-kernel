#ifndef ez5_language_h
#define ez5_language_h


#define ROM_OFF 0x3F0000

extern u8 *gl_pSaveTypeBuf;
extern int 	gl_SizeTypeFileSize;
extern u32 gl_SaveTypeOff;

extern u8 gl_language;
extern u8 gl_speedAuto;
extern u16 gl_speed;
extern u8 gl_SaveType;


/*
 -1:no card or no EEPROM
  0:unknown                 PassMe?
  1:TYPE 1  4Kbit(512Byte)  EEPROM
  2:TYPE 2 64Kbit(8KByte)or 512kbit(64Kbyte)   EEPROM
  3:TYPE 3  2Mbit(256KByte) FLASH MEMORY
*/
extern char pSaveFileName[256];

extern char* gl_speedset;
extern char* gl_speedAutoDetect;
extern char* gl_strSaveType;
extern char* gl_strAboutMsg;
extern char* gl_strAMsg;
extern char* gl_strBMsg;
extern char* gl_strLRMsg;
extern char* gl_strLXMsg;
extern char* gl_strRXMsg;
extern char* gl_strQuit;
extern char* gl_strSaveQuit;
extern char* gl_strBaseMoonShell;
extern char* gl_strThanksMoonShell;

extern char* gl_strLoadSave;
extern char* gl_strBackSave;
extern char* gl_strLoadGame;

void LoadChinese();
void LoadEnglish();
void LoadLanguage();

s32 GetSaveSize();

#endif