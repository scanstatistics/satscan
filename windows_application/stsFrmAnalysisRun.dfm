object frmAnalysisRun: TfrmAnalysisRun
  Left = 117
  Top = 106
  BorderStyle = bsDialog
  Caption = 'SatScan Analysis Run'
  ClientHeight = 373
  ClientWidth = 516
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
  OnActivate = FormActivate
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 14
  object Splitter1: TSplitter
    Left = 0
    Top = 243
    Width = 516
    Height = 5
    Cursor = crVSplit
    Align = alBottom
    Beveled = True
  end
  object Panel1: TPanel
    Left = 0
    Top = 248
    Width = 516
    Height = 125
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object lbWarning: TLabel
      Left = 0
      Top = 0
      Width = 516
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
    object Panel2: TPanel
      Left = 0
      Top = 84
      Width = 516
      Height = 41
      Align = alBottom
      BevelOuter = bvNone
      TabOrder = 0
      object btnPrint: TButton
        Left = 321
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
        Left = 417
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
        Left = 225
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
    object rteWarningsBox: TRichEdit
      Left = 0
      Top = 19
      Width = 516
      Height = 65
      TabStop = False
      Align = alClient
      ReadOnly = True
      ScrollBars = ssBoth
      TabOrder = 1
      WantReturns = False
      WordWrap = False
    end
  end
  object rteAnalysisBox: TRichEdit
    Left = 0
    Top = 0
    Width = 516
    Height = 243
    TabStop = False
    Align = alClient
    ReadOnly = True
    ScrollBars = ssBoth
    TabOrder = 1
    WantReturns = False
    WordWrap = False
  end
  object PrintDialog: TPrintDialog
    Left = 8
    Top = 12
  end
end
