#ifndef __PAGEANALYSIS_H
#define __PAGEANALYSIS_H

#include "res/resource.h"
#include "paramwin.h"

class CAnalysisPage : public CPropertyPage
{
  public:
    CAnalysisPage(CWindowsParameters* Params);
    ~CAnalysisPage() {};

   CWindowsParameters* m_pParameters;
    
    //{{AFX_DATA(CAnalysisPage)
    enum { IDD = IDD_ANALYSIS_PAGE };
    //}}AFX_DATA

  protected:
    
    static const DWORD m_nHelpIDs[];
    virtual const DWORD* GetHelpIDs() {return m_nHelpIDs;}

    //{{AFX_VIRTUAL(CAnalysisPage)
    virtual void DoDataExchange(CDataExchange* pDX);
    //}}AFX_VIRTUAL

    void EnableAnalysisType();
    void EnableDates();
    void HandlePrecsionNone();   
    void HandlePrecisionInYears();
    void HandlePrecisionInMonths();
    void HandlePrecisionInDays();
    void SetIntervalUnits();

    //{{AFX_MSG(CAnalysisPage)
    void OnPurelyTemporal();
    LONG OnHelp(UINT, LONG lParam);
    LONG OnHelpContextMenu(UINT wParam, LONG);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
      
};

void AFXAPI DDV_Month(CDataExchange* pDX, int nIDC, int& nMonth);
void AFXAPI DDV_Day(CDataExchange* pDX, int nIDC, int& nYear, int& nMonth, int& nDay);
void AFXAPI DDV_Year(CDataExchange* pDX, int nIDC, int& nYear);
void AFXAPI DDV_Date_Range(CDataExchange* pDX, int nIDC,
                           int& nStartYear, int& nStartMonth, int& nStartDay,
                           int& nEndYear, int& nEndMonth, int& nEndDay,
                           int& nIntervalUnits, long& nIntervalLength);
void AFXAPI DDV_Replicas(CDataExchange* pDX, int nIDC, int& nValue);

#endif
