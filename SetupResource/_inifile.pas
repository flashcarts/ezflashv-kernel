unit _inifile;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls, StdCtrls, ExtCtrls, Buttons, INIFiles;

function GetSepSpaceStr(str:string):string;
procedure LoadINI;
procedure SaveINI;

implementation

uses MainWin,SelDrvWin,SetuppedWin;

function GetSepSpaceStr(str:string):string;
var
  p:integer;
begin
  p:=ansipos(' ',str);

  if p=0 then begin
    Result:=str;
    end else begin
    Result:=copy(str,1,p-1);
  end;
end;

procedure LoadINI;
var
  fini:TINIFile;
  Section:string;
  procedure SetListStr(Section:string;ID:string;Lst:TComboBox);
  var
    idx:integer;
    str:string;
  begin
    str:=GetSepSpaceStr(fini.ReadString(Section,ID,''));
    for idx:=0 to Lst.Items.Count-1 do begin
      if GetSepSpaceStr(Lst.Items[idx])=str then Lst.ItemIndex:=idx;
    end;
  end;
  procedure SetListStrCP(Section:string;ID:string;Lst:TComboBox);
  var
    idx:integer;
    str:string;
  begin
    str:=GetSepSpaceStr(fini.ReadString(Section,ID,''));
    for idx:=0 to Lst.Items.Count-1 do begin
      if GetSepSpaceStr(copy(Lst.Items[idx],3,255))=str then Lst.ItemIndex:=idx;
    end;
  end;
begin
  fini:=TINIFile.Create(ChangeFileExt(Application.ExeName,'.ini'));

  Section:='system';

  SetListStr(Section,'TargetDrive',SelDrv.TargetDriveLst);
  Main.sysExistBackupChk.Checked:=fini.ReadBool(Section,'ExistBackup',Main.sysExistBackupChk.Checked);
  Setupped.OpenGlobalINIChk.Checked:=fini.ReadBool(Section,'OpenGlobalINI',Setupped.OpenGlobalINIChk.Checked);

  Section:='configuration';

  Main.confGlobalINIChk.Checked:=fini.ReadBool(Section,'global.ini',Main.confGlobalINIChk.Checked);
  Main.confDesktopBMPChk.Checked:=fini.ReadBool(Section,'desktop.bmp',Main.confDesktopBMPChk.Checked);
  Main.confStartupMP3Chk.Checked:=fini.ReadBool(Section,'startup.mp3',Main.confStartupMP3Chk.Checked);
  Main.confShutdownMP3Chk.Checked:=fini.ReadBool(Section,'shutdown.mp3',Main.confShutdownMP3Chk.Checked);
  Main.confPluginChk.Checked:=fini.ReadBool(Section,'plugin',Main.confPluginChk.Checked);
  Main.confResumeChk.OnClick:=nil;
  Main.confResumeChk.Checked:=fini.ReadBool(Section,'resume',Main.confResumeChk.Checked);
  Main.confResumeChk.OnClick:=Main.confResumeChkClick;
  Main.confBookmarkChk.OnClick:=nil;
  Main.confBookmarkChk.Checked:=fini.ReadBool(Section,'bookmark',Main.confBookmarkChk.Checked);
  Main.confBookmarkChk.OnClick:=Main.confBookmarkChkClick;
  Main.confBuiltinPluginChk.Checked:=fini.ReadBool(Section,'BuiltinPlugin',Main.confBuiltinPluginChk.Checked);

  Section:='clock';

  Main.clockUpdateChk.Checked:=fini.ReadBool(Section,'Update',Main.clockUpdateChk.Checked);
  SetListStr(Section,'Name',Main.clockNameLst);

  Section:='language';

  Main.langUpdateChk.Checked:=fini.ReadBool(Section,'Update',Main.langUpdateChk.Checked);
  SetListStrCP(Section,'CodePage',Main.langCodePageLst);
  Main.langAllItemChk.Checked:=fini.ReadBool(Section,'AllItemChk',Main.langAllItemChk.Checked);

  Section:='rom';

  Main.romMPCFChk.Checked:=fini.ReadBool(Section,'MPCF',Main.romMPCFChk.Checked);
  Main.romSCCFChk.Checked:=fini.ReadBool(Section,'SCCF',Main.romSCCFChk.Checked);
  Main.romSCSDChk.Checked:=fini.ReadBool(Section,'SCSD',Main.romSCSDChk.Checked);
  Main.romSCMSChk.Checked:=fini.ReadBool(Section,'SCMS',Main.romSCMSChk.Checked);
  Main.romM3CFChk.Checked:=fini.ReadBool(Section,'M3CF',Main.romM3CFChk.Checked);
  Main.romM3SDChk.Checked:=fini.ReadBool(Section,'M3SD',Main.romM3SDChk.Checked);
  Main.romMMCFChk.Checked:=fini.ReadBool(Section,'MMCF',Main.romMMCFChk.Checked);
  Main.romEZSDChk.Checked:=fini.ReadBool(Section,'EZSD',Main.romEZSDChk.Checked);
  Main.romEWSDChk.Checked:=fini.ReadBool(Section,'EWSD',Main.romEWSDChk.Checked);
  Main.romNMMCChk.Checked:=fini.ReadBool(Section,'NMMC',Main.romNMMCChk.Checked);
  Main.romNJSDChk.Checked:=fini.ReadBool(Section,'NJSD',Main.romNJSDChk.Checked);
  Main.romDLMSChk.Checked:=fini.ReadBool(Section,'DLMS',Main.romDLMSChk.Checked);

  Main.romNJSDChk.Checked:=False;
  
  // The NDS file for NinjaDS/SD is copied.|NinjaDS/SD用NDSファイルをコピーする。
  fini.Free;
end;

procedure SaveINI;
var
  fini:TINIFile;
  Section:string;
begin
  fini:=TINIFile.Create(ChangeFileExt(Application.ExeName,'.ini'));

  Section:='system';

  fini.WriteString(Section,'TargetDrive',GetSepSpaceStr(SelDrv.TargetDriveLst.Text));
  fini.WriteBool(Section,'ExistBackup',Main.sysExistBackupChk.Checked);
  fini.WriteBool(Section,'OpenGlobalINI',Setupped.OpenGlobalINIChk.Checked);

  Section:='configuration';

  fini.WriteBool(Section,'global.ini',Main.confGlobalINIChk.Checked);
  fini.WriteBool(Section,'desktop.bmp',Main.confDesktopBMPChk.Checked);
  fini.WriteBool(Section,'startup.mp3',Main.confStartupMP3Chk.Checked);
  fini.WriteBool(Section,'shutdown.mp3',Main.confShutdownMP3Chk.Checked);
  fini.WriteBool(Section,'plugin',Main.confPluginChk.Checked);
  fini.WriteBool(Section,'resume',Main.confResumeChk.Checked);
  fini.WriteBool(Section,'bookmark',Main.confBookmarkChk.Checked);
  fini.WriteBool(Section,'BuiltinPlugin',Main.confBuiltinPluginChk.Checked);

  Section:='clock';

  fini.WriteBool(Section,'Update',Main.clockUpdateChk.Checked);
  fini.WriteString(Section,'Name',Main.clockNameLst.Text);

  Section:='language';

  fini.WriteBool(Section,'Update',Main.langUpdateChk.Checked);
  fini.WriteString(Section,'CodePage',GetSepSpaceStr(copy(Main.langCodePageLst.Text,3,255)));
  fini.WriteBool(Section,'AllItemChk',Main.langAllItemChk.Checked);

  Section:='rom';

  fini.WriteBool(Section,'MPCF',Main.romMPCFChk.Checked);
//  fini.WriteBool(Section,'SCCF',Main.romSCCFChk.Checked);
//  fini.WriteBool(Section,'SCSD',Main.romSCSDChk.Checked);
//  fini.WriteBool(Section,'SCMS',Main.romSCMSChk.Checked);
  fini.WriteBool(Section,'M3CF',Main.romM3CFChk.Checked);
  fini.WriteBool(Section,'M3SD',Main.romM3SDChk.Checked);
  fini.WriteBool(Section,'MMCF',Main.romMMCFChk.Checked);
  fini.WriteBool(Section,'EZSD',Main.romEZSDChk.Checked);
  fini.WriteBool(Section,'EWSD',Main.romEWSDChk.Checked);
  fini.WriteBool(Section,'NMMC',Main.romNMMCChk.Checked);
  fini.WriteBool(Section,'NJSD',Main.romNJSDChk.Checked);
  fini.WriteBool(Section,'DLMS',Main.romDLMSChk.Checked);

  fini.Free;
end;

end.
