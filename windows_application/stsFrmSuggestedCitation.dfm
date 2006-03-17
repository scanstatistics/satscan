object frmSuggestedCitation: TfrmSuggestedCitation
  Left = 264
  Top = 218
  Width = 493
  Height = 325
  Caption = 'Suggested Citation:'
  Color = clBtnFace
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
    Height = 267
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
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 0
  end
  object ApplicationEvents1: TApplicationEvents
    OnIdle = ApplicationEvents1Idle
    Left = 88
    Top = 48
  end
end
