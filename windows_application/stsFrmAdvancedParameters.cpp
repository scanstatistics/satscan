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
// Resets advanced settings to default values
void __fastcall TfrmAdvancedParameters::btnSetDefaultsClick(
      TObject *Sender)
{
   if (gbAnalysisShow)
      SetDefaultsForAnalysisTabs();
   else
      SetDefaultsForOutputTab();
}
//---------------------------------------------------------------------------
/** event triggered when 'adjustment for ealier analyses' checkbox if clicked */
void __fastcall TfrmAdvancedParameters::chkAdjustForEarlierAnalysesClick(TObject *Sender) {
  EnableProspectiveStartDate(chkAdjustForEarlierAnalyses->Checked);
  gAnalysisSettings.EnableSettingsForAnalysisModelCombination();
}
//---------------------------------------------------------------------------
/** event triggered when adjust for known relative risks is clicked */
void __fastcall TfrmAdvancedParameters::chkAdjustForKnownRelativeRisksClick(TObject *Sender) {
  lblAdjustmentsByRelativeRisksFile->Enabled = gbEnableAdjustmentsByRR && chkAdjustForKnownRelativeRisks->Checked;
  edtAdjustmentsByRelativeRisksFile->Enabled = gbEnableAdjustmentsByRR && chkAdjustForKnownRelativeRisks->Checked;
  edtAdjustmentsByRelativeRisksFile->Color = edtAdjustmentsByRelativeRisksFile->Enabled ? clWindow : clInactiveBorder;
  btnBrowseAdjustmentsFile->Enabled = gbEnableAdjustmentsByRR && chkAdjustForKnownRelativeRisks->Checked;
}

/** event triggered when user selects restrict reported clusters check box */
void __fastcall TfrmAdvancedParameters::chkRestrictReportedClustersClick(TObject *Sender) {
  edtReportClustersSmallerThan->Enabled = rdgSpatialOptions->Enabled && chkRestrictReportedClusters->Checked;
  edtReportClustersSmallerThan->Color = rdgSpatialOptions->Enabled && chkRestrictReportedClusters->Checked ? clWindow : clInactiveBorder;
}

/** event triggered when user selects restrict range check box */
void __fastcall TfrmAdvancedParameters::chkRestrictTemporalRangeClick(TObject *Sender) {
  EnableSpatialOutputOptions(rdgSpatialOptions->Enabled);
  RefreshTemporalRangesEnables();
}

/** event triggered when loginear percentage control exited */
void __fastcall TfrmAdvancedParameters::edtLogLinearExit(TObject *Sender) {
  if (edtLogLinear->Text.IsEmpty() || atof(edtLogLinear->Text.c_str()) <= -100)
    edtLogLinear->Text = 0;
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
//---------------------------------------------------------------------------
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
}
//---------------------------------------------------------------------------
/** event triggered when maximum temporal cluster size edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxTemporalClusterSizeExit(TObject *Sender) {
  if (edtMaxTemporalClusterSize->Text.IsEmpty() || atof(edtMaxTemporalClusterSize->Text.c_str()) == 0)
    edtMaxTemporalClusterSize->Text = 50;
}

//---------------------------------------------------------------------------
/** event triggered when maximum temporal cluster size edit control is exited. */
void __fastcall TfrmAdvancedParameters::edtMaxTemporalClusterSizeUnitsExit(TObject *Sender) {
  if (edtMaxTemporalClusterSizeUnits->Text.IsEmpty() || atof(edtMaxTemporalClusterSizeUnits->Text.c_str()) == 0)
    edtMaxTemporalClusterSizeUnits->Text = 1;
}


/** event triggered when 'Report only clusters smaller than ...' edit control is exited */
void __fastcall TfrmAdvancedParameters::edtReportClustersSmallerThanExit(TObject *Sender) {
  if (!edtReportClustersSmallerThan->Text.Length() || atof(edtReportClustersSmallerThan->Text.c_str()) == 0)
    edtReportClustersSmallerThan->Text = GetMaxSpatialClusterSizeFromControl();
}
//---------------------------------------------------------------------------
/** event triggered when key pressed for control that can contain positive real numbers */
void __fastcall TfrmAdvancedParameters::edtReportClustersSmallerThanKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789.\b",Key))
    Key = 0;
}
//---------------------------------------------------------------------------
/** event triggered when year control, of prospective start date, is exited. */
void __fastcall TfrmAdvancedParameters::edtProspectiveStartDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtProspectiveStartDateYear, *edtProspectiveStartDateMonth, *edtProspectiveStartDateDay);
}
//---------------------------------------------------------------------------
/** event triggered when start window end ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtStartRangeEndDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtStartRangeEndYear, *edtStartRangeEndMonth, *edtStartRangeEndDay);
}
//---------------------------------------------------------------------------
/** event triggered when start window start ranges year, month or day control is exited */
void __fastcall TfrmAdvancedParameters::edtStartRangeStartDateExit(TObject *Sender) {
  TfrmAnalysis::ValidateDate(*edtStartRangeStartYear, *edtStartRangeStartMonth, *edtStartRangeStartDay);
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
/** enables adjustment options controls */
void TfrmAdvancedParameters::EnableOutputOptions(bool bEnable) {
   rdgCriteriaSecClusters->Enabled = bEnable;
}
//---------------------------------------------------------------------------
/** enabled prospective start date controls */
void TfrmAdvancedParameters::EnableProspectiveStartDate(bool bEnable) {
  //trump enabling based upon earlier analyses adjustment and precision of time controls
  bEnable = bEnable && chkAdjustForEarlierAnalyses->Checked && gAnalysisSettings.GetPrecisionOfTimesControlType() != NONE;
  edtProspectiveStartDateYear->Enabled = bEnable;
  lblProspectiveStartDate->Enabled = bEnable;
  lblProspectiveStartYear->Enabled = bEnable;
  lblProspectiveStartMonth->Enabled = bEnable;
  lblProspectiveStartDay->Enabled = bEnable;
  edtProspectiveStartDateYear->Color =  edtProspectiveStartDateYear->Enabled ? clWindow : clInactiveBorder;
  edtProspectiveStartDateMonth->Enabled = bEnable && (gAnalysisSettings.rdoUnitDay->Checked || gAnalysisSettings.rdoUnitMonths->Checked);
  edtProspectiveStartDateMonth->Color = edtProspectiveStartDateMonth->Enabled ? clWindow : clInactiveBorder;
  edtProspectiveStartDateDay->Enabled = bEnable &&  gAnalysisSettings.rdoUnitDay->Checked;
  edtProspectiveStartDateDay->Color =  edtProspectiveStartDateDay->Enabled ? clWindow : clInactiveBorder;
}
//---------------------------------------------------------------------------
/** enables or disables the prospective start date group control */
void TfrmAdvancedParameters::EnableProspectiveSurveillanceGroup(bool bEnable) {
   gbxProspectiveSurveillance->Enabled = bEnable;
   chkAdjustForEarlierAnalyses->Enabled = bEnable;
   EnableProspectiveStartDate(bEnable);
}
//---------------------------------------------------------------------------
/** enables or disables the spatial options group control */
void TfrmAdvancedParameters::EnableSpatialOptionsGroup(bool bEnable, bool bEnableIncludePurelyTemporal, bool bEnablePercentage) {
   rdgSpatialOptions->Enabled = bEnable;
   lblMaxSpatialClusterSize->Enabled = bEnable;

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

/** enables temporal options controls */
void TfrmAdvancedParameters::EnableTemporalRanges(bool bEnable, bool bEnableRanges) {
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
/** enables or disables the temporal options group control */
void TfrmAdvancedParameters::EnableTemporalOptionsGroup(bool bEnable, bool bEnableIncludePurelySpatial, bool bEnableRanges) {
  rdgTemporalOptions->Enabled = bEnable;
  lblMaxTemporalClusterSize->Enabled = bEnable;

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
//---------------------------------------------------------------------------
//** Checks to determine if only default values are set in the dialog
//** Returns true if only default values are set
//** Returns false if user specified a value other than a default
bool TfrmAdvancedParameters::GetDefaultsSetForAnalysisOptions() {
   bool bReturn = true;

   // Inference tab
   bReturn &= (chkAdjustForEarlierAnalyses->Checked == false);
   bReturn &= (chkTerminateEarly->Checked == false);
   bReturn &= (edtProspectiveStartDateYear->Text.ToInt() == 1900);
   bReturn &= (edtProspectiveStartDateMonth->Text.ToInt() == 12);
   bReturn &= (edtProspectiveStartDateDay->Text.ToInt() == 31);

   // Spatial Window tab
   bReturn &= (GetMaxSpatialClusterSizeControlType()==PERCENTOFPOPULATIONTYPE);
   bReturn &= (edtMaxSpatialClusterSize->Text.ToInt() == 50);
   bReturn &= (edtMaxSpatialPercentFile->Text.ToInt() == 50);
   bReturn &= (edtMaxSpatialRadius->Text.ToInt() == 1);
   bReturn &= (edtMaxCirclePopulationFilename->Text == "");
   bReturn &= (chkInclPureTempClust->Checked == false);

   // Temporal tab
   bReturn &= (GetMaxTemporalClusterSizeControlType()==PERCENTAGETYPE);
   bReturn &= (edtMaxTemporalClusterSize->Text.ToDouble() == 50);
   bReturn &= (edtMaxTemporalClusterSizeUnits->Text.ToInt() == 1);
   bReturn &= (chkIncludePureSpacClust->Checked == false);

   bReturn &= (edtStartRangeStartYear->Text.ToInt() == 1900);
   bReturn &= (edtStartRangeStartMonth->Text.ToInt() == 1);
   bReturn &= (edtStartRangeStartDay->Text.ToInt() == 1);
   bReturn &= (edtStartRangeEndYear->Text.ToInt() == 1900);
   bReturn &= (edtStartRangeEndMonth->Text.ToInt() == 1);
   bReturn &= (edtStartRangeEndDay->Text.ToInt() == 1);
   bReturn &= (edtEndRangeStartYear->Text.ToInt() == 1900);
   bReturn &= (edtEndRangeStartMonth->Text.ToInt() == 12);
   bReturn &= (edtEndRangeStartDay->Text.ToInt() == 31);
   bReturn &= (edtEndRangeEndYear->Text.ToInt() == 1900);
   bReturn &= (edtEndRangeEndMonth->Text.ToInt() == 12);
   bReturn &= (edtEndRangeEndDay->Text.ToInt() == 31);
   bReturn &= (chkRestrictTemporalRange->Checked == false);

   // Risk tab
   bReturn &= (chkAdjustForKnownRelativeRisks->Checked == false);
   bReturn &= (edtAdjustmentsByRelativeRisksFile->Text == "");
   bReturn &= (rdgTemporalTrendAdj->ItemIndex == 0);
   bReturn &= (edtLogLinear->Text.ToInt() == 0);

   return bReturn;
}
//---------------------------------------------------------------------------
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
  gbAnalysisShow = true;
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
/** event triggered when maximum temporal cluster size type edit control clicked */
void __fastcall TfrmAdvancedParameters::rdoMaxTemporalClusterSizelick(TObject *Sender) {
  //cause enabling to be refreshed based upon clicked radio button
  EnableTemporalOptionsGroup(rdgTemporalOptions->Enabled, chkIncludePureSpacClust->Enabled,
                             chkRestrictTemporalRange->Enabled);
}

//---------------------------------------------------------------------------
/** event triggered when maximum spatial type selected */
void __fastcall TfrmAdvancedParameters::rdoMaxSpatialTypeClick(TObject *Sender) {
  //cause enabling to be refreshed based upon clicked radio button
  EnableSpatialOptionsGroup(rdgSpatialOptions->Enabled,
                            chkInclPureTempClust->Enabled,
                            rdoSpatialPercentage->Enabled);
  SetReportingSmallerClustersText();
}

/** */
void TfrmAdvancedParameters::RefreshTemporalRangesEnables() {
  AnalysisType  eType = gAnalysisSettings.GetAnalysisControlType();
  EnableTemporalRanges(rdgTemporalOptions->Enabled, eType == PURELYTEMPORAL || eType == SPACETIME);
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
   edtProspectiveStartDateYear->Text = "1900";
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
   edtMaxTemporalClusterSize->Text = "50.0";
   edtMaxTemporalClusterSizeUnits->Text = "1";
   chkIncludePureSpacClust->Checked = false;

   ParseDate("1900/01/01", *edtStartRangeStartYear, *edtStartRangeStartMonth, *edtStartRangeStartDay, true);
   ParseDate("1900/01/01", *edtStartRangeEndYear, *edtStartRangeEndMonth, *edtStartRangeEndDay, false);
   ParseDate("1900/12/31", *edtEndRangeStartYear, *edtEndRangeStartMonth, *edtEndRangeStartDay, true);
   ParseDate("1900/12/31", *edtEndRangeEndYear, *edtEndRangeEndMonth, *edtEndRangeEndDay, false);
   chkRestrictTemporalRange->Checked = false;

   // Risk tab
   chkAdjustForKnownRelativeRisks->Checked = false;
   edtAdjustmentsByRelativeRisksFile->Text = "";
   SetTemporalTrendAdjustmentControl(NOTADJUSTED);
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
//---------------------------------------------------------------------------
/** */
void TfrmAdvancedParameters::SetRangeDateEnables(bool bYear, bool bMonth, bool bDay) {
  gbEnableRangeYears = bYear;
  gbEnableRangeMonths = bMonth;
  gbEnableRangeDays = bDay;
  RefreshTemporalRangesEnables();
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
      sTemp.printf("percent of population at risk\n        (<= %s%%)", edtMaxSpatialClusterSize->Text.c_str());
      break;
    case PERCENTOFPOPULATIONFILETYPE :
      sTemp.printf("percent of population at risk\n        (<= %s%%)", edtMaxSpatialPercentFile->Text.c_str());
      break;
    case DISTANCETYPE:
      if (gAnalysisSettings.rgpCoordinates->ItemIndex == CARTESIAN)
        sTemp.printf("cartesian units in radius\n        (<= %s)", edtMaxSpatialRadius->Text.c_str());
      else
        sTemp.printf("kilometers in radius\n        (<= %s)", edtMaxSpatialRadius->Text.c_str());
  }

  SetReportingClustersText(sTemp);
}
//---------------------------------------------------------------------------
/** Sets caption of spatial distance radio button based upon coordinates group setting. */
void TfrmAdvancedParameters::SetSpatialDistanceCaption() {
  try {
    switch (gAnalysisSettings.rgpCoordinates->ItemIndex) {
      case 0  : lblMaxRadius->Caption = "cartesian units radius";
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
    }
    catch (ZdException &x) {
      x.AddCallpath("Setup()","TfrmAdvancedParameters");
      throw;
    }
}

/** sets control to focus when form shows then shows form modal */
void TfrmAdvancedParameters::ShowDialog(TWinControl * pFocusControl, bool bAnalysis) {
  bool          bFound=false;
  int           i;

  // PAG - not the best coding here but am trying to show/hide only
  // certain pages/tabs of page control
  gbAnalysisShow = bAnalysis;
  if (bAnalysis) {  // show Analysis pages
     Caption = "Advanced Analysis Features";
     for (i=0; i < PageControl->PageCount-1; i++)
        PageControl->Pages[i]->TabVisible=true;
     PageControl->Pages[PageControl->PageCount-1]->TabVisible=false;
  }
  else {           // show Output pages
     Caption = "Advanced Output Features";
     for (i=0; i < PageControl->PageCount-1; i++)
        PageControl->Pages[i]->TabVisible=false;
     PageControl->Pages[PageControl->PageCount-1]->TabVisible=true;
  }

  for (i=0; i < PageControl->PageCount && !bFound; ++i) {
     if (PageControl->Pages[i]->ContainsControl(pFocusControl)) {
       PageControl->ActivePage = PageControl->Pages[i];
       gpFocusControl = pFocusControl;
       bFound=true;
     }
  }

  if (!bFound) {
    gpFocusControl=0;
    //PageControl->ActivePage = PageControl->Pages[0];
    // PAG - find first visible page
    PageControl->ActivePage = PageControl->FindNextPage(0, true, true);
  }
  //reporting clusters text dependent on maximum spatial cluster size
  //-- ensure that it has text
  if (!edtMaxSpatialClusterSize->Text.Length())
    edtMaxSpatialClusterSize->Text = 50;

  ShowModal();
}

/** validates all the settings in this dialog */
void TfrmAdvancedParameters::Validate() {
   ValidateSpatialClusterSize();
   ValidateAdjustmentSettings();
   ValidateTemporalWindowSettings();
   if (chkAdjustForEarlierAnalyses->Enabled && chkAdjustForEarlierAnalyses->Checked)
      ValidateProspDateRange();        // inference tab settings
}

/** validates adjustment settings - throws exception */
void TfrmAdvancedParameters::ValidateAdjustmentSettings() {
  try {
    if (chkAdjustForKnownRelativeRisks->Enabled && chkAdjustForKnownRelativeRisks->Checked) {
      if (edtAdjustmentsByRelativeRisksFile->Text.IsEmpty())
        GenerateAFException("Please specify an adjustments file.",
                            "ValidateAdjustmentSettings()", *edtAdjustmentsByRelativeRisksFile);
      if (!File_Exists(edtAdjustmentsByRelativeRisksFile->Text.c_str()))
        GenerateAFException("Adjustments file could not be opened.",
                            "ValidateAdjustmentSettings()", *edtAdjustmentsByRelativeRisksFile);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateAdjustmentSettings()","TfrmAdvancedParameters");
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
       GenerateAFException("Please specify a year.","ValidateProspDateRange()", *edtProspectiveStartDateYear);

    if (edtProspectiveStartDateMonth->Text.IsEmpty())
       GenerateAFException("Please specify a month.","ValidateProspDateRange()", *edtProspectiveStartDateMonth);

    if (edtProspectiveStartDateDay->Text.IsEmpty())
       GenerateAFException("Please specify a day.","ValidateProspDateRange()", *edtProspectiveStartDateDay);

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
                           "ValidateProspDateRange()", *edtProspectiveStartDateYear);
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
                            "ValidateReportedSpatialClusterSize()", *edtReportClustersSmallerThan,
                            atof(edtMaxSpatialClusterSize->Text.c_str()));


      if (atof(edtReportClustersSmallerThan->Text.c_str()) > GetMaxSpatialClusterSizeFromControl())
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
                            "ValidateScanningWindowRanges()", *edtStartRangeStartYear);
      if (StartRangeEndDate < StudyPeriodStartDate || StartRangeEndDate > StudyPeriodEndDate)
        GenerateAFException("The scanning window start range does not occur within study period.",
                            "ValidateScanningWindowRanges()", *edtStartRangeEndYear);
      if (StartRangeStartDate > StartRangeEndDate)
        GenerateAFException("The scanning window start range dates conflict.",
                            "ValidateScanningWindowRanges()", *edtStartRangeStartYear);
      if (EndRangeStartDate < StudyPeriodStartDate || EndRangeStartDate > StudyPeriodEndDate)
        GenerateAFException("The scanning window end range does not occur within study period.",
                            "ValidateScanningWindowRanges()", *edtEndRangeStartYear);
      if (EndRangeEndDate < StudyPeriodStartDate || EndRangeEndDate > StudyPeriodEndDate)
        GenerateAFException("The scanning window end range does not occur within study period.",
                            "ValidateScanningWindowRanges()", *edtEndRangeEndYear);
      if (EndRangeStartDate > EndRangeEndDate)
        GenerateAFException("The scanning window end range dates conflict.",
                            "ValidateScanningWindowRanges()", *edtEndRangeStartYear);
      if (StartRangeStartDate >= EndRangeEndDate)
        GenerateAFException("The scanning window start range does not occur before end range.",
                            "ValidateScanningWindowRanges()", *edtStartRangeStartYear);
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
             GenerateAFException("Please specify a maximum spatial cluster size greater than zero.","ValidateSpatialClusterSize()",*edtMaxSpatialClusterSize);
          }
          if (atof(edtMaxSpatialClusterSize->Text.c_str()) > 50.0) {
             GenerateAFException("Please specify a maximum spatial cluster size no greater than 50.",
                                  "ValidateSpatialClusterSize()", *edtMaxSpatialClusterSize);
          }
          break;
        case DISTANCETYPE :
          if (!edtMaxSpatialRadius->Text.Length() || atof(edtMaxSpatialRadius->Text.c_str()) == 0) {
             GenerateAFException("Please specify a maximum spatial cluster size greater than zero.","ValidateSpatialClusterSize()", *edtMaxSpatialRadius);
          }
          break;
        case PERCENTOFPOPULATIONFILETYPE :
          if (!edtMaxSpatialPercentFile->Text.Length() || atof(edtMaxSpatialPercentFile->Text.c_str()) == 0) {
             GenerateAFException("Please specify a maximum spatial cluster size greater than zero.","ValidateSpatialClusterSize()", *edtMaxSpatialPercentFile);
          }
          if (atof(edtMaxSpatialPercentFile->Text.c_str()) > 50.0) {
             GenerateAFException("Please specify a maximum spatial cluster size no greater than 50.",
                                  "ValidateSpatialClusterSize()", *edtMaxSpatialPercentFile);
          }
          if (edtMaxCirclePopulationFilename->Text.IsEmpty() || !File_Exists(edtMaxCirclePopulationFilename->Text.c_str())) {
             GenerateAFException("Max circle size file could not be opened.","ValidateSpatialClusterSize()", *edtMaxCirclePopulationFilename);
          }
          break;
        default :
           ZdString sErrorMessage;
           sErrorMessage << "Unknown maximum spatial clutser size type:" << GetMaxSpatialClusterSizeControlType() << ".";
           GenerateAFException(sErrorMessage.GetCString(), "ValidateSpatialClusterSize()", *rdgSpatialOptions);
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
                GenerateAFException("Please specify a maximum temporal cluster size.","ValidateTemporalClusterSize()", *edtMaxTemporalClusterSize);
              }
              //check maximum temporal cluster size(as percentage pf population) is less
              //than maximum for given probabilty model
              dValue = atof(edtMaxTemporalClusterSize->Text.c_str());
              if (!(dValue > 0.0 && dValue <= (gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90))) {
                 sErrorMessage << "For the " << gAnalysisSettings.gParameters.GetProbabiltyModelTypeAsString(gAnalysisSettings.GetModelControlType());
                 sErrorMessage << " model, the maximum temporal cluster size, as a percent of study period, is ";
                 sErrorMessage << (gAnalysisSettings.GetModelControlType() == SPACETIMEPERMUTATION ? 50 : 90) << " percent.";
                 GenerateAFException(sErrorMessage.GetCString(), "ValidateTemporalClusterSize()", *edtMaxTemporalClusterSize);
              }
              break;
           case TIMETYPE :
              if (!edtMaxTemporalClusterSizeUnits->Text.Length() || atof(edtMaxTemporalClusterSizeUnits->Text.c_str()) == 0) {
                GenerateAFException("Please specify a maximum temporal cluster size.",
                                    "ValidateTemoralClusterSize()", *edtMaxTemporalClusterSizeUnits);
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
              switch (gAnalysisSettings.GetTimeIntervalControlType()) {
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
                 sErrorMessage << "Unknown interval unit " << gAnalysisSettings.GetTimeIntervalControlType() << ".";
                 GenerateAFException(sErrorMessage, "ValidateTemporalClusterSize()", *edtMaxTemporalClusterSizeUnits);
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
                GenerateAFException(sErrorMessage.GetCString(), "ValidateTemporalClusterSize()", *edtMaxTemporalClusterSizeUnits);
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
//---------------------------------------------------------------------------

