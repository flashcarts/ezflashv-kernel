unit DSSupportWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, StdCtrls,_PicTools,
  DSPack, DirectShow9,mmsystem, ActiveX,  DirectDraw;

type
  TDSSupport = class(TForm)
    procedure FormCreate(Sender: TObject);
  private
    { Private êÈåæ }
    RequestFPS:double;
    LastTime:double;
  public
    { Public êÈåæ }
    VideoWidth,VideoHeight:integer;
    DSTotalTime,DSFPS:double;
    procedure DSOpen(_MediaFilename:string;_RequestFPS:double);
    procedure DSClose;
    procedure GetBitmap(FrameIndex:integer;bm:TBitmap;VerticalSwap:boolean);
    procedure GetStretchBitmap(FrameIndex:integer;bm:TBitmap;VerticalSwap:boolean);
    function GetWave(avifn,wavfn:string;chtype:integer):boolean;
    function GetWaveAlt(avifn,wavfn:string;chtype:integer):boolean;
  end;

var
  DSSupport: TDSSupport;

implementation

uses MainWin;

{$R *.dfm}

var
  wavefs:TFileStream=nil;

procedure wavefs_Create(fn:string);
begin
  if wavefs<>nil then wavefs.Free;
  wavefs:=TFileStream.Create(fn,fmCreate);
  wavefs.Size:=(4*5)+(sizeof(TWaveFormatEx)-2)+(4*2);
  wavefs.Position:=wavefs.Size;
end;

procedure wavefs_Close(WaveFormatEx:TWaveFormatEx);
var
  tmp:dword;
  procedure dw(dw:dword);
  begin
    wavefs.WriteBuffer(dw,4);
  end;
begin
  if wavefs=nil then exit;

  tmp:=0;
  wavefs.WriteBuffer(tmp,(4-(wavefs.Size mod 4)));

  wavefs.Position:=0;

  dw($46464952);
  dw(wavefs.Size-4);
  dw($45564157);
  dw($20746d66);
  dw(sizeof(TWaveFormatEx)-2);
  wavefs.WriteBuffer(PByteArray(@WaveFormatEx)[0],sizeof(TWaveFormatEx)-2);
  dw($61746164);
  dw(wavefs.Size-wavefs.Position-4);

  wavefs.Free;
  wavefs:=nil;
end;

//-----------------------------------------------------------------

var
  MediaFilename:string;
  lpDD:IDirectDraw;
  lpAMStream:IAMMultiMediaStream;
  lpMediaStream:IMediaStream;
  lpDDStream:IDirectDrawMediaStream;
  ppNewStream:IMediaStream;
  ddpal:IDirectDrawPalette;
  dddes:TDDSurfaceDesc;
  ddflag:dword;
  pSurface:IDirectDrawSurface;
  pRect:TRect;
  ppSample:IDirectDrawStreamSample;
  pPrimarySurface:IDirectDrawSurface;
  pSurfaceDC:HDC;

procedure TDSSupport.FormCreate(Sender: TObject);
begin
  DSTotalTime:=0;
  LastTime:=-1;
end;

procedure TDSSupport.DSOpen(_MediaFilename:string;_RequestFPS:double);
var
  _TotalTime:STREAM_TIME;
  _FPS:STREAM_TIME;
  procedure CreateDirectDraw;
  var
    ddsd:TDDSurfaceDesc;
  begin
    DirectDrawCreate(nil, lpDD, nil);
    lpDD.SetCooperativeLevel(Main.Handle, DDSCL_NORMAL);
    ddsd.dwSize := sizeof(ddsd);
    ddsd.dwFlags := DDSD_CAPS;
    ddsd.ddsCaps.dwCaps := DDSCAPS_PRIMARYSURFACE;
    lpDD.CreateSurface(ddsd, pPrimarySurface, nil);
  end;
  procedure CreateStream;
  begin
    CoCreateInstance(CLSID_AMMultiMediaStream, nil, CLSCTX_INPROC_SERVER, IID_IAMMultiMediaStream, lpAMStream);
    lpAMStream.Initialize(STREAMTYPE_READ, AMMSF_NOGRAPHTHREAD, nil);
    lpAMStream.AddMediaStream(nil, addr(MSPID_PrimaryVideo), 0, ppNewStream);
    lpAMStream.OpenFile(PWideChar(WideString(MediaFilename)), AMMSF_NOCLOCK);
    lpAMStream.GetMediaStream(MSPID_PrimaryVideo, lpMediaStream);
    lpMediaStream.QueryInterface(IID_IDirectDrawMediaStream, lpDDStream);
  end;
  procedure CreateSurface;
  var
    ddsd:TDDSurfaceDesc;
  begin
    lpDDStream.GetFormat(ddsd,ddpal,dddes,ddflag);
    ddsd.dwSize := sizeof(ddsd);
    lpDD.CreateSurface(ddsd,pSurface, nil);
    VideoWidth:=ddsd.dwWidth;
    VideoHeight:=ddsd.dwHeight;
  end;
begin
  if _MediaFilename<>'' then MediaFilename:=_MediaFilename;
  if _RequestFPS<>0 then RequestFPS:=_RequestFPS;

  CreateDirectDraw;
  CreateStream;
  CreateSurface;

  pRect:=rect(0,0,VideoWidth,VideoHeight);
  lpDDStream.CreateSample(pSurface,pRect,0,ppSample);
  lpAMStream.SetState(STREAMSTATE_RUN);

  lpAMStream.GetDuration(_TotalTime);
  DSTotalTime:=_TotalTime/10000000;

  lpDDStream.GetTimePerFrame(_FPS);
  DSFPS:=10000000/_FPS;

  RequestFPS:=DSFPS;

  LastTime:=-1;
end;

procedure TDSSupport.DSClose;
begin
  if lpAMStream<>nil then lpAMStream.SetState(STREAMSTATE_STOP);

  ppSample:=nil;
  lpDDStream:=nil;
  lpMediaStream:=nil;
  lpAMStream:=nil;

  pSurface:=nil;
  pPrimarySurface:=nil;
  lpDD:=nil;
end;

procedure TDSSupport.GetBitmap(FrameIndex:integer;bm:TBitmap;VerticalSwap:boolean);
var
  StartTime,EndTime,CurrentTime:STREAM_TIME;
  TargetTime:Double;
  tmpbm:TBitmap;
  y:integer;
begin
  if RequestFPS=0 then begin
    ShowMessage('FatalError:RequestFPS=0;');
    exit;
  end;

  TargetTime:=FrameIndex/RequestFPS;
//  if LastTime>TargetTime then exit;

  while(LastTime<TargetTime) do begin
    if ppSample.Update(0, 0,PAPCFUNC(0), 0)<>S_OK then exit;
    ppSample.GetSampleTimes(StartTime,EndTime,CurrentTime);
    LastTime:=StartTime/10000000;
  end;

  tmpbm:=TBitmap.Create;
  tmpbm.PixelFormat:=bm.PixelFormat;
  tmpbm.Width:=VideoWidth;
  tmpbm.Height:=VideoHeight;

  pSurface.GetDC(pSurfaceDC);
  BitBlt(tmpbm.Canvas.Handle,0,0,VideoWidth,VideoHeight,pSurfaceDC,0,0,SRCCOPY);
  pSurface.ReleaseDC(pSurfaceDC);

  if VerticalSwap=False then begin
    BitBlt(bm.Canvas.Handle,0,0,VideoWidth,VideoHeight,tmpbm.Canvas.Handle,0,0,SRCCOPY);
    end else begin
    for y:=0 to tmpbm.Height-1 do begin
      BitBlt(bm.Canvas.Handle,0,y,VideoWidth,1,tmpbm.Canvas.Handle,0,tmpbm.Height-1-y,SRCCOPY);
    end;
  end;

  tmpbm.Free;
end;

procedure TDSSupport.GetStretchBitmap(FrameIndex:integer;bm:TBitmap;VerticalSwap:boolean);
var
  tmpbm:TBitmap;
begin
  tmpbm:=TBitmap.Create;
  MakeBlankBM(tmpbm,VideoWidth,VideoHeight,pf24bit);

  GetBitmap(FrameIndex,tmpbm,VerticalSwap);

  SetStretchBltMode(bm.Canvas.Handle,HALFTONE);
  StretchBlt(bm.Canvas.Handle,0,0,bm.Width,bm.Height,tmpbm.Canvas.Handle,0,0,VideoWidth,VideoHeight,SRCCOPY);

  tmpbm.Free;
end;

const AudioBufCount=128*1024;

function TDSSupport.GetWave(avifn,wavfn:string;chtype:integer):boolean;
var
  wfs:TFileStream;
  AudioBuf:array[0..AudioBufCount-1] of SmallInt;
  SrcData:array[0..AudioBufCount-1] of SmallInt;
  lpAMStream:IAMMultiMediaStream;
  lpMediaStream:IMediaStream;
  lpAudioSample:IAudioStreamSample;
  lpAudioStream:IAudioMediaStream;
  lpAudioData:IAudioData;
  retcode:HRESULT;
  WaveFormat:TWAVEFORMATEX;
  ppNewStream:IMediaStream;
  Length:DWORD;
  dummyint:cardinal;
  dummyadr:Pointer;
  SrcSize:integer;
  TotalTime:double;
  SampleCount:integer;
  idx:integer;
  lasttick:dword;
  procedure w32(v:dword);
  begin
    wfs.WriteBuffer(v,4);
  end;
  procedure StartWave(fn:string;WaveFormat:TWAVEFORMATEX);
  begin
    wfs:=TFileStream.Create(fn,fmCreate);

    w32($46464952); // RIFF
    w32(0);
    w32($45564157); // WAVE
    w32($20746d66); // fmt_

    w32($10); // size
    wfs.WriteBuffer(WaveFormat,$10);

    w32($61746164); // data
    w32(0);

    Main.prgbar.Position:=0;
    SampleCount:=0;
    SrcSize:=0;
  end;
  procedure WriteWave;
  begin
    wfs.WriteBuffer(SrcData[0],SrcSize*2);

    inc(SampleCount,SrcSize);
    SrcSize:=0;
  end;
  procedure EndWave;
  begin
    if SrcSize<>0 then WriteWave;

    wfs.Position:=$04;
    w32(wfs.Size-wfs.Position-4);
    wfs.Position:=$28;
    w32(wfs.Size-wfs.Position-4);

    wfs.Free;
  end;
  function GetTotalTime:double;
  var
    _TotalTime:STREAM_TIME;
  begin
    lpAMStream.GetDuration(_TotalTime);
    Result:=_TotalTime/10000000;
  end;
begin
  if FileExists(avifn)=False then begin
    Result:=False;
    exit;
  end;

  CoCreateInstance(CLSID_AMMultiMediaStream, nil, CLSCTX_INPROC_SERVER, IID_IAMMultiMediaStream, lpAMStream);
  lpAMStream.Initialize(STREAMTYPE_READ, AMMSF_NOGRAPHTHREAD, nil);
  lpAMStream.AddMediaStream(nil, addr(MSPID_PrimaryAudio), 0, ppNewStream);
  lpAMStream.OpenFile(PWideChar(WideString(avifn)), AMMSF_RUN);
  lpAMStream.GetMediaStream(MSPID_PrimaryAudio, lpMediaStream);
  lpMediaStream.QueryInterface(IID_IAudioMediaStream, lpAudioStream);
  lpAudioStream.GetFormat(WaveFormat);
  CoCreateInstance(CLSID_AMAudioData, nil, CLSCTX_INPROC_SERVER, IID_IAudioData, lpAudioData);

  lpAudioData.SetBuffer(AudioBufCount, addr(AudioBuf[0]), 0);
  lpAudioData.SetFormat(WaveFormat);
  lpAudioStream.CreateSample(lpAudioData, 0, lpAudioSample);

  TotalTime:=GetTotalTime;
  Main.prgbar.Max:=trunc(TotalTime);

  StartWave(wavfn,WaveFormat);

  lasttick:=GetTickCount;

  retcode:=S_OK;
  while(retcode=S_OK) do begin
    retcode:=lpAudioSample.Update(0, 0,PAPCFUNC(0), 0);
    if(retcode=S_OK) then begin
      lpAudioData.GetInfo(dummyint,dummyadr, Length);
      Length:=Length div 2;

      for idx:=0 to Length-1 do begin
        SrcData[idx]:=AudioBuf[idx];
      end;
      SrcSize:=Length;

      WriteWave;

      if (lasttick+500)<GetTickCount then begin
        lasttick:=GetTickCount;
        Main.prgbar.Position:=SampleCount div integer(WaveFormat.nSamplesPerSec);
        Main.prgbar.Refresh;
      end;
    end;
  end;

  EndWave;

  Main.prgbar.Position:=0;

  if lpAMStream<>nil then lpAMStream.SetState(STREAMSTATE_STOP);

  lpAMStream:=NIL;
  lpMediaStream:=NIL;
  ppNewStream:=NIL;
  lpAudioSample:=NIL;
  lpAudioStream:=NIL;
  lpAudioData:=NIL;

  Result:=True;
end;

const AudioBufCountAlt=4;

function TDSSupport.GetWaveAlt(avifn,wavfn:string;chtype:integer):boolean;
var
  wfs:TFileStream;
  AudioBuf:array[0..AudioBufCountAlt-1] of SmallInt;
  SrcData:array[0..AudioBufCountAlt-1] of SmallInt;
  lpAMStream:IAMMultiMediaStream;
  lpMediaStream:IMediaStream;
  lpAudioSample:IAudioStreamSample;
  lpAudioStream:IAudioMediaStream;
  lpAudioData:IAudioData;
  retcode:HRESULT;
  WaveFormat:TWAVEFORMATEX;
  ppNewStream:IMediaStream;
  Length:DWORD;
  dummyint:cardinal;
  dummyadr:Pointer;
  SrcSize:integer;
  TotalTime:double;
  SampleCount:integer;
  idx:integer;
  lasttick:dword;
  procedure w32(v:dword);
  begin
    wfs.WriteBuffer(v,4);
  end;
  procedure StartWave(fn:string;WaveFormat:TWAVEFORMATEX);
  begin
    wfs:=TFileStream.Create(fn,fmCreate);

    w32($46464952); // RIFF
    w32(0);
    w32($45564157); // WAVE
    w32($20746d66); // fmt_

    w32($10); // size
    wfs.WriteBuffer(WaveFormat,$10);

    w32($61746164); // data
    w32(0);

    Main.prgbar.Position:=0;
    SampleCount:=0;
    SrcSize:=0;
  end;
  procedure WriteWave;
  begin
    wfs.WriteBuffer(SrcData[0],SrcSize*2);

    inc(SampleCount,SrcSize);
    SrcSize:=0;
  end;
  procedure EndWave;
  begin
    if SrcSize<>0 then WriteWave;

    wfs.Position:=$04;
    w32(wfs.Size-wfs.Position-4);
    wfs.Position:=$28;
    w32(wfs.Size-wfs.Position-4);

    wfs.Free;
  end;
  function GetTotalTime:double;
  var
    _TotalTime:STREAM_TIME;
  begin
    lpAMStream.GetDuration(_TotalTime);
    Result:=_TotalTime/10000000;
  end;
begin
  if FileExists(avifn)=False then begin
    Result:=False;
    exit;
  end;

  CoCreateInstance(CLSID_AMMultiMediaStream, nil, CLSCTX_INPROC_SERVER, IID_IAMMultiMediaStream, lpAMStream);
  lpAMStream.Initialize(STREAMTYPE_READ, AMMSF_NOGRAPHTHREAD, nil);
  lpAMStream.AddMediaStream(nil, addr(MSPID_PrimaryAudio), 0, ppNewStream);
  lpAMStream.OpenFile(PWideChar(WideString(avifn)), AMMSF_RUN);
  lpAMStream.GetMediaStream(MSPID_PrimaryAudio, lpMediaStream);
  lpMediaStream.QueryInterface(IID_IAudioMediaStream, lpAudioStream);
  lpAudioStream.GetFormat(WaveFormat);
  CoCreateInstance(CLSID_AMAudioData, nil, CLSCTX_INPROC_SERVER, IID_IAudioData, lpAudioData);

  lpAudioData.SetBuffer(AudioBufCountAlt, addr(AudioBuf[0]), 0);
  lpAudioData.SetFormat(WaveFormat);
  lpAudioStream.CreateSample(lpAudioData, 0, lpAudioSample);

  TotalTime:=GetTotalTime;
  Main.prgbar.Max:=trunc(TotalTime);

  StartWave(wavfn,WaveFormat);

  lasttick:=GetTickCount;

  retcode:=S_OK;
  while(retcode=S_OK) do begin
    retcode:=lpAudioSample.Update(0, 0,PAPCFUNC(0), 0);
    if(retcode=S_OK) then begin
      lpAudioData.GetInfo(dummyint,dummyadr, Length);
      Length:=Length div 2;

      for idx:=0 to Length-1 do begin
        SrcData[idx]:=AudioBuf[idx];
      end;
      SrcSize:=Length;

      WriteWave;

      if (lasttick+500)<GetTickCount then begin
        lasttick:=GetTickCount;
        Main.prgbar.Position:=SampleCount div integer(WaveFormat.nSamplesPerSec);
        Main.prgbar.Refresh;
      end;
    end;
  end;

  EndWave;

  Main.prgbar.Position:=0;

  if lpAMStream<>nil then lpAMStream.SetState(STREAMSTATE_STOP);

  lpAMStream:=NIL;
  lpMediaStream:=NIL;
  ppNewStream:=NIL;
  lpAudioSample:=NIL;
  lpAudioStream:=NIL;
  lpAudioData:=NIL;

  Result:=True;
end;

//-----------------------------------------------------------------

end.
