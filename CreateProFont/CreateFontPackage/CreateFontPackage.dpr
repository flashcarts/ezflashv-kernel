program CreateFontPackage;

uses
  Forms,
  CreateFontPackage_MainWin in 'CreateFontPackage_MainWin.pas' {Form1};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
