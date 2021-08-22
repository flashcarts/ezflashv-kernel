unit EditItemWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, StdCtrls, Buttons,_clkmsp,_PicTools, ExtDlgs,GLDPNG;

type
  TEditItem = class(TForm)
    PositionGrp: TGroupBox;
    Label1: TLabel;
    OfsXEdt: TEdit;
    Label2: TLabel;
    OfsYEdt: TEdit;
    PosImg: TImage;
    ShowChk: TCheckBox;
    Label4: TLabel;
    FormatEdt: TEdit;
    GroupBox1: TGroupBox;
    Label11: TLabel;
    TestStrEdt: TEdit;
    PrvImg: TImage;
    Label5: TLabel;
    BitBtn1: TBitBtn;
    BitBtn2: TBitBtn;
    PosPickerImg: TImage;
    OpenBMDlg: TOpenPictureDialog;
    OpenBtn: TButton;
    ClearBtn: TButton;
    procedure FormCreate(Sender: TObject);
    procedure PosImgMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure PosImgMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure PosImgMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure OpenBtnClick(Sender: TObject);
    procedure BitBtn1Click(Sender: TObject);
    procedure OfsEdtChange(Sender: TObject);
    procedure ClearBtnClick(Sender: TObject);
  private
    { Private êÈåæ }
    procedure RefreshItemPart;
  public
    { Public êÈåæ }
    ItemPart:TItemPart;
    procedure Startup(_ItemPart:TItemPart);
  end;

var
  EditItem: TEditItem;

implementation

{$R *.dfm}

var
  IgnoreOfs:boolean;

procedure TEditItem.FormCreate(Sender: TObject);
begin
  MakeBlankImg(PosImg,pf24bit);
  PosImg.Canvas.Brush.Color:=$000000;
  MakeBlankImg(PrvImg,pf24bit);
  PrvImg.Canvas.Brush.Color:=$000000;
end;

procedure TEditItem.Startup(_ItemPart:TItemPart);
begin
  ItemPart:=_ItemPart;

  EditItem.Caption:='EditItem ['+ItemPart.Name+']';

  if ItemPart.sFormat<>'unuse' then begin
    FormatEdt.Enabled:=True;
    end else begin
    FormatEdt.Enabled:=False;
  end;

  IgnoreOfs:=False;

  RefreshItemPart;
end;

procedure TEditItem.RefreshItemPart;
begin
  ShowChk.Checked:=ItemPart.Show;
  IgnoreOfs:=True;
  OfsXEdt.Text:=inttostr(ItemPart.OfsX);
  OfsYEdt.Text:=inttostr(ItemPart.OfsY);
  IgnoreOfs:=False;
  FormatEdt.Text:=ItemPart.sFormat;
  TestStrEdt.Text:=ItemPart.TestStr;

  with PosImg do begin
    Canvas.FillRect(Rect(0,0,Width,Height));
    bitblt(Canvas.Handle,0,0,256,192,ItemBG.bm.Canvas.Handle,0,0,SRCCOPY);
    bitblt(Canvas.Handle,ItemPart.OfsX,ItemPart.OfsY,PosPickerImg.Width,PosPickerImg.Height,PosPickerImg.Canvas.Handle,0,0,SRCCOPY);
  end;

  with PrvImg do begin
    Canvas.FillRect(Rect(0,0,Width,Height));
    bitblt(Canvas.Handle,0,0,ItemPart.bm.Width,ItemPart.bm.Height,ItemPart.bm.Canvas.Handle,0,0,SRCCOPY);
  end;

end;

var
  mf:boolean=False;

procedure TEditItem.PosImgMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if Button<>mbLeft then exit;
  mf:=True;

  if x<0 then x:=0;
  if 256<=x then x:=255;
  if y<0 then y:=0;
  if 192<=y then y:=191;

  ItemPart.OfsX:=x;
  ItemPart.OfsY:=y;

  RefreshItemPart;
end;

procedure TEditItem.PosImgMouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
  if mf=False then exit;
  
  if x<0 then x:=0;
  if 256<=x then x:=255;
  if y<0 then y:=0;
  if 192<=y then y:=191;

  ItemPart.OfsX:=x;
  ItemPart.OfsY:=y;

  RefreshItemPart;
end;

procedure TEditItem.PosImgMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  mf:=False;
end;

procedure Reduce15bit(var dstbm:TBitmap;Transparent:boolean;TransColor:dword);
var
  x,y:integer;
  c:dword;
begin
  for y:=0 to dstbm.Height-1 do begin
    for x:=0 to dstbm.Width-1 do begin
      c:=dstbm.Canvas.Pixels[x,y];
      if Transparent=True then begin
        if TransColor=c then begin
          c:=$00ff00;
          end else begin
          c:=c and $f8f8f8;
        end;
        end else begin
        c:=c and $f8f8f8;
      end;
      dstbm.Canvas.Pixels[x,y]:=c;
    end;
  end;
end;

procedure TEditItem.OpenBtnClick(Sender: TObject);
var
  ext:string;
  png:TGLDPNG;
begin
  if OpenBMDlg.Execute=True then begin
    if MessageDlg('Transparent?',mtConfirmation,[mbYes,mbNo],0)=mrYes then begin
      ItemPart.TransFlag:=True;
      end else begin
      ItemPart.TransFlag:=False;
    end;

    ext:=lowercase(ExtractFileExt(OpenBMDlg.FileName));

    if ext='.bmp' then begin
      ItemPart.bm.LoadFromFile(OpenBMDlg.FileName);
      end else begin
      if ext='.png' then begin
        png:=TGLDPNG.Create;
        png.Image:=ItemPart.bm;
        png.LoadFromFile(OpenBMDlg.FileName);
        png.Free;
        end else begin
        ShowMessage('not support file format');
        exit;
      end;
    end;
    ItemPart.bm.PixelFormat:=pf24bit;

    with ItemPart.bm do begin
      Reduce15bit(ItemPart.bm,ItemPart.TransFlag,Canvas.Pixels[0,0]);
      PrvImg.Canvas.FillRect(Rect(0,0,PrvImg.Width,PrvImg.Height));
      BitBlt(PrvImg.Canvas.Handle,0,0,Width,Height,Canvas.Handle,0,0,SRCCOPY);
    end;
    PrvImg.Refresh;
  end;
end;

procedure TEditItem.BitBtn1Click(Sender: TObject);
begin
  ItemPart.Show:=ShowChk.Checked;
  ItemPart.sFormat:=FormatEdt.Text;
  ItemPart.TestStr:=TestStrEdt.Text;
end;

procedure TEditItem.OfsEdtChange(Sender: TObject);
var
  x,y:integer;
begin
  if IgnoreOfs=True then exit;
  IgnoreOfs:=True;

  x:=ItemPart.OfsX;
  y:=ItemPart.OfsY;
  ItemPart.OfsX:=strtointdef(OfsXEdt.Text,x);
  ItemPart.OfsY:=strtointdef(OfsYEdt.Text,y);

  if (ItemPart.OfsX<>x) or (ItemPart.OfsY<>y) then RefreshItemPart;

  IgnoreOfs:=False;
end;

procedure TEditItem.ClearBtnClick(Sender: TObject);
begin
  if MessageDlg('Clear?',mtWarning,[mbYes,mbNo],0)=mrYes then begin
    MakeBlankBM(ItemPart.bm,0,0,pf24bit);
    PrvImg.Canvas.FillRect(Rect(0,0,PrvImg.Width,PrvImg.Height));
    PrvImg.Refresh;
  end;
end;

end.
