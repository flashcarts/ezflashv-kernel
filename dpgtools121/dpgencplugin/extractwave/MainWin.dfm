object Main: TMain
  Left = -558
  Top = 425
  BorderStyle = bsToolWindow
  Caption = 'Main'
  ClientHeight = 77
  ClientWidth = 334
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 12
  object prgbar: TProgressBar
    Left = 0
    Top = 0
    Width = 334
    Height = 15
    Align = alCustom
    TabOrder = 0
  end
  object StartupTimer: TTimer
    Enabled = False
    Interval = 1
    OnTimer = StartupTimerTimer
    Left = 8
    Top = 24
  end
end
