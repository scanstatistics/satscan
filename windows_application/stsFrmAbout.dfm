object frmAbout: TfrmAbout
  Left = 412
  Top = 232
  BorderStyle = bsDialog
  Caption = 'About SatScan'
  ClientHeight = 401
  ClientWidth = 474
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 24
    Top = 16
    Width = 114
    Height = 29
    Caption = 'SaTScan '
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -24
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object lblVersionDate: TLabel
    Left = 64
    Top = 48
    Width = 77
    Height = 16
    Caption = 'May 31, 2002'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label12: TLabel
    Left = 24
    Top = 152
    Width = 43
    Height = 17
    AutoSize = False
    Caption = 'Email:'
    Font.Charset = ANSI_CHARSET
    Font.Color = clBlack
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    WordWrap = True
  end
  object lblEmail: TLabel
    Left = 88
    Top = 152
    Width = 169
    Height = 17
    AutoSize = False
    Caption = 'satscan@nih.gov'
    Font.Charset = ANSI_CHARSET
    Font.Color = clBlack
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    WordWrap = True
  end
  object Label9: TLabel
    Left = 24
    Top = 72
    Width = 319
    Height = 16
    Caption = 'Software for the spatial and space-time scan statistics.'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label10: TLabel
    Left = 24
    Top = 104
    Width = 298
    Height = 16
    Caption = 'National Cancer Institute, NIH, Bethesda, Maryland'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label11: TLabel
    Left = 24
    Top = 128
    Width = 73
    Height = 17
    AutoSize = False
    Caption = 'Web Site:  '
    Font.Charset = ANSI_CHARSET
    Font.Color = clBlack
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    WordWrap = True
  end
  object Label13: TLabel
    Left = 96
    Top = 128
    Width = 313
    Height = 17
    AutoSize = False
    Caption = 'http://srab.cancer.gov/satscan/'
    Font.Charset = ANSI_CHARSET
    Font.Color = clBlack
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    WordWrap = True
  end
  object Label2: TLabel
    Left = 24
    Top = 176
    Width = 431
    Height = 64
    Caption = 
      'Written by Martin Kulldorff, Associate Professor of Biostatistic' +
      's, University of Connecticut and Scott Hostovich, Don Green, Kat' +
      'hrine Rand, Greg Gherman, Grey Williams and David DeFrancesco, I' +
      'nformation Management Services, Inc., Silver Spring, Maryland.'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    WordWrap = True
  end
  object Label4: TLabel
    Left = 24
    Top = 256
    Width = 421
    Height = 32
    Caption = 
      'This software may be freely used as long as a proper citation is' +
      ' given.   Suggested citation:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    WordWrap = True
  end
  object Label5: TLabel
    Left = 24
    Top = 329
    Width = 425
    Height = 32
    Caption = 
      'SatScan v.3.0:  Software for the spacial and space-time scan sta' +
      'tistics.     Bethesda, MD:  National Cancer Institute, 2002.'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    WordWrap = True
  end
  object Label6: TLabel
    Left = 24
    Top = 296
    Width = 425
    Height = 32
    Caption = 
      'Kulldorff M, Hostovich S, Green D, Rand K, Gherman G, Williams G' +
      ', DeFrancesco D.'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    WordWrap = True
  end
  object lblVersion: TLabel
    Left = 144
    Top = 16
    Width = 76
    Height = 29
    Caption = 'v 3.0.0'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -24
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label7: TLabel
    Left = 24
    Top = 48
    Width = 38
    Height = 16
    Caption = 'Date:  '
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Button1: TButton
    Left = 368
    Top = 371
    Width = 90
    Height = 25
    Caption = 'Ok'
    TabOrder = 0
    OnClick = Button1Click
  end
end
