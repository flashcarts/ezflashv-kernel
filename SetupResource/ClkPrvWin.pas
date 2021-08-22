unit ClkPrvWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, StdCtrls, GLDPNG, _resfile, _PicTools;

type
  TClkPrv = class(TForm)
    PrvImg: TImage;
    GLDPNGMemo: TMemo;
    procedure FormCreate(Sender: TObject);
  private
    { Private êÈåæ }
  public
    { Public êÈåæ }
    procedure Load(PlugName:string);
  end;

var
  ClkPrv: TClkPrv;

implementation

{$R *.dfm}

procedure TClkPrv.FormCreate(Sender: TObject);
begin
  GLDPNGMemo.WordWrap:=True;
end;

procedure TClkPrv.Load(PlugName:string);
var
  srcfn,dstfn:string;
  resdata:Tresdata;
  wfs:TFileStream;
  png:TGLDPNG;
begin
  ClkPrv.Caption:='Clock plug-in preview ';

  MakeBlankImg(PrvImg,pf24bit);

  if PlugName='' then begin
    ClkPrv.Caption:=ClkPrv.Caption+'[Disabled]';
    PrvImg.Refresh;
    exit;
  end;

  ClkPrv.Caption:=ClkPrv.Caption+'['+PlugName+']';

  srcfn:='ClockPlugin\'+PlugName+'\'+PlugName+'.png';
  dstfn:='setup.$$$';

  resfile_GetData(srcfn,resdata);

  try
    wfs:=TFileStream.Create(dstfn,fmCreate);
    wfs.WriteBuffer(resdata.data[0],resdata.size);
    wfs.Free;
    except else begin
      exit;
    end;
  end;

  png:=TGLDPNG.Create;
  png.Image:=PrvImg.Picture.Bitmap;
  png.LoadFromFile(dstfn);
  png.Free;

  PrvImg.Refresh;

  DeleteFile(dstfn);
end;

end.
