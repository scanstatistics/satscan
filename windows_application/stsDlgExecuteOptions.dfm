object dlgExecutionOptions: TdlgExecutionOptions
  Left = 366
  Top = 285
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Session Execution Options'
  ClientHeight = 94
  ClientWidth = 282
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
  object Panel1: TPanel
    Left = 204
    Top = 0
    Width = 78
    Height = 94
    Align = alRight
    BevelOuter = bvNone
    TabOrder = 0
    object btnOk: TButton
      Left = 0
      Top = 16
      Width = 72
      Height = 21
      Caption = '&Ok'
      ModalResult = 1
      TabOrder = 0
      OnClick = btnOkClick
    end
    object btnCancel: TButton
      Left = 0
      Top = 41
      Width = 72
      Height = 21
      Caption = '&Cancel'
      ModalResult = 2
      TabOrder = 1
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 0
    Width = 204
    Height = 94
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 1
    object grpProccessOptions: TGroupBox
      Left = 8
      Top = 12
      Width = 185
      Height = 77
      Anchors = [akLeft, akTop, akRight, akBottom]
      Caption = 'Parallel Proccessors'
      TabOrder = 0
      object rdoAllProcessors: TRadioButton
        Left = 8
        Top = 49
        Width = 89
        Height = 17
        Caption = 'All Processors'
        TabOrder = 0
      end
      object rdoAtMostProcesses: TRadioButton
        Left = 8
        Top = 24
        Width = 59
        Height = 17
        Anchors = [akBottom]
        Caption = 'At Most'
        Checked = True
        TabOrder = 1
        TabStop = True
      end
      object StaticText1: TStaticText
        Left = 110
        Top = 24
        Width = 56
        Height = 17
        Anchors = [akRight, akBottom]
        Caption = 'Processors'
        TabOrder = 2
        OnClick = edtAtMostProcessesAssociatedControlClick
      end
      object edtAtMostProcesses: TEdit
        Left = 69
        Top = 24
        Width = 34
        Height = 16
        Anchors = [akBottom]
        MaxLength = 3
        TabOrder = 3
        Text = '1'
        OnClick = edtAtMostProcessesAssociatedControlClick
        OnExit = edtAtMostProcessesExit
        OnKeyPress = edtAtMostProcessesKeyPress
      end
    end
  end
end
