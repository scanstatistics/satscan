object frmOptions: TfrmOptions
  Left = 320
  Top = 204
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 135
  ClientWidth = 367
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object pnlClient: TPanel
    Left = 0
    Top = 0
    Width = 367
    Height = 135
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 0
    object lblComparisonApplication: TLabel
      Left = 8
      Top = 8
      Width = 110
      Height = 13
      Caption = 'Comparison Application'
    end
    object lblArchiveApplication: TLabel
      Left = 24
      Top = 85
      Width = 91
      Height = 13
      Caption = 'Archive Application'
    end
    object lblArchiveApplicationOptions: TLabel
      Left = 255
      Top = 85
      Width = 36
      Height = 13
      Caption = 'Options'
    end
    object edtComparisonApplication: TEdit
      Left = 8
      Top = 32
      Width = 320
      Height = 21
      TabOrder = 0
    end
    object btnBrowseComparisonApp: TButton
      Left = 334
      Top = 32
      Width = 27
      Height = 21
      Hint = 'Select Comparison Program'
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -19
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
      TabOrder = 1
      OnClick = btnBrowseComparisonAppClick
    end
    object chkArchiveResults: TCheckBox
      Left = 8
      Top = 66
      Width = 161
      Height = 17
      Caption = 'Save Results Archive File'
      TabOrder = 2
    end
    object edtArchiveApplication: TEdit
      Left = 24
      Top = 105
      Width = 226
      Height = 21
      TabOrder = 3
    end
    object btnBrowseArchiveApplication: TButton
      Left = 333
      Top = 105
      Width = 27
      Height = 21
      Hint = 'Select Archive Program'
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -19
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
      TabOrder = 4
      OnClick = btnBrowseArchiveApplicationClick
    end
    object edtArchiveApplicationOptions: TEdit
      Left = 256
      Top = 105
      Width = 70
      Height = 21
      TabOrder = 5
    end
  end
  object OpenDialog: TOpenDialog
    DefaultExt = '.txt'
    Filter = 'Text Files (*.txt)|*.txt|All Files (*.*)|*.*'
    Options = [ofHideReadOnly, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Left = 336
    Top = 64
  end
end
