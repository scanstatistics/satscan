object frmAdvancedParameters: TfrmAdvancedParameters
  Left = 261
  Top = 134
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
    TabOrder = 1
    object btnOk: TButton
      Left = 6
      Top = 54
      Width = 72
      Height = 24
      Caption = '&Close'
      Default = True
      ModalResult = 1
      TabOrder = 0
    end
    object btnShowAll: TButton
      Left = 6
      Top = 22
      Width = 72
      Height = 24
      Caption = '&Show All'
      Default = True
      TabOrder = 1
      OnClick = btnShowAllClick
    end
  end
  object PageControl: TPageControl
    Left = 2
    Top = 2
    Width = 410
    Height = 344
    ActivePage = TabSheet3
    TabOrder = 0
    object TabSheet2: TTabSheet
      Caption = 'Spatial Window'
      ImageIndex = 3
    end
    object tsOther: TTabSheet
      Caption = 'Temporal Window'
      ImageIndex = 1
      object grpScanningWindow: TGroupBox
        Left = 6
        Top = 58
        Width = 390
        Height = 111
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Scanning Window'
        TabOrder = 0
        object chkRestrictTemporalRange: TCheckBox
          Left = 12
          Top = 22
          Width = 349
          Height = 17
          Caption = 'Include only windows with:'
          TabOrder = 0
          OnClick = chkRestrictTemporalRangeClick
        end
        object stStartRangeTo: TStaticText
          Left = 261
          Top = 49
          Width = 19
          Height = 17
          Caption = 'to: '
          TabOrder = 5
        end
        object edtStartRangeStartYear: TEdit
          Left = 154
          Top = 46
          Width = 37
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
          Width = 23
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
          Width = 23
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
          Width = 37
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          MaxLength = 4
          TabOrder = 6
          Text = '1900'
          OnExit = edtStartRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeEndMonth: TEdit
          Left = 327
          Top = 46
          Width = 23
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 7
          Text = '1'
          OnExit = edtStartRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeEndDay: TEdit
          Left = 356
          Top = 46
          Width = 23
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 8
          Text = '1'
          OnExit = edtStartRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object stStartWindowRange: TStaticText
          Left = 46
          Top = 49
          Width = 91
          Height = 17
          AutoSize = False
          Caption = 'Start time in range:'
          TabOrder = 1
        end
        object stEndWindowRange: TStaticText
          Left = 46
          Top = 81
          Width = 89
          Height = 17
          Caption = 'End time in range:'
          TabOrder = 9
        end
        object edtEndRangeStartYear: TEdit
          Left = 154
          Top = 78
          Width = 37
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
          Width = 23
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
          Width = 23
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
          Width = 37
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
          Width = 23
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
          Width = 23
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
    end
    object tsAdjustmentsTabSheet: TTabSheet
      Caption = 'Adjustments'
      object rdgTemporalTrendAdj: TRadioGroup
        Left = 6
        Top = 3
        Width = 390
        Height = 121
        Caption = 'Temporal Adjustments'
        Enabled = False
        ItemIndex = 0
        Items.Strings = (
          'None'
          'Nonparametric, with time stratified randomization'
          'Log linear with')
        TabOrder = 0
        OnClick = rdgTemporalTrendAdjClick
      end
      object edtLogLinear: TEdit
        Left = 106
        Top = 92
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
        Top = 96
        Width = 67
        Height = 17
        AutoSize = False
        Caption = '%  per year'
        TabOrder = 2
      end
      object grpAdjustments: TGroupBox
        Left = 6
        Top = 131
        Width = 390
        Height = 95
        Caption = 'Temporal, Spatial and/or Space-Time Adjustments'
        TabOrder = 3
        object lblAdjustmentsByRelativeRisksFile: TLabel
          Left = 12
          Top = 45
          Width = 79
          Height = 13
          Anchors = []
          Caption = 'Adjustments File:'
        end
        object edtAdjustmentsByRelativeRisksFile: TEdit
          Left = 12
          Top = 63
          Width = 335
          Height = 21
          Anchors = []
          ParentShowHint = False
          ShowHint = True
          TabOrder = 1
          OnChange = edtAdjustmentsByRelativeRisksFileChange
        end
        object btnBrowseAdjustmentsFile: TButton
          Left = 353
          Top = 63
          Width = 25
          Height = 21
          Hint = 'browse for adjustments file'
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
          TabOrder = 2
          OnClick = btnBrowseAdjustmentsFileClick
        end
        object chkAdjustForKnownRelativeRisks: TCheckBox
          Left = 12
          Top = 20
          Width = 233
          Height = 17
          Caption = 'Adjust for known relative risks'
          TabOrder = 0
          OnClick = chkAdjustForKnownRelativeRisksClick
        end
      end
    end
    object TabSheet3: TTabSheet
      Caption = 'Inference'
      ImageIndex = 4
      object grpAnalysis: TGroupBox
        Left = 6
        Top = 3
        Width = 390
        Height = 50
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Analysis'
        TabOrder = 0
        object chkTerminateEarly: TCheckBox
          Left = 12
          Top = 22
          Width = 251
          Height = 17
          Anchors = [akLeft, akTop, akRight]
          Caption = 'Terminate replications early for large p-values'
          TabOrder = 0
        end
      end
    end
    object TabSheet1: TTabSheet
      Caption = 'Output'
      ImageIndex = 2
      object lblReportSmallerClusters: TLabel
        Left = 230
        Top = 195
        Width = 139
        Height = 32
        AutoSize = False
        Caption = 'percent of population at risk.'
        WordWrap = True
      end
      object gbxCriteriaSecClusters: TGroupBox
        Left = 5
        Top = 6
        Width = 340
        Height = 50
        Caption = 'Criteria for Reporting Secondary Clusters'
        TabOrder = 0
        object cmbCriteriaSecClusters: TComboBox
          Left = 14
          Top = 18
          Width = 301
          Height = 21
          Style = csDropDownList
          ItemHeight = 13
          TabOrder = 0
          Items.Strings = (
            'No Geographical Overlap'
            'No Cluster Centers in Other Clusters'
            'No Cluster Centers in More Likely Clusters'
            'No Cluster Centers in Less Likely Clusters'
            'No Pairs of Centers Both in Each Others Clusters'
            'No Restrictions = Most Likely Cluster for Each Grid Point')
        end
      end
      object chkRestrictReportedClusters: TCheckBox
        Left = 8
        Top = 194
        Width = 175
        Height = 17
        Caption = 'Report only clusters smaller than'
        TabOrder = 1
        OnClick = chkRestrictReportedClustersClick
      end
      object edtReportClustersSmallerThan: TEdit
        Left = 184
        Top = 193
        Width = 40
        Height = 21
        TabOrder = 2
        Text = '50'
        OnExit = edtReportClustersSmallerThanExit
        OnKeyPress = edtReportClustersSmallerThanKeyPress
      end
    end
  end
  object OpenDialog: TOpenDialog
    Left = 472
    Top = 320
  end
end
