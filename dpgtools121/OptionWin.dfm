object Option: TOption
  Left = -431
  Top = 425
  BorderStyle = bsToolWindow
  Caption = 'Option'
  ClientHeight = 440
  ClientWidth = 384
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
  object PriorityLevelLbl: TLabel
    Left = 8
    Top = 384
    Width = 37
    Height = 12
    Caption = 'Priority'
  end
  object CancelBtn: TBitBtn
    Left = 184
    Top = 408
    Width = 89
    Height = 25
    Caption = #12461#12515#12531#12475#12523
    ModalResult = 2
    TabOrder = 1
    OnClick = CancelBtnClick
    Glyph.Data = {
      DE010000424DDE01000000000000760000002800000024000000120000000100
      0400000000006801000000000000000000001000000000000000000000000000
      80000080000000808000800000008000800080800000C0C0C000808080000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
      333333333333333333333333000033338833333333333333333F333333333333
      0000333911833333983333333388F333333F3333000033391118333911833333
      38F38F333F88F33300003339111183911118333338F338F3F8338F3300003333
      911118111118333338F3338F833338F3000033333911111111833333338F3338
      3333F8330000333333911111183333333338F333333F83330000333333311111
      8333333333338F3333383333000033333339111183333333333338F333833333
      00003333339111118333333333333833338F3333000033333911181118333333
      33338333338F333300003333911183911183333333383338F338F33300003333
      9118333911183333338F33838F338F33000033333913333391113333338FF833
      38F338F300003333333333333919333333388333338FFF830000333333333333
      3333333333333333333888330000333333333333333333333333333333333333
      0000}
    NumGlyphs = 2
  end
  object StartBtn: TBitBtn
    Left = 288
    Top = 408
    Width = 89
    Height = 25
    Caption = #12456#12531#12467#12540#12489
    Default = True
    ModalResult = 1
    TabOrder = 2
    OnClick = StartBtnClick
    Glyph.Data = {
      DE010000424DDE01000000000000760000002800000024000000120000000100
      0400000000006801000000000000000000001000000000000000000000000000
      80000080000000808000800000008000800080800000C0C0C000808080000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
      3333333333333333333333330000333333333333333333333333F33333333333
      00003333344333333333333333388F3333333333000033334224333333333333
      338338F3333333330000333422224333333333333833338F3333333300003342
      222224333333333383333338F3333333000034222A22224333333338F338F333
      8F33333300003222A3A2224333333338F3838F338F33333300003A2A333A2224
      33333338F83338F338F33333000033A33333A222433333338333338F338F3333
      0000333333333A222433333333333338F338F33300003333333333A222433333
      333333338F338F33000033333333333A222433333333333338F338F300003333
      33333333A222433333333333338F338F00003333333333333A22433333333333
      3338F38F000033333333333333A223333333333333338F830000333333333333
      333A333333333333333338330000333333333333333333333333333333333333
      0000}
    NumGlyphs = 2
  end
  object ReencOptGroup: TGroupBox
    Left = 8
    Top = 136
    Width = 369
    Height = 241
    Caption = #20877#22311#32302#12458#12503#12471#12519#12531' (Re-encode options)'
    TabOrder = 3
    object VideoWidthLbl: TLabel
      Left = 8
      Top = 45
      Width = 70
      Height = 12
      Caption = #30011#38754#27178#12469#12452#12474
    end
    object VideoWidthUnitLbl: TLabel
      Left = 184
      Top = 42
      Width = 30
      Height = 12
      Caption = 'pixels'
    end
    object VideoBPSLbl: TLabel
      Left = 8
      Top = 93
      Width = 57
      Height = 12
      Caption = #12499#12483#12488#12524#12540#12488
    end
    object VideoBPSUnitLbl: TLabel
      Left = 184
      Top = 93
      Width = 43
      Height = 12
      Caption = 'kbit/sec'
    end
    object VideoFPSLbl: TLabel
      Left = 8
      Top = 117
      Width = 80
      Height = 12
      Caption = #31186#38291#12501#12524#12540#12512#25968
    end
    object VideoFPSUnitLbl: TLabel
      Left = 184
      Top = 117
      Width = 59
      Height = 12
      Caption = 'frames/sec'
    end
    object VideoBrightLbl: TLabel
      Left = 8
      Top = 144
      Width = 31
      Height = 12
      Caption = #26126#12427#12373
    end
    object ReencBrightLbl: TLabel
      Left = 332
      Top = 141
      Width = 78
      Height = 12
      Caption = 'ReencBrightLbl'
    end
    object VideoBlurLbl: TLabel
      Left = 8
      Top = 165
      Width = 33
      Height = 12
      Caption = #12502#12521#12540
    end
    object ReencBlurLbl: TLabel
      Left = 332
      Top = 165
      Width = 78
      Height = 12
      Caption = 'ReencBrightLbl'
    end
    object VideoBlurLightLbl: TLabel
      Left = 289
      Top = 165
      Width = 25
      Height = 12
      Caption = 'Light'
    end
    object VideoAspectLbl: TLabel
      Left = 8
      Top = 69
      Width = 36
      Height = 12
      Caption = #32294#27178#27604
    end
    object VideoBlurDeepLbl: TLabel
      Left = 80
      Top = 165
      Width = 26
      Height = 12
      Caption = 'Deep'
    end
    object ReencPixelFormatLbl: TLabel
      Left = 8
      Top = 21
      Width = 103
      Height = 12
      Caption = #12500#12463#12475#12523#12501#12457#12540#12510#12483#12488
    end
    object ReencWidthLst: TComboBox
      Left = 120
      Top = 40
      Width = 57
      Height = 20
      Style = csDropDownList
      ItemHeight = 12
      ItemIndex = 12
      TabOrder = 0
      Text = '256'
      Items.Strings = (
        '64'
        '80'
        '96'
        '112'
        '128'
        '144'
        '160'
        '176'
        '192'
        '208'
        '224'
        '240'
        '256')
    end
    object ReencKBPSLst: TComboBox
      Left = 120
      Top = 88
      Width = 57
      Height = 20
      Style = csDropDownList
      ItemHeight = 12
      ItemIndex = 0
      TabOrder = 1
      Text = '64'
      Items.Strings = (
        '64')
    end
    object ReencFPSLst: TComboBox
      Left = 120
      Top = 112
      Width = 57
      Height = 20
      Style = csDropDownList
      ItemHeight = 12
      ItemIndex = 19
      TabOrder = 2
      Text = '20'
      Items.Strings = (
        '1'
        '2'
        '3'
        '4'
        '5'
        '6'
        '7'
        '8'
        '9'
        '10'
        '11'
        '12'
        '13'
        '14'
        '15'
        '16'
        '17'
        '18'
        '19'
        '20'
        '21'
        '22'
        '23'
        '24')
    end
    object ReencBrightnessBar: TTrackBar
      Left = 112
      Top = 136
      Width = 217
      Height = 25
      Max = 150
      Min = 50
      Position = 110
      TabOrder = 3
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = ReencBarChange
    end
    object ReencBlurBar: TTrackBar
      Left = 112
      Top = 160
      Width = 177
      Height = 25
      Max = 100
      Min = 50
      Position = 100
      TabOrder = 4
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = ReencBarChange
    end
    object ReencVerticalSwapChk: TCheckBox
      Left = 8
      Top = 184
      Width = 329
      Height = 17
      Caption = #19978#19979#21453#36578' (Upper and lower reversing) (Vertical Swap?)'
      TabOrder = 5
    end
    object ReencAspectLst: TComboBox
      Left = 160
      Top = 64
      Width = 201
      Height = 20
      Style = csDropDownList
      ItemHeight = 12
      ItemIndex = 0
      TabOrder = 6
      Text = 'auto detect'
      Items.Strings = (
        'auto detect'
        '4:3 (academy size)'
        '16:9 (wide screen)'
        '1:2.35 (Cinemascope)')
    end
    object ReencCmdLineEdt: TEdit
      Left = 328
      Top = 184
      Width = 33
      Height = 20
      ReadOnly = True
      TabOrder = 7
      Text = 
        '-noautosub -nosound -frames %0:d -demuxer rawvideo -rawvideo fps' +
        '=24:w=%1:d:h=%2:d:format=bgr24:size=%3:d -ovc lavc -lavcopts vco' +
        'dec=mpeg1video:vbitrate=%4:d:keyint=60 -ofps 24 -of rawvideo'
      Visible = False
    end
    object ReencPixelFormatLst: TComboBox
      Left = 120
      Top = 16
      Width = 241
      Height = 20
      Style = csDropDownList
      ItemHeight = 12
      ItemIndex = 2
      TabOrder = 8
      Text = 'RGB21 for quality'
      Items.Strings = (
        'RGB15 for speed'
        'RGB18 default'
        'RGB21 for quality'
        'RGB24 old version')
    end
    object ReencProfileLst: TComboBox
      Left = 8
      Top = 208
      Width = 273
      Height = 20
      Style = csDropDownList
      ItemHeight = 12
      ItemIndex = 1
      TabOrder = 9
      Text = 'MEncoder: Low-quality profile (fast encode)'
      OnChange = ReencProfileLstChange
      Items.Strings = (
        'MEncoder: Custom comand line'
        'MEncoder: Low-quality profile (fast encode)'
        'MEncoder: High-quality profile (slow encode)')
    end
    object ReencProfileCustomEditBtn: TButton
      Left = 288
      Top = 205
      Width = 75
      Height = 25
      Caption = 'custom edit'
      TabOrder = 10
      OnClick = ReencProfileCustomEditBtnClick
    end
  end
  object SoundGrp: TGroupBox
    Left = 8
    Top = 32
    Width = 369
    Height = 97
    Caption = #12469#12454#12531#12489#12458#12503#12471#12519#12531' (Sound options)'
    TabOrder = 4
    object SoundRateLbl: TLabel
      Left = 8
      Top = 20
      Width = 36
      Height = 12
      Caption = #21608#27874#25968
    end
    object SoundVolumeLbl: TLabel
      Left = 8
      Top = 45
      Width = 74
      Height = 12
      Caption = #38899#37327' (Volume)'
    end
    object sndVolumeLbl: TLabel
      Left = 332
      Top = 45
      Width = 71
      Height = 12
      Caption = 'sndVolumeLbl'
    end
    object SoundRateUnitLbl: TLabel
      Left = 184
      Top = 20
      Width = 13
      Height = 12
      Caption = 'Hz'
    end
    object SoundBPSUnitLbl: TLabel
      Left = 328
      Top = 20
      Width = 24
      Height = 12
      Caption = 'kbps'
    end
    object SoundBPSLbl: TLabel
      Left = 208
      Top = 20
      Width = 39
      Height = 12
      Caption = 'BitRate'
    end
    object SndFreqLst: TComboBox
      Left = 104
      Top = 16
      Width = 73
      Height = 20
      Style = csDropDownList
      ItemHeight = 12
      ItemIndex = 1
      TabOrder = 0
      Text = '32768 (HQ)'
      OnChange = SndFreqLstChange
      Items.Strings = (
        '32000'
        '32768 (HQ)')
    end
    object SndKBPSLst: TComboBox
      Left = 272
      Top = 16
      Width = 49
      Height = 20
      Style = csDropDownList
      ItemHeight = 12
      ItemIndex = 7
      TabOrder = 1
      Text = '128'
      OnChange = SndFreqOptionsChange
      Items.Strings = (
        '32'
        '48'
        '56'
        '64'
        '80'
        '96'
        '112'
        '128'
        '160'
        '192'
        '224'
        '256'
        '320'
        '384')
    end
    object SndVolumeBar: TTrackBar
      Left = 96
      Top = 40
      Width = 233
      Height = 25
      Max = 40
      Min = -40
      TabOrder = 2
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = SndFreqOptionsChange
    end
    object SndGetModeLst: TComboBox
      Left = 8
      Top = 64
      Width = 353
      Height = 20
      Style = csDropDownList
      ItemHeight = 12
      ItemIndex = 0
      TabOrder = 3
      Text = 'use mencoder.exe A special audio codec doesnt correspond.'
      Items.Strings = (
        'use mencoder.exe A special audio codec doesnt correspond.'
        'use ffmpeg.exe Fastest method.'
        'use extractwave.exe DirectShow normal method 128kbyte/block'
        'use extractwave.exe DirectShow alternative method 4byte/block'
        'The demuxed(defrosted) file is used. (MediaFilename.wav)')
    end
  end
  object PreencLst: TComboBox
    Left = 8
    Top = 6
    Width = 369
    Height = 20
    Style = csDropDownList
    ItemHeight = 12
    ItemIndex = 0
    TabOrder = 0
    Text = #33258#21205#36984#25246#65288'DirectShow'#12391#30452#25509#22793#25563#12391#12365#12394#12363#12387#12383#12425#21069#28310#20633#12434#35430#12415#12427#65289
    OnChange = PreencLstChange
    Items.Strings = (
      #33258#21205#36984#25246#65288'DirectShow'#12391#30452#25509#22793#25563#12391#12365#12394#12363#12387#12383#12425#21069#28310#20633#12434#35430#12415#12427#65289
      'ffmpeg'#12391#21069#28310#20633#12375#12390#12363#12425#22793#25563#12377#12427' ('#29305#27530#12501#12449#12452#12523#29992') ('#20302#36895')'
      'mencoder'#12391#21069#28310#20633#12375#12390#12363#12425#22793#25563#12377#12427' ('#26410#23550#24540')')
  end
  object AdvanceChk: TCheckBox
    Left = 8
    Top = 416
    Width = 161
    Height = 17
    Caption = 'AdvanceChk'
    TabOrder = 5
    OnClick = AdvanceChkClick
  end
  object PriorityLevelLst: TComboBox
    Left = 56
    Top = 381
    Width = 321
    Height = 20
    Style = csDropDownList
    ItemHeight = 12
    ItemIndex = 1
    TabOrder = 6
    Text = 
      'BELOW_NORMAL. Process that has priority above IDLE_PRIORITY_CLAS' +
      'S.'
    Items.Strings = (
      'IDLE. Process whose threads run only when the system is idle.'
      
        'BELOW_NORMAL. Process that has priority above IDLE_PRIORITY_CLAS' +
        'S.'
      'NORMAL. Process with no special scheduling needs.'
      
        'ABOVE_NORMAL. Process that has priority above NORMAL_PRIORITY_CL' +
        'ASS.'
      
        'HIGH. Process that performs time-critical tasks that must be exe' +
        'cuted immediately.'
      'REALTIME. Process that has the highest possible priority.')
  end
end
