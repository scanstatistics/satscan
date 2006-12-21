object frmAdvancedParameters: TfrmAdvancedParameters
  Left = 359
  Top = 246
  ActiveControl = PageControl
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Advanced Features'
  ClientHeight = 314
  ClientWidth = 534
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
    Left = 450
    Top = 0
    Width = 84
    Height = 314
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
    Left = 0
    Top = 0
    Width = 450
    Height = 314
    ActivePage = tsDataSets
    Align = alClient
    TabOrder = 0
    object tsDataSets: TTabSheet
      Caption = 'Multiple Data Sets'
      ImageIndex = 5
      object grpDataSets: TGroupBox
        Left = 8
        Top = 8
        Width = 425
        Height = 233
        Caption = 'Additional Input Data Sets'
        TabOrder = 0
        object lblCaseFile: TLabel
          Left = 112
          Top = 18
          Width = 46
          Height = 13
          Hint = 'Additional Case File Name'
          Caption = 'Case File:'
          ParentShowHint = False
          ShowHint = False
        end
        object lblControlFile: TLabel
          Left = 112
          Top = 60
          Width = 55
          Height = 13
          Caption = 'Control File:'
        end
        object lblPopulationFile: TLabel
          Left = 112
          Top = 102
          Width = 72
          Height = 13
          Caption = 'Population File:'
        end
        object lblBernoulli: TLabel
          Left = 218
          Top = 60
          Width = 78
          Height = 13
          Caption = '(Bernoulli Model)'
        end
        object lblPoisson: TLabel
          Left = 218
          Top = 102
          Width = 75
          Height = 13
          Caption = '(Poisson Model)'
        end
        object btnPopImport: TBitBtn
          Left = 393
          Top = 117
          Width = 23
          Height = 22
          Hint = 'Import population file ...'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 11
          OnClick = btnPopImportClick
          Glyph.Data = {
            DE010000424DDE01000000000000760000002800000024000000120000000100
            04000000000068010000C40E0000C40E00001000000000000000000000000000
            80000080000000808000800000008000800080800000C0C0C000808080000000
            FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00666666666666
            0000066666666666668888860000666666846668777780666666886668777788
            0000666666644668FF7780666666688668FF77880000666844444468F7F78066
            6888888868F7F7880000666486644668FF7780666886688668FF778800006664
            86846668F7F780666886886668F7F7880000666486666668FF77806668866666
            68FF778800006666666666688888806666666666688888880000000000006668
            FF7F70888888886668FF7F780000877777706666888886877777786666888886
            00008F0F0F7066666666668F7F7F786666666666000080F0F070666666666687
            F7F778666666666600008F0F0F7066666666668F7F7F786666666666000080F0
            F070666666666687F7F778666666666600008F0F000066666666668F7F888866
            66666666000080F07F86666666666687F77F86666666666600008F0F78666666
            6666668F7F786666666666660000888886666666666666888886666666666666
            0000}
          NumGlyphs = 2
        end
        object btnControlImport: TBitBtn
          Left = 393
          Top = 74
          Width = 23
          Height = 22
          Hint = 'Import control file ...'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 8
          OnClick = btnControlImportClick
          Glyph.Data = {
            DE010000424DDE01000000000000760000002800000024000000120000000100
            04000000000068010000C40E0000C40E00001000000000000000000000000000
            80000080000000808000800000008000800080800000C0C0C000808080000000
            FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00666666666666
            0000066666666666668888860000666666846668777780666666886668777788
            0000666666644668FF7780666666688668FF77880000666844444468F7F78066
            6888888868F7F7880000666486644668FF7780666886688668FF778800006664
            86846668F7F780666886886668F7F7880000666486666668FF77806668866666
            68FF778800006666666666688888806666666666688888880000000000006668
            FF7F70888888886668FF7F780000877777706666888886877777786666888886
            00008F0F0F7066666666668F7F7F786666666666000080F0F070666666666687
            F7F778666666666600008F0F0F7066666666668F7F7F786666666666000080F0
            F070666666666687F7F778666666666600008F0F000066666666668F7F888866
            66666666000080F07F86666666666687F77F86666666666600008F0F78666666
            6666668F7F786666666666660000888886666666666666888886666666666666
            0000}
          NumGlyphs = 2
        end
        object btnCaseImport: TBitBtn
          Left = 394
          Top = 32
          Width = 23
          Height = 22
          Hint = 'Import case file ...'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 5
          OnClick = btnCaseImportClick
          Glyph.Data = {
            DE010000424DDE01000000000000760000002800000024000000120000000100
            04000000000068010000C40E0000C40E00001000000000000000000000000000
            80000080000000808000800000008000800080800000C0C0C000808080000000
            FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00666666666666
            0000066666666666668888860000666666846668777780666666886668777788
            0000666666644668FF7780666666688668FF77880000666844444468F7F78066
            6888888868F7F7880000666486644668FF7780666886688668FF778800006664
            86846668F7F780666886886668F7F7880000666486666668FF77806668866666
            68FF778800006666666666688888806666666666688888880000000000006668
            FF7F70888888886668FF7F780000877777706666888886877777786666888886
            00008F0F0F7066666666668F7F7F786666666666000080F0F070666666666687
            F7F778666666666600008F0F0F7066666666668F7F7F786666666666000080F0
            F070666666666687F7F778666666666600008F0F000066666666668F7F888866
            66666666000080F07F86666666666687F77F86666666666600008F0F78666666
            6666668F7F786666666666660000888886666666666666888886666666666666
            0000}
          NumGlyphs = 2
        end
        object lstInputDataSets: TListBox
          Left = 8
          Top = 32
          Width = 95
          Height = 78
          Hint = 'All Additional Input Sets'
          ItemHeight = 13
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnClick = lstInputDataSetsClick
        end
        object btnNewDataSet: TButton
          Left = 8
          Top = 116
          Width = 47
          Height = 22
          Hint = 'Add an Additional Input Set'
          Caption = 'Add'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 1
          OnClick = btnNewClick
        end
        object btnRemoveDataSet: TButton
          Left = 57
          Top = 116
          Width = 47
          Height = 22
          Hint = 'Remove an Additional Input Set'
          Caption = 'Remove'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
          OnClick = btnRemoveDataSetClick
        end
        object edtCaseFileName: TEdit
          Left = 112
          Top = 32
          Width = 250
          Height = 21
          Hint = 'Additional Case File Name'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 3
          OnChange = edtCaseFileNameChange
        end
        object edtControlFileName: TEdit
          Left = 112
          Top = 74
          Width = 250
          Height = 21
          Hint = 'Additional Control File Name'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 6
          OnChange = edtControlFileNameChange
        end
        object edtPopFileName: TEdit
          Left = 112
          Top = 117
          Width = 250
          Height = 21
          Hint = 'Additional Population File Name'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 9
          OnChange = edtPopFileNameChange
        end
        object btnPopBrowse: TButton
          Left = 366
          Top = 117
          Width = 23
          Height = 22
          Hint = 'Browse for population file ...'
          Caption = '...'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsBold]
          ParentFont = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 10
          OnClick = btnPopBrowseClick
        end
        object btnControlBrowse: TButton
          Left = 366
          Top = 74
          Width = 23
          Height = 22
          Hint = 'Browse for control file ...'
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
          OnClick = btnControlBrowseClick
        end
        object btnCaseBrowse: TButton
          Left = 366
          Top = 32
          Width = 23
          Height = 22
          Hint = 'Browse for case file ...'
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
          OnClick = btnCaseBrowseClick
        end
        object lblMultipleDataSetPurpose: TStaticText
          Left = 8
          Top = 152
          Width = 147
          Height = 17
          Caption = 'Purpose of Multiple Data Sets:'
          TabOrder = 12
        end
        object rdoMultivariate: TRadioButton
          Left = 24
          Top = 176
          Width = 297
          Height = 17
          Caption = 'Multivariate Analysis (clusters in one or more data sets)'
          Checked = True
          TabOrder = 13
          TabStop = True
          OnClick = OnControlExit
        end
        object rdoAdjustmentByDataSets: TRadioButton
          Left = 24
          Top = 200
          Width = 297
          Height = 17
          Caption = 'Adjustment (clusters in all data sets simultaneously)'
          TabOrder = 14
          OnClick = OnControlExit
        end
      end
    end
    object tsDataChecking: TTabSheet
      Caption = 'Data Checking'
      ImageIndex = 6
      object rdgStudyPeriodCheck: TRadioGroup
        Left = 8
        Top = 8
        Width = 425
        Height = 82
        Caption = 'Study Period Check'
        Items.Strings = (
          
            'Check to ensure that cases and controls are within the Study Per' +
            'iod.'
          'Ignore cases and controls that are outside the Study Period.')
        TabOrder = 0
        OnClick = rdgStudyPeriodCheckClick
      end
      object grpGeographicalCoordinatesCheck: TGroupBox
        Left = 8
        Top = 98
        Width = 425
        Height = 105
        Caption = 'Geographical Coordinates Check'
        TabOrder = 1
        object rdoStrictCoordinates: TRadioButton
          Left = 9
          Top = 24
          Width = 406
          Height = 17
          Caption = 
            'Check to ensure that all locations in the case, control and popu' +
            'lation files'
          Checked = True
          TabOrder = 0
          TabStop = True
          OnClick = rdoStrictCoordinatesClick
        end
        object rdoRelaxedCoordinates: TRadioButton
          Left = 9
          Top = 64
          Width = 406
          Height = 17
          Caption = 
            ' Ignore data in the case, control and population files that do n' +
            'ot correspond'
          TabOrder = 1
          OnClick = rdoRelaxedCoordinatesClick
        end
        object stStrictCoodinates: TStaticText
          Left = 30
          Top = 40
          Width = 224
          Height = 17
          Caption = 'are present in the coordinates or neighbors file.'
          TabOrder = 2
          OnClick = stStrictCoodinatesClick
        end
        object stRelaxedCoodinates: TStaticText
          Left = 30
          Top = 80
          Width = 270
          Height = 17
          Caption = 'to a location ID listed in the coordinates or neighbors file.'
          TabOrder = 3
          OnClick = stRelaxedCoodinatesClick
        end
      end
    end
    object tabNeighborsFile: TTabSheet
      Caption = 'Non-Eucledian Neighbors'
      ImageIndex = 7
      object grpNeighborsFile: TGroupBox
        Left = 8
        Top = 8
        Width = 421
        Height = 125
        Caption = 'Special Neighbor Files'
        TabOrder = 0
        object edtNeighborsFile: TEdit
          Left = 30
          Top = 44
          Width = 353
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 1
          OnChange = edtNeighborsFileChange
          OnExit = OnControlExit
        end
        object btnBrowseForNeighborsFile: TButton
          Left = 388
          Top = 44
          Width = 23
          Height = 22
          Hint = 'Browse for neighbors file ...'
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
          OnClick = btnBrowseForNeighborsFileClick
        end
        object chkSpecifiyNeighborsFile: TCheckBox
          Left = 8
          Top = 21
          Width = 389
          Height = 17
          Caption = 'Specify neighbors through a user defined file'
          TabOrder = 0
          OnClick = chkSpecifiyNeighborsFileClick
        end
        object chkSpecifiyMetaLocationsFile: TCheckBox
          Left = 8
          Top = 71
          Width = 389
          Height = 17
          Caption = 'Specify meta locations through a user defined file'
          TabOrder = 3
          OnClick = chkSpecifiyNeighborsFileClick
        end
        object edtMetaLocationsFile: TEdit
          Left = 30
          Top = 90
          Width = 353
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 4
          OnChange = edtMetaLocationsFileChange
          OnExit = OnControlExit
        end
        object btnBrowseForMetaLocationsFile: TButton
          Left = 388
          Top = 90
          Width = 23
          Height = 22
          Hint = 'Browse for meta locations file ...'
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
          OnClick = btnBrowseForMetaLocationsFileClick
        end
      end
    end
    object tabMultipleCoordinates: TTabSheet
      Caption = 'Multiple Coordinates Per Location'
      ImageIndex = 8
      object gpMultipleCoordinates: TGroupBox
        Left = 8
        Top = 8
        Width = 425
        Height = 106
        Caption = 'Multiple Coordinate Locations per Location ID'
        TabOrder = 0
        object rdoOnePerLocationId: TRadioButton
          Left = 8
          Top = 18
          Width = 409
          Height = 17
          Caption = 'Allow only one coordinate location per location ID.'
          TabOrder = 0
          OnClick = OnMultipleCoordinatesClick
        end
        object rdoAllLocations: TRadioButton
          Left = 8
          Top = 70
          Width = 409
          Height = 17
          Caption = 
            'Include location in the scanning window if all coordinate locati' +
            'ons are in '
          TabOrder = 1
          OnClick = OnMultipleCoordinatesClick
        end
        object rdoAtLeastOne: TRadioButton
          Left = 8
          Top = 36
          Width = 409
          Height = 17
          Caption = 
            'Include location in the scanning window if at least one coordina' +
            'te location '
          TabOrder = 2
          OnClick = OnMultipleCoordinatesClick
        end
        object stAtLeastOne: TStaticText
          Left = 23
          Top = 52
          Width = 74
          Height = 17
          Caption = ' is in the circle.'
          TabOrder = 3
        end
        object stAllLocations: TStaticText
          Left = 26
          Top = 87
          Width = 50
          Height = 17
          Caption = 'the circle.'
          TabOrder = 4
        end
      end
    end
    object tbSpatial: TTabSheet
      Caption = 'Spatial Window'
      ImageIndex = 3
      object rdgSpatialOptions: TGroupBox
        Left = 8
        Top = 8
        Width = 425
        Height = 142
        Caption = 'Maximum Spatial Cluster Size'
        TabOrder = 0
        object lblPercentOfPopulation: TLabel
          Left = 47
          Top = 23
          Width = 263
          Height = 13
          Caption = 'percent of the population at risk (<= 50%, default = 50%)'
        end
        object lblMaxRadius: TLabel
          Left = 200
          Top = 114
          Width = 73
          Height = 13
          Caption = 'kilometer radius'
        end
        object lblPercentageOfPopFile: TLabel
          Left = 85
          Top = 56
          Width = 316
          Height = 13
          Caption = 
            'percent of the population defined in the max circle size file (<' +
            '= 50%)'
        end
        object lblDistancePrefix: TLabel
          Left = 41
          Top = 114
          Width = 85
          Height = 13
          Caption = 'is an ellipse with a'
        end
        object edtMaxSpatialClusterSize: TEdit
          Left = 8
          Top = 22
          Width = 34
          Height = 21
          AutoSelect = False
          MaxLength = 5
          TabOrder = 0
          Text = '50'
          OnExit = edtMaxSpatialClusterSizeExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object edtMaxSpatialRadius: TEdit
          Left = 132
          Top = 110
          Width = 58
          Height = 21
          TabOrder = 7
          Text = '1'
          OnExit = edtMaxSpatialRadiusExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object edtMaxSpatialPercentFile: TEdit
          Left = 46
          Top = 55
          Width = 34
          Height = 21
          MaxLength = 5
          TabOrder = 1
          Text = '50'
          OnExit = edtMaxSpatialPercentFileExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object edtMaxCirclePopulationFilename: TEdit
          Left = 45
          Top = 83
          Width = 280
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 3
          OnChange = edtMaxCirclePopulationFilenameChange
          OnExit = OnControlExit
        end
        object btnBrowseMaxCirclePopFile: TButton
          Left = 331
          Top = 82
          Width = 23
          Height = 22
          Hint = 'Browse for max circle size file ...'
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
          OnClick = btnBrowseMaxCirclePopFileClick
        end
        object btnImportMaxCirclePopFile: TBitBtn
          Left = 359
          Top = 82
          Width = 23
          Height = 22
          Hint = 'Import max circle size file ...'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 5
          OnClick = btnImportMaxCirclePopFileClick
          Glyph.Data = {
            DE010000424DDE01000000000000760000002800000024000000120000000100
            04000000000068010000C40E0000C40E00001000000000000000000000000000
            80000080000000808000800000008000800080800000C0C0C000808080000000
            FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00666666666666
            0000066666666666668888860000666666846668777780666666886668777788
            0000666666644668FF7780666666688668FF77880000666844444468F7F78066
            6888888868F7F7880000666486644668FF7780666886688668FF778800006664
            86846668F7F780666886886668F7F7880000666486666668FF77806668866666
            68FF778800006666666666688888806666666666688888880000000000006668
            FF7F70888888886668FF7F780000877777706666888886877777786666888886
            00008F0F0F7066666666668F7F7F786666666666000080F0F070666666666687
            F7F778666666666600008F0F0F7066666666668F7F7F786666666666000080F0
            F070666666666687F7F778666666666600008F0F000066666666668F7F888866
            66666666000080F07F86666666666687F77F86666666666600008F0F78666666
            6666668F7F786666666666660000888886666666666666888886666666666666
            0000}
          NumGlyphs = 2
        end
        object chkSpatialPopulationFile: TCheckBox
          Left = 19
          Top = 58
          Width = 17
          Height = 17
          TabOrder = 2
          OnClick = rdoMaxSpatialTypeClick
        end
        object chkSpatialDistance: TCheckBox
          Left = 19
          Top = 113
          Width = 18
          Height = 17
          TabOrder = 6
          OnClick = rdoMaxSpatialTypeClick
        end
      end
      object chkInclPureTempClust: TCheckBox
        Left = 8
        Top = 158
        Width = 305
        Height = 17
        Caption = 'Include Purely Temporal Clusters (Spatial Size = 100%)'
        Enabled = False
        TabOrder = 1
        OnClick = OnControlExit
      end
      object grpWindowShape: TGroupBox
        Left = 8
        Top = 182
        Width = 425
        Height = 66
        Caption = 'Spatial Window Shape'
        TabOrder = 2
        object rdoCircular: TRadioButton
          Left = 16
          Top = 17
          Width = 60
          Height = 17
          Caption = 'Circular'
          TabOrder = 0
          OnClick = OnWindowShapeClick
        end
        object rdoElliptic: TRadioButton
          Left = 16
          Top = 37
          Width = 53
          Height = 17
          Caption = 'Elliptic'
          TabOrder = 1
          OnClick = OnWindowShapeClick
        end
        object stNonCompactnessPenalty: TStaticText
          Left = 104
          Top = 37
          Width = 130
          Height = 17
          Caption = 'Non-compactness penalty:'
          TabOrder = 2
        end
        object cmbNonCompactnessPenalty: TComboBox
          Left = 240
          Top = 37
          Width = 91
          Height = 21
          Style = csDropDownList
          ItemHeight = 13
          TabOrder = 3
          OnChange = OnNonCompactnessPenaltyChange
          Items.Strings = (
            'None'
            'Medium'
            'Strong')
        end
      end
      object chkPerformIsotonicScan: TCheckBox
        Left = 8
        Top = 255
        Width = 425
        Height = 17
        Caption = 'Use Isotonic Spatial Scan'
        Enabled = False
        TabOrder = 3
        OnClick = chkPerformIsotonicScanClick
      end
    end
    object tsTemporal: TTabSheet
      Caption = 'Temporal Window'
      ImageIndex = 1
      object grpFlexibleTemporalWindowDefinition: TGroupBox
        Left = 8
        Top = 122
        Width = 421
        Height = 110
        Caption = 'Flexible Temporal Window Definition'
        TabOrder = 2
        object chkRestrictTemporalRange: TCheckBox
          Left = 12
          Top = 22
          Width = 353
          Height = 17
          Caption = 'Include only windows with:'
          TabOrder = 0
          OnClick = chkRestrictTemporalRangeClick
        end
        object stStartRangeTo: TStaticText
          Left = 257
          Top = 49
          Width = 19
          Height = 17
          Caption = 'to: '
          TabOrder = 5
        end
        object edtStartRangeStartYear: TEdit
          Left = 154
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
          Left = 194
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
          Left = 223
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
          Left = 280
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
          Left = 320
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
          Left = 349
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
          Left = 154
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
          Left = 194
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
          Left = 223
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
          Left = 257
          Top = 81
          Width = 19
          Height = 17
          Caption = 'to: '
          TabOrder = 13
        end
        object edtEndRangeEndYear: TEdit
          Left = 280
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
          Left = 320
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
          Left = 349
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
        Width = 421
        Height = 80
        Caption = 'Maximum Temporal Cluster Size'
        TabOrder = 0
        object lblPercentageOfStudyPeriod: TLabel
          Left = 81
          Top = 24
          Width = 240
          Height = 13
          Caption = 'percent of the study period (<= 90%, default = 50%)'
        end
        object lblMaxTemporalTimeUnits: TLabel
          Left = 81
          Top = 52
          Width = 25
          Height = 13
          Caption = 'years'
        end
        object edtMaxTemporalClusterSize: TEdit
          Left = 39
          Top = 22
          Width = 34
          Height = 21
          MaxLength = 5
          TabOrder = 1
          Text = '50'
          OnExit = edtMaxTemporalClusterSizeExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object rdoPercentageTemporal: TRadioButton
          Left = 7
          Top = 23
          Width = 31
          Height = 17
          Caption = 'is'
          Checked = True
          TabOrder = 0
          TabStop = True
          OnClick = rdoMaxTemporalClusterSizelick
        end
        object rdoTimeTemporal: TRadioButton
          Left = 7
          Top = 51
          Width = 28
          Height = 17
          Caption = 'is'
          TabOrder = 2
          OnClick = rdoMaxTemporalClusterSizelick
        end
        object edtMaxTemporalClusterSizeUnits: TEdit
          Left = 39
          Top = 50
          Width = 34
          Height = 21
          MaxLength = 6
          TabOrder = 3
          Text = '1'
          OnExit = edtMaxTemporalClusterSizeUnitsExit
          OnKeyPress = NaturalNumberKeyPress
        end
      end
      object chkIncludePureSpacClust: TCheckBox
        Left = 8
        Top = 98
        Width = 297
        Height = 17
        Caption = 'Include Purely Spatial Clusters (Temporal Size = 100%)'
        Enabled = False
        TabOrder = 1
        OnClick = OnControlExit
      end
    end
    object tsAdjustments: TTabSheet
      Caption = 'Space and Time Adjustments'
      object rdgTemporalTrendAdj: TRadioGroup
        Left = 8
        Top = 8
        Width = 421
        Height = 107
        Caption = 'Temporal Adjustment'
        Enabled = False
        ItemIndex = 0
        Items.Strings = (
          'None'
          'Nonparametric, with time stratified randomization'
          'Log linear trend with'
          'Log linear with automatically calculated trend')
        ParentShowHint = False
        ShowHint = False
        TabOrder = 0
        OnClick = rdgTemporalTrendAdjClick
        OnExit = OnControlExit
      end
      object edtLogLinear: TEdit
        Left = 134
        Top = 68
        Width = 34
        Height = 21
        Enabled = False
        MaxLength = 5
        TabOrder = 1
        Text = '0'
        OnExit = edtLogLinearExit
        OnKeyPress = FloatKeyPress
      end
      object lblLogLinear: TStaticText
        Left = 173
        Top = 70
        Width = 67
        Height = 17
        AutoSize = False
        Caption = '%  per year'
        TabOrder = 2
      end
      object grpAdjustments: TGroupBox
        Left = 8
        Top = 189
        Width = 421
        Height = 85
        Caption = 'Temporal, Spatial and/or Space-Time Adjustments'
        TabOrder = 4
        object lblAdjustmentsByRelativeRisksFile: TLabel
          Left = 30
          Top = 37
          Width = 79
          Height = 13
          Anchors = []
          Caption = 'Adjustments File:'
        end
        object edtAdjustmentsByRelativeRisksFile: TEdit
          Left = 30
          Top = 54
          Width = 320
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 1
          OnChange = edtAdjustmentsByRelativeRisksFileChange
          OnExit = OnControlExit
        end
        object btnBrowseAdjustmentsFile: TButton
          Left = 356
          Top = 54
          Width = 23
          Height = 22
          Hint = 'Browse for adjustments file ...'
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
          Top = 17
          Width = 233
          Height = 17
          Caption = 'Adjust for known relative risks'
          TabOrder = 0
          OnClick = chkAdjustForKnownRelativeRisksClick
        end
        object btnImportAdjustmentsFile: TBitBtn
          Left = 384
          Top = 54
          Width = 23
          Height = 22
          Hint = 'Import adjustments file ...'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 3
          OnClick = btnImportAdjustmentsFileClick
          Glyph.Data = {
            DE010000424DDE01000000000000760000002800000024000000120000000100
            04000000000068010000C40E0000C40E00001000000000000000000000000000
            80000080000000808000800000008000800080800000C0C0C000808080000000
            FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00666666666666
            0000066666666666668888860000666666846668777780666666886668777788
            0000666666644668FF7780666666688668FF77880000666844444468F7F78066
            6888888868F7F7880000666486644668FF7780666886688668FF778800006664
            86846668F7F780666886886668F7F7880000666486666668FF77806668866666
            68FF778800006666666666688888806666666666688888880000000000006668
            FF7F70888888886668FF7F780000877777706666888886877777786666888886
            00008F0F0F7066666666668F7F7F786666666666000080F0F070666666666687
            F7F778666666666600008F0F0F7066666666668F7F7F786666666666000080F0
            F070666666666687F7F778666666666600008F0F000066666666668F7F888866
            66666666000080F07F86666666666687F77F86666666666600008F0F78666666
            6666668F7F786666666666660000888886666666666666888886666666666666
            0000}
          NumGlyphs = 2
        end
      end
      object rdgSpatialAdjustments: TRadioGroup
        Left = 8
        Top = 121
        Width = 421
        Height = 63
        Caption = 'Spatial Adjustments'
        ItemIndex = 0
        Items.Strings = (
          'None'
          'Nonparametric, with spatial stratified randomization')
        TabOrder = 3
        OnClick = rdgSpatialAdjustmentsClick
        OnExit = OnControlExit
      end
    end
    object tsInference: TTabSheet
      Caption = 'Inference'
      ImageIndex = 4
      object grpAnalysis: TGroupBox
        Left = 8
        Top = 8
        Width = 421
        Height = 50
        Caption = 'Early Termination'
        TabOrder = 0
        object chkTerminateEarly: TCheckBox
          Left = 12
          Top = 22
          Width = 245
          Height = 17
          Caption = 'Terminate the analysis early for large p-values'
          TabOrder = 0
          OnClick = OnControlExit
        end
      end
      object gbxProspectiveSurveillance: TGroupBox
        Left = 8
        Top = 61
        Width = 417
        Height = 68
        Caption = 'Prospective Surveillance'
        TabOrder = 1
        object lblProspectiveStartYear: TLabel
          Left = 264
          Top = 18
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
          Left = 316
          Top = 18
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
          Left = 367
          Top = 18
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
        object edtProspectiveStartDateYear: TEdit
          Left = 257
          Top = 29
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
          Left = 317
          Top = 29
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
          Left = 363
          Top = 29
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
          Top = 21
          Width = 227
          Height = 17
          Caption = 'Adjust for earlier analyses performed since:'
          TabOrder = 0
          OnClick = chkAdjustForEarlierAnalysesClick
        end
      end
      object grpReportCriticalValues: TGroupBox
        Left = 8
        Top = 132
        Width = 421
        Height = 50
        Caption = 'Critical Values'
        TabOrder = 2
        object chkReportCriticalValues: TCheckBox
          Left = 12
          Top = 22
          Width = 389
          Height = 17
          Caption = 'Report critical values for an observed cluster to be significant'
          TabOrder = 0
          OnClick = OnControlExit
        end
      end
      object grpIterativeScan: TGroupBox
        Left = 8
        Top = 184
        Width = 417
        Height = 91
        Caption = 'Iterative Scan'
        TabOrder = 3
        object lblMaxIterativeScans: TLabel
          Left = 45
          Top = 43
          Width = 142
          Height = 13
          Caption = 'Maximum number of iterations:'
        end
        object lblIterativeCutoff: TLabel
          Left = 45
          Top = 67
          Width = 169
          Height = 13
          AutoSize = False
          Caption = 'Stop when p-value is greater than:'
        end
        object edtNumIterativeScans: TEdit
          Left = 219
          Top = 37
          Width = 41
          Height = 21
          Enabled = False
          MaxLength = 4
          TabOrder = 1
          Text = '10'
          OnExit = edtNumIterativeScansExit
          OnKeyPress = NaturalNumberKeyPress
        end
        object chkPerformIterativeScan: TCheckBox
          Left = 14
          Top = 22
          Width = 210
          Height = 14
          Caption = 'Adjust for more likely clusters'
          TabOrder = 0
          OnClick = chkPerformIterativeScanClick
        end
        object edtIterativeScanCutoff: TEdit
          Left = 219
          Top = 63
          Width = 41
          Height = 21
          Enabled = False
          TabOrder = 2
          Text = '.5'
          OnExit = edtIterativeScanCutoffExit
          OnKeyPress = PositiveFloatKeyPress
        end
      end
    end
    object tsClustersReported: TTabSheet
      Caption = 'Clusters Reported'
      ImageIndex = 2
      object rdgCriteriaSecClusters: TRadioGroup
        Left = 8
        Top = 8
        Width = 421
        Height = 142
        Caption = 'Criteria for Reporting Secondary Clusters'
        Items.Strings = (
          'No Geographical Overlap'
          'No Cluster Centers in Other Clusters'
          'No Cluster Centers in More Likely Clusters'
          'No Cluster Centers in Less Likely Clusters'
          'No Pairs of Centers Both in Each Others Clusters'
          'No Restrictions = Most Likely Cluster for Each Grid Point')
        TabOrder = 0
        OnClick = OnControlExit
      end
      object rdgReportedSpatialOptions: TGroupBox
        Left = 8
        Top = 155
        Width = 425
        Height = 121
        Caption = 'Maximum Reported Spatial Cluster Size'
        TabOrder = 1
        object lblReportedPercentOfPopulation: TLabel
          Left = 64
          Top = 42
          Width = 263
          Height = 13
          Caption = 'percent of the population at risk (<= 50%, default = 50%)'
        end
        object lblMaxReportedRadius: TLabel
          Left = 211
          Top = 94
          Width = 73
          Height = 13
          Caption = 'kilometer radius'
        end
        object lblReportedPercentageOfPopFile: TLabel
          Left = 93
          Top = 67
          Width = 316
          Height = 13
          Caption = 
            'percent of the population defined in the max circle size file (<' +
            '= 50%)'
        end
        object lblReportedMaxDistance: TLabel
          Left = 53
          Top = 94
          Width = 85
          Height = 13
          Caption = 'is an ellipse with a'
        end
        object edtMaxReportedSpatialClusterSize: TEdit
          Left = 25
          Top = 40
          Width = 34
          Height = 21
          AutoSelect = False
          MaxLength = 5
          TabOrder = 0
          Text = '50'
          OnExit = edtMaxReportedSpatialClusterSizeExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object edtMaxReportedSpatialRadius: TEdit
          Left = 144
          Top = 91
          Width = 58
          Height = 21
          TabOrder = 2
          Text = '1'
          OnExit = edtMaxReportedSpatialRadiusExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object edtMaxReportedSpatialPercentFile: TEdit
          Left = 54
          Top = 66
          Width = 34
          Height = 21
          MaxLength = 5
          TabOrder = 1
          Text = '50'
          OnExit = edtMaxReportedSpatialPercentFileExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object chkRestrictReportedClusters: TCheckBox
          Left = 8
          Top = 18
          Width = 181
          Height = 17
          Caption = 'Report only clusters smaller than:'
          TabOrder = 3
          OnClick = chkRestrictReportedClustersClick
        end
        object chkReportedSpatialDistance: TCheckBox
          Left = 33
          Top = 93
          Width = 15
          Height = 17
          TabOrder = 4
          OnClick = chkRestrictReportedClustersClick
        end
        object chkReportedSpatialPopulationFile: TCheckBox
          Left = 33
          Top = 68
          Width = 15
          Height = 17
          Caption = 'chkReportedSpatialPopulationFile'
          TabOrder = 5
          OnClick = chkRestrictReportedClustersClick
        end
      end
    end
  end
  object OpenDialog: TOpenDialog
    Left = 472
    Top = 256
  end
end
