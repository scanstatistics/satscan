//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
#include "stsFrmAdvancedParameters.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

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
__fastcall TfrmAnalysis::~TfrmAnalysis() {
  try {
    delete gpfrmAdvancedParameters;
  }
  catch (...){}   
}

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

/** Validates time interval length is not less than zero. */
void TfrmAnalysis::Check_IntervalLength() {
  if (edtTimeIntervalLength->Text.IsEmpty() || atoi(edtTimeIntervalLength->Text.c_str()) < 1) {
    PageControl1->ActivePage = tbTimeParameter;
    edtTimeIntervalLength->SetFocus();
    ZdException::GenerateNotification("Please specify an interval length greater than zero.","Check_IntervalLength()");
  }
}

/** Checks all the parameters on the 'Analysis' tab. Returns whether tab is valid. */
void TfrmAnalysis::CheckAnalysisParams() {
  try {
    CheckReplicas();
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckAnalysisParams()", "TfrmAnalysis");
    throw;
  }
}

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

/** Verifies all parameters on the 'Output Files' tab. Returns whether tab is valid.*/
void TfrmAnalysis::CheckOutputParams() {
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
}

/** Specific prospective space-time date check
    Must be between the start and end dates of the analysis */
void TfrmAnalysis::CheckProspDateRange() {
  Julian        Start, End, Prosp;
  int           iProspYear, iProspMonth, iProspDay;

  try {
    CheckDate("prospective start date", *edtProspectiveStartDateYear, *edtProspectiveStartDateMonth,
              *edtProspectiveStartDateDay,  *tbTimeParameter);

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

    if ((Prosp < Start) || (Prosp > End)) {
      PageControl1->ActivePage = tbTimeParameter;
      edtProspectiveStartDateYear->SetFocus();
      ZdException::GenerateNotification("The prospective start date must be between the study period start and end dates.",
                                        "CheckProspDateRange()");
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckProspDateRange()", "TfrmAnalysis");
    throw;
  }
}

/** Checks Monte Carlo replications */
void TfrmAnalysis::CheckReplicas() {
  if (edtMontCarloReps->Text.IsEmpty() || !IsValidReplicationRequest(atoi(edtMontCarloReps->Text.c_str()))) {
    PageControl1->ActivePage = tbAnalysis;
    edtMontCarloReps->SetFocus();
    ZdException::GenerateNotification("Invalid number of Monte Carlo replications.\nChoices are: 9, 999, or value ending in 999.", "CheckReplicas()");
  }
}

/** Checks the validity of the 'Scanning Window' tab */
void TfrmAnalysis::CheckScanningWindowParams() {
  try {
    ValidateTemoralClusterSize();
    ValidateSpatialClusterSize();
  }
  catch (ZdException &x) {
    x.AddCallpath("CheckScanningWindowParams()","TfrmAnalysis");
    throw;
  }
}


/** Checks the relationship between a start date, end date, and interval length.
    Display message box regarding errors when appropriate. Return whether relationship is valid. */
void TfrmAnalysis::CheckStudyPeriodDatesRange() {
  bool          bRangeOk = true;
  ZdString      sErrorMessage;
  ZdDate        StartDate, EndDate;
  ZdDateFilter  DateFilter("%4y/%02m/%02d");
  char          FilterBuffer[11];
  int           iIntervalLength(atoi(edtTimeIntervalLength->Text.c_str()));

  try {
    CheckDate("study period start date", *edtStudyPeriodStartDateYear, *edtStudyPeriodStartDateMonth,
              *edtStudyPeriodStartDateDay,  *tbTimeParameter);
    CheckDate("study period end date", *edtStudyPeriodEndDateYear, *edtStudyPeriodEndDateMonth,
              *edtStudyPeriodEndDateDay,  *tbTimeParameter);

    GetStudyPeriodStartDate(StartDate);
    GetStudyPeriodEndDate(EndDate);

    //check that start date is before end date
    if (StartDate >= EndDate) {
      DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), StartDate.GetRawDate());
      sErrorMessage << "The study period start date of " << FilterBuffer;
      DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), EndDate.GetRawDate());
      sErrorMessage << " does not occur before study period end date of " << FilterBuffer;
      sErrorMessage << ".\nPlease review settings.";
      PageControl1->ActivePage = tbTimeParameter;
      edtStudyPeriodStartDateYear->SetFocus();
      ZdException::GenerateNotification(sErrorMessage.GetCString(),"CheckStudyPeriodDatesRange()");
    }

    if (GetAnalysisControlType() != PURELYSPATIAL)  {/* purely spatial does not use interval length */
      //check that interval length is not greater study period
      //(.i.e. can't have study period that is 20 days and intervals of 3 months)
      //to make start and end day inclusive - add 1 to end date
      EndDate.AddDays(1);
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
        PageControl1->ActivePage = tbTimeParameter;
        edtTimeIntervalLength->SetFocus();
        ZdException::GenerateNotification(sErrorMessage.GetCString(),"CheckStudyPeriodDatesRange()");
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckStudyPeriodDatesRange()", "TfrmAnalysis");
    throw;
  }
}

/** Checks 'Time Parameters' tab */
void TfrmAnalysis::CheckTimeParams() {
  double dValue;

  try {
    if (edtTimeIntervalLength->Enabled)
      Check_IntervalLength();
    CheckStudyPeriodDatesRange();
    if (chkAdjustForEarlierAnalyses->Enabled && chkAdjustForEarlierAnalyses->Checked)
      CheckProspDateRange();
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckTimeParams()", "TfrmAnalysis");
    throw;
  }
}

/** event triggered when 'adjustment for ealier analyses' checkbox if clicked */
void __fastcall TfrmAnalysis::chkAdjustForEarlierAnalysesClick(TObject *Sender) {
  EnableProspectiveStartDate(chkAdjustForEarlierAnalyses->Checked);
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
  //non-parametric removed from interface, replaced with time stratified
  if (gParameters.GetTimeTrendAdjustmentType() == NONPARAMETRIC)
    gParameters.SetTimeTrendAdjustmentType(STRATIFIED_RANDOMIZATION);
  if (gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC)
    gParameters.SetTimeTrendAdjustmentType(NOTADJUSTED);
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

/** event triggered when maximum spatial cluster size edit control is exited. */
void __fastcall TfrmAnalysis::edtMaxSpatialClusterSizeExit(TObject *Sender) {
  if (edtMaxSpatialClusterSize->Text.IsEmpty() || atof(edtMaxSpatialClusterSize->Text.c_str()) == 0)
    edtMaxSpatialClusterSize->Text = 50;
  SetReportingSmallerClustersText();
}

/** event triggered when maximum temporal cluster size edit control is exited. */
void __fastcall TfrmAnalysis::edtMaxTemporalClusterSizeExit(TObject *Sender) {
  if (edtMaxTemporalClusterSize->Text.IsEmpty() || atof(edtMaxTemporalClusterSize->Text.c_str()) == 0)
    edtMaxTemporalClusterSize->Text = 50;
}

/** event triggered when Monte Carlo replications control is exited. */
void __fastcall TfrmAnalysis::edtMontCarloRepsExit(TObject *Sender) {
  if (edtMontCarloReps->Text.IsEmpty() || !IsValidReplicationRequest(atoi(edtMontCarloReps->Text.c_str())))
    edtMontCarloReps->Text = 999;
}

/** event triggered when population file edit control text changes */
void __fastcall TfrmAnalysis::edtPopFileNameChange(TObject *Sender){
  edtPopFileName->Hint = edtPopFileName->Text;
}
/** event triggered when year control, of prospective start date, is exited. */
void __fastcall TfrmAnalysis::edtProspectiveStartDateExit(TObject *Sender) {
  ValidateDate(*edtProspectiveStartDateYear, *edtProspectiveStartDateMonth, *edtProspectiveStartDateDay);
}

/** event triggered when results file edit control text changes */
void __fastcall TfrmAnalysis::edtResultFileChange(TObject *Sender){
  edtResultFile->Hint = edtResultFile->Text;
}

/** event triggered when year control, of study period end date, is exited. */
void __fastcall TfrmAnalysis::edtStudyPeriodEndDateExit(TObject *Sender) {
  ValidateDate(*edtStudyPeriodEndDateYear, *edtStudyPeriodEndDateMonth, *edtStudyPeriodEndDateDay);
}

/** event triggered when year control, of study period start date, is exited. */
void __fastcall TfrmAnalysis::edtStudyPeriodStartDateExit(TObject *Sender) {
  ValidateDate(*edtStudyPeriodStartDateYear, *edtStudyPeriodStartDateMonth, *edtStudyPeriodStartDateDay);
}

/** event triggered when month control, of prospective start date, is exited. */
void __fastcall TfrmAnalysis::edtTimeIntervalLengthExit(TObject *Sender) {
  if (edtTimeIntervalLength->Text.IsEmpty() || atoi(edtTimeIntervalLength->Text.c_str()) < 1)
    edtTimeIntervalLength->Text = 1;
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



/** enabled study period and prospective date precision based on time interval unit */
void TfrmAnalysis::EnableDatesByTimeIntervalUnits() {
  AnalysisType eAnalysisType(GetAnalysisControlType());

  if (eAnalysisType == PURELYSPATIAL) {
    EnableStudyPeriodDates(true, true, true);
    gpfrmAdvancedParameters->SetRangeDateEnables(true, true, true);
  }
  else if (rbUnitYear->Checked) {
    EnableStudyPeriodDates(true, false, false);
    edtProspectiveStartDateMonth->Text = edtStudyPeriodEndDateMonth->Text;
    edtProspectiveStartDateDay->Text = edtStudyPeriodEndDateDay->Text;
    gpfrmAdvancedParameters->SetRangeDateEnables(true, false, false);
  }
  else if (rbUnitMonths->Checked) {
    EnableStudyPeriodDates(true, true, false);
    edtProspectiveStartDateDay->Text = DaysThisMonth(atoi(edtProspectiveStartDateYear->Text.c_str()), atoi(edtProspectiveStartDateMonth->Text.c_str()));
    gpfrmAdvancedParameters->SetRangeDateEnables(true, true, false);
  }
  else if (rbUnitDay->Checked) {
    EnableStudyPeriodDates(true, true, true);
    gpfrmAdvancedParameters->SetRangeDateEnables(true, true, true);
  }
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
   EnableProspectiveStartDate(bEnable);
}

void TfrmAnalysis::EnableSettingsForAnalysisModelCombination() {
  bool  bPoisson(GetModelControlType() == POISSON),
        bSpaceTimePermutation(GetModelControlType() == SPACETIMEPERMUTATION);

  try {

    switch (GetAnalysisControlType()) {
      case PURELYSPATIAL             :
        gpfrmAdvancedParameters->EnableAdjustmentForTimeTrendOptionsGroup(false, false, false);
        gpfrmAdvancedParameters->EnableInputFilesGroup(true);
        EnableSpatialOptionsGroup(true, false);
        EnableTimeIntervalUnitsGroup(false);
        EnableTemporalOptionsGroup(false, false, false);
        break;
      case PURELYTEMPORAL            :
        gpfrmAdvancedParameters->EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson);
        gpfrmAdvancedParameters->EnableInputFilesGroup(false);
        EnableSpatialOptionsGroup(false, false);
        EnableTimeIntervalUnitsGroup(true);
        EnableTemporalOptionsGroup(true, false, true);
        break;
      case SPACETIME                 :
        gpfrmAdvancedParameters->EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, bPoisson, bPoisson);
        gpfrmAdvancedParameters->EnableInputFilesGroup(true);
        EnableSpatialOptionsGroup(true, !bSpaceTimePermutation);
        EnableTimeIntervalUnitsGroup(true);
        EnableTemporalOptionsGroup(true, !bSpaceTimePermutation, true);
        break;
      case PROSPECTIVESPACETIME      :
        gpfrmAdvancedParameters->EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, bPoisson, bPoisson);
        gpfrmAdvancedParameters->EnableInputFilesGroup(true);
        EnableSpatialOptionsGroup(true, !bSpaceTimePermutation);
        EnableTimeIntervalUnitsGroup(true);
        EnableTemporalOptionsGroup(true, !bSpaceTimePermutation, false);
        break;
      case PROSPECTIVEPURELYTEMPORAL :
        gpfrmAdvancedParameters->EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson);
        gpfrmAdvancedParameters->EnableInputFilesGroup(false);
        EnableSpatialOptionsGroup(false, false);
        EnableTimeIntervalUnitsGroup(true);
        EnableTemporalOptionsGroup(true, false, false);
        break;
      default : ZdGenerateException("Unknown analysis type '%d'.", "OnAnalysisTypeClick()", GetAnalysisControlType());
    }
    EnableAdditionalOutFilesOptionsGroup(!bSpaceTimePermutation);
    gpfrmAdvancedParameters->EnableAdjustmentsGroup(bPoisson);
  }
  catch (ZdException &x) {
    x.AddCallpath("EnableSettingsForAnalysisModelCombination()","TfrmAnalysis");
    throw;
  }
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
   gpfrmAdvancedParameters->EnableSpatialOutputOptions(bEnable);
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
void TfrmAnalysis::EnableTemporalOptionsGroup(bool bEnable, bool bEnableIncludePurelySpatial, bool bEnableRanges) {
  rdgTemporalOptions->Enabled = bEnable;
  rdoPercentageTemproal->Enabled = bEnable;
  rdoTimeTemproal->Enabled = bEnable;
  chkIncludePurSpacClust->Enabled = bEnable && bEnableIncludePurelySpatial;
  edtMaxTemporalClusterSize->Enabled = bEnable;
  edtMaxTemporalClusterSize->Color = bEnable ? clWindow : clInactiveBorder;
  lblMaxTemporalClusterSize->Enabled = bEnable;
  gpfrmAdvancedParameters->EnableTemporalOptions(bEnable, bEnableRanges);
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
  else
    ZdGenerateException("Analysis type not selected.","GetAnalysisControlType()");

  return eReturn;
}

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

/** Returns parameter filename fullpath. */
const char * TfrmAnalysis::GetFileName() {
  return gParameters.GetSourceFileName().c_str();
}

/** returns probability type for model control group */
ProbabiltyModelType TfrmAnalysis::GetModelControlType() const {
  ProbabiltyModelType   eReturn;

  if (rdoPoissonModel->Checked)
    eReturn = POISSON;
  else if (rdoBernoulliModel->Checked)
    eReturn = BERNOULLI;
  else if (rdoSpaceTimePermutationModel->Checked)
    eReturn = SPACETIMEPERMUTATION;
  else
    ZdGenerateException("Probability model type not selected.","GetModelControlType()");

  return eReturn;
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
  gpfrmAdvancedParameters = 0;
}

/** returns whether replications are correct */
bool TfrmAnalysis::IsValidReplicationRequest(int iReplications) {
  return  (iReplications == 0 || iReplications == 9 || iReplications == 19 || fmod(iReplications+1, 1000) == 0.0);
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
  try {
    switch (GetAnalysisControlType()) {
      case PURELYSPATIAL             :
      case PURELYTEMPORAL            :
      case PROSPECTIVEPURELYTEMPORAL : rdoSpaceTimePermutationModel->Enabled = false;
                                       if (rdoSpaceTimePermutationModel->Checked)
                                          rdoPoissonModel->Checked = true;
                                       break;
      case SPACETIME                 :
      case PROSPECTIVESPACETIME      : rdoSpaceTimePermutationModel->Enabled = true; break;
      default : ZdGenerateException("Unknown analysis type '%d'.", "OnAnalysisTypeClick()", GetAnalysisControlType());
    }
    EnableSettingsForAnalysisModelCombination();
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
    //disable analyses that don't match precision
    rdoRetrospectivePurelySpatial->Enabled = true;
    rdoRetrospectivePurelyTemporal->Enabled = eDatePrecisionType != NONE;
    rdoRetrospectiveSpaceTime->Enabled = eDatePrecisionType != NONE;
    rdoProspectivePurelyTemporal->Enabled = eDatePrecisionType != NONE;
    rdoProspectiveSpaceTime->Enabled = eDatePrecisionType != NONE;

   // switch analysis type to purely spatial if no dates in input data
   if (eDatePrecisionType == NONE && GetAnalysisControlType() != PURELYSPATIAL)
     SetAnalysisControl(PURELYSPATIAL);
  }
  catch(ZdException &x) {
    x.AddCallpath("OnPrecisionTimesClick()","TfrmAnalysis");
    throw;
  }
}

/** method called in response to 'probability model' radio group click event */
void TfrmAnalysis::OnProbabilityModelClick() {
  try {
    switch (GetModelControlType()) {
      case POISSON   		: 
      case BERNOULLI            : rdoPercentageTemproal->Caption = "Percent of Study Period (<= 90%)";
                                  lblSimulatedLogLikelihoodRatios->Caption = "Simulated Log Likelihood Ratios";
                                  break;
      case SPACETIMEPERMUTATION : rdoPercentageTemproal->Caption = "Percent of Study Period (<= 50%)";
                                  lblSimulatedLogLikelihoodRatios->Caption = "Simulated Test Statistics";
                                  break;
      default : ZdGenerateException("Unknown probabilty model '%d'.", "OnProbablityModelClick()", GetModelControlType());
    }
    EnableSettingsForAnalysisModelCombination();
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
void __fastcall TfrmAnalysis::rdoProbabilityModelClick(TObject *Sender) {
  try {
     OnProbabilityModelClick();
  }
  catch ( ZdException & x ) {
    x.AddCallpath("rgProbabilityClick()", "TfrmAnalysis" );
    DisplayBasisException( this, x );
  }
}

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

/** launches 'save as' dialog to permit user saving current settings to parameter file */
void TfrmAnalysis::SaveAs() {
  try {
    if (SaveDialog->Execute()) {
      WriteSession(SaveDialog->FileName.c_str());
      Caption = SaveDialog->FileName;
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
    gParameters.SetCoordinatesType((CoordinatesType)rgCoordinates->ItemIndex);
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
    gParameters.SetNumberMonteCarloReplications(atoi(edtMontCarloReps->Text.c_str()));
    //Scanning Window Tab
    gParameters.SetMaximumGeographicClusterSize(atof(edtMaxSpatialClusterSize->Text.c_str()));
    gParameters.SetMaximumSpacialClusterSizeType(rdoSpatialPercentage->Checked ? PERCENTAGEOFMEASURETYPE : DISTANCETYPE);
    gParameters.SetMaximumTemporalClusterSize(atof(edtMaxTemporalClusterSize->Text.c_str()));
    gParameters.SetMaximumTemporalClusterSizeType(rdoPercentageTemproal->Checked ? PERCENTAGETYPE : TIMETYPE);
    gParameters.SetIncludePurelyTemporalClusters(chkInclPurTempClust->Enabled && chkInclPurTempClust->Checked);
    gParameters.SetIncludePurelySpatialClusters(chkIncludePurSpacClust->Enabled && chkIncludePurSpacClust->Checked);
    //Time Parameter Tab
    gParameters.SetTimeIntervalUnitsType(GetTimeIntervalControlType());
    gParameters.SetTimeIntervalLength(atoi(edtTimeIntervalLength->Text.c_str()));
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
    gpfrmAdvancedParameters->SaveParameterSettings();
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveParameterSettings()","TfrmAnalysis");
    throw;
  }
}

/** Sets adjustment by relative risks filename in interface */
void TfrmAnalysis::SetAdjustmentsByRelativeRisksFile(const char * sAdjustmentsByRelativeRisksFileName) {
  gpfrmAdvancedParameters->SetAdjustmentsByRelativeRisksFile(sAdjustmentsByRelativeRisksFileName);
}

/** sets analysis type control for AnalysisType */
void TfrmAnalysis::SetAnalysisControl(AnalysisType eAnalysisType) {
  switch (eAnalysisType) {
    case PURELYTEMPORAL                 : rdoRetrospectivePurelyTemporal->Checked = true; break;
    case SPACETIME                      : rdoRetrospectiveSpaceTime->Checked = true; break;
    case PROSPECTIVEPURELYTEMPORAL      : rdoProspectivePurelyTemporal->Checked = true; break;
    case PROSPECTIVESPACETIME           : rdoProspectiveSpaceTime->Checked = true; break;
    case SPATIALVARTEMPTREND            :
    case PURELYSPATIALMONOTONE          : 
    case PURELYSPATIAL                  :
    default                             : rdoRetrospectivePurelySpatial->Checked = true;
  }
}

/** sets area scan rate type control */
void TfrmAnalysis::SetAreaScanRateControl(AreaRateType eAreaRateType) {
  switch (eAreaRateType) {
    case LOW        : rdoLowRates->Checked = true; break;
    case HIGHANDLOW : rdoHighLowRates->Checked = true; break;
    case HIGH       : 
    default         : rdoHighRates->Checked = true;
  }
}

/** Sets case filename in interface */
void TfrmAnalysis::SetCaseFile(const char * sCaseFileName) {
  edtCaseFileName->Text = sCaseFileName;
}

/** Sets control filename in interface */
void TfrmAnalysis::SetControlFile(const char * sControlFileName) {
  edtControlFileName->Text = sControlFileName;
}

/** Sets coordinates filename in interface */
void TfrmAnalysis::SetCoordinateFile(const char * sCoordinateFileName) {
  edtCoordinateFileName->Text = sCoordinateFileName;
}

/** sets coordinate type */
void TfrmAnalysis::SetCoordinateType(CoordinatesType eCoordinatesType) {
  rgCoordinates->ItemIndex = eCoordinatesType;
}

/** Sets special population filename in interface */
void TfrmAnalysis::SetMaximumCirclePopulationFile(const char * sMaximumCirclePopulationFileName) {
  gpfrmAdvancedParameters->SetMaximumCirclePopulationFile(sMaximumCirclePopulationFileName);
}

/** sets probaiity model type control for ProbabiltyModelType */
void TfrmAnalysis::SetModelControl(ProbabiltyModelType eProbabiltyModelType) {
  switch (eProbabiltyModelType) {
    case BERNOULLI            : rdoBernoulliModel->Checked = true; break;
    case SPACETIMEPERMUTATION : rdoSpaceTimePermutationModel->Checked = true; break;
    case POISSON              : 
    default                   : rdoPoissonModel->Checked = true;
  }
}

/** Sets population filename in interface */
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
    sTemp.printf("percent of population at risk\n        (<= %s%%)", edtMaxSpatialClusterSize->Text.c_str());
  else if (rgCoordinates->ItemIndex == CARTESIAN)
    sTemp.printf("cartesian units in radius\n        (<= %s)", edtMaxSpatialClusterSize->Text.c_str());
  else
    sTemp.printf("kilometers in radius\n        (<= %s)", edtMaxSpatialClusterSize->Text.c_str());

  gpfrmAdvancedParameters->SetReportingClustersText(sTemp);
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

/** Sets special grid filename in interface */
void TfrmAnalysis::SetSpecialGridFile(const char * sSpecialGridFileName) {
  gParameters.SetSpecialGridFileName(sSpecialGridFileName, false, true);
  edtGridFileName->Text = sSpecialGridFileName;
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
    gpfrmAdvancedParameters = new TfrmAdvancedParameters(*this);
    //Input File Tab
    Caption = gParameters.GetSourceFileName().c_str();
    edtCaseFileName->Text = gParameters.GetCaseFileName().c_str();
    edtControlFileName->Text = gParameters.GetControlFileName().c_str();
    SetPrecisionOfTimesControl(gParameters.GetPrecisionOfTimesType());
    edtPopFileName->Text = gParameters.GetPopulationFileName().c_str();
    edtCoordinateFileName->Text = gParameters.GetCoordinatesFileName().c_str();
    edtGridFileName->Text = gParameters.GetSpecialGridFileName().c_str();
    rgCoordinates->ItemIndex = gParameters.GetCoordinatesType();
    //Analysis Tab
    SetAnalysisControl(gParameters.GetAnalysisType());
    SetModelControl(gParameters.GetProbabiltyModelType());
    SetAreaScanRateControl(gParameters.GetAreaScanRateType());
    ParseDate(gParameters.GetStudyPeriodStartDate().c_str(), edtStudyPeriodStartDateYear, edtStudyPeriodStartDateMonth, edtStudyPeriodStartDateDay);
    ParseDate(gParameters.GetStudyPeriodEndDate().c_str(), edtStudyPeriodEndDateYear, edtStudyPeriodEndDateMonth, edtStudyPeriodEndDateDay);
    if (IsValidReplicationRequest(gParameters.GetNumReplicationsRequested()))
      edtMontCarloReps->Text = gParameters.GetNumReplicationsRequested();
    else
      edtMontCarloReps->Text = 999;
    //Scanning Window Tab
    if (gParameters.GetMaximumGeographicClusterSize() > 0)
      edtMaxSpatialClusterSize->Text = gParameters.GetMaximumGeographicClusterSize();
    else
      edtMaxSpatialClusterSize->Text = 50;
    chkInclPurTempClust->Checked = gParameters.GetIncludePurelyTemporalClusters();
    if (gParameters.GetMaximumTemporalClusterSize() > 0)
      edtMaxTemporalClusterSize->Text = gParameters.GetMaximumTemporalClusterSize();
    else
      edtMaxTemporalClusterSize->Text = 50;
    rdoPercentageTemproal->Checked = gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE;
    rdoTimeTemproal->Checked = gParameters.GetMaximumTemporalClusterSizeType() == TIMETYPE;
    chkIncludePurSpacClust->Checked = gParameters.GetIncludePurelySpatialClusters();
    rdoSpatialPercentage->Checked = gParameters.GetMaxGeographicClusterSizeType() != DISTANCETYPE; // default checked
    rdoSpatialDistance->Checked = gParameters.GetMaxGeographicClusterSizeType() == DISTANCETYPE;
    SetSpatialDistanceCaption();
    SetReportingSmallerClustersText();
    //Time Parameter Tab
    if (gParameters.GetTimeIntervalUnitsType() == NONE) gParameters.SetTimeIntervalUnitsType(YEAR);
    if (gParameters.GetTimeIntervalLength() <= 0) gParameters.SetTimeIntervalLength(1);
    rbUnitYear->Checked = (gParameters.GetTimeIntervalUnitsType() == YEAR);
    rbUnitMonths->Checked = (gParameters.GetTimeIntervalUnitsType() == MONTH);
    rbUnitDay->Checked = (gParameters.GetTimeIntervalUnitsType() == DAY);  
    edtTimeIntervalLength->Text = gParameters.GetTimeIntervalLength();
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
  }
  catch (ZdException & x) {
    x.AddCallpath("SetupInterface()", "TfrmAnalysis");
    delete gpfrmAdvancedParameters; gpfrmAdvancedParameters=0;
    throw;
  }
}

/** Modally shows advanced features dialog. */
void TfrmAnalysis::ShowAdvancedFeaturesDialog() {
  try {
    gpfrmAdvancedParameters->ShowDialog();
  }
  catch (ZdException & x) {
    x.AddCallpath("ShowAdvancedFeatureDialog()", "TfrmAnalysis");
    throw;
  }
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

/** Validates 'Input Files' tab */
void TfrmAnalysis::ValidateInputFiles() {
  try {
    //validate the case file
    if (edtCaseFileName->Text.IsEmpty()) {
      PageControl1->ActivePage = tbInputFiles;
      edtCaseFileName->SetFocus();
      ZdException::GenerateNotification("Please specify a case file.","ValidateInputFiles()");
    }
    if (!File_Exists(edtCaseFileName->Text.c_str())) {
      PageControl1->ActivePage = tbInputFiles;
      edtCaseFileName->SetFocus();
      ZdException::GenerateNotification("Case file could not be opened.","ValidateInputFiles()");
    }
    //validate the control file - Bernoulli model only
    if (GetModelControlType() == BERNOULLI) {
      if (edtControlFileName->Text.IsEmpty()) {
        PageControl1->ActivePage = tbInputFiles;
        edtControlFileName->SetFocus();
        ZdException::GenerateNotification("For the Bernoulli model, please specify a control file.","ValidateInputFiles()");
      }
      if (!File_Exists(edtControlFileName->Text.c_str())) {
        PageControl1->ActivePage = tbInputFiles;
        edtControlFileName->SetFocus();
        ZdException::GenerateNotification("Control file could not be opened.","ValidateInputFiles()");
      }
    }
    //validate the population file -  Poisson model only
    if (GetModelControlType() == POISSON) {
      if (edtPopFileName->Text.IsEmpty()) {
        PageControl1->ActivePage = tbInputFiles;
        edtPopFileName->SetFocus();
        ZdException::GenerateNotification("For the Poisson model, please specify a population file.","ValidateInputFiles()");
      }
      if (!File_Exists(edtPopFileName->Text.c_str())) {
        PageControl1->ActivePage = tbInputFiles;
        edtPopFileName->SetFocus();
        ZdException::GenerateNotification("Population file could not be opened.","ValidateInputFiles()");
      }
    }
    //validate coordinates file
    if (edtCoordinateFileName->Text.IsEmpty()) {
      PageControl1->ActivePage = tbInputFiles;
      edtCoordinateFileName->SetFocus();
      ZdException::GenerateNotification("Please specify a coordinates file.","ValidateInputFiles()");
    }
    if (!File_Exists(edtCoordinateFileName->Text.c_str())) {
      PageControl1->ActivePage = tbInputFiles;
      edtCoordinateFileName->SetFocus();
      ZdException::GenerateNotification("Coordinates file could not be opened.","ValidateInputFiles()");
    }
    //validate special grid file -- optional
    if (!edtGridFileName->Text.IsEmpty() &&  !File_Exists(edtGridFileName->Text.c_str())) {
      PageControl1->ActivePage = tbInputFiles;
      edtGridFileName->SetFocus();
      ZdException::GenerateNotification("Special Grid file could not be opened.","ValidateInputFiles()");
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateInputFiles()", "TfrmAnalysis");
    throw;
  }
}

/** This function is used right before a job is submitted.  Verifies that
    all the input files exist and can be read.  Also checks each tab to see
    if all settings are in place.                                          */
bool TfrmAnalysis::ValidateParams() {
  bool bReturn=true;

  try {
    ValidateInputFiles();
    CheckAnalysisParams();
    CheckScanningWindowParams();
    CheckTimeParams();
    CheckOutputParams();
    gpfrmAdvancedParameters->ValidateAdjustmentSettings();
    gpfrmAdvancedParameters->ValidateInputFilesSettings();
    gpfrmAdvancedParameters->ValidateScanningWindowSettings();
  }
  catch (AdvancedFeaturesException &x) {
    x.AddCallpath("ValidateParams()","TfrmAnalysis");
    bReturn = false;
    DisplayBasisException(this, x);
    gpfrmAdvancedParameters->ShowDialog(&x.GetFocusControl());
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateParams()","TfrmAnalysis");
    bReturn = false;
    DisplayBasisException(this, x);
  }
  return bReturn;
}

void TfrmAnalysis::ValidateSpatialClusterSize() {
  try {
    if (edtMaxSpatialClusterSize->Enabled) {
      if (!edtMaxSpatialClusterSize->Text.Length() || atof(edtMaxSpatialClusterSize->Text.c_str()) == 0)
        ZdException::GenerateNotification("Please specify a maximum spatial cluster size greater than zero.","ValidateSpatialClusterSize()");

      if (atof(edtMaxSpatialClusterSize->Text.c_str()) > 50.0 && rdoSpatialPercentage->Checked)
        ZdException::GenerateNotification("Please specify a maximum spatial cluster size no greater than %d.",
                                          "ValidateSpatialClusterSize()", 50);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateSpatialClusterSize()","TfrmAnalysis");
    PageControl1->ActivePage = tbScanningWindow;
    edtMaxSpatialClusterSize->SetFocus();
    throw;
  }
}

void TfrmAnalysis::ValidateTemoralClusterSize() {
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
        if (!(dValue > 0.0 && dValue <= (GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90))) {
          sErrorMessage << "For the " << gParameters.GetProbabiltyModelTypeAsString(GetModelControlType());
          sErrorMessage << " model, the maximum temporal cluster size, as a percent of study period, is ";
          sErrorMessage << (GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90) << " percent.";
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
        ulMaxClusterDays = (GetModelControlType() == SPACETIMEPERMUTATION ? ulMaxClusterDays * 0.5 : ulMaxClusterDays * 0.9);

        StartPlusIntervalDate = StartDate;
        //add time interval length as units to modified start date
        switch (GetTimeIntervalControlType()) {
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
            default               : ZdGenerateException("Unknown interval unit \"%d\"", "ValidateTemoralClusterSize()", GetTimeIntervalControlType());
        };
        ulIntervalLengthInDays = StartPlusIntervalDate.GetJulianDayFromCalendarStart() - StartDate.GetJulianDayFromCalendarStart();
        if (ulIntervalLengthInDays > ulMaxClusterDays) {
          DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), StartDate.GetRawDate());
          sErrorMessage << "For the study period starting on " << FilterBuffer << " and ending on ";
          DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), EndDate.GetRawDate());
          sErrorMessage << FilterBuffer << ",\na maximum temporal cluster size of " << edtMaxTemporalClusterSize->Text.c_str();
          sErrorMessage << " " << Buffer << " is greater than " << (GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90);
          sErrorMessage << " percent of study period.";
          ZdException::GenerateNotification(sErrorMessage.GetCString(), "ValidateTemoralClusterSize()");
        }
      }
      else
        ZdException::GenerateNotification("Type specified as neither percentage nor time.", "ValidateTemoralClusterSize()");
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateTemoralClusterSize()","TfrmAnalysis");
    PageControl1->ActivePage = tbScanningWindow;
    edtMaxTemporalClusterSize->SetFocus();
    throw;
  }
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

//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMaxSpatialClusterSizeChange(TObject *Sender) {
  if (edtMaxSpatialClusterSize->Text.Length())
    SetReportingSmallerClustersText();
}


