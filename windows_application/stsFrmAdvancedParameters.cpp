//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "stsFrmAdvancedParameters.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "Grids_ts"
#pragma link "TSGrid"
#pragma resource "*.dfm"

/** Maximum number of additional input streams permitted. */
const int TfrmAdvancedParameters::MAXIMUM_STREAMS = 3;

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
void __fastcall TfrmAdvancedParameters::btnNewClick(TObject *Sender) {
   try {
     // add new name to list box
     EnableDataStreamList(true);
     EnableDataStreamPurposeControls(true);
     lstInputStreams->Items->Add("Data Set " + IntToStr(giStreamNum++));
     lstInputStreams->ItemIndex = (lstInputStreams->Items->Count-1);

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
     EnableDataStreamList(lstInputStreams->Items->Count);
     EnableDataStreamPurposeControls(lstInputStreams->Items->Count);
     DisplayBasisException(this, x);
   }
}
//---------------------------------------------------------------------------
// when user clicks on an input streams name, and the Remove button, remove the
// details and the input stream name
void __fastcall TfrmAdvancedParameters::btnRemoveStreamClick(TObject *Sender){
   int iStreamNum = -1;

   try {
      // determine the input stream selected
      iStreamNum = lstInputStreams->ItemIndex;
      // remove files
      edtCaseFileName->Text = "";
      gvCaseFiles.erase(gvCaseFiles.begin() + iStreamNum);
      edtControlFileName->Text = "";
      gvControlFiles.erase(gvControlFiles.begin() + iStreamNum);
      edtPopFileName->Text = "";
      gvPopFiles.erase(gvPopFiles.begin() + iStreamNum);

      // update remaining list box names
      for (int i=iStreamNum+1; i < lstInputStreams->Items->Count ;i++) {
         AnsiString s = (lstInputStreams->Items->Strings[i]);
         int num = s.SubString(10, 2).ToInt();
         lstInputStreams->Items->Strings[i] = ("Data Set " + IntToStr(--num));
      }
      // remove list box name
      lstInputStreams->Items->Delete(iStreamNum);
      EnableDataStreamList(lstInputStreams->Items->Count);
      EnableDataStreamPurposeControls(lstInputStreams->Items->Count);
      // select/highlight previous name in box
      if (lstInputStreams->Items->Count) {
         iStreamNum = (iStreamNum > 0) ? iStreamNum-1 : 0;
         lstInputStreams->ItemIndex = iStreamNum;
         lstInputStreams->OnClick(this);
      }
      else
         EnableInputFileEdits(false);
      giStreamNum--;

      EnableNewButton();
      EnableRemoveButton();
      DoControlExit();
   }
   catch (ZdException &x) {
     x.AddCallpath("lstInputStreamsClick()","TfrmAdvancedParameters");
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

/** event triggered when user selects restrict reported clusters check box */
void __fastcall TfrmAdvancedParameters::chkRestrictReportedClustersClick(TObject *Sender) {
  edtReportClustersSmallerThan->Enabled = rdgSpatialOptions->Enabled && chkRestrictReportedClusters->Checked;
  edtReportClustersSmallerThan->Color = rdgSpatialOptions->Enabled && chkRestrictReportedClusters->Checked ? clWindow : clInactiveBorder;
  DoControlExit();
}

/** event triggered when user selects restrict range check box */
void __fastcall TfrmAdvancedParameters::chkRestrictTemporalRangeClick(TObject *Sender) {
  EnableSpatialOutputOptions(rdgSpatialOptions->Enabled);
  EnableDatesByTimePrecisionUnits();
  DoControlExit();
}

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

/** event triggered when text of adjustment by relative risks edit control changes */
void __fastcall TfrmAdvancedParameters::edtAdjustmentsByRelativeRisksFileChange(TObject *Sender) {
  edtAdjustmentsByRelativeRisksFile->Hint = edtAdjustmentsByRelativeRisksFile->Text;
}

/** Event triggered when TEdit for case file changes. */
void __fastcall TfrmAdvancedParameters::edtCaseFileNameChange(TObject *Sender) {
  gvCaseFiles.at(lstInputStreams->ItemIndex) = edtCaseFileName->Text;
}

/** Event triggered when TEdit for control file changes. */
void __fastcall TfrmAdvancedParameters::edtControlFileNameChange(TObject *Sender) {
  gvControlFiles.at(lstInputStreams->ItemIndex) = edtControlFileName->Text;
}

/** event triggered when end window end ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtEndRangeEndDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtEndRangeEndYear, *edtEndRangeEndMonth, *edtEndRangeEndDay);
  DoControlExit();
}

/** event triggered when end window start ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtEndRangeStartDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtEndRangeStartYear, *edtEndRangeStartMonth, *edtEndRangeStartDay);
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when loginear percentage control exited */
void __fastcall TfrmAdvancedParameters::edtLogLinearExit(TObject *Sender) {
  if (edtLogLinear->Text.IsEmpty() || atof(edtLogLinear->Text.c_str()) <= -100)
    edtLogLinear->Text = 0;
  DoControlExit();
}
/** event triggered when text of maximum circle edit control changes */
void __fastcall TfrmAdvancedParameters::edtMaxCirclePopulationFilenameChange(TObject *Sender) {
  edtMaxCirclePopulationFilename->Hint = edtMaxCirclePopulationFilename->Text;
}
//---------------------------------------------------------------------------
/** event triggered when maximum spatial cluster size edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxSpatialClusterSizeExit(TObject *Sender) {
  if (edtMaxSpatialClusterSize->Text.IsEmpty() || atof(edtMaxSpatialClusterSize->Text.c_str()) == 0)
    edtMaxSpatialClusterSize->Text = 50;
  SetReportingSmallerClustersText();
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when maximum spatial cluster size, as percentage of population at risk, edit control changes */
void __fastcall TfrmAdvancedParameters::edtMaxSpatialClusterSizeChange(TObject *Sender) {
  if (edtMaxSpatialClusterSize->Text.Length())
    SetReportingSmallerClustersText();
}
//---------------------------------------------------------------------------
/** event triggered when maximum spatial cluster size, as percentage of population file, edit control is changes */
void __fastcall TfrmAdvancedParameters::edtMaxSpatialPercentFileChange(TObject *Sender) {
  if (edtMaxSpatialPercentFile->Text.Length())
    SetReportingSmallerClustersText();
}
//---------------------------------------------------------------------------
/** event triggered when maximum spatial cluster size, as percentage of population file, edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxSpatialPercentFileExit(TObject *Sender) {
  if (edtMaxSpatialPercentFile->Text.IsEmpty() || atof(edtMaxSpatialPercentFile->Text.c_str()) == 0)
    edtMaxSpatialPercentFile->Text = 50;
  SetReportingSmallerClustersText();
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when maximum spatial cluster size, as a radius, edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxSpatialRadiusChange(TObject *Sender) {
  if (edtMaxSpatialRadius->Text.Length())
    SetReportingSmallerClustersText();
}
//---------------------------------------------------------------------------
/** event triggered when maximum spatial cluster size, as a radius, edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxSpatialRadiusExit(TObject *Sender){
  if (edtMaxSpatialRadius->Text.IsEmpty() || atof(edtMaxSpatialRadius->Text.c_str()) == 0)
    edtMaxSpatialRadius->Text = 1;
  SetReportingSmallerClustersText();
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when maximum temporal cluster size edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxTemporalClusterSizeExit(TObject *Sender) {
  if (edtMaxTemporalClusterSize->Text.IsEmpty() || atof(edtMaxTemporalClusterSize->Text.c_str()) == 0)
    edtMaxTemporalClusterSize->Text = 50;
  DoControlExit();
}

//---------------------------------------------------------------------------
/** event triggered when maximum temporal cluster size edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxTemporalClusterSizeUnitsExit(TObject *Sender) {
  if (edtMaxTemporalClusterSizeUnits->Text.IsEmpty() || atof(edtMaxTemporalClusterSizeUnits->Text.c_str()) == 0)
    edtMaxTemporalClusterSizeUnits->Text = 1;
  DoControlExit();
}


/** Event triggered when TEdit for population file changes. */
void __fastcall TfrmAdvancedParameters::edtPopFileNameChange(TObject *Sender) {
  gvPopFiles.at(lstInputStreams->ItemIndex) = edtPopFileName->Text;
}

//---------------------------------------------------------------------------
/** event triggered when year control, of prospective start date, is exited. */
void __fastcall TfrmAdvancedParameters::edtProspectiveStartDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtProspectiveStartDateYear, *edtProspectiveStartDateMonth, *edtProspectiveStartDateDay);
  DoControlExit();
}

/** event triggered when 'Report only clusters smaller than ...' edit control is exited */
void __fastcall TfrmAdvancedParameters::edtReportClustersSmallerThanExit(TObject *Sender) {
  if (!edtReportClustersSmallerThan->Text.Length() || atof(edtReportClustersSmallerThan->Text.c_str()) == 0)
    edtReportClustersSmallerThan->Text = GetMaxSpatialClusterSizeFromControl();
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when start window end ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtStartRangeEndDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtStartRangeEndYear, *edtStartRangeEndMonth, *edtStartRangeEndDay);
  DoControlExit();
}
//---------------------------------------------------------------------------
/** event triggered when start window start ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtStartRangeStartDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtStartRangeStartYear, *edtStartRangeStartMonth, *edtStartRangeStartDay);
  DoControlExit();
}

void TfrmAdvancedParameters::EnableAdjustmentForSpatialOptionsGroup(bool bEnable) {
  rdgSpatialAdjustments->Enabled = bEnable;
}

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

void TfrmAdvancedParameters::EnableSettingsForAnalysisModelCombination() {
  bool  bPoisson(gAnalysisSettings.GetModelControlType() == POISSON),
        bSpaceTimePermutation(gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION);

  try {
    switch (gAnalysisSettings.GetAnalysisControlType()) {
      case PURELYSPATIAL             :
        EnableAdjustmentForTimeTrendOptionsGroup(false, false, false, false);
        EnableAdjustmentForSpatialOptionsGroup(false);
        EnableSpatialOptionsGroup(true, false, true);
        EnableTemporalOptionsGroup(false, false, false);
        EnableProspectiveSurveillanceGroup(false);
        EnableOutputOptions(true);
        break;
      case PURELYTEMPORAL            :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson, bPoisson);
        EnableAdjustmentForSpatialOptionsGroup(false);
        EnableSpatialOptionsGroup(false, false, false);
        EnableTemporalOptionsGroup(true, false, true);
        EnableProspectiveSurveillanceGroup(false);
        EnableOutputOptions(false);
        break;
      case SPACETIME                 :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, bPoisson, bPoisson, bPoisson);
        EnableAdjustmentForSpatialOptionsGroup(bPoisson);
        EnableSpatialOptionsGroup(true, !bSpaceTimePermutation, true);
        EnableTemporalOptionsGroup(true, !bSpaceTimePermutation, true);
        EnableProspectiveSurveillanceGroup(false);
        EnableOutputOptions(true);
        break;
      case PROSPECTIVESPACETIME      :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, bPoisson, bPoisson, bPoisson);
        EnableAdjustmentForSpatialOptionsGroup(bPoisson);
        EnableSpatialOptionsGroup(true, !bSpaceTimePermutation, !chkAdjustForEarlierAnalyses->Checked);
        EnableTemporalOptionsGroup(true, !bSpaceTimePermutation, false);
        EnableProspectiveSurveillanceGroup(true);
        EnableOutputOptions(true);
        break;
      case PROSPECTIVEPURELYTEMPORAL :
        EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson, bPoisson);
        EnableAdjustmentForSpatialOptionsGroup(false);
        EnableSpatialOptionsGroup(false, false, false);
        EnableTemporalOptionsGroup(true, false, false);
        EnableProspectiveSurveillanceGroup(true);
        EnableOutputOptions(false);
        break;
      default :
        ZdGenerateException("Unknown analysis type '%d'.",
                            "EnableSettingsForAnalysisModelCombination()",gAnalysisSettings.GetAnalysisControlType());
    }
    EnableAdjustmentsGroup(bPoisson);
  }
  catch (ZdException &x) {
    x.AddCallpath("EnableSettingsForAnalysisModelCombination()","TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
/** Enables/disables TListBox that list defined data streams */
void TfrmAdvancedParameters::EnableDataStreamList(bool bEnable) {
  lstInputStreams->Enabled = bEnable;
  lstInputStreams->Color = lstInputStreams->Enabled ? clWindow : clInactiveBorder;
}

/** Enables/disables controls that indicate purpose of additional data streams. */
void TfrmAdvancedParameters::EnableDataStreamPurposeControls(bool bEnable) {
  lblMultipleStreamPurpose->Enabled = bEnable;
  rdoMultivariate->Enabled = bEnable;
  rdoAdjustmentByStreams->Enabled = bEnable;
}

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
//---------------------------------------------------------------------------
//** enables or disables the New button on the Input tab
void TfrmAdvancedParameters::EnableNewButton() {
  btnNewStream->Enabled = (lstInputStreams->Items->Count < MAXIMUM_STREAMS) ? true: false;
}
//---------------------------------------------------------------------------
/** enables adjustment options controls */
void TfrmAdvancedParameters::EnableOutputOptions(bool bEnable) {
   rdgCriteriaSecClusters->Enabled = bEnable;
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
  btnRemoveStream->Enabled = (lstInputStreams->Items->Count > 0) ? true: false;
}
//---------------------------------------------------------------------------
/** enables or disables the spatial options group control */
void TfrmAdvancedParameters::EnableSpatialOptionsGroup(bool bEnable, bool bEnableIncludePurelyTemporal, bool bEnablePercentage) {
   rdgSpatialOptions->Enabled = bEnable;

   rdoSpatialPercentage->Enabled = bEnable && bEnablePercentage;
   edtMaxSpatialClusterSize->Enabled = bEnable && bEnablePercentage && rdoSpatialPercentage->Checked;
   edtMaxSpatialClusterSize->Color = bEnable && bEnablePercentage && rdoSpatialPercentage->Checked ? clWindow : clInactiveBorder;
   lblPercentOfPopulation->Enabled = bEnable && bEnablePercentage;

   rdoSpatialPopulationFile->Enabled = bEnable;
   edtMaxSpatialPercentFile->Enabled = bEnable && rdoSpatialPopulationFile->Checked;
   edtMaxSpatialPercentFile->Color = bEnable && rdoSpatialPopulationFile->Checked ? clWindow : clInactiveBorder;
   lblPercentageOfPopFile->Enabled = bEnable;
   edtMaxCirclePopulationFilename->Enabled = bEnable && rdoSpatialPopulationFile->Checked;
   edtMaxCirclePopulationFilename->Color = bEnable && rdoSpatialPopulationFile->Checked ? clWindow : clInactiveBorder;
   btnBrowseMaxCirclePopFile->Enabled = bEnable && rdoSpatialPopulationFile->Checked;
   btnImportMaxCirclePopFile->Enabled = bEnable && rdoSpatialPopulationFile->Checked;

   rdoSpatialDistance->Enabled = bEnable;
   edtMaxSpatialRadius->Enabled = bEnable && rdoSpatialDistance->Checked;
   edtMaxSpatialRadius->Color = bEnable && rdoSpatialDistance->Checked ? clWindow : clInactiveBorder;
   lblMaxRadius->Enabled = bEnable;

   if (!rdoSpatialPercentage->Enabled && rdoSpatialDistance->Enabled && rdoSpatialPercentage->Checked)
     rdoSpatialDistance->Checked = true;

   chkInclPureTempClust->Enabled = bEnable && bEnableIncludePurelyTemporal;
   EnableSpatialOutputOptions(bEnable);
}

//---------------------------------------------------------------------------
/** enables spatial output options controls - (on output tab now)*/
void TfrmAdvancedParameters::EnableSpatialOutputOptions(bool bEnable) {
  chkRestrictReportedClusters->Enabled = bEnable;
  edtReportClustersSmallerThan->Enabled = bEnable && chkRestrictReportedClusters->Checked;
  edtReportClustersSmallerThan->Color = bEnable && chkRestrictReportedClusters->Checked ? clWindow : clInactiveBorder;
  lblReportSmallerClusters->Enabled = bEnable;
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
//** Checks to determine if only default values are set in the dialog
//** Returns true if only default values are set on enabled controls
//** Returns false if user specified a value other than a default
bool TfrmAdvancedParameters::GetDefaultsSetForAnalysisOptions() {
   bool bReturn = true;

   // Inference tab
   bReturn &= (chkAdjustForEarlierAnalyses->Checked == false);
   bReturn &= (chkTerminateEarly->Checked == false);
   bReturn &= (edtProspectiveStartDateYear->Text.ToInt() == 2000);
   bReturn &= (edtProspectiveStartDateMonth->Text.ToInt() == 12);
   bReturn &= (edtProspectiveStartDateDay->Text.ToInt() == 31);

   // Spatial Window tab
   bReturn &= (GetMaxSpatialClusterSizeControlType()==PERCENTOFPOPULATIONTYPE);
   bReturn &= (edtMaxSpatialClusterSize->Text.ToDouble() == 50);
   bReturn &= (edtMaxSpatialPercentFile->Text.ToDouble() == 50);
   bReturn &= (edtMaxSpatialRadius->Text.ToDouble() == 1);
   bReturn &= (edtMaxCirclePopulationFilename->Text == "");
   bReturn &= (chkInclPureTempClust->Checked == false);

   // Temporal tab
   bReturn &= (GetMaxTemporalClusterSizeControlType()==PERCENTAGETYPE);
   bReturn &= (edtMaxTemporalClusterSize->Text.ToDouble() == 50);
   bReturn &= (edtMaxTemporalClusterSizeUnits->Text.ToInt() == 1);
   bReturn &= (chkIncludePureSpacClust->Checked == false);

   bReturn &= (edtStartRangeStartYear->Text.ToInt() == 2000);
   bReturn &= (edtStartRangeStartMonth->Text.ToInt() == 1);
   bReturn &= (edtStartRangeStartDay->Text.ToInt() == 1);
   bReturn &= (edtStartRangeEndYear->Text.ToInt() == 2000);
   bReturn &= (edtStartRangeEndMonth->Text.ToInt() == 12);
   bReturn &= (edtStartRangeEndDay->Text.ToInt() == 31);
   bReturn &= (edtEndRangeStartYear->Text.ToInt() == 2000);
   bReturn &= (edtEndRangeStartMonth->Text.ToInt() == 1);
   bReturn &= (edtEndRangeStartDay->Text.ToInt() == 1);
   bReturn &= (edtEndRangeEndYear->Text.ToInt() == 2000);
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

   bReturn &= (lstInputStreams->Items->Count == 0);
   bReturn &= (rdoMultivariate->Checked);

   return bReturn;
}
//---------------------------------------------------------------------------
//** Checks to determine if only default values are set in the dialog
//** Returns true if only default values are set
//** Returns false if user specified a value other than a default
bool TfrmAdvancedParameters::GetDefaultsSetForOutputOptions() {
   bool bReturn = true;

   // Output tab
   bReturn &= (chkRestrictReportedClusters->Checked == false);
   bReturn &= (rdgCriteriaSecClusters->ItemIndex == NOGEOOVERLAP);
   bReturn &= (edtReportClustersSmallerThan->Text.ToDouble() == 50.0);

   return bReturn;
}
//-----------------------------------------------------------------------------
/** returns maximum spatial cluster size type for control */
SpatialSizeType TfrmAdvancedParameters::GetMaxSpatialClusterSizeControlType() const {
  SpatialSizeType   eReturn;

  if (rdoSpatialPercentage->Checked)
    eReturn = PERCENTOFPOPULATIONTYPE;
  else if (rdoSpatialPopulationFile->Checked)
    eReturn = PERCENTOFPOPULATIONFILETYPE;
  else if (rdoSpatialDistance->Checked)
    eReturn = DISTANCETYPE;
  else
    ZdGenerateException("Maximum spatial cluster size type not selected.","GetMaxSpatialClusterSizeControlType()");

  return eReturn;
}
//---------------------------------------------------------------------------
/** returns maximum spatial cluster size from appropriate control */
float TfrmAdvancedParameters::GetMaxSpatialClusterSizeFromControl() const {
  float   fReturn;

  switch (GetMaxSpatialClusterSizeControlType()) {
    case DISTANCETYPE                : fReturn = atof(edtMaxSpatialRadius->Text.c_str()); break;
    case PERCENTOFPOPULATIONFILETYPE : fReturn = atof(edtMaxSpatialPercentFile->Text.c_str()); break;
    case PERCENTOFPOPULATIONTYPE     :
    default                          : fReturn = atof(edtMaxSpatialClusterSize->Text.c_str());
  }
  return fReturn;
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
float TfrmAdvancedParameters::GetMaxTemporalClusterSizeFromControl() const {
  float fReturn;

  switch (GetMaxTemporalClusterSizeControlType()) {
    case TIMETYPE        : fReturn = atof(edtMaxTemporalClusterSizeUnits->Text.c_str()); break;
    case PERCENTAGETYPE  :
    default              : fReturn = atof(edtMaxTemporalClusterSize->Text.c_str());
  }
  return fReturn;
}
//---------------------------------------------------------------------------
/** class initialization */
void TfrmAdvancedParameters::Init() {
  gpFocusControl=0;
  rdgCriteriaSecClusters->ItemIndex = 0;
  giCategory = 0;
  giStreamNum = 2;
}
//---------------------------------------------------------------------------
/** Modally shows import dialog. */
void TfrmAdvancedParameters::LaunchImporter(const char * sFileName, InputFileType eFileType) {
  ZdString sNewFile = "";

  try {
    std::auto_ptr<TBDlgDataImporter> pDialog(new TBDlgDataImporter(this, sFileName, eFileType, (CoordinatesType)(gAnalysisSettings.rgpCoordinates->ItemIndex)));
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
// when user clicks on an input streams name, display the details in the edit
// boxes above the list box
void __fastcall TfrmAdvancedParameters::lstInputStreamsClick(TObject *Sender) {
   int iStreamNum = -1;

   try {
      // determine the input stream selected
      iStreamNum = lstInputStreams->ItemIndex;
      // set the files
      EnableInputFileEdits(true);
      edtCaseFileName->Text = gvCaseFiles.at(iStreamNum);
      edtControlFileName->Text = gvControlFiles.at(iStreamNum);
      edtPopFileName->Text = gvPopFiles.at(iStreamNum);
   }
   catch (ZdException &x) {
     x.AddCallpath("lstInputStreamsClick()","TfrmAdvancedParameters");
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
/** event triggered when 'Adjustment for time trend' type control clicked */
void __fastcall TfrmAdvancedParameters::rdgTemporalTrendAdjClick(TObject *Sender) {
  switch (GetAdjustmentTimeTrendControlType()) {
    case LOGLINEAR_PERC : edtLogLinear->Enabled = true;
                          edtLogLinear->Color = clWindow;
                          break;
    default             : edtLogLinear->Enabled = false;
                          edtLogLinear->Color = clInactiveBorder;
  }
  DoControlExit();
}
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
  EnableSpatialOptionsGroup(rdgSpatialOptions->Enabled,
                            chkInclPureTempClust->Enabled,
                            rdoSpatialPercentage->Enabled);
  SetReportingSmallerClustersText();
  DoControlExit();
}
//---------------------------------------------------------------------------
/** parameter settings to parameters class */
void TfrmAdvancedParameters::SaveParameterSettings() {
  CParameters&  ref = const_cast<CParameters&>(gAnalysisSettings.gParameters);
  ZdString      sString;

  try {
    ref.SetUseAdjustmentForRelativeRisksFile(chkAdjustForKnownRelativeRisks->Checked);
    ref.SetAdjustmentsByRelativeRisksFilename(edtAdjustmentsByRelativeRisksFile->Text.c_str(), false);
    ref.SetTimeTrendAdjustmentType(rdgTemporalTrendAdj->Enabled ? GetAdjustmentTimeTrendControlType() : NOTADJUSTED);
    ref.SetTimeTrendAdjustmentPercentage(atof(edtLogLinear->Text.c_str()));
    ref.SetSpatialAdjustmentType((SpatialAdjustmentType)rdgSpatialAdjustments->ItemIndex);
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
    ref.SetCriteriaForReportingSecondaryClusters((CriteriaSecondaryClustersType)rdgCriteriaSecClusters->ItemIndex);

    // save the input files on Input tab
    ref.SetNumDataStreams(lstInputStreams->Items->Count + 1);
    if (lstInputStreams->Items->Count) {
       for (int i = 0; i < lstInputStreams->Items->Count; i++) {
          ref.SetCaseFileName((gvCaseFiles.at(i)).c_str(), false, i+2);
          ref.SetControlFileName((gvControlFiles.at(i)).c_str(), false, i+2);
          ref.SetPopulationFileName((gvPopFiles.at(i)).c_str(), false, i+2);
       }
    }
    ref.SetMultipleDataStreamPurposeType(rdoAdjustmentByStreams->Checked ? ADJUSTMENT: MULTIVARIATE);
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveParameterSettings()","TfrmAdvancedParameters");
    throw;
  }
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

   // Spatial Window tab
   SetMaxSpatialClusterSizeTypeControl(PERCENTOFPOPULATIONTYPE);
   edtMaxSpatialClusterSize->Text = "50";
   edtMaxSpatialPercentFile->Text = "50";
   edtMaxSpatialRadius->Text = "1";
   edtMaxCirclePopulationFilename->Text = "";
   chkInclPureTempClust->Checked = false;
   SetSpatialDistanceCaption();
   SetReportingSmallerClustersText();

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
/** Sets default values for Output related tab and respective controls
    PAG:  pulled these default values from the CParameter class */
void TfrmAdvancedParameters::SetDefaultsForOutputTab() {

   chkRestrictReportedClusters->Checked = false;
   rdgCriteriaSecClusters->ItemIndex = NOGEOOVERLAP;
   edtReportClustersSmallerThan->Text = 50;
}
//---------------------------------------------------------------------------
/** Sets default values for Input related tab and respective controls */
void TfrmAdvancedParameters::SetDefaultsForInputTab() {
   // clear all visual components
   edtCaseFileName->Text = "";
   edtControlFileName->Text = "";
   edtPopFileName->Text = "";
   lstInputStreams->Items->Clear();
   EnableDataStreamList(lstInputStreams->Items->Count);
   EnableDataStreamPurposeControls(lstInputStreams->Items->Count);

   // clear the non-visual components
   gvCaseFiles.clear();
   gvControlFiles.clear();
   gvPopFiles.clear();
   giStreamNum = 2;
   EnableNewButton();
   EnableRemoveButton();
   EnableInputFileEdits(false);
   rdoMultivariate->Checked = true;
}
//---------------------------------------------------------------------------
/** Sets adjustments filename in interface */
void TfrmAdvancedParameters::SetAdjustmentsByRelativeRisksFile(const char * sAdjustmentsByRelativeRisksFileName) {
  edtAdjustmentsByRelativeRisksFile->Text = sAdjustmentsByRelativeRisksFileName;
}
//---------------------------------------------------------------------------
/** Set appropriate control for maximum spatial cluster size type. */
void TfrmAdvancedParameters::SetMaxSpatialClusterSizeControl(float fMaxSize) {
   switch (GetMaxSpatialClusterSizeControlType()) {
     case DISTANCETYPE:
        if (fMaxSize <= 0)
           edtMaxSpatialRadius->Text = 1;
        else
           edtMaxSpatialRadius->Text = fMaxSize;
        break;
     case PERCENTOFPOPULATIONFILETYPE:
        if (fMaxSize <= 0 || fMaxSize > 50)
           edtMaxSpatialPercentFile->Text = 50;
        else
           edtMaxSpatialPercentFile->Text = fMaxSize;
        break;
    case PERCENTOFPOPULATIONTYPE:
    default:
        if (fMaxSize <= 0 || fMaxSize > 50)
           edtMaxSpatialClusterSize->Text = 50;
        else
           edtMaxSpatialClusterSize->Text = fMaxSize;
   }
   SetReportingSmallerClustersText();
}
//---------------------------------------------------------------------------
/** Sets maximum spatial cluster size control for passed type */
void TfrmAdvancedParameters::SetMaxSpatialClusterSizeTypeControl(SpatialSizeType eSpatialSizeType) {
  switch (eSpatialSizeType) {
    case DISTANCETYPE                : rdoSpatialDistance->Checked = true; break;
    case PERCENTOFPOPULATIONFILETYPE : rdoSpatialPopulationFile->Checked = true; break;
    case PERCENTOFPOPULATIONTYPE     :
    default                          : rdoSpatialPercentage->Checked = true;
  }
}
//---------------------------------------------------------------------------
/** Set appropriate control for maximum spatial cluster size type. */
void TfrmAdvancedParameters::SetMaxTemporalClusterSizeControl(float fMaxSize) {
  switch (GetMaxTemporalClusterSizeControlType()) {
    case TIMETYPE:
       if (fMaxSize <= 0)
          edtMaxTemporalClusterSizeUnits->Text = 1;
       else
          edtMaxTemporalClusterSizeUnits->Text = static_cast<int>(fMaxSize);
       break;
    case PERCENTAGETYPE :
    default             :
       if (fMaxSize <= 0 || fMaxSize > 50)
          edtMaxTemporalClusterSize->Text = 50;
       else
          edtMaxTemporalClusterSize->Text = fMaxSize;
  }
  SetReportingSmallerClustersText();
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

/** sets static label that describes what the reporting clusters will be limited as */
void TfrmAdvancedParameters::SetReportingClustersText(const ZdString& sText) {
  lblReportSmallerClusters->Caption = sText.GetCString();
}
//---------------------------------------------------------------------------
/** sets caption of label next to spatial option for reporting smaller clusters
    to reflect limitations specified by maximum spatial cluster size edit control */
void TfrmAdvancedParameters::SetReportingSmallerClustersText() {
  ZdString      sTemp;

  switch (GetMaxSpatialClusterSizeControlType()) {
    case PERCENTOFPOPULATIONTYPE     :
      sTemp.printf("percent of population at risk (<= %s%%)", edtMaxSpatialClusterSize->Text.c_str());
      break;
    case PERCENTOFPOPULATIONFILETYPE :
      sTemp.printf("percent of population at risk (<= %s%%)", edtMaxSpatialPercentFile->Text.c_str());
      break;
    case DISTANCETYPE:
      if (gAnalysisSettings.rgpCoordinates->ItemIndex == CARTESIAN)
        sTemp.printf("Cartesian units in radius (<= %s)", edtMaxSpatialRadius->Text.c_str());
      else
        sTemp.printf("kilometers in radius (<= %s)", edtMaxSpatialRadius->Text.c_str());
  }

  SetReportingClustersText(sTemp);
}
//---------------------------------------------------------------------------
/** Sets caption of spatial distance radio button based upon coordinates group setting. */
void TfrmAdvancedParameters::SetSpatialDistanceCaption() {
  try {
    switch (gAnalysisSettings.rgpCoordinates->ItemIndex) {
      case 0  : lblMaxRadius->Caption = "Cartesian units radius";
                break;
      case 1  : lblMaxRadius->Caption = "kilometer radius";
                break;
      default : ZdException::Generate("Unknown coordinates radio button index: '%i'.",
                                      "rgpCoordinatesClick()", gAnalysisSettings.rgpCoordinates->ItemIndex);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("SetSpatialDistanceCaption()", "TfrmAdvancedParameters");
    throw;
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

/** internal setup function */
void TfrmAdvancedParameters::Setup() {
   const CParameters & ref = gAnalysisSettings.gParameters;

   try {
      // Spatial Window tab
      SetMaxSpatialClusterSizeTypeControl(ref.GetMaxGeographicClusterSizeType());
      SetMaxSpatialClusterSizeControl(ref.GetMaximumGeographicClusterSize());
      edtMaxCirclePopulationFilename->Text = ref.GetMaxCirclePopulationFileName().c_str();
      chkInclPureTempClust->Checked = ref.GetIncludePurelyTemporalClusters();
      SetSpatialDistanceCaption();
      SetReportingSmallerClustersText();

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

      // Output tab
      chkRestrictReportedClusters->Checked = ref.GetRestrictingMaximumReportedGeoClusterSize();
      rdgCriteriaSecClusters->ItemIndex = ref.GetCriteriaSecondClustersType();
      if (ref.GetMaximumReportedGeoClusterSize() > 0)
        edtReportClustersSmallerThan->Text = ref.GetMaximumReportedGeoClusterSize();
      else
        edtReportClustersSmallerThan->Text = 50;

      // Input tab
      EnableInputFileEdits(false);
      for (unsigned int i = 1; i < ref.GetNumDataStreams(); i++) { // multiple data streams
         lstInputStreams->Items->Add("Data Set " + IntToStr(i+1));
         gvCaseFiles.push_back(AnsiString(ref.GetCaseFileName(i+1).c_str()));
         gvControlFiles.push_back(AnsiString(ref.GetControlFileName(i+1).c_str()));
         gvPopFiles.push_back(AnsiString(ref.GetPopulationFileName(i+1).c_str()));
         giStreamNum++;
      }
      EnableDataStreamList(lstInputStreams->Items->Count);
      EnableDataStreamPurposeControls(lstInputStreams->Items->Count);
      lstInputStreams->ItemIndex = -1;
      rdoMultivariate->Checked = ref.GetMultipleDataStreamPurposeType() == MULTIVARIATE;
      rdoAdjustmentByStreams->Checked = ref.GetMultipleDataStreamPurposeType() == ADJUSTMENT;
    }
    catch (ZdException &x) {
      x.AddCallpath("Setup()","TfrmAdvancedParameters");
      throw;
    }
}

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
        for (i=1; i < PageControl->PageCount; i++)
           PageControl->Pages[i]->TabVisible=false;
        // give control to list box if it contains items but none are selected
        if (lstInputStreams->Items->Count && lstInputStreams->ItemIndex == -1) {
           lstInputStreams->ItemIndex = 0;
           lstInputStreams->OnClick(this);
        }
        EnableNewButton();
        EnableRemoveButton();
        break;
     case ANALYSIS_TABS:    // show Analysis pages
        Caption = "Advanced Analysis Features";
        PageControl->Pages[0]->TabVisible=false;
        for (i=1; i < PageControl->PageCount-1; i++)
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
//------------------------------------------------------------------
/** validates all the settings in this dialog */
void TfrmAdvancedParameters::Validate() {
   ValidateInputFiles();
   ValidateSpatialClusterSize();
   ValidateAdjustmentSettings();
   ValidateTemporalWindowSettings();
   if (chkAdjustForEarlierAnalyses->Enabled && chkAdjustForEarlierAnalyses->Checked)
      ValidateProspDateRange();        // inference tab settings
   ValidateOutputSettings();   
}

/** validates adjustment settings - throws exception */
void TfrmAdvancedParameters::ValidateAdjustmentSettings() {
  try {
    //validate spatial adjustments
    if (rdgSpatialAdjustments->Enabled && rdgSpatialAdjustments->ItemIndex == SPATIALLY_STRATIFIED_RANDOMIZATION) {
      if (chkIncludePureSpacClust->Enabled && chkIncludePureSpacClust->Checked)
         GenerateAFException("Spatial adjustments can not performed in conjunction\n"
                             " with the inclusion of purely spatial clusters.",
                             "ValidateAdjustmentSettings()", *rdgSpatialAdjustments, ANALYSIS_TABS);
      if (rdgTemporalTrendAdj->Enabled && GetAdjustmentTimeTrendControlType() == STRATIFIED_RANDOMIZATION)
         GenerateAFException("Spatial adjustments can not performed in conjunction\n"
                             "with the nonparametric temporal adjustment.",
                             "ValidateAdjustmentSettings()", *rdgSpatialAdjustments, ANALYSIS_TABS);
    }
    //validate spatial/temporal/space-time adjustments
    if (chkAdjustForKnownRelativeRisks->Enabled && chkAdjustForKnownRelativeRisks->Checked) {
      if (edtAdjustmentsByRelativeRisksFile->Text.IsEmpty())
        GenerateAFException("Please specify an adjustments file.",
                            "ValidateAdjustmentSettings()", *edtAdjustmentsByRelativeRisksFile, ANALYSIS_TABS);
      if (!File_Exists(edtAdjustmentsByRelativeRisksFile->Text.c_str()))
        GenerateAFException("Adjustments file could not be opened.",
                            "ValidateAdjustmentSettings()", *edtAdjustmentsByRelativeRisksFile, ANALYSIS_TABS);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateAdjustmentSettings()","TfrmAdvancedParameters");
    throw;
  }
}
//---------------------------------------------------------------------------
/** Validates 'Input Files' */
void TfrmAdvancedParameters::ValidateInputFilesAtInput() {
  try {
    //validate the case file
    if (edtCaseFileName->Text.IsEmpty()) {
      edtCaseFileName->SetFocus();
      GenerateAFException("Please specify a case file.", "ValidateInputFiles()",*edtCaseFileName, INPUT_TABS);
    }
    if (!File_Exists(edtCaseFileName->Text.c_str())) {
      edtCaseFileName->SetFocus();
      GenerateAFException("Case file could not be opened.", "ValidateInputFiles()",*edtCaseFileName, INPUT_TABS);
    }

    //validate the control file - Bernoulli model only
    if (gAnalysisSettings.GetModelControlType() == BERNOULLI) {
      if (edtControlFileName->Text.IsEmpty()) {
        edtControlFileName->SetFocus();
        GenerateAFException("For the Bernoulli model, please specify a control file.","ValidateInputFiles()", *edtControlFileName, INPUT_TABS);
      }
      if (!File_Exists(edtControlFileName->Text.c_str())) {
        edtControlFileName->SetFocus();
        GenerateAFException("Control file could not be opened.","ValidateInputFiles()", *edtControlFileName, INPUT_TABS);
      }
    }

    //validate the population file -  Poisson model only
    if (gAnalysisSettings.GetModelControlType() == POISSON) {
      if (edtPopFileName->Text.IsEmpty()) {
        edtPopFileName->SetFocus();
        GenerateAFException("For the Poisson model, please specify a population file.","ValidateInputFiles()", *edtPopFileName, INPUT_TABS);
      }
      if (!File_Exists(edtPopFileName->Text.c_str())) {
        edtPopFileName->SetFocus();
        GenerateAFException("Population file could not be opened.","ValidateInputFiles()", *edtPopFileName, INPUT_TABS);
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateInputFilesAtInput()", "TfrmAdvancedParameters");
    throw;
  }
}
//------------------------------------------------------------------
void TfrmAdvancedParameters::ValidateInputFiles() {
  try {
    for (unsigned int i = 0; i < gvCaseFiles.size(); i++){
       lstInputStreams->ItemIndex = i;
       lstInputStreams->OnClick(this);
       //validate the case file
       if (gvCaseFiles.at(i).IsEmpty()) {
          GenerateAFException("Please specify a case file for this additional data set.", "ValidateInputFiles()",*edtCaseFileName, INPUT_TABS);
       }
       if (!File_Exists(gvCaseFiles.at(i).c_str())) {
         GenerateAFException("Case file could not be opened for this additional data set.", "ValidateInputFiles()",*edtCaseFileName, INPUT_TABS);
       }

       //validate the control file - Bernoulli model only
       if (gAnalysisSettings.GetModelControlType() == BERNOULLI) {
          if (gvControlFiles.at(i).IsEmpty()) {
             GenerateAFException("For the Bernoulli model, please specify a control file for this additional data set.","ValidateInputFiles()", *edtControlFileName, INPUT_TABS);
          }
          if (!File_Exists(gvControlFiles.at(i).c_str())) {
             GenerateAFException("Control file could not be opened for this additional data set.","ValidateInputFiles()", *edtControlFileName, INPUT_TABS);
          }
       }

       //validate the population file -  Poisson model only
       if (gAnalysisSettings.GetModelControlType() == POISSON) {
          if (gvPopFiles.at(i).IsEmpty()) {
             GenerateAFException("For the Poisson model, please specify a population file for this additional data set.","ValidateInputFiles()", *edtPopFileName, INPUT_TABS);
          }
          if (!File_Exists(gvPopFiles.at(i).c_str())) {
             GenerateAFException("Population file could not be opened for this additional data set.","ValidateInputFiles()", *edtPopFileName, INPUT_TABS);
          }
       }
    }  //for loop
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateInputFiles()", "TfrmAdvancedParameters");
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
    if (rdgSpatialOptions->Enabled && edtReportClustersSmallerThan->Enabled) {
      if (!edtReportClustersSmallerThan->Text.Length() || atof(edtReportClustersSmallerThan->Text.c_str()) == 0)
        GenerateAFException("Please specify a maximum cluster size for reported clusters\n"
                            "greater than 0 and less than or equal to the maximum spatial cluster size of %g.",
                            "ValidateReportedSpatialClusterSize()", *edtReportClustersSmallerThan, OUTPUT_TABS,
                            atof(edtMaxSpatialClusterSize->Text.c_str()));


      if (atof(edtReportClustersSmallerThan->Text.c_str()) > GetMaxSpatialClusterSizeFromControl())
        GenerateAFException("The maximum cluster size for reported clusters can not be greater than the maximum spatial cluster size.\n",
                            "ValidateReportedSpatialClusterSize()", *edtReportClustersSmallerThan, OUTPUT_TABS);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateReportedSpatialClusterSize()","TfrmAdvancedParameters");
    throw;
  }
}

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
    if (rdgSpatialOptions->Enabled) {
      switch (GetMaxSpatialClusterSizeControlType()) {
        case PERCENTOFPOPULATIONTYPE :
          if (!edtMaxSpatialClusterSize->Text.Length() || atof(edtMaxSpatialClusterSize->Text.c_str()) == 0) {
             GenerateAFException("Please specify a maximum spatial cluster size greater than zero.","ValidateSpatialClusterSize()",*edtMaxSpatialClusterSize, ANALYSIS_TABS);
          }
          if (atof(edtMaxSpatialClusterSize->Text.c_str()) > 50.0) {
             GenerateAFException("Please specify a maximum spatial cluster size no greater than 50.",
                                  "ValidateSpatialClusterSize()", *edtMaxSpatialClusterSize, ANALYSIS_TABS);
          }
          break;
        case DISTANCETYPE :
          if (!edtMaxSpatialRadius->Text.Length() || atof(edtMaxSpatialRadius->Text.c_str()) == 0) {
             GenerateAFException("Please specify a maximum spatial cluster size greater than zero.","ValidateSpatialClusterSize()", *edtMaxSpatialRadius, ANALYSIS_TABS);
          }
          break;
        case PERCENTOFPOPULATIONFILETYPE :
          if (!edtMaxSpatialPercentFile->Text.Length() || atof(edtMaxSpatialPercentFile->Text.c_str()) == 0) {
             GenerateAFException("Please specify a maximum spatial cluster size greater than zero.","ValidateSpatialClusterSize()", *edtMaxSpatialPercentFile, ANALYSIS_TABS);
          }
          if (atof(edtMaxSpatialPercentFile->Text.c_str()) > 50.0) {
             GenerateAFException("Please specify a maximum spatial cluster size no greater than 50.",
                                  "ValidateSpatialClusterSize()", *edtMaxSpatialPercentFile, ANALYSIS_TABS);
          }
          if (edtMaxCirclePopulationFilename->Text.IsEmpty() || !File_Exists(edtMaxCirclePopulationFilename->Text.c_str())) {
             GenerateAFException("Max circle size file could not be opened.","ValidateSpatialClusterSize()", *edtMaxCirclePopulationFilename, ANALYSIS_TABS);
          }
          break;
        default :
           ZdString sErrorMessage;
           sErrorMessage << "Unknown maximum spatial clutser size type:" << GetMaxSpatialClusterSizeControlType() << ".";
           GenerateAFException(sErrorMessage.GetCString(), "ValidateSpatialClusterSize()", *rdgSpatialOptions, ANALYSIS_TABS);
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateSpatialClusterSize()","TfrmAdvancedParameters");
    throw;
  }
}

//---------------------------------------------------------------------------
void TfrmAdvancedParameters::ValidateTemporalClusterSize() {
  float         dValue;
  ZdString      sErrorMessage;
  ZdDate        StartDate, EndDate, EndDatePlusOne,StartPlusIntervalDate;
  ZdDateFilter  DateFilter("%4y/%02m/%02d");
  char          FilterBuffer[11], Buffer[10];
  unsigned long ulMaxClusterDays, ulIntervalLengthInDays;

  try {
     //check whether we are specifiying temporal information
     if (rdgTemporalOptions->Enabled) {
        switch (GetMaxTemporalClusterSizeControlType()) {
           case PERCENTAGETYPE :
              if (!edtMaxTemporalClusterSize->Text.Length() || atof(edtMaxTemporalClusterSize->Text.c_str()) == 0) {
                GenerateAFException("Please specify a maximum temporal cluster size.","ValidateTemporalClusterSize()", *edtMaxTemporalClusterSize, ANALYSIS_TABS);
              }
              //check maximum temporal cluster size(as percentage pf population) is less
              //than maximum for given probabilty model
              dValue = atof(edtMaxTemporalClusterSize->Text.c_str());
              if (!(dValue > 0.0 && dValue <= (gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90))) {
                 sErrorMessage << "For the " << gAnalysisSettings.gParameters.GetProbabiltyModelTypeAsString(gAnalysisSettings.GetModelControlType());
                 sErrorMessage << " model, the maximum temporal cluster size, as a percent of study period, is ";
                 sErrorMessage << (gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90) << " percent.";
                 GenerateAFException(sErrorMessage.GetCString(), "ValidateTemporalClusterSize()", *edtMaxTemporalClusterSize, ANALYSIS_TABS);
              }
              break;
           case TIMETYPE :
              if (!edtMaxTemporalClusterSizeUnits->Text.Length() || atof(edtMaxTemporalClusterSizeUnits->Text.c_str()) == 0) {
                GenerateAFException("Please specify a maximum temporal cluster size.",
                                    "ValidateTemoralClusterSize()", *edtMaxTemporalClusterSizeUnits, ANALYSIS_TABS);
              }
              //check that maximum temporal cluster size(in time units) is less than
              //maximum for probabilty model. Determine the number of days the maximum
              //temporal cluster can be. Compare that against start date plus interval
              //length units.
              gAnalysisSettings.GetStudyPeriodStartDate(StartDate);
              gAnalysisSettings.GetStudyPeriodEndDate(EndDate);
              //to make start and end day inclusive - add 1 to EndDate date
              EndDatePlusOne = EndDate;
              EndDatePlusOne.AddDays(1);
              ulMaxClusterDays = EndDatePlusOne.GetJulianDayFromCalendarStart() - StartDate.GetJulianDayFromCalendarStart();
              ulMaxClusterDays = (gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION ? ulMaxClusterDays * 0.5 : ulMaxClusterDays * 0.9);
              StartPlusIntervalDate = StartDate;
              //add time interval length as units to modified start date
              switch (gAnalysisSettings.GetTimeAggregationControlType()) {
              case (YEAR):
                 StartPlusIntervalDate.AddYears(static_cast<unsigned short>(atoi((edtMaxTemporalClusterSizeUnits)->Text.c_str())));
                 strcpy(Buffer,"year(s)");
                 break;
              case (MONTH) :
                 StartPlusIntervalDate.AddMonths(static_cast<unsigned short>(atoi((edtMaxTemporalClusterSizeUnits)->Text.c_str())));
                 //to make start and end day inclusive - add one day to interval
                 StartPlusIntervalDate.AddDays(1);
                 strcpy(Buffer,"month(s)");
                 break;
              case (DAY) :
                 //to make start and end day inclusive - add interval length minus 1
                 StartPlusIntervalDate.AddDays(static_cast<unsigned short>(atoi((edtMaxTemporalClusterSizeUnits)->Text.c_str())));
                 strcpy(Buffer,"day(s)");
                 break;
              default  :
                 sErrorMessage << "Unknown interval unit " << gAnalysisSettings.GetTimeAggregationControlType() << ".";
                 GenerateAFException(sErrorMessage, "ValidateTemporalClusterSize()", *edtMaxTemporalClusterSizeUnits, ANALYSIS_TABS);
                 sErrorMessage = 0;
              };
              ulIntervalLengthInDays = StartPlusIntervalDate.GetJulianDayFromCalendarStart() - StartDate.GetJulianDayFromCalendarStart();
              if (ulIntervalLengthInDays > ulMaxClusterDays) {
                DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), StartDate.GetRawDate());
                sErrorMessage << "For the study period starting on " << FilterBuffer << " and ending on ";
                DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), EndDate.GetRawDate());
                sErrorMessage << FilterBuffer << ",\na maximum temporal cluster size of " << edtMaxTemporalClusterSizeUnits->Text.c_str();
                sErrorMessage << " " << Buffer << " is greater than " << (gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90);
                sErrorMessage << " percent of study period.";
                GenerateAFException(sErrorMessage.GetCString(), "ValidateTemporalClusterSize()", *edtMaxTemporalClusterSizeUnits, ANALYSIS_TABS);
              }
              break;
           default :
             ZdException::GenerateNotification("Unknown temporal percentage type: %d.",
                                            "ValidateTemporalClusterSize()", GetMaxTemporalClusterSizeControlType());
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateTemporalClusterSize()","TfrmAdvancedParameters");
    throw;
  }
}

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
void __fastcall TfrmAdvancedParameters::btnShowAllClick(TObject *Sender)
{
   int i;

   for (i=0; i < PageControl->PageCount; i++)
      PageControl->Pages[i]->TabVisible=true;
}

/**  Construct. This is an alternate constructor for when the varArgs list for sMessage
     has already been prepared. Primarily, this will be used by derived classes.        */
AdvancedFeaturesException::AdvancedFeaturesException(va_list varArgs, const char *sMessage,
                                                     const char *sSourceModule, Level iLevel, TWinControl& FocusControl, int iTabCategory)
          : ZdException(varArgs, sMessage, sSourceModule, iLevel), gFocusControl(FocusControl), giTabCategory(iTabCategory) {
   SetData(varArgs, sMessage, sSourceModule, iLevel);
}

/** Destructor. */
AdvancedFeaturesException::~AdvancedFeaturesException() {}

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


