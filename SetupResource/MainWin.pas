unit MainWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, _inifile, StdCtrls, Buttons, _resfile, _SplitML,_dosbox;

type
  TMain = class(TForm)
    StartupTimer: TTimer;
    GroupBox1: TGroupBox;
    confGlobalINIChk: TCheckBox;
    confDesktopBMPChk: TCheckBox;
    confStartupMP3Chk: TCheckBox;
    Label2: TLabel;
    GroupBox2: TGroupBox;
    langUpdateChk: TCheckBox;
    langCodePageLst: TComboBox;
    GroupBox3: TGroupBox;
    SetupBtn: TBitBtn;
    CancelBtn: TBitBtn;
    TargetDriveLbl: TLabel;
    sysExistBackupChk: TCheckBox;
    langAllItemChk: TCheckBox;
    confPluginChk: TCheckBox;
    romEXFSCreate1Lbl: TLabel;
    GroupBox4: TGroupBox;
    clockUpdateChk: TCheckBox;
    clockNameLst: TComboBox;
    ClkPrvBtn: TButton;
    ClkPrvPosTimer: TTimer;
    romMPCFChk: TCheckBox;
    romSCCFChk: TCheckBox;
    romM3SDChk: TCheckBox;
    romM3CFChk: TCheckBox;
    romEZSDChk: TCheckBox;
    romMMCFChk: TCheckBox;
    romSCSDChk: TCheckBox;
    romSCMSChk: TCheckBox;
    romEWSDChk: TCheckBox;
    romNMMCChk: TCheckBox;
    confShutdownMP3Chk: TCheckBox;
    romEXFSCreate2Lbl: TLabel;
    confResumeChk: TCheckBox;
    romNJSDChk: TCheckBox;
    confBuiltinPluginChk: TCheckBox;
    romDLMSChk: TCheckBox;
    confBookmarkChk: TCheckBox;
    procedure FormCreate(Sender: TObject);
    procedure StartupTimerTimer(Sender: TObject);
    procedure SetupBtnClick(Sender: TObject);
    procedure CancelBtnClick(Sender: TObject);
    procedure langAllItemChkClick(Sender: TObject);
    procedure ClkPrvBtnClick(Sender: TObject);
    procedure clockNameLstChange(Sender: TObject);
    procedure ClkPrvPosTimerTimer(Sender: TObject);
    procedure confResumeChkClick(Sender: TObject);
    procedure romChkClick(Sender: TObject);
    procedure confBookmarkChkClick(Sender: TObject);
  private
    { Private 宣言 }
    ReadyClkPrv:boolean;
    procedure clockNameLstReset;
    procedure langCodePageLstReset(AllFlag:boolean);
  public
    { Public 宣言 }
  end;

var
  Main: TMain;

procedure filecopy_allinpath(srcpath,dstpath:string);

implementation

uses SelDrvWin, LoadingWin, ClkPrvWin, SelMLWin, SetuppedWin, ROMInfoWin;

{$R *.dfm}

const CRLF:string=char($0d)+char($0a);

var
  TargetDrive:string;

var
  StartPath,TargetPath:string;
  LogFilename:string;


procedure WriteLog(str:string);
var
  tf:TextFile;
begin
  AssignFile(tf,LogFilename);
  if FileExists(LogFilename)=False then begin
    Rewrite(tf);
    end else begin
    Append(tf);
  end;

  Writeln(tf,str);

  CloseFile(tf);

  Main.Caption:=str;
end;

procedure TMain.FormCreate(Sender: TObject);
begin
  Application.Title:='Setup MoonShell Version 1.5';
  Main.Caption:=Application.Title;

  StartPath:=ExtractFilePath(Application.ExeName);

  TargetPath:='';

  LogFilename:=ChangeFileExt(Application.ExeName,'.log');
  DeleteFile(LogFilename);

  ReadyClkPrv:=False;

  StartupTimer.Enabled:=True;
end;

procedure TMain.clockNameLstReset;
var
  LastIndex:integer;      
begin
  LastIndex:=clockNameLst.ItemIndex;

  clockNameLst.Clear;
  clockNameLst.Items.Add('Disabled');
  clockNameLst.Items.Add('clk_analogdot');
  clockNameLst.Items.Add('clk_analogled');
  clockNameLst.Items.Add('clk_bluemoon1');
  clockNameLst.Items.Add('clk_bluemoon2');
  clockNameLst.Items.Add('clk_bluemoon3');
  clockNameLst.Items.Add('clk_bluemoon4');
  clockNameLst.Items.Add('clk_bluemoon5');
  clockNameLst.Items.Add('clk_chocolate');
  clockNameLst.Items.Add('clk_chocolate2');
  clockNameLst.Items.Add('clk_simple');
  clockNameLst.Items.Add('clk_vu');

  with clockNameLst do begin
    if Items.Count<=LastIndex then LastIndex:=0;
    ItemIndex:=LastIndex;
  end;
end;

procedure TMain.langCodePageLstReset(AllFlag:boolean);
var
  LastIndex:integer;
begin
  with langCodePageLst do begin
    LastIndex:=ItemIndex;
    Clear;

    Items.Add('MS0 (ank only) small memory package. (省メモリ)');
    Items.Add('MS100 (UTF-8) Unicode Windows CharSet');
    Items.Add('MS110 (UTF-8test) 8pt large font for request.');
    Items.Add('CP874 (windows-874) Thai (Windows)');
    Items.Add('CP932 (shift-jis) Japanese (Shift-JIS) 標準日本語フォント');
    if AllFlag=True then begin
      Items.Add('CP9322 (shift-jis) Japanese (Shift-JIS) LCフォント風');
      Items.Add('CP9323 (shift-jis) Japanese (Shift-JIS) 固定幅フォント');
      Items.Add('CP9324 (shift-jis) Japanese (Shift-JIS) 標準日本語フォント ArialUnicode版');
      Items.Add('CP9325 (shift-jis) Japanese (Shift-JIS) 固定幅フォント ArialUnicode版');
    end;
    Items.Add('CP936 (gb2312) Chinese Simplified by Skikeyu & Fachman');
    Items.Add('CP9362 (gb2312) Chinese Simplified Old version');
    Items.Add('CP949 (EUC-KR) Unified Hangul');
    Items.Add('CP950 (big5) Chinese Traditional (Big5) by czw');
    Items.Add('CP9502 (big5) Chinese Traditional (Big5) Old version');
    if AllFlag=True then begin
      Items.Add('CP1250 (windows-1250) Central European Alphabet (Windows)');
      Items.Add('CP1251 (windows-1251) Cyrillic Alphabet (Windows)');
      Items.Add('CP1252 (windows-1252) Western Alphabet (Windows)');
      Items.Add('CP1253 (windows-1253) Greek Alphabet (Windows)');
      Items.Add('CP1254 (windows-1254) Turkish Alphabet');
      Items.Add('CP1255 (windows-1255) Hebrew Alphabet (Windows)');
      Items.Add('CP1256 (windows-1256) Arabic Alphabet (Windows)');
      Items.Add('CP1257 (windows-1257) Baltic Alphabet (Windows)');
      Items.Add('CP1258 (windows-1258) Vietnamese Alphabet (Windows)');
    end;

    if Items.Count<=LastIndex then LastIndex:=4;
    ItemIndex:=LastIndex;
  end;
end;

procedure ExtractAll(dstpath:string);
var
  strs:TStrings;
  idx:integer;
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  strs:=TStringList.Create;

  resfile_FindFile('','',strs);

  for idx:=0 to strs.Count-1 do begin
    fn:=strs[idx];

    ForceDirectories(ExtractFilePath(dstpath+fn));

    fs:=TFileStream.Create(dstpath+fn,fmCreate);
    resfile_GetData(fn,resdata);
    fs.Write(resdata.data[0],resdata.size);
    fs.Free;
  end;

  strs.Free;

  ShowMessage('Extracted. ['+dstpath+']');
end;

procedure CreateFontPackage_One(fpkfn,ankfn,l2ufn,fonfn:string);
var
  wfs:TFileStream;
  adrpos:integer;
  adrtbl:array[0..5] of dword;
  dw:dword;
  procedure Padding;
  var
    ofs:integer;
    dw:dword;
  begin
    ofs:=wfs.Position;
    ofs:=ofs and $3;
    ofs:=4-ofs;
    if (ofs mod 4)<>0 then begin
      dw:=0;
      wfs.WriteBuffer(dw,ofs);
    end;
  end;
  procedure WriteFile(idx:integer;fn:string);
  var
    resdata:Tresdata;
  begin
    resfile_GetData(fn,resdata);

    adrtbl[idx*2+0]:=wfs.Position;
    adrtbl[idx*2+1]:=resdata.size;
    wfs.WriteBuffer(resdata.data[0],resdata.size);
  end;
begin
  wfs:=TFileStream.Create(fpkfn,fmCreate);

  dw:=$006b7066; // fpk\0
  wfs.WriteBuffer(dw,4);

  adrpos:=wfs.Position;
  wfs.WriteBuffer(adrtbl,3*2*4);

  Padding;
  WriteFile(0,ankfn);
  Padding;
  WriteFile(1,l2ufn);
  Padding;
  WriteFile(2,fonfn);
  Padding;

  wfs.Position:=adrpos;
  wfs.WriteBuffer(adrtbl,3*2*4);

  wfs.Free;
end;

procedure CreateFontPackage(dstpath:string);
var
  msg:string;
  procedure cr(FontName:string);
  var
    cp:string;
  begin
    cp:=copy(FontName,1,ansipos('_',FontName)-1);
    cp:='CP'+copy(cp,3,length(cp))+'\system';
    CreateFontPackage_One(dstpath+FontName+'-system.fpk',cp+'.ank',cp+'.l2u',cp+'.fon');
  end;
begin
  cr('MS0_ank only_small memory package');
  cr('MS100_UTF-8_Unicode Windows CharSet');
  cr('MS110_UTF-8test_8pt large font');
  cr('CP874_windows-874_Thai');
  cr('CP932_shift-jis_Japanese_Default');
  cr('CP9322_shift-jis_Japanese_LCFontLike');
  cr('CP9323_shift-jis_Japanese_Fixed');
  cr('CP9324_shift-jis_Japanese_Default_ArialUnicode');
  cr('CP9325_shift-jis_Japanese_Fixed_ArialUnicode');
  cr('CP936_gb2312_Chinese Simplified by Skikeyu & Fachman');
  cr('CP9362_gb2312_Chinese Simplified Old version');
  cr('CP949_EUC-KR_Unified Hangul');
  cr('CP950_big5_Chinese Traditional');
  cr('CP1250_windows-1250_Central European Alphabet');
  cr('CP1251_windows-1251_Cyrillic Alphabet');
  cr('CP1252_windows-1252_Western Alphabet');
  cr('CP1253_windows-1253_Greek Alphabet');
  cr('CP1254_windows-1254_Turkish Alphabet');
  cr('CP1255_windows-1255_Hebrew Alphabet');
  cr('CP1256_windows-1256_Arabic Alphabet');
  cr('CP1257_windows-1257_Baltic Alphabet');
  cr('CP1258_windows-1258_Vietnamese Alphabet');

  msg:=msg+'The font package file was created.'+CRLF;
  msg:=msg+'Please put in CF/SD/EXFS and select it from MoonShell.'+CRLF;
  msg:=msg+'|';
  msg:=msg+'フォントパッケージファイルを作成しました。'+CRLF;
  msg:=msg+'CF/SD/EXFSに入れてMoonShellから選択してください。'+CRLF;

  ShowMessage(dstpath+'*.fpk'+CRLF+CRLF+GetMLStr(msg));
end;

procedure TMain.StartupTimerTimer(Sender: TObject);
var
  lx,ly:integer;
begin
  StartupTimer.Enabled:=False;

  if GetMLTypeLoaded=False then begin
    if SelML.ShowModal=mrCancel then begin
      Application.Terminate;
      exit;
    end;
  end;

  SetMLLbl(Label2);

  SetMLGrp(GroupBox1);
  SetMLChk(confGlobalINIChk);
  SetMLChk(confDesktopBMPChk);
  SetMLChk(confStartupMP3Chk);
  SetMLChk(confShutdownMP3Chk);
  SetMLChk(confPluginChk);
  SetMLChk(confResumeChk);
  SetMLChk(confBookmarkChk);
  SetMLChk(confBuiltinPluginChk);
  
  SetMLGrp(GroupBox4);
  SetMLChk(clockUpdateChk);
  SetMLBtn(ClkPrvBtn);

  SetMLGrp(GroupBox2);
  SetMLChk(langUpdateChk);
  SetMLChk(langAllItemChk);

  SetMLGrp(GroupBox3);
  SetMLLbl(romEXFSCreate1Lbl);
  SetMLLbl(romEXFSCreate2Lbl);
  SetMLChk(romMPCFChk);
  SetMLChk(romSCCFChk);
  SetMLChk(romSCSDChk);
  SetMLChk(romSCMSChk);
  SetMLChk(romM3CFChk);
  SetMLChk(romM3SDChk);
  SetMLChk(romMMCFChk);
  SetMLChk(romEZSDChk);
  SetMLChk(romEWSDChk);
  SetMLChk(romNMMCChk);
  SetMLChk(romNJSDChk);
  SetMLChk(romDLMSChk);

  SetMLChk(sysExistBackupChk);

  SetMLBitBtn(SetupBtn);
  SetMLBitBtn(CancelBtn);

  Loading.Visible:=True;
  Loading.Refresh;
  resfile_Init;
  Loading.Visible:=False;

  lx:=Main.Left;
  ly:=Main.Top;
//  Main.Left:=-Main.Width;
//  Main.Top:=-Main.Top;

  clockNameLst.ItemIndex:=0;
  clockNameLstReset;

  langCodePageLst.ItemIndex:=0;
  langCodePageLstReset(True);
  langCodePageLst.ItemIndex:=4;
  LoadINI;
  langCodePageLstReset(langAllItemChk.Checked);

  TargetDriveLbl.Caption:='';

  SelDrv.Caption:=Application.Title;
  SelDrv.InitML;
  if SelDrv.ShowModal=mrCancel then begin
    Application.Terminate;
    exit;
  end;

  if SelDrv.ReqExtractPath<>'' then begin
    ExtractAll(SelDrv.ReqExtractPath);
    Application.Terminate;
    exit;
  end;

  if SelDrv.ReqFontPackagePath<>'' then begin
    CreateFontPackage(SelDrv.ReqFontPackagePath);
    Application.Terminate;
    exit;
  end;

  TargetDriveLbl.Caption:='TargetDrive '+SelDrv.TargetDriveLst.Text;
  TargetDrive:=GetSepSpaceStr(SelDrv.TargetDriveLst.Text);

  Main.Left:=lx;
  Main.Top:=ly;

  if TargetDrive='EXFS' then begin
    confResumeChk.Checked:=False;
    confResumeChk.Enabled:=False;
    confBookmarkChk.Checked:=False;
    confBookmarkChk.Enabled:=False;
    with romEXFSCreate1Lbl do begin
      Left:=romMPCFChk.Left;
      Top:=romMPCFChk.Top+32;
      Visible:=True;
    end;
    with romEXFSCreate2Lbl do begin
      Left:=romEXFSCreate1Lbl.Left;
      Top:=romEXFSCreate1Lbl.Top+romEXFSCreate1Lbl.Height+8;
      Visible:=True;
    end;
    romMPCFChk.Visible:=False;
    romSCCFChk.Visible:=False;
    romM3SDChk.Visible:=False;
    romM3CFChk.Visible:=False;
    romEZSDChk.Visible:=False;
    romMMCFChk.Visible:=False;
    romSCSDChk.Visible:=False;
    romSCMSChk.Visible:=False;
    romEWSDChk.Visible:=False;
    romNMMCChk.Visible:=False;
    romNJSDChk.Visible:=False;
    ROMInfo.Visible:=False;
    end else begin
    romMPCFChk.Visible:=True;
    romSCCFChk.Visible:=True;
    romM3SDChk.Visible:=True;
    romM3CFChk.Visible:=True;
    romEZSDChk.Visible:=True;             
    romMMCFChk.Visible:=True;
    romSCSDChk.Visible:=True;
    romSCMSChk.Visible:=True;
    romEWSDChk.Visible:=True;
    romNMMCChk.Visible:=True;
    romNJSDChk.Visible:=True;
    ROMInfo.Visible:=True;
    ROMInfo.MsgMemoRefresh;
    Main.SetFocus;
  end;

  ReadyClkPrv:=True;
  clockNameLstChange(nil);
  ClkPrvPosTimer.Enabled:=True;
end;

procedure filecopy(srcpath,dstpath,fn:string);
var
  fs:TFileStream;
  buf:array of byte;
  bufsize:integer;
  resdata:Tresdata;
begin
  if Main.sysExistBackupChk.Checked=True then begin
    if FileExists(dstpath+fn)=True then begin
      WriteLog('Create backup to "'+dstpath+fn+'.bak"');

      fs:=TFileStream.Create(dstpath+fn,fmOpenRead);
      bufsize:=fs.Size;
      setlength(buf,bufsize);
      fs.ReadBuffer(buf[0],bufsize);
      fs.Free;

      fs:=TFileStream.Create(dstpath+fn+'.bak',fmCreate);
      fs.Write(buf[0],bufsize);
      fs.Free;
    end;
  end;                                       

  WriteLog('copy "'+srcpath+fn+'" to "'+dstpath+fn+'"');

  ForceDirectories(dstpath);

  fs:=TFileStream.Create(dstpath+fn,fmCreate);
  resfile_GetData(srcpath+fn,resdata);
  fs.Write(resdata.data[0],resdata.size);
  fs.Free;
end;

procedure filecopy_allinpath(srcpath,dstpath:string);
var
  strs:TStrings;
  idx:integer;
  fn:string;
begin
  strs:=TStringList.Create;

  resfile_FindFile(srcpath,'',strs);

  for idx:=0 to strs.Count-1 do begin
    fn:=ExtractFilename(strs[idx]);
    filecopy(srcpath,dstpath,fn);
  end;

  strs.Free;
end;

procedure ExecConfiguration;
var
  srcpath,dstpath:string;
  procedure CopyWhenNotExists(fn:string);
  begin
    if FileExists(dstpath+fn)=False then filecopy(srcpath,dstpath,fn);
  end;
  procedure DeleteWhenExists(fn:string);
  var
    msg:string;
  begin
    if FileExists(dstpath+fn)=False then exit;
    if DeleteFile(dstpath+fn)=True then exit;
    msg:='';
    msg:=msg+fn+CRLF;
    msg:=msg+CRLF;
    msg:=msg+'ファイルを削除できませんでした。'+CRLF;
    msg:=msg+'ほかのアプリケーションで開いていないか確認してください。'+CRLF;
    msg:=msg+CRLF;
    msg:=msg+'Failed in the deletion of file.'+CRLF;
    msg:=msg+'Please do not use file by other applications.'+CRLF;
    ShowMessage(msg);
  end;
begin
  srcpath:='shell\';
  dstpath:=TargetPath+'shell\';

  if Main.confGlobalINIChk.Checked=True then filecopy(srcpath,dstpath,'global.ini');
  if Main.confDesktopBMPChk.Checked=True then filecopy(srcpath,dstpath,'desktop.bmp');
  if Main.confStartupMP3Chk.Checked=True then filecopy(srcpath,dstpath,'startup.mp3');
  if Main.confShutdownMP3Chk.Checked=True then filecopy(srcpath,dstpath,'shutdown.mp3');
  if Main.confPluginChk.Checked=True then filecopy_allinpath(srcpath+'plugin\',dstpath+'plugin\');

  if Main.confResumeChk.Checked=True then begin
    filecopy(srcpath,dstpath,'resume.sav');
    end else begin
    DeleteWhenExists('resume.sav');
  end;

  if Main.confBookmarkChk.Checked=True then begin
    CopyWhenNotExists('bookmrk0.sav');
    CopyWhenNotExists('bookmrk1.sav');
    CopyWhenNotExists('bookmrk2.sav');
    CopyWhenNotExists('bookmrk3.sav');
    end else begin
    DeleteWhenExists('bookmrk0.sav');
    DeleteWhenExists('bookmrk1.sav');
    DeleteWhenExists('bookmrk2.sav');
    DeleteWhenExists('bookmrk3.sav');
  end;
end;

procedure ExecClock_RemoveInside(fullname:string);
  procedure backup(fn:string);
  var
    fs:TFileStream;
    buf:array of byte;
    bufsize:integer;
  begin
    if FileExists(fn)=False then exit;

    if Main.sysExistBackupChk.Checked=True then begin
      WriteLog('Create backup to "'+fn+'.bak"');

      fs:=TFileStream.Create(fn,fmOpenRead);
      bufsize:=fs.Size;
      setlength(buf,bufsize);
      fs.ReadBuffer(buf[0],bufsize);
      fs.Free;

      fs:=TFileStream.Create(fn+'.bak',fmCreate);
      fs.Write(buf[0],bufsize);
      fs.Free;
    end;

    WriteLog('Delete "'+fn+'"');
    DeleteFile(fn);
  end;
begin
  fullname:=ChangeFileExt(fullname,'');

  backup(fullname+'.msp');
  backup(fullname+'.ini');
  backup(fullname+'.bin');
end;

procedure ExecClock_Remove(path:string);
var
  res:integer;
  SearchRec: TSearchRec;
  fname:string;
  rfs:TFileStream;
  VersionHigh,PluginMode:byte;
begin
  res:=FindFirst(path+'*.msp', (FaAnyFile), SearchRec);
  if res=0 then begin
    repeat
      fname:=SearchRec.Name;
      if (SearchRec.Attr and faDirectory)=0 then begin
        if fname<>'' then begin
          rfs:=TFileStream.Create(path+fname,fmOpenRead);
          rfs.Position:=4;
          rfs.ReadBuffer(VersionHigh,1);
          rfs.Position:=6;
          rfs.ReadBuffer(PluginMode,1);
          rfs.Free;
          if (3<=VersionHigh) and (PluginMode=3) then begin
            ExecClock_RemoveInside(path+fname);
          end;
        end;
      end;
      res:=FindNext(SearchRec);
    until (res<>0);
  end;
  FindClose(SearchRec);
end;

procedure ExecClock;
var
  PluginName:string;
  srcpath,dstpath:string;
begin
  if Main.clockUpdateChk.Checked=False then exit;

  PluginName:=Main.clockNameLst.Text;

  srcpath:='ClockPlugin\'+PluginName+'\';
  dstpath:=TargetPath+'shell\plugin\';

  ExecClock_Remove(dstpath);

  if PluginName<>'Disabled' then begin
    if resfile_FileExists(srcpath+PluginName+'.msp')=True then filecopy(srcpath,dstpath,PluginName+'.msp');
    if resfile_FileExists(srcpath+PluginName+'.ini')=True then filecopy(srcpath,dstpath,PluginName+'.ini');
    if resfile_FileExists(srcpath+PluginName+'.bin')=True then filecopy(srcpath,dstpath,PluginName+'.bin');
  end;
end;

procedure ExecLanguage;
var
  cp:integer;
  srcpath,dstpath:string;
begin
  if Main.langUpdateChk.Checked=False then exit;

  cp:=strtointdef(GetSepSpaceStr(copy(Main.langCodePageLst.Text,3,255)),0);
  srcpath:='CP'+inttostr(cp)+'\';
  dstpath:=TargetPath+'shell\';

  filecopy(srcpath,dstpath,'system.ank');
  filecopy(srcpath,dstpath,'system.fon');
  filecopy(srcpath,dstpath,'system.l2u');
end;

// --- execrom utils

const BootStrapBin_SuperCard='BootStrap_SuperCard.bin';
const BootStrapBin_M3='BootStrap_M3_pepsiman-m3powerloader-20060117.bin';

procedure ExecRom_GetROM(var resdata:Tresdata);
begin
  if Main.confBuiltinPluginChk.Checked=True then begin
    resfile_GetData('_BOOT_MP.nds',resdata);
    end else begin
    resfile_GetData('_BOOT_MP_Mini.nds',resdata);
  end;
end;

procedure ExecRom_GetBootStrap(fn:string;var resdata:Tresdata);
begin
  resfile_GetData(fn,resdata);
end;

procedure ExecRom_SetAdapter(var buf:array of byte;bufsize:integer;ID:string);
var
  idx:integer;
  tagpos:integer;
begin
  for idx:=0 to bufsize-14 do begin
    if ((buf[idx+0]=byte('A')) and (buf[idx+1]=byte('d')) and (buf[idx+2]=byte('a')) and (buf[idx+3]=byte('p')) and (buf[idx+4]=byte('t')) and (buf[idx+5]=byte('e')) and (buf[idx+6]=byte('r')) and (buf[idx+7]=byte(':'))) then begin
      tagpos:=idx+8;
      buf[tagpos+0]:=byte(ID[1]);
      buf[tagpos+1]:=byte(ID[2]);
      buf[tagpos+2]:=byte(ID[3]);
      buf[tagpos+3]:=byte(ID[4]);
      exit;
    end;
  end;

  ShowMessage('アダプタIDが見つかりませんでした。('+ID+')');
end;

// --- end of execrom utils

procedure ExecRom_EXFS;
var
  fs:TFileStream;
  resdata:Tresdata;
begin
  ForceDirectories(StartPath);
  fs:=TFileStream.Create(StartPath+'_BOOT_MP.nds',fmCreate);

  ExecRom_GetROM(resdata);
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_MPCF;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP.nds';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'" for GBAMP/AdMovie');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'MPCF');
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_SCCF;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP_SCCF_SuperCard_CF.nds';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'"');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetBootStrap(BootStrapBin_SuperCard,resdata);
  fs.Write(resdata.data[0],resdata.size);
  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'SCCF');              
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_SCSD;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP_SCSD_SuperCard_SD.nds';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'"');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetBootStrap(BootStrapBin_SuperCard,resdata);
  fs.Write(resdata.data[0],resdata.size);
  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'SCSD');
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_SCMS;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP_SCMS_SuperCard_MicroSD.nds';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'"');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetBootStrap(BootStrapBin_SuperCard,resdata);
  fs.Write(resdata.data[0],resdata.size);
  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'SCMS');
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_M3CF;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP_M3CF_M3_CF.nds';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'"');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetBootStrap(BootStrapBin_M3,resdata);
  fs.Write(resdata.data[0],resdata.size);
  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'M3CF');
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_M3SD;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP_M3SD_M3_SD.nds';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'"');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetBootStrap(BootStrapBin_M3,resdata);
  fs.Write(resdata.data[0],resdata.size);
  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'M3SD');
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_MMCF;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP_MMCF_MaxMedia_CF.nds';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'"');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'MMCF');
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_EZSD;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP_EZSD_EZ-Flash4_SD.nds.bin';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'"');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'EZSD');
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_EWSD;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP_EWSD_Ewin2_MicroSD.nds';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'"');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'EWSD');
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_NMMC;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP_NMMC_MagicKey2-3_SD.nds';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'"');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'NMMC');
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_NJSD;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP_NJSD_NinjaDS_SD.nds';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'"');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'NJSD');
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom_DLMS;
var
  fn:string;
  fs:TFileStream;
  resdata:Tresdata;
begin
  fn:='_BOOT_MP_DLMS_DS-Link_MicroSD.nds';
  WriteLog('Create ROM image and copy to "'+TargetPath+fn+'"');

  ForceDirectories(TargetPath);
  fs:=TFileStream.Create(TargetPath+fn,fmCreate);

  ExecRom_GetROM(resdata);
  ExecRom_SetAdapter(resdata.data,resdata.size,'DLMS');
  fs.Write(resdata.data[0],resdata.size);

  fs.Free;
end;

procedure ExecRom;
begin
  if TargetDrive='EXFS' then begin
    ExecRom_EXFS;
    filecopy('',TargetPath,'readme_EXFS.txt');
    filecopy('',StartPath,'CreateNDSROM_for_EXFS.bat');
    filecopy_allinpath('wintools\',StartPath+'wintools\');
    exit;
  end;

  if Main.romMPCFChk.Checked=True then ExecRom_MPCF;
  if Main.romSCCFChk.Checked=True then ExecRom_SCCF;
  if Main.romSCSDChk.Checked=True then ExecRom_SCSD;
  if Main.romSCMSChk.Checked=True then ExecRom_SCMS;
  if Main.romM3CFChk.Checked=True then ExecRom_M3CF;
  if Main.romM3SDChk.Checked=True then ExecRom_M3SD;
  if Main.romMMCFChk.Checked=True then ExecRom_MMCF;
  if Main.romEZSDChk.Checked=True then ExecRom_EZSD;
  if Main.romEWSDChk.Checked=True then ExecRom_EWSD;
  if Main.romNMMCChk.Checked=True then ExecRom_NMMC;
  if Main.romNJSDChk.Checked=True then ExecRom_NJSD;
  if Main.romDLMSChk.Checked=True then ExecRom_DLMS;
end;

procedure TMain.SetupBtnClick(Sender: TObject);
begin
  if TargetDrive='EXFS' then begin
    TargetPath:=StartPath+'files_EXFS\';
    end else begin
    TargetPath:=TargetDrive;
  end;

  WriteLog('StartPath='+StartPath);
  WriteLog('TargetPath='+TargetPath);
  WriteLog('TargetInfo='+SelDrv.TargetDriveLst.Text);

  ExecConfiguration;
  ExecClock;
  ExecLanguage;
  ExecRom;

  Setupped.Caption:=Application.Title;
  Setupped.InitML;
  if TargetDrive='EXFS' then begin
    Setupped.ShowEXFS(True);
    end else begin
    Setupped.ShowEXFS(False);
  end;
  Setupped.ShowModal;

  if Setupped.OpenGlobalINIChk.Checked=True then begin
    CreateDOSBOX(False,TargetPath+'shell','"notepad.exe" "global.ini"');
  end;

  SaveINI;

  Application.Terminate;
end;

procedure TMain.CancelBtnClick(Sender: TObject);
begin
  Application.Terminate;
end;

procedure TMain.langAllItemChkClick(Sender: TObject);
begin
  langCodePageLstReset(langAllItemChk.Checked);
end;

procedure TMain.ClkPrvBtnClick(Sender: TObject);
begin
  ClkPrv.Visible:=True;
end;

procedure TMain.clockNameLstChange(Sender: TObject);
var
  PlugName:string;
begin
  if ReadyClkPrv=False then exit;

  if clockNameLst.ItemIndex=0 then begin
    PlugName:='';
    end else begin
    PlugName:=clockNameLst.Items[clockNameLst.ItemIndex];
  end;

  ClkPrv.Load(PlugName);
end;

procedure TMain.ClkPrvPosTimerTimer(Sender: TObject);
begin
  if ReadyClkPrv=False then exit;

  with Main do begin
    ClkPrv.Left:=Left+Width;
    ClkPrv.Top:=Top;
  end;
end;

procedure OpenWarnText(fn:string);
begin
  if FileExists(StartPath+fn)=True then DeleteFile(StartPath+fn);
  filecopy('',StartPath,fn);
  CreateDOSBOX(False,StartPath,'notepad.exe "'+fn+'"');
end;

procedure TMain.confResumeChkClick(Sender: TObject);
begin
  if confResumeChk.Checked=False then exit;
  OpenWarnText('resume.txt');
end;

procedure TMain.confBookmarkChkClick(Sender: TObject);
begin
  if confBookmarkChk.Checked=False then exit;
  OpenWarnText('bookmark.txt');
end;

procedure TMain.romChkClick(Sender: TObject);
begin
  ROMInfo.MsgMemoRefresh;
end;

end.
