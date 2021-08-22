
static void boot_GBAMP(u32 FileIndex)
{
  u32 FileCluster=FileSys_GetFileCluster(FileIndex);
  
  if(FileCluster==0){
    _consolePrintf("GetFileCluster return=0 broken FAT?\n");
    ShowLogHalt();
    return;
  }
  
  videoSetModeSub_SetShowLog(true);
  
//  _consolePrintf("DebugMode. MultiBoot Stop. Halt.\n"); while(1);
  
  _consolePrintf("start NDSROM.\n[%s/%s]\n\nPlease wait for a while.\n",FileSys_GetPathName(),FileSys_GetAlias(FileIndex));
  
  {
    u16 KEYS_CUR;
    
    KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
    while(KEYS_CUR!=0){
      KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
    }
  }
  
// base source writed by SaTa. (include 2005/09/27 Moonlight.)
// **********************************************************************
// Use this code to start an NDS file
	REG_IME = IME_DISABLE;	// Disable interrupts
	WAIT_CR |= (0x8080);  // ARM7 has access to GBA cart
	*((vu32*)0x027FFFFC) = FileCluster;  // Start cluster of NDS to load
	*((vu32*)0x027FFE04) = (u32)0xE59FF018;  // ldr pc, 0x027FFE24
	*((vu32*)0x027FFE24) = (u32)0x027FFE04;  // Set ARM9 Loop address
    IPC3->RESET=RESET_GBAMP;
	swiSoftReset();  // Reset
// **********************************************************************

  return; // åƒÇ—èoÇµå≥Ç…ÇÕãAÇÁÇ»Ç¢ÅB
}

static void (*lp_boot_GBAMP)(u32 FileIndex)=boot_GBAMP;

