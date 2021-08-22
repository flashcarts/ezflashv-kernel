unit _encaudio;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, StdCtrls;

function DemuxAudio(avifn:string;wavfn:string;altmode:boolean):boolean;
function EncodeMP2(srcfn,dstfn:string;freq:integer;kbps:integer):boolean;

function DemuxAudio_HQ32768Hz_MEncoder(avifn:string;wavfn:string;Volume:integer):boolean;
function EncodeMP2_HQ32768Hz_twolame(srcfn,dstfn:string;freq:integer;kbps:integer):boolean;

function DirectEncodeMP2_MEncoder(srcfn,dstfn:string;freq:integer;kbps:integer):boolean;
function DirectEncodeMP2_FFMpeg(srcfn,dstfn:string;freq:integer;kbps:integer):boolean;

var
  encaudio_StartPath:string;
  encaudio_PluginPath:string;

implementation

uses _m_Tools,_queue,_dosbox,dpgenc_language,MainWin,enclogWin;

const CRLF:string=char($0d)+char($0a);

function DemuxAudio(avifn:string;wavfn:string;altmode:boolean):boolean;
var
  appfn:string;
  cmdline:string;
begin
  appfn:=encaudio_PluginPath+'extractwave.exe';

  if fileexists(appfn)=False then begin
    Current_SetError('not found plugin.',appfn);
    Result:=False;
    exit;
  end;

  cmdline:='"'+avifn+'" "'+wavfn+'"';
  if altmode=True then cmdline:=cmdline+' -A';

  CreateDOSBOX_UseCMD(encaudio_StartPath,appfn,cmdline);

  if GetFileSize(wavfn)=0 then begin
    Current_SetError('error func DemuxAudio','');
    Result:=False;
    exit;
  end;

  Result:=True;
end;

function EncodeMP2(srcfn,dstfn:string;freq:integer;kbps:integer):boolean;
var
  appfn:string;
begin
  appfn:=encaudio_PluginPath+'twolame.exe';

  if fileexists(appfn)=False then begin
    Current_SetError('not found plugin.',appfn);
    Result:=False;
    exit;
  end;

  SetMainTitle('Encode Wave to MP2... (use twolame)');
  CreateDOSBOX_UseCMD(encaudio_StartPath,appfn,format('-b %d -m j "'+srcfn+'" "'+dstfn+'"',[kbps]));

  if GetFileSize(dstfn)=0 then begin
    Current_SetError('error func EncodeMP2','');
    Result:=False;
    exit;
  end;

  Result:=True;
end;

const RiffHeaderSize=11;

function DemuxAudio_HQ32768Hz_MEncoder(avifn:string;wavfn:string;Volume:integer):boolean;
var
  hInputRead,hInputWrite:THANDLE;
  hOutputRead,hOutputWrite:THANDLE;
  ErrorStr:string;
  PipeBufStr:string;
  PipeBufFlag:boolean;
  lasttick:dword;
  CaptionText:string;
  perpos:integer;
  perstr:string;
  perint:integer;
  function CreatePipes(ReadBufSize:dword):boolean;
  var
    SA:SECURITY_ATTRIBUTES;
    SD:TSecurityDescriptor;
  begin
    hInputRead:=0;
    hInputWrite:=0;
    hOutputRead:=0;
    hOutputWrite:=0;

    sa.nLength:=sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor:=nil;
    sa.bInheritHandle:=True;
    InitializeSecurityDescriptor(@SD,SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(@SD,True,nil,False);
    sa.lpSecurityDescriptor:=@SD;

    if CreatePipe(hInputRead,hInputWrite,@sa,ReadBufSize)=False then begin
      Current_SetError(lng(LI_PipeErrorCreate),'');
      Result:=False;
      exit;
    end;
    if DuplicateHandle(GetCurrentProcess(),hInputWrite,GetCurrentProcess(),nil,0,False,DUPLICATE_SAME_ACCESS)=False then begin
      Current_SetError(lng(LI_PipeErrorAttribute),'');
      Result:=False;
      exit;
    end;

    if CreatePipe(hOutputRead,hOutputWrite,@sa,0)=False then begin
      Current_SetError(lng(LI_PipeErrorCreate),'');
      Result:=False;
      exit;
    end;
    if DuplicateHandle(GetCurrentProcess(),hOutputRead,GetCurrentProcess(),nil,0,False,DUPLICATE_SAME_ACCESS)=False then begin
      Current_SetError(lng(LI_PipeErrorAttribute),'');
      Result:=False;
      exit;
    end;

    Result:=True;
  end;
  function StartEncode:boolean;
  var
    appfn:string;
    cmdline:string;
  begin
    appfn:=encaudio_PluginPath+'mencoder.exe';

    if fileexists(appfn)=False then begin
      Current_SetError('not found plugin.',appfn);
      Result:=False;
      exit;
    end;

    if CreatePipes(1024)=False then begin
      Result:=False;
      exit;
    end;

    cmdline:=format('-noautosub -ovc copy -af format=s16le,resample=32768:1:2,channels=2,volume=%d:1 -oac pcm -of rawaudio',[Volume]);
    cmdline:='-v "'+avifn+'" '+cmdline+' -o "'+wavfn+'"';
    enclog.loglst.Lines.Add(appfn);
    enclog.loglst.Lines.Add(cmdline);
    enclog.loglst.Lines.Add('');

    if CreateDOSBOX2(encaudio_StartPath,hInputRead,hOutputWrite,hOutputWrite,appfn,cmdline)=False then begin
      Current_SetError('CreateDOSBOX error.','');
      Result:=False;
      exit;
    end;

    Result:=True;
  end;
  function ReadPipe(hnd:THANDLE):string;
  var
    ansistr:array[0..1024] of ansichar;
    i:integer;
    len:dword;
    readsize:dword;
    c:ansichar;
  begin
    Result:='';

    len:=0;
    if PeekNamedPipe(hnd, nil, 0, nil,@len,nil)=True then begin
      if len<>0 then begin
        if 1024<=len then len:=1024;
        if ReadFile(hnd,ansistr[0],len,readsize,nil)=True then begin
          for i:=0 to readsize-1 do begin
            c:=ansistr[i];
            if PipeBufFlag=False then begin
              if c=ansichar($0d) then begin
                PipeBufFlag:=True;
                end else begin
                PipeBufStr:=PipeBufStr+c;
              end;
              end else begin
              PipeBufFlag:=False;
              if c=ansichar($0a) then begin
                Result:=Result+PipeBufStr+CRLF;
                PipeBufStr:='';
                end else begin
                CaptionText:=PipeBufStr;
                PipeBufStr:=c;
              end;
            end;
          end;
        end;
      end;
    end;
  end;
  procedure EndEncode;
  begin
    if Current_GetRequestCancel=False then begin
      CloseDOSBOX2(True); // I—¹‚ð‘Ò‚Â
      end else begin
      CloseDOSBOX2(False);
    end;

    ErrorStr:=ReadPipe(hOutputRead);
    if ErrorStr<>'' then begin
      enclog.loglst.Lines.Add(ErrorStr);
      enclog.loglst.Refresh;
    end;
    try
      enclog.loglst.Lines.SaveToFile(changefileext(Application.ExeName,'')+'_mencoder.log');
      except else begin
      end;
    end;

    CloseHandle(hInputRead);
    CloseHandle(hInputWrite);
    CloseHandle(hOutputRead);
    CloseHandle(hOutputWrite);
  end;
  procedure RewriteWaveHeader(rate:integer);
  var
    Count:integer;
    wfs:TFileStream;
    RiffHeader:array[0..RiffHeaderSize-1] of dword;
    fsize:integer;
    function SwapHiLow(d:dword):dword;
    begin
      Result:=dword((int64(d) div $1000000 and $FF)+((int64(d) div $10000 and $FF)*$100)+((int64(d) div $100 and $FF)*$10000)+((int64(d) and $FF)*$1000000));
    end;
  begin
    // RiffWave Header of 44.1khz 16bit stereo
    RiffHeader[ 0]:=$52494646; // RIFF Header
    RiffHeader[ 1]:=$00000000; // TotalFileSize-8;
    RiffHeader[ 2]:=$57415645; // WAVE Header
    RiffHeader[ 3]:=$666D7420; // fmt  Header
    RiffHeader[ 4]:=$10000000;
    RiffHeader[ 5]:=$01000200; // (word)wFormatTag,(word)nChannels
    RiffHeader[ 6]:=$44AC0000; // (dword)nSamplesPerSec(44kHz)
    RiffHeader[ 7]:=$10B10200; // (dword)nAvgBytesPerSec(44kHz*4)
    RiffHeader[ 8]:=$04001000; // (word)nBlockAlign,(word)wBitsPerSample
    RiffHeader[ 9]:=$64617461; // data Header
    RiffHeader[10]:=$00000000; // WaveSize (bytesize)

    RiffHeader[ 6]:=SwapHiLow(rate);
    RiffHeader[ 7]:=SwapHiLow(rate*4);

    fsize:=GetFileSize(wavfn);
    RiffHeader[ 1]:=SwapHiLow(fsize-8);
    RiffHeader[10]:=SwapHiLow(fsize-44);

    // Swap Hi Low
    for Count:=0 to RiffHeaderSize-1 do begin
      RiffHeader[Count]:=SwapHiLow(RiffHeader[Count]);
    end;

    wfs:=TFileStream.Create(wavfn,fmOpenReadWrite);
    wfs.WriteBuffer(RiffHeader[0],RiffHeaderSize*4);
    wfs.Free;
  end;
begin
  if StartEncode=False then begin
    Result:=False;
    exit;
  end;

  SetMainTitle('High quality decode to wave... (use mencoder)');

  SetPrgBarPos(0,'');
  SetPrgBarMax(100);

  PipeBufStr:='';
  PipeBufFlag:=False;
  CaptionText:='';

  lasttick:=GetTickCount-1000;

  while(isTerminatedDOSBOX2=False) do begin
    if (1000<=(GetTickCount-lasttick)) then begin
      lasttick:=GetTickCount;
      if CaptionText<>'' then begin
        PerStr:=CaptionText;
        PerPos:=ansipos(' (',PerStr);
        if PerPos<>0 then begin
          PerStr:=copy(PerStr,PerPos+2,length(PerStr));
          PerPos:=ansipos('%) ',PerStr);
          if PerPos<>0 then begin
            PerStr:=copy(PerStr,1,PerPos-1);
            PerInt:=strtointdef(PerStr,0);
            SetPrgBarPos(PerInt,'');
          end;
        end;
        enclog.Caption:=CaptionText;
        CaptionText:='';
      end;
    end;

    ErrorStr:=ReadPipe(hOutputRead);
    if ErrorStr<>'' then begin
      enclog.loglst.Lines.Add(ErrorStr);
      enclog.loglst.Refresh;
    end;

    Application.ProcessMessages;
    if Current_GetRequestCancel=True then break;
  end;

  enclog.Caption:='Encode Terminate';
  SetPrgBarPos(0,'Encode Terminate');

  EndEncode;

  if Current_GetRequestCancel=True then begin
    Result:=False;
    exit;
  end;

  if GetFileSize(wavfn)=0 then begin
    Current_SetError('error func DemuxAudio_HQ32768Hz_MEncoder','');
    Result:=False;
    exit;
  end;

  RewriteWaveHeader(32000);

  Result:=True;
end;

{
function DemuxAudio_HQ32768Hz_MEncoder(avifn:string;wavfn:string):boolean;
var
  appfn:string;
  cmdline:string;
  procedure RewriteWaveHeader(rate:integer);
  var
    Count:integer;
    wfs:TFileStream;
    RiffHeader:array[0..RiffHeaderSize-1] of dword;
    fsize:integer;
    function SwapHiLow(d:dword):dword;
    begin
      Result:=dword((int64(d) div $1000000 and $FF)+((int64(d) div $10000 and $FF)*$100)+((int64(d) div $100 and $FF)*$10000)+((int64(d) and $FF)*$1000000));
    end;
  begin
    // RiffWave Header of 44.1khz 16bit stereo
    RiffHeader[ 0]:=$52494646; // RIFF Header
    RiffHeader[ 1]:=$00000000; // TotalFileSize-8;
    RiffHeader[ 2]:=$57415645; // WAVE Header
    RiffHeader[ 3]:=$666D7420; // fmt  Header
    RiffHeader[ 4]:=$10000000;
    RiffHeader[ 5]:=$01000200; // (word)wFormatTag,(word)nChannels
    RiffHeader[ 6]:=$44AC0000; // (dword)nSamplesPerSec(44kHz)
    RiffHeader[ 7]:=$10B10200; // (dword)nAvgBytesPerSec(44kHz*4)
    RiffHeader[ 8]:=$04001000; // (word)nBlockAlign,(word)wBitsPerSample
    RiffHeader[ 9]:=$64617461; // data Header
    RiffHeader[10]:=$00000000; // WaveSize (bytesize)

    RiffHeader[ 6]:=SwapHiLow(rate);
    RiffHeader[ 7]:=SwapHiLow(rate*4);

    fsize:=GetFileSize(wavfn);
    RiffHeader[ 1]:=SwapHiLow(fsize-8);
    RiffHeader[10]:=SwapHiLow(fsize-44);

    // Swap Hi Low
    for Count:=0 to RiffHeaderSize-1 do begin
      RiffHeader[Count]:=SwapHiLow(RiffHeader[Count]);
    end;

    wfs:=TFileStream.Create(wavfn,fmOpenReadWrite);
    wfs.WriteBuffer(RiffHeader[0],RiffHeaderSize*4);
    wfs.Free;
  end;
begin
  appfn:=encaudio_PluginPath+'mencoder.exe';

  if fileexists(appfn)=False then begin
    Current_SetError('not found plugin.',appfn);
    Result:=False;
    exit;
  end;

  SetMainTitle('High quality decode to wave... (use mencoder)');
  cmdline:=format('-noautosub -ovc copy -af format=s16le,resample=32768:1:2,channels=2 -oac pcm -of rawaudio',[]);
  CreateDOSBOX_Direct(encaudio_StartPath,appfn,'-v "'+avifn+'" '+cmdline+' -o "'+wavfn+'"');

  if GetFileSize(wavfn)=0 then begin
    Current_SetError('error func DemuxAudio_HQ32768Hz_MEncoder','');
    Result:=False;
    exit;
  end;

  RewriteWaveHeader(32000);

  Result:=True;
end;
}

function EncodeMP2_HQ32768Hz_twolame(srcfn,dstfn:string;freq:integer;kbps:integer):boolean;
var
  hInputRead,hInputWrite:THANDLE;
  hOutputRead,hOutputWrite:THANDLE;
  ErrorStr:string;
  PipeBufStr:string;
  PipeBufFlag:boolean;
  lasttick:dword;
  CaptionText:string;
  perlen:integer;
  permax:integer;
  perstr:string;
  perint:integer;
  batfn:string;
  function CreatePipes(ReadBufSize:dword):boolean;
  var
    SA:SECURITY_ATTRIBUTES;
    SD:TSecurityDescriptor;
  begin
    hInputRead:=0;
    hInputWrite:=0;
    hOutputRead:=0;
    hOutputWrite:=0;

    sa.nLength:=sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor:=nil;
    sa.bInheritHandle:=True;
    InitializeSecurityDescriptor(@SD,SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(@SD,True,nil,False);
    sa.lpSecurityDescriptor:=@SD;

    if CreatePipe(hInputRead,hInputWrite,@sa,ReadBufSize)=False then begin
      Current_SetError(lng(LI_PipeErrorCreate),'');
      Result:=False;
      exit;
    end;
    if DuplicateHandle(GetCurrentProcess(),hInputWrite,GetCurrentProcess(),nil,0,False,DUPLICATE_SAME_ACCESS)=False then begin
      Current_SetError(lng(LI_PipeErrorAttribute),'');
      Result:=False;
      exit;
    end;

    if CreatePipe(hOutputRead,hOutputWrite,@sa,0)=False then begin
      Current_SetError(lng(LI_PipeErrorCreate),'');
      Result:=False;
      exit;
    end;
    if DuplicateHandle(GetCurrentProcess(),hOutputRead,GetCurrentProcess(),nil,0,False,DUPLICATE_SAME_ACCESS)=False then begin
      Current_SetError(lng(LI_PipeErrorAttribute),'');
      Result:=False;
      exit;
    end;

    Result:=True;
  end;
  function StartEncode:boolean;
  var
    appfn:string;
    cmdline:string;
    wfs:TFileStream;
    str:string;
  begin
    appfn:=encaudio_PluginPath+'twolame.exe';
    batfn:=ChangeFileExt(appfn,'.bat');

    if fileexists(appfn)=False then begin
      Current_SetError('not found plugin.',appfn);
      Result:=False;
      exit;
    end;

    if CreatePipes(1024)=False then begin
      Result:=False;
      exit;
    end;

    cmdline:=format('-t 4 -b %d -m j "'+srcfn+'" "'+dstfn+'"',[kbps]);
    enclog.loglst.Lines.Add(appfn);
    enclog.loglst.Lines.Add(cmdline);
    enclog.loglst.Lines.Add('');

    wfs:=TFileStream.Create(batfn,fmCreate);
    str:='"'+ChangeFileExt(appfn,'.exe')+'" '+cmdline;
    wfs.WriteBuffer(str[1],length(str));
    wfs.Free;

    cmdline:='';
    if CreateDOSBOX2(encaudio_PluginPath,hInputRead,hOutputWrite,hOutputWrite,batfn,cmdline)=False then begin
      Current_SetError('CreateDOSBOX error.','');
      Result:=False;
      exit;
    end;

    Result:=True;
  end;
  function ReadPipe(hnd:THANDLE):string;
  var
    ansistr:array[0..1024] of ansichar;
    i:integer;
    len:dword;
    readsize:dword;
    c:ansichar;
  begin
    Result:='';

    len:=0;
    if PeekNamedPipe(hnd, nil, 0, nil,@len,nil)=True then begin
      if len<>0 then begin
        if 1024<=len then len:=1024;
        if ReadFile(hnd,ansistr[0],len,readsize,nil)=True then begin
          for i:=0 to readsize-1 do begin
            c:=ansistr[i];
            if PipeBufFlag=False then begin
              if c=ansichar($0d) then begin
                PipeBufFlag:=True;
                end else begin
                PipeBufStr:=PipeBufStr+c;
              end;
              end else begin
              PipeBufFlag:=False;
              if c=ansichar($0a) then begin
                Result:=Result+PipeBufStr+CRLF;
                PipeBufStr:='';
                end else begin
                CaptionText:=PipeBufStr;
                PipeBufStr:=c;
              end;
            end;
          end;
        end;
      end;
    end;
  end;
  procedure EndEncode;
  begin
    if Current_GetRequestCancel=False then begin
      CloseDOSBOX2(True); // I—¹‚ð‘Ò‚Â
      end else begin
      CloseDOSBOX2(False);
    end;

    ErrorStr:=ReadPipe(hOutputRead);
    if ErrorStr<>'' then begin
      enclog.loglst.Lines.Add(ErrorStr);
      enclog.loglst.Refresh;
    end;
    try
      enclog.loglst.Lines.SaveToFile(changefileext(Application.ExeName,'')+'_mencoder.log');
      except else begin
      end;
    end;

    CloseHandle(hInputRead);
    CloseHandle(hInputWrite);
    CloseHandle(hOutputRead);
    CloseHandle(hOutputWrite);
  end;
begin
  if StartEncode=False then begin
    Result:=False;
    exit;
  end;

  SetMainTitle('High quality encode to mp2... (use twolame)');

  permax:=GetFileSize(srcfn) div 6000;

  SetPrgBarPos(0,'');
  SetPrgBarMax(permax);

  PipeBufStr:='';
  PipeBufFlag:=False;
  CaptionText:='';

  lasttick:=GetTickCount-1000;

  while(isTerminatedDOSBOX2=False) do begin
    if (1000<=(GetTickCount-lasttick)) then begin
      lasttick:=GetTickCount;
      PerStr:=CaptionText;
      PerLen:=length(PerStr);
      if (6<=PerLen) and (PerLen<10) then begin
        if (PerStr[1]='[') and (PerStr[PerLen]=']') then begin
          PerStr:=copy(PerStr,2,PerLen-2);
          PerInt:=strtointdef(PerStr,0);
          if perint<permax then SetPrgBarPos(PerInt,'');
          enclog.Caption:=CaptionText;
        end;
        CaptionText:='';
      end;
    end;

    ErrorStr:=ReadPipe(hOutputRead);
    if ErrorStr<>'' then begin
      enclog.loglst.Lines.Add(ErrorStr);
      enclog.loglst.Refresh;
    end;

    Application.ProcessMessages;
    if Current_GetRequestCancel=True then break;
  end;

  enclog.Caption:='Encode Terminate';
  SetPrgBarPos(0,'Encode Terminate');

  EndEncode;

  DeleteFile(batfn);

  if Current_GetRequestCancel=True then begin
    Result:=False;
    exit;
  end;

  if GetFileSize(dstfn)=0 then begin
    Current_SetError('error func EncodeMP2_HQ32768Hz_twolame','');
    Result:=False;
    exit;
  end;

  Result:=True;
end;

{
function EncodeMP2_HQ32768Hz_twolame(srcfn,dstfn:string;freq:integer;kbps:integer):boolean;
var
  appfn:string;
begin
  appfn:=encaudio_PluginPath+'twolame.exe';

  if fileexists(appfn)=False then begin
    Current_SetError('not found plugin.',appfn);
    Result:=False;
    exit;
  end;

  SetMainTitle('High quality encode to mp2... (use twolame)');
  CreateDOSBOX_UseCMD(encaudio_StartPath,appfn,format('-b %d -m j "'+srcfn+'" "'+dstfn+'"',[kbps]));

  if GetFileSize(dstfn)=0 then begin
    Current_SetError('error func EncodeMP2_HQ32768Hz_twolame','');
    Result:=False;
    exit;
  end;

  Result:=True;
end;
}

function DirectEncodeMP2_MEncoder(srcfn,dstfn:string;freq:integer;kbps:integer):boolean;
var
  appfn:string;
  cmdline:string;
begin
  appfn:=encaudio_PluginPath+'mencoder.exe';

  if fileexists(appfn)=False then begin
    Current_SetError('not found plugin.',appfn);
    Result:=False;
    exit;
  end;

  SetMainTitle('Encode Wave to MP2...');
  cmdline:=format('-noautosub -ovc copy -oac lavc -lavcopts acodec=mp2:abitrate=%d -af resample=%d:1:2 -of rawaudio',[kbps,freq]);
  CreateDOSBOX_Direct(encaudio_StartPath,appfn,'-v "'+srcfn+'" '+cmdline+' -o "'+dstfn+'"');

  if GetFileSize(dstfn)=0 then begin
    Current_SetError('error func DirectEncodeMP2_MEncoder','');
    Result:=False;
    exit;
  end;

  Result:=True;
end;

function DirectEncodeMP2_FFMpeg(srcfn,dstfn:string;freq:integer;kbps:integer):boolean;
var
  appfn:string;
  cmdline:string;
begin
  appfn:=encaudio_PluginPath+'ffmpeg.exe';

  if fileexists(appfn)=False then begin
    Current_SetError('not found plugin.',appfn);
    Result:=False;
    exit;
  end;

  SetMainTitle('Encode Wave to MP2...');
  cmdline:=format('-acodec mp2 -ab %d -ar %d',[kbps,freq]);
  CreateDOSBOX_UseCMD(encaudio_StartPath,appfn,'-i "'+srcfn+'" '+cmdline+' "'+dstfn+'"');

  if GetFileSize(dstfn)=0 then begin
    Current_SetError('error func DirectEncodeMP2_FFMpeg','');
    Result:=False;
    exit;
  end;

  Result:=True;
end;

end.

