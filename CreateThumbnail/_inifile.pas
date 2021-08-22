unit _inifile;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls, StdCtrls, ExtCtrls, Buttons, INIFiles;

procedure CreateINI;
procedure LoadINI;
procedure SaveINI;

var
  INI_ShowCompletionDialog:boolean;
  INI_ColorCount:integer;
  INI_ScreenRatio:integer;

implementation

uses MainWin;

const CRLF:string=char($0d)+char($0a);

function GetINIFilename:string;
begin
  Result:=ChangeFileExt(Application.ExeName,'.ini');
end;

procedure CreateINI;
var
  fn:string;
  wfs:TFileStream;
begin
  fn:=GetINIFilename;
  if FileExists(fn)=True then exit;

  wfs:=TFileStream.Create(fn,fmCreate);
  wfs.WriteBuffer(Main.BaseINIMemo.Text[1],length(Main.BaseINIMemo.Text));
  wfs.Free;

  fn:=ExtractFilename(fn);
  ShowMessage('ïWèÄê›íËÇÃ'+fn+'ÇçÏê¨ÇµÇ‹ÇµÇΩÅB'+CRLF+CRLF+''+fn+' of a standard setting was made.');
end;

procedure LoadINI;
var
  fini:TINIFile;
  Section:string;
begin
  fini:=TINIFile.Create(GetINIFilename);

  Section:='System';

  INI_ShowCompletionDialog:=fini.ReadBool(Section,'ShowCompletionDialog',True);

  Section:='Format';

  INI_ColorCount:=fini.ReadInteger(Section,'ColorCount',256);
  INI_ScreenRatio:=fini.ReadInteger(Section,'ScreenRatio',100);

  fini.Free;
end;

procedure SaveINI;
begin
end;

end.
