unit ROMInfoWin;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, _SplitML, ExtCtrls;

type
  TROMInfo = class(TForm)
    MsgMemo: TMemo;
    PosTimer: TTimer;
    procedure PosTimerTimer(Sender: TObject);
  private
    { Private �錾 }
  public
    { Public �錾 }
    procedure MsgMemoRefresh;
  end;

var
  ROMInfo: TROMInfo;

implementation

uses MainWin;

const CRLF:string=char($0d)+char($0a);

{$R *.dfm}

procedure TROMInfo.MsgMemoRefresh;
var
  msg:string;
begin
  msg:='';

  if Main.romMPCFChk.Checked then begin
    msg:=msg+'for GBAMP/AdMovie'+CRLF+CRLF;
    case GetMLType of
      MLType_EN: begin
        msg:=msg+'When other automatic execution BOOTROM exists, it overwrites.'+CRLF;
      end;
      MLType_JP: begin
        msg:=msg+'���̎����N��BOOTROM�����݂����Ƃ��͏㏑�����܂��B'+CRLF;
      end;
      else begin end;
    end;
    msg:=msg+CRLF;
  end;

  if Main.romSCCFChk.Checked or Main.romSCSDChk.Checked or Main.romSCMSChk.Checked then begin
    case GetMLType of
      MLType_EN: begin
      end;
      MLType_JP: begin
        msg:=msg+'for SuperCard'+CRLF+CRLF;
        msg:=msg+'�s�Ǖi�C����SuperCard�̂��߂̉����@������܂��B'+CRLF;
        msg:=msg+'�s������������Ƃ��̓}�j���A�����Q�Ƃ��Ă��������B'+CRLF;
      end;
      else begin end;
    end;
    msg:=msg+CRLF;
  end;

{
  if Main.romEZSDChk.Checked then begin
    msg:=msg+'for EZ-Flash4'+CRLF+CRLF;
    case GetMLType of
      MLType_EN: begin
        msg:=msg+'This cannot be written.'+CRLF;
        msg:=msg+'The resume and bookmark cannot be used.'+CRLF;
      end;
      MLType_JP: begin
        msg:=msg+'�������ݔ�Ή��A�_�v�^�ł��B'+CRLF;
        msg:=msg+'���W���[���ƃu�b�N�}�[�N�͎g���܂���B'+CRLF;
      end;
      else begin end;
    end;
    msg:=msg+CRLF;
  end;
}

  if Main.romEWSDChk.Checked then begin
    msg:=msg+'for Ewin2'+CRLF+CRLF;
    case GetMLType of
      MLType_EN: begin
        msg:=msg+'Requests the firmware version 1.90.'+CRLF;
      end;
      MLType_JP: begin
        msg:=msg+'�t�@�[���E�F�A�o�[�W����1.90�ȏ��v�����܂��B'+CRLF;
      end;
      else begin end;
    end;
    msg:=msg+CRLF;
  end;

  if Main.romNMMCChk.Checked then begin
    msg:=msg+'for MagicKey2-3'+CRLF+CRLF;
    case GetMLType of
      MLType_EN: begin
        msg:=msg+'Because the transfer rate is slow, DPG cannot be performed.'+CRLF;
      end;
      MLType_JP: begin
        msg:=msg+'�]�����x���x���̂ŁADPG�͍Đ��ł��܂���B'+CRLF;
      end;
      else begin end;
    end;
    msg:=msg+CRLF;
  end;

  if Main.romNJSDChk.Checked then begin
    msg:=msg+'for NinjaDS'+CRLF+CRLF;
    case GetMLType of
      MLType_EN: begin
        msg:=msg+'Because the transfer rate is slow, DPG cannot be performed.'+CRLF;
      end;
      MLType_JP: begin
        msg:=msg+'�]�����x���x���̂ŁADPG�͍Đ��ł��܂���B'+CRLF;
      end;
      else begin end;
    end;
    msg:=msg+CRLF;
  end;

  MsgMemo.Text:=msg;
end;

procedure TROMInfo.PosTimerTimer(Sender: TObject);
begin
  with Main do begin
    ROMInfo.Left:=Main.Left+Main.Width;
    ROMInfo.Top:=Main.Top;
  end;
end;

end.
