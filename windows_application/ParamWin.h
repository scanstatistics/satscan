//ParamWin.h

#ifndef _PARAMWIN_H
#define _PARAMWIN_H

#include "stdafx.h"
#include "Param.h"
#include "basic.h"
#include <windows.h>

#define WINPARAMFILENAME "STSWIN.TXT"

enum { STSVALUE, STSDIALOG };

class CWindowsParameters : public CParameters
{
  public:
    CWindowsParameters(bool bDisplayErrors);    
    virtual ~CWindowsParameters();

    CString m_cszParamPath;      // Full Path name of Parameter File.
    CString m_cszCaseFilename;   // Name of file w/case data.           
    CString m_cszControlFilename;// Name of file w/control data.           
    CString m_cszPopFilename;    // Name of file w/pop data.            
    CString m_cszCoordFilename;  // Name of file w/coordinates tracts.  
    CString m_cszGridFilename;   // Name of file w/grid data.           
    CString m_cszOutputFilename; // Name of file w/results of SaTScan.  

    int  m_nStartYear;
    int  m_nStartMonth;
    int  m_nStartDay;
    int  m_nEndYear;
    int  m_nEndMonth;
    int  m_nEndDay;

    // Define w/type BOOL for use with Windows dialogs
    BOOL m_bAliveClustersOnly_Win;      
    BOOL m_bIncludePurelySpatial_Win;
    BOOL m_bIncludePurelyTemporal_Win;
    BOOL m_bOutputRelRisks_Win;
    BOOL m_bSaveSimLogLikelihoods_Win;

    int  m_nIntervals;  // ???

    void AssignFilenames();
    void TransToDialog();
    void TransFromDialog();
    void SaveParameters();

    CWindowsParameters& operator=(const CWindowsParameters& p);
};

#endif