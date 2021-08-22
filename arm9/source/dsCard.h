/**************************************************************************************************************
 * ���ļ�Ϊ dsCard.h �ļ��ĵڶ��� 
 * ���ڣ�2006��11��27��11��33��  ��һ�� version 1.0
 * ���ߣ�aladdin
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
//DS �� ��������
		//Arm9 ���������������ARM7����slot1
		void		Enable_Arm7DS(void);

		//Arm9 ���������������ARM9����slot1
		void		Enable_Arm9DS(void);

		//����DS��8����������
		inline static void		dsCardi_SetRomOP(uint8 * command);
		
		//B7������Ļ���ָ���4�ֽڶ�ģʽ
		inline static  uint32		dsCardi_Read4ByteMode(uint8 * command);

		//B7������Ļ���ָ���512�ֽڶ�ģʽ��wait �ȴ�ʱ�� 0x10f8 ���ʱ��650ns ʱ����Ե���
		inline static  uint32		dsCardi_Read512ByteMode(uint8 * command,uint8* buf,uint32 wait);

		//��ID B8 XX XX XX XX XX XX XX
		uint32		dsCardi_ReadID(void);

		//�ڻ���ж���,�����Ķ�����,�ȴ�0
		extern uint32 dsCardi_ReadCard(uint32 address, uint8* buf);


//---------------------------------------------------
//�Զ��忨���Ĳ�������
		//SD_multiblock ��������Ҫ��Modify rom ģʽ�¹��� type 01 ����Ķ�,����״̬ 02�м��״̬
		uint32		dsCardi_ReadCard_status(uint32 address,uint8 type);

		//SD_multiblock ��������Ҫ��Modify rom ģʽ�¹���
		uint32		dsCardi_ReadCard_M(uint32 address, uint8* buf);
		
		//�趨�浵���� 0-3 �ֱ� 4k\64k\2M\4M
		uint32		dsCardi_SetSaverType(uint8 data);

		//��������:	дEZ4 DS��SRAM,���ַaddress:д������data Sram��С��8M Bits(1M Bytes)
		uint32		dsCardi_WriteSram(uint32 address,uint16 data);

		//��EZ5 DS��SRAM,�ӵ�ַaddress��������
		uint32		dsCardi_ReadSram(uint32 address);

		//��EZ5 DS��SRAM,�ӵ�ַaddress����512����
		uint32		dsCardi_ReadSramblock(uint32 address,uint8 *pbuf);
	
		//OSģʽ�µĶ�����FAT������������ļ��Զ���,��Ҫ������PSRAM �Լ� SectorPerClus
		uint32		dsCardi_TstSDCard(uint8 cmd,uint32 address,uint8* pbuf,uint32 wait) ;

		//����FAT16�е�ÿ��������,���߼�оƬ��Ҫ
		uint32		dsCardi_SectorPerClus(uint8 SecNO);

		//����Slot1����������MODIFY mode,���Եȴ�����ʱ����ж�
		uint32		dsCardi_SetModifyMode(void);

		//��Slot1��������MODIFY modeģʽ����
		uint32		dsCardi_ModifyReturn(void);

		//������CleanROM ģʽ�����ٽ�����������
		uint32		dsCardi_SectorCleanromMode(void);

		//����LOADER�����ROMPAGE ,ʵ������������������LOADER
		uint32		dsCardi_SetRompage(uint32 address);

		//��ĳ��ַaddressдĳ����cmd
		uint32		dsCardi_WriteFlashCommand(uint32 address,uint8 cmd);

		//��ĳ��ַaddress��ĳ����cmd
		uint32		dsCardi_ReadFlashCommand(uint32 address);
		
		//д�浵оƬ
		void dsCardWriteSaveFlash(uint32 address, uint8 *data, uint32 length) ;
			
		//���浵оƬ
		void dsCardReadSaveFlash(uint32 address, uint8 *data, uint32 length);
//---------------------------------------------------
//SLOT1�Ͽ��� Flash �Ļ�������
		//��λFlash����
		uint32		ResetFlash(void);

		//����Flash �� ����0��ȷ����0ʧ��
		uint32		EraseFlash(uint32 address);

		//����Flash �� ����0��ȷ����0ʧ��
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
