unit MainWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls,_clkmsp, Menus, ExtDlgs,_PicTools;

type
  TMain = class(TForm)
    ItemLst: TListBox;
    MainMenu1: TMainMenu;
    MMF: TMenuItem;
    MME: TMenuItem;
    MMA: TMenuItem;
    MME_BGFill: TMenuItem;
    MME_Item0: TMenuItem;
    MME_Item1: TMenuItem;
    MME_Item2: TMenuItem;
    MME_Item3: TMenuItem;
    MME_Item4: TMenuItem;
    GroupBox1: TGroupBox;
    PrvImg: TImage;
    OpenMSPO1: TMenuItem;
    Saveas1: TMenuItem;
    N1: TMenuItem;
    SavePreviewP1: TMenuItem;
    N2: TMenuItem;
    ExitX1: TMenuItem;
    OpenMSPDlg: TOpenDialog;
    SaveMSPDlg: TSaveDialog;
    SavePreviewDlg: TSavePictureDialog;
    MME_BGLoadClick: TMenuItem;
    N3: TMenuItem;
    OpenBGBMDlg: TOpenPictureDialog;
    procedure Button1Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure MME_ItemClick(Sender: TObject);
    procedure MMAClick(Sender: TObject);
    procedure OpenMSPO1Click(Sender: TObject);
    procedure SavePreviewP1Click(Sender: TObject);
    procedure MME_BGLoadClickClick(Sender: TObject);
    procedure MME_BGFillClick(Sender: TObject);
    procedure Saveas1Click(Sender: TObject);
    procedure ItemLstDblClick(Sender: TObject);
    procedure ExitX1Click(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
  private
    { Private êÈåæ }
    procedure RefreshItemLst;
    procedure RedrawPreview;
  public
    { Public êÈåæ }
  end;

var
  Main: TMain;

implementation

uses ColorPickWin, LoadBGBMWin, EditItemWin, AboutWin;

{$R *.dfm}

const CRLF:string=char($0d)+char($0a);

var
  StartPath:string;

procedure TMain.Button1Click(Sender: TObject);
begin
  ColorPick.StartPick($ffffff);
  ColorPick.ShowModal;
end;

procedure TMain.FormCreate(Sender: TObject);
var
  idx:integer;
begin
  Main.Caption:='Clock plug-in custom kit ver 0.2';
  
  StartPath:=ExtractFilePath(Application.ExeName);

  MakeBlankImg(PrvImg,pf24bit);
  
  ItemLst.Clear;
  for idx:=0 to ItemPartCount-1 do begin
    ItemLst.Items.Add('');
  end;

  clkmsp_Init;
  clkmsp_Load(StartPath+'default\default.msp');

  RefreshItemLst;
  RedrawPreview;
end;

procedure TMain.RefreshItemLst;
var
  idx:integer;
  str:string;
begin
  for idx:=0 to ItemPartCount-1 do begin
    with ItemParts[idx] do begin
      str:='';
      str:=str+format('%-12s',[Name]);
      str:=str+' '+format('''%s''(%2d)',[char(FontStart),FontCount]);
      if Show=True then begin
        str:=str+' Show';
        end else begin
        str:=str+' Hide';
      end;
      str:=str+' '+format('(%3d,%3d)',[OfsX,OfsY]);
      if sFormat<>'unuse' then str:=str+' "'+sFormat+'"';
    end;
    ItemLst.Items[idx]:=str;
  end;

end;

procedure TMain.RedrawPreview;
var
  idx:integer;
  procedure DrawItemPart(ItemPart:TItemPart);
  var
    px,py:integer;
    fw,fh:integer;
    idx:integer;
    fc:integer;
    procedure BitBltTrans(dst:TBitmap;x,y,w,h:integer;src:TBitmap;oy:integer);
    var
      dx,dy:integer;
      c:dword;
    begin
      for dy:=0 to h-1 do begin
        if (0<=(y+dy)) and ((y+dy)<192) then begin
          for dx:=0 to w-1 do begin
            if (0<=(x+dx)) and ((x+dx)<256) then begin
              c:=src.Canvas.Pixels[dx,oy+dy];
              if c<>$00ff00 then dst.Canvas.Pixels[dx+x,dy+y]:=c;
            end;
          end;
        end;
      end;
    end;
  begin
    if ItemPart.Show=False then exit;

    px:=ItemPart.OfsX;
    py:=ItemPart.OfsY;

    fw:=ItemPart.bm.Width;
    fh:=ItemPart.bm.Height div ItemPart.FontCount;

    for idx:=0 to length(ItemPart.TestStr)-1 do begin
      fc:=integer(ItemPart.TestStr[1+idx])-ItemPart.FontStart;
      if (0<=fc) and (fc<ItemPart.FontCount) then begin
        BitBltTrans(PrvImg.Picture.Bitmap,px,py,fw,fh,ItemPart.bm,fc*fh);
      end;
      inc(px,fw);
    end;
  end;
begin
  BitBlt(PrvImg.Canvas.Handle,0,0,256,192,ItemBG.bm.Canvas.Handle,0,0,SRCCOPY);

  for idx:=0 to ItemPartCount-1 do begin
    DrawItemPart(ItemParts[idx]);
  end;

  PrvImg.Refresh;
end;

procedure TMain.MME_ItemClick(Sender: TObject);
var
  idx:integer;
begin
  idx:=TMenuItem(Sender).GroupIndex;

  EditItem.Startup(ItemParts[idx]);
  if EditItem.ShowModal=mrOk then begin
    ItemParts[idx]:=EditItem.ItemPart;
    RefreshItemLst;
    RedrawPreview;
  end;
end;

procedure TMain.MMAClick(Sender: TObject);
begin
  About.ShowModal;
end;

procedure TMain.OpenMSPO1Click(Sender: TObject);
begin
  if OpenMSPDlg.Execute=True then begin
    SaveMSPDlg.FileName:=OpenMSPDlg.FileName;
    SavePreviewDlg.FileName:=ChangeFileExt(OpenMSPDlg.FileName,'.bmp');
    clkmsp_Load(OpenMSPDlg.FileName);
    RefreshItemLst;
    RedrawPreview;
  end;
end;

procedure TMain.SavePreviewP1Click(Sender: TObject);
begin
  if SavePreviewDlg.Execute=True then begin
    RedrawPreview;
    PrvImg.Picture.Bitmap.SaveToFile(SavePreviewDlg.Filename);
    ShowMessage('Saved! ['+SavePreviewDlg.Filename+']');
  end;
end;

procedure TMain.MME_BGLoadClickClick(Sender: TObject);
begin
  if OpenBGBMDlg.Execute=True then begin
    if LoadBGBM.Startup(OpenBGBMDlg.FileName)=True then begin
      if LoadBGBM.ShowModal=mrOk then begin
        BitBlt(ItemBG.bm.Canvas.Handle,0,0,256,192,LoadBGBM.dstbm.Canvas.Handle,0,0,SRCCOPY);
        RedrawPreview;
      end;
    end;
  end;
end;

procedure TMain.MME_BGFillClick(Sender: TObject);
begin
  ColorPick.StartPick($ffffffff);
  if ColorPick.ShowModal=mrOk then begin
    ItemBG.bm.Canvas.Brush.Color:=ColorPick.PickColor;
    ItemBG.bm.Canvas.FillRect(Rect(0,0,256,192));
    RedrawPreview;
  end;
end;

procedure TMain.Saveas1Click(Sender: TObject);
begin
  if SaveMSPDlg.Execute=True then begin
    OpenMSPDlg.FileName:=SaveMSPDlg.FileName;
    SavePreviewDlg.FileName:=ChangeFileExt(SaveMSPDlg.FileName,'.bmp');
    clkmsp_Save(SaveMSPDlg.FileName);
    ShowMessage('Saved! ['+SaveMSPDlg.FileName+']');
  end;
end;

procedure TMain.ItemLstDblClick(Sender: TObject);
var
  idx:integer;
begin
  idx:=TListBox(Sender).ItemIndex;

  EditItem.Startup(ItemParts[idx]);
  if EditItem.ShowModal=mrOk then begin
    ItemParts[idx]:=EditItem.ItemPart;
    RefreshItemLst;
    RedrawPreview;
  end;
end;

procedure TMain.ExitX1Click(Sender: TObject);
begin
  Main.Close;
end;

procedure TMain.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
  if MessageDlg('Exit?',mtWarning,[mbYes,mbNo],0)=mrNo then CanClose:=False;
end;

end.
