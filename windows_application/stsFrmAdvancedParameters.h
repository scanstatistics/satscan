//---------------------------------------------------------------------------
#ifndef stsFrmAdvancedParametersH
#define stsFrmAdvancedParametersH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include "stsFrmAnalysisParameters.h"
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include "Grids_ts.hpp"
#include "TSGrid.hpp"
#include <Buttons.hpp>
//---------------------------------------------------------------------------
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
   TCheckBox *chkRestrictReportedClusters;
   TEdit *edtReportClustersSmallerThan;
   TLabel *lblReportSmallerClusters;
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
   TRadioButton *rdoSpatialPercentage;
   TRadioButton *rdoSpatialDistance;
   TRadioButton *rdoSpatialPopulationFile;
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
   TTabSheet *tsDataStreams;
   TGroupBox *grpDataStreams;
   TLabel *Label3;
   TLabel *Label1;
   TLabel *Label2;
   TLabel *Label4;
   TLabel *Label5;
   TBitBtn *btnPopImport;
   TBitBtn *btnControlImport;
   TBitBtn *btnCaseImport;
   TListBox *lstInputStreams;
   TButton *btnNewStream;
   TButton *btnRemoveStream;
   TEdit *edtCaseFileName;
   TEdit *edtControlFileName;
   TEdit *edtPopFileName;
   TButton *btnPopBrowse;
   TButton *btnControlBrowse;
   TButton *btnCaseBrowse;
   TBitBtn *btnImportMaxCirclePopFile;
   TBitBtn *btnImportAdjustmentsFile;
   TRadioGroup *rdgSpatialAdjustments;
   TStaticText *lblMultipleStreamPurpose;
   TRadioButton *rdoMultivariate;
   TRadioButton *rdoAdjustmentByStreams;
   TCheckBox *chkInclPureTempClust;
   TCheckBox *chkIncludePureSpacClust;

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
   void __fastcall edtMaxSpatialClusterSizeChange(TObject *Sender);
   void __fastcall edtMaxSpatialClusterSizeExit(TObject *Sender);
   void __fastcall edtMaxSpatialPercentFileChange(TObject *Sender);
   void __fastcall edtMaxSpatialPercentFileExit(TObject *Sender);
   void __fastcall edtMaxSpatialRadiusChange(TObject *Sender);
   void __fastcall edtMaxSpatialRadiusExit(TObject *Sender);
   void __fastcall edtMaxTemporalClusterSizeExit(TObject *Sender) ;
   void __fastcall edtMaxTemporalClusterSizeUnitsExit(TObject *Sender) ;
   void __fastcall edtProspectiveStartDateExit(TObject *Sender);
   void __fastcall edtReportClustersSmallerThanExit(TObject *Sender);
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
   void __fastcall lstInputStreamsClick(TObject *Sender);
   void __fastcall btnRemoveStreamClick(TObject *Sender);
   void __fastcall edtCaseFileNameChange(TObject *Sender);
   void __fastcall edtControlFileNameChange(TObject *Sender);
   void __fastcall edtPopFileNameChange(TObject *Sender);

 private:
   const TfrmAnalysis     & gAnalysisSettings;
   TWinControl            * gpFocusControl;
   int                      giCategory;              /** category - input,analysis,output - of parameters to show */
   std::vector<AnsiString>  gvCaseFiles;
   std::vector<AnsiString>  gvControlFiles;
   std::vector<AnsiString>  gvPopFiles;
   int                      giStreamNum;             /** number of additional input streams added
                                                         does not go down with removals */
   static const int         MAXIMUM_STREAMS;         /* maximum number of additional input streams */

   void                     DoControlExit();
   void                     EnableDataStreamList(bool bEnable);
   void                     EnableDataStreamPurposeControls(bool bEnable);
   void                     EnableNewButton();
   void                     EnableRemoveButton();
   void                     EnableInputFileEdits(bool bEnable);
   TimeTrendAdjustmentType  GetAdjustmentTimeTrendControlType() const;
   void                     Init();
   void                     LaunchImporter(const char * sFileName, InputFileType eFileType) ;
   void                     ParseDate(const std::string& sDate, TEdit& Year, TEdit& Month, TEdit& Day, bool bStartRange);
   void                     SetDefaultsForAnalysisTabs();
   void                     SetDefaultsForInputTab();
   void                     SetDefaultsForOutputTab();
   void                     Setup();
   void                     ValidateAdjustmentSettings();
   void                     ValidateInputFilesAtInput();
   void                     ValidateInputFiles();
   void                     ValidateOutputSettings();
   void                     ValidateProspDateRange();
   void                     ValidateReportedSpatialClusterSize();
   void                     ValidateScanningWindowRanges();
   void                     ValidateSpatialClusterSize();
   void                     ValidateTemporalClusterSize();
   void                     ValidateTemporalWindowSettings();

public:
   __fastcall TfrmAdvancedParameters(const TfrmAnalysis& AnalysisSettings);

   void                  EnableAdjustmentsGroup(bool bEnable);
   void                  EnableAdjustmentForSpatialOptionsGroup(bool bEnable);
   void                  EnableAdjustmentForTimeTrendOptionsGroup(bool bEnable, bool bTimeStratified, bool bLogYearPercentage, bool bCalculatedLog);
   void                  EnableSettingsForAnalysisModelCombination();
   void                  EnableDatesByTimePrecisionUnits();
   void                  EnableOutputOptions(bool bEnable);
   void                  EnableProspectiveStartDate();
   void                  EnableProspectiveSurveillanceGroup(bool bEnable);
   void                  EnableSpatialOptionsGroup(bool bEnable, bool bEnableIncludePurelyTemporal, bool bEnablePercentage);
   void                  EnableSpatialOutputOptions(bool bEnable);
   void                  EnableTemporalOptionsGroup(bool bEnable, bool bEnableIncludePurelySpatial, bool bEnableRanges);
   void                  EnableTemporalRanges(bool bEnable, bool bEnableRanges);
   bool                  GetDefaultsSetForAnalysisOptions();
   bool                  GetDefaultsSetForInputOptions();
   bool                  GetDefaultsSetForOutputOptions();
   SpatialSizeType       GetMaxSpatialClusterSizeControlType() const;
   float                 GetMaxSpatialClusterSizeFromControl() const;
   TemporalSizeType      GetMaxTemporalClusterSizeControlType() const;
   float                 GetMaxTemporalClusterSizeFromControl() const;
   void                  SaveParameterSettings();
   void                  SetAdjustmentsByRelativeRisksFile(const char * sAdjustmentsForRelativeRisksFileName);
   void                  SetMaxSpatialClusterSizeControl(float fMaxSize);
   void                  SetMaxSpatialClusterSizeTypeControl(SpatialSizeType eSpatialSizeType);
   void                  SetMaxTemporalClusterSizeControl(float fMaxSize);
   void                  SetMaxTemporalClusterSizeTypeControl(TemporalSizeType eTemporalSizeType);
   void                  SetReportingClustersText(const ZdString& sText);
   void                  SetReportingSmallerClustersText();
   void                  SetSpatialDistanceCaption();
   void                  SetTemporalTrendAdjustmentControl(TimeTrendAdjustmentType eTimeTrendAdjustmentType);
   void                  ShowDialog(TWinControl * pFocusControl=0, int iCategory=-1);
   void                  Validate();
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

//---------------------------------------------------------------------------
#endif
