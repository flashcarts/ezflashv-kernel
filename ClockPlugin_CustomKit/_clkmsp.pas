unit _clkmsp;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls, StdCtrls, ExtCtrls, Buttons, INIFiles,_PicTools;

type
  TItemBG=record
    bm:TBitmap;
  end;

type
  TItemPart=record
    Name:string;
    FontStart,FontCount:integer;
    Show:boolean;
    sFormat:string;
    TestStr:string;
    OfsX,OfsY:integer;
    TransFlag:boolean;
    bm:TBitmap;
  end;

const ItemPartCount=5;

var
  MSPSize:integer;
  MSPData:array of byte;

var
  ItemBG:TItemBG;
  ItemParts:array[0..ItemPartCount-1] of TItemPart;

procedure clkmsp_Init;

procedure clkmsp_Load(fn:string);
procedure clkmsp_Save(fn:string);

implementation

const CRLF:string=char($0d)+char($0a);

function Bool2Int(b:boolean):integer;
begin
  if b=False then begin
    Result:=0;
    end else begin
    Result:=1;
  end;
end;

function Int2Bool(i:integer):boolean;
begin
  if i=0 then begin
    Result:=False;
    end else begin
    Result:=True;
  end;
end;

procedure clkmsp_Init;
var
  idx:integer;
begin
  with ItemBG do begin
    bm:=TBitmap.Create;
    MakeBlankBM(bm,256,192,pf24bit);
  end;

  with ItemParts[0] do begin
    Name:='Date';
    FontStart:=integer(',');
    FontCount:=14;
    sFormat:='unuse';
    TestStr:='1234/56/78';
  end;
  with ItemParts[1] do begin
    Name:='HourMinuts';
    FontStart:=integer('0');
    FontCount:=11;
    sFormat:='';
    TestStr:='90:12';
  end;
  with ItemParts[2] do begin
    Name:='Second';
    FontStart:=integer('0');
    FontCount:=11;
    sFormat:='';
    TestStr:=':34';
  end;
  with ItemParts[3] do begin
    Name:='AMPM';
    FontStart:=integer('0');
    FontCount:=2;
    sFormat:='unuse';
    TestStr:='1';
  end;
  with ItemParts[4] do begin
    Name:='Temperature';
    FontStart:=integer('.');
    FontCount:=13;
    sFormat:='';
    TestStr:='12.34:';
  end;

  for idx:=0 to ItemPartCount-1 do begin
    with ItemParts[idx] do begin
      bm:=TBitmap.Create;
      MakeBlankBM(bm,0,0,pf24bit);
    end;
  end;
end;

procedure LoadINI(fn:string);
var
  fini:TINIFile;
  Section:string;
  idx:integer;
begin
  fini:=TINIFile.Create(fn);

  Section:='ClockPlugin';

  with ItemBG do begin
  end;

  for idx:=0 to ItemPartCount-1 do begin
    with ItemParts[idx] do begin
      Show:=Int2Bool(fini.ReadInteger(Section,Name+'Show',Bool2Int(Show)));
      if sFormat<>'unuse' then sFormat:=fini.ReadString(Section,Name+'Format',sFormat);
      OfsX:=fini.ReadInteger(Section,Name+'OfsX',OfsX);
      OfsY:=fini.ReadInteger(Section,Name+'OfsY',OfsY);
    end;
  end;

  fini.Free;
end;

procedure SaveINI(fn:string);
var
  fini:TINIFile;
  Section:string;
  idx:integer;
begin
  fini:=TINIFile.Create(fn);

  Section:='ClockPlugin';

  with ItemBG do begin
  end;

  for idx:=0 to ItemPartCount-1 do begin
    with ItemParts[idx] do begin
      fini.WriteInteger(Section,Name+'Show',Bool2Int(Show));
      if sFormat<>'unuse' then fini.WriteString(Section,Name+'Format',sFormat);
      fini.WriteInteger(Section,Name+'OfsX',OfsX);
      fini.WriteInteger(Section,Name+'OfsY',OfsY);
    end;
  end;

  fini.Free;
end;

procedure LoadBIN(fn:string);
var
  rfs:TFileStream;
  idxs:array[0..6-1] of dword;
  function RGB15to24(w:word):dword;
  var
    r,g,b:byte;
  begin
    if w=$0000 then begin
      Result:=$00ff00;
      end else begin
      b:=(w shr 10) and $1f;
      g:=(w shr 5) and $1f;
      r:=(w shr 0) and $1f;
      Result:=RGB(r shl 3,g shl 3,b shl 3);
    end;
  end;
  procedure LoadBIN_B15(var ItemPart:TItemPart;ofs:dword);
  var
    w,h,t:word;
    data:array of word;
    x,y:integer;
  begin
    MakeBlankBM(ItemPart.bm,0,0,pf24bit);

    if ofs=0 then exit;

    rfs.Position:=ofs;

    rfs.ReadBuffer(w,2);
    rfs.ReadBuffer(h,2);

    if (w=0) or (h=0) then exit;

    rfs.ReadBuffer(t,2);

    ItemPart.TransFlag:=Int2Bool(t);

    setlength(data,w*h);
    rfs.ReadBuffer(data[0],w*h*2);

    MakeBlankBM(ItemPart.bm,w,h,pf24bit);

    for y:=0 to h-1 do begin
      for x:=0 to w-1 do begin
        ItemPart.bm.Canvas.Pixels[x,y]:=RGB15to24(data[x+(y*w)]);
      end;
    end;
  end;
  procedure LoadBIN_BGB15(var ItemBG:TItemBG;ofs:dword);
  var
    w,h,t:word;
    data:array of word;
    x,y:integer;
  begin
    MakeBlankBM(ItemBG.bm,256,192,pf24bit);
    ItemBG.bm.Canvas.Brush.Color:=$000000;
    ItemBG.bm.Canvas.FillRect(Rect(0,0,256,192));

    if ofs=0 then exit;

    rfs.Position:=ofs;

    rfs.ReadBuffer(w,2);
    rfs.ReadBuffer(h,2);

    if (w=0) or (h=0) then exit;
    if (w<>256) or (h<>192) then exit;

    rfs.ReadBuffer(t,2);

    setlength(data,w*h);
    rfs.ReadBuffer(data[0],w*h*2);

    MakeBlankBM(ItemBG.bm,256,192,pf24bit);

    for y:=0 to 192-1 do begin
      for x:=0 to 256-1 do begin
        ItemBG.bm.Canvas.Pixels[x,y]:=RGB15to24(data[x+(y*256)]);
      end;
    end;
  end;
begin
  rfs:=TFileStream.Create(fn,fmOpenRead);

  rfs.ReadBuffer(idxs[0],6*4);

  LoadBIN_B15(ItemParts[3],idxs[0]);
  LoadBIN_BGB15(ItemBG,idxs[1]);
  LoadBIN_B15(ItemParts[0],idxs[2]);
  LoadBIN_B15(ItemParts[1],idxs[3]);
  LoadBIN_B15(ItemParts[2],idxs[4]);
  LoadBIN_B15(ItemParts[4],idxs[5]);

  rfs.Free;
end;

procedure SaveBIN(fn:string);
var
  wfs:TFileStream;
  idxs:array[0..6-1] of dword;
  idx:integer;
  function RGB24to15(dw:dword):word;
  var
    r,g,b:byte;
  begin
    if dw=$00ff00 then begin
      Result:=$0000;
      end else begin
      b:=(dw shr 16) and $ff;
      b:=b shr 3;
      g:=(dw shr 8) and $ff;
      g:=g shr 3;
      r:=(dw shr 0) and $ff;
      r:=r shr 3;
      Result:=(b shl 10)+(g shl 5)+(r shl 0)+(1 shl 15);
    end;
  end;
  procedure SaveBIN_B15(var ItemPart:TItemPart;var ofs:dword);
  var
    w,h,t:word;
    c:word;
    x,y:integer;
  begin
    ofs:=0;

    w:=ItemPart.bm.Width;
    h:=ItemPart.bm.Height;
    t:=Bool2Int(ItemPart.TransFlag);

    if (w=0) or (h=0) then exit;

    ofs:=wfs.Position;
    wfs.WriteBuffer(w,2);
    wfs.WriteBuffer(h,2);
    wfs.WriteBuffer(t,2);

    for y:=0 to h-1 do begin
      for x:=0 to w-1 do begin
        c:=RGB24to15(ItemPart.bm.Canvas.Pixels[x,y]);
        wfs.WriteBuffer(c,2);
      end;
    end;
  end;
  procedure SaveBIN_BGB15(var ItemBG:TItemBG;var ofs:dword);
  var
    w,h,t:word;
    c:word;
    x,y:integer;
  begin
    ofs:=0;

    w:=ItemBG.bm.Width;
    h:=ItemBG.bm.Height;
    t:=Bool2Int(False);

    if (w=0) or (h=0) then exit;
    if (w<>256) or (h<>192) then exit;

    ofs:=wfs.Position;
    wfs.WriteBuffer(w,2);
    wfs.WriteBuffer(h,2);
    wfs.WriteBuffer(t,2);

    for y:=0 to h-1 do begin
      for x:=0 to w-1 do begin
        c:=RGB24to15(ItemBG.bm.Canvas.Pixels[x,y]);
        wfs.WriteBuffer(c,2);
      end;
    end;
  end;
begin
  wfs:=TFileStream.Create(fn,fmCreate);

  for idx:=0 to 6-1 do begin
    idxs[idx]:=0;
  end;
  wfs.WriteBuffer(idxs[0],6*4);

  SaveBIN_B15(ItemParts[3],idxs[0]);
  SaveBIN_BGB15(ItemBG,idxs[1]);
  SaveBIN_B15(ItemParts[0],idxs[2]);
  SaveBIN_B15(ItemParts[1],idxs[3]);
  SaveBIN_B15(ItemParts[2],idxs[4]);
  SaveBIN_B15(ItemParts[4],idxs[5]);

  wfs.Position:=0;
  wfs.WriteBuffer(idxs[0],6*4);

  wfs.Free;
end;

function CheckMSP(fn:string):boolean;
var
  rfs:TFileStream;
  MSPSize:integer;
  MSPData:array of dword;
  CRC:dword;
  idx:integer;
begin
  rfs:=TFileStream.Create(fn,fmOpenRead);
  MSPSize:=rfs.Size div 4;
  setlength(MSPData,MSPSize);
  rfs.ReadBuffer(MSPData[0],MSPSize*4);
  rfs.Free;

  Result:=False;

  if MSPSize<(128 div 4) then exit;

  CRC:=0;
  for idx:=0 to (128 div 4)-1 do begin
    CRC:=CRC xor MSPData[idx];
  end;

  if CRC<>$21A33A8D then exit; // not "Custom ClockPlugin Moonlight May 22 2006 16:28:14 GMT+09:00"

  Result:=True;
end;

procedure LoadMSP(fn:string);
var
  rfs:TFileStream;
begin
  rfs:=TFileStream.Create(fn,fmOpenRead);
  MSPSize:=rfs.Size;
  setlength(MSPData,MSPSize);
  rfs.ReadBuffer(MSPData[0],MSPSize);
  rfs.Free;
end;

procedure SaveMSP(fn:string);
var
  wfs:TFileStream;
begin
  wfs:=TFileStream.Create(fn,fmCreate);
  wfs.WriteBuffer(MSPData[0],MSPSize);
  wfs.Free;
end;

procedure clkmsp_Load(fn:string);
var
  mspfn,inifn,binfn:string;
begin
  mspfn:=ChangeFileExt(fn,'.msp');
  inifn:=ChangeFileExt(fn,'.ini');
  binfn:=ChangeFileExt(fn,'.bin');

  if (FileExists(mspfn)=False) or (FileExists(inifn)=False) or (FileExists(binfn)=False) then begin
    ShowMessage('Can not found files ['+mspfn+'/.ini/.bin].');
    exit;
  end;

  if CheckMSP(mspfn)=False then begin
    ShowMessage('This plug-in is not compatible with the custom plug-in.'+CRLF+'このプラグインはカスタムプラグインと互換性がありません。');
    exit;
  end;

  LoadMSP(mspfn);
  LoadINI(inifn);
  LoadBIN(binfn);
end;

procedure clkmsp_Save(fn:string);
var
  mspfn,inifn,binfn:string;
begin
  mspfn:=ChangeFileExt(fn,'.msp');
  inifn:=ChangeFileExt(fn,'.ini');
  binfn:=ChangeFileExt(fn,'.bin');

  SaveMSP(mspfn);
  SaveINI(inifn);
  SaveBIN(binfn);
end;

end.
