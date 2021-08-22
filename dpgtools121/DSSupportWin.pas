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
    LastTime,CurTime:double;
    procedure GetBitmap(FrameIndex:integer;bm:TBitmap;VerticalSwap:boolean);
  public
    { Public êÈåæ }
    VideoWidth,VideoHeight:integer;
    DSTotalTime:double;
    procedure DSOpen(_MediaFilename:string;_RequestFPS:double;reqWidth,reqHeight:integer);
    function GetSourceFPS:double;
    procedure DSClose;
    procedure GetStretchBitmap(FrameIndex:integer;bm:TBitmap;VerticalSwap:boolean);
    function GetWave(avifn,wavfn:string;chtype:integer):boolean;
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

var
  RenderLastBM,RenderCurBM:TBitmap;
  RenderFirst:boolean;

procedure TDSSupport.FormCreate(Sender: TObject);
begin
  DSTotalTime:=0;
  LastTime:=-1;
  CurTime:=-1;
end;

procedure TDSSupport.DSOpen(_MediaFilename:string;_RequestFPS:double;reqWidth,reqHeight:integer);
var
  _TotalTime:STREAM_TIME;
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
    ddsd.dwSize := sizeof(ddsd);
    lpDDStream.GetFormat(ddsd,ddpal,dddes,ddflag);
    lpDD.CreateSurface(ddsd,pSurface, nil);
    VideoWidth:=ddsd.dwWidth;
    VideoHeight:=ddsd.dwHeight;
    if (VideoWidth<0) or (2048<VideoWidth) then VideoWidth:=0;
    if (VideoHeight<0) or (2048<VideoHeight) then VideoHeight:=0;
  end;
begin
  if _MediaFilename<>'' then MediaFilename:=_MediaFilename;
  if _RequestFPS<>0 then RequestFPS:=_RequestFPS;

  CreateDirectDraw;
  CreateStream;
  CreateSurface;

  if (reqWidth<>0) and (reqHeight<>0) then begin
    pRect:=rect(0,0,reqWidth,reqHeight);
    end else begin
    pRect:=rect(0,0,VideoWidth,VideoHeight);
  end;
  lpDDStream.CreateSample(pSurface,pRect,0,ppSample);

  lpAMStream.SetState(STREAMSTATE_RUN);
  sleep(1000);
  Application.ProcessMessages;

  lpAMStream.GetDuration(_TotalTime);
  DSTotalTime:=_TotalTime/10000000;

  LastTime:=-1;
  CurTime:=-1;

  RenderLastBM:=TBitmap.Create;
  MakeBlankBM(RenderLastBM,VideoWidth,VideoHeight,pf24bit);
  RenderCurBM:=TBitmap.Create;
  MakeBlankBM(RenderCurBM,VideoWidth,VideoHeight,pf24bit);

  RenderFirst:=True;
end;

function TDSSupport.GetSourceFPS:double;
var
  _FPS:STREAM_TIME;
  DSFPS:double;
begin
  _FPS:=0;

  try
    if assigned(lpDDStream) then begin
      if lpDDStream.GetTimePerFrame(_FPS)=E_POINTER then _FPS:=0;
    end;
    except else begin
    end;
  end;

  if _FPS=0 then begin
    DSFPS:=0;
    end else begin
    DSFPS:=10000000/_FPS;
  end;

{
  if trunc(DSFPS)=15 then DSFPS:=15;
  if trunc(DSFPS)=23 then DSFPS:=23.976;
  if trunc(DSFPS)=24 then DSFPS:=24;
  if trunc(DSFPS)=25 then DSFPS:=25;
  if trunc(DSFPS)=29 then DSFPS:=29.97;
  if trunc(DSFPS)=30 then DSFPS:=30;
  if trunc(DSFPS)=50 then DSFPS:=50;
  if trunc(DSFPS)=59 then DSFPS:=59.94;
  if trunc(DSFPS)=60 then DSFPS:=60;
}

  Result:=DSFPS;
end;

procedure TDSSupport.DSClose;
begin
  if assigned(lpAMStream) then lpAMStream.SetState(STREAMSTATE_STOP);
  sleep(1000);
  Application.ProcessMessages;

  ppSample:=nil;
  lpDDStream:=nil;
  lpMediaStream:=nil;
  lpAMStream:=nil;

  pSurface:=nil;
  pPrimarySurface:=nil;
  lpDD:=nil;

  if assigned(RenderLastBM) then begin
    RenderLastBM.Free;
    RenderLastBM:=nil;
  end;
  if assigned(RenderCurBM) then begin
    RenderCurBM.Free;
    RenderCurBM:=nil;
  end;

  RenderFirst:=False;
end;

{
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
}

var
  lastbm:TBitmap=nil;

procedure TDSSupport.GetBitmap(FrameIndex:integer;bm:TBitmap;VerticalSwap:boolean);
var
  StartTime,EndTime,CurrentTime:STREAM_TIME;
  TargetTime:Double;
  tmpbm:TBitmap;
  x,y:integer;
  w,h:integer;
  blend:integer;
  plb,pcb,ptb:PByteArray;
begin
  if RequestFPS=0 then begin
    ShowMessage('FatalError:RequestFPS=0;');
    exit;
  end;

  w:=VideoWidth;
  h:=VideoHeight;

  TargetTime:=FrameIndex/RequestFPS;

  while(CurTime<TargetTime) do begin
    if ppSample.Update(0, 0,PAPCFUNC(0), 0)<>S_OK then exit;
    ppSample.GetSampleTimes(StartTime,EndTime,CurrentTime);
    LastTime:=CurTime;
    CurTime:=StartTime/10000000;

    BitBlt(RenderLastBM.Canvas.Handle,0,0,w,h,RenderCurBM.Canvas.Handle,0,0,SRCCOPY);
    pSurface.GetDC(pSurfaceDC);
    BitBlt(RenderCurBM.Canvas.Handle,0,0,w,h,pSurfaceDC,0,0,SRCCOPY);
    pSurface.ReleaseDC(pSurfaceDC);
  end;

  tmpbm:=TBitmap.Create;
  tmpbm.PixelFormat:=bm.PixelFormat;
  tmpbm.Width:=VideoWidth;
  tmpbm.Height:=VideoHeight;

  if (LastTime<=0) or (CurTime<=0) or (TargetTime<=LastTime) or (CurTime<=LastTime) or (CurTime<=TargetTime) then begin
    BitBlt(tmpbm.Canvas.Handle,0,0,w,h,RenderCurBM.Canvas.Handle,0,0,SRCCOPY);
    end else begin
    blend:=trunc((TargetTime-LastTime)/(CurTime-LastTime)*$100);
    if blend<$00 then blend:=$00;
    if $ff<blend then blend:=$ff;
    for y:=0 to h-1 do begin
      plb:=RenderLastBM.ScanLine[y];
      pcb:=RenderCurBM.ScanLine[y];
      ptb:=tmpbm.ScanLine[y];
      for x:=0 to (w*3)-1 do begin
        ptb[x]:=(plb[x]*($ff-blend) div $100)+(pcb[x]*blend div $100);
      end;
    end;
  end;

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
  AudioBuf:array[0..2-1] of SmallInt;
  SrcData:array[0..AudioBufCount-1] of SmallInt;
  lpAMStream:IAMMultiMediaStream;
  lpMediaStream:IMediaStream;
  lpAudioSample:IAudioStreamSample;
  lpAudioStream:IAudioMediaStream;
  lpAudioData:IAudioData;
  retcode:HRESULT;
  WaveFormat:TWAVEFORMATEX;
  DownMixFlag:boolean;
  ppNewStream:IMediaStream;
  Length:DWORD;
  dummyint:cardinal;
  dummyadr:Pointer;
  SrcSize:integer;
  TotalTime:double;
  SampleCount:integer;
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

    if DownMixFlag=True then begin
      with WaveFormat do begin
        nChannels:=nChannels div 2;
        nAvgBytesPerSec:=nAvgBytesPerSec div 2;
        nBlockAlign:=nBlockAlign div 2;
      end;
    end;

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
    Main.prgbar.Position:=SampleCount div integer(WaveFormat.nSamplesPerSec);
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

  lpAudioData.SetBuffer(4, addr(AudioBuf[0]), 0);
  lpAudioData.SetFormat(WaveFormat);
  lpAudioStream.CreateSample(lpAudioData, 0, lpAudioSample);

  // Stereo to Mono
  if WaveFormat.nChannels=2 then begin
    DownMixFlag:=True;
    end else begin
    DownMixFlag:=False;
  end;

  TotalTime:=GetTotalTime;
  Main.prgbar.Max:=trunc(TotalTime);

  StartWave(wavfn,WaveFormat);

{
  retcode:=S_OK;
  while(retcode=S_OK) do begin
    retcode:=lpAudioSample.Update(0, 0,PAPCFUNC(0), 0);
    if(retcode=S_OK) then begin
      lpAudioData.GetInfo(dummyint,dummyadr, Length);
      if DownMixFlag=False then begin
        SrcData[SrcSize+0]:=AudioBuf[0];
        SrcData[SrcSize+1]:=AudioBuf[1];
        inc(SrcSize,2);
        end else begin
        case chtype of
          0: begin // mix
            SrcData[SrcSize]:=(AudioBuf[0] div 2)+(AudioBuf[1] div 2);
          end;
          1: begin // left
            SrcData[SrcSize]:=AudioBuf[0];
          end;
          2: begin // right
            SrcData[SrcSize]:=AudioBuf[1];
          end;
        end;
        inc(SrcSize,1);
      end;

      if AudioBufCount<=SrcSize then WriteWave;
    end;
  end;
}

  if DownMixFlag=False then begin
    retcode:=S_OK;
    while(retcode=S_OK) do begin
      retcode:=lpAudioSample.Update(0, 0,PAPCFUNC(0), 0);
      if(retcode=S_OK) then begin
        lpAudioData.GetInfo(dummyint,dummyadr, Length);
        SrcData[SrcSize+0]:=AudioBuf[0];
        SrcData[SrcSize+1]:=AudioBuf[1];
        inc(SrcSize,2);
        if AudioBufCount<=SrcSize then WriteWave;
      end;
    end;
    end else begin
    case chtype of
      0: begin // mix
        retcode:=S_OK;
        while(retcode=S_OK) do begin
          retcode:=lpAudioSample.Update(0, 0,PAPCFUNC(0), 0);
          if(retcode=S_OK) then begin
            lpAudioData.GetInfo(dummyint,dummyadr, Length);
            SrcData[SrcSize]:=(AudioBuf[0] div 2)+(AudioBuf[1] div 2);
            inc(SrcSize,1);
            if AudioBufCount<=SrcSize then WriteWave;
          end;
        end;
      end;
      1: begin // left
        retcode:=S_OK;
        while(retcode=S_OK) do begin
          retcode:=lpAudioSample.Update(0, 0,PAPCFUNC(0), 0);
          if(retcode=S_OK) then begin
            lpAudioData.GetInfo(dummyint,dummyadr, Length);
            SrcData[SrcSize]:=AudioBuf[0];
            inc(SrcSize,1);
            if AudioBufCount<=SrcSize then WriteWave;
          end;
        end;
      end;
      2: begin // right
        retcode:=S_OK;
        while(retcode=S_OK) do begin
          retcode:=lpAudioSample.Update(0, 0,PAPCFUNC(0), 0);
          if(retcode=S_OK) then begin
            lpAudioData.GetInfo(dummyint,dummyadr, Length);
            SrcData[SrcSize]:=AudioBuf[1];
            inc(SrcSize,1);
            if AudioBufCount<=SrcSize then WriteWave;
          end;
        end;
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
