
#ifndef _const_h
#define _const_h

#ifdef __cplusplus
extern "C" {
#endif

void ShowLogHalt(void);
void videoSetModeSub_SetShowLog(bool e);
bool CheckITCM(void);

#ifdef __cplusplus
}	   // extern "C"
#endif

#define ScreenWidth (256)
#define ScreenHeight (192)

//#define attrinline __attribute__ ((always_inline, const)) static inline 
#define attrinline __attribute__ ((always_inline)) static inline 
//#define attrinline 

#define BIT0 (1<<0)
#define BIT1 (1<<1)
#define BIT2 (1<<2)
#define BIT3 (1<<3)
#define BIT4 (1<<4)
#define BIT5 (1<<5)
#define BIT6 (1<<6)
#define BIT7 (1<<7)
#define BIT8 (1<<8)
#define BIT9 (1<<9)
#define BIT10 (1<<10)
#define BIT11 (1<<11)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT14 (1<<14)
#define BIT15 (1<<15)

typedef s32 sfix5; // 8.5bit�Œ菬��
typedef u32 ufix5; // 8.5bit�Œ菬��

#define UFIX5SHIFT (5)
#define SFIX5VALUE (0x20)

typedef s32 sfix6; // 8.6bit�Œ菬��
typedef u32 ufix6; // 8.6bit�Œ菬��

#define UFIX6SHIFT (6)
#define SFIX6VALUE (0x40)

typedef s32 sfix8; // 8.8bit�Œ菬��
typedef u32 ufix8; // 8.8bit�Œ菬��

#define UFIX8SHIFT (8)
#define SFIX8VALUE (0x100)

typedef s32 sfix16; // 8.16bit�Œ菬��
typedef u32 ufix16; // 8.16bit�Œ菬��

#define UFIX16SHIFT (16)
#define SFIX16VALUE (0x10000)

typedef s32 sfix24; // 8.24bit�Œ菬��
typedef u32 ufix24; // 8.24bit�Œ菬��

#define UFIX24SHIFT (24)
#define SFIX24VALUE (0x1000000)

#endif

