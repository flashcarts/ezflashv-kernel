program Setup_Packer;

uses
  Forms,
  Setup_Packer_MainWin in 'Setup_Packer_MainWin.pas' {Main};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TMain, Main);
  Application.Run;
end.
