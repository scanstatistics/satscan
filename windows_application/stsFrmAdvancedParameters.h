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
//---------------------------------------------------------------------------
class TfrmAdvancedParameters : public TForm {
__published:	// IDE-managed Components
        TPanel *pnlButtons;
        TPanel *pnlClient;
        TGroupBox *grpInputFiles;
        TLabel *Label21;
        TEdit *edtMaxCirclePopulationFilename;
        TButton *btnBrowseMaxCirclePopFile;
        TGroupBox *grpAnalysis;
        TCheckBox *chkTerminateEarly;
        TGroupBox *grpScanningWindow;
        TLabel *lblReportSmallerClusters;
        TCheckBox *chkRestrictReportedClusters;
        TEdit *edtReportClustersSmallerThan;
        TButton *btnOk;
        TCheckBox *chkRestrictTemporalRange;
        TStaticText *stStartRangeTo;
        TEdit *edtStartRangeStartYear;
        TEdit *edtStartRangeStartMonth;
        TEdit *edtStartRangeStartDay;
        TEdit *edtStartRangeEndYear;
        TEdit *edtStartRangeEndMonth;
        TEdit *edtStartRangeEndDay;
        TOpenDialog *OpenDialog;
        TStaticText *stStartWindowRange;
        TStaticText *stEndWindowRange;
        TEdit *edtEndRangeStartYear;
        TEdit *edtEndRangeStartMonth;
        TEdit *edtEndRangeStartDay;
        TStaticText *stEndRangeTo;
        TEdit *edtEndRangeEndYear;
        TEdit *edtEndRangeEndMonth;
        TEdit *edtEndRangeEndDay;
        void __fastcall FormKeyPress(TObject *Sender, char &Key);
        void __fastcall chkRestrictTemporalRangeClick(TObject *Sender);
        void __fastcall btnBrowseMaxCirclePopFileClick(TObject *Sender);
        void __fastcall edtMaxCirclePopulationFilenameChange(TObject *Sender);
        void __fastcall chkRestrictReportedClustersClick(TObject *Sender);
        void __fastcall edtReportClustersSmallerThanExit(TObject *Sender);
        void __fastcall edtReportClustersSmallerThanKeyPress(TObject *Sender, char &Key);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall NaturalNumberKeyPress(TObject *Sender, char &Key);
        void __fastcall edtStartRangeStartDateExit(TObject *Sender);
        void __fastcall edtEndRangeStartDateExit(TObject *Sender);
        void __fastcall edtStartRangeEndDateExit(TObject *Sender);
        void __fastcall edtEndRangeEndDateExit(TObject *Sender);

  private:
    TfrmAnalysis              & gAnalysisSettings;
    TWinControl               * gpFocusControl;

    void                        Init() {gpFocusControl=0;}
    void                        ParseDate(const std::string& sDate, TEdit& Year, TEdit& Month, TEdit& Day, bool bStartRange);
    void                        Setup();
    void                        ValidateReportedSpatialClusterSize();
    void                        ValidateScanningWindowRanges();

  public:
    __fastcall TfrmAdvancedParameters(TfrmAnalysis & AnalysisSettings);

    void                        EnableSpatialOptions(bool bEnable);
    void                        EnableTemporalOptions(bool bEnable, bool bEnableRanges);
    void                        SaveParameterSettings();
    void                        SetMaximumCirclePopulationFile(const char * sMaximumCirclePopulationFileName);
    void                        SetReportingClustersText(const ZdString& sText);
    void                        ShowDialog(TWinControl * pFocusControl=0);
    static void                 ValidateDate(TEdit& YearControl, TEdit& MonthControl, TEdit& DayControl);
    void                        ValidateInputFilesSettings();
    void                        ValidateScanningWindowSettings();
};

class AdvancedFeaturesException : public virtual ZdException {
  private:
    TWinControl       & gFocusControl;
    
  public:
    AdvancedFeaturesException(va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel, TWinControl& FocusControl);
    virtual ~AdvancedFeaturesException();

    TWinControl       & GetFocusControl() const {return gFocusControl;}
};

void GenerateAFException(const char * sMessage, const char * sSourceModule, TWinControl & FocusControl, ...);

//---------------------------------------------------------------------------
#endif
