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
#include <graphics.hpp>
#include "JulianDates.h"
#include "Parameters.h"
#include "stsBaseAnalysisChildForm.h"
#include "UtilityFunctions.h"
//---------------------------------------------------------------------------

class TfrmAdvancedParameters;

class TfrmAnalysis : public stsBaseAnalysisChildForm  {
  friend class TfrmAdvancedParameters;

  __published:  // IDE-managed Components
   TButton *btnCoordBrowse;
   TButton *btnGridBrowse;
   TButton *btnPopBrowse;
   TCheckBox *chkCensusAreasReportedClustersAscii;
   TCheckBox *chkCensusAreasReportedClustersDBase;
   TCheckBox *chkClustersInColumnFormatAscii;
   TCheckBox *chkClustersInColumnFormatDBase;
   TCheckBox *chkRelativeRiskEstimatesAreaAscii;
   TCheckBox *chkRelativeRiskEstimatesAreaDBase;
   TCheckBox *chkSimulatedLogLikelihoodRatiosAscii;
   TCheckBox *chkSimulatedLogLikelihoodRatiosDBase;
   TEdit *edtCoordinateFileName;
   TEdit *edtGridFileName;
   TEdit *edtMontCarloReps;
   TEdit *edtPopFileName;
   TGroupBox *grpGeographical;
   TGroupBox *gbxAdditionalOutputFiles;
   TLabel *ASCII;
   TLabel *lblCensusAreasReportedClusters;
   TLabel *lblClustersInColumnFormat;
   TLabel *lbldBaseOutput;
   TLabel *lblRelativeRiskEstimatesArea;
   TLabel *lblSimulatedLogLikelihoodRatios;
   TLabel *Label4;
   TLabel *Label5;
   TLabel *Label6;
   TLabel *Label7;
   TLabel *Label16;
   TOpenDialog *OpenDialog1;
   TPageControl *PageControl1;
   TRadioGroup *rgpCoordinates;
   TSaveDialog *SaveDialog;
   TTabSheet *tbAnalysis;
   TTabSheet *tbInputFiles;
   TTabSheet *tbOutputFiles;
   TPanel *pnlTop;
   TLabel *Label29;
   TEdit *edtResultFile;
   TButton *btnResultFileBrowse;
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
        TGroupBox *rgpTimeAggregationUnits;
        TLabel *lblTimeAggregationUnits;
        TLabel *lblTimeAggregationLength;
        TRadioButton *rdoTimeAggregationYear;
        TRadioButton *rdoTimeAggregationMonths;
        TRadioButton *rdoTimeAggregationDay;
        TEdit *edtTimeAggregationLength;
   TLabel *Label11;
   TLabel *Label13;
   TGroupBox *grpCountData;
   TLabel *Label1;
   TLabel *Label2;
   TLabel *Label3;
   TRadioGroup *rgpPrecisionTimes;
   TEdit *edtCaseFileName;
   TEdit *edtControlFileName;
   TButton *btnCaseBrowse;
   TButton *btnControlBrowse;
   TBitBtn *btnAdvancedAnalysis;
   TBitBtn *btnAdvancedOutput;
   TBitBtn *btnAdvancedInput;
   TGroupBox *grpStudyPeriod;
   TLabel *Label8;
   TLabel *Label10;
   TEdit *edtStudyPeriodStartDateYear;
   TLabel *Label12;
   TEdit *edtStudyPeriodStartDateMonth;
   TLabel *Label14;
   TEdit *edtStudyPeriodStartDateDay;
   TLabel *Label9;
   TLabel *Label15;
   TEdit *edtStudyPeriodEndDateYear;
   TLabel *Label17;
   TEdit *edtStudyPeriodEndDateMonth;
   TLabel *Label18;
   TEdit *edtStudyPeriodEndDateDay;
   TBitBtn *btnCaseImport;
   TBitBtn *btnControlImport;
   TBitBtn *btnPopImport;
   TBitBtn *btnCoordImport;
   TBitBtn *btnGridImport;
   TStaticText *stUnitText;
   
   void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
   void __fastcall rgpPrecisionTimesClick(TObject *Sender);
   void __fastcall btnCaseBrowseClick(TObject *Sender);
   void __fastcall btnCaseImportClick(TObject *Sender);
   void __fastcall btnControlBrowseClick(TObject *Sender);
   void __fastcall btnControlImportClick(TObject *Sender);
   void __fastcall btnPopBrowseClick(TObject *Sender);
   void __fastcall btnPopImportClick(TObject *Sender);
   void __fastcall btnCoordBrowseClick(TObject *Sender);
   void __fastcall btnCoordImportClick(TObject *Sender);
   void __fastcall btnGridBrowseClick(TObject *Sender);
   void __fastcall btnGridImportClick(TObject *Sender);
   void __fastcall btnResultFileBrowseClick(TObject *Sender);
   void __fastcall rdoAnalysisTypeClick(TObject *Sender);
   void __fastcall rdoProbabilityModelClick(TObject *Sender);
   void __fastcall rgpCoordinatesClick(TObject *Sender);
   void __fastcall rdoTimeAggregationYearClick(TObject *Sender);
   void __fastcall rdoTimeAggregationMonthsClick(TObject *Sender);
   void __fastcall rdoTimeAggregationDayClick(TObject *Sender);
   void __fastcall edtMontCarloRepsExit(TObject *Sender);
   void __fastcall NaturalNumberKeyPress(TObject *Sender, char &Key);
   void __fastcall PositiveFloatKeyPress(TObject *Sender, char &Key);
   void __fastcall FloatKeyPress(TObject *Sender, char &Key);
   void __fastcall edtStudyPeriodStartDateExit(TObject *Sender);
   void __fastcall edtStudyPeriodEndDateExit(TObject *Sender);
   void __fastcall edtTimeAggregationLengthExit(TObject *Sender);
   void __fastcall edtCaseFileNameChange(TObject *Sender);
   void __fastcall edtControlFileNameChange(TObject *Sender);
   void __fastcall edtPopFileNameChange(TObject *Sender);
   void __fastcall edtCoordinateFileNameChange(TObject *Sender);
   void __fastcall edtGridFileNameChange(TObject *Sender);
   void __fastcall edtResultFileChange(TObject *Sender);
   void __fastcall FormActivate(TObject *Sender);
   void __fastcall btnAdvancedParametersClick(TObject *Sender);

  private:
    PrintNull                   gNullPrint;
    CParameters                 gParameters;
    TfrmAdvancedParameters    * gpfrmAdvancedParameters;

    double                      CalculateTimeAggregationUnitsInStudyPeriod() const;
    void                        CheckAnalysisParams();
    void                        CheckDate(const char * sDateTitle, TEdit& Year, TEdit& Month, TEdit& Day,  TTabSheet& Container);
    void                        CheckOutputParams();
    void                        CheckReplicas();
    void                        CheckScanningWindowParams();
    void                        CheckStudyPeriodDatesRange();
    void                        CheckTimeAggregationLength();
    void                        DefaultHiddenParameters();
    void                        EnableAdditionalOutFilesOptionsGroup(bool bRelativeRisks);
    void                        EnableAdvancedButtons();
    void                        EnableAnalysisControlForModelType();
    void                        EnableDatesByTimePrecisionUnits();
    void                        EnableModelControlForAnalysisType();
    void                        EnableSettingsForAnalysisModelCombination();
    void                        EnableStudyPeriodDates(bool bYear, bool bMonth, bool bDay);
    void                        EnableTimeAggregationGroup(bool bEnable);
    AnalysisType                GetAnalysisControlType() const;
    AreaRateType                GetAreaScanRateControlType() const;
    ProbabiltyModelType         GetModelControlType() const;
    DatePrecisionType           GetPrecisionOfTimesControlType() const;
    ZdDate                    & GetStudyPeriodEndDate(ZdDate& Date) const;
    ZdDate                    & GetStudyPeriodStartDate(ZdDate& Date) const;
    DatePrecisionType           GetTimeAggregationControlType() const;
    void                        Init();
    bool                        IsValidReplicationRequest(int iReplications);
    void                        OnAnalysisTypeClick();
    void                        OnPrecisionTimesClick();
    void                        OnProbabilityModelClick();
    void                        ParseDate(const char * szDate, TEdit *pYear, TEdit *pMonth, TEdit *pDay);
    void                        SaveParameterSettings();
    void                        SetAnalysisControl(AnalysisType eAnalysisType);
    void                        SetAreaScanRateControl(AreaRateType eAreaRateType);
    void                        SetModelControl(ProbabiltyModelType eProbabiltyModelType);
    void                        Setup(const char * sParameterFileName);
    void                        SetupInterface();
    void                        ValidateInputFiles();
    bool                        VerifyUnitAndLength();

protected:
    virtual void                EnableActions(bool bEnable);

public:		// User declarations
            __fastcall TfrmAnalysis(TComponent* Owner, TActionList* theList, char *sParamFileName = 0);
    virtual __fastcall ~TfrmAnalysis();

    const char                * GetFileName();
    CParameters               * GetSession();
    void                        LaunchImporter(const char * sFileName, InputFileType eFileType) ;
    void                        SaveAs();
    void                        SetAdjustmentsByRelativeRisksFile(const char * sAdjustmentsByRelativeRisksFileName);
    void                        SetCaseFile(const char * sCaseFileName);
    void                        SetControlFile(const char * sControlFileName);
    void                        SetCoordinateFile(const char * sCoordinateFileName);
    void                        SetCoordinateType(CoordinatesType eCoordinatesType);
    void                        SetDayEditText(TEdit& Day, bool bEnablingDay, int iDayText);
    void                        SetMaximumCirclePopulationFile(const char * sMaximumCirclePopulationFileName);
    void                        SetMonthEditText(TEdit& Month, bool bEnablingMonth, int iMonthText);
    void                        SetPopulationFile(const char * sPopulationFileName);
    void                        SetPrecisionOfTimesControl(DatePrecisionType eDatePrecisionType);
    void                        SetSpecialGridFile(const char * sSpecialGridFileName);
    void                        ShowAdvancedFeaturesDialog();
    static void                 StoreEditText(TEdit& Month, TEdit& Day);
    static void                 ValidateDate(TEdit& YearControl, TEdit& MonthControl, TEdit& DayControl);
    bool                        ValidateParams();
    void                        WriteSession(const char * sParameterFilename=0);
};
//---------------------------------------------------------------------------
#endif
