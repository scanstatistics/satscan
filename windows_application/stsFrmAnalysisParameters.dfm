object frmAnalysis: TfrmAnalysis
  Left = 265
  Top = 197
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'SaTScan'
  ClientHeight = 341
  ClientWidth = 505
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsMDIChild
  OldCreateOrder = False
  Position = poDefault
  Visible = True
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object PageControl1: TPageControl
    Left = 8
    Top = 8
    Width = 489
    Height = 321
    ActivePage = tbOutputFiles
    TabOrder = 0
    object tbInputFiles: TTabSheet
      Caption = 'Input Files'
      object Label4: TLabel
        Left = 24
        Top = 136
        Width = 72
        Height = 13
        Caption = 'Population File:'
      end
      object Label5: TLabel
        Left = 128
        Top = 136
        Width = 75
        Height = 13
        Caption = '(Poisson Model)'
      end
      object GroupBox1: TGroupBox
        Left = 16
        Top = 8
        Width = 457
        Height = 113
        Color = clBtnFace
        ParentColor = False
        TabOrder = 0
        object Label1: TLabel
          Left = 8
          Top = 16
          Width = 46
          Height = 13
          Caption = 'Case File:'
        end
        object Label2: TLabel
          Left = 8
          Top = 64
          Width = 55
          Height = 13
          Caption = 'Control File:'
          Color = clBtnFace
          ParentColor = False
        end
        object Label3: TLabel
          Left = 112
          Top = 64
          Width = 78
          Height = 13
          Caption = '(Bernoulli Model)'
        end
        object rgPrecisionTimes: TRadioGroup
          Left = 320
          Top = 16
          Width = 129
          Height = 89
          Caption = 'Precision of Times'
          ItemIndex = 1
          Items.Strings = (
            'None'
            'Year'
            'Months'
            'Days')
          TabOrder = 0
          OnClick = rgPrecisionTimesClick
        end
        object edtCaseFileName: TEdit
          Left = 8
          Top = 32
          Width = 273
          Height = 21
          ParentShowHint = False
          ReadOnly = True
          ShowHint = True
          TabOrder = 1
          OnChange = edtCaseFileNameChange
        end
        object edtControlFileName: TEdit
          Left = 8
          Top = 80
          Width = 273
          Height = 21
          ParentShowHint = False
          ReadOnly = True
          ShowHint = True
          TabOrder = 2
          OnChange = edtControlFileNameChange
        end
        object btnCaseBrowse: TButton
          Left = 288
          Top = 32
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
          Left = 288
          Top = 80
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
        Left = 24
        Top = 152
        Width = 273
        Height = 21
        ParentShowHint = False
        ReadOnly = True
        ShowHint = True
        TabOrder = 1
        OnChange = edtPopFileNameChange
      end
      object btnPopBrowse: TButton
        Left = 304
        Top = 152
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
        Left = 16
        Top = 192
        Width = 457
        Height = 97
        TabOrder = 3
        object Label6: TLabel
          Left = 8
          Top = 11
          Width = 78
          Height = 13
          Caption = 'Coordinates File:'
        end
        object Label7: TLabel
          Left = 8
          Top = 54
          Width = 79
          Height = 13
          Caption = 'Special Grid File:'
        end
        object edtCoordinateFileName: TEdit
          Left = 8
          Top = 27
          Width = 273
          Height = 21
          ParentShowHint = False
          ReadOnly = True
          ShowHint = True
          TabOrder = 0
          OnChange = edtCoordinateFileNameChange
        end
        object edtGridFileName: TEdit
          Left = 8
          Top = 70
          Width = 273
          Height = 21
          ParentShowHint = False
          PopupMenu = SpecialGridPopupMenu
          ReadOnly = True
          ShowHint = True
          TabOrder = 1
          OnChange = edtGridFileNameChange
        end
        object btnCoordBrowse: TButton
          Left = 288
          Top = 27
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
          Left = 288
          Top = 70
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
          Left = 328
          Top = 16
          Width = 121
          Height = 73
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
        Left = 48
        Top = 264
        Width = 271
        Height = 13
        Caption = 'Monte Carlo Replications (9, 999, or value ending in 999):'
      end
      object rgTypeAnalysis: TRadioGroup
        Left = 8
        Top = 8
        Width = 161
        Height = 121
        Caption = 'Type of Analysis'
        ItemIndex = 0
        Items.Strings = (
          'Purely Spatial'
          'Purely Temporal'
          'Retrospective Space-Time '
          'Prospective Space-Time')
        TabOrder = 0
        OnClick = rgTypeAnalysisClick
      end
      object rgProbability: TRadioGroup
        Left = 176
        Top = 8
        Width = 153
        Height = 121
        Caption = 'Probability Model'
        ItemIndex = 0
        Items.Strings = (
          'Poisson'
          'Bernoulli'
          'Space-Time Permutation')
        TabOrder = 1
        OnClick = rgProbabilityClick
      end
      object rgScanAreas: TRadioGroup
        Left = 336
        Top = 8
        Width = 137
        Height = 121
        Caption = 'Scan for Areas with:'
        ItemIndex = 0
        Items.Strings = (
          'High Rates'
          'Low Rates'
          'High or Low Rates')
        TabOrder = 2
        OnClick = rgScanAreasClick
      end
      object GroupBox3: TGroupBox
        Left = 8
        Top = 136
        Width = 465
        Height = 105
        Caption = 'Study Period'
        TabOrder = 3
        object Label8: TLabel
          Left = 56
          Top = 48
          Width = 51
          Height = 13
          Caption = 'Start Date:'
        end
        object Label9: TLabel
          Left = 56
          Top = 80
          Width = 48
          Height = 13
          Caption = 'End Date:'
        end
        object Label10: TLabel
          Left = 120
          Top = 8
          Width = 22
          Height = 13
          Caption = 'Year'
        end
        object Label11: TLabel
          Left = 120
          Top = 24
          Width = 34
          Height = 13
          Caption = '(YYYY)'
        end
        object Label12: TLabel
          Left = 171
          Top = 8
          Width = 30
          Height = 13
          Caption = 'Month'
        end
        object Label13: TLabel
          Left = 171
          Top = 24
          Width = 24
          Height = 13
          Caption = '(MM)'
        end
        object Label14: TLabel
          Left = 208
          Top = 8
          Width = 19
          Height = 13
          Caption = 'Day'
        end
        object Label15: TLabel
          Left = 208
          Top = 24
          Width = 22
          Height = 13
          Caption = '(DD)'
        end
        object edtStartYear: TEdit
          Left = 120
          Top = 40
          Width = 41
          Height = 21
          MaxLength = 4
          TabOrder = 0
          Text = '1900'
          OnExit = edtStartYearExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtEndYear: TEdit
          Left = 120
          Top = 72
          Width = 41
          Height = 21
          MaxLength = 4
          TabOrder = 1
          Text = '1900'
          OnExit = edtEndYearExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartMonth: TEdit
          Left = 171
          Top = 40
          Width = 25
          Height = 21
          Color = clInactiveBorder
          Enabled = False
          MaxLength = 2
          TabOrder = 2
          Text = '1'
          OnExit = edtStartMonthExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtEndMonth: TEdit
          Left = 171
          Top = 72
          Width = 25
          Height = 21
          Color = clInactiveBorder
          Enabled = False
          MaxLength = 2
          TabOrder = 3
          Text = '12'
          OnExit = edtEndMonthExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtStartDay: TEdit
          Left = 208
          Top = 40
          Width = 25
          Height = 21
          Color = clInactiveBorder
          Enabled = False
          MaxLength = 2
          TabOrder = 4
          Text = '1'
          OnExit = edtStartDayExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtEndDay: TEdit
          Left = 208
          Top = 72
          Width = 25
          Height = 21
          Color = clInactiveBorder
          Enabled = False
          MaxLength = 2
          TabOrder = 5
          Text = '31'
          OnExit = edtEndDayExit
          OnKeyPress = NaturalNumberKeyPress
        end
      end
      object edtMontCarloReps: TEdit
        Left = 328
        Top = 256
        Width = 65
        Height = 21
        MaxLength = 9
        TabOrder = 4
        Text = '999'
        OnExit = edtMontCarloRepsExit
        OnKeyPress = NaturalNumberKeyPress
      end
    end
    object tbTimeParameter: TTabSheet
      Caption = 'Time Parameters'
      ImageIndex = 3
      object GroupBox6: TGroupBox
        Left = 16
        Top = 8
        Width = 193
        Height = 137
        Caption = 'Time Intervals'
        TabOrder = 0
        object Label19: TLabel
          Left = 24
          Top = 24
          Width = 33
          Height = 13
          Caption = 'Units:  '
        end
        object Label20: TLabel
          Left = 8
          Top = 104
          Width = 129
          Height = 13
          Caption = 'Length (in specified units):  '
        end
        object rbUnitYear: TRadioButton
          Left = 64
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
          Left = 64
          Top = 48
          Width = 57
          Height = 17
          Caption = 'Month'
          Enabled = False
          TabOrder = 1
          OnClick = rbUnitMonthsClick
        end
        object rbUnitDay: TRadioButton
          Left = 64
          Top = 72
          Width = 57
          Height = 17
          Caption = 'Day'
          Enabled = False
          TabOrder = 2
          OnClick = rbUnitDayClick
        end
        object edtUnitLength: TEdit
          Left = 144
          Top = 96
          Width = 25
          Height = 21
          Color = clInactiveBorder
          Enabled = False
          MaxLength = 3
          TabOrder = 3
          Text = '1'
          OnKeyPress = NaturalNumberKeyPress
        end
      end
      object rgTemporalTrendAdj: TRadioGroup
        Left = 16
        Top = 160
        Width = 449
        Height = 105
        Caption = 'Adjustment for Temporal Trend'
        Enabled = False
        ItemIndex = 0
        Items.Strings = (
          'None'
          'Nonparametric'
          'Log Linear with')
        TabOrder = 1
        OnClick = rgTemporalTrendAdjClick
      end
      object edtLogPerYear: TEdit
        Left = 120
        Top = 232
        Width = 41
        Height = 21
        Color = clInactiveBorder
        Enabled = False
        MaxLength = 5
        TabOrder = 2
        Text = '0'
        OnExit = edtLogPerYearExit
        OnKeyPress = FloatKeyPress
      end
      object Edit1: TEdit
        Left = 168
        Top = 240
        Width = 73
        Height = 13
        AutoSelect = False
        BorderStyle = bsNone
        Color = clInactiveBorder
        ReadOnly = True
        TabOrder = 3
        Text = '% per year'
      end
      object GroupBox8: TGroupBox
        Left = 232
        Top = 8
        Width = 233
        Height = 137
        Caption = 'Start Date of Prospective Space-Time'
        TabOrder = 4
        object Label23: TLabel
          Left = 43
          Top = 40
          Width = 22
          Height = 13
          Caption = 'Year'
        end
        object Label24: TLabel
          Left = 42
          Top = 56
          Width = 34
          Height = 13
          Caption = '(YYYY)'
        end
        object Label25: TLabel
          Left = 100
          Top = 56
          Width = 24
          Height = 13
          Caption = '(MM)'
        end
        object Label26: TLabel
          Left = 99
          Top = 40
          Width = 30
          Height = 13
          Caption = 'Month'
        end
        object Label27: TLabel
          Left = 146
          Top = 40
          Width = 19
          Height = 13
          Caption = 'Day'
        end
        object Label28: TLabel
          Left = 146
          Top = 56
          Width = 22
          Height = 13
          Caption = '(DD)'
        end
        object edtProspYear: TEdit
          Left = 40
          Top = 72
          Width = 41
          Height = 21
          Color = clInactiveBorder
          Enabled = False
          MaxLength = 4
          TabOrder = 0
          Text = '1900'
          OnExit = edtProspYearExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtProspMonth: TEdit
          Left = 100
          Top = 72
          Width = 25
          Height = 21
          Color = clInactiveBorder
          Enabled = False
          MaxLength = 2
          TabOrder = 1
          Text = '1'
          OnExit = edtProspMonthExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object edtProspDay: TEdit
          Left = 146
          Top = 72
          Width = 25
          Height = 21
          Color = clInactiveBorder
          Enabled = False
          MaxLength = 2
          TabOrder = 2
          Text = '1'
          OnExit = edtProspDayExit
          OnKeyPress = NaturalNumberKeyPress
        end
      end
    end
    object tbScanningWindow: TTabSheet
      Caption = 'Scanning Window'
      ImageIndex = 2
      object GroupBox4: TGroupBox
        Left = 16
        Top = 8
        Width = 457
        Height = 87
        Caption = 'Spatial'
        TabOrder = 0
        object Label17: TLabel
          Left = 24
          Top = 22
          Width = 146
          Height = 13
          Caption = 'Maximum Spatial Cluster Size:  '
        end
        object edtMaxClusterSize: TEdit
          Left = 184
          Top = 16
          Width = 38
          Height = 21
          AutoSelect = False
          MaxLength = 5
          TabOrder = 0
          Text = '50'
          OnExit = edtMaxClusterSizeExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object chkInclPurTempClust: TCheckBox
          Left = 24
          Top = 57
          Width = 305
          Height = 17
          Caption = 'Include Purely Temporal Cluster (Spatial Size = 100%)'
          Enabled = False
          TabOrder = 1
          OnClick = chkInclPurTempClustClick
        end
        object rdoSpatialPercentage: TRadioButton
          Left = 232
          Top = 14
          Width = 217
          Height = 17
          Caption = 'Percent of Population at Risk (<= 50%)'
          Checked = True
          TabOrder = 2
          TabStop = True
          OnClick = rdoSpatialPercentageClick
        end
        object rdoSpatialDistance: TRadioButton
          Left = 232
          Top = 34
          Width = 217
          Height = 17
          Caption = 'Kilometer Radius'
          TabOrder = 3
          OnClick = rdoSpatialDistanceClick
        end
      end
      object GroupBox5: TGroupBox
        Left = 16
        Top = 103
        Width = 457
        Height = 89
        Caption = 'Temporal'
        TabOrder = 1
        object Label18: TLabel
          Left = 24
          Top = 22
          Width = 158
          Height = 13
          Caption = 'Maximum Temporal Cluster Size:  '
        end
        object edtMaxTemporalClusterSize: TEdit
          Left = 184
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
          Left = 24
          Top = 59
          Width = 297
          Height = 17
          Caption = 'Include Purely Spatial Clusters (Temporal Size = 100%)'
          Enabled = False
          TabOrder = 1
          OnClick = chkIncludePurSpacClustClick
        end
        object rdoPercentageTemproal: TRadioButton
          Left = 232
          Top = 14
          Width = 217
          Height = 17
          Caption = 'Percent of Study Period (<= 90%)'
          Checked = True
          TabOrder = 2
          TabStop = True
        end
        object rdoTimeTemproal: TRadioButton
          Left = 232
          Top = 34
          Width = 217
          Height = 17
          Caption = 'Years'
          TabOrder = 3
        end
      end
      object rgClustersToInclude: TRadioGroup
        Left = 16
        Top = 199
        Width = 457
        Height = 81
        Caption = 'Clusters to Include'
        Enabled = False
        ItemIndex = 0
        Items.Strings = (
          'All clusters'
          
            'Only clusters that include the study period end date (alive clus' +
            'ters)')
        TabOrder = 2
        OnClick = rgClustersToIncludeClick
      end
    end
    object tbOutputFiles: TTabSheet
      Caption = 'Output Files'
      ImageIndex = 4
      object Label29: TLabel
        Left = 16
        Top = 8
        Width = 57
        Height = 13
        Caption = 'Results File:'
      end
      object edtResultFile: TEdit
        Left = 16
        Top = 24
        Width = 408
        Height = 21
        AutoSelect = False
        ParentShowHint = False
        ShowHint = True
        TabOrder = 0
        OnChange = edtResultFileChange
      end
      object btnResultFileBrowse: TButton
        Left = 434
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
      object grpAdditionalOutputFiles: TGroupBox
        Left = 16
        Top = 61
        Width = 449
        Height = 153
        Caption = 'Additional Optional Output Files:'
        TabOrder = 2
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
          Width = 151
          Height = 13
          Caption = 'Simulated Log Likelihood Ratios'
        end
        object ASCII: TLabel
          Left = 312
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
          Left = 381
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
          Left = 318
          Top = 72
          Width = 15
          Height = 17
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
          OnClick = chkCensusAreasReportedClustersAsciiClick
        end
        object chkClustersInColumnFormatAscii: TCheckBox
          Left = 318
          Top = 48
          Width = 15
          Height = 17
          BiDiMode = bdLeftToRight
          ParentBiDiMode = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnClick = chkClustersInColumnFormatAsciiClick
        end
        object chkRelativeRiskEstimatesAreaAscii: TCheckBox
          Left = 318
          Top = 96
          Width = 16
          Height = 17
          ParentShowHint = False
          ShowHint = True
          TabOrder = 4
          OnClick = chkRelativeRiskEstimatesAreaAsciiClick
        end
        object chkSimulatedLogLikelihoodRatiosAscii: TCheckBox
          Left = 318
          Top = 120
          Width = 16
          Height = 17
          TabOrder = 6
          OnClick = chkSimulatedLogLikelihoodRatiosAsciiClick
        end
        object chkClustersInColumnFormatDBase: TCheckBox
          Left = 388
          Top = 48
          Width = 16
          Height = 17
          BiDiMode = bdLeftToRight
          ParentBiDiMode = False
          TabOrder = 1
        end
        object chkCensusAreasReportedClustersDBase: TCheckBox
          Left = 388
          Top = 72
          Width = 16
          Height = 17
          BiDiMode = bdLeftToRight
          ParentBiDiMode = False
          TabOrder = 3
        end
        object chkRelativeRiskEstimatesAreaDBase: TCheckBox
          Left = 389
          Top = 96
          Width = 16
          Height = 17
          TabOrder = 5
        end
        object chkSimulatedLogLikelihoodRatiosDBase: TCheckBox
          Left = 389
          Top = 120
          Width = 16
          Height = 17
          TabOrder = 7
        end
      end
      object grpCriteriaSecClusters: TGroupBox
        Left = 16
        Top = 232
        Width = 449
        Height = 50
        Caption = 'Criteria for Reporting Secondary Clusters'
        TabOrder = 3
        object cboCriteriaSecClusters: TComboBox
          Left = 10
          Top = 18
          Width = 301
          Height = 21
          Style = csDropDownList
          ItemHeight = 13
          TabOrder = 0
          OnChange = cboCriteriaSecClustersChange
          Items.Strings = (
            'No Geographical Overlap'
            'No Cluster Centers in Other Clusters'
            'No Cluster Centers in More Likely Clusters'
            'No Cluster Centers in Less Likely Clusters'
            'No Pairs of Centers Both in Each Others Clusters'
            'No Restrictions = Most Likely Cluster for Each Grid Point')
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
  object SpecialGridPopupMenu: TPopupMenu
    Left = 472
    object mitClearSpecialGridEdit: TMenuItem
      Caption = 'Clear'
      OnClick = mitClearSpecialGridEditClick
    end
  end
end
