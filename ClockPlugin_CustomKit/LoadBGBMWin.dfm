object LoadBGBM: TLoadBGBM
  Left = -493
  Top = 331
  BorderStyle = bsToolWindow
  Caption = 'LoadBGBM'
  ClientHeight = 455
  ClientWidth = 271
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
  object PrvImg: TImage
    Left = 8
    Top = 8
    Width = 256
    Height = 192
    OnMouseDown = PrvImgMouseDown
    OnMouseMove = PrvImgMouseMove
    OnMouseUp = PrvImgMouseUp
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 232
    Width = 257
    Height = 113
    Caption = #25313#22823#32302#23567
    TabOrder = 0
    object ZoomSizeLbl: TLabel
      Left = 8
      Top = 24
      Width = 64
      Height = 12
      Caption = #25313#22823#29575' 100%'
    end
    object ZoomWidthBtn: TButton
      Left = 8
      Top = 48
      Width = 57
      Height = 25
      Caption = #27178#26368#22823
      TabOrder = 0
      OnClick = ZoomWidthBtnClick
    end
    object ZoomHeightBtn: TButton
      Left = 72
      Top = 48
      Width = 57
      Height = 25
      Caption = #32294#26368#22823
      TabOrder = 1
      OnClick = ZoomHeightBtnClick
    end
    object Zoom100Btn: TButton
      Left = 160
      Top = 48
      Width = 41
      Height = 25
      Caption = '100%'
      TabOrder = 2
      OnClick = Zoom100BtnClick
    end
    object ZoomManBtn: TButton
      Left = 192
      Top = 16
      Width = 57
      Height = 25
      Caption = #25968#20516#25351#23450
      TabOrder = 3
      OnClick = ZoomManBtnClick
    end
    object Zoom50Btn: TButton
      Left = 208
      Top = 48
      Width = 41
      Height = 25
      Caption = '50%'
      TabOrder = 4
      OnClick = Zoom50BtnClick
    end
    object ZoomBar: TTrackBar
      Left = 2
      Top = 80
      Width = 253
      Height = 25
      Min = 1
      Position = 1
      TabOrder = 5
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = ZoomBarChange
    end
  end
  object GroupBox2: TGroupBox
    Left = 8
    Top = 352
    Width = 257
    Height = 65
    Caption = #20301#32622#35519#25972
    TabOrder = 1
    object PosLbl: TLabel
      Left = 8
      Top = 20
      Width = 52
      Height = 12
      Caption = #31227#21205'=(0,0)'
    end
    object PosInitBtn: TButton
      Left = 176
      Top = 16
      Width = 73
      Height = 25
      Caption = #12476#12525#21021#26399#21270
      TabOrder = 0
      OnClick = PosInitBtnClick
    end
    object TileModeChk: TCheckBox
      Left = 8
      Top = 40
      Width = 137
      Height = 17
      Caption = #12479#12452#12523#25975#12365#35440#12417#12514#12540#12489
      Checked = True
      State = cbChecked
      TabOrder = 1
      OnClick = TileModeChkClick
    end
  end
  object BitBtn1: TBitBtn
    Left = 8
    Top = 424
    Width = 153
    Height = 25
    Caption = #19978#35352#35373#23450#12391#35501#12415#36796#12415
    TabOrder = 2
    OnClick = BitBtn1Click
    Kind = bkOK
  end
  object BitBtn3: TBitBtn
    Left = 168
    Top = 424
    Width = 97
    Height = 25
    TabOrder = 3
    Kind = bkCancel
  end
  object DiffuseChk: TCheckBox
    Left = 8
    Top = 208
    Width = 257
    Height = 17
    Caption = '24bit->15bit'#22793#25563#12391#35492#24046#25313#25955#12377#12427#12290
    Checked = True
    State = cbChecked
    TabOrder = 4
    OnClick = DiffuseChkClick
  end
end
