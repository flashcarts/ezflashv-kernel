unit dpgenc_language;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, _m_Tools, StdCtrls;

{$WARN UNIT_PLATFORM OFF}
{$WARN SYMBOL_PLATFORM OFF}

const LI_ID_Title=0;
const LI_ID_FontName=1;
const LI_ID_FontSize=2;
const LI_ID_FontCodepage=3;

const LI_FormSourceLabel=0;
const LI_FormSelect=1;
const LI_FormCancel=2;
const LI_FormStart=3;
const LI_FormShowOptions=4;
const LI_FormAudioOptions=5;
const LI_FormAudioRate=6;
const LI_FormAudioRateUnit=7;
const LI_FormAudioBPS=8;
const LI_FormAudioBPSUnit=9;
const LI_FormAudioVolume=10;
const LI_FormAudioGetModeItem0=11;
const LI_FormAudioGetModeItem1=12;
const LI_FormAudioGetModeItem2=13;
const LI_FormAudioGetModeItem3=14;
const LI_FormAudioGetModeItem4=15;
//const LI_FormVideoReencode=16; // not used
const LI_FormVideoOptions=17;
const LI_FormVideoWidth=18;
const LI_FormVideoWidthUnit=19;
const LI_FormVideoAspect=20;
const LI_FormVideoAspectItem0=21;
const LI_FormVideoAspectItem1=22;
const LI_FormVideoAspectItem2=23;
const LI_FormVideoAspectItem3=24;
const LI_FormVideoBPS=25;
const LI_FormVideoBPSUnit=26;
const LI_FormVideoFPS=27;
const LI_FormVideoFPSUnit=28;
const LI_FormVideoBright=29;
const LI_FormVideoBlur=30;
const LI_FormVideoBlurDeep=31;
const LI_FormVideoBlurLight=32;
const LI_FormVideoFlip=33;

const LI_DirectMpegVideoSizeError=34;
const LI_AboutFrameRate=35;
const LI_DetectErrorVideoSize=36;
const LI_DetectErrorTotalTime=37;
const LI_DetectErrorIlligalAspect=38;
const LI_PipeErrorCreate=39;
const LI_PipeErrorAttribute=40;
const LI_GetAudioError=41;

const LI_Preenc_DS=42;
const LI_Preenc_ffmpeg=43;
const LI_Preenc_menc=44;

const LI_OutputPathTag=45;
const LI_OutputPathChange=46;
const LI_OutputPathDlgTitle=47;
const LI_OutputPathDlgFilename=48;

const LI_MainMenu_File=49;
const LI_MainMenu_File_Exit=50;
const LI_MainMenu_Option=51;
const LI_MainMenu_ShowLog=52;
const LI_MainMenu_Language=53;
const LI_MainMenu_About=54;

const LI_QueueMessage=55;

const LI_CancelPopup=56;
const LI_Canceling=57;
const LI_Canceled=58;

const LI_DetectErrorFPS=59;
const LI_DetectErrorWMV3=60;

const LI_FormVideoPixelFormat=61;
const LI_FormVideoPixelFormatItem0=62;
const LI_FormVideoPixelFormatItem1=63;
const LI_FormVideoPixelFormatItem2=64;
const LI_FormVideoPixelFormatItem3=65;

const LI_OptionAdvance=66;

const LI_FormCustomEdit=67;
const LI_FormProfileItem0=68;
const LI_FormProfileItem1=69;
const LI_FormProfileItem2=70;

procedure LoadLngFile(fn:string);
function lngID(LI_ID:integer):string;
function lng(LI:integer):string;

implementation

const CRLF:string=char($0d)+char($0a);

var
  lngstr:TStringList;

procedure LoadLngFile(fn:string);
begin
  if assigned(lngstr)=true then begin
    lngstr.Free;
    lngstr:=NIL;
  end;

  if FileExists(fn)=False then begin
    ShowMessage('FatalError:not found '+fn);
    exit;
  end;

  lngstr:=TStringList.Create;
  lngstr.LoadFromFile(fn);
end;

function lngID(LI_ID:integer):string;
begin
  if assigned(lngstr)=false then begin
    Result:='';
    exit;
  end;
  if (LI_ID<0) or (lngstr.Count<=LI_ID) then begin
    Result:='';
    exit;
  end;

  Result:=lngstr[LI_ID];
end;

function lng(LI:integer):string;
var
  pos:integer;
begin
  inc(LI,4);

  if assigned(lngstr)=false then begin
    Result:='load not .lng file';
    exit;
  end;

  Result:='';

  if (0<=LI) and (LI<lngstr.Count) then begin
    Result:=lngstr[LI];
  end;

  if Result='' then begin
    Result:='nodef';
    if LI=(4+LI_Preenc_DS) then Result:='Directly convert with DirectShow. (for format supported by WMP) (fast)';
    if LI=(4+LI_Preenc_ffmpeg) then Result:='Prepares video with using ffmpeg. (for specific format) (slow)';
    if LI=(4+LI_Preenc_menc) then Result:='Prepares video with using mencoder. (not support)';
    if LI=(4+LI_OutputPathTag) then Result:='Output path : ';
    if LI=(4+LI_OutputPathChange) then Result:='Change';
    if LI=(4+LI_OutputPathDlgTitle) then Result:='Select output path.';
    if LI=(4+LI_OutputPathDlgFilename) then Result:='Move to output path. and press save button.';
    if LI=(4+LI_MainMenu_File) then Result:='File (&F)';
    if LI=(4+LI_MainMenu_File_Exit) then Result:='Exit (&X)';
    if LI=(4+LI_MainMenu_Option) then Result:='Option (&O)';
    if LI=(4+LI_MainMenu_ShowLog) then Result:='ShowLog (&S)';
    if LI=(4+LI_MainMenu_Language) then Result:='Language (&L)';
    if LI=(4+LI_MainMenu_About) then Result:='About (&A)';
    if LI=(4+LI_QueueMessage) then Result:='Please drag and drop the movie file(s).\nPlease never drop the format that cannot correspond.';
    if LI=(4+LI_CancelPopup) then Result:='Cancel this';
    if LI=(4+LI_Canceling) then Result:='Wait for cancel...';
    if LI=(4+LI_Canceled) then Result:='Canceled.';
    if LI=(4+LI_DetectErrorFPS) then Result:='can not get FPS from movie file.';
    if LI=(4+LI_DetectErrorWMV3) then Result:='not support [WMV3 / Windows Media Video 9] format.';
    if LI=(4+LI_FormVideoPixelFormat) then Result:='Pixel format';
    if LI=(4+LI_FormVideoPixelFormatItem0) then Result:='RGB15 for speed';
    if LI=(4+LI_FormVideoPixelFormatItem1) then Result:='RGB18 default';
    if LI=(4+LI_FormVideoPixelFormatItem2) then Result:='RGB21 for quality';
    if LI=(4+LI_FormVideoPixelFormatItem3) then Result:='RGB24 old version default';
    if LI=(4+LI_OptionAdvance) then Result:='Advance settings';
    if LI=(4+LI_FormCustomEdit) then Result:='custom edit';
    if LI=(4+LI_FormProfileItem0) then Result:='MEncoder: Custom comand line';
    if LI=(4+LI_FormProfileItem1) then Result:='MEncoder: Low-quality profile (fast encode)';
    if LI=(4+LI_FormProfileItem2) then Result:='MEncoder: High-quality profile (slow encode)';
    exit;
  end;

  pos:=ansipos('\n',Result);
  while(pos<>0) do begin
    Result[pos+0]:=char($0d);
    Result[pos+1]:=char($0a);
    pos:=ansipos('\n',Result);
  end;

end;

end.
