object frmAdvancedParameters: TfrmAdvancedParameters
  Left = 187
  Top = 168
  ActiveControl = PageControl
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Advanced Features'
  ClientHeight = 311
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
    Height = 311
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
    Height = 311
    ActivePage = tsDataStreams
    Align = alClient
    TabOrder = 0
    object tsDataStreams: TTabSheet
      Caption = 'Multiple Data Sets'
      ImageIndex = 5
      object grpDataStreams: TGroupBox
        Left = 8
        Top = 8
        Width = 425
        Height = 233
        Caption = 'Additional Input Data Sets'
        TabOrder = 0
        object Label3: TLabel
          Left = 112
          Top = 18
          Width = 46
          Height = 13
          Hint = 'Additional Case File Name'
          Caption = 'Case File:'
          ParentShowHint = False
          ShowHint = False
        end
        object Label1: TLabel
          Left = 112
          Top = 60
          Width = 55
          Height = 13
          Caption = 'Control File:'
        end
        object Label2: TLabel
          Left = 112
          Top = 102
          Width = 72
          Height = 13
          Caption = 'Population File:'
        end
        object Label4: TLabel
          Left = 218
          Top = 60
          Width = 78
          Height = 13
          Caption = '(Bernoulli Model)'
        end
        object Label5: TLabel
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
        object lstInputStreams: TListBox
          Left = 8
          Top = 32
          Width = 95
          Height = 78
          Hint = 'All Additional Input Sets'
          ItemHeight = 13
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnClick = lstInputStreamsClick
        end
        object btnNewStream: TButton
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
        object btnRemoveStream: TButton
          Left = 57
          Top = 116
          Width = 47
          Height = 22
          Hint = 'Remove an Additional Input Set'
          Caption = 'Remove'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
          OnClick = btnRemoveStreamClick
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
        object lblMultipleStreamPurpose: TStaticText
          Left = 8
          Top = 152
          Width = 164
          Height = 17
          Caption = 'Purpose of Multiple Data Streams:'
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
        object rdoAdjustmentByStreams: TRadioButton
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
    object tbSpatial: TTabSheet
      Caption = 'Spatial Window'
      ImageIndex = 3
      object rdgSpatialOptions: TGroupBox
        Left = 8
        Top = 8
        Width = 425
        Height = 146
        Caption = 'Maximum Spatial Cluster Size'
        TabOrder = 0
        object lblPercentOfPopulation: TLabel
          Left = 78
          Top = 25
          Width = 263
          Height = 13
          Caption = 'percent of the population at risk (<= 50%, default = 50%)'
        end
        object lblMaxRadius: TLabel
          Left = 174
          Top = 111
          Width = 73
          Height = 13
          Caption = 'kilometer radius'
        end
        object lblPercentageOfPopFile: TLabel
          Left = 78
          Top = 49
          Width = 316
          Height = 13
          Caption = 
            'percent of the population defined in the max circle size file (<' +
            '= 50%)'
          WordWrap = True
        end
        object edtMaxSpatialClusterSize: TEdit
          Left = 39
          Top = 23
          Width = 34
          Height = 21
          AutoSelect = False
          MaxLength = 5
          TabOrder = 1
          Text = '50'
          OnChange = edtMaxSpatialClusterSizeChange
          OnExit = edtMaxSpatialClusterSizeExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object rdoSpatialPercentage: TRadioButton
          Left = 7
          Top = 23
          Width = 28
          Height = 17
          Caption = 'is '
          Checked = True
          TabOrder = 0
          TabStop = True
          OnClick = rdoMaxSpatialTypeClick
        end
        object rdoSpatialDistance: TRadioButton
          Left = 7
          Top = 111
          Width = 97
          Height = 17
          Caption = 'is a circle with a'
          TabOrder = 7
          OnClick = rdoMaxSpatialTypeClick
        end
        object rdoSpatialPopulationFile: TRadioButton
          Left = 7
          Top = 49
          Width = 28
          Height = 17
          Caption = 'is'
          TabOrder = 2
          OnClick = rdoMaxSpatialTypeClick
        end
        object edtMaxSpatialRadius: TEdit
          Left = 110
          Top = 110
          Width = 58
          Height = 21
          TabOrder = 8
          Text = '1'
          OnChange = edtMaxSpatialRadiusChange
          OnExit = edtMaxSpatialRadiusExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object edtMaxSpatialPercentFile: TEdit
          Left = 39
          Top = 49
          Width = 34
          Height = 21
          MaxLength = 5
          TabOrder = 3
          Text = '50'
          OnChange = edtMaxSpatialPercentFileChange
          OnExit = edtMaxSpatialPercentFileExit
          OnKeyPress = PositiveFloatKeyPress
        end
        object edtMaxCirclePopulationFilename: TEdit
          Left = 26
          Top = 81
          Width = 280
          Height = 21
          ParentShowHint = False
          ShowHint = True
          TabOrder = 4
          OnChange = edtMaxCirclePopulationFilenameChange
          OnExit = OnControlExit
        end
        object btnBrowseMaxCirclePopFile: TButton
          Left = 312
          Top = 81
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
          TabOrder = 5
          OnClick = btnBrowseMaxCirclePopFileClick
        end
        object btnImportMaxCirclePopFile: TBitBtn
          Left = 340
          Top = 81
          Width = 23
          Height = 22
          Hint = 'Import max circle size file ...'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 6
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
      end
      object chkInclPureTempClust: TCheckBox
        Left = 8
        Top = 165
        Width = 305
        Height = 17
        Caption = 'Include Purely Temporal Clusters (Spatial Size = 100%)'
        Enabled = False
        TabOrder = 1
        OnClick = OnControlExit
      end
    end
    object tsTemporal: TTabSheet
      Caption = 'Temporal Window'
      ImageIndex = 1
      object grpFlexibleTemporalWindowDefinition: TGroupBox
        Left = 8
        Top = 131
        Width = 421
        Height = 111
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
        Height = 84
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
          Top = 50
          Width = 25
          Height = 13
          Caption = 'years'
        end
        object edtMaxTemporalClusterSize: TEdit
          Left = 39
          Top = 23
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
          Top = 49
          Width = 28
          Height = 17
          Caption = 'is'
          TabOrder = 2
          OnClick = rdoMaxTemporalClusterSizelick
        end
        object edtMaxTemporalClusterSizeUnits: TEdit
          Left = 39
          Top = 49
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
        Top = 104
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
        Hint = 'Temporal Trend Adjustments'
        Caption = 'Temporal Adjustment'
        Enabled = False
        ItemIndex = 0
        Items.Strings = (
          'None'
          'Nonparametric, with time stratified randomization'
          'Log linear trend with'
          'Log linear with automatically calculated trend')
        ParentShowHint = False
        ShowHint = True
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
        OnClick = OnControlExit
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
        Top = 76
        Width = 417
        Height = 103
        Caption = 'Prospective Surveillance'
        TabOrder = 1
        object lblProspectiveStartYear: TLabel
          Left = 48
          Top = 53
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
          Left = 100
          Top = 53
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
          Left = 151
          Top = 53
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
          Left = 41
          Top = 64
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
          Left = 101
          Top = 64
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
          Left = 147
          Top = 64
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
          Width = 227
          Height = 17
          Caption = 'Adjust for earlier analyses performed since:'
          TabOrder = 0
          OnClick = chkAdjustForEarlierAnalysesClick
        end
      end
    end
    object tsClustersReported: TTabSheet
      Caption = 'Clusters Reported'
      ImageIndex = 2
      object lblReportSmallerClusters: TLabel
        Left = 221
        Top = 222
        Width = 213
        Height = 27
        AutoSize = False
        Caption = 'percent of population at risk.'
        WordWrap = True
      end
      object chkRestrictReportedClusters: TCheckBox
        Left = 8
        Top = 222
        Width = 175
        Height = 17
        Caption = 'Report only clusters smaller than'
        TabOrder = 0
        OnClick = chkRestrictReportedClustersClick
      end
      object edtReportClustersSmallerThan: TEdit
        Left = 183
        Top = 220
        Width = 34
        Height = 21
        TabOrder = 1
        Text = '50'
        OnExit = edtReportClustersSmallerThanExit
        OnKeyPress = PositiveFloatKeyPress
      end
      object rdgCriteriaSecClusters: TRadioGroup
        Left = 8
        Top = 8
        Width = 421
        Height = 197
        Caption = 'Criteria for Reporting Secondary Clusters'
        Items.Strings = (
          'No Geographical Overlap'
          'No Cluster Centers in Other Clusters'
          'No Cluster Centers in More Likely Clusters'
          'No Cluster Centers in Less Likely Clusters'
          'No Pairs of Centers Both in Each Others Clusters'
          'No Restrictions = Most Likely Cluster for Each Grid Point')
        TabOrder = 2
        OnClick = OnControlExit
      end
    end
  end
  object OpenDialog: TOpenDialog
    Left = 472
    Top = 256
  end
end
