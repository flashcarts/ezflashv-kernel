
static bool PlugInfo_CustomMessageFlag=false;
static char PlugInfo_CustomMessage0[64]={0,};
static char PlugInfo_CustomMessage1[64]={0,};
static char PlugInfo_CustomMessage2[64]={0,};
static char PlugInfo_CustomMessage3[64]={0,};
static char PlugInfo_CustomMessage4[64]={0,};

static void PlugInfo_InitCustomMessage(bool Enabled)
{
  PlugInfo_CustomMessageFlag=Enabled;
  
  PlugInfo_CustomMessage0[0]=(char)0;
  PlugInfo_CustomMessage1[0]=(char)0;
  PlugInfo_CustomMessage2[0]=(char)0;
  PlugInfo_CustomMessage3[0]=(char)0;
  PlugInfo_CustomMessage4[0]=(char)0;
}

static void PlugInfo_ShowCustomMessage_LoadError(void)
{
  PlugInfo_InitCustomMessage(true);
  if(DefaultCodePage!=CP932){
    snprintf(PlugInfo_CustomMessage0,64,"load error.");
    snprintf(PlugInfo_CustomMessage1,64,"Press START button.");
    snprintf(PlugInfo_CustomMessage2,64,"Show start log.");
    }else{
    snprintf(PlugInfo_CustomMessage0,64,"ファイルが正常に開けませんでした。");
    snprintf(PlugInfo_CustomMessage1,64,"STARTボタンを押して、");
    snprintf(PlugInfo_CustomMessage2,64,"プラグインのログを確認してください。");
  }
}

static bool PlugInfo_GetShowFlag(void)
{
  if(PlugInfo_CustomMessageFlag==true) return(true);
  
  switch(ExecMode){ cwl();
    case EM_None: return(true); break;
    case EM_FPK: return(true); break;
    case EM_MSPSound: return(true); break;
    case EM_MSPImage: return(true); break;
    case EM_GMENSF: return(true); break;
    case EM_GMEGBS: return(true); break;
//    case EM_GMEVGM: return(true); break;
//    case EM_GMEGYM: return(true); break;
    case EM_DPG: return(true); break;
    default: return(false); break;
  }
  return(false);
}

static int PlugInfo_GetInfoIndexCount(void)
{
  if(PlugInfo_CustomMessageFlag==true) return(5);
  
  switch(ExecMode){ cwl();
    case EM_None: return(1); break;
    case EM_FPK: return(FPK_GetInfoIndexCount()); break;
    case EM_MSPSound: return(pPluginBody->pSL->GetInfoIndexCount()); break;
    case EM_MSPImage: return(pPluginBody->pIL->GetInfoIndexCount()); break;
    case EM_GMENSF: return(GMENSF_GetInfoIndexCount()); break;
    case EM_GMEGBS: return(GMEGBS_GetInfoIndexCount()); break;
//    case EM_GMEVGM: return(GMEVGM_GetInfoIndexCount()); break;
//    case EM_GMEGYM: return(GMEGYM_GetInfoIndexCount()); break;
    case EM_DPG: return(DPG_GetInfoIndexCount()); break;
    default: return(0); break;
  }
  return(0);
}


static bool PlugInfo_GetInfoStrL(int idx,char *str,int len)
{
  if(PlugInfo_CustomMessageFlag==true) return(false);
  
  switch(ExecMode){ cwl();
    case EM_None: {
      if(idx==0){
        snprintf(str,len,"not support file format.");
        return(true);
      }
      return(false);
    } break;
    case EM_FPK: return(FPK_GetInfoStrL(idx,str,len)); break;
    case EM_MSPSound: return(pPluginBody->pSL->GetInfoStrL(idx,str,len)); break;
    case EM_MSPImage: return(pPluginBody->pIL->GetInfoStrL(idx,str,len)); break;
    case EM_GMENSF: return(GMENSF_GetInfoStrL(idx,str,len)); break;
    case EM_GMEGBS: return(GMEGBS_GetInfoStrL(idx,str,len)); break;
//    case EM_GMEVGM: return(GMEVGM_GetInfoStrL(idx,str,len)); break;
//    case EM_GMEGYM: return(GMEGYM_GetInfoStrL(idx,str,len)); break;
    case EM_DPG: return(DPG_GetInfoStrL(idx,str,len)); break;
    default: return(false); break;
  }
  return(false);
}

static bool PlugInfo_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  if(PlugInfo_CustomMessageFlag==true) return(false);
  
  switch(ExecMode){ cwl();
    case EM_None: return(false); break;
    case EM_FPK: return(FPK_GetInfoStrW(idx,str,len)); break;
    case EM_MSPSound: return(pPluginBody->pSL->GetInfoStrW(idx,str,len)); break;
    case EM_MSPImage: return(pPluginBody->pIL->GetInfoStrW(idx,str,len)); break;
    case EM_GMENSF: return(GMENSF_GetInfoStrW(idx,str,len)); break;
    case EM_GMEGBS: return(GMEGBS_GetInfoStrW(idx,str,len)); break;
//    case EM_GMEVGM: return(GMEVGM_GetInfoStrW(idx,str,len)); break;
//    case EM_GMEGYM: return(GMEGYM_GetInfoStrW(idx,str,len)); break;
    case EM_DPG: return(DPG_GetInfoStrW(idx,str,len)); break;
    default: return(false); break;
  }
  return(false);
}

static bool PlugInfo_GetInfoStrUTF8(int idx,char *str,int len)
{
  if(PlugInfo_CustomMessageFlag==true){
    switch(idx){
      case 0: snprintf(str,len,"%s",PlugInfo_CustomMessage0); return(true);
      case 1: snprintf(str,len,"%s",PlugInfo_CustomMessage1); return(true);
      case 2: snprintf(str,len,"%s",PlugInfo_CustomMessage2); return(true);
      case 3: snprintf(str,len,"%s",PlugInfo_CustomMessage3); return(true);
      case 4: snprintf(str,len,"%s",PlugInfo_CustomMessage4); return(true);
    }
    return(false);
  }
  
  switch(ExecMode){ cwl();
    case EM_None: return(false); break;
    case EM_FPK: return(FPK_GetInfoStrUTF8(idx,str,len)); break;
    case EM_MSPSound: return(pPluginBody->pSL->GetInfoStrUTF8(idx,str,len)); break;
    case EM_MSPImage: return(pPluginBody->pIL->GetInfoStrUTF8(idx,str,len)); break;
    case EM_GMENSF: return(GMENSF_GetInfoStrUTF8(idx,str,len)); break;
    case EM_GMEGBS: return(GMEGBS_GetInfoStrUTF8(idx,str,len)); break;
//    case EM_GMEVGM: return(GMEVGM_GetInfoStrUTF8(idx,str,len)); break;
//    case EM_GMEGYM: return(GMEGYM_GetInfoStrUTF8(idx,str,len)); break;
    case EM_DPG: return(DPG_GetInfoStrUTF8(idx,str,len)); break;
    default: return(false); break;
  }
  return(false);
}

