object ClkPrv: TClkPrv
  Left = -539
  Top = 523
  BorderStyle = bsToolWindow
  Caption = 'ClkPrv'
  ClientHeight = 255
  ClientWidth = 256
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  OldCreateOrder = False
  Position = poDefaultPosOnly
  Scaled = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 12
  object PrvImg: TImage
    Left = 0
    Top = 0
    Width = 256
    Height = 192
  end
  object GLDPNGMemo: TMemo
    Left = 8
    Top = 200
    Width = 241
    Height = 49
    BorderStyle = bsNone
    Enabled = False
    Lines.Strings = (
      'GLDPNG (c) 1998-2001 Copyright Tarquin All Rights Reserved.'
      
        'ZLIB ver 1.1.3 (c) 1995-1996 Copyright Jean-loup Gailly and Mark' +
        ' Adler')
    ParentColor = True
    TabOrder = 0
    WordWrap = False
  end
end
