unit _dosbox;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, _m_Tools, StdCtrls;

{$WARN UNIT_PLATFORM OFF}
{$WARN SYMBOL_PLATFORM OFF}

procedure setlog;
procedure wlog(msg:string);

procedure SetDefaultPriorityLevel(Level:integer);

//procedure CreateDOSBOX(StartPath:string;cmdline:string);
procedure CreateDOSBOX_UseCMD(StartPath:string;exefn,cmdline:string);
procedure CreateDOSBOX_Direct(StartPath:string;exefn,cmdline:string);
procedure CreateDOSBOX_SetRequestCancel(f:boolean);
function CreateDOSBOX2(StartPath:string;hInput,hOutput,hError:THANDLE;exefn,cmdline:string):boolean;
procedure CloseDOSBOX2(WaitTerminate:boolean);
function isTerminatedDOSBOX2:boolean;

implementation

const CRLF:string=char($0d)+char($0a);

var
  logfn:string;

procedure setlog;
begin
  logfn:=changefileext(Application.ExeName,'.log');
  if FileExists(logfn)=true then DeleteFile(logfn);
end;

procedure wlog(msg:string);
var
  wfs:TFileStream;
begin
  if FileExists(logfn)=true then begin
    wfs:=TFileStream.Create(logfn,fmOpenReadWrite);
    wfs.Position:=wfs.Size;
    end else begin
    wfs:=TFileStream.Create(logfn,fmCreate);
  end;
  wfs.WriteBuffer(msg[1],length(msg));
  wfs.Free;
end;

var
  DefaultPriorityLevel:integer;

procedure SetDefaultPriorityLevel(Level:integer);
begin
  DefaultPriorityLevel:=Level;
end;

procedure SetPriorityLevel(hProcess:Cardinal);
begin
  case DefaultPriorityLevel of
    0: SetPriorityClass(hProcess,IDLE_PRIORITY_CLASS);
    1: SetPriorityClass(hProcess,BELOW_NORMAL_PRIORITY_CLASS);
    2: SetPriorityClass(hProcess,NORMAL_PRIORITY_CLASS);
    3: SetPriorityClass(hProcess,ABOVE_NORMAL_PRIORITY_CLASS);
    4: SetPriorityClass(hProcess,HIGH_PRIORITY_CLASS);
    5: SetPriorityClass(hProcess,REALTIME_PRIORITY_CLASS);
    else SetPriorityClass(hProcess,NORMAL_PRIORITY_CLASS);
  end;
end;

var
  CreateDOSBOX_RequestCancel:boolean;

procedure CreateDOSBOX(StartPath:string;cmdline:string);
begin
end;

procedure CreateDOSBOX_UseCMD(StartPath:string;exefn,cmdline:string);
var
  CmdStrLen:integer;
  CmdStr:String;
  SI:TStartupInfo;
  PI:TProcessInformation;
begin
  SetLength(CmdStr,1024);
  CmdStrLen:=GetEnvironmentVariable('ComSpec',PChar(CmdStr),256);
  SetLength(CmdStr,CmdStrLen);
  CmdStr:=CmdStr+' /c ""'+exefn+'" '+cmdline+'"'+char(0);

  GetStartupInfo(SI);
  SI.dwFlags:=STARTF_USESHOWWINDOW;
  SI.wShowWindow:=SW_HIDE;

  wlog(StartPath+CRLF);
  wlog(exefn+' '+cmdline+CRLF);
  wlog(CRLF);

  if CreateProcess(nil,PChar(CmdStr),nil,nil,False,CREATE_DEFAULT_ERROR_MODE,nil,PChar(StartPath),SI,PI)=False then begin
    MessageDlg('�ϊ��Ɏ��s���܂����B', mtError,[mbOk], 0);
    end else begin
    SetPriorityLevel(pi.hProcess);
    // INFINITE �̏ꍇ�n���O�����悤�ɂȂ邽�߃��[�v�����ďI����҂�
    // WaitForSingleObject(PI.hProcess, INFINITE);
    while(WaitForsingleobject(PI.hProcess,100)<>WAIT_OBJECT_0) do begin
      Application.Processmessages;
//      if CreateDOSBOX_RequestCancel=True then break; // CommandPrompt���g���ƏI���ł��Ȃ�
    end;
    CloseHandle(PI.hThread);
    CloseHandle(PI.hProcess);
  end;
end;

procedure CreateDOSBOX_Direct(StartPath:string;exefn,cmdline:string);
var
  SI:TStartupInfo;
  PI:TProcessInformation;
begin
  GetStartupInfo(SI);
  SI.dwFlags:=STARTF_USESHOWWINDOW;
  SI.wShowWindow:=SW_HIDE;

  wlog(StartPath+CRLF);
  wlog(exefn+' '+cmdline+CRLF);
  wlog(CRLF);

  if CreateProcess(PChar(exefn),PChar(cmdline),nil,nil,False,CREATE_DEFAULT_ERROR_MODE,nil,PChar(StartPath),SI,PI)=False then begin
    MessageDlg('�ϊ��Ɏ��s���܂����B', mtError,[mbOk], 0);
    end else begin
    SetPriorityLevel(pi.hProcess);
    // INFINITE �̏ꍇ�n���O�����悤�ɂȂ邽�߃��[�v�����ďI����҂�
    // WaitForSingleObject(PI.hProcess, INFINITE);
    while(WaitForsingleobject(PI.hProcess,100)<>WAIT_OBJECT_0) do begin
      Application.Processmessages;
      if CreateDOSBOX_RequestCancel=True then begin
        TerminateProcess(PI.hProcess,0);
        while(WaitForsingleobject(PI.hProcess,100)<>WAIT_OBJECT_0) do Application.Processmessages;
        break;
      end;
    end;
    CloseHandle(PI.hThread);
    CloseHandle(PI.hProcess);
  end;
end;

procedure CreateDOSBOX_SetRequestCancel(f:boolean);
begin
  CreateDOSBOX_RequestCancel:=f;
end;

var
  PI:TProcessInformation;

function CreateDOSBOX2(StartPath:string;hInput,hOutput,hError:THANDLE;exefn,cmdline:string):boolean;
var
  SI:TStartupInfo;
  pcmd:PChar;
begin
  pcmd:=PChar(cmdline);

  GetStartupInfo(SI);
  SI.dwFlags:=STARTF_USESHOWWINDOW or STARTF_USESTDHANDLES;
  SI.wShowWindow:=SW_HIDE;
  SI.hStdInput:=hInput;
  SI.hStdOutput:=hOutput;
  SI.hStdError:=hError;

  if CreateProcess(PChar(exefn),pcmd,nil,nil,True,CREATE_DEFAULT_ERROR_MODE,nil,PChar(StartPath),SI,PI)=False then begin
    Result:=False;
    end else begin
    SetPriorityLevel(pi.hProcess);
    Result:=True;
  end;

end;

procedure CloseDOSBOX2(WaitTerminate:boolean);
begin
  // INFINITE �̏ꍇ�n���O�����悤�ɂȂ邽�߃��[�v�����ďI����҂�
  // WaitForSingleObject(PI.hProcess, INFINITE);

  if WaitTerminate=False then begin
    TerminateProcess(PI.hProcess,0);
  end;
  while(WaitForsingleobject(PI.hProcess,100)<>WAIT_OBJECT_0) do begin
    Application.Processmessages;
  end;

  CloseHandle(PI.hThread);
  CloseHandle(PI.hProcess);
end;

function isTerminatedDOSBOX2:boolean;
begin
  if WaitForsingleobject(PI.hProcess,0)<>WAIT_OBJECT_0 then begin
    Result:=False;
    end else begin
    Result:=True;
  end;
end;

end.
