object About: TAbout
  Left = -584
  Top = 447
  BorderStyle = bsDialog
  Caption = 'About...'
  ClientHeight = 71
  ClientWidth = 384
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
  Scaled = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 12
  object VerLbl: TLabel
    Left = 8
    Top = 16
    Width = 33
    Height = 12
    Caption = 'VerLbl'
  end
  object Label1: TLabel
    Left = 208
    Top = 16
    Width = 70
    Height = 12
    Caption = 'for MoonShell'
  end
  object BitBtn1: TBitBtn
    Left = 304
    Top = 8
    Width = 75
    Height = 25
    TabOrder = 0
    Kind = bkOK
  end
  object GLDPNGMemo: TMemo
    Left = 8
    Top = 40
    Width = 370
    Height = 25
    BorderStyle = bsNone
    Lines.Strings = (
      'GLDPNG (c) 1998-2001 Copyright Tarquin All Rights Reserved.'
      
        'ZLIB ver 1.1.3 (c) 1995-1996 Copyright Jean-loup Gailly and Mark' +
        ' Adler')
    ParentColor = True
    TabOrder = 1
    WordWrap = False
  end
end
