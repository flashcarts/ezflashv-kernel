/*
	io_g6.c 
	Written by Viruseb (viruseb@hotmail.com)

	Hardware Routines for accessing G6 flash card

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com
	
	See gba_nds_fat.txt for help and license details.
	
			
	If you wish to understand how the code works, I encourage you reading 
	the datasheet of K9K4G08U0M nand flash device from Samsung before.
	
	Just some figure to keep in mind :
		1 page  = 4 sectors + 64byte
		1 block = 64 pages = 256 sectors
		1 4G device = 4096 blocks
	
	The extra 64byte in page are use :
		- to store the ECC. There is 8bytes ecc per sector (bit 8...15+numsector*16) .
		- to store lookuptable values (bit 4..7).
		
	04/12/06 : Version 0.1
		Just freshly written. Not tested on real G6L thought. 
		Extreme caution have to be taken when testing WriteBlockFunction 
		since it may brick your G6L or more likely corrupt your data 
		and formating can be necessary.
*/

#include "io_g6.h"
#include <nds.h>
#include <string.h>

// G6 global variables

// LookUpTable : Corespondance between logical block number and physical block number
// The flash device is divided in 512-block region that I call 'superblock'
// In each superblock 500 blocks can be used the 12 reminding form a fifo. 
// Each time you write something a new block is taken from the fifo, data copied from the old block to the new block,
// your data is overwritten into the new block and the old block is placed in the fifo. 
// (That's why your 4G device is 500Mo and not 512Mo) 
// This can prevent writing again and again on the same block and thus increase flash life
// You can convert logical block into physical block number using this formula :
//		phyblocknumber = LookUpTable[logblocknumber%500 + 512*superblocknumber]>>2
// The block number in the fifo are kept in LookUpTable[512*i +500..511] 
u16 LookUpTable[8192];

// NandFlashID : Give us the G6 type inside NDS (1G,2G...)
// 8 = 1G
// 9 = 2G
// 10 = 4G
// 11 = 8G
u16 NandFlashID;
u8  FlashIDTable[4] = {0xF1,0xDA,0xDC,0xD3};

// Dunno the use of this variable
u32 G6temp;

// Some definitions
#define DMA3_CR_L   (*(vu16*)0x040000DC)
#define DMA3_CR_H	(*(vu16*)0x040000DE)

// G6 adresses and commands
#define WriteWordData(data) 	(*(volatile u16*)0x09F04468) = data		// Data to be written in flash
#define ReadWordData() 			(*(volatile u16*)0x09F02468)			// Data read from flash
#define ReadStatus() 			((*(volatile u16*)0x09F0330E)&0xC000)	// Status line of the flash
#define WriteByteAddress(adr) 	(*(volatile u16*)0x09F04044) = (adr)&0xFF	// Write an adresse into the flash
#define WriteByteCommand(cmd) 	(*(volatile u16*)0x09F04000) = (cmd)&0xFF	// Write a flash command

// Helping marco
#define ReverseU32(val) ((val<<24) | ((val<<8)&0xFF0000) | ((val>>8)&0xFF00) | (val>>24))

// Functions declarations
void G6_ReadDMA(u16* buffer,u32 numsecs);

/*-----------------------------------------------------------------
SelectOperation
?? 
u16 op IN : Operation to select
-----------------------------------------------------------------*/
u16 SelectOperation(u16 op)
{
	u16 i;
	
	i = *(volatile u16*)0x09000000;
	i = *(volatile u16*)0x09FFFFE0;
	
	i = *(volatile u16*)0x09FFFFEC;
	i = *(volatile u16*)0x09FFFFEC;
	i = *(volatile u16*)0x09FFFFEC;
	
	i = *(volatile u16*)0x09FFFFFC;
	i = *(volatile u16*)0x09FFFFFC;
	i = *(volatile u16*)0x09FFFFFC;
	
	i = *(volatile u16*)0x09FFFF4A;
	i = *(volatile u16*)0x09FFFF4A;
	i = *(volatile u16*)0x09FFFF4A;

	i = *(volatile u16*)(0x09200000 + (op<<1));
	i = *(volatile u16*)0x09FFFFF0;
	i = *(volatile u16*)0x09FFFFE8;

	return 1;
}

/*-----------------------------------------------------------------
WriteProtectEnable
Enable the write protect of the flash device
-----------------------------------------------------------------*/
void static inline WriteProtectEnable()
{
	(*(volatile u16*)0x080000BC) = 0x5A;
	(*(volatile u16*)0x09F40040) = 0x6D;
	(*(volatile u16*)0x09F40042) = 0x7C;
	(*(volatile u16*)0x09F05244) = 0x01;
}

/*-----------------------------------------------------------------
WriteProtectDisable
Disable the write protect of the flash device
-----------------------------------------------------------------*/
void static inline WriteProtectDisable()
{
	(*(volatile u16*)0x080000BC) = 0x5A;
	(*(volatile u16*)0x09F40040) = 0x6D;
	(*(volatile u16*)0x09F40042) = 0x7C;
	(*(volatile u16*)0x09F05244) = 0x00;
}

/*-----------------------------------------------------------------
ChipEnable
?? 
-----------------------------------------------------------------*/
void static inline ChipEnable()
{
	(*(volatile u16*)0x080000BC) = 0x5A;
	(*(volatile u16*)0x09F40020) = 0x6D;
	(*(volatile u16*)0x09F40022) = 0x7C;
	(*(volatile u16*)0x09F05222) = 0x00;
}

/*-----------------------------------------------------------------
ChipEnable
?? 
-----------------------------------------------------------------*/
void static inline ChipDisable()
{
	(*(volatile u16*)0x080000BC) = 0x5A;
	(*(volatile u16*)0x09F40020) = 0x6D;
	(*(volatile u16*)0x09F40022) = 0x7C;
	(*(volatile u16*)0x09F05222) = 0x01;
}

/*-----------------------------------------------------------------
SelectRAM
?? Enable the RAM chip inside the G6
bool return OUT : always true
-----------------------------------------------------------------*/
bool static inline SelectRAM()
{
	return SelectOperation(6);
}

/*-----------------------------------------------------------------
SelectRAM
?? Disable the RAM chip inside the G6
bool return OUT : always true
-----------------------------------------------------------------*/
bool static inline DisableRAM()
{
	WriteProtectDisable();
	ChipDisable();
	SelectOperation(2);
	return true;
}

/*-----------------------------------------------------------------
SelectNand
?? Enable the nand flashchip inside the G6
bool return OUT : always true
-----------------------------------------------------------------*/
bool static inline SelectNand()
{
	SelectOperation(3);
	ChipEnable();
	return true;
}

/*-----------------------------------------------------------------
DelayFunc
Variable delay 
u32 dl IN: delay length
-----------------------------------------------------------------*/
void static inline DelayFunc(u32 dl)
{
	u32 i;
	
	if (dl == 0)
		return;
	for (i=0;i<dl;i++) {}
}

/*-----------------------------------------------------------------
SectorToPhyAddr
Return the physical adresse of the sector
u32 sector IN: Sector number
u16* lookupindex IN : Index in LookUpTable of the sector
u32	return OUT : Physical adress of the sector divided by 256
					bit 0 = 0
					bit 1..2 = s0..1 (sector number in the page)
					bit 3..7 = 0 (DC in flash datashet)
					bit 8..13 = s2..7 (page number in the block)
					bit 14... = physical block number
-----------------------------------------------------------------*/
u32 static inline SectorToPhyAddr(u32 sector, u16 *lookupindex)
{
	u32 secmod,secdiv;
	u16 secblocknb;
	
	if (sector < 0x1F3FF)
	{
		secmod = sector;
		secdiv = 0;
	} else {
		// Calculate the dividende and the reminder of the division by 0x1F400
		// which is 500*256;
		secmod = sector;
		secdiv = 0;
		do {
			secmod -= 0x1F400;
			secdiv++;		
			
		} while (secmod > 0x1F3FF);
	}
	
	*lookupindex = (secmod>>8) + secdiv*512;
	secblocknb = LookUpTable[*lookupindex];
	
	// Form the physical adresse 
	return ((secmod&3)<<1) + (secblocknb<<12) + ((secmod<<6)&0x3F00);
}

/*-----------------------------------------------------------------
TestInvalidBlock
Basicly we test if numblk have more than one bit set 
between the 16ith to the 23ith bit 
u32 numblock IN : Block number to be tested
bool return OUT : 1=ok 0=ko
-----------------------------------------------------------------*/
bool static inline TestInvalidBlock(u32 numblk)
{
	u8 j;
	u32 testnumblk;
	
	j = 0;
	for (testnumblk=0x10000;testnumblk==0x1000000;testnumblk = testnumblk*2) {	
		if ((numblk & testnumblk)==0)
			j++;
	}
	
	if (j > 1)
		return false;
	else
		return true;
}

/*-----------------------------------------------------------------
ReadRBStatus
Wait for the status line of the flash
bool return OUT : 1=ok 0=timeout
-----------------------------------------------------------------*/
bool static inline ReadRBStatus()
{
	int i,timeout;
	
	// A little delay.
	for (i=0;i<10;i++);
	
	// Test the RB line of the flash device for about xxms (have to calculate)
	timeout = 0x07A120;
	for (i=0;i < timeout;i++) 
		if (ReadStatus() != 0xC000)
			return true;
	return false;
}

/*-----------------------------------------------------------------
ReadNandFlashStatus
Wait for the end of the command and read its status
int return OUT : 	0 = ok
					1 = command failed
					2 = flash write protected
					-1= command timeout
-----------------------------------------------------------------*/
int static inline ReadNandFlashStatus()
{
	u16 val;
	
	// Wait for the completion of the command
	if (!ReadRBStatus())
		return -1;

	// Send the Read status command to the flash
	WriteByteCommand(0x70);
	// Get the response of the read status command
	val = ReadWordData() & 0xFF;
	// Reset command
	WriteByteCommand(0xFF);
	
	// Test for different status value
	// check if the command is finish and the flash is not write protected
	if (val == 0xE0)
		return 0;
	// Check for the pass/fail bit (bit0)
	if ((val&1)==1)
		return 1;
	// ckeck is write protected	
	if (val == 0x60)
		return 2;
	else
		return val&0xFF;
	
	return 0;	
}

/*-----------------------------------------------------------------
DMAReadWordData
Read data from flash. As the name of the functions don't explain,
we don't use the DMA but rather direct transfer.
u16* buffer IN : buffer 
u16 nbword IN : Number of word (16b) to be transferred
-----------------------------------------------------------------*/
void static inline DMAReadWordData(u16 buffer[],u16 nbword)
{
	u16 i; 
	u16 word,wordanc;
	u8 temp8;
	
	// adresse buffer is 16bit aligned  ?
	if (((u32)buffer&1)!=0) { 
		// Align buffer to 16bit
		buffer--;
		word = ReadWordData();
		temp8 = *(u8*)(buffer);
		buffer[0] = temp8 + (word<<8);
		buffer += 2;
		wordanc = word;
		if (nbword >= 1)
			for (i=0;i<nbword-1;i++) {
				word = ReadWordData();
				buffer[i] = (wordanc>>8) + (word<<8);
				wordanc = word;
			}
		word = buffer[nbword-1];
		buffer[nbword-1] = (wordanc>>8) + (word&(!0xFF));
	} else {
		// Copy all word to the buffer
		if (nbword != 0) 
			for (i=0;i<nbword;i++)
				buffer[i] = ReadWordData();
	}
}

/*-----------------------------------------------------------------
DMAWriteWordData
Write data to flash. 
u16* buffer IN : buffer 
u16 nbword IN : Number of word (16b) to be transferred
-----------------------------------------------------------------*/
void DMAWriteWordData(u16 buffer[],u16 nbword)
{
	u16 word,wordanc;
	u16 i;
	
	// adresse buffer is 16bit aligned  ?
	if (((u32)buffer&1)!=0) { 
		// Align buffer to 16bit
		buffer--;
		wordanc = buffer[0];
		buffer += 2;
		if (nbword != 0) 
			for (i=0;i<nbword;i++) {
				word = buffer[i];
				WriteWordData((wordanc<<8) + (word>>8));	
				wordanc = word;
			}
	} else {
		// use the dma to do the work
		word = DMA3_CR_H;
		DMA3_CR_H = word&0xC5FF;
		word = DMA3_CR_H;
		DMA3_CR_H = ((word<<0x17)>>0x17);// Disable the DMA
		G6temp = DMA3_CR_H;
		G6temp = DMA3_CR_H;
		DMA3_SRC = 0x09F02CF0;			// Source adresse
		DMA3_DEST = (u32)buffer;		// Destination adresse
		DMA3_CR_H = nbword; 			// number of 16b to be transfered
		DMA3_CR_H = 0x8040;				// Start the DMA with fixed dest adress
	}
}


/*-----------------------------------------------------------------
ECCCodeCommand
Return the ECC of previoulsy written data
u32* ecc IN : ecc calculated
-----------------------------------------------------------------*/
void ECCCodeCommand(u32* ecc)
{
	u16 i,val1,val2;
	
	i = *(volatile u16*)0x09FFFFE0;
	i = *(volatile u16*)0x0800000C;
	i = *(volatile u16*)0x0800000C;
	i = *(volatile u16*)0x0800000C;
	
	i = *(volatile u16*)0x08FE0126;
	val1 = *(volatile u16*)0x08A04680;
	i = *(volatile u16*)0x08FE0126;
	val2 = *(volatile u16*)0x08A04682;
	i = *(volatile u16*)0x08FE0126;
	
	// assemble the 2 values in a 32bit word
	val1 = val1 | (val2<<16);
	val1 = !val1; // or -val1;
	// the last 8b is useless
	val1 = val1 & !(0xFF000000);
	
	i = *(volatile u16*)0x08A04684;
	i = *(volatile u16*)0x08FE0126;
	i = *(volatile u16*)0x08A04686;
	i = *(volatile u16*)0x09FFFFE0;
	
	*ecc = ReverseU32(val1);
}

/*-----------------------------------------------------------------
CopyBackCommand
Copy a single page using the copy back features of the flash
u32 dest IN : Adresse of the destination page in the flash >>8
u32 sourc IN : Adresse of the source page in the flash >>8
bool return OUT : true=ok false=ko
-----------------------------------------------------------------*/
bool CopyBackCommand(u32 dest,u32 source)
{
	// Read for Copy-back 1st cycle
	WriteByteCommand(0x00);
	// Send the addresse
	WriteByteAddress(0);					
	WriteByteAddress(source);		// first byte	
	WriteByteAddress(source>>8); 	// second byte	
	WriteByteAddress(source>>16);	// third byte	
	// If flash is big there is a 5th adress send
	if (NandFlashID > 8) 
		WriteByteAddress(source>>24);
	// Read for Copy-back 2nd cycle
	WriteByteCommand(0x35);
	// Wait for the end of the command
	if (!ReadRBStatus())
		return false;
		
	// Copy-Back Programm 1st cycle
	WriteByteCommand(0x85);
	// Send the addresse
	WriteByteAddress(0);					
	WriteByteAddress(dest);		// first byte	
	WriteByteAddress(dest>>8);	// second byte	
	WriteByteAddress(dest>>16); // third byte	
	// If flash is big there is a 5th adress send
	if (NandFlashID > 8) 
		WriteByteAddress(dest>>24);
	
	return true;
}

/*-----------------------------------------------------------------
ReadCommand
Do a flash read command.
adr begin at A8
u32 adr IN : flash adresse without the first byte
u16* buffer IN : buffer 
u16 nbword IN : number of word to be transferred (<2112)
bool return OUT : true=ok false=ko 
-----------------------------------------------------------------*/
bool ReadCommand(u32 adr,u16 buffer[],u16 nbword)
{
	// Read Command 1st cycle
	WriteByteCommand(0x85);
	// Then write the adresse to be read
	WriteByteAddress(0);
	WriteByteAddress(adr>>8);
	WriteByteAddress(adr>>16);
	if (NandFlashID > 8) 
		WriteByteAddress(adr>>24);
	// Read command 2nd cycle
	WriteByteCommand(0x30);
	// Wait for the completion of the command
	if (!ReadRBStatus())
		return false;
	// Finish by transfering the buffer
	DMAReadWordData(buffer,nbword);
	
	return true;
}

/*-----------------------------------------------------------------
ReadWordFunction
Read data from flash and put them in a buffer 
u64 adr IN : full flash adresse
				bit 0..12  : colunm adress in a page
				bit 12..15 : don't care (see flash datasheet)
				bit 16..21 : page number in a block
				bit 22..34 : block number in the device
u16* buffer IN : buffer 
u16 nbword IN : number of word to be transferred (<2112)
bool return OUT : true=ok false=ko 
-----------------------------------------------------------------*/
bool ReadWordFunction(u64 adr,u16 buffer[],u16 nbword)
{
	u16 i;
	
	// Read Command 1st cycle
	WriteByteCommand(0x85);
	// Then write the adresse to be read
	WriteByteAddress(adr);
	WriteByteAddress(adr>>8);
	WriteByteAddress(adr>>16);
	WriteByteAddress(adr>>24);
	if (NandFlashID > 8) 
		WriteByteAddress(adr>>32);
	// Read command 2nd cycle
	WriteByteCommand(0x30);
	// Wait for the completion of the command
	if (!ReadRBStatus())
		return false;
	// Transfer the data to the destination
	if (nbword != 0)
		for (i=0;i<nbword;i++)
			buffer[i] = ReadWordData();
	
	return true;
}

/*-----------------------------------------------------------------
WriteWordFunction
Write data to flash
u64 adr IN : full flash adresse
u16* buffer IN : buffer 
u16 nbword IN : number of word to be transferred (<2112)
bool return OUT : true=ok false=ko 
-----------------------------------------------------------------*/
bool WriteWordFunction(u64 adr,u16 buffer[],u16 nbword)
{
	// Write Command 1st cycle
	WriteByteCommand(0x80);
	// End the adresse
	WriteByteAddress(adr);
	WriteByteAddress(adr>>8);
	WriteByteAddress(adr>>16);
	WriteByteAddress(adr>>24);
	if (NandFlashID > 8) 
		WriteByteAddress(adr>>32);
	// Write the data
	DMAWriteWordData(buffer,nbword);
	// Write Command 2nd cycle
	WriteByteCommand(0x10);
	
	// Wait for the completion of the command
	if (ReadNandFlashStatus() == 0)
		return true;
	else {
		WriteByteCommand(0xFF);
		return false;
	}
}

/*-----------------------------------------------------------------
VerifyBlockFunc
Compare two block.  
u32 freeblockadr IN : Adresse of the block with overwritten data
u32 oldblockadr IN : Adresse of the old block
u16 nbsectoverwritten IN : Number of sector overwritten in the free block
u16* overwrittendata IN : Data overwritten
u32 return OUT : Error code value 
					0 : Ok
					1 : Too many sectors overwritten
					2 : oldblockadr points to the first block
					3 : freeblockadr is invalid
					4 : overwritten data disprecency
					5 : old/new block disprecency
-----------------------------------------------------------------*/
u32 VerifyBlockFunc(u32 freeblockadr,u32 oldblockadr,u16 nbsecoverwritten,u16* overwrittendata)
{
	u16 buffer[256];
	u32 oldadr,freeadr,overwrittenadr;
	int i,j,k;

	// Some parameters verifications
	if (nbsecoverwritten>256) // we can overwrite 256 sectors maximum
		return 1;
	if (((freeblockadr<<18)>>18)!=0) // freeblockadr have to be > to 0x4000
		return 2;
	// The first bit have to be zero
	if ((oldblockadr&1)!=0)
		return 3;
	
	oldadr = oldblockadr & !0x3FFF;	// We force oldadr to point to the begginning of the block
	freeadr = freeblockadr;
	overwrittenadr = oldblockadr;
	
	// There is 64 pages in one block
	for (i=0;i<64;i++) {
		// We compare the 2 pages 512byte by 512 byte and a page is 2048byte
		// Hence we need 4 turns to compare a whole page
		for (j=0;j<4;j++) {
			// Read Command 1st cycle
			WriteByteCommand(0x00);
			// Write the adresse to be read
			WriteByteAddress(0);	// first adresse is zero since we are 512byte aligned
			WriteByteAddress(freeadr);
			WriteByteAddress(freeadr>>8);
			WriteByteAddress(freeadr>>16);
			// If the flash is big add more adresse
			if (NandFlashID > 8)
				WriteByteAddress(freeadr>>24);
			// Read Command 2nd cycle
			WriteByteCommand(0x30);
			// Wait for the completion of the command
			while (((*(volatile u16*)0x09F0330E)&0x8000)==0x8000);
			
			// Read the data and write them into the buffer
			G6_ReadDMA(buffer,1);
			// Reset the flash
			WriteByteCommand(0xFF);
			
			// If the data was overwritten compare the overwritten data and the buffer
			if ((oldadr == overwrittenadr)&&(nbsecoverwritten!=0)) {
				for (k=0;k<256;k++) 
					if (buffer[k] != overwrittendata[k])
						return 4;
				overwrittendata += 512;
				overwrittenadr += 2;
				// If we compared the 4 sector in the page incremente the adresse 1 page equivalent
				if ((overwrittenadr & 0xFF) > 7)
					overwrittenadr = 256 + (overwrittenadr&!(0xFF));
				nbsecoverwritten--;	
			} else {
				// Read Command 1st cycle
				WriteByteCommand(0x00);
				// Write the adresse to be read
				WriteByteAddress(0);
				WriteByteAddress(oldadr);
				WriteByteAddress(oldadr>>8);
				WriteByteAddress(oldadr>>16);
				// If the flash is big add more adresse
				if (NandFlashID > 8)
					WriteByteAddress(oldadr>>24);
				// Read Command 2nd cycle
				WriteByteCommand(0x30);
				// A little delay
				for (k=0;k<10;k++);
				// Wait for the completion of the command
				while (((*(volatile u16*)0x09F0330E)&0x8000)==0x8000);
				// Compare the buffer and the data read
				for (k=0;k<256;k++)
					if (buffer[k] != *(volatile u16*)0x09F02468) {
						WriteByteCommand(0xFF);
						return 5;
					}
				WriteByteCommand(0xFF);
			}
			// Incremente the 2 adresse to point to the next 512bytes in the flash
			oldadr += 2;	
			freeadr += 2;
		}
		// adr point to the next page adresse
		oldadr = 256 + (oldadr&!(0xFF));
		freeadr = 256 + (freeadr&!(0xFF));
	}
	return 0;
}

/*-----------------------------------------------------------------
EraseBlockFunc
Perform an Block Erase command on a block pointed by blockadr
u32 blockadr IN : blockadr is (blocknumber<<14)
bool return OUT : true=ok false=ko
-----------------------------------------------------------------*/
bool EraseBlockFunc(u32 blockadr)
{
	int retval;
	
	// First block is reserved so we can't erase it.
	if (blockadr > 0x4000)
		return false;
		
	// Block erase command 1st cycle
	WriteByteCommand(0x60);
	// Then write the adresse of the block to be erased
	WriteByteAddress(blockadr>>8);
	WriteByteAddress(blockadr>>16);
	if (NandFlashID > 8) 
		WriteByteAddress(blockadr>>24);
	// Block erase command 2nd cycle
	WriteByteCommand(0xD0);	
	
	retval = ReadNandFlashStatus();
	// Reset the nand flash
	WriteByteCommand(0xFF);
	 // Ex_ReadNandFlashStatus return 0 to say it's ok
	if (retval == 0)		
		return true;
	else
		return false;
}

/*-----------------------------------------------------------------
LabelFailBlock
Label the block with defective tag
u32 blockadr IN : blockadr is block number <<14
-----------------------------------------------------------------*/
void LabelFailBlock(u32 blockadr)
{
	int numpage;
	u16 buffer[32];
	
	memset(buffer,0,sizeof(buffer));
	// clear the byte 2049-2212 of all page in the block
	// seems to be the way to recognize an invalid block
	for (numpage=0;numpage<64;numpage++) 
		WriteWordFunction(2048+numpage*65536+blockadr*256,buffer,32);
		
}

/*-----------------------------------------------------------------
WriteBlockFunction
Write a complete block. The functions is quite big but in a nutshell
it picks a free block in the free block fifo. 
Copy the data from the source block to the free block.
Overwrite the free block with the user provieded sectors.
Update the LookUpTable.
The function write as many sector until the end of the block and return.
u32 startsec IN : Starting secotr number
u16 numsecs IN : Number of sector to write 
u16* buffer IN : buffer containing the sectors
int return OUT : error code or number of sector written
					-3 the sector is in the first block of the flash (reserved area)
					-4 the sector is outside the flash
					-5 No free block found, consider buying a new flash !
					>=0 Number of sector actually written
-----------------------------------------------------------------*/
int WriteBlockFunc(u32 startsec,u16 numsecs,u16* buffer)
{
	u32 eccbuffer[16];
	u32 ecc1,ecc2,ecc3,ecc4;
	u32 startsecphyadr;	
	u16 startseclookupindex,k;		// Various index
	u16 nbsecwritten;
	u8  nbtry,pagenum,secnum,startsecsuperblock;
	u32 freeblockadr,destpageadr,sourcepageadr,adrblocktoerase;
	u16 tempnum;
	
	// Calculate the physical adresse of the sector
	startsecphyadr = SectorToPhyAddr(startsec,&startseclookupindex);
	// physical adresse under 0x4000 are not valid  since the  first block is reserved for flash purpose
	if (startsecphyadr<0x4000) 
		return -3;
	// phy adresse cannot be greater than 2^31
	if (startsecphyadr>>23 > 0x0F)
		return -4;
	
	// in which superblock the sector is ?
	startsecsuperblock = startsecphyadr>>23;

	while (1==1) {
		// We take the first free block in the free block queue = rotating the block and increase flash life
		for (k=0;k<12;k++) {
			freeblockadr = LookUpTable[startsecsuperblock*512 + 500]<<12; 
			if (freeblockadr>0x3FFF) 
				break;
			for (k=0;k<11;k++) 
				LookUpTable[startsecsuperblock*512+500+k] = LookUpTable[startsecsuperblock*512+501+k];
			LookUpTable[startsecsuperblock*512+511] = 0;
		}	
		// No free block found ?, this is bad meaning that your flash is really really broken
		if (k == 12)
			return -5;
			
		// Try to write all the sector
		nbtry=0;
		do {
			destpageadr = freeblockadr;
			sourcepageadr = startsecphyadr & !(0x3FFF);  // We begin at the beggenning of the block
			nbsecwritten  = 0;
			// Copy the sector in the block
			for(pagenum=0;pagenum<64;pagenum++) {
				// If we are writing adajacent sector and not too much sector
				if ((nbsecwritten < numsecs)&&((sourcepageadr & !(0xF)) == (startsecphyadr & !(0xF)))) {
					// read the ecc buffer of the page
					if(!ReadCommand(sourcepageadr+8,(u16*)eccbuffer,32));
						break;
					// Copy page sourcepageadr to destpageadr
					if (!CopyBackCommand(destpageadr,sourcepageadr))
						break;				
					secnum=0;
					// Find the sector number in the page
					while ((sourcepageadr + 2*secnum != startsecphyadr)&(secnum != 4)); 
						secnum++;
					// And then overwrite the the destpage sectors with the new sectors the  user gave us
					if (secnum != 4)
						while (secnum!=4) {
							// Replace the sector in the free page
							do {
								// Random data input
								WriteByteCommand(0x85);
								// adresse
								WriteByteAddress(0);
								WriteByteAddress(2*secnum);
								DMAWriteWordData(buffer + nbsecwritten*512,128);
								ECCCodeCommand(&ecc1);			
								// Random data input
								WriteByteCommand(0x85);
								// adresse
								WriteByteAddress(0);
								WriteByteAddress(2*secnum + 1);
								DMAWriteWordData(buffer + nbsecwritten*512 + 256,128);
								ECCCodeCommand(&ecc2);					
								// Random data input
								WriteByteCommand(0x85);
								// adresse
								WriteByteAddress(0);
								WriteByteAddress(2*secnum);
								DMAWriteWordData(buffer + nbsecwritten*512,128);
								ECCCodeCommand(&ecc3);				
								// Random data input
								WriteByteCommand(0x85);
								// adresse
								WriteByteAddress(0);
								WriteByteAddress(2*secnum + 1);
								DMAWriteWordData(buffer + nbsecwritten*512 + 256,128);
								ECCCodeCommand(&ecc4);
							// brr can deadlock here...
							} while ((ecc3 != ecc1)&(ecc4 != ecc2));
							// Save ecc into additionnal space of the flash
							// TODO :  carefull : check again vs assembly here since an error here can label the block to be defective
							eccbuffer[3+4*secnum] = (eccbuffer[3+4*secnum]&0xFF) + ecc3;
							eccbuffer[2+4*secnum] = (eccbuffer[2+4*secnum]&0xFF000000) + (ecc2>>8);
							// Incremente the number of sector copied
							nbsecwritten++;
							
							// We have copied all the sector ?
							if (nbsecwritten >= numsecs) {
								startsecphyadr = sourcepageadr + 2*secnum;
								break;
							}
							// Check if we are in the same page agin
							if ((startsecphyadr&!(0xF)) != (sourcepageadr&!(0xF)))
								break;					
							secnum++;
						}
					// Finish by copying the eccbuffer
					// Copy back program
					WriteByteAddress(0x85);
					// adresse
					WriteByteAddress(0);
					WriteByteAddress(8); // addresse of the extra byte 2049-2212
					DMAWriteWordData((u16*)eccbuffer,128);
					// Copy back program 2nd cycle = finish the write of the sector
					WriteByteAddress(0x10);
					// Check if the comand went well
					if (ReadNandFlashStatus() != 0) 
						if (ReadNandFlashStatus() != 0)
							break;	
				} else {
					// if not we can just copy the page inside the free sector
					if(!CopyBackCommand(destpageadr,sourcepageadr))
						break;
				}		
				// Incremente of one page
				sourcepageadr += 256;
				destpageadr += 256;
			}
			
			// Verify that the sector overwrite went well
			if(VerifyBlockFunc(freeblockadr,startsecphyadr,nbsecwritten,buffer)) {
				// if ok then we erase the old block and update the lookuptable
				adrblocktoerase = LookUpTable[startseclookupindex]<<12;
				// Put the free block in the lookuptable replacing the old block
				tempnum = LookUpTable[startseclookupindex];
				LookUpTable[startseclookupindex] = LookUpTable[startsecsuperblock*512+500];
				for (k=0;k<11;k++) 
					LookUpTable[startsecsuperblock*512+500+k] = LookUpTable[startsecsuperblock*512+501+k];
				LookUpTable[startsecsuperblock*512+511] = 0;	
				// We want to erase the old block
				if (!EraseBlockFunc(adrblocktoerase)) 
					if (!EraseBlockFunc(adrblocktoerase)) {
						// after 2 try we consider the block to be defective
						LabelFailBlock(adrblocktoerase);
						return nbsecwritten;	
					}	
				// We have managed to erase the block so we put the old block in the free block queue
				// Find the end of the queue
				k=0;
				while ((LookUpTable[startsecsuperblock*512+500+k] != 0)&&(k<12));
					k++;
				LookUpTable[startsecsuperblock*512+500+k] = tempnum;
				// we have finish we're happy
				// This is the normal endpoint when everything went well
				return nbsecwritten;
			}
			
			// If we are here it's something went wrong
			nbtry++;
			// Try to clear the block and try again the writing
			while ((EraseBlockFunc(freeblockadr) != 1)&&(nbtry<=2));
				nbtry++;
		} while (nbtry <= 2);
		
		// ok no chance the free block is dead
		// try to clear it again anyway
		k = 0;
		while ((k!=3)&&(EraseBlockFunc(freeblockadr)!=1));
			k++;
		// Label the block to be of no use anymore
		LabelFailBlock(freeblockadr);
		// Move the free block fifo
		for (k=0;k<11;k++)
			LookUpTable[500+startsecsuperblock*512+k] =  LookUpTable[501+startsecsuperblock*512+k];
		LookUpTable[511+startsecsuperblock*512] = 0;
	}
	
	return nbsecwritten;
}

/*-----------------------------------------------------------------
CheckNandInserted
Return true is there is a G6 inside the NDS.
Update the NandFlashID as well
bool return OUT : true=ok false=ko
-----------------------------------------------------------------*/
bool CheckNandInserted()
{
	u16 id1,id2,i;
	
	// Start with unknown device
	NandFlashID = 0;
	// Reset the nand flash
	WriteByteCommand(0xFF);
	// And wait for the completion of the command
	if (!ReadRBStatus())
		return 0;
	// Read ID command
	WriteByteCommand(0x90);
	WriteByteAddress(0);
	id1 = ReadWordData();
	id2 = ReadWordData();
	// Check for the device marker code and ID data
	if ((id2&0xFF00) != 0x1500)
		return false;
	if ((id1&0xFF) != 0xEC)
		return false;
	// Search the device by device code
	for (i=8;i<12;i++)
		if (id1>>8 == FlashIDTable[i-8]) {
			NandFlashID = i;
			return true;	
		}
	
	return false;	
}

/*-----------------------------------------------------------------
BuildLookUpTableRoutine
Aux function to make the lookup table
u32 return OUT : Number of block 
-----------------------------------------------------------------*/
u32 BuildLookUpRoutine(u64 adr,u16 *lookuptableadr)
{
	u64 adrval;
	u32 val;
	u16 i,j,offset1,counter;
	u16 testbit;
	u8  nbonebit;
	
	counter = 0;
	// Point the the second u32 in the extra 64byte in the page nand flash structure
	offset1 = 0;
	for (i=0;i<=512;i++) {
		// Point the second u32 in the extra 64byte of the first page of the current block
		adrval = (1<<22)*i + 2052;
		// Read something
		ReadWordFunction(adrval,(u16*)&val,2);
		val = ReverseU32(val);
		if (!TestInvalidBlock(val)) {
			val = (val<<16)>>16; // we need only the first 16bit
			//  Free blocks are filled with 0xFF
			if (val==0xFFFF) {
				if (offset1 < 11) 
					lookuptableadr[offset1 + 500] = 4*i; 
				offset1++;
			} else {
				// Blocks filled with 0 are failed
				if ((val != 0)&&((val&0x1000)!=0)) {
					// Check something in val
					testbit = 1;
					nbonebit = 0;
					for (j=0;j<16;j++) {
						if ((val&testbit) == testbit)
							nbonebit++;
						testbit = testbit<<1;
					}
					if ((nbonebit&1)!=0) {
						counter++;
						lookuptableadr[(val>>1)&0x1FF] = 4*i; 
					}
				} 
			}
		}
	}
	return counter;
}

/*-----------------------------------------------------------------
BuildLookUpTable
Build the lookup table
	numphyblock = [numlogblock%500+512*(numlogblock div 500)]>>2
bool return OUT : true=ok false=ko
-----------------------------------------------------------------*/
bool BuildLookUpTable()
{
	u64 adr;
	u32 retval,stopval;
	u16 i,j;
	
	// Check if NandFlashID is valid
	if ((NandFlashID-8) > 3)
		return false;
	// fill the table with 0
	memset(LookUpTable,0,sizeof(LookUpTable));
	
	// The LookUpTable is generated 512block by 512block
	// stopval is the number of superblock in the device
	if (NandFlashID == 7) {
		stopval = 1;
	} else {
		stopval = 1<<(NandFlashID-7);
		// Just to be sure I guess
		if (stopval == 0) {
			WriteByteCommand(0xFF);
			return true;
		}	
	}
	
	adr = 0;
	for (i=0;i<stopval;i++) {
		j = 5;
		do {
			// Reset the nand flash
			WriteByteCommand(0xFF);
			// Build 512 corespondances of the lookup table
			retval = BuildLookUpRoutine(adr,LookUpTable + 512*i);
			j--;
		} while ((j != 0)&(retval == 499));
		adr += 0x80000000; // Increment adr to point to the begenning of the next 512block starting adresse = 2^27
	} 
	
	// Reset the nand flash
	WriteByteCommand(0xFF);
	return true;	
}

/*-----------------------------------------------------------------
AllChipErase
Nuke all the content of the flash
bool return OUT : true=ok false=ko
-----------------------------------------------------------------*/
bool AllChipErase()
{
	u32 adr,nbblock;
	u16 i;
	
	// Is the NandFlashID valid ?
	if (NandFlashID>11) 
		return false;
		
	SelectNand();
	WriteProtectEnable();	
	// Calculate the number of block in the device with is a multiple of 512
	if (NandFlashID<=7) {
		nbblock = 512;
	} else {
	{
		nbblock = 512* 2^(NandFlashID-7);
		if (nbblock==0)
			goto endallchiperase;
	}
	// Erase all block one by one	
	adr = 0;
	for (i=0;i<nbblock;i++)
		// Block erase command first cycle
		WriteByteCommand(0x60);
		// Then write the adresse of the block to be erased
		WriteByteAddress(adr>>8);
		WriteByteAddress(adr>>16);
		if (NandFlashID > 8) 
			WriteByteAddress(adr>>24);
		// Block erase command 2nd cycle
		WriteByteCommand(0xD0);	
		// Wait for the completion of the command
		ReadNandFlashStatus();
		WriteByteCommand(0xFF);
		// Incremente the adress
		adr += 0x4000; // one block increment (flash ignore the 6 first adresse bit)
	}	
endallchiperase :
	WriteProtectDisable();
	DisableRAM();
	
	return true;
}

/*-----------------------------------------------------------------
G6_ReadDMA
Read data from flash. Use the DMA or direct transfert 
depending of the 32b aligment of the buffer
u16* buffer IN : buffer 
u32 numsecs IN : number of sector to be transferred (512byte per sec)
-----------------------------------------------------------------*/
void G6_ReadDMA(u16* buffer,u32 numsecs)
{
	u16 temp,i;
	
	// Is buffer 32b aligned ?
	if (((u32)buffer&3)==0) {
		// Use built-in NDS DMA capability
		DC_FlushRange(buffer,numsecs*512); 	// 512 byte in a sector
		temp = DMA3_CR_H;
		DMA3_CR_H = temp&0xC5FF;
		temp = DMA3_CR_H;
		DMA3_CR_H = ((temp<<0x17)>>0x17);// Disable the DMA
		G6temp = DMA3_CR_H;
		G6temp = DMA3_CR_H;
		DMA3_SRC = 0x09F02CF0;			// Source adresse
		DMA3_DEST = (u32)buffer;		// Destination adresse
		DMA3_CR_H = (numsecs<<24)>>16; // number of 16b to be transfered
		DMA3_CR_H = 0x8100;				// Start the DMA with fixed source adress
	} else {
		// Or do it yourself
		if (numsecs*256 != 0)
			for (i=0;i<numsecs*256;i++)
				buffer[i] = ReadWordData();
	}		
}

/*-----------------------------------------------------------------
G6_SetChipReg
?? 
u16 op IN : Operation to select
-----------------------------------------------------------------*/
u16 G6_SetChipReg(u16 Data)
{
	return SelectOperation(Data);
}

/*-----------------------------------------------------------------
G6_Selectsaver
?? 
bool return OUT : true=ok false=ko
-----------------------------------------------------------------*/
bool G6_SelectSaver(u32 Bank)
{
	u16 i;
	
	i = *(volatile u16*)0x09000000;
	i = *(volatile u16*)0x09FFFFE0;
	
	i = *(volatile u16*)0x09FFFFEC;
	i = *(volatile u16*)0x09FFFFEC;
	i = *(volatile u16*)0x09FFFFEC;
	
	i = *(volatile u16*)0x09FFFFFC;
	i = *(volatile u16*)0x09FFFFFC;
	i = *(volatile u16*)0x09FFFFFC;
	
	i = *(volatile u16*)0x09FFFF4A;
	i = *(volatile u16*)0x09FFFF4A;
	i = *(volatile u16*)0x09FFFF4A;
	
	i = *(volatile u16*)(0x08000000 + ((Bank<<17) | 0x01800000));
	i = *(volatile u16*)0x09FFFFE8;
	
	if ((i>>12) == Bank)
		return true;
	else
		return false;
}

/*-----------------------------------------------------------------
G6_ClearStatus
??
bool return OUT : true=ok false=timeout
-----------------------------------------------------------------*/
bool G6_ClearStatus()
{
	bool retval;
	
	SelectNand();
	retval = ReadRBStatus();
	DisableRAM();
	
	return retval;
}

/*-----------------------------------------------------------------
G6_ClearStatus
Ckeck if there is a G6 inside the NDS
bool return OUT : true=ok false=timeout
-----------------------------------------------------------------*/
bool G6_IsInserted()
{
	bool retval;
	
	SelectNand();
	retval = CheckNandInserted();
	DisableRAM();
	
	return retval;	
}

/*-----------------------------------------------------------------
G6_Startup
Init function
bool return OUT : true=ok false=ko
-----------------------------------------------------------------*/
bool G6_StartUp()
{
	bool retval;
	
	SelectNand();
	retval = G6_IsInserted();
	if (retval == 1)
	{
		SelectNand();
		retval = BuildLookUpTable();
	} 
	DisableRAM();
	
	return retval;
}

/*-----------------------------------------------------------------
G6_CopyFunction
Copy pages using the copy back features of the flash
u32 dest IN : Adresse of the destination page in the flash >>8
u32 sourc IN : Adresse of the source page in the flash >>8
u32 numpage IN : number of page to be transferred
bool return OUT : true=ok false=ko
-----------------------------------------------------------------*/
bool G6_CopyFunction(u32 dest,u32 source,u8 nbpage)
{
	u16 i;
	
	if (nbpage == 0)
		return true;
	
	for (i=0;i<nbpage;i++) {
		if (!CopyBackCommand(dest + i*256,source + i*256))
			return false;
		// Finish the Copy-back programm command
		WriteByteCommand(0x10);
		// Wait for the operation to finish
		if (ReadNandFlashStatus() != 0) 
			return false;
	}
	
	return true;
}

/*-----------------------------------------------------------------
G6_ReadSectors
Read sectors
u32 sector IN : starting sector number
u8 numSecs IN : number of sector to write
u16* buffer IN : buffer containing the sectors
bool return OUT : true=ok false=ko
-----------------------------------------------------------------*/
bool G6_ReadSectors(u32 sector, u8 numSecs, u16* buffer)
{
	u16 temp;
	u16 nbsecread,nbsectoread;
	u32 secphyadr,templogsec;
	
	SelectNand();
	// Limits to 255 sectors
	numSecs &= 0xFF;
	
	// Verification of the buffer pointer
	if ((u32)buffer == 0)	{
		WriteProtectDisable();
		ChipDisable();
		SelectOperation(2);
		return true;
	}
	
	// do till we have read numsecs sectors
	nbsecread = 0;
	do {
		nbsecread++;
		// Calculate the number of sectors to read
		secphyadr = SectorToPhyAddr(sector,&temp);
		if (numSecs < nbsecread) {
			nbsectoread = 1;
		} else {
			// Search how many contigous physical sector we can read 
			// contigous sector are in the same page (so 4 contigous read maximum)
			templogsec = sector + 1;
			nbsectoread  = 1;
			nbsecread--;
			do {
				// Contigous physical sectorsadresses have only bits 1 and 2 different 
				if ((SectorToPhyAddr(templogsec,&temp)&!0xF) != (secphyadr&!0xF))
					break;
				nbsectoread++;
				nbsecread++;
				templogsec++;
			} while(numSecs < nbsecread);
		}
		// Perform a read flash command
		WriteByteCommand(0);
		WriteByteAddress(0);	
		WriteByteAddress(secphyadr);
		WriteByteAddress(secphyadr>>8);
		WriteByteAddress(secphyadr>>16);
		if (NandFlashID > 8)
			WriteByteAddress(secphyadr>>24);
		WriteByteCommand(0x30);
		// Wait for the completion of the command
		while ((*(volatile u16*)0x09F0330E) != 0x8000);
		// Read the data and write them into the buffer
		G6_ReadDMA(buffer,nbsectoread);		
		buffer += nbsectoread*512; // Incremente the buffer for the next read
		sector += nbsectoread;	   // sector point to the next logical sector to read
		// reset the flash
		WriteByteCommand(0xFF);
	} while(numSecs > nbsecread);
	
	DisableRAM();
	return true;
}

/*-----------------------------------------------------------------
G6_WriteSectors
Write sectors in the flash
u32 sector IN : starting sector number
u8 numSecs IN : number of sector to write
u16* buffer IN : buffer containing the sectors
bool return OUT : true=ok false=ko
-----------------------------------------------------------------*/
bool G6_WriteSectors(u32 sector, u8 numSecs, u16* buffer)
{
	bool retval;
	u16 numsectowrite;
	int nbsecwrite;
	
	SelectNand();
	WriteProtectEnable();
	
	// 0 stand for 256
	if (numSecs!=0)
		numsectowrite = numSecs;
	else
		numsectowrite = 256;
	
	retval = 1;
	do {
		nbsecwrite = WriteBlockFunc(sector,numsectowrite,buffer);
		buffer += nbsecwrite*256;
		sector += nbsecwrite;
		if (nbsecwrite >= -0x100) {
			retval = 0;
			break;
		}
		numsectowrite -= nbsecwrite;
	} while(numsectowrite != 0);
	
	//  Why 2 times ?
	WriteProtectDisable();
	DisableRAM();
	
	return retval;
}
	
/*-----------------------------------------------------------------
G6_Shutdown
Last function to call
bool return OUT : true=ok false=ko
-----------------------------------------------------------------*/
bool G6_Shutdown()
{
	bool retval;
	
	// Why the hell do we have to do everything 2 times ??
	SelectNand();
	SelectNand();
	if (!ReadRBStatus())
		retval = false;
	else 
		retval = true;
	DisableRAM();
	DisableRAM();
	
	return retval;
}


IO_INTERFACE io_G6_int =
{
	DEVICE_TYPE_G6,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&G6_StartUp,
	(FN_MEDIUM_ISINSERTED)&G6_IsInserted,
	(FN_MEDIUM_READSECTORS)&G6_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&G6_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&G6_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&G6_Shutdown
};

/*-----------------------------------------------------------------
G6_GetInterface
Give the functions pointer to access the G6
LPIO_INTERFACE return OUT : structure of all functions
-----------------------------------------------------------------*/
LPIO_INTERFACE G6_GetInterface()
{
	return &io_G6_int;
}

