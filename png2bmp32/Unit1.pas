unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, StdCtrls,GLDPNG, _PicTools;

type
  TMain = class(TForm)
    cimg: TImage;
    aimg: TImage;
    Timer1: TTimer;
    GLDPNGMemo: TMemo;
    picOpenDlg: TOpenDialog;
    procedure FormCreate(Sender: TObject);
  private
    { Private êÈåæ }
  public
    { Public êÈåæ }
  end;

var
  Main: TMain;

implementation

{$R *.dfm}

procedure LoadPNG(fn:string;var bm,abm:TBitmap);
var
  png:TGLDPNG;
begin
  png:=TGLDPNG.Create;

  png.Image:=bm;
  png.AlphaBitmap:=abm;
  png.LoadFromFile(fn);

  png.Free;
end;

procedure SaveBMP32(fn:string;var cbm,abm:TBitmap;alphaflag:boolean);
var
  x,y,w,h:integer;
  bm:TBitmap;
  pabm,pbm:pbytearray;
begin
  w:=cbm.Width;
  h:=cbm.Height;

  bm:=TBitmap.Create;
  MakeBlankBM(bm,w,h,pf32bit);

  BitBlt(bm.Canvas.Handle,0,0,w,h,cbm.Canvas.Handle,0,0,SRCCOPY);

  if alphaflag=False then begin
    for y:=0 to h-1 do begin
      pabm:=abm.ScanLine[y];
      pbm:=bm.ScanLine[y];
      for x:=0 to w-1 do begin
        pbm[x*4+3]:=$ff;
      end;
    end;
    end else begin
    for y:=0 to h-1 do begin
      pabm:=abm.ScanLine[y];
      pbm:=bm.ScanLine[y];
      for x:=0 to w-1 do begin
        pbm[x*4+3]:=$ff-pabm[x];
      end;
    end;
  end;

  bm.SaveToFile(fn);
end;

procedure png2tgf(pngfn:string);
var
  pngcbm,pngabm:TBitmap;
  alpha:boolean;
  sw,sh:integer;
begin
  pngcbm:=TBitmap.Create;
  pngabm:=TBitmap.Create;

  LoadPNG(pngfn,pngcbm,pngabm);

  sw:=pngcbm.Width;
  sh:=pngcbm.Height;

  alpha:=True;
  if (assigned(pngabm)=False) or (pngabm.Width<>sw) or (pngabm.Height<>sh) then begin
    alpha:=False;
{
    ShowMessage('not found AlphaChannel.');
    Application.Terminate;
    exit;
}
  end;

  Main.ClientWidth:=sw;
  Main.ClientHeight:=sh*2;

  with Main.cimg do begin
    Left:=0;
    Top:=0;
    Width:=sw;
    Height:=sh;
    bitblt(Canvas.Handle,0,0,sw,sh,pngcbm.Canvas.Handle,0,0,SRCCOPY);
  end;

  if alpha=True then begin
    with Main.aimg do begin
      Left:=0;
      Top:=sh;
      Width:=sw;
      Height:=sh;
      bitblt(Canvas.Handle,0,0,sw,sh,pngabm.Canvas.Handle,0,0,SRCCOPY);
    end;
  end;

  Main.cimg.Refresh;
  Main.aimg.Refresh;

  SaveBMP32(changefileext(pngfn,'.bmp'),pngcbm,pngabm,alpha);

  pngcbm.Free;
  pngabm.Free;
end;

procedure TMain.FormCreate(Sender: TObject);
var
  picfn:string;
begin
  Show;

  if 1<=ParamCount then begin
    picfn:=ParamStr(1);
    end else begin
    if picOpenDlg.Execute=False then begin
      Application.Terminate;
      exit;
    end;
    picfn:=picOpenDlg.FileName;
  end;
  caption:=picfn;

  GLDPNGMemo.Visible:=False;
  png2tgf(picfn);

  Application.Terminate;
end;

end.
