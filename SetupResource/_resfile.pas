unit _resfile;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls, StdCtrls, ExtCtrls, Buttons, INIFiles;


type
  Tresdata=record
    data:array of byte;
    size:integer;
  end;

procedure resfile_Init;
procedure resfile_FindFile(path:string;extmask:string;var FileList:TStrings);
procedure resfile_GetData(fn:string;var resdata:Tresdata);
function resfile_FileExists(fn:string):boolean;

implementation

uses LoadingWin;

type
  TFile=record
    Filename:string;
    fnofs,dataofs,datasize:dword;
    data:array of byte;
  end;

var
  Files:array of TFile;
  FilesCount:integer;

procedure resfile_Init;
var
  resfs:TResourceStream;
  tmpch:ansichar;
  idx:integer;
begin
  resfs:=TResourceStream.Create(hInstance,'setupdata',RT_RCDATA);

  resfs.ReadBuffer(FilesCount,4);
  setlength(Files,FilesCount);

  for idx:=0 to FilesCount-1 do begin
    with Files[idx] do begin
      resfs.ReadBuffer(fnofs,4);
      resfs.ReadBuffer(dataofs,4);
      resfs.ReadBuffer(datasize,4);
    end;
  end;

  Loading.ProgressBar1.Max:=FilesCount;
  for idx:=0 to FilesCount-1 do begin
    if (idx mod 16)=0 then Loading.ProgressBar1.Position:=idx;
    with Files[idx] do begin
      resfs.Position:=fnofs;
      Filename:='';
      tmpch:=char(1);
      while (tmpch<>char(0)) do begin
        resfs.ReadBuffer(tmpch,1);
        if tmpch<>char(0) then Filename:=Filename+tmpch;
      end;

      resfs.Position:=dataofs;
      setlength(data,datasize);
      resfs.ReadBuffer(data[0],datasize);
    end;
  end;

  resfs.Free;
end;

procedure resfile_FindFile(path:string;extmask:string;var FileList:TStrings);
var
  idx:integer;
  useflag:boolean;
begin
  FileList.Clear;

  path:=ansilowercase(path);
  extmask:=ansilowercase(extmask);

  for idx:=0 to FilesCount-1 do begin
    with Files[idx] do begin
      useflag:=True;

      if path<>'' then begin
        if path<>ansilowercase(copy(Filename,1,length(path))) then useflag:=False;
      end;

      if extmask<>'' then begin
        if extmask<>ansilowercase(ExtractFileExt(Filename)) then useflag:=False;
      end;

      if useflag=True then FileList.Add(Filename);
    end;
  end;

end;

function fn2idx(fn:string):integer;
var
  idx:integer;
begin
  fn:=ansilowercase(fn);

  for idx:=0 to FilesCount-1 do begin
    if fn=ansilowercase(Files[idx].Filename) then begin
      Result:=idx;
      exit;
    end;
  end;

  Result:=-1;
end;

procedure resfile_GetData(fn:string;var resdata:Tresdata);
var
  idx:integer;
begin                                                              
  idx:=fn2idx(fn);

  if idx=-1 then begin
    ShowMessage('no file found. ['+fn+']');
    Application.Terminate;
    exit;
  end;

  with Files[idx] do begin
    resdata.size:=datasize;
    setlength(resdata.data,resdata.size);
    copymemory(resdata.data,data,resdata.size);
  end;

end;

function resfile_FileExists(fn:string):boolean;
begin
  if fn2idx(fn)=-1 then begin
    Result:=False;
    end else begin
    Result:=True;
  end;
end;

end.

