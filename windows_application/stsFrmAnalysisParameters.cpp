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

//---------------------------------------------------------------------------
// Constructor
// If a parameter file is passed it, it will parse it (read it) and set up the interface.
//---------------------------------------------------------------------------
__fastcall TfrmAnalysis::TfrmAnalysis(TComponent* Owner, TActionList* theList, char *sParamFileName) : stsBaseAnalysisChildForm (Owner, theList) {
  try {
    Init();
    Setup(sParamFileName);
  }
  catch (ZdException & x) {
    x.AddCallpath("constructor()", "TfrmAnalysis");
    throw;
  }
}
//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
__fastcall TfrmAnalysis::~TfrmAnalysis() { }

//---------------------------------------------------------------------------
// case file selector
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
// Control file selector -- *.ctl files
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
// Geographic file selector -- *.geo files
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
//  Grid file selector -- *.grd files
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
// Population file selector -- *.pop files
//---------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
// Selects a Results file
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnResultFileBrowseClick(TObject *Sender) {
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.txt";
    OpenDialog1->Filter = "Results files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->FilterIndex = 0;
    OpenDialog1->Title = "Select Results File";
    if (OpenDialog1->Execute()) {
      gParameters.SetOutputFileName(OpenDialog1->FileName.c_str());
      edtResultFile->Text = OpenDialog1->FileName.c_str();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnResultFileBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
// Generic Days checker -- simulates old interface
//MUST CHECK TO SEE IF THE DAY EDIT BOX IS ENABLED FIRST !!!!
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_Days(int iYear, int iMonth, int iDay, char *sDateName) {
  char szMessage[100];
  AnsiString sFinalMessage;
  int iMin = 1, iMax;

  iMax = DaysThisMonth(iYear, iMonth);
  if ((iDay < iMin) || (iDay > iMax)) {
    sFinalMessage += sDateName;
    sprintf(szMessage, ":  Please specify a day between %i and %i.", iMin, iMax);
    sFinalMessage += szMessage;
    Application->MessageBox(sFinalMessage.c_str(), "Parameter Error" , MB_OK);
    return false;
  }
  else
    return true;
}
//------------------------------------------------------------------------------
//Ensures that interval length is not less than one.
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_IntervalLength(int iStartYear, int iStartMonth, int iStartDay,
                                        int iEndYear, int iEndMonth, int iEndDay,
                                        int iIntervalUnits, int iIntervalLength) {
    if (atoi(edtUnitLength->Text.c_str()) < 1) {
      Application->MessageBox("Interval length can not be zero.\nPlease specify an interval length.", "Notification" , MB_OK);
       return false;
    }
    else
       return true;
}
//------------------------------------------------------------------------------
// Generic Month checker -- simulates old interface
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_Month(int iMonth, char *sDateName) {
  if ((iMonth < 1) || (iMonth > 12)) {
    std::string sFinalMessage(sDateName);
    sFinalMessage += ":  Please specify an month between 1 and 12.";
    Application->MessageBox(sFinalMessage.c_str(), "Parameter Error" , MB_OK);
    return false;
  }
  else
     return true;
}
//------------------------------------------------------------------------------
// Simple time trend percentage check.  
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_TimeTrendPercentage(double dValue) {
  if ( ! (dValue > -100.00)) {
    Application->MessageBox("Invalid time trend percentage specified.", "Parameter Error" , MB_OK);
    return false;
  }
  else
     return true;
}
//------------------------------------------------------------------------------
// Generic Year checker -- simulates old interface
//MUST CHECK TO SEE IF THE DAY EDIT BOX IS ENABLED FIRST !!!!
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_Year(int iYear, char *sDateName) {
  char          szMessage[100];

  if ( ! (iYear >= MIN_YEAR) && (iYear <= MAX_YEAR)) {
    std::string sFinalMessage(sDateName);
    sprintf(szMessage, ":  Please specify a year between %i and %i.", MIN_YEAR, MAX_YEAR);
    sFinalMessage += szMessage;
    Application->MessageBox(sFinalMessage.c_str(), "Parameter Error" , MB_OK);
    return false;
  }
  else
     return true;
}
//------------------------------------------------------------------------------
// Verifies all the parameters on the Analysis Tab
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckAnalysisParams() {
  bool bParamsOk = true;

  try {
    //if years enabled, then check values...
    //if start year enabled, assume end year enabled.
    if (edtStartYear->Enabled) {
      bParamsOk = Check_Year(atoi(edtStartYear->Text.c_str()),"Study Period Start Year");
      if (bParamsOk)
         bParamsOk = Check_Year(atoi(edtEndYear->Text.c_str()),"Study Period End Year");
    }
    //if Months enabled, then check values...
    //if start month enabled, assume end month enabled.
    if (bParamsOk && edtStartMonth->Enabled) {
      bParamsOk = Check_Month(atoi(edtStartMonth->Text.c_str()), "Study Period Start Month");
      if (bParamsOk)
         bParamsOk = Check_Month(atoi(edtEndMonth->Text.c_str()), "Study Period End Month");
    }
    //if Days enabled, then check values...
    //if start days enabled, assume end days enabled.
    if (bParamsOk && edtStartDay->Enabled) {
      bParamsOk = Check_Days(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()), atoi(edtStartDay->Text.c_str()),"Study Period Start Date");
      if (bParamsOk)
         bParamsOk = Check_Days(atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),"Study Period End Date");
    }


    if (bParamsOk)
      bParamsOk = CheckDateRange(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()), atoi(edtStartDay->Text.c_str()),
                                 atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),
                                 gParameters.GetTimeIntervalUnitsType(), atoi(edtUnitLength->Text.c_str()));
    if (bParamsOk)
      bParamsOk = CheckReplicas(atoi(edtMontCarloReps->Text.c_str()));
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckAnalysisParams()", "TfrmAnalysis");
    throw;
  }
  return bParamsOk;
}
//------------------------------------------------------------------------------
// Verifies the relationship between a start date, end date, and interval length.
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                  int iEndYear, int iEndMonth, int iEndDay,
                                  int iIntervalUnits, int iIntervalLength) {
  bool          bRangeOk = true;
  ZdString      sErrorMessage;
  ZdDate        StartDate, EndDate;
  ZdDateFilter  DateFilter("%4y/%02m/%02d");
  char          FilterBuffer[11];

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

    if (rgTypeAnalysis->ItemIndex > 0) {/* purely spatial does not use interval length */
      //check that interval length is not greater study period
      //(.i.e. can't have study period that is 20 days and intervals of 3 months)
      //to make start and end day inclusive - add 1 to end date
      EndDate.AddDays(1);
      if (bRangeOk) {
        switch (iIntervalUnits) {
          case      (YEAR)      : StartDate.AddYears(static_cast<unsigned short>(iIntervalLength));
                                  strcpy(FilterBuffer,"year(s)");
                                  break;
          case      (MONTH)     : StartDate.AddMonths(static_cast<unsigned short>(iIntervalLength));
                                  strcpy(FilterBuffer,"month(s)");
                                  break;
          case      (DAY)       : StartDate.AddDays(static_cast<unsigned short>(iIntervalLength));
                                  strcpy(FilterBuffer,"day(s)");
                                  break;
          default               : ZdGenerateException("Unknown interval unit \"%d\"","CheckDateRange()",iIntervalUnits);
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
    x.AddCallpath("CheckDateRange", "TfrmAnalysis");
    throw;
  }
  return bRangeOk;
}
//------------------------------------------------------------------------------
// Verifies all parameters on the Output tab
//------------------------------------------------------------------------------
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
 //   edtResultFile->SetFocus();
    throw;
  }
  return true;
}
//------------------------------------------------------------------------------
// Specific prospective space-time date check
//  Must be between the start and end dates of the analysis
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckProspDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                  int iEndYear, int iEndMonth, int iEndDay,
                                  int iProspYear, int iProspMonth, int iProspDay) {
  bool bRangeOk = true;
  Julian Start, End, Prosp;

  try {
    Start = MDYToJulian(iStartMonth, iStartDay, iStartYear);
    End   = MDYToJulian(iEndMonth, iEndDay, iEndYear);
    Prosp = MDYToJulian(iProspMonth, iProspDay, iProspYear);
    if (! Check_Days(iProspYear, iProspMonth, iProspDay, "Start date of Prospective Space-Time")) {
      PageControl1->ActivePage = tbTimeParameter;
      edtProspDay->SetFocus();
      bRangeOk = false;
    }
    else if ((Prosp < Start) || (Prosp > End)) {
      Application->MessageBox("The Start date of Prospective Space-Time must be between the Study Period start and end dates.", "Parameter Error" , MB_OK);
      PageControl1->ActivePage = tbTimeParameter;
      edtProspYear->SetFocus();
      bRangeOk = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckProspDateRange()", "TfrmAnalysis");
    throw;
  }
  return bRangeOk;
}
//------------------------------------------------------------------------------
// Monte Carol Replications must follow a specific numeric selection
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckReplicas(int iReplicas) {
    if (! ((iReplicas == 0) || (iReplicas == 9) || (iReplicas == 19) || (fmod(iReplicas+1, 1000) == 0.0)) ) {
      Application->MessageBox("Invalid number of replicas specified.", "Parameter Error" , MB_OK);
      return false;
    }
    else
       return true;
}
//------------------------------------------------------------------------------
// Checks the validity of the scanning tab controls
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckScanningWindowParams() {
  return ValidateTemoralClusterSize() && ValidateSpatialClusterSize();
}
//------------------------------------------------------------------------------
// Checks all the time parameters
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckTimeParams() {
  bool bParamsOk = true;
  double dValue;

  try {
    if (gParameters.GetAnalysisType() != PURELYSPATIAL) { //not purely spacial    use to be 0
      if (edtUnitLength->Enabled)
         bParamsOk = Check_IntervalLength(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()),
                                          atoi(edtStartDay->Text.c_str()),  atoi(edtEndYear->Text.c_str()),
                                          atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),
                                          gParameters.GetTimeIntervalUnitsType(), atoi(edtUnitLength->Text.c_str()));
      if (bParamsOk && edtLogPerYear->Enabled) {
        if (edtLogPerYear->Text.IsEmpty()) {
          bParamsOk = false;
          Application->MessageBox("Please enter a number in time trend percentage.", "Parameter Error" , MB_OK);
          PageControl1->ActivePage = tbTimeParameter;
        }
        else {
          dValue = atof(edtLogPerYear->Text.c_str());
          bParamsOk = Check_TimeTrendPercentage(dValue);
        }
      }
      //just need to check if the Prospective year is enabled.
      // if year is enabled, then all others are too...
      if (bParamsOk && edtProspYear->Enabled) {
         bParamsOk = CheckProspDateRange(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()),
                                         atoi(edtStartDay->Text.c_str()), atoi(edtEndYear->Text.c_str()),
                                         atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),
                                         atoi(edtProspYear->Text.c_str()), atoi(edtProspMonth->Text.c_str()),
                                         atoi(edtProspDay->Text.c_str()));
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckTimeParams()", "TfrmAnalysis");
    throw;
  }
  return bParamsOk;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkCensusAreasReportedClustersAsciiClick(TObject *Sender) {
   gParameters.SetOutputAreaSpecificAscii(chkCensusAreasReportedClustersAscii->Checked);
}
//------------------------------------------------------------------------------
// Include relative risks in output
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkRelativeRiskEstimatesAreaAsciiClick(TObject *Sender) {
//    gParameters.m_bOutputRelRisks = chkRelativeRiskEstimatesAreaAscii->Checked;
}
//------------------------------------------------------------------------------
// Include Purely Spacial Clusters selection control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkIncludePurSpacClustClick(TObject *Sender) {
    gParameters.SetIncludePurelySpatialClusters(chkIncludePurSpacClust->Checked);
}
//------------------------------------------------------------------------------
// Include Purely Temporal Clusters selection control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkInclPurTempClustClick(TObject *Sender) {
    gParameters.SetIncludePurelyTemporalClusters(chkInclPurTempClust->Checked);
}
//------------------------------------------------------------------------------
// Simulated Log likelihood ratio set
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkSimulatedLogLikelihoodRatiosAsciiClick(TObject *Sender) {
    gParameters.SetOutputSimLogLikeliRatiosAscii(chkSimulatedLogLikelihoodRatiosAscii->Checked);
}

void __fastcall TfrmAnalysis::chkClustersInColumnFormatAsciiClick(TObject *Sender) {
   gParameters.SetOutputClusterLevelAscii(chkClustersInColumnFormatAscii->Checked);
}

void __fastcall TfrmAnalysis::edtEndDayExit(TObject *Sender) {
  if ((atoi(edtEndDay->Text.c_str()) < 1) || (atoi(edtEndDay->Text.c_str()) > 31)) {
    PageControl1->ActivePage = tbAnalysis;
    edtEndDay->SetFocus();
  }
}

/** Resets parameters that are not present in interface to default value.
    Hidden features are to be used soley in dos version at this time.     */
void TfrmAnalysis::DefaultHiddenParameters() {
  gParameters.SetSequentialScanning(false);
  gParameters.SetNumberEllipses(0);
  gParameters.SetRiskType(STANDARDRISK);
  gParameters.SetPowerCalculation(false);
  gParameters.SetValidatePriorToCalculation(true);
}

//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtEndMonthExit(TObject *Sender) {
  try {
    if ( ! Check_Month(atoi(edtEndMonth->Text.c_str()), "Study Period End Month")) {
      PageControl1->ActivePage = tbAnalysis;
      edtEndMonth->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtEndMonthExit", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtEndYearExit(TObject *Sender) {
  try {
    if ( ! Check_Year(atoi(edtEndYear->Text.c_str()), "Study Period End Year")) {
      PageControl1->ActivePage = tbAnalysis;
      edtEndYear->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtEndYearExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtLogPerYearExit(TObject *Sender) {
  double dValue = atof(edtLogPerYear->Text.c_str());
  if (edtLogPerYear->Text.IsEmpty()) {
    Application->MessageBox("The percentage per year value can not be blank.", "Parameter Error" , MB_OK);
    PageControl1->ActivePage = tbTimeParameter;
    edtLogPerYear->SetFocus();
  }
  else if (!Check_TimeTrendPercentage(dValue)) {
    PageControl1->ActivePage = tbTimeParameter;
    edtLogPerYear->SetFocus();
  }
}
//------------------------------------------------------------------------------
// Validates value entered for Cluster size
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMaxClusterSizeExit(TObject *Sender) {
  try {
    if (!edtMaxClusterSize->Text.Length() || atof(edtMaxClusterSize->Text.c_str()) == 0)
      ZdException::GenerateNotification("Please specify a maximum spatial cluster size.","edtMaxClusterSizeExit()");
    else
      gParameters.SetMaximumGeographicClusterSize(atof(edtMaxClusterSize->Text.c_str()));
  }
  catch (ZdException & x) {
    x.AddCallpath("edtMaxClusterSizeExit()", "TfrmAnalysis");
    edtMaxClusterSize->SetFocus();
    DisplayBasisException(this, x);
  }     
}
//------------------------------------------------------------------------------
// Validates Number of Monte Carlo reps value
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMontCarloRepsExit(TObject *Sender) {
  try {
     if (! CheckReplicas(atoi(edtMontCarloReps->Text.c_str()))) {
       PageControl1->ActivePage = tbAnalysis;
       edtMontCarloReps->SetFocus();
     }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtMontCarloRepsExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
//  Validates value entered for Maximum Temporal Cluster Size
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMaxTemporalClusterSizeExit(TObject *Sender) {
  try {
    if (!edtMaxTemporalClusterSize->Text.Length() || atof(edtMaxTemporalClusterSize->Text.c_str()) == 0)
      ZdException::GenerateNotification("Please specify a maximum temporal cluster size.","edtMaxTemporalClusterSizeExit()");
    else
      gParameters.SetMaximumTemporalClusterSize(atof(edtMaxTemporalClusterSize->Text.c_str()));
  }
  catch (ZdException & x) {
    x.AddCallpath("edtMaxTemporalClusterSizeExit()", "TfrmAnalysis");
    edtMaxTemporalClusterSize->SetFocus();
    DisplayBasisException(this, x);
  }     
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtProspDayExit(TObject *Sender) {
  if ((atoi(edtProspDay->Text.c_str()) < 1) || (atoi(edtProspDay->Text.c_str()) > 31)) {
    Application->MessageBox("Please specify a valid day.", "Parameter Error" , MB_OK);
    PageControl1->ActivePage = tbTimeParameter;
    edtProspDay->SetFocus();
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtProspYearExit(TObject *Sender) {
  try {
    if ( ! Check_Year(atoi(edtProspYear->Text.c_str()), "Prospective Space-Time Start Year")) {
      PageControl1->ActivePage = tbTimeParameter;
      edtProspYear->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtProspYearExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtProspMonthExit(TObject *Sender) {
  try {
    if ( ! Check_Month(atoi(edtProspMonth->Text.c_str()), "Prospective Space-Time Start Month")) {
      PageControl1->ActivePage = tbTimeParameter;
      edtProspMonth->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtProspMonthExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtStartDayExit(TObject *Sender) {
  if ((atoi(edtStartDay->Text.c_str()) < 1) || (atoi(edtStartDay->Text.c_str()) > 31)) {
    Application->MessageBox("Please specify a valid day.", "Parameter Error" , MB_OK);
    PageControl1->ActivePage = tbAnalysis;
    edtStartDay->SetFocus();
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtStartMonthExit(TObject *Sender) {
  try {
    if ( ! Check_Month(atoi(edtStartMonth->Text.c_str()), "Study Period Start Month")) {
      PageControl1->ActivePage = tbAnalysis;
      edtStartMonth->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtStartMonthExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtStartYearExit(TObject *Sender) {
  try {
    if ( ! Check_Year(atoi(edtStartYear->Text.c_str()), "Study Period Start Year")) {
      PageControl1->ActivePage = tbAnalysis;
      edtStartYear->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtStartYearExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtUnitLengthExit(TObject *Sender) {
  bool bParamsOk = Check_IntervalLength(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()),
                                   atoi(edtStartDay->Text.c_str()),  atoi(edtEndYear->Text.c_str()),
                                   atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),
                                   gParameters.GetTimeIntervalUnitsType(), atoi(edtUnitLength->Text.c_str()));

  if (! bParamsOk) {
    PageControl1->ActivePage = tbTimeParameter;
    edtUnitLength->SetFocus();
  }
}

// enables/disables the appropraite buttons and controls based on their category type
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

// enables or disables the PST start date control
void TfrmAnalysis::EnablePSTDate(bool bEnable) {
   GroupBox8->Enabled = bEnable;

   edtProspYear->Enabled = (bEnable && (rgPrecisionTimes->ItemIndex == DAY || rgPrecisionTimes->ItemIndex == MONTH || rgPrecisionTimes->ItemIndex == YEAR));
   edtProspMonth->Enabled = (bEnable && (rgPrecisionTimes->ItemIndex == DAY || rgPrecisionTimes->ItemIndex == MONTH));
   edtProspDay->Enabled = bEnable && rgPrecisionTimes->ItemIndex == DAY;
   edtProspYear->Color =  edtProspYear->Enabled ? clWindow : clInactiveBorder;
   edtProspMonth->Color = edtProspMonth->Enabled ? clWindow : clInactiveBorder;
   edtProspDay->Color =  edtProspDay->Enabled ? clWindow : clInactiveBorder;
}

// enables or disables the spatial control
void TfrmAnalysis::EnableSpatial(bool bEnable, bool bEnableCheckbox, bool bEnableSpatialPercentage) {
   rdoSpatialPercentage->Enabled = bEnableSpatialPercentage;
   rdoSpatialPercentage->Checked = (gParameters.GetMaxGeographicClusterSizeType() == PERCENTAGEOFMEASURETYPE && bEnableSpatialPercentage) ? true : false;
   rdoSpatialDistance->Enabled = bEnable;
   rdoSpatialDistance->Checked = (gParameters.GetMaxGeographicClusterSizeType() == PERCENTAGEOFMEASURETYPE && bEnableSpatialPercentage) ? false : true;
   chkInclPurTempClust->Enabled = bEnableCheckbox;
   chkInclPurTempClust->Checked = (bEnableCheckbox && gParameters.GetIncludePurelyTemporalClusters());
   edtMaxClusterSize->Enabled = bEnable;
   edtMaxClusterSize->Color = bEnable ? clWindow : clInactiveBorder;
}

// enables or disables the start and end day time interval controls
void TfrmAnalysis::EnableStartAndEndDay(bool bEnable) {
   edtStartDay->Enabled = bEnable;
   edtEndDay->Enabled = bEnable;
   edtStartDay->Color = bEnable ? clWindow : clInactiveBorder;
   edtEndDay->Color = bEnable ? clWindow : clInactiveBorder;
}

// enables or disables the start and end month time interval controls
void TfrmAnalysis::EnableStartAndEndMonth(bool bEnable) {
   edtStartMonth->Enabled = bEnable;
   edtEndMonth->Enabled = bEnable;
   edtStartMonth->Color = bEnable ? clWindow : clInactiveBorder;
   edtEndMonth->Color = bEnable ? clWindow : clInactiveBorder;
}

// enables or disables the start and end year time interval controls
void TfrmAnalysis::EnableStartAndEndYear(bool bEnable) {
   edtStartYear->Enabled = bEnable;
   edtEndYear->Enabled = bEnable;
   edtStartYear->Color = bEnable ? clWindow : clInactiveBorder;
   edtEndYear->Color = bEnable ? clWindow : clInactiveBorder;
}

// enables or disables the temporal time trend adjustment
void TfrmAnalysis::EnableTemporalTimeTrendAdjust(bool bEnableRadioGroup, bool bEnableNonParametric, bool bEnableLogYearEditBox) {
  rgTemporalTrendAdj->Enabled = bEnableRadioGroup;
  rgTemporalTrendAdj->Controls[1]->Enabled = bEnableNonParametric;
  edtLogPerYear->Enabled = bEnableLogYearEditBox;
  rgTemporalTrendAdj->ItemIndex = gParameters.GetTimeTrendAdjustmentType();
  Edit1->Enabled = bEnableLogYearEditBox;
}

// enables or disables the temporal control
void TfrmAnalysis::EnableTemporal(bool bEnable, bool bEnableCheckbox, bool bEnablePercentage) {
  GroupBox5->Enabled = bEnable;
  rdoPercentageTemproal->Enabled = bEnablePercentage;
  rdoPercentageTemproal->Checked = (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE && bEnablePercentage) ? true : false;
  rdoTimeTemproal->Enabled = bEnable;
  rdoTimeTemproal->Checked = (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE && bEnablePercentage) ? false : true;
  chkIncludePurSpacClust->Enabled = bEnableCheckbox;
  chkIncludePurSpacClust->Checked = (bEnableCheckbox && gParameters.GetIncludePurelySpatialClusters());
  edtMaxTemporalClusterSize->Enabled = bEnable;
  edtMaxTemporalClusterSize->Color = bEnable ? clWindow : clInactiveBorder;
}

// enables or disables the time interval control
void TfrmAnalysis::EnableTimeInterval(bool bEnable) {
   GroupBox6->Enabled = bEnable;
   rbUnitDay->Enabled = bEnable;
   rbUnitMonths->Enabled = bEnable;
   rbUnitYear->Enabled = bEnable;
   edtUnitLength->Enabled = bEnable;
   edtUnitLength->Color = bEnable ? clWindow : clInactiveBorder;

   rbUnitYear->Enabled = (rgPrecisionTimes->ItemIndex == DAY || rgPrecisionTimes->ItemIndex == MONTH || rgPrecisionTimes->ItemIndex == YEAR) && bEnable;
   rbUnitMonths->Enabled = (rgPrecisionTimes->ItemIndex == DAY || rgPrecisionTimes->ItemIndex == MONTH) && bEnable;
   rbUnitDay->Enabled = (rgPrecisionTimes->ItemIndex == DAY) && bEnable;
}

//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::FloatKeyPress(TObject *Sender, char &Key) {
  if (!strchr("-0123456789.\b",Key))
    Key = 0;
}

//---------------------------------------------------------------------------
// The form is closed and all allocated memory for the form is freed.
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::FormClose(TObject *Sender, TCloseAction &Action) {
  Action = caFree;
}

//---------------------------------------------------------------------------
// Returns parameter filename fullpath.
//---------------------------------------------------------------------------
char * TfrmAnalysis::GetFileName() {
  return gsParamFileName.c_str();
}

//---------------------------------------------------------------------------
// returns the class global gpParams
//---------------------------------------------------------------------------
CParameters * TfrmAnalysis::GetSession() {
  try {
    SaveTextParameters();
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
    Date.SetYear(static_cast<unsigned short>(atoi(edtEndYear->Text.c_str())));
    Date.SetMonth(static_cast<unsigned short>(atoi(edtEndMonth->Text.c_str())));
    Date.SetDay(static_cast<unsigned short>(atoi(edtEndDay->Text.c_str())));
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
    Date.SetYear(static_cast<unsigned short>(atoi(edtStartYear->Text.c_str())));
    Date.SetMonth(static_cast<unsigned short>(atoi(edtStartMonth->Text.c_str())));
    Date.SetDay(static_cast<unsigned short>(atoi(edtStartDay->Text.c_str())));
  }
  catch (ZdException & x) {
    x.AddCallpath("GetStudyPeriodStartDate()", "TfrmAnalysis");
    throw;
  }
  return Date;
}

// global initializations
void TfrmAnalysis::Init() {
   cboCriteriaSecClusters->ItemIndex = 0;
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
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::mitClearSpecialGridEditClick(TObject *Sender) {
  edtGridFileName->Clear();
  gParameters.SetSpecialGridFileName("", false, true);
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::NaturalNumberKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789\b",Key))
    Key = 0;
}

//------------------------------------------------------------------------------
// Type of Analsyis is changed - this is the major affector of all other controls
// this does all enabling and disabling of other controls with the exception of the
// model control which enables and disables this control's options and the precision
// time control which sync's the precision between time controls
//------------------------------------------------------------------------------
void TfrmAnalysis::OnAnalysisTypeClick() {
   try {
      gParameters.SetAnalysisType((AnalysisType)(rgTypeAnalysis->ItemIndex + 1));
      bool bPoisson = (gParameters.GetProbabiltyModelType() == POISSON);
    switch (rgTypeAnalysis->ItemIndex) {
       case 0:                     // purely spatial
          // disable time trend adjustment
          EnableTemporalTimeTrendAdjust(false, false, false);

          // disable clusters to include
          rgClustersToInclude->Enabled = false;
          
          EnableSpatial(true, false, true);       // enable spatial but not checkbox
          EnableTimeInterval(false);              // disable time intervals
          EnableTemporal(false, false, false);    // disable temporal
          EnablePSTDate(false);                   // disable start date PST
          break;
       case 1:                     // purely temporal
          // Enables Time Trend Adjust without Non-Param
          EnableTemporalTimeTrendAdjust(bPoisson, false, (bPoisson && rgTemporalTrendAdj->ItemIndex == 2));
          rgTemporalTrendAdj->ItemIndex = ((rgTemporalTrendAdj->ItemIndex != 1) ? gParameters.GetTimeTrendAdjustmentType() : 0 );

          // Enables Clusters to include
          rgClustersToInclude->Enabled = true;
          rgClustersToInclude->ItemIndex = (gParameters.GetAliveClustersOnly() ? 1 : 0);

          EnableSpatial(false, false, false);  // Disables Spatial
          EnableTimeInterval(true);            // Enables time intervals
          EnableTemporal(true, false, true);   // Enables temporal without checkbox
          EnablePSTDate(false);                // Disables Start date PST
          break;
       case 2:                     // retrospective space-time
          //Enables Time Trend Adjust
          EnableTemporalTimeTrendAdjust(bPoisson, bPoisson, (bPoisson && rgTemporalTrendAdj->ItemIndex == 2));
          //Enables clusters to include
          rgClustersToInclude->Enabled = true;
          rgClustersToInclude->ItemIndex = (gParameters.GetAliveClustersOnly() ? 1 : 0);

          EnableSpatial(true, !(gParameters.GetProbabiltyModelType() == 2), true);     //Enables spatial
          EnableTimeInterval(true);                                  //Enables time intervals
          EnableTemporal(true,!(gParameters.GetProbabiltyModelType() == 2), true);     //Enables temporal
          EnablePSTDate(false);                                      //Disables Start date PST
          break;
       case 3:                     // prospective space-time
          //Enables Time Trend Adjust
          EnableTemporalTimeTrendAdjust(bPoisson, bPoisson, (bPoisson && rgTemporalTrendAdj->ItemIndex == 2));
          //disables clusters to include
          rgClustersToInclude->Enabled = false;

          EnableSpatial(true, !(gParameters.GetProbabiltyModelType() == 2), false);      //Enables Spatial % box disable
          EnableTimeInterval(true);                                    //Enables time intervals
          EnableTemporal(true, !(gParameters.GetProbabiltyModelType() == 2), false);     //Enables temporal with checkbox but disable % option radio button
          EnablePSTDate(true);                                         //Enables Start Date PST
          break;
    }

    // if none not enabled and set, then set to case precision Year instead - AJV 10/4/2002
    if(rgPrecisionTimes->ItemIndex == 0 && !(rgPrecisionTimes->Controls[0]->Enabled))
       rgPrecisionTimes->ItemIndex = 1;
   }
   catch (ZdException &x) {
      x.AddCallpath("OnAnalysisTypeClick()", "TfrmAnalysis");
      throw;
   }
}


//---------------------------------------------------------------------------
//  When the time precision control is changed, various interace options are
//  toggled and changed.
void TfrmAnalysis::OnPrecisionTimesClick() {
   try {
    gParameters.SetPrecisionOfTimesType((DatePrecisionType)rgPrecisionTimes->ItemIndex);
    switch (rgPrecisionTimes->ItemIndex) {
      case NONE : EnableStartAndEndYear(true);
                  EnableStartAndEndMonth(true);
                  EnableStartAndEndDay(true);
                  GroupBox6->Enabled = false;
                  break;
      case YEAR   : //Study Period same precision
                  EnableStartAndEndYear(true);
                  EnableStartAndEndMonth(false);
                  EnableStartAndEndDay(false);
                  edtStartMonth->Text="1";
                  edtStartDay->Text = "1";
                  edtEndMonth->Text = "12";
                  edtEndDay->Text = "31";
                  GroupBox6->Enabled = true;
                  break;
      case MONTH  :  //Study Period same precision
                  EnableStartAndEndYear(true);
                  EnableStartAndEndMonth(true);
                  EnableStartAndEndDay(false);
                  edtEndDay->Text = DaysThisMonth(atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()));
                  edtStartDay->Text = "1";
                  GroupBox6->Enabled = true;
                  break;
      case DAY    : //Study Period same precision
                  EnableStartAndEndYear(true);
                  EnableStartAndEndMonth(true);
                  EnableStartAndEndDay(true);
                  GroupBox6->Enabled = true;
                  break;
    }

    // enable or disable the appropraite Analysis Types and Space-Time Model
    rgTypeAnalysis->Controls[0]->Enabled = true;
    rgTypeAnalysis->Controls[1]->Enabled = !(rgPrecisionTimes->ItemIndex == NONE);
    rgTypeAnalysis->Controls[2]->Enabled = !(rgPrecisionTimes->ItemIndex == NONE);
    rgTypeAnalysis->Controls[3]->Enabled = !(rgPrecisionTimes->ItemIndex == NONE);
    if(rgPrecisionTimes->ItemIndex == NONE && (rgTypeAnalysis->ItemIndex == 1 || rgTypeAnalysis->ItemIndex == 2 || rgTypeAnalysis->ItemIndex == 3) )
       rgTypeAnalysis->ItemIndex = 0;    // if none was selected, set the anbalysis type to purely spatial
    rgProbability->Controls[2]->Enabled = !(rgPrecisionTimes->ItemIndex == NONE);
    if(rgPrecisionTimes->ItemIndex == NONE && rgProbability->ItemIndex == 2)
       rgProbability->ItemIndex = 0;    // if none was selected and space time was selected reset model to Poisson

    //Time intervals same or greater precision enabled if enabled
    if (GroupBox6->Enabled) {
      switch (gParameters.GetPrecisionOfTimesType()) {
         case 1  : rbUnitYear->Checked = true;
                   rbUnitMonths->Checked = false;
                   rbUnitDay->Checked = false;
                   break;
         case 2  : if (! rbUnitYear->Checked && ! rbUnitMonths->Checked)
                     rbUnitMonths->Checked = true;
                   rbUnitDay->Checked = false;
                   break;
         case 3  : if (!rbUnitYear->Checked && !rbUnitMonths->Checked && !rbUnitDay->Checked)
                     rbUnitDay->Checked = true;
                   break;
         default : if (!rbUnitYear->Checked && !rbUnitMonths->Checked && !rbUnitDay->Checked)
                     rbUnitYear->Checked = true;
      }
    }

    // prospective year group box
    EnablePSTDate(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME);

     // this is the OnPrecisionChange() function
     if (gParameters.GetAnalysisType() == PURELYSPATIAL && gParameters.GetPrecisionOfTimesType() != NONE)  // used to be geAnalysisType == 0
       if (gParameters.GetTimeIntervalUnitsType() > gParameters.GetPrecisionOfTimesType() )
         gParameters.SetTimeIntervalUnitsType(gParameters.GetPrecisionOfTimesType());

     OnProbabilityModelClick();
   }
   catch(ZdException &x) {
      x.AddCallpath("OnPrecisionTimesClick()", "TfrmAnalysis");
      throw;
   }
}

//------------------------------------------------------------------------------
// Probability is changed which should only affect Analysis Type  - AJV 10/04/2002
//------------------------------------------------------------------------------
void TfrmAnalysis::OnProbabilityModelClick() {
   try {
      gParameters.SetProbabilityModelType((ProbabiltyModelType)rgProbability->ItemIndex);
     //enable buttons based on selected model
     switch (rgProbability->ItemIndex) {
        case 0:  // drop through, same as 1
        case 1:
           rgTypeAnalysis->Controls[0]->Enabled = true;
           rgTypeAnalysis->Controls[1]->Enabled =(rgPrecisionTimes->ItemIndex != 0);
           chkRelativeRiskEstimatesAreaAscii->Enabled = true;
           chkRelativeRiskEstimatesAreaDBase->Enabled = true;
           rdoPercentageTemproal->Caption = "Percent of Study Period (<= 90%)";
           break;
        case 2:
           rgTypeAnalysis->Controls[0]->Enabled = false;
           rgTypeAnalysis->Controls[1]->Enabled = false;
           rgTypeAnalysis->Controls[2]->Enabled = true;
           rgTypeAnalysis->Controls[3]->Enabled = true;
           if(rgTypeAnalysis->ItemIndex == 0 || rgTypeAnalysis->ItemIndex == 1)
              rgTypeAnalysis->ItemIndex = 2;
           chkRelativeRiskEstimatesAreaAscii->Enabled = false;
           chkRelativeRiskEstimatesAreaDBase->Enabled = false;
           rdoPercentageTemproal->Caption = "Percent of Study Period (<= 50%)";
           break;
     }
     OnAnalysisTypeClick();
   }
   catch (ZdException &x) {
      x.AddCallpath("OnProbablityModelClick()", "TfrmAnalysis");
      throw;
   }
}

//------------------------------------------------------------------------------
// Temporal Time Trend control
//------------------------------------------------------------------------------
void TfrmAnalysis::OnTemporalTrendClick() {
   try {
      gParameters.SetTimeTrendAdjustmentType((TimeTrendAdjustmentType)rgTemporalTrendAdj->ItemIndex);
      switch (rgTemporalTrendAdj->ItemIndex) {
       case 0:                           // none
       case 1:                           // non-parametric
         edtLogPerYear->Enabled = false;
         edtLogPerYear->Color = clInactiveBorder;
         break;
       case 2:                             // log linear
         edtLogPerYear->Enabled = true;
         edtLogPerYear->Color = clWindow;
         break;
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("OnTemporalTrendClick()", "TfrmAnalysis");
      throw;
   }
}

//---------------------------------------------------------------------------
// parses up a date string and places it into the given month, day, year
// interace text control (TEdit *).
//---------------------------------------------------------------------------
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
               }
               break;
      case 1 : if (uiYear >= MIN_YEAR && uiYear <= MAX_YEAR)
                 pYear->Text = uiYear;
               break;
    };
  }
}
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::PositiveFloatKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789.\b",Key))
    Key = 0;
}

//---------------------------------------------------------------------------
// main function to save a parameter file AS
//---------------------------------------------------------------------------
void TfrmAnalysis::SaveAs() {
  try {
    if (SaveDialog->Execute()) {
      gsParamFileName = SaveDialog->FileName;
      frmAnalysis->Caption = SaveDialog->FileName;
      WriteSession();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("SaveAs()", "TfrmAnalysis");
    throw;
  }
}

//-----------------------------------------------------------------------------
//  MAIN FUNCTION TO SET TRANSFER TEXT PARAMETERS INTO SESSION OBJ GLOBALS.
//---------------------------------------------------------------------------
void TfrmAnalysis::SaveTextParameters() {
  Caption = gsParamFileName;
  ZdString      sString;

  try {
    //Input File Tab
    gParameters.SetCaseFileName(edtCaseFileName->Text.c_str());
    gParameters.SetControlFileName(edtControlFileName->Text.c_str());
    gParameters.SetPopulationFileName(edtPopFileName->Text.c_str());
    gParameters.SetCoordinatesFileName(edtCoordinateFileName->Text.c_str());
    gParameters.SetSpecialGridFileName(edtGridFileName->Text.c_str(), false, true);
    //Analysis Tab
    sString.printf("%i/%i/%i", atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()), atoi(edtStartDay->Text.c_str()));
    gParameters.SetStudyPeriodStartDate(sString.GetCString());
    sString.printf("%i/%i/%i", atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()));
    gParameters.SetStudyPeriodEndDate(sString.GetCString());
    gParameters.SetNumberMonteCarloReplications(atoi(edtMontCarloReps->Text.c_str()));
    //Scanning Window Tab
    gParameters.SetMaximumGeographicClusterSize(atof(edtMaxClusterSize->Text.c_str()));
    if (rdoPercentageTemproal->Checked)
      gParameters.SetMaximumTemporalClusterSize(atof(edtMaxTemporalClusterSize->Text.c_str()));
    else
      gParameters.SetMaximumTemporalClusterSize(atoi(edtMaxTemporalClusterSize->Text.c_str()));
    gParameters.SetMaximumTemporalClusterSizeType(rdoPercentageTemproal->Checked ? PERCENTAGETYPE : TIMETYPE);
    //Time Parameter Tab
    gParameters.SetTimeIntervalLength(atoi(edtUnitLength->Text.c_str()));
    gParameters.SetTimeTrendAdjustmentPercentage(atof(edtLogPerYear->Text.c_str()));
    //rest time adjustment type if needed - something needs to be worked out so that this isn't needed.
    //Also, this changes settings so that the next time a parameter file is opened, the settings
    //of time trend adjustment are potentially different for model types other than Poisson.
    gParameters.SetTimeTrendAdjustmentType(gParameters.GetProbabiltyModelType() == POISSON ? gParameters.GetTimeTrendAdjustmentType() : NOTADJUSTED);
    sString.printf("%i/%i/%i", atoi(edtProspYear->Text.c_str()), atoi(edtProspMonth->Text.c_str()), atoi(edtProspDay->Text.c_str()));
    gParameters.SetProspectiveStartDate(sString.GetCString());
    //Output File Tab
    gParameters.SetOutputFileName(edtResultFile->Text.c_str());

    gParameters.SetOutputRelativeRisksAscii(chkRelativeRiskEstimatesAreaAscii->Enabled && chkRelativeRiskEstimatesAreaAscii->Checked);

    gParameters.SetOutputClusterLevelDBase(chkClustersInColumnFormatDBase->Checked);
    gParameters.SetOutputAreaSpecificDBase(chkCensusAreasReportedClustersDBase->Checked);
    gParameters.SetOutputRelativeRisksDBase(chkRelativeRiskEstimatesAreaDBase->Enabled && chkRelativeRiskEstimatesAreaDBase->Checked);
    gParameters.SetOutputSimLogLikeliRatiosDBase(chkSimulatedLogLikelihoodRatiosDBase->Checked);
  }  
  catch (ZdException & x) {
    x.AddCallpath("SaveTextParameters()", "TfrmAnalysis");
    throw;
  }
}

/** Sets case filename in interface and parameters class. */
void TfrmAnalysis::SetCaseFile(const char * sCaseFileName) {
  gParameters.SetCaseFileName(sCaseFileName);
  edtCaseFileName->Text = sCaseFileName;
}

/** Sets control filename in interface and parameters class. */
void TfrmAnalysis::SetControlFile(const char * sControlFileName) {
  gParameters.SetControlFileName(sControlFileName);
  edtControlFileName->Text = sControlFileName;
}

/** Sets coordinates filename in interface and parameters class. */
void TfrmAnalysis::SetCoordinateFile(const char * sCoordinateFileName) {
  gParameters.SetCoordinatesFileName(sCoordinateFileName);
  edtCoordinateFileName->Text = sCoordinateFileName;
}

/** Sets population filename in interface and parameters class. */
void TfrmAnalysis::SetPopulationFile(const char * sPopulationFileName) {
  gParameters.SetPopulationFileName(sPopulationFileName);
  edtPopFileName->Text = sPopulationFileName;
}

// Sets caption of radio button that indicates maximum spatial cluster size is
// in distance units.
void TfrmAnalysis::SetSpatialDistanceCaption() {
  try {
    switch (rgCoordinates->ItemIndex)
       {
       case 0  : rdoSpatialDistance->Caption = "Cartesian Units Radius";
                 break;
       case 1  : rdoSpatialDistance->Caption = "Kilometer Radius";
                 break;
       default : ZdException::Generate("Unknown coordinates radio button index: \"%i\".",
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

/** Internal setup */
void TfrmAnalysis::Setup(const char * sParameterFileName) {
  try {
    PageControl1->ActivePage = tbInputFiles;
    if (sParameterFileName) {
      gsParamFileName = sParameterFileName;
      gParameters.Read(sParameterFileName, gNullPrint);
      DefaultHiddenParameters();
    }  
    SetupInterface();
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()", "TfrmAnalysis");
    throw;
  }
}

//---------------------------------------------------------------------------
//  Sets all interface controls using the gpParams session object
//---------------------------------------------------------------------------
void TfrmAnalysis::SetupInterface() {
  Caption = gsParamFileName;

  try {
    // Odd SatScan changed these values... WHY ???
    //gParameters.geAnalysisType --;
    //gParameters.m_nAreas --;

    // THIS "IF" STATEMENT IS HERE JUST TO MATCH THE CODE FOUND IN THE OLD VISUAL C++ INTERFACE....
    if (gParameters.GetAnalysisType() == PURELYSPATIAL) {
      gParameters.SetTimeIntervalUnitsType(YEAR);
      gParameters.SetTimeIntervalLength(1);
    }

    // because precision times has the heirarchal affect on model and analysis type, we need to set these two
    // gui controls before we get to the precision time control setting - AJV 10/23/2002
    rgTypeAnalysis->ItemIndex = (int)gParameters.GetAnalysisType() - 1;
    rgProbability->ItemIndex = gParameters.GetProbabiltyModelType();

    //Input File Tab
    edtCaseFileName->Text = gParameters.GetCaseFileName().c_str();
    edtControlFileName->Text = gParameters.GetControlFileName().c_str();
    rgPrecisionTimes->ItemIndex = gParameters.GetPrecisionOfTimesType();
    edtPopFileName->Text = gParameters.GetPopulationFileName().c_str();
    edtCoordinateFileName->Text = gParameters.GetCoordinatesFileName().c_str();
    edtGridFileName->Text = gParameters.GetSpecialGridFileName().c_str();
    rgCoordinates->ItemIndex = gParameters.GetCoordinatesType();
    //Analysis Tab
    rgScanAreas->ItemIndex = (int)gParameters.GetAreaScanRateType() - 1;
    ParseDate(gParameters.GetStudyPeriodStartDate().c_str(), edtStartYear, edtStartMonth, edtStartDay);
    ParseDate(gParameters.GetStudyPeriodEndDate().c_str(), edtEndYear, edtEndMonth, edtEndDay);
    edtMontCarloReps->Text = gParameters.GetNumReplicationsRequested();
    //Scanning Window Tab
    edtMaxClusterSize->Text = gParameters.GetMaximumGeographicClusterSize();
    chkInclPurTempClust->Checked = gParameters.GetIncludePurelyTemporalClusters();
    edtMaxTemporalClusterSize->Text = gParameters.GetMaximumTemporalClusterSize();
    rdoPercentageTemproal->Checked = gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE;
    rdoTimeTemproal->Checked = gParameters.GetMaximumTemporalClusterSizeType() == TIMETYPE;
    chkIncludePurSpacClust->Checked = gParameters.GetIncludePurelySpatialClusters();
    rdoSpatialPercentage->Checked = gParameters.GetMaxGeographicClusterSizeType() != DISTANCETYPE; // default checked
    rdoSpatialDistance->Checked = gParameters.GetMaxGeographicClusterSizeType() == DISTANCETYPE;
    SetSpatialDistanceCaption();

    //***************** check this code ******************************
    rgClustersToInclude->ItemIndex = (gParameters.GetAliveClustersOnly() ? 1:0);  // IS THIS RETURNING THE RIGHT INDEX OR SHOULD I SWITCH IT AROUND ???
    if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME) { //DISABLE the Include Purely Spacial Clusters option.
      chkIncludePurSpacClust->Checked = false;
      chkIncludePurSpacClust->Enabled = false;
    }
    else
      chkIncludePurSpacClust->Enabled = true;
    //***************** check this code ******************************

    //Time Parameter Tab
    if (gParameters.GetTimeIntervalUnitsType() == 0) gParameters.SetTimeIntervalUnitsType(YEAR);
    if (gParameters.GetTimeIntervalLength() == 0) gParameters.SetTimeIntervalLength(1);
    rbUnitYear->Checked = (gParameters.GetTimeIntervalUnitsType() == YEAR);  // use to be 0
    rbUnitMonths->Checked = (gParameters.GetTimeIntervalUnitsType() == MONTH);  // use to be 1
    rbUnitDay->Checked = (gParameters.GetTimeIntervalUnitsType() == DAY);  // use to be 2
    edtUnitLength->Text = gParameters.GetTimeIntervalLength();
    rgTemporalTrendAdj->ItemIndex = gParameters.GetTimeTrendAdjustmentType();
    edtLogPerYear->Text = gParameters.GetTimeTrendAdjustmentPercentage();
    if (gParameters.GetProspectiveStartDate().length() > 0)
      ParseDate(gParameters.GetProspectiveStartDate().c_str(), edtProspYear, edtProspMonth, edtProspDay);
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
    //now enable or disable controls appropriately
    OnPrecisionTimesClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("SetupInterface()", "TfrmAnalysis");
    throw;
  }
}

//------------------------------------------------------------------------------
// Validates the existence of the input files...  this is more for any
// session file that was read in....  If the Browse buttons are used, then you
// know that the files exist.  But, if a session file was directly read in
// and run, you do not know if the original files still exist etc...
//  THEIR EXISTENCE MUST BE VERIFIED.
//------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
// This function is used right before a job is submitted.  Verifies that
// all the input files exist and can be read.  Also checks each tab to see
// if all settings are in place.
//---------------------------------------------------------------------------
bool TfrmAnalysis::ValidateParams() {
  bool bDataOk;

  try {
    SaveTextParameters();
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

bool TfrmAnalysis::ValidateSpatialClusterSize() {
  double dValue;
  bool   bOkParams=true;

  try {
    if (edtMaxClusterSize->Enabled) {
      if (!edtMaxClusterSize->Text.Length() || atof(edtMaxClusterSize->Text.c_str()) == 0)
        ZdException::GenerateNotification("Please specify a maximum spatial cluster size.","ValidateSpatialClusterSize()");

      dValue = atof(edtMaxClusterSize->Text.c_str());
      if (!(dValue > 0.0 && dValue <= 50.0) && rdoSpatialPercentage->Checked)
        ZdException::GenerateNotification("Please specify valid maximum spatial cluster size between %d - %d.",
                                          "ValidateSpatialClusterSize()", 0, 50);

      gParameters.SetMaximumGeographicClusterSize(atof(edtMaxClusterSize->Text.c_str()));
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateSpatialClusterSize()", "TfrmAnalysis");
    PageControl1->ActivePage = tbScanningWindow;
    edtMaxClusterSize->SetFocus();
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

//---------------------------------------------------------------------------
// Writes the session information to disk
//---------------------------------------------------------------------------
void TfrmAnalysis::WriteSession() {
  try {
    if (! gsParamFileName.IsEmpty()) {
      SaveTextParameters();
      gParameters.Write(gsParamFileName.c_str());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("WriteSession()", "TfrmAnalysis");
    throw;
  }
}


// ==========================================================================
// fastcalls

void __fastcall TfrmAnalysis::edtCaseFileNameChange(TObject *Sender) {
  edtCaseFileName->Hint = edtCaseFileName->Text;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtControlFileNameChange(TObject *Sender){
  edtControlFileName->Hint = edtControlFileName->Text;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtPopFileNameChange(TObject *Sender){
  edtPopFileName->Hint = edtPopFileName->Text;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtCoordinateFileNameChange(TObject *Sender){
  edtCoordinateFileName->Hint = edtCoordinateFileName->Text;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtGridFileNameChange(TObject *Sender) {
  edtGridFileName->Hint = edtGridFileName->Text;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtResultFileChange(TObject *Sender){
  edtResultFile->Hint = edtResultFile->Text;
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::cboCriteriaSecClustersChange(TObject *Sender){
   gParameters.SetCriteriaForReportingSecondaryClusters((CriteriaSecondaryClustersType)cboCriteriaSecClusters->ItemIndex);
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rdoSpatialPercentageClick(TObject *Sender) {
   gParameters.SetMaximumSpacialClusterSizeType(PERCENTAGEOFMEASURETYPE);
}

//------------------------------------------------------------------------------
// Specific Day unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitDayClick(TObject *Sender) {
    gParameters.SetTimeIntervalUnitsType(DAY); // use to be 2
    rdoTimeTemproal->Caption = "Days";
}
//------------------------------------------------------------------------------
// Specific Months unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitMonthsClick(TObject *Sender) {
    gParameters.SetTimeIntervalUnitsType(MONTH); // use to be 1
    rdoTimeTemproal->Caption = "Months";
}
//------------------------------------------------------------------------------
// Specific Year unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitYearClick(TObject *Sender) {
    gParameters.SetTimeIntervalUnitsType(YEAR); // use to be 0
    rdoTimeTemproal->Caption = "Years";
}
//------------------------------------------------------------------------------
//  Control to include "Alive" clusters
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgClustersToIncludeClick(TObject *Sender) {
    gParameters.SetAliveClustersOnly(rgClustersToInclude->ItemIndex == 0 ? 0:1);
}
//------------------------------------------------------------------------------
// If the types of coordinates are changed, then various interface options
// need to be updated
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgCoordinatesClick(TObject *Sender) {
  try {
    gParameters.SetCoordinatesType((CoordinatesType)rgCoordinates->ItemIndex);
    SetSpatialDistanceCaption();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgCoordinatesClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rdoSpatialDistanceClick(TObject *Sender){
   gParameters.SetMaximumSpacialClusterSizeType(DISTANCETYPE);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgPrecisionTimesClick(TObject *Sender) {
  try {
     OnPrecisionTimesClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgPrecisionTimesClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

void __fastcall TfrmAnalysis::rgProbabilityClick(TObject *Sender) {
  try {
     OnProbabilityModelClick();
  }
  catch ( ZdException & x ) {
    x.AddCallpath( "rgProbabilityClick()", "TfrmAnalysis" );
    DisplayBasisException( this, x );
  }
}

void __fastcall TfrmAnalysis::rgTemporalTrendAdjClick(TObject *Sender) {
  try {
     OnTemporalTrendClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgTemporalTrendAdjClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
   }
}

//------------------------------------------------------------------------------
// Scan Areas is changed.  Need to perform a "dataexchange" to see if any
// other tab controls need to be enabled or disabled
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgScanAreasClick(TObject *Sender) {
  try {
    gParameters.SetAreaRateType((AreaRateType)(rgScanAreas->ItemIndex + 1));
  }
  catch (ZdException & x) {
    x.AddCallpath("rgScanAreasClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

void __fastcall TfrmAnalysis::rgTypeAnalysisClick(TObject *Sender) {
  try {
     OnAnalysisTypeClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgTypeAnalysisClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

void __fastcall TfrmAnalysis::FormActivate(TObject *Sender)
{
   EnableActions(true);
}
//---------------------------------------------------------------------------

