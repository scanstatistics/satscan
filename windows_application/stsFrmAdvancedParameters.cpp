//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "stsFrmAdvancedParameters.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"


/** constructor */
__fastcall TfrmAdvancedParameters::TfrmAdvancedParameters(TfrmAnalysis & AnalysisSettings)
        : TForm(&AnalysisSettings), gAnalysisSettings(AnalysisSettings) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","TfrmAdvancedParameters");
    throw;
  }
}

/** event triggered when selects browse button for maximum circle population file */
void __fastcall TfrmAdvancedParameters::btnBrowseMaxCirclePopFileClick(TObject *Sender){
  try {
    OpenDialog->FileName = "";
    OpenDialog->DefaultExt = "*.pop";
    OpenDialog->Filter = "Maximum Circle Population files (*.max)|*.max|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog->FilterIndex = 0;
    OpenDialog->Title = "Select Maximum Circle Population File";
    if (OpenDialog->Execute())
      edtMaxCirclePopulationFilename->Text = OpenDialog->FileName;
  }
  catch (ZdException & x) {
    x.AddCallpath("btnBrowseMaxCirclePopFileClick()","TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}

/** event triggered when selects browse button for adjustment for relative risks file */
void __fastcall TfrmAdvancedParameters::btnBrowseAdjustmentsFileClick(TObject *Sender) {
  try {
    OpenDialog->FileName = "";
    OpenDialog->DefaultExt = "*.rr";
    OpenDialog->Filter = "Adjustment files (*.adj)|*.adj|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog->FilterIndex = 0;
    OpenDialog->Title = "Select Adjustments File";
    if (OpenDialog->Execute())
      edtAdjustmentsByRelativeRisksFile->Text = OpenDialog->FileName;
  }
  catch (ZdException &x) {
    x.AddCallpath("btnBrowseAdjustmentsFileClick()","TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}

/** event triggered when adjust for known relative risks is clicked */
void __fastcall TfrmAdvancedParameters::chkAdjustForKnownRelativeRisksClick(TObject *Sender) {
  lblAdjustmentsByRelativeRisksFile->Enabled = gbEnableAdjustmentsByRR && chkAdjustForKnownRelativeRisks->Checked;
  edtAdjustmentsByRelativeRisksFile->Enabled = gbEnableAdjustmentsByRR && chkAdjustForKnownRelativeRisks->Checked;
  edtAdjustmentsByRelativeRisksFile->Color = edtAdjustmentsByRelativeRisksFile->Enabled ? clWindow : clInactiveBorder;
  btnBrowseAdjustmentsFile->Enabled = gbEnableAdjustmentsByRR && chkAdjustForKnownRelativeRisks->Checked;
}

/** event triggered when user selects restrict reported clusters check box */
void __fastcall TfrmAdvancedParameters::chkRestrictReportedClustersClick(TObject *Sender) {
  edtReportClustersSmallerThan->Enabled = gAnalysisSettings.rdgSpatialOptions->Enabled && chkRestrictReportedClusters->Checked;
  edtReportClustersSmallerThan->Color = gAnalysisSettings.edtMaxSpatialClusterSize->Enabled && chkRestrictReportedClusters->Checked ? clWindow : clInactiveBorder;
}

/** event triggered when user selects restrict range check box */
void __fastcall TfrmAdvancedParameters::chkRestrictTemporalRangeClick(TObject *Sender) {
  EnableSpatialOutputOptions(gAnalysisSettings.rdgSpatialOptions->Enabled);
  RefreshTemporalOptionsEnables();
}

/** event triggered when loginear percentage control exited */
void __fastcall TfrmAdvancedParameters::edtLogLinearExit(TObject *Sender) {
  if (edtLogLinear->Text.IsEmpty() || atof(edtLogLinear->Text.c_str()) <= -100)
    edtLogLinear->Text = 0;
}

/** event triggered when text of maximum circle edit control changes */
void __fastcall TfrmAdvancedParameters::edtMaxCirclePopulationFilenameChange(TObject *Sender) {
  edtMaxCirclePopulationFilename->Hint = edtMaxCirclePopulationFilename->Text;
}

/** event triggered when end window end ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtEndRangeEndDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtEndRangeEndYear, *edtEndRangeEndMonth, *edtEndRangeEndDay);
}

/** event triggered when end window start ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtEndRangeStartDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtEndRangeStartYear, *edtEndRangeStartMonth, *edtEndRangeStartDay);
}

/** event triggered when text of adjustment by relative risks edit control changes */
void __fastcall TfrmAdvancedParameters::edtAdjustmentsByRelativeRisksFileChange(TObject *Sender) {
  edtAdjustmentsByRelativeRisksFile->Hint = edtAdjustmentsByRelativeRisksFile->Text;
}

/** event triggered when 'Report only clusters smaller than ...' edit control is exited */
void __fastcall TfrmAdvancedParameters::edtReportClustersSmallerThanExit(TObject *Sender) {
  if (!edtReportClustersSmallerThan->Text.Length()
       || atof(edtReportClustersSmallerThan->Text.c_str()) == 0
       || atof(edtReportClustersSmallerThan->Text.c_str()) > atof(gAnalysisSettings.edtMaxSpatialClusterSize->Text.c_str()))
    edtReportClustersSmallerThan->Text = gAnalysisSettings.edtMaxSpatialClusterSize->Text;
}

/** event triggered when key pressed for control that can contain positive real numbers */
void __fastcall TfrmAdvancedParameters::edtReportClustersSmallerThanKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789.\b",Key))
    Key = 0;
}

/** event triggered when start window end ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtStartRangeEndDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtStartRangeEndYear, *edtStartRangeEndMonth, *edtStartRangeEndDay);
}

/** event triggered when start window start ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtStartRangeStartDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtStartRangeStartYear, *edtStartRangeStartMonth, *edtStartRangeStartDay);
}

/** enables or disables the temporal time trend adjustment control group */
void TfrmAdvancedParameters::EnableAdjustmentForTimeTrendOptionsGroup(bool bEnable, bool bTimeStratified, bool bLogYearPercentage) {
  TimeTrendAdjustmentType eTimeTrendAdjustmentType(GetAdjustmentTimeTrendControlType());

  // trump control enables
  bTimeStratified &= bEnable;
  bLogYearPercentage &= bEnable;

  rdgTemporalTrendAdj->Enabled = bEnable;

  rdgTemporalTrendAdj->Controls[1]->Enabled = bTimeStratified;
  if (bEnable && !bTimeStratified && eTimeTrendAdjustmentType == STRATIFIED_RANDOMIZATION)
    SetTemporalTrendAdjustmentControl(NOTADJUSTED);

  rdgTemporalTrendAdj->Controls[2]->Enabled = bLogYearPercentage;
  lblLogLinear->Enabled = bLogYearPercentage;
  edtLogLinear->Enabled = bLogYearPercentage && eTimeTrendAdjustmentType == LOGLINEAR_PERC;
  edtLogLinear->Color = edtLogLinear->Enabled ? clWindow : clInactiveBorder;
  if (bEnable && !bLogYearPercentage && eTimeTrendAdjustmentType == LOGLINEAR_PERC)
    SetTemporalTrendAdjustmentControl(NOTADJUSTED);
}

/** enables adjustment options controls */
void TfrmAdvancedParameters::EnableAdjustmentsGroup(bool bEnable) {
  gbEnableAdjustmentsByRR = bEnable;
  grpAdjustments->Enabled = bEnable;
  chkAdjustForKnownRelativeRisks->Enabled = bEnable;
  lblAdjustmentsByRelativeRisksFile->Enabled = bEnable && chkAdjustForKnownRelativeRisks->Checked;
  edtAdjustmentsByRelativeRisksFile->Enabled = bEnable && chkAdjustForKnownRelativeRisks->Checked;
  edtAdjustmentsByRelativeRisksFile->Color = edtAdjustmentsByRelativeRisksFile->Enabled ? clWindow : clInactiveBorder;
  btnBrowseAdjustmentsFile->Enabled = bEnable && chkAdjustForKnownRelativeRisks->Checked;
}

/** enables spatial output options controls */
void TfrmAdvancedParameters::EnableSpatialOutputOptions(bool bEnable) {
  chkRestrictReportedClusters->Enabled = bEnable;
  edtReportClustersSmallerThan->Enabled = bEnable && chkRestrictReportedClusters->Checked;
  edtReportClustersSmallerThan->Color = bEnable && chkRestrictReportedClusters->Checked ? clWindow : clInactiveBorder;
  lblReportSmallerClusters->Enabled = bEnable;
}

/** enables spatial options controls */
void TfrmAdvancedParameters::EnableTemporalOptions(bool bEnable, bool bEnableRanges) {
  chkRestrictTemporalRange->Enabled = bEnable && bEnableRanges;
  stStartWindowRange->Enabled = bEnable && bEnableRanges;
  stStartRangeTo->Enabled = bEnable && bEnableRanges;
  edtStartRangeStartYear->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeYears;
  edtStartRangeStartYear->Color = edtStartRangeStartYear->Enabled ? clWindow : clInactiveBorder;
  edtStartRangeStartMonth->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeMonths;
  edtStartRangeStartMonth->Color = edtStartRangeStartMonth->Enabled ? clWindow : clInactiveBorder;
  if (!edtStartRangeStartMonth->Enabled) edtStartRangeStartMonth->Text = 1; 
  edtStartRangeStartDay->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeDays;
  edtStartRangeStartDay->Color = edtStartRangeStartDay->Enabled ? clWindow : clInactiveBorder;
  if (!edtStartRangeStartDay->Enabled) edtStartRangeStartDay->Text = 1; 
  edtStartRangeEndYear->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeYears;
  edtStartRangeEndYear->Color = edtStartRangeEndYear->Enabled ? clWindow : clInactiveBorder;
  edtStartRangeEndMonth->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeMonths;
  edtStartRangeEndMonth->Color = edtStartRangeEndMonth->Enabled ? clWindow : clInactiveBorder;
  if (!edtStartRangeEndMonth->Enabled) edtStartRangeEndMonth->Text = 1; 
  edtStartRangeEndDay->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeDays;
  edtStartRangeEndDay->Color = edtStartRangeEndDay->Enabled ? clWindow : clInactiveBorder;
  if (!edtStartRangeEndDay->Enabled) edtStartRangeEndDay->Text = 1; 

  stEndWindowRange->Enabled = bEnable && bEnableRanges;
  stEndRangeTo->Enabled = bEnable && bEnableRanges;
  edtEndRangeStartYear->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeYears;
  edtEndRangeStartYear->Color = edtEndRangeStartYear->Enabled ? clWindow : clInactiveBorder;
  edtEndRangeStartMonth->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeMonths;
  edtEndRangeStartMonth->Color = edtEndRangeStartMonth->Enabled ? clWindow : clInactiveBorder;
  if (!edtEndRangeStartMonth->Enabled) edtEndRangeStartMonth->Text = 12;
  edtEndRangeStartDay->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeDays;
  edtEndRangeStartDay->Color = edtEndRangeStartDay->Enabled ? clWindow : clInactiveBorder;
  if (!edtEndRangeStartDay->Enabled) edtEndRangeStartDay->Text = DaysThisMonth(atoi(edtEndRangeStartYear->Text.c_str()), atoi(edtEndRangeStartMonth->Text.c_str())); 
  edtEndRangeEndYear->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeYears;
  edtEndRangeEndYear->Color = edtEndRangeEndYear->Enabled ? clWindow : clInactiveBorder;
  edtEndRangeEndMonth->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeMonths;
  edtEndRangeEndMonth->Color = edtEndRangeEndMonth->Enabled ? clWindow : clInactiveBorder;
  if (!edtEndRangeEndMonth->Enabled) edtEndRangeEndMonth->Text = 12; 
  edtEndRangeEndDay->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked && gbEnableRangeDays;
  edtEndRangeEndDay->Color = edtEndRangeEndDay->Enabled ? clWindow : clInactiveBorder;
  if (!edtEndRangeStartDay->Enabled) edtEndRangeStartDay->Text = DaysThisMonth(atoi(edtEndRangeEndYear->Text.c_str()), atoi(edtEndRangeStartDay->Text.c_str()));
}

/** event triggered when key pressed for controls that can contain real numbers. */
void __fastcall TfrmAdvancedParameters::FloatKeyPress(TObject *Sender, char &Key) {
  if (!strchr("-0123456789.\b",Key))
    Key = 0;
}

/** event triggered when user presses key -- cancels dialog if escape key pressed */
void __fastcall TfrmAdvancedParameters::FormKeyPress(TObject *Sender, char &Key) {
  if (Key == VK_ESCAPE)
    Close();
}

/** event triggered when form shows */
void __fastcall TfrmAdvancedParameters::FormShow(TObject *Sender) {
  if (gpFocusControl)
   gpFocusControl->SetFocus();
}

/** returns adjustment for time trend type for control index */
TimeTrendAdjustmentType TfrmAdvancedParameters::GetAdjustmentTimeTrendControlType() const {
  TimeTrendAdjustmentType eReturn;

  switch (rdgTemporalTrendAdj->ItemIndex) {
    case 0  : eReturn = NOTADJUSTED; break;
    case 1  : eReturn = STRATIFIED_RANDOMIZATION; break;
    case 2  : eReturn = LOGLINEAR_PERC; break;
    default : ZdGenerateException("Unknown index type '%d'.", "GetAdjustmentTimeTrendControlType()", rdgTemporalTrendAdj->ItemIndex);
  }
  return eReturn;
}

/** event triggered when key pressed for control that can contain natural numbers */
void __fastcall TfrmAdvancedParameters::NaturalNumberKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789\b",Key))
    Key = 0;
}

/**  parses date into passed TEdit controls - defaults settings as needed, so that interface
     won't contain an invalid date.                                                          */
void TfrmAdvancedParameters::ParseDate(const std::string& sDate, TEdit& Year, TEdit& Month, TEdit& Day, bool bStartRange) {
  UInt  uiMonth, uiDay, uiYear;
  int   iPrecision;

  //set values only if valid, prevent interface from having invalid date when first loaded.
  if ((iPrecision = CharToMDY(&uiMonth, &uiDay, &uiYear, sDate.c_str())) > 0) {
    switch (iPrecision) {
      case 3 : if (uiYear >= MIN_YEAR && uiYear <= MAX_YEAR &&
                   uiMonth >= 1 && uiMonth <= 12 &&  uiDay >= 1 && uiDay <= DaysThisMonth(uiYear, uiMonth)) {
                  Year.Text = uiYear;
                  Month.Text = uiMonth;
                  Day.Text = uiDay;
               }
               break;
      case 2 : if (uiYear >= MIN_YEAR && uiYear <= MAX_YEAR && uiMonth >= 1 && uiMonth <= 12) {
                 Year.Text = uiYear;
                 Month.Text = uiMonth;
                 Day.Text = (bStartRange ? 1 : DaysThisMonth(uiYear, uiMonth));
               }
               break;
      case 1 : if (uiYear >= MIN_YEAR && uiYear <= MAX_YEAR)
                 Year.Text = uiYear;
               break;
    };
  }
}

/** event triggered when 'Adjustment for time trend' type control clicked */
void __fastcall TfrmAdvancedParameters::rdgTemporalTrendAdjClick(TObject *Sender) {
  switch (GetAdjustmentTimeTrendControlType()) {
    case LOGLINEAR_PERC : edtLogLinear->Enabled = true;
                          edtLogLinear->Color = clWindow;
                          break;
    default             : edtLogLinear->Enabled = false;
                          edtLogLinear->Color = clInactiveBorder;
  }
}

/** */
void TfrmAdvancedParameters::RefreshTemporalOptionsEnables() {
  AnalysisType  eType = gAnalysisSettings.GetAnalysisControlType();
  EnableTemporalOptions(gAnalysisSettings.rdgTemporalOptions->Enabled, eType == PURELYTEMPORAL || eType == SPACETIME);
}

/** parameter settings to parameters class */
void TfrmAdvancedParameters::SaveParameterSettings() {
  CParameters & ref = gAnalysisSettings.gParameters;
  ZdString      sString;

  try {
    ref.SetUseAdjustmentForRelativeRisksFile(chkAdjustForKnownRelativeRisks->Checked);
    ref.SetAdjustmentsByRelativeRisksFilename(edtAdjustmentsByRelativeRisksFile->Text.c_str(), false);
    ref.SetTimeTrendAdjustmentType(rdgTemporalTrendAdj->Enabled ? GetAdjustmentTimeTrendControlType() : NOTADJUSTED);
    ref.SetTimeTrendAdjustmentPercentage(atof(edtLogLinear->Text.c_str()));
    ref.SetMaxCirclePopulationFileName(edtMaxCirclePopulationFilename->Text.c_str(), false, true);
    ref.SetTerminateSimulationsEarly(chkTerminateEarly->Checked);
    ref.SetRestrictReportedClusters(chkRestrictReportedClusters->Enabled && chkRestrictReportedClusters->Checked);
    ref.SetMaximumReportedGeographicalClusterSize(atof(edtReportClustersSmallerThan->Text.c_str()));
    if (chkRestrictTemporalRange->Enabled && chkRestrictTemporalRange->Checked)
      ref.SetIncludeClustersType(CLUSTERSINRANGE);
    else
      ref.SetIncludeClustersType(ALLCLUSTERS);

    sString.printf("%i/%i/%i", atoi(edtStartRangeStartYear->Text.c_str()),
                               atoi(edtStartRangeStartMonth->Text.c_str()),
                               atoi(edtStartRangeStartDay->Text.c_str()));
    ref.SetStartRangeStartDate(sString.GetCString());
    sString.printf("%i/%i/%i", atoi(edtStartRangeEndYear->Text.c_str()),
                               atoi(edtStartRangeEndMonth->Text.c_str()),
                               atoi(edtStartRangeEndDay->Text.c_str()));
    ref.SetStartRangeEndDate(sString.GetCString());
    sString.printf("%i/%i/%i", atoi(edtEndRangeStartYear->Text.c_str()),
                               atoi(edtEndRangeStartMonth->Text.c_str()),
                               atoi(edtEndRangeStartDay->Text.c_str()));
    ref.SetEndRangeStartDate(sString.GetCString());
    sString.printf("%i/%i/%i", atoi(edtEndRangeEndYear->Text.c_str()),
                               atoi(edtEndRangeEndMonth->Text.c_str()),
                               atoi(edtEndRangeEndDay->Text.c_str()));
    ref.SetEndRangeEndDate(sString.GetCString());
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveParameterSettings()","TfrmAdvancedParameters");
    throw;
  }
}

/** Sets adjustments filename in interface */
void TfrmAdvancedParameters::SetAdjustmentsByRelativeRisksFile(const char * sAdjustmentsByRelativeRisksFileName) {
  edtAdjustmentsByRelativeRisksFile->Text = sAdjustmentsByRelativeRisksFileName;
}

/** Sets special population filename in interface */
void TfrmAdvancedParameters::SetMaximumCirclePopulationFile(const char * sMaximumCirclePopulationFileName) {
  edtMaxCirclePopulationFilename->Text = sMaximumCirclePopulationFileName;
}

/** */
void TfrmAdvancedParameters::SetRangeDateEnables(bool bYear, bool bMonth, bool bDay) {
  gbEnableRangeYears = bYear;
  gbEnableRangeMonths = bMonth;
  gbEnableRangeDays = bDay;
  RefreshTemporalOptionsEnables();
}

/** sets static label that describes what the reporting clusters will be limited as */
void TfrmAdvancedParameters::SetReportingClustersText(const ZdString& sText) {
  lblReportSmallerClusters->Caption = sText.GetCString();
}

/** Sets time trend adjustment control's index */
void TfrmAdvancedParameters::SetTemporalTrendAdjustmentControl(TimeTrendAdjustmentType eTimeTrendAdjustmentType) {
  switch (eTimeTrendAdjustmentType) {
    case NOTADJUSTED               : rdgTemporalTrendAdj->ItemIndex = 0; break;
    case NONPARAMETRIC             : rdgTemporalTrendAdj->ItemIndex = 1; break;
    case LOGLINEAR_PERC            : rdgTemporalTrendAdj->ItemIndex = 2; break;
    case STRATIFIED_RANDOMIZATION  : rdgTemporalTrendAdj->ItemIndex = 1; break;
    default                        : rdgTemporalTrendAdj->ItemIndex = 0;
  }
}

/** internal setup function */
void TfrmAdvancedParameters::Setup() {
  const CParameters & ref = gAnalysisSettings.gParameters;

  try {
    chkAdjustForKnownRelativeRisks->Checked = ref.UseAdjustmentForRelativeRisksFile();
    edtAdjustmentsByRelativeRisksFile->Text = ref.GetAdjustmentsByRelativeRisksFilename().c_str();
    SetTemporalTrendAdjustmentControl(ref.GetTimeTrendAdjustmentType());
    if (ref.GetTimeTrendAdjustmentPercentage() <= -100)
      edtLogLinear->Text = 0;
    else
      edtLogLinear->Text = ref.GetTimeTrendAdjustmentPercentage();
    edtMaxCirclePopulationFilename->Text = ref.GetMaxCirclePopulationFileName().c_str();
    chkTerminateEarly->Checked = ref.GetTerminateSimulationsEarly();
    if (ref.GetMaximumReportedGeoClusterSize() > atof(gAnalysisSettings.edtMaxSpatialClusterSize->Text.c_str()))
      edtReportClustersSmallerThan->Text = gAnalysisSettings.edtMaxSpatialClusterSize->Text;
    else
      edtReportClustersSmallerThan->Text = ref.GetMaximumReportedGeoClusterSize();
    chkRestrictReportedClusters->Checked = ref.GetRestrictingMaximumReportedGeoClusterSize();
    chkRestrictTemporalRange->Checked = ref.GetIncludeClustersType() == CLUSTERSINRANGE;
    if (ref.GetStartRangeStartDate().length() > 0)
      ParseDate(ref.GetStartRangeStartDate(), *edtStartRangeStartYear, *edtStartRangeStartMonth, *edtStartRangeStartDay, true);
    if (ref.GetStartRangeEndDate().length() > 0)
      ParseDate(ref.GetStartRangeEndDate(), *edtStartRangeEndYear, *edtStartRangeEndMonth, *edtStartRangeEndDay, true);
    if (ref.GetEndRangeStartDate().length() > 0)
      ParseDate(ref.GetEndRangeStartDate(), *edtEndRangeStartYear, *edtEndRangeStartMonth, *edtEndRangeStartDay, false);
    if (ref.GetEndRangeEndDate().length() > 0)
      ParseDate(ref.GetEndRangeEndDate(), *edtEndRangeEndYear, *edtEndRangeEndMonth, *edtEndRangeEndDay, false);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","TfrmAdvancedParameters");
    throw;
  }
}

/** sets control to focus when form shows then shows form modal */
void TfrmAdvancedParameters::ShowDialog(TWinControl * pFocusControl) {
  bool          bFound=false;
  int           i;      

  for (i=0; i < PageControl->PageCount && !bFound; ++i) {
     if (PageControl->Pages[i]->ContainsControl(pFocusControl)) {
       PageControl->ActivePage = PageControl->Pages[i];
       gpFocusControl = pFocusControl;
       bFound=true;
     }
  }

  if (!bFound) {
    gpFocusControl=0;
    PageControl->ActivePage = PageControl->Pages[0];
  }
  //reporting clusters text dependent on maximum spatial cluster size
  //-- ensure that it has text
  if (!gAnalysisSettings.edtMaxSpatialClusterSize->Text.Length())
    gAnalysisSettings.edtMaxSpatialClusterSize->Text = 50;

  ShowModal();
}

/** validates input file settings - throws exception */
void TfrmAdvancedParameters::ValidateInputFilesSettings() {
  try {
    if (gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION &&
        gAnalysisSettings.rdoSpatialPercentage->Checked && !edtMaxCirclePopulationFilename->Text.Length())
       GenerateAFException("For a Space-Time Permutation model with the maximum spatial cluster size defined as a\n"
                           "percentage of the population at risk, a Maximum Circle Population file must be specified.\n"
                           "Alternatively you may choose to specify the maximum as a fixed radius,\n"
                           "in which no Maximum Circle Population file is required.",
                           "ValidateInputFilesSettings()", *edtMaxCirclePopulationFilename);

    if (gAnalysisSettings.gParameters.GetAnalysisType() == PROSPECTIVESPACETIME &&
        gAnalysisSettings.chkAdjustForEarlierAnalyses->Checked && gAnalysisSettings.rdoSpatialPercentage->Checked &&
        gAnalysisSettings.rdoSpatialPercentage->Enabled && !edtMaxCirclePopulationFilename->Text.Length())
      GenerateAFException("For a Prospective Space-Time analysis adjusting for ealier analyses,\n"
                          "with the maximum spatial cluster size defined as a percentage of the\n"
                          "population at risk, a Maximum Circle Population file must be specified.\n"
                          "Alternatively you may choose to specify the maximum as a fixed radius,\n"
                          "in which no Maximum Circle Population file is required.",
                          "ValidateInputFilesSettings()", *edtMaxCirclePopulationFilename);
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateInputFilesSettings()","TfrmAdvancedParameters");
    throw;
  }
}

/** validates reported clusters limiting control setting - throws exception */
void TfrmAdvancedParameters::ValidateReportedSpatialClusterSize() {
  try {
    if (gAnalysisSettings.edtMaxSpatialClusterSize->Enabled && edtReportClustersSmallerThan->Enabled) {
      if (!edtReportClustersSmallerThan->Text.Length() || atof(edtReportClustersSmallerThan->Text.c_str()) == 0)
        GenerateAFException("Please specify a maximum cluster size for reported clusters\n"
                            "greater than 0 and less than or equal to the maximum spatial cluster size of %g.",
                            "ValidateReportedSpatialClusterSize()", *edtReportClustersSmallerThan,
                            atof(gAnalysisSettings.edtMaxSpatialClusterSize->Text.c_str()));


      if (atof(edtReportClustersSmallerThan->Text.c_str()) > atof(gAnalysisSettings.edtMaxSpatialClusterSize->Text.c_str()))
        GenerateAFException("The maximum cluster size for reported clusters can not be greater than the maximum spatial cluster size.\n",
                            "ValidateReportedSpatialClusterSize()", *edtReportClustersSmallerThan);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateReportedSpatialClusterSize()","TfrmAdvancedParameters");
    throw;
  }
}

/** validates scanning window range settings - throws exception */
void TfrmAdvancedParameters::ValidateScanningWindowRanges() {
  try {
    /** Until Martin decides how the study period dates should behave in regards
        to time interval units, let the parameters validation handle this. The result
        will be that any invalid settings will be printed to 'Warnings/Errors' window
        instead of the preferred message box.
        
        Why is this a problem now?
          The precision of times control has been replaced by a 'yes/no' control which
          more simply indicates whether input case/control files possess dates. This control
          would normally dictate whether dates would be defaulted in GUI. Given the past and
          continued behavior of CParameters::ValidateStudyPeriodDateString(...) I believe
          the current solution is correct, but that's not saying much :) */
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateScanningWindowRanges()","TfrmAdvancedParameters");
    throw;
  }
}

/** validates scanning window settings - throws exception */
void TfrmAdvancedParameters::ValidateScanningWindowSettings() {
  try {
    ValidateReportedSpatialClusterSize();
    ValidateScanningWindowRanges();
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateScanningWindowSettings()","TfrmAdvancedParameters");
    throw;
  }
}

/**  Construct. This is an alternate constructor for when the varArgs list for sMessage
     has already been prepared. Primarily, this will be used by derived classes.        */
AdvancedFeaturesException::AdvancedFeaturesException(va_list varArgs, const char *sMessage,
                                                     const char *sSourceModule, Level iLevel, TWinControl& FocusControl)
          : ZdException(varArgs, sMessage, sSourceModule, iLevel), gFocusControl(FocusControl) {
   SetData(varArgs, sMessage, sSourceModule, iLevel);
}

/** Destructor. */
AdvancedFeaturesException::~AdvancedFeaturesException() {}

/**  This function will throw the exception with the parameters.  It is equivalent to
     throw ZdException(...), but includes the ability to format the message string.
     This function should be used to generate all Zd Exceptions within ZD.             */
void GenerateAFException(const char * sMessage, const char * sSourceModule, TWinControl& FocusControl, ...) {
  va_list      varArgs;
  va_start (varArgs, sSourceModule);

  AdvancedFeaturesException  theException(varArgs, sMessage, sSourceModule, ZdException::Notify, FocusControl);
  va_end(varArgs);

  throw theException;
}



