//PageTimeParameters.h

#ifndef __PAGETIMEPARAMETERS_H
#define __PAGETIMEPARAMETERS_H

#include "res/resource.h"
#include "paramwin.h"

class CTimeParametersPage : public CPropertyPage
{
  public:
    CTimeParametersPage(CWindowsParameters* Params);
    ~CTimeParametersPage() {};
    
    CWindowsParameters* m_pParameters;

    //{{AFX_DATA(CTimeParametersPage)
    enum { IDD = IDD_TIMEPARAMETERS_PAGE };
    //}}AFX_DATA

  protected:
    
    static const DWORD m_nHelpIDs[];
    virtual const DWORD* GetHelpIDs() {return m_nHelpIDs;}

    //{{AFX_VIRTUAL(CTimeParametersPage)
    virtual void DoDataExchange(CDataExchange* pDX);
    //}}AFX_VIRTUAL

    void EnableTimeTrendAdj();
    void EnableTimeIntervals();
    void OnTimeTrendNone();
    void OnTimeTrendNonparametric();
    void OnTimeTrendLinear();

    //{{AFX_MSG(CTimeParametersPage)
    LONG OnHelp(UINT, LONG lParam);
    LONG OnHelpContextMenu(UINT wParam, LONG);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
      
};

void AFXAPI DDV_IntervalLength(CDataExchange* pDX, int nIDC,
                               int& nStartYear, int& nStartMonth, int& nStartDay,
                               int& nEndYear, int& nEndMonth, int& nEndDay,
                               int& nIntervalUnits, long& nIntervalLength);
void AFXAPI DDV_TimeTrendPercentage(CDataExchange* pDX, int nIDC, double& nValue);

#endif
