// PageOutput.h

#ifndef __PAGEOUTPUT_H
#define __PAGEOUTPUT_H

#include "res/resource.h"
#include "paramwin.h"

class COutputPage : public CPropertyPage
{
  public:
    COutputPage(CWindowsParameters* Params);
    ~COutputPage() {};

    CWindowsParameters* m_pParameters;
    
    //{{AFX_DATA(COutputPage)
    enum { IDD = IDD_OUTPUT_PAGE };
    //}}AFX_DATA

    bool ValidateOutputFiles();

  protected:
    
    static const DWORD m_nHelpIDs[];
    virtual const DWORD* GetHelpIDs() {return m_nHelpIDs;}

    //{{AFX_VIRTUAL(COutputPage)
    virtual void DoDataExchange(CDataExchange* pDX);
    //}}AFX_VIRTUAL

    void OnBrowseResults();
    bool ValidateFileCanCreate(int nIDC, CString szFilename, char* szDescription);

    //{{AFX_MSG(COutputPage)
    LONG OnHelp(UINT, LONG lParam);
    LONG OnHelpContextMenu(UINT wParam, LONG);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
      
};

void AFXAPI DDV_FileCanCreate(CDataExchange* pDX, int nIDC, CString szFilename);

#endif
