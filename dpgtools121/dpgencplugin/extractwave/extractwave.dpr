program extractwave;

uses
  Forms,
  MainWin in 'MainWin.pas' {Main},
  DSSupportWin in 'DSSupportWin.pas' {DSSupport};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TMain, Main);
  Application.CreateForm(TDSSupport, DSSupport);
  Application.Run;
end.
