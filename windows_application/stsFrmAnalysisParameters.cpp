//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmAnalysis *frmAnalysis;

//ClassDesc Begin TfrmAnalysis
// This class contains all the main interface controls and relationships.
// Since it the main session interface is a tab dialog, decided to keep
// everything in one class and one cpp.
//ClassDesc End TfrmAnalysis

/** constructor */
//---------------------------------------------------------------------------
__fastcall TfrmAnalysis::TfrmAnalysis(TComponent* Owner, TActionList* theList, char *sParamFileName)
                        :stsBaseAnalysisChildForm (Owner, theList) {
  try {
    Init();
    Setup(sParamFileName);
  }
  catch (ZdException & x) {
    x.AddCallpath("constructor()", "TfrmAnalysis");
    throw;
  }
}
/** destructor */
__fastcall TfrmAnalysis::~TfrmAnalysis() {}

/** button click event for case file browse
    - shows open dialog and sets appropriate case file interface controls */
void __fastcall TfrmAnalysis::btnCaseBrowseClick(TObject *Sender) {
  try {
    OpenDialog1->FileName =  "";
    OpenDialog1->DefaultExt = "*.cas";
    OpenDialog1->Filter = "Case files (*.cas)|*.cas|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->FilterIndex = 0;
    OpenDialog1->Title = "Select Case File";
    if (OpenDialog1->Execute())
      SetCaseFile(OpenDialog1->FileName.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("btnCaseBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** button click event for control file browse
    - shows open dialog and sets appropriate control file interface controls */
void __fastcall TfrmAnalysis::btnControlBrowseClick(TObject *Sender) {
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.ctl";
    OpenDialog1->Filter = "Control files (*.ctl)|*.ctl|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->FilterIndex = 0;
    OpenDialog1->Title = "Select Control File";
    if (OpenDialog1->Execute())
      SetControlFile(OpenDialog1->FileName.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("btnControlBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** button click event for coordinates file browse
    - shows open dialog and sets appropriate coordinates file interface controls */
void __fastcall TfrmAnalysis::btnCoordBrowseClick(TObject *Sender) {
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.geo";
    OpenDialog1->Filter = "Coordinates files (*.geo)|*.geo|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->FilterIndex = 0;
    OpenDialog1->Title = "Select Coordinates File";
    if (OpenDialog1->Execute())
      SetCoordinateFile(OpenDialog1->FileName.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("btnCoordBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** button click event for special grid file browse
    - shows open dialog and sets appropriate special grid file interface controls */
void __fastcall TfrmAnalysis::btnGridBrowseClick(TObject *Sender) {
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.grd";
    OpenDialog1->Filter = "Special Grid files (*.grd)|*.grd|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->FilterIndex = 0;
    OpenDialog1->Title = "Select Special Grid File";
    if (OpenDialog1->Execute())
      SetSpecialGridFile(OpenDialog1->FileName.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("btnGridBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

void __fastcall TfrmAnalysis::btnMaxCirclePopFileBrowseClick(TObject *Sender) {
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.pop";
    OpenDialog1->Filter = "Maximum Circle Population files (*.max)|*.max|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->FilterIndex = 0;
    OpenDialog1->Title = "Select Maximum Circle Population File";
    if (OpenDialog1->Execute())
      SetMaximumCirclePopulationFile(OpenDialog1->FileName.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("btnPopBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** button click event for population file browse
    - shows open dialog and sets appropriate population file interface controls */
void __fastcall TfrmAnalysis::btnPopBrowseClick(TObject *Sender) {
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.pop";
    OpenDialog1->Filter = "Population files (*.pop)|*.pop|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->FilterIndex = 0;
    OpenDialog1->Title = "Select Population File";
    if (OpenDialog1->Execute())
      SetPopulationFile(OpenDialog1->FileName.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("btnPopBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** button click event for result file browse
    - shows open dialog and sets appropriate result file interface controls */
void __fastcall TfrmAnalysis::btnResultFileBrowseClick(TObject *Sender) {
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.txt";
    OpenDialog1->Filter = "Results files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->FilterIndex = 0;
    OpenDialog1->Title = "Select Results File";
    if (OpenDialog1->Execute())
      edtResultFile->Text = OpenDialog1->FileName.c_str();
  }
  catch (ZdException & x) {
    x.AddCallpath("btnResultFileBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** Returns whether day of date is proper given year and month.
    Displays message box of proper range if day is indeed incorrect. */
bool TfrmAnalysis::Check_Days(int iYear, int iMonth, int iDay, const char *sDateName) {
  char szMessage[100];
  AnsiString sFinalMessage;
  int iMin = 1, iMax;

  iMax = DaysThisMonth(iYear, iMonth);
  if ((iDay < iMin) || (iDay > iMax)) {
    sFinalMessage += sDateName;
    sprintf(szMessage, ":  Please specify a day between %i and %i.", iMin, iMax);
    sFinalMessage += szMessage;
    Application->MessageBox(sFinalMessage.c_str(), "Notification" , MB_OK);
    return false;
  }
  else
    return true;
}

/** Displays message box if time interval length is less than zero.
    Returns whether time interval length is valid. */
bool TfrmAnalysis::Check_IntervalLength() {
    if (atoi(edtTimeIntervalLength->Text.c_str()) < 1) {
      Application->MessageBox("Interval length can not be zero. "
                              "Please specify an interval length.", "Notification" , MB_OK);
       return false;
    }
    else
       return true;
}

/** Returns whether month of date is proper.
    Displays message box of valid range if month is indeed incorrect. */
bool TfrmAnalysis::Check_Month(int iMonth, const char *sDateName) {
  if ((iMonth < 1) || (iMonth > 12)) {
    std::string sFinalMessage(sDateName);
    sFinalMessage += ":  Please specify an month between 1 and 12.";
    Application->MessageBox(sFinalMessage.c_str(), "Notification" , MB_OK);
    return false;
  }
  else
     return true;
}

/** Displays message box if log linear time trend adjustment percentage is invalid.
    Return whether invalid. */
bool TfrmAnalysis::Check_TimeTrendPercentage(double dValue) {
  if (!(dValue > -100.00)) {
    Application->MessageBox("Invalid Ajustment for Time Trend setting.\n"
                            "Log Linear perentage must be greater than -100.", "Notification" , MB_OK);
    return false;
  }
  else
     return true;
}

/** Displays message box if year of date out of valid range. Returns whether date is valid. */
bool TfrmAnalysis::Check_Year(int iYear, const char *sDateName) {
  char          szMessage[100];

  if ( ! (iYear >= MIN_YEAR) && (iYear <= MAX_YEAR)) {
    std::string sFinalMessage(sDateName);
    sprintf(szMessage, ":  Please specify a year between %i and %i.", MIN_YEAR, MAX_YEAR);
    sFinalMessage += szMessage;
    Application->MessageBox(sFinalMessage.c_str(), "Notification" , MB_OK);
    return false;
  }
  else
     return true;
}

/** Checks all the parameters on the 'Analysis' tab. Returns whether tab is valid. */
bool TfrmAnalysis::CheckAnalysisParams() {
  bool bParamsOk = true;

  try {
    //if years enabled, then check values...
    //if start year enabled, assume end year enabled.
    if (edtStudyPeriodStartDateYear->Enabled) {
      bParamsOk = Check_Year(atoi(edtStudyPeriodStartDateYear->Text.c_str()),"Study Period Start Year");
      if (bParamsOk)
         bParamsOk = Check_Year(atoi(edtStudyPeriodEndDateYear->Text.c_str()),"Study Period End Year");
    }
    //if Months enabled, then check values...
    //if start month enabled, assume end month enabled.
    if (bParamsOk && edtStudyPeriodStartDateMonth->Enabled) {
      bParamsOk = Check_Month(atoi(edtStudyPeriodStartDateMonth->Text.c_str()), "Study Period Start Month");
      if (bParamsOk)
         bParamsOk = Check_Month(atoi(edtStudyPeriodEndDateMonth->Text.c_str()), "Study Period End Month");
    }
    //if Days enabled, then check values...
    //if start days enabled, assume end days enabled.
    if (bParamsOk && edtStudyPeriodStartDateDay->Enabled) {
      bParamsOk = Check_Days(atoi(edtStudyPeriodStartDateYear->Text.c_str()), atoi(edtStudyPeriodStartDateMonth->Text.c_str()), atoi(edtStudyPeriodStartDateDay->Text.c_str()),"Study Period Start Date");
      if (bParamsOk)
         bParamsOk = Check_Days(atoi(edtStudyPeriodEndDateYear->Text.c_str()), atoi(edtStudyPeriodEndDateMonth->Text.c_str()),
                                atoi(edtStudyPeriodEndDateDay->Text.c_str()),"Study Period End Date");
    }


    if (bParamsOk)
      bParamsOk = CheckStudyPeriodDatesRange();
    if (bParamsOk)
      bParamsOk = CheckReplicas();
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckAnalysisParams()", "TfrmAnalysis");
    throw;
  }
  return bParamsOk;
}

/** Verifies all parameters on the 'Output Files' tab. Returns whether tab is valid.*/
bool TfrmAnalysis::CheckOutputParams() {
  ZdFileName    OutPutFileName;

  try {
    if (edtResultFile->Text.Length() == 0)
      ZdException::GenerateNotification("Please specify a results file.", "CheckOutputParams()");

    OutPutFileName.SetFullPath(edtResultFile->Text.c_str());
    if (! DirectoryExists(OutPutFileName.GetLocation()))
      ZdException::GenerateNotification("Invalid file path specified for results file.\nPlease review settings.",
                                        "CheckOutputParams()");

    if (! FileCanBeCreated(edtResultFile->Text.c_str()))
      ZdException::GenerateNotification("Result file could not be opened. Access denied.\nPlease review settings.",
                                        "CheckOutputParams()");
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckOutputParams", "TfrmAnalysis");
    PageControl1->ActivePage = tbOutputFiles;
    edtResultFile->SetFocus();
    throw;
  }
  return true;
}

/** Specific prospective space-time date check
    Must be between the start and end dates of the analysis */
bool TfrmAnalysis::CheckProspDateRange() {
  bool          bRangeOk = true;
  Julian        Start, End, Prosp;
  ZdString      sAnalysisName;
  int           iProspYear, iProspMonth, iProspDay;

  try {
    Start = MDYToJulian(atoi(edtStudyPeriodStartDateMonth->Text.c_str()),
                        atoi(edtStudyPeriodStartDateDay->Text.c_str()),
                        atoi(edtStudyPeriodStartDateYear->Text.c_str()));
    End = MDYToJulian(atoi(edtStudyPeriodEndDateMonth->Text.c_str()),
                      atoi(edtStudyPeriodEndDateDay->Text.c_str()),
                      atoi(edtStudyPeriodEndDateYear->Text.c_str()));
    iProspMonth = atoi(edtProspectiveStartDateMonth->Text.c_str());
    iProspDay = atoi(edtProspectiveStartDateDay->Text.c_str());
    iProspYear = atoi(edtProspectiveStartDateYear->Text.c_str());
    Prosp = MDYToJulian(iProspMonth, iProspDay, iProspYear);

    sAnalysisName.printf("Start date of %s analysis",
                         rgTypeAnalysis->Items->Strings[rgTypeAnalysis->ItemIndex].c_str());
    if (! Check_Days(iProspYear, iProspMonth, iProspDay, sAnalysisName.GetCString())) {
      PageControl1->ActivePage = tbTimeParameter;
      edtProspectiveStartDateDay->SetFocus();
      bRangeOk = false;
    }
    else if ((Prosp < Start) || (Prosp > End)) {
      sAnalysisName.printf("The start date of %s analysis must be between the study period start and end dates.",
                            rgTypeAnalysis->Items->Strings[rgTypeAnalysis->ItemIndex].c_str());
      Application->MessageBox(sAnalysisName.GetCString(), "Notification" , MB_OK);
      PageControl1->ActivePage = tbTimeParameter;
      edtProspectiveStartDateYear->SetFocus();
      bRangeOk = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckProspDateRange()", "TfrmAnalysis");
    throw;
  }
  return bRangeOk;
}

/** Checks Monte Carlo replications */
bool TfrmAnalysis::CheckReplicas() {
  int   iNumSimulations = atoi(edtMontCarloReps->Text.c_str());

  if (! ((iNumSimulations == 0) || (iNumSimulations == 9) ||
        (iNumSimulations == 19) || (fmod(iNumSimulations+1, 1000) == 0.0)) ) {
      Application->MessageBox("Invalid number of Monte Carlo replications.\n"
                              "Choices are: 9, 999, or value ending in 999.", "Notification" , MB_OK);
      return false;
    }
    else
       return true;
}

/** Checks the validity of the 'Scanning Window' tab */
bool TfrmAnalysis::CheckScanningWindowParams() {
  return ValidateTemoralClusterSize() && ValidateSpatialClusterSize() && ValidateReportedSpatialClusterSize();
}

/** Checks the relationship between a start date, end date, and interval length.
    Display message box regarding errors when appropriate. Return whether relationship is valid. */
bool TfrmAnalysis::CheckStudyPeriodDatesRange() {
  bool          bRangeOk = true;
  ZdString      sErrorMessage;
  ZdDate        StartDate, EndDate;
  ZdDateFilter  DateFilter("%4y/%02m/%02d");
  char          FilterBuffer[11];
  int           iIntervalLength(atoi(edtTimeIntervalLength->Text.c_str()));

  try {
    GetStudyPeriodStartDate(StartDate);
    GetStudyPeriodEndDate(EndDate);

    //check that start date is before end date
    if (StartDate >= EndDate) {
      DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), StartDate.GetRawDate());
      sErrorMessage << "The study period start date of " << FilterBuffer;
      DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), EndDate.GetRawDate());
      sErrorMessage << " does not occur before study period end date of " << FilterBuffer;
      sErrorMessage << ".\nPlease review settings.";
      Application->MessageBox(sErrorMessage.GetCString(), "Notification" , MB_OK);
      PageControl1->ActivePage = tbAnalysis;
      bRangeOk = false;
    }

    if (GetAnalysisControlType() != PURELYSPATIAL)  {/* purely spatial does not use interval length */
      //check that interval length is not greater study period
      //(.i.e. can't have study period that is 20 days and intervals of 3 months)
      //to make start and end day inclusive - add 1 to end date
      EndDate.AddDays(1);
      if (bRangeOk) {
        switch (GetTimeIntervalControlType()) {
          case      YEAR      : StartDate.AddYears(static_cast<unsigned short>(iIntervalLength));
                                strcpy(FilterBuffer,"year(s)");
                                break;
          case      MONTH     : StartDate.AddMonths(static_cast<unsigned short>(iIntervalLength));
                                strcpy(FilterBuffer,"month(s)");
                                break;
          case      DAY       : StartDate.AddDays(static_cast<unsigned short>(iIntervalLength));
                                strcpy(FilterBuffer,"day(s)");
                                break;
          default             : ZdGenerateException("Unknown interval unit \"%d\"","CheckDateRange()",GetTimeIntervalControlType());
        };

        if (StartDate > EndDate) {
          sErrorMessage << "Interval length of " << iIntervalLength << " " << FilterBuffer;
          sErrorMessage << " is greater than study period length.\nPlease review settings.";
          Application->MessageBox(sErrorMessage.GetCString(), "Notification" , MB_OK);
          PageControl1->ActivePage = tbTimeParameter;
          bRangeOk = false;
        }
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckStudyPeriodDatesRange()", "TfrmAnalysis");
    throw;
  }
  return bRangeOk;
}

/** Checks 'Time Parameters' tab */
bool TfrmAnalysis::CheckTimeParams() {
  bool bParamsOk = true;
  double dValue;

  try {
    if (GetAnalysisControlType() != PURELYSPATIAL) { //not purely spacial    use to be 0
      if (edtTimeIntervalLength->Enabled)
         bParamsOk = Check_IntervalLength();
      if (bParamsOk && edtLogLinear->Enabled) {
        if (edtLogLinear->Text.IsEmpty()) {
          bParamsOk = false;
          Application->MessageBox("Please enter a number in time trend percentage.", "Parameter Error" , MB_OK);
          PageControl1->ActivePage = tbTimeParameter;
        }
        else {
          dValue = atof(edtLogLinear->Text.c_str());
          bParamsOk = Check_TimeTrendPercentage(dValue);
        }
      }
      //just need to check if the Prospective year is enabled.
      // if year is enabled, then all others are too...
      if (bParamsOk && edtProspectiveStartDateYear->Enabled)
         bParamsOk = CheckProspDateRange();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckTimeParams()", "TfrmAnalysis");
    throw;
  }
  return bParamsOk;
}

/** event triggered when 'adjustment for ealier analyses' checkbox if clicked */
void __fastcall TfrmAnalysis::chkAdjustForEarlierAnalysesClick(TObject *Sender) {
  EnableProspectiveStartDate(chkAdjustForEarlierAnalyses->Checked);
}

/** event triggered when 'Report only clusters smaller than ...' checkbox if clicked */
void __fastcall TfrmAnalysis::chkRestrictReportedClustersClick(TObject *Sender){
  edtReportClustersSmallerThan->Enabled = rdgSpatialOptions->Enabled && chkRestrictReportedClusters->Checked;
  edtReportClustersSmallerThan->Color = edtMaxSpatialClusterSize->Enabled && chkRestrictReportedClusters->Checked ? clWindow : clInactiveBorder;
}

/** Resets parameters that are not present in interface to default value.
    Hidden features are to be used soley in dos version at this time.     */
void TfrmAnalysis::DefaultHiddenParameters() {
  if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND || gParameters.GetAnalysisType() == PURELYSPATIALMONOTONE)
    gParameters.SetAnalysisType(PURELYSPATIAL);
  gParameters.SetSequentialScanning(false);
  gParameters.SetNumberEllipses(0);
  gParameters.SetDuczmalCorrectionEllipses(false);
  gParameters.SetRiskType(STANDARDRISK);
  gParameters.SetPowerCalculation(false);
  gParameters.SetValidatePriorToCalculation(true);
  if (gParameters.GetTimeTrendAdjustmentType() == NONPARAMETRIC)
    gParameters.SetTimeTrendAdjustmentType(STRATIFIED_RANDOMIZATION);
  gParameters.SetSimulationType(STANDARD);
  gParameters.SetOutputSimulationData(false);
  //since 'clusters to include' feature no longer in interface, adjust settings so that analysis is equivalent
  if (gParameters.GetAnalysisType() == PURELYTEMPORAL && gParameters.GetIncludeClustersType() == ALIVECLUSTERS) {
    gParameters.SetAnalysisType(PROSPECTIVEPURELYTEMPORAL);
    gParameters.SetAdjustForEarlierAnalyses(false);
  }
  if (gParameters.GetAnalysisType() == SPACETIME && gParameters.GetIncludeClustersType() == ALIVECLUSTERS) {
    gParameters.SetAnalysisType(PROSPECTIVESPACETIME);
    gParameters.SetAdjustForEarlierAnalyses(false);
  }
  gParameters.SetIncludeClustersType(ALLCLUSTERS);
}

/** event triggered when case file edit control text changes */
void __fastcall TfrmAnalysis::edtCaseFileNameChange(TObject *Sender) {
  edtCaseFileName->Hint = edtCaseFileName->Text;
}

/** event triggered when control file edit control text changes */
void __fastcall TfrmAnalysis::edtControlFileNameChange(TObject *Sender){
  edtControlFileName->Hint = edtControlFileName->Text;
}

/** event triggered when coordinates file edit control text changes */
void __fastcall TfrmAnalysis::edtCoordinateFileNameChange(TObject *Sender){
  edtCoordinateFileName->Hint = edtCoordinateFileName->Text;
}

/** event triggered when special grid file edit control text changes */
void __fastcall TfrmAnalysis::edtGridFileNameChange(TObject *Sender) {
  edtGridFileName->Hint = edtGridFileName->Text;
}

/** event triggered when log linear time trend adjustment edit control is exited. */
void __fastcall TfrmAnalysis::edtLogLinearExit(TObject *Sender) {
  double dValue = atof(edtLogLinear->Text.c_str());
  if (edtLogLinear->Text.IsEmpty()) {
    Application->MessageBox("The percentage per year value can not be blank.", "Parameter Error" , MB_OK);
    PageControl1->ActivePage = tbTimeParameter;
    edtLogLinear->SetFocus();
  }
  else if (!Check_TimeTrendPercentage(dValue)) {
    PageControl1->ActivePage = tbTimeParameter;
    edtLogLinear->SetFocus();
  }
}

/** event triggered when maximum circle population file edit control text changes */
void __fastcall TfrmAnalysis::edtMaxCirclePopulationFilenameChange(TObject *Sender) {
  edtMaxCirclePopulationFilename->Hint = edtMaxCirclePopulationFilename->Text;
}

/** event triggered when maximum spatial cluster size edit control is exited. */
void __fastcall TfrmAnalysis::edtMaxSpatialClusterSizeExit(TObject *Sender) {
  try {
    if (!edtMaxSpatialClusterSize->Text.Length() || atof(edtMaxSpatialClusterSize->Text.c_str()) == 0)
      ZdException::GenerateNotification("Please specify a maximum spatial cluster size.","edtMaxSpatialClusterSizeExit()");
    SetReportingSmallerClustersText();
  }
  catch (ZdException & x) {
    x.AddCallpath("edtMaxSpatialClusterSizeExit()", "TfrmAnalysis");
    PageControl1->ActivePage = tbScanningWindow;
    edtMaxSpatialClusterSize->SetFocus();
    DisplayBasisException(this, x);
  }
}

/** event triggered when maximum temporal cluster size edit control is exited. */
void __fastcall TfrmAnalysis::edtMaxTemporalClusterSizeExit(TObject *Sender) {
  try {
    if (!edtMaxTemporalClusterSize->Text.Length() || atof(edtMaxTemporalClusterSize->Text.c_str()) == 0)
      ZdException::GenerateNotification("Please specify a maximum temporal cluster size.","edtMaxTemporalClusterSizeExit()");
  }
  catch (ZdException & x) {
    x.AddCallpath("edtMaxTemporalClusterSizeExit()", "TfrmAnalysis");
    PageControl1->ActivePage = tbScanningWindow;
    edtMaxTemporalClusterSize->SetFocus();
    DisplayBasisException(this, x);
  }
}

/** event triggered when Monte Carlo replications control is exited. */
void __fastcall TfrmAnalysis::edtMontCarloRepsExit(TObject *Sender) {
  try {
     if (! CheckReplicas()) {
       PageControl1->ActivePage = tbAnalysis;
       edtMontCarloReps->SetFocus();
     }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtMontCarloRepsExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** event triggered when population file edit control text changes */
void __fastcall TfrmAnalysis::edtPopFileNameChange(TObject *Sender){
  edtPopFileName->Hint = edtPopFileName->Text;
}

/** event triggered when day control, of prospective start date, is exited. */
void __fastcall TfrmAnalysis::edtProspectiveStartDateDayExit(TObject *Sender) {
  if ((atoi(edtProspectiveStartDateDay->Text.c_str()) < 1) || (atoi(edtProspectiveStartDateDay->Text.c_str()) > 31)) {
    Application->MessageBox("Please specify a valid day.", "Parameter Error" , MB_OK);
    PageControl1->ActivePage = tbTimeParameter;
    edtProspectiveStartDateDay->SetFocus();
  }
}

/** event triggered when month control, of prospective start date, is exited. */
void __fastcall TfrmAnalysis::edtProspectiveStartDateMonthExit(TObject *Sender) {
  try {
    if ( ! Check_Month(atoi(edtProspectiveStartDateMonth->Text.c_str()), "Prospective Space-Time Start Month")) {
      PageControl1->ActivePage = tbTimeParameter;
      edtProspectiveStartDateMonth->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtProspectiveStartDateMonthExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** event triggered when year control, of prospective start date, is exited. */
void __fastcall TfrmAnalysis::edtProspectiveStartDateYearExit(TObject *Sender) {
  try {
    if ( ! Check_Year(atoi(edtProspectiveStartDateYear->Text.c_str()), "Prospective Space-Time Start Year")) {
      PageControl1->ActivePage = tbTimeParameter;
      edtProspectiveStartDateYear->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtProspectiveStartDateYearExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** event triggered when 'Report only clusters smaller than ...' edit control is exited */
void __fastcall TfrmAnalysis::edtReportClustersSmallerThanExit(TObject *Sender){
  try {                                     
    if (!edtReportClustersSmallerThan->Text.Length() || atof(edtReportClustersSmallerThan->Text.c_str()) == 0)
      ZdException::GenerateNotification("Please specify a maximum cluster size for reported clusters\n"
                                        "between 0 and the maximum spatial cluster size of %g.",
                                        "edtReportClustersSmallerThanExit()", atof(edtMaxSpatialClusterSize->Text.c_str()));
  }
  catch (ZdException & x) {
    PageControl1->ActivePage = tbScanningWindow;
    edtReportClustersSmallerThan->SetFocus();
    x.AddCallpath("edtReportClustersSmallerThanExit()","TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** event triggered when results file edit control text changes */
void __fastcall TfrmAnalysis::edtResultFileChange(TObject *Sender){
  edtResultFile->Hint = edtResultFile->Text;
}

/** event triggered when day control, of study period end date, is exited. */
void __fastcall TfrmAnalysis::edtStudyPeriodEndDateDayExit(TObject *Sender) {
  if ((atoi(edtStudyPeriodEndDateDay->Text.c_str()) < 1) || (atoi(edtStudyPeriodEndDateDay->Text.c_str()) > 31)) {
    PageControl1->ActivePage = tbAnalysis;
    edtStudyPeriodEndDateDay->SetFocus();
  }
}

/** event triggered when month control, of study period end date, is exited. */
void __fastcall TfrmAnalysis::edtStudyPeriodEndDateMonthExit(TObject *Sender) {
  try {
    if (! Check_Month(atoi(edtStudyPeriodEndDateMonth->Text.c_str()), "Study Period End Month")) {
      PageControl1->ActivePage = tbAnalysis;
      edtStudyPeriodEndDateMonth->SetFocus();
    }
    else if (!edtStudyPeriodEndDateDay->Enabled)  // reset day to last day of month
      edtStudyPeriodEndDateDay->Text = DaysThisMonth(atoi(edtStudyPeriodEndDateYear->Text.c_str()),
                                                     atoi(edtStudyPeriodEndDateMonth->Text.c_str()));
  }
  catch (ZdException &x) {
    x.AddCallpath("edtStudyPeriodEndDateMonthExit","TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** event triggered when year control, of study period end date, is exited. */
void __fastcall TfrmAnalysis::edtStudyPeriodEndDateYearExit(TObject *Sender) {
  int   iYear = atoi(edtStudyPeriodEndDateYear->Text.c_str()),
        iMonth =  atoi(edtStudyPeriodEndDateMonth->Text.c_str());

  try {
    if (! Check_Year(iYear, "Study Period End Year")) {
      PageControl1->ActivePage = tbAnalysis;
      edtStudyPeriodEndDateYear->SetFocus();
    }
    else if (!edtStudyPeriodEndDateDay->Enabled && iMonth == 2/*February*/) // reset day to last day of month
      edtStudyPeriodEndDateDay->Text = DaysThisMonth(iYear, iMonth);
  }
  catch (ZdException &x) {
    x.AddCallpath("edtStudyPeriodEndDateYearExit()","TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** event triggered when day control, of study period start date, is exited. */
void __fastcall TfrmAnalysis::edtStudyPeriodStartDateDayExit(TObject *Sender) {
  if ((atoi(edtStudyPeriodStartDateDay->Text.c_str()) < 1) || (atoi(edtStudyPeriodStartDateDay->Text.c_str()) > 31)) {
    Application->MessageBox("Please specify a valid day.", "Parameter Error" , MB_OK);
    PageControl1->ActivePage = tbAnalysis;
    edtStudyPeriodStartDateDay->SetFocus();
  }
}

/** event triggered when month control, of study period start date, is exited. */
void __fastcall TfrmAnalysis::edtStudyPeriodStartDateMonthExit(TObject *Sender) {
  try {
    if ( ! Check_Month(atoi(edtStudyPeriodStartDateMonth->Text.c_str()), "Study Period Start Month")) {
      PageControl1->ActivePage = tbAnalysis;
      edtStudyPeriodStartDateMonth->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtStudyPeriodStartDateMonthExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** event triggered when year control, of study period start date, is exited. */
void __fastcall TfrmAnalysis::edtStudyPeriodStartDateYearExit(TObject *Sender) {
  try {
    if ( ! Check_Year(atoi(edtStudyPeriodStartDateYear->Text.c_str()), "Study Period Start Year")) {
      PageControl1->ActivePage = tbAnalysis;
      edtStudyPeriodStartDateYear->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtStudyPeriodStartDateYearExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** event triggered when month control, of prospective start date, is exited. */
void __fastcall TfrmAnalysis::edtTimeIntervalLengthExit(TObject *Sender) {
  bool bParamsOk = Check_IntervalLength();
  if (! bParamsOk) {
    PageControl1->ActivePage = tbTimeParameter;
    edtTimeIntervalLength->SetFocus();
  }
}

/** enables/disables the appropraite buttons and controls based on their category type */
void TfrmAnalysis::EnableActions(bool bEnable) {
   for(int i = 0; i < gpList->ActionCount; ++i) {
      TAction* pAction = dynamic_cast<TAction*>(gpList->Actions[i]);
      if (pAction) {
         if(pAction->Category == CATEGORY_ALL || pAction->Category == CATEGORY_ANALYSIS)
             pAction->Enabled = bEnable;
         else if(pAction->Category == CATEGORY_ANALYSIS_RUN)
             pAction->Enabled = !bEnable;
      }
   }
}

/** enables controls of 'additional optional output file' radio group */
void TfrmAnalysis::EnableAdditionalOutFilesOptionsGroup(bool bRelativeRisks) {
  chkRelativeRiskEstimatesAreaAscii->Enabled = bRelativeRisks;
  chkRelativeRiskEstimatesAreaDBase->Enabled = bRelativeRisks;
  lblRelativeRiskEstimatesArea->Enabled = bRelativeRisks;
}

/** enables or disables the temporal time trend adjustment control group */
void TfrmAnalysis::EnableAdjustmentForTimeTrendOptionsGroup(bool bEnable, bool bTimeStratified, bool bLogLinearCalculated, bool bLogYearPercentage) {
  TimeTrendAdjustmentType eTimeTrendAdjustmentType(GetAdjustmentTimeTrendControlType());

  // trump control enables
  bTimeStratified &= bEnable;
  bLogLinearCalculated &= bEnable;
  bLogYearPercentage &= bEnable;

  rdgTemporalTrendAdj->Enabled = bEnable;

  rdgTemporalTrendAdj->Controls[1]->Enabled = bTimeStratified;
  if (bEnable && !bTimeStratified && eTimeTrendAdjustmentType == STRATIFIED_RANDOMIZATION)
    SetTemporalTrendAdjustmentControl(NOTADJUSTED);

  rdgTemporalTrendAdj->Controls[2]->Enabled = bLogLinearCalculated;
  if (bEnable && !bLogLinearCalculated && eTimeTrendAdjustmentType == CALCULATED_LOGLINEAR_PERC)
    SetTemporalTrendAdjustmentControl(NOTADJUSTED);

  rdgTemporalTrendAdj->Controls[3]->Enabled = bLogYearPercentage;
  lblLogLinear->Enabled = bLogYearPercentage;
  edtLogLinear->Enabled = bLogYearPercentage && eTimeTrendAdjustmentType == LOGLINEAR_PERC;
  edtLogLinear->Color = edtLogLinear->Enabled ? clWindow : clInactiveBorder;
  if (bEnable && !bLogYearPercentage && eTimeTrendAdjustmentType == LOGLINEAR_PERC)
    SetTemporalTrendAdjustmentControl(NOTADJUSTED);
}

/** enabled study period and prospective date precision based on time interval unit */
void TfrmAnalysis::EnableDatesByTimeIntervalUnits() {
  AnalysisType eAnalysisType(GetAnalysisControlType());

  if (eAnalysisType == PURELYSPATIAL)
    EnableStudyPeriodDates(true, true, true);
  else if (rbUnitYear->Checked) {
    EnableStudyPeriodDates(true, false, false);
    edtProspectiveStartDateMonth->Text = edtStudyPeriodEndDateMonth->Text;
    edtProspectiveStartDateDay->Text = edtStudyPeriodEndDateDay->Text;
  }
  else if (rbUnitMonths->Checked) {
    EnableStudyPeriodDates(true, true, false);
    edtProspectiveStartDateDay->Text = DaysThisMonth(atoi(edtProspectiveStartDateYear->Text.c_str()), atoi(edtProspectiveStartDateMonth->Text.c_str()));
  }
  else if (rbUnitDay->Checked)
    EnableStudyPeriodDates(true, true, true);
  else
    ZdGenerateException("Time interval units no set.","EnableDatesByTimeIntervalUnits()");

  EnableProspectiveSurveillanceGroup(eAnalysisType == PROSPECTIVEPURELYTEMPORAL || eAnalysisType == PROSPECTIVESPACETIME);
}

/** enabled prospective start date controls */
void TfrmAnalysis::EnableProspectiveStartDate(bool bEnable) {
  //trump enabling based upon earlier analyses adjustment and precision of time controls
  bEnable = bEnable && chkAdjustForEarlierAnalyses->Checked && GetPrecisionOfTimesControlType() != NONE;
  edtProspectiveStartDateYear->Enabled = bEnable;
  lblProspectiveStartDate->Enabled = bEnable;
  lblProspectiveStartYear->Enabled = bEnable;
  lblProspectiveStartMonth->Enabled = bEnable;
  lblProspectiveStartDay->Enabled = bEnable;
  edtProspectiveStartDateYear->Color =  edtProspectiveStartDateYear->Enabled ? clWindow : clInactiveBorder;
  edtProspectiveStartDateMonth->Enabled = bEnable && (rbUnitDay->Checked || rbUnitMonths->Checked);
  edtProspectiveStartDateMonth->Color = edtProspectiveStartDateMonth->Enabled ? clWindow : clInactiveBorder;
  edtProspectiveStartDateDay->Enabled = bEnable &&  rbUnitDay->Checked;
  edtProspectiveStartDateDay->Color =  edtProspectiveStartDateDay->Enabled ? clWindow : clInactiveBorder;
}

/** enables or disables the prospective start date group control */
void TfrmAnalysis::EnableProspectiveSurveillanceGroup(bool bEnable) {
   grpProspectiveSurveillance->Enabled = bEnable;
   chkAdjustForEarlierAnalyses->Enabled = bEnable;
   lblAdjustForEarlierAnalyses->Enabled = bEnable;
   EnableProspectiveStartDate(bEnable);
}

/** enables or disables the spatial options group control */
void TfrmAnalysis::EnableSpatialOptionsGroup(bool bEnable, bool bEnableIncludePurelyTemporal) {
   rdgSpatialOptions->Enabled = bEnable;
   rdoSpatialPercentage->Enabled = bEnable;
   rdoSpatialDistance->Enabled = bEnable;
   chkInclPurTempClust->Enabled = bEnable && bEnableIncludePurelyTemporal;
   lblMaxSpatialClusterSize->Enabled = bEnable;
   edtMaxSpatialClusterSize->Enabled = bEnable;
   edtMaxSpatialClusterSize->Color = bEnable ? clWindow : clInactiveBorder;
   chkRestrictReportedClusters->Enabled = bEnable;
   edtReportClustersSmallerThan->Enabled = bEnable && chkRestrictReportedClusters->Checked;
   edtReportClustersSmallerThan->Color = bEnable && chkRestrictReportedClusters->Checked ? clWindow : clInactiveBorder;
   lblReportSmallerClusters->Enabled = bEnable;
}

/** enables or disables the study period group controls */
void TfrmAnalysis::EnableStudyPeriodDates(bool bYear, bool bMonth, bool bDay) {
   edtStudyPeriodStartDateYear->Enabled = bYear;
   edtStudyPeriodStartDateYear->Color = bYear ? clWindow : clInactiveBorder;
   edtStudyPeriodEndDateYear->Enabled = bYear;
   edtStudyPeriodEndDateYear->Color = bYear ? clWindow : clInactiveBorder;

   edtStudyPeriodStartDateMonth->Enabled = bMonth;
   edtStudyPeriodStartDateMonth->Color = bMonth ? clWindow : clInactiveBorder;
   if (!bMonth) edtStudyPeriodStartDateMonth->Text= "1";
   edtStudyPeriodEndDateMonth->Enabled = bMonth;
   edtStudyPeriodEndDateMonth->Color = bMonth ? clWindow : clInactiveBorder;
   if (!bMonth) edtStudyPeriodEndDateMonth->Text = "12";

   edtStudyPeriodStartDateDay->Enabled = bDay;
   edtStudyPeriodStartDateDay->Color = bDay ? clWindow : clInactiveBorder;
   if (!bDay) edtStudyPeriodStartDateDay->Text = "1";
   edtStudyPeriodEndDateDay->Enabled = bDay;
   edtStudyPeriodEndDateDay->Color = bDay ? clWindow : clInactiveBorder;
   if (!bDay) edtStudyPeriodEndDateDay->Text = DaysThisMonth(atoi(edtStudyPeriodEndDateYear->Text.c_str()), atoi(edtStudyPeriodEndDateMonth->Text.c_str()));
}

/** enables or disables the temporal options group control */
void TfrmAnalysis::EnableTemporalOptionsGroup(bool bEnable, bool bEnableIncludePurelySpatial) {
  rdgTemporalOptions->Enabled = bEnable;
  rdoPercentageTemproal->Enabled = bEnable;
  rdoTimeTemproal->Enabled = bEnable;
  chkIncludePurSpacClust->Enabled = bEnable && bEnableIncludePurelySpatial;
  edtMaxTemporalClusterSize->Enabled = bEnable;
  edtMaxTemporalClusterSize->Color = bEnable ? clWindow : clInactiveBorder;
  lblMaxTemporalClusterSize->Enabled = bEnable;
}

/** enables or disables the time interval group control */
void TfrmAnalysis::EnableTimeIntervalUnitsGroup(bool bEnable) {
   //trump enable if precision of times control indicates that there is no dates
   bEnable = bEnable && GetPrecisionOfTimesControlType() != NONE;
   rdgTimeIntervalUnits->Enabled = bEnable;
   lblTimeIntervalUnits->Enabled = bEnable;
   edtTimeIntervalLength->Enabled = bEnable;
   edtTimeIntervalLength->Color = bEnable ? clWindow : clInactiveBorder;
   lblTimeIntervalLength->Enabled = bEnable;
   rbUnitYear->Enabled =  bEnable;
   rbUnitMonths->Enabled = bEnable;
   rbUnitDay->Enabled = bEnable;
   EnableDatesByTimeIntervalUnits();
}

/** event triggered when key pressed for controls that can contain real numbers. */
void __fastcall TfrmAnalysis::FloatKeyPress(TObject *Sender, char &Key) {
  if (!strchr("-0123456789.\b",Key))
    Key = 0;
}

/** event triggered when form is activated */
void __fastcall TfrmAnalysis::FormActivate(TObject *Sender) {
   EnableActions(true);
}

/** form close event - added parameter filename to history list */
void __fastcall TfrmAnalysis::FormClose(TObject *Sender, TCloseAction &Action) {
  Action = caFree;
  try {
    GetToolkit().AddParameterToHistory(gParameters.GetSourceFileName().c_str());
    frmMainForm->RefreshOpenList();
  }
  catch(...){}
}

/** returns adjustment for time trend type for control index */
TimeTrendAdjustmentType TfrmAnalysis::GetAdjustmentTimeTrendControlType() const {
  TimeTrendAdjustmentType eReturn;

  switch (rdgTemporalTrendAdj->ItemIndex) {
    case 0  : eReturn = NOTADJUSTED; break;
    case 1  : eReturn = STRATIFIED_RANDOMIZATION; break;
    case 2  : eReturn = CALCULATED_LOGLINEAR_PERC; break;
    case 3  : eReturn = LOGLINEAR_PERC; break;
    default : ZdGenerateException("Unknown index type '%d'.", "GetAdjustmentTimeTrendControlType()", rdgTemporalTrendAdj->ItemIndex);
  }
  return eReturn;
}

/** returns analysis type for analysis control index */
AnalysisType TfrmAnalysis::GetAnalysisControlType() const {
  AnalysisType eReturn;

  switch (rgTypeAnalysis->ItemIndex) {
    case 0  : eReturn = PURELYSPATIAL; break;
    case 1  : eReturn = PURELYTEMPORAL; break;
    case 2  : eReturn = PROSPECTIVEPURELYTEMPORAL; break;
    case 3  : eReturn = SPACETIME; break;
    case 4  : eReturn = PROSPECTIVESPACETIME; break;
    default : ZdGenerateException("Unknown index type '%d'.", "GetAnalysisControlType()", rgTypeAnalysis->ItemIndex);
  }
  return eReturn;
}

/** Returns parameter filename fullpath. */
const char * TfrmAnalysis::GetFileName() {
  return gParameters.GetSourceFileName().c_str();
}

/** returns precision of time type for precision control index */
DatePrecisionType TfrmAnalysis::GetPrecisionOfTimesControlType() const {
  DatePrecisionType eReturn;

  switch (rgPrecisionTimes->ItemIndex) {
    case 0  : eReturn = DAY; break;
    case 1  : eReturn = NONE; break;
    default : ZdGenerateException("Unknown index type '%d'.", "GetPrecisionOfTimesControlType()", rgPrecisionTimes->ItemIndex);
  };
  return eReturn;
}

/** save interface parameters to parameter class and return it */
CParameters * TfrmAnalysis::GetSession() {
  try {
    SaveParameterSettings();
  }
  catch (ZdException & x) {
    x.AddCallpath("GetSession()", "TfrmAnalysis");
    throw;
  }
  return &gParameters;
}

/** Sets passed ZdDate to study period end date as defined by TEditBoxes.*/
ZdDate & TfrmAnalysis::GetStudyPeriodEndDate(ZdDate & Date) {
  try {
    Date.SetYear(static_cast<unsigned short>(atoi(edtStudyPeriodEndDateYear->Text.c_str())));
    Date.SetMonth(static_cast<unsigned short>(atoi(edtStudyPeriodEndDateMonth->Text.c_str())));
    Date.SetDay(static_cast<unsigned short>(atoi(edtStudyPeriodEndDateDay->Text.c_str())));
  }
  catch (ZdException & x) {
    x.AddCallpath("GetStudyPeriodEndDate()", "TfrmAnalysis");
    throw;
  }
  return Date;
}

/** Sets passed ZdDate to study period start date as defined by TEditBoxes.*/
ZdDate & TfrmAnalysis::GetStudyPeriodStartDate(ZdDate & Date) {
  try {
    Date.SetYear(static_cast<unsigned short>(atoi(edtStudyPeriodStartDateYear->Text.c_str())));
    Date.SetMonth(static_cast<unsigned short>(atoi(edtStudyPeriodStartDateMonth->Text.c_str())));
    Date.SetDay(static_cast<unsigned short>(atoi(edtStudyPeriodStartDateDay->Text.c_str())));
  }
  catch (ZdException & x) {
    x.AddCallpath("GetStudyPeriodStartDate()", "TfrmAnalysis");
    throw;
  }
  return Date;
}

/** return precision type for time intervals */
DatePrecisionType TfrmAnalysis::GetTimeIntervalControlType() const {
  DatePrecisionType eReturn;

  if (rbUnitYear->Checked)
    eReturn = YEAR;
  else if (rbUnitMonths->Checked)
    eReturn = MONTH;
  else
    eReturn = DAY;
  return eReturn;
}

/** class initialization */
void TfrmAnalysis::Init() {
  cboCriteriaSecClusters->ItemIndex = 0;
  rgPrecisionTimes->ItemIndex = -1; //ensures that click event will trigger
}

/** Modally shows import dialog. */
void TfrmAnalysis::LaunchImporter() {
  try {
    std::auto_ptr<TBDlgDataImporter> pDialog(new TBDlgDataImporter(0, *this));
    pDialog->ShowModal();
  }
  catch (ZdException & x) {
    x.AddCallpath("LaunchImporter()", "TfrmAnalysis");
    throw;
  }
}

/** event triggered when key pressed for control that can contain natural numbers */
void __fastcall TfrmAnalysis::NaturalNumberKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789\b",Key))
    Key = 0;
}

/** method called in response to 'type of analysis' radio group click event */
void TfrmAnalysis::OnAnalysisTypeClick() {
  bool  bPoisson(rgProbability->ItemIndex == POISSON),
        bBernoulli(rgProbability->ItemIndex == BERNOULLI),
        bSpaceTimePermutation(rgProbability->ItemIndex == SPACETIMEPERMUTATION);

  try {
    switch (GetAnalysisControlType()) {
      case PURELYSPATIAL             :
        EnableAdjustmentForTimeTrendOptionsGroup(false, false, false, false);
        EnableSpatialOptionsGroup(true, false);
        EnableTimeIntervalUnitsGroup(false);
        EnableTemporalOptionsGroup(false, false);
        break;
      case PURELYTEMPORAL            :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson, bPoisson);
        EnableSpatialOptionsGroup(false, false);
        EnableTimeIntervalUnitsGroup(true);
        EnableTemporalOptionsGroup(true, false);
        break;
      case SPACETIME                 :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, bPoisson, bPoisson, bPoisson);
        EnableSpatialOptionsGroup(true, !bSpaceTimePermutation);
        EnableTimeIntervalUnitsGroup(true);
        EnableTemporalOptionsGroup(true, !bSpaceTimePermutation);
        break;
      case PROSPECTIVESPACETIME      :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, bPoisson, bPoisson, bPoisson);
        EnableSpatialOptionsGroup(true, !bSpaceTimePermutation);
        EnableTimeIntervalUnitsGroup(true);
        EnableTemporalOptionsGroup(true, !bSpaceTimePermutation);
        break;
      case PROSPECTIVEPURELYTEMPORAL :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson, bPoisson);
        EnableSpatialOptionsGroup(false, false);
        EnableTimeIntervalUnitsGroup(true);
        EnableTemporalOptionsGroup(true, false);
        break;
      default : ZdGenerateException("Unknown analysis type '%d'.", "OnAnalysisTypeClick()", GetAnalysisControlType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("OnAnalysisTypeClick()","TfrmAnalysis");
    throw;
  }
}

/** method called in response to 'precision of times' radio group click event */
void TfrmAnalysis::OnPrecisionTimesClick() {
  DatePrecisionType     eDatePrecisionType(GetPrecisionOfTimesControlType());
  try {
    // disable probability models that don't match precision of time/analysis type combination
    rgProbability->Controls[SPACETIMEPERMUTATION]->Enabled = eDatePrecisionType != NONE;
    if (eDatePrecisionType == NONE) {
      // if none was selected and space-time permutation was selected, reset model to Poisson
      if (rgProbability->ItemIndex == SPACETIMEPERMUTATION)
        rgProbability->ItemIndex = POISSON;
      // switch analysis type to purely spatial if no dates in input data
      if (GetAnalysisControlType() != PURELYSPATIAL)
        SetAnalysisControl(PURELYSPATIAL);
    }
    // simulate probability model type control click, to ensure that all controls are synchronized
    OnProbabilityModelClick();
  }
  catch(ZdException &x) {
    x.AddCallpath("OnPrecisionTimesClick()","TfrmAnalysis");
    throw;
  }
}

/** method called in response to 'probability model' radio group click event */
void TfrmAnalysis::OnProbabilityModelClick() {
  DatePrecisionType     eDatePrecisionType(GetPrecisionOfTimesControlType());
  AnalysisType          eAnalysisType;

  try {
    switch (rgProbability->ItemIndex) {
      case POISSON   		:
      case BERNOULLI            : rgTypeAnalysis->Controls[0]->Enabled = true;
                                  rgTypeAnalysis->Controls[1]->Enabled = eDatePrecisionType != NONE;
                                  rgTypeAnalysis->Controls[2]->Enabled = eDatePrecisionType != NONE;
                                  rgTypeAnalysis->Controls[3]->Enabled = eDatePrecisionType != NONE;
                                  rgTypeAnalysis->Controls[4]->Enabled = eDatePrecisionType != NONE;
                                  EnableAdditionalOutFilesOptionsGroup(true);
                                  rdoPercentageTemproal->Caption = "Percent of Study Period (<= 90%)";
                                  lblSimulatedLogLikelihoodRatios->Caption = "Simulated Log Likelihood Ratios";
                                  break;
      case SPACETIMEPERMUTATION : rgTypeAnalysis->Controls[0]->Enabled = false;
                                  rgTypeAnalysis->Controls[1]->Enabled = false;
                                  rgTypeAnalysis->Controls[2]->Enabled = false;
                                  rgTypeAnalysis->Controls[3]->Enabled = true;
                                  rgTypeAnalysis->Controls[4]->Enabled = true;
                                  eAnalysisType = GetAnalysisControlType();
                                  if (!(eAnalysisType == SPACETIME || eAnalysisType == PROSPECTIVESPACETIME))
                                    SetAnalysisControl(SPACETIME);
                                  EnableAdditionalOutFilesOptionsGroup(false);
                                  rdoPercentageTemproal->Caption = "Percent of Study Period (<= 50%)";
                                  lblSimulatedLogLikelihoodRatios->Caption = "Simulated Test Statistics";
                                  break;
      default : ZdGenerateException("Unknown probabilty model index '%d'.",
                                    "OnProbablityModelClick()", rgProbability->ItemIndex);
    }
    // simulate analysis type control click to synchronize controls
    OnAnalysisTypeClick();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnProbablityModelClick()","TfrmAnalysis");
    throw;
  }
}

/** parses up a date string and places it into the given month, day, year
    interace text control (TEdit *). Defaults prospective survallience start
    date to months/days to like study period end date.                       */
void TfrmAnalysis::ParseDate(const char * szDate, TEdit *pYear, TEdit *pMonth, TEdit *pDay) {
  UInt  uiMonth, uiDay, uiYear;
  int   iPrecision;

  //set values only if valid, prevent interface from having invalid date when first loaded.
  if ((iPrecision = CharToMDY(&uiMonth, &uiDay, &uiYear, szDate)) > 0) {
    switch (iPrecision) {
      case 3 : if (uiYear >= MIN_YEAR && uiYear <= MAX_YEAR &&
                   uiMonth >= 1 && uiMonth <= 12 &&  uiDay >= 1 && uiDay <= DaysThisMonth(uiYear, uiMonth)) {
                  pYear->Text = uiYear;
                  pMonth->Text = uiMonth;
                  pDay->Text = uiDay;
               }
               break;
      case 2 : if (uiYear >= MIN_YEAR && uiYear <= MAX_YEAR && uiMonth >= 1 && uiMonth <= 12) {
                 pYear->Text = uiYear;
                 pMonth->Text = uiMonth;
                 if (pYear == edtProspectiveStartDateYear)
                   pDay->Text = std::min(static_cast<unsigned int>(atoi(edtStudyPeriodEndDateDay->Text.c_str())), DaysThisMonth(uiYear, uiMonth));
                 else if (pYear == edtStudyPeriodEndDateYear)
                   pDay->Text = DaysThisMonth(uiYear, uiMonth);
                 else
                   pDay->Text = 1;
               }
               break;
      case 1 : if (uiYear >= MIN_YEAR && uiYear <= MAX_YEAR) {
                 pYear->Text = uiYear;
                 if (pYear == edtProspectiveStartDateYear) {
                   pMonth->Text = edtStudyPeriodEndDateMonth->Text;
                   pDay->Text = std::min(static_cast<unsigned int>(atoi(edtStudyPeriodEndDateDay->Text.c_str())),
                                         DaysThisMonth(uiYear, static_cast<unsigned int>(atoi(edtStudyPeriodEndDateMonth->Text.c_str()))));
                 }
               }
               break;
    };
  }
}

/** event triggered when key pressed for control that can contain positive real numbers */
void __fastcall TfrmAnalysis::PositiveFloatKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789.\b",Key))
    Key = 0;
}

/** event triggered when time interval unit type selected as 'day' */
void __fastcall TfrmAnalysis::rbUnitDayClick(TObject *Sender) {
  rdoTimeTemproal->Caption = "Days";
  EnableDatesByTimeIntervalUnits();
}

/** event triggered when time interval unit type selected as 'month' */
void __fastcall TfrmAnalysis::rbUnitMonthsClick(TObject *Sender) {
  rdoTimeTemproal->Caption = "Months";
  EnableDatesByTimeIntervalUnits();
}

/** event triggered when time interval unit type selected as 'year' */
void __fastcall TfrmAnalysis::rbUnitYearClick(TObject *Sender) {
    rdoTimeTemproal->Caption = "Years";
    EnableDatesByTimeIntervalUnits();
}

/** event triggered when 'Adjustment for time trend' type control clicked */
void __fastcall TfrmAnalysis::rdgTemporalTrendAdjClick(TObject *Sender) {
  switch (GetAdjustmentTimeTrendControlType()) {
    case LOGLINEAR_PERC : edtLogLinear->Enabled = true;
                          edtLogLinear->Color = clWindow;
                          break;
    default             : edtLogLinear->Enabled = false;
                          edtLogLinear->Color = clInactiveBorder;
  }
}

/** event triggered when maximum spatial type selected */
void __fastcall TfrmAnalysis::rdoMaximumSpatialTypeClick(TObject *Sender) {
   SetReportingSmallerClustersText();
}

/** event triggered when coordinates type control clicked */
void __fastcall TfrmAnalysis::rgCoordinatesClick(TObject *Sender) {
  try {
    SetSpatialDistanceCaption();
    SetReportingSmallerClustersText();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgCoordinatesClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** event triggered when 'precision of times' type control clicked */
void __fastcall TfrmAnalysis::rgPrecisionTimesClick(TObject *Sender) {
  try {
    OnPrecisionTimesClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgPrecisionTimesClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** event triggered when 'probability model' type control clicked */
void __fastcall TfrmAnalysis::rgProbabilityClick(TObject *Sender) {
  try {
     OnProbabilityModelClick();
  }
  catch ( ZdException & x ) {
    x.AddCallpath("rgProbabilityClick()", "TfrmAnalysis" );
    DisplayBasisException( this, x );
  }
}

/** event triggered when 'analysis' type control clicked */
void __fastcall TfrmAnalysis::rgTypeAnalysisClick(TObject *Sender) {
  try {
     OnAnalysisTypeClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgTypeAnalysisClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

/** launches 'save as' dialog to permit user saving current settings to parameter file */
void TfrmAnalysis::SaveAs() {
  try {
    if (SaveDialog->Execute()) {
      WriteSession(SaveDialog->FileName.c_str());
      frmAnalysis->Caption = SaveDialog->FileName;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("SaveAs()", "TfrmAnalysis");
    throw;
  }
}

/** sets CParameters class with settings in form */
void TfrmAnalysis::SaveParameterSettings() {
  ZdString sString;

  try {
    Caption = gParameters.GetSourceFileName().c_str();
    //Input File Tab
    gParameters.SetCaseFileName(edtCaseFileName->Text.c_str());
    gParameters.SetControlFileName(edtControlFileName->Text.c_str());
    gParameters.SetPrecisionOfTimesType(GetPrecisionOfTimesControlType());
    gParameters.SetPopulationFileName(edtPopFileName->Text.c_str());
    gParameters.SetCoordinatesFileName(edtCoordinateFileName->Text.c_str());
    gParameters.SetSpecialGridFileName(edtGridFileName->Text.c_str(), false, true);
    gParameters.SetMaxCirclePopulationFileName(edtMaxCirclePopulationFilename->Text.c_str(), false, true);
    gParameters.SetCoordinatesType((CoordinatesType)rgCoordinates->ItemIndex);
    //Analysis Tab
    gParameters.SetAnalysisType(GetAnalysisControlType());
    gParameters.SetProbabilityModelType((ProbabiltyModelType)rgProbability->ItemIndex);
    gParameters.SetAreaRateType((AreaRateType)(rgScanAreas->ItemIndex + 1));
    sString.printf("%i/%i/%i", atoi(edtStudyPeriodStartDateYear->Text.c_str()),
                   atoi(edtStudyPeriodStartDateMonth->Text.c_str()), atoi(edtStudyPeriodStartDateDay->Text.c_str()));
    gParameters.SetStudyPeriodStartDate(sString.GetCString());
    sString.printf("%i/%i/%i", atoi(edtStudyPeriodEndDateYear->Text.c_str()),
                   atoi(edtStudyPeriodEndDateMonth->Text.c_str()), atoi(edtStudyPeriodEndDateDay->Text.c_str()));
    gParameters.SetStudyPeriodEndDate(sString.GetCString());
    gParameters.SetNumberMonteCarloReplications(atoi(edtMontCarloReps->Text.c_str()));
    gParameters.SetTerminateSimulationsEarly(chkTerminateEarly->Checked);
    //Scanning Window Tab
    gParameters.SetMaximumGeographicClusterSize(atof(edtMaxSpatialClusterSize->Text.c_str()));
    gParameters.SetMaximumSpacialClusterSizeType(rdoSpatialPercentage->Checked ? PERCENTAGEOFMEASURETYPE : DISTANCETYPE);
    gParameters.SetMaximumTemporalClusterSize(atof(edtMaxTemporalClusterSize->Text.c_str()));
    gParameters.SetMaximumTemporalClusterSizeType(rdoPercentageTemproal->Checked ? PERCENTAGETYPE : TIMETYPE);
    gParameters.SetIncludePurelyTemporalClusters(chkInclPurTempClust->Enabled && chkInclPurTempClust->Checked);
    gParameters.SetRestrictReportedClusters(chkRestrictReportedClusters->Enabled && chkRestrictReportedClusters->Checked);
    gParameters.SetMaximumReportedGeographicalClusterSize(atof(edtReportClustersSmallerThan->Text.c_str()));
    gParameters.SetIncludePurelySpatialClusters(chkIncludePurSpacClust->Enabled && chkIncludePurSpacClust->Checked);
    //Time Parameter Tab
    gParameters.SetTimeIntervalUnitsType(GetTimeIntervalControlType());
    gParameters.SetTimeIntervalLength(atoi(edtTimeIntervalLength->Text.c_str()));
    gParameters.SetTimeTrendAdjustmentPercentage(atof(edtLogLinear->Text.c_str()));
    gParameters.SetTimeTrendAdjustmentType(rdgTemporalTrendAdj->Enabled ? GetAdjustmentTimeTrendControlType() : NOTADJUSTED);
    gParameters.SetAdjustForEarlierAnalyses(chkAdjustForEarlierAnalyses->Checked);
    sString.printf("%i/%i/%i", atoi(edtProspectiveStartDateYear->Text.c_str()),
                   atoi(edtProspectiveStartDateMonth->Text.c_str()), atoi(edtProspectiveStartDateDay->Text.c_str()));
    gParameters.SetProspectiveStartDate(sString.GetCString());
    //Output File Tab
    gParameters.SetOutputFileName(edtResultFile->Text.c_str());
    gParameters.SetOutputClusterLevelAscii(chkClustersInColumnFormatAscii->Checked);
    gParameters.SetOutputClusterLevelDBase(chkClustersInColumnFormatDBase->Checked);
    gParameters.SetOutputAreaSpecificAscii(chkCensusAreasReportedClustersAscii->Checked);
    gParameters.SetOutputAreaSpecificDBase(chkCensusAreasReportedClustersDBase->Checked);
    gParameters.SetOutputRelativeRisksAscii(chkRelativeRiskEstimatesAreaAscii->Enabled && chkRelativeRiskEstimatesAreaAscii->Checked);
    gParameters.SetOutputRelativeRisksDBase(chkRelativeRiskEstimatesAreaDBase->Enabled && chkRelativeRiskEstimatesAreaDBase->Checked);
    gParameters.SetOutputSimLogLikeliRatiosAscii(chkSimulatedLogLikelihoodRatiosAscii->Checked);
    gParameters.SetOutputSimLogLikeliRatiosDBase(chkSimulatedLogLikelihoodRatiosDBase->Checked);
    gParameters.SetCriteriaForReportingSecondaryClusters((CriteriaSecondaryClustersType)cboCriteriaSecClusters->ItemIndex);
  }
  catch (ZdException & x) {
    x.AddCallpath("SaveParameterSettings()", "TfrmAnalysis");
    throw;
  }
}

/** sets analysis type control for AnalysisType */
void TfrmAnalysis::SetAnalysisControl(AnalysisType eAnalysisType) {
  switch (eAnalysisType) {
    case PURELYSPATIAL                  : rgTypeAnalysis->ItemIndex = 0; break;
    case PURELYTEMPORAL                 : rgTypeAnalysis->ItemIndex = 1; break;
    case PROSPECTIVEPURELYTEMPORAL      : rgTypeAnalysis->ItemIndex = 2; break;
    case SPACETIME                      : rgTypeAnalysis->ItemIndex = 3; break;
    case PROSPECTIVESPACETIME           : rgTypeAnalysis->ItemIndex = 4; break;
    case SPATIALVARTEMPTREND            :
    case PURELYSPATIALMONOTONE          : //not in interface -- default
    default                             : rgTypeAnalysis->ItemIndex = 0;
  }
}

/** Sets case filename in interface and parameters class. */
void TfrmAnalysis::SetCaseFile(const char * sCaseFileName) {
  edtCaseFileName->Text = sCaseFileName;
}

/** Sets control filename in interface and parameters class. */
void TfrmAnalysis::SetControlFile(const char * sControlFileName) {
  edtControlFileName->Text = sControlFileName;
}

/** Sets coordinates filename in interface and parameters class. */
void TfrmAnalysis::SetCoordinateFile(const char * sCoordinateFileName) {
  edtCoordinateFileName->Text = sCoordinateFileName;
}

/** sets coordinate type */
void TfrmAnalysis::SetCoordinateType(CoordinatesType eCoordinatesType) {
  rgCoordinates->ItemIndex = eCoordinatesType;
}

/** Sets special population filename in interface and parameters class. */
void TfrmAnalysis::SetMaximumCirclePopulationFile(const char * sMaximumCirclePopulationFileName) {
  edtMaxCirclePopulationFilename->Text = sMaximumCirclePopulationFileName;
}

/** Sets population filename in interface and parameters class. */
void TfrmAnalysis::SetPopulationFile(const char * sPopulationFileName) {
  edtPopFileName->Text = sPopulationFileName;
}

/** sets precision of times type control for DatePrecisionType */
void TfrmAnalysis::SetPrecisionOfTimesControl(DatePrecisionType eDatePrecisionType) {
  switch (eDatePrecisionType) {
    case NONE  : rgPrecisionTimes->ItemIndex = 1; break;
    case YEAR  :
    case MONTH :
    case DAY   :
    default    : rgPrecisionTimes->ItemIndex = 0;
  }
}

/** sets caption of label next to spatial option for reporting smaller clusters
    to reflect limitations specified by maximum spatial cluster size edit control */
void TfrmAnalysis::SetReportingSmallerClustersText() {
  ZdString      sTemp;

  if (rdoSpatialPercentage->Checked)
    sTemp.printf("percent of population at risk (< %s).", edtMaxSpatialClusterSize->Text.c_str());
  else if (rgCoordinates->ItemIndex == CARTESIAN)
    sTemp.printf("cartesian units in radius (< %s).", edtMaxSpatialClusterSize->Text.c_str());
  else
    sTemp.printf("kilometers in radius (< %s).", edtMaxSpatialClusterSize->Text.c_str());

  lblReportSmallerClusters->Caption = sTemp.GetCString();
}

/** Sets caption of spatial distance radio button based upon coordinates group setting. */
void TfrmAnalysis::SetSpatialDistanceCaption() {
  try {
    switch (rgCoordinates->ItemIndex) {
      case 0  : rdoSpatialDistance->Caption = "Cartesian Units Radius";
                break;
      case 1  : rdoSpatialDistance->Caption = "Kilometer Radius";
                break;
      default : ZdException::Generate("Unknown coordinates radio button index: '%i'.",
                                      "rgCoordinatesClick()", rgCoordinates->ItemIndex);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("SetSpatialDistanceCaption()", "TfrmAnalysis");
    throw;
  }
}

/** Sets special grid filename in interface and parameters class. */
void TfrmAnalysis::SetSpecialGridFile(const char * sSpecialGridFileName) {
  gParameters.SetSpecialGridFileName(sSpecialGridFileName, false, true);
  edtGridFileName->Text = sSpecialGridFileName;
}

/** Sets time trend adjustment control's index */
void TfrmAnalysis::SetTemporalTrendAdjustmentControl(TimeTrendAdjustmentType eTimeTrendAdjustmentType) {
  switch (eTimeTrendAdjustmentType) {
    case NOTADJUSTED               : rdgTemporalTrendAdj->ItemIndex = NOTADJUSTED; break;
    case NONPARAMETRIC             : rdgTemporalTrendAdj->ItemIndex = NONPARAMETRIC; break;
    case LOGLINEAR_PERC            : rdgTemporalTrendAdj->ItemIndex = LOGLINEAR_PERC; break;
    case STRATIFIED_RANDOMIZATION  : rdgTemporalTrendAdj->ItemIndex = LOGLINEAR_PERC + 1; break;
    default                        : rdgTemporalTrendAdj->ItemIndex = NOTADJUSTED;
  }
}

/** Internal setup */
void TfrmAnalysis::Setup(const char * sParameterFileName) {
  try {
    PageControl1->ActivePage = tbInputFiles;
    if (sParameterFileName)
      gParameters.Read(sParameterFileName, gNullPrint);
    DefaultHiddenParameters();
    SetupInterface();
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()", "TfrmAnalysis");
    throw;
  }
}

/** Sets all interface controls using the CParameters session object */
void TfrmAnalysis::SetupInterface() {
  try {
    //Input File Tab
    Caption = gParameters.GetSourceFileName().c_str();
    edtCaseFileName->Text = gParameters.GetCaseFileName().c_str();
    edtControlFileName->Text = gParameters.GetControlFileName().c_str();
    SetPrecisionOfTimesControl(gParameters.GetPrecisionOfTimesType());
    edtPopFileName->Text = gParameters.GetPopulationFileName().c_str();
    edtMaxCirclePopulationFilename->Text = gParameters.GetMaxCirclePopulationFileName().c_str();
    edtCoordinateFileName->Text = gParameters.GetCoordinatesFileName().c_str();
    edtGridFileName->Text = gParameters.GetSpecialGridFileName().c_str();
    rgCoordinates->ItemIndex = gParameters.GetCoordinatesType();
    //Analysis Tab
    rgProbability->ItemIndex = gParameters.GetProbabiltyModelType();
    SetAnalysisControl(gParameters.GetAnalysisType());
    rgScanAreas->ItemIndex = (int)gParameters.GetAreaScanRateType() - 1;
    ParseDate(gParameters.GetStudyPeriodStartDate().c_str(), edtStudyPeriodStartDateYear, edtStudyPeriodStartDateMonth, edtStudyPeriodStartDateDay);
    ParseDate(gParameters.GetStudyPeriodEndDate().c_str(), edtStudyPeriodEndDateYear, edtStudyPeriodEndDateMonth, edtStudyPeriodEndDateDay);
    edtMontCarloReps->Text = gParameters.GetNumReplicationsRequested();
    chkTerminateEarly->Checked = gParameters.GetTerminateSimulationsEarly();
    //Scanning Window Tab
    edtMaxSpatialClusterSize->Text = gParameters.GetMaximumGeographicClusterSize();
    chkInclPurTempClust->Checked = gParameters.GetIncludePurelyTemporalClusters();
    edtMaxTemporalClusterSize->Text = gParameters.GetMaximumTemporalClusterSize();
    rdoPercentageTemproal->Checked = gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE;
    rdoTimeTemproal->Checked = gParameters.GetMaximumTemporalClusterSizeType() == TIMETYPE;
    chkIncludePurSpacClust->Checked = gParameters.GetIncludePurelySpatialClusters();
    rdoSpatialPercentage->Checked = gParameters.GetMaxGeographicClusterSizeType() != DISTANCETYPE; // default checked
    rdoSpatialDistance->Checked = gParameters.GetMaxGeographicClusterSizeType() == DISTANCETYPE;
    SetSpatialDistanceCaption();
    //Time Parameter Tab
    if (gParameters.GetTimeIntervalUnitsType() == NONE) gParameters.SetTimeIntervalUnitsType(YEAR);
    if (gParameters.GetTimeIntervalLength() == 0) gParameters.SetTimeIntervalLength(1);
    rbUnitYear->Checked = (gParameters.GetTimeIntervalUnitsType() == YEAR);  // use to be 0
    rbUnitMonths->Checked = (gParameters.GetTimeIntervalUnitsType() == MONTH);  // use to be 1
    rbUnitDay->Checked = (gParameters.GetTimeIntervalUnitsType() == DAY);  // use to be 2
    edtTimeIntervalLength->Text = gParameters.GetTimeIntervalLength();
    SetTemporalTrendAdjustmentControl(gParameters.GetTimeTrendAdjustmentType());
    edtLogLinear->Text = gParameters.GetTimeTrendAdjustmentPercentage();
    chkAdjustForEarlierAnalyses->Checked = gParameters.GetAdjustForEarlierAnalyses();
    if (gParameters.GetProspectiveStartDate().length() > 0)
      ParseDate(gParameters.GetProspectiveStartDate().c_str(), edtProspectiveStartDateYear, edtProspectiveStartDateMonth, edtProspectiveStartDateDay);
    //Output File Tab
    edtResultFile->Text = gParameters.GetOutputFileName().c_str();
    chkRelativeRiskEstimatesAreaAscii->Checked = gParameters.GetOutputRelativeRisksAscii();
    chkRelativeRiskEstimatesAreaDBase->Checked = gParameters.GetOutputRelativeRisksDBase();
    chkSimulatedLogLikelihoodRatiosAscii->Checked  = gParameters.GetOutputSimLoglikeliRatiosAscii();
    chkSimulatedLogLikelihoodRatiosDBase->Checked = gParameters.GetOutputSimLoglikeliRatiosDBase();
    chkCensusAreasReportedClustersAscii->Checked    = gParameters.GetOutputAreaSpecificAscii();  // Output Census areas in Reported Clusters
    chkClustersInColumnFormatAscii->Checked = gParameters.GetOutputClusterLevelAscii();  // Output Most Likely Cluster for each Centroid
    cboCriteriaSecClusters->ItemIndex = gParameters.GetCriteriaSecondClustersType();
    chkClustersInColumnFormatDBase->Checked = gParameters.GetOutputClusterLevelDBase();
    chkCensusAreasReportedClustersDBase->Checked = gParameters.GetOutputAreaSpecificDBase();
    edtReportClustersSmallerThan->Text = gParameters.GetMaximumReportedGeoClusterSize();
    chkRestrictReportedClusters->Checked = gParameters.GetRestrictingMaximumReportedGeoClusterSize();
    //now enable or disable controls appropriately
    SetReportingSmallerClustersText();
  }
  catch (ZdException & x) {
    x.AddCallpath("SetupInterface()", "TfrmAnalysis");
    throw;
  }
}

/** Validates 'Input Files' tab */
bool TfrmAnalysis::ValidateInputFiles() {
  bool bOk = true;

  try {
    if (edtCaseFileName->Enabled) {// case file edit box enabled
      if (edtCaseFileName->Text.IsEmpty()) {
        Application->MessageBox("Please specify a case file.", "Parameter Error" , MB_OK);
        bOk = false;
      }
      else
        bOk = ValidateFileExists(edtCaseFileName->Text, "Case");

      if (!bOk) {
        PageControl1->ActivePage = tbInputFiles;
        edtCaseFileName->SetFocus();
      }
    }
    //Control file for Bernoulli model only
    if (bOk & (rgProbability->ItemIndex==1)) {// Control file edit box enabled
      if (edtControlFileName->Text.IsEmpty()) {
        Application->MessageBox("For the Bernoulli model, please specify a control file.", "Parameter Error" , MB_OK);
        bOk = false;
      }
      else
        bOk = ValidateFileExists(edtControlFileName->Text, "Control");

      if (!bOk) {
        PageControl1->ActivePage = tbInputFiles;
        edtControlFileName->SetFocus();
      }
    }
    //Pop file for Poisson model only
    if (bOk & (rgProbability->ItemIndex==0)) {// Population file edit box enabled
      if (edtPopFileName->Text.IsEmpty()) {
        Application->MessageBox("For the Poisson model, please specify a population file.", "Parameter Error" , MB_OK);
        bOk = false;
      }
      else
        bOk = ValidateFileExists(edtPopFileName->Text, "Population");

      if (!bOk) {
        PageControl1->ActivePage = tbInputFiles;
        edtPopFileName->SetFocus();
      }
    }
    if (bOk & edtCoordinateFileName->Enabled) {// Coordinates file edit box enabled
      if (edtCoordinateFileName->Text.IsEmpty()) {
        Application->MessageBox("Please specify a coordinates file.", "Parameter Error" , MB_OK);
        bOk = false;
      }
      else
        bOk = ValidateFileExists(edtCoordinateFileName->Text, "Coordinate");

      if (!bOk) {
        PageControl1->ActivePage = tbInputFiles;
        edtCoordinateFileName->SetFocus();
      }
    }
    if (bOk & edtGridFileName->Enabled & !edtGridFileName->Text.IsEmpty()) {// Special grid file edit box enabled
      bOk = ValidateFileExists(edtGridFileName->Text, "Special Grid");
      if (!bOk) {
        PageControl1->ActivePage = tbInputFiles;
        edtGridFileName->SetFocus();
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateInputFiles()", "TfrmAnalysis");
    throw;
  }
  return bOk;
}

/** This function is used right before a job is submitted.  Verifies that
    all the input files exist and can be read.  Also checks each tab to see
    if all settings are in place.                                          */
bool TfrmAnalysis::ValidateParams() {
  bool bDataOk;

  try {
    SaveParameterSettings();
    // check all input tab params
    bDataOk = ValidateInputFiles();
    // check all Analsis and other tab params
    if (bDataOk)
      bDataOk = CheckAnalysisParams();
    if (bDataOk)
      bDataOk = CheckScanningWindowParams();
    if (bDataOk)
      bDataOk = CheckTimeParams();
    if (bDataOk)
      bDataOk = CheckOutputParams();
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateParams()", "TfrmAnalysis");
    bDataOk = false;
    DisplayBasisException(this, x);
  }
  return bDataOk;
}

bool TfrmAnalysis::ValidateReportedSpatialClusterSize() {
  bool   bOkParams=true;

  try {
    if (edtMaxSpatialClusterSize->Enabled && edtReportClustersSmallerThan->Enabled) {
      if (!edtReportClustersSmallerThan->Text.Length() || atof(edtReportClustersSmallerThan->Text.c_str()) == 0)
        ZdException::GenerateNotification("Please specify a maximum cluster size for reported clusters\n"
                                          "between 0 and the maximum spatial cluster size of %g.",
                                          "edtReportClustersSmallerThanExit()", atof(edtMaxSpatialClusterSize->Text.c_str()));


      if (atof(edtReportClustersSmallerThan->Text.c_str()) >= atof(edtMaxSpatialClusterSize->Text.c_str()))
        ZdException::GenerateNotification("The maximum cluster size for reported clusters must be less than the maximum spatial cluster size.\n",
                                          "ValidateReportedSpatialClusterSize()");
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateReportedSpatialClusterSize()","TfrmAnalysis");
    PageControl1->ActivePage = tbScanningWindow;
    edtReportClustersSmallerThan->SetFocus();
    bOkParams = false;
    DisplayBasisException(this, x);
  }
  return bOkParams;
}

bool TfrmAnalysis::ValidateSpatialClusterSize() {
  double dValue;
  bool   bOkParams=true;

  try {
    if (edtMaxSpatialClusterSize->Enabled) {
      if (!edtMaxSpatialClusterSize->Text.Length() || atof(edtMaxSpatialClusterSize->Text.c_str()) == 0)
        ZdException::GenerateNotification("Please specify a maximum spatial cluster size.","ValidateSpatialClusterSize()");

      dValue = atof(edtMaxSpatialClusterSize->Text.c_str());
      if (!(dValue > 0.0 && dValue <= 50.0) && rdoSpatialPercentage->Checked)
        ZdException::GenerateNotification("Please specify valid maximum spatial cluster size between %d - %d.",
                                          "ValidateSpatialClusterSize()", 0, 50);

      gParameters.SetMaximumGeographicClusterSize(atof(edtMaxSpatialClusterSize->Text.c_str()));

      if (rgProbability->ItemIndex == SPACETIMEPERMUTATION && rdoSpatialPercentage->Checked && !edtMaxCirclePopulationFilename->Text.Length())
        ZdException::GenerateNotification("For a Space-Time Permutation model with the maximum spatial cluster size defined as a\n"
                                          "percentage of the population at risk, a Maximum Circle Population file must be specified.\n"
                                          "Alternatively you may choose to specify the maximum as a fixed radius,\n"
                                          " in which no Maximum Circle Population file is required.","ValidateSpatialClusterSize()");

      if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && chkAdjustForEarlierAnalyses->Checked &&
          rdoSpatialPercentage->Checked && rdoSpatialPercentage->Enabled && !edtMaxCirclePopulationFilename->Text.Length())
        ZdException::GenerateNotification("For a Prospective Space-Time analysis adjusting for ealier analyses,\n"
                                          "with the maximum spatial cluster size defined as a percentage of the\n"
                                          "population at risk, a Maximum Circle Population file must be specified.\n"
                                          "Alternatively you may choose to specify the maximum as a fixed radius,\n"
                                          "in which no Maximum Circle Population file is required.","ValidateSpatialClusterSize()");

    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateSpatialClusterSize()","TfrmAnalysis");
    PageControl1->ActivePage = tbScanningWindow;
    edtMaxSpatialClusterSize->SetFocus();
    bOkParams = false;
    DisplayBasisException(this, x);
  }
  return bOkParams;
}

bool TfrmAnalysis::ValidateTemoralClusterSize() {
  bool          bParamsOk = true;
  float         dValue;
  ZdString      sErrorMessage;
  ZdDate        StartDate, EndDate, EndDatePlusOne,StartPlusIntervalDate;
  ZdDateFilter  DateFilter("%4y/%02m/%02d");
  char          FilterBuffer[11], Buffer[10];
  unsigned long ulMaxClusterDays, ulIntervalLengthInDays;

  try {
    //check whether we are specifiying temporal information
    if (edtMaxTemporalClusterSize->Enabled) {
      if (!edtMaxTemporalClusterSize->Text.Length() || atof(edtMaxTemporalClusterSize->Text.c_str()) == 0)
        ZdException::GenerateNotification("Please specify a maximum temporal cluster size.","ValidateTemoralClusterSize()");

      //check maximum temporal cluster size(as percentage pf population) is less
      //than maximum for given probabilty model
      if (rdoPercentageTemproal->Checked) {
        dValue = atof(edtMaxTemporalClusterSize->Text.c_str());
        if (!(dValue > 0.0 && dValue <= (rgProbability->ItemIndex == SPACETIMEPERMUTATION ? 50 : 90))) {
          sErrorMessage << "For the " << rgProbability->Items->Strings[rgProbability->ItemIndex].c_str();
          sErrorMessage << " model, the maximum temporal cluster size as a percent of study period is ";
          sErrorMessage << (rgProbability->ItemIndex == SPACETIMEPERMUTATION ? 50 : 90);
          sErrorMessage << " percent.\nPlease review settings.";
          ZdException::GenerateNotification(sErrorMessage.GetCString(), "ValidateTemoralClusterSize()");
        }
      }
      //check that maximum temporal cluster size(in time units) is less than
      //maximum for probabilty model. Determine the number of days the maximum
      //temporal cluster can be. Compare that against start date plus interval
      //length units.
      else if (rdoTimeTemproal->Checked) {
        GetStudyPeriodStartDate(StartDate);
        GetStudyPeriodEndDate(EndDate);

        //to make start and end day inclusive - add 1 to EndDate date
        EndDatePlusOne = EndDate;
        EndDatePlusOne.AddDays(1);
        ulMaxClusterDays = EndDatePlusOne.GetJulianDayFromCalendarStart() - StartDate.GetJulianDayFromCalendarStart();
        ulMaxClusterDays = (rgProbability->ItemIndex == SPACETIMEPERMUTATION ? ulMaxClusterDays * 0.5 : ulMaxClusterDays * 0.9);

        StartPlusIntervalDate = StartDate;
        //add time interval length as units to modified start date
        switch (gParameters.GetTimeIntervalUnitsType()) {
            case      (YEAR)      : StartPlusIntervalDate.AddYears(static_cast<unsigned short>(atoi(edtMaxTemporalClusterSize->Text.c_str())));
                                    strcpy(Buffer,"year(s)");
                                    break;
            case      (MONTH)     : StartPlusIntervalDate.AddMonths(static_cast<unsigned short>(atoi(edtMaxTemporalClusterSize->Text.c_str())));
                                    //to make start and end day inclusive - add one day to interval
                                    StartPlusIntervalDate.AddDays(1);
                                    strcpy(Buffer,"month(s)");
                                    break;
            case      (DAY)       : //to make start and end day inclusive - add interval length minus 1
                                    StartPlusIntervalDate.AddDays(static_cast<unsigned short>(atoi(edtMaxTemporalClusterSize->Text.c_str())));
                                    strcpy(Buffer,"day(s)");
                                    break;
            default               : ZdGenerateException("Unknown interval unit \"%d\"", "ValidateTemoralClusterSize()", gParameters.GetTimeIntervalUnitsType());
        };
        ulIntervalLengthInDays = StartPlusIntervalDate.GetJulianDayFromCalendarStart() - StartDate.GetJulianDayFromCalendarStart();
        if (ulIntervalLengthInDays > ulMaxClusterDays) {
          DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), StartDate.GetRawDate());
          sErrorMessage << "For the study period starting on " << FilterBuffer << " and ending on ";
          DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), EndDate.GetRawDate());
          sErrorMessage << FilterBuffer << ",\na maximum temporal cluster size of " << edtMaxTemporalClusterSize->Text.c_str();
          sErrorMessage << " " << Buffer << " is greater than " << (rgProbability->ItemIndex == SPACETIMEPERMUTATION ? 50 : 90);
          sErrorMessage << " percent of study period.\nPlease review settings.";
          ZdGenerateException(sErrorMessage.GetCString(), "ValidateTemoralClusterSize()");
        }
      }
      else
        ZdException::GenerateNotification("Type specified as neither percentage nor time.", "ValidateTemoralClusterSize()");
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateTemoralClusterSize()", "TfrmAnalysis");
    bParamsOk = false;
    PageControl1->ActivePage = tbScanningWindow;
    edtMaxTemporalClusterSize->SetFocus();
    Application->MessageBox(x.GetErrorMessage(), "Notification", MB_OK );
  }
  return bParamsOk;
}

/** Writes the session information to disk */
void TfrmAnalysis::WriteSession(const char * sParameterFilename) {
  std::string   sFilename;

  try {
    sFilename = (sParameterFilename ? sParameterFilename : gParameters.GetSourceFileName().c_str());
    if (sFilename.empty())
      SaveAs();
    else {
      if (!access(sFilename.c_str(), 00) && access(sFilename.c_str(), 02))
        ZdException::GenerateNotification("Unable to save session parameters.\n"
                                          "The file is either read only or you do not have write privledges to the directory.",
                                          "WriteSession()");
      SaveParameterSettings();
      gParameters.Write(sFilename.c_str());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("WriteSession()", "TfrmAnalysis");
    throw;
  }
}

