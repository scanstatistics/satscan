// PageScanningWindow.cpp

#include "stdafx.h"
#include "PageScanningWindow.h"
#include "resource.hm"

CScanningWindowPage::CScanningWindowPage(CWindowsParameters* Params)
              :CPropertyPage(CScanningWindowPage::IDD)
{
  //{{AFX_DATA_INIT(CScanningWindowPage)
  //}}AFX_DATA_INIT

  m_pParameters = Params;
}

void CScanningWindowPage::EnableScanningWindow()
{
  BOOL bSpatial  = (m_pParameters->m_nAnalysisType==0 || m_pParameters->m_nAnalysisType==1); //PS, ST
  BOOL bTemporal = (m_pParameters->m_nAnalysisType==1 || m_pParameters->m_nAnalysisType==2); //ST, PT
   
  GetDlgItem(IDC_MAXGEOSIZE)->EnableWindow(bSpatial);
  GetDlgItem(IDC_INCLPURETEMP)->EnableWindow(bSpatial && bTemporal);

  GetDlgItem(IDC_MAXTIMESIZE)->EnableWindow(bTemporal);
  GetDlgItem(IDC_INCLPURESP)->EnableWindow(bSpatial && bTemporal);

  GetDlgItem(IDC_ALIVEONLY)->EnableWindow(bTemporal);
  GetDlgItem(IDC_ALL_CLUSTERS)->EnableWindow(bTemporal);
}

void CScanningWindowPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  
  //{{AFX_DATA_MAP(CScanningWindowPage)
  DDX_Text (pDX, IDC_MAXGEOSIZE,   m_pParameters->m_nMaxGeographicClusterSize);
  DDX_Check(pDX, IDC_INCLPURETEMP, m_pParameters->m_bIncludePurelyTemporal_Win);
  DDX_Text (pDX, IDC_MAXTIMESIZE,  m_pParameters->m_nMaxTemporalClusterSize);
  DDX_Check(pDX, IDC_INCLPURESP,   m_pParameters->m_bIncludePurelySpatial_Win);
  DDX_Radio(pDX, IDC_ALL_CLUSTERS, m_pParameters->m_bAliveClustersOnly_Win);

  DDV_MaxGeoSize(pDX, IDC_MAXGEOSIZE, m_pParameters->m_nMaxGeographicClusterSize);
  DDV_MaxTimeSize(pDX, IDC_MAXTIMESIZE, m_pParameters->m_nMaxTemporalClusterSize);
  //}}AFX_DATA_MAP

  if (!(pDX->m_bSaveAndValidate))
    EnableScanningWindow();

}

LONG CScanningWindowPage::OnHelp(UINT, LONG lParam)
{
  ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
            AfxGetApp()->m_pszHelpFilePath,
            HELP_WM_HELP,
            (DWORD)(LPVOID)GetHelpIDs());
  return 0;
}

const DWORD CScanningWindowPage::m_nHelpIDs[] =
{
  IDC_MAXGEOSIZE_TEXT, IDH_MAXGEOSIZE,
  IDC_MAXGEOSIZE, IDH_MAXGEOSIZE,
  IDC_INCLPURETEMP, IDH_INCLPURETEMP,

  IDC_MAXTIMESIZE_TEXT, IDH_MAXTIMESIZE,
  IDC_MAXTIMESIZE, IDH_MAXTIMESIZE,
  IDC_INCLPURESP, IDH_INCLPURESP,

  IDC_CLUSTERS_TO_INCLUDE_GB, IDH_CLUSTERS_TO_INCLUDE,
  IDC_ALIVEONLY, IDH_CLUSTERS_TO_INCLUDE,
  IDC_ALL_CLUSTERS, IDH_CLUSTERS_TO_INCLUDE
};

BEGIN_MESSAGE_MAP(CScanningWindowPage, CPropertyPage)
  //{{AFX_MSG_MAP(CScanningWindowPage)
  ON_MESSAGE(WM_HELP, OnHelp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()
      
void AFXAPI DDV_MaxGeoSize(CDataExchange* pDX, int nIDC, int& nValue)
{
  pDX->PrepareEditCtrl(nIDC);

  if (pDX->m_bSaveAndValidate)
  {
    if (!(nValue > 0.0  && nValue <= 50.0))
    {
       pDX->m_pDlgWnd->MessageBox("Invalid maximum geographic size specified.", "Parameter Error");
       pDX->Fail();
    }
  }
}

void AFXAPI DDV_MaxTimeSize(CDataExchange* pDX, int nIDC, int& nValue)
{
  pDX->PrepareEditCtrl(nIDC);

  if (pDX->m_bSaveAndValidate)
  {
    if (!(nValue > 0.0 && nValue <= 90.0))
    {
       pDX->m_pDlgWnd->MessageBox("Invalid maximum time size specified.", "Parameter Error");
       pDX->Fail();
    }
  }
}

