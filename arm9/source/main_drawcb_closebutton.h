
void MWinCallAbout_CloseButton(u32 WinIndex)
{
  MWin_SetVisible(WinIndex,false);
  MWin_SetActiveTopMost();
  MWin_RefreshScreenMask();
  MWin_AllRefresh();
}

void MWinCallHelp_CloseButton(u32 WinIndex)
{
  MWin_SetVisible(WinIndex,false);
  MWin_SetActiveTopMost();
  MWin_RefreshScreenMask();
  MWin_AllRefresh();
}

void MWinCallPlayControl_CloseButton(u32 WinIndex)
{
  RequestFileCloseFromMWin=true;
}

void MWinCallFileSelect_CloseButton(u32 WinIndex)
{
  MWin_SetVisible(WinIndex,false);
  MWin_SetActiveTopMost();
  MWin_RefreshScreenMask();
  MWin_AllRefresh();
}

void MWinCallDateTime_CloseButton(u32 WinIndex)
{
  MWin_SetVisible(WinIndex,false);
  MWin_SetActiveTopMost();
  MWin_RefreshScreenMask();
  MWin_AllRefresh();
}

void MWinCallPicView_CloseButton(u32 WinIndex)
{
  RequestFileCloseFromMWin=true;
}

void MWinCallTextView_CloseButton(u32 WinIndex)
{
  RequestFileCloseFromMWin=true;
}

void MWinCallProgress_CloseButton(u32 WinIndex)
{
}

void MWinCallSetNext_CloseButton(u32 WinIndex)
{
  MWin_SetVisible(WM_SetNext,false);
  if(MWin_GetVisible(WM_PlayControl)==true){
    MWin_SetActive(WM_PlayControl);
    }else{
    MWin_SetActiveTopMost();
  }
  MWin_RefreshScreenMask();
  MWin_AllRefresh();
}

void MWinCallFileInfo_CloseButton(u32 WinIndex)
{
  MWin_SetVisible(WinIndex,false);
  MWin_SetActiveTopMost();
  MWin_RefreshScreenMask();
  MWin_AllRefresh();
}

