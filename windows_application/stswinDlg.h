#ifndef __STSWINDLG_H
#define __STSWINDLG_H

#include "res/resource.h"
#include "ParamWin.h"
#include "STSPropSheet.h"


class CSTSWinDlg : public CDialog
{
  // Construction
  public:
    CSTSWinDlg(CWnd* pParent = NULL);

  // Dialog Data
    //{{AFX_DATA(CSTSWinDlg)
    enum { IDD = IDD_STSWIN_DIALOG };
    //}}AFX_DATA

    CWindowsParameters   m_Parameters;
    CSTSPropSheet m_PropSheet;

    // Virtual function overrides
    //{{AFX_VIRTUAL(CSTSWinDlg)

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual void OnOk();
    virtual void OnHelp();
    virtual void OnHelpFinder();
    virtual void OnCancel();
    //}}AFX_VIRTUAL

  // Implementation
  protected:
    HICON m_hIcon;
    HICON m_hSmallIcon;

    // Message map functions
    //{{AFX_MSG(CSTSWinDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnDestroy();
    //    afx_msg LONG OnContextHelp(WPARAM wParam, LPARAM lParam);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

#endif

