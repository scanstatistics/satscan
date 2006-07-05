//******************************************************************************
#ifndef stsFrmAdvancedParametersH
#define stsFrmAdvancedParametersH
//******************************************************************************
#include "stsFrmAnalysisParameters.h"
#include <Buttons.hpp>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>

const int INPUT_TABS     = 1;
const int ANALYSIS_TABS  = 2;
const int OUTPUT_TABS    = 3;

class TfrmAdvancedParameters : public TForm {
__published:	// IDE-managed Components
   TPanel *pnlButtons;
   TButton *btnOk;
   TOpenDialog *OpenDialog;
   TPageControl *PageControl;
   TTabSheet *tsAdjustments;
   TTabSheet *tsTemporal;
   TGroupBox *grpFlexibleTemporalWindowDefinition;
   TCheckBox *chkRestrictTemporalRange;
   TStaticText *stStartRangeTo;
   TEdit *edtStartRangeStartYear;
   TEdit *edtStartRangeStartMonth;
   TEdit *edtStartRangeStartDay;
   TEdit *edtStartRangeEndYear;
   TEdit *edtStartRangeEndMonth;
   TEdit *edtStartRangeEndDay;
   TStaticText *stStartWindowRange;
   TStaticText *stEndWindowRange;
   TEdit *edtEndRangeStartYear;
   TEdit *edtEndRangeStartMonth;
   TEdit *edtEndRangeStartDay;
   TStaticText *stEndRangeTo;
   TEdit *edtEndRangeEndYear;
   TEdit *edtEndRangeEndMonth;
   TEdit *edtEndRangeEndDay;
   TRadioGroup *rdgTemporalTrendAdj;
   TEdit *edtLogLinear;
   TStaticText *lblLogLinear;
   TGroupBox *grpAdjustments;
   TLabel *lblAdjustmentsByRelativeRisksFile;
   TEdit *edtAdjustmentsByRelativeRisksFile;
   TButton *btnBrowseAdjustmentsFile;
   TCheckBox *chkAdjustForKnownRelativeRisks;
   TTabSheet *tsClustersReported;
   TTabSheet *tbSpatial;
   TTabSheet *tsInference;
   TGroupBox *grpAnalysis;
   TCheckBox *chkTerminateEarly;
   TButton *btnShowAll;
   TRadioGroup *rdgCriteriaSecClusters;
   TGroupBox *rdgTemporalOptions;
   TLabel *lblPercentageOfStudyPeriod;
   TLabel *lblMaxTemporalTimeUnits;
   TEdit *edtMaxTemporalClusterSize;
   TRadioButton *rdoPercentageTemporal;
   TRadioButton *rdoTimeTemporal;
   TEdit *edtMaxTemporalClusterSizeUnits;
   TGroupBox *rdgSpatialOptions;
   TLabel *lblPercentOfPopulation;
   TLabel *lblMaxRadius;
   TLabel *lblPercentageOfPopFile;
   TEdit *edtMaxSpatialClusterSize;
   TEdit *edtMaxSpatialRadius;
   TEdit *edtMaxSpatialPercentFile;
   TEdit *edtMaxCirclePopulationFilename;
   TButton *btnBrowseMaxCirclePopFile;
   TGroupBox *gbxProspectiveSurveillance;
   TLabel *lblProspectiveStartYear;
   TLabel *lblProspectiveStartMonth;
   TLabel *lblProspectiveStartDay;
   TEdit *edtProspectiveStartDateYear;
   TEdit *edtProspectiveStartDateMonth;
   TEdit *edtProspectiveStartDateDay;
   TCheckBox *chkAdjustForEarlierAnalyses;
   TButton *btnSetDefaults;
   TTabSheet *tsDataSets;
   TGroupBox *grpDataSets;
   TLabel *Label3;
   TLabel *Label1;
   TLabel *Label2;
   TLabel *Label4;
   TLabel *Label5;
   TBitBtn *btnPopImport;
   TBitBtn *btnControlImport;
   TBitBtn *btnCaseImport;
   TListBox *lstInputDataSets;
   TButton *btnNewDataSet;
   TButton *btnRemoveDataSet;
   TEdit *edtCaseFileName;
   TEdit *edtControlFileName;
   TEdit *edtPopFileName;
   TButton *btnPopBrowse;
   TButton *btnControlBrowse;
   TButton *btnCaseBrowse;
   TBitBtn *btnImportMaxCirclePopFile;
   TBitBtn *btnImportAdjustmentsFile;
   TRadioGroup *rdgSpatialAdjustments;
   TStaticText *lblMultipleDataSetPurpose;
   TRadioButton *rdoMultivariate;
   TRadioButton *rdoAdjustmentByDataSets;
   TCheckBox *chkInclPureTempClust;
   TCheckBox *chkIncludePureSpacClust;
   TGroupBox *grpReportCriticalValues;
   TCheckBox *chkReportCriticalValues;
   TGroupBox *grpWindowShape;
   TRadioButton *rdoCircular;
   TRadioButton *rdoElliptic;
   TStaticText *stNonCompactnessPenalty;
   TComboBox *cmbNonCompactnessPenalty;
   TGroupBox *rdgReportedSpatialOptions;
   TLabel *lblReportedPercentOfPopulation;
   TLabel *lblMaxReportedRadius;
   TLabel *lblReportedPercentageOfPopFile;
   TEdit *edtMaxReportedSpatialClusterSize;
   TEdit *edtMaxReportedSpatialRadius;
   TEdit *edtMaxReportedSpatialPercentFile;
   TCheckBox *chkRestrictReportedClusters;
   TGroupBox *grpSequentialScan;
   TEdit *edtNumSequentialScans;
   TCheckBox *chkPerformSequentialScan;
   TLabel *lblMaxSequentialScans;
   TEdit *edtSequentialScanCutoff;
   TLabel *lblSeqentialCutoff;
   TTabSheet *tsDataChecking;
   TRadioGroup *rdgStudyPeriodCheck;
   TGroupBox *grpGeographicalCoordinatesCheck;
   TRadioButton *rdoStrictCoordinates;
   TRadioButton *rdoRelaxedCoordinates;
   TStaticText *stStrictCoodinates;
   TStaticText *stRelaxedCoodinates;
   TCheckBox *chkSpatialPopulationFile;
   TCheckBox *chkSpatialDistance;
   TLabel *lblDistancePrefix;
   TCheckBox *chkReportedSpatialDistance;
   TCheckBox *chkReportedSpatialPopulationFile;
   TLabel *lblReportedMaxDistance;
   TTabSheet *tabNeighborsFile;
   TGroupBox *grpNeighborsFile;
   TLabel *lblNeighborsFile;
   TEdit *edtNeighborsFile;
   TButton *btnBrowseForNeighborsFile;
   TCheckBox *chkSpecifiyNeighborsFile;

   void __fastcall btnNewClick(TObject *Sender) ;
   void __fastcall btnBrowseAdjustmentsFileClick(TObject *Sender);
   void __fastcall btnImportAdjustmentsFileClick(TObject *Sender);
   void __fastcall btnBrowseMaxCirclePopFileClick(TObject *Sender);
   void __fastcall btnImportMaxCirclePopFileClick(TObject *Sender);
   void __fastcall btnCaseBrowseClick(TObject *Sender) ;
   void __fastcall btnCaseImportClick(TObject *Sender) ;
   void __fastcall btnControlBrowseClick(TObject *Sender) ;
   void __fastcall btnControlImportClick(TObject *Sender) ;
   void __fastcall btnPopBrowseClick(TObject *Sender) ;
   void __fastcall btnPopImportClick(TObject *Sender) ;
   void __fastcall btnShowAllClick(TObject *Sender);
   void __fastcall chkAdjustForEarlierAnalysesClick(TObject *Sender);
   void __fastcall chkAdjustForKnownRelativeRisksClick(TObject *Sender);
   void __fastcall chkRestrictTemporalRangeClick(TObject *Sender);
   void __fastcall chkRestrictReportedClustersClick(TObject *Sender);
   void __fastcall edtMaxCirclePopulationFilenameChange(TObject *Sender);
   void __fastcall edtMaxSpatialClusterSizeExit(TObject *Sender);
   void __fastcall edtMaxSpatialPercentFileExit(TObject *Sender);
   void __fastcall edtMaxSpatialRadiusExit(TObject *Sender);
   void __fastcall edtMaxTemporalClusterSizeExit(TObject *Sender) ;
   void __fastcall edtMaxTemporalClusterSizeUnitsExit(TObject *Sender) ;
   void __fastcall edtNumSequentialScansExit(TObject *Sender);
   void __fastcall edtStartRangeStartDateExit(TObject *Sender);
   void __fastcall edtEndRangeStartDateExit(TObject *Sender);
   void __fastcall edtStartRangeEndDateExit(TObject *Sender);
   void __fastcall edtEndRangeEndDateExit(TObject *Sender);
   void __fastcall edtLogLinearExit(TObject *Sender);
   void __fastcall edtAdjustmentsByRelativeRisksFileChange(TObject *Sender);
   void __fastcall FloatKeyPress(TObject *Sender, char &Key);
   void __fastcall FormKeyPress(TObject *Sender, char &Key);
   void __fastcall FormShow(TObject *Sender);
   void __fastcall NaturalNumberKeyPress(TObject *Sender, char &Key);
   void __fastcall PositiveFloatKeyPress(TObject *Sender, char &Key);
   void __fastcall rdgTemporalTrendAdjClick(TObject *Sender);
   void __fastcall rdoMaxTemporalClusterSizelick(TObject *Sender);
   void __fastcall rdoMaxSpatialTypeClick(TObject *Sender);
   void __fastcall btnSetDefaultsClick(TObject *Sender);
   void __fastcall OnControlExit(TObject *Sender);
   void __fastcall lstInputDataSetsClick(TObject *Sender);
   void __fastcall btnRemoveDataSetClick(TObject *Sender);
   void __fastcall edtCaseFileNameChange(TObject *Sender);
   void __fastcall edtControlFileNameChange(TObject *Sender);
   void __fastcall edtPopFileNameChange(TObject *Sender);
   void __fastcall rdgSpatialAdjustmentsClick(TObject *Sender);
   void __fastcall OnWindowShapeClick(TObject *Sender);
   void __fastcall OnNonCompactnessPenaltyChange(TObject *Sender);
   void __fastcall edtMaxReportedSpatialClusterSizeExit(TObject *Sender);
   void __fastcall edtMaxReportedSpatialPercentFileExit(TObject *Sender);
   void __fastcall edtMaxReportedSpatialRadiusExit(TObject *Sender);
   void __fastcall chkPerformSequentialScanClick(TObject *Sender);
   void __fastcall edtSequentialScanCutoffExit(TObject *Sender);
   void __fastcall rdgStudyPeriodCheckClick(TObject *Sender);
   void __fastcall stStrictCoodinatesClick(TObject *Sender);
   void __fastcall stRelaxedCoodinatesClick(TObject *Sender);
   void __fastcall rdoStrictCoordinatesClick(TObject *Sender);
   void __fastcall rdoRelaxedCoordinatesClick(TObject *Sender);
   void __fastcall chkSpecifiyNeighborsFileClick(TObject *Sender);
   void __fastcall edtNeighborsFileChange(TObject *Sender);
   void __fastcall btnBrowseForNeighborsFileClick(TObject *Sender);

 private:
   const TfrmAnalysis     & gAnalysisSettings;
   TWinControl            * gpFocusControl;
   int                      giCategory;              /** category - input,analysis,output - of parameters to show */
   std::vector<AnsiString>  gvCaseFiles;
   std::vector<AnsiString>  gvControlFiles;
   std::vector<AnsiString>  gvPopFiles;
   int                      giDataSetNum;             /** number of additional input datasets added
                                                         does not go down with removals */
   static const int         MAXIMUM_ADDITIONAL_SETS;         /* maximum number of additional input sets */

   void                     DoControlExit();
   void                     EnableAdjustmentsGroup(bool bEnable);
   void                     EnableAdjustmentForSpatialOptionsGroup(bool bEnable, bool bEnableStratified);
   void                     EnableAdjustmentForTimeTrendOptionsGroup(bool bEnable, bool bTimeStratified, bool bLogYearPercentage, bool bCalculatedLog);
   void                     EnableCoordinatesCheckGroup(bool bEnable);
   void                     EnableDataSetList(bool bEnable);
   void                     EnableDataSetPurposeControls(bool bEnable);
   void                     EnableNeighborsFileGroup(bool bEnable);
   void                     EnableNewButton();
   void                     EnableRemoveButton();
   void                     EnableInputFileEdits(bool bEnable);
   void                     EnableOutputOptions(bool bEnable);
   void                     EnableProspectiveStartDate();
   void                     EnableProspectiveSurveillanceGroup(bool bEnable);
   void                     EnableReportedSpatialOptionsGroup(bool bEnable);
   void                     EnableSequentialScanOptionsGroup(bool bEnable);
   void                     EnableSpatialOptionsGroup(bool bEnable, bool bEnableIncludePurelyTemporal);
   void                     EnableTemporalOptionsGroup(bool bEnable, bool bEnableIncludePurelySpatial, bool bEnableRanges);
   void                     EnableTemporalRanges(bool bEnable, bool bEnableRanges);
   void                     EnableWindowShapeGroup(bool bEnable);
   SpatialAdjustmentType    GetAdjustmentSpatialControlType() const;
   TimeTrendAdjustmentType  GetAdjustmentTimeTrendControlType() const;
   void                     Init();
   void                     LaunchImporter(const char * sFileName, InputFileType eFileType) ;
   void                     ParseDate(const std::string& sDate, TEdit& Year, TEdit& Month, TEdit& Day, bool bStartRange);
   void                     SetDefaultsForAnalysisTabs();
   void                     SetDefaultsForInputTab();
   void                     SetDefaultsForOutputTab();
   void                     Setup();
   void                     ValidateAdjustmentSettings();
   void                     ValidateInputFiles();
   void                     ValidateOutputSettings();
   void                     ValidateNeighborsFileSettings();
   void                     ValidateProspDateRange();
   void                     ValidateReportedSpatialClusterSize();
   void                     ValidateScanningWindowRanges();
   void                     ValidateSpatialClusterSize();
   void                     ValidateTemporalClusterSize();
   void                     ValidateTemporalWindowSettings();

public:
   __fastcall TfrmAdvancedParameters(const TfrmAnalysis& AnalysisSettings);

   void                     EnableSettingsForAnalysisModelCombination();
   void                     EnableDatesByTimePrecisionUnits();
   CoordinatesDataCheckingType GetCoordinatesDataCheckingTypeFromControl() const;
   bool                     GetDefaultsSetForAnalysisOptions();
   bool                     GetDefaultsSetForInputOptions();
   bool                     GetDefaultsSetForOutputOptions();
   double                   GetMaxReportedSpatialClusterSizeFromControl(SpatialSizeType eSpatialType) const;
   double                   GetMaxSpatialClusterSizeFromControl(SpatialSizeType eSpatialType) const;
   TemporalSizeType         GetMaxTemporalClusterSizeControlType() const;
   double                   GetMaxTemporalClusterSizeFromControl() const;
   unsigned int             GetNumAdditionalDataSets() const {return lstInputDataSets->Items->Count;}
   StudyPeriodDataCheckingType GetStudyPeriodDataCheckingFromControl() const;
   void                     SaveParameterSettings();
   void                     SetAdjustmentsByRelativeRisksFile(const char * sAdjustmentsForRelativeRisksFileName);
   void                     SetCoordinatesDataCheckingControl(CoordinatesDataCheckingType eCoordinatesDataCheckingType);
   void                     SetMaxReportedSpatialClusterSizeControl(SpatialSizeType eSpatialSizeType, double dMaxSize);
   void                     SetMaxSpatialClusterSizeControl(SpatialSizeType eSpatialSizeType, double dMaxSize);
   void                     SetMaxTemporalClusterSizeControl(double dMaxSize);
   void                     SetMaxTemporalClusterSizeTypeControl(TemporalSizeType eTemporalSizeType);
   void                     SetSpatialDistanceCaption();
   void                     SetStudyPeriodDataCheckingControl(StudyPeriodDataCheckingType eStudyPeriodDataCheckingType);
   void                     SetTemporalTrendAdjustmentControl(TimeTrendAdjustmentType eTimeTrendAdjustmentType);
   void                     ShowDialog(TWinControl * pFocusControl=0, int iCategory=-1);
   void                     Validate();
};

class AdvancedFeaturesException : public virtual ZdException {
  private:
    TWinControl       & gFocusControl;
    int                 giTabCategory; // type of tab showing - input, analysis, output

  public:
    AdvancedFeaturesException(va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel, TWinControl& FocusControl, int iTabCategory);
    virtual ~AdvancedFeaturesException();

    TWinControl       & GetFocusControl() const {return gFocusControl;}
    int                 GetTabCategory()  const {return giTabCategory;}
};

void GenerateAFException(const char * sMessage, const char * sSourceModule, TWinControl & FocusControl, int iTabCategory, ...);
//******************************************************************************
#endif

