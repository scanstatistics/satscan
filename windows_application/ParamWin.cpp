//ParamWin.cpp

#include "paramwin.h"
//#include "analysis.h"
#include "date.h"
#include <io.h>

CWindowsParameters* pStuff;

CWindowsParameters::CWindowsParameters(bool bDisplayErrors)
                   :CParameters(bDisplayErrors)
{
  pStuff = this;

  bool bValidParams;

  char szBuffer[255];
  GetCurrentDirectory(255, szBuffer);
  m_cszParamPath = CString(szBuffer) + "//" + PARAMFILENAME;

  bValidParams = SetParameters(m_cszParamPath);

  if (!bValidParams)
    SetDefaults();
}

CWindowsParameters::~CWindowsParameters()
{      
}                    

void CWindowsParameters::AssignFilenames()
{

  if (!(strlen(m_szCaseFilename)==0 || strlen(m_szCoordFilename)==0 ||
        strlen(m_szOutputFilename)==0))
  {
    CFile CaseFile(m_szCaseFilename, CFile::modeRead);
    m_cszCaseFilename = CaseFile.GetFilePath();
    CaseFile.Close();

    if (m_nModel == BERNOULLI && strlen(m_szControlFilename) != 0)
    {
      CFile ControlFile(m_szControlFilename, CFile::modeRead);
      m_cszControlFilename = ControlFile.GetFilePath();
      ControlFile.Close();
    }
    else if (m_nModel == POISSON && strlen(m_szPopFilename) != 0)
    {
      CFile PopFile(m_szPopFilename, CFile::modeRead);
      m_cszPopFilename = PopFile.GetFilePath();
      PopFile.Close();
    }

    CFile CoordFile(m_szCoordFilename, CFile::modeRead);
    m_cszCoordFilename = CoordFile.GetFilePath();
    CoordFile.Close();

    CFile OutputFile(m_szOutputFilename, CFile::modeCreate | CFile::modeNoTruncate);
    m_cszOutputFilename = OutputFile.GetFilePath();
    OutputFile.Close();

    if (m_bSpecialGridFile && strlen(m_szGridFilename) != 0)
    {
      CFile GridFile(m_szGridFilename, CFile::modeRead);
      m_cszGridFilename = GridFile.GetFilePath();
      GridFile.Close();
    }
  }
}

void CWindowsParameters::TransToDialog()
{
  CString cszDate;
  CString cszYear;
  CString cszMonth;
  CString cszDay;

  // Parse Date Strings into Y, M, D
  cszDate  = CString(m_szStartDate);
  cszYear  = cszDate.SpanExcluding("/");
  cszDate  = cszDate.Right(cszDate.GetLength()-cszYear.GetLength()-1);
  cszMonth = cszDate.SpanExcluding("/");
  cszDay   = cszDate.Right(cszDate.GetLength()-cszMonth.GetLength()-1);

  m_nStartYear  = atoi(cszYear);
  m_nStartMonth = atoi(cszMonth);
  m_nStartDay   = atoi(cszDay);

  cszDate  = CString(m_szEndDate);
  cszYear  = cszDate.SpanExcluding("/");
  cszDate  = cszDate.Right(cszDate.GetLength()-cszYear.GetLength()-1);
  cszMonth = cszDate.SpanExcluding("/");
  cszDay   = cszDate.Right(cszDate.GetLength()-cszMonth.GetLength()-1);

  m_nEndYear  = atoi(cszYear);
  m_nEndMonth = atoi(cszMonth);
  m_nEndDay   = atoi(cszDay);

  // Get full path names
  AssignFilenames();

  // Adjust Parameters to use with dialog controls
  if (m_nAnalysisType==PURELYSPATIAL)
  {
    m_nIntervalUnits=YEAR;
    m_nIntervalLength=1;
  }

  m_nAnalysisType--;
  m_nAreas--;
  m_nIntervalUnits--;

  m_bAliveClustersOnly_Win     = (BOOL)m_bAliveClustersOnly;      
  m_bIncludePurelySpatial_Win  = (BOOL)m_bIncludePurelySpatial;
  m_bIncludePurelyTemporal_Win = (BOOL)m_bIncludePurelyTemporal;
  m_bOutputRelRisks_Win        = (BOOL)m_bOutputRelRisks;
  m_bSaveSimLogLikelihoods_Win = (BOOL)m_bSaveSimLogLikelihoods;

}

void CWindowsParameters::TransFromDialog()
{
  sprintf(m_szStartDate, "%i/%i/%i", m_nStartYear, m_nStartMonth, m_nStartDay);
  sprintf(m_szEndDate, "%i/%i/%i", m_nEndYear, m_nEndMonth, m_nEndDay);

  strncpy(m_szCaseFilename,    m_cszCaseFilename,    MAX_STR_LEN);
  strncpy(m_szControlFilename, m_cszControlFilename, MAX_STR_LEN);
  strncpy(m_szPopFilename,     m_cszPopFilename,     MAX_STR_LEN);
  strncpy(m_szCoordFilename,   m_cszCoordFilename,   MAX_STR_LEN);
  strncpy(m_szGridFilename,    m_cszGridFilename,    MAX_STR_LEN);
  strncpy(m_szOutputFilename,  m_cszOutputFilename,  MAX_STR_LEN);

  // Adjust Parameters from use with dialog controls
  m_nAnalysisType++;
  m_nAreas++;
  m_nIntervalUnits++;

  m_bAliveClustersOnly     = m_bAliveClustersOnly_Win ? true : false;      
  m_bIncludePurelySpatial  = m_bIncludePurelySpatial_Win ? true : false;
  m_bIncludePurelyTemporal = m_bIncludePurelyTemporal_Win ? true : false;
  m_bOutputRelRisks        = m_bOutputRelRisks_Win ? true : false;
  m_bSaveSimLogLikelihoods = m_bSaveSimLogLikelihoods_Win ? true : false;

  if (m_nAnalysisType==PURELYSPATIAL)
  {
    m_nIntervalUnits=NONE;
    m_nIntervalLength=0;
  }

}

CWindowsParameters& CWindowsParameters::operator=(const CWindowsParameters& p)
{
  m_nAnalysisType     = p.m_nAnalysisType;
  m_nAreas            = p.m_nAreas;
  m_nModel            = p.m_nModel;
  m_nRiskFunctionType = p.m_nRiskFunctionType;
  m_nReplicas         = p.m_nReplicas;

  m_bPowerCalc = p.m_bPowerCalc;
  m_nPower_X   = p.m_nPower_X;
  m_nPower_Y   = p.m_nPower_Y;

  strcpy(m_szStartDate, p.m_szStartDate);
  strcpy(m_szEndDate, p.m_szEndDate);

  m_nMaxGeographicClusterSize = p.m_nMaxGeographicClusterSize;

  m_nMaxTemporalClusterSize = p.m_nMaxTemporalClusterSize;
  m_bAliveClustersOnly      = p.m_bAliveClustersOnly;

  m_nIntervalUnits  = p.m_nIntervalUnits;
  m_nIntervalLength = p.m_nIntervalLength;

  m_nTimeAdjustType = p.m_nTimeAdjustType;
  m_nTimeAdjPercent = p.m_nTimeAdjPercent;

  m_bIncludePurelySpatial  = p.m_bIncludePurelySpatial;
  m_bIncludePurelyTemporal = p.m_bIncludePurelyTemporal;

  strcpy(m_szCaseFilename, p.m_szCaseFilename);
  strcpy(m_szControlFilename, p.m_szControlFilename);
  strcpy(m_szPopFilename, p.m_szPopFilename);
  strcpy(m_szCoordFilename, p.m_szCoordFilename);
  strcpy(m_szGridFilename, p.m_szGridFilename);

  m_bSpecialGridFile = p.m_bSpecialGridFile;

  m_nPrecision = p.m_nPrecision;
  m_nDimension = p.m_nDimension;
   
  m_nCoordType = p.m_nCoordType;

  strcpy(m_szOutputFilename, p.m_szOutputFilename);
  strcpy(m_szGISFilename, p.m_szGISFilename);
  m_bSaveSimLogLikelihoods = p.m_bSaveSimLogLikelihoods;
  m_bOutputRelRisks        = p.m_bOutputRelRisks;

  m_bSequential    = p.m_bSequential;
  m_nAnalysisTimes = p.m_nAnalysisTimes;
  m_nCutOffPVal    = p.m_nCutOffPVal;

  m_bExactTimes  = p.m_bExactTimes;
  m_nClusterType = p.m_nClusterType;

  m_bValidatePriorToCalc   = p.m_bValidatePriorToCalc;

  m_nExtraParam1 = p.m_nExtraParam1;
  m_nExtraParam2 = p.m_nExtraParam2;
  m_nExtraParam3 = p.m_nExtraParam3;
  m_nExtraParam4 = p.m_nExtraParam4;

  m_bDisplayErrors = p.m_bDisplayErrors;

  m_cszParamPath       = p.m_cszParamPath;
  m_cszCaseFilename    = p.m_cszCaseFilename;
  m_cszControlFilename = p.m_cszControlFilename;
  m_cszPopFilename     = p.m_cszPopFilename;
  m_cszCoordFilename   = p.m_cszCoordFilename;
  m_cszGridFilename    = p.m_cszGridFilename;
  m_cszOutputFilename  = p.m_cszOutputFilename;

  m_nStartYear  = p.m_nStartYear;
  m_nStartMonth = p.m_nStartMonth;
  m_nStartDay   = p.m_nStartDay;
  m_nEndYear    = p.m_nEndYear;
  m_nEndMonth   = p.m_nEndMonth;
  m_nEndDay     = p.m_nEndDay;

  m_nIntervals  = p.m_nIntervals; // ???
  return *this;
}

void CWindowsParameters::SaveParameters()
{
  CParameters::SaveParameters(m_cszParamPath.GetBuffer(0));
  m_cszParamPath.ReleaseBuffer();
}

