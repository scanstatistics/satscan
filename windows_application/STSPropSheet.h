#ifndef __STSPROPD_H
#define __STSPROPD_H

#include "res/resource.h"
#include "paramwin.h"
#include "PageAnalysis.h"
#include "PageInputFiles.h"
#include "PageScanningWindow.h"
#include "PageTimeParameters.h"
#include "PageOutput.h"

class CSTSPropSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CSTSPropSheet)

  public:
	CSTSPropSheet(CWindowsParameters* Params, CWnd* pWndParent);
   virtual ~CSTSPropSheet();

// Attributes

  CAnalysisPage       m_AnalysisPage;
  CInputFilesPage     m_InputFilesPage;
  CScanningWindowPage m_ScanningWindowPage;
  CTimeParametersPage m_TimeParametersPage;
  COutputPage          m_OutputPage;

// Operations

// Overrides
 	virtual BOOL OnInitDialog();

// Message Handlers
protected:
	//{{AFX_MSG(CSTSPropSheet)
//	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif
