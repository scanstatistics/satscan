object frmStartWindow: TfrmStartWindow
  Left = 390
  Top = 336
  BorderStyle = bsDialog
  Caption = 'SaTScan v4.0'
  ClientHeight = 149
  ClientWidth = 182
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
  object pnlClient: TPanel
    Left = 0
    Top = 0
    Width = 182
    Height = 149
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 0
    object btnCreateNew: TButton
      Left = 16
      Top = 16
      Width = 150
      Height = 25
      Caption = 'Create &New Session'
      Default = True
      TabOrder = 0
      OnClick = btnCreateNewClick
    end
    object btnOpenSaved: TButton
      Left = 16
      Top = 48
      Width = 150
      Height = 25
      Caption = 'Open &Saved Session'
      TabOrder = 1
      OnClick = btnOpenSavedClick
    end
    object btnOpenLast: TButton
      Left = 16
      Top = 80
      Width = 150
      Height = 25
      Caption = 'Open &Last Session'
      TabOrder = 2
      OnClick = btnOpenLastClick
    end
    object btnCancel: TButton
      Left = 16
      Top = 112
      Width = 150
      Height = 25
      Caption = '&Cancel'
      TabOrder = 3
      OnClick = btnCancelClick
    end
  end
end
