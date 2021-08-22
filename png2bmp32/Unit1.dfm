object Main: TMain
  Left = -590
  Top = 369
  BorderStyle = bsDialog
  Caption = 'png to bmp 32bit converter'
  ClientHeight = 64
  ClientWidth = 369
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  OldCreateOrder = False
  Position = poDefaultPosOnly
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 12
  object cimg: TImage
    Left = 0
    Top = 0
    Width = 64
    Height = 64
  end
  object aimg: TImage
    Left = 72
    Top = 0
    Width = 64
    Height = 64
  end
  object GLDPNGMemo: TMemo
    Left = 8
    Top = 8
    Width = 353
    Height = 49
    BorderStyle = bsNone
    Lines.Strings = (
      'GLDPNG (c) 1998-2001 Copyright Tarquin All Rights Reserved.'
      
        'ZLIB ver 1.1.3 (c) 1995-1996 Copyright Jean-loup Gailly and Mark' +
        ' '
      'Adler')
    ParentColor = True
    TabOrder = 0
  end
  object Timer1: TTimer
    Interval = 1
    Left = 16
    Top = 16
  end
  object picOpenDlg: TOpenDialog
    Filter = 'Portable Network Graphics files (*.png)|*.png'
    Left = 48
    Top = 16
  end
end
