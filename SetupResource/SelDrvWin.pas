unit SelDrvWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls,_m_Tools, Buttons, _SplitML;

type
  TSelDrv = class(TForm)
    Label1: TLabel;
    TargetDriveLst: TComboBox;
    BitBtn1: TBitBtn;
    BitBtn2: TBitBtn;
    ExtractBtn: TButton;
    ToolChk: TCheckBox;
    ExtractDlg: TSaveDialog;
    CreateFPKBtn: TButton;
    FontPackageDlg: TSaveDialog;
    procedure FormCreate(Sender: TObject);
    procedure ExtractBtnClick(Sender: TObject);
    procedure ToolChkClick(Sender: TObject);
    procedure CreateFPKBtnClick(Sender: TObject);
  private
    { Private êÈåæ }
  public
    { Public êÈåæ }
    ReqExtractPath,ReqFontPackagePath:string;
    procedure InitML;
  end;

var
  SelDrv: TSelDrv;

implementation

{$R *.dfm}

procedure TSelDrv.FormCreate(Sender: TObject);
var
  drvidx:integer;
  drv:string;
  drvinfo:TDriveInfo;
  drvtype:string;
  defdrvidx:integer;
begin
  ReqExtractPath:='';
  ReqFontPackagePath:='';
  ToolChk.Checked:=False;
  ToolChkClick(nil);

  TargetDriveLst.Clear;

  TargetDriveLst.Items.Add('EXFS for FlashCart');
  defdrvidx:=1;

  for drvidx:=0 to integer('Z')-integer('A') do begin
    drv:=char(integer('A')+drvidx)+':\';
    drvinfo:=GetDriveInfomation(drv);
    if drvinfo.Enabled=True then begin
      drvtype:=GetDriveTypeName(drv);
      TargetDriveLst.Items.Add(format('%s %s [%s]',[drv,drvtype,drvinfo.VolumeLabel]));
      if drvtype='Removable' then defdrvidx:=TargetDriveLst.Items.Count-1;
    end;
  end;

  TargetDriveLst.ItemIndex:=defdrvidx;
end;

procedure TSelDrv.InitML;
begin
  SetMLLbl(Label1);
  SetMLChk(ToolChk);
  SetMLBtn(ExtractBtn);
  SetMLBtn(CreateFPKBtn);
end;

procedure TSelDrv.ExtractBtnClick(Sender: TObject);
begin
  if ExtractDlg.Execute=False then exit;

  ReqExtractPath:=ExtractFilePath(ExtractDlg.FileName);

  ModalResult:=mrOk;
end;

procedure TSelDrv.ToolChkClick(Sender: TObject);
var
  h:integer;
begin
  if ToolChk.Checked=False then begin
    h:=ToolChk.Top+ToolChk.Height+4;
    end else begin
    h:=CreateFPKBtn.Top+CreateFPKBtn.Height+4;
  end;

  SelDrv.ClientHeight:=h;
end;

procedure TSelDrv.CreateFPKBtnClick(Sender: TObject);
begin
  if FontPackageDlg.Execute=False then exit;

  ReqFontPackagePath:=ExtractFilePath(FontPackageDlg.FileName);

  ModalResult:=mrOk;
end;

end.
