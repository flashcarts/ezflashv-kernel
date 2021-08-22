
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "_const.h"
#include "_console.h"

#include "memtool.h"

#include "directdisk.h"

#include "gba_nds_fat.h"
#include "disc_io.h"
#include "mediatype.h"
#include "filesys.h"
#include "shell.h"

extern LPIO_INTERFACE active_interface;

typedef struct {
  char *pFilename;
  u32 Sector,SectorMirror;
  u32 CartOffset;
} TFileInfo;

static TFileInfo FileInfo[EDDFile_MaxCount];

static EDDSaveType DDSaveType=EDDST_None;

enum ECartType {ECT_EEPROM,ECT_SRAM,ECT_Flash};

static ECartType CartType;

char DDFilename_resume[128];
char DDFilename_bookmrk0[128];
char DDFilename_bookmrk1[128];
char DDFilename_bookmrk2[128];
char DDFilename_bookmrk3[128];

// --------------------

//#define M3TESTCODE

#ifdef M3TESTCODE

// --- M3 stuff

static void SetM3_EnableCard(void) 
{
	// run unlock sequence
	volatile unsigned short tmp ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08E00002 ;
	tmp = *(volatile unsigned short *)0x0800000E ;
	tmp = *(volatile unsigned short *)0x08801FFC ;
	tmp = *(volatile unsigned short *)0x0800104A ;
	tmp = *(volatile unsigned short *)0x08800612 ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08801B66 ;
	tmp = *(volatile unsigned short *)0x08800006 ;
	tmp = *(volatile unsigned short *)0x08000000 ;
}

static void SetM3_EnablePSRAM(void) 
{
	// run unlock sequence
	volatile unsigned short tmp ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08E00002 ;
	tmp = *(volatile unsigned short *)0x0800000E ;
	tmp = *(volatile unsigned short *)0x08801FFC ;
	tmp = *(volatile unsigned short *)0x0800104A ;
	tmp = *(volatile unsigned short *)0x08800612 ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08801B66 ;
	tmp = *(volatile unsigned short *)0x08800004 ; // 0=bios?, 4 or c=8MByte?
	tmp = *(volatile unsigned short *)0x08000000 ;

        *(volatile u16*)0x09FFEFFE=0xAA55; // PepsiMan vote: The RAM on the M3 is made writable by writing 0xaa55 to 0x09ffeffe.

    // a=4; err=(n=0-7),(0x088nxxxx,0x089nxxxx,0x08anxxxx,0x08bnxxxx,0x08cnxxxx,0x08dnxxxx,0x08enxxxx,0x08fnxxxx)
}

#else

static void SetM3_EnableCard(void) 
{
  return;
}

static void SetM3_EnablePSRAM(void) 

{
  return;
}

#endif // M3TESTCODE

// --------------------

#define GBACartRAM ((vu8*)0x0a000000)

static inline u8 EEPROMr8(u32 adr)
{
  return(0xff);
}

static inline void EEPROMw8(u32 adr,u8 data)
{
}

bool CartCheck_EEPROM(void)
{
  u8 back[16];
  bool ef;
  
  // EEPROM内容を待避
  for(u32 idx=0;idx<16;idx++) back[idx]=EEPROMr8(idx);
  
  // テストデータ書き込み
  for(u32 idx=0;idx<16;idx++) EEPROMw8(idx,idx);
  
  // 書き込み内容チェック
  ef=true;
  for(u32 idx=0;idx<16;idx++){
    if(idx!=EEPROMr8(idx)) ef=false;
  }
  
  // EEPROM内容を復帰
  for(u32 idx=0;idx<16;idx++) EEPROMw8(idx,back[idx]);
  
  if(ef==true){
    _consolePrintf("CartCheck_EEPROM:ok.\n");
    }else{
    _consolePrintf("CartCheck_EEPROM:ng.\n");
  }
  
  return(ef);
}

static inline u8 SRAMr8(u32 adr)
{
  return(GBACartRAM[adr]);
}

static inline void SRAMw8(u32 adr,u8 data)
{
  GBACartRAM[adr]=data;
}

static bool CartCheck_SRAM(void)
{
  u8 back[16];
  bool ef;
  
  // SRAM内容を待避
  for(u32 idx=0;idx<16;idx++) back[idx]=SRAMr8(idx);
  
  // テストデータ書き込み
  for(u32 idx=0;idx<16;idx++) SRAMw8(idx,idx);
  
  // 書き込み内容チェック
  ef=true;
  for(u32 idx=0;idx<16;idx++){
    if(idx!=SRAMr8(idx)) ef=false;
  }
  
  // SRAM内容を復帰
  for(u32 idx=0;idx<16;idx++) SRAMw8(idx,back[idx]);
  
  if(ef==true){
    _consolePrintf("CartCheck_SRAM:ok.\n");
    }else{
    _consolePrintf("CartCheck_SRAM:ng.\n");
  }
  
  return(ef);
}

static inline u8 Flashr8(u32 adr)
{
  return(GBACartRAM[adr]);
}

static inline void Flashw8(u32 adr,u8 data)
{
  GBACartRAM[0x5555]=0xaa; // to CMD1
  GBACartRAM[0x2aaa]=0x55; // to CMD2
  GBACartRAM[0x5555]=0xa0; // to PROGRAM
  GBACartRAM[adr]=data;
}

static bool CartCheck_Flash(void)
{
  u8 back[16];
  bool ef;
  
  // Flash内容を待避
  for(u32 idx=0;idx<16;idx++) back[idx]=Flashr8(idx);
  
  // テストデータ書き込み
  for(u32 idx=0;idx<16;idx++) Flashw8(idx,idx);
  
  // 書き込み内容チェック
  ef=true;
  for(u32 idx=0;idx<16;idx++){
    if(idx!=Flashr8(idx)) ef=false;
  }
  
  // Flash内容を復帰
  for(u32 idx=0;idx<16;idx++) Flashw8(idx,back[idx]);
  
  if(ef==true){
    _consolePrintf("CartCheck_Flash:ok.\n");
    }else{
    _consolePrintf("CartCheck_Flash:ng.\n");
  }
  
  return(ef);
}

static void CartRead_AutoSelect(u32 offset,void *pbuf,u32 bufsize)
{
  SetM3_EnablePSRAM();
  u8 *pbuf8=(u8*)pbuf;
  switch(CartType){
    case ECT_EEPROM: {
      for(u32 idx=0;idx<bufsize;idx++) pbuf8[idx]=EEPROMr8(offset+idx);
    } break;
    case ECT_SRAM: {
      for(u32 idx=0;idx<bufsize;idx++) pbuf8[idx]=SRAMr8(offset+idx);
    } break;
    case ECT_Flash: {
      for(u32 idx=0;idx<bufsize;idx++) pbuf8[idx]=Flashr8(offset+idx);
    } break;
  }
  SetM3_EnableCard();
}

static void CartWrite_AutoSelect(u32 offset,void *pbuf,u32 bufsize)
{
  SetM3_EnablePSRAM();
  u8 *pbuf8=(u8*)pbuf;
  switch(CartType){
    case ECT_EEPROM: {
      for(u32 idx=0;idx<bufsize;idx++) EEPROMw8(offset+idx,pbuf8[idx]);
    } break;
    case ECT_SRAM: {
      for(u32 idx=0;idx<bufsize;idx++) SRAMw8(offset+idx,pbuf8[idx]);
    } break;
    case ECT_Flash: {
      for(u32 idx=0;idx<bufsize;idx++) Flashw8(offset+idx,pbuf8[idx]);
    } break;
  }
  SetM3_EnableCard();
}

// --------------------

#define CARTID "MoonShell SaveData for FlashCart\x00"
#define CARTID_SIZE (75)
#define CARTID_Store0ofs (0*512)
#define CARTID_Store1ofs (15*512)
#define CARTSIZE (16*512)

bool CartCheckData(void)
{
  char readid0[CARTID_SIZE],readid1[CARTID_SIZE];
  
  CartRead_AutoSelect(CARTID_Store0ofs,readid0,CARTID_SIZE);
  CartRead_AutoSelect(CARTID_Store1ofs,readid1,CARTID_SIZE);
  
  if(strncmp(CARTID,readid0,CARTID_SIZE)!=0) return(false);
  if(strncmp(CARTID,readid1,CARTID_SIZE)!=0) return(false);
  
  return(true);
}

void CartFormat(void)
{
  u8 *pbuf=(u8*)safemalloc(CARTSIZE);
  
  MemSet32CPU(0,pbuf,CARTSIZE);
  MemCopy8CPU(CARTID,&pbuf[CARTID_Store0ofs],CARTID_SIZE);
  MemCopy8CPU(CARTID,&pbuf[CARTID_Store1ofs],CARTID_SIZE);
  
  CartWrite_AutoSelect(0,pbuf,CARTSIZE);
  
  safefree(pbuf); pbuf=NULL;
}

// --------------------

void DD_Init(EDDSaveType st)
{
  TFileInfo fi={NULL,0,0,0};
  
  snprintf(DDFilename_resume,128,"%s/resume.sav",FATShellPath);
  snprintf(DDFilename_bookmrk0,128,"%s/bookmrk0.sav",FATShellPath);
  snprintf(DDFilename_bookmrk1,128,"%s/bookmrk1.sav",FATShellPath);
  snprintf(DDFilename_bookmrk2,128,"%s/bookmrk2.sav",FATShellPath);
  snprintf(DDFilename_bookmrk3,128,"%s/bookmrk3.sav",FATShellPath);
  
  fi.pFilename=DDFilename_resume;
  fi.CartOffset=1*512;
  FileInfo[EDDFile_resume]=fi;
  
  fi.pFilename=DDFilename_bookmrk0;
  fi.CartOffset=2*512;
  FileInfo[EDDFile_bookmrk0]=fi;
  
  fi.pFilename=DDFilename_bookmrk1;
  fi.CartOffset=3*512;
  FileInfo[EDDFile_bookmrk1]=fi;
  
  fi.pFilename=DDFilename_bookmrk2;
  fi.CartOffset=4*512;
  FileInfo[EDDFile_bookmrk2]=fi;
  
  fi.pFilename=DDFilename_bookmrk3;
  fi.CartOffset=5*512;
  FileInfo[EDDFile_bookmrk3]=fi;
  
  DDSaveType=st;
  
  switch(DDSaveType){
    case EDDST_None: {
    } break;
    case EDDST_FAT: {
      if(DIMediaType==DIMT_NONE) DDSaveType=EDDST_None;
      if(active_interface==NULL) DDSaveType=EDDST_None;
    } break;
    case EDDST_CART: {
      SetM3_EnablePSRAM();
      if(CartCheck_EEPROM()==true){
        CartType=ECT_EEPROM;
        }else{
        if(CartCheck_SRAM()==true){
          CartType=ECT_SRAM;
          }else{
          if(CartCheck_Flash()==true){
            CartType=ECT_Flash;
            }else{
            DDSaveType=EDDST_None;
          }
        }
      }
      SetM3_EnableCard();
    } break;
  }
  
  if(DDSaveType==EDDST_CART){
    if(CartCheckData()==false) CartFormat();
  }
}

EDDSaveType DD_GetSaveType(void)
{
  return(DDSaveType);
}

static bool DD_InitFile_FAT(TFileInfo *pfi)
{
  if((active_interface->ul_Features & FEATURE_MEDIUM_CANWRITE)==0){
    _consolePrintf("\nnot support disk write function.\nAdapter = %s\n",DIMediaName);
    return(false);
  }
  
  FAT_FILE *fp=FAT_fopen(pfi->pFilename,"r");
  if(fp==NULL){
    _consolePrintf("\nfile not found. [%s]\n",pfi->pFilename);
    return(false);
  }
  
  u32 clus=fp->curClus;
  u32 Sector=FAT_ClustToSect_extern(clus);
  _consolePrintf("Cluster=0x%08x\n",clus);
  _consolePrintf("Sector=0x%08x\n",Sector);
  
  if(Sector==0){
    _consolePrintf("\nSector is null.\n");
    return(false);
  }
  
  FAT_fseek(fp,0,SEEK_END);
  u32 fsize=FAT_ftell(fp);
  FAT_fseek(fp,0,SEEK_SET);
  
  if(fsize!=DD_SectorSize){
    _consolePrintf("\nfile size is not %dbyte.\n",DD_SectorSize);
    return(false);
  }
  
  u8 *pfbuf0=(u8*)safemalloc(DD_SectorSize);
  u8 *pfbuf1=(u8*)safemalloc(DD_SectorSize);
  
  {
    _consolePrintf("Test: direct disk reading.");
    
    // read source
    active_interface->fn_ReadSectors(Sector,1,pfbuf0);
    FAT_fread(pfbuf1,1,DD_SectorSize,fp);
    
    // verify
    for(u32 idx=0;idx<DD_SectorSize;idx++){
      if(pfbuf0[idx]!=pfbuf1[idx]){
        _consolePrintf(" failed.\nposition = (%d,0x%02x!=0x%02x)\n",idx,pfbuf0[idx],pfbuf1[idx]);
        return(false);
      }
    }
    _consolePrintf("\n");
  }
  
  {
    _consolePrintf("Test: direct disk writing.");
    
    // write dummy
    for(u32 idx=0;idx<DD_SectorSize;idx++){
      pfbuf0[idx]=(idx-126)&0xff;
    }
    active_interface->fn_WriteSectors(Sector,1,pfbuf0);
    
    // read dummy
//    for(vu32 idx=0;idx<0x10000;idx++);
    for(u32 idx=0;idx<DD_SectorSize;idx++){
      pfbuf0[idx]=0xa5;
    }
    active_interface->fn_ReadSectors(Sector,1,pfbuf0);
    
    // verify
    for(u32 idx=0;idx<DD_SectorSize;idx++){
      if(pfbuf0[idx]!=((idx-126)&0xff)){
        _consolePrintf(" failed.\nposition = (%d,0x%02x!=0x%02x)\n",idx,pfbuf0[idx],(idx-126)&0xff);
        return(false);
      }
    }
    
    // restore
    active_interface->fn_WriteSectors(Sector,1,pfbuf1);
    
    _consolePrintf("\n");
  }
  
  safefree(pfbuf0);
  safefree(pfbuf1);
  
  FAT_fclose(fp);
  
  pfi->Sector=Sector;
  pfi->SectorMirror=Sector;
  
  return(true);
}

static bool DD_InitFile_CART(TFileInfo *pfi)
{
  return(true);
}

bool DD_isEnabled(void)
{
  bool res=false;
  
  switch(DDSaveType){
    case EDDST_None: res=false; break;
    case EDDST_FAT: {
      if((active_interface->ul_Features & FEATURE_MEDIUM_CANWRITE)!=0) res=true;
    	return true;
    } break;
    case EDDST_CART: res=true; break;
  }
  
  return(res);
}

bool DD_InitFile(EDDFile DDFile)
{
  vu16 ime=REG_IME;
  REG_IME=0;
  
  TFileInfo *pfi=&FileInfo[DDFile];
  
  pfi->Sector=0;
  pfi->SectorMirror=0;
  
  bool res=false;
  
  switch(DDSaveType){
    case EDDST_None: res=false; break;
    case EDDST_FAT: res=DD_InitFile_FAT(pfi); break;
    case EDDST_CART: res=DD_InitFile_CART(pfi); break;
  }
  
  REG_IME=ime;
  
  return(res);
}

// --------------------

static void fatinfochk(EDDFile DDFile)
{
  TFileInfo *pfi=&FileInfo[DDFile];
  
  if((pfi->Sector==0)||(pfi->Sector!=pfi->SectorMirror)){
    _consolePrintf("Sector(%d)!=SectorMirror(%d)\n",pfi->Sector,pfi->SectorMirror);
    _consolePrintf("Critical error!! CPU HALT for safety.\n");
    ShowLogHalt();
    while(1);
  }
}

static void FAT_ReadSector(EDDFile DDFile,void *pbuf,u32 bufsize)
{
  fatinfochk(DDFile);
//  for(vu32 idx=0;idx<0x10000;idx++);
  active_interface->fn_ReadSectors(FileInfo[DDFile].Sector,1,pbuf);
//  for(vu32 idx=0;idx<0x10000;idx++);
}

static void FAT_WriteSector(EDDFile DDFile,void *pbuf,u32 bufsize)
{
  fatinfochk(DDFile);
//  for(vu32 idx=0;idx<0x10000;idx++);
  active_interface->fn_WriteSectors(FileInfo[DDFile].Sector,1,pbuf);
//  for(vu32 idx=0;idx<0x10000;idx++);
}

// --------------------

void DD_ReadFile(EDDFile DDFile,void *pbuf,u32 bufsize)
{
  vu16 ime=REG_IME;
  REG_IME=0;
  
  if(((int)DDFile<0)||((int)EDDFile_MaxCount<=(int)DDFile)){
    _consolePrintf("request error DDFile=%d\n",(int)DDFile);
    ShowLogHalt();
    while(1);
  }
  
  if(pbuf==NULL){
    _consolePrintf("DD_xSector(%d,0x%x,%d); pbuf=NULL.\n",DDFile,pbuf,bufsize);
    ShowLogHalt();
    while(1);
  }
  
  if(bufsize!=DD_SectorSize){
    _consolePrintf("DD_xSector(%d,0x%x,%d); Illigal bufsize error.\n",DDFile,pbuf,bufsize);
    ShowLogHalt();
    while(1);
  }
  
  switch(DDSaveType){
    case EDDST_None: break;
    case EDDST_FAT: FAT_ReadSector(DDFile,pbuf,bufsize); break;
    case EDDST_CART: CartRead_AutoSelect(FileInfo[DDFile].CartOffset,pbuf,bufsize); break;
  }
  
  REG_IME=ime;
}

void DD_WriteFile(EDDFile DDFile,void *pbuf,u32 bufsize)
{
  vu16 ime=REG_IME;
  REG_IME=0;
  
  if(((int)DDFile<0)||((int)EDDFile_MaxCount<=(int)DDFile)){
    _consolePrintf("request error DDFile=%d\n",(int)DDFile);
    ShowLogHalt();
    while(1);
  }
  
  if(pbuf==NULL){
    _consolePrintf("DD_xSector(%d,0x%x,%d); pbuf=NULL.\n",DDFile,pbuf,bufsize);
    ShowLogHalt();
    while(1);
  }
  
  if(bufsize!=DD_SectorSize){
    _consolePrintf("DD_xSector(%d,0x%x,%d); Illigal bufsize error.\n",DDFile,pbuf,bufsize);
    ShowLogHalt();
    while(1);
  }
  
  switch(DDSaveType){
    case EDDST_None: break;
    case EDDST_FAT: FAT_WriteSector(DDFile,pbuf,bufsize); break;
    case EDDST_CART: CartWrite_AutoSelect(FileInfo[DDFile].CartOffset,pbuf,bufsize); break;
  }
  
  REG_IME=ime;
}

