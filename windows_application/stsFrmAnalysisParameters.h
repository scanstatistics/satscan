//---------------------------------------------------------------------------
#ifndef stsFrmAnalysisParametersH
#define stsFrmAnalysisParametersH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <io.h>
#include <Menus.hpp>
#include <Buttons.hpp>
#include "JulianDates.h"
#include "Parameters.h"
#include "stsBaseAnalysisChildForm.h"
//---------------------------------------------------------------------------

class TfrmAdvancedParameters;

class TfrmAnalysis : public stsBaseAnalysisChildForm  {
  friend class TfrmAdvancedParameters;

  __published:  // IDE-managed Components
        TButton *btnCaseBrowse;
        TButton *btnControlBrowse;
        TButton *btnCoordBrowse;
        TButton *btnGridBrowse;
        TButton *btnPopBrowse;
        TCheckBox *chkCensusAreasReportedClustersAscii;
        TCheckBox *chkCensusAreasReportedClustersDBase;
        TCheckBox *chkClustersInColumnFormatAscii;
        TCheckBox *chkClustersInColumnFormatDBase;
        TCheckBox *chkIncludePurSpacClust;
        TCheckBox *chkInclPurTempClust;
        TCheckBox *chkRelativeRiskEstimatesAreaAscii;
        TCheckBox *chkRelativeRiskEstimatesAreaDBase;
        TCheckBox *chkSimulatedLogLikelihoodRatiosAscii;
        TCheckBox *chkSimulatedLogLikelihoodRatiosDBase;
   TComboBox *cmbCriteriaSecClusters;
        TEdit *edtCaseFileName;
        TEdit *edtControlFileName;
        TEdit *edtCoordinateFileName;
        TEdit *edtGridFileName;
        TEdit *edtMaxSpatialClusterSize;
        TEdit *edtMaxTemporalClusterSize;
        TEdit *edtMontCarloReps;
        TEdit *edtPopFileName;
        TEdit *edtProspectiveStartDateDay;
        TEdit *edtProspectiveStartDateMonth;
        TEdit *edtProspectiveStartDateYear;
        TGroupBox *GroupBox1;
        TGroupBox *GroupBox2;
        TGroupBox *rdgSpatialOptions;
        TGroupBox *rdgTemporalOptions;
   TGroupBox *gbxProspectiveSurveillance;
   TGroupBox *gbxAdditionalOutputFiles;
   TGroupBox *gbxCriteriaSecClusters;
        TLabel *ASCII;
        TLabel *lblCensusAreasReportedClusters;
        TLabel *lblClustersInColumnFormat;
        TLabel *lbldBaseOutput;
        TLabel *lblRelativeRiskEstimatesArea;
        TLabel *lblSimulatedLogLikelihoodRatios;
        TLabel *Label1;
        TLabel *Label2;
        TLabel *Label3;
        TLabel *Label4;
        TLabel *Label5;
        TLabel *Label6;
        TLabel *Label7;
        TLabel *Label16;
        TLabel *lblMaxSpatialClusterSize;
        TLabel *lblMaxTemporalClusterSize;
        TLabel *lblProspectiveStartYear;
        TLabel *lblProspectiveStartMonth;
        TLabel *lblProspectiveStartDay;
        TOpenDialog *OpenDialog1;
        TPageControl *PageControl1;
        TRadioButton *rdoPercentageTemproal;
        TRadioButton *rdoSpatialDistance;
        TRadioButton *rdoSpatialPercentage;
        TRadioButton *rdoTimeTemproal;
   TRadioGroup *rgpCoordinates;
   TRadioGroup *rgpPrecisionTimes;
        TSaveDialog *SaveDialog;
        TTabSheet *tbAnalysis;
        TTabSheet *tbInputFiles;
        TTabSheet *tbOutputFiles;
        TTabSheet *tbScanningWindow;
        TTabSheet *tbTimeParameter;
        TPanel *pnlTop;
        TLabel *Label29;
        TEdit *edtResultFile;
        TButton *btnResultFileBrowse;
        TLabel *lblProspectiveStartDate;
        TCheckBox *chkAdjustForEarlierAnalyses;
   TGroupBox *rgpTypeProbability;
        TRadioButton *rdoPoissonModel;
        TRadioButton *rdoBernoulliModel;
        TRadioButton *rdoSpaceTimePermutationModel;
        TGroupBox *rgpTypeAnalysis;
        TStaticText *stRetrospectiveAnalyses;
        TStaticText *stProspectiveAnalyses;
        TRadioButton *rdoRetrospectivePurelySpatial;
        TRadioButton *rdoRetrospectivePurelyTemporal;
        TRadioButton *rdoRetrospectiveSpaceTime;
        TRadioButton *rdoProspectivePurelyTemporal;
        TRadioButton *rdoProspectiveSpaceTime;
   TGroupBox *rgpScanAreas;
        TRadioButton *rdoHighRates;
        TRadioButton *rdoLowRates;
        TRadioButton *rdoHighLowRates;
        TLabel *lblPercentOfPopulation;
        TRadioButton *rdoSpatialPopulationFile;
        TLabel *lblMaxRadius;
        TEdit *edtMaxSpatialRadius;
        TEdit *edtMaxSpatialPercentFile;
        TLabel *lblPercentageOfPopFile;
        TEdit *edtMaxCirclePopulationFilename;
        TButton *btnBrowseMaxCirclePopFile;
        TLabel *lblPercentageOfStudyPeriod;
        TEdit *edtMaxTemporalClusterSizeUnits;
        TLabel *lblMaxTemporalTimeUnits;
   TGroupBox *rgpTimeIntervalUnits;
   TLabel *lblTimeIntervalUnits;
   TLabel *lblTimeIntervalLength;
   TRadioButton *rdoUnitYear;
   TRadioButton *rdoUnitMonths;
   TRadioButton *rdoUnitDay;
   TEdit *edtTimeIntervalLength;
   TGroupBox *gbxStudyPeriod;
   TLabel *Label8;
   TLabel *Label9;
   TLabel *Label10;
   TLabel *Label12;
   TLabel *Label14;
   TEdit *edtStudyPeriodStartDateYear;
   TEdit *edtStudyPeriodEndDateYear;
   TEdit *edtStudyPeriodStartDateMonth;
   TEdit *edtStudyPeriodEndDateMonth;
   TEdit *edtStudyPeriodStartDateDay;
   TEdit *edtStudyPeriodEndDateDay;
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall rgpPrecisionTimesClick(TObject *Sender);
        void __fastcall btnCaseBrowseClick(TObject *Sender);
        void __fastcall btnControlBrowseClick(TObject *Sender);
        void __fastcall btnPopBrowseClick(TObject *Sender);
        void __fastcall btnCoordBrowseClick(TObject *Sender);
        void __fastcall btnGridBrowseClick(TObject *Sender);
        void __fastcall btnResultFileBrowseClick(TObject *Sender);
        void __fastcall rdoAnalysisTypeClick(TObject *Sender);
        void __fastcall rdoProbabilityModelClick(TObject *Sender);
        void __fastcall rgpCoordinatesClick(TObject *Sender);
        void __fastcall rdoUnitYearClick(TObject *Sender);
        void __fastcall rdoUnitMonthsClick(TObject *Sender);
        void __fastcall rdoUnitDayClick(TObject *Sender);
        void __fastcall edtMaxTemporalClusterSizeExit(TObject *Sender);
        void __fastcall edtMaxSpatialClusterSizeExit(TObject *Sender);
        void __fastcall edtMontCarloRepsExit(TObject *Sender);
        void __fastcall NaturalNumberKeyPress(TObject *Sender, char &Key);
        void __fastcall PositiveFloatKeyPress(TObject *Sender, char &Key);
        void __fastcall FloatKeyPress(TObject *Sender, char &Key);
        void __fastcall edtStudyPeriodStartDateExit(TObject *Sender);
        void __fastcall edtStudyPeriodEndDateExit(TObject *Sender);
        void __fastcall edtProspectiveStartDateExit(TObject *Sender);
        void __fastcall edtTimeIntervalLengthExit(TObject *Sender);
        void __fastcall edtCaseFileNameChange(TObject *Sender);
        void __fastcall edtControlFileNameChange(TObject *Sender);
        void __fastcall edtPopFileNameChange(TObject *Sender);
        void __fastcall edtCoordinateFileNameChange(TObject *Sender);
        void __fastcall edtGridFileNameChange(TObject *Sender);
        void __fastcall edtResultFileChange(TObject *Sender);
        void __fastcall rdoMaximumSpatialTypeClick(TObject *Sender);
        void __fastcall FormActivate(TObject *Sender);
        void __fastcall chkAdjustForEarlierAnalysesClick(TObject *Sender);
        void __fastcall edtMaxSpatialClusterSizeChange(TObject *Sender);
        void __fastcall btnBrowseMaxCirclePopFileClick(TObject *Sender);
        void __fastcall edtMaxCirclePopulationFilenameChange(
          TObject *Sender);
        void __fastcall edtMaxSpatialPercentFileExit(TObject *Sender);
        void __fastcall edtMaxSpatialPercentFileChange(TObject *Sender);
        void __fastcall edtMaxSpatialRadiusChange(TObject *Sender);
        void __fastcall edtMaxSpatialRadiusExit(TObject *Sender);
        void __fastcall rdoTemproalMaxClusterClick(TObject *Sender);
        void __fastcall edtMaxTemporalClusterSizeUnitsExit(
          TObject *Sender);

  private:	
    PrintNull                   gNullPrint;
    CParameters                 gParameters;
    TfrmAdvancedParameters    * gpfrmAdvancedParameters;  

    void                        Check_IntervalLength();
    void                        CheckAnalysisParams();
    void                        CheckDate(const char * sDateTitle, TEdit& Year, TEdit& Month, TEdit& Day,  TTabSheet& Container);
    void                        CheckOutputParams();
    void                        CheckProspDateRange();
    void                        CheckReplicas();
    void                        CheckScanningWindowParams();
    void                        CheckStudyPeriodDatesRange();
    void                        CheckTimeParams();
    void                        DefaultHiddenParameters();
    void                        EnableAdditionalOutFilesOptionsGroup(bool bRelativeRisks);
    void                        EnableAnalysisControlForModelType();
    void                        EnableDatesByTimeIntervalUnits();
    void                        EnableModelControlForAnalysisType();
    void                        EnableProspectiveStartDate(bool bEnable);
    void                        EnableProspectiveSurveillanceGroup(bool bEnable);
    void                        EnableSettingsForAnalysisModelCombination();
    void                        EnableSpatialOptionsGroup(bool bEnable, bool bEnableIncludePurelyTemporal, bool bEnablePercentage);
    void                        EnableStudyPeriodDates(bool bYear, bool bMonth, bool bDay);
    void                        EnableTemporalOptionsGroup(bool bEnable, bool bEnableIncludePurelySpatial, bool bEnableRanges);
    void                        EnableTimeIntervalUnitsGroup(bool bEnable);
    AnalysisType                GetAnalysisControlType() const;
    AreaRateType                GetAreaScanRateControlType() const;
    SpatialSizeType             GetMaxSpatialClusterSizeControlType() const;
    float                       GetMaxSpatialClusterSizeFromControl() const;
    TemporalSizeType            GetMaxTemporalClusterSizeControlType() const;
    float                       GetMaxTemporalClusterSizeFromControl() const;
    ProbabiltyModelType         GetModelControlType() const;
    DatePrecisionType           GetPrecisionOfTimesControlType() const;
    ZdDate                    & GetStudyPeriodEndDate(ZdDate & Date);
    ZdDate                    & GetStudyPeriodStartDate(ZdDate & Date);
    DatePrecisionType           GetTimeIntervalControlType() const;
    void                        Init();
    bool                        IsValidReplicationRequest(int iReplications);
    void                        OnAnalysisTypeClick();
    void                        OnPrecisionTimesClick();
    void                        OnProbabilityModelClick();
    void                        ParseDate(const char * szDate, TEdit *pYear, TEdit *pMonth, TEdit *pDay);
    void                        SaveParameterSettings();
    void                        SetAnalysisControl(AnalysisType eAnalysisType);
    void                        SetAreaScanRateControl(AreaRateType eAreaRateType);
    void                        SetMaxSpatialClusterSizeControl(float fMaxSize);
    void                        SetMaxSpatialClusterSizeTypeControl(SpatialSizeType eSpatialSizeType);
    void                        SetMaxTemporalClusterSizeControl(float fMaxSize);
    void                        SetMaxTemporalClusterSizeTypeControl(TemporalSizeType eTemporalSizeType);
    void                        SetModelControl(ProbabiltyModelType eProbabiltyModelType);
    void                        SetReportingSmallerClustersText();
    void                        SetSpatialDistanceCaption();
    void                        Setup(const char * sParameterFileName);
    void                        SetupInterface();
    void                        ValidateInputFiles();
    void                        ValidateSpatialClusterSize();
    void                        ValidateTemoralClusterSize();
    bool                        VerifyUnitAndLength();

protected:
    virtual void                EnableActions(bool bEnable);

public:		// User declarations
            __fastcall TfrmAnalysis(TComponent* Owner, TActionList* theList, char *sParamFileName = 0);
    virtual __fastcall ~TfrmAnalysis();

    const char                * GetFileName();
    CParameters               * GetSession();
    void                        LaunchImporter();
    void                        SaveAs();
    void                        SetAdjustmentsByRelativeRisksFile(const char * sAdjustmentsByRelativeRisksFileName);
    void                        SetCaseFile(const char * sCaseFileName);
    void                        SetControlFile(const char * sControlFileName);
    void                        SetCoordinateFile(const char * sCoordinateFileName);
    void                        SetCoordinateType(CoordinatesType eCoordinatesType);
    void                        SetMaximumCirclePopulationFile(const char * sMaximumCirclePopulationFileName);
    void                        SetPopulationFile(const char * sPopulationFileName);
    void                        SetPrecisionOfTimesControl(DatePrecisionType eDatePrecisionType);
    void                        SetSpecialGridFile(const char * sSpecialGridFileName);
    void                        ShowAdvancedFeaturesDialog();
    static void                 ValidateDate(TEdit& YearControl, TEdit& MonthControl, TEdit& DayControl);
    bool                        ValidateParams();
    void                        WriteSession(const char * sParameterFilename=0);
};
//---------------------------------------------------------------------------
#endif
