//PageScanningWindow.h

#ifndef __PAGESCANNINGWINDOW_H
#define __PAGESCANNINGWINDOW_H

#include "res/resource.h"
#include "paramwin.h"

class CScanningWindowPage : public CPropertyPage
{
  public:
    CScanningWindowPage(CWindowsParameters* Params);
    ~CScanningWindowPage() {};
    
    CWindowsParameters* m_pParameters;
    //{{AFX_DATA(CScanningWindowPage)
    enum { IDD = IDD_SCANNINGWINDOW_PAGE };
    //}}AFX_DATA

  protected:
    
    static const DWORD m_nHelpIDs[];
    virtual const DWORD* GetHelpIDs() {return m_nHelpIDs;}

    //{{AFX_VIRTUAL(CScanningWindowPage)
    virtual void DoDataExchange(CDataExchange* pDX);
    //}}AFX_VIRTUAL

    void EnableScanningWindow();

    //{{AFX_MSG(CScanningWindowPage)
    LONG OnHelp(UINT, LONG lParam);
    LONG OnHelpContextMenu(UINT wParam, LONG);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
      
};

void AFXAPI DDV_MaxGeoSize(CDataExchange* pDX, int nIDC, float& nValue);
void AFXAPI DDV_MaxTimeSize(CDataExchange* pDX, int nIDC, float& nValue);

#endif
