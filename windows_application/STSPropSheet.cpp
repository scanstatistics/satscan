#include "stdafx.h"
#include "STSPropSheet.h"
#include "date.h"
#include "resource.hm"

IMPLEMENT_DYNAMIC(CSTSPropSheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CSTSPropSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CModalShapePropSheet)
  	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSTSPropSheet::CSTSPropSheet(CWindowsParameters* Params, CWnd* pWndParent)
              :CPropertySheet(AFX_IDS_APP_TITLE, pWndParent),
               m_InputFilesPage(Params),
               m_AnalysisPage(Params),
               m_ScanningWindowPage(Params),
               m_TimeParametersPage(Params),
               m_OutputPage(Params)
{
  AddPage(&m_InputFilesPage);
	AddPage(&m_AnalysisPage);
	AddPage(&m_ScanningWindowPage);
	AddPage(&m_TimeParametersPage);
	AddPage(&m_OutputPage);
}

CSTSPropSheet::~CSTSPropSheet()
{
  CWnd::DestroyWindow();
}

BOOL CSTSPropSheet::OnInitDialog()
{
  CPropertySheet::OnInitDialog();

  ModifyStyle(0, DS_CONTROL, 0);
  ModifyStyleEx(0, WS_EX_CONTROLPARENT, 0);
  //KR-10/29/97MoveWindow(5, 5, 400, 260, TRUE);

  return TRUE;
}

