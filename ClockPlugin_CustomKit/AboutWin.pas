unit AboutWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Buttons;

type
  TAbout = class(TForm)
    VerLbl: TLabel;
    BitBtn1: TBitBtn;
    GLDPNGMemo: TMemo;
    Label1: TLabel;
    procedure FormCreate(Sender: TObject);
  private
    { Private �錾 }
  public
    { Public �錾 }
  end;

var
  About: TAbout;

implementation

uses MainWin;

{$R *.dfm}

procedure TAbout.FormCreate(Sender: TObject);
begin
  VerLbl.Caption:=Main.Caption;
end;

end.
