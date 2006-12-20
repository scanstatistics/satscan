/** \file stsFrmAnalysisParameters.cpp
 * \brief  class TfrmAnalysis
 *
 * \author
 *
 * <b>Code Review History</b> (list: Type, Reviewer(s), Date)
 * \li 
 *
 * $Revision:
 *
 * $Date:
 */
//ClassDesc Begin TfrmAnalysis
// This class contains all the main interface controls and relationships.
// Since the main session interface is a tab dialog, decided to keep
// everything in one class and one cpp.
//ClassDesc End TfrmAnalysis
//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
#include "stsFrmAdvancedParameters.h"
#include "ParameterFileAccess.h"
#include "RandomNumberGenerator.h"
#include "Randomizer.h"
#include "Toolkit.h"
#pragma package(smart_init)
#pragma resource "*.dfm"

/** constructor */
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
//---------------------------------------------------------------------------
/** destructor */
__fastcall TfrmAnalysis::~TfrmAnalysis() {
  try {
    delete gpfrmAdvancedParameters;
  }
  catch (...){}   
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnAdvancedParametersClick(TObject *Sender)
{
  try {
      ShowAdvancedFeaturesDialog();
  }
  catch (ZdException &x) {
    x.AddCallpath("btnAdvancedParametersClick()","TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
/** Button click event for case file import
    Shows open file dialog to select file to import as a case file; then
    launches the import wizard to guide the user through the import process.*/
void __fastcall TfrmAnalysis::btnCaseImportClick(TObject *Sender) {
  InputFileType eType = Case;

  try {
    OpenDialog1->FileName =  "";
    OpenDialog1->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Case files (*.cas)|*.cas|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Source Case File";
    if (OpenDialog1->Execute()) {
       LaunchImporter(OpenDialog1->FileName.c_str(), eType);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnCaseImportClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
/** Button click event for control file import
    Shows open file dialog to select file to import as a control file; then
    launches the import wizard to guide the user through the import process.*/
void __fastcall TfrmAnalysis::btnControlImportClick(TObject *Sender) {
  InputFileType eType = Control;

  try {
    OpenDialog1->FileName =  "";
    OpenDialog1->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Control files (*.ctl)|*.ctl|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Source Control File";
    if (OpenDialog1->Execute()) {
       LaunchImporter(OpenDialog1->FileName.c_str(), eType);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnControlImportClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
/** Button click event for coordinate file import
    Shows open file dialog to select file to import as a coordinate file; then
    launches the import wizard to guide the user through the import process.*/
void __fastcall TfrmAnalysis::btnCoordImportClick(TObject *Sender) {
  InputFileType eType = Coordinates;

  try {
    OpenDialog1->FileName =  "";
    OpenDialog1->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Coordinates files (*.geo)|*.geo|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Source Coordinates File";
    if (OpenDialog1->Execute()) {
       LaunchImporter(OpenDialog1->FileName.c_str(), eType);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnCoordImportClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
/** button click event for special grid file browse
    - shows open dialog and sets appropriate special grid file interface controls */
void __fastcall TfrmAnalysis::btnGridBrowseClick(TObject *Sender) {
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.grd";
    OpenDialog1->Filter = "Grid files (*.grd)|*.grd|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->FilterIndex = 0;
    OpenDialog1->Title = "Select Grid File";
    if (OpenDialog1->Execute())
      SetSpecialGridFile(OpenDialog1->FileName.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("btnGridBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
/** Button click event for grid file import
    Shows open file dialog to select file to import as a grid file; then
    launches the import wizard to guide the user through the import process.*/
void __fastcall TfrmAnalysis::btnGridImportClick(TObject *Sender) {
  InputFileType eType = SpecialGrid;

  try {
    OpenDialog1->FileName =  "";
    OpenDialog1->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Grid files (*.grd)|*.grd|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Source Grid File";
    if (OpenDialog1->Execute()) {
       LaunchImporter(OpenDialog1->FileName.c_str(), eType);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnGridImportClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
/** Button click event for pop file import
    Shows open file dialog to select file to import as a pop file; then
    launches the import wizard to guide the user through the import process.*/
void __fastcall TfrmAnalysis::btnPopImportClick(TObject *Sender) {
  InputFileType eType = Population;

  try {
    OpenDialog1->FileName =  "";
    OpenDialog1->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Population files (*.pop)|*.pop|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Source Population File";
    if (OpenDialog1->Execute()) {
       LaunchImporter(OpenDialog1->FileName.c_str(), eType);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnPopImportClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
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

/** Calculates number of time aggregation units in study period. */
double TfrmAnalysis::CalculateTimeAggregationUnitsInStudyPeriod() const {
  ZdDate        StartDate, EndDate;
  double        dStudyPeriodLengthInUnits;

  dStudyPeriodLengthInUnits = ceil(CalculateNumberOfTimeIntervals(GetStudyPeriodStartDate(StartDate).GetJulianDayFromCalendarStart() + ceil(1721424.5),
                                                                  GetStudyPeriodEndDate(EndDate).GetJulianDayFromCalendarStart() + ceil(1721424.5),
                                                                  GetTimeAggregationControlType(), 1));
  return dStudyPeriodLengthInUnits;
}

//---------------------------------------------------------------------------
/** Validates time interval length is not less than zero. */
void TfrmAnalysis::CheckTimeAggregationLength() {
  std::string   sPrecisionString;
  double        dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

  try {
    //validate that the time aggregation length agrees with study period and maximum temporal cluster size
    if (GetAnalysisControlType() != PURELYSPATIAL) {
      GetDatePrecisionAsString(GetTimeAggregationControlType(), sPrecisionString, false, false);
      //report error if control is empty or specified time aggregation length is less than one.
      if (edtTimeAggregationLength->Text.IsEmpty() || atoi(edtTimeAggregationLength->Text.c_str()) < 1)
        ZdException::GenerateNotification("Please specify a time aggregation length greater than zero.","CheckTimeAggregationLength()");

      dStudyPeriodLengthInUnits = CalculateTimeAggregationUnitsInStudyPeriod();
      if (dStudyPeriodLengthInUnits < edtTimeAggregationLength->Text.ToDouble())
        ZdException::GenerateNotification("A time aggregation of %d %s%s is greater than the %d %s study period.\n",
                                          "CheckTimeAggregationLength()", edtTimeAggregationLength->Text.ToInt(),
                                          sPrecisionString.c_str(),
                                          (edtTimeAggregationLength->Text.ToInt() == 1 ? "" : "s"),
                                          static_cast<int>(dStudyPeriodLengthInUnits),
                                          sPrecisionString.c_str());
      if (ceil(dStudyPeriodLengthInUnits/edtTimeAggregationLength->Text.ToDouble()) <= 1)
        ZdException::GenerateNotification("A time aggregation of %d %s%s with a %d %s study period results in only\n"
                                          "one time period to analyze. Temporal and space-time analyses can not be performed\n"
                                          "on less than two time periods.\n",
                                          "CheckTimeAggregationLength()",
                                          edtTimeAggregationLength->Text.ToInt(),
                                          sPrecisionString.c_str(),
                                          (edtTimeAggregationLength->Text.ToInt() == 1 ? "" : "s"),
                                          static_cast<int>(dStudyPeriodLengthInUnits),
                                          sPrecisionString.c_str());
      if (gpfrmAdvancedParameters->GetMaxTemporalClusterSizeControlType() == PERCENTAGETYPE)
        dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * gpfrmAdvancedParameters->GetMaxTemporalClusterSizeFromControl()/100.0);
      else if (gpfrmAdvancedParameters->GetMaxTemporalClusterSizeControlType() == TIMETYPE)
        dMaxTemporalLengthInUnits = gpfrmAdvancedParameters->GetMaxTemporalClusterSizeFromControl();

      //validate the time aggregation agrees with maximum temporal cluster size
      if (static_cast<int>(floor(dMaxTemporalLengthInUnits /edtTimeAggregationLength->Text.ToDouble())) == 0) {
        if (gpfrmAdvancedParameters->GetMaxTemporalClusterSizeControlType() == TIMETYPE)
          ZdException::GenerateNotification("The time aggregation of %d %s%s is greater than the maximum temporal "
                                            "cluster size of %g %s%s.\nPlease review settings.",
                                            "CheckTimeAggregationLength()", edtTimeAggregationLength->Text.ToInt(),
                                            sPrecisionString.c_str(),
                                            (edtTimeAggregationLength->Text.ToInt() == 1 ? "" : "s"),
                                            gpfrmAdvancedParameters->GetMaxTemporalClusterSizeFromControl(),
                                            sPrecisionString.c_str(),
                                            (gpfrmAdvancedParameters->GetMaxTemporalClusterSizeFromControl() == 1 ? "" : "s"));
        else if (gpfrmAdvancedParameters->GetMaxTemporalClusterSizeControlType() == PERCENTAGETYPE)
          ZdException::GenerateNotification("With the maximum temporal cluster size as %g%% of a %d %s study period,\n"
                                            "the time aggregation as %d %s%s is greater than the resulting maximum\n"
                                            "temporal cluster size of %g %s%s.\nPlease review settings.",
                                            "CheckTimeAggregationLength()",
                                            gpfrmAdvancedParameters->GetMaxTemporalClusterSizeFromControl(),
                                            static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.c_str(),
                                            edtTimeAggregationLength->Text.ToInt(),
                                            sPrecisionString.c_str(),
                                            (edtTimeAggregationLength->Text.ToInt() == 1 ? "" : "s"),
                                            dMaxTemporalLengthInUnits,
                                            sPrecisionString.c_str(),
                                            (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
      }
    }
  }
  catch (ZdException &x) {
    PageControl1->ActivePage = tbAnalysis;
    edtTimeAggregationLength->SetFocus();
    x.AddCallpath("CheckTimeAggregationLength()","TfrmAnalysis");
    throw;
  }
}
//---------------------------------------------------------------------------
/** Checks all the parameters on the 'Analysis' tab. Returns whether tab is valid. */
void TfrmAnalysis::CheckAnalysisParams() {
  try {
    CheckReplicas();
    CheckTimeAggregationLength();
  }
  catch (ZdException &x) {
    x.AddCallpath("CheckAnalysisParams()","TfrmAnalysis");
    throw;
  }
}
//---------------------------------------------------------------------------
/** checks that date, comprised of passed TEdit controls is not missing text
    -- focuses control and throw exception */
void TfrmAnalysis::CheckDate(const char * sDateTitle, TEdit& Year, TEdit& Month, TEdit& Day,  TTabSheet& Container) {
  if (Year.Text.IsEmpty()) {
    PageControl1->ActivePage = &Container;
    Year.SetFocus();
    ZdException::GenerateNotification("Please specify a year for the %s.","CheckProspDateRange()", sDateTitle);
  }
  if (Month.Text.IsEmpty()) {
    PageControl1->ActivePage = &Container;
    Month.SetFocus();
    ZdException::GenerateNotification("Please specify a month for the %s.","CheckProspDateRange()", sDateTitle);
  }
  if (Day.Text.IsEmpty()) {
    PageControl1->ActivePage = &Container;
    Day.SetFocus();
    ZdException::GenerateNotification("Please specify a day for the %s.","CheckProspDateRange()", sDateTitle);
  }
}
//---------------------------------------------------------------------------
/** Verifies all parameters on the 'Output Files' tab. Returns whether tab is valid.*/
void TfrmAnalysis::CheckOutputParams() {
  try {
    if (edtResultFile->Text.Length() == 0)
      ZdException::GenerateNotification("Please specify a results file.", "CheckOutputParams()");
    if (!ValidateFileAccess(edtResultFile->Text.c_str(), true))
      ZdException::GenerateNotification("Results file could not be opened for writing.\n"
                                        "Please confirm that the path and/or file name\n"
                                        "are valid and that you have permissions to write\nto this directory and file.", "CheckOutputParams()");
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckOutputParams", "TfrmAnalysis");
    PageControl1->ActivePage = tbOutputFiles;
    edtResultFile->SetFocus();
    throw;
  }
}
//---------------------------------------------------------------------------
/** Checks Monte Carlo replications */
void TfrmAnalysis::CheckReplicas() {
  double        dNumReplications, dMaxReplications;

  try {
    if (edtMontCarloReps->Text.IsEmpty())
      ZdException::GenerateNotification("Please specify a number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.", "CheckReplicas()");
    //validate that specified value is within allowable range - coincidentally this
    //check also checks that value does not exceed numeric limits w/32-bit system for unsigned int
    dNumReplications = edtMontCarloReps->Text.ToDouble();
    dMaxReplications = (double)RandomNumberGenerator::glM - (double)RandomNumberGenerator::glDefaultSeed - (double)gpfrmAdvancedParameters->GetNumAdditionalDataSets() * AbstractRandomizer::glDataSetSeedOffSet;
    dMaxReplications = (floor((dMaxReplications)/1000) - 1)  * 1000 + 999;
    if (dNumReplications > dMaxReplications)
      ZdException::GenerateNotification("Number of Monte Carlo replications can not exceed %.0lf.", "CheckReplicas()", dMaxReplications);
    if (!((dNumReplications == 0 || dNumReplications == 9 || dNumReplications == 19 || fmod(dNumReplications+1, 1000) == 0.0)))
      ZdException::GenerateNotification("Invalid number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.", "CheckReplicas()");
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckReplicas", "TfrmAnalysis");
    PageControl1->ActivePage = tbAnalysis;
    edtMontCarloReps->SetFocus();
    throw;
  }
}
//---------------------------------------------------------------------------
/** Checks the relationship between a start date and end date.
    Display message box regarding errors when appropriate. Return whether relationship is valid. */
void TfrmAnalysis::CheckStudyPeriodDatesRange() {
  ZdDate        StartDate, EndDate;

  try {
    CheckDate("study period start date", *edtStudyPeriodStartDateYear, *edtStudyPeriodStartDateMonth,
              *edtStudyPeriodStartDateDay,  *tbInputFiles);
    CheckDate("study period end date", *edtStudyPeriodEndDateYear, *edtStudyPeriodEndDateMonth,
              *edtStudyPeriodEndDateDay,  *tbInputFiles);

    GetStudyPeriodStartDate(StartDate);
    GetStudyPeriodEndDate(EndDate);

    //check that start date is before end date
    if (StartDate > EndDate)
      ZdException::GenerateNotification("The study period start date can not be greater than the end date.",
                                        "CheckStudyPeriodDatesRange()");
  }
  catch (ZdException &x) {
    x.AddCallpath("CheckStudyPeriodDatesRange()","TfrmAnalysis");
    throw;
  }
}
//---------------------------------------------------------------------------
/** Resets parameters that are not present in interface to default value.
    Hidden features are to be used soley in command line version at this time. */
void TfrmAnalysis::DefaultHiddenParameters() {
  gParameters.SetPowerCalculation(false);
  //non-parametric removed from interface, replaced with time stratified
  if (gParameters.GetTimeTrendAdjustmentType() == NONPARAMETRIC)
    gParameters.SetTimeTrendAdjustmentType(STRATIFIED_RANDOMIZATION);
  gParameters.SetTimeTrendConvergence(.0000001); //default value in CParameters
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
  //if still ALIVECLUSTERS, default to ALLCLUSTERS
  if (gParameters.GetIncludeClustersType() == ALIVECLUSTERS)
    gParameters.SetIncludeClustersType(ALLCLUSTERS);
  //prevent stratified temporal and spatial adjustments from being set as same time
  if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION &&
      gParameters.GetSpatialAdjustmentType() == SPATIALLY_STRATIFIED_RANDOMIZATION)
    gParameters.SetSpatialAdjustmentType(NO_SPATIAL_ADJUSTMENT);
  gParameters.SetExecutionType(AUTOMATIC);
  //before version 6, critical values were always reported
  if (gParameters.GetCreationVersion().iMajor < 6)
    gParameters.SetReportCriticalValues(true);
  //If parameter file was created with version 4 of SaTScan, use time interval
  //units as specifier for date precision. This was the behavior in v4 but it
  //was decided to revert to time precision units. Note that for a purely spatial
  //analysis, we have no way of knowing what the time precision should be; settings
  //to YEAR is safe since it is permittable to have more precise dates.
  if (gParameters.GetCreationVersion().iMajor == 4)
    gParameters.SetPrecisionOfTimesType(gParameters.GetAnalysisType() == PURELYSPATIAL ? YEAR : gParameters.GetTimeAggregationUnitsType());
  //Default elliptic shapes and rotations.  
  gParameters.AddEllipsoidShape(1.5, true);
  gParameters.AddEllipsoidRotations(4, true);
  gParameters.AddEllipsoidShape(2, false);
  gParameters.AddEllipsoidRotations(6, false);
  gParameters.AddEllipsoidShape(3, false);
  gParameters.AddEllipsoidRotations(9, false);
  gParameters.AddEllipsoidShape(4, false);
  gParameters.AddEllipsoidRotations(12, false);
  gParameters.AddEllipsoidShape(5, false);
  gParameters.AddEllipsoidRotations(15, false);
}
//---------------------------------------------------------------------------
/** event triggered when case file edit control text changes */
void __fastcall TfrmAnalysis::edtCaseFileNameChange(TObject *Sender) {
  edtCaseFileName->Hint = edtCaseFileName->Text;
}

//---------------------------------------------------------------------------
/** event triggered when control file edit control text changes */
void __fastcall TfrmAnalysis::edtControlFileNameChange(TObject *Sender){
  edtControlFileName->Hint = edtControlFileName->Text;
}

//---------------------------------------------------------------------------
/** event triggered when coordinates file edit control text changes */
void __fastcall TfrmAnalysis::edtCoordinateFileNameChange(TObject *Sender){
  edtCoordinateFileName->Hint = edtCoordinateFileName->Text;
}

//---------------------------------------------------------------------------
/** event triggered when special grid file edit control text changes */
void __fastcall TfrmAnalysis::edtGridFileNameChange(TObject *Sender) {
  edtGridFileName->Hint = edtGridFileName->Text;
}
//---------------------------------------------------------------------------
/** event triggered when Monte Carlo replications control is exited. */
void __fastcall TfrmAnalysis::edtMontCarloRepsExit(TObject *Sender) {
  if (edtMontCarloReps->Text.IsEmpty()) edtMontCarloReps->Text = 999;
}

//---------------------------------------------------------------------------
/** event triggered when population file edit control text changes */
void __fastcall TfrmAnalysis::edtPopFileNameChange(TObject *Sender){
  edtPopFileName->Hint = edtPopFileName->Text;
}
//---------------------------------------------------------------------------
/** event triggered when results file edit control text changes */
void __fastcall TfrmAnalysis::edtResultFileChange(TObject *Sender){
  edtResultFile->Hint = edtResultFile->Text;
}

//---------------------------------------------------------------------------
/** event triggered when year control, of study period end date, is exited. */
void __fastcall TfrmAnalysis::edtStudyPeriodEndDateExit(TObject *Sender) {
  ValidateDate(*edtStudyPeriodEndDateYear, *edtStudyPeriodEndDateMonth, *edtStudyPeriodEndDateDay);
  //store value in control's Tag property
  StoreEditText(*edtStudyPeriodEndDateMonth, *edtStudyPeriodEndDateDay);
}

//---------------------------------------------------------------------------
/** event triggered when year control, of study period start date, is exited. */
void __fastcall TfrmAnalysis::edtStudyPeriodStartDateExit(TObject *Sender) {
  ValidateDate(*edtStudyPeriodStartDateYear, *edtStudyPeriodStartDateMonth, *edtStudyPeriodStartDateDay);
  //store value in control's Tag property
  StoreEditText(*edtStudyPeriodStartDateMonth, *edtStudyPeriodStartDateDay);
}

//---------------------------------------------------------------------------
/** event triggered when month control, of prospective start date, is exited. */
void __fastcall TfrmAnalysis::edtTimeAggregationLengthExit(TObject *Sender) {
  if (edtTimeAggregationLength->Text.IsEmpty() || atoi(edtTimeAggregationLength->Text.c_str()) < 1)
    edtTimeAggregationLength->Text = 1;
}

//---------------------------------------------------------------------------
/** enables/disables the appropraite buttons and controls based on their category type */
void TfrmAnalysis::EnableActions(bool bEnable) {
   for(int i = 0; i < gpList->ActionCount; ++i) {
      TAction* pAction = dynamic_cast<TAction*>(gpList->Actions[i]);
      if (pAction) {
         if(pAction->Category == CATEGORY_ALL || pAction->Category == CATEGORY_ANALYSIS)
             pAction->Enabled = bEnable;
         else
             pAction->Enabled = !bEnable;
      }
   }
}

//---------------------------------------------------------------------------
/** enables controls of 'additional optional output file' radio group */
void TfrmAnalysis::EnableAdditionalOutFilesOptionsGroup(bool bRelativeRisks) {
  chkRelativeRiskEstimatesAreaAscii->Enabled = bRelativeRisks;
  chkRelativeRiskEstimatesAreaDBase->Enabled = bRelativeRisks;
  lblRelativeRiskEstimatesArea->Enabled = bRelativeRisks;
}
//---------------------------------------------------------------------------
/** enables correct advanced settings button on Analysis and Output tabs */
void TfrmAnalysis::EnableAdvancedButtons() {
  // Input tab Advanced button
  if (!gpfrmAdvancedParameters->GetDefaultsSetForInputOptions())
    btnAdvancedInput->Font->Style = TFontStyles() << fsBold;
  else
    btnAdvancedInput->Font->Style = TFontStyles();
  // Analysis tab Advanced button
  if (!gpfrmAdvancedParameters->GetDefaultsSetForAnalysisOptions())
    btnAdvancedAnalysis->Font->Style = TFontStyles() << fsBold;
  else
    btnAdvancedAnalysis->Font->Style = TFontStyles();
  // Output tab Advanced button
  if (!gpfrmAdvancedParameters->GetDefaultsSetForOutputOptions())
    btnAdvancedOutput->Font->Style = TFontStyles() << fsBold;
  else
    btnAdvancedOutput->Font->Style = TFontStyles();
}
//---------------------------------------------------------------------------
/** enables analysis control based upon the setting in probability model control */
void TfrmAnalysis::EnableAnalysisControlForModelType() {
  DatePrecisionType     eDatePrecisionType(GetPrecisionOfTimesControlType());

  try {
    switch (GetModelControlType()) {
      case POISSON   		:
      case BERNOULLI            :
      case ORDINAL              :
      case NORMAL               :
      case EXPONENTIAL          : rdoRetrospectivePurelySpatial->Enabled = true;
                                  rdoRetrospectivePurelyTemporal->Enabled = eDatePrecisionType != NONE;
                                  rdoRetrospectiveSpaceTime->Enabled = eDatePrecisionType != NONE;
                                  rdoProspectivePurelyTemporal->Enabled = eDatePrecisionType != NONE;
                                  rdoProspectiveSpaceTime->Enabled = eDatePrecisionType != NONE;
                                  rdoSVTT->Enabled = eDatePrecisionType != NONE;
                                  stSVTT->Enabled = rdoSVTT->Enabled;
                                  break;
      case SPACETIMEPERMUTATION : rdoRetrospectivePurelySpatial->Enabled = false;
                                  rdoRetrospectivePurelyTemporal->Enabled = false;
                                  rdoRetrospectiveSpaceTime->Enabled = true;
                                  rdoProspectivePurelyTemporal->Enabled = false;
                                  rdoSVTT->Enabled = false;
                                  stSVTT->Enabled = rdoSVTT->Enabled;
                                  rdoProspectiveSpaceTime->Enabled = true;
                                  if (!rdoRetrospectiveSpaceTime->Checked && !rdoProspectiveSpaceTime->Checked)
                                    rdoRetrospectiveSpaceTime->Checked = true;
                                  break;
      default : ZdGenerateException("Unknown probability model '%d'.", "EnableAnalysisControlForModelType()", GetModelControlType());
    }
    EnableSettingsForAnalysisModelCombination();
  }
  catch (ZdException &x) {
    x.AddCallpath("EnableAnalysisControlForModelType()","TfrmAnalysis");
    throw;
  }
}

//---------------------------------------------------------------------------
/** enabled study period and prospective date precision based on time interval unit */
void TfrmAnalysis::EnableDatesByTimePrecisionUnits() {
  switch (GetPrecisionOfTimesControlType()) {
    case NONE   :
    case DAY    : EnableStudyPeriodDates(true, true, true); break;
    case YEAR   : EnableStudyPeriodDates(true, false, false);
                  break;
    case MONTH  : EnableStudyPeriodDates(true, true, false);
                  break;
    default     :
      ZdGenerateException("Time precion type unknown '%d'.","EnableDatesByTimePrecisionUnits()", GetPrecisionOfTimesControlType());
  };
  gpfrmAdvancedParameters->EnableDatesByTimePrecisionUnits();
}

//---------------------------------------------------------------------------
/** enables probability model control based upon the setting in analysis control */
void TfrmAnalysis::EnableModelControlForAnalysisType() {
  try {
    switch (GetAnalysisControlType()) {
      case PURELYSPATIAL             :
      case PURELYTEMPORAL            :
      case PROSPECTIVEPURELYTEMPORAL : rdoBernoulliModel->Enabled = true;
                                       rdoSpaceTimePermutationModel->Enabled = true;
                                       rdoOrdinalModel->Enabled = true;
                                       rdoExponentialModel->Enabled = true;
                                       rdoNormalModel->Enabled = true;
                                       rdoSpaceTimePermutationModel->Enabled = false;
                                       if (rdoSpaceTimePermutationModel->Checked)
                                          rdoPoissonModel->Checked = true;
                                       break;
      case SPACETIME                 :
      case PROSPECTIVESPACETIME      : rdoBernoulliModel->Enabled = true;
                                       rdoSpaceTimePermutationModel->Enabled = true;
                                       rdoOrdinalModel->Enabled = true;
                                       rdoExponentialModel->Enabled = true;
                                       rdoNormalModel->Enabled = true;
                                       rdoSpaceTimePermutationModel->Enabled = true;
                                       break;
      case SPATIALVARTEMPTREND       : rdoBernoulliModel->Enabled = false;
                                       rdoSpaceTimePermutationModel->Enabled = false;
                                       rdoOrdinalModel->Enabled = false;
                                       rdoExponentialModel->Enabled = false;
                                       rdoNormalModel->Enabled = false;
                                       if (!rdoPoissonModel->Checked) rdoPoissonModel->Checked = true;
                                       break;
      default : ZdGenerateException("Unknown analysis type '%d'.", "EnableModelControlForAnalysisType()", GetAnalysisControlType());
    }
    EnableSettingsForAnalysisModelCombination();
    SetAreaScanRateControlText(GetModelControlType());
  }
  catch (ZdException &x) {
    x.AddCallpath("EnableModelControlForAnalysisType()","TfrmAnalysis");
    throw;
  }
}
//---------------------------------------------------------------------------
void TfrmAnalysis::EnableSettingsForAnalysisModelCombination() {
  try {
    EnableDatesByTimePrecisionUnits();
    EnableTimeAggregationGroup(GetAnalysisControlType() != PURELYSPATIAL);
    EnableAdditionalOutFilesOptionsGroup(GetModelControlType() != SPACETIMEPERMUTATION &&
                                         GetAnalysisControlType() != PURELYTEMPORAL &&
                                         GetAnalysisControlType() != PROSPECTIVEPURELYTEMPORAL);
    gpfrmAdvancedParameters->EnableSettingsForAnalysisModelCombination();
  }
  catch (ZdException &x) {
    x.AddCallpath("EnableSettingsForAnalysisModelCombination()","TfrmAnalysis");
    throw;
  }
}
//---------------------------------------------------------------------------
/** enables or disables the study period group controls */
void TfrmAnalysis::EnableStudyPeriodDates(bool bYear, bool bMonth, bool bDay) {
   //enable study period year controls
   edtStudyPeriodStartDateYear->Enabled = bYear;
   edtStudyPeriodStartDateYear->Color = bYear ? clWindow : clInactiveBorder;
   edtStudyPeriodEndDateYear->Enabled = bYear;
   edtStudyPeriodEndDateYear->Color = bYear ? clWindow : clInactiveBorder;
   //store values and restore values or set as default values
   SetMonthEditText(*edtStudyPeriodStartDateMonth, bMonth, 1);
   SetDayEditText(*edtStudyPeriodStartDateDay, bDay, 1);
   SetMonthEditText(*edtStudyPeriodEndDateMonth, bMonth, 12);
   SetDayEditText(*edtStudyPeriodEndDateDay, bDay, DaysThisMonth(StrToInt(edtStudyPeriodEndDateYear->Text),
                                                                 StrToInt(edtStudyPeriodEndDateMonth->Text)));
   //enable study period month controls
   edtStudyPeriodStartDateMonth->Enabled = bMonth;
   edtStudyPeriodStartDateMonth->Color = bMonth ? clWindow : clInactiveBorder;
   edtStudyPeriodEndDateMonth->Enabled = bMonth;
   edtStudyPeriodEndDateMonth->Color = bMonth ? clWindow : clInactiveBorder;
   //enable study period day controls
   edtStudyPeriodStartDateDay->Enabled = bDay;
   edtStudyPeriodStartDateDay->Color = bDay ? clWindow : clInactiveBorder;
   edtStudyPeriodEndDateDay->Enabled = bDay;
   edtStudyPeriodEndDateDay->Color = bDay ? clWindow : clInactiveBorder;
}

//---------------------------------------------------------------------------
/** enables or disables the time aggregation group control */
void TfrmAnalysis::EnableTimeAggregationGroup(bool bEnable) {
   DatePrecisionType ePrecisionType = GetPrecisionOfTimesControlType();

   rgpTimeAggregationUnits->Enabled = bEnable && ePrecisionType != NONE;
   lblTimeAggregationUnits->Enabled =  bEnable && ePrecisionType != NONE;

   edtTimeAggregationLength->Enabled = bEnable && ePrecisionType != NONE;
   edtTimeAggregationLength->Color = edtTimeAggregationLength->Enabled ? clWindow : clInactiveBorder;
   lblTimeAggregationLength->Enabled = edtTimeAggregationLength->Enabled;
   stUnitText->Enabled = edtTimeAggregationLength->Enabled;
   
   rdoTimeAggregationYear->Enabled =  bEnable && ePrecisionType != NONE;
   rdoTimeAggregationMonths->Enabled = bEnable && ePrecisionType != NONE && ePrecisionType != YEAR;
   if (rgpTimeAggregationUnits->Enabled && rdoTimeAggregationMonths->Checked && !rdoTimeAggregationMonths->Enabled)
     rdoTimeAggregationYear->Checked = true;
   rdoTimeAggregationDay->Enabled = bEnable && ePrecisionType == DAY;
   if (rgpTimeAggregationUnits->Enabled && rdoTimeAggregationDay->Checked && !rdoTimeAggregationDay->Enabled) {
     if (rdoTimeAggregationMonths->Enabled)
       rdoTimeAggregationMonths->Checked = true;
     else
       rdoTimeAggregationYear->Checked = true;
   }
}

//---------------------------------------------------------------------------
/** event triggered when key pressed for controls that can contain real numbers. */
void __fastcall TfrmAnalysis::FloatKeyPress(TObject *Sender, char &Key) {
  TEdit * pEdit;
  //permit only characters that could comprise a floating point variable
  if (!strchr("-0123456789.\b", Key))
    Key = 0;
  //permit only one decimal place or negative character
  else if ((Key == '.' || Key == '-') && (pEdit = dynamic_cast<TEdit*>(Sender)) != 0 && strchr(pEdit->Text.c_str(), Key))
    Key = 0;
}

//---------------------------------------------------------------------------
/** event triggered when form is activated */
void __fastcall TfrmAnalysis::FormActivate(TObject *Sender) {
   EnableActions(true);
}

//---------------------------------------------------------------------------
/** form close event - added parameter filename to history list */
void __fastcall TfrmAnalysis::FormClose(TObject *Sender , TCloseAction &Action) {
  try {
    if ((Action = (gbPromptOnExist ? (QueryWindowCanClose() ? caFree : caNone) : caFree)) == caFree) {
      AppToolkit::getToolkit().AddParameterToHistory(gParameters.GetSourceFileName().c_str());
      frmMainForm->RefreshOpenList();
    }
  }
  catch(...){}
}

/** Determines whether window can be closed by comparing parameter settings contained
    in window verse intial parameter settings. */
bool TfrmAnalysis::QueryWindowCanClose() {
  bool  bReturn=true;

    SaveParameterSettings();
    if (gParameters != gInitialParameters) {
      BringToFront();
      switch (Application->MessageBox("Parameter settings have changed. Do you want to save?", "Save?", MB_YESNOCANCEL)) {
        case IDYES    : if (WriteSession())
                          gbPromptOnExist = false;
                        else
                          bReturn = false;
                        break;
        case IDCANCEL : bReturn = false;
                        break;
        case IDNO     :                 
        default       : gbPromptOnExist = false;
                        break;
      }
    }
    return bReturn;
}

//---------------------------------------------------------------------------
/** returns analysis type for analysis control group */
AnalysisType TfrmAnalysis::GetAnalysisControlType() const {
  AnalysisType eReturn;

  if (rdoRetrospectivePurelySpatial->Checked)
    eReturn = PURELYSPATIAL;
  else if (rdoRetrospectivePurelyTemporal->Checked)
    eReturn = PURELYTEMPORAL;
  else if (rdoRetrospectiveSpaceTime->Checked)
    eReturn = SPACETIME;
  else if (rdoProspectivePurelyTemporal->Checked)
    eReturn = PROSPECTIVEPURELYTEMPORAL;
  else if (rdoProspectiveSpaceTime->Checked)
    eReturn = PROSPECTIVESPACETIME;
  else if (rdoSVTT->Checked)
    eReturn = SPATIALVARTEMPTREND;
  else
    ZdGenerateException("Analysis type not selected.","GetAnalysisControlType()");

  return eReturn;
}

//---------------------------------------------------------------------------
/** returns area scan rate type for control group */
AreaRateType TfrmAnalysis::GetAreaScanRateControlType() const {
  AreaRateType  eReturn;

  if (rdoHighRates->Checked)
    eReturn = HIGH;
  else if (rdoLowRates->Checked)
    eReturn = LOW;
  else if (rdoHighLowRates->Checked)
    eReturn = HIGHANDLOW;
  else
    ZdGenerateException("Scan for areas type not selected.","GetAreaScanRateControlType()");

  return eReturn;
}

//---------------------------------------------------------------------------
/** Returns parameter filename fullpath. */
const char * TfrmAnalysis::GetFileName() {
  return gParameters.GetSourceFileName().c_str();
}
//---------------------------------------------------------------------------
/** returns probability type for model control group */
ProbabilityModelType TfrmAnalysis::GetModelControlType() const {
  ProbabilityModelType   eReturn;

  if (rdoPoissonModel->Checked)
    eReturn = POISSON;
  else if (rdoBernoulliModel->Checked)
    eReturn = BERNOULLI;
  else if (rdoSpaceTimePermutationModel->Checked)
    eReturn = SPACETIMEPERMUTATION;
  else if (rdoOrdinalModel->Checked)
    eReturn = ORDINAL;
  else if (rdoExponentialModel->Checked)
    eReturn = EXPONENTIAL;
  else if (rdoNormalModel->Checked)
    eReturn = NORMAL;
  else
    ZdGenerateException("Probability model type not selected.","GetModelControlType()");

  return eReturn;
}
//---------------------------------------------------------------------------
AnsiString TfrmAnalysis::GetModelControlTypeName() const {
  if (rdoPoissonModel->Checked)
    return rdoPoissonModel->Caption;
  else if (rdoBernoulliModel->Checked)
    return rdoBernoulliModel->Caption;
  else if (rdoSpaceTimePermutationModel->Checked)
    return rdoSpaceTimePermutationModel->Caption;
  else if (rdoOrdinalModel->Checked)
    return rdoOrdinalModel->Caption;
  else if (rdoExponentialModel->Checked)
    return rdoExponentialModel->Caption;
  else if (rdoNormalModel->Checked)
    return rdoNormalModel->Caption;
  else
    ZdGenerateException("Probability model type not selected.","GetModelControlType()");
  return "";  
}
//---------------------------------------------------------------------------
/** returns precision of time type for precision control index */
DatePrecisionType TfrmAnalysis::GetPrecisionOfTimesControlType() const {
  DatePrecisionType eReturn;

  switch (rgpPrecisionTimes->ItemIndex) {
    case 0  : eReturn = NONE; break;
    case 1  : eReturn = YEAR; break;
    case 2  : eReturn = MONTH; break;
    case 3  : eReturn = DAY; break;
    default : ZdGenerateException("Unknown index type '%d'.", "GetPrecisionOfTimesControlType()", rgpPrecisionTimes->ItemIndex);
  };
  return eReturn;
}

//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
/** Sets passed ZdDate to study period end date as defined by TEditBoxes.*/
ZdDate& TfrmAnalysis::GetStudyPeriodEndDate(ZdDate& Date) const {
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

//---------------------------------------------------------------------------
/** Sets passed ZdDate to study period start date as defined by TEditBoxes.*/
ZdDate & TfrmAnalysis::GetStudyPeriodStartDate(ZdDate& Date) const {
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
//---------------------------------------------------------------------------
/** return precision type for time aggregation type */
DatePrecisionType TfrmAnalysis::GetTimeAggregationControlType() const {
  DatePrecisionType eReturn;

  if (rdoTimeAggregationYear->Checked)
    eReturn = YEAR;
  else if (rdoTimeAggregationMonths->Checked)
    eReturn = MONTH;
  else
    eReturn = DAY;
  return eReturn;
}
//---------------------------------------------------------------------------
/** class initialization */
void TfrmAnalysis::Init() {
  gpfrmAdvancedParameters = 0;
  gbPromptOnExist = true;
}
//---------------------------------------------------------------------------
/** Modally shows import dialog. */
void TfrmAnalysis::LaunchImporter(const char * sFileName, InputFileType eFileType) {
  ZdString sNewFile = "";

  try {
    std::auto_ptr<TBDlgDataImporter> pDialog(new TBDlgDataImporter(this,
                                                                   sFileName,
                                                                   eFileType,
                                                                   GetModelControlType(),
                                                                   (CoordinatesType)rgpCoordinates->ItemIndex));
    if (pDialog->ShowModal() == mrOk) {
       switch (eFileType) {  // set parameters
          case Case :       SetCaseFile(pDialog->GetDestinationFilename(sNewFile));
                            SetPrecisionOfTimesControl(pDialog->GetDateFieldImported()?(GetPrecisionOfTimesControlType()==NONE?YEAR:GetPrecisionOfTimesControlType()):NONE);
                            SetModelControl(pDialog->GetModelControlType());
                            break;
          case Control :    SetControlFile(pDialog->GetDestinationFilename(sNewFile));
                            SetPrecisionOfTimesControl(pDialog->GetDateFieldImported()?(GetPrecisionOfTimesControlType()==NONE?YEAR:GetPrecisionOfTimesControlType()):NONE);
                            SetModelControl(BERNOULLI);
                            break;
          case Population : SetPopulationFile(pDialog->GetDestinationFilename(sNewFile));
                            SetModelControl(POISSON);
                            break;
          case Coordinates: SetCoordinateFile(pDialog->GetDestinationFilename(sNewFile));
                            SetCoordinateType(pDialog->GetCoorinatesControlType());
                            break;
          case SpecialGrid: SetSpecialGridFile(pDialog->GetDestinationFilename(sNewFile));
                            SetCoordinateType(pDialog->GetCoorinatesControlType());
                            break;
          default :         ZdGenerateException("Unknown file type index: \"%d\"","LaunchImporter()", eFileType);
       };
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("LaunchImporter()", "TfrmAnalysis");
    throw;
  }
}

//---------------------------------------------------------------------------
/** event triggered when key pressed for control that can contain natural numbers */
void __fastcall TfrmAnalysis::NaturalNumberKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789\b",Key))
    Key = 0;
}

//---------------------------------------------------------------------------
/** method called in response to 'type of analysis' radio group click event */
void TfrmAnalysis::OnAnalysisTypeClick() {
  try {
    EnableModelControlForAnalysisType();
    EnableSettingsForAnalysisModelCombination();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnAnalysisTypeClick()","TfrmAnalysis");
    throw;
  }
}

//---------------------------------------------------------------------------
/** method called in response to 'precision of times' radio group click event */
void TfrmAnalysis::OnPrecisionTimesClick() {
  DatePrecisionType     eDatePrecisionType(GetPrecisionOfTimesControlType());

  try {
    //disable analyses that don't match precision
    rdoRetrospectivePurelySpatial->Enabled = true;
    rdoRetrospectivePurelyTemporal->Enabled = eDatePrecisionType != NONE;
    rdoRetrospectiveSpaceTime->Enabled = eDatePrecisionType != NONE;
    rdoProspectivePurelyTemporal->Enabled = eDatePrecisionType != NONE;
    rdoProspectiveSpaceTime->Enabled = eDatePrecisionType != NONE;
    rdoSVTT->Enabled = eDatePrecisionType != NONE;

    // switch analysis type to purely spatial if no dates in input data
    if (eDatePrecisionType == NONE && GetAnalysisControlType() != PURELYSPATIAL)
      SetAnalysisControl(PURELYSPATIAL);
    EnableTimeAggregationGroup(GetAnalysisControlType() != PURELYSPATIAL);
    EnableDatesByTimePrecisionUnits();
  }
  catch(ZdException &x) {
    x.AddCallpath("OnPrecisionTimesClick()","TfrmAnalysis");
    throw;
  }
}
//---------------------------------------------------------------------------
/** method called in response to 'probability model' radio group click event */
void TfrmAnalysis::OnProbabilityModelClick() {
  try {
    EnableAnalysisControlForModelType();
    switch (GetModelControlType()) {
      case POISSON   		:
      case BERNOULLI            :
      case ORDINAL              :
      case NORMAL               :
      case EXPONENTIAL          : lblSimulatedLogLikelihoodRatios->Caption = "Simulated Log Likelihood Ratios";
                                  gpfrmAdvancedParameters->lblPercentageOfStudyPeriod->Caption = "percent of the study period (<= 90%, default = 50%)";
                                  break;
      case SPACETIMEPERMUTATION : lblSimulatedLogLikelihoodRatios->Caption = "Simulated Test Statistics";
                                  gpfrmAdvancedParameters->lblPercentageOfStudyPeriod->Caption = "percent of the study period (<= 50%, default = 50%)";
                                  break;
      default : ZdGenerateException("Unknown probability model '%d'.", "OnProbablityModelClick()", GetModelControlType());
    }
    SetAreaScanRateControlText(GetModelControlType());
    EnableSettingsForAnalysisModelCombination();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnProbablityModelClick()","TfrmAnalysis");
    throw;
  }
}

//---------------------------------------------------------------------------
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
                 if (pYear == gpfrmAdvancedParameters->edtProspectiveStartDateYear)
                   pDay->Text = std::min(static_cast<unsigned int>(atoi(edtStudyPeriodEndDateDay->Text.c_str())), DaysThisMonth(uiYear, uiMonth));
                 else if (pYear == edtStudyPeriodEndDateYear)
                   pDay->Text = DaysThisMonth(uiYear, uiMonth);
                 else
                   pDay->Text = 1;
               }
               break;
      case 1 : if (uiYear >= MIN_YEAR && uiYear <= MAX_YEAR) {
                 pYear->Text = uiYear;
                 if (pYear == gpfrmAdvancedParameters->edtProspectiveStartDateYear) {
                   pMonth->Text = edtStudyPeriodEndDateMonth->Text;
                   pDay->Text = std::min(static_cast<unsigned int>(atoi(edtStudyPeriodEndDateDay->Text.c_str())),
                                         DaysThisMonth(uiYear, static_cast<unsigned int>(atoi(edtStudyPeriodEndDateMonth->Text.c_str()))));
                 }
               }
               break;
    };
  }
  //store intial value in control's Tag property
  pYear->Tag = StrToInt(pYear->Text);
  pMonth->Tag = StrToInt(pMonth->Text);
  pDay->Tag = StrToInt(pDay->Text);
}

//---------------------------------------------------------------------------
/** event triggered when key pressed for control that can contain positive real numbers */
void __fastcall TfrmAnalysis::PositiveFloatKeyPress(TObject *Sender, char &Key) {
  TEdit * pEdit;
  //permit only characters that could comprise a positive floating point variable
  if (!strchr("0123456789.\b", Key))
    Key = 0;
  //permit only one decimal place
  else if (Key == '.' && (pEdit = dynamic_cast<TEdit*>(Sender)) != 0 && strchr(pEdit->Text.c_str(), Key))
    Key = 0;
}

//---------------------------------------------------------------------------
/** event triggered when time interval unit type selected as 'day' */
void __fastcall TfrmAnalysis::rdoTimeAggregationDayClick(TObject *Sender) {
  gpfrmAdvancedParameters->lblMaxTemporalTimeUnits->Caption = "days";
  stUnitText->Caption = "Days";
}

//---------------------------------------------------------------------------
/** event triggered when time interval unit type selected as 'month' */
void __fastcall TfrmAnalysis::rdoTimeAggregationMonthsClick(TObject *Sender) {
  gpfrmAdvancedParameters->lblMaxTemporalTimeUnits->Caption = "months";
  stUnitText->Caption = "Months";
}

//---------------------------------------------------------------------------
/** event triggered when time interval unit type selected as 'year' */
void __fastcall TfrmAnalysis::rdoTimeAggregationYearClick(TObject *Sender) {
    gpfrmAdvancedParameters->lblMaxTemporalTimeUnits->Caption = "years";
    stUnitText->Caption = "Years";
}
//---------------------------------------------------------------------------
/** event triggered when 'analysis' type control clicked */
void __fastcall TfrmAnalysis::rdoAnalysisTypeClick(TObject *Sender) {
  try {
     OnAnalysisTypeClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgTypeAnalysisClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

//---------------------------------------------------------------------------
/** event triggered when 'probability model' type control clicked */
void __fastcall TfrmAnalysis::rdoProbabilityModelClick(TObject *Sender) {
  try {
     OnProbabilityModelClick();
  }
  catch ( ZdException & x ) {
    x.AddCallpath("rgProbabilityClick()", "TfrmAnalysis" );
    DisplayBasisException( this, x );
  }
}

//---------------------------------------------------------------------------
/** event triggered when coordinates type control clicked */
void __fastcall TfrmAnalysis::rgpCoordinatesClick(TObject *Sender) {
  try {
    gpfrmAdvancedParameters->SetSpatialDistanceCaption();
    gpfrmAdvancedParameters->EnableSettingsForAnalysisModelCombination();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgCoordinatesClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

//---------------------------------------------------------------------------
/** event triggered when 'precision of times' type control clicked */
void __fastcall TfrmAnalysis::rgpPrecisionTimesClick(TObject *Sender) {
  try {
    OnPrecisionTimesClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgPrecisionTimesClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

//---------------------------------------------------------------------------
/** launches 'save as' dialog to permit user saving current settings to parameter file */
bool TfrmAnalysis::SaveAs() {
  bool  bSaved=true;

  try {
    if (SaveDialog->Execute()) {
      WriteSession(SaveDialog->FileName.c_str());
      Caption = SaveDialog->FileName;
    }
    else
     bSaved = false; 
  }
  catch (ZdException & x) {
    x.AddCallpath("SaveAs()", "TfrmAnalysis");
    throw;
  }
  return bSaved;
}

//---------------------------------------------------------------------------
/** sets CParameters class with settings in form */
void TfrmAnalysis::SaveParameterSettings() {
  ZdString sString;

  try {
    //Force calling of methods that would have been performed by TWinControl::OnExit.
    //Otherwise it is possible that they do not get called prior to this function call.
    TriggerOnExitEvents();

    Caption = gParameters.GetSourceFileName().c_str();

    //set version parameter
    CParameters::CreationVersion vVersion = {atoi(VERSION_MAJOR), atoi(VERSION_MINOR), atoi(VERSION_RELEASE)};
    gParameters.SetVersion(vVersion);
    //Input File Tab
    gParameters.SetCaseFileName(edtCaseFileName->Text.c_str());
    gParameters.SetControlFileName(edtControlFileName->Text.c_str());
    gParameters.SetPrecisionOfTimesType(GetPrecisionOfTimesControlType());
    gParameters.SetPopulationFileName(edtPopFileName->Text.c_str());
    gParameters.SetCoordinatesFileName(edtCoordinateFileName->Text.c_str());
    gParameters.SetSpecialGridFileName(edtGridFileName->Text.c_str(), false, true);
    gParameters.SetCoordinatesType((CoordinatesType)rgpCoordinates->ItemIndex);
    //Analysis Tab
    gParameters.SetAnalysisType(GetAnalysisControlType());
    gParameters.SetProbabilityModelType(GetModelControlType());
    gParameters.SetAreaRateType(GetAreaScanRateControlType());
    sString.printf("%i/%i/%i", atoi(edtStudyPeriodStartDateYear->Text.c_str()),
                   atoi(edtStudyPeriodStartDateMonth->Text.c_str()), atoi(edtStudyPeriodStartDateDay->Text.c_str()));
    gParameters.SetStudyPeriodStartDate(sString.GetCString());
    sString.printf("%i/%i/%i", atoi(edtStudyPeriodEndDateYear->Text.c_str()),
                   atoi(edtStudyPeriodEndDateMonth->Text.c_str()), atoi(edtStudyPeriodEndDateDay->Text.c_str()));
    gParameters.SetStudyPeriodEndDate(sString.GetCString());
    if (edtMontCarloReps->Text.IsEmpty()) edtMontCarloReps->Text = 999;
    gParameters.SetNumberMonteCarloReplications(static_cast<unsigned long>(edtMontCarloReps->Text.ToDouble()));
    gParameters.SetTimeAggregationUnitsType(GetTimeAggregationControlType());
    gParameters.SetTimeAggregationLength(atoi(edtTimeAggregationLength->Text.c_str()));
    //Output File Tab
    gParameters.SetOutputFileName(edtResultFile->Text.c_str());
    gParameters.SetOutputClusterLevelAscii(chkClustersInColumnFormatAscii->Checked);
    gParameters.SetOutputClusterLevelDBase(chkClustersInColumnFormatDBase->Checked);
    gParameters.SetOutputClusterCaseAscii(chkClusterCaseInColumnFormatAscii->Checked);
    gParameters.SetOutputClusterCaseDBase(chkClusterCaseInColumnFormatDBase->Checked);
    gParameters.SetOutputAreaSpecificAscii(chkCensusAreasReportedClustersAscii->Checked);
    gParameters.SetOutputAreaSpecificDBase(chkCensusAreasReportedClustersDBase->Checked);
    gParameters.SetOutputRelativeRisksAscii(chkRelativeRiskEstimatesAreaAscii->Enabled && chkRelativeRiskEstimatesAreaAscii->Checked);
    gParameters.SetOutputRelativeRisksDBase(chkRelativeRiskEstimatesAreaDBase->Enabled && chkRelativeRiskEstimatesAreaDBase->Checked);
    gParameters.SetOutputSimLogLikeliRatiosAscii(chkSimulatedLogLikelihoodRatiosAscii->Checked);
    gParameters.SetOutputSimLogLikeliRatiosDBase(chkSimulatedLogLikelihoodRatiosDBase->Checked);
    gpfrmAdvancedParameters->SaveParameterSettings();
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveParameterSettings()","TfrmAnalysis");
    throw;
  }
}

//---------------------------------------------------------------------------
/** Sets adjustment by relative risks filename in interface */
void TfrmAnalysis::SetAdjustmentsByRelativeRisksFile(const char * sAdjustmentsByRelativeRisksFileName) {
  gpfrmAdvancedParameters->SetAdjustmentsByRelativeRisksFile(sAdjustmentsByRelativeRisksFileName);
}

//---------------------------------------------------------------------------
/** sets analysis type control for AnalysisType */
void TfrmAnalysis::SetAnalysisControl(AnalysisType eAnalysisType) {
  DatePrecisionType     eDatePrecisionType(GetPrecisionOfTimesControlType());

  switch (eAnalysisType) {
    case PURELYTEMPORAL                 : if (rdoRetrospectivePurelyTemporal->Enabled && eDatePrecisionType != NONE) {rdoRetrospectivePurelyTemporal->Checked = true; break;}
    case SPACETIME                      : if (rdoRetrospectiveSpaceTime->Enabled && eDatePrecisionType != NONE) {rdoRetrospectiveSpaceTime->Checked = true; break;}
    case PROSPECTIVEPURELYTEMPORAL      : if (rdoProspectivePurelyTemporal->Enabled && eDatePrecisionType != NONE) {rdoProspectivePurelyTemporal->Checked = true; break;}
    case PROSPECTIVESPACETIME           : if (rdoProspectiveSpaceTime->Enabled && eDatePrecisionType != NONE) {rdoProspectiveSpaceTime->Checked = true; break;}
    case SPATIALVARTEMPTREND            : if (rdoSVTT->Enabled && eDatePrecisionType != NONE) {rdoSVTT->Checked = true; break;}
    case PURELYSPATIAL                  :
    default                             : rdoRetrospectivePurelySpatial->Checked = true;
  }
  EnableModelControlForAnalysisType();
}

//---------------------------------------------------------------------------
/** sets area scan rate type control */
void TfrmAnalysis::SetAreaScanRateControl(AreaRateType eAreaRateType) {
  switch (eAreaRateType) {
    case LOW        : rdoLowRates->Checked = true; break;
    case HIGHANDLOW : rdoHighLowRates->Checked = true; break;
    case HIGH       :
    default         : rdoHighRates->Checked = true;
  }
}
//---------------------------------------------------------------------------
/** Sets captions of TRadioButton controls of 'Scan for Areas with:' group based upon selected probablility model. */
void TfrmAnalysis::SetAreaScanRateControlText(ProbabilityModelType eProbabilityModelType) {
    switch (GetModelControlType()) {
      case POISSON   		: if (GetAnalysisControlType() == SPATIALVARTEMPTREND) {
                                    rdoLowRates->Caption = "Decreasing Rates";
                                    rdoHighRates->Caption = "Increasing Rates";
                                    rdoHighLowRates->Caption = "Increasing or";
                                    stScanAreaRates->Visible = true;
                                    break;
                                  }
      case BERNOULLI            :
      case SPACETIMEPERMUTATION : rdoLowRates->Caption = "Low Rates";
                                  rdoHighRates->Caption = "High Rates";
                                  rdoHighLowRates->Caption = "High or Low Rates";
                                  stScanAreaRates->Visible = false;
                                  break;
      case ORDINAL              :
      case NORMAL               : rdoLowRates->Caption = "Low Values";
                                  rdoHighRates->Caption = "High Values";
                                  rdoHighLowRates->Caption = "High or Low Values";
                                  stScanAreaRates->Visible = false;
                                  break;
      case EXPONENTIAL          : rdoLowRates->Caption = "Long Survival";
                                  rdoHighRates->Caption = "Short Survival";
                                  rdoHighLowRates->Caption = "Short or Long Survival";
                                  stScanAreaRates->Visible = false;
                                  break;
      default : ZdGenerateException("Unknown probability model '%d'.", "SetAreaScanRateControlText()", GetModelControlType());
    }
}
//---------------------------------------------------------------------------
/** Sets case filename in interface */
void TfrmAnalysis::SetCaseFile(const char * sCaseFileName) {
  edtCaseFileName->Text = sCaseFileName;
}

//---------------------------------------------------------------------------
/** Sets control filename in interface */
void TfrmAnalysis::SetControlFile(const char * sControlFileName) {
  edtControlFileName->Text = sControlFileName;
}

//---------------------------------------------------------------------------
/** Sets coordinates filename in interface */
void TfrmAnalysis::SetCoordinateFile(const char * sCoordinateFileName) {
  edtCoordinateFileName->Text = sCoordinateFileName;
}

//---------------------------------------------------------------------------
/** sets coordinate type */
void TfrmAnalysis::SetCoordinateType(CoordinatesType eCoordinatesType) {
  rgpCoordinates->ItemIndex = eCoordinatesType;
}

/** Method that helps store/restore specified values and set default value in
    conjunction with enabling/disabling study period dates. */
void TfrmAnalysis::SetDayEditText(TEdit& Day, bool bEnablingDay, int iDayText) {
  try {
    if (Day.Enabled && !bEnablingDay) {
      Day.Tag = StrToInt(Day.Text);
      Day.Text = iDayText;
    }
    if (!Day.Enabled && bEnablingDay)
      Day.Text = Day.Tag;
  }
  catch (EConvertError& e){}
}
//---------------------------------------------------------------------------
/** Sets special population filename in interface */
void TfrmAnalysis::SetMaximumCirclePopulationFile(const char * sMaximumCirclePopulationFileName) {
  gpfrmAdvancedParameters->edtMaxCirclePopulationFilename->Text = sMaximumCirclePopulationFileName;
}

//---------------------------------------------------------------------------
/** sets probaiity model type control for ProbabilityModelType */
void TfrmAnalysis::SetModelControl(ProbabilityModelType eProbabilityModelType) {
  switch (eProbabilityModelType) {
    case BERNOULLI            : if (rdoBernoulliModel->Enabled) {rdoBernoulliModel->Checked = true; break;}
    case SPACETIMEPERMUTATION : if (rdoSpaceTimePermutationModel->Enabled) {rdoSpaceTimePermutationModel->Checked = true; break;}
    case ORDINAL              : if (rdoOrdinalModel->Enabled) {rdoOrdinalModel->Checked = true; break;}
    case EXPONENTIAL          : if (rdoExponentialModel->Enabled) {rdoExponentialModel->Checked = true; break;}
    case NORMAL               : if (rdoNormalModel->Enabled) {rdoNormalModel->Checked = true; break;}
    case POISSON              :
    default                   : rdoPoissonModel->Checked = true;
  }
  EnableAnalysisControlForModelType();
}

/** Method that helps store/restore specified values and set default value in
    conjunction with enabling/disabling study period dates. */
void TfrmAnalysis::SetMonthEditText(TEdit& Month, bool bEnablingMonth, int iMonthText) {
  try {
    if (Month.Enabled && !bEnablingMonth) {
      Month.Tag = StrToInt(Month.Text);
      Month.Text = iMonthText;
    }
    if (!Month.Enabled && bEnablingMonth)
      Month.Text = Month.Tag;
  }
  catch (EConvertError& e){}
}

//---------------------------------------------------------------------------
/** Sets population filename in interface */
void TfrmAnalysis::SetPopulationFile(const char * sPopulationFileName) {
  edtPopFileName->Text = sPopulationFileName;
}

//---------------------------------------------------------------------------
/** sets precision of times type control for DatePrecisionType */
void TfrmAnalysis::SetPrecisionOfTimesControl(DatePrecisionType eDatePrecisionType) {
  switch (eDatePrecisionType) {
    case YEAR  : rgpPrecisionTimes->ItemIndex = 1; break;
    case MONTH : rgpPrecisionTimes->ItemIndex = 2; break;
    case DAY   : rgpPrecisionTimes->ItemIndex = 3; break;
    case NONE  :
    default    : rgpPrecisionTimes->ItemIndex = 0;
  }
}

//---------------------------------------------------------------------------
/** Sets special grid filename in interface */
void TfrmAnalysis::SetSpecialGridFile(const char * sSpecialGridFileName) {
  gParameters.SetSpecialGridFileName(sSpecialGridFileName, false, true);
  edtGridFileName->Text = sSpecialGridFileName;
}

//---------------------------------------------------------------------------
/** Internal setup */
//---------------------------------------------------------------------------
void TfrmAnalysis::Setup(const char * sParameterFileName) {
  try {
    PageControl1->ActivePage = tbInputFiles;
    try {
      if (sParameterFileName)
        ParameterAccessCoordinator(gParameters).Read(sParameterFileName, gNullPrint);
      else {
        //New session - creation version is this version.
        CParameters::CreationVersion vVersion = {atoi(VERSION_MAJOR), atoi(VERSION_MINOR), atoi(VERSION_RELEASE)};
        gParameters.SetVersion(vVersion);
      }
    }
    catch (ZdException &x) {
      x.SetLevel(ZdException::Notify);
      x.SetErrorMessage((const char*)"SaTScan is unable to read parameters from file \"%s\".\n", sParameterFileName);
      throw;
    }
    DefaultHiddenParameters();
    SetupInterface();

    //Save orginal parameter settings to compare against when window closes but
    //first save what the interface has produced for the settings read from file.
    SaveParameterSettings();
    gInitialParameters = gParameters;
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()", "TfrmAnalysis");
    throw;
  }
}

//---------------------------------------------------------------------------
/** Sets all interface controls using the CParameters session object */
void TfrmAnalysis::SetupInterface() {
  gpfrmAdvancedParameters = new TfrmAdvancedParameters(*this);

  try {
    //Input File Tab
    Caption = gParameters.GetSourceFileName().c_str();
    //PAG - make caption 'new' if no file name
    if (Caption.Length() == 0)
       Caption = "New Session";
    edtCaseFileName->Text = gParameters.GetCaseFileName().c_str();
    edtControlFileName->Text = gParameters.GetControlFileName().c_str();
    SetPrecisionOfTimesControl(gParameters.GetPrecisionOfTimesType());
    edtPopFileName->Text = gParameters.GetPopulationFileName().c_str();
    edtCoordinateFileName->Text = gParameters.GetCoordinatesFileName().c_str();
    edtGridFileName->Text = gParameters.GetSpecialGridFileName().c_str();
    rgpCoordinates->ItemIndex = gParameters.GetCoordinatesType();
    //Analysis Tab
    SetAnalysisControl(gParameters.GetAnalysisType());
    SetModelControl(gParameters.GetProbabilityModelType());
    SetAreaScanRateControl(gParameters.GetAreaScanRateType());
    ParseDate(gParameters.GetStudyPeriodStartDate().c_str(), edtStudyPeriodStartDateYear, edtStudyPeriodStartDateMonth, edtStudyPeriodStartDateDay);
    ParseDate(gParameters.GetStudyPeriodEndDate().c_str(), edtStudyPeriodEndDateYear, edtStudyPeriodEndDateMonth, edtStudyPeriodEndDateDay);
    edtMontCarloReps->Text = gParameters.GetNumReplicationsRequested();
    if (gParameters.GetTimeAggregationUnitsType() == NONE) gParameters.SetTimeAggregationUnitsType(YEAR);
    if (gParameters.GetTimeAggregationLength() <= 0) gParameters.SetTimeAggregationLength(1);
    rdoTimeAggregationYear->Checked = (gParameters.GetTimeAggregationUnitsType() == YEAR);
    rdoTimeAggregationMonths->Checked = (gParameters.GetTimeAggregationUnitsType() == MONTH);
    rdoTimeAggregationDay->Checked = (gParameters.GetTimeAggregationUnitsType() == DAY);
    edtTimeAggregationLength->Text = gParameters.GetTimeAggregationLength();
    if (gParameters.GetProspectiveStartDate().length() > 0)
       ParseDate(gParameters.GetProspectiveStartDate().c_str(), gpfrmAdvancedParameters->edtProspectiveStartDateYear, gpfrmAdvancedParameters->edtProspectiveStartDateMonth, gpfrmAdvancedParameters->edtProspectiveStartDateDay);
    gpfrmAdvancedParameters->chkAdjustForEarlierAnalyses->Checked = gParameters.GetAdjustForEarlierAnalyses();
    //Output File Tab
    edtResultFile->Text = gParameters.GetOutputFileName().c_str();
    chkRelativeRiskEstimatesAreaAscii->Checked = gParameters.GetOutputRelativeRisksAscii();
    chkRelativeRiskEstimatesAreaDBase->Checked = gParameters.GetOutputRelativeRisksDBase();
    chkSimulatedLogLikelihoodRatiosAscii->Checked  = gParameters.GetOutputSimLoglikeliRatiosAscii();
    chkSimulatedLogLikelihoodRatiosDBase->Checked = gParameters.GetOutputSimLoglikeliRatiosDBase();
    chkCensusAreasReportedClustersAscii->Checked    = gParameters.GetOutputAreaSpecificAscii();  // Output Census areas in Reported Clusters
    chkClustersInColumnFormatAscii->Checked = gParameters.GetOutputClusterLevelAscii();  // Output Most Likely Cluster for each Centroid
    chkClustersInColumnFormatDBase->Checked = gParameters.GetOutputClusterLevelDBase();
    chkClusterCaseInColumnFormatAscii->Checked = gParameters.GetOutputClusterCaseAscii();  // Output Most Likely Cluster for each Centroid
    chkClusterCaseInColumnFormatDBase->Checked = gParameters.GetOutputClusterCaseDBase();
    chkCensusAreasReportedClustersDBase->Checked = gParameters.GetOutputAreaSpecificDBase();
    EnableSettingsForAnalysisModelCombination();
    EnableAdvancedButtons();
  }
  catch (ZdException & x) {
    x.AddCallpath("SetupInterface()", "TfrmAnalysis");
    delete gpfrmAdvancedParameters;
    gpfrmAdvancedParameters=0;
    throw;
  }
}

//---------------------------------------------------------------------------
/** Modally shows advanced features dialog. */
void TfrmAnalysis::ShowAdvancedFeaturesDialog() {

   try {
     gpfrmAdvancedParameters->ShowDialog(0, PageControl1->ActivePageIndex+1);
     EnableAdvancedButtons();
   }
   catch (ZdException & x) {
     x.AddCallpath("ShowAdvancedFeatureDialog()", "TfrmAnalysis");
     throw;
   }
}

/** If controls are enabled, stores Text property of TEdit control in Tag property */
void TfrmAnalysis::StoreEditText(TEdit& Month, TEdit& Day) {
  try {
    if (Month.Enabled)
      Month.Tag = StrToInt(Month.Text);
    if (Day.Enabled)
      Day.Tag = StrToInt(Day.Text);
  }
  catch (EConvertError& e){}
}

/** Triggers TWinControl::OnExit events for controls that defines such events.
    Event TWinControl::OnExit is protected, so we can not do this programmatically . */
void TfrmAnalysis::TriggerOnExitEvents() {
  edtStudyPeriodStartDateExit(this);
  edtStudyPeriodEndDateExit(this);
  edtTimeAggregationLengthExit(this);
  edtMontCarloRepsExit(this);
}

/** validates date controls represented by three passed edit controls - prevents an invalid date */
void TfrmAnalysis::ValidateDate(TEdit& YearControl, TEdit& MonthControl, TEdit& DayControl) {
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
//---------------------------------------------------------------------------
/** Validates 'Input Files' tab */
void TfrmAnalysis::ValidateInputFiles() {
  try {
    CheckStudyPeriodDatesRange();

    //validate the case file
    if (edtCaseFileName->Text.IsEmpty()) {
      PageControl1->ActivePage = tbInputFiles;
      edtCaseFileName->SetFocus();
      ZdException::GenerateNotification("Please specify a case file.","ValidateInputFiles()");
    }
    if (!ValidateFileAccess(edtCaseFileName->Text.c_str())) {
      PageControl1->ActivePage = tbInputFiles;
      edtCaseFileName->SetFocus();
      ZdException::GenerateNotification("The case file could not be opened for reading.\n"
                                        "Please confirm that the path and/or file name\n"
                                        "are valid and that you have permissions to read\nfrom this directory and file.", "ValidateInputFiles()");
    }
    //validate the control file - Bernoulli model only
    if (GetModelControlType() == BERNOULLI) {
      if (edtControlFileName->Text.IsEmpty()) {
        PageControl1->ActivePage = tbInputFiles;
        edtControlFileName->SetFocus();
        ZdException::GenerateNotification("For the Bernoulli model, please specify a control file.","ValidateInputFiles()");
      }
      if (!ValidateFileAccess(edtControlFileName->Text.c_str())) {
        PageControl1->ActivePage = tbInputFiles;
        edtControlFileName->SetFocus();
        ZdException::GenerateNotification("The control file could not be opened for reading.\n"
                                          "Please confirm that the path and/or file name are\n"
                                          "valid and that you have permissions to read from\nthis directory and file.", "ValidateInputFiles()");
      }
    }
    //validate the population file -  Poisson model only
    if (GetModelControlType() == POISSON) {
      if (edtPopFileName->Text.IsEmpty()) {
        if (GetAnalysisControlType() != PURELYTEMPORAL && GetAnalysisControlType() != PROSPECTIVEPURELYTEMPORAL) {
          //for purely temporal analyses, the population file is optional
          PageControl1->ActivePage = tbInputFiles;
          edtPopFileName->SetFocus();
          ZdException::GenerateNotification("For the Poisson model, please specify a population file.\n"
                                            "Note that for purely temporal analyses, if the risk does\n"
                                            "not change over time, the population file is optional.","ValidateInputFiles()");
        }                                    
      }
      else if (!ValidateFileAccess(edtPopFileName->Text.c_str())) {
        PageControl1->ActivePage = tbInputFiles;
        edtPopFileName->SetFocus();
        ZdException::GenerateNotification("The population file could not be opened for reading.\n"
                                          "Please confirm that the path and/or file name are\n"
                                          "valid and that you have permissions to read from this\ndirectory and file.", "ValidateInputFiles()");
      }
    }
    AnalysisType eAnalysisType(GetAnalysisControlType());
    //validate coordinates and grid file -- ignore validation if using neighbors file or purely temporal analysis
    if (!(gpfrmAdvancedParameters->chkSpecifiyNeighborsFile->Enabled && gpfrmAdvancedParameters->chkSpecifiyNeighborsFile->Checked) &&
        !(eAnalysisType == PURELYTEMPORAL || eAnalysisType == PROSPECTIVEPURELYTEMPORAL)) {
      if (edtCoordinateFileName->Text.IsEmpty()) {
        PageControl1->ActivePage = tbInputFiles;
        edtCoordinateFileName->SetFocus();
        ZdException::GenerateNotification("Please specify a coordinates file.","ValidateInputFiles()");
      }
      else if (!ValidateFileAccess(edtCoordinateFileName->Text.c_str())) {
        PageControl1->ActivePage = tbInputFiles;
        edtCoordinateFileName->SetFocus();
        ZdException::GenerateNotification("The coordinates file could not be opened for reading.\n"
                                          "Please confirm that the path and/or file name are\n"
                                          "valid and that you have permissions to read from this\ndirectory and file.", "ValidateInputFiles()");
      }
      //validate special grid file -- optional
      if (!edtGridFileName->Text.IsEmpty() &&  !ValidateFileAccess(edtGridFileName->Text.c_str())) {
        PageControl1->ActivePage = tbInputFiles;
        edtGridFileName->SetFocus();
        ZdException::GenerateNotification("The grid file could not be opened for reading.\n"
                                          "Please confirm that the path and/or file name\n"
                                          "are valid and that you have permissions to read\nfrom this directory and file.", "ValidateInputFiles()");
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateInputFiles()", "TfrmAnalysis");
    throw;
  }
}
//---------------------------------------------------------------------------
/** This function is used right before a job is submitted.  Verifies that
    all the input files exist and can be read.  Also checks each tab to see
    if all settings are in place.                                          */
bool TfrmAnalysis::ValidateParams() {
  bool bReturn=true;

  try {
    //Force calling of methods that would have been performed by TWinControl::OnExit.
    //Otherwise it is possible that they do not get called prior to this validation.
    TriggerOnExitEvents(); 
    ValidateInputFiles();           // validate 'input' tab parameters
    CheckAnalysisParams();          // validate 'analysis' tab parameters
    CheckOutputParams();
    gpfrmAdvancedParameters->Validate();
  }
  catch (AdvancedFeaturesException &x) {
    x.AddCallpath("ValidateParams()","TfrmAnalysis");
    bReturn = false;
    DisplayBasisException(this, x);
    gpfrmAdvancedParameters->ShowDialog(&x.GetFocusControl(), x.GetTabCategory());
    EnableAdvancedButtons();
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateParams()","TfrmAnalysis");
    bReturn = false;
    DisplayBasisException(this, x);
  }
  return bReturn;
}
//---------------------------------------------------------------------------
/** Writes the session information to disk */
bool TfrmAnalysis::WriteSession(const char * sParameterFilename) {
  std::string   sFilename;
  bool          bSaved=true;

  try {
    sFilename = (sParameterFilename ? sParameterFilename : gParameters.GetSourceFileName().c_str());
    if (sFilename.empty())
      bSaved = SaveAs();
    else {
      if (!access(sFilename.c_str(), 00) && access(sFilename.c_str(), 02))
        ZdException::GenerateNotification("Unable to save session parameters.\n"
                                          "The file is either read only or you do not have write privledges to the directory.",
                                          "WriteSession()");
      SaveParameterSettings();
      ParameterAccessCoordinator(gParameters).Write(sFilename.c_str(), gNullPrint);
      gInitialParameters = gParameters;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("WriteSession()", "TfrmAnalysis");
    throw;
  }
  return bSaved;
}
//---------------------------------------------------------------------------
/** Event triggered when the static text control that accompanies SVTT radio
    button is clicked -- causing the radio button to be selected and focused. */
void __fastcall TfrmAnalysis::stSVTTClick(TObject *Sender) {
  rdoSVTT->Checked = true;
  rdoSVTT->SetFocus();
}
//---------------------------------------------------------------------------
/** Event triggered when the static text control that accompanies the high/low radio
    button is clicked -- causing the radio button to be selected and focused. */
void __fastcall TfrmAnalysis::stScanAreaRatesClick(TObject *Sender) {
  rdoHighLowRates->Checked = true;
  rdoHighLowRates->SetFocus();
}
//---------------------------------------------------------------------------

