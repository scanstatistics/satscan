object frmAdvancedParameters: TfrmAdvancedParameters
  Left = 233
  Top = 215
  BorderStyle = bsDialog
  Caption = 'Advanced Features'
  ClientHeight = 348
  ClientWidth = 496
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object pnlButtons: TPanel
    Left = 412
    Top = 0
    Width = 84
    Height = 348
    Align = alRight
    BevelOuter = bvNone
    TabOrder = 0
    object btnOk: TButton
      Left = 6
      Top = 19
      Width = 72
      Height = 24
      Caption = '&Close'
      Default = True
      ModalResult = 1
      TabOrder = 0
    end
  end
  object PageControl: TPageControl
    Left = 0
    Top = 0
    Width = 412
    Height = 348
    ActivePage = tsAdjustmentsTabSheet
    Align = alClient
    TabOrder = 1
    object tsAdjustmentsTabSheet: TTabSheet
      Caption = 'Adjustments'
      object rdgTemporalTrendAdj: TRadioGroup
        Left = 6
        Top = 81
        Width = 392
        Height = 121
        Caption = 'Adjustment for Temporal Trend'
        Enabled = False
        ItemIndex = 0
        Items.Strings = (
          'None'
          'Nonparametric, with time stratified randomization'
          'Log linear with')
        TabOrder = 0
        TabStop = True
        OnClick = rdgTemporalTrendAdjClick
      end
      object edtLogLinear: TEdit
        Left = 106
        Top = 170
        Width = 35
        Height = 21
        Enabled = False
        MaxLength = 5
        TabOrder = 1
        Text = '0'
        OnExit = edtLogLinearExit
        OnKeyPress = FloatKeyPress
      end
      object lblLogLinear: TStaticText
        Left = 144
        Top = 174
        Width = 67
        Height = 17
        AutoSize = False
        Caption = '%  per year'
        TabOrder = 2
      end
      object grpRelativeRiskAdjustment: TGroupBox
        Left = 6
        Top = 3
        Width = 392
        Height = 70
        Caption = 'Adjustment for Relative Risks'
        TabOrder = 3
        object lblRelativeRisksAdjustmentFile: TLabel
          Left = 12
          Top = 21
          Width = 163
          Height = 13
          Anchors = []
          Caption = 'Adjustment For Relative Risks File:'
        end
        object edtRelativeRisksAdjustmentFile: TEdit
          Left = 13
          Top = 37
          Width = 337
          Height = 21
          Anchors = []
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = edtRelativeRisksAdjustmentFileChange
        end
        object btnBrowseRelativeRisksFile: TButton
          Left = 357
          Top = 37
          Width = 25
          Height = 21
          Hint = 'browse for special population file'
          Anchors = []
          Caption = '...'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsBold]
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 1
          OnClick = btnBrowseRelativeRisksFileClick
        end
      end
    end
    object tsOther: TTabSheet
      Caption = 'Other'
      ImageIndex = 1
      object grpInputFiles: TGroupBox
        Left = 6
        Top = 3
        Width = 392
        Height = 73
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Input Files'
        TabOrder = 0
        object Label21: TLabel
          Left = 12
          Top = 22
          Width = 162
          Height = 13
          Caption = 'Special Max Circle Population File:'
        end
        object edtMaxCirclePopulationFilename: TEdit
          Left = 12
          Top = 39
          Width = 337
          Height = 21
          Anchors = [akLeft, akRight]
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = edtMaxCirclePopulationFilenameChange
        end
        object btnBrowseMaxCirclePopFile: TButton
          Left = 355
          Top = 39
          Width = 25
          Height = 21
          Hint = 'browse for special population file'
          Anchors = [akRight, akBottom]
          Caption = '...'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsBold]
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 1
          OnClick = btnBrowseMaxCirclePopFileClick
        end
      end
      object grpAnalysis: TGroupBox
        Left = 6
        Top = 81
        Width = 392
        Height = 50
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Analysis'
        TabOrder = 1
        object chkTerminateEarly: TCheckBox
          Left = 12
          Top = 22
          Width = 253
          Height = 17
          Anchors = [akLeft, akTop, akRight]
          Caption = 'Terminate replications early for large p-values'
          TabOrder = 0
        end
      end
      object grpScanningWindow: TGroupBox
        Left = 6
        Top = 137
        Width = 392
        Height = 111
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Scanning Window'
        TabOrder = 2
        object chkRestrictTemporalRange: TCheckBox
          Left = 12
          Top = 22
          Width = 349
          Height = 17
          Caption = 'Include only clusters in:'
          TabOrder = 0
          OnClick = chkRestrictTemporalRangeClick
        end
        object stStartRangeTo: TStaticText
          Left = 261
          Top = 49
          Width = 19
          Height = 17
          Caption = 'to: '
          TabOrder = 1
        end
        object edtStartRangeStartYear: TEdit
          Left = 154
          Top = 46
          Width = 39
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          MaxLength = 4
          TabOrder = 2
          Text = '1900'
          OnExit = edtStartRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeStartMonth: TEdit
          Left = 197
          Top = 46
          Width = 25
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 3
          Text = '1'
          OnExit = edtStartRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeStartDay: TEdit
          Left = 226
          Top = 46
          Width = 25
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 4
          Text = '1'
          OnExit = edtStartRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeEndYear: TEdit
          Left = 284
          Top = 46
          Width = 39
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          MaxLength = 4
          TabOrder = 5
          Text = '1900'
          OnExit = edtStartRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeEndMonth: TEdit
          Left = 327
          Top = 46
          Width = 25
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 6
          Text = '1'
          OnExit = edtStartRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeEndDay: TEdit
          Left = 356
          Top = 46
          Width = 25
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 7
          Text = '1'
          OnExit = edtStartRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object stStartWindowRange: TStaticText
          Left = 30
          Top = 49
          Width = 106
          Height = 17
          Caption = 'Start Window Range:'
          TabOrder = 8
        end
        object stEndWindowRange: TStaticText
          Left = 30
          Top = 81
          Width = 103
          Height = 17
          Caption = 'End Window Range:'
          TabOrder = 9
        end
        object edtEndRangeStartYear: TEdit
          Left = 154
          Top = 78
          Width = 39
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          MaxLength = 4
          TabOrder = 10
          Text = '1900'
          OnExit = edtEndRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtEndRangeStartMonth: TEdit
          Left = 197
          Top = 78
          Width = 25
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 11
          Text = '12'
          OnExit = edtEndRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtEndRangeStartDay: TEdit
          Left = 226
          Top = 78
          Width = 25
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 12
          Text = '31'
          OnExit = edtEndRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object stEndRangeTo: TStaticText
          Left = 261
          Top = 81
          Width = 19
          Height = 17
          Caption = 'to: '
          TabOrder = 13
        end
        object edtEndRangeEndYear: TEdit
          Left = 284
          Top = 78
          Width = 39
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          MaxLength = 4
          TabOrder = 14
          Text = '1900'
          OnExit = edtEndRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtEndRangeEndMonth: TEdit
          Left = 327
          Top = 78
          Width = 25
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 15
          Text = '12'
          OnExit = edtEndRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtEndRangeEndDay: TEdit
          Left = 356
          Top = 78
          Width = 25
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 16
          Text = '31'
          OnExit = edtEndRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
      end
      object grpOutput: TGroupBox
        Left = 6
        Top = 253
        Width = 391
        Height = 61
        Caption = 'Output'
        TabOrder = 3
        object lblReportSmallerClusters: TLabel
          Left = 234
          Top = 23
          Width = 139
          Height = 32
          AutoSize = False
          Caption = 'percent of population at risk.'
          WordWrap = True
        end
        object chkRestrictReportedClusters: TCheckBox
          Left = 12
          Top = 22
          Width = 175
          Height = 17
          Caption = 'Report only clusters smaller than'
          TabOrder = 0
          OnClick = chkRestrictReportedClustersClick
        end
        object edtReportClustersSmallerThan: TEdit
          Left = 188
          Top = 21
          Width = 40
          Height = 21
          TabOrder = 1
          Text = '49'
          OnExit = edtReportClustersSmallerThanExit
          OnKeyPress = edtReportClustersSmallerThanKeyPress
        end
      end
    end
  end
  object OpenDialog: TOpenDialog
    Left = 472
    Top = 320
  end
end
