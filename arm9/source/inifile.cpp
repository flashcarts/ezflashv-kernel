
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <NDS.h>

#include "_console.h"
#include "_const.h"

#include "inifile.h"
#include "strtool.h"
#include "memtool.h"

#include "mwin_color.h"

TGlobalINI GlobalINI;

static void InitOverrideWindowRect(TiniOverrideWindowRectBody *tag)
{
  tag->x=-1;
  tag->y=-1;
  tag->w=-1;
  tag->h=-1;
  tag->DisabledCloseButton=false;
}

void InitINI(void)
{
  {
    TiniSystem *System=&GlobalINI.System;
    
    strcpy(System->StartPath,"//AUTO");
    System->FileSelectSubScreen=true;
    System->ClosedSholderButton=ECSB_Flexible;
    System->MusicNext=MusicNext_ShuffleLoop;
    System->SoundVolume=16;
    System->FullScreenOverlaySubScreen=true;
    System->FileMaxCount=256;
    System->NDSLiteDefaultBrightness=4;
    System->WhenPanelClose=EISWPC_BacklightOff;
    System->TopScreenFlip=ESF_Normal;
    System->ResumeUsingWhileMusicPlaying=false;
    System->NDSROMBootFunctionWithEZ4SD=false;
    System->PrivateFunction=false;
  }
  
  {
    TiniForSuperCard *ForSuperCard=&GlobalINI.ForSuperCard;
    ForSuperCard->ROM1stAccessCycleControl=0;
    ForSuperCard->ROM2stAccessCycleControl=0;
  }
  
  {
    TiniHiddenItem *HiddenItem=&GlobalINI.HiddenItem;
    HiddenItem->Attribute_Archive=false;
    HiddenItem->Attribute_Hidden=true;
    HiddenItem->Attribute_System=true;
    HiddenItem->Attribute_Readonly=false;
    HiddenItem->Path_Shell=false;
    HiddenItem->File_Thumbnail=true;
  }
  
  {
    TiniKeyRepeat *KeyRepeat=&GlobalINI.KeyRepeat;
    KeyRepeat->DelayCount=15;
    KeyRepeat->RateCount=1;
  }
  
  {
    TiniBacklightTimeout *BacklightTimeout=&GlobalINI.BacklightTimeout;
    BacklightTimeout->WhenStandby=10;
    BacklightTimeout->WhenPicture=15;
    BacklightTimeout->WhenText=30;
    BacklightTimeout->WhenSound=3;
  }
  
  {
    TiniBoot *Boot=&GlobalINI.Boot;
    Boot->hiddenAboutWindow=true;
    Boot->hiddenHelpWindow=true;
    Boot->hiddenDateTimeWindow=true;
    Boot->StartupSound=EIBSS_Startupmp3;
  }
  
  {
    TiniThumbnail *Thumbnail=&GlobalINI.Thumbnail;
    Thumbnail->Mode=EITM_Both;
    Thumbnail->SelectItemAlpha=16;
    Thumbnail->UnselectItemAlpha=8;
  }
  
  {
    TiniTextPlugin *TextPlugin=&GlobalINI.TextPlugin;
    TextPlugin->SpacePixel=1;
    TextPlugin->ScrollType=EITPST_Normal;
    TextPlugin->SelectDisplay=EITPSD_Bottom;
  }
  
  {
    TiniNDSROMPlugin *NDSROMPlugin=&GlobalINI.NDSROMPlugin;
    NDSROMPlugin->CrashMeProtection=true;
  }
  
  {
    TiniDPGPlugin *DPGPlugin=&GlobalINI.DPGPlugin;
    DPGPlugin->Dithering=EDPGD_HalfCnt;
    DPGPlugin->GOPSkip=true;
    DPGPlugin->AdaptiveDelayControl=true;
    DPGPlugin->BlockGSMAudio=true;
  }
  
  {
    TiniImagePlugin *ImagePlugin=&GlobalINI.ImagePlugin;
    ImagePlugin->GUITimeOutSec=2;
    ImagePlugin->ImageMode=EIM_Single;
    ImagePlugin->DrawFileInfo=true;
    ImagePlugin->VerticalPadding=96;
    ImagePlugin->Interpolation=1;
  }
  
  {
    TiniClockPlugin *ClockPlugin=&GlobalINI.ClockPlugin;
    ClockPlugin->Enabled=true;
    ClockPlugin->RTC24Hour=true;
    strcpy(ClockPlugin->FormatDate,"YYYY/MM/DD");
  }
  
  {
    TiniOverrideWindowRect *OverrideWindowRect=&GlobalINI.OverrideWindowRect;
    InitOverrideWindowRect(&OverrideWindowRect->About);
    InitOverrideWindowRect(&OverrideWindowRect->Help);
    InitOverrideWindowRect(&OverrideWindowRect->PlayControl);
    InitOverrideWindowRect(&OverrideWindowRect->FileSelect);
    InitOverrideWindowRect(&OverrideWindowRect->DateTime);
    InitOverrideWindowRect(&OverrideWindowRect->PicView);
    InitOverrideWindowRect(&OverrideWindowRect->TextView);
    InitOverrideWindowRect(&OverrideWindowRect->Progress);
    InitOverrideWindowRect(&OverrideWindowRect->SetNext);
    InitOverrideWindowRect(&OverrideWindowRect->FileInfo);
  }
  
  {
    TiniAdapterConfig *AdapterConfig=&GlobalINI.AdapterConfig;
    AdapterConfig->EnabledWrite=true;
    AdapterConfig->SC_EnabledDRAM=false;
    AdapterConfig->EZ4_PSRAMSizeMByte=8;
  }
  
}

static char section[128];
static u32 readline;

static void readsection(char *str)
{
  str++;
  
  u32 ofs;
  
  ofs=0;
  while(*str!=']'){
    if((128<=ofs)||(*str==0)){
      _consolePrintf("line%d error.\nThe section name doesn't end correctly.\n",readline);
      ShowLogHalt();
    }
    section[ofs]=*str;
    str++;
    ofs++;
  }
  section[ofs]=0;
}

static void setColorCoord(u16 *storeitem,const int ivalue,const char *value)
{
  if(ivalue==-1) return;
  
  u32 v=0;
  
  while(*value!=0){
    char c=*value;
    
    if(('0'<=c)&&(c<='9')){
      v<<=4;
      v|=0x00+(c-'0');
    }
    if(('a'<=c)&&(c<='f')){
      v<<=4;
      v|=0x0a+(c-'a');
    }
    if(('A'<=c)&&(c<='F')){
      v<<=4;
      v|=0x0a+(c-'A');
    }
    
    value++;
  }
  
  u32 r,g,b;
  
  r=(v >> 16) & 0xff;
  g=(v >> 8) & 0xff;
  b=(v >> 0) & 0xff;
  
  *storeitem=RGB15(r/8,g/8,b/8) | BIT(15);
}

static bool SetOverrideWindowRectValue(char *key,s32 ivalue,bool bvalue,TiniOverrideWindowRectBody *Rect)
{
  if(ivalue<=-1) ivalue=-1;
  
  if(strcmp(key,"x")==0){
    Rect->x=ivalue;
    return(true);
  }
  if(strcmp(key,"y")==0){
    Rect->y=ivalue;
    return(true);
  }
  if(strcmp(key,"w")==0){
    Rect->w=ivalue;
    return(true);
  }
  if(strcmp(key,"h")==0){
    Rect->h=ivalue;
    return(true);
  }
  if(strcmp(key,"DisabledCloseButton")==0){
    Rect->DisabledCloseButton=bvalue;
    return(true);
  }
  
  return(false);
}

static void readkey(char *str)
{
  if(section[0]==0){
    _consolePrintf("line%d error.\nThere is a key ahead of the section name.\n",readline);
//    ShowLogHalt();
    return;
  }
  
  char key[128],value[128];
  
  u32 ofs;
  
  ofs=0;
  while(*str!='='){
    if((128<=ofs)||(*str==0)){
      _consolePrintf("line%d error.\nThe key name doesn't end correctly.\n",readline);
      ShowLogHalt();
    }
    key[ofs]=*str;
    str++;
    ofs++;
  }
  key[ofs]=0;
  
  str++;
  
  ofs=0;
  while(*str!=0){
    if(128<=ofs){
      _consolePrintf("line%d error.\nThe value doesn't end correctly.\n",readline);
      ShowLogHalt();
    }
    value[ofs]=*str;
    str++;
    ofs++;
  }
  value[ofs]=0;
  
  s32 ivalue=atoi(value);
  bool bvalue;
  
  if(ivalue==0){
    bvalue=false;
    }else{
    bvalue=true;
  }
  
  if(strcmp(section,"System")==0){
    TiniSystem *System=&GlobalINI.System;
    
    if(strcmp(key,"StartPath")==0){
      strcpy(System->StartPath,value);
      return;
    }
    if(strcmp(key,"FileSelectSubScreen")==0){
      System->FileSelectSubScreen=bvalue;
      return;
    }
    if(strcmp(key,"ClosedSholderButton")==0){
      System->ClosedSholderButton=(EClosedSholderButton)ivalue;
      return;
    }
    if(strcmp(key,"MusicNext")==0){
      System->MusicNext=ivalue;
      return;
    }
    if(strcmp(key,"SoundVolume")==0){
      System->SoundVolume=ivalue;
      return;
    }
    if(strcmp(key,"FullScreenOverlaySubScreen")==0){
      System->FullScreenOverlaySubScreen=bvalue;
      return;
    }
    if(strcmp(key,"FileMaxCount")==0){
      System->FileMaxCount=ivalue;
      return;
    }
    if(strcmp(key,"NDSLiteDefaultBrightness")==0){
      System->NDSLiteDefaultBrightness=ivalue;
      return;
    }
    if(strcmp(key,"WhenPanelClose")==0){
      System->WhenPanelClose=(EiniSystemWhenPanelClose)ivalue;
      return;
    }
    if(strcmp(key,"TopScreenFlip")==0){
      System->TopScreenFlip=(EScreenFlip)ivalue;
      return;
    }
    if(strcmp(key,"ResumeUsingWhileMusicPlaying")==0){
      System->ResumeUsingWhileMusicPlaying=bvalue;
      return;
    }
    if(strcmp(key,"NDSROMBootFunctionWithEZ4SD")==0){
      System->NDSROMBootFunctionWithEZ4SD=bvalue;
      return;
    }
    if(strcmp(key,"PrivateFunction")==0){
      System->PrivateFunction=bvalue;
      return;
    }
    
  }
  
  if(strcmp(section,"ForSuperCard")==0){
    TiniForSuperCard *ForSuperCard=&GlobalINI.ForSuperCard;
    
    if(strcmp(key,"ROM1stAccessCycleControl")==0){
      ForSuperCard->ROM1stAccessCycleControl=ivalue;
      return;
    }
    if(strcmp(key,"ROM2stAccessCycleControl")==0){
      ForSuperCard->ROM2stAccessCycleControl=ivalue;
      return;
    }
  }
  
  if(strcmp(section,"HiddenItem")==0){
    TiniHiddenItem *HiddenItem=&GlobalINI.HiddenItem;
    
    if(strcmp(key,"Attribute_Archive")==0){
      HiddenItem->Attribute_Archive=bvalue;
      return;
    }
    if(strcmp(key,"Attribute_Hidden")==0){
      HiddenItem->Attribute_Hidden=bvalue;
      return;
    }
    if(strcmp(key,"Attribute_System")==0){
      HiddenItem->Attribute_System=bvalue;
      return;
    }
    if(strcmp(key,"Attribute_Readonly")==0){
      HiddenItem->Attribute_Readonly=bvalue;
      return;
    }
    if(strcmp(key,"Path_Shell")==0){
      HiddenItem->Path_Shell=bvalue;
      return;
    }
    if(strcmp(key,"File_Thumbnail")==0){
      HiddenItem->File_Thumbnail=bvalue;
      return;
    }
    
  }
  
  if(strcmp(section,"KeyRepeat")==0){
    TiniKeyRepeat *KeyRepeat=&GlobalINI.KeyRepeat;
    
    if(strcmp(key,"DelayCount")==0){
      if(ivalue!=0) KeyRepeat->DelayCount=ivalue;
      return;
    }
    if(strcmp(key,"RateCount")==0){
      if(ivalue!=0) KeyRepeat->RateCount=ivalue;
      return;
    }
    
  }
  
  if(strcmp(section,"BacklightTimeout")==0){
    TiniBacklightTimeout *BacklightTimeout=&GlobalINI.BacklightTimeout;
    
    if(strcmp(key,"WhenStandby")==0){
      BacklightTimeout->WhenStandby=ivalue;
      return;
    }
    if(strcmp(key,"WhenPicture")==0){
      BacklightTimeout->WhenPicture=ivalue;
      return;
    }
    if(strcmp(key,"WhenText")==0){
      BacklightTimeout->WhenText=ivalue;
      return;
    }
    if(strcmp(key,"WhenSound")==0){
      BacklightTimeout->WhenSound=ivalue;
      return;
    }
    
  }
  
  if(strcmp(section,"Boot")==0){
    TiniBoot *Boot=&GlobalINI.Boot;
    
    if(strcmp(key,"hiddenAboutWindow")==0){
      Boot->hiddenAboutWindow=bvalue;
      return;
    }
    if(strcmp(key,"hiddenHelpWindow")==0){
      Boot->hiddenHelpWindow=bvalue;
      return;
    }
    if(strcmp(key,"hiddenDateTimeWindow")==0){
      Boot->hiddenDateTimeWindow=bvalue;
      return;
    }
    if(strcmp(key,"StartupSound")==0){
      Boot->StartupSound=(EiniBootStartupSound)ivalue;
      return;
    }
    
  }
  
  if(strcmp(section,"Thumbnail")==0){
    TiniThumbnail *Thumbnail=&GlobalINI.Thumbnail;
    
    if(strcmp(key,"Mode")==0){
      Thumbnail->Mode=(EiniThumbnailMode)ivalue;
      return;
    }
    if(strcmp(key,"SelectItemAlpha")==0){
      Thumbnail->SelectItemAlpha=ivalue;
      return;
    }
    if(strcmp(key,"UnselectItemAlpha")==0){
      Thumbnail->UnselectItemAlpha=ivalue;
      return;
    }
    
  }
  
  if(strcmp(section,"ColorCoordination")==0){
    
    if(strcmp(key,"DesktopBG")==0){
      setColorCoord(&MWC_DesktopBG,ivalue,value);
      return;
    }
    if(strcmp(key,"TitleA_Bright")==0){
      setColorCoord(&MWC_TitleA_Bright,ivalue,value);
      return;
    }
    if(strcmp(key,"TitleA_Dark")==0){
      setColorCoord(&MWC_TitleA_Dark,ivalue,value);
      return;
    }
    if(strcmp(key,"TitleA_BG")==0){
      setColorCoord(&MWC_TitleA_BG,ivalue,value);
      return;
    }
    if(strcmp(key,"TitleA_Text")==0){
      setColorCoord(&MWC_TitleA_Text,ivalue,value);
      return;
    }
    if(strcmp(key,"TitleA_TextShadow")==0){
      setColorCoord(&MWC_TitleA_TextShadow,ivalue,value);
      return;
    }
    if(strcmp(key,"TitleD_Bright")==0){
      setColorCoord(&MWC_TitleD_Bright,ivalue,value);
      return;
    }
    if(strcmp(key,"TitleD_Dark")==0){
      setColorCoord(&MWC_TitleD_Dark,ivalue,value);
      return;
    }
    if(strcmp(key,"TitleD_BG")==0){
      setColorCoord(&MWC_TitleD_BG,ivalue,value);
      return;
    }
    if(strcmp(key,"TitleD_Text")==0){
      setColorCoord(&MWC_TitleD_Text,ivalue,value);
      return;
    }
    if(strcmp(key,"ScrollBar")==0){
      setColorCoord(&MWC_ScrollBar,ivalue,value);
      return;
    }
    if(strcmp(key,"FrameBright")==0){
      setColorCoord(&MWC_FrameBright,ivalue,value);
      return;
    }
    if(strcmp(key,"FrameDark")==0){
      setColorCoord(&MWC_FrameDark,ivalue,value);
      return;
    }
    if(strcmp(key,"ClientBG")==0){
      setColorCoord(&MWC_ClientBG,ivalue,value);
      return;
    }
    if(strcmp(key,"ClientText")==0){
      setColorCoord(&MWC_ClientText,ivalue,value);
      return;
    }
    if(strcmp(key,"FileSelectBG")==0){
      setColorCoord(&MWC_FileSelectBG,ivalue,value);
      return;
    }
    if(strcmp(key,"FileSelectText")==0){
      setColorCoord(&MWC_FileSelectText,ivalue,value);
      return;
    }
    if(strcmp(key,"FileSelectCursor")==0){
      setColorCoord(&MWC_FileSelectCursor,ivalue,value);
      return;
    }
    if(strcmp(key,"FileSelectPlay")==0){
      setColorCoord(&MWC_FileSelectPlay,ivalue,value);
      return;
    }
    if(strcmp(key,"TextViewBG")==0){
      setColorCoord(&MWC_TextViewBG,ivalue,value);
      return;
    }
    if(strcmp(key,"TextViewText")==0){
      setColorCoord(&MWC_TextViewText,ivalue,value);
      return;
    }
    if(strcmp(key,"ProgressEnd")==0){
      setColorCoord(&MWC_ProgressEnd,ivalue,value);
      return;
    }
    if(strcmp(key,"ProgressLast")==0){
      setColorCoord(&MWC_ProgressLast,ivalue,value);
      return;
    }
    
  }
  
  if(strcmp(section,"TextPlugin")==0){
    TiniTextPlugin *TextPlugin=&GlobalINI.TextPlugin;
    
    if(strcmp(key,"SpacePixel")==0){
      TextPlugin->SpacePixel=ivalue;
      return;
    }
    if(strcmp(key,"ScrollType")==0){
      TextPlugin->ScrollType=(EiniTextPluginScrollType)ivalue;
      return;
    }
    if(strcmp(key,"SelectDisplay")==0){
      TextPlugin->SelectDisplay=(EiniTextPluginSelectDisplay)ivalue;
      return;
    }
    
  }
  
  if(strcmp(section,"NDSROMPlugin")==0){
    TiniNDSROMPlugin *NDSROMPlugin=&GlobalINI.NDSROMPlugin;
    
    if(strcmp(key,"CrashMeProtection")==0){
      NDSROMPlugin->CrashMeProtection=bvalue;
      return;
    }
    
  }
  
  if(strcmp(section,"DPGPlugin")==0){
    TiniDPGPlugin *DPGPlugin=&GlobalINI.DPGPlugin;
    
    if(strcmp(key,"Dithering")==0){
      DPGPlugin->Dithering=(EDPGDithering)ivalue;
      return;
    }
    if(strcmp(key,"GOPSkip")==0){
      DPGPlugin->GOPSkip=bvalue;
      return;
    }
    if(strcmp(key,"AdaptiveDelayControl")==0){
      DPGPlugin->AdaptiveDelayControl=bvalue;
      return;
    }
    if(strcmp(key,"BlockGSMAudio")==0){
      DPGPlugin->BlockGSMAudio=bvalue;
      return;
    }
    
  }
  
  {
    TiniImagePlugin *ImagePlugin=&GlobalINI.ImagePlugin;
    
    if(strcmp(key,"GUITimeOutSec")==0){
      ImagePlugin->GUITimeOutSec=ivalue;
      return;
    }
    if(strcmp(key,"ImageMode")==0){
      ImagePlugin->ImageMode=(EImageMode)ivalue;
      return;
    }
    if(strcmp(key,"DrawFileInfo")==0){
      ImagePlugin->DrawFileInfo=bvalue;
      return;
    }
    if(strcmp(key,"VerticalPadding")==0){
      ImagePlugin->VerticalPadding=ivalue;
      return;
    }
    if(strcmp(key,"Interpolation")==0){
      ImagePlugin->Interpolation=ivalue;
      return;
    }
    
  }
  
  if(strcmp(section,"ClockPlugin")==0){
    TiniClockPlugin *ClockPlugin=&GlobalINI.ClockPlugin;
    
    if(strcmp(key,"Enabled")==0){
      ClockPlugin->Enabled=bvalue;
      return;
    }
    if(strcmp(key,"RTC24Hour")==0){
      ClockPlugin->RTC24Hour=bvalue;
      return;
    }
    if(strcmp(key,"FormatDate")==0){
      strcpy(ClockPlugin->FormatDate,value);
      return;
    }
  }
  
  if(strcmp(section,"SkinInfo")==0){
    // ignore
    if(strcmp(key,"Title")==0) return;
    if(strcmp(key,"Author")==0) return;
    if(strcmp(key,"LastUpdate")==0) return;
  }
  
  if(strcmp(section,"OverrideWindowRect_About")==0){
    TiniOverrideWindowRectBody *Rect=&GlobalINI.OverrideWindowRect.About;
    
    if(SetOverrideWindowRectValue(key,ivalue,bvalue,Rect)==true) return;
  }
  
  if(strcmp(section,"OverrideWindowRect_Help")==0){
    TiniOverrideWindowRectBody *Rect=&GlobalINI.OverrideWindowRect.Help;
    
    if(SetOverrideWindowRectValue(key,ivalue,bvalue,Rect)==true) return;
  }
  
  if(strcmp(section,"OverrideWindowRect_PlayControl")==0){
    TiniOverrideWindowRectBody *Rect=&GlobalINI.OverrideWindowRect.PlayControl;
    
    if(SetOverrideWindowRectValue(key,ivalue,bvalue,Rect)==true) return;
  }
  
  if(strcmp(section,"OverrideWindowRect_FileSelect")==0){
    TiniOverrideWindowRectBody *Rect=&GlobalINI.OverrideWindowRect.FileSelect;
    
    if(SetOverrideWindowRectValue(key,ivalue,bvalue,Rect)==true) return;
  }
  
  if(strcmp(section,"OverrideWindowRect_DateTime")==0){
    TiniOverrideWindowRectBody *Rect=&GlobalINI.OverrideWindowRect.DateTime;
    
    if(SetOverrideWindowRectValue(key,ivalue,bvalue,Rect)==true) return;
  }
  
  if(strcmp(section,"OverrideWindowRect_PicView")==0){
    TiniOverrideWindowRectBody *Rect=&GlobalINI.OverrideWindowRect.PicView;
    
    if(SetOverrideWindowRectValue(key,ivalue,bvalue,Rect)==true) return;
  }
  
  if(strcmp(section,"OverrideWindowRect_TextView")==0){
    TiniOverrideWindowRectBody *Rect=&GlobalINI.OverrideWindowRect.TextView;
    
    if(SetOverrideWindowRectValue(key,ivalue,bvalue,Rect)==true) return;
  }
  
  if(strcmp(section,"OverrideWindowRect_Progress")==0){
    TiniOverrideWindowRectBody *Rect=&GlobalINI.OverrideWindowRect.Progress;
    
    if(SetOverrideWindowRectValue(key,ivalue,bvalue,Rect)==true) return;
  }
  
  if(strcmp(section,"OverrideWindowRect_SetNext")==0){
    TiniOverrideWindowRectBody *Rect=&GlobalINI.OverrideWindowRect.SetNext;
    
    if(SetOverrideWindowRectValue(key,ivalue,bvalue,Rect)==true) return;
  }
  
  if(strcmp(section,"OverrideWindowRect_FileInfo")==0){
    TiniOverrideWindowRectBody *Rect=&GlobalINI.OverrideWindowRect.FileInfo;
    
    if(SetOverrideWindowRectValue(key,ivalue,bvalue,Rect)==true) return;
  }
  
  if(strcmp(section,"AdapterConfig")==0){
    TiniAdapterConfig *AdapterConfig=&GlobalINI.AdapterConfig;
    
    if(strcmp(key,"EnabledWrite")==0){
      AdapterConfig->EnabledWrite=bvalue;
      return;
    }
    if(strcmp(key,"SC_EnabledDRAM")==0){
      AdapterConfig->SC_EnabledDRAM=bvalue;
      return;
    }
    if(strcmp(key,"EZ4_PSRAMSizeMByte")==0){
      AdapterConfig->EZ4_PSRAMSizeMByte=ivalue;
      return;
    }
    
  }
  _consolePrintf("line%d error.\ncurrent section [%s] unknown key=%s value=%s\n",readline,section,key,value);
//  ShowLogHalt();
}

static void internal_LoadGlobalINI(char *pini,u32 inisize)
{
  section[0]=0;
  readline=0;
  
  u32 iniofs=0;
  
  while(iniofs<inisize){
    
    readline++;
    
    u32 linelen=0;
    
    // Calc Line Length
    {
      char *s=&pini[iniofs];
      
      while(0x20<=*s){
        linelen++;
        s++;
        if(inisize<=(iniofs+linelen)) break;
      }
      *s=0;
    }
    
    if(linelen!=0){
      char c=pini[iniofs];
      if((c==';')||(c=='/')||(c=='!')){
        // comment line
        }else{
        if(c=='['){
          readsection(&pini[iniofs]);
          }else{
          readkey(&pini[iniofs]);
        }
      }
    }
    
    iniofs+=linelen;
    
    // skip NULL,CR,LF
    {
      char *s=&pini[iniofs];
      
      while(*s<0x20){
        iniofs++;
        s++;
        if(inisize<=iniofs) break;
      }
    }
    
  }
}

#include "shell.h"

void LoadINI(char *inifn)
{
  char *pini;
  u32 inisize;
  
  Shell_ReadFile(inifn,(void**)&pini,(int*)&inisize);
  if((pini==NULL)||(inisize==0)) return;
  
  _consolePrintf("load '/shell/%s\n",inifn);
  internal_LoadGlobalINI(pini,inisize);
  
  free(pini); pini=NULL;
  
  GlobalINI.NDSROMPlugin.CrashMeProtection=true; // override every enabled.
}

