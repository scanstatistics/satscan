object frmOptions: TfrmOptions
  Left = 304
  Top = 123
  BorderStyle = bsDialog
  Caption = 'Options'
  ClientHeight = 473
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
    Height = 473
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
      Top = 91
      Width = 91
      Height = 13
      Caption = 'Archive Application'
    end
    object lblArchiveApplicationOptions: TLabel
      Left = 255
      Top = 91
      Width = 36
      Height = 13
      Caption = 'Options'
    end
    object Bevel1: TBevel
      Left = 2
      Top = 300
      Width = 364
      Height = 2
    end
    object Bevel2: TBevel
      Left = 3
      Top = 62
      Width = 364
      Height = 2
    end
    object Bevel3: TBevel
      Left = 3
      Top = 366
      Width = 364
      Height = 2
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
      Top = 72
      Width = 161
      Height = 17
      Caption = 'Save Results Archive File'
      TabOrder = 2
    end
    object edtArchiveApplication: TEdit
      Left = 24
      Top = 111
      Width = 226
      Height = 21
      TabOrder = 3
    end
    object btnBrowseArchiveApplication: TButton
      Left = 333
      Top = 111
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
      Top = 111
      Width = 70
      Height = 21
      TabOrder = 5
    end
    object chkDeleteFileAfterArchiving: TCheckBox
      Left = 24
      Top = 137
      Width = 177
      Height = 17
      Caption = 'Delete Files After Archiving'
      TabOrder = 6
    end
    object chkCreateReadMeFile: TCheckBox
      Left = 24
      Top = 166
      Width = 161
      Height = 17
      Caption = 'Add '#39'ReadMe'#39' File to Archive'
      TabOrder = 7
    end
    object memReadMeText: TMemo
      Left = 24
      Top = 190
      Width = 335
      Height = 98
      TabOrder = 8
    end
    object chkSuppressDosWindow: TCheckBox
      Left = 8
      Top = 376
      Width = 110
      Height = 17
      Caption = 'Suppress Console'
      TabOrder = 9
      OnClick = chkSuppressDosWindowClick
    end
    object rdoGroupThreadPriority: TRadioGroup
      Left = 8
      Top = 403
      Width = 352
      Height = 63
      Caption = 'Thread Priority'
      Columns = 2
      ItemIndex = 0
      Items.Strings = (
        'High Priority Class'
        'Idle Priority Class'
        'Normal Priority Class'
        'Realtime Priority Class')
      TabOrder = 10
    end
    object chkMinimizeConsoleWindow: TCheckBox
      Left = 143
      Top = 376
      Width = 152
      Height = 17
      Caption = 'Inactive Minimized Console'
      TabOrder = 11
      OnClick = chkMinimizeConsoleWindowClick
    end
    object rdgExecuteMethod: TRadioGroup
      Left = 7
      Top = 306
      Width = 352
      Height = 50
      Caption = 'Execute Method'
      Columns = 2
      ItemIndex = 0
      Items.Strings = (
        'Create Process Each Analysis'
        'Batch File Execute')
      TabOrder = 12
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
