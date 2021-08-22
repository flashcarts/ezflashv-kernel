/**************************************************************************************************************
 * 此文件为 dsCard.cpp 文件的第二版 
 * 日期：2006年11月27日11点33分  第一版 version 1.0
 * 作者：aladdin
 * CopyRight : EZFlash Group
 * 
 **************************************************************************************************************/
#include "dscard.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//---------------------------------------------------
//DS 卡 基本操作
void Enable_Arm9DS()
{
	WAIT_CR &= ~0x0800;
}

void Enable_Arm7DS()
{
	WAIT_CR |= 0x0800;
}

inline static void dsCardi_SetRomOP(uint8 * command) {
//---------------------------------------------------------------------------------
	//检测卡带是否处与忙状态
	uint32 status ,index;
	do
	{
		status = CARD_CR2 ;
	}while(status&0x80000000);
	//卡带不忙，开始写，如下s是准备写的过程
	
	// 0-4  Not used            (always zero)
	//5    SPI Enable          (Both Bit13 and Bit15 must be set for SPI)
	//6    Transfer Ready IRQ  (0=Disable, 1=Enable) (for ROM, not for AUXSPI)
	//7    NDS Slot Enable     (0=Disable, 1=Enable) (for both ROM and AUXSPI)
	CARD_CR1H = CARD_CR1_IRQ|CARD_CR1_ENABLE ;

	for (index = 0; index < 8; index++) {
		CARD_COMMAND[7-index] = command[index];
	}
}

inline static  uint32		dsCardi_Read4ByteMode(uint8 * command)
{
	uint32 status=0,flags = 0x00586000;
	WAIT_CR &= ~0x0800;
	dsCardi_SetRomOP(command);

//设置控制寄存器0x40001A4
	flags &= (~0x07000000);
	flags |= 0xa7000000 ;
	flags &= 0xFFFFE000 ;
	CARD_CR2 = flags ;

	do{
		status = CARD_CR2; //0x40001a4
	}while(!(status & 0x800000));

	uint32 DD = CARD_DATA_RD ;
	WAIT_CR |= 0x0800;
	return DD ;

}

inline static  uint32 dsCardi_Read512ByteMode(uint8 * command,uint8* buf,uint32 wait)
{
	uint32 status=0,flags = 0x00586000;
	WAIT_CR &= ~0x0800;
	dsCardi_SetRomOP(command);
	
//设置控制寄存器0x40001A4
	flags += (wait&0x1FFF) ;
	flags &= (~0x07000000);
	flags |= 0xa1000000 ;
	CARD_CR2 = flags ;

	register uint32 i  = 0 ;
	do{
		status = CARD_CR2; //0x40001a4
		if((status & 0x800000)&&(i<0x200))
		{
			((uint32*)buf)[i] = CARD_DATA_RD ;
			i ++ ;
		}
	}while(status & 0x80000000);
	
	WAIT_CR |= 0x0800;
	return 0 ;
}

uint32 dsCardi_ReadID(void)
{
	uint8 cmd[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb8};
	return dsCardi_Read4ByteMode(cmd);
}



uint32 dsCardi_ReadCard_status(uint32 address,uint8 type)
{
	uint8 command[8];
	command[7] = 0xB7;
	command[6] = (address >> 24) & 0xff;
	command[5] = (address >> 16) & 0xff;
	command[4] = (address >> 8) & 0xff;
	command[3] = address & 0xff;
	command[2] = type;
	command[1] = 0;
	command[0] = 0;
	return dsCardi_Read4ByteMode(command);
}	

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//---------------------------------------------------
//自定义卡带的操作函数

uint32 dsCardi_ReadCard_M(uint32 address, uint8* buf)
{
	uint32 status;
	status = dsCardi_ReadCard_status(address,1);
	while(status&0xff000000)
	{
		status = dsCardi_ReadCard_status(address,2);						
	}
	return dsCardi_ReadCard(address,buf);	
}


uint32 dsCardi_SetSaverType(uint8 data)
{
	//0 -9bit 1-16bit 2-24bit(2M) , 3-24bit(4M)
	uint8 command[8];
	command[0]= 0xaa;
	command[1]= 0x55;
	command[2]= 0xaa;
	command[3]= 0x55;
	command[4]= data;
	command[5]= 0x02;
	command[6]= 0xF4; 
	command[7]= 0xB8;
	return dsCardi_Read4ByteMode(command);
}

uint32 dsCardi_WriteSram(uint32 address,uint16 data)
{
	/*
	uint8 command[8];
	command[0]= data>>8;
	command[1]= data;
	command[2]= 0x00;
	command[3]= address;
	command[4]= address>>8;
	command[5]= address>>16;
	command[6]= 0xF2; 
	command[7]= 0xB8;
	return dsCardi_Read4ByteMode(command);
	*/
	uint32 status=0, flags= 0x00586000;
	uint32 temp;
	WAIT_CR &= ~0x0800;
	//(*(vuint16*)0x04000200) |=0x2000; 
	uint8 command[8];
	command[0]= data>>8;
	command[1]= data;
	command[2]= 0x00;
	command[3]= address;
	command[4]= address>>8;
	command[5]= address>>16;
	command[6]= 0xF2; 
	command[7]= 0xB8;
  	cardWriteCommand(command);
  	//CARD_CR2 = 0 | CARD_ACTIVATE | CARD_nRESET | 0x07000000;
  	flags = 0x00586000;
  	flags &= (~0x07000000);
	flags |= 0xa7000000 ;
	flags &= 0xFFFFE000 ;
	CARD_CR2 = flags ;
	do
	{
		status = CARD_CR2; //0x40001a4
	}while(!(status & 0x800000));
  	temp = CARD_DATA_RD;
  	
}

uint32 dsCardi_ReadSram(uint32 address) 
{
	uint8 command[8];
	command[0]= 0x00;
	command[1]= 0x00;
	command[2]= 0x00;
	command[3]= address;
	command[4]= address>>8;
	command[5]= address>>16;
	command[6]= 0x01;
	command[7]= 0xB7;
	return dsCardi_Read4ByteMode(command);
}

uint32		dsCardi_ReadSramblock(uint32 address,uint8 *pbuf)
{
	uint8 command[8];
	command[0]= 0x00;
	command[1]= 0x00;
	command[2]= 0x00;
	command[3]= address;
	command[4]= address>>8;
	command[5]= address>>16;
	command[6]= 0x01;/*address>>24*/;
	command[7]= 0xB7;
	return dsCardi_ReadCard(address,pbuf);	
}

uint32 dsCardi_TstSDCard(uint8 cmd,uint32 address,uint8* pbuf,uint32 wait) 
{

	uint8 command[8];
	command[0]= 0x00;
	command[1]= 0x00;
	command[2]= 0x00;
	command[3]= address;
	command[4]= address>>8;
	command[5]= address>>16;
	command[6]= address>>24;
	command[7]= 0xba;
	return dsCardi_Read512ByteMode(command,pbuf,wait);

}

uint32 dsCardi_SectorPerClus(uint8 SecNO)
{
	uint8 command[8];
	command[0]= 0xAA;
	command[1]= 0x55;
	command[2]= 0xAA;
	command[3]= 0x55;
	command[4]= SecNO;
	command[5]= 0x03;
	command[6]= 0xf4;
	command[7]= 0xB8;
	return dsCardi_Read4ByteMode(command);
}

uint32 dsCardi_SetModifyMode()
{
	uint8 command[8];
	command[0]= 0x00;
	command[1]= 0x00;
	command[2]= 0x00;
	command[3]= 0x00;
	command[4]= 0x5a;
	command[5]= 0xa5;
	command[6]= 0xff;
	command[7]= 0xB9;
	return dsCardi_Read4ByteMode(command);
}

uint32 dsCardi_ModifyReturn()
{
	uint8 command[8];
	command[0]= 0x00;
	command[1]= 0x00;
	command[2]= 0x00;
	command[3]= 0x00;
	command[4]= 0xaa;
	command[5]= 0x55;
	command[6]= 0xff;
	command[7]= 0xB9;
	return dsCardi_Read4ByteMode(command);
}

uint32 dsCardi_SectorCleanromMode()
{
	uint8 command[8];
	command[0]= 0x00;
	command[1]= 0x00;
	command[2]= 0x00;
	command[3]= 0x00;
	command[4]= 0xa5;
	command[5]= 0x5a;
	command[6]= 0xff;
	command[7]= 0xB9;
	return dsCardi_Read4ByteMode(command);
}

uint32		dsCardi_SetRompage(uint32 address)
{
	uint32 page= address/0x20000 ;
	uint8 command[8];
	command[0]= 0x00;
	command[1]= 0x55;
	command[2]= 0xaa;
	command[3]= 0x55;
	command[4]= page;
	command[5]= 01;
	command[6]= 0xF4;/*address>>24*/;
	command[7]= 0xB8;
	return dsCardi_Read4ByteMode(command);
}

uint32 dsCardi_WriteFlashCommand(uint32 address,uint8 cmd)
{
	uint32 temp;
	uint32 status=0, flags= 0x00586000;
	WAIT_CR &= ~0x0800;
	uint8 command[8];
	command[0]= cmd;
	command[1]= 0x00;
	command[2]= 0x00;
	command[3]= address;
	command[4]= address>>8;
	command[5]= address>>16;
	command[6]= 0xF0; 
	command[7]= 0xB8;
  	cardWriteCommand(command);
  	flags = 0x00586000;
  	flags &= (~0x07000000);
	flags |= 0xa7000000 ;
	flags &= 0xFFFFE000 ;
	CARD_CR2 = flags ;
	do
	{
		status = CARD_CR2; //0x40001a4
	}while(!(status & 0x800000));
  	temp = CARD_DATA_RD;
}

uint32 dsCardi_ReadFlashCommand(uint32 address)
{
	uint8 command[8];
	command[0]= 0x00;
	command[1]= 0x00;
	command[2]= 0x00;
	command[3]= address;
	command[4]= address>>8;
	command[5]= address>>16;
	command[6]= address>>24;
	command[7]= 0xB7;
	return dsCardi_Read4ByteMode(command);
}

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//SLOT1上卡带 Flash 的基本操作
uint32 ResetFlash()
{
	dsCardi_WriteFlashCommand(0,0xFF);
	return 0 ;
}

uint32 EraseFlash(uint32 address)
{
	uint32 status,loop;
	dsCardi_WriteFlashCommand(0,0x50);
	dsCardi_WriteFlashCommand(address,0xFF);
	dsCardi_WriteFlashCommand(address,0x20);
	dsCardi_WriteFlashCommand(address,0xD0);
	loop = 0x100000;
	do
	{
		status=dsCardi_ReadFlashCommand(address);
		loop-- ;
	}while( ((status&0x80)!=0x80)&&loop);
	if(loop)
		return 0;
	else
		return 0xEFEE0000 ;//超时
}

uint32 WriteFlash(uint32 address,uint8 *pbuf,uint32 len)
{
	uint16 i;
	uint32 temp,status;
	uint32 loop;
	uint8 command[8];
	uint32 tempadd = address;
	
	if(len%256!=0)
		return 0xEFEE0000;
	for(loop=0;loop<len;loop+=256)
	{
		tempadd = address;
		dsCardi_WriteFlashCommand(address,0x41);
		for(i=0;i<256;i+=2)
		{
			command[0]= pbuf[loop+i+1];
			command[1]= pbuf[loop+i];
			command[2]= 0x00;
			command[3]= tempadd;
			command[4]= tempadd>>8;
			command[5]= tempadd>>16;
			command[6]= 0xF1; 
			command[7]= 0xB8;
	  		temp = dsCardi_Read4ByteMode(command);
	  		tempadd+=2;
		}
		dsCardi_WriteFlashCommand(address,0x0E);
		dsCardi_WriteFlashCommand(address,0xD0);
		//wait
		dsCardi_WriteFlashCommand(address,0x70);
		do
		{
			status=dsCardi_ReadFlashCommand(address);
		}while( (status&0x80)!=0x80);	
		address+=256;
	}

	return 0;

}



void SD_ReadLoop_wait(int lp,int wait0)
{
	uint8 ppbuf[512];
	uint8 command[8];
	command[0]= 0x00;
	command[1]= 0x00;
	command[2]= 0x00;
	command[3]= 0x00;
	command[4]= 0x00;
	command[5]= 0x00;
	command[6]= 0xF8;
	command[7]= 0xB8;
	dsCardi_Read512ByteMode(command,ppbuf,wait0);
}

#define EepromWaitBusy()	while (CARD_CR1 & /*BUSY*/0x80);
void dsCardWriteSaveFlash(uint32 address, uint8 *data, uint32 length) 
{
	uint32 address_end = address + length;
	int i;
	while (address < address_end)
	{
		// set WEL (Write Enable Latch)
		CARD_CR1 = /*E*/0x8000 | /*SEL*/0x2000 | /*MODE*/0x40;
		CARD_EEPDATA = 0x06; EepromWaitBusy();
		CARD_CR1 = /*MODE*/0x40;

		// program maximum of 32 bytes
		CARD_CR1 = /*E*/0x8000 | /*SEL*/0x2000 | /*MODE*/0x40;
		CARD_EEPDATA = 0x02 ;EepromWaitBusy();
		CARD_EEPDATA = address >> 16; EepromWaitBusy(); 
		CARD_EEPDATA = address >> 8; EepromWaitBusy(); 
		CARD_EEPDATA = address & 0xFF; EepromWaitBusy();
		for (i=0; address<address_end && i<32; i++, address++) { CARD_EEPDATA = *data++; EepromWaitBusy(); }
		CARD_CR1 = /*MODE*/0x40;

		// wait programming to finish
		CARD_CR1 = /*E*/0x8000 | /*SEL*/0x2000 | /*MODE*/0x40;
		CARD_EEPDATA = 0x05; EepromWaitBusy();
		do { CARD_EEPDATA = 0; EepromWaitBusy(); } while (CARD_EEPDATA & 0x01);	// WIP (Write In Progress) ?
		CARD_CR1 = /*MODE*/0x40;
	}
}


void dsCardReadSaveFlash(uint32 address, uint8 *data, uint32 length)
{
	CARD_CR1 = /*E*/0x8000 | /*SEL*/0x2000 | /*MODE*/0x40;
	CARD_EEPDATA = 0x03; EepromWaitBusy();
	 CARD_EEPDATA = address >> 16; EepromWaitBusy(); 
	 CARD_EEPDATA = address >> 8; EepromWaitBusy();
	CARD_EEPDATA = address & 0xFF; EepromWaitBusy();
	while (length > 0)
	{
		CARD_EEPDATA = 0; EepromWaitBusy();
		*data++ = CARD_EEPDATA;
	}
	CARD_CR1 = /*MODE*/0x40;
}

void WriteFlashCommand(uint32 address,uint8 cmd)
{
	uint32 temp;
	uint32 status=0, flags= 0x00586000;
	WAIT_CR &= ~0x0800;
	uint8 command[8];
	command[0]= cmd;
	command[1]= 0x00;
	command[2]= 0x00;
	command[3]= address;
	command[4]= address>>8;
	command[5]= address>>16;
	command[6]= 0xF0; 
	command[7]= 0xB8;
  	cardWriteCommand(command);
  	flags = 0x00586000;
  	flags &= (~0x07000000);
	flags |= 0xa7000000 ;
	flags &= 0xFFFFE000 ;
	CARD_CR2 = flags ;
	do
	{
		status = CARD_CR2; //0x40001a4
	}while(!(status & 0x800000));
  	temp = CARD_DATA_RD;
}
uint32 ReadFlashCommand(uint32 address)
{
	uint32 temp,temp1;
	uint32 status=0, flags= 0x00586000;
	WAIT_CR &= ~0x0800;
	uint8 command[8];
	command[0]= 0x00;
	command[1]= 0x00;
	command[2]= 0x00;
	command[3]= address;
	command[4]= address>>8;
	command[5]= address>>16;
	command[6]= address>>24;
	command[7]= 0xB7;
	cardWriteCommand(command);
  	flags = 0x00586000;
  	flags &= (~0x07000000);
	flags |= 0xa7000000 ;
	flags &= 0xFFFFE000 ;
	CARD_CR2 = flags ;
	do
	{
		status = CARD_CR2; //0x40001a4
	}while(!(status & 0x800000));  
  	temp = CARD_DATA_RD;
  	temp1= CARD_DATA;
  	return temp;
}
void BL_EraseFlash(uint32 address)
{
	uint32 status;
	WriteFlashCommand(0,0x50);
	WriteFlashCommand(address,0xFF);
	WriteFlashCommand(address,0x20);
	WriteFlashCommand(address,0xD0);
	do
	{
		status=ReadFlashCommand(address);
	}while( (status&0x80)!=0x80);
}
void BL_ReadFlashBuffer(uint32 address,uint8 *ppbuf,uint32 size)
{
	uint32 tempdata;
	uint8 *p = (uint8 *)(&tempdata);
	WriteFlashCommand(0,0xFF);
	for(int32 loop=0;loop<size;loop+=4)
	{
			tempdata = ReadFlashCommand(address+loop);
			ppbuf[loop]=p[0];
			ppbuf[loop+1]=p[1];
			ppbuf[loop+2]=p[2];
			ppbuf[loop+3]=p[3];
	}	
}
bool BL_WriteFlash(uint32 address,uint8 *pbuf,uint32 len)
{
	uint16 i;
	uint32 temp;
	uint32 loop;
	uint8 command[8];
	uint32 status=0, flags= 0x00586000;
	uint32 tempadd = address;
	
	if(len%256!=0)
		return false;
	for(loop=0;loop<len;loop+=256)
	{
		tempadd = address;
		WriteFlashCommand(address,0x41);
		for(i=0;i<256;i+=2)
		{
			//WriteFlashCommand(address+i,pbuf[i]);
			WAIT_CR &= ~0x0800;
			command[0]= pbuf[i+loop+1];
			command[1]= pbuf[i+loop];
			command[2]= 0x00;
			command[3]= tempadd;
			command[4]= tempadd>>8;
			command[5]= tempadd>>16;
			command[6]= 0xF1; 
			command[7]= 0xB8;
	  		cardWriteCommand(command);
	  		flags = 0x00586000;
	  		flags &= (~0x07000000);
			flags |= 0xa7000000 ;
			flags &= 0xFFFFE000 ;
			CARD_CR2 = flags ;
			do
			{
				status = CARD_CR2; //0x40001a4
			}while(!(status & 0x800000));
	  		temp = CARD_DATA_RD;
	  		tempadd+=2;
		}
		WriteFlashCommand(address,0x0E);
		WriteFlashCommand(address,0xD0);
		//wait
		WriteFlashCommand(address,0x70);
		do
		{
			status=ReadFlashCommand(address);
		}while( (status&0x80)!=0x80);	
		address+=256;
	}
	return true;
}
#ifdef __cplusplus
}
#endif
