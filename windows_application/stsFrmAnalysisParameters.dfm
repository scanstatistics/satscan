object frmAnalysis: TfrmAnalysis
  Left = 201
  Top = 83
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'SaTScan Analysis Settings'
  ClientHeight = 399
  ClientWidth = 496
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
  object Label11: TLabel
    Left = 372
    Top = 53
    Width = 22
    Height = 13
    Caption = 'Year'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label13: TLabel
    Left = 402
    Top = 53
    Width = 30
    Height = 13
    Caption = 'Month'
  end
  object PageControl1: TPageControl
    Left = 2
    Top = 8
    Width = 493
    Height = 389
    ActivePage = tbOutputFiles
    TabOrder = 0
    object tbInputFiles: TTabSheet
      Caption = 'Input'
      object Label4: TLabel
        Left = 15
        Top = 165
        Width = 72
        Height = 13
        Caption = 'Population File:'
      end
      object Label5: TLabel
        Left = 128
        Top = 165
        Width = 75
        Height = 13
        Caption = '(Poisson Model)'
      end
      object GroupBox1: TGroupBox
        Left = 8
        Top = 4
        Width = 466
        Height = 151
        Color = clBtnFace
        ParentColor = False
        TabOrder = 4
        object Label10: TLabel
          Left = 136
          Top = 13
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
        object Label12: TLabel
          Left = 170
          Top = 13
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
        object Label14: TLabel
          Left = 204
          Top = 13
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
        object Label1: TLabel
          Left = 12
          Top = 52
          Width = 46
          Height = 13
          Caption = 'Case File:'
        end
        object Label2: TLabel
          Left = 12
          Top = 100
          Width = 55
          Height = 13
          Caption = 'Control File:'
          Color = clBtnFace
          ParentColor = False
        end
        object Label3: TLabel
          Left = 120
          Top = 100
          Width = 78
          Height = 13
          Caption = '(Bernoulli Model)'
        end
        object Label8: TLabel
          Left = 12
          Top = 27
          Width = 114
          Height = 13
          Caption = 'Study Period Start Date:'
        end
        object Label9: TLabel
          Left = 245
          Top = 27
          Width = 111
          Height = 13
          Caption = 'Study Period End Date:'
        end
        object Label15: TLabel
          Left = 364
          Top = 13
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
        object Label17: TLabel
          Left = 399
          Top = 13
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
        object Label18: TLabel
          Left = 432
          Top = 13
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
        object rgpPrecisionTimes: TRadioGroup
          Left = 361
          Top = 63
          Width = 92
          Height = 74
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
          Top = 68
          Width = 284
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = edtCaseFileNameChange
        end
        object edtControlFileName: TEdit
          Left = 8
          Top = 116
          Width = 284
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
          OnChange = edtControlFileNameChange
        end
        object btnCaseBrowse: TButton
          Left = 300
          Top = 68
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
          Top = 116
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
        object edtStudyPeriodEndDateYear: TEdit
          Left = 361
          Top = 24
          Width = 33
          Height = 21
          MaxLength = 4
          TabOrder = 5
          Text = '1900'
          OnExit = edtStudyPeriodEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodStartDateYear: TEdit
          Left = 132
          Top = 24
          Width = 33
          Height = 21
          MaxLength = 4
          TabOrder = 6
          Text = '1900'
          OnExit = edtStudyPeriodStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodStartDateMonth: TEdit
          Left = 172
          Top = 24
          Width = 23
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 7
          Text = '1'
          OnExit = edtStudyPeriodStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodEndDateMonth: TEdit
          Left = 401
          Top = 24
          Width = 23
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 8
          Text = '12'
          OnExit = edtStudyPeriodEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodEndDateDay: TEdit
          Left = 431
          Top = 24
          Width = 23
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 9
          Text = '31'
          OnExit = edtStudyPeriodEndDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStudyPeriodStartDateDay: TEdit
          Left = 202
          Top = 24
          Width = 23
          Height = 21
          Enabled = False
          MaxLength = 2
          TabOrder = 10
          Text = '1'
          OnExit = edtStudyPeriodStartDateExit
          OnKeyPress = NaturalNumberKeyPress
        end
      end
      object edtPopFileName: TEdit
        Left = 15
        Top = 181
        Width = 284
        Height = 21
        ParentShowHint = False
        ShowHint = True
        TabOrder = 0
        OnChange = edtPopFileNameChange
      end
      object btnPopBrowse: TButton
        Left = 308
        Top = 181
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
        TabOrder = 1
        OnClick = btnPopBrowseClick
      end
      object GroupBox2: TGroupBox
        Left = 8
        Top = 208
        Width = 466
        Height = 111
        TabOrder = 2
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
          Top = 24
          Width = 124
          Height = 74
          Caption = 'Coordinates'
          ItemIndex = 0
          Items.Strings = (
            'Cartesian'
            'Latitude/Longitude')
          TabOrder = 4
          OnClick = rgpCoordinatesClick
        end
      end
      object btnImportFile: TButton
        Left = 396
        Top = 328
        Width = 76
        Height = 24
        Hint = 'Import File Wizard'
        Caption = 'Import Wizard'
        ParentShowHint = False
        ShowHint = True
        TabOrder = 3
        OnClick = btnImportFileClick
      end
    end
    object tbAnalysis: TTabSheet
      Caption = 'Analysis'
      ImageIndex = 1
      object Label16: TLabel
        Left = 52
        Top = 288
        Width = 266
        Height = 13
        Caption = 'Monte Carlo replications (9, 999, or value ending in 999):'
      end
      object edtMontCarloReps: TEdit
        Left = 328
        Top = 285
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
        Left = 156
        Top = 8
        Width = 160
        Height = 253
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
          Top = 75
          Width = 87
          Height = 17
          Caption = 'Bernoulli'
          TabOrder = 1
          OnClick = rdoProbabilityModelClick
        end
        object rdoSpaceTimePermutationModel: TRadioButton
          Left = 13
          Top = 125
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
        Width = 137
        Height = 253
        Caption = 'Type of Analysis'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        object stRetrospectiveAnalyses: TStaticText
          Left = 10
          Top = 25
          Width = 118
          Height = 17
          Caption = 'Retrospective Analyses:'
          TabOrder = 0
        end
        object stProspectiveAnalyses: TStaticText
          Left = 10
          Top = 159
          Width = 108
          Height = 17
          Caption = 'Prospective Analyses:'
          TabOrder = 4
        end
        object rdoRetrospectivePurelySpatial: TRadioButton
          Left = 14
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
          Left = 14
          Top = 85
          Width = 113
          Height = 17
          Caption = 'Purely Temporal'
          TabOrder = 2
          OnClick = rdoAnalysisTypeClick
        end
        object rdoRetrospectiveSpaceTime: TRadioButton
          Left = 14
          Top = 120
          Width = 113
          Height = 17
          Caption = 'Space-Time'
          TabOrder = 3
          OnClick = rdoAnalysisTypeClick
        end
        object rdoProspectivePurelyTemporal: TRadioButton
          Left = 14
          Top = 184
          Width = 113
          Height = 17
          Caption = 'Purely Temporal'
          TabOrder = 5
          OnClick = rdoAnalysisTypeClick
        end
        object rdoProspectiveSpaceTime: TRadioButton
          Left = 14
          Top = 219
          Width = 113
          Height = 17
          Caption = 'Space-Time'
          TabOrder = 6
          OnClick = rdoAnalysisTypeClick
        end
      end
      object rgpScanAreas: TGroupBox
        Left = 328
        Top = 8
        Width = 148
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
        Left = 328
        Top = 140
        Width = 145
        Height = 121
        Caption = 'Time Intervals'
        TabOrder = 4
        object lblTimeIntervalUnits: TLabel
          Left = 10
          Top = 21
          Width = 33
          Height = 13
          Caption = 'Units:  '
        end
        object lblTimeIntervalLength: TLabel
          Left = 10
          Top = 92
          Width = 42
          Height = 13
          Caption = 'Length:  '
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
          Left = 62
          Top = 88
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
        Left = 396
        Top = 328
        Width = 76
        Height = 24
        Hint = 'Advanced Analysis Features'
        Caption = 'Advanced...'
        ParentShowHint = False
        ShowHint = True
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
        Width = 465
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
          Width = 16
          Height = 17
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
        end
        object chkClustersInColumnFormatAscii: TCheckBox
          Left = 344
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
          Left = 414
          Top = 96
          Width = 16
          Height = 17
          TabOrder = 5
        end
        object chkSimulatedLogLikelihoodRatiosDBase: TCheckBox
          Left = 414
          Top = 120
          Width = 16
          Height = 17
          TabOrder = 7
        end
      end
      object pnlTop: TPanel
        Left = 0
        Top = 0
        Width = 485
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
        Left = 396
        Top = 328
        Width = 76
        Height = 24
        Hint = 'Advanced Output Features'
        Caption = 'Advanced...'
        ParentShowHint = False
        ShowHint = True
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
