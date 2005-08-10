object frmQueueWindow: TfrmQueueWindow
  Left = 170
  Top = 192
  Width = 615
  Height = 330
  BorderIcons = [biSystemMenu, biMinimize]
  Caption = 'SaTScan Batch Queue'
  Color = clBtnFace
  Constraints.MinHeight = 284
  Constraints.MinWidth = 325
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object pnlClient: TPanel
    Left = 0
    Top = 0
    Width = 607
    Height = 303
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 0
    object ltvScheduledBatchs: TListView
      Left = 12
      Top = 12
      Width = 583
      Height = 279
      Anchors = [akLeft, akTop, akRight, akBottom]
      Columns = <
        item
          Caption = 'Executable Name'
          Width = 250
        end
        item
          Caption = 'Parameter Filename'
          Width = 250
        end
        item
          Caption = 'Status'
          Width = 125
        end
        item
          Caption = 'Running Time'
          Width = 125
        end>
      HideSelection = False
      MultiSelect = True
      RowSelect = True
      TabOrder = 0
      ViewStyle = vsReport
    end
  end
end
