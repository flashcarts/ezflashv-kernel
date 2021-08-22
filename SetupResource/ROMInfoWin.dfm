object ROMInfo: TROMInfo
  Left = -555
  Top = 396
  BorderStyle = bsNone
  Caption = 'ROMInfo'
  ClientHeight = 304
  ClientWidth = 349
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 12
  object MsgMemo: TMemo
    Left = 0
    Top = 0
    Width = 349
    Height = 304
    Align = alClient
    Lines.Strings = (
      'MsgMemo')
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 0
  end
  object PosTimer: TTimer
    Interval = 10
    OnTimer = PosTimerTimer
    Left = 8
    Top = 8
  end
end
