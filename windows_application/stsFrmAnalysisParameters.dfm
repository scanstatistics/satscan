object frmAnalysis: TfrmAnalysis
  Left = 201
  Top = 83
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'SaTScan Analysis Settings'
  ClientHeight = 431
  ClientWidth = 497
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
    Left = 0
    Top = 12
    Width = 489
    Height = 417
    ActivePage = tbInputFiles
    TabOrder = 0
    object tbInputFiles: TTabSheet
      Caption = 'Input'
      object Label4: TLabel
        Left = 15
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
        Width = 461
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
        object rgpPrecisionTimes: TRadioGroup
          Left = 341
          Top = 23
          Width = 92
          Height = 75
          Caption = 'Time Included'
          ItemIndex = 0
          Items.Strings = (
            'Yes'
            'No')
          TabOrder = 4
          OnClick = rgpPrecisionTimesClick
        end
        object edtCaseFileName: TEdit
          Left = 8
          Top = 28
          Width = 284
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = edtCaseFileNameChange
        end
        object edtControlFileName: TEdit
          Left = 8
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
          TabOrder = 1
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
          TabOrder = 3
          OnClick = btnControlBrowseClick
        end
      end
      object edtPopFileName: TEdit
        Left = 15
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
          Left = 8
          Top = 12
          Width = 78
          Height = 13
          Caption = 'Coordinates File:'
        end
        object Label7: TLabel
          Left = 8
          Top = 60
          Width = 125
          Height = 13
          Caption = 'Special Grid File: (optional)'
        end
        object edtCoordinateFileName: TEdit
          Left = 8
          Top = 28
          Width = 284
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = edtCoordinateFileNameChange
        end
        object edtGridFileName: TEdit
          Left = 8
          Top = 76
          Width = 284
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
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
          TabOrder = 1
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
        object rgpCoordinates: TRadioGroup
          Left = 333
          Top = 23
          Width = 124
          Height = 75
          Caption = 'Coordinates'
          ItemIndex = 0
          Items.Strings = (
            'Cartesian'
            'Latitude/Longitude')
          TabOrder = 4
          OnClick = rgpCoordinatesClick
        end
      end
      object gbxStudyPeriod: TGroupBox
        Left = 8
        Top = 292
        Width = 337
        Height = 85
        Caption = 'Study Period'
        TabOrder = 4
        object Label8: TLabel
          Left = 25
          Top = 31
          Width = 51
          Height = 13
          Caption = 'Start Date:'
        end
        object Label9: TLabel
          Left = 25
          Top = 59
          Width = 48
          Height = 13
          Caption = 'End Date:'
        end
        object Label10: TLabel
          Left = 88
          Top = 13
          Width = 22
          Height = 13
          Caption = 'Year'
        end
        object Label12: TLabel
          Left = 125
          Top = 13
          Width = 30
          Height = 13
          Caption = 'Month'
        end
        object Label14: TLabel
          Left = 162
          Top = 13
          Width = 19
          Height = 13
          Caption = 'Day'
        end
        object edtStudyPeriodStartDateYear: TEdit
          Left = 84
          Top = 27
          Width = 35
          Height = 21
          MaxLength = 4
          TabOrder = 0
          Text = '1900'
          OnExit = edtStudyPeriodStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodEndDateYear: TEdit
          Left = 84
          Top = 55
          Width = 35
          Height = 21
          MaxLength = 4
          TabOrder = 3
          Text = '1900'
          OnExit = edtStudyPeriodEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodStartDateMonth: TEdit
          Left = 127
          Top = 27
          Width = 25
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 1
          Text = '1'
          OnExit = edtStudyPeriodStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodEndDateMonth: TEdit
          Left = 127
          Top = 55
          Width = 25
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 4
          Text = '12'
          OnExit = edtStudyPeriodEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodStartDateDay: TEdit
          Left = 160
          Top = 27
          Width = 25
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 2
          Text = '1'
          OnExit = edtStudyPeriodStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodEndDateDay: TEdit
          Left = 160
          Top = 55
          Width = 25
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 5
          Text = '31'
          OnExit = edtStudyPeriodEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
      end
      object btnImportFile: TButton
        Left = 400
        Top = 344
        Width = 72
        Height = 24
        Caption = 'Import File'
        TabOrder = 5
        OnClick = btnImportFileClick
      end
    end
    object tbAnalysis: TTabSheet
      Caption = 'Analysis'
      ImageIndex = 1
      object Label16: TLabel
        Left = 48
        Top = 284
        Width = 266
        Height = 13
        Caption = 'Monte Carlo replications (9, 999, or value ending in 999):'
      end
      object edtMontCarloReps: TEdit
        Left = 330
        Top = 281
        Width = 65
        Height = 21
        BiDiMode = bdLeftToRight
        MaxLength = 9
        ParentBiDiMode = False
        TabOrder = 3
        Text = '999'
        OnExit = edtMontCarloRepsExit
        OnKeyPress = NaturalNumberKeyPress
      end
      object rgpTypeProbability: TGroupBox
        Left = 164
        Top = 8
        Width = 161
        Height = 121
        Caption = 'Probability Model'
        TabOrder = 1
        object rdoPoissonModel: TRadioButton
          Left = 13
          Top = 25
          Width = 82
          Height = 17
          Caption = 'Poisson'
          Checked = True
          TabOrder = 0
          TabStop = True
          OnClick = rdoProbabilityModelClick
        end
        object rdoBernoulliModel: TRadioButton
          Left = 13
          Top = 55
          Width = 87
          Height = 17
          Caption = 'Bernoulli'
          TabOrder = 1
          OnClick = rdoProbabilityModelClick
        end
        object rdoSpaceTimePermutationModel: TRadioButton
          Left = 13
          Top = 85
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
        Width = 149
        Height = 241
        Caption = 'Type of Analysis'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        object stRetrospectiveAnalyses: TStaticText
          Left = 12
          Top = 25
          Width = 118
          Height = 17
          Caption = 'Retrospective Analyses:'
          TabOrder = 0
        end
        object stProspectiveAnalyses: TStaticText
          Left = 12
          Top = 147
          Width = 108
          Height = 17
          Caption = 'Prospective Analyses:'
          TabOrder = 4
        end
        object rdoRetrospectivePurelySpatial: TRadioButton
          Left = 17
          Top = 50
          Width = 113
          Height = 17
          Caption = 'Purely Spatial'
          Checked = True
          TabOrder = 1
          TabStop = True
          OnClick = rdoAnalysisTypeClick
        end
        object rdoRetrospectivePurelyTemporal: TRadioButton
          Left = 17
          Top = 80
          Width = 113
          Height = 17
          Caption = 'Purely Temporal'
          TabOrder = 2
          OnClick = rdoAnalysisTypeClick
        end
        object rdoRetrospectiveSpaceTime: TRadioButton
          Left = 17
          Top = 110
          Width = 113
          Height = 17
          Caption = 'Space-Time'
          TabOrder = 3
          OnClick = rdoAnalysisTypeClick
        end
        object rdoProspectivePurelyTemporal: TRadioButton
          Left = 17
          Top = 172
          Width = 113
          Height = 17
          Caption = 'Purely Temporal'
          TabOrder = 5
          OnClick = rdoAnalysisTypeClick
        end
        object rdoProspectiveSpaceTime: TRadioButton
          Left = 17
          Top = 202
          Width = 113
          Height = 17
          Caption = 'Space-Time'
          TabOrder = 6
          OnClick = rdoAnalysisTypeClick
        end
      end
      object rgpScanAreas: TGroupBox
        Left = 332
        Top = 8
        Width = 141
        Height = 121
        Caption = 'Scan for Areas with:'
        TabOrder = 2
        object rdoHighRates: TRadioButton
          Left = 13
          Top = 25
          Width = 113
          Height = 17
          Caption = 'High Rates'
          Checked = True
          TabOrder = 0
          TabStop = True
        end
        object rdoLowRates: TRadioButton
          Left = 13
          Top = 55
          Width = 113
          Height = 17
          Caption = 'Low Rates'
          TabOrder = 1
        end
        object rdoHighLowRates: TRadioButton
          Left = 13
          Top = 85
          Width = 113
          Height = 17
          Caption = 'High or Low Rates'
          TabOrder = 2
        end
      end
      object rgpTimeIntervalUnits: TGroupBox
        Left = 164
        Top = 136
        Width = 309
        Height = 113
        Caption = 'Time Intervals'
        TabOrder = 4
        object lblTimeIntervalUnits: TLabel
          Left = 14
          Top = 21
          Width = 33
          Height = 13
          Caption = 'Units:  '
        end
        object lblTimeIntervalLength: TLabel
          Left = 14
          Top = 84
          Width = 129
          Height = 13
          Caption = 'Length (in specified units):  '
        end
        object rdoUnitYear: TRadioButton
          Left = 62
          Top = 21
          Width = 57
          Height = 17
          Caption = 'Year'
          Checked = True
          Enabled = False
          TabOrder = 0
          TabStop = True
          OnClick = rdoUnitYearClick
        end
        object rdoUnitMonths: TRadioButton
          Left = 62
          Top = 42
          Width = 57
          Height = 17
          Caption = 'Month'
          Enabled = False
          TabOrder = 1
          OnClick = rdoUnitMonthsClick
        end
        object rdoUnitDay: TRadioButton
          Left = 62
          Top = 63
          Width = 57
          Height = 17
          Caption = 'Day'
          Enabled = False
          TabOrder = 2
          OnClick = rdoUnitDayClick
        end
        object edtTimeIntervalLength: TEdit
          Left = 150
          Top = 80
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
      object btnAdvanced1: TButton
        Left = 400
        Top = 344
        Width = 72
        Height = 24
        Caption = 'Advanced...'
        TabOrder = 5
        OnClick = btnAdvancedParametersClick
      end
    end
    object tbOutputFiles: TTabSheet
      Caption = 'Output'
      ImageIndex = 4
      object gbxAdditionalOutputFiles: TGroupBox
        Left = 8
        Top = 68
        Width = 461
        Height = 153
        Caption = 'Additional Optional Output Files:'
        TabOrder = 1
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
          Left = 314
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
          Left = 383
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
          Left = 320
          Top = 72
          Width = 16
          Height = 17
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
        end
        object chkClustersInColumnFormatAscii: TCheckBox
          Left = 320
          Top = 48
          Width = 16
          Height = 17
          BiDiMode = bdLeftToRight
          Caption = ' '
          ParentBiDiMode = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
        end
        object chkRelativeRiskEstimatesAreaAscii: TCheckBox
          Left = 320
          Top = 96
          Width = 16
          Height = 17
          ParentShowHint = False
          ShowHint = True
          TabOrder = 4
        end
        object chkSimulatedLogLikelihoodRatiosAscii: TCheckBox
          Left = 320
          Top = 120
          Width = 16
          Height = 17
          TabOrder = 6
        end
        object chkClustersInColumnFormatDBase: TCheckBox
          Left = 390
          Top = 48
          Width = 16
          Height = 17
          BiDiMode = bdLeftToRight
          ParentBiDiMode = False
          TabOrder = 1
        end
        object chkCensusAreasReportedClustersDBase: TCheckBox
          Left = 390
          Top = 72
          Width = 16
          Height = 17
          BiDiMode = bdLeftToRight
          ParentBiDiMode = False
          TabOrder = 3
        end
        object chkRelativeRiskEstimatesAreaDBase: TCheckBox
          Left = 390
          Top = 96
          Width = 16
          Height = 17
          TabOrder = 5
        end
        object chkSimulatedLogLikelihoodRatiosDBase: TCheckBox
          Left = 390
          Top = 120
          Width = 16
          Height = 17
          TabOrder = 7
        end
      end
      object pnlTop: TPanel
        Left = 0
        Top = 0
        Width = 481
        Height = 58
        Align = alTop
        BevelOuter = bvNone
        TabOrder = 0
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
          Width = 337
          Height = 21
          AutoSelect = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = edtResultFileChange
        end
        object btnResultFileBrowse: TButton
          Left = 357
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
      object btnAdvanced2: TButton
        Left = 400
        Top = 344
        Width = 72
        Height = 24
        Caption = 'Advanced...'
        TabOrder = 2
        OnClick = btnAdvancedParametersClick
      end
    end
  end
  object OpenDialog1: TOpenDialog
    Left = 364
  end
  object SaveDialog: TSaveDialog
    DefaultExt = '*.prm'
    Filter = 'Parameter Files (*.prm)|*.prm'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofEnableSizing]
    Title = 'Save Parameter File As'
    Left = 392
  end
end
