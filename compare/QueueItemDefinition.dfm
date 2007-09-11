object frmBatchDefinition: TfrmBatchDefinition
  Left = 241
  Top = 280
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Batch Definition'
  ClientHeight = 203
  ClientWidth = 457
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object pnlClient: TPanel
    Left = 0
    Top = 0
    Width = 457
    Height = 174
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 0
    object lblExecutable: TLabel
      Left = 12
      Top = 12
      Width = 84
      Height = 13
      Caption = 'Executable Name'
    end
    object lblParameterFile: TLabel
      Left = 12
      Top = 64
      Width = 93
      Height = 13
      Caption = 'Parameter Filename'
    end
    object lblCommandLineOptions: TLabel
      Left = 12
      Top = 120
      Width = 284
      Height = 13
      Caption = 'Command Line Options (v4.0 and up have -v and -o options)'
      Enabled = False
    end
    object edtExecutableFileName: TEdit
      Left = 12
      Top = 32
      Width = 389
      Height = 21
      ReadOnly = True
      TabOrder = 0
    end
    object Button1: TButton
      Left = 408
      Top = 32
      Width = 40
      Height = 21
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -19
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = BrowseExecutableFilenameClick
    end
    object edtParameterFilename: TEdit
      Left = 12
      Top = 88
      Width = 389
      Height = 21
      ReadOnly = True
      TabOrder = 2
    end
    object Button2: TButton
      Left = 408
      Top = 88
      Width = 40
      Height = 21
      Caption = '...'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -19
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 3
      OnClick = BrowseParameterFilenameClick
    end
    object edtCommandLineOptions: TEdit
      Left = 12
      Top = 144
      Width = 389
      Height = 21
      Enabled = False
      ReadOnly = True
      TabOrder = 4
    end
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 174
    Width = 457
    Height = 29
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    object btnOk: TButton
      Left = 134
      Top = 0
      Width = 72
      Height = 22
      Caption = '&Add'
      ModalResult = 1
      TabOrder = 0
    end
    object btnCancel: TButton
      Left = 214
      Top = 0
      Width = 72
      Height = 22
      Caption = '&Cancel'
      ModalResult = 2
      TabOrder = 1
    end
  end
  object OpenDialog: TOpenDialog
    Options = [ofHideReadOnly, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Left = 424
  end
end
