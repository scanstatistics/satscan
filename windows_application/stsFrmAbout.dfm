object frmAbout: TfrmAbout
  Left = 423
  Top = 383
  BorderStyle = bsDialog
  Caption = 'About SaTScan'
  ClientHeight = 175
  ClientWidth = 397
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Icon.Data = {
    0000010002002020100000000000E80200002600000010101000000000002801
    00000E0300002800000020000000400000000100040000000000000200000000
    0000000000000000000000000000000000000000800000800000008080008000
    00008000800080800000C0C0C000808080000000FF0000FF000000FFFF00FF00
    0000FF00FF00FFFF0000FFFFFF00077777777777777777777777777777707766
    6666666676666666666766666677766666666677766666666667666666677666
    6666676676666666666766666667766666699999966666666667666666677666
    6999999999900000000066666667766699976666799900000000006666677669
    9976666000999666666000066667777997666600076996666667600066677699
    7666600077779977777766000667769977777007666699666667667007777699
    6666000766669966666777600067769966667777777699666667666600677699
    6677777777779966667666660067769967777067667799666676666600677669
    9776006766699777777766660067766999660067669997766766766600677667
    9996006769996777676667760067766779999999999676776766666700677667
    7669999996676677676666670067766776660006766766776676667000677777
    7776600777676677667666700777766776677700677666776667770006677667
    7766660007666777666670006667766677666660000667766660000666677666
    7776666700007770000000666667766667777676667777000000676666677666
    6677777777777666666667666667766666667777777666666666766666677666
    6666667666666666666766666667776666666676666666666667666666770777
    7777777777777777777777777770800000010000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000080000001280000001000000020000000010004000000
    0000800000000000000000000000000000000000000000000000000080000080
    000000808000800000008000800080800000C0C0C000808080000000FF0000FF
    000000FFFF00FF000000FF00FF00FFFF0000FFFFFF0077777777777777776666
    6766666766676669996666676667669766990000066779766079666700676977
    0766966760776966777696676607697707679666660760960769767666076709
    9996677667076766066767666007676770766767706766766006766006676677
    6667700076676666777666666667066666666667666700000000000000000000
    0000000000000000000000000000000000000000800040000000200080000000
    000000008000000000000000C000000080008000EE01}
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object pnlRootPanel: TPanel
    Left = 0
    Top = 0
    Width = 397
    Height = 175
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 0
    object pnlRootPanelTop: TPanel
      Left = 0
      Top = 0
      Width = 397
      Height = 31
      Align = alTop
      BevelOuter = bvNone
      TabOrder = 0
      object lblVersion: TLabel
        Left = 0
        Top = 9
        Width = 397
        Height = 22
        Align = alBottom
        Alignment = taCenter
        AutoSize = False
        Caption = 'SaTScan Version Number'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
      end
    end
    object pnlRootPanelRight: TPanel
      Left = 385
      Top = 31
      Width = 12
      Height = 132
      Align = alRight
      BevelOuter = bvNone
      TabOrder = 1
    end
    object pnlRootPanelLeft: TPanel
      Left = 0
      Top = 31
      Width = 12
      Height = 132
      Align = alLeft
      BevelOuter = bvNone
      TabOrder = 2
    end
    object pnlRootPanelBottom: TPanel
      Left = 0
      Top = 163
      Width = 397
      Height = 12
      Align = alBottom
      BevelOuter = bvNone
      TabOrder = 3
    end
    object Panel1: TPanel
      Left = 12
      Top = 31
      Width = 373
      Height = 132
      Align = alClient
      BevelOuter = bvNone
      TabOrder = 4
      object lblTitle: TLabel
        Left = 0
        Top = 0
        Width = 373
        Height = 16
        Align = alTop
        Alignment = taCenter
        AutoSize = False
        Caption = 'Software for the Spatial and Space-Time Scan Statistics.'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
      end
      object lblWebSiteLabel: TLabel
        Left = 0
        Top = 32
        Width = 61
        Height = 17
        AutoSize = False
        Caption = 'Web Site:  '
        Font.Charset = ANSI_CHARSET
        Font.Color = clBlack
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        WordWrap = True
      end
      object lblWebSite: TLabel
        Left = 71
        Top = 32
        Width = 136
        Height = 16
        Cursor = crHandPoint
        Caption = 'http://www.satscan.org/'
        Font.Charset = ANSI_CHARSET
        Font.Color = clBlue
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsUnderline]
        ParentFont = False
        WordWrap = True
        OnClick = lblWebSiteClick
      end
      object lblEmail: TLabel
        Left = 0
        Top = 54
        Width = 37
        Height = 16
        Caption = 'Email:'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
      end
      object lblEmailAddress: TLabel
        Left = 71
        Top = 54
        Width = 130
        Height = 16
        Cursor = crHandPoint
        Caption = 'kulldorff@satscan.org'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlue
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsUnderline]
        ParentFont = False
        OnClick = lblEmailAddressClick
      end
      object lblReleaseDate: TLabel
        Left = 0
        Top = 78
        Width = 192
        Height = 16
        Caption = 'Release Date : January 31, 2003'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
      end
      object Label1: TLabel
        Left = 0
        Top = 99
        Width = 373
        Height = 33
        Align = alBottom
        AutoSize = False
        Caption = 
          'Authors: Martin Kulldorff, Harvard Medical School, Boston and In' +
          'formation Management Services Inc, Silver Spring, Maryland.'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        WordWrap = True
      end
    end
  end
end
