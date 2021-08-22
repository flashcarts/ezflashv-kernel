object SelDrv: TSelDrv
  Left = -559
  Top = 412
  BorderStyle = bsDialog
  Caption = 'SelDrv'
  ClientHeight = 184
  ClientWidth = 239
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 12
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 354
    Height = 12
    Caption = 'Select setup target drive.|'#12475#12483#12488#12450#12483#12503#23550#35937#12489#12521#12452#12502#12434#36984#25246#12375#12390#12367#12384#12373#12356#12290
  end
  object TargetDriveLst: TComboBox
    Left = 8
    Top = 32
    Width = 225
    Height = 21
    Style = csDropDownList
    Font.Charset = SHIFTJIS_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Terminal'
    Font.Style = []
    ItemHeight = 13
    ParentFont = False
    TabOrder = 0
  end
  object BitBtn1: TBitBtn
    Left = 79
    Top = 64
    Width = 73
    Height = 25
    TabOrder = 1
    Kind = bkOK
  end
  object BitBtn2: TBitBtn
    Left = 159
    Top = 64
    Width = 73
    Height = 25
    Caption = 'Cancel'
    TabOrder = 2
    Kind = bkCancel
  end
  object ExtractBtn: TButton
    Left = 8
    Top = 120
    Width = 217
    Height = 25
    Caption = 'extract all files to...|'#20840#12390#12398#20869#37096#12501#12449#12452#12523#12434#23637#38283#12377#12427
    TabOrder = 3
    OnClick = ExtractBtnClick
  end
  object ToolChk: TCheckBox
    Left = 8
    Top = 96
    Width = 225
    Height = 17
    Caption = 'tool for developers.|'#38283#30330#32773#21521#12369#12484#12540#12523
    TabOrder = 4
    OnClick = ToolChkClick
  end
  object CreateFPKBtn: TButton
    Left = 8
    Top = 152
    Width = 217
    Height = 25
    Caption = 'create dynamic font package files to...|'#36215#21205#20013#12395#22793#26356#12391#12365#12427#12501#12457#12531#12488#12434#20316#25104
    TabOrder = 5
    OnClick = CreateFPKBtnClick
  end
  object ExtractDlg: TSaveDialog
    FileName = '_BOOT_MP.NDS'
    Filter = 'variable|*.*'
    Title = 'Extract to... '#23637#38283#20808#12434#25351#23450#12375#12390#12367#12384#12373#12356#12290
    Left = 144
    Top = 120
  end
  object FontPackageDlg: TSaveDialog
    FileName = 'system.fpk'
    Filter = 'Dynamic font package files (*.fpk)|*.fpk'
    Title = 'Create to... '#20316#25104#20808#12434#25351#23450#12375#12390#12367#12384#12373#12356#12290
    Left = 184
    Top = 120
  end
end
