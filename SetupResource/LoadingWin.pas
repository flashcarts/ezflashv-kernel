unit LoadingWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ComCtrls;

type
  TLoading = class(TForm)
    ProgressBar1: TProgressBar;
    Label1: TLabel;
  private
    { Private �錾 }
  public
    { Public �錾 }
  end;

var
  Loading: TLoading;

implementation

{$R *.dfm}

end.
