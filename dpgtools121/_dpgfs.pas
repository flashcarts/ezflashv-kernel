unit _dpgfs;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls,_PicTools, StdCtrls;

var
  BaseFilename:string;

procedure SetBaseFilename(srcfn,dstfn:string);
function GetSourceFilename:string;
function GetDPGFilename:string;
function GetDPGMovieFilename:string;
function GetDPGWaveFilename:string;
function GetDPGMP2Filename:string;
function GetDPGGOPListFilename:string;
function GetExternalWaveFilename:string;
function GetTempFilename:string;
function GetTempffmpegFilename:string;

implementation

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

var
  SourceFilename:string;

procedure SetBaseFilename(srcfn,dstfn:string);
begin
  SourceFilename:=srcfn;
  BaseFilename:=ChangeFileExt(dstfn,'');
end;

function GetDPGFilename:string;
begin
  Result:=BaseFilename+'.dpg';
end;

function GetDPGMovieFilename:string;
begin
  Result:=BaseFilename+'.dpgm1v';
end;

function GetDPGWaveFilename:string;
begin
  Result:=BaseFilename+'.dpg.wav';
end;

function GetDPGMP2Filename:string;
begin
  Result:=BaseFilename+'.dpg.mp2';
end;

function GetDPGGOPListFilename:string;
begin
  Result:=BaseFilename+'.dpg.gls';
end;

function GetExternalWaveFilename:string;
begin
  Result:=ChangeFileExt(SourceFilename,'.wav');
end;

function GetSourceFilename:string;
begin
  Result:=SourceFilename;
end;

function GetTempFilename:string;
begin
  Result:=BaseFilename+'.temp$$$';
end;

function GetTempffmpegFilename:string;
begin
  Result:=BaseFilename+'.ffmpeg.mpg';
end;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

end.
