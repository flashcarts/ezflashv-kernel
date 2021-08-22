/**************************************************************************************************************
 * 此文件为 dsCard.h 文件的第二版 
 * 日期：2006年11月27日11点33分  第一版 version 1.0
 * 作者：aladdin
 * CopyRight : EZFlash Group
 * 
 **************************************************************************************************************/

#ifndef NDS_DSCARD_V2_INCLUDE
#define NDS_DSCARD_V2_INCLUDE

#include "nds.h"

	#ifdef __cplusplus
	extern "C" {
	#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

#ifndef DWORD
typedef unsigned long DWORD;
#endif

#ifndef BOOL
typedef bool BOOL ;
#endif
// export interface




//---------------------------------------------------
//DS 卡 基本操作
		//Arm9 方面基本操作容许ARM7访问slot1
		void		Enable_Arm7DS(void);

		//Arm9 方面基本操作容许ARM9访问slot1
		void		Enable_Arm9DS(void);

		//设置DS的8个参数函数
		inline static void		dsCardi_SetRomOP(uint8 * command);
		
		//B7等命令的基本指令函数4字节读模式
		inline static  uint32		dsCardi_Read4ByteMode(uint8 * command);

		//B7等命令的基本指令函数512字节读模式，wait 等待时间 0x10f8 大概时间650ns 时间可以调节
		inline static  uint32		dsCardi_Read512ByteMode(uint8 * command,uint8* buf,uint32 wait);

		//读ID B8 XX XX XX XX XX XX XX
		uint32		dsCardi_ReadID(void);

		//在汇编中定义,基本的读操作,等待0
		extern uint32 dsCardi_ReadCard(uint32 address, uint8* buf);


//---------------------------------------------------
//自定义卡带的操作函数
		//SD_multiblock 读函数需要在Modify rom 模式下工作 type 01 最初的读,返回状态 02中间读状态
		uint32		dsCardi_ReadCard_status(uint32 address,uint8 type);

		//SD_multiblock 读函数需要在Modify rom 模式下工作
		uint32		dsCardi_ReadCard_M(uint32 address, uint8* buf);
		
		//设定存档类型 0-3 分别 4k\64k\2M\4M
		uint32		dsCardi_SetSaverType(uint8 data);

		//功能描述:	写EZ4 DS卡SRAM,向地址address:写入数据data Sram大小是8M Bits(1M Bytes)
		uint32		dsCardi_WriteSram(uint32 address,uint16 data);

		//读EZ5 DS卡SRAM,从地址address读入数据
		uint32		dsCardi_ReadSram(uint32 address);

		//读EZ5 DS卡SRAM,从地址address读入512数据
		uint32		dsCardi_ReadSramblock(uint32 address,uint8 *pbuf);
	
		//OS模式下的读即按FAT表的索引来读文件自动读,需要先设置PSRAM 以及 SectorPerClus
		uint32		dsCardi_TstSDCard(uint8 cmd,uint32 address,uint8* pbuf,uint32 wait) ;

		//设置FAT16中的每簇扇区数,被逻辑芯片需要
		uint32		dsCardi_SectorPerClus(uint8 SecNO);

		//设置Slot1卡带工作于MODIFY mode,可以等待少量时间进行读
		uint32		dsCardi_SetModifyMode(void);

		//从Slot1卡带工作MODIFY mode模式返回
		uint32		dsCardi_ModifyReturn(void);

		//工作于CleanROM 模式，不再接受其他命令
		uint32		dsCardi_SectorCleanromMode(void);

		//设置LOADER区域的ROMPAGE ,实际上用于启动真正的LOADER
		uint32		dsCardi_SetRompage(uint32 address);

		//朝某地址address写某命令cmd
		uint32		dsCardi_WriteFlashCommand(uint32 address,uint8 cmd);

		//朝某地址address读某命令cmd
		uint32		dsCardi_ReadFlashCommand(uint32 address);
		
		//写存档芯片
		void dsCardWriteSaveFlash(uint32 address, uint8 *data, uint32 length) ;
			
		//读存档芯片
		void dsCardReadSaveFlash(uint32 address, uint8 *data, uint32 length);
//---------------------------------------------------
//SLOT1上卡带 Flash 的基本操作
		//复位Flash操作
		uint32		ResetFlash(void);

		//擦除Flash ， 返回0正确，非0失败
		uint32		EraseFlash(uint32 address);

		//擦除Flash ， 返回0正确，非0失败
		uint32		WriteFlash(uint32 address,uint8 *pbuf,uint32 len);


		void SD_ReadLoop_wait(int lp,int wait0);
		
		//blowfish
		void WriteFlashCommand(uint32 address,uint8 cmd);
		uint32 ReadFlashCommand(uint32 address);
		void BL_EraseFlash(uint32 address);
		bool BL_WriteFlash(uint32 address,uint8 *pbuf,uint32 len);
		void BL_ReadFlashBuffer(uint32 address,uint8 *ppbuf,uint32 size);
	#ifdef __cplusplus
	}
	#endif
#endif
