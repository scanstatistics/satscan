// PageAnalysis.cpp

#include "stdafx.h"
#include "PageAnalysis.h"
#include "date.h"
#include "resource.hm"

CAnalysisPage::CAnalysisPage(CWindowsParameters* Params)
              :CPropertyPage(CAnalysisPage::IDD)
{
  //{{AFX_DATA_INIT(CAnalysisPage)
  //}}AFX_DATA_INIT

  m_pParameters = Params;
}

void CAnalysisPage::EnableAnalysisType()
{
  GetDlgItem(IDC_PURE_SPATIAL)->EnableWindow(TRUE);
  GetDlgItem(IDC_SPACETIME)->EnableWindow(m_pParameters->m_nPrecision != 0);
  GetDlgItem(IDC_PURE_TEMPORAL)->EnableWindow(m_pParameters->m_nPrecision != 0);
}

void CAnalysisPage::EnableDates()
{
  switch (m_pParameters->m_nPrecision)
  {
    case NONE  : HandlePrecsionNone();      break;
    case YEAR  : HandlePrecisionInYears();  break;
    case MONTH : HandlePrecisionInMonths(); break;
    case DAY   : HandlePrecisionInDays();   break;
  }
}

void CAnalysisPage::HandlePrecsionNone()
{
  GetDlgItem(IDC_SDT_MO)->EnableWindow(TRUE);
  GetDlgItem(IDC_SDT_DY)->EnableWindow(TRUE);
  GetDlgItem(IDC_EDT_MO)->EnableWindow(TRUE);
  GetDlgItem(IDC_EDT_DY)->EnableWindow(TRUE);
}

void CAnalysisPage::HandlePrecisionInYears()
{
  GetDlgItem(IDC_SDT_MO)->EnableWindow(FALSE);
  GetDlgItem(IDC_EDT_MO)->EnableWindow(FALSE);
  GetDlgItem(IDC_SDT_DY)->EnableWindow(FALSE);
  GetDlgItem(IDC_EDT_DY)->EnableWindow(FALSE);

  GetDlgItem(IDC_SDT_MO)->SetWindowText("1");
  GetDlgItem(IDC_EDT_MO)->SetWindowText("12");
  GetDlgItem(IDC_SDT_DY)->SetWindowText("1");
  GetDlgItem(IDC_EDT_DY)->SetWindowText("31");
}

void CAnalysisPage::HandlePrecisionInMonths()
{
  CString cszEndYear;
  CString cszEndMonth;
  UInt nEndDay;
  char szEndDay [10];

  GetDlgItem(IDC_SDT_MO)->EnableWindow(TRUE);
  GetDlgItem(IDC_EDT_MO)->EnableWindow(TRUE);
  GetDlgItem(IDC_SDT_DY)->EnableWindow(FALSE);
  GetDlgItem(IDC_EDT_DY)->EnableWindow(FALSE);

  GetDlgItem(IDC_EDT_YR)->GetWindowText(cszEndYear);
  GetDlgItem(IDC_EDT_MO)->GetWindowText(cszEndMonth);

  GetDlgItem(IDC_SDT_DY)->SetWindowText("1");

  nEndDay = DaysThisMonth(atoi(cszEndYear.GetBuffer(0)), atoi(cszEndMonth.GetBuffer(0)));
  itoa(nEndDay, szEndDay, 10);
  GetDlgItem(IDC_EDT_DY)->SetWindowText(szEndDay);
  cszEndYear.ReleaseBuffer();
  cszEndMonth.ReleaseBuffer();
}

void CAnalysisPage::HandlePrecisionInDays()
{
  GetDlgItem(IDC_SDT_MO)->EnableWindow(TRUE);
  GetDlgItem(IDC_SDT_DY)->EnableWindow(TRUE);
  GetDlgItem(IDC_EDT_MO)->EnableWindow(TRUE);
  GetDlgItem(IDC_EDT_DY)->EnableWindow(TRUE);
}

void CAnalysisPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  
  //{{AFX_DATA_MAP(CAnalysisPage)
  DDX_Radio(pDX, IDC_PURE_SPATIAL, m_pParameters->m_nAnalysisType);
  DDX_Radio(pDX, IDC_POISSON,      m_pParameters->m_nModel);
  DDX_Radio(pDX, IDC_SCAN_HIGH,    m_pParameters->m_nAreas);
  DDX_Text (pDX, IDC_REPLICAS,     m_pParameters->m_nReplicas);

  DDX_Text (pDX, IDC_SDT_YR,       m_pParameters->m_nStartYear);
  DDX_Text (pDX, IDC_SDT_MO,       m_pParameters->m_nStartMonth);
  DDX_Text (pDX, IDC_SDT_DY,       m_pParameters->m_nStartDay);
  DDX_Text (pDX, IDC_EDT_YR,       m_pParameters->m_nEndYear);
  DDX_Text (pDX, IDC_EDT_MO,       m_pParameters->m_nEndMonth);
  DDX_Text (pDX, IDC_EDT_DY,       m_pParameters->m_nEndDay);

  DDV_Month(pDX, IDC_SDT_MO,       m_pParameters->m_nStartMonth);
  DDV_Month(pDX, IDC_EDT_MO,       m_pParameters->m_nEndMonth);
  DDV_Day(pDX,   IDC_SDT_DY,       m_pParameters->m_nStartYear,
                                   m_pParameters->m_nStartMonth,
                                   m_pParameters->m_nStartDay);
  DDV_Day(pDX,   IDC_EDT_DY,       m_pParameters->m_nEndYear,
                                   m_pParameters->m_nEndMonth,
                                   m_pParameters->m_nEndDay);
  DDV_Year(pDX,  IDC_SDT_YR,       m_pParameters->m_nStartYear);
  DDV_Year(pDX,  IDC_EDT_YR,       m_pParameters->m_nEndYear);

  if (m_pParameters->m_nAnalysisType != 0)   // Not Purely Spatial
    DDV_Date_Range(pDX, IDC_SDT_YR,m_pParameters->m_nStartYear,
                                   m_pParameters->m_nStartMonth,
                                   m_pParameters->m_nStartDay,
                                   m_pParameters->m_nEndYear,   
                                   m_pParameters->m_nEndMonth, 
                                   m_pParameters->m_nEndDay,
                                   m_pParameters->m_nIntervalUnits, 
                                   m_pParameters->m_nIntervalLength);

  DDV_Replicas(pDX, IDC_REPLICAS,  m_pParameters->m_nReplicas);
  //}}AFX_DATA_MAP

  if (!(pDX->m_bSaveAndValidate))
  {
    EnableAnalysisType();
    EnableDates();
  }
}

void CAnalysisPage::OnPurelyTemporal()
{
  if (m_pParameters->m_nTimeAdjustType = 1)
    m_pParameters->m_nTimeAdjustType = 0; // Discrete Adjustment not valid for P.T. Analysis 
}

LONG CAnalysisPage::OnHelp(UINT, LONG lParam)
{
  ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
            AfxGetApp()->m_pszHelpFilePath,
            HELP_WM_HELP,
            (DWORD)(LPVOID)GetHelpIDs());
  return 0;
}

const DWORD CAnalysisPage::m_nHelpIDs[] =
{
  IDC_ANALYSIS_TYPE_GB, IDH_ANALYSIS_TYPE,
  IDC_PURE_SPATIAL, IDH_ANALYSIS_TYPE,
  IDC_SPACETIME, IDH_ANALYSIS_TYPE,
  IDC_PURE_TEMPORAL, IDH_ANALYSIS_TYPE,
  IDC_MODEL_GB, IDH_MODEL_TYPE,
  IDC_POISSON, IDH_MODEL_TYPE,
  IDC_BERNOULLI, IDH_MODEL_TYPE,
  IDC_AREAS_GB, IDH_AREAS,
  IDC_SCAN_HIGH, IDH_AREAS,
  IDC_SCAN_BOTH, IDH_AREAS,
  IDC_SCAN_LOW, IDH_AREAS,

  IDC_STUDY_PERIOD_GB, IDH_STUDY_PERIOD,
  IDC_SP_YEAR_TEXT, IDH_STUDY_PERIOD,
  IDC_SP_YYYY_TEXT, IDH_STUDY_PERIOD,
  IDC_SP_MONTH_TEXT,   IDH_STUDY_PERIOD,
  IDC_SP_MM_TEXT, IDH_STUDY_PERIOD,
  IDC_SP_DAY_TEXT, IDH_STUDY_PERIOD,
  IDC_SP_DD_TEXT, IDH_STUDY_PERIOD,
  IDC_STARTDT_TEXT, IDH_STUDY_PERIOD,
  IDC_SDT_YR, IDH_STUDY_PERIOD,
  IDC_SDT_MO, IDH_STUDY_PERIOD,
  IDC_SDT_DY, IDH_STUDY_PERIOD,
  IDC_ENDDT_TEXT, IDH_STUDY_PERIOD,
  IDC_EDT_YR, IDH_STUDY_PERIOD,
  IDC_EDT_MO, IDH_STUDY_PERIOD,
  IDC_EDT_DY, IDH_STUDY_PERIOD,

  IDC_REPLICAS_TEXT, IDH_REPLICAS,
  IDC_REPLICAS, IDH_REPLICAS
};

BEGIN_MESSAGE_MAP(CAnalysisPage, CPropertyPage)
  //{{AFX_MSG_MAP(CAnalysisPage)
  ON_BN_CLICKED(IDC_PURE_TEMPORAL, OnPurelyTemporal)
  ON_MESSAGE(WM_HELP, OnHelp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void AFXAPI DDV_Month(CDataExchange* pDX, int nIDC, int& nMonth)
{
  pDX->PrepareEditCtrl(nIDC);

  if (pDX->m_bSaveAndValidate)
  {
    if ((nMonth < 1 || nMonth > 12))
    {
      pDX->m_pDlgWnd->MessageBox("Please specify a month between 1 and 12.", "Parameter Error");
      pDX->Fail();
    }
  }
}

void AFXAPI DDV_Day(CDataExchange* pDX, int nIDC, int& nYear, int& nMonth, int& nDay)
{
  pDX->PrepareEditCtrl(nIDC);
  char szMessage[100];
  int nMin = 1;
  int nMax;

  if (pDX->m_bSaveAndValidate &&
      pDX->m_pDlgWnd->GetDlgItem(nIDC)->IsWindowEnabled())
  {
    nMax = DaysThisMonth(nYear, nMonth);
    if (nDay < nMin || nDay > nMax)
    {
      sprintf(szMessage, "Please specify a day between %i and %i.", nMin, nMax);
      pDX->m_pDlgWnd->MessageBox(szMessage, "Parameter Error");
      pDX->Fail();
    }
  }
}

void AFXAPI DDV_Year(CDataExchange* pDX, int nIDC, int& nYear)
{
  pDX->PrepareEditCtrl(nIDC);
  char szMessage[100];

  if (pDX->m_bSaveAndValidate)
  {
    if (!(nYear >= MIN_YEAR && nYear <= MAX_YEAR))
    {
      sprintf(szMessage, "Year must be between %i and %i.", MIN_YEAR, MAX_YEAR);
      pDX->m_pDlgWnd->MessageBox(szMessage, "Parameter Error");
      pDX->Fail();
    }
  }

}

void AFXAPI DDV_Date_Range(CDataExchange* pDX, int nIDC,
                           int& nStartYear, int& nStartMonth, int& nStartDay,
                           int& nEndYear, int& nEndMonth, int& nEndDay,
                           int& nIntervalUnits, long& nIntervalLength)
{
  HWND  hWndCtrl = pDX->PrepareEditCtrl(nIDC);
  char szMessage[100];
  char szUnit[10];
  Julian Start, End;
  long nTimeBetween;

  if (pDX->m_bSaveAndValidate)
  {
    Start = MDYToJulian(nStartMonth, nStartDay, nStartYear);
    End   = MDYToJulian(nEndMonth,   nEndDay,   nEndYear);
    nTimeBetween  = TimeBetween(Start, End, nIntervalUnits+1);

    switch(nIntervalUnits+1)
    {
      case(YEAR) : strcpy(szUnit, "year(s)"); break;
      case(MONTH): strcpy(szUnit, "month(s)"); break;
      case(DAY)  : strcpy(szUnit, "day(s)"); break;
      default    : break;
    };

    if (nIntervalLength > nTimeBetween)
    {
      sprintf(szMessage, "Due to interval length specified (Time Parameters tab) the start and end dates must be at least %d %s apart.", nIntervalLength, szUnit);
      pDX->m_pDlgWnd->MessageBox(szMessage, "Parameter Error");
      pDX->Fail();
    }
  }
}

void AFXAPI DDV_Replicas(CDataExchange* pDX, int nIDC, int& nValue)
{
  pDX->PrepareEditCtrl(nIDC);

  if (pDX->m_bSaveAndValidate)
  {
    if (!((nValue == 0 || nValue == 9 || nValue == 19 || fmod(nValue+1, 1000) == 0.0 ) /*&& nValue <= 29999*/))
    {
       pDX->m_pDlgWnd->MessageBox("Invalid number of replicas specified.", "Parameter Error");
       pDX->Fail();
    }
  }
}

