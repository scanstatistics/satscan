#include "stdafx.h"
#include "stswindlg.h"
#include "resource.hm"

CSTSWinDlg::CSTSWinDlg(CWnd* pParent)
           :CDialog(CSTSWinDlg::IDD, pParent),
            m_Parameters(false),
            m_PropSheet(&m_Parameters, this)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  m_hSmallIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAMESM);
}

CSTSWinDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

//ModifyStyleEx(0, WS_EX_CONTEXTHELP | WS_MINIMIZEBOX);
//  ModifyStyle(0, WS_MINIMIZEBOX);

  CMenu* pSysMenu = GetSystemMenu(FALSE);
  CString strAboutMenu;
  strAboutMenu.LoadString(IDD_ABOUTBOX);
  if (!strAboutMenu.IsEmpty())
  {
    pSysMenu->AppendMenu(MF_SEPARATOR);
    pSysMenu->AppendMenu(MF_STRING, ID_APP_ABOUT/*KR-8/14/97IDD_ABOUTBOX*/, strAboutMenu);
  }

  SetIcon(m_hIcon, TRUE);
  SetIcon(m_hSmallIcon, FALSE);


  m_Parameters.TransToDialog();
  m_PropSheet.Create(this, WS_CHILD|WS_VISIBLE);

//  SetWindowLong(m_hWnd, GWL_EXSTYLE,
//                ::GetWindowLong(m_hWnd, GWL_EXSTYLE) |
//                WS_EX_CONTEXTHELP);

//  if (m_Parameters.m_bStandardDefaults)
//    MessageBox("Invalid value found in default parameter file STSPARAM.TXT.  Standard defaults will be used.", "Parameter Error");

  return TRUE;
}

void CSTSWinDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSTSWinDlg, CDialog)
  //{{AFX_MSG_MAP(CSTSWinDlg)
  ON_WM_SYSCOMMAND()
  ON_COMMAND(IDOK, OnOk)
  ON_COMMAND(IDCANCEL, OnCancel)
  ON_COMMAND(ID_HELP, OnHelpFinder)
  ON_MESSAGE(WM_HELP, OnHelp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSTSWinDlg::OnHelpFinder()
{
  WinHelp(ID_HELP, HELP_FINDER);
}

void CSTSWinDlg::OnHelp()
{
}

void CSTSWinDlg::OnOk()
{
  int nStartPage = m_PropSheet.GetActiveIndex();
  int nMaxIndex  = m_PropSheet.GetPageCount()-1;
  int nPageIndex = nStartPage;
  int nPageHits = 0;
  BOOL bDone = FALSE;
  BOOL bSetPageSuccess;

  while (!bDone)
  {
    nPageIndex++;
    nPageHits++;
    if (nPageIndex > nMaxIndex)
      nPageIndex = 0;
    bSetPageSuccess = m_PropSheet.SetActivePage(nPageIndex);
    if (!bSetPageSuccess)
      return;
    if (nPageHits == nMaxIndex+1)
      bDone = TRUE;
  }
  
  // Validate Files on Input Tab
  m_PropSheet.SetActivePage(0);
  if (!( ((CInputFilesPage*)(m_PropSheet.GetPage(0)))->ValidateInputFiles() )) // Input Files
    return;

  // Validate Files on Output Tab
  m_PropSheet.SetActivePage(nMaxIndex);
  if (!( ((COutputPage*)(m_PropSheet.GetPage(nMaxIndex)))->ValidateOutputFiles() )) // Output Files
    return;


  m_Parameters.TransFromDialog();
  m_Parameters.SaveParameters();

  AfxGetApp()->WinHelp(0L, HELP_QUIT); 
  CDialog::OnOK();
}

void CSTSWinDlg::OnCancel()
{
//  AfxGetApp()->WinHelp(0L, HELP_QUIT);
  BOOL bRet = ::WinHelp((HWND)AfxGetMainWnd(), "stswin.hlp", HELP_QUIT, 0L);
/*  if (bRet)
    MessageBox("Help Closed");
  else
    MessageBox("Help Not Closed");*/

  CDialog::OnCancel();
}

void CSTSWinDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  if ((nID & 0xFFF0) == ID_APP_ABOUT/*KR-8/14/97IDD_ABOUTBOX*/)
    CDialog(IDD_ABOUTBOX).DoModal();
  else
    CDialog::OnSysCommand(nID, lParam);
}

/*void CSTSWinDlg::OnDestroy()
{
	WinHelp(0L, HELP_QUIT);
	CDialog::OnDestroy();
}
*/





