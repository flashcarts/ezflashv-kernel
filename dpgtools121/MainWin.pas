unit MainWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls,_PicTools, StdCtrls,_dpg_const,_dpgfs,_dosbox, ComCtrls,_m_Tools,dpgenc_language,
  Menus, ShellAPI, Grids;

const DPGEncVersion='dpgenc.exe for DPG2HQ';

type
  TMain = class(TForm)
    EncodeProcTimer: TTimer;
    StartupTimer: TTimer;
    prgbar: TProgressBar;
    EncodeStartTimer: TTimer;
    PreencodeffmpegTimer: TTimer;
    StatusBar1: TStatusBar;
    MainMenu1: TMainMenu;
    MainMenu_File: TMenuItem;
    MainMenu_Option: TMenuItem;
    MainMenu_Language: TMenuItem;
    MainMenu_About: TMenuItem;
    MainMenu_Language_Template: TMenuItem;
    StandbyTimer: TTimer;
    ing_ngimg: TImage;
    ing_okimg: TImage;
    QueueGrid: TStringGrid;
    MainMenu_ShowLog: TMenuItem;
    QueuePopup: TPopupMenu;
    QueuePopup_Delete: TMenuItem;
    MainMenu_File_Exit: TMenuItem;
    OutputPathChangeBtn: TButton;
    OutputPathLbl: TLabel;
    OutputPathDlg: TSaveDialog;
    EncodeEndTimer: TTimer;
    PreencodeAutoDetectTimer: TTimer;
    procedure FormCreate(Sender: TObject);
    procedure EncodeProcTimerTimer(Sender: TObject);
    procedure StartupTimerTimer(Sender: TObject);
    procedure FormKeyUp(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure PreencodeffmpegTimerTimer(Sender: TObject);
    procedure EncodeStartTimerTimer(Sender: TObject);
    procedure MainMenu_Language_TemplateClick(Sender: TObject);
    procedure MainMenu_OptionClick(Sender: TObject);
    procedure StandbyTimerTimer(Sender: TObject);
    procedure QueueGridDrawCell(Sender: TObject; ACol, ARow: Integer;
      Rect: TRect; State: TGridDrawState);
    procedure MainMenu_ShowLogClick(Sender: TObject);
    procedure QueuePopup_DeleteClick(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
    procedure MainMenu_File_ExitClick(Sender: TObject);
    procedure MainMenu_AboutClick(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure OutputPathChangeBtnClick(Sender: TObject);
    procedure EncodeEndTimerTimer(Sender: TObject);
    procedure PreencodeAutoDetectTimerTimer(Sender: TObject);
    procedure QueueGridKeyUp(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure QueueGridMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure QueueGridMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
  private
    { Private 宣言 }
    procedure WMDROPFILES(var msg:TWMDROPFILES);message WM_DROPFILES;
  public
    { Public 宣言 }
    optReencWidth,optReencKBPS,optReencFPS:integer;
    OptOutputPath:string;
    MainFormWidth,MainFormHeight:integer;
    function StartEncode:boolean;
    procedure LoadLanguage;
  end;

var
  Main: TMain;

procedure SetPrgBarMax(max:integer);
procedure SetPrgBarPos(pos:integer;zerostr:string);

implementation

uses DSSupportWin, OptionWin, enclogWin, encprvWin, OptionCmdLineWin,_queue,_encaudio;

{$R *.dfm}

var
  StartPath:string;
  LanguagePath:string;
  PluginPath:string;

var
  DPGINFO:TDPGINFO;

const CRLF:string=char($0d)+char($0a);

const BELOW_NORMAL_PRIORITY_CLASS=$00004000;
const ABOVE_NORMAL_PRIORITY_CLASS=$00008000;

procedure SetPriorityLevel(Level:integer);
begin
  case Level of
    0: SetPriorityClass(GetCurrentProcess,IDLE_PRIORITY_CLASS);
    1: SetPriorityClass(GetCurrentProcess,BELOW_NORMAL_PRIORITY_CLASS);
    2: SetPriorityClass(GetCurrentProcess,NORMAL_PRIORITY_CLASS);
    3: SetPriorityClass(GetCurrentProcess,ABOVE_NORMAL_PRIORITY_CLASS);
    4: SetPriorityClass(GetCurrentProcess,HIGH_PRIORITY_CLASS);
    5: SetPriorityClass(GetCurrentProcess,REALTIME_PRIORITY_CLASS);
    else SetPriorityClass(GetCurrentProcess,NORMAL_PRIORITY_CLASS);
  end;
end;

function isCheckOS:boolean;
var
  osverinfo:OSVERSIONINFO;
begin
  osverinfo.dwOSVersionInfoSize:=sizeof(OSVERSIONINFO);
  GetVersionEx(osverinfo);

  if osverinfo.dwPlatformId=VER_PLATFORM_WIN32_WINDOWS then begin
    Result:=False;
    end else begin
    Result:=True;
  end;

end;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

var
  MasterFontCodepage:integer;
  MasterFontName:string;
  MasterFontSize:integer;

procedure ResetLanguageSet;
var
  res:integer;
  SearchRec: TSearchRec;
  fname:string;
  NewItem:TMenuItem;
  ttl:TStringList;
  procedure ClearAndFree(var t:TMenuItem);
  var
    cnt,count:integer;
  begin
    count:=t.Count;
    for cnt:=count-1 downto 0 do begin
      t.Delete(cnt);
    end;
  end;
begin
  ClearAndFree(Main.MainMenu_Language);

  res:=FindFirst(LanguagePath+ChangeFileExt(extractfilename(Application.ExeName),'')+'.*.lng', (FaAnyFile), SearchRec);
  if res=0 then begin
    repeat
      fname:=SearchRec.Name;
      if (SearchRec.Attr and faDirectory)=0 then begin
        ttl:=TStringList.Create;
        ttl.LoadFromFile(LanguagePath+fname);
        fname:=ChangeFileExt(fname,'');
        fname:=ExtractFileExt(fname);
        NewItem:=TMenuItem.Create(Main.MainMenu_Language);
        NewItem.Enabled:=True;
        NewItem.Caption:=fname+' / '+ttl[0];
        NewItem.Tag:=0;
        NewItem.Checked:=False;
        NewItem.OnClick:=Main.MainMenu_Language_TemplateClick;
        Main.MainMenu_Language.Add(NewItem);
        ttl.Free;
      end;
      res:=FindNext(SearchRec);
    until (res<>0);
  end;
  FindClose(SearchRec);
end;

procedure TMain.LoadLanguage;
var
  FontName:string;
  FontSize:integer;
  FontCodepage:integer;
  idx:integer;
  function GetLang:string;
  var
    t:TMenuItem;
    cnt,count:integer;
    tlang:string;
  begin
    t:=Main.MainMenu_Language;
    count:=t.Count;
    for cnt:=0 to count-1 do begin
      if t.Items[cnt].Checked=True then begin
        tlang:=t.Items[cnt].Caption;
        tlang:=copy(tlang,1,ansipos(' ',tlang)-1);
        Result:=tlang;
        exit;
      end;
    end;
    tlang:=t.Items[0].Caption;
    tlang:=copy(tlang,1,ansipos(' ',tlang)-1);
    Result:=tlang;
  end;
  procedure SetFont(font:TFont);
  begin
    with font do begin
      Charset:=FontCodepage;
      Name:=FontName;
      Size:=FontSize;
      Charset:=FontCodepage;
    end;
  end;
begin
  LoadLngFile(LanguagePath+ChangeFileExt(extractfilename(Application.ExeName),'')+GetLang+'.lng');

  FontName:=lngID(LI_ID_FontName);
  if FontName='' then FontName:=MasterFontName;
  FontSize:=strtointdef(lngID(LI_ID_FontSize),MasterFontSize);
  FontCodepage:=strtointdef(lngID(LI_ID_FontCodepage),MasterFontCodepage);

  SetFont(Main.Font);
  SetFont(Option.Font);
  SetFont(OptionCmdLine.Font);
  SetFont(enclog.Font);
  SetFont(QueueGrid.Font);

  QueueGrid.DefaultRowHeight:=2+(QueueGrid.Canvas.TextHeight('Agyz')*2);
  for idx:=0 to QueueGrid.RowCount-1 do begin
    QueueGrid.RowHeights[idx]:=QueueGrid.DefaultRowHeight;
  end;

  with Main do begin
    OutputPathChangeBtn.Caption:=lng(LI_OutputPathChange);
    MainMenu_File.Caption:=lng(LI_MainMenu_File);
    MainMenu_File_Exit.Caption:=lng(LI_MainMenu_File_Exit);
    MainMenu_Option.Caption:=lng(LI_MainMenu_Option);
    MainMenu_ShowLog.Caption:=lng(LI_MainMenu_ShowLog);
    MainMenu_Language.Caption:=lng(LI_MainMenu_Language);
    MainMenu_About.Caption:=lng(LI_MainMenu_About);
    QueuePopup_Delete.Caption:=lng(LI_CancelPopup);
  end;

  with Option do begin
    CancelBtn.Caption:=lng(LI_FormCancel);
    StartBtn.Caption:=lng(LI_FormStart);
    with PreencLst do begin
      idx:=ItemIndex;
      Items[0]:=lng(LI_Preenc_DS);
      Items[1]:=lng(LI_Preenc_ffmpeg);
      Items[2]:=lng(LI_Preenc_menc);
      ItemIndex:=idx;
    end;
    SoundGrp.Caption:=lng(LI_FormAudioOptions);
    SoundRateLbl.Caption:=lng(LI_FormAudioRate);
    SoundRateUnitLbl.Caption:=lng(LI_FormAudioRateUnit);
    SoundBPSLbl.Caption:=lng(LI_FormAudioBPS);
    SoundBPSUnitLbl.Caption:=lng(LI_FormAudioBPSUnit);
    SoundVolumeLbl.Caption:=lng(LI_FormAudioVolume);
    with SndGetModeLst do begin
      idx:=ItemIndex;
      Items[0]:=lng(LI_FormAudioGetModeItem0);
      Items[1]:=lng(LI_FormAudioGetModeItem1);
      Items[2]:=lng(LI_FormAudioGetModeItem2);
      Items[3]:=lng(LI_FormAudioGetModeItem3);
      Items[4]:=lng(LI_FormAudioGetModeItem4);
      ItemIndex:=idx;
    end;
    ReencOptGroup.Caption:=lng(LI_FormVideoOptions);

    ReencPixelFormatLbl.Caption:=lng(LI_FormVideoPixelFormat);
    with ReencPixelFormatLst do begin
      idx:=ItemIndex;
      Items[0]:=lng(LI_FormVideoPixelFormatItem0);
      Items[1]:=lng(LI_FormVideoPixelFormatItem1);
      Items[2]:=lng(LI_FormVideoPixelFormatItem2);
      Items[3]:=lng(LI_FormVideoPixelFormatItem3);
      ItemIndex:=idx;
    end;
    VideoWidthLbl.Caption:=lng(LI_FormVideoWidth);
    VideoWidthUnitLbl.Caption:=lng(LI_FormVideoWidthUnit);
    VideoAspectLbl.Caption:=lng(LI_FormVideoAspect);
    with ReencAspectLst do begin
      idx:=ItemIndex;
      Items[0]:=lng(LI_FormVideoAspectItem0);
      Items[1]:=lng(LI_FormVideoAspectItem1);
      Items[2]:=lng(LI_FormVideoAspectItem2);
      Items[3]:=lng(LI_FormVideoAspectItem3);
      ItemIndex:=idx;
    end;
    VideoBPSLbl.Caption:=lng(LI_FormVideoBPS);
    VideoBPSUnitLbl.Caption:=lng(LI_FormVideoBPSUnit);
    VideoFPSLbl.Caption:=lng(LI_FormVideoFPS);
    VideoFPSUnitLbl.Caption:=lng(LI_FormVideoFPSUnit);
    VideoBrightLbl.Caption:=lng(LI_FormVideoBright);
    VideoBlurLbl.Caption:=lng(LI_FormVideoBlur);
    VideoBlurDeepLbl.Caption:=lng(LI_FormVideoBlurDeep);
    VideoBlurLightLbl.Caption:=lng(LI_FormVideoBlurLight);
    ReencVerticalSwapChk.Caption:=lng(LI_FormVideoFlip);

    AdvanceChk.Caption:=lng(LI_OptionAdvance);

    Option.ReencProfileCustomEditBtn.Caption:=lng(LI_FormCustomEdit);
    with ReencProfileLst do begin
      idx:=ItemIndex;
      Items[0]:=lng(LI_FormProfileItem0);
      Items[1]:=lng(LI_FormProfileItem1);
      Items[2]:=lng(LI_FormProfileItem2);
      ItemIndex:=idx;
    end;

  end;

  MainMenu_Language.Caption:='Language (&L)';
end;

procedure RefreshOutputPathLbl;
begin
  Main.OutputPathLbl.Caption:=lng(LI_OutputPathTag)+Main.OptOutputPath;
end;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

const StoreBufSize=1*1024*1024;

function CreateDPG:boolean;
var
  wfs:TFileStream;
  ofspos:integer;
  posa,sizea:integer;
  posv,sizev:integer;
  posg,sizeg:integer;
  function AddFile(ttl:string;fn:string):integer;
  var
    rfs:TFileStream;
    buf:array of byte;
    size:integer;
    idx:integer;
    bufsize:integer;
  begin
    if FileExists(fn)=False then begin
      Current_SetError('empty file.',fn);
      Result:=0;
      exit;
    end;

    Main.Caption:=ttl;

    setlength(buf,StoreBufSize);

    try
      rfs:=TFileStream.Create(fn,fmOpenRead);
      except else begin
        Current_SetError('can not open file.',fn);
        Result:=0;
        exit;
      end;
    end;

    size:=rfs.Size;
    if size<>0 then begin
      SetPrgBarPos(0,'');
      Main.prgbar.Max:=size div StoreBufSize;
      idx:=0;
      while(idx<size) do begin
        bufsize:=size-idx;
        if StoreBufSize<=bufsize then bufsize:=StoreBufSize;
        rfs.ReadBuffer(buf[0],bufsize);
        wfs.WriteBuffer(buf[0],bufsize);
        inc(idx,bufsize);

        SetPrgBarPos(idx div StoreBufSize,'');
      end;
      SetPrgBarPos(0,'');
    end;
    rfs.Free;

    Result:=size;
  end;
  procedure AddDW(dw:dword);
  begin
    wfs.WriteBuffer(dw,4);
  end;
  procedure padding32bit;
  var
    tmp:byte;
  begin
    tmp:=$00;
    if (wfs.Size mod 4)<>0 then wfs.WriteBuffer(tmp,1);
    if (wfs.Size mod 4)<>0 then wfs.WriteBuffer(tmp,1);
    if (wfs.Size mod 4)<>0 then wfs.WriteBuffer(tmp,1);
  end;
begin
  SetPrgBarPos(0,'Create DPG file');

  try
    wfs:=TFileStream.Create(GetDPGFilename,fmCreate,fmShareExclusive);
    except else begin
      Current_SetError('can not open file.',GetDPGFilename);
      Result:=False;
      exit;
    end;
  end;

  AddDW(DPG2ID);

  AddDW(DPGINFO.TotalFrame);
  AddDW(trunc(DPGINFO.FPS*$100));
  AddDW(DPGINFO.SndFreq);
  AddDW(DPGINFO.SndCh);

  ofspos:=wfs.Position;

  AddDW(0);
  AddDW(0);
  AddDW(0);
  AddDW(0);
  AddDW(0);
  AddDW(0);

  AddDW(DPGINFO.PixelFormat);

  posa:=wfs.Position;
  sizea:=AddFile('Store Audio...',GetDPGMP2Filename);
  padding32bit();

  posv:=wfs.Position;
  sizev:=AddFile('Store Video...',GetDPGMovieFilename);
  padding32bit();

  posg:=wfs.Position;
  sizeg:=AddFile('Store GOP list...',GetDPGGOPListFilename);
  padding32bit();

  wfs.Position:=ofspos;

  AddDW(posa);
  AddDW(sizea);
  AddDW(posv);
  AddDW(sizev);
  AddDW(posg);
  AddDW(sizeg);

  wfs.Free;

  Result:=True;
end;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

const SndGetMode_MEncoder=0;
const SndGetMode_FFMpeg=1;
const SndGetMode_ExtractWaveNormal=2;
const SndGetMode_ExtractWaveAlt=3;
const SndGetMode_External=4;

type
  TAudioInfo=record
    GetMode:integer;
    Volume:integer;
    Freq:integer;
    kbps:integer;
  end;

var
  AudioInfo:TAudioInfo;

type
  TVideoInfo=record
    PixelFormat:integer;
    Width,Height:integer;
    FPS:double;
    kbps:integer;
    Brightness:integer;
    Blur:integer;
    VerticalSwap:boolean;
    CmdLineFormat:string;
    RequestDeleteOverrideSource:boolean;
  end;

var
  VideoInfo:TVideoInfo;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

procedure TMain.FormCreate(Sender: TObject);
begin
  Main.OnResize:=nil;

  StartPath:=ExtractFilePath(Application.ExeName);
  LanguagePath:=StartPath+'dpgenclng\';
  PluginPath:=StartPath+'dpgencplugin\';

  if isCheckOS=False then begin
    Application.Title:='windows version check.';
    ShowMessage('WindowsNT系(Win2000/XP等)のOSでしか起動できません。'+CRLF+CRLF+'The tool related to DPG works only in the WindowsNT system. (Win2k/WinXP etc.)');
    Application.Terminate;
    exit;
  end;

  MainFormWidth:=Main.Width;
  MainFormHeight:=Main.Height;

  setlog;

  SetBaseFilename('','');

  DragAcceptFiles(Main.handle,True); // D&D Start

  StartupTimer.Enabled:=True;
end;

var
  LastPrgBarMsg:string;

procedure SetPrgBarMax(max:integer);
begin
  Main.prgbar.Max:=max;
end;

procedure SetPrgBarPos(pos:integer;zerostr:string);
begin
  Main.prgbar.Position:=pos;
  if (pos<>0) and (Main.prgbar.Max<>0) then begin
//    Main.QueueGrid.Cells[0,QueueGetQueueIdx]:=inttostr((Main.prgbar.Position*100) div Main.prgbar.Max)+'%';
    Main.QueueGrid.Cells[0,QueueGetQueueIdx]:=inttostr(Main.prgbar.Position)+'/ '+inttostr(Main.prgbar.Max);
    end else begin
    if zerostr<>'' then LastPrgBarMsg:=zerostr;
    Main.QueueGrid.Cells[0,QueueGetQueueIdx]:=LastPrgBarMsg;
    Application.ProcessMessages;
  end;
end;

procedure TMain.WMDROPFILES(var msg:TWMDROPFILES);
var
  Drop:hdrop;
  index:longint;
  idx:integer;
  Filename:string;
  filebuf:array[0..1024] of char;
  cnt:integer;
begin
  Filename:=StringOfChar(' ',1024);
  Drop:=msg.Drop;
  index:=DragQueryFile(Drop,$FFFFFFFF,nil,0);

  for idx:=0 to index-1 do begin
    DragQueryFile(Drop,idx,filebuf,1024);
    Filename:='';
    cnt:=0;
    while ((filebuf[cnt]<>char($00)) and (cnt<1024)) do begin
      Filename:=Filename+filebuf[cnt];
      inc(cnt);
    end;
    QueueAdd(Filename);
  end;

  DragFinish(Drop);
end;

procedure TMain.StartupTimerTimer(Sender: TObject);
var
  idx:integer;
begin
  StartupTimer.Enabled:=False;

//  MainMenu_ShowLogClick(Sender);

  MasterFontCodepage:=Main.Font.Charset;
  MasterFontName:=Main.Font.Name;
  MasterFontSize:=Main.Font.Size;

  Application.Title:=DPGEncVersion;
  Main.Caption:=Application.Title;

  ResetLanguageSet;

  OptOutputPath:=GetDesktopPath+'\';

  Option.LoadINI;
  LoadLanguage;
  Option.Init;

  Main.Width:=MainFormWidth;
  Main.Height:=MainFormHeight;

  Main.OnResize:=Main.FormResize;
  Main.FormResize(nil);

  if DirectoryExists(OptOutputPath)=False then OptOutputPath:=GetDesktopPath+'\';
  RefreshOutputPathLbl;

  QueueInit;
  Current_Init;

  QueueGrid.ColWidths[0]:=48;
  QueueGrid.ColWidths[1]:=1024;
  QueueGrid.Cells[0,0]:='';
  QueueGrid.Cells[1,0]:=lng(LI_QueueMessage);

  QueueRefreshStatus;

  for idx:=1 to ParamCount do begin
    if FileExists(ParamStr(idx))=True then begin
      QueueAdd(ParamStr(idx));
    end;
  end;

  StandbyTimer.Enabled:=True;
end;

procedure TMain.StandbyTimerTimer(Sender: TObject);
var
  fn:string;
begin
  if QueueGetQueueCount<=QueueGetQueueIdx then exit;

  Main.StandbyTimer.Enabled:=False;

  SetPrgBarPos(0,'');

  fn:=QueueGrid.Cells[1,QueueGetQueueIdx];

  if ansilowercase(ExtractFileExt(fn))='.dpg' then begin
    Current_SetError('Can not re-encode from DPG file.','');
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  SetPriorityLevel(Option.PriorityLevelLst.ItemIndex);
  SetDefaultPriorityLevel(Option.PriorityLevelLst.ItemIndex);

  Current_Init;
  Current_SetSrcFilename(fn);
  Current_SetDstFilename(OptOutputPath+ExtractFilename(ChangeFileExt(fn,'.dpg')));

  case Option.PreencLst.ItemIndex of
    0: PreencodeAutoDetectTimer.Enabled:=True;
    1: PreencodeffmpegTimer.Enabled:=True;
    2: Application.Terminate;
    else Application.Terminate;
  end;
end;

procedure TMain.PreencodeAutoDetectTimerTimer(Sender: TObject);
{
  function isWMV3:boolean;
  var
    rfs:TFileStream;
    chkbuf:array[0..$200] of byte;
    chkidx,chkofs:integer;
  begin
    rfs:=TFileStream.Create(GetSourceFilename,fmOpenRead or fmShareDenyNone);
    if rfs.Size<$200 then begin
      rfs.ReadBuffer(chkbuf[0],rfs.Size);
      end else begin
      rfs.ReadBuffer(chkbuf[0],$200);
    end;
    rfs.Free;

    for chkidx:=$00 to ($200 div 4)-1 do begin
      chkofs:=chkidx*4;
      if (chkbuf[chkofs+0]=$57) and (chkbuf[chkofs+1]=$4d) and (chkbuf[chkofs+2]=$56) and (chkbuf[chkofs+3]=$33) then begin
        Result:=True;
        exit;
      end;
    end;
    Result:=False;
  end;
}
  function isWMV3:boolean;
  var
    appfn:string;
    tfn:string;
    strs:TStringList;
  begin
    Result:=False;

    appfn:=PluginPath+'mencoder.exe';
    tfn:=StartPath+'mencoder_detect.txt';
    DeleteFile(tfn);

    if fileexists(appfn)=False then begin
      Current_SetError('not found plugin.',appfn);
      exit;
    end;

    CreateDOSBOX_UseCMD(StartPath,appfn,'"'+GetSourceFilename+'" > "'+tfn+'"');

    if FileExists(tfn)=False then exit; // unknown error.

    strs:=TStringList.Create;
    try
      strs.LoadFromFile(tfn);
      except else begin
        strs.Free;
        exit;
      end;
    end;

    if AnsiPos('VIDEO:  [WMV3]',strs.Text)<>0 then Result:=True;
    if AnsiPos('VIDEO: [WMV3]',strs.Text)<>0 then Result:=True;

    strs.Free;
    DeleteFile(tfn);
  end;
  function supportds:boolean;
  var
    SourceFPS:double;
    Width,Height:integer;
    Aspect:double;
  begin
    Result:=True;

    DSSupport.DSOpen(GetSourceFilename,0,0,0);
    SourceFPS:=DSSupport.GetSourceFPS;
    DSSupport.DSClose;

    if SourceFPS=0 then Result:=False;

    if (DSSupport.VideoWidth=0) or (DSSupport.VideoHeight=0) then Result:=False;

    if DSSupport.DSTotalTime=0 then Result:=False;

    Width:=strtoint(Option.ReencWidthLst.Text);
    Width:=(Width div 16)*16;
    if 256<Width then Width:=256;

    if DSSupport.VideoWidth=0 then begin
      Aspect:=0;
      end else begin
      case Option.ReencAspectLst.ItemIndex of
        0: Aspect:=DSSupport.VideoHeight/DSSupport.VideoWidth;
        1: Aspect:=3/4;
        2: Aspect:=9/16;
        3: Aspect:=1/2.35;
        else Aspect:=0;
      end;
    end;
    if Aspect=0 then Result:=False;

    Height:=trunc(Width*Aspect);
    Height:=(Height div 16)*16;
    if 192<Height then Height:=192;

    if DSSupport.DSTotalTime=0 then Result:=False;

    if (256<Width) or (192<Height) then Result:=False;
  end;
  function supportffmpeg:boolean;
  var
    srcfn,dstfn,appfn:string;
  begin
    Result:=True;

    srcfn:=GetSourceFilename;
    dstfn:=GetTempffmpegFilename;

    if FileExists(dstfn)=True then begin
      if DeleteFile(dstfn)=False then Result:=False;
    end;

    if Result=True then begin
      appfn:=PluginPath+'ffmpeg.exe';

      if fileexists(appfn)=False then Result:=False;
    end;

    if Result=True then begin
      CreateDOSBOX_UseCMD(StartPath,appfn,'-v 2 -y -t 1 -b 1500 -ab 256 -i "'+srcfn+'" "'+dstfn+'"');

      if GetFileSize(dstfn)=0 then Result:=False;
      DeleteFile(dstfn);
    end;
  end;
begin
  PreencodeAutoDetectTimer.Enabled:=False;

  // ----

  SetPrgBarPos(0,'Detect WMV3');
  if isWMV3=True then begin
    Current_SetError(lng(LI_DetectErrorWMV3),'');
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  // ----

  SetPrgBarPos(0,'Detect DirectShow');
  Main.Caption:='Check stream by DirectShow.';
  if supportds=True then begin
    EncodeStartTimer.Enabled:=True;
    exit;
  end;

  // ----

  SetPrgBarPos(0,'Detect ffmpeg');
  Main.Caption:='Check stream by ffmpeg.';
  if supportffmpeg=True then begin
    PreencodeffmpegTimer.Enabled:=True;
    exit;
  end;

  // ---

  EncodeStartTimer.Enabled:=True;
end;

procedure TMain.PreencodeffmpegTimerTimer(Sender: TObject);
var
  appfn:string;
  srcfn,dstfn:string;
begin
  PreencodeffmpegTimer.Enabled:=False;
  if Current_GetRequestCancel=True then begin
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  SetPrgBarPos(0,'Preencode ffmpeg');

  srcfn:=GetSourceFilename;
  dstfn:=GetTempffmpegFilename;

  if FileExists(srcfn)=False then begin
    Current_SetError('file not found.','');
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  if FileExists(dstfn)=True then begin
    if DeleteFile(dstfn)=False then begin
      Current_SetError('can not delete file.',dstfn);
      EncodeEndTimer.Enabled:=True;
      exit;
    end;
  end;

  appfn:=PluginPath+'ffmpeg.exe';

  if fileexists(appfn)=False then begin
    Current_SetError('can not found plugin.',appfn);
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  Main.Caption:='Pre-encode using ffmpeg...';
  CreateDOSBOX_UseCMD(StartPath,appfn,'-v 2 -y -b 1500 -ab 256 -i "'+srcfn+'" "'+dstfn+'"');

  SetBaseFilename(dstfn,Current_GetDstFilename); // override
  VideoInfo.RequestDeleteOverrideSource:=True;

  if Current_GetRequestCancel=True then begin
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  if GetFileSize(dstfn)=0 then begin
    Current_SetError('format not supported by ffmpeg.','');
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  EncodeStartTimer.Enabled:=True;
end;

procedure TMain.EncodeStartTimerTimer(Sender: TObject);
var
  Aspect:double;
  SourceFPS:double;
  msg:string;
begin
  EncodeStartTimer.Enabled:=False;
  if Current_GetRequestCancel=True then begin
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  SetPrgBarPos(0,'Encode Start');

  DSSupport.DSOpen(GetSourceFilename,0,0,0);
  SourceFPS:=DSSupport.GetSourceFPS;
  DSSupport.DSClose;

  if SourceFPS=0 then begin
    Current_SetError(lng(LI_DetectErrorFPS),'');
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  with AudioInfo do begin
    Volume:=Option.SndVolumeBar.Position;
    GetMode:=Option.SndGetModeLst.ItemIndex;
    Freq:=Option.GetParam_SndFreq;
    kbps:=Option.GetParam_SndKBPS;
  end;

  if (DSSupport.VideoWidth=0) or (DSSupport.VideoHeight=0) then begin
    Current_SetError(lng(LI_DetectErrorVideoSize),'');
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  if DSSupport.DSTotalTime=0 then begin
    Current_SetError(lng(LI_DetectErrorTotalTime),'');
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  with VideoInfo do begin
    PixelFormat:=Option.ReencPixelFormatLst.ItemIndex;

    Width:=strtoint(Option.ReencWidthLst.Text);
    Width:=(Width div 16)*16;
    if 256<Width then Width:=256;

    if DSSupport.VideoWidth=0 then begin
      Aspect:=0;
      end else begin
      case Option.ReencAspectLst.ItemIndex of
        0: Aspect:=DSSupport.VideoHeight/DSSupport.VideoWidth;
        1: Aspect:=3/4;
        2: Aspect:=9/16;
        3: Aspect:=1/2.35;
        else Aspect:=0;
      end;
    end;
    if Aspect=0 then begin
      Current_SetError(lng(LI_DetectErrorIlligalAspect),'');
      EncodeEndTimer.Enabled:=True;
      exit;
    end;

    Height:=trunc(Width*Aspect);
    Height:=(Height div 16)*16;
    if 192<Height then Height:=192;

    FPS:=strtoint(Option.ReencFPSLst.Text);
    kbps:=strtoint(Option.ReencKBPSLst.Text);
    Brightness:=Option.ReencBrightnessBar.Position;
    Blur:=Option.ReencBlurBar.Position;
    VerticalSwap:=Option.ReencVerticalSwapChk.Checked;
    CmdLineFormat:=Option.GetParam_ReencCmdLine;

    RequestDeleteOverrideSource:=False;
  end;

  with DPGINFO do begin
    PixelFormat:=VideoInfo.PixelFormat;
    FPS:=VideoInfo.FPS;
    TotalFrame:=trunc(DSSupport.DSTotalTime*FPS);
    SndFreq:=AudioInfo.Freq;
    SndCh:=0;
  end;

  if (256<VideoInfo.Width) or (192<VideoInfo.Height) then begin
    Current_SetError(lng(LI_DirectMpegVideoSizeError),'');
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  if 24<VideoInfo.FPS then begin
    msg:=format(lng(LI_AboutFrameRate),[VideoInfo.FPS]);
    if MessageDlg(msg,mtInformation,[mbYes,mbCancel],0)=mrCancel then begin
      Current_SetError(msg,'');
      EncodeEndTimer.Enabled:=True;
      exit;
    end;
  end;

  EncodeProcTimer.Enabled:=True;
end;

function EncodeVideo(avifn:string;m1vfn:string):boolean;
var
  hInputRead,hInputWrite:THANDLE;
  hOutputRead,hOutputWrite:THANDLE;
  FrameIdx:integer;
  w,h:integer;
  bm,lastbm:TBitmap;
  ErrorStr:string;
  readsize:dword;
  PipeBufStr:string;
  PipeBufFlag:boolean;
  lasttick:dword;
  CaptionText:string;
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
  function StartEncode(w,h:integer;fps:double;frames,kbps:integer):boolean;
  var
    appfn:string;
    cmdline:string;
    fixkbps:integer;
  begin
    appfn:=PluginPath+'mencoder.exe';

    if fileexists(appfn)=False then begin
      Current_SetError('not found plugin.',appfn);
      Result:=False;
      exit;
    end;

    if CreatePipes(w*3*h)=False then begin
      Result:=False;
      exit;
    end;

    fixkbps:=trunc(kbps*24/fps);
    cmdline:=format(VideoInfo.CmdLineFormat,[frames,w,h,w*3*h,fixkbps]);
    cmdline:='-v - -o "'+m1vfn+'" '+cmdline;
    enclog.loglst.Lines.Add(appfn);
    enclog.loglst.Lines.Add(cmdline);
    enclog.loglst.Lines.Add('');

    if CreateDOSBOX2(StartPath,hInputRead,hOutputWrite,hOutputWrite,appfn,cmdline)=False then begin
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
      CloseDOSBOX2(True); // 終了を待つ
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
  procedure BrightnessEffect(var bm:TBitmap;Brightness:integer);
  var
    br:integer;
    pb:PByteArray;
    y,x:integer;
    c:integer;
  begin
    if Brightness=100 then exit;
    br:=(Brightness*$100) div 100;
    for y:=0 to bm.Height-1 do begin
      pb:=bm.ScanLine[y];
      for x:=0 to bm.Width*3-1 do begin
        c:=pb[x];
        c:=(c*br) div $100;
        if c<=$ff then begin
          pb[x]:=c;
          end else begin
          pb[x]:=$ff;
        end;
      end;
    end;
  end;
  procedure Blur(var lastbm,bm:TBitmap;percent:integer);
  var
    lastper,per:integer;
    lastpb,pb:PByteArray;
    y,x:integer;
    lastc,c,dstc:integer;
  begin
    if percent=100 then exit;
    lastper:=((100-percent)*$100) div 100;
    per:=(percent*$100) div 100;
    for y:=0 to bm.Height-1 do begin
      lastpb:=lastbm.ScanLine[y];
      pb:=bm.ScanLine[y];
      for x:=0 to bm.Width*3-1 do begin
        lastc:=lastpb[x]*lastper;
        c:=pb[x]*per;
        dstc:=(lastc+c) div $100;
        if $ff<dstc then dstc:=$ff;
        pb[x]:=dstc;
      end;
    end;
  end;
  procedure SendRAW(var bm:TBitmap;w,h:integer);
  var
    pb:PByteArray;
    size,y,idx:integer;
    buf:array of byte;
    bs:integer;
  begin
    size:=w*3*h;
    setlength(buf,size);
    for y:=0 to h-1 do begin
      pb:=bm.ScanLine[y];
      CopyMemory(@buf[w*3*y],@pb[0],w*3);
    end;
    case VideoInfo.PixelFormat of
      DPGPixelFormat_RGB15: bs:=3;
      DPGPixelFormat_RGB18: bs:=2;
      DPGPixelFormat_RGB21: bs:=1;
      DPGPixelFormat_RGB24: bs:=0;
      else bs:=0;
    end;
    for idx:=0 to size-1 do begin
      buf[idx]:=buf[idx] shr bs;
    end;
    WriteFile(hInputWrite,buf[0],size,readsize,nil);
  end;
begin
  with VideoInfo do begin
    w:=Width;
    h:=Height;
    if StartEncode(w,h,FPS,DPGINFO.TotalFrame,kbps)=False then begin
      Result:=False;
      exit;
    end;
  end;

  DSSupport.DSOpen(GetSourceFilename,VideoInfo.FPS,0,0);

  bm:=TBitmap.Create;
  MakeBlankBM(bm,w,h,pf24bit);
  lastbm:=TBitmap.Create;
  MakeBlankBM(lastbm,w,h,pf24bit);

  SetPrgBarPos(0,'');
  SetPrgBarMax(DPGINFO.TotalFrame);

  PipeBufStr:='';
  PipeBufFlag:=False;
  CaptionText:='';

  lasttick:=GetTickCount-1000;

  for FrameIdx:=0 to DPGINFO.TotalFrame-1 do begin
    bitblt(lastbm.Canvas.Handle,0,0,w,h,bm.Canvas.Handle,0,0,SRCCOPY);
    DSSupport.GetStretchBitmap(FrameIdx,bm,VideoInfo.VerticalSwap);
    BrightnessEffect(bm,VideoInfo.Brightness);
    Blur(lastbm,bm,VideoInfo.Blur);

    if (FrameIdx<>0) and (1000<=(GetTickCount-lasttick)) then begin
      lasttick:=GetTickCount;
      SetPrgBarPos(FrameIdx,'');
      BitBlt(encprv.Canvas.Handle,0,0,w,h,bm.Canvas.Handle,0,0,SRCCOPY);
      if CaptionText<>'' then begin
        enclog.Caption:=CaptionText;
        CaptionText:='';
      end;
    end;

    SendRAW(bm,w,h);

    ErrorStr:=ReadPipe(hOutputRead);
    if ErrorStr<>'' then begin
      enclog.loglst.Lines.Add(ErrorStr);
      enclog.loglst.Refresh;
    end;

    Application.ProcessMessages;
    if Current_GetRequestCancel=True then break;

    if isTerminatedDOSBOX2=True then break;
  end;

{
  // send dummy data
  for FrameIdx:=0 to 30-1 do begin
    SendRAW(bm,w,h);
    Application.ProcessMessages;
  end;
}

  enclog.Caption:='Encode Terminate';
  SetPrgBarPos(0,'Encode Terminate');

  bm.Free;
  DSSupport.DSClose;

  EndEncode;

  if Current_GetRequestCancel=True then begin
    Result:=False;
    exit;
  end;

  if GetFileSize(m1vfn)=0 then begin
    Current_SetError('unknown encoding error.','');
    Result:=False;
    exit;
  end;

  Result:=True;
end;

const readbufsize=64*1024;
const chkbufsize=$14+2;

type
  TGOPList=record
    FrameIndex:dword;
    Offset:dword;
  end;

function CreateGOPList(m1vfn,GOPListfn:string):boolean;
var
  GOPList:array of TGOPList;
  GOPListCount:integer;
  rfs:TFileStream;
  filepos,filesize:integer;
  readbufpos:integer;
  readbuf:array[0..readbufsize-1] of byte;
  chkbuf:array[0..chkbufsize-1] of byte;
  chkflag:boolean;
  FrameIndex:integer;
  PicData:dword;
  PicData_refidx,PicData_type:integer;
  wfs:TFileStream;
  idx:integer;
  procedure getchkbuf;
  var
    idx:integer;
  begin
    if readbufsize<=(readbufpos+chkbufsize) then begin
      rfs.Position:=filepos;
      if (filepos+readbufsize)<filesize then begin
        rfs.ReadBuffer(readbuf[0],readbufsize);
        end else begin
        rfs.ReadBuffer(readbuf[0],filesize-filepos);
      end;
      readbufpos:=0;
    end;
    for idx:=0 to chkbufsize-1 do begin
      chkbuf[idx]:=readbuf[readbufpos+idx];
    end;
  end;
begin
  FrameIndex:=0;

  GOPListCount:=0;

  rfs:=TFileStream.Create(m1vfn,fmOpenRead);

  filepos:=0;
  filesize:=rfs.Size;

  SetPrgBarPos(0,'');
  Main.prgbar.Max:=filesize;

  rfs.Position:=filepos;
  rfs.ReadBuffer(readbuf[0],readbufsize);
  readbufpos:=0;

  while((filepos+chkbufsize)<filesize) do begin
    getchkbuf;

    if (chkbuf[$00]=$00) and (chkbuf[$01]=$00) then begin
      chkflag:=True;
      // Seq.Header
      if chkbuf[$02]<>$01 then chkflag:=False;
      if chkbuf[$03]<>$b3 then chkflag:=False;
      if chkbuf[$08]<>$ff then chkflag:=False;
      if chkbuf[$09]<>$ff then chkflag:=False;

      if chkflag=True then begin
        SetPrgBarPos(filepos,'');
        setlength(GOPList,GOPListCount+1);
        GOPList[GOPListCount].FrameIndex:=FrameIndex;
        GOPList[GOPListCount].Offset:=filepos;
        inc(GOPListCount);
      end;

      chkflag:=True;
      // Pic.Header
      if chkbuf[$02]<>$01 then chkflag:=False;
      if chkbuf[$03]<>$00 then chkflag:=False;

      if chkflag=True then begin
        PicData:=0;
        PicData:=PicData or (chkbuf[$04] shl 24);
        PicData:=PicData or (chkbuf[$05] shl 16);
        PicData:=PicData or (chkbuf[$06] shl 8);
        PicData:=PicData or (chkbuf[$07] shl 0);
        PicData_refidx:=PicData shr 22;
        PicData_type:=(PicData shr 19) and 7;
        if (PicData_refidx<$400) and ((PicData_type=1) or (PicData_type=2) or (PicData_type=3)) then begin
          inc(FrameIndex);
        end;
      end;
    end;

    inc(filepos);
    inc(readbufpos);
  end;

  rfs.Free;

  SetPrgBarPos(0,'');

  wfs:=TFileStream.Create(GOPListfn,fmCreate);
  for idx:=0 to GOPListCount-1 do begin
    wfs.WriteBuffer(GOPList[idx].FrameIndex,4);
    wfs.WriteBuffer(GOPList[idx].Offset,4);
  end;
  wfs.Free;

  Result:=True;
end;

function TMain.StartEncode:boolean;
begin
  Result:=False;

  Main.Caption:='Demultiplex AudioStream...';
  SetPrgBarPos(0,'Encode Audio');

  encaudio_StartPath:=StartPath;
  encaudio_PluginPath:=PluginPath;
  
  if AudioInfo.Freq=32768 then begin
    if DemuxAudio_HQ32768Hz_MEncoder(GetSourceFilename,GetDPGWaveFilename,AudioInfo.Volume)=False then exit;
    if EncodeMP2_HQ32768Hz_twolame(GetDPGWaveFilename,GetDPGMP2Filename,DPGINFO.SndFreq,AudioInfo.kbps)=False then exit;
    end else begin
    case AudioInfo.GetMode of
      SndGetMode_MEncoder: begin
        if DirectEncodeMP2_MEncoder(GetSourceFilename,GetDPGMP2Filename,DPGINFO.SndFreq,AudioInfo.kbps)=False then exit;
      end;
      SndGetMode_ExtractWaveNormal: begin
        if AudioInfo.GetMode=SndGetMode_External then begin
          if DemuxAudio(GetSourceFilename,GetDPGWaveFilename,False)=False then exit;
        end;
        if EncodeMP2(GetDPGWaveFilename,GetDPGMP2Filename,DPGINFO.SndFreq,AudioInfo.kbps)=False then exit;
      end;
      SndGetMode_ExtractWaveAlt: begin
        if AudioInfo.GetMode=SndGetMode_External then begin
          if DemuxAudio(GetSourceFilename,GetDPGWaveFilename,True)=False then exit;
        end;
        if EncodeMP2(GetDPGWaveFilename,GetDPGMP2Filename,DPGINFO.SndFreq,AudioInfo.kbps)=False then exit;
      end;
      SndGetMode_FFMpeg: begin
        if DirectEncodeMP2_FFMpeg(GetSourceFilename,GetDPGMP2Filename,DPGINFO.SndFreq,AudioInfo.kbps)=False then exit;
      end;
      SndGetMode_External: begin
        if FileExists(GetExternalWaveFilename)=False then begin
          Current_SetError('not found external wave file. '+GetExternalWaveFilename,'');
          exit;
        end;
        if EncodeMP2(GetExternalWaveFilename,GetDPGMP2Filename,DPGINFO.SndFreq,AudioInfo.kbps)=False then exit;
      end;
    end;
  end;

  if GetFileSize(GetDPGMP2Filename)<1024 then begin
    Current_SetError(lng(LI_GetAudioError),'');
    exit;
  end;

  if Current_GetRequestCancel=True then exit;

  Main.Caption:='Encode VideoStream...';
  SetPrgBarPos(0,'Encode Video');

  Main.Caption:='Encode VideoStream...';
  if EncodeVideo(GetSourceFilename,GetDPGMovieFilename)=False then exit;

  if Current_GetRequestCancel=True then exit;

  Main.Caption:='Create GOP list...';
  if CreateGOPList(GetDPGMovieFilename,GetDPGGOPListFilename)=False then exit;

  Result:=True;
  Main.Caption:='';
end;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// ------------------------------------------------------------
// ------------------------------------------------------------

procedure TMain.EncodeProcTimerTimer(Sender: TObject);
begin
  EncodeProcTimer.Enabled:=False;
  if Current_GetRequestCancel=True then begin
    EncodeEndTimer.Enabled:=True;
    exit;
  end;

  if StartEncode=True then begin
    if CreateDPG=True then begin
      EncodeEndTimer.Enabled:=True;
      exit;
    end;
  end;

  EncodeEndTimer.Enabled:=True;
end;

procedure TMain.EncodeEndTimerTimer(Sender: TObject);
  procedure exdel(fn:string);
  var
    timeout:integer;
  begin
    if FileExists(fn)=False then exit;

    timeout:=10000; // max 10sec.
    while(True) do begin
      DeleteFile(fn);
      if FileExists(fn)=False then break;
      Application.ProcessMessages;
      sleep(100);
      dec(timeout,100);
      if timeout<0 then break;
    end;
  end;
begin
  EncodeEndTimer.Enabled:=False;

  SetPriorityLevel(3);
  SetDefaultPriorityLevel(3);

  if GetSourceFilename<>GetDPGMovieFilename then exdel(GetDPGMovieFilename);
  exdel(GetDPGWaveFilename);
  exdel(GetDPGMP2Filename);
  exdel(GetDPGGOPListFilename);
  exdel(GetTempFilename);
  exdel(GetTempffmpegFilename);

  if VideoInfo.RequestDeleteOverrideSource=True then begin
    if Current_GetSrcFilename<>GetSourceFilename then exdel(GetSourceFilename);
  end;

  Main.Caption:=Application.Title;

  if Current_isError=True then begin
    exdel(GetDPGFilename);
    QueueSetResult('NG',Current_GetErrorMsg);
    end else begin
    if Current_GetRequestCancel=True then begin
      exdel(GetDPGFilename);
      QueueSetResult('NG',lng(LI_Canceled));
      end else begin
      QueueSetResult('OK','');
      if isQueueLast=True then begin
        if (enclog.Visible=True) or (encprv.Visible=True) then begin
          ShowMessage('Encode was completed.'+CRLF+CRLF+LoadDPGINFOString(GetDPGFilename));
        end;
      end;
    end;
  end;

  QueueNext;
end;

procedure TMain.FormKeyUp(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if Key=VK_ESCAPE then Main.Close;
end;

procedure TMain.MainMenu_Language_TemplateClick(Sender: TObject);
var
  t:TMenuItem;
  cnt,count:integer;
begin
  t:=Main.MainMenu_Language;
  count:=t.Count;
  for cnt:=0 to count-1 do begin
    t.Items[cnt].Checked:=False;
  end;

  (Sender as TMenuItem).Checked:=True;

  Main.LoadLanguage;

  Option.SaveINI;

  if QueueGetQueueCount=0 then begin
    QueueGrid.Cells[1,0]:=lng(LI_QueueMessage);
    RefreshOutputPathLbl;
  end;
end;

procedure TMain.MainMenu_OptionClick(Sender: TObject);
begin
  if QueueNowEncoding=True then begin
    ShowMessage('エンコード中は変更できません。'+CRLF+'can not change while processing.');
    exit;
  end;

  Option.ShowModal;
end;

procedure TMain.QueueGridDrawCell(Sender: TObject; ACol, ARow: Integer;
  Rect: TRect; State: TGridDrawState);
var
  ps:string;
  fn,msg0,msg1:string;
  x,y,w,h:integer;
begin
  with (Sender as TStringGrid).Canvas do begin
    FillRect(Rect);

    x:=Rect.Left+2;
    y:=Rect.Top+1;
    w:=Rect.Right-2-x;
    h:=Rect.Bottom-1-y;

    case ACol of
      0: begin
        ps:=QueueGrid.Cells[0,ARow];
        if ps='OK' then begin
          Draw(x+((w-32) div 2),y+((h-16) div 2),Main.ing_okimg.Picture.Graphic);
          end else begin
          if ps='NG' then begin
            Draw(x+((w-32) div 2),y+((h-16) div 2),Main.ing_ngimg.Picture.Graphic);
            end else begin
            if ansipos(' ',ps)=0 then begin
              TextOut(x+((w-TextWidth(ps)) div 2),y+((h-(h div 2)) div 2),ps);
              end else begin
              msg0:=copy(ps,1,AnsiPos(' ',ps)-1);
              msg1:=copy(ps,AnsiPos(' ',ps)+1,length(ps));
              h:=h div 2;
//              TextOut(x+((w-TextWidth(msg0)) div 2),y+(h*0),msg0);
//              TextOut(x+((w-TextWidth(msg1)) div 2),y+(h*1),msg1);
              TextOut(x,y+(h*0),msg0);
              TextOut(x,y+(h*1),msg1);
            end;
          end;
        end;
      end;
      1: begin
        fn:=QueueGrid.Cells[1,ARow];

        if AnsiPos(CRLF,fn)=0 then begin
          msg0:=ExtractFilePath(fn);
          msg1:=ExtractFilename(fn);
          end else begin
          msg0:=copy(fn,1,AnsiPos(CRLF,fn)-1);
          msg1:=copy(fn,AnsiPos(CRLF,fn)+2,length(fn));
        end;

        h:=h div 2;
        TextOut(x+2,y+(h*0),msg0);
        TextOut(x+2,y+(h*1),msg1);
      end;
    end;

  end;
end;

procedure TMain.MainMenu_ShowLogClick(Sender: TObject);
begin
  enclog.Visible:=True;

  with enclog do begin
    encprv.Top:=Top;
    encprv.Left:=Left+Width;
    encprv.ClientWidth:=256;
    encprv.ClientHeight:=192;
  end;
  encprv.Visible:=True;
end;

procedure TMain.QueuePopup_DeleteClick(Sender: TObject);
var
  delidx:integer;
  idx:integer;
begin
  delidx:=QueueGrid.Selection.Top;

  if delidx<QueueGetQueueIdx then begin
    exit;
  end;
  
  if delidx=QueueGetQueueIdx then begin
    Current_RequestCancel;
    exit;
  end;

  for idx:=delidx to QueueGetQueueCount-1 do begin
    QueueGrid.Cells[0,idx]:=QueueGrid.Cells[0,idx+1];
    QueueGrid.Cells[1,idx]:=QueueGrid.Cells[1,idx+1];
  end;

  QueueDecQueueCount;
  QueueGrid.RowCount:=QueueGetQueueCount;
end;

procedure TMain.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
  if QueueNowEncoding=True then begin
    ShowMessage('エンコード中は終了できません。'+CRLF+'can not terminate while processing.');
    CanClose:=False;
  end;
end;

procedure TMain.MainMenu_File_ExitClick(Sender: TObject);
begin
  Main.Close;
end;

procedure TMain.MainMenu_AboutClick(Sender: TObject);
begin
  ShowMessage('nDs-mPeG encoder'+CRLF+DPGEncVersion+' by Moonlight');
end;

procedure TMain.FormResize(Sender: TObject);
begin
  with OutputPathChangeBtn do begin
    Left:=Main.ClientWidth-Width-8;
  end;

  with prgbar do begin
    Width:=Main.ClientWidth-Left-8;
  end;

  with QueueGrid do begin
    Width:=Main.ClientWidth-Left-8;
    Height:=Main.ClientHeight-Top-StatusBar1.Height-8;
  end;

  MainFormWidth:=Main.Width;
  MainFormHeight:=Main.Height;
end;

procedure TMain.OutputPathChangeBtnClick(Sender: TObject);
begin
  if QueueNowEncoding=True then begin
    ShowMessage('エンコード中は変更できません。'+CRLF+'can not change while processing.');
    exit;
  end;

  OutputPathDlg.InitialDir:=OptOutputPath;
  OutputPathDlg.Title:=lng(LI_OutputPathDlgTitle);
  OutputPathDlg.FileName:=lng(LI_OutputPathDlgFilename);

  if OutputPathDlg.Execute=True then begin
    OptOutputPath:=ExtractFilePath(OutputPathDlg.FileName);
    RefreshOutputPathLbl;
    Option.SaveINI;
  end;
end;

procedure TMain.QueueGridKeyUp(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if Key=VK_ESCAPE then Main.Close;
  if Key=VK_DELETE then QueuePopup_DeleteClick(nil);
end;

procedure TMain.QueueGridMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
  p:TPoint;
begin
  if Button=mbRight then begin
    p.X:=x;
    p.Y:=y;
    p:=QueueGrid.ClientToScreen(p);
    QueuePopup.Popup(p.x,p.y);
  end;
end;

procedure TMain.QueueGridMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
  col,row:integer;
  sel:TGridRect;
  e:boolean;
begin
  if Button=mbRight then begin
    QueueGrid.MouseToCell(x,y,col,row);
    sel.Left:=0;
    sel.Right:=1;
    sel.Top:=row;
    sel.Bottom:=row;
    QueueGrid.Selection:=sel;
    QueueGrid.Refresh;

    if QueueGetQueueCount=0 then begin
      e:=False; // for tip
      end else begin
      if row<QueueGetQueueIdx then begin
        e:=False; // for Ended files.
        end else begin
        if row=QueueGetQueueIdx then begin
          // for Current file.
          if Current_GetRequestCancel=True then begin
            e:=False;
            end else begin
            e:=True;
          end;
          end else begin
          e:=True; // for Queue files.
        end;
      end;
    end;
    QueuePopup_Delete.Enabled:=e;
  end;
end;

procedure TMain.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Option.SaveINI;
end;

end.


