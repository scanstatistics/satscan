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
//---------------------------------------------------------------------------
class TfrmAdvancedParameters : public TForm {
__published:	// IDE-managed Components
        TPanel *pnlButtons;
        TButton *btnOk;
        TOpenDialog *OpenDialog;
        TPageControl *PageControl;
        TTabSheet *tsAdjustmentsTabSheet;
        TTabSheet *tsOther;
        TGroupBox *grpAnalysis;
        TCheckBox *chkTerminateEarly;
        TGroupBox *grpScanningWindow;
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
        TGroupBox *grpOutput;
        TLabel *lblReportSmallerClusters;
        TCheckBox *chkRestrictReportedClusters;
        TEdit *edtReportClustersSmallerThan;
        TRadioGroup *rdgTemporalTrendAdj;
        TEdit *edtLogLinear;
        TStaticText *lblLogLinear;
        TGroupBox *grpAdjustments;
        TLabel *lblAdjustmentsByRelativeRisksFile;
        TEdit *edtAdjustmentsByRelativeRisksFile;
        TButton *btnBrowseAdjustmentsFile;
        TCheckBox *chkAdjustForKnownRelativeRisks;
        void __fastcall FormKeyPress(TObject *Sender, char &Key);
        void __fastcall chkRestrictTemporalRangeClick(TObject *Sender);
        void __fastcall chkRestrictReportedClustersClick(TObject *Sender);
        void __fastcall edtReportClustersSmallerThanExit(TObject *Sender);
        void __fastcall edtReportClustersSmallerThanKeyPress(TObject *Sender, char &Key);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall NaturalNumberKeyPress(TObject *Sender, char &Key);
        void __fastcall edtStartRangeStartDateExit(TObject *Sender);
        void __fastcall edtEndRangeStartDateExit(TObject *Sender);
        void __fastcall edtStartRangeEndDateExit(TObject *Sender);
        void __fastcall edtEndRangeEndDateExit(TObject *Sender);
        void __fastcall rdgTemporalTrendAdjClick(TObject *Sender);
        void __fastcall edtLogLinearExit(TObject *Sender);
        void __fastcall FloatKeyPress(TObject *Sender, char &Key);
        void __fastcall btnBrowseAdjustmentsFileClick(TObject *Sender);
        void __fastcall edtAdjustmentsByRelativeRisksFileChange(TObject *Sender);
        void __fastcall chkAdjustForKnownRelativeRisksClick(TObject *Sender);

  private:
    TfrmAnalysis              & gAnalysisSettings;
    TWinControl               * gpFocusControl;
    bool                        gbEnableRangeYears;  /** stores enable dictated by main interface */
    bool                        gbEnableRangeMonths; /** stores enable dictated by main interface */
    bool                        gbEnableRangeDays;   /** stores enable dictated by main interface */
    bool                        gbEnableAdjustmentsByRR; /** stores enable dictated by main interface */

    TimeTrendAdjustmentType     GetAdjustmentTimeTrendControlType() const;
    void                        Init() {gpFocusControl=0;}
    void                        ParseDate(const std::string& sDate, TEdit& Year, TEdit& Month, TEdit& Day, bool bStartRange);
    void                        RefreshTemporalOptionsEnables();
    void                        Setup();
    void                        ValidateReportedSpatialClusterSize();
    void                        ValidateScanningWindowRanges();

  public:
    __fastcall TfrmAdvancedParameters(TfrmAnalysis & AnalysisSettings);

    void                        EnableAdjustmentsGroup(bool bEnable);
    void                        EnableAdjustmentForTimeTrendOptionsGroup(bool bEnable, bool bTimeStratified, bool bLogYearPercentage);
    void                        EnableSpatialOutputOptions(bool bEnable);
    void                        EnableTemporalOptions(bool bEnable, bool bEnableRanges);
    void                        SaveParameterSettings();
    void                        SetAdjustmentsByRelativeRisksFile(const char * sAdjustmentsForRelativeRisksFileName);
    void                        SetRangeDateEnables(bool bYear, bool bMonth, bool bDay);
    void                        SetReportingClustersText(const ZdString& sText);
    void                        SetTemporalTrendAdjustmentControl(TimeTrendAdjustmentType eTimeTrendAdjustmentType);
    void                        ShowDialog(TWinControl * pFocusControl=0);
    void                        ValidateAdjustmentSettings();
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
