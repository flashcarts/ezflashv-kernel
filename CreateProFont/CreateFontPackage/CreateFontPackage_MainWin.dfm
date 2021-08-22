object Form1: TForm1
  Left = -502
  Top = 405
  Width = 340
  Height = 342
  Caption = 'CreateFontPackage'
  Color = clBtnFace
  ParentFont = True
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 12
  object fonOpenDlg: TOpenDialog
    DefaultExt = 'fon'
    Filter = 'MoonShell FontFile (*.ank;*.l2u;*.fon)|*.ank;*.l2u;*.fon'
    Title = 'Select source font file'
    Left = 8
    Top = 8
  end
  object fpkSaveDlg: TSaveDialog
    DefaultExt = 'fpk'
    Filter = 'MoonShell Portable FontPackage (*.fpk)|*.fpk'
    Title = 'Create file.'
    Left = 40
    Top = 8
  end
end
