object BdlgException: TBdlgException
  Left = 222
  Top = 136
  VertScrollBar.Visible = False
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Exception!!'
  ClientHeight = 365
  ClientWidth = 475
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
  object imgAlarmLevelImage: TImage
    Left = 8
    Top = 8
    Width = 49
    Height = 49
  end
  object Bevel1: TBevel
    Left = 0
    Top = 206
    Width = 483
    Height = 5
    Anchors = [akLeft, akTop, akRight]
    Style = bsRaised
    Visible = False
  end
  object rteUserInfo: TRichEdit
    Left = 8
    Top = 56
    Width = 369
    Height = 145
    TabStop = False
    ParentColor = True
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 4
    WantReturns = False
  end
  object btnOK: TButton
    Left = 392
    Top = 56
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 0
  end
  object btnPrint: TButton
    Left = 392
    Top = 96
    Width = 75
    Height = 25
    Caption = 'Print'
    TabOrder = 1
    OnClick = btnPrintClick
  end
  object btnEmail: TButton
    Left = 392
    Top = 136
    Width = 75
    Height = 25
    Caption = 'Email'
    Enabled = False
    TabOrder = 2
    OnClick = btnEmailClick
  end
  object btnMoreLess: TButton
    Left = 392
    Top = 176
    Width = 75
    Height = 25
    Caption = 'More-->>'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    OnClick = btnMoreLessClick
  end
  object rteExtendedInfo: TRichEdit
    Left = 8
    Top = 216
    Width = 369
    Height = 145
    TabStop = False
    ParentColor = True
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 5
    WantReturns = False
  end
  object rteAlarmLevelInfo: TRichEdit
    Left = 72
    Top = 8
    Width = 393
    Height = 41
    TabStop = False
    BorderStyle = bsNone
    Enabled = False
    ParentColor = True
    ReadOnly = True
    TabOrder = 6
    WantReturns = False
  end
end
