object Main: TMain
  Left = 159
  Top = 89
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Main'
  ClientHeight = 492
  ClientWidth = 544
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
  object Label2: TLabel
    Left = 8
    Top = 8
    Width = 637
    Height = 12
    Caption = 
      'A main body of MoonShell and various configuration files are set' +
      ' up.|MoonShell'#26412#20307#12392#21508#31278#35373#23450#12501#12449#12452#12523#12434#12475#12483#12488#12450#12483#12503#12375#12414#12377#12290
  end
  object TargetDriveLbl: TLabel
    Left = 8
    Top = 448
    Width = 75
    Height = 12
    Caption = 'TargetDriveLbl'
  end
  object GroupBox1: TGroupBox
    Left = 8
    Top = 32
    Width = 233
    Height = 217
    Caption = 'Configuration files|'#35373#23450#12501#12449#12452#12523
    TabOrder = 1
    object confGlobalINIChk: TCheckBox
      Left = 16
      Top = 24
      Width = 210
      Height = 17
      Caption = '/shell/global.ini is copied.|/shell/global.ini'#12434#12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object confDesktopBMPChk: TCheckBox
      Left = 16
      Top = 48
      Width = 210
      Height = 17
      Caption = '/shell/desktop.bmp is copied.|/shell/desktop.bmp'#12434#12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 1
    end
    object confStartupMP3Chk: TCheckBox
      Left = 16
      Top = 72
      Width = 210
      Height = 17
      Caption = '/shell/startup.mp3 is copied.|/shell/startup.mp3'#12434#12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 2
    end
    object confPluginChk: TCheckBox
      Left = 16
      Top = 120
      Width = 210
      Height = 17
      Caption = '/shell/plugin/*.msp is copied.|/shell/plugin/*.msp'#12434#12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 3
    end
    object confShutdownMP3Chk: TCheckBox
      Left = 16
      Top = 96
      Width = 210
      Height = 17
      Caption = '/shell/shutdown.mp3 is copied.|/shell/shutdown.mp3'#12434#12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 4
    end
    object confResumeChk: TCheckBox
      Left = 15
      Top = 144
      Width = 210
      Height = 17
      Caption = 'Resume function is made effective.|'#12524#12472#12517#12540#12512#12434#26377#21177#12395#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 5
      OnClick = confResumeChkClick
    end
    object confBuiltinPluginChk: TCheckBox
      Left = 15
      Top = 192
      Width = 210
      Height = 17
      Caption = 'DPG/NSF/GBS/NDS is effective.|DPG/NSF/GBS/NDS'#12434#26377#21177#12395#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 6
    end
    object confBookmarkChk: TCheckBox
      Left = 15
      Top = 168
      Width = 210
      Height = 17
      Caption = 'Bookmark function is made effective.|'#12502#12483#12463#12510#12540#12463#12434#26377#21177#12395#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 7
      OnClick = confBookmarkChkClick
    end
  end
  object GroupBox2: TGroupBox
    Left = 8
    Top = 344
    Width = 529
    Height = 97
    Caption = 'Language setting and font file.|'#35328#35486#35373#23450#12392#12501#12457#12531#12488#12501#12449#12452#12523
    TabOrder = 2
    object langUpdateChk: TCheckBox
      Left = 16
      Top = 24
      Width = 505
      Height = 17
      Caption = 'The setting is updated and copied.|'#35373#23450#12434#26356#26032#12375#12390#12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object langCodePageLst: TComboBox
      Left = 16
      Top = 48
      Width = 497
      Height = 20
      Style = csDropDownList
      ItemHeight = 12
      ItemIndex = 0
      TabOrder = 1
      Text = 'langCodePageLst'
      Items.Strings = (
        'langCodePageLst')
    end
    object langAllItemChk: TCheckBox
      Left = 16
      Top = 72
      Width = 505
      Height = 17
      Caption = 'All languages set are displayed.|'#20840#12390#12398#35328#35486#12434#12522#12473#12488#12395#34920#31034#12377#12427
      TabOrder = 2
      OnClick = langAllItemChkClick
    end
  end
  object sysExistBackupChk: TCheckBox
    Left = 8
    Top = 466
    Width = 329
    Height = 17
    Caption = 'When existing, backup is created.|'#21516#21517#12501#12449#12452#12523#12399#12496#12483#12463#12450#12483#12503#12434#20316#25104#12377#12427#12290
    Checked = True
    State = cbChecked
    TabOrder = 5
  end
  object GroupBox4: TGroupBox
    Left = 8
    Top = 256
    Width = 233
    Height = 81
    Caption = 'Clock plugin setting.|'#26178#35336#12503#12521#12464#12452#12531#35373#23450
    TabOrder = 6
    object clockUpdateChk: TCheckBox
      Left = 16
      Top = 24
      Width = 210
      Height = 17
      Caption = 'The setting is updated and copied.|'#35373#23450#12434#26356#26032#12375#12390#12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object clockNameLst: TComboBox
      Left = 16
      Top = 48
      Width = 129
      Height = 20
      Style = csDropDownList
      ItemHeight = 12
      ItemIndex = 0
      TabOrder = 1
      Text = 'clockNameLst'
      OnChange = clockNameLstChange
      Items.Strings = (
        'clockNameLst')
    end
    object ClkPrvBtn: TButton
      Left = 152
      Top = 48
      Width = 65
      Height = 21
      Caption = 'Preview|'#12503#12524#12499#12517#12540
      TabOrder = 2
      OnClick = ClkPrvBtnClick
    end
  end
  object GroupBox3: TGroupBox
    Left = 248
    Top = 32
    Width = 289
    Height = 281
    Caption = 'ROM image|ROM'#12452#12513#12540#12472
    TabOrder = 3
    object romEXFSCreate1Lbl: TLabel
      Left = 32
      Top = 96
      Width = 475
      Height = 12
      Caption = 
        'Please execute '#39'CreateNDSROM_for_EXFS.bat'#39'|'#12475#12483#12488#12450#12483#12503#24460#12395#39'CreateNDSROM' +
        '_for_EXFS.bat'#39
      Visible = False
    end
    object romEXFSCreate2Lbl: TLabel
      Left = 29
      Top = 136
      Width = 240
      Height = 12
      Caption = 'after completing the setup.|'#12434#23455#34892#12375#12390#12367#12384#12373#12356#12290
      Visible = False
    end
    object romMPCFChk: TCheckBox
      Left = 16
      Top = 24
      Width = 264
      Height = 17
      Caption = 
        'The NDS file for GBAMP/AdMovie is copied.|GBAMP/AdMovie'#29992'NDS'#12501#12449#12452#12523#12434 +
        #12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 0
      OnClick = romChkClick
    end
    object romSCCFChk: TCheckBox
      Left = 16
      Top = 48
      Width = 264
      Height = 17
      Caption = 
        'The NDS file for SuperCard/CF is copied.|SuperCard/CF'#29992'NDS'#12501#12449#12452#12523#12434#12467#12500 +
        #12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 1
      OnClick = romChkClick
    end
    object romM3SDChk: TCheckBox
      Left = 16
      Top = 120
      Width = 264
      Height = 17
      Caption = 
        'The NDS file for M3/SD series is copied.|M3/SD'#12471#12522#12540#12474#29992'NDS'#12501#12449#12452#12523#12434#12467#12500#12540#12377#12427 +
        #12290
      Checked = True
      State = cbChecked
      TabOrder = 2
      OnClick = romChkClick
    end
    object romM3CFChk: TCheckBox
      Left = 16
      Top = 104
      Width = 264
      Height = 17
      Caption = 
        'The NDS file for M3/CF series is copied.|M3/CF'#12471#12522#12540#12474#29992'NDS'#12501#12449#12452#12523#12434#12467#12500#12540#12377#12427 +
        #12290
      Checked = True
      State = cbChecked
      TabOrder = 3
      OnClick = romChkClick
    end
    object romEZSDChk: TCheckBox
      Left = 16
      Top = 168
      Width = 264
      Height = 17
      Caption = 
        'The NDS file for EZ-Flash 4/MicroSD is copied.|EZ-Flash4/MicroSD' +
        #29992'NDS'#12501#12449#12452#12523#12434#12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 4
      OnClick = romChkClick
    end
    object romMMCFChk: TCheckBox
      Left = 16
      Top = 144
      Width = 264
      Height = 17
      Caption = 
        'The NDS file for Max Media/CF is copied.|Max Media/CF'#29992'NDS'#12501#12449#12452#12523#12434#12467#12500 +
        #12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 5
      OnClick = romChkClick
    end
    object romSCSDChk: TCheckBox
      Left = 16
      Top = 64
      Width = 264
      Height = 17
      Caption = 
        'The NDS file for SuperCard/SD is copied.|SuperCard/SD'#29992'NDS'#12501#12449#12452#12523#12434#12467#12500 +
        #12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 6
      OnClick = romChkClick
    end
    object romSCMSChk: TCheckBox
      Left = 16
      Top = 80
      Width = 264
      Height = 17
      Caption = 
        'The NDS file for SuperCard/MicroSD is copied.|SuperCard/MicroSD'#29992 +
        'NDS'#12501#12449#12452#12523#12434#12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 7
      OnClick = romChkClick
    end
    object romEWSDChk: TCheckBox
      Left = 16
      Top = 192
      Width = 264
      Height = 17
      Caption = 
        'The NDS file for Ewin2/MicroSD is copied.|Ewin2/MicroSD'#29992'NDS'#12501#12449#12452#12523#12434 +
        #12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 8
      OnClick = romChkClick
    end
    object romNMMCChk: TCheckBox
      Left = 16
      Top = 216
      Width = 264
      Height = 17
      Caption = 
        'The NDS file for MagicKey2-3/SD is copied.|MagicKey2-3/SD'#29992'NDS'#12501#12449#12452 +
        #12523#12434#12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 9
      OnClick = romChkClick
    end
    object romNJSDChk: TCheckBox
      Left = 16
      Top = 232
      Width = 264
      Height = 17
      Caption = 'The NDS file for NinjaDS/SD is copied.|NinjaDS/SD'#29992'NDS'#12501#12449#12452#12523#12434#12467#12500#12540#12377#12427#12290
      Enabled = False
      TabOrder = 10
      OnClick = romChkClick
    end
    object romDLMSChk: TCheckBox
      Left = 16
      Top = 248
      Width = 264
      Height = 17
      Caption = 
        'The NDS file for DS-Link/MicroSD is copied.|DS-Link/MicroSD'#29992'NDS'#12501 +
        #12449#12452#12523#12434#12467#12500#12540#12377#12427#12290
      Checked = True
      State = cbChecked
      TabOrder = 11
      OnClick = romChkClick
    end
  end
  object SetupBtn: TBitBtn
    Left = 344
    Top = 456
    Width = 89
    Height = 25
    Caption = 'Setup|'#12475#12483#12488#12450#12483#12503
    TabOrder = 0
    OnClick = SetupBtnClick
    Kind = bkOK
  end
  object CancelBtn: TBitBtn
    Left = 440
    Top = 456
    Width = 89
    Height = 25
    Caption = 'Cancel|'#12461#12515#12531#12475#12523
    TabOrder = 4
    OnClick = CancelBtnClick
    Kind = bkCancel
  end
  object StartupTimer: TTimer
    Enabled = False
    Interval = 1
    OnTimer = StartupTimerTimer
    Left = 352
    Top = 48
  end
  object ClkPrvPosTimer: TTimer
    Enabled = False
    Interval = 16
    OnTimer = ClkPrvPosTimerTimer
    Left = 392
    Top = 48
  end
end
