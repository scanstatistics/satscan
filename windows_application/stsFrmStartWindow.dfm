object frmStartWindow: TfrmStartWindow
  Left = 382
  Top = 183
  BorderStyle = bsDialog
  Caption = 'Welcome to SaTScan v4.0'
  ClientHeight = 157
  ClientWidth = 289
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poMainFormCenter
  OnKeyDown = FormKeyDown
  PixelsPerInch = 96
  TextHeight = 13
  object pnlAlignRight: TPanel
    Left = 192
    Top = 0
    Width = 97
    Height = 157
    Align = alRight
    BevelOuter = bvNone
    TabOrder = 0
    object btnOk: TButton
      Left = 12
      Top = 12
      Width = 72
      Height = 24
      Caption = '&Ok'
      ModalResult = 1
      TabOrder = 0
    end
    object btnCancel: TButton
      Left = 12
      Top = 41
      Width = 72
      Height = 24
      Caption = '&Cancel'
      ModalResult = 2
      TabOrder = 1
    end
  end
  object pnlClient: TPanel
    Left = 0
    Top = 0
    Width = 192
    Height = 157
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 1
    object rdgOpenChoices: TRadioGroup
      Left = 12
      Top = 8
      Width = 179
      Height = 134
      Anchors = [akLeft, akTop, akRight, akBottom]
      ItemIndex = 0
      Items.Strings = (
        'Create new session'
        'Open an existing session'
        'Open last session')
      TabOrder = 0
      OnClick = rdgOpenChoicesClick
    end
  end
end
