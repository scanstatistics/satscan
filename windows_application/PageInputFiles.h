#ifndef __PAGEINPUTFILES_H
#define __PAGEINPUTFILES_H

#include "res/resource.h"
#include "paramwin.h"

class CInputFilesPage : public CPropertyPage
{
  public:
    CInputFilesPage(CWindowsParameters* Params);
    ~CInputFilesPage() {};
    
   CWindowsParameters* m_pParameters;

    //{{AFX_DATA(CInputFilesPage)
    enum { IDD = IDD_INPUTFILES_PAGE };
    //}}AFX_DATA

    bool ValidateInputFiles();

  protected:
    
    static const DWORD m_nHelpIDs[];
    virtual const DWORD* GetHelpIDs() {return m_nHelpIDs;}

    //{{AFX_VIRTUAL(CInputFilesPage)
    virtual void DoDataExchange(CDataExchange* pDX);
    //}}AFX_VIRTUAL

    void OnBrowseCase();
    void OnBrowseControl();
    void OnBrowsePop();
    void OnBrowseCoord();
    void OnBrowseSpGrid();
    void EnableFiles();
    void EnablePrecision();
    void OnPrecisionChange();

    bool ValidateFileExists(int nIDC, CString szFilename, char* szDescription);

    //{{AFX_MSG(CInputFilesPage)
    LONG OnHelp(UINT, LONG lParam);
    LONG OnHelpContextMenu(UINT wParam, LONG);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
      
};

void AFXAPI DDV_FileExists(CDataExchange* pDX, int nIDC, CString szFilename);

#endif
