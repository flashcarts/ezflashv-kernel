unit enclogWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  Tenclog = class(TForm)
    loglst: TMemo;
    procedure FormCreate(Sender: TObject);
  private
    { Private �錾 }
  public
    { Public �錾 }
  end;

var
  enclog: Tenclog;

implementation

{$R *.dfm}

procedure Tenclog.FormCreate(Sender: TObject);
begin
  loglst.Clear;
end;

end.
