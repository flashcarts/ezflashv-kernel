unit MainWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, GLDPNG, ComCtrls, ExtCtrls,_PicTools,jpeg,zlib,NkDIB;

type
  TMain = class(TForm)
    FilesLst: TListBox;
    prv64: TImage;
    prv256: TImage;
    PrgBar: TProgressBar;
    PrgLbl: TLabel;
    StopBtn: TButton;
    GLDPNGMemo: TMemo;
    PathDlg: TOpenDialog;
    StartupTimer: TTimer;
    BaseINIMemo: TMemo;
    procedure StopBtnClick(Sender: TObject);
    procedure StartupTimerTimer(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    { Private 宣言 }
  public
    { Public 宣言 }
  end;

var
  Main: TMain;

implementation

{$R *.dfm}

uses _inifile;

const CRLF:string=char($0d)+char($0a);

function GetWStrCRC32Fast(var wstr:widestring):dword;
var
  CRC:dword;
  uc:dword;
  cnt:integer;
begin
  CRC:=0;

  for cnt:=0 to Length(wstr)-1 do begin
    if word(wstr[cnt+1])<>0 then begin
      uc:=word(wstr[cnt+1]);
//      if (dword('a')<=uc) and (uc<=dword('z')) then dec(uc,$20);
      CRC:=CRC xor uc;
      CRC:=((CRC shr (32-8)) and $000000ff)+((CRC and $00ffffff) shl 8);
    end;
  end;

  if CRC=0 then CRC:=1;

  Result:=dword(CRC);
end;

type
  TFileInfo=record
    FilenameW:widestring;
    FilenameCRC:dword;
    ext:string;
    Offset:dword;
  end;

type
  TImgData=record
    bmmst,bm64,bm256:TBitmap;
    cmp64data,cmp256data:pbytearray;
    cmp64size,cmp256size:integer;
    decmp64size,decmp256size:integer;
  end;

var
  BasePath:string;
  FileInfo:array of TFileInfo;
  FileInfoCount:integer;
  ImgData:TImgData;

procedure GetFilesLst(var FilesLst:TListBox;path:string);
var
  res:integer;
  SearchRec: TSearchRec;
  fname,ext:string;
begin
  FilesLst.Clear;

  if DirectoryExists(path)=False then exit;

  res:=FindFirst(path+'*.*', (FaAnyFile), SearchRec);
  if res=0 then begin
    repeat
      if (SearchRec.Attr and faDirectory)=0 then begin
        fname:=SearchRec.Name;
        ext:=ansilowercase(extractfileext(fname));
        if (ext='.bmp') or (ext='.jpg') or (ext='.png') then begin
          FilesLst.Items.Add(fname);
        end;
      end;
      res:=FindNext(SearchRec);
    until (res<>0);
  end;
  FindClose(SearchRec);
end;

procedure ResetFileInfo(var FilesLst:TListBox);
var
  idx:integer;
  idx0,idx1:integer;
  MultiHashCount:integer;
  fn:string;
begin
  FileInfoCount:=FilesLst.Items.Count;
  setlength(FileInfo,FileInfoCount);

  for idx:=0 to FileInfoCount-1 do begin
    with FileInfo[idx] do begin
      fn:=FilesLst.Items[idx];
      FilenameW:=widestring(fn);
      FilenameCRC:=GetWStrCRC32Fast(FilenameW);
      ext:=ansilowercase(extractfileext(fn));
      Offset:=0;

      FilesLst.Items[idx]:=inttohex(FilenameCRC,8)+' '+FilesLst.Items[idx];
    end;
  end;

  MultiHashCount:=0;

  for idx0:=0 to FileInfoCount-2 do begin
    for idx1:=idx0+1 to FileInfoCount-1 do begin
      if FileInfo[idx0].FilenameCRC=FileInfo[idx1].FilenameCRC then inc(MultiHashCount);
    end;
  end;

  Main.Caption:=format('MultiHashCount:%d FileInfoCount:%d',[MultiHashCount,FileInfoCount]);;
end;

function ProcThumb_LoadFromFile_bmp(var bm:TBitmap;fn:widestring):boolean;
begin
  bm:=TBitmap.Create;
  try
    bm.LoadFromFile(BasePath+fn);
    bm.PixelFormat:=pf24bit;
    except else begin
      Result:=False;
      exit;
    end;
  end;

  if (bm.Height=0) or (bm.Width=0) then begin
    bm.Free;
    Result:=False;
    exit;
  end;

  Result:=True;
end;

function ProcThumb_LoadFromFile_jpg(var bm:TBitmap;fn:widestring):boolean;
var
  JpegImg:TJpegImage;
begin
  JpegImg:=TJpegImage.Create;

  JpegImg.PixelFormat:=jf24bit;
  JpegImg.Grayscale:=False;

  JpegImg.ProgressiveDisplay:=False;
  JpegImg.LoadFromFile(BasePath+fn);

  if (JpegImg.Height=0) or (JpegImg.Width=0) then begin
    JpegImg.Free;
    Result:=False;
    exit;
  end;

  bm:=TBitmap.Create;
  MakeBlankBM(bm,JpegImg.Width,JpegImg.Height,pf24bit);

  bm.Canvas.Draw(0,0,JpegImg);

  JpegImg.Free;

  Result:=True;
end;

function ProcThumb_LoadFromFile_png(var bm:TBitmap;fn:widestring):boolean;
var
  png:TGLDPNG;
begin
  bm:=TBitmap.Create;

  png:=TGLDPNG.Create;
  png.Image:=bm;
  png.LoadFromFile(BasePath+fn);
  png.Free;

  Result:=True;
end;

function ProcThumb_LoadFromFile(var bm:TBitmap;fn:widestring;ext:string):boolean;
begin
  if ext='.bmp' then begin
    Result:=ProcThumb_LoadFromFile_bmp(bm,fn);
    exit;
  end;

  if ext='.jpg' then begin
    Result:=ProcThumb_LoadFromFile_jpg(bm,fn);
    exit;
  end;

  if ext='.png' then begin
    Result:=ProcThumb_LoadFromFile_png(bm,fn);
    exit;
  end;

  Result:=False;
end;

procedure ProcThumb_Reduce(var bmmst,bmdst:TBitmap;lx,ly:integer;ScreenRatio:integer);
var
  aspect:double;
  cx,cy:integer;
  px,py:integer;
begin
  aspect:=bmmst.Height/bmmst.Width;
  cx:=trunc(ly/aspect);
  cy:=ly;
  if cx>lx then begin
    cx:=lx;
    cy:=trunc(lx*aspect);
  end;
  cx:=(cx*ScreenRatio) div 100;
  cy:=(cy*ScreenRatio) div 100;
  if cx=0 then cx:=1;
  if cy=0 then cy:=1;

  px:=(lx-cx) div 2;
  py:=(ly-cy) div 2;

  bmdst:=TBitmap.Create;
  MakeBlankBM(bmdst,lx,ly,pf24bit);

//  SetStretchBltMode(bmdst.Canvas.Handle,HALFTONE);
  SetStretchBltMode(bmdst.Canvas.Handle,MAXSTRETCHBLTMODE);
  StretchBlt(bmdst.Canvas.Handle,px,py,cx,cy,bmmst.Canvas.Handle,0,0,bmmst.Width,bmmst.Height,SRCCOPY);
end;

procedure ProcThumb(idx:integer);
begin
  with FileInfo[idx] do begin
    Main.PrgLbl.Caption:=FilenameW;
    Main.PrgLbl.Refresh;

    if ProcThumb_LoadFromFile(ImgData.bmmst,FilenameW,ext)=False then begin
      FilenameW:='';
      FilenameCRC:=0;
      ext:='';
      exit;
    end;
  end;

  with ImgData do begin
    ProcThumb_Reduce(bmmst,bm64,64,48,100);
    BitBlt(Main.prv64.Canvas.Handle,0,0,bm64.Width,bm64.Height,bm64.Canvas.Handle,0,0,SRCCOPY);
    Main.prv64.Refresh;

    if INI_ScreenRatio<>0 then begin
      ProcThumb_Reduce(ImgData.bmmst,bm256,256,192,INI_ScreenRatio);
      BitBlt(Main.prv256.Canvas.Handle,0,0,bm256.Width,bm256.Height,bm256.Canvas.Handle,0,0,SRCCOPY);
      Main.prv256.Refresh;
    end;
  end;

end;

// --- 15bit stuff

var
  B15Data:array of word;
  B15DataCount:integer;

procedure dither(srcbm:TBitmap);
var
  x,y,w,h:integer;
  psbm:PByteArray;
  r,g,b:integer;
  dr,dg,db:integer;
  function d2(c:integer;var dc:integer):integer;
  var
    a:integer;
  begin
    a:=c+dc;
    dc:=a and 7;
    Result:=a shr 3;
    if Result>31 then Result:=31;
  end;
  function RGB15(r,g,b:integer):word;
  begin
    Result:=(b shl 10)+(g shl 5)+(r shl 0)+(1 shl 15);
  end;
begin
  w:=srcbm.Width;
  h:=srcbm.Height;

  dr:=0;
  dg:=0;
  db:=0;

  B15DataCount:=w*h;
  setlength(B15Data,B15DataCount);

  for y:=0 to h-1 do begin
    psbm:=srcbm.ScanLine[y];
    for x:=0 to w-1 do begin
      b:=psbm[x*3+0];
      g:=psbm[x*3+1];
      r:=psbm[x*3+2];
      B15Data[x+(y*w)]:=RGB15(d2(r,dr),d2(g,dg),d2(b,db));
    end;
  end;
end;

// --- end of 15bit stuff

// --- 8bit stuff

var
  B8Data:array of byte;
  B8DataCount:integer;

procedure dither8bit(srcbm:TBitmap;ColorCount:integer;prvimg:TImage);
var
  x,y,w,h:integer;
  psbm:PByteArray;
  dataofs:integer;
  dib:TNkDIB;
  dibc:TNkDIBCanvas;
  ppal:pwordarray;
  palcnt:integer;
  palidx:integer;
  function RGB15(rgb:dword):word;
  var
    r,g,b:integer;
  begin
    r:=((rgb shr 0) and $ff) shr 3;
    g:=((rgb shr 8) and $ff) shr 3;
    b:=((rgb shr 16) and $ff) shr 3;

    Result:=(b shl 10)+(g shl 5)+(r shl 0)+(1 shl 15);
  end;
begin
  case ColorCount of
    16: begin end;
    256: begin end;
    else begin
      ShowMessage('Unknown color count. ColorCount=(16=4bitColor, 256=8bitColor)');
      B8DataCount:=1;
      setlength(B8Data,B8DataCount);
      B8Data[0]:=0;
      exit;
    end;
  end;

  dib:=TNkDIB.Create;

  dib.PixelFormat:=NkPf24bit;
  dib.Width:=srcbm.Width;
  dib.Height:=srcbm.Height;

  dibc:=TNkDIBCanvas.Create(dib);
  BitBlt(dibc.Handle,0,0,dib.Width,dib.Height,srcbm.Canvas.Handle,0,0,SRCCOPY);
  dibc.Free;

  dib.ConvertMode:=nkCmFine;
  case ColorCount of
    16: dib.PixelFormat:=NkPf4bit;
    256: dib.PixelFormat:=NkPf8bit;
  end;

  dibc:=TNkDIBCanvas.Create(dib);
  BitBlt(prvimg.Canvas.Handle,0,0,dib.Width,dib.Height,dibc.Handle,0,0,SRCCOPY);
  prvimg.Refresh;
  dibc.Free;

  palcnt:=dib.PaletteSize;
  if (palcnt=0) or (palcnt<dib.PaletteSize) then begin
    ShowMessage('unknown error:Illigal palette size. ('+inttostr(palcnt)+')');
    exit;
  end;

  dataofs:=2+(ColorCount*2);
  w:=dib.Width;
  h:=dib.Height;

  case ColorCount of
    16: w:=w div 2;
    256: w:=w*1;
  end;

  B8DataCount:=dataofs+(w*h);

  setlength(B8Data,B8DataCount);

  ppal:=pwordarray(B8Data);
  ppal[0]:=word(ColorCount);
  for palidx:=0 to ColorCount-1 do begin
    ppal[1+palidx]:=0;
  end;
  for palidx:=0 to palcnt-1 do begin
    ppal[1+palidx]:=RGB15(dib.Colors[palidx]);
  end;

  for y:=0 to h-1 do begin
    psbm:=dib.ScanLine[y];
    for x:=0 to w-1 do begin
      B8Data[dataofs+x+(y*w)]:=psbm[x];
    end;
  end;

  dib.Free;
end;

// --- end of 8bit stuff

procedure zlibCompress_CompressBuf(const InBuf: Pointer; InBytes: Integer;
                      out OutBuf: Pointer; out OutBytes: Integer);
var
  strm: TZStreamRec;
  P: Pointer;
  function CCheck(code: Integer): Integer;
  begin
    Result := code;
    if code < 0 then
      ShowMessage('ZLIB compression error('+inttostr(code)+')');
  end;
begin
  FillChar(strm, sizeof(strm), 0);
  strm.zalloc := zlibAllocMem;
  strm.zfree := zlibFreeMem;
  OutBytes := ((InBytes + (InBytes div 10) + 12) + 255) and not 255;
  GetMem(OutBuf, OutBytes);
  try
    strm.next_in := InBuf;
    strm.avail_in := InBytes;
    strm.next_out := OutBuf;
    strm.avail_out := OutBytes;
    CCheck(deflateInit_(strm, Z_BEST_COMPRESSION, zlib_version, sizeof(strm)));
    try
      while CCheck(deflate(strm, Z_FINISH)) <> Z_STREAM_END do
      begin
        P := OutBuf;
        Inc(OutBytes, 256);
        ReallocMem(OutBuf, OutBytes);
        strm.next_out := PChar(Integer(OutBuf) + (Integer(strm.next_out) - Integer(P)));
        strm.avail_out := 256;
      end;
    finally
      CCheck(deflateEnd(strm));
    end;
    ReallocMem(OutBuf, strm.total_out);
    OutBytes := strm.total_out;
  except
    FreeMem(OutBuf);
    raise
  end;
end;

procedure zlibCompress(srcdata:pointer;srcsize:integer;out dstdata:pbytearray;out dstsize:integer;var decmpsize:integer);
begin
  decmpsize:=0;

  if srcsize<=0 then exit;

  zlibCompress_CompressBuf(srcdata,srcsize,pointer(dstdata),dstsize);

  if (dstdata=nil) or (dstsize=0) then exit;

  decmpsize:=srcsize;

//  FreeMem(_EncData,_EncDataSize);
end;

procedure WriteData(idx:integer;var wfs:TFileStream);
var
  fnw:widestring;
  fnl:integer;
  tmp:integer;
begin
  fnw:=FileInfo[idx].FilenameW+widechar(0);
  fnl:=length(fnw)*2;

  wfs.WriteBuffer(fnl,4);
  wfs.WriteBuffer(fnw[1],fnl);

  with ImgData do begin
    tmp:=cmp64size+4;
    wfs.WriteBuffer(tmp,4);
    wfs.WriteBuffer(decmp64size,4);
    wfs.WriteBuffer(cmp64data[0],cmp64size);
    if INI_ScreenRatio=0 then begin
      tmp:=0;
      wfs.WriteBuffer(tmp,4);
      end else begin
      tmp:=cmp256size+4;
      wfs.WriteBuffer(tmp,4);
      wfs.WriteBuffer(decmp256size,4);
      wfs.WriteBuffer(cmp256data[0],cmp256size);
    end;
  end;
end;

procedure FreeThumb(idx:integer);
  procedure f(var bm:TBitmap);
  begin
    if Assigned(bm)=True then bm.Free;
    bm:=nil;
  end;
  procedure fm(var p:pbytearray;var s:integer);
  begin
    if Assigned(p)=True then FreeMem(p,s);
    p:=nil;
    s:=0;
  end;
begin
  with ImgData do begin
    f(bmmst);
    f(bm64);
    f(bm256);
    fm(cmp64data,cmp64size);
    fm(cmp256data,cmp256size);
    decmp64size:=0;
    decmp256size:=0;
  end;
end;

var
  ReqStop:boolean;

procedure TMain.StopBtnClick(Sender: TObject);
begin
  if MessageDlg('サムネイル生成を中止してもよろしいですか？'+CRLF+CRLF+'May I discontinue thumbnail generation?',mtConfirmation,[mbOK,mbCancel],0)=mrCancel then exit;
  ReqStop:=True;
end;

function GetBasePath:string;
begin
  Result:='';

  if ParamStr(1)<>'' then begin
    Result:=ParamStr(1);
    if copy(Result,length(Result),1)<>'\' then Result:=Result+'\';
    end else begin
    if Main.PathDlg.Execute=True then Result:=ExtractFilePath(Main.PathDlg.FileName);
  end;

  if Result='' then exit;
  
  if DirectoryExists(Result)=False then begin
    ShowMessage('not found path ['+Result+']');
    Result:='';
    exit;
  end;
end;

procedure TMain.StartupTimerTimer(Sender: TObject);
var
  idx:integer;
  wfs:TFileStream;
  tmp0:dword;
begin
  StartupTimer.Enabled:=False;

  ReqStop:=False;

  PrgLbl.Caption:='';
  Main.Show;
  Application.ProcessMessages;

  CreateINI;
  
  BasePath:=GetBasePath;
  if BasePath='' then begin
    Application.Terminate;
    exit;
  end;

  LoadINI;

  GLDPNGMemo.Visible:=False;
  Application.ProcessMessages;

  MakeBlankImg(prv64,pf24bit);
  MakeBlankImg(prv256,pf24bit);

  GetFilesLst(FilesLst,BasePath);

  ResetFileInfo(FilesLst);

  if FileInfoCount=0 then begin
    ShowMessage('can not found image files.');
    Application.Terminate;
    exit;
  end;

  tmp0:=0;

  wfs:=TFileStream.Create(BasePath+'_THUMBNL.MSL',fmCreate);
  wfs.Size:=0;

  wfs.Position:=0;
  wfs.WriteBuffer(tmp0,4); // count
  for idx:=0 to FileInfoCount-1 do begin
    wfs.WriteBuffer(tmp0,4); // crc
    wfs.WriteBuffer(tmp0,4); // ofs
  end;

  PrgBar.Max:=FileInfoCount-1;

  for idx:=0 to FileInfoCount-1 do begin
    FilesLst.ItemIndex:=idx;
    PrgBar.Position:=idx;
    Application.ProcessMessages;

    if ReqStop=True then begin
      wfs.Free;
      DeleteFile(BasePath+'_THUMBNL.MSL');
      Application.Terminate;
      exit;
    end;

    ProcThumb(idx);

{
    with ImgData do begin
      dither(bm64,16);
      zlibCompress(B15Data,B15DataCount*2,cmp64data,cmp64size);
      dither(bm256,INI_ColorCount);
      zlibCompress(B15Data,B15DataCount*2,cmp256data,cmp256size);
      FilesLst.Items[idx]:=format('%d->%d',[B15DataCount*2,cmp256size]);
    end;
}

    with ImgData do begin
      dither8bit(bm64,16,prv64);
      zlibCompress(B8Data,B8DataCount,cmp64data,cmp64size,decmp64size);
      if INI_ScreenRatio<>0 then begin
        dither8bit(bm256,INI_ColorCount,prv256);
        zlibCompress(B8Data,B8DataCount,cmp256data,cmp256size,decmp256size);
        FilesLst.Items[idx]:=format('%d->%d',[decmp256size,cmp256size])+' '+FilesLst.Items[idx];
      end;
    end;

    FileInfo[idx].Offset:=wfs.Position;
    WriteData(idx,wfs);

    FreeThumb(idx);
  end;

  wfs.Position:=0;
  wfs.WriteBuffer(FileInfoCount,4); // count
  for idx:=0 to FileInfoCount-1 do begin
    wfs.WriteBuffer(FileInfo[idx].FilenameCRC,4); // crc
    wfs.WriteBuffer(FileInfo[idx].Offset,4); // ofs
  end;

  wfs.Free;

  if INI_ShowCompletionDialog=True then ShowMessage('Thumbnail file created.');
  Main.Close;
end;

procedure TMain.FormCreate(Sender: TObject);
begin
  Application.Title:='MoonShell Thumbnail maker';
  Main.Caption:=Application.Title;
end;

end.
