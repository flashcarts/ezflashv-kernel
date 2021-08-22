program gbfsHelper;

{$APPTYPE CONSOLE}

uses
  SysUtils;

var
  fns:array of string;
  fnsCount:integer;

procedure GetFileList;
var
  res:integer;
  SearchRec: TSearchRec;
  fname:string;
begin
  fnsCount:=0;

  res:=FindFirst('files_gbfs\*.*', (FaAnyFile), SearchRec);
  if res=0 then begin
    repeat
      fname:=SearchRec.Name;
      if (SearchRec.Attr and faDirectory)=0 then begin
        setlength(fns,fnsCount+1);
        fns[fnsCount]:=fname;
        inc(fnsCount);
      end;
      res:=FindNext(SearchRec);
    until (res<>0);
  end;
  FindClose(SearchRec);
end;

var
  tf:TextFile;
  cnt:integer;
  linestr:string;
begin
  linestr:='wintools\gbfs.exe gbfsdata.bin';

  GetFileList;
  for cnt:=0 to fnsCount-1 do begin
    linestr:=linestr+' "files_gbfs\'+fns[cnt]+'"';
  end;

  AssignFile(tf,'gbfsHelper.bat');
  rewrite(tf);
  writeln(tf,linestr);
  closefile(tf);


end.
