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
    Width = 461
    Height = 286
    Anchors = [akLeft, akTop, akRight, akBottom]
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
    Top = 305
    Width = 483
    Height = 19
    Panels = <>
    SimplePanel = False
  end
  object ApplicationEvents1: TApplicationEvents
    OnIdle = ApplicationEvents1Idle
    Left = 88
    Top = 48
  end
end
