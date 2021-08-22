object Main: TMain
  Left = -578
  Top = 378
  BorderStyle = bsToolWindow
  Caption = 'Main'
  ClientHeight = 264
  ClientWidth = 496
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
  object prv64: TImage
    Left = 232
    Top = 8
    Width = 64
    Height = 48
  end
  object prv256: TImage
    Left = 232
    Top = 64
    Width = 256
    Height = 192
  end
  object PrgLbl: TLabel
    Left = 304
    Top = 11
    Width = 129
    Height = 12
    AutoSize = False
    Caption = 'PrgLbl'
  end
  object FilesLst: TListBox
    Left = 8
    Top = 8
    Width = 217
    Height = 249
    Enabled = False
    Font.Charset = SHIFTJIS_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Terminal'
    Font.Style = []
    ItemHeight = 13
    ParentFont = False
    TabOrder = 0
  end
  object PrgBar: TProgressBar
    Left = 304
    Top = 32
    Width = 185
    Height = 15
    TabOrder = 1
  end
  object StopBtn: TButton
    Left = 440
    Top = 8
    Width = 49
    Height = 17
    Cancel = True
    Caption = 'STOP!!'
    TabOrder = 2
    OnClick = StopBtnClick
  end
  object GLDPNGMemo: TMemo
    Left = 8
    Top = 176
    Width = 385
    Height = 81
    BorderStyle = bsNone
    Lines.Strings = (
      ''
      'GLDPNG (c) 1998-2001 Copyright Tarquin All Rights Reserved.'
      
        'ZLIB ver 1.1.3 (c) 1995-1996 Copyright Jean-loup Gailly and Mark' +
        ' Adler'
      ''
      'Unit NkDIB  -- DIB '#29992' '#12464#12521#12501#12451#12483#12463#12463#12521#12473' Ver. 0.73 2001.7.7'
      'Coded By T.Nakamura('#20013#26449' '#25299#30007')')
    ParentColor = True
    TabOrder = 3
  end
  object BaseINIMemo: TMemo
    Left = 16
    Top = 24
    Width = 185
    Height = 89
    Lines.Strings = (
      '; Setting file for CreateThumbnail.exe'
      ''
      '[System]'
      ''
      'ShowCompletionDialog=1'
      ''
      '[Format]'
      ''
      '; The color depth for the full-screen preview is set.'
      '; (16=4bitColors, 256=8bitColors)'
      '; '#12501#12523#12473#12463#12522#12540#12531#12503#12524#12499#12517#12540#12398#12383#12417#12398#33394#28145#24230#12434#35373#23450#12375#12414#12377#12290
      '; (16=16'#33394#12499#12483#12488#12510#12483#12503', 256=256'#33394#12499#12483#12488#12510#12483#12503')'
      ''
      'ColorCount=256'
      ''
      '; The screen size of the full-screen preview is unit of percent.'
      '; '#12501#12523#12473#12463#12522#12540#12531#12503#12524#12499#12517#12540#12398#30011#38754#12469#12452#12474#12434#65285#21336#20301#12391#35373#23450#12375#12414#12377#12290
      ''
      'ScreenRatio=100'
      '')
    TabOrder = 4
    Visible = False
    WordWrap = False
  end
  object PathDlg: TOpenDialog
    FileName = 'All files in the folder are processed. '
    Filter = 'Support file types (*.bmp/*.jpg/*.bmp)|*.bmp;*.jpg;*.png'
    Title = #12501#12457#12523#12480#20869#12398#20840#12501#12449#12452#12523#12434#20966#29702#12375#12414#12377#12290
    Left = 240
    Top = 72
  end
  object StartupTimer: TTimer
    Interval = 1
    OnTimer = StartupTimerTimer
    Left = 272
    Top = 72
  end
end
