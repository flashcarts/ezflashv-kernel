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
    { Private éŒ¾ }
  public
    { Public éŒ¾ }
  end;

var
  Loading: TLoading;

implementation

{$R *.dfm}

end.
