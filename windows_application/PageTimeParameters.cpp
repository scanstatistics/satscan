// PageTimeParameters.cpp

#include "stdafx.h"
#include "PageTimeParameters.h"
#include "date.h"
#include "resource.hm"

CTimeParametersPage::CTimeParametersPage(CWindowsParameters* Params)
              :CPropertyPage(CTimeParametersPage::IDD)
{
  //{{AFX_DATA_INIT(CTimeParametersPage)
  //}}AFX_DATA_INIT

  m_pParameters = Params;
}

void CTimeParametersPage::EnableTimeTrendAdj()
{
  BOOL bTemporal = (m_pParameters->m_nAnalysisType==1 || m_pParameters->m_nAnalysisType==2); //ST, PT
   
  if (m_pParameters->m_nModel == POISSON)
  {
    GetDlgItem(IDC_TT_NONE)->EnableWindow(bTemporal);
    GetDlgItem(IDC_TT_NONPARAMETRIC)->EnableWindow(bTemporal && m_pParameters->m_nAnalysisType!=2);
    GetDlgItem(IDC_TT_LINEAR)->EnableWindow(bTemporal);
    GetDlgItem(IDC_TT_LINEAR_PERC)->EnableWindow(bTemporal && m_pParameters->m_nTimeAdjustType==LINEAR);
  }
  else if (m_pParameters->m_nModel == BERNOULLI)
  {
    GetDlgItem(IDC_TT_NONE)->EnableWindow(FALSE);
    GetDlgItem(IDC_TT_NONPARAMETRIC)->EnableWindow(FALSE);
    GetDlgItem(IDC_TT_LINEAR)->EnableWindow(FALSE);
    GetDlgItem(IDC_TT_LINEAR_PERC)->EnableWindow(FALSE);
  }
}

void CTimeParametersPage::EnableTimeIntervals()
{
  BOOL bTemporal = (m_pParameters->m_nAnalysisType==1 || m_pParameters->m_nAnalysisType==2); //ST, PT
  BOOL bEnableMo = (m_pParameters->m_nPrecision >= 2);
  BOOL bEnableDy = (m_pParameters->m_nPrecision >= 3);

  GetDlgItem(IDC_INTU_YR)->EnableWindow(bTemporal);
  GetDlgItem(IDC_INTU_MO)->EnableWindow(bTemporal && bEnableMo);
  GetDlgItem(IDC_INTU_DY)->EnableWindow(bTemporal && bEnableDy);
  GetDlgItem(IDC_TIMEINTLEN)->EnableWindow(bTemporal);
}

void CTimeParametersPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  
  //{{AFX_DATA_MAP(CTimeParametersPage)
    DDX_Radio(pDX, IDC_INTU_YR,      m_pParameters->m_nIntervalUnits);
    DDX_Text (pDX, IDC_TIMEINTLEN,   m_pParameters->m_nIntervalLength);
    DDX_Radio(pDX, IDC_TT_NONE,      m_pParameters->m_nTimeAdjustType);
    DDX_Text (pDX, IDC_PERC_TT_ADJ,   m_pParameters->m_nTimeAdjPercent);

  if (m_pParameters->m_nAnalysisType != 0)   // Not Purely Spatial
  {
    DDV_IntervalLength(pDX, IDC_TIMEINTLEN,
                       m_pParameters->m_nStartYear, m_pParameters->m_nStartMonth, m_pParameters->m_nStartDay,
                       m_pParameters->m_nEndYear,   m_pParameters->m_nEndMonth, m_pParameters->m_nEndDay,
                       m_pParameters->m_nIntervalUnits, m_pParameters->m_nIntervalLength);

    DDV_TimeTrendPercentage(pDX, IDC_TT_LINEAR_PERC, m_pParameters->m_nTimeAdjPercent);
  }
  //}}AFX_DATA_MAP

  if (!(pDX->m_bSaveAndValidate))
  {
    EnableTimeTrendAdj();
    EnableTimeIntervals();
  }
}

void CTimeParametersPage::OnTimeTrendNone()
{
  GetDlgItem(IDC_TT_LINEAR_PERC)->EnableWindow(FALSE);
}

void CTimeParametersPage::OnTimeTrendNonparametric()
{
  GetDlgItem(IDC_TT_LINEAR_PERC)->EnableWindow(FALSE);
}

void CTimeParametersPage::OnTimeTrendLinear()
{
  GetDlgItem(IDC_TT_LINEAR_PERC)->EnableWindow(TRUE);
}

LONG CTimeParametersPage::OnHelp(UINT, LONG lParam)
{
  ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
            AfxGetApp()->m_pszHelpFilePath,
            HELP_WM_HELP,
            (DWORD)(LPVOID)GetHelpIDs());
  return 0;
}

const DWORD CTimeParametersPage::m_nHelpIDs[] =
{
  IDC_INTERVALS_GB, IDH_INTERVALS,
  IDC_INTU_TEXT, IDH_INTERVALS,
  IDC_INTU_YR, IDH_INTERVALS,
  IDC_INTU_MO, IDH_INTERVALS,
  IDC_INTU_DY, IDH_INTERVALS,
  IDC_TIMEINTLEN_TEXT, IDH_INTERVALS,
  IDC_TIMEINTLEN, IDH_INTERVALS,

  IDC_TIMETREND_GB, IDH_TIMETREND,
  IDC_TT_NONE, IDH_TIMETREND,
  IDC_TT_NONPARAMETRIC, IDH_TIMETREND,
  IDC_TT_LINEAR, IDH_TIMETREND,
  IDC_TT_LINEAR_PERC, IDH_TIMETREND,
  IDC_TT_LINEAR_2, IDH_TIMETREND
};

BEGIN_MESSAGE_MAP(CTimeParametersPage, CPropertyPage)
  //{{AFX_MSG_MAP(CTimeParametersPage)
  ON_BN_CLICKED(IDC_TT_NONE,     OnTimeTrendNone)
  ON_BN_CLICKED(IDC_TT_NONPARAMETRIC, OnTimeTrendNonparametric)
  ON_BN_CLICKED(IDC_TT_LINEAR,     OnTimeTrendLinear)
  ON_MESSAGE(WM_HELP, OnHelp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()
      
void AFXAPI DDV_IntervalLength(CDataExchange* pDX, int nIDC,
                               int& nStartYear, int& nStartMonth, int& nStartDay,
                               int& nEndYear, int& nEndMonth, int& nEndDay,
                               int& nIntervalUnits, long& nIntervalLength)
{
  pDX->PrepareEditCtrl(nIDC);
  char szMessage[100];
  Julian Start, End;
  long nMin = 1;
  long nMax;

  if (pDX->m_bSaveAndValidate)
  {
    Start = MDYToJulian(nStartMonth, nStartDay, nStartYear);
    End   = MDYToJulian(nEndMonth,   nEndDay,   nEndYear);
    nMax  = TimeBetween(Start, End, nIntervalUnits+1);
    if (nIntervalLength < nMin || nIntervalLength > nMax)
    {
      sprintf(szMessage, "The interval length must be between %i and %i.", nMin, nMax);
      pDX->m_pDlgWnd->MessageBox(szMessage, "Parameter Error");
      pDX->Fail();
    }
  }
}

void AFXAPI DDV_TimeTrendPercentage(CDataExchange* pDX, int nIDC, double& nValue)
{
  pDX->PrepareEditCtrl(nIDC);

  if (pDX->m_bSaveAndValidate)
  {
    if (!(nValue > -100.00))
    {
       pDX->m_pDlgWnd->MessageBox("Invalid time trend percentage specified.", "Parameter Error");
       pDX->Fail();
    }
  }
}

