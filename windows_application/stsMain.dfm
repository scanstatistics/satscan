object frmMainForm: TfrmMainForm
  Left = 112
  Top = 136
  Width = 783
  Height = 529
  Caption = 'SatScan'
  Color = clGray
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsMDIForm
  Menu = MainMenu1
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Bevel1: TBevel
    Left = 0
    Top = 0
    Width = 775
    Height = 2
    Align = alTop
  end
  object barStatusBar: TStatusBar
    Left = 0
    Top = 459
    Width = 775
    Height = 24
    Panels = <
      item
        Width = 575
      end
      item
        Width = 75
      end
      item
        Width = 50
      end>
    SimplePanel = False
  end
  object pnlSpeedButton: TPanel
    Left = 0
    Top = 2
    Width = 775
    Height = 31
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 1
    object sbOpen: TSpeedButton
      Left = 48
      Top = 4
      Width = 25
      Height = 25
      Hint = 'Open'
      Flat = True
      Glyph.Data = {
        5A010000424D5A01000000000000760000002800000013000000130000000100
        040000000000E400000000000000000000001000000010000000000000000000
        80000080000000808000800000008000800080800000C0C0C000808080000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00777777777777
        7777777000007777777777777777777000007777777777777777777000007700
        00000000077777700000770033333333307777700000770B0333333333077770
        0000770FB0333333333077700000770BFB033333333307700000770FBFB00000
        000000700000770BFBFBFBFB077777700000770FBFBFBFBF077777700000770B
        FB00000007777770000077700077777777000770000077777777777777700770
        0000777777777707770707700000777777777770007777700000777777777777
        777777700000777777777777777777700000777777777777777777700000}
      ParentShowHint = False
      ShowHint = True
      OnClick = sbOpenClick
    end
    object sbSave: TSpeedButton
      Left = 80
      Top = 4
      Width = 25
      Height = 25
      Hint = 'Save'
      Flat = True
      Glyph.Data = {
        5A010000424D5A01000000000000760000002800000013000000130000000100
        040000000000E400000000000000000000001000000010000000000000000000
        80000080000000808000800000008000800080800000C0C0C000808080000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00777777777777
        7777777000007777777777777777777000007777000000000000077000007770
        3300000077030770000077703300000077030770000077703300000077030770
        0000777033000000000307700000777033333333333307700000777033000000
        0033077000007770307777777703077000007770307777777703077000007770
        3077777777030770000077703077777777030770000077703077777777000770
        0000777030777777770707700000777000000000000007700000777777777777
        777777700000777777777777777777700000777777777777777777700000}
      ParentShowHint = False
      ShowHint = True
      OnClick = sbSaveClick
    end
    object sbHelp: TSpeedButton
      Left = 160
      Top = 4
      Width = 25
      Height = 25
      Hint = 'Help'
      Flat = True
      Glyph.Data = {
        5A010000424D5A01000000000000760000002800000013000000130000000100
        040000000000E400000000000000000000001000000010000000000000000000
        80000080000000808000800000008000800080800000C0C0C000808080000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00777777777777
        7777777000007777777777777777777000007777777777777777777000007777
        7777000777777770000077777770BB0077777770000077777770BB0077777770
        000077777777007777777770000077777770B00777777770000077777770B007
        77777770000077777770B00777777770000077777770BB007777777000007777
        70070BB007777770000077770B0070BB00777770000077770B00770B00777770
        000077770BB000BB007777700000777770BBBBB0077777700000777777000000
        777777700000777777777777777777700000777777777777777777700000}
      ParentShowHint = False
      ShowHint = True
      OnClick = sbHelpClick
    end
    object sbExecute: TSpeedButton
      Left = 120
      Top = 4
      Width = 25
      Height = 25
      Hint = 'Execute'
      Flat = True
      Glyph.Data = {
        5A010000424D5A01000000000000760000002800000013000000130000000100
        040000000000E400000000000000000000001000000010000000000000000000
        80000080000000808000800000008000800080800000C0C0C000808080000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00777777777777
        7777777000007777777777777777777000007777777077777777777000007777
        77780777777777700000777777770077777777700000777777770B0777777770
        00007777770000B07777777000007777770BBBBB07777770000077777770FB00
        00777770000077777770BBB0777777700000777700000BFB0777777000007777
        0FBBBFBBB07777700000777770FBFB00000777700000777770BFBFB077777770
        00007777770BFBFB0777777000007777770FFFFFF07777700000777777700000
        000777700000777777777777777777700000777777777777777777700000}
      ParentShowHint = False
      ShowHint = True
      OnClick = sbExecuteClick
    end
    object sbEditSession: TSpeedButton
      Left = 8
      Top = 4
      Width = 25
      Height = 25
      Hint = 'SatScan Session'
      Flat = True
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000120B0000120B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00555555555555
        555555555555555555555555555555555555555555FF55555555555559055555
        55555555577FF5555555555599905555555555557777F5555555555599905555
        555555557777FF5555555559999905555555555777777F555555559999990555
        5555557777777FF5555557990599905555555777757777F55555790555599055
        55557775555777FF5555555555599905555555555557777F5555555555559905
        555555555555777FF5555555555559905555555555555777FF55555555555579
        05555555555555777FF5555555555557905555555555555777FF555555555555
        5990555555555555577755555555555555555555555555555555}
      NumGlyphs = 2
      ParentShowHint = False
      ShowHint = True
      OnClick = sbEditSessionClick
    end
    object Bevel2: TBevel
      Left = 40
      Top = 4
      Width = 2
      Height = 23
    end
    object Bevel3: TBevel
      Left = 112
      Top = 4
      Width = 2
      Height = 23
    end
    object Bevel4: TBevel
      Left = 152
      Top = 4
      Width = 2
      Height = 23
    end
  end
  object MainMenu1: TMainMenu
    Left = 40
    Top = 72
    object File1: TMenuItem
      Caption = 'File'
      object New1: TMenuItem
        Caption = 'New'
        OnClick = New1Click
      end
      object Open1: TMenuItem
        Caption = 'Open'
        object ParameterFile1: TMenuItem
          Caption = 'Parameter File'
          OnClick = ParameterFile1Click
        end
        object OutputTextFile1: TMenuItem
          Caption = 'Output Text File'
          Enabled = False
          OnClick = OutputTextFile1Click
        end
      end
      object Close1: TMenuItem
        Caption = 'Close'
        OnClick = Close1Click
      end
      object N2: TMenuItem
        Caption = '-'
      end
      object Save1: TMenuItem
        Caption = 'Save'
        OnClick = Save1Click
      end
      object SaveAs1: TMenuItem
        Caption = 'Save as'
        OnClick = SaveAs1Click
      end
      object Exit1: TMenuItem
        Caption = '-'
      end
      object Print1: TMenuItem
        Caption = 'Print...'
        Enabled = False
        OnClick = Print1Click
      end
      object PrintSetup1: TMenuItem
        Caption = 'Print Setup...'
        OnClick = PrintSetup1Click
      end
      object N3: TMenuItem
        Caption = '-'
      end
      object Exit2: TMenuItem
        Caption = 'Exit'
      end
    end
    object Session1: TMenuItem
      Caption = 'Session'
      object Execute1: TMenuItem
        Caption = 'Execute'
        OnClick = Execute1Click
      end
    end
    object Help1: TMenuItem
      Caption = 'Help'
      object Contents1: TMenuItem
        Caption = 'Contents'
        OnClick = Contents1Click
      end
      object UsingHelp1: TMenuItem
        Caption = 'Using Help'
        OnClick = UsingHelp1Click
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object AboutSatScan1: TMenuItem
        Caption = 'About SatScan'
        OnClick = AboutSatScan1Click
      end
    end
  end
  object OpenDialog1: TOpenDialog
    Left = 88
    Top = 72
  end
  object SaveDialog1: TSaveDialog
    Left = 128
    Top = 72
  end
  object PrintDialog1: TPrintDialog
    Left = 176
    Top = 72
  end
  object PrinterSetupDialog1: TPrinterSetupDialog
    Left = 216
    Top = 72
  end
end
