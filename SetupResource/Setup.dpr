program Setup;

{$R 'Setup_resource.res' 'Setup_resource.rc'}

uses
  Forms,
  MainWin in 'MainWin.pas' {Main},
  SelDrvWin in 'SelDrvWin.pas' {SelDrv},
  LoadingWin in 'LoadingWin.pas' {Loading},
  ClkPrvWin in 'ClkPrvWin.pas' {ClkPrv},
  SelMLWin in 'SelMLWin.pas' {SelML},
  SetuppedWin in 'SetuppedWin.pas' {Setupped},
  ROMInfoWin in 'ROMInfoWin.pas' {ROMInfo};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TMain, Main);
  Application.CreateForm(TSelDrv, SelDrv);
  Application.CreateForm(TLoading, Loading);
  Application.CreateForm(TClkPrv, ClkPrv);
  Application.CreateForm(TSelML, SelML);
  Application.CreateForm(TSetupped, Setupped);
  Application.CreateForm(TROMInfo, ROMInfo);
  Application.Run;
end.
