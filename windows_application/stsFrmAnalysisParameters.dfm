object frmAnalysis: TfrmAnalysis
  Left = 224
  Top = 206
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'SaTScan'
  ClientHeight = 325
  ClientWidth = 493
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsMDIChild
  OldCreateOrder = False
  Position = poDefaultPosOnly
  Visible = True
  OnActivate = FormActivate
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object PageControl1: TPageControl
    Left = 2
    Top = 2
    Width = 489
    Height = 321
    ActivePage = tbInputFiles
    TabOrder = 0
    object tbInputFiles: TTabSheet
      Caption = 'Input Files'
      object Label4: TLabel
        Left = 20
        Top = 129
        Width = 72
        Height = 13
        Caption = 'Population File:'
      end
      object Label5: TLabel
        Left = 128
        Top = 129
        Width = 75
        Height = 13
        Caption = '(Poisson Model)'
      end
      object GroupBox1: TGroupBox
        Left = 8
        Top = 8
        Width = 465
        Height = 109
        Color = clBtnFace
        ParentColor = False
        TabOrder = 0
        object Label1: TLabel
          Left = 12
          Top = 12
          Width = 46
          Height = 13
          Caption = 'Case File:'
        end
        object Label2: TLabel
          Left = 12
          Top = 60
          Width = 55
          Height = 13
          Caption = 'Control File:'
          Color = clBtnFace
          ParentColor = False
        end
        object Label3: TLabel
          Left = 120
          Top = 60
          Width = 78
          Height = 13
          Caption = '(Bernoulli Model)'
        end
        object rgPrecisionTimes: TRadioGroup
          Left = 333
          Top = 23
          Width = 121
          Height = 75
          Caption = 'Time Included'
          ItemIndex = 0
          Items.Strings = (
            'Yes'
            'No')
          TabOrder = 0
          OnClick = rgPrecisionTimesClick
        end
        object edtCaseFileName: TEdit
          Left = 12
          Top = 28
          Width = 284
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 1
          OnChange = edtCaseFileNameChange
        end
        object edtControlFileName: TEdit
          Left = 12
          Top = 76
          Width = 284
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
          OnChange = edtControlFileNameChange
        end
        object btnCaseBrowse: TButton
          Left = 300
          Top = 28
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
        object btnControlBrowse: TButton
          Left = 300
          Top = 76
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
          TabOrder = 4
          OnClick = btnControlBrowseClick
        end
      end
      object edtPopFileName: TEdit
        Left = 20
        Top = 145
        Width = 284
        Height = 21
        ParentShowHint = False
        ShowHint = True
        TabOrder = 1
        OnChange = edtPopFileNameChange
      end
      object btnPopBrowse: TButton
        Left = 308
        Top = 145
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
        TabOrder = 2
        OnClick = btnPopBrowseClick
      end
      object GroupBox2: TGroupBox
        Left = 8
        Top = 176
        Width = 465
        Height = 109
        TabOrder = 3
        object Label6: TLabel
          Left = 12
          Top = 12
          Width = 78
          Height = 13
          Caption = 'Coordinates File:'
        end
        object Label7: TLabel
          Left = 12
          Top = 60
          Width = 125
          Height = 13
          Caption = 'Special Grid File: (optional)'
        end
        object edtCoordinateFileName: TEdit
          Left = 12
          Top = 28
          Width = 284
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = edtCoordinateFileNameChange
        end
        object edtGridFileName: TEdit
          Left = 12
          Top = 76
          Width = 284
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 1
          OnChange = edtGridFileNameChange
        end
        object btnCoordBrowse: TButton
          Left = 300
          Top = 28
          Width = 25
          Height = 21
          Hint = 'browse for coordinates file'
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
          OnClick = btnCoordBrowseClick
        end
        object btnGridBrowse: TButton
          Left = 300
          Top = 76
          Width = 25
          Height = 21
          Hint = 'browse for special grid file'
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
          OnClick = btnGridBrowseClick
        end
        object rgCoordinates: TRadioGroup
          Left = 333
          Top = 23
          Width = 121
          Height = 75
          Caption = 'Coordinates'
          ItemIndex = 0
          Items.Strings = (
            'Cartesian'
            'Latitude/Longitude')
          TabOrder = 4
          OnClick = rgCoordinatesClick
        end
      end
    end
    object tbAnalysis: TTabSheet
      Caption = 'Analysis'
      ImageIndex = 1
      object Label16: TLabel
        Left = 40
        Top = 268
        Width = 266
        Height = 13
        Caption = 'Monte Carlo replications (9, 999, or value ending in 999):'
      end
      object edtMontCarloReps: TEdit
        Left = 318
        Top = 265
        Width = 65
        Height = 21
        MaxLength = 9
        TabOrder = 0
        Text = '999'
        OnExit = edtMontCarloRepsExit
        OnKeyPress = NaturalNumberKeyPress
      end
      object GroupBox4: TGroupBox
        Left = 168
        Top = 8
        Width = 162
        Height = 183
        Caption = 'Probability Model'
        TabOrder = 1
        object rdoPoissonModel: TRadioButton
          Left = 12
          Top = 37
          Width = 82
          Height = 17
          Caption = 'Poisson'
          Checked = True
          TabOrder = 0
          TabStop = True
          OnClick = rdoProbabilityModelClick
        end
        object rdoBernoulliModel: TRadioButton
          Left = 12
          Top = 90
          Width = 87
          Height = 17
          Caption = 'Bernoulli'
          TabOrder = 1
          OnClick = rdoProbabilityModelClick
        end
        object rdoSpaceTimePermutationModel: TRadioButton
          Left = 12
          Top = 142
          Width = 140
          Height = 17
          Caption = 'Space-Time Permutation'
          TabOrder = 2
          OnClick = rdoProbabilityModelClick
        end
      end
      object rgpTypeAnalysis: TGroupBox
        Left = 8
        Top = 8
        Width = 154
        Height = 183
        Caption = 'Type of Analysis'
        TabOrder = 2
        object stRetrospectiveAnalyses: TStaticText
          Left = 15
          Top = 21
          Width = 118
          Height = 17
          Caption = 'Retrospective Analyses:'
          TabOrder = 0
        end
        object stProspectiveAnalyses: TStaticText
          Left = 15
          Top = 111
          Width = 108
          Height = 17
          Caption = 'Prospective Analyses:'
          TabOrder = 1
        end
        object rdoRetrospectivePurelySpatial: TRadioButton
          Left = 23
          Top = 40
          Width = 113
          Height = 17
          Caption = 'Purely Spatial'
          Checked = True
          TabOrder = 2
          TabStop = True
          OnClick = rdoAnalysisTypeClick
        end
        object rdoRetrospectivePurelyTemporal: TRadioButton
          Left = 23
          Top = 61
          Width = 113
          Height = 17
          Caption = 'Purely Temporal'
          TabOrder = 3
          OnClick = rdoAnalysisTypeClick
        end
        object rdoRetrospectiveSpaceTime: TRadioButton
          Left = 23
          Top = 83
          Width = 113
          Height = 17
          Caption = 'Space-Time'
          TabOrder = 4
          OnClick = rdoAnalysisTypeClick
        end
        object rdoProspectivePurelyTemporal: TRadioButton
          Left = 23
          Top = 130
          Width = 113
          Height = 17
          Caption = 'Purely Temporal'
          TabOrder = 5
          OnClick = rdoAnalysisTypeClick
        end
        object rdoProspectiveSpaceTime: TRadioButton
          Left = 23
          Top = 151
          Width = 113
          Height = 17
          Caption = 'Space-Time'
          TabOrder = 6
          OnClick = rdoAnalysisTypeClick
        end
      end
      object GroupBox5: TGroupBox
        Left = 336
        Top = 8
        Width = 137
        Height = 183
        Caption = 'Scan for Areas with:'
        TabOrder = 3
        object rdoHighRates: TRadioButton
          Left = 13
          Top = 37
          Width = 113
          Height = 17
          Caption = 'High Rates'
          Checked = True
          TabOrder = 0
          TabStop = True
        end
        object rdoLowRates: TRadioButton
          Left = 13
          Top = 90
          Width = 113
          Height = 17
          Caption = 'Low Rates'
          TabOrder = 1
        end
        object rdoHighLowRates: TRadioButton
          Left = 13
          Top = 142
          Width = 113
          Height = 17
          Caption = 'High or Low Rates'
          TabOrder = 2
        end
      end
    end
    object tbTimeParameter: TTabSheet
      Caption = 'Time Parameters'
      ImageIndex = 3
      object rdgTimeIntervalUnits: TGroupBox
        Left = 8
        Top = 8
        Width = 209
        Height = 137
        Caption = 'Time Intervals'
        TabOrder = 0
        TabStop = True
        object lblTimeIntervalUnits: TLabel
          Left = 14
          Top = 24
          Width = 33
          Height = 13
          Caption = 'Units:  '
        end
        object lblTimeIntervalLength: TLabel
          Left = 10
          Top = 104
          Width = 129
          Height = 13
          Caption = 'Length (in specified units):  '
        end
        object rbUnitYear: TRadioButton
          Left = 54
          Top = 24
          Width = 57
          Height = 17
          Caption = 'Year'
          Checked = True
          Enabled = False
          TabOrder = 0
          TabStop = True
          OnClick = rbUnitYearClick
        end
        object rbUnitMonths: TRadioButton
          Left = 54
          Top = 48
          Width = 57
          Height = 17
          Caption = 'Month'
          Enabled = False
          TabOrder = 1
          OnClick = rbUnitMonthsClick
        end
        object rbUnitDay: TRadioButton
          Left = 54
          Top = 72
          Width = 57
          Height = 17
          Caption = 'Day'
          Enabled = False
          TabOrder = 2
          OnClick = rbUnitDayClick
        end
        object edtTimeIntervalLength: TEdit
          Left = 146
          Top = 96
          Width = 25
          Height = 21
          Enabled = False
          MaxLength = 3
          TabOrder = 3
          Text = '1'
          OnExit = edtTimeIntervalLengthExit
          OnKeyPress = NaturalNumberKeyPress
        end
      end
      object grpProspectiveSurveillance: TGroupBox
        Left = 224
        Top = 8
        Width = 249
        Height = 137
        Caption = 'Prospective Surveillance'
        TabOrder = 1
        TabStop = True
        object lblProspectiveStartYear: TLabel
          Left = 41
          Top = 78
          Width = 22
          Height = 13
          Caption = 'Year'
        end
        object lblProspectiveStartMonth: TLabel
          Left = 97
          Top = 78
          Width = 30
          Height = 13
          Caption = 'Month'
        end
        object lblProspectiveStartDay: TLabel
          Left = 144
          Top = 78
          Width = 19
          Height = 13
          Caption = 'Day'
        end
        object lblProspectiveStartDate: TLabel
          Left = 36
          Top = 58
          Width = 81
          Height = 13
          Caption = ' performed since:'
        end
        object lblAdjustForEarlierAnalyses: TLabel
          Left = 36
          Top = 30
          Width = 119
          Height = 13
          Caption = 'Adjust for earlier analyses'
        end
        object edtProspectiveStartDateYear: TEdit
          Left = 38
          Top = 97
          Width = 41
          Height = 21
          Enabled = False
          MaxLength = 4
          TabOrder = 0
          Text = '1900'
          OnExit = edtProspectiveStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtProspectiveStartDateMonth: TEdit
          Left = 98
          Top = 97
          Width = 25
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 1
          Text = '12'
          OnExit = edtProspectiveStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtProspectiveStartDateDay: TEdit
          Left = 144
          Top = 97
          Width = 25
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 2
          Text = '31'
          OnExit = edtProspectiveStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object chkAdjustForEarlierAnalyses: TCheckBox
          Left = 14
          Top = 28
          Width = 15
          Height = 17
          TabOrder = 3
          OnClick = chkAdjustForEarlierAnalysesClick
        end
      end
      object GroupBox3: TGroupBox
        Left = 8
        Top = 148
        Width = 466
        Height = 97
        Caption = 'Study Period'
        TabOrder = 2
        object Label8: TLabel
          Left = 56
          Top = 39
          Width = 51
          Height = 13
          Anchors = [akLeft, akTop, akRight]
          Caption = 'Start Date:'
        end
        object Label9: TLabel
          Left = 56
          Top = 71
          Width = 48
          Height = 13
          Anchors = [akLeft, akTop, akRight]
          Caption = 'End Date:'
        end
        object Label10: TLabel
          Left = 120
          Top = 13
          Width = 22
          Height = 13
          Anchors = [akLeft, akTop, akRight]
          Caption = 'Year'
        end
        object Label12: TLabel
          Left = 171
          Top = 13
          Width = 30
          Height = 13
          Anchors = [akLeft, akTop, akRight]
          Caption = 'Month'
        end
        object Label14: TLabel
          Left = 208
          Top = 13
          Width = 19
          Height = 13
          Anchors = [akLeft, akTop, akRight]
          Caption = 'Day'
        end
        object edtStudyPeriodStartDateYear: TEdit
          Left = 120
          Top = 31
          Width = 42
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          MaxLength = 4
          TabOrder = 0
          Text = '1900'
          OnExit = edtStudyPeriodStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodEndDateYear: TEdit
          Left = 120
          Top = 63
          Width = 42
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          MaxLength = 4
          TabOrder = 1
          Text = '1900'
          OnExit = edtStudyPeriodEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodStartDateMonth: TEdit
          Left = 171
          Top = 31
          Width = 26
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 2
          Text = '1'
          OnExit = edtStudyPeriodStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodEndDateMonth: TEdit
          Left = 171
          Top = 63
          Width = 26
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 3
          Text = '12'
          OnExit = edtStudyPeriodEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodStartDateDay: TEdit
          Left = 208
          Top = 31
          Width = 26
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 4
          Text = '1'
          OnExit = edtStudyPeriodStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodEndDateDay: TEdit
          Left = 208
          Top = 63
          Width = 26
          Height = 21
          Anchors = [akLeft, akTop, akRight]
          Enabled = False
          MaxLength = 2
          TabOrder = 5
          Text = '31'
          OnExit = edtStudyPeriodEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
      end
    end
    object tbScanningWindow: TTabSheet
      Caption = 'Scanning Window'
      ImageIndex = 2
      object rdgSpatialOptions: TGroupBox
        Left = 8
        Top = 8
        Width = 465
        Height = 87
        Caption = 'Spatial'
        TabOrder = 0
        object lblMaxSpatialClusterSize: TLabel
          Left = 18
          Top = 22
          Width = 146
          Height = 13
          Caption = 'Maximum Spatial Cluster Size:  '
        end
        object edtMaxSpatialClusterSize: TEdit
          Left = 178
          Top = 16
          Width = 38
          Height = 21
          AutoSelect = False
          MaxLength = 5
          TabOrder = 0
          Text = '50'
          OnChange = edtMaxSpatialClusterSizeChange
          OnExit = edtMaxSpatialClusterSizeExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object chkInclPurTempClust: TCheckBox
          Left = 18
          Top = 57
          Width = 305
          Height = 17
          Caption = 'Include Purely Temporal Cluster (Spatial Size = 100%)'
          Enabled = False
          TabOrder = 1
        end
        object rdoSpatialPercentage: TRadioButton
          Left = 226
          Top = 14
          Width = 217
          Height = 17
          Caption = 'Percent of Population at Risk (<= 50%)'
          Checked = True
          TabOrder = 2
          TabStop = True
          OnClick = rdoMaximumSpatialTypeClick
        end
        object rdoSpatialDistance: TRadioButton
          Left = 226
          Top = 34
          Width = 217
          Height = 17
          Caption = 'Kilometer Radius'
          TabOrder = 3
          OnClick = rdoMaximumSpatialTypeClick
        end
      end
      object rdgTemporalOptions: TGroupBox
        Left = 8
        Top = 105
        Width = 465
        Height = 89
        Caption = 'Temporal'
        TabOrder = 1
        object lblMaxTemporalClusterSize: TLabel
          Left = 18
          Top = 22
          Width = 158
          Height = 13
          Caption = 'Maximum Temporal Cluster Size:  '
        end
        object edtMaxTemporalClusterSize: TEdit
          Left = 178
          Top = 16
          Width = 38
          Height = 21
          MaxLength = 5
          TabOrder = 0
          Text = '50'
          OnExit = edtMaxTemporalClusterSizeExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object chkIncludePurSpacClust: TCheckBox
          Left = 18
          Top = 59
          Width = 297
          Height = 17
          Caption = 'Include Purely Spatial Clusters (Temporal Size = 100%)'
          Enabled = False
          TabOrder = 1
        end
        object rdoPercentageTemproal: TRadioButton
          Left = 226
          Top = 14
          Width = 217
          Height = 17
          Caption = 'Percent of Study Period (<= 90%)'
          Checked = True
          TabOrder = 2
          TabStop = True
        end
        object rdoTimeTemproal: TRadioButton
          Left = 226
          Top = 34
          Width = 217
          Height = 17
          Caption = 'Years'
          TabOrder = 3
        end
      end
    end
    object tbOutputFiles: TTabSheet
      Caption = 'Output'
      ImageIndex = 4
      object grpAdditionalOutputFiles: TGroupBox
        Left = 8
        Top = 60
        Width = 465
        Height = 153
        Caption = 'Additional Optional Output Files:'
        TabOrder = 0
        TabStop = True
        object lblClustersInColumnFormat: TLabel
          Left = 16
          Top = 48
          Width = 87
          Height = 13
          Caption = 'Cluster Information'
        end
        object lblCensusAreasReportedClusters: TLabel
          Left = 16
          Top = 72
          Width = 96
          Height = 13
          Caption = 'Location Information'
        end
        object lblRelativeRiskEstimatesArea: TLabel
          Left = 16
          Top = 96
          Width = 155
          Height = 13
          Caption = 'Relative Risks for Each Location'
        end
        object lblSimulatedLogLikelihoodRatios: TLabel
          Left = 16
          Top = 120
          Width = 222
          Height = 13
          Caption = 'Simulated Log Likelihood Ratios/Test Statistics'
        end
        object ASCII: TLabel
          Left = 338
          Top = 23
          Width = 27
          Height = 13
          Caption = 'ASCII'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsUnderline]
          ParentFont = False
        end
        object lbldBaseOutput: TLabel
          Left = 407
          Top = 23
          Width = 30
          Height = 13
          Caption = 'dBase'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsUnderline]
          ParentFont = False
        end
        object chkCensusAreasReportedClustersAscii: TCheckBox
          Left = 344
          Top = 72
          Width = 15
          Height = 17
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
        end
        object chkClustersInColumnFormatAscii: TCheckBox
          Left = 344
          Top = 48
          Width = 15
          Height = 17
          BiDiMode = bdLeftToRight
          ParentBiDiMode = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
        end
        object chkRelativeRiskEstimatesAreaAscii: TCheckBox
          Left = 344
          Top = 96
          Width = 16
          Height = 17
          ParentShowHint = False
          ShowHint = True
          TabOrder = 4
        end
        object chkSimulatedLogLikelihoodRatiosAscii: TCheckBox
          Left = 344
          Top = 120
          Width = 16
          Height = 17
          TabOrder = 6
        end
        object chkClustersInColumnFormatDBase: TCheckBox
          Left = 414
          Top = 48
          Width = 16
          Height = 17
          BiDiMode = bdLeftToRight
          ParentBiDiMode = False
          TabOrder = 1
        end
        object chkCensusAreasReportedClustersDBase: TCheckBox
          Left = 414
          Top = 72
          Width = 16
          Height = 17
          BiDiMode = bdLeftToRight
          ParentBiDiMode = False
          TabOrder = 3
        end
        object chkRelativeRiskEstimatesAreaDBase: TCheckBox
          Left = 415
          Top = 96
          Width = 16
          Height = 17
          TabOrder = 5
        end
        object chkSimulatedLogLikelihoodRatiosDBase: TCheckBox
          Left = 415
          Top = 120
          Width = 16
          Height = 17
          TabOrder = 7
        end
      end
      object grpCriteriaSecClusters: TGroupBox
        Left = 8
        Top = 222
        Width = 465
        Height = 50
        Caption = 'Criteria for Reporting Secondary Clusters'
        TabOrder = 1
        TabStop = True
        object cboCriteriaSecClusters: TComboBox
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
      object pnlTop: TPanel
        Left = 0
        Top = 0
        Width = 481
        Height = 58
        Align = alTop
        BevelOuter = bvNone
        TabOrder = 2
        TabStop = True
        object Label29: TLabel
          Left = 8
          Top = 8
          Width = 57
          Height = 13
          Caption = 'Results File:'
        end
        object edtResultFile: TEdit
          Left = 8
          Top = 24
          Width = 427
          Height = 21
          AutoSelect = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = edtResultFileChange
        end
        object btnResultFileBrowse: TButton
          Left = 441
          Top = 24
          Width = 30
          Height = 21
          Hint = 'browse for results file'
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
          OnClick = btnResultFileBrowseClick
        end
      end
    end
  end
  object OpenDialog1: TOpenDialog
    Left = 408
  end
  object SaveDialog: TSaveDialog
    DefaultExt = '*.prm'
    Filter = 'Parameter Files (*.prm)|*.prm'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofEnableSizing]
    Title = 'Save Parameter File As'
    Left = 440
  end
end
