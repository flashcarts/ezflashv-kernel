unit ColorPickWin;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ExtCtrls, StdCtrls, Buttons;

type
  TColorPick = class(TForm)
    HImg: TImage;
    HBevel: TBevel;
    PickImg: TImage;
    PickBevel: TBevel;
    Label1: TLabel;
    Label2: TLabel;
    MasterColImg: TImage;
    PickColImg: TImage;
    MasterColLbl: TLabel;
    PickColLbl: TLabel;
    OKBtn: TBitBtn;
    CancelBtn: TBitBtn;
    MasterColBevel: TBevel;
    PickColBevel: TBevel;
    CustomColorBox: TComboBox;
    NDSLimitChk: TCheckBox;
    ManualBtn: TButton;
    DragImg: TImage;
    procedure FormCreate(Sender: TObject);
    procedure HImgMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure HImgMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure PickImgMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure PickImgMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure NDSLimitChkClick(Sender: TObject);
    procedure CustomColorBoxDrawItem(Control: TWinControl; Index: Integer;
      Rect: TRect; State: TOwnerDrawState);
    procedure CustomColorBoxChange(Sender: TObject);
    procedure CancelBtnClick(Sender: TObject);
    procedure ManualBtnClick(Sender: TObject);
    procedure DragImgMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure DragImgMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure DragImgMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure OKBtnClick(Sender: TObject);
  private
    { Private 宣言 }
    mdrag:boolean;
    procedure RefreshPicker(Color:dword);
    procedure RefreshColorBox;
  public
    { Public 宣言 }
    MasterColor,PickColor:dword;
    procedure StartPick(Color:dword);
  end;

var
  ColorPick: TColorPick;

implementation

uses _PicTools;

{$R *.DFM}

procedure TColorPick.FormCreate(Sender: TObject);
  procedure SetBevel(var Img:TImage;var Bevel:TBevel);
  begin
    with Bevel do begin
      Left:=Img.Left-2;
      Top:=Img.Top-2;
      Width:=Img.Width+4;
      Height:=Img.Height+4;
    end;
  end;
begin
  MasterColor:=$ffffff;
  PickColor:=$ffffff;

  PickImg.Width:=$100;
  PickImg.Height:=$100;
  MakeBlankImg(PickImg,pf24bit);

  SetBevel(HImg,HBevel);
  SetBevel(PickImg,PickBevel);
  SetBevel(MasterColImg,MasterColBevel);
  SetBevel(PickColImg,PickColBevel);

  DragImg.Picture.Bitmap.TransparentColor:=$00ff00;
  DragImg.Picture.Bitmap.Transparent:=True;
  DragImg.Transparent:=True;
end;

procedure TColorPick.StartPick(Color:dword);
begin
  if Color<>$ffffffff then begin
    MasterColor:=Color;
    PickColor:=Color;
  end;

  RefreshPicker(PickColor);
  RefreshColorBox;

  mdrag:=False;
end;

procedure TColorPick.HImgMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  RefreshPicker(HImg.Canvas.Pixels[x,y]);
end;

procedure TColorPick.HImgMouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
  if Shift=[ssLeft] then begin
    if x<0 then x:=0;
    if HImg.Width<=x then x:=HImg.Width-1;
    if y<0 then y:=0;
    if HImg.Height<=y then y:=HImg.Height-1;
    RefreshPicker(HImg.Canvas.Pixels[x,y]);
  end;
end;

procedure TColorPick.RefreshPicker(Color:dword);
var
  DstPtr:PByteArray;
  x,y:integer;
  col0,col1,col2:word;
  xcol0,xcol1,xcol2:array[0..$ff] of byte;
  offset:word;
  tc:word;
begin
  col0:=word((Color shr 16) and $ff);
  col1:=word((Color shr 8) and $ff);
  col2:=word(Color and $ff);
  for x:=0 to $100-1 do begin
    xcol0[x]:=byte((col0*x div $FF)+($ff-x));
    xcol1[x]:=byte((col1*x div $FF)+($ff-x));
    xcol2[x]:=byte((col2*x div $FF)+($ff-x));
  end;

  with PickImg.Picture.Bitmap do begin
    for y:=0 to $100-1 do begin
      DstPtr:=ScanLine[y];
      if NDSLimitChk.Checked=False then begin
        for x:=0 to $100-1 do begin
          offset:=x*3;
          tc:=xcol0[x]*($FF-y) div $FF;
          DstPtr[offset+0]:=byte(tc);
          tc:=xcol1[x]*($FF-y) div $FF;
          DstPtr[offset+1]:=byte(tc);
          tc:=xcol2[x]*($FF-y) div $FF;
          DstPtr[offset+2]:=byte(tc);
        end;
        end else begin
        for x:=0 to $100-1 do begin
          offset:=x*3;
          tc:=xcol0[x]*($FF-y) div $FF;
          tc:=tc and $f8;
          DstPtr[offset+0]:=byte(tc);
          tc:=xcol1[x]*($FF-y) div $FF;
          tc:=tc and $f8;
          DstPtr[offset+1]:=byte(tc);
          tc:=xcol2[x]*($FF-y) div $FF;
          tc:=tc and $f8;
          DstPtr[offset+2]:=byte(tc);
        end;
      end;
    end;
  end;
  PickImg.Refresh;
end;

procedure TColorPick.RefreshColorBox;
begin
  MasterColLbl.Caption:='$'+IntToHex(MasterColor,6);
  with MasterColImg do begin
    Canvas.Brush.Color:=MasterColor;
    Canvas.FillRect(Rect(0,0,Width,Height));
  end;
  PickColLbl.Caption:='$'+IntToHex(PickColor,6);
  with PickColImg do begin
    Canvas.Brush.Color:=PickColor;
    Canvas.FillRect(Rect(0,0,Width,Height));
  end;
end;

procedure TColorPick.PickImgMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
  PickColor:=PickImg.Canvas.Pixels[x,y];
  RefreshColorBox;
end;

procedure TColorPick.PickImgMouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
begin
  if Shift=[ssLeft] then begin
    if x<0 then x:=0;
    if PickImg.Width<=x then x:=PickImg.Width-1;
    if y<0 then y:=0;
    if PickImg.Height<=y then y:=PickImg.Height-1;
    PickColor:=PickImg.Canvas.Pixels[x,y];
    RefreshColorBox;
  end;
end;

procedure TColorPick.NDSLimitChkClick(Sender: TObject);
begin
  RefreshPicker(PickImg.Canvas.Pixels[254,0]);
end;

procedure TColorPick.CustomColorBoxDrawItem(Control: TWinControl;
  Index: Integer; Rect: TRect; State: TOwnerDrawState);
var
  cm:string;
  c:dword;
	Offset:Integer;
  BoxY,TextY:integer;
begin
  with (Control as TComboBox).Canvas do begin
    cm:=(Control as TComboBox).Items[Index];
    c:=StrToInt('$'+copy(cm,5,2)+copy(cm,3,2)+copy(cm,1,2));

    FillRect(Rect);
    Offset:=2;
    BoxY:=Rect.Bottom-Rect.Top-2;
    TextY:=TextHeight('0');

    Brush.Style:=bsSolid;
    with Rect do begin
      Brush.Color:=$000000;
      FillRect(Bounds(Left+Offset+(BoxY div 4)-1,Top+(BoxY div 4)-1,(BoxY div 4)*2+2,(BoxY div 4)*2+2));
      Brush.Color:=c;
      FillRect(Bounds(Left+Offset+(BoxY div 4),Top+(BoxY div 4),(BoxY div 4)*2,(BoxY div 4)*2));
    end;
    inc(Offset,BoxY);

    Brush.Style:=bsClear;
    TextOut(Rect.Left+Offset,Rect.Top+((BoxY-TextY) div 2),copy(cm,8,255));
  end;
end;

procedure TColorPick.CustomColorBoxChange(Sender: TObject);
var
  cm:string;
begin
  cm:=CustomColorBox.Items[CustomColorBox.ItemIndex];
  PickColor:=strtoint('$'+copy(cm,5,2)+copy(cm,3,2)+copy(cm,1,2));
  RefreshPicker(PickColor);
  RefreshColorBox;
end;

procedure TColorPick.CancelBtnClick(Sender: TObject);
begin
  PickColor:=MasterColor;
end;

procedure TColorPick.ManualBtnClick(Sender: TObject);
var
  ColorStr:string;
begin
  ColorStr:=IntToHex(PickColor,6);
  if InputQuery('数値入力ウィンドウ','色情報をBBGGRR形式で入力して下さい。',ColorStr)=True then begin
    if ColorStr<>'' then begin
      if ColorStr[1]<>'$' then ColorStr:='$'+ColorStr;
      PickColor:=StrToIntDef(copy(ColorStr,1,7),PickColor);
      RefreshPicker(PickColor);
      RefreshColorBox;
    end;
  end;
end;

procedure TColorPick.DragImgMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
  if mdrag=False then begin
    mdrag:=True;
    DragImg.Transparent:=False;
  end;
end;

procedure TColorPick.DragImgMouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
var
  dc:hdc;
  p:TPoint;
  DesktopBM:TBitmap;
begin
  if mdrag=True then begin
    DesktopBM:=TBitmap.Create;
    MakeBlankBM(DesktopBM,1,1,pf24bit);
    p:=DragImg.ClientToScreen(Point(x,y));
    dc:=GetDC(GetDesktopWindow);
    BitBlt(DesktopBM.Canvas.Handle,0,0,1,1,dc,p.x,p.y,SRCCOPY);
    ReleaseDC(dc,GetDesktopWindow);
    PickColor:=DesktopBM.Canvas.Pixels[0,0];
    DesktopBM.Free;
    RefreshColorBox;
  end;
end;

procedure TColorPick.DragImgMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if mdrag=True then begin
    DragImg.Transparent:=True;
    RefreshPicker(PickColor);
    mdrag:=False;
  end;
end;

procedure TColorPick.OKBtnClick(Sender: TObject);
begin
  PickColor:=PickColor and $f8f8f8;
end;

end.
