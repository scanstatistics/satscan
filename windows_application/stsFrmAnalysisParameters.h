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

//---------------------------------------------------------------------------

#include "stsBaseAnalysisChildForm.h"

class TfrmAnalysis : public stsBaseAnalysisChildForm  {
  __published:  // IDE-managed Components
        TButton *btnCaseBrowse;
        TButton *btnControlBrowse;
        TButton *btnCoordBrowse;
        TButton *btnGridBrowse;
        TButton *btnPopBrowse;
        TButton *btnResultFileBrowse;
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
        TComboBox *cboCriteriaSecClusters;
        TEdit *edtLogLinearPercentageRadioTag;
        TEdit *edtCaseFileName;
        TEdit *edtControlFileName;
        TEdit *edtCoordinateFileName;
        TEdit *edtEndDay;
        TEdit *edtEndMonth;
        TEdit *edtEndYear;
        TEdit *edtGridFileName;
        TEdit *edtLogPerYear;
        TEdit *edtMaxClusterSize;
        TEdit *edtMaxTemporalClusterSize;
        TEdit *edtMontCarloReps;
        TEdit *edtPopFileName;
        TEdit *edtProspDay;
        TEdit *edtProspMonth;
        TEdit *edtProspYear;
        TEdit *edtResultFile;
        TEdit *edtStartDay;
        TEdit *edtStartMonth;
        TEdit *edtStartYear;
        TEdit *edtUnitLength;
        TGroupBox *GroupBox1;
        TGroupBox *GroupBox2;
        TGroupBox *GroupBox3;
        TGroupBox *GroupBox4;
        TGroupBox *GroupBox5;
        TGroupBox *GroupBox6;
        TGroupBox *GroupBox8;
        TGroupBox *grpAdditionalOutputFiles;
        TGroupBox *grpCriteriaSecClusters;
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
        TLabel *Label8;
        TLabel *Label9;
        TLabel *Label10;
        TLabel *Label11;
        TLabel *Label12;
        TLabel *Label13;
        TLabel *Label14;
        TLabel *Label15;
        TLabel *Label16;
        TLabel *Label17;
        TLabel *Label18;
        TLabel *Label19;
        TLabel *Label20;
        TLabel *Label23;
        TLabel *Label24;
        TLabel *Label25;
        TLabel *Label26;
        TLabel *Label27;
        TLabel *Label28;
        TLabel *Label29;
        TMenuItem *mitClearSpecialGridEdit;
        TOpenDialog *OpenDialog1;
        TPageControl *PageControl1;
        TPopupMenu *SpecialGridPopupMenu;
        TRadioButton *rdoPercentageTemproal;
        TRadioButton *rdoSpatialDistance;
        TRadioButton *rdoSpatialPercentage;
        TRadioButton *rdoTimeTemproal;
        TRadioButton *rbUnitDay;
        TRadioButton *rbUnitMonths;
        TRadioButton *rbUnitYear;
        TRadioGroup *rgClustersToInclude;
        TRadioGroup *rgCoordinates;
        TRadioGroup *rgPrecisionTimes;
        TRadioGroup *rgProbability;
        TRadioGroup *rgScanAreas;
        TRadioGroup *rgTemporalTrendAdj;
        TRadioGroup *rgTypeAnalysis;
        TSaveDialog *SaveDialog;
        TTabSheet *tbAnalysis;
        TTabSheet *tbInputFiles;
        TTabSheet *tbOutputFiles;
        TTabSheet *tbScanningWindow;
        TTabSheet *tbTimeParameter;
        TLabel *Label21;
        TEdit *edtMaxCirclePopulationFilename;
        TButton *Button1;
        TPanel *pnlOutputFileBottom;
        TCheckBox *chkRestrictReportedClusters;
        TEdit *edtReportClustersSmallerThan;
        TLabel *lblReportSmallerClusters;
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall rgPrecisionTimesClick(TObject *Sender);
        void __fastcall btnCaseBrowseClick(TObject *Sender);
        void __fastcall btnControlBrowseClick(TObject *Sender);
        void __fastcall btnPopBrowseClick(TObject *Sender);
        void __fastcall btnCoordBrowseClick(TObject *Sender);
        void __fastcall btnGridBrowseClick(TObject *Sender);
        void __fastcall btnResultFileBrowseClick(TObject *Sender);
        void __fastcall rgTemporalTrendAdjClick(TObject *Sender);
        void __fastcall rgTypeAnalysisClick(TObject *Sender);
        void __fastcall rgScanAreasClick(TObject *Sender);
        void __fastcall rgProbabilityClick(TObject *Sender);
        void __fastcall rgCoordinatesClick(TObject *Sender);
        void __fastcall chkInclPurTempClustClick(TObject *Sender);
        void __fastcall chkIncludePurSpacClustClick(TObject *Sender);
        void __fastcall rgClustersToIncludeClick(TObject *Sender);
        void __fastcall rbUnitYearClick(TObject *Sender);
        void __fastcall rbUnitMonthsClick(TObject *Sender);
        void __fastcall rbUnitDayClick(TObject *Sender);
        void __fastcall chkRelativeRiskEstimatesAreaAsciiClick(TObject *Sender);
        void __fastcall chkSimulatedLogLikelihoodRatiosAsciiClick(TObject *Sender);
        void __fastcall edtMaxTemporalClusterSizeExit(TObject *Sender);
        void __fastcall edtMaxClusterSizeExit(TObject *Sender);
        void __fastcall edtMontCarloRepsExit(TObject *Sender);
        void __fastcall NaturalNumberKeyPress(TObject *Sender, char &Key);
        void __fastcall PositiveFloatKeyPress(TObject *Sender, char &Key);
        void __fastcall FloatKeyPress(TObject *Sender, char &Key);
        void __fastcall edtStartMonthExit(TObject *Sender);
        void __fastcall edtStartYearExit(TObject *Sender);
        void __fastcall edtEndMonthExit(TObject *Sender);
        void __fastcall edtEndYearExit(TObject *Sender);
        void __fastcall edtStartDayExit(TObject *Sender);
        void __fastcall edtEndDayExit(TObject *Sender);
        void __fastcall edtProspYearExit(TObject *Sender);
        void __fastcall edtProspMonthExit(TObject *Sender);
        void __fastcall edtProspDayExit(TObject *Sender);
        void __fastcall chkCensusAreasReportedClustersAsciiClick(TObject *Sender);
        void __fastcall chkClustersInColumnFormatAsciiClick(TObject *Sender);
        void __fastcall edtUnitLengthExit(TObject *Sender);
        void __fastcall edtLogPerYearExit(TObject *Sender);
        void __fastcall edtCaseFileNameChange(TObject *Sender);
        void __fastcall edtControlFileNameChange(TObject *Sender);
        void __fastcall edtPopFileNameChange(TObject *Sender);
        void __fastcall edtCoordinateFileNameChange(TObject *Sender);
        void __fastcall edtGridFileNameChange(TObject *Sender);
        void __fastcall edtResultFileChange(TObject *Sender);
        void __fastcall cboCriteriaSecClustersChange(TObject *Sender);
        void __fastcall rdoSpatialPercentageClick(TObject *Sender);
        void __fastcall rdoSpatialDistanceClick(TObject *Sender);
        void __fastcall mitClearSpecialGridEditClick(TObject *Sender);
        void __fastcall FormActivate(TObject *Sender);
        void __fastcall edtMaxCirclePopulationFilenameChange(TObject *Sender);
        void __fastcall BrowseMaxCirclePopulationFileClick(TObject *Sender);
        void __fastcall edtReportClustersSmallerThanExit(TObject *Sender);
        void __fastcall chkRestrictReportedClustersClick(TObject *Sender);

  private:	// User declarations
    PrintNull           gNullPrint;
    CParameters         gParameters;
    AnsiString          gsParamFileName;

    bool                Check_Days(int iYear, int iMonth, int iDay, const char *sDateName);
    bool                Check_IntervalLength(int iStartYear, int iStartMonth, int iStartDay,
                                             int iEndYear, int iEndMonth, int iEndDay,
                                             int iIntervalUnits, int iIntervalLength);
    bool                Check_Month(int iMonth, const char *sDateName);
    bool                Check_TimeTrendPercentage(double dValue);
    bool                Check_Year(int iYear, const char *sDateName);
    bool                CheckAnalysisParams();
    bool                CheckDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                       int iEndYear, int iEndMonth, int iEndDay,
                                       int iIntervalUnits, int iIntervalLength);
    bool                CheckOutputParams();
    bool                CheckProspDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                            int iEndYear, int iEndMonth, int iEndDay,
                                            int iProspYear, int iProspMonth, int iProspDay);
    bool                CheckReplicas(int iReplicas);
    bool                CheckScanningWindowParams();
    bool                CheckTimeParams();
    void                DefaultHiddenParameters();
    void                EnablePSTDate(bool bEnable);
    void                EnableSpatial(bool bEnable, bool bEnableCheckbox, bool bEnableSpatialPercentage);
    void                EnableStartAndEndYear(bool bEnable);
    void                EnableStartAndEndMonth(bool bEnable);
    void                EnableStartAndEndDay(bool bEnable);
    void                EnableTemporal(bool bEnable, bool bEnableCheckbox, bool bEnablePercentage);
    void                EnableTemporalTimeTrendAdjust(bool bEnableRadioGroup, bool bEnableNonParametric,
                                                      bool bEnableLogYearEditBox, bool bEnableTimeStratifiedRandomization);
    void                EnableTimeInterval(bool bEnable);
    ZdDate            & GetStudyPeriodEndDate(ZdDate & Date);
    ZdDate            & GetStudyPeriodStartDate(ZdDate & Date);
    void                Init();
    void                OnAnalysisTypeClick();
    void                OnPrecisionTimesClick();
    void                OnProbabilityModelClick();
    void                OnTemporalTrendClick();
    void                ParseDate(const char * szDate, TEdit *pYear, TEdit *pMonth, TEdit *pDay);
    void                SaveTextParameters();
    void                SetReportingSmallerClustersText();
    void                SetSpatialDistanceCaption();
    void                SetTemporalTrendAdjustmentControl(TimeTrendAdjustmentType eTimeTrendAdjustmentType);
    void                Setup(const char * sParameterFileName);
    void                SetupInterface();
    bool                ValidateInputFiles();
    bool                ValidateReportedSpatialClusterSize();
    bool                ValidateSpatialClusterSize();
    bool                ValidateTemoralClusterSize();
    bool                VerifyUnitAndLength();
protected:
    virtual void        EnableActions(bool bEnable);
public:		// User declarations
            __fastcall TfrmAnalysis(TComponent* Owner, TActionList* theList, char *sParamFileName = 0);
    virtual __fastcall ~TfrmAnalysis();

    void                LaunchImporter();
    char              * GetFileName();
    CParameters       * GetSession();
    void                SaveAs();
    void                SetCaseFile(const char * sCaseFileName);
    void                SetControlFile(const char * sControlFileName);
    void                SetCoordinateFile(const char * sCoordinateFileName);
    void                SetMaximumCirclePopulationFile(const char * sMaximumCirclePopulationFileName);
    void                SetPopulationFile(const char * sPopulationFileName);
    void                SetSpecialGridFile(const char * sSpecialGridFileName);
    bool                ValidateParams();
    void                WriteSession();
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmAnalysis *frmAnalysis;
//---------------------------------------------------------------------------
#endif
