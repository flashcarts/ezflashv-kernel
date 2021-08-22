
static int MWinSetNext_LastMusicNext;

static s32 PicView_LastX,PicView_LastY;
static s32 Global_LastY;

static void Global_MouseDown(u32 WinIndex,s32 x,s32 y)
{ cwl();
  y/=FontProHeight/2;
  
  Global_LastY=y;
}

static void Global_MouseMove(u32 WinIndex,s32 x,s32 y)
{ cwl();
  y/=FontProHeight/2;
  
  MWin_SetSBarVPos(WinIndex,MWin_GetSBarVPos(WinIndex)-(y-Global_LastY));
  MWin_DrawSBarVH(WinIndex);
  MWin_DrawClient(WinIndex);
  MWin_TransWindow(WinIndex);
  
  Global_LastY=y;
}

static void Global_MouseUp(u32 WinIndex,s32 x,s32 y)
{ cwl();
  y/=FontProHeight/2;
  
  Global_LastY=0;
}

void MWinCallAbout_Draw(TWinBody *pwb)
{ cwl();
  u32 TopIndex=MWin_GetSBarVPos(WM_About);
  
  s32 BufWidth=pwb->Rect.w;
  u16 col=MWC_ClientText;
  
  s32 wLimit=pwb->ClientRect.x+pwb->ClientRect.w;
  u32 ofsx=pwb->ClientRect.x;
  u32 ofsy=pwb->ClientRect.y;
  
  u16 *pBuf=&pwb->WinBuf[0];
  
  if(pwb->WinBuf==NULL) return;
  
  for(s32 y=0;y<pwb->ClientRect.h/FontProHeight;y++){ cwl();
    s32 idx=TopIndex+y;
    char *str;
    
    switch(idx){ cwl();
      case 0: str=ROMVERSION; break;
      case 1: str=ROMDATE; break;
      case 3: str=MPCFIOTitle0; break;
      case 4: str=MPCFIOTitle1; break;
      case 5: str=ZLIBTitle; break;
      case 6: str=LIBGMETitle; break;
      case 7: str=DPGTitle0; break;
      case 8: str=DPGTitle1; break;
      case 9: str=DPGTitle2; break;
      default: str=""; break;
    }
    
    VRAM_WriteStringRect(pBuf,BufWidth,wLimit,ofsx+4,ofsy+(y*FontProHeight),str,col);
  }
}

void MWinCallHelp_Draw(TWinBody *pwb)
{ cwl();
  u16 col=MWC_ClientText;
  
  u32 TopIndex=MWin_GetSBarVPos(WM_Help);
  
  s32 BufWidth=pwb->Rect.w;
  
  s32 wLimit=pwb->ClientRect.x+pwb->ClientRect.w;
  u32 ofsx=pwb->ClientRect.x+4;
  u32 ofsy=pwb->ClientRect.y;
  
  u16 *pBuf=&pwb->WinBuf[0];
  
  if(pwb->WinBuf==NULL) return;
  
  for(s32 y=0;y<pwb->ClientRect.h/FontProHeight;y++){ cwl();
    s32 idx=TopIndex+y;
    char *pstr=NULL;
    
    if(DefaultCodePage!=CP932){
      switch(idx){ cwl();
        case 0: pstr="Up/Down CursorMove"; break;
        case 1: pstr="Left/Right CursorPage"; break;
        case 2: pstr="A/B FileOpen/FileClose"; break;
        case 3: pstr="START ShowDebugLog"; break;
        
        case 5: pstr="HoldSelect FastPlay"; break;
        case 6: pstr="L Pause"; break;
        case 7: pstr="R NextFile"; break;
        case 8: pstr="X/Y Volume for image"; break;
        case 9: pstr="X/Y Zoom for audio"; break;
        
        case 11: pstr="When panel closed."; break;
        case 12: pstr="R+L Pause"; break;
        case 13: pstr="L+R NextFile"; break;
        
        case 15: pstr="Backlight for NDS Lite."; break;
        case 16: pstr="Select+L Down light"; break;
        case 17: pstr="Select+R Up light"; break;
        
        case 19: pstr="Text bookmark"; break;
        case 20: pstr="L Open load menu"; break;
        case 21: pstr="R Open save menu"; break;
        
        default: pstr=NULL; break;
      }
      }else{
      switch(idx){ cwl();
        case 0: pstr="↑/↓ カーソル移動"; break;
        case 1: pstr="←/→ ページ移動"; break;
        case 2: pstr="A/B ファイルを開く/閉じる"; break;
        case 3: pstr="START デバッグウィンドウ"; break;
        
        case 5: pstr="HoldSelect 倍速再生"; break;
        case 6: pstr="L 一時停止"; break;
        case 7: pstr="R 次のファイルを開く"; break;
        case 8: pstr="X/Y ボリュームを変える"; break;
        case 9: pstr="X/Y 拡大率を変える"; break;
        
        case 11: pstr="パネルを閉じているとき"; break;
        case 12: pstr="R+L 一時停止"; break;
        case 13: pstr="L+R 次のファイルを開く"; break;
        
        case 15: pstr="NDS Lite バックライト操作"; break;
        case 16: pstr="Select+L 暗くする"; break;
        case 17: pstr="Select+R 明るくする"; break;
        
        case 19: pstr="テキストブックマーク"; break;
        case 20: pstr="L ロードメニューを開く"; break;
        case 21: pstr="R セーブメニューを開く"; break;
        
        default: pstr=NULL; break;
      }
    }
    VRAM_WriteStringRectUTF8(pBuf,BufWidth,wLimit,ofsx,ofsy+(y*FontProHeight),pstr,col);
  }
  
}

void MWinCallPlayControl_Draw(TWinBody *pwb,TSkinBM *pSkinBM)
{ cwl();
  u16 col=MWC_ClientText;
  
  s32 BufWidth=pwb->Rect.w;
  
  s32 wLimit=pwb->ClientRect.x+pwb->ClientRect.w;
  u32 ofsx=pwb->ClientRect.x;
  u32 ofsy=pwb->ClientRect.y;
  
  u16 *pBuf=&pwb->WinBuf[0];
  
  if(pwb->WinBuf==NULL) return;
  
  {
    const char *pformstr=NULL;
    char strbuf[256];
    if(DefaultCodePage!=CP932){
      pformstr="Volume:x%d";
      }else{
      pformstr="音量:x%d";
    }
    snprintf(strbuf,256,pformstr,(u32)(((float)strpcmGetVolume16()/16)*100));
    VRAM_WriteStringRectUTF8(pBuf,BufWidth,wLimit,ofsx,ofsy+(0*FontProHeight),strbuf,col);
  }
  
  const char *pstr=NULL;
  
  if(DefaultCodePage!=CP932){
    switch(GlobalINI.System.MusicNext){ cwl();
      case MusicNext_Stop: pstr="MusicNext:Stop"; break;
      case MusicNext_Repeat: pstr="MusicNext:OneRepeat"; break;
      case MusicNext_NormalLoop: pstr="MusicNext:NormalLoop"; break;
      case MusicNext_NormalPOff: pstr="MusicNext:NormalPowerOff"; break;
      case MusicNext_ShuffleLoop: pstr="MusicNext:ShuffleLoop"; break;
      case MusicNext_ShufflePOff: pstr="MusicNext:ShufflePowerOff"; break;
      case MusicNext_PowerOff: pstr="MusicNext:End PowerOFF"; break;
      default: pstr=NULL;
    }
    }else{
    switch(GlobalINI.System.MusicNext){ cwl();
      case MusicNext_Stop: pstr="MusicNext:一曲終わったら停止"; break;
      case MusicNext_Repeat: pstr="MusicNext:一曲リピート"; break;
      case MusicNext_NormalLoop: pstr="MusicNext:順ループ"; break;
      case MusicNext_NormalPOff: pstr="MusicNext:順ループ後電源OFF"; break;
      case MusicNext_ShuffleLoop: pstr="MusicNext:シャッフルループ"; break;
      case MusicNext_ShufflePOff: pstr="MusicNext:シャッフル後電源OFF"; break;
      case MusicNext_PowerOff: pstr="MusicNext:一曲再生後電源OFF"; break;
      default: pstr=NULL;
    }
  }
  
  VRAM_WriteStringRectUTF8(pBuf,BufWidth,wLimit,ofsx+64,ofsy+(0*FontProHeight),pstr,col);
  
  if(DefaultCodePage!=CP932){
    pstr="[change]";
    }else{
    pstr="[変更]";
  }
  VRAM_WriteStringRectUTF8(pBuf,BufWidth,wLimit,ofsx+210,ofsy+(0*FontProHeight),pstr,col);
  
  s32 fsize=0,fofs=0;
  
  switch(ExecMode){ cwl();
    case EM_MSPSound: { cwl();
      fsize=pPluginBody->pSL->GetPosMax();
      fofs=pPluginBody->pSL->GetPosOffset();
    } break;
    case EM_GMENSF: { cwl();
      fsize=GMENSF_GetPosMax();
      fofs=GMENSF_GetPosOffset();
    } break;
    case EM_GMEGBS: { cwl();
      fsize=GMEGBS_GetPosMax();
      fofs=GMEGBS_GetPosOffset();
    } break;
/*
    case EM_GMEVGM: { cwl();
      fsize=GMEVGM_GetPosMax();
      fofs=GMEVGM_GetPosOffset();
    } break;
    case EM_GMEGYM: { cwl();
      fsize=GMEGYM_GetPosMax();
      fofs=GMEGYM_GetPosOffset();
    } break;
*/
    case EM_DPG: { cwl();
      fsize=DPG_GetTotalFrameCount();
      fofs=DPG_GetCurrentFrameCount();
    } break;
    default: break;
  }
  
//  _consolePrintf("pcd=%d,%d\n",fsize,fofs);
  
  if(fsize==0) return;
  
  { cwl();
    s32 w=pwb->ClientRect.w;
    s32 h=16;
    
    s32 BufWidth=pwb->Rect.w;
    
    u32 ofsx=pwb->ClientRect.x;
    u32 ofsy=pwb->ClientRect.y+(pwb->ClientRect.h-h);
    
    u16 *pPrgBuf=&pBuf[ofsx+(ofsy*BufWidth)];
    
    float fv;
    s32 v;
    
    fv=(float)fofs/fsize;
    v=(s32)(fv*w);
    
    if(w<=v) v=w;
    if(v<0) v=0;
    
    const TSkinBM *psbm=pSkinBM;
    
    u16 *pPrgBarABM=psbm->pPrgBarABM;
    u16 *pPrgBarDBM=psbm->pPrgBarDBM;
    u32 PrgBarBM_W=psbm->PrgBarBM_W;
    
    if((pPrgBarABM!=NULL)&&(pPrgBarDBM!=NULL)){
      for(s32 y=0;y<h;y++){ cwl();
        MemCopy16DMA3(&pPrgBarABM[0],&pPrgBuf[0],v*2);
        if(v<w){ cwl();
          MemCopy16DMA3(&pPrgBarDBM[v],&pPrgBuf[v],(w-v)*2);
        }
        pPrgBarABM+=PrgBarBM_W;
        pPrgBarDBM+=PrgBarBM_W;
        pPrgBuf+=BufWidth;
      }
      }else{
      for(s32 y=0;y<h;y++){ cwl();
        MemSet16DMA3(MWC_ProgressEnd,&pPrgBuf[0],v*2);
        if(v<w){ cwl();
          MemSet16DMA3(MWC_ProgressLast,&pPrgBuf[v],(w-v)*2);
        }
        pPrgBuf+=BufWidth;
      }
    }
  }
  
}

static void MWinCallFileSelect_Draw_Thumb(TWinBody *pwb,TSkinBM *pSkinBM)
{ cwl();
  s32 BufWidth=pwb->Rect.w;
  
  u32 ofsx=pwb->ClientRect.x;
  u32 ofsy=pwb->ClientRect.y;
  
  u16 *pBuf=&pwb->WinBuf[0];
  
  if(pwb->WinBuf==NULL) return;
  
  for(s32 y=0;y<FS_DispCount;y++){ cwl();
    if(FS_RedrawFlag[y]==true){ cwl();
      FS_RedrawFlag[y]=false;
      
      u32 itemw=80;
      u32 itemh=58;
      u32 itemx=2+((y%3)*itemw);
      u32 itemy=1+((y/3)*itemh);
      
      for(u32 dy=0;dy<itemh;dy++){ cwl();
        u16 *pFileBM=pSkinBM->pFileBM;
        u32 FileBM_W=pSkinBM->FileBM_W;
        u16 *_pBuf=&pBuf[(ofsx+itemx)+((ofsy+itemy+dy)*BufWidth)];
        
        if(pFileBM!=NULL){
          pFileBM=&pFileBM[itemx+((itemy+dy)*FileBM_W)];
          MemCopy16DMA3(pFileBM,_pBuf,itemw*2);
          }else{
          MemSet16DMA3(MWC_FileSelectBG,_pBuf,itemw*2);
        }
        
        strpcmUpdate_mainloop();
      }
      
      u16 col=0;
      s32 idx=FS_TopIndex+y;
      
      if((0<=idx)&&(idx<FS_IndexCount)){ cwl();
        {
          u16 *pbm=thumbGetImage64(idx);
          
          if(pbm!=NULL){
            u16 *psrcbm=pbm;
            u16 *pdstbm=&pBuf[(ofsx+itemx)+((ofsy+itemy)*BufWidth)];
            
            pdstbm+=(itemw-64)/2;
            
            u32 ThumbAlpha;
            
            if(idx==FS_CursorIndex){
              ThumbAlpha=GlobalINI.Thumbnail.SelectItemAlpha;
              }else{
              ThumbAlpha=GlobalINI.Thumbnail.UnselectItemAlpha;
            }
            
            switch(ThumbAlpha){
              case 0: {
              } break;
              case 16: {
                for(u32 y=48;y!=0;y--){
                  MemCopy16DMA3(psrcbm,pdstbm,64*2);
                  psrcbm+=64;
                  pdstbm+=BufWidth;
                }
              } break;
              case 8: {
                for(u32 y=48;y!=0;y--){
                  for(u32 x=0;x<64;x++){
                    u16 col;
                    col=(psrcbm[x]&RGB15(30,30,30))+(pdstbm[x]&RGB15(30,30,30));
                    pdstbm[x]=(col>>1) | BIT15;
                  }
                  psrcbm+=64;
                  pdstbm+=BufWidth;
                }
              } break;
              default: {
                u32 sa=ThumbAlpha;
                u32 da=16-sa;
                for(u32 y=48;y!=0;y--){
                  for(u32 x=0;x<64;x++){
                    u32 scol=psrcbm[x];
                    u32 dcol=pdstbm[x];
                    
                    u32 r,g,b;
                    
                    r=(((scol>>0)&0x1f)*sa)+(((dcol>>0)&0x1f)*da);
                    g=(((scol>>5)&0x1f)*sa)+(((dcol>>5)&0x1f)*da);
                    b=(((scol>>10)&0x1f)*sa)+(((dcol>>10)&0x1f)*da);
                    
                    pdstbm[x]=RGB15(r>>4,g>>4,b>>4) | BIT15;
                  }
                  psrcbm+=64;
                  pdstbm+=BufWidth;
                }
              } break;
            }
            safefree(pbm); pbm=NULL;
          }
        }
        
        {
          if(idx==FS_PlayIndex) col=MWC_FileSelectPlay;
          if(idx==FS_CursorIndex) col=MWC_FileSelectCursor;
          
          if(col!=0){
            VRAM_SetCharLineRect(pBuf,BufWidth,ofsx+itemx,ofsy+itemy+(itemh-FontProHeight),itemw,FontProHeight,col);
            }else{
            col=MWC_FileSelectBG;
          }
        }
        
        if(col==MWC_FileSelectBG){ cwl();
          col=MWC_FileSelectText;
          }else{ cwl();
          col=MWC_FileSelectBG;
        }
        
        UnicodeChar fn[512];
        UnicodeChar *dst=&fn[0];
        
        {
          UnicodeChar *src=FileSys_GetFilename(idx);
          while(*src!=0){
            *dst++=*src++;
          }
          *dst=0;
        }
        
        if(FileSys_GetFileTrackNum(idx)!=-1){
          char srcarr[512];
          snprintf(srcarr,512,":%d",FileSys_GetFileTrackNum(idx));
          
          char *src=&srcarr[0];
          while(*src!=0){
            *dst++=(UnicodeChar)(*src++);
          }
          *dst=0;
        }
        
        VRAM_WriteStringRectW(pBuf,BufWidth,ofsx+itemx+itemw,ofsx+itemx+2,ofsy+itemy+(itemh-FontProHeight),fn,col);
      }
    }
    strpcmUpdate_mainloop();
//    while(strpcmUpdate_mainloop()==true){ }
  }
}

void MWinCallFileSelect_Draw(TWinBody *pwb,TSkinBM *pSkinBM)
{ cwl();
  if(GlobalINI.System.FileSelectSubScreen==true){
    PluginBodyClock_RequestRefresh=true;
  }
  
  { cwl();
    s32 LastTopIndex=FS_TopIndex;
    FS_TopIndex=MWin_GetSBarVPos(WM_FileSelect);
    if(LastTopIndex!=FS_TopIndex) FS_Redraw_SetTopIndex();
  }
  
  s32 BufWidth=pwb->Rect.w;
  
  s32 wLimit=pwb->ClientRect.x+pwb->ClientRect.w;
  u32 ofsx=pwb->ClientRect.x;
  u32 ofsy=pwb->ClientRect.y;
  
  u16 *pBuf=&pwb->WinBuf[0];
  
  if(pwb->WinBuf==NULL) return;
  
  if(FS_ExculsiveAllRedrawFlag==true){
    FS_ExculsiveAllRedrawFlag=false;
    for(s32 y=0;y<pwb->ClientRect.h;y++){ cwl();
      u16 *pFileBM=pSkinBM->pFileBM;
      u32 FileBM_W=pSkinBM->FileBM_W;
      u16 *_pBuf=&pBuf[ofsx+((ofsy+y)*BufWidth)];
      
      if(pFileBM!=NULL){
        pFileBM=&pFileBM[y*FileBM_W];
        MemCopy16DMA3(pFileBM,_pBuf,pwb->ClientRect.w*2);
        }else{
        MemSet16DMA3(MWC_FileSelectBG,_pBuf,pwb->ClientRect.w*2);
      }
    }
  }
  
  if(thumbExists()==true){
    MWinCallFileSelect_Draw_Thumb(pwb,pSkinBM);
    return;
  }
  
  s32 DispCount;
  if(FS_IndexCount<(FS_TopIndex+FS_DispCount)){
    DispCount=FS_IndexCount-FS_TopIndex;
    }else{
    DispCount=FS_DispCount;
  }
  
  for(s32 y=0;y<DispCount;y++){ cwl();
    if(FS_RedrawFlag[y]==true){ cwl();
      FS_RedrawFlag[y]=false;
      
      s32 idx=FS_TopIndex+y;
      
      {
        u16 *pFileBM=pSkinBM->pFileBM;
        u32 FileBM_W=pSkinBM->FileBM_W;
        
        if(pFileBM!=NULL){
          u16 *_pBuf=&pBuf[ofsx+((ofsy+(y*FontProHeight))*BufWidth)];
          pFileBM=&pFileBM[(y*FontProHeight)*FileBM_W];
          for(u32 y=0;y<FontProHeight;y++){
            MemCopy16DMA3(pFileBM,_pBuf,pwb->ClientRect.w*2);
            pFileBM+=FileBM_W;
            _pBuf+=BufWidth;
          }
          }else{
          u16 col=MWC_FileSelectBG;
          VRAM_SetCharLineRect(pBuf,BufWidth,ofsx,ofsy+(y*FontProHeight),pwb->ClientRect.w,FontProHeight,col);
        }
      }
      
      u16 textcol;
      
      if((idx!=FS_PlayIndex)&&(idx!=FS_CursorIndex)){
        textcol=MWC_FileSelectText;
        }else{
        textcol=MWC_FileSelectBG;
        
        u16 col=0;
        u32 *pBM32=NULL;
        const u32 bmw=pSkinBM->FileBarBM32_W; //,bmh=pSkinBM->FileBarBM32_H;
        if(idx==FS_PlayIndex){
          col=MWC_FileSelectPlay;
          pBM32=pSkinBM->pFileBarPlayBM32;
        }
        if(idx==FS_CursorIndex){
          col=MWC_FileSelectCursor;
          pBM32=pSkinBM->pFileBarCurrentBM32;
        }
        
        const u32 dstx=ofsx;
        const u32 dsty=ofsy+(y*FontProHeight);
        const u32 dstw=pwb->ClientRect.w;
        const u32 dsth=FontProHeight;
        
        if(pBM32!=NULL){
          for(u32 y=0;y<dsth;y++){
            u32 *psrc32=&pBM32[y*bmw];
            u16 *pdst16=&pBuf[((dsty+y)*BufWidth)+dstx];
            for(u32 x=0;x<dstw;x++){
              u32 src=psrc32[x];
              u32 sb=(src >> 0) & 0x1f;
              u32 sg=(src >> 8) & 0x1f;
              u32 sr=(src >> 16) & 0x1f;
              u32 sa=(src >> 24) & 0x1f;
              u32 dst=pdst16[x];
              u32 db=(dst >> 10) & 0x1f;
              u32 dg=(dst >> 5) & 0x1f;
              u32 dr=(dst >> 0) & 0x1f;
              u32 da=32-sa;
              
              db=(db*da/32)+(sb*sa/32);
              dg=(dg*da/32)+(sg*sa/32);
              dr=(dr*da/32)+(sr*sa/32);
              
              pdst16[x]=RGB15(dr,dg,db) | BIT15;
            }
          }
          }else{
          if(col!=0){
            VRAM_SetCharLineRect(pBuf,BufWidth,dstx,dsty,dstw,dsth,col);
            }else{
          }
        }
      }
      
      if((0<=idx)&&(idx<FS_IndexCount)){ cwl();
        u32 fsizestrWidth;
        
        { cwl();
          u32 fsize=FileSys_GetFileDataSize(idx);
          char fsizestr[256];
          if(fsize!=0){ cwl();
            sprintf(fsizestr,"%d",fsize);
            fsizestrWidth=VRAM_GetStringWidth(fsizestr);
            }else{ cwl();
            fsizestr[0]=(char)NULL;
            fsizestrWidth=0;
          }
          
          VRAM_WriteStringRect(pBuf,BufWidth,wLimit,wLimit-1-fsizestrWidth,ofsy+(y*FontProHeight),fsizestr,textcol);
        }
        
        UnicodeChar fn[512];
        UnicodeChar *dst=&fn[0];
        
        {
          UnicodeChar *src=FileSys_GetFilename(idx);
          while(*src!=0){
            *dst++=*src++;
          }
          *dst=0;
        }
        
        if(FileSys_GetFileTrackNum(idx)!=-1){
          char srcarr[512];
          snprintf(srcarr,512,":%d",FileSys_GetFileTrackNum(idx));
          
          char *src=&srcarr[0];
          while(*src!=0){
            *dst++=(UnicodeChar)(*src++);
          }
          *dst=0;
        }
        
        VRAM_WriteStringRectW(pBuf,BufWidth,wLimit-1-fsizestrWidth-1,ofsx+4,ofsy+(y*FontProHeight),fn,textcol);
      }
    }
    strpcmUpdate_mainloop();
//    while(strpcmUpdate_mainloop()==true){ }
  }
  
}

void MWinCallDateTime_Draw(TWinBody *pwb)
{ cwl();
}

void MWinCallPicView_Draw(TWinBody *pwb)
{ cwl();
  if(pwb->WinBuf==NULL) return;
  
  Pic_SetPos(MWin_GetSBarHPos(WM_PicView),MWin_GetSBarVPos(WM_PicView));
  Pic_RefreshScreen();
}

void MWinCallTextView_Draw(TWinBody *pwb)
{ cwl();
  if(pwb->WinBuf==NULL) return;
  
  Text_SetLine(MWin_GetSBarVPos(WM_TextView),pwb->ClientRect);
  Text_RefreshScreen(pwb->WinBuf,pwb->Rect.w,pwb->ClientRect);
  
  {
    char str[256];
    sprintf(str,"TextView... %d/%d",Text_GetLine(),Text_GetLineCount());
    MWin_SetWindowTitle(WM_TextView,str);
    MWin_DrawTitleBar(WM_TextView);
  }
}

// MouseCallback

void MWinCallAbout_MouseDown(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseDown(WinIndex,x,y);
}

void MWinCallAbout_MouseMove(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseMove(WinIndex,x,y);
}

void MWinCallAbout_MouseUp(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseUp(WinIndex,x,y);
}

void MWinCallHelp_MouseDown(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseDown(WinIndex,x,y);
}

void MWinCallHelp_MouseMove(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseMove(WinIndex,x,y);
}

void MWinCallHelp_MouseUp(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseUp(WinIndex,x,y);
}

void MWinCallPlayControl_MouseDown(u32 WinIndex,s32 x,s32 y)
{ cwl();
  if(y<FontProHeight){ cwl();
    MWinSetNext_LastMusicNext=-1;
    MWin_SetVisible(WM_SetNext,true);
    u32 WinIndex=MWin_SetActive(WM_SetNext);
    MWin_RefreshScreenMask();
    if(WinIndex!=(u32)-1) MWin_RefreshWindow(WinIndex);
    MWin_RefreshWindow(WM_SetNext);
    return;
  }
  
  s32 fsize=0,fofs=0;
  
  switch(ExecMode){ cwl();
    case EM_MSPSound: { cwl();
      fsize=pPluginBody->pSL->GetPosMax();
    } break;
    case EM_GMENSF: { cwl();
      fsize=GMENSF_GetPosMax();
    } break;
    case EM_GMEGBS: { cwl();
      fsize=GMEGBS_GetPosMax();
    } break;
/*
    case EM_GMEVGM: { cwl();
      fsize=GMEVGM_GetPosMax();
    } break;
    case EM_GMEGYM: { cwl();
      fsize=GMEGYM_GetPosMax();
    } break;
*/
    case EM_DPG: { cwl();
      fsize=DPG_GetTotalFrameCount();
    } break;
    default: break;
  }
  
  if(fsize==0) return;
  
  { cwl();
    s32 w=MWin_GetClientWidth(WinIndex);
    s32 h=FontProHeight;
    
    if(y<(MWin_GetClientHeight(WinIndex)-h)) return;
    
    fofs=(s32)(x*((float)fsize/w));
    
    if(fofs<0) fofs=0;
    if(fsize<=fofs) fofs=fsize-1;
  }
  
  switch(ExecMode){ cwl();
    case EM_MSPSound: { cwl();
      pPluginBody->pSL->SetPosOffset(fofs);
    } break;
    case EM_GMENSF: { cwl();
      GMENSF_SetPosOffset(fofs);
    } break;
    case EM_GMEGBS: { cwl();
      GMEGBS_SetPosOffset(fofs);
    } break;
/*
    case EM_GMEVGM: { cwl();
      GMEVGM_SetPosOffset(fofs);
    } break;
    case EM_GMEGYM: { cwl();
      GMEGYM_SetPosOffset(fofs);
    } break;
*/
    case EM_DPG: { cwl();
      strpcmStop();
      DPG_SetFrameCount(fofs);
      DPG_RequestSyncStart=true;
    } break;
    default: break;
  }
  
  MWin_DrawClient(WinIndex);
  MWin_TransWindow(WinIndex);
}

void MWinCallPlayControl_MouseMove(u32 WinIndex,s32 x,s32 y)
{ cwl();
}

void MWinCallPlayControl_MouseUp(u32 WinIndex,s32 x,s32 y)
{ cwl();
}

void MWinCallFileSelect_MouseDown(u32 WinIndex,s32 x,s32 y)
{ cwl();
  if(y<0){ cwl();
    y=(y/FontProHeight)-1;
    }else{ cwl();
    y=y/FontProHeight;
  }
  
  FS_SetCursor(FS_TopIndex+y);
  MWin_SetSBarVPos(WinIndex,FS_TopIndex);
  MWin_DrawSBarVH(WinIndex);
  MWin_DrawClient(WinIndex);
  MWin_TransWindow(WinIndex);
}

void MWinCallFileSelect_MouseMove(u32 WinIndex,s32 x,s32 y)
{ cwl();
  MWinCallFileSelect_MouseDown(WinIndex,x,y);
}

void MWinCallFileSelect_MouseUp(u32 WinIndex,s32 x,s32 y)
{ cwl();
  if(y<0){ cwl();
    y=(y/FontProHeight)-1;
    }else{ cwl();
    y=y/FontProHeight;
  }
  
  y=FS_TopIndex+y;
  
  if(FS_CursorIndex==y){ cwl();
    FS_StartFromCursor();
  }
}

void MWinCallDateTime_MouseDown(u32 WinIndex,s32 x,s32 y)
{ cwl();
}

void MWinCallDateTime_MouseMove(u32 WinIndex,s32 x,s32 y)
{ cwl();
}

void MWinCallDateTime_MouseUp(u32 WinIndex,s32 x,s32 y)
{ cwl();
}

void MWinCallPicView_MouseDown(u32 WinIndex,s32 x,s32 y)
{ cwl();
  if(ExecMode==EM_None) return;
  
  if(ExecMode==EM_DPG){ cwl();
    if(MWin_GetVideoFullScreen()==false){ cwl();
      MWin_SetVideoFullScreen(true);
      MWin_RefreshVideoFullScreen(false);
    }
    return;
  }
  
  PicView_LastX=x;
  PicView_LastY=y;
}

void MWinCallPicView_MouseMove(u32 WinIndex,s32 x,s32 y)
{ cwl();
  if(ExecMode==EM_None) return;
  
  if(ExecMode==EM_DPG) return;
  
  s32 PicView_BaseX,PicView_BaseY;
  
  // FullScreenPicture
  TRect ClientRect=ImageRect;
  
  PicView_BaseX=Pic_GetPosX()-(int)((x-PicView_LastX)*2/Pic_GetRatio());
  PicView_BaseY=Pic_GetPosY()-(int)((y-PicView_LastY)*2/Pic_GetRatio());
  Pic_SetPos(PicView_BaseX,PicView_BaseY);
  MWin_SetSBarHPos(WM_PicView,Pic_GetPosX());
  MWin_SetSBarVPos(WM_PicView,Pic_GetPosY());
  
  MWin_DrawSBarVH(WinIndex);
  MWin_DrawClient(WinIndex);
  MWin_TransWindow(WinIndex);
  
  PicView_LastX=x;
  PicView_LastY=y;
}

void MWinCallPicView_MouseUp(u32 WinIndex,s32 x,s32 y)
{ cwl();
  if(ExecMode==EM_None) return;
  
  if(ExecMode==EM_DPG) return;
  
  PicView_LastX=0;
  PicView_LastY=0;
}

void MWinCallTextView_MouseDown(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseDown(WinIndex,x,y);
}

void MWinCallTextView_MouseMove(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseMove(WinIndex,x,y);
}

void MWinCallTextView_MouseUp(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseUp(WinIndex,x,y);
}

void MWinCallSetNext_Draw(TWinBody *pwb)
{ cwl();
  u16 col=MWC_ClientText;
  
  s32 BufWidth=pwb->Rect.w;
  
  s32 wLimit=pwb->ClientRect.x+pwb->ClientRect.w;
  u32 ofsx=pwb->ClientRect.x;
  u32 ofsy=pwb->ClientRect.y;
  
  u16 *pBuf=&pwb->WinBuf[0];
  
  if(pwb->WinBuf==NULL) return;
  
  for(int idx=0;idx<MusicNext_Count;idx++){
    u16 bgcol=0;
    if(idx==(int)GlobalINI.System.MusicNext) bgcol=MWC_FrameBright;
    if(idx==MWinSetNext_LastMusicNext) bgcol=MWC_FrameDark;
    if(bgcol!=0){
      VRAM_SetCharLineRect(pBuf,BufWidth,ofsx,ofsy+(idx*FontProHeight),pwb->ClientRect.w,FontProHeight,bgcol);
    }
    
    const char *pstr=NULL;
    
    if(DefaultCodePage!=CP932){
      switch(idx){ cwl();
        case MusicNext_Stop: pstr="Stop"; break;
        case MusicNext_Repeat: pstr="OneRepeat"; break;
        case MusicNext_NormalLoop: pstr="NormalLoop"; break;
        case MusicNext_NormalPOff: pstr="NormalPowOff"; break;
        case MusicNext_ShuffleLoop: pstr="ShuffleLoop"; break;
        case MusicNext_ShufflePOff: pstr="ShufflePowOff"; break;
        case MusicNext_PowerOff: pstr="End PowOFF"; break;
        default: pstr=NULL;
      }
      }else{
      switch(idx){ cwl();
        case MusicNext_Stop: pstr="一曲停止"; break;
        case MusicNext_Repeat: pstr="一曲リピート"; break;
        case MusicNext_NormalLoop: pstr="順ループ"; break;
        case MusicNext_NormalPOff: pstr="順ループOFF"; break;
        case MusicNext_ShuffleLoop: pstr="ランダムループ"; break;
        case MusicNext_ShufflePOff: pstr="ランダムOFF"; break;
        case MusicNext_PowerOff: pstr="一曲OFF"; break;
        default: pstr=NULL;
      }
    }
    
    if(pstr!=NULL){
      int textwidth=VRAM_GetStringWidthUTF8(pstr);
      if(pwb->ClientRect.w<textwidth) textwidth=pwb->ClientRect.w;
      VRAM_WriteStringRectUTF8(pBuf,BufWidth,wLimit,ofsx+((pwb->ClientRect.w-textwidth)/2),ofsy+(idx*FontProHeight),pstr,col);
    }
  }
}

void MWinCallSetNext_MouseDown(u32 WinIndex,s32 x,s32 y)
{ cwl();
  MWinCallSetNext_MouseMove(WinIndex,x,y);
}

void MWinCallSetNext_MouseMove(u32 WinIndex,s32 x,s32 y)
{ cwl();
  TWinBody *pwb=MWin_GetWinBody(WinIndex);
  
  int back=MWinSetNext_LastMusicNext;
  
  if((0<=x)&&(x<pwb->ClientRect.w)){
    MWinSetNext_LastMusicNext=y/FontProHeight;
    }else{
    MWinSetNext_LastMusicNext=-1;
  }
  
  if(back==MWinSetNext_LastMusicNext) return;
  
  MWin_DrawClient(WM_SetNext);
  MWin_TransWindow(WM_SetNext);
}

void MWinCallSetNext_MouseUp(u32 WinIndex,s32 x,s32 y)
{ cwl();
  MWinCallSetNext_MouseMove(WinIndex,x,y);
  
  if((MWinSetNext_LastMusicNext<0)||(MusicNext_Count<=MWinSetNext_LastMusicNext)) return;
  
  GlobalINI.System.MusicNext=(u32)MWinSetNext_LastMusicNext;
  if(MWin_GetVisible(WM_PlayControl)==true){
    MWin_DrawClient(WM_PlayControl);
    MWin_TransWindow(WM_PlayControl);
  }
  
  MWin_SetVisible(WM_SetNext,false);
  if(MWin_GetVisible(WM_PlayControl)==true){
    MWin_SetActive(WM_PlayControl);
    }else{
    MWin_SetActiveTopMost();
  }
  MWin_RefreshScreenMask();
  MWin_AllRefresh();
}

void MWinCallFileInfo_Draw(TWinBody *pwb)
{ cwl();
  if(PlugInfo_GetShowFlag()==false) return;
  
  u32 TopIndex=MWin_GetSBarVPos(WM_FileInfo);
  
  s32 BufWidth=pwb->Rect.w;
  u16 col=MWC_ClientText;
  
  s32 wLimit=pwb->ClientRect.x+pwb->ClientRect.w;
  u32 ofsx=pwb->ClientRect.x+4;
  u32 ofsy=pwb->ClientRect.y+2;
  
  u16 *pBuf=&pwb->WinBuf[0];
  
  if(pwb->WinBuf==NULL) return;
  
  for(s32 y=0;y<(pwb->ClientRect.h-2)/FontProHeight;y++){ cwl();
    s32 idx=TopIndex+y;
    
    {
      char str[256]={0,};
      if(PlugInfo_GetInfoStrL(idx,str,256)==true){
        VRAM_WriteStringRectL(pBuf,BufWidth,wLimit,ofsx,ofsy+(y*FontProHeight),str,col);
      }
    }
	
    {
      UnicodeChar str[256]={0,};
      if(PlugInfo_GetInfoStrW(idx,str,256)==true){
        VRAM_WriteStringRectW(pBuf,BufWidth,wLimit,ofsx,ofsy+(y*FontProHeight),str,col);
      }
    }
    {
      char str[256*2]={0,};
      if(PlugInfo_GetInfoStrUTF8(idx,str,256)==true){
        VRAM_WriteStringRectUTF8(pBuf,BufWidth,wLimit,ofsx,ofsy+(y*FontProHeight),str,col);
      }
    }
    
  }
}

void MWinCallFileInfo_MouseDown(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseDown(WinIndex,x,y);
}

void MWinCallFileInfo_MouseMove(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseMove(WinIndex,x,y);
}

void MWinCallFileInfo_MouseUp(u32 WinIndex,s32 x,s32 y)
{ cwl();
  Global_MouseUp(WinIndex,x,y);
}
