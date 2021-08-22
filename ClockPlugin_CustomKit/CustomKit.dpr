program CustomKit;

uses
  Forms,
  MainWin in 'MainWin.pas' {Main},
  ColorPickWin in 'ColorPickWin.pas' {ColorPick},
  AboutWin in 'AboutWin.pas' {About},
  LoadBGBMWin in 'LoadBGBMWin.pas' {LoadBGBM},
  EditItemWin in 'EditItemWin.pas' {EditItem};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TMain, Main);
  Application.CreateForm(TColorPick, ColorPick);
  Application.CreateForm(TAbout, About);
  Application.CreateForm(TLoadBGBM, LoadBGBM);
  Application.CreateForm(TEditItem, EditItem);
  Application.Run;
end.
