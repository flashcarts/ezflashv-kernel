object EditItem: TEditItem
  Left = -434
  Top = 329
  BorderStyle = bsToolWindow
  Caption = 'EditItem'
  ClientHeight = 416
  ClientWidth = 360
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
  object PosImg: TImage
    Left = 8
    Top = 32
    Width = 256
    Height = 192
    OnMouseDown = PosImgMouseDown
    OnMouseMove = PosImgMouseMove
    OnMouseUp = PosImgMouseUp
  end
  object Label4: TLabel
    Left = 8
    Top = 296
    Width = 69
    Height = 12
    Caption = 'Format string'
  end
  object PrvImg: TImage
    Left = 272
    Top = 32
    Width = 81
    Height = 377
  end
  object Label5: TLabel
    Left = 272
    Top = 9
    Width = 40
    Height = 12
    Caption = 'Preview'
  end
  object PosPickerImg: TImage
    Left = 16
    Top = 40
    Width = 16
    Height = 16
    AutoSize = True
    Picture.Data = {
      07544269746D617036030000424D360300000000000036000000280000001000
      000010000000010018000000000000030000120B0000120B0000000000000000
      00000B0B0B010101000000000000000000000000000000000000000000000000
      0000000000000000000000000000000000001918190B0B0B0101010000000000
      0000000000000000000000000000000000000000000000000000000000000000
      00002828271819190B0B0B010101000000000000000000000000000000000000
      00000000000000000000000000000000000039393A2828281818180B0B0B0101
      0100000000000000000000000000000000000000000000000000000000000000
      00004C4D4D3A39392828281818180B0B0B000001000000000000000000000000
      0000000000000000000000000000000000006061614C4C4D393A392828281918
      180B0B0C00000100000000000000000000000000000000000000000000000000
      00007576766161614D4D4D3939392828281818180B0B0B010101000000000000
      0000000000000000000000000000000000008A8A8B7676766160604D4C4D393A
      392828281819180B0B0B01010100000000000000000000000000000000000000
      0000A09FA08B8B8A7676766061604C4D4D3A393A2828281819180B0B0B010101
      000000000000000000000000000000000000B4B4B4A0A0A08B8B8B7676766160
      614D4D4C3A39392828281818190B0C0B00010100000000000000000000000000
      0000C7C7C7B4B4B49FA0A08B8B8B7676766161614D4C4C3A3A39272828181818
      0B0B0B010001000000000000000000000000D9D8D8C7C7C7B4B3B49FA0A08B8B
      8B7676766161614D4D4C3A39392828281818190B0B0B01010100000000000000
      0000E8E8E8D9D8D8C7C7C7B4B4B4A0A0A08A8A8B7576766161604D4D4D39393A
      2828281818180C0B0B000001000000000000F4F5F4E8E8E8D8D9D8C7C7C7B4B4
      B4A09FA08B8A8B7575766161604C4C4D393A3A2828281818180B0B0B00010000
      0000FFFFFFF5F5F5E8E8E8D8D8D8C7C7C7B4B4B4A0A09F8B8B8B767676616161
      4D4C4C3A3A3A2828281818180B0B0B010101FFFFFFFFFFFFF5F5F5E8E8E8D8D9
      D8C7C7C7B4B4B4A0A0A08B8B8B7676766060614D4C4C39393A2828281818180B
      0B0B}
    Visible = False
  end
  object PositionGrp: TGroupBox
    Left = 8
    Top = 232
    Width = 145
    Height = 49
    Caption = 'Position'
    TabOrder = 0
    object Label1: TLabel
      Left = 8
      Top = 24
      Width = 13
      Height = 12
      Caption = 'X='
    end
    object Label2: TLabel
      Left = 72
      Top = 24
      Width = 19
      Height = 12
      Caption = ', Y='
    end
    object OfsXEdt: TEdit
      Left = 24
      Top = 21
      Width = 41
      Height = 20
      TabOrder = 0
      Text = 'OfsXEdt'
      OnChange = OfsEdtChange
    end
    object OfsYEdt: TEdit
      Left = 96
      Top = 21
      Width = 41
      Height = 20
      TabOrder = 1
      Text = 'OfsYEdt'
      OnChange = OfsEdtChange
    end
  end
  object ShowChk: TCheckBox
    Left = 8
    Top = 8
    Width = 145
    Height = 17
    Caption = 'This item is displayed.'
    TabOrder = 1
  end
  object FormatEdt: TEdit
    Left = 88
    Top = 293
    Width = 177
    Height = 21
    Font.Charset = SHIFTJIS_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Terminal'
    Font.Style = []
    ParentFont = False
    TabOrder = 2
    Text = 'FormatEdt'
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 328
    Width = 257
    Height = 49
    Caption = 'Preview setting (no save)'
    TabOrder = 3
    object Label11: TLabel
      Left = 8
      Top = 24
      Width = 56
      Height = 12
      Caption = 'Test string'
    end
    object TestStrEdt: TEdit
      Left = 80
      Top = 21
      Width = 169
      Height = 21
      Font.Charset = SHIFTJIS_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Terminal'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      Text = 'TestStrEdt'
    end
  end
  object BitBtn1: TBitBtn
    Left = 88
    Top = 384
    Width = 81
    Height = 25
    Caption = 'OK (&O)'
    TabOrder = 4
    OnClick = BitBtn1Click
    Kind = bkOK
  end
  object BitBtn2: TBitBtn
    Left = 184
    Top = 384
    Width = 83
    Height = 25
    Caption = 'Cancel'
    TabOrder = 5
    Kind = bkCancel
  end
  object OpenBtn: TButton
    Left = 168
    Top = 232
    Width = 97
    Height = 20
    Caption = 'Open file (&F)'
    TabOrder = 6
    OnClick = OpenBtnClick
  end
  object ClearBtn: TButton
    Left = 168
    Top = 264
    Width = 97
    Height = 20
    Caption = 'Clear image (&C)'
    TabOrder = 7
    OnClick = ClearBtnClick
  end
  object OpenBMDlg: TOpenPictureDialog
    Filter = 'all support files (*.bmp;*.png)|*.bmp;*.png'
    Left = 40
    Top = 40
  end
end
