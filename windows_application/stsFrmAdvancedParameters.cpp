//******************************************************************************
#include "stsSaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsFrmAdvancedParameters.h"
#pragma resource "*.dfm"

/** Maximum number of additional input datasets permitted. */
const int TfrmAdvancedParameters::MAXIMUM_ADDITIONAL_SETS = 11;
//---------------------------------------------------------------------------
/** class constructor */
__fastcall TfrmAdvancedParameters::TfrmAdvancedParameters(const TfrmAnalysis& AnalysisSettings)
        : TForm(const_cast<TfrmAnalysis*>(&AnalysisSettings)), gAnalysisSettings(AnalysisSettings) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
/** event triggered when selects browse button for maximum circle population file */
void __fastcall TfrmAdvancedParameters::btnBrowseMaxCirclePopFileClick(TObject *Sender) {
  try {
    OpenDialog->FileName = "";
    OpenDialog->DefaultExt = "*.pop";
    OpenDialog->Filter = "Max Circle Size files (*.max)|*.max|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog->FilterIndex = 0;
    OpenDialog->Title = "Select Max Circle Size File";
    if (OpenDialog->Execute())
      edtMaxCirclePopulationFilename->Text = OpenDialog->FileName;
  }
  catch (ZdException & x) {
    x.AddCallpath("btnBrowseMaxCirclePopFileClick()","TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
/** button click event for case file browse
    - shows open dialog and sets appropriate case file interface controls */
void __fastcall TfrmAdvancedParameters::btnCaseBrowseClick(TObject *Sender) {
  try {
    OpenDialog->FileName =  "";
    OpenDialog->DefaultExt = "*.cas";
    OpenDialog->Filter = "Case files (*.cas)|*.cas|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog->FilterIndex = 0;
    OpenDialog->Title = "Select Case File";
    if (OpenDialog->Execute())
      edtCaseFileName->Text = OpenDialog->FileName.c_str();
  }
  catch (ZdException & x) {
    x.AddCallpath("btnCaseBrowseClick()", "TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
/** Event triggered when user selects button to import neighbors file. A file selection
    dialog is display and if user selects a file, the file import dialog is launched. */
void __fastcall TfrmAdvancedParameters::btnBrowseForNeighborsFileClick(TObject *Sender) {
  try {
    OpenDialog->FileName = "";
    OpenDialog->DefaultExt = "*.rr";
    OpenDialog->Filter = "Neighbors files (*.nei)|*.nei|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog->FilterIndex = 0;
    OpenDialog->Title = "Select Neighbors File";
    if (OpenDialog->Execute())
      edtNeighborsFile->Text = OpenDialog->FileName;
  }
  catch (ZdException &x) {
    x.AddCallpath("btnBrowseForNeighborsFileClick()","TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
/** button click event for case file import    */
void __fastcall TfrmAdvancedParameters::btnCaseImportClick(TObject *Sender) {
  InputFileType eType = Case;

  try {
    OpenDialog->FileName =  "";
    OpenDialog->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Case files (*.cas)|*.cas|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog->Title = "Select Source Case File";
    if (OpenDialog->Execute())
       LaunchImporter(OpenDialog->FileName.c_str(), eType);
  }
  catch (ZdException & x) {
    x.AddCallpath("btnCaseImportClick()", "TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
/** button click event for control file browse
    - shows open dialog and sets appropriate control file interface controls */
void __fastcall TfrmAdvancedParameters::btnControlBrowseClick(TObject *Sender) {
  try {
    OpenDialog->FileName = "";
    OpenDialog->DefaultExt = "*.ctl";
    OpenDialog->Filter = "Control files (*.ctl)|*.ctl|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog->FilterIndex = 0;
    OpenDialog->Title = "Select Control File";
    if (OpenDialog->Execute())
       edtControlFileName->Text = OpenDialog->FileName.c_str();
  }
  catch (ZdException & x) {
    x.AddCallpath("btnControlBrowseClick()", "TfrmAdvancedParamters");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
/** button click event for case file import    */
void __fastcall TfrmAdvancedParameters::btnControlImportClick(TObject *Sender) {
  InputFileType eType = Control;

  try {
    OpenDialog->FileName =  "";
    OpenDialog->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Control files (*.ctl)|*.ctl|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog->Title = "Select Source Control File";
    if (OpenDialog->Execute())
       LaunchImporter(OpenDialog->FileName.c_str(), eType);
  }
  catch (ZdException & x) {
    x.AddCallpath("btnControlImportClick()", "TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
/** event triggered when selects import button for adjustment for relative risks file */
void __fastcall TfrmAdvancedParameters::btnImportAdjustmentsFileClick(TObject *Sender) {
  InputFileType eType = AdjustmentsByRR;

  try {
    OpenDialog->FileName = "";
    OpenDialog->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Adjustments files (*.adj)|*.adj|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog->FilterIndex = 0;
    OpenDialog->Title = "Select Source Adjustments File";
    if (OpenDialog->Execute())
       LaunchImporter(OpenDialog->FileName.c_str(), eType);
  }
  catch (ZdException &x) {
    x.AddCallpath("btnImportAdjustmentsFileClick()","TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
/** event triggered when selects import button for maximum circle population file */
void __fastcall TfrmAdvancedParameters::btnImportMaxCirclePopFileClick(TObject *Sender) {
  InputFileType eType = MaxCirclePopulation;

  try {
    OpenDialog->FileName = "";
    OpenDialog->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Max Circle Size files (*.max)|*.max|Population files (*.pop)|*.pop|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog->Title = "Select Source Max Circle Size File";
    if (OpenDialog->Execute())
       LaunchImporter(OpenDialog->FileName.c_str(), eType);
  }
  catch (ZdException &x) {
    x.AddCallpath("btnImportMaxCirclePopFileClick()","TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAdvancedParameters::btnNewClick(TObject *Sender) {
   try {
     // add new name to list box
     EnableDataSetList(true);
     EnableDataSetPurposeControls(true);
     lstInputDataSets->Items->Add("Data Set " + IntToStr(giDataSetNum++));
     lstInputDataSets->ItemIndex = (lstInputDataSets->Items->Count-1);

     // enable and clear the edit boxes
     EnableInputFileEdits(true);
     gvCaseFiles.push_back("");
     gvControlFiles.push_back("");
     gvPopFiles.push_back("");
     edtCaseFileName->Text = "";
     edtControlFileName->Text = "";
     edtPopFileName->Text = "";
     edtCaseFileName->SetFocus();
     EnableNewButton();
     EnableRemoveButton();
     DoControlExit();
   }
   catch (ZdException &x) {
     x.AddCallpath("btnNewClick()","TfrmAdvancedParameters");
     EnableDataSetList(lstInputDataSets->Items->Count);
     EnableDataSetPurposeControls(lstInputDataSets->Items->Count);
     DisplayBasisException(this, x);
   }
}
//---------------------------------------------------------------------------
/** button click event for population file browse
    - shows open dialog and sets appropriate population file interface controls */
void __fastcall TfrmAdvancedParameters::btnPopBrowseClick(TObject *Sender) {
  try {
    OpenDialog->FileName = "";
    OpenDialog->DefaultExt = "*.pop";
    OpenDialog->Filter = "Population files (*.pop)|*.pop|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog->FilterIndex = 0;
    OpenDialog->Title = "Select Population File";
    if (OpenDialog->Execute())
      edtPopFileName->Text = OpenDialog->FileName.c_str();
  }
  catch (ZdException & x) {
    x.AddCallpath("btnPopBrowseClick()", "TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
/** button click event for case file import    */
void __fastcall TfrmAdvancedParameters::btnPopImportClick(TObject *Sender) {
  InputFileType eType = Population;

  try {
    OpenDialog->FileName =  "";
    OpenDialog->Title = "Select Source Population File";
    OpenDialog->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Population files (*.pop)|*.pop|Text files (*.txt)|*.txt|All files (*.*)|*.*";
    if (OpenDialog->Execute())
       LaunchImporter(OpenDialog->FileName.c_str(), eType);
  }
  catch (ZdException & x) {
    x.AddCallpath("btnPopImportClick()", "TfrmAdvancedParameters");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
// when user clicks on an input dataset name, and the Remove button, remove the
// details and the input dataset name
void __fastcall TfrmAdvancedParameters::btnRemoveDataSetClick(TObject *Sender){
   int iDataSetNum = -1;

   try {
      // determine the input dataset selected
      iDataSetNum = lstInputDataSets->ItemIndex;
      // remove files
      edtCaseFileName->Text = "";
      gvCaseFiles.erase(gvCaseFiles.begin() + iDataSetNum);
      edtControlFileName->Text = "";
      gvControlFiles.erase(gvControlFiles.begin() + iDataSetNum);
      edtPopFileName->Text = "";
      gvPopFiles.erase(gvPopFiles.begin() + iDataSetNum);

      // update remaining list box names
      for (int i=iDataSetNum+1; i < lstInputDataSets->Items->Count ;i++) {
         AnsiString s = (lstInputDataSets->Items->Strings[i]);
         int num = s.SubString(10, 2).ToInt();
         lstInputDataSets->Items->Strings[i] = ("Data Set " + IntToStr(--num));
      }
      // remove list box name
      lstInputDataSets->Items->Delete(iDataSetNum);
      EnableDataSetList(lstInputDataSets->Items->Count);
      EnableDataSetPurposeControls(lstInputDataSets->Items->Count);
      // select/highlight previous name in box
      if (lstInputDataSets->Items->Count) {
         iDataSetNum = (iDataSetNum > 0) ? iDataSetNum-1 : 0;
         lstInputDataSets->ItemIndex = iDataSetNum;
         lstInputDataSets->OnClick(this);
      }
      else
         EnableInputFileEdits(false);
      giDataSetNum--;

      EnableNewButton();
      EnableRemoveButton();
      DoControlExit();
   }
   catch (ZdException &x) {
     x.AddCallpath("btnRemoveDataSetClick()","TfrmAdvancedParameters");
     DisplayBasisException(this, x);
   }
}
//---------------------------------------------------------------------------
// Resets advanced settings to default values
void __fastcall TfrmAdvancedParameters::btnSetDefaultsClick(TObject *Sender)
{
   switch (giCategory) {
      case INPUT_TABS:
         SetDefaultsForInputTab();
         break;
      case ANALYSIS_TABS:
         SetDefaultsForAnalysisTabs();
         break;
      case OUTPUT_TABS:
         SetDefaultsForOutputTab();
         break;
      }
   btnSetDefaults->Enabled = false;  // defaults are set so can't choose button again
}
//---------------------------------------------------------------------------
void __fastcall TfrmAdvancedParameters::btnShowAllClick(TObject *Sender) {
   for (int i=0; i < PageControl->PageCount; i++)
      PageControl->Pages[i]->TabVisible=true;
}
//---------------------------------------------------------------------------
/** event triggered when 'adjustment for ealier analyses' checkbox if clicked */
void __fastcall TfrmAdvancedParameters::chkAdjustForEarlierAnalysesClick(TObject *Sender) {
  EnableSettingsForAnalysisModelCombination();
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when adjust for known relative risks is clicked */
void __fastcall TfrmAdvancedParameters::chkAdjustForKnownRelativeRisksClick(TObject *Sender) {
  bool  bEnabled = grpAdjustments->Enabled && chkAdjustForKnownRelativeRisks->Checked;

  lblAdjustmentsByRelativeRisksFile->Enabled = bEnabled;
  edtAdjustmentsByRelativeRisksFile->Enabled = bEnabled;
  edtAdjustmentsByRelativeRisksFile->Color = bEnabled ? clWindow : clInactiveBorder;
  btnBrowseAdjustmentsFile->Enabled = bEnabled;
  btnImportAdjustmentsFile->Enabled = bEnabled;
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when user selects perform sequential scans check box */
void __fastcall TfrmAdvancedParameters::chkPerformSequentialScanClick(TObject *Sender) {
  EnableSettingsForAnalysisModelCombination();
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when user selects restrict reported clusters check box */
void __fastcall TfrmAdvancedParameters::chkRestrictReportedClustersClick(TObject *Sender) {
  EnableReportedSpatialOptionsGroup(rdgSpatialOptions->Enabled);
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when user selects restrict range check box */
void __fastcall TfrmAdvancedParameters::chkRestrictTemporalRangeClick(TObject *Sender) {
  EnableReportedSpatialOptionsGroup(rdgSpatialOptions->Enabled);
  EnableDatesByTimePrecisionUnits();
  DoControlExit();
}
//---------------------------------------------------------------------------
/** Event triggered when user selects check box which enables/disables corresponding
    feature controls. Causes re-evaluation of interface control enabling. */
void __fastcall TfrmAdvancedParameters::chkSpecifiyNeighborsFileClick(TObject *Sender) {
   EnableSettingsForAnalysisModelCombination();
   DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when user exits a control*/
void TfrmAdvancedParameters::DoControlExit() {
   // update enable/disable of Set Defaults button
   switch (giCategory) {
      case INPUT_TABS:
         btnSetDefaults->Enabled = !GetDefaultsSetForInputOptions();
         break;
      case ANALYSIS_TABS:
         btnSetDefaults->Enabled = !GetDefaultsSetForAnalysisOptions();
         break;
      case OUTPUT_TABS:
         btnSetDefaults->Enabled = !GetDefaultsSetForOutputOptions();
         break;
   }
}
//---------------------------------------------------------------------------
/** event triggered when text of adjustment by relative risks edit control changes */
void __fastcall TfrmAdvancedParameters::edtAdjustmentsByRelativeRisksFileChange(TObject *Sender) {
  edtAdjustmentsByRelativeRisksFile->Hint = edtAdjustmentsByRelativeRisksFile->Text;
}
//---------------------------------------------------------------------------
/** Event triggered when TEdit for case file changes. */
void __fastcall TfrmAdvancedParameters::edtCaseFileNameChange(TObject *Sender) {
  gvCaseFiles.at(lstInputDataSets->ItemIndex) = edtCaseFileName->Text;
}
//---------------------------------------------------------------------------
/** Event triggered when TEdit for control file changes. */
void __fastcall TfrmAdvancedParameters::edtControlFileNameChange(TObject *Sender) {
  gvControlFiles.at(lstInputDataSets->ItemIndex) = edtControlFileName->Text;
}
//---------------------------------------------------------------------------
/** event triggered when end window end ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtEndRangeEndDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtEndRangeEndYear, *edtEndRangeEndMonth, *edtEndRangeEndDay);
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when end window start ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtEndRangeStartDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtEndRangeStartYear, *edtEndRangeStartMonth, *edtEndRangeStartDay);
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when loginear percentage control exited */
void __fastcall TfrmAdvancedParameters::edtLogLinearExit(TObject *Sender) {
  if (edtLogLinear->Text.IsEmpty() || edtLogLinear->Text.ToDouble() <= -100)
    edtLogLinear->Text = 0;
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when text of maximum circle edit control changes */
void __fastcall TfrmAdvancedParameters::edtMaxCirclePopulationFilenameChange(TObject *Sender) {
  edtMaxCirclePopulationFilename->Hint = edtMaxCirclePopulationFilename->Text;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAdvancedParameters::edtMaxReportedSpatialClusterSizeExit(TObject *Sender) {
  if (edtMaxReportedSpatialClusterSize->Text.IsEmpty() || edtMaxReportedSpatialClusterSize->Text.ToDouble() == 0)
    edtMaxReportedSpatialClusterSize->Text = 50;
  DoControlExit();
}
//---------------------------------------------------------------------------
void __fastcall TfrmAdvancedParameters::edtMaxReportedSpatialPercentFileExit(TObject *Sender) {
  if (edtMaxReportedSpatialPercentFile->Text.IsEmpty() || edtMaxReportedSpatialPercentFile->Text.ToDouble() == 0)
    edtMaxReportedSpatialPercentFile->Text = 50;
  DoControlExit();
}
//---------------------------------------------------------------------------
void __fastcall TfrmAdvancedParameters::edtMaxReportedSpatialRadiusExit(TObject *Sender) {
  if (edtMaxReportedSpatialRadius->Text.IsEmpty() || edtMaxReportedSpatialRadius->Text.ToDouble() == 0)
    edtMaxReportedSpatialRadius->Text = 1;
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when maximum spatial cluster size edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxSpatialClusterSizeExit(TObject *Sender) {
  if (edtMaxSpatialClusterSize->Text.IsEmpty() || edtMaxSpatialClusterSize->Text.ToDouble() == 0 || edtMaxSpatialClusterSize->Text.ToDouble() > 50.0)
    edtMaxSpatialClusterSize->Text = 50;
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when maximum spatial cluster size, as percentage of population file, edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxSpatialPercentFileExit(TObject *Sender) {
  if (edtMaxSpatialPercentFile->Text.IsEmpty() || edtMaxSpatialPercentFile->Text.ToDouble() == 0 || edtMaxSpatialPercentFile->Text.ToDouble() > 50.0)
    edtMaxSpatialPercentFile->Text = 50;
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when maximum spatial cluster size, as a radius, edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxSpatialRadiusExit(TObject *Sender){
  if (edtMaxSpatialRadius->Text.IsEmpty() || edtMaxSpatialRadius->Text.ToDouble() == 0)
    edtMaxSpatialRadius->Text = 1;
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when maximum temporal cluster size edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxTemporalClusterSizeExit(TObject *Sender) {
  double dMaxValue = gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION ? 50.0 : 90.0;
  if (edtMaxTemporalClusterSize->Text.IsEmpty() || edtMaxTemporalClusterSize->Text.ToDouble() == 0 || edtMaxTemporalClusterSize->Text.ToDouble() > dMaxValue)
    edtMaxTemporalClusterSize->Text = dMaxValue;
  DoControlExit();
}

//---------------------------------------------------------------------------
/** event triggered when maximum temporal cluster size edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxTemporalClusterSizeUnitsExit(TObject *Sender) {
  if (edtMaxTemporalClusterSizeUnits->Text.IsEmpty() || edtMaxTemporalClusterSizeUnits->Text.ToDouble() == 0)
    edtMaxTemporalClusterSizeUnits->Text = 1;
  DoControlExit();
}
//---------------------------------------------------------------------------
/** Event triggered when user changes the text of the neighbors filename control.
    Control hint is updated and DoControlExit(). */
void __fastcall TfrmAdvancedParameters::edtNeighborsFileChange(TObject *Sender) {
  edtNeighborsFile->Hint = edtNeighborsFile->Text;
  DoControlExit();
}
//---------------------------------------------------------------------------
/** Event triggered when TEdit for population file changes. */
void __fastcall TfrmAdvancedParameters::edtPopFileNameChange(TObject *Sender) {
  gvPopFiles.at(lstInputDataSets->ItemIndex) = edtPopFileName->Text;
}
//---------------------------------------------------------------------------
/** event triggered when year control, of prospective start date, is exited. */
void __fastcall TfrmAdvancedParameters::edtNumSequentialScansExit(TObject *Sender) {
  if (edtNumSequentialScans->Text.IsEmpty() ||
      atoi(edtNumSequentialScans->Text.c_str()) < 1 ||
      atoi(edtNumSequentialScans->Text.c_str()) > CParameters::MAXIMUM_SEQUENTIAL_ANALYSES)
    edtNumSequentialScans->Text = 10;
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when start window end ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtStartRangeEndDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtStartRangeEndYear, *edtStartRangeEndMonth, *edtStartRangeEndDay);
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when sequential scan cut off control is exited */
void __fastcall TfrmAdvancedParameters::edtSequentialScanCutoffExit(TObject *Sender) {
  if (edtSequentialScanCutoff->Text.IsEmpty() ||
      edtSequentialScanCutoff->Text.ToDouble() <= 0 || edtSequentialScanCutoff->Text.ToDouble() > 1)
    edtSequentialScanCutoff->Text = ".05";
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when start window start ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtStartRangeStartDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtStartRangeStartYear, *edtStartRangeStartMonth, *edtStartRangeStartDay);
  DoControlExit();
}
//---------------------------------------------------------------------------
void TfrmAdvancedParameters::EnableAdjustmentForSpatialOptionsGroup(bool bEnable, bool bEnableStratified) {
  rdgSpatialAdjustments->Enabled = bEnable;
  bEnableStratified &= bEnable;
  rdgSpatialAdjustments->Controls[1]->Enabled = bEnableStratified;
  if (bEnable && !bEnableStratified && rdgSpatialAdjustments->ItemIndex == 1)
      rdgSpatialAdjustments->ItemIndex = 0;
}
//---------------------------------------------------------------------------
/** enables or disables the temporal time trend adjustment control group */
void TfrmAdvancedParameters::EnableAdjustmentForTimeTrendOptionsGroup(bool bEnable, bool bTimeStratified, bool bLogYearPercentage, bool bCalculatedLog) {
  TimeTrendAdjustmentType eTimeTrendAdjustmentType(GetAdjustmentTimeTrendControlType());

  // trump control enables
  bTimeStratified &= bEnable;
  bLogYearPercentage &= bEnable;
  bCalculatedLog &= bEnable;

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

  rdgTemporalTrendAdj->Controls[3]->Enabled = bCalculatedLog;
  if (bEnable && !bCalculatedLog && eTimeTrendAdjustmentType == CALCULATED_LOGLINEAR_PERC)
    SetTemporalTrendAdjustmentControl(NOTADJUSTED);
}
//---------------------------------------------------------------------------
/** enables adjustment options controls */
void TfrmAdvancedParameters::EnableAdjustmentsGroup(bool bEnable) {
  grpAdjustments->Enabled = bEnable;
  chkAdjustForKnownRelativeRisks->Enabled = bEnable;
  lblAdjustmentsByRelativeRisksFile->Enabled = bEnable && chkAdjustForKnownRelativeRisks->Checked;
  edtAdjustmentsByRelativeRisksFile->Enabled = bEnable && chkAdjustForKnownRelativeRisks->Checked;
  edtAdjustmentsByRelativeRisksFile->Color = edtAdjustmentsByRelativeRisksFile->Enabled ? clWindow : clInactiveBorder;
  btnBrowseAdjustmentsFile->Enabled = bEnable && chkAdjustForKnownRelativeRisks->Checked;
  btnImportAdjustmentsFile->Enabled = bEnable && chkAdjustForKnownRelativeRisks->Checked;
}
//---------------------------------------------------------------------------
/** Enables/disables TListBox that list defined data sets */
void TfrmAdvancedParameters::EnableDataSetList(bool bEnable) {
  lstInputDataSets->Enabled = bEnable;
  lstInputDataSets->Color = lstInputDataSets->Enabled ? clWindow : clInactiveBorder;
}
//---------------------------------------------------------------------------
/** Enables/disables controls that indicate purpose of additional data sets. */
void TfrmAdvancedParameters::EnableDataSetPurposeControls(bool bEnable) {
  lblMultipleDataSetPurpose->Enabled = bEnable;
  rdoMultivariate->Enabled = bEnable;
  rdoAdjustmentByDataSets->Enabled = bEnable;
}
//---------------------------------------------------------------------------
/** Enables dates of flexible temporal window and prospective surveillance groups.
    Enabling is determined through:
    - querying the 'precision of time' control contained in the analysis window
    - the Enabled property of the TGroupBox of which dates are contained
    - the Enabled and Checked properties of the TCheckBox that indicates whether
      user wishes to adjust for earlier analyses. */
void TfrmAdvancedParameters::EnableDatesByTimePrecisionUnits() {
  bool bYears, bMonths, bDays,
       bEnable = grpFlexibleTemporalWindowDefinition->Enabled &&
                 chkRestrictTemporalRange->Enabled && chkRestrictTemporalRange->Checked;

  switch (gAnalysisSettings.GetPrecisionOfTimesControlType()) {
    case NONE  :
    case DAY   : bYears = bMonths = bDays = bEnable; break;
    case YEAR  : bYears = bEnable; bMonths = bDays = false; break;
    case MONTH : bYears = bMonths = bEnable; bDays = false; break;
    default    :
    ZdGenerateException("Time precision type unknown '%d'.","EnableDatesByTimePrecisionUnits()",
                        gAnalysisSettings.GetPrecisionOfTimesControlType());
  };

  //enable start range dates
  edtStartRangeStartYear->Enabled = bYears;
  edtStartRangeStartYear->Color = edtStartRangeStartYear->Enabled ? clWindow : clInactiveBorder;
  edtStartRangeStartMonth->Enabled = bMonths;
  edtStartRangeStartMonth->Color = edtStartRangeStartMonth->Enabled ? clWindow : clInactiveBorder;
  if (!edtStartRangeStartMonth->Enabled && bEnable) edtStartRangeStartMonth->Text = 1;
  edtStartRangeStartDay->Enabled = bDays;
  edtStartRangeStartDay->Color = edtStartRangeStartDay->Enabled ? clWindow : clInactiveBorder;
  if (!edtStartRangeStartDay->Enabled && bEnable) edtStartRangeStartDay->Text = 1;
  edtStartRangeEndYear->Enabled = bYears;
  edtStartRangeEndYear->Color = edtStartRangeEndYear->Enabled ? clWindow : clInactiveBorder;
  edtStartRangeEndMonth->Enabled = bMonths;
  edtStartRangeEndMonth->Color = edtStartRangeEndMonth->Enabled ? clWindow : clInactiveBorder;
  if (!edtStartRangeEndMonth->Enabled && bEnable) edtStartRangeEndMonth->Text = 12;
  edtStartRangeEndDay->Enabled = bDays;
  edtStartRangeEndDay->Color = edtStartRangeEndDay->Enabled ? clWindow : clInactiveBorder;
  if (!edtStartRangeEndDay->Enabled && bEnable) edtStartRangeEndDay->Text = DaysThisMonth(atoi(edtStartRangeEndYear->Text.c_str()), atoi(edtStartRangeEndMonth->Text.c_str()));
  //enable end range dates
  edtEndRangeStartYear->Enabled = bYears;
  edtEndRangeStartYear->Color = edtEndRangeStartYear->Enabled ? clWindow : clInactiveBorder;
  edtEndRangeStartMonth->Enabled = bMonths;
  edtEndRangeStartMonth->Color = edtEndRangeStartMonth->Enabled ? clWindow : clInactiveBorder;
  if (!edtEndRangeStartMonth->Enabled && bEnable) edtEndRangeStartMonth->Text = 1;
  edtEndRangeStartDay->Enabled = bDays;
  edtEndRangeStartDay->Color = edtEndRangeStartDay->Enabled ? clWindow : clInactiveBorder;
  if (!edtEndRangeStartDay->Enabled && bEnable) edtEndRangeStartDay->Text = 1;
  edtEndRangeEndYear->Enabled = bYears;
  edtEndRangeEndYear->Color = edtEndRangeEndYear->Enabled ? clWindow : clInactiveBorder;
  edtEndRangeEndMonth->Enabled = bMonths;
  edtEndRangeEndMonth->Color = edtEndRangeEndMonth->Enabled ? clWindow : clInactiveBorder;
  if (!edtEndRangeEndMonth->Enabled && bEnable) edtEndRangeEndMonth->Text = 12;
  edtEndRangeEndDay->Enabled = bDays;
  edtEndRangeEndDay->Color = edtEndRangeEndDay->Enabled ? clWindow : clInactiveBorder;
  if (!edtEndRangeEndDay->Enabled && bEnable) edtEndRangeEndDay->Text = DaysThisMonth(atoi(edtEndRangeEndYear->Text.c_str()), atoi(edtEndRangeEndMonth->Text.c_str()));
  //enable date contained in prospective surveillance group
  EnableProspectiveStartDate();
}
//---------------------------------------------------------------------------
/** enables input tab case/control/pop files edit boxes */
void TfrmAdvancedParameters::EnableInputFileEdits(bool bEnable) {
   edtCaseFileName->Enabled = bEnable;
   edtCaseFileName->Color = edtCaseFileName->Enabled ? clWindow : clInactiveBorder;
   btnCaseBrowse->Enabled = bEnable;
   btnCaseImport->Enabled = bEnable;
   edtControlFileName->Enabled = bEnable;
   edtControlFileName->Color = edtControlFileName->Enabled ? clWindow : clInactiveBorder;
   btnControlBrowse->Enabled = bEnable;
   btnControlImport->Enabled = bEnable;
   edtPopFileName->Enabled = bEnable;
   edtPopFileName->Color = edtPopFileName->Enabled ? clWindow : clInactiveBorder;
   btnPopBrowse->Enabled = bEnable;
   btnPopImport->Enabled = bEnable;
}

/** Enables neighbors file group. */
void TfrmAdvancedParameters::EnableNeighborsFileGroup(bool bEnable) {
   grpNeighborsFile->Enabled = bEnable;
   chkSpecifiyNeighborsFile->Enabled = bEnable;
   edtNeighborsFile->Enabled = bEnable && chkSpecifiyNeighborsFile->Checked;
   lblNeighborsFile->Enabled = bEnable && chkSpecifiyNeighborsFile->Checked;
   edtNeighborsFile->Color = edtNeighborsFile->Enabled ? clWindow : clInactiveBorder;
   btnBrowseForNeighborsFile->Enabled = bEnable && chkSpecifiyNeighborsFile->Checked;
}
//---------------------------------------------------------------------------
//** enables or disables the New button on the Input tab
void TfrmAdvancedParameters::EnableNewButton() {
  btnNewDataSet->Enabled = (lstInputDataSets->Items->Count < MAXIMUM_ADDITIONAL_SETS) ? true: false;
}
//---------------------------------------------------------------------------
/** enables adjustment options controls */
void TfrmAdvancedParameters::EnableOutputOptions(bool bEnable) {
   rdgCriteriaSecClusters->Enabled = bEnable && !chkSpecifiyNeighborsFile->Checked;
}
//---------------------------------------------------------------------------
/** enabled prospective start date controls */
void TfrmAdvancedParameters::EnableProspectiveStartDate() {
  bool bYears, bMonths, bDays,
       bEnable = gbxProspectiveSurveillance->Enabled &&
                 chkAdjustForEarlierAnalyses->Enabled &&
                 chkAdjustForEarlierAnalyses->Checked;

  switch (gAnalysisSettings.GetPrecisionOfTimesControlType()) {
    case NONE  :
    case DAY   : bYears = bMonths = bDays = bEnable; break;
    case YEAR  : bYears = bEnable; bMonths = bDays = false; break;
    case MONTH : bYears = bMonths = bEnable; bDays = false; break;
    default    :
    ZdGenerateException("Time precision type unknown '%d'.","EnableProspectiveStartDate()",
                        gAnalysisSettings.GetPrecisionOfTimesControlType());
  };

  lblProspectiveStartYear->Enabled = bYears;
  edtProspectiveStartDateYear->Enabled = bYears;
  edtProspectiveStartDateYear->Color = bYears ? clWindow : clInactiveBorder;
  lblProspectiveStartMonth->Enabled = bMonths;
  edtProspectiveStartDateMonth->Enabled = bMonths;
  edtProspectiveStartDateMonth->Color = bMonths ? clWindow : clInactiveBorder;
  if (!bMonths && bEnable)
    edtProspectiveStartDateMonth->Text = gAnalysisSettings.edtStudyPeriodEndDateMonth->Text;
  lblProspectiveStartDay->Enabled = bDays;
  edtProspectiveStartDateDay->Enabled = bDays;
  edtProspectiveStartDateDay->Color =  bDays ? clWindow : clInactiveBorder;
  if (!bDays && bEnable)
    edtProspectiveStartDateDay->Text = DaysThisMonth(StrToInt(edtProspectiveStartDateYear->Text), StrToInt(edtProspectiveStartDateMonth->Text));
}
//---------------------------------------------------------------------------
/** enables or disables the prospective start date group control */
void TfrmAdvancedParameters::EnableProspectiveSurveillanceGroup(bool bEnable) {
   gbxProspectiveSurveillance->Enabled = bEnable;
   chkAdjustForEarlierAnalyses->Enabled = bEnable;
   EnableProspectiveStartDate();
}
//---------------------------------------------------------------------------
//** enables or disables the New button on the Input tab
void TfrmAdvancedParameters::EnableRemoveButton() {
  btnRemoveDataSet->Enabled = (lstInputDataSets->Items->Count > 0) ? true: false;
}
//---------------------------------------------------------------------------
/** enables or disables the spatial options group control */
void TfrmAdvancedParameters::EnableReportedSpatialOptionsGroup(bool bEnable) {
   bEnable &= !chkSpecifiyNeighborsFile->Checked;

   rdgReportedSpatialOptions->Enabled = bEnable;
   chkRestrictReportedClusters->Enabled = bEnable;

   bool bEnablePopPercentage = !(gAnalysisSettings.GetAnalysisControlType() == PROSPECTIVESPACETIME &&
                                 chkAdjustForEarlierAnalyses->Enabled && chkAdjustForEarlierAnalyses->Checked); 
   edtMaxReportedSpatialClusterSize->Enabled = bEnable && chkRestrictReportedClusters->Checked && bEnablePopPercentage;
   edtMaxReportedSpatialClusterSize->Color = bEnable && chkRestrictReportedClusters->Checked && bEnablePopPercentage ? clWindow : clInactiveBorder;
   lblReportedPercentOfPopulation->Enabled = bEnable && chkRestrictReportedClusters->Checked && bEnablePopPercentage;

   chkReportedSpatialPopulationFile->Enabled = bEnable && chkRestrictReportedClusters->Checked;
   edtMaxReportedSpatialPercentFile->Enabled = bEnable && chkRestrictReportedClusters->Checked && chkReportedSpatialPopulationFile->Checked;
   edtMaxReportedSpatialPercentFile->Color = bEnable && chkRestrictReportedClusters->Checked && chkReportedSpatialPopulationFile->Checked ? clWindow : clInactiveBorder;
   lblReportedPercentageOfPopFile->Enabled = bEnable && chkRestrictReportedClusters->Checked;

   chkReportedSpatialDistance->Enabled = bEnable && chkRestrictReportedClusters->Checked;
   lblReportedMaxDistance->Enabled = bEnable && chkRestrictReportedClusters->Checked;
   edtMaxReportedSpatialRadius->Enabled = bEnable && chkRestrictReportedClusters->Checked && chkReportedSpatialDistance->Checked;
   edtMaxReportedSpatialRadius->Color = bEnable && chkRestrictReportedClusters->Checked && chkReportedSpatialDistance->Checked ? clWindow : clInactiveBorder;
   lblMaxReportedRadius->Enabled = bEnable && chkRestrictReportedClusters->Checked;
}
//---------------------------------------------------------------------------
void TfrmAdvancedParameters::EnableSequentialScanOptionsGroup(bool bEnable) {
  grpSequentialScan->Enabled = bEnable;
  chkPerformSequentialScan->Enabled = bEnable;
  edtNumSequentialScans->Enabled = chkPerformSequentialScan->Checked && bEnable;
  edtNumSequentialScans->Color = edtNumSequentialScans->Enabled ? clWindow : clInactiveBorder;
  lblMaxSequentialScans->Enabled = chkPerformSequentialScan->Checked && bEnable;
  edtSequentialScanCutoff->Enabled = chkPerformSequentialScan->Checked && bEnable;
  edtSequentialScanCutoff->Color = edtSequentialScanCutoff->Enabled ? clWindow : clInactiveBorder;
  lblSeqentialCutoff->Enabled = chkPerformSequentialScan->Checked && bEnable;
  stMaximumIterationsCriteria->Enabled = chkPerformSequentialScan->Checked && bEnable;
  stStopPValue->Enabled = chkPerformSequentialScan->Checked && bEnable;
}
//---------------------------------------------------------------------------
void TfrmAdvancedParameters::EnableSettingsForAnalysisModelCombination() {
  bool  bPoisson(gAnalysisSettings.GetModelControlType() == POISSON),
        bSpaceTimePermutation(gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION),
        bExponential(gAnalysisSettings.GetModelControlType() == EXPONENTIAL);

  try {
    switch (gAnalysisSettings.GetAnalysisControlType()) {
      case PURELYSPATIAL             :
        EnableAdjustmentForTimeTrendOptionsGroup(false, false, false, false);
        EnableAdjustmentForSpatialOptionsGroup(false, false);
        EnableSpatialOptionsGroup(true, false);
        EnableWindowShapeGroup(true);
        EnableTemporalOptionsGroup(false, false, false);
        EnableProspectiveSurveillanceGroup(false);
        EnableOutputOptions(true);
        EnableNeighborsFileGroup(true);
        break;
      case PURELYTEMPORAL            :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson, bPoisson);
        EnableAdjustmentForSpatialOptionsGroup(false, false);
        EnableSpatialOptionsGroup(false, false);
        EnableWindowShapeGroup(false);
        EnableTemporalOptionsGroup(true, false, true);
        EnableProspectiveSurveillanceGroup(false);
        EnableOutputOptions(false);
        EnableNeighborsFileGroup(false);
        break;
      case SPACETIME                 :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson,
                                                 bPoisson && GetAdjustmentSpatialControlType() != SPATIALLY_STRATIFIED_RANDOMIZATION,
                                                 bPoisson, bPoisson);
        EnableAdjustmentForSpatialOptionsGroup(bPoisson, GetAdjustmentTimeTrendControlType() != STRATIFIED_RANDOMIZATION);
        EnableSpatialOptionsGroup(true, !bSpaceTimePermutation);
        EnableWindowShapeGroup(true);
        EnableTemporalOptionsGroup(true, !bSpaceTimePermutation, true);
        EnableProspectiveSurveillanceGroup(false);
        EnableOutputOptions(true);
        EnableNeighborsFileGroup(true);
        break;
      case PROSPECTIVESPACETIME      :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson,
                                                 bPoisson && GetAdjustmentSpatialControlType() != SPATIALLY_STRATIFIED_RANDOMIZATION,
                                                 bPoisson, bPoisson);
        EnableAdjustmentForSpatialOptionsGroup(bPoisson, GetAdjustmentTimeTrendControlType() != STRATIFIED_RANDOMIZATION);
        EnableSpatialOptionsGroup(true, !bSpaceTimePermutation);
        EnableWindowShapeGroup(true);
        EnableTemporalOptionsGroup(true, !bSpaceTimePermutation, false);
        EnableProspectiveSurveillanceGroup(true);
        EnableOutputOptions(true);
        EnableNeighborsFileGroup(true);
        break;
      case PROSPECTIVEPURELYTEMPORAL :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson, bPoisson);
        EnableAdjustmentForSpatialOptionsGroup(false, false);
        EnableSpatialOptionsGroup(false, false);
        EnableWindowShapeGroup(false);
        EnableTemporalOptionsGroup(true, false, false);
        EnableProspectiveSurveillanceGroup(true);
        EnableOutputOptions(false);
        EnableNeighborsFileGroup(false);
        break;
      default :
        ZdGenerateException("Unknown analysis type '%d'.",
                            "EnableSettingsForAnalysisModelCombination()",gAnalysisSettings.GetAnalysisControlType());
    }
    EnableAdjustmentsGroup(bPoisson);
    EnableSequentialScanOptionsGroup(!bSpaceTimePermutation && !bExponential);
  }
  catch (ZdException &x) {
    x.AddCallpath("EnableSettingsForAnalysisModelCombination()","TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
/** enables or disables the spatial options group control */
void TfrmAdvancedParameters::EnableSpatialOptionsGroup(bool bEnable, bool bEnableIncludePurelyTemporal) {
   bEnable &= !chkSpecifiyNeighborsFile->Checked;

   rdgSpatialOptions->Enabled = bEnable;

   bool bEnablePopPercentage = !(gAnalysisSettings.GetAnalysisControlType() == PROSPECTIVESPACETIME &&
                                 chkAdjustForEarlierAnalyses->Enabled && chkAdjustForEarlierAnalyses->Checked); 
   
   edtMaxSpatialClusterSize->Enabled = bEnable && bEnablePopPercentage;
   edtMaxSpatialClusterSize->Color = bEnable && bEnablePopPercentage ? clWindow : clInactiveBorder;
   lblPercentOfPopulation->Enabled = bEnable && bEnablePopPercentage;

   chkSpatialPopulationFile->Enabled = bEnable;
   edtMaxSpatialPercentFile->Enabled = bEnable && chkSpatialPopulationFile->Checked;
   edtMaxSpatialPercentFile->Color = bEnable && chkSpatialPopulationFile->Checked ? clWindow : clInactiveBorder;
   lblPercentageOfPopFile->Enabled = bEnable;
   edtMaxCirclePopulationFilename->Enabled = bEnable;
   edtMaxCirclePopulationFilename->Color = bEnable ? clWindow : clInactiveBorder;
   btnBrowseMaxCirclePopFile->Enabled = bEnable;
   btnImportMaxCirclePopFile->Enabled = bEnable;

   chkSpatialDistance->Enabled = bEnable;
   edtMaxSpatialRadius->Enabled = bEnable && chkSpatialDistance->Checked;
   edtMaxSpatialRadius->Color = bEnable && chkSpatialDistance->Checked ? clWindow : clInactiveBorder;
   lblDistancePrefix->Enabled = bEnable;
   lblMaxRadius->Enabled = bEnable;

   chkInclPureTempClust->Enabled = bEnable && bEnableIncludePurelyTemporal;
   EnableReportedSpatialOptionsGroup(bEnable);
}
//---------------------------------------------------------------------------
/** enables temporal options controls */
void TfrmAdvancedParameters::EnableTemporalRanges(bool bEnable, bool bEnableRanges) {
  grpFlexibleTemporalWindowDefinition->Enabled = bEnable && bEnableRanges;
  chkRestrictTemporalRange->Enabled = bEnable && bEnableRanges;

  stStartWindowRange->Enabled = bEnable && bEnableRanges && chkRestrictTemporalRange->Enabled;
  stStartRangeTo->Enabled = bEnable && bEnableRanges;
  stEndWindowRange->Enabled = bEnable && bEnableRanges;
  stEndRangeTo->Enabled = bEnable && bEnableRanges;
}
//---------------------------------------------------------------------------
/** enables or disables the temporal options group control */
void TfrmAdvancedParameters::EnableTemporalOptionsGroup(bool bEnable, bool bEnableIncludePurelySpatial, bool bEnableRanges) {
  rdgTemporalOptions->Enabled = bEnable;

  rdoPercentageTemporal->Enabled = bEnable;
  edtMaxTemporalClusterSize->Enabled = bEnable && rdoPercentageTemporal->Checked;
  edtMaxTemporalClusterSize->Color = bEnable && rdoPercentageTemporal->Checked ? clWindow : clInactiveBorder;
  lblPercentageOfStudyPeriod->Enabled = bEnable;

  rdoTimeTemporal->Enabled = bEnable;
  edtMaxTemporalClusterSizeUnits->Enabled = bEnable && rdoTimeTemporal->Checked;
  edtMaxTemporalClusterSizeUnits->Color = bEnable && rdoTimeTemporal->Checked ? clWindow : clInactiveBorder;
  lblMaxTemporalTimeUnits->Enabled = bEnable;

  chkIncludePureSpacClust->Enabled = bEnable && bEnableIncludePurelySpatial;
  EnableTemporalRanges(bEnable, bEnableRanges);
}
//---------------------------------------------------------------------------
/** enables or disables the window shape options group control */
void TfrmAdvancedParameters::EnableWindowShapeGroup(bool bEnable) {
   bEnable &= !chkSpecifiyNeighborsFile->Checked;
   
   grpWindowShape->Enabled = bEnable;
   rdoCircular->Enabled = bEnable;
   rdoElliptic->Enabled = bEnable && (CoordinatesType)gAnalysisSettings.rgpCoordinates->ItemIndex == CARTESIAN;
   stNonCompactnessPenalty->Enabled = rdoElliptic->Enabled && rdoElliptic->Checked;
   cmbNonCompactnessPenalty->Enabled = rdoElliptic->Enabled && rdoElliptic->Checked;
   cmbNonCompactnessPenalty->Color = cmbNonCompactnessPenalty->Enabled ? clWindow : clInactiveBorder;
   if (!rdoElliptic->Enabled && rdoElliptic->Checked && rdoCircular->Enabled) rdoCircular->Checked = true;
   SetSpatialDistanceCaption();
}
//---------------------------------------------------------------------------
/** event triggered when key pressed for controls that can contain real numbers. */
void __fastcall TfrmAdvancedParameters::FloatKeyPress(TObject *Sender, char &Key) {
  TEdit * pEdit;
  //permit only characters that could comprise a floating point variable
  if (!strchr("-0123456789.\b", Key))
    Key = 0;
  //permit only one decimal place or negative character
  else if ((Key == '.' || Key == '-') && (pEdit = dynamic_cast<TEdit*>(Sender)) != 0 && strchr(pEdit->Text.c_str(), Key))
    Key = 0;
}
//---------------------------------------------------------------------------
/** event triggered when user presses key -- cancels dialog if escape key pressed */
void __fastcall TfrmAdvancedParameters::FormKeyPress(TObject *Sender, char &Key) {
  if (Key == VK_ESCAPE)
    Close();
}
//---------------------------------------------------------------------------
/** event triggered when form shows */
void __fastcall TfrmAdvancedParameters::FormShow(TObject *Sender) {
  if (gpFocusControl)
   gpFocusControl->SetFocus();
}
//---------------------------------------------------------------------------
/** returns spatial adjustment type from control index  */
SpatialAdjustmentType TfrmAdvancedParameters::GetAdjustmentSpatialControlType() const {
  SpatialAdjustmentType eReturn;

  switch (rdgSpatialAdjustments->ItemIndex) {
    case 0  : eReturn = NO_SPATIAL_ADJUSTMENT; break;
    case 1  : eReturn = SPATIALLY_STRATIFIED_RANDOMIZATION; break;
    default : ZdGenerateException("Unknown index type '%d'.", "GetAdjustmentSpatialControlType()", rdgSpatialAdjustments->ItemIndex);
  }
  return eReturn;
}
//---------------------------------------------------------------------------
/** returns adjustment for time trend type for control index */
TimeTrendAdjustmentType TfrmAdvancedParameters::GetAdjustmentTimeTrendControlType() const {
  TimeTrendAdjustmentType eReturn;

  switch (rdgTemporalTrendAdj->ItemIndex) {
    case 0  : eReturn = NOTADJUSTED; break;
    case 1  : eReturn = STRATIFIED_RANDOMIZATION; break;
    case 2  : eReturn = LOGLINEAR_PERC; break;
    case 3  : eReturn = CALCULATED_LOGLINEAR_PERC; break;
    default : ZdGenerateException("Unknown index type '%d'.", "GetAdjustmentTimeTrendControlType()", rdgTemporalTrendAdj->ItemIndex);
  }
  return eReturn;
}
//---------------------------------------------------------------------------
/** returns geographical coordinates data type for control */
CoordinatesDataCheckingType TfrmAdvancedParameters::GetCoordinatesDataCheckingTypeFromControl() const {
   CoordinatesDataCheckingType eReturn;

   if (rdoStrictCoordinates->Checked)
     eReturn = STRICTCOORDINATES;
   else if (rdoRelaxedCoordinates->Checked)
     eReturn = RELAXEDCOORDINATES;
   else
     ZdGenerateException("Unknown geographical coordinates.", "GetCoordinatesDataCheckingTypeFromControl()");

  return eReturn;
}
//---------------------------------------------------------------------------
//** Checks to determine if only default values are set in the dialog
//** Returns true if only default values are set on enabled controls
//** Returns false if user specified a value other than a default
bool TfrmAdvancedParameters::GetDefaultsSetForAnalysisOptions() {
   bool bReturn = true;

   // Inference tab
   bReturn &= (chkAdjustForEarlierAnalyses->Checked == false);
   bReturn &= (chkReportCriticalValues->Checked == false);
   bReturn &= (chkTerminateEarly->Checked == false);
   bReturn &= (edtProspectiveStartDateYear->Text.ToInt() == 1900 || edtProspectiveStartDateYear->Text.ToInt() == 2000);
   bReturn &= (edtProspectiveStartDateMonth->Text.ToInt() == 12);
   bReturn &= (edtProspectiveStartDateDay->Text.ToInt() == 31);
   bReturn &= (chkPerformSequentialScan->Checked == false);
   bReturn &= (edtNumSequentialScans->Text.ToInt() == 10);
   bReturn &= (edtSequentialScanCutoff->Text.ToDouble() == .05);

   // Spatial Window tab
   bReturn &= (edtMaxSpatialClusterSize->Text.ToDouble() == 50);
   bReturn &= (chkSpatialPopulationFile->Checked == false);
   bReturn &= (edtMaxSpatialPercentFile->Text.ToDouble() == 50);
   bReturn &= (edtMaxSpatialRadius->Text.ToDouble() == 1);
   bReturn &= (chkSpatialDistance->Checked == false);
   bReturn &= (edtMaxCirclePopulationFilename->Text == "");
   bReturn &= (chkInclPureTempClust->Checked == false);
   bReturn &= (rdoCircular->Checked == true);
   bReturn &= (cmbNonCompactnessPenalty->ItemIndex == 1);

   // Temporal tab
   bReturn &= (GetMaxTemporalClusterSizeControlType()==PERCENTAGETYPE);
   bReturn &= (edtMaxTemporalClusterSize->Text.ToDouble() == 50);
   bReturn &= (edtMaxTemporalClusterSizeUnits->Text.ToInt() == 1);
   bReturn &= (chkIncludePureSpacClust->Checked == false);

   bReturn &= (edtStartRangeStartYear->Text.ToInt() == 1900 || edtStartRangeStartYear->Text.ToInt() == 2000);
   bReturn &= (edtStartRangeStartMonth->Text.ToInt() == 1);
   bReturn &= (edtStartRangeStartDay->Text.ToInt() == 1);
   bReturn &= (edtStartRangeEndYear->Text.ToInt() == 1900 || edtStartRangeEndYear->Text.ToInt() == 2000);
   bReturn &= (edtStartRangeEndMonth->Text.ToInt() == 1 || edtStartRangeEndMonth->Text.ToInt() == 12);
   bReturn &= (edtStartRangeEndDay->Text.ToInt() == 1 || edtStartRangeEndDay->Text.ToInt() == 31);
   bReturn &= (edtEndRangeStartYear->Text.ToInt() == 1900 || edtEndRangeStartYear->Text.ToInt() == 2000);
   bReturn &= (edtEndRangeStartMonth->Text.ToInt() == 1 || edtEndRangeStartMonth->Text.ToInt() == 12);
   bReturn &= (edtEndRangeStartDay->Text.ToInt() == 1 || edtEndRangeStartDay->Text.ToInt() == 31);
   bReturn &= (edtEndRangeEndYear->Text.ToInt() == 1900 || edtEndRangeEndYear->Text.ToInt() == 2000);
   bReturn &= (edtEndRangeEndMonth->Text.ToInt() == 12);
   bReturn &= (edtEndRangeEndDay->Text.ToInt() == 31);
   bReturn &= (chkRestrictTemporalRange->Checked == false);

   // Risk tab
   bReturn &= (chkAdjustForKnownRelativeRisks->Checked == false);
   bReturn &= (edtAdjustmentsByRelativeRisksFile->Text == "");
   bReturn &= (rdgTemporalTrendAdj->ItemIndex == 0);
   bReturn &= (edtLogLinear->Text.ToDouble() == 0);
   bReturn &= (rdgSpatialAdjustments->ItemIndex == 0);

   return bReturn;
}
//---------------------------------------------------------------------------
//** Checks to determine if only default values are set in the dialog
//** Returns true if only default values are set
//** Returns false if user specified a value other than a default
bool TfrmAdvancedParameters::GetDefaultsSetForInputOptions() {
   bool bReturn = true;

   bReturn &= (lstInputDataSets->Items->Count == 0);
   bReturn &= (rdoMultivariate->Checked);
   bReturn &= (rdgStudyPeriodCheck->ItemIndex == 0);
   bReturn &= (rdoStrictCoordinates->Checked == true);
   bReturn &= (chkSpecifiyNeighborsFile->Checked == false);
   bReturn &= (edtNeighborsFile->Text == "");

   return bReturn;
}
//---------------------------------------------------------------------------
//** Checks to determine if only default values are set in the dialog
//** Returns true if only default values are set
//** Returns false if user specified a value other than a default
bool TfrmAdvancedParameters::GetDefaultsSetForOutputOptions() {
   bool bReturn = true;

   // Output tab
   bReturn &= (rdgCriteriaSecClusters->ItemIndex == NOGEOOVERLAP);
   bReturn &= (edtMaxReportedSpatialClusterSize->Text.ToDouble() == 50);
   bReturn &= (edtMaxReportedSpatialPercentFile->Text.ToDouble() == 50);
   bReturn &= (chkReportedSpatialPopulationFile->Checked == false);
   bReturn &= (edtMaxReportedSpatialRadius->Text.ToDouble() == 1);
   bReturn &= (chkReportedSpatialDistance->Checked == false);

   return bReturn;
}
//---------------------------------------------------------------------------
/** returns maximum spatial cluster size from appropriate control */
double TfrmAdvancedParameters::GetMaxReportedSpatialClusterSizeFromControl(SpatialSizeType eSpatialType) const {
  double dReturn;

  switch (eSpatialType) {
    case MAXDISTANCE                 : dReturn = edtMaxReportedSpatialRadius->Text.ToDouble(); break;
    case PERCENTOFMAXCIRCLEFILE      : dReturn = edtMaxReportedSpatialPercentFile->Text.ToDouble(); break;
    case PERCENTOFPOPULATION         :
    default                          : dReturn = edtMaxReportedSpatialClusterSize->Text.ToDouble();
  }
  return dReturn;
}
//---------------------------------------------------------------------------
/** returns maximum spatial cluster size from appropriate control */
double TfrmAdvancedParameters::GetMaxSpatialClusterSizeFromControl(SpatialSizeType eSpatialType) const {
  double   dReturn;

  switch (eSpatialType) {
    case MAXDISTANCE                 : dReturn = edtMaxSpatialRadius->Text.ToDouble(); break;
    case PERCENTOFMAXCIRCLEFILE      : dReturn = edtMaxSpatialPercentFile->Text.ToDouble(); break;
    case PERCENTOFPOPULATION         :
    default                          : dReturn = edtMaxSpatialClusterSize->Text.ToDouble();
  }
  return dReturn;
}
//---------------------------------------------------------------------------
/** returns maximum temporal cluster size type for control */
TemporalSizeType TfrmAdvancedParameters::GetMaxTemporalClusterSizeControlType() const {
  TemporalSizeType eReturn;

  if (rdoPercentageTemporal->Checked)
    eReturn = PERCENTAGETYPE;
  else if (rdoTimeTemporal->Checked)
    eReturn = TIMETYPE;
  else
    ZdGenerateException("Maximum temporal cluster size type not selected.","GetMaxSpatialClusterSizeControlType()");

  return eReturn;
}
//---------------------------------------------------------------------------
/** returns maximum temporal cluster size from appropriate control */
double TfrmAdvancedParameters::GetMaxTemporalClusterSizeFromControl() const {
  double dReturn;

  switch (GetMaxTemporalClusterSizeControlType()) {
    case TIMETYPE        : dReturn = edtMaxTemporalClusterSizeUnits->Text.ToDouble(); break;
    case PERCENTAGETYPE  :
    default              : dReturn = edtMaxTemporalClusterSize->Text.ToDouble();
  }
  return dReturn;
}
//---------------------------------------------------------------------------
/** Returns study period data checking type as specified by control. */
StudyPeriodDataCheckingType TfrmAdvancedParameters::GetStudyPeriodDataCheckingFromControl() const {
  return (StudyPeriodDataCheckingType)rdgStudyPeriodCheck->ItemIndex;
}
//---------------------------------------------------------------------------
/** class initialization */
void TfrmAdvancedParameters::Init() {
  gpFocusControl=0;
  rdgCriteriaSecClusters->ItemIndex = 0;
  giCategory = 0;
  giDataSetNum = 2;
  cmbNonCompactnessPenalty->ItemIndex = 1;
}
//---------------------------------------------------------------------------
/** Modally shows import dialog. */
void TfrmAdvancedParameters::LaunchImporter(const char * sFileName, InputFileType eFileType) {
  ZdString sNewFile = "";

  try {
    std::auto_ptr<TBDlgDataImporter> pDialog(new TBDlgDataImporter(this,
                                                                   sFileName,
                                                                   eFileType,
                                                                   gAnalysisSettings.GetModelControlType(),
                                                                   (CoordinatesType)(gAnalysisSettings.rgpCoordinates->ItemIndex)));
    if (pDialog->ShowModal() == mrOk) {
       switch (eFileType) {  // set parameters
          case Case                : edtCaseFileName->Text = pDialog->GetDestinationFilename(sNewFile);
                                     break;
          case Control             : edtControlFileName->Text = pDialog->GetDestinationFilename(sNewFile);
                                     break;
          case Population          : edtPopFileName->Text = pDialog->GetDestinationFilename(sNewFile);
                                     break;
          case MaxCirclePopulation : edtMaxCirclePopulationFilename->Text = pDialog->GetDestinationFilename(sNewFile);
                                     break;
          case AdjustmentsByRR     : SetAdjustmentsByRelativeRisksFile(pDialog->GetDestinationFilename(sNewFile));
                                     break;
          default                  : ZdGenerateException("Unknown file type index: \"%d\"","LaunchImporter()", eFileType);
       };
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("LaunchImporter()", "TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
// when user clicks on an input dataset name, display the details in the edit
// boxes above the list box
void __fastcall TfrmAdvancedParameters::lstInputDataSetsClick(TObject *Sender) {
   int iDataSetNum = -1;

   try {
      // determine the input dataset selected
      iDataSetNum = lstInputDataSets->ItemIndex;
      // set the files
      EnableInputFileEdits(true);
      edtCaseFileName->Text = gvCaseFiles.at(iDataSetNum);
      edtControlFileName->Text = gvControlFiles.at(iDataSetNum);
      edtPopFileName->Text = gvPopFiles.at(iDataSetNum);
   }
   catch (ZdException &x) {
     x.AddCallpath("lstInputDataSetsClick()","TfrmAdvancedParameters");
     DisplayBasisException(this, x);
   }
}
//---------------------------------------------------------------------------
/** event triggered when key pressed for control that can contain natural numbers */
void __fastcall TfrmAdvancedParameters::NaturalNumberKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789\b",Key))
    Key = 0;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAdvancedParameters::OnControlExit(TObject *Sender) {
   DoControlExit();
}
//---------------------------------------------------------------------------
void __fastcall TfrmAdvancedParameters::OnNonCompactnessPenaltyChange(TObject *Sender) {
  DoControlExit();
}
//---------------------------------------------------------------------------
void __fastcall TfrmAdvancedParameters::OnWindowShapeClick(TObject *Sender) {
  EnableWindowShapeGroup(grpWindowShape->Enabled);
  DoControlExit();
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
/** event triggered when key pressed for control that can contain positive real numbers */
void __fastcall TfrmAdvancedParameters::PositiveFloatKeyPress(TObject *Sender, char &Key) {
  TEdit * pEdit;
  //permit only characters that could comprise a positive floating point variable
  if (!strchr("0123456789.\b", Key))
    Key = 0;
  //permit only one decimal place
  else if (Key == '.' && (pEdit = dynamic_cast<TEdit*>(Sender)) != 0 && strchr(pEdit->Text.c_str(), Key))
    Key = 0;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAdvancedParameters::rdgSpatialAdjustmentsClick(TObject *Sender) {
  DoControlExit();
  EnableSettingsForAnalysisModelCombination();
}
//---------------------------------------------------------------------------
void __fastcall TfrmAdvancedParameters::rdgStudyPeriodCheckClick(TObject *Sender) {
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when 'Adjustment for time trend' type control clicked */
void __fastcall TfrmAdvancedParameters::rdgTemporalTrendAdjClick(TObject *Sender) {
  switch (GetAdjustmentTimeTrendControlType()) {
    case LOGLINEAR_PERC           : edtLogLinear->Enabled = true;
                                    edtLogLinear->Color = clWindow;
                                    break;
    case STRATIFIED_RANDOMIZATION : rdgSpatialAdjustments->Controls[1]->Enabled = false;
    default                       : edtLogLinear->Enabled = false;
                                    edtLogLinear->Color = clInactiveBorder;
  }
  DoControlExit();
  EnableSettingsForAnalysisModelCombination();
}
//---------------------------------------------------------------------------
/** event triggered when maximum temporal cluster size type edit control clicked */
void __fastcall TfrmAdvancedParameters::rdoMaxTemporalClusterSizelick(TObject *Sender) {
  //cause enabling to be refreshed based upon clicked radio button
  EnableTemporalOptionsGroup(rdgTemporalOptions->Enabled, chkIncludePureSpacClust->Enabled,
                             chkRestrictTemporalRange->Enabled);
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when maximum spatial type selected */
void __fastcall TfrmAdvancedParameters::rdoMaxSpatialTypeClick(TObject *Sender) {
  //cause enabling to be refreshed based upon clicked radio button
  EnableSpatialOptionsGroup(rdgSpatialOptions->Enabled, chkInclPureTempClust->Enabled);
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when rdoRelaxedCoordinates control selected */
void __fastcall TfrmAdvancedParameters::rdoRelaxedCoordinatesClick(TObject *Sender) {
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when rdoStrictCoordinates control selected */
void __fastcall TfrmAdvancedParameters::rdoStrictCoordinatesClick(TObject *Sender) {
  DoControlExit();
}
//---------------------------------------------------------------------------
/** parameter settings to parameters class */
void TfrmAdvancedParameters::SaveParameterSettings() {
  CParameters&  ref = const_cast<CParameters&>(gAnalysisSettings.gParameters);
  ZdString      sString;

  try {
    ref.UseLocationNeighborsFile(chkSpecifiyNeighborsFile->Checked);
    ref.SetLocationNeighborsFileName(edtNeighborsFile->Text.c_str());
    ref.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, GetMaxSpatialClusterSizeFromControl(PERCENTOFPOPULATION), false);
    ref.SetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, GetMaxSpatialClusterSizeFromControl(PERCENTOFMAXCIRCLEFILE), false);
    ref.SetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, chkSpatialPopulationFile->Checked, false);
    ref.SetMaxSpatialSizeForType(MAXDISTANCE, GetMaxSpatialClusterSizeFromControl(MAXDISTANCE), false);
    ref.SetRestrictMaxSpatialSizeForType(MAXDISTANCE, chkSpatialDistance->Checked, false);
    ref.SetRestrictReportedClusters(chkRestrictReportedClusters->Enabled && chkRestrictReportedClusters->Checked);
    ref.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, GetMaxReportedSpatialClusterSizeFromControl(PERCENTOFPOPULATION), true);
    ref.SetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, GetMaxReportedSpatialClusterSizeFromControl(PERCENTOFMAXCIRCLEFILE), true);
    ref.SetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, chkReportedSpatialPopulationFile->Checked, true);
    ref.SetMaxSpatialSizeForType(MAXDISTANCE, GetMaxReportedSpatialClusterSizeFromControl(MAXDISTANCE), true);
    ref.SetRestrictMaxSpatialSizeForType(MAXDISTANCE, chkReportedSpatialDistance->Checked, true);

    ref.SetSequentialCutOffPValue(edtSequentialScanCutoff->Text.ToDouble());
    ref.SetSequentialScanning(chkPerformSequentialScan->Checked);
    ref.SetNumSequentialScans(atoi(edtNumSequentialScans->Text.c_str()));
    ref.SetUseAdjustmentForRelativeRisksFile(chkAdjustForKnownRelativeRisks->Enabled && chkAdjustForKnownRelativeRisks->Checked);
    ref.SetAdjustmentsByRelativeRisksFilename(edtAdjustmentsByRelativeRisksFile->Text.c_str(), false);
    ref.SetTimeTrendAdjustmentType(rdgTemporalTrendAdj->Enabled ? GetAdjustmentTimeTrendControlType() : NOTADJUSTED);
    ref.SetTimeTrendAdjustmentPercentage(edtLogLinear->Text.ToDouble());
    if (rdgSpatialAdjustments->Enabled)
      ref.SetSpatialAdjustmentType((SpatialAdjustmentType)rdgSpatialAdjustments->ItemIndex);
    else
      ref.SetSpatialAdjustmentType(NO_SPATIAL_ADJUSTMENT);
    ref.SetTerminateSimulationsEarly(chkTerminateEarly->Checked);
    ref.SetReportCriticalValues(chkReportCriticalValues->Checked);
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
    ref.SetCriteriaForReportingSecondaryClusters((CriteriaSecondaryClustersType)rdgCriteriaSecClusters->ItemIndex);

    // save the input files on Input tab
    ref.SetNumDataSets(lstInputDataSets->Items->Count + 1);
    if (lstInputDataSets->Items->Count) {
       for (int i = 0; i < lstInputDataSets->Items->Count; i++) {
          ref.SetCaseFileName((gvCaseFiles.at(i)).c_str(), false, i+2);
          ref.SetControlFileName((gvControlFiles.at(i)).c_str(), false, i+2);
          ref.SetPopulationFileName((gvPopFiles.at(i)).c_str(), false, i+2);
       }
    }
    ref.SetMultipleDataSetPurposeType(rdoAdjustmentByDataSets->Checked ? ADJUSTMENT: MULTIVARIATE);
    ref.SetSpatialWindowType(rdoCircular->Checked ? CIRCULAR : ELLIPTIC);
    ref.SetNonCompactnessPenalty((NonCompactnessPenaltyType)cmbNonCompactnessPenalty->ItemIndex); //this needs further investigation
    ref.SetStudyPeriodDataCheckingType(GetStudyPeriodDataCheckingFromControl());
    ref.SetCoordinatesDataCheckingType(GetCoordinatesDataCheckingTypeFromControl());
    ref.SetAdjustForEarlierAnalyses(chkAdjustForEarlierAnalyses->Enabled && chkAdjustForEarlierAnalyses->Checked);
    if (gbxProspectiveSurveillance->Enabled && !chkAdjustForEarlierAnalyses->Checked)
      sString.printf("%i/%i/%i", atoi(gAnalysisSettings.edtStudyPeriodEndDateYear->Text.c_str()),
                     atoi(gAnalysisSettings.edtStudyPeriodEndDateMonth->Text.c_str()), atoi(gAnalysisSettings.edtStudyPeriodEndDateDay->Text.c_str()));
    else
      sString.printf("%i/%i/%i", atoi(edtProspectiveStartDateYear->Text.c_str()),
                     atoi(edtProspectiveStartDateMonth->Text.c_str()), atoi(edtProspectiveStartDateDay->Text.c_str()));
    ref.SetProspectiveStartDate(sString.GetCString());
    ref.SetMaxCirclePopulationFileName(edtMaxCirclePopulationFilename->Text.c_str(), false);
    ref.SetMaximumTemporalClusterSize(GetMaxTemporalClusterSizeFromControl());
    ref.SetMaximumTemporalClusterSizeType(GetMaxTemporalClusterSizeControlType());
    ref.SetIncludePurelyTemporalClusters(chkInclPureTempClust->Enabled && chkInclPureTempClust->Checked);
    ref.SetIncludePurelySpatialClusters((chkIncludePureSpacClust->Enabled) && (chkIncludePureSpacClust->Checked));
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveParameterSettings()","TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
/** Sets adjustments filename in interface */
void TfrmAdvancedParameters::SetAdjustmentsByRelativeRisksFile(const char * sAdjustmentsByRelativeRisksFileName) {
  edtAdjustmentsByRelativeRisksFile->Text = sAdjustmentsByRelativeRisksFileName;
}
//---------------------------------------------------------------------------
void TfrmAdvancedParameters::SetCoordinatesDataCheckingControl(CoordinatesDataCheckingType eCoordinatesDataCheckingType) {
   switch (eCoordinatesDataCheckingType) {
     case STRICTCOORDINATES  : rdoStrictCoordinates->Checked = true; break;
     case RELAXEDCOORDINATES : rdoRelaxedCoordinates->Checked = true; break;
     default : ZdGenerateException("Unknown geographical coordinates type %d.", "SetCoordinatesDataCheckingControl()", eCoordinatesDataCheckingType);
   };  
}
//---------------------------------------------------------------------------
/** Sets default values for Analysis related tabs and their respective controls
    PAG:  pulled these default values from the CParameter class */
void TfrmAdvancedParameters::SetDefaultsForAnalysisTabs() {
   // Inference tab
   // PAG - moved to first position since other controls dependent on the 'earlier analyses' control
   chkAdjustForEarlierAnalyses->Checked = false;
   chkTerminateEarly->Checked = false;
   edtProspectiveStartDateYear->Text = "2000";
   edtProspectiveStartDateMonth->Text = "12";
   edtProspectiveStartDateDay->Text = "31";
   chkReportCriticalValues->Checked = false;
   chkPerformSequentialScan->Checked = false;
   edtNumSequentialScans->Text = "10";
   edtSequentialScanCutoff->Text = ".05";

   // Spatial Window tab
   edtMaxSpatialClusterSize->Text = "50";
   edtMaxSpatialPercentFile->Text = "50";
   edtMaxSpatialRadius->Text = "1";
   chkSpatialPopulationFile->Checked = false;
   chkSpatialDistance->Checked = false;
   edtMaxCirclePopulationFilename->Text = "";
   chkInclPureTempClust->Checked = false;
   rdoCircular->Checked = true;
   cmbNonCompactnessPenalty->ItemIndex = 1;
   SetSpatialDistanceCaption();

   // Temporal tab
   SetMaxTemporalClusterSizeTypeControl(PERCENTAGETYPE);
   edtMaxTemporalClusterSize->Text = "50";
   edtMaxTemporalClusterSizeUnits->Text = "1";
   chkIncludePureSpacClust->Checked = false;

   ParseDate("2000/01/01", *edtStartRangeStartYear, *edtStartRangeStartMonth, *edtStartRangeStartDay, true);
   ParseDate("2000/12/31", *edtStartRangeEndYear, *edtStartRangeEndMonth, *edtStartRangeEndDay, false);
   ParseDate("2000/01/01", *edtEndRangeStartYear, *edtEndRangeStartMonth, *edtEndRangeStartDay, true);
   ParseDate("2000/12/31", *edtEndRangeEndYear, *edtEndRangeEndMonth, *edtEndRangeEndDay, false);
   chkRestrictTemporalRange->Checked = false;

   // Risk tab
   chkAdjustForKnownRelativeRisks->Checked = false;
   edtAdjustmentsByRelativeRisksFile->Text = "";
   SetTemporalTrendAdjustmentControl(NOTADJUSTED);
   rdgSpatialAdjustments->ItemIndex = NO_SPATIAL_ADJUSTMENT;
   edtLogLinear->Text = "0";
}
//---------------------------------------------------------------------------
/** Sets default values for Input related tab and respective controls */
void TfrmAdvancedParameters::SetDefaultsForInputTab() {
   // clear all visual components
   edtCaseFileName->Text = "";
   edtControlFileName->Text = "";
   edtPopFileName->Text = "";
   lstInputDataSets->Items->Clear();
   EnableDataSetList(lstInputDataSets->Items->Count);
   EnableDataSetPurposeControls(lstInputDataSets->Items->Count);

   // clear the non-visual components
   gvCaseFiles.clear();
   gvControlFiles.clear();
   gvPopFiles.clear();
   giDataSetNum = 2;
   EnableNewButton();
   EnableRemoveButton();
   EnableInputFileEdits(false);
   rdoMultivariate->Checked = true;

   //data checking
   rdgStudyPeriodCheck->ItemIndex = STRICTBOUNDS;
   rdoStrictCoordinates->Checked = true;

   chkSpecifiyNeighborsFile->Checked = false;
   edtNeighborsFile->Text = "";
}
//---------------------------------------------------------------------------
/** Sets default values for Output related tab and respective controls
    PAG:  pulled these default values from the CParameter class */
void TfrmAdvancedParameters::SetDefaultsForOutputTab() {
   rdgCriteriaSecClusters->ItemIndex = NOGEOOVERLAP;
   chkRestrictReportedClusters->Checked = false;
   chkReportedSpatialPopulationFile->Checked = false;
   chkReportedSpatialDistance->Checked = false;
   edtMaxReportedSpatialClusterSize->Text = "50";
   edtMaxReportedSpatialPercentFile->Text = "50";
   edtMaxReportedSpatialRadius->Text = "1";
}
//---------------------------------------------------------------------------
/** Set appropriate control for maximum spatial cluster size type. */
void TfrmAdvancedParameters::SetMaxReportedSpatialClusterSizeControl(SpatialSizeType eSpatialSizeType, double dMaxSize) {
   switch (eSpatialSizeType) {
     case MAXDISTANCE:
        if (dMaxSize <= 0)
           edtMaxReportedSpatialRadius->Text = 1;
        else
           edtMaxReportedSpatialRadius->Text = dMaxSize;
        break;
     case PERCENTOFMAXCIRCLEFILE:
        if (dMaxSize <= 0 || dMaxSize > 50)
           edtMaxReportedSpatialPercentFile->Text = 50;
        else
           edtMaxReportedSpatialPercentFile->Text = dMaxSize;
        break;
    case PERCENTOFPOPULATION:
    default:
        if (dMaxSize <= 0 || dMaxSize > 50)
           edtMaxReportedSpatialClusterSize->Text = 50;
        else
           edtMaxReportedSpatialClusterSize->Text = dMaxSize;
   }
}
//---------------------------------------------------------------------------
/** Set appropriate control for maximum spatial cluster size type. */
void TfrmAdvancedParameters::SetMaxSpatialClusterSizeControl(SpatialSizeType eSpatialSizeType, double dMaxSize) {
   switch (eSpatialSizeType) {
     case MAXDISTANCE:
        if (dMaxSize <= 0)
           edtMaxSpatialRadius->Text = 1;
        else
           edtMaxSpatialRadius->Text = dMaxSize;
        break;
     case PERCENTOFMAXCIRCLEFILE:
        if (dMaxSize <= 0 || dMaxSize > 50)
           edtMaxSpatialPercentFile->Text = 50;
        else
           edtMaxSpatialPercentFile->Text = dMaxSize;
        break;
    case PERCENTOFPOPULATION:
    default:
        if (dMaxSize <= 0 || dMaxSize > 50)
           edtMaxSpatialClusterSize->Text = 50;
        else
           edtMaxSpatialClusterSize->Text = dMaxSize;
   }
}
//---------------------------------------------------------------------------
/** Set appropriate control for maximum spatial cluster size type. */
void TfrmAdvancedParameters::SetMaxTemporalClusterSizeControl(double dMaxSize) {
  switch (GetMaxTemporalClusterSizeControlType()) {
    case TIMETYPE:
       if (dMaxSize <= 0)
          edtMaxTemporalClusterSizeUnits->Text = 1;
       else
          edtMaxTemporalClusterSizeUnits->Text = static_cast<int>(dMaxSize);
       break;
    case PERCENTAGETYPE :
    default             :
       if (dMaxSize <= 0 || dMaxSize > 50)
          edtMaxTemporalClusterSize->Text = 50;
       else
          edtMaxTemporalClusterSize->Text = dMaxSize;
  }
}
//---------------------------------------------------------------------------
/** Sets maximum temporal cluster size control for passed type */
void TfrmAdvancedParameters::SetMaxTemporalClusterSizeTypeControl(TemporalSizeType eTemporalSizeType) {
  switch (eTemporalSizeType) {
    case TIMETYPE       : rdoTimeTemporal->Checked = true; break;
    case PERCENTAGETYPE :
    default             : rdoPercentageTemporal->Checked = true;
  }
}
//---------------------------------------------------------------------------
/** Sets caption of spatial distance radio button based upon coordinates group setting. */
void TfrmAdvancedParameters::SetSpatialDistanceCaption() {
  AnsiString    sRadioCaption, sLabelCaption;
  
  try {
    sRadioCaption.printf("is %s with a", (rdoCircular->Checked ? "a circle" : "an ellipse"));
    lblDistancePrefix->Caption = sRadioCaption;
    edtMaxSpatialRadius->Left = (rdoCircular->Checked ? 123 : 132);
    lblMaxRadius->Left = (rdoCircular->Checked ? 191 : 200);
    sRadioCaption.printf("%s with a", (rdoCircular->Checked ? "a circle" : "an ellipse"));
    lblReportedMaxDistance->Caption = sRadioCaption;
    edtMaxReportedSpatialRadius->Left = (rdoCircular->Checked ? 136 : 144);
    lblMaxReportedRadius->Left = (rdoCircular->Checked ? 203 : 211);
    switch (gAnalysisSettings.rgpCoordinates->ItemIndex) {
      case 0  : sLabelCaption.printf("Cartesian units %s", (rdoCircular->Checked ? "radius" : "minor axis"));
                break;
      case 1  : sLabelCaption.printf("kilometer %s", (rdoCircular->Checked ? "radius" : "minor axis"));
                break;
      default : ZdException::Generate("Unknown coordinates radio button index: '%i'.",
                                      "rgpCoordinatesClick()", gAnalysisSettings.rgpCoordinates->ItemIndex);
    }
    lblMaxRadius->Caption = sLabelCaption;
    lblMaxReportedRadius->Caption = sLabelCaption;
  }
  catch (ZdException & x) {
    x.AddCallpath("SetSpatialDistanceCaption()", "TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
/** Sets study period data checking control's index */
void TfrmAdvancedParameters::SetStudyPeriodDataCheckingControl(StudyPeriodDataCheckingType eStudyPeriodDataCheckingType) {
  switch (eStudyPeriodDataCheckingType) {
    case RELAXEDBOUNDS  : rdgStudyPeriodCheck->ItemIndex = 1; break;
    case STRICTBOUNDS   :
    default             : rdgStudyPeriodCheck->ItemIndex = 0;
  }
}
//---------------------------------------------------------------------------
/** Sets time trend adjustment control's index */
void TfrmAdvancedParameters::SetTemporalTrendAdjustmentControl(TimeTrendAdjustmentType eTimeTrendAdjustmentType) {
  switch (eTimeTrendAdjustmentType) {
    case NOTADJUSTED               : rdgTemporalTrendAdj->ItemIndex = 0; break;
    case NONPARAMETRIC             : rdgTemporalTrendAdj->ItemIndex = 1; break;
    case LOGLINEAR_PERC            : rdgTemporalTrendAdj->ItemIndex = 2; break;
    case CALCULATED_LOGLINEAR_PERC : rdgTemporalTrendAdj->ItemIndex = 3; break;
    case STRATIFIED_RANDOMIZATION  : rdgTemporalTrendAdj->ItemIndex = 1; break;
    default                        : rdgTemporalTrendAdj->ItemIndex = 0;
  }
}
//---------------------------------------------------------------------------
/** internal setup function */
void TfrmAdvancedParameters::Setup() {
   const CParameters & ref = gAnalysisSettings.gParameters;

   try {
      // Spatial Window tab
      SetMaxSpatialClusterSizeControl(PERCENTOFPOPULATION, ref.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false));
      SetMaxSpatialClusterSizeControl(PERCENTOFMAXCIRCLEFILE, ref.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false));
      SetMaxSpatialClusterSizeControl(MAXDISTANCE, ref.GetMaxSpatialSizeForType(MAXDISTANCE, false));
      chkSpatialPopulationFile->Checked = ref.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false);
      chkSpatialDistance->Checked = ref.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false);

      edtMaxCirclePopulationFilename->Text = ref.GetMaxCirclePopulationFileName().c_str();
      chkInclPureTempClust->Checked = ref.GetIncludePurelyTemporalClusters();

      rdoCircular->Checked = ref.GetSpatialWindowType() == CIRCULAR;
      rdoElliptic->Checked = ref.GetSpatialWindowType() == ELLIPTIC;
      cmbNonCompactnessPenalty->ItemIndex = ref.GetNonCompactnessPenaltyType();
      SetSpatialDistanceCaption();

      // Temporal tab
      SetMaxTemporalClusterSizeTypeControl(ref.GetMaximumTemporalClusterSizeType());
      SetMaxTemporalClusterSizeControl(ref.GetMaximumTemporalClusterSize());
      chkIncludePureSpacClust->Checked = ref.GetIncludePurelySpatialClusters();
      chkRestrictTemporalRange->Checked = ref.GetIncludeClustersType() == CLUSTERSINRANGE;
      if (ref.GetStartRangeStartDate().length() > 0)
        ParseDate(ref.GetStartRangeStartDate(), *edtStartRangeStartYear, *edtStartRangeStartMonth, *edtStartRangeStartDay, true);
      if (ref.GetStartRangeEndDate().length() > 0)
        ParseDate(ref.GetStartRangeEndDate(), *edtStartRangeEndYear, *edtStartRangeEndMonth, *edtStartRangeEndDay, true);
      if (ref.GetEndRangeStartDate().length() > 0)
        ParseDate(ref.GetEndRangeStartDate(), *edtEndRangeStartYear, *edtEndRangeStartMonth, *edtEndRangeStartDay, false);
      if (ref.GetEndRangeEndDate().length() > 0)
        ParseDate(ref.GetEndRangeEndDate(), *edtEndRangeEndYear, *edtEndRangeEndMonth, *edtEndRangeEndDay, false);

      // Risk tab
      rdgSpatialAdjustments->ItemIndex = ref.GetSpatialAdjustmentType();
      chkAdjustForKnownRelativeRisks->Checked = ref.UseAdjustmentForRelativeRisksFile();
      edtAdjustmentsByRelativeRisksFile->Text = ref.GetAdjustmentsByRelativeRisksFilename().c_str();
      SetTemporalTrendAdjustmentControl(ref.GetTimeTrendAdjustmentType());
      if (ref.GetTimeTrendAdjustmentPercentage() <= -100)
        edtLogLinear->Text = 0;
      else
        edtLogLinear->Text = ref.GetTimeTrendAdjustmentPercentage();

      // Inference tab
      chkTerminateEarly->Checked = ref.GetTerminateSimulationsEarly();
      chkReportCriticalValues->Checked = ref.GetReportCriticalValues();
      chkPerformSequentialScan->Checked = ref.GetIsSequentialScanning();
      edtNumSequentialScans->Text = (ref.GetNumSequentialScansRequested() < 1 || ref.GetNumSequentialScansRequested() > (unsigned int)CParameters::MAXIMUM_SEQUENTIAL_ANALYSES ? 10 : ref.GetNumSequentialScansRequested());
      edtSequentialScanCutoff->Text = (ref.GetSequentialCutOffPValue() <= 0 || ref.GetSequentialCutOffPValue() > 1 ? 0.05 : ref.GetSequentialCutOffPValue());   

      // Output tab
      chkRestrictReportedClusters->Checked = ref.GetRestrictingMaximumReportedGeoClusterSize();
      rdgCriteriaSecClusters->ItemIndex = ref.GetCriteriaSecondClustersType();
      SetMaxReportedSpatialClusterSizeControl(PERCENTOFPOPULATION, ref.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, true));
      SetMaxReportedSpatialClusterSizeControl(PERCENTOFMAXCIRCLEFILE, ref.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true));
      SetMaxReportedSpatialClusterSizeControl(MAXDISTANCE, ref.GetMaxSpatialSizeForType(MAXDISTANCE, true));
      chkReportedSpatialPopulationFile->Checked = ref.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true);
      chkReportedSpatialDistance->Checked = ref.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true);
      
      // Input tab
      EnableInputFileEdits(false);
      for (unsigned int i = 1; i < ref.GetNumDataSets(); i++) { // multiple data sets
         lstInputDataSets->Items->Add("Data Set " + IntToStr(i+1));
         gvCaseFiles.push_back(AnsiString(ref.GetCaseFileName(i+1).c_str()));
         gvControlFiles.push_back(AnsiString(ref.GetControlFileName(i+1).c_str()));
         gvPopFiles.push_back(AnsiString(ref.GetPopulationFileName(i+1).c_str()));
         giDataSetNum++;
      }
      EnableDataSetList(lstInputDataSets->Items->Count);
      EnableDataSetPurposeControls(lstInputDataSets->Items->Count);
      lstInputDataSets->ItemIndex = -1;
      rdoMultivariate->Checked = ref.GetMultipleDataSetPurposeType() == MULTIVARIATE;
      rdoAdjustmentByDataSets->Checked = ref.GetMultipleDataSetPurposeType() == ADJUSTMENT;

      // Data Checking Tab
      SetStudyPeriodDataCheckingControl(ref.GetStudyPeriodDataCheckingType());
      SetCoordinatesDataCheckingControl(ref.GetCoordinatesDataCheckingType());

      // Neighbors Tab
      chkSpecifiyNeighborsFile->Checked = ref.UseLocationNeighborsFile();
      edtNeighborsFile->Text = ref.GetLocationNeighborsFileName().c_str(); 
    }
    catch (ZdException &x) {
      x.AddCallpath("Setup()","TfrmAdvancedParameters");
      throw;
    }
}
//---------------------------------------------------------------------------
/** sets control to focus when form shows then shows form modal */
void TfrmAdvancedParameters::ShowDialog(TWinControl * pFocusControl, int iCategory) {
  bool          bFound=false;
  int           i;

  // PAG - not the best coding here but am trying to show/hide only
  // certain pages/tabs of page control
  giCategory = iCategory;

  switch (iCategory){
     case INPUT_TABS:       // show Input page
        Caption = "Advanced Input Features";
        PageControl->Pages[0]->TabVisible=true;
        PageControl->Pages[1]->TabVisible=true;
        PageControl->Pages[2]->TabVisible=true;
        for (i=3; i < PageControl->PageCount; i++)
           PageControl->Pages[i]->TabVisible=false;
        // give control to list box if it contains items but none are selected
        if (lstInputDataSets->Items->Count && lstInputDataSets->ItemIndex == -1) {
           lstInputDataSets->ItemIndex = 0;
           lstInputDataSets->OnClick(this);
        }
        EnableNewButton();
        EnableRemoveButton();
        break;
     case ANALYSIS_TABS:    // show Analysis pages
        Caption = "Advanced Analysis Features";
        PageControl->Pages[0]->TabVisible=false;
        PageControl->Pages[1]->TabVisible=false;
        PageControl->Pages[2]->TabVisible=false;
        for (i=3; i < PageControl->PageCount-1; i++)
           PageControl->Pages[i]->TabVisible=true;
        PageControl->Pages[PageControl->PageCount-1]->TabVisible=false;
        break;
     case OUTPUT_TABS:     // show Output pages
        Caption = "Advanced Output Features";
        for (i=0; i < PageControl->PageCount-1; i++)
           PageControl->Pages[i]->TabVisible=false;
        PageControl->Pages[PageControl->PageCount-1]->TabVisible=true;
        break;
  }
  // locate the page the passed in control in on - that should be page to display
  for (i=0; i < PageControl->PageCount && !bFound; ++i) {
     if (PageControl->Pages[i]->ContainsControl(pFocusControl)) {
       PageControl->ActivePage = PageControl->Pages[i];
       gpFocusControl = pFocusControl;
       bFound=true;
     }
  }

  if (!bFound) { // if no passed-in control or it wasn't found
    gpFocusControl=0;
    PageControl->ActivePage = PageControl->FindNextPage(NULL, true, true);
    //PageControl->ActivePage = PageControl->Pages[0]; // first page
  }

  //reporting clusters text dependent on maximum spatial cluster size
  //-- ensure that it has text
  if (!edtMaxSpatialClusterSize->Text.Length())
    edtMaxSpatialClusterSize->Text = 50;

  // PAG - update the Set Defaults button enabling/disabling
  DoControlExit();

  ShowModal();
}
//---------------------------------------------------------------------------
/** event triggered when user clicks static text associated with rdoRelaxedCoordinates control. */
void __fastcall TfrmAdvancedParameters::stRelaxedCoodinatesClick(TObject *Sender) {
  rdoRelaxedCoordinates->Checked = true;
  rdoRelaxedCoordinates->SetFocus();
}
//---------------------------------------------------------------------------
/** event triggered when user clicks static text associated with rdoStrictCoordinates control. */
void __fastcall TfrmAdvancedParameters::stStrictCoodinatesClick(TObject *Sender) {
  rdoStrictCoordinates->Checked = true;
  rdoStrictCoordinates->SetFocus();
}
//---------------------------------------------------------------------------
/** validates all the settings in this dialog */
void TfrmAdvancedParameters::Validate() {
   ValidateInputFiles();
   ValidateNeighborsFileSettings();
   ValidateSpatialClusterSize();
   ValidateAdjustmentSettings();
   ValidateTemporalWindowSettings();
   if (chkAdjustForEarlierAnalyses->Enabled && chkAdjustForEarlierAnalyses->Checked)
      ValidateProspDateRange();        // inference tab settings
   ValidateOutputSettings();   
}
//---------------------------------------------------------------------------
/** validates adjustment settings - throws exception */
void TfrmAdvancedParameters::ValidateAdjustmentSettings() {
  bool bAnalysisIsPurelyTemporal = gAnalysisSettings.GetAnalysisControlType() == PURELYTEMPORAL ||
                                   gAnalysisSettings.GetAnalysisControlType() == PROSPECTIVEPURELYTEMPORAL;

  try {
    //validate spatial adjustments
    if (rdgSpatialAdjustments->Enabled && rdgSpatialAdjustments->ItemIndex == SPATIALLY_STRATIFIED_RANDOMIZATION) {
      if (chkIncludePureSpacClust->Enabled && chkIncludePureSpacClust->Checked)
         GenerateAFException("Spatial adjustments can not performed in conjunction\n"
                             " with the inclusion of purely spatial clusters.",
                             "ValidateAdjustmentSettings()", *rdgSpatialAdjustments, ANALYSIS_TABS);
      if (rdgTemporalTrendAdj->Enabled && GetAdjustmentTimeTrendControlType() == NOTADJUSTED)
         GenerateAFException("Spatial adjustments can not performed in conjunction\n"
                             "with the nonparametric temporal adjustment.",
                             "ValidateAdjustmentSettings()", *rdgSpatialAdjustments, ANALYSIS_TABS);
    }
    //validate temporal adjustments
    if (rdgTemporalTrendAdj->Enabled && gAnalysisSettings.GetModelControlType() == POISSON && bAnalysisIsPurelyTemporal &&
        gAnalysisSettings.edtPopFileName->Text.IsEmpty() && GetAdjustmentTimeTrendControlType() != NOTADJUSTED) {
         GenerateAFException("Temporal adjustments can not be performed for a purely temporal analysis\n"
                             "using the Poisson model, when no population file has been specfied.",
                             "ValidateAdjustmentSettings()", *rdgTemporalTrendAdj, ANALYSIS_TABS);
    }
    //validate spatial/temporal/space-time adjustments
    if (chkAdjustForKnownRelativeRisks->Enabled && chkAdjustForKnownRelativeRisks->Checked) {
      if (edtAdjustmentsByRelativeRisksFile->Text.IsEmpty())
        GenerateAFException("Please specify an adjustments file.",
                            "ValidateAdjustmentSettings()", *edtAdjustmentsByRelativeRisksFile, ANALYSIS_TABS);
      if (!ValidateFileAccess(edtAdjustmentsByRelativeRisksFile->Text.c_str()))
        GenerateAFException("The adjustments file could not be opened for reading.\n"
                            "Please confirm that the path and/or file name are valid\n"
                            "and that you have permissions to read from this directory\nand file.",
                            "ValidateNeighborsFileSettings()",*edtAdjustmentsByRelativeRisksFile, ANALYSIS_TABS);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateAdjustmentSettings()","TfrmAdvancedParameters");
    throw;
  }
}
//------------------------------------------------------------------
void TfrmAdvancedParameters::ValidateInputFiles() {
  bool bAnalysisIsPurelyTemporal = gAnalysisSettings.GetAnalysisControlType() == PURELYTEMPORAL ||
                                   gAnalysisSettings.GetAnalysisControlType() == PROSPECTIVEPURELYTEMPORAL;
  bool bFirstDataSetHasPopulationFile = !gAnalysisSettings.edtPopFileName->Text.IsEmpty();

  try {
    for (unsigned int i=0; i < gvCaseFiles.size(); i++){
       //Ensure that controls have this dataset display, should we need to
       //show window regarding an error with settings.
       lstInputDataSets->ItemIndex = i;
       lstInputDataSets->OnClick(this);
       //validate the case file for this dataset
       if (gvCaseFiles.at(i).IsEmpty())
          GenerateAFException("Please specify a case file for this additional data set.", "ValidateInputFiles()",*edtCaseFileName, INPUT_TABS);
       if (!ValidateFileAccess(gvCaseFiles.at(i).c_str()))
         GenerateAFException("The case file for this additional data set could not be opened for reading.\n"
                             "Please confirm that the path and/or file name are valid and that you have\n"
                             "permissions to read from this directory and file.", "ValidateInputFiles()",*edtCaseFileName, INPUT_TABS);
       //validate the control file for this dataset - Bernoulli model only
       if (gAnalysisSettings.GetModelControlType() == BERNOULLI) {
         if (gvControlFiles.at(i).IsEmpty())
           GenerateAFException("For the Bernoulli model, please specify a control file for this additional data set.","ValidateInputFiles()", *edtControlFileName, INPUT_TABS);
         if (!ValidateFileAccess(gvControlFiles.at(i).c_str()))
           GenerateAFException("The control file for this additional data set could not be opened for reading.\n"
                               "Please confirm that the path and/or file name are valid and that you have\n"
                               "permissions to read from this directory and file.", "ValidateInputFiles()",*edtControlFileName, INPUT_TABS);
       }
       //validate the population file for this dataset-  Poisson model only
       if (gAnalysisSettings.GetModelControlType() == POISSON) {
         //For purely temporal analyses, the population file is optional. But if one first
         //dataset does or does not supply a population file; the other dataset must do the same.
         if (bAnalysisIsPurelyTemporal) {
           if ((gvPopFiles.at(i).IsEmpty() && bFirstDataSetHasPopulationFile) ||
                (!gvPopFiles.at(i).IsEmpty() && !bFirstDataSetHasPopulationFile))
             GenerateAFException("For the Poisson model with purely temporal analyses, the population file is optional but all data\n"
                                 "sets must either specify a population file or omit it.","ValidateInputFiles()", *edtPopFileName, INPUT_TABS);
           else if (!gvPopFiles.at(i).IsEmpty() && !ValidateFileAccess(gvPopFiles.at(i).c_str()))
             GenerateAFException("The population file for this additional data set could not be opened for reading.\n"
                                 "Please confirm that the path and/or file name are valid and that you have\n"
                                 "permissions to read from this directory and file.", "ValidateInputFiles()",*edtPopFileName, INPUT_TABS);
         }
         else if (gvPopFiles.at(i).IsEmpty())
            GenerateAFException("For the Poisson model, please specify a population file for this additional data set.","ValidateInputFiles()", *edtPopFileName, INPUT_TABS);
         else if (!ValidateFileAccess(gvPopFiles.at(i).c_str()))
             GenerateAFException("The population file for this additional data set could not be opened for reading.\n"
                                 "Please confirm that the path and/or file name are valid and that you have\n"
                                 "permissions to read from this directory and file.", "ValidateInputFiles()",*edtPopFileName, INPUT_TABS);
       }
    }  //for loop

    //validate that purpose for multiple data sets is not 'adjustment' if probability model is ordinal
    if (gAnalysisSettings.GetModelControlType() == ORDINAL && rdoAdjustmentByDataSets->Enabled && rdoAdjustmentByDataSets->Checked)
      GenerateAFException("For the ordinal probability model with input data defined in multiple data sets,\n"
                          "the adjustment option has not been implemented.","ValidateInputFiles()", *rdoAdjustmentByDataSets, INPUT_TABS);
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateInputFiles()", "TfrmAdvancedParameters");
    throw;
  }
}

/** Validate user settings of the Neighbors File tab. */
void TfrmAdvancedParameters::ValidateNeighborsFileSettings() {
  try {
    if (grpNeighborsFile->Enabled && chkSpecifiyNeighborsFile->Enabled && chkSpecifiyNeighborsFile->Checked) {
      //validate the case file for this dataset
      if (edtNeighborsFile->Text.IsEmpty())
        GenerateAFException("Please specify a neighbors file.", "ValidateNeighborsFileSettings()",*edtNeighborsFile, INPUT_TABS);
      if (!ValidateFileAccess(edtNeighborsFile->Text.c_str()))
        GenerateAFException("The neighbors file could not be opened for reading.\n"
                            "Please confirm that the path and/or file name are\n"
                            "valid and that you have permissions to read from\nthis directory and file.",
                            "ValidateNeighborsFileSettings()",*edtNeighborsFile, INPUT_TABS);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateNeighborsFileSettings()", "TfrmAdvancedParameters");
    throw;
  }
}

//---------------------------------------------------------------------------
/** Specific prospective space-time date check
    Must be between the start and end dates of the analysis */
void TfrmAdvancedParameters::ValidateProspDateRange() {
  Julian        Start, End, Prosp;
  int           iProspYear, iProspMonth, iProspDay;

  try {
    if (edtProspectiveStartDateYear->Text.IsEmpty())
       GenerateAFException("Please specify a year.","ValidateProspDateRange()", *edtProspectiveStartDateYear, ANALYSIS_TABS);

    if (edtProspectiveStartDateMonth->Text.IsEmpty())
       GenerateAFException("Please specify a month.","ValidateProspDateRange()", *edtProspectiveStartDateMonth, ANALYSIS_TABS);

    if (edtProspectiveStartDateDay->Text.IsEmpty())
       GenerateAFException("Please specify a day.","ValidateProspDateRange()", *edtProspectiveStartDateDay, ANALYSIS_TABS);

    Start = MDYToJulian(atoi(gAnalysisSettings.edtStudyPeriodStartDateMonth->Text.c_str()),
                        atoi(gAnalysisSettings.edtStudyPeriodStartDateDay->Text.c_str()),
                        atoi(gAnalysisSettings.edtStudyPeriodStartDateYear->Text.c_str()));
    End = MDYToJulian(atoi(gAnalysisSettings.edtStudyPeriodEndDateMonth->Text.c_str()),
                      atoi(gAnalysisSettings.edtStudyPeriodEndDateDay->Text.c_str()),
                      atoi(gAnalysisSettings.edtStudyPeriodEndDateYear->Text.c_str()));
    iProspMonth = atoi(edtProspectiveStartDateMonth->Text.c_str());
    iProspDay = atoi(edtProspectiveStartDateDay->Text.c_str());
    iProspYear = atoi(edtProspectiveStartDateYear->Text.c_str());
    Prosp = MDYToJulian(iProspMonth, iProspDay, iProspYear);

    if ((Prosp < Start) || (Prosp > End)) {
      GenerateAFException("The prospective start date must be between the study period start and end dates.",
                           "ValidateProspDateRange()", *edtProspectiveStartDateYear, ANALYSIS_TABS);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateProspDateRange()", "TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
/** validates reported clusters limiting control setting - throws exception */
void TfrmAdvancedParameters::ValidateReportedSpatialClusterSize() {
  try {
    //When analysis type is prospective space-time, adjusting for earlier analyses - max spatial size must be defined
    //as percentage of max circle population or as a distance.
    if (gAnalysisSettings.GetAnalysisControlType() == PROSPECTIVESPACETIME && chkAdjustForEarlierAnalyses->Enabled &&
        chkAdjustForEarlierAnalyses->Checked && chkRestrictReportedClusters->Enabled && chkRestrictReportedClusters->Checked &&
        !chkReportedSpatialPopulationFile->Checked && !chkReportedSpatialDistance->Checked)
      GenerateAFException("For a prospective space-time analysis adjusting for ealier analyses, the maximum spatial\n"
                          "cluster size must be defined as a percentage of the population as defined in a max\n"
                          "circle size file.\n"
                          "Alternatively you may choose to specify the maximum as a fixed radius, in which case a\n"
                          "max circle size file is not required.\n", "ValidateReportedSpatialClusterSize()", *chkReportedSpatialPopulationFile, ANALYSIS_TABS);
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateReportedSpatialClusterSize()","TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
/** validates scanning window range settings - throws exception */
void TfrmAdvancedParameters::ValidateScanningWindowRanges() {
  Julian        StudyPeriodStartDate, StudyPeriodEndDate,
                StartRangeStartDate, StartRangeEndDate,
                EndRangeStartDate, EndRangeEndDate;
  try {
    if (chkRestrictTemporalRange->Enabled && chkRestrictTemporalRange->Checked) {
      StudyPeriodStartDate = MDYToJulian(atoi(gAnalysisSettings.edtStudyPeriodStartDateMonth->Text.c_str()),
                                         atoi(gAnalysisSettings.edtStudyPeriodStartDateDay->Text.c_str()),
                                         atoi(gAnalysisSettings.edtStudyPeriodStartDateYear->Text.c_str()));
      StudyPeriodEndDate = MDYToJulian(atoi(gAnalysisSettings.edtStudyPeriodEndDateMonth->Text.c_str()),
                                       atoi(gAnalysisSettings.edtStudyPeriodEndDateDay->Text.c_str()),
                                       atoi(gAnalysisSettings.edtStudyPeriodEndDateYear->Text.c_str()));
      StartRangeStartDate = MDYToJulian(atoi(edtStartRangeStartMonth->Text.c_str()),
                                        atoi(edtStartRangeStartDay->Text.c_str()),
                                        atoi(edtStartRangeStartYear->Text.c_str()));
      StartRangeEndDate = MDYToJulian(atoi(edtStartRangeEndMonth->Text.c_str()),
                                      atoi(edtStartRangeEndDay->Text.c_str()),
                                      atoi(edtStartRangeEndYear->Text.c_str()));
      EndRangeStartDate = MDYToJulian(atoi(edtEndRangeStartMonth->Text.c_str()),
                                      atoi(edtEndRangeStartDay->Text.c_str()),
                                      atoi(edtEndRangeStartYear->Text.c_str()));
      EndRangeEndDate = MDYToJulian(atoi(edtEndRangeEndMonth->Text.c_str()),
                                    atoi(edtEndRangeEndDay->Text.c_str()),
                                    atoi(edtEndRangeEndYear->Text.c_str()));

      //check that scanning ranges are within study period
      if (StartRangeStartDate < StudyPeriodStartDate || StartRangeStartDate > StudyPeriodEndDate)
        GenerateAFException("The scanning window start range does not occur within study period.",
                            "ValidateScanningWindowRanges()", *edtStartRangeStartYear, ANALYSIS_TABS);
      if (StartRangeEndDate < StudyPeriodStartDate || StartRangeEndDate > StudyPeriodEndDate)
        GenerateAFException("The scanning window start range does not occur within study period.",
                            "ValidateScanningWindowRanges()", *edtStartRangeEndYear, ANALYSIS_TABS);
      if (StartRangeStartDate > StartRangeEndDate)
        GenerateAFException("The scanning window start range dates conflict.",
                            "ValidateScanningWindowRanges()", *edtStartRangeStartYear, ANALYSIS_TABS);
      if (EndRangeStartDate < StudyPeriodStartDate || EndRangeStartDate > StudyPeriodEndDate)
        GenerateAFException("The scanning window end range does not occur within study period.",
                            "ValidateScanningWindowRanges()", *edtEndRangeStartYear, ANALYSIS_TABS);
      if (EndRangeEndDate < StudyPeriodStartDate || EndRangeEndDate > StudyPeriodEndDate)
        GenerateAFException("The scanning window end range does not occur within study period.",
                            "ValidateScanningWindowRanges()", *edtEndRangeEndYear, ANALYSIS_TABS);
      if (EndRangeStartDate > EndRangeEndDate)
        GenerateAFException("The scanning window end range dates conflict.",
                            "ValidateScanningWindowRanges()", *edtEndRangeStartYear, ANALYSIS_TABS);
      if (StartRangeStartDate >= EndRangeEndDate)
        GenerateAFException("The scanning window start range does not occur before end range.",
                            "ValidateScanningWindowRanges()", *edtStartRangeStartYear, ANALYSIS_TABS);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateScanningWindowRanges()","TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
void TfrmAdvancedParameters::ValidateSpatialClusterSize() {
  try {
    if (rdgSpatialOptions->Enabled && chkSpatialPopulationFile->Checked && edtMaxCirclePopulationFilename->Text.IsEmpty())
      GenerateAFException("Please specify a maximum circle size file.","ValidateSpatialClusterSize()", *edtMaxCirclePopulationFilename, ANALYSIS_TABS);

    if (rdgSpatialOptions->Enabled && chkSpatialPopulationFile->Checked && !ValidateFileAccess(edtMaxCirclePopulationFilename->Text.c_str()))
      GenerateAFException("The maximum circle file could not be opened for reading.\n"
                          "Please confirm that the path and/or file name are valid\n"
                          "and that you have permissions to read from this directory\nand file.",
                           "ValidateSpatialClusterSize()",*edtMaxCirclePopulationFilename, ANALYSIS_TABS);

    if (rdgReportedSpatialOptions->Enabled && chkRestrictReportedClusters->Checked &&
        chkReportedSpatialPopulationFile->Checked&& !ValidateFileAccess(edtMaxCirclePopulationFilename->Text.c_str()))
      GenerateAFException("The maximum circle file could not be opened for reading.\n"
                          "Please confirm that the path and/or file name are valid\n"
                          "and that you have permissions to read from this directory\nand file.\n"
                          "A maximum circle file is required when restricting the maximum\n"
                          "reported spatial cluster size by a population defined through a\n"
                          "maximum circle file.","ValidateSpatialClusterSize()", *edtMaxCirclePopulationFilename, ANALYSIS_TABS);
    //When analysis type is prospective space-time, adjusting for earlier analyses - max spatial size must be defined
    //as percentage of max circle population or as a distance.
    if (gAnalysisSettings.GetAnalysisControlType() == PROSPECTIVESPACETIME && chkAdjustForEarlierAnalyses->Enabled &&
        chkAdjustForEarlierAnalyses->Checked && !chkSpatialPopulationFile->Checked && !chkSpatialDistance->Checked)
      GenerateAFException("For a prospective space-time analysis adjusting for ealier analyses, the maximum spatial\n"
                          "cluster size must be defined as a percentage of the population as defined in a max\n"
                          "circle size file.\n"
                          "Alternatively you may choose to specify the maximum as a fixed radius, in which case a\n"
                          "max circle size file is not required.\n", "ValidateSpatialClusterSize()", *chkSpatialPopulationFile, ANALYSIS_TABS);
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateSpatialClusterSize()","TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
void TfrmAdvancedParameters::ValidateTemporalClusterSize() {
  ZdString      sErrorMessage, sPrecisionString;
  double        dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

  try {
    //check whether we are specifiying temporal information
    if (!rdgTemporalOptions->Enabled)
      return;

    if (GetMaxTemporalClusterSizeControlType() == PERCENTAGETYPE) {
      if (!edtMaxTemporalClusterSize->Text.Length() || edtMaxTemporalClusterSize->Text.ToDouble() == 0)
        GenerateAFException("Please specify a maximum temporal cluster size.","ValidateTemporalClusterSize()", *edtMaxTemporalClusterSize, ANALYSIS_TABS);
      //check maximum temporal cluster size(as percentage of population) is less than maximum for given probability model
      if (edtMaxTemporalClusterSize->Text.ToDouble() > (gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90)) {
        sErrorMessage.printf("For the %s model, the maximum temporal cluster size as a percent of the study period is %d percent.",
                             gAnalysisSettings.gParameters.GetProbabilityModelTypeAsString(gAnalysisSettings.GetModelControlType()),
                             (gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90));
        GenerateAFException(sErrorMessage.GetCString(), "ValidateTemporalClusterSize()", *edtMaxTemporalClusterSize, ANALYSIS_TABS);
      }
      //validate that the time aggregation length agrees with the study period and maximum temporal cluster size
      dStudyPeriodLengthInUnits = gAnalysisSettings.CalculateTimeAggregationUnitsInStudyPeriod();
      dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * edtMaxTemporalClusterSize->Text.ToDouble()/100.0);
      if (dMaxTemporalLengthInUnits < 1) {
        GetDatePrecisionAsString(gAnalysisSettings.GetTimeAggregationControlType(), sPrecisionString, false, false);
        sErrorMessage.printf("A maximum temporal cluster size as %g percent of a %d %s study period\n"
                             "results in a maximum temporal cluster size that is less than one time\n"
                             "aggregation %s.\n", edtMaxTemporalClusterSize->Text.ToDouble(),
                             static_cast<int>(dStudyPeriodLengthInUnits),
                             sPrecisionString.GetCString(), sPrecisionString.GetCString());
        GenerateAFException(sErrorMessage.GetCString(), "ValidateTemoralClusterSize()", *edtMaxTemporalClusterSize, ANALYSIS_TABS);
      }
    }
    else if (GetMaxTemporalClusterSizeControlType() == TIMETYPE) {
      if (!edtMaxTemporalClusterSizeUnits->Text.Length() || edtMaxTemporalClusterSizeUnits->Text.ToDouble() == 0)
        GenerateAFException("Please specify a maximum temporal cluster size.", "ValidateTemoralClusterSize()", *edtMaxTemporalClusterSizeUnits, ANALYSIS_TABS);
      GetDatePrecisionAsString(gAnalysisSettings.GetTimeAggregationControlType(), sPrecisionString, false, false);
      dStudyPeriodLengthInUnits = gAnalysisSettings.CalculateTimeAggregationUnitsInStudyPeriod();
      dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * (gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90)/100.0);
      if (edtMaxTemporalClusterSizeUnits->Text.ToDouble() > dMaxTemporalLengthInUnits) {
        sErrorMessage.printf("A maximum temporal cluster size of %d %s%s exceeds %d percent of a %d %s study period.\n"
                             "Note that current settings limit the maximum to %d %s%s.",
                             edtMaxTemporalClusterSizeUnits->Text.ToInt(), sPrecisionString.GetCString(),
                             (edtMaxTemporalClusterSizeUnits->Text.ToInt() == 1 ? "" : "s"),
                             (gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90),
                             static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.GetCString(),
                             static_cast<int>(dMaxTemporalLengthInUnits), sPrecisionString.GetCString(),
                             (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
        GenerateAFException(sErrorMessage.GetCString(), "ValidateTemoralClusterSize()", *edtMaxTemporalClusterSizeUnits, ANALYSIS_TABS);
      }
    }
    else
      ZdException::GenerateNotification("Unknown temporal percentage type: %d.","ValidateTemporalClusterSize()", GetMaxTemporalClusterSizeControlType());
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateTemporalClusterSize()","TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
/** validates output settings - throws exception */
void TfrmAdvancedParameters::ValidateOutputSettings() {
  try {
    ValidateReportedSpatialClusterSize();
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateOutputSettings()","TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
/** validates temporal window settings - throws exception */
void TfrmAdvancedParameters::ValidateTemporalWindowSettings() {
  try {
     ValidateTemporalClusterSize();
     ValidateScanningWindowRanges();
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateTemporalWindowSettings()","TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
/**  Construct. This is an alternate constructor for when the varArgs list for sMessage
     has already been prepared. Primarily, this will be used by derived classes.        */
AdvancedFeaturesException::AdvancedFeaturesException(va_list varArgs, const char *sMessage,
                                                     const char *sSourceModule, Level iLevel, TWinControl& FocusControl, int iTabCategory)
          : ZdException(varArgs, sMessage, sSourceModule, iLevel), gFocusControl(FocusControl), giTabCategory(iTabCategory) {
   SetData(varArgs, sMessage, sSourceModule, iLevel);
}
//---------------------------------------------------------------------------
/** Destructor. */
AdvancedFeaturesException::~AdvancedFeaturesException() {}
//---------------------------------------------------------------------------
/**  This function will throw the exception with the parameters.  It is equivalent to
     throw ZdException(...), but includes the ability to format the message string.
     This function should be used to generate all Zd Exceptions within ZD.             */
void GenerateAFException(const char * sMessage, const char * sSourceModule, TWinControl& FocusControl, int iTabCategory, ...) {
  va_list      varArgs;
  va_start (varArgs, sSourceModule);

  AdvancedFeaturesException  theException(varArgs, sMessage, sSourceModule, ZdException::Notify, FocusControl, iTabCategory);
  va_end(varArgs);

  throw theException;
}
//---------------------------------------------------------------------------

