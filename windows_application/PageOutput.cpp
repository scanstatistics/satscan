// PageOutput.cpp

#include "stdafx.h"
#include "PageOutput.h"
#include "resource.hm"

COutputPage::COutputPage(CWindowsParameters* Params)
              :CPropertyPage(COutputPage::IDD)
{
  //{{AFX_DATA_INIT(COutputPage)
  //}}AFX_DATA_INIT

  m_pParameters = Params;
}

bool COutputPage::ValidateOutputFiles()
{
  if (!(ValidateFileCanCreate(IDC_OUTPUTFILE, m_pParameters->m_cszOutputFilename, "Output")))
    return false;
  else
    return true;
}

bool COutputPage::ValidateFileCanCreate(int nIDC, CString szFilename, char* szDescription)
{
  FILE* fp;
  bool  bValid = true;
  char  szMessage[400];
  char  szTitle[40];

  strcpy(szTitle, szDescription);
  strcat(szTitle, " File Error");

  if (GetDlgItem(nIDC)->IsWindowEnabled())
  {
    if (szFilename.GetLength()==0 || (fp = fopen(szFilename, "w")) == NULL) 
    {
       sprintf(szMessage, "File %s could not be created.", szFilename.GetBuffer(0));
       szFilename.ReleaseBuffer();
       MessageBox(szMessage, szTitle);
       GetDlgItem(nIDC)->SetFocus();
       bValid = false;
    }
    else 
      fclose(fp);
  }
  return bValid;
}

void COutputPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  
  //{{AFX_DATA_MAP(COutputPage)
  DDX_Text (pDX, IDC_OUTPUTFILE, m_pParameters->m_cszOutputFilename);
  DDX_Check(pDX, IDC_OUTPUT_RR,  m_pParameters->m_bOutputRelRisks_Win);
  DDX_Check(pDX, IDC_OUTPUT_LLR, m_pParameters->m_bSaveSimLogLikelihoods_Win);

//  DDV_FileCanCreate(pDX, IDC_OUTPUTFILE, m_pParameters->m_cszOutputFilename);
  //}}AFX_DATA_MAP
}

void COutputPage::OnBrowseResults()
{
  CString cszFilename;
  CString cszDlgTitle = CString("Select Results File");
  CFileDialog dlg(TRUE,_T("TXT"),_T("*.TXT"),OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
                  _T("Results Files (*.TXT)|*.TXT|All Files (*.*)|*.*||"),this);

  dlg.m_ofn.lpstrTitle=cszDlgTitle.GetBuffer(0);
  cszDlgTitle.ReleaseBuffer();

  if (dlg.DoModal()==IDOK)
  {
    m_pParameters->m_cszOutputFilename = dlg.GetPathName();
    UpdateData(FALSE);
  }

}

LONG COutputPage::OnHelp(UINT, LONG lParam)
{
  ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
            AfxGetApp()->m_pszHelpFilePath,
            HELP_WM_HELP,
            (DWORD)(LPVOID)GetHelpIDs());
  return 0;
}

const DWORD COutputPage::m_nHelpIDs[] =
{
  IDC_OUTPUTFILE_TEXT, IDH_OUTPUTFILE,
  IDC_OUTPUTFILE, IDH_OUTPUTFILE,
  IDC_BRWS_RSLTS, IDH_OUTPUTFILE,
  IDC_OUTPUT_RR,  IDH_OUTPUT_RR,
  IDC_OUTPUT_LLR, IDH_OUTPUT_LLR
};

BEGIN_MESSAGE_MAP(COutputPage, CPropertyPage)
  //{{AFX_MSG_MAP(COutputPage)
  ON_BN_CLICKED(IDC_BRWS_RSLTS,  OnBrowseResults)
  ON_MESSAGE(WM_HELP, OnHelp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()
      
void AFXAPI DDV_FileCanCreate(CDataExchange* pDX, int nIDC, CString szFilename)
{
  pDX->PrepareEditCtrl(nIDC);
  char szMessage[400];
  FILE* fp;

  if (pDX->m_bSaveAndValidate && pDX->m_pDlgWnd->GetDlgItem(nIDC)->IsWindowEnabled())
  {
    if ((fp = fopen(szFilename, "w")) == NULL)
    {
       sprintf(szMessage, "File %s could not be created.", szFilename.GetBuffer(0));
       szFilename.ReleaseBuffer();
       pDX->m_pDlgWnd->MessageBox(szMessage, "Parameter Error");
       pDX->Fail();
    }
    fclose(fp);
  }
}

