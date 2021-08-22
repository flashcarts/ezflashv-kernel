
void ProcessTouchPad_DPG(void)
{ cwl();
  MWin_SetVideoFullScreen(false);
  MWin_RefreshVideoFullScreen(false);
  
  if(MWin_GetVisible(WM_PlayControl)==true){
    u32 WinIndex=MWin_SetActive(WM_PlayControl);
    MWin_RefreshScreenMask();
    if(WinIndex!=(u32)-1) MWin_RefreshWindow(WinIndex);
    MWin_RefreshWindow(WM_PlayControl);
  }
}

static bool TPA_Drag=false;
static int TPA_LastX,TPA_LastY;

static void ProcessTouchPad_AvgTouch_Reset(void)
{
  TPA_Drag=false;
}

static void ProcessTouchPad_AvgTouch_Proc(void)
{
  if(TPA_Drag==false){
    TPA_Drag=true;
    TPA_LastX=RIPC3.touchXpx;
    TPA_LastY=RIPC3.touchYpx;
    return;
  }
  
  int cx=TPA_LastX;
  int cy=TPA_LastY;
  int lx=RIPC3.touchXpx-cx;
  int ly=RIPC3.touchYpx-cy;
  
  RIPC3.touchXpx=cx+(lx*15/16);
  RIPC3.touchYpx=cy+(ly*15/16);
  
  TPA_LastX=RIPC3.touchXpx;
  TPA_LastY=RIPC3.touchYpx;
}

static void ProcessTouchPad(void)
{ cwl();
  if((KEYS_CUR&KEY_TOUCH)==0){ cwl();
    ProcessTouchPad_AvgTouch_Reset();
    if(tpIgnore==true){
      tpIgnore=false;
      return;
    }
    if(tpDragFlag==true){ cwl();
      tpDragFlag=false;
      MWin_MouseUp();
    }
    }else{ cwl();
    if(tpIgnore==true) return;
    
    ProcessTouchPad_AvgTouch_Proc();
    
    s32 Xpx,Ypx;
    
    Xpx=RIPC3.touchXpx;
    Ypx=RIPC3.touchYpx;
    
    if(tpDragFlag==false){ cwl();
      tpDragFlag=true;
      if(ExecMode==EM_DPG){ cwl();
        if(MWin_GetVideoFullScreen()==true){ cwl();
          ProcessTouchPad_DPG();
          return;
        }
      }
      MWin_MouseDown(Xpx,Ypx);
      }else{ cwl();
      MWin_MouseMove(Xpx,Ypx);
    }
  }
  
}

static void ProcessKeys(u16 loadkeys)
{ cwl();
  if(loadkeys & (KEY_START | KEY_SELECT | KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT | KEY_A | KEY_B | KEY_X | KEY_Y | KEY_L | KEY_R)==0) return;
  
  u32 TopWinIndex=MWin_GetTopWinIndex();
  
  if(GlobalINI.System.PrivateFunction==true){
    if(ExecMode==EM_MSPSound){
      if(loadkeys & KEY_A){
        loadkeys&=~KEY_A;
        loadkeys|=KEY_R;
      }
    }
  }
  /*增加设置菜单L+R***********************/
  if(loadkeys&KEY_L)
  {
  	

	 loadkeys &=~KEY_L;
	  u8 SaveTypeArr[6];
	  SaveTypeArr[0] = 0;
	  SaveTypeArr[1] = 1;
//	  SaveTypeArr[2] = 2;
//	  SaveTypeArr[3] = 3;
	  SaveTypeArr[2] = 4;
	  SaveTypeArr[3] = 5;
	  SaveTypeArr[4] = 6;
	  SaveTypeArr[5] = 0xFF;
//	  SaveTypeArr[6] = 0xFE;
	  if(loadkeys & KEY_R)
  	  {

		u8 indexY=0;
	  	MWin_ShowConfigDialog();
		MWIn_ShowSelcetItem(gl_language,gl_speedAuto,0,gl_speed,20);
		while(1){cwl();
			scanKeys();
			if(keysUp()&KEY_B)
			{
				cwl();
				u8 *pFlash = NULL;
				pFlash = (u8*)safemalloc(0x2000);
				if(pFlash==NULL)
				{

					_consolePrintf("Alloc mem fail\n");
				}
				else
				{
					dsCardi_SetRompage(0);
					ResetFlash();
					for(s16 ii=0;ii<0x2000;ii+=0x200)
					  dsCardi_ReadCard(ROM_OFF+ii,&pFlash[ii]);	
					//BL_ReadFlashBuffer(ROM_OFF,pFlash,0x2000);
				}
				if(pFlash!=NULL)
				{
	        					
					*pFlash = gl_language;
					*(pFlash+1) = gl_speedAuto;
					*((u16 *)(pFlash+2))=gl_speed;
					//_consolePrintf("erasing..\n");
					//EraseFlash(ROM_OFF);
					//_consolePrintf("writing..\n");
					//WriteFlash(ROM_OFF,pFlash,0x2000);
					//_consolePrintf("end..\n");
					dsCardi_SetRompage(0);
					BL_EraseFlash(ROM_OFF);
					BL_WriteFlash(ROM_OFF,pFlash,0x2000);
					free(pFlash);
					pFlash=NULL;
				}
				LoadLanguage();
				MWin_ShowDesktop();
			  	FS_StartDrawFileInfoFromCursor(0x55);
				WaitKeyClear(true);
				return;
			}
			if(keysUp()&KEY_A)
			{

				LoadLanguage();
				MWin_ShowDesktop();
			  	FS_StartDrawFileInfoFromCursor(0x55);
				return;
			}
			if(keysUp() & KEY_UP)
			{
				if(indexY==0)
					indexY=2;
				else
					indexY--;
				if(indexY==1)
					gl_speedAuto=0;
				else if(indexY==2)
					gl_speedAuto=1;
				MWIn_ShowSelcetItem(gl_language,gl_speedAuto,indexY,gl_speed,20);
			}
			if(keysUp() & KEY_DOWN)
			{
				if(indexY==2)
					indexY=0;
				else
					indexY++;
				if(indexY==1)
					gl_speedAuto=0;
				else if(indexY==2)
					gl_speedAuto=1;
				MWIn_ShowSelcetItem(gl_language,gl_speedAuto,indexY,gl_speed,20);
			}
			if(keysUp() & KEY_LEFT)
			{
				if(indexY==0)
				{
					gl_language=1;
					MWIn_ShowSelcetItem(gl_language,gl_speedAuto,indexY,gl_speed,20);
				}
				if(indexY==1)
				{

					if(gl_speed<=100)
						gl_speed=100;
					else
						gl_speed-=100;
					MWIn_ShowSelcetItem(gl_language,gl_speedAuto,indexY,gl_speed,20);
				}     
			}
			if(keysUp() & KEY_RIGHT)
			{
				if(indexY==0)
				{
					gl_language=2;
					MWIn_ShowSelcetItem(gl_language,gl_speedAuto,indexY,gl_speed,20);
				}
				if(indexY==1)
				{

					if(gl_speed>=1200)
						gl_speed=1200;
					else
						gl_speed+=100;
					MWIn_ShowSelcetItem(gl_language,gl_speedAuto,indexY,gl_speed,20);
				}
			}
		}
		
  	  }
	  if(loadkeys & KEY_X)
	  {
	  	 s16 kk;
//		 _consolePrintf("\n\n\n");
//		 _consolePrintf("L+X is press\n");
//		 _consolePrintf("begin gl_SaveType=%d\n",gl_SaveType);
		 for(kk=0;kk<6;kk++)
		 {
		 	if(gl_SaveType==SaveTypeArr[kk])
				break;
		 }
		 if(kk==5)
		 	kk=0;
		 else
		 	kk++;
//		 _consolePrintf("kk=%d\n",kk);
//		 _consolePrintf("gl_SaveType=%d\n",gl_SaveType);
//		 _consolePrintf("\n\n\n");
		 FS_StartDrawFileInfoFromCursor(SaveTypeArr[kk]);
		 gl_pSaveTypeBuf[gl_SaveTypeOff] = SaveTypeArr[kk];
		 WriteSaveTypeFile();
	  }
	  if(loadkeys & KEY_Y)
	  {
	  	
		s16 kk;
		 _consolePrintf("L+X is press\n");
		 for(kk=0;kk<6;kk++)
		 {
		 	if(gl_SaveType==SaveTypeArr[kk])
				break;
		 }
		 if(kk==0)
		 	kk=5;
		 else
		 	kk--;
		 FS_StartDrawFileInfoFromCursor(SaveTypeArr[kk]);
		 gl_pSaveTypeBuf[gl_SaveTypeOff] = SaveTypeArr[kk];
		 WriteSaveTypeFile();
	  }
  }
  if(loadkeys & KEY_START){ cwl();
  	/*
  	 if(ShowLog==false){ cwl();
      videoSetModeSub_SetShowLog(true);
      WaitKeyClear(false);
      videoSetModeSub_SetShowLog(false);
    }
    */
    //这里弹出帮助对话框
   // MWin_ShowProgressDialog(1,0,0);
   // MWin_ShowProgressDialog(1,3,0);
    MWin_ShowHelpDialog();
	while(1){cwl();
			scanKeys();
			if(keysUp()&KEY_B)
			{
				MWin_ShowDesktop();
			  	//FS_StartDrawFileInfoFromCursor(0x55);
				WaitKeyClear(true);
				break;
			}
	}
    VsyncPassedCount=0;
    
  }
  
  if(loadkeys & KEY_SELECT){ cwl();
    loadkeys&=~KEY_SELECT;
    u32 LastBR=NDSLite_Brightness;
    if(loadkeys & KEY_L){
      loadkeys&=~KEY_L;
      if(0<LastBR) LastBR--;
    }
    if(loadkeys & KEY_R){
      loadkeys&=~KEY_R;
      if(LastBR<3) LastBR++;
    }
    if(NDSLite_Brightness!=LastBR){
      NDSLite_Brightness=LastBR;
      NDSLite_SetBrightness(LastBR);
    }
  }
  
  if(ExecMode==EM_DPG){
    if((loadkeys&KEY_SELECT)!=0){ cwl();
      if(MWin_GetVideoFullScreen()==false){ cwl();
        MWin_SetVideoFullScreen(true);
        MWin_RefreshVideoFullScreen(false);
        }else{ cwl();
        ProcessTouchPad_DPG();
      }
      loadkeys&=~KEY_SELECT;
      KeyRepeat_Delay(10);
    }
    
    if((loadkeys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))!=0){
      s32 v=0;
      
      if(loadkeys & KEY_UP) v=-10;
      if(loadkeys & KEY_DOWN) v=+2;
      if(loadkeys & KEY_LEFT) v=-5;
      if(loadkeys & KEY_RIGHT) v=+1;
      
      if(loadkeys & KEY_A) v*=4;
      
      s32 tagf;
      tagf=(s32)DPG_GetCurrentFrameCount();
      tagf+=(s32)DPG_GetFPS()*v/0x100;
      
      if(tagf<0) tagf=0;
      
      if((s32)DPG_GetTotalFrameCount()<=tagf){ cwl();
        strpcmRequestStop=true;
        }else{ cwl();
        strpcmStop();
        DPG_SetFrameCount((u32)tagf);
        DPG_RequestSyncStart=true;
      }
    }
    loadkeys&=~(KEY_A | KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT);
  }
  
  if(ExecMode==EM_Text){
    EBookmarkDialogMode bdm=BDM_Exit;
    if((loadkeys&KEY_L)!=0) bdm=BDM_LoadMenu;
    if((loadkeys&KEY_R)!=0) bdm=BDM_SaveMenu;
    while(bdm!=BDM_Exit){
      bdm=Text_BookmarkDialog(bdm);
    }
    loadkeys&=~(KEY_L | KEY_R);
  }
  
  if((loadkeys & (KEY_A | KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))!=0){ cwl();
    switch(TopWinIndex){ cwl();
      case WM_PicView:
        { cwl();
          if((loadkeys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))!=0){
            u32 x,y,v;
            
            x=MWin_GetSBarHPos(WM_PicView);
            y=MWin_GetSBarVPos(WM_PicView);
            
            v=8;
            if(loadkeys & KEY_A) v*=4;
            v=(int)(v/Pic_GetRatio());
            
            if(loadkeys & KEY_UP) y-=v;
            if(loadkeys & KEY_DOWN) y+=v;
            if(loadkeys & KEY_LEFT) x-=v;
            if(loadkeys & KEY_RIGHT) x+=v;
            
            MWin_SetSBarHPos(WM_PicView,x);
            MWin_SetSBarVPos(WM_PicView,y);
            MWin_DrawSBarVH(WM_PicView);
            MWin_DrawClient(WM_PicView);
            MWin_TransWindow(WM_PicView);
          }
        }
        break;
      case WM_TextView:
        { cwl();
          u32 y,v;
          
          y=MWin_GetSBarVPos(WM_TextView);
          
          switch(GlobalINI.TextPlugin.ScrollType){
            case EITPST_Normal: v=Text_GetDispLineCount()-2; break;
            case EITPST_FullPage: v=Text_GetDispLineCount(); break;
            default: v=0; break;
          }
          
          if(loadkeys & KEY_A) y+=v;
          if(loadkeys & KEY_UP) y-=1;
          if(loadkeys & KEY_DOWN) y+=1;
          if(loadkeys & KEY_LEFT) y-=v;
          if(loadkeys & KEY_RIGHT) y+=v;
          
          MWin_SetSBarVPos(WM_TextView,y);
          MWin_DrawSBarVH(WM_TextView);
          MWin_DrawClient(WM_TextView);
          MWin_TransWindow(WM_TextView);
        }
        break;
      default:
        { cwl();
          if(loadkeys & KEY_A){
            Shuffle_Clear();
            FS_StartFromCursor();
            WaitKeyClear(true);
            RefreshBacklightOffCount();
            }else{
            if(thumbExists()==false){
              if(loadkeys & KEY_UP) 
			  {
			  		 cwl();
			    		//这里执行上键的操作,显示当前选中文件的图标
    					FS_MoveCursor(-1);
					FS_StartDrawFileInfoFromCursor(0x55);
              }
              if(loadkeys & KEY_DOWN) 
	    		{
	    				 cwl();
			  		FS_MoveCursor(1);
			  		FS_StartDrawFileInfoFromCursor(0x55);
	    		}
              if(loadkeys & KEY_LEFT) FS_MoveCursor(-FS_DispCount/2);
              if(loadkeys & KEY_RIGHT) FS_MoveCursor(FS_DispCount/2);
              }else{
              if(loadkeys & KEY_UP) FS_MoveCursor(-3);
              if(loadkeys & KEY_DOWN) FS_MoveCursor(3);
              if(loadkeys & KEY_LEFT) FS_MoveCursor(-1);
              if(loadkeys & KEY_RIGHT) FS_MoveCursor(1);
            }
            
            MWin_SetSBarVPos(WM_FileSelect,FS_TopIndex);
            MWin_DrawSBarVH(WM_FileSelect);
            MWin_DrawClient(WM_FileSelect);
            MWin_TransWindow(WM_FileSelect);
          }
        }
        break;
    }
    loadkeys&=~(KEY_A | KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT);
  }
  
  if(loadkeys & KEY_L){ cwl();
    u32 id=0; // 0=Null 1=DPG 2=Pause 3=Prev
    
    switch(ExecMode){
      case EM_None: case EM_NDSROM: case EM_FPK: case EM_MP3Boot: id=0; break;
      case EM_DPG: id=1; break;
      case EM_MSPSound: case EM_GMENSF: case EM_GMEGBS: id=2; break; // ,EM_GMEVGM,EM_GMEGYM
      case EM_MSPImage: case EM_Text: id=3; break;
      default: id=0; break;
    }
    
    switch(id){
      case 0: break;
      case 1: {
        if(strpcmGetPause()==true){ cwl();
          strpcmSetPause(false);
          strpcmStop();
          DPG_SetFrameCount(DPG_GetCurrentFrameCount());
          DPG_RequestSyncStart=true;
          }else{ cwl();
          strpcmStop();
          strpcmSetPause(true);
        }
      } break;
      case 2: {
        if(strpcmGetPause()==true){ cwl();
          strpcmSetPause(false);
          }else{ cwl();
          strpcmSetPause(true);
        }
      } break;
      case 3: {
        u32 MusicNext=GlobalINI.System.MusicNext;
        if((MusicNext==MusicNext_Stop)||(MusicNext==MusicNext_NormalPOff)||(MusicNext==MusicNext_PowerOff)){ cwl();
          MusicNext=MusicNext_NormalLoop;
        }
        if((MusicNext==MusicNext_ShufflePOff)){
          MusicNext=MusicNext_ShuffleLoop;
        }
        FS_MusicPrev(MusicNext);
      }
    }
    WaitKeyClear(false);
  }
  
  if(loadkeys & KEY_R){ cwl();
    u32 MusicNext=GlobalINI.System.MusicNext;
    if((MusicNext==MusicNext_Stop)||(MusicNext==MusicNext_NormalPOff)||(MusicNext==MusicNext_PowerOff)){ cwl();
      MusicNext=MusicNext_NormalLoop;
    }
    if((MusicNext==MusicNext_ShufflePOff)){
      MusicNext=MusicNext_ShuffleLoop;
    }
    FS_MusicNext(MusicNext);
    WaitKeyClear(true);
  }
  
  if(loadkeys & KEY_B){ cwl();
    if(ExecMode==EM_None){
      FS_SetCursor(0);
      FS_StartFromCursor();
      WaitKeyClear(true);
      RefreshBacklightOffCount();
      }else{
      FS_ExecuteStop();
      Shuffle_Clear();
      MWin_SetActive(WM_FileSelect);
      MWin_AllRefresh();
      MWin_TransWindow(WM_FileSelect);
      RefreshBacklightOffCount();
    }
  }
  
  if((loadkeys & (KEY_Y | KEY_X))!=0){ cwl();
    if(ExecMode==EM_MSPImage){
      float ratio=Pic_GetRatio();
      float lastratio=ratio;
      
      int RatioIndex=-1;
      
      if(loadkeys & KEY_Y) RatioIndex=imgcalc_GetNextIndex(ratio,-1);
      if(loadkeys & KEY_X) RatioIndex=imgcalc_GetNextIndex(ratio,1);
      
      if(RatioIndex!=-1){
        ratio=imgcalc_GetRatio(RatioIndex);
        if(imgcalc_GetKeyWait(RatioIndex)==true) KeyRepeat_Delay(1);
      }
      
      if(lastratio!=ratio){
        if(MWin_GetVisible(WM_PicView)==true){
          Pic_SetRatio(ratio);
          if(0.5<ratio){
            MWin_SetVideoOverlaySize(0,0,ratio);
            }else{
            MWin_SetVideoOverlaySize(0,0,ratio*2);
          }
//          swiWaitForVBlank();
          MWin_RefreshVideoFullScreen(true);
          Pic_SetSBar(ratio);
          MWin_DrawSBarVH(WM_PicView);
          MWin_DrawTitleBar(WM_PicView);
          MWin_DrawClient(WM_PicView);
          MWin_TransWindow(WM_PicView);
        }
      }
    }
    
    if((ExecMode==EM_MSPSound)||(ExecMode==EM_MP3Boot)||(ExecMode==EM_GMENSF)||(ExecMode==EM_GMEGBS)||(ExecMode==EM_DPG)){ // ||(ExecMode==EM_GMEVGM)||(ExecMode==EM_GMEGYM)
      int Volume=strpcmGetVolume16();
      
      if(loadkeys & KEY_Y) Volume-=1;
      if(loadkeys & KEY_X) Volume+=1;
      
      if(Volume<0) Volume=0;
      if(Volume>64) Volume=64;
      
      strpcmSetVolume16(Volume);
      
      if(MWin_GetVisible(WM_PlayControl)==true){
        MWin_DrawClient(WM_PlayControl);
        MWin_TransWindow(WM_PlayControl);
      }
    }
  }
  
}

