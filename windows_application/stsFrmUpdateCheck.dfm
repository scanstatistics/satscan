object frmUpdateCheck: TfrmUpdateCheck
  Left = 344
  Top = 284
  Cursor = crHourGlass
  BorderStyle = bsDialog
  Caption = 'SaTScan Update'
  ClientHeight = 67
  ClientWidth = 313
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  Visible = True
  PixelsPerInch = 96
  TextHeight = 13
  object lblMessage: TLabel
    Left = 10
    Top = 22
    Width = 293
    Height = 22
    Alignment = taCenter
    Anchors = [akLeft, akRight]
    AutoSize = False
    Caption = 'Checking for updates ...'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object pHTTPConnect: TNMHTTP
    Port = 0
    ReportLevel = 0
    OnConnectionFailed = OnConnectionFailed
    Body = 'Default.htm'
    Header = 'Head.txt'
    InputFileMode = False
    OutputFileMode = True
    OnFailure = OnCommandFailure
    ProxyPort = 0
    Left = 8
    Top = 32
  end
end
