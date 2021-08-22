unit MainWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls, ExtCtrls;

type
  TMain = class(TForm)
    prgbar: TProgressBar;
    StartupTimer: TTimer;
    procedure FormCreate(Sender: TObject);
    procedure StartupTimerTimer(Sender: TObject);
  private
    { Private êÈåæ }
    AltMode:boolean;
  public
    { Public êÈåæ }
  end;

var
  Main: TMain;

implementation

uses DSSupportWin;

{$R *.dfm}

const CRLF:string=char($0d)+char($0a);

procedure ShowCmdline;
var
  msg:string;
begin
  msg:=Main.Caption+CRLF;
  msg:=msg+CRLF;
  msg:=msg+ExtractFilename(Application.ExeName)+' input.avi output.wav';

  showmessage(msg);
end;

var
  avifn,wavfn:string;

procedure TMain.FormCreate(Sender: TObject);
begin
  Application.Title:='DirectShow AudioStream Demultiplexer ver 0.2 by Moonlight';
  Main.Caption:=Application.Title;

  if ParamCount<=1 then begin
    ShowCmdline;
    Application.Terminate;
    exit;
  end;

  avifn:=ParamStr(1);
  wavfn:=ParamStr(2);

  if lowercase(ParamStr(3))='-a' then begin
    Application.Title:='Alternate(slow) mode '+Application.Title;
    Main.Caption:=Application.Title;
    AltMode:=True;
    end else begin
    AltMode:=False;
  end;

  if FileExists(avifn)=False then begin
    ShowMessage('no file found. ['+avifn+']');
    Application.Terminate;
  end;

  StartupTimer.Enabled:=True;
end;

procedure TMain.StartupTimerTimer(Sender: TObject);
var
  res:boolean;
begin
  Main.ClientWidth:=prgbar.Width;
  Main.ClientHeight:=prgbar.Height;

  StartupTimer.Enabled:=False;

  DSSupport.DSOpen(avifn,0);
  DSSupport.DSClose();

  if AltMode=False then begin
    res:=DSSupport.GetWave(avifn,wavfn,0);
    end else begin
    res:=DSSupport.GetWaveAlt(avifn,wavfn,0);
  end;

  if res=False then begin
    ShowMessage('Error ExtractWave.');
    Application.Terminate;
  end;

  Application.Terminate;
end;

end.
