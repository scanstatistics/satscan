object frmOutputViewer: TfrmOutputViewer
  Left = 305
  Top = 241
  BorderStyle = bsDialog
  Caption = 'View Output File'
  ClientHeight = 281
  ClientWidth = 404
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Courier New'
  Font.Style = []
  FormStyle = fsMDIChild
  OldCreateOrder = False
  Position = poDefault
  Visible = True
  OnCanResize = FormCanResize
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 14
  object btnClose: TButton
    Left = 228
    Top = 250
    Width = 89
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Close'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 0
    OnClick = btnCloseClick
  end
  object reOutputBox: TRichEdit
    Left = 0
    Top = 0
    Width = 404
    Height = 241
    Align = alTop
    Anchors = [akLeft, akTop, akRight, akBottom]
    Lines.Strings = (
      '')
    ParentShowHint = False
    ReadOnly = True
    ScrollBars = ssBoth
    ShowHint = True
    TabOrder = 1
    WantReturns = False
    WordWrap = False
  end
  object btnPrint: TButton
    Left = 60
    Top = 250
    Width = 89
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Print'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 2
    OnClick = btnPrintClick
  end
  object PrintDialog1: TPrintDialog
    Left = 352
    Top = 248
  end
end
