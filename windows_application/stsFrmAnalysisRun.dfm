object frmAnalysisRun: TfrmAnalysisRun
  Left = 208
  Top = 182
  BorderStyle = bsDialog
  Caption = 'SatScan Analysis Run'
  ClientHeight = 373
  ClientWidth = 505
  Color = clBtnFace
  Constraints.MinHeight = 400
  Constraints.MinWidth = 500
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Courier New'
  Font.Style = []
  FormStyle = fsMDIChild
  OldCreateOrder = False
  Position = poMainFormCenter
  Visible = True
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 14
  object Splitter1: TSplitter
    Left = 0
    Top = 267
    Width = 505
    Height = 5
    Cursor = crVSplit
    Align = alBottom
    Beveled = True
  end
  object reAnalysisBox: TRichEdit
    Left = 0
    Top = 0
    Width = 505
    Height = 267
    Align = alClient
    Lines.Strings = (
      '')
    ParentShowHint = False
    ReadOnly = True
    ScrollBars = ssBoth
    ShowHint = False
    TabOrder = 0
    WantReturns = False
    WordWrap = False
  end
  object Panel1: TPanel
    Left = 0
    Top = 272
    Width = 505
    Height = 101
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    object lbWarning: TLabel
      Left = 0
      Top = 0
      Width = 505
      Height = 19
      Align = alTop
      Caption = ' Warnings / Errors:'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -16
      Font.Name = 'Times New Roman'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object reWarningsBox: TRichEdit
      Left = 0
      Top = 19
      Width = 505
      Height = 41
      Align = alClient
      ParentShowHint = False
      ReadOnly = True
      ScrollBars = ssBoth
      ShowHint = False
      TabOrder = 0
      WantReturns = False
      WordWrap = False
    end
    object Panel2: TPanel
      Left = 0
      Top = 60
      Width = 505
      Height = 41
      Align = alBottom
      BevelOuter = bvNone
      TabOrder = 1
      object btnPrint: TButton
        Left = 310
        Top = 8
        Width = 90
        Height = 25
        Anchors = [akRight, akBottom]
        Caption = 'Print...'
        Enabled = False
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = OnPrintClick
      end
      object btnCancel: TButton
        Left = 406
        Top = 8
        Width = 90
        Height = 25
        Anchors = [akRight, akBottom]
        Caption = 'Cancel'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = OnCancelClick
      end
      object btnEMail: TButton
        Left = 214
        Top = 8
        Width = 90
        Height = 25
        Anchors = [akRight, akBottom]
        Caption = 'E-Mail...'
        Enabled = False
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = OnEMailClick
      end
    end
  end
  object RichEdit1: TRichEdit
    Left = 16
    Top = 8
    Width = 41
    Height = 25
    Lines.Strings = (
      'Rich'
      'Edit'
      '1')
    TabOrder = 2
    Visible = False
  end
  object PrintDialog1: TPrintDialog
    Left = 248
    Top = 448
  end
end
