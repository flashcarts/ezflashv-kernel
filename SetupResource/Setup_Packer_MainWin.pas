unit Setup_Packer_MainWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TMain = class(TForm)
    FilenameLst: TListBox;
    procedure FormCreate(Sender: TObject);
  private
    { Private 宣言 }
  public
    { Public 宣言 }
  end;

var
  Main: TMain;

implementation

{$R *.dfm}

const BasePath:string='D:\MyDocuments\NDS\MoonShell\SetupData\';

procedure GetFiles(path:string);
var
  res:integer;
  SearchRec:TSearchRec;
begin
  res:=FindFirst(BasePath+path+'*.*', (faReadOnly or faHidden or faSysFile or faArchive), SearchRec);
  if res=0 then begin
    repeat
      Main.FilenameLst.Items.Add(Path+SearchRec.Name);
      res:=FindNext(SearchRec);
    until (res<>0);
  end;
  FindClose(SearchRec);

  res:=FindFirst(BasePath+path+'*.*', (faDirectory or faReadOnly or faHidden or faSysFile or faArchive), SearchRec);
  if res=0 then begin
    repeat
      if (SearchRec.Attr and faDirectory)<>0 then begin
        if (SearchRec.Name<>'..') and (SearchRec.Name<>'.') then begin
          GetFiles(path+SearchRec.Name+'\');
        end;
      end;
      res:=FindNext(SearchRec);
    until (res<>0);
  end;
  FindClose(SearchRec);

end;

type
  TFile=record
    Filename:string;
    fnofs,dataofs,datasize:dword;
    data:array of byte;
  end;

var
  Files:array of TFile;
  FilesCount:integer;

procedure TMain.FormCreate(Sender: TObject);
var
  tmp:dword;
  fs:TFileStream;
  buf:array of byte;
  bufsize:integer;
  idx:integer;
begin
  if FileExists('Setup_resource.res')=True then begin
    ShowMessage('リソース構築の前に初期化が必要です。');
    Application.Terminate;
    exit;
  end;

  tmp:=0;

  GetFiles('');

  FilesCount:=FilenameLst.Items.Count;
  setlength(Files,FilesCount);

  for idx:=0 to FilesCount-1 do begin
    with Files[idx] do begin
      Filename:=FilenameLst.Items[idx];
      fnofs:=0;
      dataofs:=0;

      fs:=TFileStream.Create(BasePath+Filename,fmOpenRead);
      datasize:=fs.Size;
      setlength(data,datasize);
      fs.ReadBuffer(data[0],datasize);
      fs.Free;
    end;
  end;

  fs:=TFileStream.Create('Setup_resource.dat',fmCreate);

  fs.Position:=0;
  fs.WriteBuffer(FilesCount,4);
  
  for idx:=0 to FilesCount-1 do begin
    with Files[idx] do begin
      fs.WriteBuffer(fnofs,4);
      fs.WriteBuffer(dataofs,4);
      fs.WriteBuffer(datasize,4);
    end;
  end;

  for idx:=0 to FilesCount-1 do begin
    with Files[idx] do begin
      fnofs:=fs.Position;
      fs.WriteBuffer(Filename[1],length(Filename));
      fs.WriteBuffer(tmp,1);

      dataofs:=fs.Position;
      fs.WriteBuffer(data[0],datasize);
    end;
  end;

  fs.Position:=0;
  fs.WriteBuffer(FilesCount,4);

  for idx:=0 to FilesCount-1 do begin
    with Files[idx] do begin
      fs.WriteBuffer(fnofs,4);
      fs.WriteBuffer(dataofs,4);
      fs.WriteBuffer(datasize,4);
    end;
  end;

  fs.Free;

  Application.Terminate;
end;

end.
