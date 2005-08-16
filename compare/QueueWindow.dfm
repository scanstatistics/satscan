object frmQueueWindow: TfrmQueueWindow
  Left = 117
  Top = 168
  Width = 693
  Height = 372
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
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object pnlButtons: TPanel
    Left = 0
    Top = 0
    Width = 84
    Height = 345
    Align = alLeft
    BevelOuter = bvNone
    TabOrder = 0
    object btnAdd: TButton
      Left = 12
      Top = 14
      Width = 72
      Height = 22
      Action = ActionScheduleBatch
      TabOrder = 0
    end
    object btnRemove: TButton
      Left = 12
      Top = 40
      Width = 72
      Height = 22
      Action = ActionRemoveBatch
      TabOrder = 1
    end
    object btnStart: TBitBtn
      Left = 12
      Top = 136
      Width = 71
      Height = 33
      Action = ActionStartBatches
      Caption = 'Start'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 2
      Glyph.Data = {
        5A010000424D5A01000000000000760000002800000013000000130000000100
        040000000000E400000000000000000000001000000010000000000000000000
        80000080000000808000800000008000800080800000C0C0C000808080000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00777777777777
        7777777000007777777777777777777000007777777077777777777000007777
        77780777777777700000777777770077777777700000777777770B0777777770
        00007777770000B07777777000007777770BBBBB07777770000077777770FB00
        00777770000077777770BBB0777777700000777700000BFB0777777000007777
        0FBBBFBBB07777700000777770FBFB00000777700000777770BFBFB077777770
        00007777770BFBFB0777777000007777770FFFFFF07777700000777777700000
        000777700000777777777777777777700000777777777777777777700000}
    end
    object btnSaveBatchDefinitions: TBitBtn
      Left = 12
      Top = 75
      Width = 72
      Height = 22
      Action = ActionSaveDefinitions
      Caption = '&Save'
      TabOrder = 3
    end
    object btnLoadDefinitions: TBitBtn
      Left = 12
      Top = 101
      Width = 72
      Height = 22
      Action = ActionLoadDefinitions
      Caption = '&Load'
      TabOrder = 4
    end
    object btnSelectCompareExecutable: TBitBtn
      Left = 12
      Top = 208
      Width = 72
      Height = 22
      Action = ActionCompareOutputFiles
      Caption = 'Co&mpare'
      TabOrder = 5
    end
  end
  object pnlClient: TPanel
    Left = 84
    Top = 0
    Width = 601
    Height = 345
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 1
    object ltvScheduledBatchs: TListView
      Left = 12
      Top = 12
      Width = 577
      Height = 321
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
      OnKeyDown = ltvScheduledBatchsKeyDown
      OnSelectItem = OnScheduledBatchsSelectItem
    end
  end
  object ActionList1: TActionList
    Left = 104
    Top = 40
    object ActionScheduleBatch: TAction
      Caption = '&Add'
      Hint = 'adds new batch process'
      OnExecute = ActionScheduleBatchExecute
    end
    object ActionCancelBatches: TAction
      Caption = '&Cancel'
      Hint = 'Cancel Batches'
      OnExecute = ActionCancelBatchesExecute
    end
    object ActionStartBatches: TAction
      Caption = '&Start'
      Hint = 'Start Batch Processing'
      OnExecute = ActionStartBatchesExecute
    end
    object ActionRemoveBatch: TAction
      Caption = '&Remove'
      OnExecute = ActionRemoveBatchExecute
    end
    object ActionSaveDefinitions: TAction
      Caption = '&Save'
      Hint = 'Save Definitions to File'
      OnExecute = ActionSaveDefinitionsExecute
    end
    object ActionLoadDefinitions: TAction
      Caption = '&Load'
      OnExecute = ActionLoadDefinitionsExecute
    end
    object ActionCompareOutputFiles: TAction
      Caption = 'Co&mpare'
      OnExecute = ActionCompareOutputFilesExecute
    end
    object ActionOptions: TAction
      Caption = '&Options'
    end
  end
  object OpenDialog: TOpenDialog
    DefaultExt = '*.batch_defs'
    Filter = 'Batch Definition Files (*.batch_defs)|*.batch_defs'
    Options = [ofHideReadOnly, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Left = 104
    Top = 72
  end
  object SaveDialog: TSaveDialog
    DefaultExt = '*.batch_defs'
    Filter = 'Batch Definition Files (*.batch_defs)|*.batch_defs'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofCreatePrompt, ofEnableSizing]
    Left = 104
    Top = 104
  end
  object OpenDialog2: TOpenDialog
    DefaultExt = '*.exe'
    Filter = 'Compare Program (*.exe)|*.exe'
    Options = [ofHideReadOnly, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Title = 'Select Compare Program'
    Left = 105
    Top = 136
  end
end
