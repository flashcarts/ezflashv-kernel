unit MainWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs,_PicTools, ExtCtrls, StdCtrls, FileCtrl;

type
  TMain = class(TForm)
    Memo1: TMemo;
    AnkLabel: TLabel;
    cp932_SJISLabel: TLabel;
    PaintBox1: TPaintBox;
    UnicodeLabel: TLabel;
    FileListBox1: TFileListBox;
    cp1251_CyrillicLbl1: TLabel;
    cp1251_CyrillicLbl2: TLabel;
    cp874_ThaiLbl: TLabel;
    cp936_Lbl: TLabel;
    cp950_Lbl: TLabel;
    UnicodeAll7ptLbl: TLabel;
    UnicodeAll8ptLbl: TLabel;
    cp949_8Lbl: TLabel;
    cp949_9Lbl: TLabel;
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

const FontProHeight=12;

var
  CodePage:integer;
  l2utbl:array[$0000..$10000] of word;
  u2ltbl:array[$0000..$10000] of word;
  anktbl:array[$00..$100] of byte;

type
  TFontPro=record
    Offset:dword;
    Width:byte;
    BitStream:array of boolean;
    BitSize:integer;
  end;

var
  FontCount:integer;
  FontPro:array[$0000..$10000] of TFontPro;

procedure LoadUnicodeTable(tblfn:string);
var
  tblstrs:TStringList;
  cnt:integer;
  lc,uc:word;
  str:string;
  ustr,lstr:string;
  p:integer;
begin
  for cnt:=0 to $10000-1 do begin
    l2utbl[cnt]:=0;
    u2ltbl[cnt]:=0;
  end;

  tblstrs:=TStringList.Create;
  tblstrs.LoadFromFile(tblfn);

  for cnt:=0 to tblstrs.Count-1 do begin
    str:=tblstrs[cnt];
    if copy(str,1,2)='0x' then begin
      str:=copy(str,1,ansipos('#',str));

      p:=ansipos(char(9),str);

      lstr:=copy(str,1,p-1);
      lstr:=copy(lstr,3,4);
      lstr:=trim(lstr);

      ustr:=copy(str,p+1,6);
      ustr:=copy(ustr,3,4);
      ustr:=trim(ustr);

      if (ustr<>'') and (lstr<>'') then begin
        uc:=strtoint('$'+ustr);
        lc:=strtoint('$'+lstr);

        l2utbl[lc]:=uc;
        u2ltbl[uc]:=lc;

      end;
    end;
  end;

  tblstrs.Free;
end;

procedure LoadUnicodeList(lstfn:string);
var
  lststrs:TStringList;
  cnt:integer;
  uc:word;
  str:string;
begin
  for cnt:=0 to $10000-1 do begin
    l2utbl[cnt]:=0;
    u2ltbl[cnt]:=0;
  end;

  lststrs:=TStringList.Create;
  lststrs.LoadFromFile(lstfn);

  for cnt:=0 to lststrs.Count-1 do begin
    str:=lststrs[cnt];
    if ansipos('<control>',str)=0 then begin
      uc:=strtoint('$'+copy(str,1,4));
      u2ltbl[uc]:=uc;
    end;
  end;

  lststrs.Free;
end;

procedure LoadUnicode16LE(fn:string);
var
  rfs:TFileStream;
  cnt:integer;
  uc:word;
begin
  for cnt:=0 to $10000-1 do begin
    l2utbl[cnt]:=0;
    u2ltbl[cnt]:=0;
  end;

  rfs:=TFileStream.Create(fn,fmOpenRead);

  for cnt:=0 to (rfs.Size div 2)-1 do begin
    rfs.ReadBuffer(uc,2);
    u2ltbl[uc]:=uc;
  end;

  rfs.Free;

  for uc:=$20 to $80-1 do begin
    u2ltbl[uc]:=uc;
  end;
end;

procedure SetFontCount;
var
  cnt:integer;
  lastcnt:integer;
begin
  lastcnt:=0;

  for cnt:=0 to $10000-1 do begin
    if u2ltbl[cnt]<>0 then lastcnt:=cnt;
  end;

  FontCount:=lastcnt+1;
end;

procedure SaveUnicodeTable(l2ufn:string);
var
  wfs:TFileStream;
  cnt:integer;
  tmp:integer;
begin
  wfs:=TFileStream.Create(l2ufn,fmCreate);

  wfs.WriteBuffer(CodePage,2);

  case CodePage of
    100: begin
      tmp:=$ffff;
      wfs.WriteBuffer(tmp,2);
    end;
    110: begin
      tmp:=$ffff;
      wfs.WriteBuffer(tmp,2);
    end;
    else begin
      wfs.WriteBuffer(FontCount,2);
      for cnt:=0 to FontCount-1 do begin
        wfs.WriteBuffer(l2utbl[cnt],2);
      end;
    end;
  end;

  wfs.Free;
end;

procedure SaveAnkTable(ankfn:string);
var
  cnt:integer;
  SingleByteFlag:boolean;
  wfs:TFileStream;
begin
  for cnt:=$00 to $7f do begin
    anktbl[cnt]:=1;
  end;

  for cnt:=$80 to $ff do begin
    anktbl[cnt]:=0;
  end;

  SingleByteFlag:=False;

  case CodePage of
    0: SingleByteFlag:=True;
    874: SingleByteFlag:=True;
    932: begin
      anktbl[$80]:=1;
      anktbl[$a0]:=1;
      for cnt:=$a1 to $df do begin
        anktbl[cnt]:=1;
      end;
      anktbl[$fd]:=1;
      anktbl[$fe]:=1;
      anktbl[$ff]:=1;
    end;
    936: begin
      anktbl[$80]:=1;
      anktbl[$ff]:=1;
    end;
    949: begin
      anktbl[$80]:=1;
      anktbl[$ff]:=1;
    end;
    950: begin
      anktbl[$80]:=1;
      anktbl[$ff]:=1;
    end;
    else begin
      if (100<=CodePage) and (CodePage<=199) then begin
        for cnt:=$00 to $ff do begin
          anktbl[cnt]:=0;
        end;
        end else begin
        if (1250<=CodePage) and (CodePage<=1258) then begin
          SingleByteFlag:=True;
          end else begin
          ShowMessage('not support codepage='+inttostr(CodePage));
        end;
      end;
    end;
  end;

  if SingleByteFlag=True then begin
    for cnt:=$80 to $ff do begin
      anktbl[cnt]:=1;
    end;
  end;

  wfs:=TFileStream.Create(ankfn,fmCreate);

  for cnt:=0 to $100-1 do begin
    wfs.WriteBuffer(anktbl[cnt],1);
  end;

  wfs.Free;
end;

function GetWidth(uc,lc:word;var tmpbm:TBitmap):integer;
var
  x,y:integer;
begin
  if lc=$00 then begin
    Result:=trunc(FontProHeight*0.5);
    exit;
  end;

  if lc=$20 then begin
    Result:=trunc(FontProHeight*0.5);
    exit;
  end;

  for x:=tmpbm.Width-1 downto 0 do begin
    for y:=0 to tmpbm.Height-1 do begin
      if tmpbm.Canvas.Pixels[x,y]<>0 then begin
        Result:=x+2;

        case CodePage of
          932: begin
            if lc=$8140 then Result:=trunc(FontProHeight*0.8);
            if lc=$4181 then Result:=Result*2; // [、]
            if lc=$4281 then Result:=Result*2; // [。]
            if lc=$4381 then Result:=Result*2; // [，]
            if lc=$4481 then Result:=Result*2; // [．]
          end;
        end;

        exit;
      end;
    end;
  end;


  Result:=0;
end;

procedure SetFontPro(ccnt:integer;var tmpbm:TBitmap);
var
  x,y:integer;
  bcnt:integer;
begin
  if ccnt=integer('\') then begin
    tmpbm.Canvas.Pixels[1,1]:=0;
  end;

  with FontPro[ccnt] do begin
    Offset:=0;
    Width:=tmpbm.Width;

    BitSize:=Width*FontProHeight;
    setlength(BitStream,BitSize);

    bcnt:=0;
    for y:=0 to FontProHeight-1 do begin
      for x:=0 to Width-1 do begin
        if tmpbm.Canvas.Pixels[x,y]<>0 then begin
          BitStream[bcnt]:=True;
          end else begin
          BitStream[bcnt]:=False;
        end;
        inc(bcnt);
      end;
    end;
  end;

end;

procedure WriteFontPro(binfn:string);
var
  headsize:integer;
  wfs:TFileStream;
  ccnt:integer;
  procedure wu8(b:integer);
  begin
    if (b<$00) or ($ff<b) then begin
      showmessage('wu8 over.');
    end;
    wfs.WriteBuffer(b,1);
  end;
  procedure wu16(w:integer);
  begin
    if (w<$0000) or ($ffff<w) then begin
      showmessage('wu16 over.');
    end;
    wfs.WriteBuffer(w,2);
  end;
  procedure wu32(dw:dword);
  begin
    wfs.WriteBuffer(dw,4);
  end;
  procedure wbs;
  var
    bcnt:integer;
    bit:byte;
    bitimage:byte;
    bitcount:integer;
  begin
    if FontPro[ccnt].BitSize=0 then exit;

    bitimage:=0;
    bitcount:=0;

    with FontPro[ccnt] do begin
      for bcnt:=0 to BitSize-1 do begin
        if BitStream[bcnt]=True then begin
          bit:=1;
          end else begin
          bit:=0;
        end;
        bitimage:=bitimage or (bit shl bitcount);
        inc(bitcount);
        if bitcount=8 then begin
          wu8(bitimage);
          bitimage:=0;
          bitcount:=0;
        end;
      end;
    end;

    if bitcount<>0 then wu8(bitimage);
  end;
begin
  wfs:=TFileStream.Create(binfn,fmCreate);

  wfs.Position:=0;

  wu16(FontProHeight);
  wu16(FontCount);

  headsize:=wfs.Size;

  wfs.Size:=headsize+(FontCount*4);
  wfs.Position:=wfs.Size;

  for ccnt:=$0 to FontCount-1 do begin
    with FontPro[ccnt] do begin
      if BitSize=0 then begin
        Offset:=0;
        end else begin
        Offset:=wfs.Position-headsize;
        wu8(Width);
        wbs;
      end;
    end;
  end;

  wfs.Position:=headsize;

  for ccnt:=$0 to FontCount-1 do begin
    with FontPro[ccnt] do begin
      if Offset<>0 then begin
        wu32(Offset);
        end else begin
        wu32(FontPro[$20].Offset);
      end;
    end;
  end;

  if (wfs.Size and $3)<>0 then begin
    wfs.Position:=wfs.Size;
    wu8(0);
  end;
  if (wfs.Size and $3)<>0 then begin
    wfs.Position:=wfs.Size;
    wu8(0);
  end;
  if (wfs.Size and $3)<>0 then begin
    wfs.Position:=wfs.Size;
    wu8(0);
  end;

  wfs.Free;
end;

var
  fsuni:TFileStream;

procedure CreateUnicodeFont;
var
  lcnt:integer;
  ucnt:integer;

  tmpbm:TBitmap;
  w,h:integer;
  t:integer;
  isAnk:boolean;

  prevx:integer;
  prevy:integer;
  procedure DrawFont;
  var
    astr:string;
    lc:word;
    unistr:WideString;
  begin
    if isAnk=True then begin
      t:=-1;
      if ucnt=integer('_') then t:=-2;
      tmpbm.Canvas.Font:=Main.AnkLabel.Font;
      tmpbm.Canvas.Font.Color:=$ffffff;
      astr:=char(ucnt);
      TextOutA(tmpbm.Canvas.Handle,0,t,pAnsiChar(@astr[1]),length(astr));
      end else begin

      lc:=u2ltbl[ucnt];
      if lc<$100 then begin
        astr:=char(lc);
        end else begin
        astr:=char(lc shr 8)+char(lc);
      end;

      unistr:=WideChar(ucnt);

      case CodePage of
        0: begin
        end;
        100: begin
          if (ucnt<=$007f) then begin
            t:=-2;
            tmpbm.Canvas.Font:=Main.UnicodeAll8ptLbl.Font;
            end else begin
            if (ucnt=$00e5) or (ucnt=$00e4) or (ucnt=$00f6) then begin
              t:=-2;
              tmpbm.Canvas.Font:=Main.UnicodeAll8ptLbl.Font;
              end else begin
              if (ucnt=$00c5) or (ucnt=$00c4) or (ucnt=$00d6) then begin
                t:=0;
                tmpbm.Canvas.Font:=Main.UnicodeAll7ptLbl.Font;
                end else begin
                t:=0;
                tmpbm.Canvas.Font:=Main.UnicodeAll7ptLbl.Font;
              end;
            end;
          end;
          tmpbm.Canvas.Font.Color:=$ffffff;
          TextOutW(tmpbm.Canvas.Handle,0,t,pWideChar(@unistr[1]),length(unistr));
        end;
        110: begin
          t:=-2;
          tmpbm.Canvas.Font:=Main.UnicodeAll8ptLbl.Font;
          tmpbm.Canvas.Font.Color:=$ffffff;
          TextOutW(tmpbm.Canvas.Handle,0,t,pWideChar(@unistr[1]),length(unistr));
        end;
        874: begin
          t:=0;
          tmpbm.Canvas.Font:=Main.cp874_thailbl.Font;
          tmpbm.Canvas.Font.Color:=$ffffff;
          TextOutA(tmpbm.Canvas.Handle,0,t,pAnsiChar(@astr[1]),length(astr));
        end;
        932: begin
          t:=0;
          tmpbm.Canvas.Font:=Main.cp932_SJISLabel.Font;
          tmpbm.Canvas.Font.Color:=$ffffff;
          TextOutA(tmpbm.Canvas.Handle,0,t,pAnsiChar(@astr[1]),length(astr));
        end;
        936: begin
          t:=-1;
          tmpbm.Canvas.Font:=Main.cp936_Lbl.Font;
          tmpbm.Canvas.Font.Color:=$ffffff;
          TextOutW(tmpbm.Canvas.Handle,0,t,pWideChar(@unistr[1]),length(unistr));
        end;
        949: begin
          t:=-3;
          tmpbm.Canvas.Font:=Main.cp949_9lbl.Font;
          tmpbm.Canvas.Font.Color:=$ffffff;
          TextOutW(tmpbm.Canvas.Handle,0,t,pWideChar(@unistr[1]),length(unistr));
        end;
        950: begin
          t:=0;
          tmpbm.Canvas.Font:=Main.cp950_Lbl.Font;
          tmpbm.Canvas.Font.Color:=$ffffff;
          TextOutW(tmpbm.Canvas.Handle,0,t,pWideChar(@unistr[1]),length(unistr));
        end;
        else begin
          t:=0;
          tmpbm.Canvas.Font:=Main.UnicodeLabel.Font;
          tmpbm.Canvas.Font.Color:=$ffffff;
          TextOutW(tmpbm.Canvas.Handle,0,t,pWideChar(@unistr[1]),length(unistr));
        end;
      end;
    end;
  end;
begin
  prevx:=0;
  prevy:=0;

  for ucnt:=$0000 to $10000-1 do begin
    if (ucnt and $ff)=0 then Main.Caption:=inttohex(ucnt,4);

    lcnt:=u2ltbl[ucnt];
    if lcnt<>0 then begin

      w:=32;
      h:=FontProHeight;
      tmpbm:=TBitmap.Create;
      MakeBlankBM(tmpbm,w,h,pf24bit);

      if (100<=CodePage) and (CodePage<=199) then begin
        isAnk:=False;
        end else begin
        if ucnt<$100 then begin
          isAnk:=True;
          end else begin
          isAnk:=False;
        end;
      end;

      DrawFont;

      w:=GetWidth(ucnt,lcnt,tmpbm);
      if w<>0 then begin
        MakeBlankBM(tmpbm,w,h,pf24bit);
        DrawFont;
        SetFontPro(ucnt,tmpbm);

//        fsuni.WriteBuffer(ucnt,2);
      end;

      if $0000<=ucnt then begin
        BitBlt(Main.PaintBox1.Canvas.Handle,prevx,prevy,32,32,tmpbm.Canvas.Handle,0,0,SRCCOPY);
        inc(prevx,w);
        if Main.PaintBox1.Width<prevx then begin
          prevx:=0;
          inc(prevy,h);
        end;
      end;

//      Main.memo1.Lines.Add(format('%4x',[ucnt]));

      tmpbm.Free;
    end;

  end;
end;

procedure TMain.FormCreate(Sender: TObject);
var
  filecnt:integer;
  tblfn:string;
  cpstr:string;
  pathname:string;
  cnt,ucnt:integer;
begin
  with PaintBox1 do begin
    Width:=Main.ClientWidth-Left;
    Height:=Main.ClientHeight-Top;
  end;

  Show;

  FileListBox1.Mask:='cp949.txt';
  FileListBox1.Refresh;

  for filecnt:=0 to FileListBox1.Items.Count-1 do begin
    for cnt:=0 to $10000-1 do begin
      l2utbl[cnt]:=$0000;
      u2ltbl[cnt]:=$0000;
    end;

    tblfn:=FileListBox1.Items[filecnt];

    cpstr:=copy(changefileext(tblfn,''),3,255);
    CodePage:=strtoint(cpstr);

    pathname:=ChangeFileExt(tblfn,'')+'\';
    CreateDir(pathname);

    LoadUnicodeTable(tblfn);
    SetFontCount;
    SaveUnicodeTable(pathname+'system.l2u');
    SaveAnkTable(pathname+'system.ank');

    Main.memo1.Lines.Add(format('%d %s FontCount=%d',[CodePage,tblfn,FontCount]));

    for ucnt:=$0 to $10000-1 do begin
      FontPro[ucnt].BitSize:=0;
    end;

    CreateUnicodeFont;
    WriteFontPro(pathname+'system.fon');
  end;

{
  begin
    fsuni:=TFileStream.Create('a.txt',fmCreate);

    for cnt:=0 to $10000-1 do begin
      l2utbl[cnt]:=$0000;
      u2ltbl[cnt]:=$0000;
    end;

    tblfn:='CP100';

    cpstr:=copy(changefileext(tblfn,''),3,255);
    CodePage:=strtoint(cpstr);

    pathname:=ChangeFileExt(tblfn,'')+'\';
    CreateDir(pathname);

//    LoadUnicodeList('Unicode_Alphabet.txt');
    LoadUnicode16LE('utf16le_WindowsCharSet.txt');

    SetFontCount;
    SaveUnicodeTable(pathname+'system.l2u');
    SaveAnkTable(pathname+'system.ank');

    Main.memo1.Lines.Add(format('%d %s FontCount=%d',[CodePage,tblfn,FontCount]));

    for ucnt:=$0 to $10000-1 do begin
      FontPro[ucnt].BitSize:=0;
    end;

    CreateUnicodeFont;
    WriteFontPro(pathname+'system.fon');

    fsuni.Free;
  end;
}

{
  begin
    fsuni:=TFileStream.Create('a.txt',fmCreate);

    for cnt:=0 to $10000-1 do begin
      l2utbl[cnt]:=$0000;
      u2ltbl[cnt]:=$0000;
    end;

    tblfn:='CP110';

    cpstr:=copy(changefileext(tblfn,''),3,255);
    CodePage:=strtoint(cpstr);

    pathname:=ChangeFileExt(tblfn,'')+'\';
    CreateDir(pathname);

//    LoadUnicodeList('Unicode_Alphabet.txt');
    LoadUnicode16LE('utf16le_WindowsCharSet.txt');

    SetFontCount;
    SaveUnicodeTable(pathname+'system.l2u');
    SaveAnkTable(pathname+'system.ank');

    Main.memo1.Lines.Add(format('%d %s FontCount=%d',[CodePage,tblfn,FontCount]));

    for ucnt:=$0 to $10000-1 do begin
      FontPro[ucnt].BitSize:=0;
    end;

    CreateUnicodeFont;
    WriteFontPro(pathname+'system.fon');

    fsuni.Free;
  end;
}

end;


end.
