// PageInputFiles.cpp

#include "stdafx.h"
#include "PageInputFiles.h"
#include "resource.hm"

CInputFilesPage::CInputFilesPage(CWindowsParameters* Params)
                :CPropertyPage(CInputFilesPage::IDD)
{
  //{{AFX_DATA_INIT(CInputFilesPage)
  //}}AFX_DATA_INIT

  m_pParameters = Params;
}

void CInputFilesPage::EnableFiles()
{
/*  if (m_pParameters->m_nModel == POISSON)
  {
    GetDlgItem(IDC_CONTROLFILE)->EnableWindow(FALSE);
    GetDlgItem(IDC_POPFILE)->EnableWindow(TRUE);
  }
  else if (m_pParameters->m_nModel == BERNOULLI)
  {
    GetDlgItem(IDC_CONTROLFILE)->EnableWindow(TRUE);
    GetDlgItem(IDC_POPFILE)->EnableWindow(FALSE);
  } */
}

void CInputFilesPage::EnablePrecision()
{
  if (m_pParameters->m_nAnalysisType == 0)  // Purely Spatial
  {
    GetDlgItem(IDC_PRCD_NONE)->EnableWindow(TRUE);
    GetDlgItem(IDC_PRCD_YR)->EnableWindow(TRUE);
    GetDlgItem(IDC_PRCD_MO)->EnableWindow(TRUE);
  }
  else                                   // Space-Time, Purely Temporal
  {
    GetDlgItem(IDC_PRCD_NONE)->EnableWindow(FALSE);
    GetDlgItem(IDC_PRCD_YR)->EnableWindow(m_pParameters->m_nIntervalUnits<1);
    GetDlgItem(IDC_PRCD_MO)->EnableWindow(m_pParameters->m_nIntervalUnits<2);
  }
}

bool CInputFilesPage::ValidateInputFiles()
{
  if (!(ValidateFileExists(IDC_CASEFILE,    m_pParameters->m_cszCaseFilename, "Case")))
    return false;

  if (m_pParameters->m_nModel == POISSON)
  {
    if (!(ValidateFileExists(IDC_POPFILE,     m_pParameters->m_cszPopFilename, "Population")))
      return false;
  }
  else if (m_pParameters->m_nModel == BERNOULLI)
  {
    if (!(ValidateFileExists(IDC_CONTROLFILE, m_pParameters->m_cszControlFilename, "Control")))
      return false;
  }

  if (!(ValidateFileExists(IDC_COORDFILE,   m_pParameters->m_cszCoordFilename, "Coordinates")))
    return false;

  if (m_pParameters->m_cszGridFilename.GetLength() > 0)
    if (!(ValidateFileExists(IDC_SPGRIDFILE,  m_pParameters->m_cszGridFilename, "Special Grid")))
      return false;

  return true;
}

bool CInputFilesPage::ValidateFileExists(int nIDC, CString szFilename, char* szDescription)
{
  FILE* fp;
  bool  bValid = true;
  char  szMessage[400];
  char  szTitle[40];

  strcpy(szTitle, szDescription);
  strcat(szTitle, " File Error");

  if (GetDlgItem(nIDC)->IsWindowEnabled())
  {
    if (szFilename.GetLength()==0 || (fp = fopen(szFilename, "r")) == NULL)  
    {
       sprintf(szMessage, "File %s could not be opened.", szFilename.GetBuffer(0));
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

void CInputFilesPage::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  
  //{{AFX_DATA_MAP(CInputFilesPage)
  DDX_Text (pDX, IDC_CASEFILE,     m_pParameters->m_cszCaseFilename);
  DDX_Text (pDX, IDC_CONTROLFILE,  m_pParameters->m_cszControlFilename);
  DDX_Text (pDX, IDC_POPFILE,      m_pParameters->m_cszPopFilename);
  DDX_Text (pDX, IDC_COORDFILE,    m_pParameters->m_cszCoordFilename);
  DDX_Text (pDX, IDC_SPGRIDFILE,   m_pParameters->m_cszGridFilename);

  DDX_Radio(pDX, IDC_PRCD_NONE,    m_pParameters->m_nPrecision);
  DDX_Radio(pDX, IDC_CARTESIAN,    m_pParameters->m_nCoordType);

 // DDV_FileExists(pDX, IDC_CASEFILE,    m_pParameters->m_cszCaseFilename);
 // DDV_FileExists(pDX, IDC_CONTROLFILE, m_pParameters->m_cszControlFilename);
 // DDV_FileExists(pDX, IDC_POPFILE,     m_pParameters->m_cszPopFilename);
 // DDV_FileExists(pDX, IDC_COORDFILE,   m_pParameters->m_cszCoordFilename);
  //}}AFX_DATA_MAP

//  if (strlen(m_pParameters->m_cszGridFilename) != 0)
//    DDV_FileExists(pDX, IDC_SPGRIDFILE,  m_pParameters->m_cszGridFilename);

  if (!(pDX->m_bSaveAndValidate))
  {
    EnableFiles();
    EnablePrecision();
  }
}

void CInputFilesPage::OnPrecisionChange()
{
  //Ensure that time interval units does not conflict with percision
  if (m_pParameters->m_nAnalysisType == 0 && m_pParameters->m_nPrecision != 0)  // Purely Spatial
  {
    UpdateData(TRUE);
    if (m_pParameters->m_nIntervalUnits > m_pParameters->m_nPrecision-1)
      m_pParameters->m_nIntervalUnits = m_pParameters->m_nPrecision-1;
  }
}

void CInputFilesPage::OnBrowseCase()
{
  CString cszFilename;
  CString cszDlgTitle = CString("Select Case File");
  CFileDialog dlg(TRUE,_T("CAS"),_T("*.CAS"),OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                  _T("Case Files (*.CAS)|*.CAS|All Files (*.*)|*.*||"),this);

  dlg.m_ofn.lpstrTitle=cszDlgTitle.GetBuffer(0);
  cszDlgTitle.ReleaseBuffer();

  if (dlg.DoModal()==IDOK)
  {
    m_pParameters->m_cszCaseFilename = dlg.GetPathName();
    UpdateData(FALSE);
  }

}

void CInputFilesPage::OnBrowseControl()
{
  CString cszFilename;
  CString cszDlgTitle = CString("Select Control File");
  CFileDialog dlg(TRUE,_T("CTL"),_T("*.CTL"),OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                  _T("Control Files (*.CTL)|*.CTL|All Files (*.*)|*.*||"),this);

  dlg.m_ofn.lpstrTitle=cszDlgTitle.GetBuffer(0);
  cszDlgTitle.ReleaseBuffer();

  if (dlg.DoModal()==IDOK)
  {
    m_pParameters->m_cszControlFilename = dlg.GetPathName();
    UpdateData(FALSE);
  }

}

void CInputFilesPage::OnBrowsePop()
{
  CString cszFilename;
  CString cszDlgTitle = CString("Select Population File");
  CFileDialog dlg(TRUE,_T("POP"),_T("*.POP"),OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                  _T("Population Files (*.POP)|*.POP|All Files (*.*)|*.*||"),this);

  dlg.m_ofn.lpstrTitle=cszDlgTitle.GetBuffer(0);
  cszDlgTitle.ReleaseBuffer();

  if (dlg.DoModal()==IDOK)
  {
    m_pParameters->m_cszPopFilename = dlg.GetPathName();
    UpdateData(FALSE);
  }

}

void CInputFilesPage::OnBrowseCoord()
{
  CString cszFilename;
  CString cszDlgTitle = CString("Select Coordinates File");
  CFileDialog dlg(TRUE,_T("GEO"),_T("*.GEO"),OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                  _T("Coordinates Files (*.GEO)|*.GEO|All Files (*.*)|*.*||"),this);

  dlg.m_ofn.lpstrTitle=cszDlgTitle.GetBuffer(0);
  cszDlgTitle.ReleaseBuffer();

  if (dlg.DoModal()==IDOK)
  {
    m_pParameters->m_cszCoordFilename = dlg.GetPathName();
    UpdateData(FALSE);
  }

}

void CInputFilesPage::OnBrowseSpGrid()
{
  CString cszFilename;
  CString cszDlgTitle = CString("Select Special Grid File");
  CFileDialog dlg(TRUE,_T("GRD"),_T("*.GRD"),OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                  _T("Special Grid Files (*.GRD)|*.GRD|All Files (*.*)|*.*||"),this);

  dlg.m_ofn.lpstrTitle=cszDlgTitle.GetBuffer(0);
  cszDlgTitle.ReleaseBuffer();

  if (dlg.DoModal()==IDOK)
  {
    m_pParameters->m_cszGridFilename = dlg.GetPathName();
    UpdateData(FALSE);
  }

}

LONG CInputFilesPage::OnHelp(UINT, LONG lParam)
{
  ::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
            AfxGetApp()->m_pszHelpFilePath,
            HELP_WM_HELP,
            (DWORD)(LPVOID)GetHelpIDs());
  return 0;
}

const DWORD CInputFilesPage::m_nHelpIDs[] =
{
  IDC_CASEFILE_TEXT, IDH_CASEFILE,
  IDC_CASEFILE, IDH_CASEFILE,
  IDC_BRWS_CASE, IDH_CASEFILE,
  IDC_CONTROLFILE_TEXT, IDH_CONTROLFILE,
  IDC_CONTROLFILE_TEXT2, IDH_CONTROLFILE,
  IDC_CONTROLFILE, IDH_CONTROLFILE,
  IDC_BRWS_CONTROL, IDH_CONTROLFILE,
  IDC_POPFILE_TEXT, IDH_POPFILE,
  IDC_POPFILE_TEXT2, IDH_POPFILE,
  IDC_POPFILE, IDH_POPFILE,
  IDC_BRWS_POP, IDH_POPFILE,
  IDC_COORDFILE_TEXT, IDH_COORDFILE,
  IDC_COORDFILE, IDH_COORDFILE,
  IDC_BRWS_COORD, IDH_COORDFILE,
  IDC_SPGRIDFILE_TEXT, IDH_SPGRIDFILE,
  IDC_SPGRIDFILE_TEXT2, IDH_SPGRIDFILE,
  IDC_SPGRIDFILE, IDH_SPGRIDFILE,
  IDC_BRWS_SPGRID, IDH_SPGRIDFILE,

  IDC_PRECISION_GB, IDH_PRECISION,
  IDC_PRCD_NONE, IDH_PRECISION,
  IDC_PRCD_YR, IDH_PRECISION,
  IDC_PRCD_MO, IDH_PRECISION,
  IDC_PRCD_DY, IDH_PRECISION,

  IDC_COORDINATES_GB, IDH_COORDINATETYPE,
  IDC_CARTESIAN, IDH_COORDINATETYPE,
  IDC_LAT_LON, IDH_COORDINATETYPE

};

BEGIN_MESSAGE_MAP(CInputFilesPage, CPropertyPage)
  //{{AFX_MSG_MAP(CInputFilesPage)
  ON_BN_CLICKED(IDC_BRWS_CASE,    OnBrowseCase)
  ON_BN_CLICKED(IDC_BRWS_CONTROL, OnBrowseControl)
  ON_BN_CLICKED(IDC_BRWS_POP,     OnBrowsePop)
  ON_BN_CLICKED(IDC_BRWS_COORD,   OnBrowseCoord)
  ON_BN_CLICKED(IDC_BRWS_SPGRID,  OnBrowseSpGrid)
  ON_BN_CLICKED(IDC_PRCD_YR,      OnPrecisionChange)
  ON_BN_CLICKED(IDC_PRCD_MO,      OnPrecisionChange)
  ON_BN_CLICKED(IDC_PRCD_DY,      OnPrecisionChange)
  ON_MESSAGE(WM_HELP, OnHelp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()
      
void AFXAPI DDV_FileExists(CDataExchange* pDX, int nIDC, CString szFilename)
{
  /*HWND  hWndCtrl = */pDX->PrepareEditCtrl(nIDC);
  char szMessage[400];
  FILE* fp;

  if (pDX->m_bSaveAndValidate && 
      pDX->m_pDlgWnd->GetDlgItem(nIDC)->IsWindowEnabled())
  {
    if ((fp = fopen(szFilename, "r")) == NULL)  // Check for blank filename also
    {
       sprintf(szMessage, "File %s could not be opened.", szFilename.GetBuffer(0));
       szFilename.ReleaseBuffer();
       pDX->m_pDlgWnd->MessageBox(szMessage, "Parameter Error");
       pDX->Fail();
    }
    fclose(fp);
  }
}

