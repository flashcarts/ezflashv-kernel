unit LoadBGBMWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, Buttons, ComCtrls, StdCtrls, ExtCtrls,_PicTools, GLDPNG;

type
  TLoadBGBM = class(TForm)
    PrvImg: TImage;
    GroupBox1: TGroupBox;
    ZoomWidthBtn: TButton;
    ZoomHeightBtn: TButton;
    Zoom100Btn: TButton;
    ZoomSizeLbl: TLabel;
    ZoomManBtn: TButton;
    GroupBox2: TGroupBox;
    PosLbl: TLabel;
    Zoom50Btn: TButton;
    BitBtn1: TBitBtn;
    BitBtn3: TBitBtn;
    PosInitBtn: TButton;
    TileModeChk: TCheckBox;
    ZoomBar: TTrackBar;
    DiffuseChk: TCheckBox;
    procedure FormCreate(Sender: TObject);
    procedure ZoomWidthBtnClick(Sender: TObject);
    procedure ZoomHeightBtnClick(Sender: TObject);
    procedure Zoom100BtnClick(Sender: TObject);
    procedure Zoom50BtnClick(Sender: TObject);
    procedure PosInitBtnClick(Sender: TObject);
    procedure PrvImgMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure PrvImgMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure PrvImgMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure ZoomManBtnClick(Sender: TObject);
    procedure BitBtn1Click(Sender: TObject);
    procedure TileModeChkClick(Sender: TObject);
    procedure ZoomBarChange(Sender: TObject);
    procedure DiffuseChkClick(Sender: TObject);
  private
    { Private êÈåæ }
    ZoomBarApplyFlag:boolean;
    ZoomSize:double;
    PosX,PosY:integer;
    procedure RefreshParam;
    procedure RefreshBM;
  public
    { Public êÈåæ }
    srcbm,dstbm:TBitmap;
    function Startup(Filename:string):boolean;
  end;

var
  LoadBGBM: TLoadBGBM;

implementation

{$R *.dfm}

procedure TLoadBGBM.FormCreate(Sender: TObject);
begin
  ZoomBarApplyFlag:=False;

  ZoomSize:=1;
  PosX:=0;
  PosY:=0;

  ZoomBar.Max:=2*100;
  ZoomBar.Position:=ZoomBar.Max;
  ZoomBar.Min:=trunc(0.5*100);
  ZoomBar.Position:=trunc(ZoomSize*100);

  srcbm:=TBitmap.Create;
  MakeBlankBM(srcbm,1,1,pf24bit);
  dstbm:=TBitmap.Create;
  MakeBlankBM(dstbm,PrvImg.Width,PrvImg.Height,pf24bit);

  MakeBlankImg(PrvImg,pf24bit);

  ZoomBarApplyFlag:=True;
end;

function TLoadBGBM.Startup(Filename:string):boolean;
var
  ext:string;
  png:TGLDPNG;
begin
  if FileExists(Filename)=False then begin
    ShowMessage('Can not found ['+Filename+']');
    Result:=False;
    exit;
  end;

  ext:=lowercase(ExtractFileExt(Filename));

  if ext='.bmp' then begin
    srcbm.LoadFromFile(Filename);
    end else begin
    if ext='.png' then begin
      png:=TGLDPNG.Create;
      png.Image:=srcbm;
      png.LoadFromFile(Filename);
      png.Free;
      end else begin
      ShowMessage('not support file format');
      Result:=False;
      exit;
    end;
  end;

  srcbm.PixelFormat:=pf24bit;
  Result:=True;

  RefreshParam;
end;

procedure TLoadBGBM.RefreshParam;
begin
  ZoomBarApplyFlag:=False;
  ZoomBar.Position:=trunc(ZoomSize*100);
  ZoomBarApplyFlag:=True;

  ZoomSizeLbl.Caption:=format('Zoom %7.3f%%',[ZoomSize*100]);
  PosLbl.Caption:=format('Position=(%d,%d)',[PosX,PosY]);
  RefreshBM;
end;

procedure Reduce15bit(var dstbm:TBitmap;Diffuse:boolean);
var
  x,y:integer;
  pb:PByteArray;
  g0,g1,g2:dword;
  procedure diff(var data:byte;var g:dword);
  var
    c:dword;
  begin
    c:=(dword(data) and $ff)+g;
    g:=c and $07;
    if 256<=c then c:=255;
    data:=byte(c and $f8);
  end;
begin
  if Diffuse=False then begin
    for y:=0 to dstbm.Height-1 do begin
      pb:=dstbm.ScanLine[y];
      for x:=0 to (dstbm.Width*3)-1 do begin
        pb[x]:=pb[x] and $f8;
      end;
    end;

    end else begin
    g0:=0; g1:=0; g2:=0;
    for y:=0 to dstbm.Height-1 do begin
      pb:=dstbm.ScanLine[y];
      for x:=0 to dstbm.Width-1 do begin
        diff(pb[x*3+0],g0);
        diff(pb[x*3+1],g1);
        diff(pb[x*3+2],g2);
      end;
    end;
  end;
end;

procedure TLoadBGBM.RefreshBM;
var
  sx,sy:integer;
  dx,dy:integer;
  px,py:integer;
  tsx,tsy,tex,tey:integer;
  tx,ty:integer;
begin
  sx:=srcbm.Width;
  sy:=srcbm.Height;
  dx:=trunc(sx*ZoomSize);
  dy:=trunc(sy*ZoomSize);
  px:=PosX;
  py:=PosY;

  if dx<=0 then dx:=1;
  if dy<=0 then dy:=1;

  if TileModeChk.Checked=False then begin
    tsx:=0;
    tex:=1;
    tsy:=0;
    tey:=1;
    end else begin
    if px<=0 then begin
      tsx:=0;
      end else begin
      tsx:=-(trunc(px/dx)+1);
    end;
    if py<=0 then begin
      tsy:=0;
      end else begin
      tsy:=-(trunc(py/dy)+1);
    end;
    tex:=trunc((-px+256)/dx)+1;
    tey:=trunc((-py+192)/dy)+1;
  end;

  dstbm.Canvas.Brush.Color:=$000000;
  dstbm.Canvas.FillRect(Rect(0,0,256,192));

  SetStretchBltMode(srcbm.Canvas.Handle,HALFTONE);
  SetStretchBltMode(dstbm.Canvas.Handle,HALFTONE);
  for ty:=tsy to tey-1 do begin
    for tx:=tsx to tex-1 do begin
      StretchBlt(dstbm.Canvas.Handle,px+(dx*tx),py+(dy*ty),dx,dy,srcbm.Canvas.Handle,0,0,sx,sy,SRCCOPY);
    end;
  end;

  Reduce15bit(dstbm,DiffuseChk.Checked);

  BitBlt(PrvImg.Canvas.Handle,0,0,256,192,dstbm.Canvas.Handle,0,0,SRCCOPY);
  PrvImg.Refresh;
end;

procedure TLoadBGBM.ZoomWidthBtnClick(Sender: TObject);
begin
  if srcbm.Width=0 then exit;
  ZoomSize:=256/srcbm.Width;
  RefreshParam;
end;

procedure TLoadBGBM.ZoomHeightBtnClick(Sender: TObject);
begin
  if srcbm.Height=0 then exit;
  ZoomSize:=192/srcbm.Height;
  RefreshParam;
end;

procedure TLoadBGBM.Zoom100BtnClick(Sender: TObject);
begin
  ZoomSize:=1;
  RefreshParam;
end;

procedure TLoadBGBM.Zoom50BtnClick(Sender: TObject);
begin
  ZoomSize:=0.5;
  RefreshParam;
end;

procedure TLoadBGBM.ZoomManBtnClick(Sender: TObject);
var
  str:string;
begin
  str:=format('%3.10f',[ZoomSize*100]);

  str:=InputBox('ägëÂó¶Çì¸óÕÇµÇƒÇ≠ÇæÇ≥Ç¢ÅB','',str);

  ZoomSize:=strtofloatdef(str,ZoomSize*100)/100;
  RefreshParam;
end;

procedure TLoadBGBM.PosInitBtnClick(Sender: TObject);
begin
  PosX:=0;
  PosY:=0;
  RefreshParam;
end;

var
  mf:boolean;
  mx,my:integer;

procedure TLoadBGBM.PrvImgMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if Button=mbLeft then mf:=True;

  mx:=X;
  my:=Y;
end;

procedure TLoadBGBM.PrvImgMouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
  if mf=True then begin
    PosX:=PosX+(X-mx);
    PosY:=PosY+(Y-my);
    RefreshParam;
  end;

  mx:=X;
  my:=Y;
end;

procedure TLoadBGBM.PrvImgMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  mf:=False;
end;

procedure TLoadBGBM.BitBtn1Click(Sender: TObject);
begin
  // dummy
end;

procedure TLoadBGBM.TileModeChkClick(Sender: TObject);
begin
  RefreshBM;
end;

procedure TLoadBGBM.ZoomBarChange(Sender: TObject);
begin
  if ZoomBarApplyFlag=False then exit;

  ZoomSize:=ZoomBar.Position/100;
  RefreshParam;
end;

procedure TLoadBGBM.DiffuseChkClick(Sender: TObject);
begin
  RefreshParam;
end;

end.
