
static u32 KeyRepeatLastKey;
static bool KeyRepeatFlag;
static u32 KeyRepeatCount;

void KeyRepeat_Flash(void)
{
  KeyRepeatLastKey=0;
  KeyRepeatFlag=false;
}

u32 KeyRepeat_On(u32 NowKey)
{
  if(NowKey!=KeyRepeatLastKey) KeyRepeatFlag=false;
  KeyRepeatLastKey=NowKey;
  
  if(KeyRepeatFlag==false){ cwl();
    KeyRepeatFlag=true;
    KeyRepeatCount=GlobalINI.KeyRepeat.DelayCount;
    }else{ cwl();
    if(KeyRepeatCount==0){ cwl();
      KeyRepeatCount=GlobalINI.KeyRepeat.RateCount;
      }else{ cwl();
      NowKey=0;
    }
  }
  
  return(NowKey);
}

u32 KeyRepeat_Proc(u32 NowKey)
{
  if(NowKey==0){ cwl();
    KeyRepeat_Flash();
    }else{ cwl();
    NowKey=KeyRepeat_On(NowKey);
  }
  
  return(NowKey);
}

void KeyRepeat_Delay(u32 Multiple)
{
  KeyRepeatCount=GlobalINI.KeyRepeat.DelayCount*Multiple;
}

