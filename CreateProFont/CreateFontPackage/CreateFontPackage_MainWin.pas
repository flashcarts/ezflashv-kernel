unit CreateFontPackage_MainWin;

interface

uses
  Windows, SysUtils, Classes, Controls, Forms, StdCtrls,
  Dialogs;

type
  TForm1 = class(TForm)
    fonOpenDlg: TOpenDialog;
    fpkSaveDlg: TSaveDialog;
    procedure FormCreate(Sender: TObject);
  private
    { Private 宣言 }
  public
    { Public 宣言 }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

const CRLF:string=char($0d)+char($0a);

procedure CreateFontPackage(fpkfn,ankfn,l2ufn,fonfn:string);
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
    rfs:TFileStream;
    buf:array of byte;
    size:integer;
  begin
    rfs:=TFileStream.Create(fn,fmOpenRead);
    size:=rfs.Size;
    setlength(buf,size);
    rfs.ReadBuffer(buf[0],size);
    rfs.Free;

    adrtbl[idx*2+0]:=wfs.Position;
    adrtbl[idx*2+1]:=size;
    wfs.WriteBuffer(buf[0],size);
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

procedure TForm1.FormCreate(Sender: TObject);
var
  FontPath:string;
  Fontfn,FontBasefn:string;
  fpkfn:string;
  errstr:string;
  msg:string;
begin
  Fontfn:=ParamStr(1);

  if FileExists(Fontfn)=False then begin
    FontPath:=ExtractFilePath(Application.ExeName)+'SetupData';
    if DirectoryExists(FontPath)=True then fonOpenDlg.InitialDir:=FontPath;
    if fonOpenDlg.Execute=False then begin
      Application.Terminate;
      exit;
    end;
    Fontfn:=fonOpenDlg.FileName;
  end;

  FontBasefn:=ChangeFileExt(Fontfn,'');

  errstr:='';

  if FileExists(FontBasefn+'.ank')=False then errstr:=errstr+'File not found. '+FontBasefn+'.ank'+CRLF;
  if FileExists(FontBasefn+'.l2u')=False then errstr:=errstr+'File not found. '+FontBasefn+'.l2u'+CRLF;
  if FileExists(FontBasefn+'.fon')=False then errstr:=errstr+'File not found. '+FontBasefn+'.fon'+CRLF;

  if errstr<>'' then begin
    ShowMessage(errstr);
    Application.Terminate;
    exit;
  end;

  fpkfn:=FontBasefn;
  fpkfn:=ExtractFilePath(fpkfn);
  fpkfn:=copy(fpkfn,1,length(fpkfn)-1);
  fpkfn:=ExtractFilename(fpkfn);

  fpkSaveDlg.FileName:=fpkfn+'-'+ExtractFilename(FontBasefn)+'.fpk';

  if fpkSaveDlg.Execute=False then begin
    Application.Terminate;
    exit;
  end;
  fpkfn:=fpkSaveDlg.FileName;

  CreateFontPackage(fpkfn,FontBasefn+'.ank',FontBasefn+'.l2u',FontBasefn+'.fon');

  msg:='';
  msg:=msg+fpkfn+CRLF;
  msg:=msg+''+CRLF;
  msg:=msg+'フォントパッケージファイルを作成しました。'+CRLF;
  msg:=msg+'CF/SD/EXFSに入れてMoonShellから選択してください。'+CRLF;
  msg:=msg+''+CRLF;
  msg:=msg+'The font package file was created.'+CRLF;
  msg:=msg+'Please put in CF/SD/EXFS and select it from MoonShell.'+CRLF;

  ShowMessage(msg);
  
  Application.Terminate;
end;

end.
