object frmAdvancedParameters: TfrmAdvancedParameters
  Left = 189
  Top = 131
  ActiveControl = PageControl
  BorderStyle = bsDialog
  Caption = 'Advanced Features'
  ClientHeight = 307
  ClientWidth = 511
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
    Left = 427
    Top = 0
    Width = 84
    Height = 307
    Align = alRight
    BevelOuter = bvNone
    TabOrder = 1
    object btnOk: TButton
      Left = 6
      Top = 28
      Width = 72
      Height = 24
      Caption = '&Close'
      Default = True
      ModalResult = 1
      TabOrder = 0
    end
    object btnShowAll: TButton
      Left = 6
      Top = 214
      Width = 72
      Height = 24
      Caption = '&Show All'
      TabOrder = 1
      Visible = False
      OnClick = btnShowAllClick
    end
    object btnSetDefaults: TButton
      Left = 6
      Top = 60
      Width = 72
      Height = 24
      Caption = 'Set Defaults'
      Enabled = False
      TabOrder = 2
      OnClick = btnSetDefaultsClick
    end
  end
  object PageControl: TPageControl
    Left = 2
    Top = 6
    Width = 423
    Height = 299
    ActivePage = tsInput
    TabOrder = 0
    object tsInput: TTabSheet
      Caption = 'Input'
      ImageIndex = 5
      object GroupBox1: TGroupBox
        Left = 8
        Top = 8
        Width = 399
        Height = 141
        Caption = 'Additional Input Stream Detail'
        TabOrder = 0
        object Label3: TLabel
          Left = 16
          Top = 16
          Width = 24
          Height = 13
          Caption = 'Case'
        end
        object Label1: TLabel
          Left = 16
          Top = 54
          Width = 33
          Height = 13
          Caption = 'Control'
        end
        object Label2: TLabel
          Left = 16
          Top = 92
          Width = 50
          Height = 13
          Caption = 'Population'
        end
      end
      object edtCaseFileName: TEdit
        Left = 24
        Top = 36
        Width = 336
        Height = 21
        ParentShowHint = False
        ShowHint = True
        TabOrder = 2
      end
      object btnCaseBrowse: TButton
        Left = 368
        Top = 36
        Width = 25
        Height = 21
        Hint = 'browse for case file'
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
        ParentShowHint = False
        ShowHint = True
        TabOrder = 3
        OnClick = btnCaseBrowseClick
      end
      object edtControlFileName: TEdit
        Left = 24
        Top = 74
        Width = 336
        Height = 21
        ParentShowHint = False
        ShowHint = True
        TabOrder = 4
      end
      object btnControlBrowse: TButton
        Left = 368
        Top = 74
        Width = 25
        Height = 21
        Hint = 'browse for control file'
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
        ParentShowHint = False
        ShowHint = True
        TabOrder = 5
        OnClick = btnControlBrowseClick
      end
      object edtPopFileName: TEdit
        Left = 24
        Top = 113
        Width = 336
        Height = 21
        ParentShowHint = False
        ShowHint = True
        TabOrder = 6
      end
      object btnPopBrowse: TButton
        Left = 368
        Top = 113
        Width = 25
        Height = 21
        Hint = 'browse for population file'
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
        ParentShowHint = False
        ShowHint = True
        TabOrder = 7
        OnClick = btnPopBrowseClick
      end
      object GroupBox2: TGroupBox
        Left = 8
        Top = 156
        Width = 399
        Height = 105
        Caption = 'All Additional Input Streams'
        TabOrder = 1
        object btnAddStream: TButton
          Left = 312
          Top = 28
          Width = 75
          Height = 25
          Caption = 'Add'
          TabOrder = 1
          OnClick = btnAddClick
        end
        object btnRemoveStream: TButton
          Left = 312
          Top = 64
          Width = 75
          Height = 25
          Caption = 'Remove'
          TabOrder = 2
          OnClick = btnRemoveStreamClick
        end
        object lstInputStreams: TListBox
          Left = 16
          Top = 24
          Width = 285
          Height = 69
          ItemHeight = 13
          TabOrder = 0
          OnClick = lstInputStreamsClick
        end
      end
    end
    object TabSheet2: TTabSheet
      Caption = 'Spatial Window'
      ImageIndex = 3
      object rdgSpatialOptions: TGroupBox
        Left = 8
        Top = 8
        Width = 385
        Height = 177
        Caption = 'Spatial'
        TabOrder = 0
        object lblMaxSpatialClusterSize: TLabel
          Left = 18
          Top = 18
          Width = 168
          Height = 13
          Caption = 'The Maximum Spatial Cluster Size:  '
        end
        object lblPercentOfPopulation: TLabel
          Left = 96
          Top = 38
          Width = 263
          Height = 13
          Caption = 'percent of the population at risk (<= 50%, default = 50%)'
        end
        object lblMaxRadius: TLabel
          Left = 166
          Top = 124
          Width = 73
          Height = 13
          Caption = 'kilometer radius'
        end
        object lblPercentageOfPopFile: TLabel
          Left = 96
          Top = 62
          Width = 259
          Height = 26
          Caption = 
            'percent of the population defined in the max circle size file (<' +
            '= 50%)'
          WordWrap = True
        end
        object edtMaxSpatialClusterSize: TEdit
          Left = 64
          Top = 35
          Width = 25
          Height = 21
          AutoSelect = False
          MaxLength = 5
          TabOrder = 1
          Text = '50'
          OnChange = edtMaxSpatialClusterSizeChange
          OnExit = edtMaxSpatialClusterSizeExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object chkInclPureTempClust: TCheckBox
          Left = 18
          Top = 149
          Width = 305
          Height = 17
          Caption = 'Include Purely Temporal Cluster (Spatial Size = 100%)'
          Enabled = False
          TabOrder = 8
          OnClick = OnControlExit
        end
        object rdoSpatialPercentage: TRadioButton
          Left = 34
          Top = 36
          Width = 28
          Height = 17
          Caption = 'is '
          Checked = True
          TabOrder = 0
          TabStop = True
          OnClick = rdoMaxSpatialTypeClick
        end
        object rdoSpatialDistance: TRadioButton
          Left = 34
          Top = 124
          Width = 97
          Height = 17
          Caption = 'is a circle with a'
          TabOrder = 6
          OnClick = rdoMaxSpatialTypeClick
        end
        object rdoSpatialPopulationFile: TRadioButton
          Left = 34
          Top = 62
          Width = 28
          Height = 17
          Caption = 'is'
          TabOrder = 2
          OnClick = rdoMaxSpatialTypeClick
        end
        object edtMaxSpatialRadius: TEdit
          Left = 133
          Top = 122
          Width = 25
          Height = 21
          TabOrder = 7
          Text = '1'
          OnChange = edtMaxSpatialRadiusChange
          OnExit = edtMaxSpatialRadiusExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtMaxSpatialPercentFile: TEdit
          Left = 64
          Top = 60
          Width = 25
          Height = 21
          TabOrder = 3
          Text = '50'
          OnChange = edtMaxSpatialPercentFileChange
          OnExit = edtMaxSpatialPercentFileExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtMaxCirclePopulationFilename: TEdit
          Left = 53
          Top = 94
          Width = 278
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 4
          OnChange = edtMaxCirclePopulationFilenameChange
          OnExit = OnControlExit
        end
        object btnBrowseMaxCirclePopFile: TButton
          Left = 341
          Top = 94
          Width = 25
          Height = 21
          Hint = 'browse for special population file'
          Caption = '...'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsBold]
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 5
          OnClick = btnBrowseMaxCirclePopFileClick
        end
      end
    end
    object tsOther: TTabSheet
      Caption = 'Temporal Window'
      ImageIndex = 1
      object grpScanningWindow: TGroupBox
        Left = 8
        Top = 134
        Width = 380
        Height = 111
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
          Left = 245
          Top = 49
          Width = 19
          Height = 17
          Caption = 'to: '
          TabOrder = 5
        end
        object edtStartRangeStartYear: TEdit
          Left = 142
          Top = 46
          Width = 31
          Height = 21
          MaxLength = 4
          TabOrder = 2
          Text = '1900'
          OnExit = edtStartRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeStartMonth: TEdit
          Left = 182
          Top = 46
          Width = 20
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 3
          Text = '1'
          OnExit = edtStartRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeStartDay: TEdit
          Left = 211
          Top = 46
          Width = 20
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 4
          Text = '1'
          OnExit = edtStartRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeEndYear: TEdit
          Left = 268
          Top = 46
          Width = 31
          Height = 21
          MaxLength = 4
          TabOrder = 6
          Text = '1900'
          OnExit = edtStartRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeEndMonth: TEdit
          Left = 308
          Top = 46
          Width = 20
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 7
          Text = '1'
          OnExit = edtStartRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartRangeEndDay: TEdit
          Left = 337
          Top = 46
          Width = 20
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 8
          Text = '1'
          OnExit = edtStartRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object stStartWindowRange: TStaticText
          Left = 30
          Top = 49
          Width = 91
          Height = 17
          AutoSize = False
          Caption = 'Start time in range:'
          TabOrder = 1
        end
        object stEndWindowRange: TStaticText
          Left = 30
          Top = 81
          Width = 89
          Height = 17
          Caption = 'End time in range:'
          TabOrder = 9
        end
        object edtEndRangeStartYear: TEdit
          Left = 142
          Top = 78
          Width = 31
          Height = 21
          MaxLength = 4
          TabOrder = 10
          Text = '1900'
          OnExit = edtEndRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtEndRangeStartMonth: TEdit
          Left = 182
          Top = 78
          Width = 20
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 11
          Text = '12'
          OnExit = edtEndRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtEndRangeStartDay: TEdit
          Left = 211
          Top = 78
          Width = 20
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 12
          Text = '31'
          OnExit = edtEndRangeStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object stEndRangeTo: TStaticText
          Left = 245
          Top = 81
          Width = 19
          Height = 17
          Caption = 'to: '
          TabOrder = 13
        end
        object edtEndRangeEndYear: TEdit
          Left = 268
          Top = 78
          Width = 31
          Height = 21
          MaxLength = 4
          TabOrder = 14
          Text = '1900'
          OnExit = edtEndRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtEndRangeEndMonth: TEdit
          Left = 308
          Top = 78
          Width = 20
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 15
          Text = '12'
          OnExit = edtEndRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtEndRangeEndDay: TEdit
          Left = 337
          Top = 78
          Width = 20
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 16
          Text = '31'
          OnExit = edtEndRangeEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
      end
      object rdgTemporalOptions: TGroupBox
        Left = 8
        Top = 8
        Width = 380
        Height = 112
        Caption = 'Temporal Cluster'
        TabOrder = 1
        object lblMaxTemporalClusterSize: TLabel
          Left = 18
          Top = 18
          Width = 180
          Height = 13
          Caption = 'The Maximum Temporal Cluster Size:  '
        end
        object lblPercentageOfStudyPeriod: TLabel
          Left = 108
          Top = 37
          Width = 240
          Height = 13
          Caption = 'percent of the study period (<= 90%, default = 50%)'
        end
        object lblMaxTemporalTimeUnits: TLabel
          Left = 108
          Top = 62
          Width = 25
          Height = 13
          Caption = 'years'
        end
        object edtMaxTemporalClusterSize: TEdit
          Left = 62
          Top = 35
          Width = 38
          Height = 21
          MaxLength = 5
          TabOrder = 1
          Text = '50'
          OnExit = edtMaxTemporalClusterSizeExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object chkIncludePureSpacClust: TCheckBox
          Left = 18
          Top = 87
          Width = 297
          Height = 17
          Caption = 'Include Purely Spatial Clusters (Temporal Size = 100%)'
          Enabled = False
          TabOrder = 3
          OnClick = OnControlExit
        end
        object rdoPercentageTemporal: TRadioButton
          Left = 30
          Top = 36
          Width = 31
          Height = 17
          Caption = 'is'
          Checked = True
          TabOrder = 0
          TabStop = True
          OnClick = rdoMaxTemporalClusterSizelick
        end
        object rdoTimeTemporal: TRadioButton
          Left = 30
          Top = 62
          Width = 28
          Height = 17
          Caption = 'is'
          TabOrder = 2
          OnClick = rdoMaxTemporalClusterSizelick
        end
        object edtMaxTemporalClusterSizeUnits: TEdit
          Left = 62
          Top = 60
          Width = 38
          Height = 21
          MaxLength = 5
          TabOrder = 4
          Text = '1'
          OnExit = edtMaxTemporalClusterSizeUnitsExit
          OnKeyPress = NaturalNumberKeyPress
        end
      end
    end
    object tsAdjustments: TTabSheet
      Caption = 'Risk Adjustments'
      object rdgTemporalTrendAdj: TRadioGroup
        Left = 8
        Top = 8
        Width = 390
        Height = 113
        Caption = 'Temporal Adjustments'
        Enabled = False
        ItemIndex = 0
        Items.Strings = (
          'None'
          'Nonparametric, with time stratified randomization'
          'Log linear with')
        TabOrder = 0
        OnClick = rdgTemporalTrendAdjClick
        OnExit = OnControlExit
      end
      object edtLogLinear: TEdit
        Left = 108
        Top = 88
        Width = 25
        Height = 21
        Enabled = False
        MaxLength = 5
        TabOrder = 1
        Text = '0'
        OnExit = edtLogLinearExit
        OnKeyPress = FloatKeyPress
      end
      object lblLogLinear: TStaticText
        Left = 140
        Top = 92
        Width = 67
        Height = 17
        AutoSize = False
        Caption = '%  per year'
        TabOrder = 2
      end
      object grpAdjustments: TGroupBox
        Left = 8
        Top = 155
        Width = 390
        Height = 102
        Caption = 'Temporal, Spatial and/or Space-Time Adjustments'
        TabOrder = 3
        object lblAdjustmentsByRelativeRisksFile: TLabel
          Left = 12
          Top = 49
          Width = 79
          Height = 13
          Anchors = []
          Caption = 'Adjustments File:'
        end
        object edtAdjustmentsByRelativeRisksFile: TEdit
          Left = 12
          Top = 68
          Width = 335
          Height = 21
          Anchors = []
          ParentShowHint = False
          ShowHint = True
          TabOrder = 1
          OnChange = edtAdjustmentsByRelativeRisksFileChange
          OnExit = OnControlExit
        end
        object btnBrowseAdjustmentsFile: TButton
          Left = 355
          Top = 68
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
    object tsInference: TTabSheet
      Caption = 'Inference'
      ImageIndex = 4
      object grpAnalysis: TGroupBox
        Left = 8
        Top = 8
        Width = 373
        Height = 50
        Caption = 'Analysis'
        TabOrder = 0
        object chkTerminateEarly: TCheckBox
          Left = 12
          Top = 22
          Width = 245
          Height = 17
          Caption = 'Terminate replications early for large p-values'
          TabOrder = 0
          OnClick = OnControlExit
        end
      end
      object gbxProspectiveSurveillance: TGroupBox
        Left = 8
        Top = 76
        Width = 373
        Height = 137
        Caption = 'Prospective Surveillance'
        TabOrder = 1
        object lblProspectiveStartYear: TLabel
          Left = 40
          Top = 86
          Width = 19
          Height = 11
          Caption = 'Year'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -9
          Font.Name = 'Small Fonts'
          Font.Style = []
          ParentFont = False
        end
        object lblProspectiveStartMonth: TLabel
          Left = 92
          Top = 86
          Width = 28
          Height = 11
          Caption = 'Month'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -9
          Font.Name = 'Small Fonts'
          Font.Style = []
          ParentFont = False
        end
        object lblProspectiveStartDay: TLabel
          Left = 143
          Top = 86
          Width = 16
          Height = 11
          Caption = 'Day'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -9
          Font.Name = 'Small Fonts'
          Font.Style = []
          ParentFont = False
        end
        object lblProspectiveStartDate: TLabel
          Left = 31
          Top = 58
          Width = 81
          Height = 13
          Caption = ' performed since:'
        end
        object edtProspectiveStartDateYear: TEdit
          Left = 33
          Top = 97
          Width = 41
          Height = 21
          Enabled = False
          MaxLength = 4
          TabOrder = 1
          Text = '1900'
          OnExit = edtProspectiveStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtProspectiveStartDateMonth: TEdit
          Left = 93
          Top = 97
          Width = 25
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 2
          Text = '12'
          OnExit = edtProspectiveStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtProspectiveStartDateDay: TEdit
          Left = 139
          Top = 97
          Width = 25
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 3
          Text = '31'
          OnExit = edtProspectiveStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object chkAdjustForEarlierAnalyses: TCheckBox
          Left = 14
          Top = 28
          Width = 171
          Height = 17
          Caption = 'Adjust for earlier analyses'
          TabOrder = 0
          OnClick = chkAdjustForEarlierAnalysesClick
        end
      end
    end
    object tsOutput: TTabSheet
      Caption = 'Output'
      ImageIndex = 2
      object lblReportSmallerClusters: TLabel
        Left = 238
        Top = 226
        Width = 151
        Height = 27
        AutoSize = False
        Caption = 'percent of population at risk.'
        WordWrap = True
      end
      object chkRestrictReportedClusters: TCheckBox
        Left = 20
        Top = 226
        Width = 175
        Height = 17
        Caption = 'Report only clusters smaller than'
        TabOrder = 0
        OnClick = chkRestrictReportedClustersClick
      end
      object edtReportClustersSmallerThan: TEdit
        Left = 200
        Top = 224
        Width = 30
        Height = 21
        TabOrder = 1
        Text = '50'
        OnExit = edtReportClustersSmallerThanExit
        OnKeyPress = edtReportClustersSmallerThanKeyPress
      end
      object rdgCriteriaSecClusters: TRadioGroup
        Left = 16
        Top = 8
        Width = 377
        Height = 201
        Caption = 'Criteria for Reporting Secondary Clusters'
        Items.Strings = (
          'No Geographical Overlap'
          'No Cluster Centers in Other Clusters'
          'No Cluster Centers in More Likely Clusters'
          'No Cluster Centers in Less Likely Clusters'
          'No Pairs of Centers Both in Each Others Clusters'
          'No Restrictions = Most Likely Cluster for Each Grid Point')
        TabOrder = 2
        OnExit = OnControlExit
      end
    end
  end
  object OpenDialog: TOpenDialog
    Left = 472
    Top = 256
  end
end
