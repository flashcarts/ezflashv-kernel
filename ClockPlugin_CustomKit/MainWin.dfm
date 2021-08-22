object Main: TMain
  Left = -408
  Top = 410
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Main'
  ClientHeight = 312
  ClientWidth = 288
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  Position = poDefaultPosOnly
  Scaled = False
  OnCloseQuery = FormCloseQuery
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 12
  object ItemLst: TListBox
    Left = 8
    Top = 232
    Width = 273
    Height = 73
    ExtendedSelect = False
    Font.Charset = SHIFTJIS_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Terminal'
    Font.Style = []
    ItemHeight = 13
    ParentFont = False
    TabOrder = 0
    OnDblClick = ItemLstDblClick
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 8
    Width = 273
    Height = 217
    Caption = 'Preview'
    TabOrder = 1
    object PrvImg: TImage
      Left = 8
      Top = 16
      Width = 256
      Height = 192
    end
  end
  object MainMenu1: TMainMenu
    Left = 48
    Top = 48
    object MMF: TMenuItem
      Caption = 'File (&F)'
      object OpenMSPO1: TMenuItem
        Caption = 'Open MSP (&O) ...'
        OnClick = OpenMSPO1Click
      end
      object Saveas1: TMenuItem
        Caption = 'Save MSP as (&A) ...'
        OnClick = Saveas1Click
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object SavePreviewP1: TMenuItem
        Caption = 'Save Preview (&P) ...'
        OnClick = SavePreviewP1Click
      end
      object N2: TMenuItem
        Caption = '-'
      end
      object ExitX1: TMenuItem
        Caption = 'Exit (&X)'
        OnClick = ExitX1Click
      end
    end
    object MME: TMenuItem
      Caption = 'Edit (&E)'
      object MME_BGLoadClick: TMenuItem
        Caption = 'BG Load from file (&B) ...'
        OnClick = MME_BGLoadClickClick
      end
      object MME_BGFill: TMenuItem
        Caption = 'BG Fill select color (&B) ...'
        OnClick = MME_BGFillClick
      end
      object N3: TMenuItem
        Caption = '-'
      end
      object MME_Item0: TMenuItem
        Caption = 'Date (&D)'
        OnClick = MME_ItemClick
      end
      object MME_Item1: TMenuItem
        Caption = 'HourMinuts (&H)'
        GroupIndex = 1
        OnClick = MME_ItemClick
      end
      object MME_Item2: TMenuItem
        Caption = 'Second (&S)'
        GroupIndex = 2
        OnClick = MME_ItemClick
      end
      object MME_Item3: TMenuItem
        Caption = 'AMPM (&A)'
        GroupIndex = 3
        OnClick = MME_ItemClick
      end
      object MME_Item4: TMenuItem
        Caption = 'Temperature (&T)'
        GroupIndex = 4
        OnClick = MME_ItemClick
      end
    end
    object MMA: TMenuItem
      Caption = 'About (&A)'
      OnClick = MMAClick
    end
  end
  object OpenMSPDlg: TOpenDialog
    DefaultExt = 'msp'
    Filter = 'MoonShell Clock Plug-in (*.msp)|*.msp'
    Left = 120
    Top = 48
  end
  object SaveMSPDlg: TSaveDialog
    FileName = 'clk_customkit.msp'
    Filter = 'MoonShell Clock Plug-in (*.msp)|*.msp'
    Left = 160
    Top = 48
  end
  object SavePreviewDlg: TSavePictureDialog
    DefaultExt = 'bmp'
    FileName = 'clk_customkit.bmp'
    Filter = 'MS-Windows bitmap file (*.bmp)|*.bmp'
    Left = 192
    Top = 48
  end
  object OpenBGBMDlg: TOpenPictureDialog
    Filter = 'all support files (*.bmp;*.png)|*.bmp;*.png'
    Left = 80
    Top = 48
  end
end
