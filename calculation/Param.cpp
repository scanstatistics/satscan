// Param.cpp

#include "Param.h"
#include "date.h"
//#include "display.h"
#include "error.h"
#include <limits.h>
#include <float.h>

CParameters::CParameters(bool bDisplayErrors)
{
  m_bDisplayErrors = bDisplayErrors;
}

bool CParameters::SetParameters(const char* szFilename)
{
  FILE* pFile;
  char  szTemp [MAX_STR_LEN];
  bool  bValid = true;

  printf("Reading Parameters.\n");

  if ((pFile = fopen(szFilename, "r")) == NULL)
  {
    printf("  Error: Unable to open parameter file.\n");
    return false;
  }

  int  i    = 1;
  bool bEOF = false;

  while (i<=PARAMETERS && !bEOF)
  {
    if (fgets(szTemp, MAX_STR_LEN, pFile) == NULL)
      bEOF   = true;
    else if (!SetParameter(i, szTemp))
      bValid = false;
    i++;
  }

  fclose(pFile);

  if (bEOF && i-1==MODEL)              // Accept V.1 parameter files
    SetDefaultsV2();

  if (!SetGISFilename())
    bValid = false;

  if (!SetLLRFilename())
    bValid = false;

  if (bValid)
    bValid = ValidateParameters();

  return bValid;
}

bool CParameters::SetParameter(int nParam, const char* szParam)
{
  bool bValid;
  int  nScanCount;
  int  nTemp;

  switch (nParam)
  {
    case ANALYSISTYPE : nScanCount=sscanf(szParam, "%i", &m_nAnalysisType); break;
    case SCANAREAS    : nScanCount=sscanf(szParam, "%i", &m_nAreas); break;
    case CASEFILE     : strncpy(m_szCaseFilename, szParam, strlen(szParam)-1);
                        m_szCaseFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
    case POPFILE      : strncpy(m_szPopFilename, szParam, strlen(szParam)-1);
                        m_szPopFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
    case COORDFILE    : strncpy(m_szCoordFilename, szParam, strlen(szParam)-1);
                        m_szCoordFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
    case OUTPUTFILE   : strncpy(m_szOutputFilename, szParam, strlen(szParam)-1);
                        m_szOutputFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
    case PRECISION    : nScanCount=sscanf(szParam, "%i", &m_nPrecision); break;
    case DIMENSION    : nScanCount=sscanf(szParam, "%i", &m_nDimension); break;
    case SPECIALGRID  : nScanCount=sscanf(szParam, "%i", &nTemp); m_bSpecialGridFile = nTemp?true:false; break;
    case GRIDFILE     : strncpy(m_szGridFilename, szParam, strlen(szParam)-1);
                        m_szGridFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
    case GEOSIZE      : nScanCount=sscanf(szParam, "%f", &m_nMaxGeographicClusterSize); break;
    case STARTDATE    : nScanCount=sscanf(szParam, "%s", &m_szStartDate); break;
    case ENDDATE      : nScanCount=sscanf(szParam, "%s", &m_szEndDate); break;
    case CLUSTERS     : nScanCount=sscanf(szParam, "%i", &nTemp); m_bAliveClustersOnly = nTemp?true:false; break;
    case EXACTTIMES   : nScanCount=sscanf(szParam, "%i", &nTemp); m_bExactTimes = nTemp?true:false; break;
    case INTERVALUNITS: nScanCount=sscanf(szParam, "%i", &m_nIntervalUnits); break;
    case TIMEINTLEN   : nScanCount=sscanf(szParam, "%i", &m_nIntervalLength); break;
    case PURESPATIAL  : nScanCount=sscanf(szParam, "%i", &nTemp); m_bIncludePurelySpatial = nTemp?true:false; break;
    case TIMESIZE     : nScanCount=sscanf(szParam, "%f", &m_nMaxTemporalClusterSize); break;
    case REPLICAS     : nScanCount=sscanf(szParam, "%i", &m_nReplicas); break;
    case MODEL        : nScanCount=sscanf(szParam, "%i", &m_nModel); break;
    case RISKFUNCTION : nScanCount=sscanf(szParam, "%i", &m_nRiskFunctionType); break;
    case POWERCALC    : nScanCount=sscanf(szParam, "%i", &nTemp); m_bPowerCalc = nTemp?true:false; break;
    case POWERX       : nScanCount=sscanf(szParam, "%i", &m_nPower_X); break;
    case POWERY       : nScanCount=sscanf(szParam, "%i", &m_nPower_Y); break;
    case TIMETREND    : nScanCount=sscanf(szParam, "%i", &m_nTimeAdjustType); break;
    case TIMETRENDPERC: nScanCount=sscanf(szParam, "%lf", &m_nTimeAdjPercent); break;
    case PURETEMPORAL : nScanCount=sscanf(szParam, "%i", &nTemp); m_bIncludePurelyTemporal = nTemp?true:false; break;
    case CONTROLFILE  : strncpy(m_szControlFilename, szParam, strlen(szParam)-1);
                        m_szControlFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
    case COORDTYPE    : nScanCount=sscanf(szParam, "%i", &m_nCoordType); break;
    case SAVESIMLL    : nScanCount=sscanf(szParam, "%i", &nTemp); m_bSaveSimLogLikelihoods = nTemp?true:false; break;
    case SEQUENTIAL   : nScanCount=sscanf(szParam, "%i", &nTemp); m_bSequential = nTemp?true:false; break;
    case SEQNUM       : nScanCount=sscanf(szParam, "%i", &m_nAnalysisTimes); break;
    case SEQPVAL      : nScanCount=sscanf(szParam, "%lf", &m_nCutOffPVal); break;
    case VALIDATE     : nScanCount=sscanf(szParam, "%i", &nTemp); m_bValidatePriorToCalc = nTemp?true:false; break;
    case OUTPUTRR     : nScanCount=sscanf(szParam, "%i", &nTemp); m_bOutputRelRisks = nTemp?true:false; break;
    case EXTRA1       : nScanCount=sscanf(szParam, "%i", &m_nExtraParam1); break;
    case EXTRA2       : nScanCount=sscanf(szParam, "%i", &m_nExtraParam2); break;
    case EXTRA3       : nScanCount=sscanf(szParam, "%i", &m_nExtraParam3); break;
    case EXTRA4       : nScanCount=sscanf(szParam, "%i", &m_nExtraParam4); break;
  }

  if (nParam==POPFILE || nParam==GRIDFILE || nParam==CONTROLFILE)
    bValid = true;
  else
  {
    bValid = (nScanCount == 1);
    if (!bValid)
      DisplayParamError(nParam);
  }

  return bValid;
}

bool CParameters::SetGISFilename()
{
  int  nReportNameLen = strlen(m_szOutputFilename);
  int  nIndex         = nReportNameLen-1;
  bool bDone          = false;
  bool bExtFound      = false;

  while (!bDone && !bExtFound && nIndex>=0)
  {
    if (m_szOutputFilename[nIndex]=='/' || m_szOutputFilename[nIndex]=='\\')
      bDone=true;
    else if (m_szOutputFilename[nIndex]=='.')
      bExtFound=true;
    else
      nIndex--;
  }

  if (!bExtFound)
    nIndex = nReportNameLen;

  strncpy(m_szGISFilename, m_szOutputFilename, nIndex);
  strcpy(m_szGISFilename+nIndex, ".gis");

  if (strcmp(m_szGISFilename, m_szOutputFilename)==0)
  {
    fprintf(stderr,"  Error: Attempting to write report to GIS file.\n");
    return false;
  }
  // Add LLR error check
  else
    return true; 
}

bool CParameters::SetLLRFilename()
{
  int  nReportNameLen = strlen(m_szOutputFilename);
  int  nIndex         = nReportNameLen-1;
  bool bDone          = false;
  bool bExtFound      = false;

  while (!bDone && !bExtFound && nIndex>=0)
   {
     if (m_szOutputFilename[nIndex]=='/' || m_szOutputFilename[nIndex]=='\\')
       bDone=true;
     else if (m_szOutputFilename[nIndex]=='.')
       bExtFound=true;
     else
       nIndex--;
   }

  if (!bExtFound)
    nIndex = nReportNameLen;

  strncpy(m_szLLRFilename, m_szOutputFilename, nIndex);
  strcpy(m_szLLRFilename+nIndex, ".llr");
  

  if (strcmp(m_szLLRFilename, m_szOutputFilename)==0)
  {
    fprintf(stderr,"  Error: Attempting to write report to LLR file.\n");
    return false;
  }
  else
    return true; 
}

bool CParameters::ValidateParameters()
{
  bool  bValid = true;
  FILE* pFile;

  if (m_bValidatePriorToCalc)
  {

    if (!(PURELYSPATIAL <= m_nAnalysisType && m_nAnalysisType <= PURELYTEMPORAL))
      bValid = DisplayParamError(ANALYSISTYPE);

    if (!(HIGH <= m_nAreas && m_nAreas<= HIGHANDLOW))
      bValid = DisplayParamError(SCANAREAS);

    if (!(POISSON == m_nModel || m_nModel == BERNOULLI))
      bValid = DisplayParamError(MODEL);

    if (!(STANDARDRISK == m_nRiskFunctionType || m_nRiskFunctionType == MONOTONERISK))
      bValid = DisplayParamError(RISKFUNCTION);

    if (!(m_bSequential==0 || m_bSequential==1))
      bValid = DisplayParamError(SEQUENTIAL);

    if (m_bSequential)
    {
      if (!(1 <= m_nAnalysisTimes && m_nAnalysisTimes <= INT_MAX))
        bValid = DisplayParamError(SEQNUM);
      if (!(0.0 <= m_nCutOffPVal && m_nCutOffPVal <= 1.0))
        bValid = DisplayParamError(SEQPVAL);
    }
    else
    {
      m_nAnalysisTimes = 0;
      m_nCutOffPVal    = 0.0;
    }

    if (!(m_bPowerCalc == 0 || m_bPowerCalc == 1))
      bValid = DisplayParamError(POWERCALC);

    if (m_bPowerCalc)
    {
      if (!(0.0 <= m_nPower_X && m_nPower_X<= DBL_MAX))
        bValid = DisplayParamError(POWERX);
      if (!(0.0 <= m_nPower_Y && m_nPower_Y <= DBL_MAX))
        bValid = DisplayParamError(POWERY);
    }
    else
    {
      m_nPower_X = 0.0;
      m_nPower_Y = 0.0;
    }

    if (!(CARTESIAN == m_nCoordType || m_nCoordType == LATLON))
      bValid = DisplayParamError(COORDTYPE);

    if (!(m_bSaveSimLogLikelihoods==0 || m_bSaveSimLogLikelihoods==1))
      bValid = DisplayParamError(SAVESIMLL);

    if (!(m_bOutputRelRisks==0 || m_bOutputRelRisks==1))
      bValid = DisplayParamError(OUTPUTRR);

    if (!ValidateReplications(m_nReplicas))
      bValid = DisplayParamError(REPLICAS);

    if (m_nAnalysisType == PURELYSPATIAL)
    {
      if (!(NONE <= m_nPrecision && m_nPrecision <= DAY))
        bValid = DisplayParamError(PRECISION);
    }
    else
      if (!(YEAR <= m_nPrecision && m_nPrecision <= DAY))  // Change to DAYS, YEARS
        bValid = DisplayParamError(PRECISION);

    if (!ValidateDateString(m_szStartDate, STARTDATE))
      bValid = DisplayParamError(STARTDATE);
    else if (!ValidateDateString(m_szEndDate, ENDDATE))
      bValid = DisplayParamError(ENDDATE);
    else if (strcmp(m_szStartDate, m_szEndDate) > 0)
      bValid = DisplayParamError(ENDDATE);

    // Spatial Options
    if (m_nAnalysisType == PURELYSPATIAL || m_nAnalysisType == SPACETIME)
    {
      if (!(0.0 < m_nMaxGeographicClusterSize && m_nMaxGeographicClusterSize <= 50.0)) //GG980716
      bValid = DisplayParamError(GEOSIZE);
    }
    else
      m_nMaxGeographicClusterSize = 50.0; //KR980707 0 GG980716;

    // Temporal Options
    if (m_nAnalysisType == PURELYTEMPORAL || m_nAnalysisType == SPACETIME)
    {
      if (!(0.0 < m_nMaxTemporalClusterSize && m_nMaxTemporalClusterSize <= 90.0)) //GG980716
        bValid = DisplayParamError(TIMESIZE);
      if (!(m_bAliveClustersOnly==0 || m_bAliveClustersOnly==1))
        bValid = DisplayParamError(CLUSTERS);
      if (!(YEAR <= m_nIntervalUnits && m_nIntervalUnits <= m_nPrecision))
        bValid = DisplayParamError(INTERVALUNITS);
      if (!(1 <= m_nIntervalLength && m_nIntervalLength <= TimeBetween(CharToJulian(m_szStartDate), CharToJulian(m_szEndDate), m_nIntervalUnits) ))// Change to Max Interval
        bValid = DisplayParamError(TIMEINTLEN);

      if (m_nModel == BERNOULLI)
      {
        m_nTimeAdjustType == NOTADJUSTED;
        m_nTimeAdjPercent == 0.0;
      }
      else
      {
        if (!(NOTADJUSTED <= m_nTimeAdjustType && m_nTimeAdjustType <= LINEAR))
          bValid = DisplayParamError(TIMETREND);
        if (m_nTimeAdjustType == NONPARAMETRIC && m_nAnalysisType == PURELYTEMPORAL)
          bValid = DisplayParamError(TIMETREND);
        if (m_nTimeAdjustType == LINEAR)
          if (!(-100.0 < m_nTimeAdjPercent))
            bValid = DisplayParamError(TIMETRENDPERC);
      }
    }
    else
    {
      m_nMaxTemporalClusterSize = 50.0; // KR980707 0 GG980716;
      m_bAliveClustersOnly      = false;
      m_nIntervalUnits          = NONE;
      m_nIntervalLength         = 0;
      m_nTimeAdjustType         = NOTADJUSTED;
      m_nTimeAdjPercent         = 0;
    }

    // Space-Time Options
    if (m_nAnalysisType == SPACETIME)
    {
      if (!(m_bIncludePurelySpatial==0 || m_bIncludePurelySpatial==1))
        bValid = DisplayParamError(PURESPATIAL);
      if (!(m_bIncludePurelyTemporal==0 || m_bIncludePurelyTemporal==1))
        bValid = DisplayParamError(PURETEMPORAL);
    }
    else
    {
      m_bIncludePurelySpatial  = false;
      m_bIncludePurelyTemporal = false;
    }

  }

  if (strlen(m_szCaseFilename)==0 || (pFile = fopen(m_szCaseFilename, "r")) == NULL)
    bValid = DisplayParamError(CASEFILE);
  else 
    fclose(pFile);

  if (m_nModel == POISSON)
  {
    if (strlen(m_szPopFilename)==0 || (pFile = fopen(m_szPopFilename, "r")) == NULL)
      bValid = DisplayParamError(POPFILE);
    else 
      fclose(pFile);
    strcpy(m_szControlFilename, "");
  }
  else if (m_nModel == BERNOULLI)
  {
    if (strlen(m_szControlFilename)==0 || (pFile = fopen(m_szControlFilename, "r")) == NULL)
      bValid = DisplayParamError(CONTROLFILE);
    else
      fclose(pFile);
    strcpy(m_szPopFilename, "");
  }

  if (strlen(m_szCoordFilename)==0 || (pFile = fopen(m_szCoordFilename, "r")) == NULL)
    bValid = DisplayParamError(COORDFILE);
  else 
    fclose(pFile);

  if (!(m_bSpecialGridFile==0 || m_bSpecialGridFile== 1))
    bValid = DisplayParamError(SPECIALGRID);

  if (m_bSpecialGridFile)
  {
    if (strlen(m_szGridFilename)==0 || (pFile = fopen(m_szGridFilename, "r")) == NULL)
      bValid = DisplayParamError(GRIDFILE);
    else
      fclose(pFile);
  }
  else
    strcpy(m_szGridFilename, "");

  if (strlen(m_szOutputFilename)==0 || (pFile = fopen(m_szOutputFilename, "r")) == NULL)
  {
    if ((pFile = fopen(m_szOutputFilename, "w")) == NULL)
      bValid = DisplayParamError(OUTPUTFILE);
    else
      fclose(pFile);
  }
  else
    fclose(pFile);

  m_bExactTimes = 0;
  m_nDimension  = 0;

  return bValid;
}

bool CParameters::ValidateDateString(char* szDate, int nDateType)
{
  UInt nYear, nMonth, nDay;
  int  nScanCount;

  nScanCount = CharToMDY(&nMonth, &nDay, &nYear, szDate);

  if (nScanCount < 1)
    return false;

  if (m_nPrecision == YEAR || nScanCount == 1)
  {
    switch(nDateType)
    {
      case STARTDATE: nMonth = 1; break;
      case ENDDATE  : nMonth = 12; break;
    }
  }

  if (m_nPrecision== YEAR || m_nPrecision == MONTH ||
      nScanCount  == 1    || nScanCount   == 2)
  {
    switch(nDateType)
    {
      case STARTDATE: nDay = 1; break;
      case ENDDATE  : nDay = DaysThisMonth(nYear, nMonth); break;
    }
  }

  if (!IsDateValid(nMonth, nDay, nYear))
    return false;
  else
    return true;
}

bool CParameters::ValidateReplications(int nReps)
{
  return (nReps >= 0) && (nReps == 0 || nReps == 9 || nReps == 19 || fmod(nReps+1, 1000) == 0.0);
}

bool CParameters::DisplayParamError(int nLine)
{
  if (m_bDisplayErrors)
    fprintf(stderr, "  Error: Invalid value in parameter file, line %i.\n", nLine);

  return false;
}

void CParameters::SetDefaults()
{
  m_nAnalysisType             = PURELYSPATIAL;
  m_nAreas                    = HIGH;
  strcpy(m_szCaseFilename, "");
  strcpy(m_szPopFilename, "");
  strcpy(m_szCoordFilename, "");
  strcpy(m_szOutputFilename, "");
  m_nPrecision                = YEAR;
  m_nDimension                = 0;
  m_bSpecialGridFile          = false;
  strcpy(m_szGridFilename, "");
  m_nMaxGeographicClusterSize = 50.0; //GG980716
  strcpy(m_szStartDate, "1900/01/01");
  strcpy(m_szEndDate, "1900/12/31");
  m_bAliveClustersOnly        = false; 
  m_bExactTimes               = false;
  m_nIntervalUnits            = 0;
  m_nIntervalLength           = 0;
  m_bIncludePurelySpatial     = false;
  m_nMaxTemporalClusterSize   = 50.0;//KR980707 0 GG980716;
  m_nReplicas                 = 999;

  SetDefaultsV2();
}

void CParameters::SetDefaultsV2()
{
  m_nModel                 = POISSON;
  m_nRiskFunctionType      = STANDARDRISK;
  m_bPowerCalc             = false;
  m_nPower_X               = 0.0;
  m_nPower_Y               = 0.0;
  m_nTimeAdjustType        = NOTADJUSTED;
  m_nTimeAdjPercent        = 0;
  m_bIncludePurelyTemporal = false;
  strcpy(m_szControlFilename, "");
  m_nCoordType             = CARTESIAN;
  m_bSaveSimLogLikelihoods = false;
  m_bSequential            = false;
  m_nAnalysisTimes         = 0;
  m_nCutOffPVal            = 0.0;
  m_bValidatePriorToCalc   = true;
  m_bOutputRelRisks        = false;
  m_nExtraParam1           = 0;
  m_nExtraParam2           = 0;
  m_nExtraParam3           = 0;
  m_nExtraParam4           = 0;
}

bool CParameters::SaveParameters(char* szFilename)
{
  FILE* pFile;
  
  printf("Saving Parameters.\n");

  if ((pFile = fopen(szFilename, "w")) == NULL)
  {
    printf("  Error: Unable to open parameter file.\n");
    return false;
  }


  if (strlen(m_szGridFilename) > 0)
    m_bSpecialGridFile = true;
  else
    m_bSpecialGridFile = false;

  m_nExtraParam1 = 0;
  m_nExtraParam2 = 0;
  m_nExtraParam3 = 0;
  m_nExtraParam4 = 0;
      
  fprintf(pFile, "%i                     // Analysis Type (1=PS, 2=ST, 3=PT)\n", m_nAnalysisType);
  fprintf(pFile, "%i                     // Scan Areas (1=High, 2=Low, 3=HighLow)\n", m_nAreas);
  fprintf(pFile, "%s\n", m_szCaseFilename);
  fprintf(pFile, "%s\n", m_szPopFilename);
  fprintf(pFile, "%s\n", m_szCoordFilename);
  fprintf(pFile, "%s\n", m_szOutputFilename);
  fprintf(pFile, "%i                     // Precision of Case Times (0=None, 1=Year, 2=Month, 3=Day)\n", m_nPrecision);
  fprintf(pFile, "%i                     // Not Applicable \n", m_nDimension);
  fprintf(pFile, "%i                     // Use Special Grid File? (0=No, 1=Yes)\n", m_bSpecialGridFile);
  fprintf(pFile, "%s\n", m_szGridFilename);
  fprintf(pFile, "%f                    // Max Geographic Size (1-50)\n", m_nMaxGeographicClusterSize);
  fprintf(pFile, "%s              // Start Date (YYYY/MM/DD)\n", m_szStartDate);
  fprintf(pFile, "%s            // End Date (YYYY/MM/DD)\n", m_szEndDate);
  fprintf(pFile, "%i                     // Alive Cluster Only? (0=No, 1=Yes)\n", m_bAliveClustersOnly);
  fprintf(pFile, "%i                     // Exact Times? (0)\n", m_bExactTimes);
  fprintf(pFile, "%i                     // Interval Units (0=None, 1=Year, 2=Month, 3=Day)\n", m_nIntervalUnits);
  fprintf(pFile, "%i                     // Interval Length (1-Max Interval)\n", m_nIntervalLength);
  fprintf(pFile, "%i                     // Include Pure Spatial? (0=No, 1=Yes)\n", m_bIncludePurelySpatial);
  fprintf(pFile, "%f                     // Max Temporal Size (1-90)\n", m_nMaxTemporalClusterSize);
  fprintf(pFile, "%i                     // Replications (0, 9, 999, n999)\n", m_nReplicas);
  fprintf(pFile, "%i                     // Model Type (0=Poisson, 1=Bernoulli)\n", m_nModel);
  fprintf(pFile, "%i                     // R Parameter #1 (0=S, 1=M)\n", m_nRiskFunctionType);
  fprintf(pFile, "%i                     // P Parameter #1 (0=No, 1=Yes)\n", m_bPowerCalc);
  fprintf(pFile, "%f              // P Parameter #2\n", m_nPower_X);
  fprintf(pFile, "%f              // P Parameter #3\n", m_nPower_Y);
  fprintf(pFile, "%i                     // Time Trend Adjustment Type (0=None, 1=Nonparametric, 2=Linear)\n", m_nTimeAdjustType);
  fprintf(pFile, "%f                     // Time Trend Percentage (>-100)\n", m_nTimeAdjPercent);
  fprintf(pFile, "%i                     // Include Purely Temporal? (0=No, 1=Yes)\n", m_bIncludePurelyTemporal);
  fprintf(pFile, "%s\n", m_szControlFilename);
  fprintf(pFile, "%i                     // Coordinates Type (0=Cartesian, 1=Lat/Lon)\n", m_nCoordType);
  fprintf(pFile, "%i                     // Save Sim LLR's? (0=No, 1=Yes)\n", m_bSaveSimLogLikelihoods);
  fprintf(pFile, "%i                     // S Parameter #1 (0=No, 1=Yes)\n", m_bSequential);
  fprintf(pFile, "%i                     // S Parameter #2 (1-32,000)\n", m_nAnalysisTimes);
  fprintf(pFile, "%f              // S Parameter #3  (0-1)\n", m_nCutOffPVal);
  fprintf(pFile, "%i                     // Validate Parameters? (0=No, 1=Yes)\n", m_bValidatePriorToCalc);
  fprintf(pFile, "%i                     // Include Relative Risks for Census Areas?\n",m_bOutputRelRisks);
  fprintf(pFile, "%i                     // Extra Parameter #1\n", m_nExtraParam1);
  fprintf(pFile, "%i                     // Extra Parameter #2\n", m_nExtraParam2);
  fprintf(pFile, "%i                     // Extra Parameter #3\n", m_nExtraParam3);
  fprintf(pFile, "%i                     // Extra Parameter #4\n", m_nExtraParam4);

  fclose(pFile);

  return true;
}

void CParameters::DisplayParameters(FILE* fp)
{
  fprintf(fp, "\n________________________________________________________________\n\n");
  fprintf(fp, "PARAMETER SETTINGS\n\n");

  fprintf(fp, "Input Files\n");
  fprintf(fp, "-----------\n");

  fprintf(fp, "  Case File        : %s\n", m_szCaseFilename);

  if (m_nModel == POISSON)
    fprintf(fp, "  Population File  : %s\n", m_szPopFilename);
  else if (m_nModel == BERNOULLI)
    fprintf(fp, "  Control File     : %s\n", m_szControlFilename);

  fprintf(fp, "  Coordinates File : %s\n", m_szCoordFilename);
  if (m_bSpecialGridFile)
    fprintf(fp, "  Special Grid File: %s\n", m_szGridFilename);

  fprintf(fp, "\n  Precision of Times : ");
  switch (m_nPrecision)
  {
    case NONE  : fprintf(fp, "None\n"); break;
    case YEAR  : fprintf(fp, "Years\n"); break;
    case MONTH : fprintf(fp, "Months\n"); break;
    case DAY   : fprintf(fp, "Days\n"); break;
  }

  fprintf(fp, "  Coordinates        : ");
  switch (m_nCoordType)
  {
    case CARTESIAN : fprintf(fp, "Cartesian\n"); break;
    case LATLON    : fprintf(fp, "Latitude/Longitude\n"); break;
  }

  fprintf(fp, "\nAnalysis\n");
  fprintf(fp, "--------\n");

  fprintf(fp, "  Type of Analysis    : ");
  switch (m_nAnalysisType)
  {
    case PURELYSPATIAL  : fprintf(fp, "Purely Spatial\n"); break;
    case SPACETIME      : fprintf(fp, "Space-Time\n"); break;
    case PURELYTEMPORAL : fprintf(fp, "Purely Temporal\n"); break;
  }

  fprintf(fp, "  Probability Model   : ");
  switch (m_nModel)
  {
    case POISSON   : fprintf(fp, "Poisson\n"); break;
    case BERNOULLI : fprintf(fp, "Bernoulli\n"); break;
  }

  fprintf(fp, "  Scan for Areas with : ");
  switch (m_nAreas)
  {
    case HIGH       : fprintf(fp, "High Rates\n"); break;
    case LOW        : fprintf(fp, "Low Rates\n"); break;
    case HIGHANDLOW : fprintf(fp, "High or Low Rates\n"); break;
  }

  fprintf(fp, "\n  Start Date : %s\n", m_szStartDate);
  fprintf(fp, "  End Date   : %s\n\n", m_szEndDate);

  fprintf(fp, "  Number of Replications : %i\n", m_nReplicas);

  fprintf(fp, "\nScanning Window\n");
  fprintf(fp, "---------------\n");

  if (m_nAnalysisType == PURELYSPATIAL || m_nAnalysisType == SPACETIME)
    fprintf(fp, "  Maximum Spatial Cluster Size : %.2f\n", m_nMaxGeographicClusterSize);
  if (m_nAnalysisType == SPACETIME)
  {
    fprintf(fp, "  Also Include Purely Temporal Clusters : ");
    switch (m_bIncludePurelyTemporal)
    {
      case true  : fprintf(fp, "Yes\n"); break;
      case false : fprintf(fp, "No\n");  break;
    }
  }

  if (m_nAnalysisType == PURELYTEMPORAL || m_nAnalysisType == SPACETIME)
    fprintf(fp, "  Maximum Temporal Cluster Size : %.2f\n", m_nMaxTemporalClusterSize);
  if (m_nAnalysisType == SPACETIME)
  {
    fprintf(fp, "  Also Include Purely Spatial Clusters : ");
    switch (m_bIncludePurelySpatial)
    {
      case true  : fprintf(fp, "Yes\n"); break;
      case false : fprintf(fp, "No\n");  break;
    }
  }

  if (m_nAnalysisType == PURELYTEMPORAL || m_nAnalysisType == SPACETIME)
  {
    fprintf(fp, "  Clusters to Include : ");
    switch (m_bAliveClustersOnly)
    {
      case ALLCLUSTERS   : fprintf(fp, "All\n"); break;
      case ALIVECLUSTERS : fprintf(fp, "Only those including the study end date\n"); break;
    }
  }

  if (m_nAnalysisType == PURELYTEMPORAL || m_nAnalysisType == SPACETIME)
  {
    fprintf(fp, "\nTime Parameters\n");
    fprintf(fp, "---------------\n");

    fprintf(fp, "\n  Time Interval Units  : ");
    switch (m_nIntervalUnits)
    {
      case 1 : fprintf(fp, "Years\n"); break;
      case 2 : fprintf(fp, "Months\n"); break;
      case 3 : fprintf(fp, "Days\n"); break;
    }
    fprintf(fp, "  Time Interval Length : %i\n", m_nIntervalLength);

    fprintf(fp, "\n  Adjustment for Time Trend : ");
    switch (m_nTimeAdjustType)
    {
      case NOTADJUSTED : fprintf(fp, "None\n"); break;
      case NONPARAMETRIC    : fprintf(fp, "Nonparametric\n"); break;
      case LINEAR  : fprintf(fp, "Linear with %0.2f%% per year\n", m_nTimeAdjPercent); break;
    }
  }

  fprintf(fp, "\nOutput\n");
  fprintf(fp, "------\n");
  fprintf(fp, "  Results File : %s\n", m_szOutputFilename);
  fprintf(fp, "  GIS File     : %s\n", m_szGISFilename);
  if (m_bSaveSimLogLikelihoods)
    fprintf(fp, "  LLR File     : %s\n", m_szLLRFilename);

  fprintf(fp, "\n________________________________________________________________\n");
}

void CParameters::DisplayAnalysisType(FILE* fp)
{
  switch (m_nAnalysisType)
  {
    case (PURELYSPATIAL) : fprintf(fp, "Purely Spatial analysis "); break;
    case (SPACETIME)     : fprintf(fp, "Space Time analysis "); break;
    case (PURELYTEMPORAL): fprintf(fp, "Purely Temporal analysis "); break;
  }

  fprintf(fp, "scanning for ");

  if (m_nRiskFunctionType==MONOTONERISK)
    fprintf(fp, "monotone ");

  fprintf(fp, "clusters with \n");

  switch (m_nAreas)
  {
    case (HIGH)      : fprintf(fp, "high rates"); break;
    case (LOW)       : fprintf(fp, "low rates"); break;
    case (HIGHANDLOW): fprintf(fp, "high or low rates"); break;
  }

  if (m_nModel == POISSON)
    fprintf(fp, " using the Poisson model.\n");
  else if (m_nModel == BERNOULLI)
    fprintf(fp, " using the Bernoulli model.\n");

  if (m_nAnalysisType == SPACETIME)
  {
    if (m_bIncludePurelySpatial && m_bIncludePurelyTemporal)
      fprintf(fp, "Analysis includes purely spatial and purely temporal clusters.\n");
    else if (m_bIncludePurelySpatial)
      fprintf(fp, "Analysis includes purely spatial clusters.\n");
    else if (m_bIncludePurelyTemporal)
      fprintf(fp, "Analysis includes purely temporal clusters.\n");
  }

  if (m_bSequential)
    fprintf(fp, "Sequential analysis performed.\n");

//  fprintf(fp,"\n");
}


void CParameters::DisplayTimeAdjustments(FILE* fp)
{
  if (m_nTimeAdjustType == NOTADJUSTED)
    return;

  fprintf(fp, "Adjusted for time ");

  if (m_nTimeAdjustType == NONPARAMETRIC)
    fprintf(fp, "nonparametrically.\n");
  else if (m_nTimeAdjustType == LINEAR)
  {
    if (m_nTimeAdjPercent < 0)
      fprintf(fp, "with a decrease ");
    else
      fprintf(fp, "with an increase ");
    fprintf(fp, "of %0.2f%% per year.\n", fabs(m_nTimeAdjPercent));
  }
}


