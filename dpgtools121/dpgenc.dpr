program dpgenc;

uses
  Forms,
  MainWin in 'MainWin.pas' {Main},
  OptionWin in 'OptionWin.pas' {Option},
  DSSupportWin in 'DSSupportWin.pas' {DSSupport},
  enclogWin in 'enclogWin.pas' {enclog},
  encprvWin in 'encprvWin.pas' {encprv},
  OptionCmdLineWin in 'OptionCmdLineWin.pas' {OptionCmdLine};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TMain, Main);
  Application.CreateForm(TOption, Option);
  Application.CreateForm(TDSSupport, DSSupport);
  Application.CreateForm(Tenclog, enclog);
  Application.CreateForm(Tencprv, encprv);
  Application.CreateForm(TOptionCmdLine, OptionCmdLine);
  Application.Run;
end.
