object frmAnalysis: TfrmAnalysis
  Left = 182
  Top = 78
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'SaTScan Analysis Settings'
  ClientHeight = 399
  ClientWidth = 514
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
    Width = 507
    Height = 389
    ActivePage = tbAnalysis
    TabOrder = 0
    object tbInputFiles: TTabSheet
      Caption = 'Input'
      object Label4: TLabel
        Left = 15
        Top = 175
        Width = 72
        Height = 13
        Caption = 'Population File:'
      end
      object Label5: TLabel
        Left = 128
        Top = 175
        Width = 75
        Height = 13
        Caption = '(Poisson Model)'
      end
      object grpCountData: TGroupBox
        Left = 8
        Top = 6
        Width = 485
        Height = 163
        Color = clBtnFace
        ParentColor = False
        TabOrder = 0
        object Label1: TLabel
          Left = 8
          Top = 12
          Width = 46
          Height = 13
          Caption = 'Case File:'
        end
        object Label2: TLabel
          Left = 8
          Top = 54
          Width = 55
          Height = 13
          Caption = 'Control File:'
          Color = clBtnFace
          ParentColor = False
        end
        object Label3: TLabel
          Left = 120
          Top = 54
          Width = 78
          Height = 13
          Caption = '(Bernoulli Model)'
        end
        object rgpPrecisionTimes: TRadioGroup
          Left = 373
          Top = 20
          Width = 100
          Height = 70
          Caption = 'Time Included'
          ItemIndex = 0
          Items.Strings = (
            'Yes'
            'No')
          TabOrder = 6
          OnClick = rgpPrecisionTimesClick
        end
        object edtCaseFileName: TEdit
          Left = 8
          Top = 26
          Width = 284
          Height = 22
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = edtCaseFileNameChange
        end
        object edtControlFileName: TEdit
          Left = 8
          Top = 68
          Width = 284
          Height = 22
          ParentShowHint = False
          ShowHint = True
          TabOrder = 3
          OnChange = edtControlFileNameChange
        end
        object btnCaseBrowse: TButton
          Left = 298
          Top = 26
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
          TabOrder = 1
          OnClick = btnCaseBrowseClick
        end
        object btnControlBrowse: TButton
          Left = 298
          Top = 68
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
          TabOrder = 4
          OnClick = btnControlBrowseClick
        end
        object grpStudyPeriod: TGroupBox
          Left = 8
          Top = 96
          Width = 465
          Height = 53
          Caption = 'Study Period'
          TabOrder = 7
          object Label8: TLabel
            Left = 38
            Top = 27
            Width = 51
            Height = 13
            Caption = 'Start Date:'
          end
          object Label10: TLabel
            Left = 100
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
            Left = 138
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
            Left = 176
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
          object Label9: TLabel
            Left = 233
            Top = 27
            Width = 48
            Height = 13
            Caption = 'End Date:'
          end
          object Label15: TLabel
            Left = 292
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
            Left = 331
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
            Left = 369
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
          object edtStudyPeriodStartDateYear: TEdit
            Left = 96
            Top = 24
            Width = 33
            Height = 21
            MaxLength = 4
            TabOrder = 0
            Text = '1900'
            OnExit = edtStudyPeriodStartDateExit
            OnKeyPress = NaturalNumberKeyPress
          end
          object edtStudyPeriodStartDateMonth: TEdit
            Left = 139
            Top = 24
            Width = 25
            Height = 21
            Enabled = False
            MaxLength = 2
            TabOrder = 1
            Text = '1'
            OnExit = edtStudyPeriodStartDateExit
            OnKeyPress = NaturalNumberKeyPress
          end
          object edtStudyPeriodStartDateDay: TEdit
            Left = 174
            Top = 24
            Width = 25
            Height = 21
            Enabled = False
            MaxLength = 2
            TabOrder = 2
            Text = '1'
            OnExit = edtStudyPeriodStartDateExit
            OnKeyPress = NaturalNumberKeyPress
          end
          object edtStudyPeriodEndDateYear: TEdit
            Left = 289
            Top = 24
            Width = 33
            Height = 21
            MaxLength = 4
            TabOrder = 3
            Text = '1900'
            OnExit = edtStudyPeriodEndDateExit
            OnKeyPress = NaturalNumberKeyPress
          end
          object edtStudyPeriodEndDateMonth: TEdit
            Left = 332
            Top = 24
            Width = 25
            Height = 21
            Enabled = False
            MaxLength = 2
            TabOrder = 4
            Text = '12'
            OnExit = edtStudyPeriodEndDateExit
            OnKeyPress = NaturalNumberKeyPress
          end
          object edtStudyPeriodEndDateDay: TEdit
            Left = 366
            Top = 24
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
        object btnCaseImport: TBitBtn
          Left = 326
          Top = 26
          Width = 23
          Height = 22
          Hint = 'Import case file ...'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
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
        object btnControlImport: TBitBtn
          Left = 326
          Top = 68
          Width = 23
          Height = 22
          Hint = 'Import control file ...'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 5
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
      end
      object btnAdvanced3_Yes: TBitBtn
        Left = 395
        Top = 328
        Width = 90
        Height = 24
        Hint = 'Advanced Input Features'
        Caption = 'Advanced...'
        ParentShowHint = False
        ShowHint = True
        TabOrder = 6
        Visible = False
        OnClick = btnAdvancedParametersClick
        Glyph.Data = {
          DE010000424DDE01000000000000760000002800000024000000120000000100
          0400000000006801000000000000000000001000000000000000000000000000
          80000080000000808000800000008000800080800000C0C0C000808080000000
          FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
          3333333333333333333333330000333333333333333333333333F33333333333
          00003333344333333333333333388F3333333333000033334224333333333333
          338338F3333333330000333422224333333333333833338F3333333300003342
          222224333333333383333338F3333333000034222A22224333333338F338F333
          8F33333300003222A3A2224333333338F3838F338F33333300003A2A333A2224
          33333338F83338F338F33333000033A33333A222433333338333338F338F3333
          0000333333333A222433333333333338F338F33300003333333333A222433333
          333333338F338F33000033333333333A222433333333333338F338F300003333
          33333333A222433333333333338F338F00003333333333333A22433333333333
          3338F38F000033333333333333A223333333333333338F830000333333333333
          333A333333333333333338330000333333333333333333333333333333333333
          0000}
        NumGlyphs = 2
      end
      object edtPopFileName: TEdit
        Left = 16
        Top = 189
        Width = 284
        Height = 22
        ParentShowHint = False
        ShowHint = True
        TabOrder = 1
        OnChange = edtPopFileNameChange
      end
      object btnPopBrowse: TButton
        Left = 306
        Top = 189
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
        TabOrder = 2
        OnClick = btnPopBrowseClick
      end
      object grpGeographical: TGroupBox
        Left = 8
        Top = 216
        Width = 485
        Height = 101
        TabOrder = 4
        object Label6: TLabel
          Left = 8
          Top = 10
          Width = 78
          Height = 13
          Caption = 'Coordinates File:'
        end
        object Label7: TLabel
          Left = 8
          Top = 54
          Width = 125
          Height = 13
          Caption = 'Special Grid File: (optional)'
        end
        object edtCoordinateFileName: TEdit
          Left = 8
          Top = 24
          Width = 284
          Height = 22
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = edtCoordinateFileNameChange
        end
        object edtGridFileName: TEdit
          Left = 8
          Top = 68
          Width = 284
          Height = 22
          ParentShowHint = False
          ShowHint = True
          TabOrder = 3
          OnChange = edtGridFileNameChange
        end
        object btnCoordBrowse: TButton
          Left = 298
          Top = 24
          Width = 23
          Height = 22
          Hint = 'Browse for coordinates file ...'
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
          Left = 298
          Top = 68
          Width = 23
          Height = 22
          Hint = 'Browse for special grid file ...'
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
          OnClick = btnGridBrowseClick
        end
        object rgpCoordinates: TRadioGroup
          Left = 354
          Top = 16
          Width = 122
          Height = 75
          Caption = 'Coordinates'
          ItemIndex = 0
          Items.Strings = (
            'Cartesian'
            'Latitude/Longitude')
          TabOrder = 6
          OnClick = rgpCoordinatesClick
        end
        object btnCoordImport: TBitBtn
          Left = 326
          Top = 24
          Width = 23
          Height = 22
          Hint = 'Import coordinates file ...'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 2
          OnClick = btnCoordImportClick
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
        object btnGridImport: TBitBtn
          Left = 326
          Top = 68
          Width = 23
          Height = 22
          Hint = 'Import special grid file ...'
          ParentShowHint = False
          ShowHint = True
          TabOrder = 5
          OnClick = btnGridImportClick
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
      object btnAdvanced3_No: TBitBtn
        Left = 395
        Top = 328
        Width = 90
        Height = 24
        Hint = 'Advanced Input Features'
        Caption = 'Advanced...'
        ParentShowHint = False
        ShowHint = True
        TabOrder = 5
        OnClick = btnAdvancedParametersClick
        NumGlyphs = 2
      end
      object btnPopImport: TBitBtn
        Left = 334
        Top = 189
        Width = 23
        Height = 22
        Hint = 'Import population file ...'
        ParentShowHint = False
        ShowHint = True
        TabOrder = 3
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
        Left = 332
        Top = 285
        Width = 65
        Height = 21
        BiDiMode = bdLeftToRight
        MaxLength = 9
        ParentBiDiMode = False
        TabOrder = 4
        Text = '999'
        OnExit = edtMontCarloRepsExit
        OnKeyPress = NaturalNumberKeyPress
      end
      object rgpTypeProbability: TGroupBox
        Left = 160
        Top = 8
        Width = 164
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
        Left = 340
        Top = 8
        Width = 145
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
        Left = 340
        Top = 140
        Width = 145
        Height = 121
        Caption = 'Time Intervals'
        TabOrder = 3
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
      object btnAdvanced1_Yes: TBitBtn
        Left = 395
        Top = 328
        Width = 90
        Height = 24
        Hint = 'Advanced Analysis Features'
        Caption = 'Advanced...'
        ParentShowHint = False
        ShowHint = True
        TabOrder = 5
        Visible = False
        OnClick = btnAdvancedParametersClick
        Glyph.Data = {
          DE010000424DDE01000000000000760000002800000024000000120000000100
          0400000000006801000000000000000000001000000000000000000000000000
          80000080000000808000800000008000800080800000C0C0C000808080000000
          FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
          3333333333333333333333330000333333333333333333333333F33333333333
          00003333344333333333333333388F3333333333000033334224333333333333
          338338F3333333330000333422224333333333333833338F3333333300003342
          222224333333333383333338F3333333000034222A22224333333338F338F333
          8F33333300003222A3A2224333333338F3838F338F33333300003A2A333A2224
          33333338F83338F338F33333000033A33333A222433333338333338F338F3333
          0000333333333A222433333333333338F338F33300003333333333A222433333
          333333338F338F33000033333333333A222433333333333338F338F300003333
          33333333A222433333333333338F338F00003333333333333A22433333333333
          3338F38F000033333333333333A223333333333333338F830000333333333333
          333A333333333333333338330000333333333333333333333333333333333333
          0000}
        NumGlyphs = 2
      end
      object btnAdvanced1_No: TBitBtn
        Left = 395
        Top = 328
        Width = 90
        Height = 24
        Hint = 'Advanced Analysis Features'
        Caption = 'Advanced...'
        ParentShowHint = False
        ShowHint = True
        TabOrder = 6
        OnClick = btnAdvancedParametersClick
        NumGlyphs = 2
      end
    end
    object tbOutputFiles: TTabSheet
      Caption = 'Output'
      ImageIndex = 4
      object gbxAdditionalOutputFiles: TGroupBox
        Left = 8
        Top = 59
        Width = 477
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
        Width = 499
        Height = 58
        Align = alTop
        BevelOuter = bvNone
        TabOrder = 0
        object Label29: TLabel
          Left = 8
          Top = 10
          Width = 57
          Height = 13
          Caption = 'Results File:'
        end
        object edtResultFile: TEdit
          Left = 8
          Top = 26
          Width = 391
          Height = 22
          AutoSelect = False
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = edtResultFileChange
        end
        object btnResultFileBrowse: TButton
          Left = 406
          Top = 26
          Width = 23
          Height = 22
          Hint = 'Browse for results file ...'
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
      object btnAdvanced2_Yes: TBitBtn
        Left = 395
        Top = 328
        Width = 90
        Height = 24
        Hint = 'Advanced Output Features'
        Caption = 'Advanced...'
        ParentShowHint = False
        ShowHint = True
        TabOrder = 3
        Visible = False
        OnClick = btnAdvancedParametersClick
        Glyph.Data = {
          DE010000424DDE01000000000000760000002800000024000000120000000100
          0400000000006801000000000000000000001000000000000000000000000000
          80000080000000808000800000008000800080800000C0C0C000808080000000
          FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
          3333333333333333333333330000333333333333333333333333F33333333333
          00003333344333333333333333388F3333333333000033334224333333333333
          338338F3333333330000333422224333333333333833338F3333333300003342
          222224333333333383333338F3333333000034222A22224333333338F338F333
          8F33333300003222A3A2224333333338F3838F338F33333300003A2A333A2224
          33333338F83338F338F33333000033A33333A222433333338333338F338F3333
          0000333333333A222433333333333338F338F33300003333333333A222433333
          333333338F338F33000033333333333A222433333333333338F338F300003333
          33333333A222433333333333338F338F00003333333333333A22433333333333
          3338F38F000033333333333333A223333333333333338F830000333333333333
          333A333333333333333338330000333333333333333333333333333333333333
          0000}
        NumGlyphs = 2
      end
      object btnAdvanced2_No: TBitBtn
        Left = 395
        Top = 328
        Width = 90
        Height = 24
        Hint = 'Advanced Output Features'
        Caption = 'Advanced...'
        ParentShowHint = False
        ShowHint = True
        TabOrder = 2
        OnClick = btnAdvancedParametersClick
        NumGlyphs = 2
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
