unit _dosbox;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, _m_Tools, StdCtrls;

{$WARN UNIT_PLATFORM OFF}
{$WARN SYMBOL_PLATFORM OFF}

procedure CreateDOSBOX(WaitTerm:boolean;StartPath:string;cmdline:string);

implementation

const CRLF:string=char($0d)+char($0a);

procedure CreateDOSBOX(WaitTerm:boolean;StartPath:string;cmdline:string);
var
  CmdStrLen:integer;
  CmdStr:String;
  SI:TStartupInfo;
  PI:TProcessInformation;
begin
  SetLength(CmdStr,1024);
  CmdStrLen:=GetEnvironmentVariable('ComSpec',PChar(CmdStr),256);
  SetLength(CmdStr,CmdStrLen);
  CmdStr:=CmdStr+' /c "'+cmdline+'"'+char(0);

  GetStartupInfo(SI);
  SI.dwFlags:=STARTF_USESHOWWINDOW;
  SI.wShowWindow:=SW_HIDE;

  if CreateProcess(nil,PChar(CmdStr),nil,nil,False,CREATE_DEFAULT_ERROR_MODE,nil,PChar(StartPath),SI,PI)=False then begin
//    MessageDlg('変換に失敗しました。', mtError,[mbOk], 0);
    end else begin
    if WaitTerm=True then begin
      // INFINITE の場合ハングしたようになるためループさせて終了を待つ
      // WaitForSingleObject(PI.hProcess, INFINITE);
      while(WaitForsingleobject(PI.hProcess,100)=WAIT_TIMEOUT) do Application.Processmessages;
      CloseHandle(PI.hThread);
      CloseHandle(PI.hProcess);
    end;
  end;
end;

end.
