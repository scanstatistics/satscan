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
      gAnalysisSettings.SetMaximumCirclePopulationFile(OpenDialog->FileName.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("btnBrowseMaxCirclePopFileClick()","TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}

/** event triggered when user selects restrict reported clusters check box */
void __fastcall TfrmAdvancedParameters::chkRestrictReportedClustersClick(TObject *Sender) {
  edtReportClustersSmallerThan->Enabled = gAnalysisSettings.rdgSpatialOptions->Enabled && chkRestrictReportedClusters->Checked;
  edtReportClustersSmallerThan->Color = gAnalysisSettings.edtMaxSpatialClusterSize->Enabled && chkRestrictReportedClusters->Checked ? clWindow : clInactiveBorder;
}

/** event triggered when user selects restrict range check box */
void __fastcall TfrmAdvancedParameters::chkRestrictTemporalRangeClick(TObject *Sender) {
  AnalysisType  eType = gAnalysisSettings.GetAnalysisControlType();
  
  EnableSpatialOptions(gAnalysisSettings.rdgSpatialOptions->Enabled);
  EnableTemporalOptions(gAnalysisSettings.rdgTemporalOptions->Enabled, eType == PURELYTEMPORAL || eType == SPACETIME);
}

/** event triggered when text of maximum circle edit control changes */
void __fastcall TfrmAdvancedParameters::edtMaxCirclePopulationFilenameChange(TObject *Sender) {
  edtMaxCirclePopulationFilename->Hint = edtMaxCirclePopulationFilename->Text;
}

/** event triggered when end window end ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtEndRangeEndDateExit(TObject *Sender) {
  ValidateDate(*edtEndRangeEndYear, *edtEndRangeEndMonth, *edtEndRangeEndDay);
}

/** event triggered when end window start ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtEndRangeStartDateExit(TObject *Sender) {
  ValidateDate(*edtEndRangeStartYear, *edtEndRangeStartMonth, *edtEndRangeStartDay);
}

/** event triggered when 'Report only clusters smaller than ...' edit control is exited */
void __fastcall TfrmAdvancedParameters::edtReportClustersSmallerThanExit(TObject *Sender) {
  try {
    if (!edtReportClustersSmallerThan->Text.Length() || atof(edtReportClustersSmallerThan->Text.c_str()) == 0)
      ZdException::GenerateNotification("Please specify a maximum cluster size for reported clusters\n"
                                        "between 0 and the maximum spatial cluster size of %g.",
                                        "edtReportClustersSmallerThanExit()",
                                        atof(gAnalysisSettings.edtMaxSpatialClusterSize->Text.c_str()));
  }
  catch (ZdException & x) {
    edtReportClustersSmallerThan->SetFocus();
    x.AddCallpath("edtReportClustersSmallerThanExit()","TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}

/** event triggered when key pressed for control that can contain positive real numbers */
void __fastcall TfrmAdvancedParameters::edtReportClustersSmallerThanKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789.\b",Key))
    Key = 0;
}

/** event triggered when start window end ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtStartRangeEndDateExit(TObject *Sender) {
  ValidateDate(*edtStartRangeEndYear, *edtStartRangeEndMonth, *edtStartRangeEndDay);
}

/** event triggered when start window start ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtStartRangeStartDateExit(TObject *Sender) {
  ValidateDate(*edtStartRangeStartYear, *edtStartRangeStartMonth, *edtStartRangeStartDay);
}

/** enables spatial options controls */
void TfrmAdvancedParameters::EnableSpatialOptions(bool bEnable) {
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
  edtStartRangeStartYear->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtStartRangeStartYear->Color = edtStartRangeStartYear->Enabled ? clWindow : clInactiveBorder;
  edtStartRangeStartMonth->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtStartRangeStartMonth->Color = edtStartRangeStartMonth->Enabled ? clWindow : clInactiveBorder;
  edtStartRangeStartDay->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtStartRangeStartDay->Color = edtStartRangeStartDay->Enabled ? clWindow : clInactiveBorder;
  edtStartRangeEndYear->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtStartRangeEndYear->Color = edtStartRangeEndYear->Enabled ? clWindow : clInactiveBorder;
  edtStartRangeEndMonth->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtStartRangeEndMonth->Color = edtStartRangeEndMonth->Enabled ? clWindow : clInactiveBorder;
  edtStartRangeEndDay->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtStartRangeEndDay->Color = edtStartRangeEndDay->Enabled ? clWindow : clInactiveBorder;

  stEndWindowRange->Enabled = bEnable && bEnableRanges;
  stEndRangeTo->Enabled = bEnable && bEnableRanges;
  edtEndRangeStartYear->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtEndRangeStartYear->Color = edtEndRangeStartYear->Enabled ? clWindow : clInactiveBorder;
  edtEndRangeStartMonth->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtEndRangeStartMonth->Color = edtEndRangeStartMonth->Enabled ? clWindow : clInactiveBorder;
  edtEndRangeStartDay->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtEndRangeStartDay->Color = edtEndRangeStartDay->Enabled ? clWindow : clInactiveBorder;
  edtEndRangeEndYear->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtEndRangeEndYear->Color = edtEndRangeEndYear->Enabled ? clWindow : clInactiveBorder;
  edtEndRangeEndMonth->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtEndRangeEndMonth->Color = edtEndRangeEndMonth->Enabled ? clWindow : clInactiveBorder;
  edtEndRangeEndDay->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Checked;
  edtEndRangeEndDay->Color = edtEndRangeEndDay->Enabled ? clWindow : clInactiveBorder;
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

/** parameter settings to parameters class */
void TfrmAdvancedParameters::SaveParameterSettings() {
  CParameters & ref = gAnalysisSettings.gParameters;
  ZdString      sString;

  try {
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

/** Sets special population filename in interface and parameters class. */
void TfrmAdvancedParameters::SetMaximumCirclePopulationFile(const char * sMaximumCirclePopulationFileName) {
  edtMaxCirclePopulationFilename->Text = sMaximumCirclePopulationFileName;
}

/** sets static label that describes what the reporting clusters will be limited as */
void TfrmAdvancedParameters::SetReportingClustersText(const ZdString& sText) {
  lblReportSmallerClusters->Caption = sText.GetCString();
}

/** internal setup function */
void TfrmAdvancedParameters::Setup() {
  const CParameters & ref = gAnalysisSettings.gParameters;

  try {
    edtMaxCirclePopulationFilename->Text = ref.GetMaxCirclePopulationFileName().c_str();
    chkTerminateEarly->Checked = ref.GetTerminateSimulationsEarly();
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
  gpFocusControl = pFocusControl;
  ShowModal();
}

/** validates date controls represented by three passed edit controls - prevents an invalid date */
void TfrmAdvancedParameters::ValidateDate(TEdit& YearControl, TEdit& MonthControl, TEdit& DayControl) {
  int   iDay, iMonth, iYear, iDaysInMonth;

  //first check year
  if (YearControl.Text.IsEmpty())
    YearControl.Undo();
  else {
    //set year to a valid setting if out of valid range
    iYear = atoi(YearControl.Text.c_str());
    if (iYear < MIN_YEAR)
      YearControl.Text = MIN_YEAR;
    else if (iYear > MAX_YEAR)
      YearControl.Text = MAX_YEAR;
  }
  //now check month
  if (MonthControl.Text.IsEmpty())
    MonthControl.Undo();
  else {
    //set month to a valid setting if out of valid range
    iMonth = atoi(MonthControl.Text.c_str());
    if (iMonth < 1)
      MonthControl.Text = 1;
    else if (iMonth > 12)
      MonthControl.Text = 12;
  }
  //now check day
  if (DayControl.Text.IsEmpty())
    DayControl.Undo();
  else {
    iDaysInMonth = DaysThisMonth(atoi(YearControl.Text.c_str()), atoi(MonthControl.Text.c_str()));
    //set month to a valid setting if out of valid range
    iDay = atoi(DayControl.Text.c_str());
    if (iDay < 1)
      DayControl.Text = 1;
    else if (iDay > iDaysInMonth)
      DayControl.Text = iDaysInMonth;
  }
}

/** validates input file settings - throws exception */
void TfrmAdvancedParameters::ValidateInputFilesSettings() {
  try {
    if (gAnalysisSettings.rgProbability->ItemIndex == SPACETIMEPERMUTATION &&
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
                            "between 0 and the maximum spatial cluster size of %g.",
                            "edtReportClustersSmallerThanExit()", *edtReportClustersSmallerThan,
                            atof(gAnalysisSettings.edtMaxSpatialClusterSize->Text.c_str()));


      if (atof(edtReportClustersSmallerThan->Text.c_str()) >= atof(gAnalysisSettings.edtMaxSpatialClusterSize->Text.c_str()))
        GenerateAFException("The maximum cluster size for reported clusters must be less than the maximum spatial cluster size.\n",
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

