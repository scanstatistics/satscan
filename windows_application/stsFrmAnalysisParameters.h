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

#include "JulianDates.h"
#include "Parameters.h"

//---------------------------------------------------------------------------
class TfrmAnalysis : public TForm {
  __published:  // IDE-managed Components
        TPageControl *PageControl1;
        TTabSheet *tbInputFiles;
        TTabSheet *tbAnalysis;
        TTabSheet *tbScanningWindow;
        TTabSheet *tbTimeParameter;
        TTabSheet *tbOutputFiles;
        TGroupBox *GroupBox1;
        TRadioGroup *rgPrecisionTimes;
        TEdit *edtCaseFileName;
        TLabel *Label1;
        TEdit *edtControlFileName;
        TLabel *Label2;
        TButton *btnCaseBrowse;
        TButton *btnControlBrowse;
        TLabel *Label3;
        TEdit *edtPopFileName;
        TLabel *Label4;
        TLabel *Label5;
        TButton *btnPopBrowse;
        TGroupBox *GroupBox2;
        TEdit *edtCoordinateFileName;
        TLabel *Label6;
        TLabel *Label7;
        TEdit *edtGridFileName;
        TButton *btnCoordBrowse;
        TButton *btnGridBrowse;
        TRadioGroup *rgCoordinates;
        TRadioGroup *rgTypeAnalysis;
        TRadioGroup *rgProbability;
        TRadioGroup *rgScanAreas;
        TGroupBox *GroupBox3;
        TLabel *Label8;
        TLabel *Label9;
        TEdit *edtStartYear;
        TEdit *edtEndYear;
        TEdit *edtStartMonth;
        TEdit *edtEndMonth;
        TEdit *edtStartDay;
        TEdit *edtEndDay;
        TLabel *Label10;
        TLabel *Label11;
        TLabel *Label12;
        TLabel *Label13;
        TLabel *Label14;
        TLabel *Label15;
        TEdit *edtMontCarloReps;
        TLabel *Label16;
        TGroupBox *GroupBox4;
        TGroupBox *GroupBox5;
        TLabel *Label17;
        TEdit *edtMaxClusterSize;
        TCheckBox *chkInclPurTempClust;
        TLabel *Label18;
        TEdit *edtMaxTemporalClusterSize;
        TCheckBox *chkIncludePurSpacClust;
        TRadioGroup *rgClustersToInclude;
        TGroupBox *GroupBox6;
        TLabel *Label19;
        TRadioButton *rbUnitYear;
        TRadioButton *rbUnitMonths;
        TRadioButton *rbUnitDay;
        TLabel *Label20;
        TEdit *edtUnitLength;
        TRadioGroup *rgTemporalTrendAdj;
        TEdit *edtLogPerYear;
        TOpenDialog *OpenDialog1;
        TSaveDialog *SaveDialog;
        TEdit *Edit1;
        TGroupBox *GroupBox8;
        TLabel *Label23;
        TLabel *Label24;
        TLabel *Label25;
        TLabel *Label26;
        TLabel *Label27;
        TLabel *Label28;
        TEdit *edtProspYear;
        TEdit *edtProspMonth;
        TEdit *edtProspDay;
        TLabel *Label29;
        TEdit *edtResultFile;
        TButton *btnResultFileBrowse;
        TGroupBox *grpB1;
        TCheckBox *chkCensusAreas;
        TCheckBox *chkLikelyClusters;
        TCheckBox *chkInclRelRiskEst;
        TCheckBox *chkInclSimLogLik;
        TRadioButton *rdoPercentageTemproal;
        TRadioButton *rdoTimeTemproal;
        TGroupBox *grpCriteriaSecClusters;
        TComboBox *cboCriteriaSecClusters;
        TGroupBox *grpB2;
        TCheckBox *chkDbaseFile1;
        TCheckBox *chkDbaseFile2;
        TRadioButton *rdoSpatialPercentage;
        TRadioButton *rdoSpatialDistance;
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
        void __fastcall chkInclRelRiskEstClick(TObject *Sender);
        void __fastcall chkInclSimLogLikClick(TObject *Sender);
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
        void __fastcall chkCensusAreasClick(TObject *Sender);
        void __fastcall chkLikelyClustersClick(TObject *Sender);
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

  private:	// User declarations
    CParameters       * gpParams;
    AnsiString          gsParamFileName;
    ZdVector<const char*>       gvGeoFileFieldDescriptors,
                                gvCaseFileFieldDescriptors,
                                gvGridFileFieldDescriptors,
                                gvPopFileFieldDescriptors,
                                gvControlFileFieldDescriptors;

    bool                Check_Days(int iYear, int iMonth, int iDay, char *sDateName);
    bool                Check_IntervalLength(int iStartYear, int iStartMonth, int iStartDay,
                                             int iEndYear, int iEndMonth, int iEndDay,
                                             int iIntervalUnits, int iIntervalLength);
    bool                Check_Month(int iMonth, char *sDateName);
    bool                Check_TimeTrendPercentage(double dValue);
    bool                Check_Year(int iYear, char *sDateName);
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
    void                ConvertPurelySpacialIntervals();
    void                CreateTXDFile(const ZdFileName& sFileName, const ZdVector<const char*>& vFieldNames);
    void                DataExchange();
    bool                DetermineIfDbfExtension(AnsiString sFileName);
    void                EnableAdditionalAsciiFileOptions();
    void                EnableAnalysisType(bool bValue);
    void                EnablePopulationFileInput();
    void                EnablePrecision();
    void                EnableProspStartDate();
    void                EnableScanningWindow();
    void                EnableTimeIntervals();
    void                EnableTimeTrendAdj();
    void                Init();
    void                ParseDate(char * szDate, TEdit *pYear, TEdit *pMonth, TEdit *pDay);
    bool                ReadSession(char *sFileName);
    void                SaveTextParameters();
    void                SetSpatialDistanceCaption();
    void                SetupCaseFileFieldDescriptors();
    void                SetupControlFileFieldDescriptors();
    void                SetupGeoFileFieldDescriptors();
    void                SetupGridFileFieldDescriptors();
    void                SetupImportDescriptor(BFTFImportDescriptor& descrip, const ZdString& sImportFileName);
    void                SetupPopFileFieldDescriptors();
    void                SetupInterface();
    bool                ValidateInputFiles();
    bool                ValidateSpatialClusterSize();
    bool                ValidateTemoralClusterSize();
    bool                VerifyUnitAndLength();

public:		// User declarations
            __fastcall TfrmAnalysis(TComponent* Owner, char *sParamFileName = 0);
    virtual __fastcall ~TfrmAnalysis();

    char              * GetFileName();
    CParameters       * GetSession();
    void                SaveAs();
    bool                ValidateParams();
    bool                WriteSession();
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmAnalysis *frmAnalysis;
//---------------------------------------------------------------------------
#endif
