object Main: TMain
  Left = -599
  Top = 245
  Width = 1834
  Height = 763
  Caption = 'Main'
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'MS UI Gothic'
  Font.Style = []
  OldCreateOrder = False
  Scaled = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 12
  object AnkLabel: TLabel
    Left = 203
    Top = 16
    Width = 45
    Height = 13
    Caption = 'AnkLabel'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Microsoft Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object cp932_SJISLabel: TLabel
    Left = 200
    Top = 32
    Width = 85
    Height = 12
    Caption = 'cp932_SJISLabel'
    Font.Charset = SHIFTJIS_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'MS UI Gothic'
    Font.Style = []
    ParentFont = False
  end
  object PaintBox1: TPaintBox
    Left = 0
    Top = 248
    Width = 529
    Height = 32
    Font.Charset = SHIFTJIS_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'MS UI Gothic'
    Font.Style = []
    ParentFont = False
  end
  object UnicodeLabel: TLabel
    Left = 192
    Top = 224
    Width = 68
    Height = 12
    Caption = 'UnicodeLabel'
    Font.Charset = ANSI_CHARSET
    Font.Color = clBlack
    Font.Height = -12
    Font.Name = 'MS UI Gothic'
    Font.Style = []
    ParentFont = False
  end
  object cp1251_CyrillicLbl1: TLabel
    Left = 200
    Top = 48
    Width = 133
    Height = 15
    Caption = 'cp1251_CyrillicLbl1'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
  end
  object cp1251_CyrillicLbl2: TLabel
    Left = 200
    Top = 64
    Width = 91
    Height = 13
    Caption = 'cp1251_CyrillicLbl2'
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Microsoft Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object cp874_ThaiLbl: TLabel
    Left = 200
    Top = 88
    Width = 71
    Height = 13
    Caption = 'cp874_ThaiLbl'
    Font.Charset = THAI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Microsoft Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object cp936_Lbl: TLabel
    Left = 200
    Top = 104
    Width = 50
    Height = 13
    Caption = 'cp936_Lbl'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Microsoft Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object cp950_Lbl: TLabel
    Left = 200
    Top = 120
    Width = 54
    Height = 13
    Caption = 'cp950_Lbl'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS UI Gothic'
    Font.Style = []
    ParentFont = False
  end
  object UnicodeAll7ptLbl: TLabel
    Left = 272
    Top = 152
    Width = 69
    Height = 12
    Caption = 'UnicodeAll7ptLbl'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Arial Unicode MS'
    Font.Style = []
    ParentFont = False
  end
  object UnicodeAll8ptLbl: TLabel
    Left = 272
    Top = 168
    Width = 80
    Height = 15
    Caption = 'UnicodeAll8ptLbl'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Arial Unicode MS'
    Font.Style = []
    ParentFont = False
  end
  object cp949_8Lbl: TLabel
    Left = 200
    Top = 136
    Width = 56
    Height = 15
    Caption = 'cp949_8Lbl'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Arial Unicode MS'
    Font.Style = []
    ParentFont = False
  end
  object cp949_9Lbl: TLabel
    Left = 200
    Top = 152
    Width = 64
    Height = 16
    Caption = 'cp949_9Lbl'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Arial Unicode MS'
    Font.Style = []
    ParentFont = False
  end
  object Memo1: TMemo
    Left = 0
    Top = 8
    Width = 185
    Height = 233
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'MS UI Gothic'
    Font.Style = []
    Lines.Strings = (
      'Memo1')
    ParentFont = False
    TabOrder = 0
  end
  object FileListBox1: TFileListBox
    Left = 88
    Top = 136
    Width = 89
    Height = 97
    ItemHeight = 12
    Mask = 'cp*.txt'
    TabOrder = 1
    Visible = False
  end
end
