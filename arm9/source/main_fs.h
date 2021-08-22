
static s32 FS_TopIndex;
static s32 FS_CursorIndex;
static s32 FS_IndexCount;
static s32 FS_PlayIndex;
static s32 FS_DispCount;
static bool FS_RedrawFlag[32]; // overspec.
static bool FS_ExculsiveAllRedrawFlag;

static int FileHandle=0;
static int FileHandle1=0,FileHandle2=0;

char forResume_PlayPathName[forResume_PlayPathNameLen];
char forResume_PlayFileAliasName[forResume_PlayFileAliasNameLen];
UnicodeChar forResume_PlayFileUnicodeName[forResume_PlayFileUnicodeNameLen];

static void FS_Redraw_SetAll(void)
{ cwl();
  for(u32 idx=0;idx<(u32)FS_DispCount;idx++){ cwl();
    FS_RedrawFlag[idx]=true;
  }
  FS_ExculsiveAllRedrawFlag=true;
}

static void FS_Redraw_SetPlayIndex(void)
{ cwl();
  if(FS_PlayIndex==-1) return;
  
  s32 idx=FS_PlayIndex-FS_TopIndex;
  if((idx<0)||(FS_DispCount<=idx)) return;
  FS_RedrawFlag[idx]=true;
}

static void FS_Redraw_SetCursorIndex(void)
{ cwl();
  if(FS_CursorIndex==-1) return;
  
  s32 idx=FS_CursorIndex-FS_TopIndex;
  if((idx<0)||(FS_DispCount<=idx)) return;
  FS_RedrawFlag[idx]=true;
}

static void FS_Redraw_SetTopIndex(void)
{ cwl();
  FS_Redraw_SetAll();
}

void FS_ChangePath(char *TargetPathName)
{ cwl();
  FileSys_ChangePath(TargetPathName);
  
  thumbChangePath();
  if(thumbExists()==true){
    FS_DispCount=9;
    }else{
    FS_DispCount=MWin_GetClientHeight(WM_FileSelect)/FontProHeight;
  }
  
  FS_TopIndex=0;
  FS_CursorIndex=0;
  FS_IndexCount=FileSys_GetFileCount();
  FS_PlayIndex=-1;
  FS_Redraw_SetAll();
  
  { cwl();
    s32 lastidx=FileSys_GetLastPathItemIndex();
    if(lastidx!=-1){ cwl();
      FS_SetCursor(lastidx);
    }
  }
  
  s32 idxcnt=FS_IndexCount;
  if(thumbExists()==true) idxcnt=(idxcnt+2)/3*3;
  MWin_SetSBarV(WM_FileSelect,idxcnt,FS_DispCount,FS_TopIndex);
  
  { cwl();
    char str[256];
    sprintf(str,"%s %dfiles.",FileSys_GetPathName(),FileSys_GetPureFileCount());
    MWin_SetWindowTitle(WM_FileSelect,str);
  }
  
  MWin_RefreshWindow(WM_FileSelect);
  MWin_TransWindow(WM_FileSelect);
}

static void FS_ExecutePath(u32 FileIndex)
{ cwl();
  char *TargetPathName=FileSys_GetAlias(FileIndex);
  
  FS_ChangePath(TargetPathName);
}

void FS_ExecuteStop(void)
{ cwl();
  thumbReturnImage();
  
  if(ExecMode==EM_Text){
    Bookmark_CurrentResumeBackup();
    Bookmark_Save();
    Bookmark_End();
  }
  
  if(ExecMode!=EM_None) LastExecMode=ExecMode;
  ExecMode=EM_None;
  
  MWin_PlayControl_SetWindowTitleW(NULL);
  
  strpcmStop();
  
  if(pPluginBody!=NULL){
    DLLList_FreePlugin(pPluginBody);
    safefree(pPluginBody); pPluginBody=NULL;
  }
  
  FreeFPK();
  FreeText();
  FreeDPG();
  FreeGMENSF();
  FreeGMEGBS();
  
  if(FileHandle!=0){ cwl();
    FileSys_fclose(FileHandle);
    FileHandle=0;
  }
  
  if(FileHandle1!=0){ cwl();
    FileSys_fclose(FileHandle1);
    FileHandle1=0;
  }
  
  if(FileHandle2!=0){ cwl();
    FileSys_fclose(FileHandle2);
    FileHandle2=0;
  }
  
  bool RefreshFlag=false;
  
  if(MWin_GetVisible(WM_PlayControl)==true){ cwl();
    MWin_SetVisible(WM_PlayControl,false);
    RefreshFlag=true;
  }
  MWin_FreeMem(WM_PlayControl);
  
  PlugInfo_InitCustomMessage(false);
  if(MWin_GetVisible(WM_FileInfo)==true){ cwl();
    MWin_SetVisible(WM_FileInfo,false);
    RefreshFlag=true;
  }
  MWin_FreeMem(WM_FileInfo);
  
  if(MWin_GetVisible(WM_PicView)==true){ cwl();
    MWin_SetVisible(WM_PicView,false);
    RefreshFlag=true;
  }
  MWin_FreeMem(WM_PicView);
  
  if(MWin_GetVisible(WM_TextView)==true){ cwl();
    MWin_SetVisible(WM_TextView,false);
    RefreshFlag=true;
  }
  MWin_FreeMem(WM_TextView);
  
  if(RefreshFlag==true){ cwl();
    MWin_SetActiveTopMost();
    MWin_RefreshScreenMask();
    MWin_AllRefresh();
  }
  
  switch(LastExecMode){
    case EM_DPG: case EM_None: case EM_MP3Boot: case EM_NDSROM: case EM_FPK: case EM_MSPImage: case EM_Text: {
      MWin_SetVideoWideFlag(false);
      MWin_SetVideoOverlaySize(ScreenWidth,ScreenHeight,0);
      MWin_SetVideoOverlay(false);
      MWin_RefreshVideoFullScreen(false);
      
      if(GlobalINI.System.FileSelectSubScreen==false){
        // exclusive reset
        BacklightOffVsyncCount=1;
        PluginBodyClock_Execute=true;
        PluginBodyClock_RequestRefresh=true;
      }
//      videoSetModeSub_SetShowLog(true);
    } break;
    case EM_MSPSound: case EM_GMENSF: case EM_GMEGBS: { // case EM_GMEVGM: case EM_GMEGYM: {
    } break;
  }
  
  PrintFreeMem();
}

static bool FS_FileOpen(u32 FileIndex)
{ cwl();
  FileHandle=FileSys_fopen(FileIndex);
  
  if(FileHandle==0) return(false);
  
  return(true);
}

static bool FS_FileOpen2(u32 FileIndex)
{ cwl();
  FileHandle1=FileSys_fopen(FileIndex);
  
  if(FileHandle1==0) return(false);
  
  FileHandle2=FileSys_fopen(FileIndex);
  
  if(FileHandle2==0) return(false);
  
  return(true);
}

static EExecMode FS_GetExecMode(u32 FileIndex)
{ cwl();
  if(FileSys_GetFileType(FileIndex)!=FT_File) return(EM_None);
  
  char ext[256];
  
  FileSys_GetFileExt(FileIndex,ext);
  
  if(ext[0]==0) return(EM_None);
  
#ifdef USEGME
  if(isStrEqual(ext,".NSF")==true) return(EM_GMENSF);
  if(isStrEqual(ext,".GBS")==true) return(EM_GMEGBS);
//  if(isStrEqual(ext,".VGM")==true) return(EM_GMEVGM);
//  if(isStrEqual(ext,".GYM")==true) return(EM_GMEGYM);
#endif
  
  {
    char fn[PluginFilenameMax];
    
    EPluginType EPT=DLLList_GetPluginFilename(ext,fn);
    
    switch(EPT){
      case EPT_None: break;
      case EPT_Image: return(EM_MSPImage); break;
      case EPT_Sound: return(EM_MSPSound); break;
      case EPT_Clock: {
        _consolePrintf("Error:Clock plugin is not execute for file.\n");
      } break;
    }
  }
  
#ifdef USENDSROM
  if(isStrEqual(ext,".NDS")==true) return(EM_NDSROM);
#endif
  
  if(isStrEqual(ext,".FPK")==true) return(EM_FPK);
  
  if((isStrEqual(ext,".TXT")==true)||(isStrEqual(ext,".DOC")==true)||(isStrEqual(ext,".INI")==true)) return(EM_Text);
  if((isStrEqual(ext,".C")==true)||(isStrEqual(ext,".CPP")==true)||(isStrEqual(ext,".H")==true)) return(EM_Text);
  if((isStrEqual(ext,".MML")==true)||(isStrEqual(ext,".FRM")==true)) return(EM_Text);
  
#ifdef USEDPG
  if(isStrEqual(ext,".DPG")==true) return(EM_DPG);
#endif
  
  return(EM_None);
}

static void FS_ExecuteFile_SetWindowTitle(u32 WinIndex,u32 FileIndex,char *WindowTitle)
{ cwl();
  char ttlbuf[512];
  
  sprintf(ttlbuf,"%s %s",FileSys_GetAlias(FileIndex),WindowTitle);
  MWin_SetWindowTitle(WinIndex,ttlbuf);
  
  MWin_PlayControl_SetWindowTitleW(FileSys_GetFilename(FileIndex));
}

static void FS_ExecuteFile_InitPlayControlWindow(u32 FileIndex)
{ cwl();
  FS_ExecuteFile_SetWindowTitle(WM_PlayControl,FileIndex,"PlayControl...");
  
  MWin_SetVisible(WM_PlayControl,true);
  u32 WinIndex=MWin_SetActive(WM_PlayControl);
  MWin_RefreshScreenMask();
  if(WinIndex!=(u32)-1) MWin_RefreshWindow(WinIndex);
  MWin_RefreshWindow(WM_PlayControl);
}

static void FS_ExecuteFile_InitFileInfoWindow(void)
{ cwl();
  if(PlugInfo_GetShowFlag()==false) return;
  
  TWinBody *pwb=MWin_GetWinBody(WM_FileInfo);
  if(pwb==NULL) return;
  
  MWin_SetSBarV(WM_FileInfo,PlugInfo_GetInfoIndexCount(),pwb->ClientRect.h/FontProHeight,0);
  
  MWin_SetVisible(WM_FileInfo,true);
  u32 WinIndex=MWin_SetActive(WM_FileInfo);
  MWin_RefreshScreenMask();
  if(WinIndex!=(u32)-1) MWin_RefreshWindow(WinIndex);
  MWin_RefreshWindow(WM_FileInfo);
}

static void FS_ExecuteFile_PicView_DrawFileInfo(void)
{
  TWinBody *pwb=MWin_GetWinBody(WM_PicView);
  if(pwb==NULL) return;
  
  u16 *pBuf=&pwb->WinBuf[0];
  s32 BufWidth=pwb->Rect.w;
  
  {
    for(int y=0;y<pwb->ClientRect.h;y++){
      MemSet16DMA3(0,&pBuf[pwb->ClientRect.x+((pwb->ClientRect.y+y)*BufWidth)],pwb->ClientRect.w*2);
    }
  }
  
  if(GlobalINI.ImagePlugin.DrawFileInfo==false) return;
  
  if(PlugInfo_GetShowFlag()==false) return;
  
  int InfoCount=PlugInfo_GetInfoIndexCount();
  int LineCount=(pwb->ClientRect.h/FontProHeight)-1;
  int TopIndex=LineCount-InfoCount;
  
  if(TopIndex<0) TopIndex=0;
  
  u16 colp=RGB15(31,31,31)|BIT15;
  u16 coln=RGB15(0,0,0)|BIT15;
  
  s32 wLimit=pwb->ClientRect.x+pwb->ClientRect.w;
  u32 ofsx=pwb->ClientRect.x+4;
  u32 ofsy=pwb->ClientRect.y+0;
  
  for(int idx=0;idx<InfoCount;idx++){
    int drawidx=TopIndex+idx;
    if(drawidx<LineCount){
      {
        char str[256]={0,};
        if(PlugInfo_GetInfoStrL(idx,str,256)==true){
          VRAM_WriteStringRectL(pBuf,BufWidth,wLimit,ofsx+1,ofsy+(drawidx*FontProHeight)+1,str,coln);
          VRAM_WriteStringRectL(pBuf,BufWidth,wLimit,ofsx,ofsy+(drawidx*FontProHeight),str,colp);
        }
      }
      {
        UnicodeChar str[256]={0,};
        if(PlugInfo_GetInfoStrW(idx,str,256)==true){
          VRAM_WriteStringRectW(pBuf,BufWidth,wLimit,ofsx+1,ofsy+(drawidx*FontProHeight)+1,str,coln);
          VRAM_WriteStringRectW(pBuf,BufWidth,wLimit,ofsx,ofsy+(drawidx*FontProHeight),str,colp);
        }
      }
      {
        char str[256*2]={0,};
        if(PlugInfo_GetInfoStrUTF8(idx,str,256)==true){
          VRAM_WriteStringRectUTF8(pBuf,BufWidth,wLimit,ofsx+1,ofsy+(drawidx*FontProHeight)+1,str,coln);
          VRAM_WriteStringRectUTF8(pBuf,BufWidth,wLimit,ofsx,ofsy+(drawidx*FontProHeight),str,colp);
        }
      }
    }
  }
}

static void FS_ExecuteFile_LoadFail(void)
{
  ExecMode=EM_None;
  LastExecMode=EM_None;
}

static EstrpcmFormat FS_ExecuteFile_GetOversamplingFactorFromSampleRate(u32 SampleRate)
{
  _consolePrintf("SampleRate=%dHz\n",SampleRate);
  
  EstrpcmFormat SPF;
  
  if(SampleRate==(32768/4)){
    _consolePrintf("Setup oversampling x4 PCM.\n");
    SPF=SPF_PCMx4;
    }else{
    if(SampleRate==(32768/2)){
      _consolePrintf("Setup oversampling x2 PCM.\n");
      SPF=SPF_PCMx2;
      }else{
      if(SampleRate==(32768/1)){
        _consolePrintf("Setup oversampling x1 PCM.\n");
        SPF=SPF_PCMx1;
        }else{
        if(SampleRate<=48000){
          SPF=SPF_PCMx4;
          _consolePrintf("Setup oversampling x4 PCM.\n");
          }else{
          if(SampleRate<=96000){
            SPF=SPF_PCMx2;
            _consolePrintf("Setup oversampling x2 PCM.\n");
            }else{
            SPF=SPF_PCMx1;
            _consolePrintf("Setup oversampling x1 PCM.\n");
          }
        }
      }
    }
  }
  
  return(SPF);
}
 void FS_DrawFileDetail(u32 FileIndex,u8 savetype)
{
	cwl();

	FS_ExecuteStop();
	EExecMode mode=FS_GetExecMode(FileIndex);
	if(mode==EM_MSPSound){ cwl();
		//MWin_ClearDesktop();
		 u16 col=MWC_ClientText;
		extern TSkinBM SkinBM;
		const TSkinBM *psbm=&SkinBM;
		u16 *pDstBuf;
		 u16 *pDesktopBuf;
		 u16 xOff=64;
		 u16 yOff=136;
		 u16 *pVRAMBuf=pScreenMainOverlay->GetVRAMBuf();
		 pDstBuf=&pVRAMBuf[0];
		  pDesktopBuf=&psbm->ppDesktopBnBM[0];
		  pDstBuf+=28672;
		  MemCopy16DMA3(pDesktopBuf,pDstBuf,40960);//40960 = 256*80*2
		  pDstBuf=&pVRAMBuf[0];
		  pDstBuf+=256*yOff+xOff;
		  pDesktopBuf=&psbm->pMp3Icon[0];
		  for(s32 y=0;y<32;y++){ cwl();
		  	for(s32 x=0;x<32;x++){ cwl();
			   pDstBuf[x] =pDesktopBuf[x];
		  	}
			pDstBuf+=256;
			pDesktopBuf+=32;
		  }
		  
		 pDstBuf=&pVRAMBuf[0];
		 char fn[PluginFilenameMax];
		 char ext[256];
		 FileSys_GetFileExt(FileIndex,ext);
		 DLLList_GetPluginFilename(ext,fn);
		 pPluginBody=DLLList_LoadPlugin(fn);
		 if(FS_FileOpen(FileIndex)==false) return;
		    if(pPluginBody->pSL->Start(FileHandle)==false){ cwl();
		      return;
		    }
		 for(s32 y=0;y<3;y++)
		 {
		 	cwl();
		    	s32 idx=y;
		    
		      	char str[256]={0,};
				char *p=&str[0];
			if(pPluginBody->pSL->GetInfoStrL(idx,str,256)==true)
			{
				do
		
				{
					p++;
			  	}while(*p!=':');
				p++;
       				 VRAM_WriteStringRectL(pDstBuf,256,256,xOff+32,(y*FontProHeight)+yOff,p,col);
      			}
		      
		   }
//		FS_FileClose(FileIndex);
	/*
	    {
			
	      char fn[PluginFilenameMax];
	      char ext[256];
	      
	      FileSys_GetFileExt(FileIndex,ext);
	      DLLList_GetPluginFilename(ext,fn);
	      pPluginBody=DLLList_LoadPlugin(fn);
	    }
	    if(FS_FileOpen(FileIndex)==false) return;
	    if(pPluginBody->pSL->Start(FileHandle)==false){ cwl();
	      return;
	    }
		//MWin_ShowFileDetail(64,136);
		 cwl();
		  u16 *pDstBuf;
		  u16 *pDesktopBuf;
		  u16 xOff=64;
		  u16 yOff=136;
		  
		  u16 *pVRAMBuf=pScreenMainOverlay->GetVRAMBuf();
		  
		 extern TSkinBM SkinBM;
		  const TSkinBM *psbm=&SkinBM;
		  
		  pDstBuf=&pVRAMBuf[0];
		  pDesktopBuf=&psbm->ppDesktopBnBM[0];
		  
		  pDstBuf+=28672;
		  MemCopy16DMA3(pDesktopBuf,pDstBuf,40960);//40960 = 256*80*2
		  pDstBuf=&pVRAMBuf[0];
		  pDstBuf+=256*yOff+xOff;
		  pDesktopBuf=&psbm->pMp3Icon[0];
		  for(s32 y=0;y<32;y++){ cwl();
		  	for(s32 x=0;x<32;x++){ cwl();
			   pDstBuf[x] =pDesktopBuf[x];
		  	}
			pDstBuf+=256;
			pDesktopBuf+=32;
		  }
			pDstBuf=&pVRAMBuf[0];
		   for(s32 y=0;y<3;y++){ cwl();
		    s32 idx=y;
		    
		      char str[256]={0,};
			  char *p=&str[0];
		      if(pPluginBody->pSL->GetInfoStrL(idx,str,256)==true){
			  	do
		
				{
					p++;
			  	}while(*p!='=');
				p++;
		        VRAM_WriteStringRectL(pDstBuf,256,256,xOff+32,(y*FontProHeight)+yOff,p,MWC_TitleD_Text);
		      }
		   }
		   */
	    return;
	}
	if(mode==EM_NDSROM){
		 cwl();
		 char p[20];
		 char *ptemp;
		 s32 ii;
		 uint32 *bm=(uint32*)safemalloc(32*32/2);
	          uint16 *pal=(uint16*)safemalloc(0x10*2);	
		 uint16  *pName=(uint16*)safemalloc(0x100);	
		 uint32 off=0;
		 
		 int fh=FileSys_fopen(FileIndex);
		 FileSys_fseek(fh,0,SEEK_SET);
		 FileSys_fread((uint8 *)bm,0x200,1,fh);
		 ptemp = (char *)bm;
		 for(ii=0;ii<20;ii++)
		  	p[ii]=ptemp[ii];
		 off = bm[0x1A];
		 if(off==0)
		 {
		 	 FileSys_fclose(fh);
		 	 free(bm); bm=NULL;
			 free(pal);pal=NULL;
			 free(pName);pName=NULL;
			 return;
		 }
		 FileSys_fseek(fh,off+0x20,SEEK_SET);
		 FileSys_fread((uint8 *)bm,0x200,1,fh);
 		 FileSys_fseek(fh,off+0x220,SEEK_SET);
		 FileSys_fread((uint8 *)pal,0x20,1,fh); 
		 FileSys_fseek(fh,off+0x340,SEEK_SET);
		 FileSys_fread((uint8 *)pName,0x100,1,fh);
		 FileSys_fclose(fh);
		 
		 for(uint32 i=0;i<0x80;i++)
		 {
		 	if(pName[i]==0x0A)
		 	{
				pName[i]=0;
				break;
			}
		 }
		 //²éÕÒ´æµµÄÚÐÍ
		  gl_SaveTypeOff=0;
		  for(ii=0;ii<gl_SizeTypeFileSize;ii+=20)
		  {
			if(memcmp(p,&gl_pSaveTypeBuf[ii],16)==0)
			{
				gl_SaveType=gl_pSaveTypeBuf[ii+16];
				gl_SaveTypeOff = ii+16;
				break;
			}
		  }
		  if(savetype==0x55)
		  {

		  }
		  else
		  {
		  	gl_SaveType=savetype;
			
			if(ii>=gl_SizeTypeFileSize)
			{
				u8 *ppTemp = (u8*)safemalloc(gl_SizeTypeFileSize+20);
				memcpy(ppTemp,gl_pSaveTypeBuf,gl_SizeTypeFileSize);
				for(ii=0;ii<16;ii++)
					ppTemp[gl_SizeTypeFileSize+ii] = p[ii];
				ppTemp[gl_SizeTypeFileSize+17]=0;
				ppTemp[gl_SizeTypeFileSize+18]=0;
				ppTemp[gl_SizeTypeFileSize+19]=0;
				free(gl_pSaveTypeBuf);
				gl_pSaveTypeBuf=ppTemp;
				gl_SaveTypeOff=gl_SizeTypeFileSize+16;
				gl_SizeTypeFileSize+=20;
			}
		  }
		 // _consolePrintf("ii=%d\n,gl_SaveType=%d\n",ii,gl_SaveType);
		  //²éÕÒ´æµµÄÚÐÍ
		  
		 MWin_ShowNdsRomIcon(bm,pal,64,136,pName);
		 free(bm); bm=NULL;
		 free(pal);pal=NULL;
		 free(pName);pName=NULL;
		 
		 return;
	}
	else
	{

		//MWin_ShowDesktop();
		MWin_ClearDesktop();
	}
}
#include "dsCard.h"

static void FS_ExecuteFile(u32 FileIndex)
{ cwl();
  
#ifdef ExecuteShowDebug
  videoSetModeSub(MODE_0_2D | DISPLAY_BG3_ACTIVE);
#endif
  
  _consoleClear();
  _consolePrintSet(0,2);
  
  FS_ExecuteStop();
  
  PrintFreeMem();
  
  {
    strncpy(forResume_PlayPathName,FileSys_GetPathName(),forResume_PlayPathNameLen);
    strncpy(forResume_PlayFileAliasName,FileSys_GetAlias(FileIndex),forResume_PlayFileAliasNameLen);
    Unicode_StrCopy(FileSys_GetFilename(FileIndex),forResume_PlayFileUnicodeName);
  }
  
  ExecMode=FS_GetExecMode(FileIndex);
  
  if(ExecMode==EM_None){ cwl();
    FS_ExecuteFile_LoadFail();
    
    char ext[256];
    
    FileSys_GetFileExt(FileIndex,ext);
    PlugInfo_InitCustomMessage(true);
    if(DefaultCodePage!=CP932){
      snprintf(PlugInfo_CustomMessage0,64,"not support ext%s",ext);
      }else{
      snprintf(PlugInfo_CustomMessage0,64,"[%s]ã¯æœªå¯¾å¿œã®æ‹¡å¼µå­ã§ã™ã€‚",ext);
      snprintf(PlugInfo_CustomMessage1,64,"å¯¾å¿œã™ã‚‹MSPãƒ—ãƒ©ã‚°ã‚¤ãƒ³ãŒæ­£å¸¸ã«");
      snprintf(PlugInfo_CustomMessage2,64,"ã‚¤ãƒ³ã‚¹ãƒˆãƒ¼ãƒ«ã•ã‚Œã¦ã„ã‚‹ã‹ç¢ºèªã—ã¦ãã ã•ã„ã€‚");
    }
    FS_ExecuteFile_InitFileInfoWindow();
    return;
  }
  
#ifdef USENDSROM
 
  
  if(ExecMode==EM_NDSROM){ cwl();
   
	const char *pathname=NULL;
	
	char bbuf[512] __attribute__ ((aligned(4)));
	char longname[256];
	u16 len;
	//u8 *pFlash = NULL;
	char pSave[20];
	u32 ii;
	char pRead[20];
	char str[512];
	char curpath[256];
	
	pathname=FileSys_GetAlias(FileIndex);
	char *pPath = FileSys_GetPathName();
	FileSys_fGetLongFileName(FileIndex,longname);
	len = strlen(longname);
	u8 *pFlash = (u8*)safemalloc(0x2000);
	dsCardi_SetRompage(0);
	for(s16 ii=0;ii<0x2000;ii+=0x200)
	  dsCardi_ReadCard(ROM_OFF+ii,&pFlash[ii]);	
	//_consolePrintf("1---%d %d %d\n\n",pFlash[0],pFlash[1],*((u16 *)&pFlash[2]));
//	for(ii=0;ii<0x2000;ii++)
		//pFlash[ii] = 0;
	*pFlash = gl_language;
	*(pFlash+1) = gl_speedAuto;
	*((u16 *)(pFlash+2))=gl_speed;
	for(ii=0;ii<len;ii++)
		pFlash[256+ii] = longname[ii];
	pFlash[256+len-3]='s';
	pFlash[256+len-2]='a';
	pFlash[256+len-1]='v';
	pFlash[256+len]='\0';	
	//_consolePrintf("2---%d %d %d\n\n",pFlash[0],pFlash[1],*((u16 *)&pFlash[2]));
	/**********¿ªÊ¼¼ÓÔØ´æµµÎÄ¼þ****************/
	u8 pSaveData[512];
	s32 LenSaveFile;
	FAT_FILE *h=NULL;
	FAT_InitFiles();
	sprintf(str,"%s%s","/save/",(char *)&pFlash[256]);
	_consolePrintf("%s\n",str);
	LenSaveFile = GetSaveSize();
	h=FAT_fopen(str,"r");
	if(h==NULL)
  		_consolePrintf("open file %s fail\n",str);
    	else
   	{
	  	_consolePrintf("load save file...\n");
	   MWin_ShowProgressDialog(1,0,0);
   	   for(ii=0;ii<LenSaveFile;ii+=512)
  	   {
  	   		FAT_fread(pSaveData,512,1,h);
		  	cardmeWriteEeprom(ii,pSaveData,512,3);			
			for(s32 kk=0;kk<512;kk++)
			{
				dsCardi_WriteSram(0xE0000+ii*2+kk*2, (pSaveData[kk]|0xFF00)); //¸ßÎ»0xFF¸üÊ¡µç
			}
			if( (ii*10/LenSaveFile)>0)
					MWin_ShowProgressDialog(1,ii*10/LenSaveFile,0);
				
  		}
		FAT_fclose(h);
  		_consolePrintf("load save file OK\n");
    	}
	FAT_FreeFiles();
	if(strlen(pPath)>6)
	{
		pPath +=6;
		FAT_CWD(pPath);
		_consolePrintf("path=%s",pPath);
	}
	else
		FAT_CWD("/");
	MWin_ShowProgressDialog(3,0,0);
	/**********½áÊø¼ÓÔØ´æµµÎÄ¼þ******************/
	if(pFlash)
	{
		free(pFlash);
		pFlash=NULL;
	}
	
    return;
  }
#endif
  
  if(ExecMode==EM_FPK){ cwl();
    MWin_AllocMem(WM_PlayControl);
    if(FS_FileOpen(FileIndex)==false) return;
    if(StartFPK(FileHandle)==false){ cwl();
      FreeFPK();
      FS_ExecuteFile_LoadFail();
      PlugInfo_ShowCustomMessage_LoadError();
      FS_ExecuteFile_InitFileInfoWindow();
      return;
    }
    FS_ExecuteFile_InitFileInfoWindow();
    
    FS_Redraw_SetAll();
    MWin_RefreshWindow(WM_FileSelect);
    MWin_TransWindow(WM_FileSelect);
    
    MWin_AllRefresh();
    return;
  }
  
//--------------------------------------------------------
  
  if(ExecMode==EM_MSPSound){ cwl();
    {
      char fn[PluginFilenameMax];
      char ext[256];
      
      FileSys_GetFileExt(FileIndex,ext);
      DLLList_GetPluginFilename(ext,fn);
      pPluginBody=DLLList_LoadPlugin(fn);
    }
    MWin_AllocMem(WM_PlayControl);
    if(FS_FileOpen(FileIndex)==false) return;
    if(pPluginBody->pSL->Start(FileHandle)==false){ cwl();
      FS_ExecuteFile_LoadFail();
      PlugInfo_ShowCustomMessage_LoadError();
      FS_ExecuteFile_InitFileInfoWindow();
      return;
    }
    FS_ExecuteFile_InitFileInfoWindow();
    FS_ExecuteFile_InitPlayControlWindow(FileIndex);
    
    u32 SampleRate=pPluginBody->pSL->GetSampleRate();
    EstrpcmFormat SPF=FS_ExecuteFile_GetOversamplingFactorFromSampleRate(SampleRate);
    
    Resume_Backup(true);
    strpcmStart(false,SampleRate,pPluginBody->pSL->GetSamplePerFrame(),pPluginBody->pSL->GetChannelCount(),SPF);
    PrintFreeMem();
    return;
  }
  
#ifdef USEGME

  if(ExecMode==EM_GMENSF){ cwl();
    MWin_AllocMem(WM_PlayControl);
    if(FS_FileOpen(FileIndex)==false) return;
    
    int TrackNum=FileSys_GetFileTrackNum(FileIndex);
    if(TrackNum==-1) TrackNum=0;
    
    if(StartGMENSF(FileHandle,TrackNum)==false){ cwl();
      FS_ExecuteFile_LoadFail();
      PlugInfo_ShowCustomMessage_LoadError();
      FS_ExecuteFile_InitFileInfoWindow();
      return;
    }
    FS_ExecuteFile_InitFileInfoWindow();
    FS_ExecuteFile_InitPlayControlWindow(FileIndex);
    
    u32 SampleRate=GMENSF_GetSampleRate();
    EstrpcmFormat SPF=FS_ExecuteFile_GetOversamplingFactorFromSampleRate(SampleRate);
    
    Resume_Backup(true);
    strpcmStart(false,SampleRate,GMENSF_GetSamplePerFrame(),GMENSF_GetChannelCount(),SPF);
    PrintFreeMem();
    return;
  }
  
  if(ExecMode==EM_GMEGBS){ cwl();
    MWin_AllocMem(WM_PlayControl);
    if(FS_FileOpen(FileIndex)==false) return;
    
    int TrackNum=FileSys_GetFileTrackNum(FileIndex);
    if(TrackNum==-1) TrackNum=0;
    
    if(StartGMEGBS(FileHandle,TrackNum)==false){ cwl();
      FS_ExecuteFile_LoadFail();
      PlugInfo_ShowCustomMessage_LoadError();
      FS_ExecuteFile_InitFileInfoWindow();
      return;
    }
    FS_ExecuteFile_InitFileInfoWindow();
    FS_ExecuteFile_InitPlayControlWindow(FileIndex);
    
    u32 SampleRate=GMEGBS_GetSampleRate();
    EstrpcmFormat SPF=FS_ExecuteFile_GetOversamplingFactorFromSampleRate(SampleRate);
    
    Resume_Backup(true);
    strpcmStart(false,SampleRate,GMEGBS_GetSamplePerFrame(),GMEGBS_GetChannelCount(),SPF);
    PrintFreeMem();
    return;
  }
  
/*
  if(ExecMode==EM_GMEVGM){ cwl();
    MWin_AllocMem(WM_PlayControl);
    if(FS_FileOpen(FileIndex)==false) return;
    
    int TrackNum=FileSys_GetFileTrackNum(FileIndex);
    if(TrackNum==-1) TrackNum=0;
    
    if(StartGMEVGM(FileHandle,TrackNum)==false){ cwl();
      FS_ExecuteFile_LoadFail();
      PlugInfo_ShowCustomMessage_LoadError();
      FS_ExecuteFile_InitFileInfoWindow();
      return;
    }
    FS_ExecuteFile_InitFileInfoWindow();
    FS_ExecuteFile_InitPlayControlWindow(FileIndex);
    
    u32 SampleRate=GMEVGM_GetSampleRate();
    EstrpcmFormat SPF=FS_ExecuteFile_GetOversamplingFactorFromSampleRate(SampleRate);
    
    Resume_Backup(true);
    strpcmStart(false,SampleRate,GMEVGM_GetSamplePerFrame(),GMEVGM_GetChannelCount(),SPF);
    PrintFreeMem();
    return;
  }
*/
  
/*
  if(ExecMode==EM_GMEGYM){ cwl();
    MWin_AllocMem(WM_PlayControl);
    if(FS_FileOpen(FileIndex)==false) return;
    
    int TrackNum=FileSys_GetFileTrackNum(FileIndex);
    if(TrackNum==-1) TrackNum=0;
    
    if(StartGMEGYM(FileHandle,TrackNum)==false){ cwl();
      FS_ExecuteFile_LoadFail();
      PlugInfo_ShowCustomMessage_LoadError();
      FS_ExecuteFile_InitFileInfoWindow();
      return;
    }
    FS_ExecuteFile_InitFileInfoWindow();
    FS_ExecuteFile_InitPlayControlWindow(FileIndex);
    
    u32 SampleRate=GMEGYM_GetSampleRate();
    EstrpcmFormat SPF=FS_ExecuteFile_GetOversamplingFactorFromSampleRate(SampleRate);
    
    Resume_Backup(true);
    strpcmStart(false,SampleRate,GMEGYM_GetSamplePerFrame(),GMEGYM_GetChannelCount(),SPF);
    PrintFreeMem();
    return;
  }
*/
  
#endif

//--------------------------------------------------------
  
#ifdef USEDPG

  if(ExecMode==EM_DPG){ cwl();
/*
    if(DIMediaType==DIMT_MPMD){
      _consolePrintf("GBAMP MicroDrive driver not support DPG.\n");
      FS_ExecuteFile_LoadFail();
      PlugInfo_ShowCustomMessage_LoadError();
      FS_ExecuteFile_InitFileInfoWindow();
      return;
    }
*/
    
    MWin_AllocMem(WM_PlayControl);
    MWin_AllocMem(WM_PicView);
    
    if(FS_FileOpen2(FileIndex)==false) return;
    if(StartDPG(FileHandle1,FileHandle2)==false){ cwl();
      FS_ExecuteFile_LoadFail();
      PlugInfo_ShowCustomMessage_LoadError();
      FS_ExecuteFile_InitFileInfoWindow();
      return;
    }
    DPG_RequestSyncStart=true;
    
    FS_ExecuteFile_SetWindowTitle(WM_PicView,FileIndex,"MovieView...");
    
    { cwl();
      MWin_SetVideoWideFlag(false);
      MWin_ClearVideoOverlay();
      MWin_SetVideoOverlaySize(DPG_GetWidth(),DPG_GetHeight(),0);
      MWin_SetVideoOverlay(true);
      MWin_RefreshVideoFullScreen(false);
    }
    
    { cwl();
      MWin_SetSBarH(WM_PicView,ScreenWidth/2,ScreenWidth/2,0);
      MWin_SetSBarV(WM_PicView,ScreenHeight/2,ScreenHeight/2,0);
      
      MWin_SetVisible(WM_PicView,true);
      MWin_SetVisible(WM_PlayControl,true);
      
      MWin_SetActive(WM_PicView);
      MWin_SetActive(WM_PlayControl);
      
      MWin_RefreshScreenMask();
      MWin_AllRefresh();
    }
    
    FS_ExecuteFile_InitFileInfoWindow();
    Resume_Backup(true);
    return;
  }

#endif
  
  if(ExecMode==EM_Text){ cwl();
    MWin_AllocMem(WM_TextView);
    if(FS_FileOpen(FileIndex)==false) return;
    if(StartText(FileHandle,MWin_GetClientWidth(WM_TextView),MWin_GetClientHeight(WM_TextView))==false){ cwl();
      FreeText();
      FS_ExecuteFile_LoadFail();
      PlugInfo_ShowCustomMessage_LoadError();
      FS_ExecuteFile_InitFileInfoWindow();
      return;
    }
    FS_ExecuteFile_InitFileInfoWindow();
    MWin_SetSBarV(WM_TextView,Text_GetLineCount()+1,Text_GetDispLineCount(),Text_GetLine());
    
    MWin_SetVisible(WM_TextView,true);
    u32 WinIndex=MWin_SetActive(WM_TextView);
    MWin_RefreshScreenMask();
    if(WinIndex!=(u32)-1) MWin_RefreshWindow(WinIndex);
    MWin_RefreshWindow(WM_TextView);
    
    switch(GlobalINI.TextPlugin.SelectDisplay){
      case EITPSD_Bottom: IPC3->LCDPowerControl=LCDPC_ON_BOTTOM; break;
      case EITPSD_Top: IPC3->LCDPowerControl=LCDPC_ON_TOP_LEDON; break;
      default: break;
    }
    
    Bookmark_Start();
    Bookmark_CurrentResumeRestore();
    Resume_Backup(true);
    return;
  }
  
//--------------------------------------------------------
  
  if(ExecMode!=EM_None){ cwl();
    MWin_AllocMem(WM_PicView);
    
    switch(ExecMode){ cwl();
      case EM_MSPImage:
        {
          char fn[PluginFilenameMax];
          char ext[256];
          
          FileSys_GetFileExt(FileIndex,ext);
          DLLList_GetPluginFilename(ext,fn);
          pPluginBody=DLLList_LoadPlugin(fn);
        }
        if(FS_FileOpen(FileIndex)==false) return;
        if(pPluginBody->pIL->Start(FileHandle)==false){ cwl();
          pPluginBody->pIL->Free();
          FS_ExecuteFile_LoadFail();
          PlugInfo_ShowCustomMessage_LoadError();
          FS_ExecuteFile_InitFileInfoWindow();
          return;
        }
        break;
      default: {
        _consolePrintf("no-regist ExecMode type.\n");
        ShowLogHalt();
      } break;
    }
    
    PrintFreeMem();
    
    ImageControlTimeOut_SetDefault(GlobalINI.ImagePlugin.GUITimeOutSec);
    imgcalc_SetImageMode(GlobalINI.ImagePlugin.ImageMode,GlobalINI.ImagePlugin.VerticalPadding);
    
    Pic_InitPos();
    
    const float startratio=1;
    
    Pic_SetRatio(startratio);
    
    { cwl();
      MWin_SetVideoWideFlag(imgcalc_GetWideFlag());
      MWin_ClearVideoOverlay();
      MWin_SetVideoOverlaySize(0,0,Pic_GetRatio());
      MWin_SetVideoOverlay(true);
      MWin_RefreshVideoFullScreen(true);
    }
    pScreenMain->Flip(true);
    
    Pic_SetSBar(startratio);
    
    FS_ExecuteFile_SetWindowTitle(WM_PicView,FileIndex,"PictureView...");
    
    MWin_SetVisible(WM_PicView,true);
    u32 WinIndex=MWin_SetActive(WM_PicView);
    MWin_RefreshScreenMask();
    if(WinIndex!=(u32)-1) MWin_RefreshWindow(WinIndex);
    FS_ExecuteFile_PicView_DrawFileInfo();
    MWin_RefreshWindow(WM_PicView);
    
    Resume_Backup(true);
    return;
  }
  
}

void FS_StartFromIndex(s32 idx)
{ cwl();
  if((0<=idx)&&(idx<FS_IndexCount)){ cwl();
    FS_Redraw_SetPlayIndex();
    FS_PlayIndex=idx;
    FS_Redraw_SetPlayIndex();
    
    MWin_DrawClient(WM_FileSelect);
    if(PluginBodyClock_Execute==false) MWin_TransWindow(WM_FileSelect);
    
    u32 FileType=FileSys_GetFileType(idx);
    
    if(FileType==FT_Path) FS_ExecutePath(idx);
    if(FileType==FT_File) FS_ExecuteFile(idx);
  }
}
/*blowfish add for ez5 2006-11-29*****************************************/
void FS_DrawFileInfoFromIndex(s32 idx,int8 savetype)
{
	cwl();
	if((0<=idx)&&(idx<FS_IndexCount)){ cwl();
		u32 FileType=FileSys_GetFileType(idx);
		if(FileType==FT_Path) MWin_ClearDesktop();
		if(FileType==FT_File) FS_DrawFileDetail(idx,savetype);
	}
}
/*blowfish add for ez5 2006-11-29*****************************************/
static void FS_Restart(void)
{ cwl();
  if(FS_PlayIndex==-1) return;
  
  FS_StartFromIndex(FS_PlayIndex);
}

static void FS_StartFromCursor(void)
{ cwl();
  FS_StartFromIndex(FS_CursorIndex);
}
/*blowfish add for ez5 2006-11-29*****************************************/
static void FS_StartDrawFileInfoFromCursor(int8 savetype)
{ cwl();
  FS_DrawFileInfoFromIndex(FS_CursorIndex,savetype);

}
/*blowfish add for ez5 2006-11-29*****************************************/
static void FS_SetPlayIndex(s32 idx)
{ cwl();
  if(FS_IndexCount<0) idx=0;
  if(FS_IndexCount<=idx) idx=FS_IndexCount-1;
  
  FS_Redraw_SetPlayIndex();
  FS_PlayIndex=idx;
  FS_Redraw_SetPlayIndex();
}

static void FS_MovePlayIndex(s32 v)
{ cwl();
  if(FS_PlayIndex==-1) return;
  
  FS_SetPlayIndex(FS_PlayIndex+v);
}

void FS_SetCursor(s32 idx)
{ cwl();
  if(idx<0) idx=0;
  if((s32)FS_IndexCount<=idx) idx=FS_IndexCount-1;
  
  FS_Redraw_SetCursorIndex();
  FS_CursorIndex=idx;
  FS_Redraw_SetCursorIndex();
  
  if(GlobalINI.System.FileSelectSubScreen==true) thumbChangeCursorIndex(idx);
  
  s32 curidx=FS_CursorIndex;
  s32 lasttop=FS_TopIndex;
  s32 newtop=lasttop;
  s32 dispcnt=FS_DispCount;
  
  if(thumbExists()==false){
    if(dispcnt<=(curidx-newtop)){ cwl();
      newtop=curidx-(dispcnt-1);
      if(newtop<0) newtop=0;
    }
    if((curidx-newtop)<0){ cwl();
      newtop=curidx;
    }
    }else{
    
    if(dispcnt<=(curidx-newtop)){ cwl();
      newtop=curidx;
      newtop=newtop/3*3;
      newtop-=dispcnt-3;
      if(newtop<0) newtop=0;
    }
    if((curidx-newtop)<0){ cwl();
      newtop=curidx;
      newtop=newtop/3*3;
    }
  }
  
  if(lasttop!=newtop){
    FS_TopIndex=newtop;
    FS_Redraw_SetTopIndex();
  }
  
}

static void FS_MoveCursor(s32 v)
{ cwl();
  FS_SetCursor(FS_CursorIndex+v);
}

static void FS_MusicNext(u32 MusicNext)
{ cwl();
  if(FS_PlayIndex==-1) return;
  
  FS_Redraw_SetPlayIndex();
  
  EExecMode TagEM;
  
  TagEM=ExecMode;
  if(TagEM==EM_None) TagEM=LastExecMode;
  
  if(TagEM==EM_None) return;
  if(TagEM==EM_MP3Boot) return;
  
  switch(MusicNext){ cwl();
    case MusicNext_Stop: { cwl();
    } break;
    case MusicNext_Repeat: { cwl();
      FS_StartFromIndex(FS_PlayIndex);
    } break;
    case MusicNext_NormalLoop: { cwl();
      while(1){ cwl();
        int LastIndex=FS_PlayIndex;
        
        FS_PlayIndex=Normal_GetNextIndex(LastIndex);
        if(FS_PlayIndex==-1) FS_PlayIndex=0;
        
        if(FileSys_GetFileType(FS_PlayIndex)==FT_File){ cwl();
          if(TagEM==FS_GetExecMode(FS_PlayIndex)){ cwl();
            FS_StartFromIndex(FS_PlayIndex);
            break;
          }
        }
      }
    } break;
    case MusicNext_NormalPOff: { cwl();
      while(1){ cwl();
        int LastIndex=FS_PlayIndex;
        
        FS_PlayIndex=Normal_GetNextIndex(LastIndex);
        if(FS_PlayIndex==-1){
          IPC3->LCDPowerControl=LCDPC_SOFT_POWEROFF;
          while(1) swiWaitForVBlank();
        }
        
        if(FileSys_GetFileType(FS_PlayIndex)==FT_File){ cwl();
          if(TagEM==FS_GetExecMode(FS_PlayIndex)){ cwl();
            FS_StartFromIndex(FS_PlayIndex);
            break;
          }
        }
      }
    } break;
    case MusicNext_ShuffleLoop: { cwl();
      while(1){ cwl();
        int LastIndex=FS_PlayIndex;
        
        FS_PlayIndex=Shuffle_GetNextIndex(LastIndex);
        if(FS_PlayIndex==-1){
          Shuffle_Clear();
          FS_PlayIndex=Shuffle_GetNextIndex(LastIndex);
        }
        
        if(FileSys_GetFileType(FS_PlayIndex)==FT_File){ cwl();
          if(TagEM==FS_GetExecMode(FS_PlayIndex)){ cwl();
            FS_StartFromIndex(FS_PlayIndex);
            break;
          }
        }
      }
    } break;
    case MusicNext_ShufflePOff: { cwl();
      while(1){ cwl();
        int LastIndex=FS_PlayIndex;
        
        FS_PlayIndex=Shuffle_GetNextIndex(LastIndex);
        if(FS_PlayIndex==-1){
          IPC3->LCDPowerControl=LCDPC_SOFT_POWEROFF;
          while(1) swiWaitForVBlank();
        }
        
        if(FileSys_GetFileType(FS_PlayIndex)==FT_File){ cwl();
          if(TagEM==FS_GetExecMode(FS_PlayIndex)){ cwl();
            FS_StartFromIndex(FS_PlayIndex);
            break;
          }
        }
      }
    } break;
    case MusicNext_PowerOff: { cwl();
      IPC3->LCDPowerControl=LCDPC_SOFT_POWEROFF;
      while(1) swiWaitForVBlank();
    } break;
  }
}

static void FS_MusicPrev(u32 MusicNext)
{ cwl();
  if(FS_PlayIndex==-1) return;
  
  FS_Redraw_SetPlayIndex();
  
  EExecMode TagEM;
  
  TagEM=ExecMode;
  if(TagEM==EM_None) TagEM=LastExecMode;
  
  if(TagEM==EM_None) return;
  if(TagEM==EM_MP3Boot) return;
  
  switch(MusicNext){ cwl();
    case MusicNext_Stop: { cwl();
      _consolePrintf("FatalError.MusicPrev_Stop\n");
      ShowLogHalt();
    } break;
    case MusicNext_Repeat: { cwl();
      FS_StartFromIndex(FS_PlayIndex);
    } break;
    case MusicNext_NormalLoop: { cwl();
      while(1){ cwl();
        int LastIndex=FS_PlayIndex;
        
        FS_PlayIndex=Normal_GetPrevIndex(LastIndex);
        if(FS_PlayIndex==-1) break;
        
        if(FileSys_GetFileType(FS_PlayIndex)==FT_File){ cwl();
          if(TagEM==FS_GetExecMode(FS_PlayIndex)){ cwl();
            FS_StartFromIndex(FS_PlayIndex);
            break;
          }
        }
      }
    } break;
    case MusicNext_NormalPOff: { cwl();
      _consolePrintf("FatalError.MusicPrev_NormalPOff\n");
      ShowLogHalt();
    } break;
    case MusicNext_ShuffleLoop: { cwl();
      while(1){ cwl();
        int LastIndex=FS_PlayIndex;
        
        FS_PlayIndex=Shuffle_GetPrevIndex(LastIndex);
        if(FS_PlayIndex==-1) break;
        
        if(FileSys_GetFileType(FS_PlayIndex)==FT_File){ cwl();
          if(TagEM==FS_GetExecMode(FS_PlayIndex)){ cwl();
            FS_StartFromIndex(FS_PlayIndex);
            break;
          }
        }
      }
    } break;
    case MusicNext_ShufflePOff: { cwl();
      while(1){ cwl();
        int LastIndex=FS_PlayIndex;
        
        FS_PlayIndex=Shuffle_GetNextIndex(LastIndex);
        if(FS_PlayIndex==-1){
          IPC3->LCDPowerControl=LCDPC_SOFT_POWEROFF;
          while(1) swiWaitForVBlank();
        }
        
        if(FileSys_GetFileType(FS_PlayIndex)==FT_File){ cwl();
          if(TagEM==FS_GetExecMode(FS_PlayIndex)){ cwl();
            FS_StartFromIndex(FS_PlayIndex);
            break;
          }
        }
      }
    } break;
    case MusicNext_PowerOff: { cwl();
      _consolePrintf("FatalError.MusicPrev_PowerOff\n");
      ShowLogHalt();
    } break;
  }
}


