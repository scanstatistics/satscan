object frmSuggestedCitation: TfrmSuggestedCitation
  Left = 368
  Top = 204
  Width = 493
  Height = 361
  BorderIcons = [biSystemMenu]
  Caption = 'Suggested Citation:'
  Color = clBtnFace
  Constraints.MinHeight = 350
  Constraints.MinWidth = 350
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object RichEdit1: TRichEdit
    Left = 12
    Top = 12
    Width = 457
    Height = 278
    Align = alClient
    BorderStyle = bsNone
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ImeMode = imDisable
    Lines.Strings = (
      'RichEdit1')
    ParentFont = False
    ParentShowHint = False
    ReadOnly = True
    ScrollBars = ssVertical
    ShowHint = False
    TabOrder = 0
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 302
    Width = 481
    Height = 19
    Panels = <>
    SimplePanel = False
  end
  object pnlClientNorth: TPanel
    Left = 0
    Top = 0
    Width = 481
    Height = 12
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 2
  end
  object pnlClientSouth: TPanel
    Left = 0
    Top = 290
    Width = 481
    Height = 12
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 3
  end
  object pnlClientWest: TPanel
    Left = 0
    Top = 12
    Width = 12
    Height = 278
    Align = alLeft
    BevelOuter = bvNone
    TabOrder = 4
  end
  object pnlClientEast: TPanel
    Left = 469
    Top = 12
    Width = 12
    Height = 278
    Align = alRight
    BevelOuter = bvNone
    TabOrder = 5
  end
  object ApplicationEvents1: TApplicationEvents
    OnIdle = ApplicationEvents1Idle
    Left = 88
    Top = 48
  end
end
