#include "SaTScan.h"
#pragma hdrstop
#include "Parameters.h"

char mgsVariableLabels[44][100] = {
   "Analysis Type",
   "Scan Areas",
   "Case File",
   "Population File",
   "Coordinates File",
   "Results File",
   "Precision of Case Times",
   "Not applicable",
   "Special Grid File Use",
   "Grid File",
   "Max Geographic size",
   "Start Date",
   "End Date",
   "Alive Clusters Only",
   "Exact Times",
   "Interval Units",
   "Interval Length",
   "Include Purely Spacial",
   "Max Temporal Size",
   "Replications",
   "Model Type (Poisson, Bernoulli, Poisson, Bernoulli or Space-Time Permutation)",
   "R Parameter 1",
   "P Parameter 1",
   "P Parameter 2",
   "P Parameter 3",
   "Time Trend Adjustment Type",
   "Time Trend Percentage",
   "Include Purely Temporal",
   "Control File",
   "Coordinates Type",
   "Save Sim LLRs",
   "S Parameter 1",
   "S Parameter 2",
   "S Parameter 3",
   "Validate Parameters",
   "Include Relative Risks for Census Areas",
   "Number of Ellipses",
   "Ellipse Shapes",
   "Ellipse Angles",
   "Prospective Start Date",
   "Output Census areas in Reported Clusters",
   "Output Most Likely Cluster for each Centroid",
   "Criteria for Reporting Secondary Clusters",
   "How Max Temporal Size Should Be Interperated"
   };

CParameters::CParameters(bool bDisplayErrors)
{
   SetDefaults();
   m_bDisplayErrors = bDisplayErrors;
}

//------------------------------------------------------------------------------
//Copy constructor
//------------------------------------------------------------------------------
CParameters::CParameters(const CParameters &other)
{
   SetDefaults();
   copy(other);
}

CParameters::~CParameters()
{
   Free();
}

//------------------------------------------------------------------------------
//Overload assignment operator
//------------------------------------------------------------------------------
CParameters &CParameters::operator= (const CParameters &rhs)
{
   if (this != &rhs)
      copy(rhs);

   return (*this);
}

//------------------------------------------------------------------------------
// copies all class variables from the given CParameters object (rhs) into this one
//------------------------------------------------------------------------------
void CParameters::copy(const CParameters &rhs)
{
    m_nNumEllipses = rhs.m_nNumEllipses;
    if (m_nNumEllipses > 0)
       {
       if (mp_dEShapes)
          delete [] mp_dEShapes;
       if (mp_nENumbers)
          delete [] mp_nENumbers;
       mp_dEShapes = new double[ m_nNumEllipses ];
       mp_nENumbers = new int[ m_nNumEllipses ];
       memcpy(mp_dEShapes, rhs.mp_dEShapes, (sizeof(double)*m_nNumEllipses));
       memcpy(mp_nENumbers, rhs.mp_nENumbers, (sizeof(int)*m_nNumEllipses));
       }
    else
       {
       mp_dEShapes = 0;
       mp_nENumbers = 0;
       }
    m_lTotalNumEllipses = rhs.m_lTotalNumEllipses;
    m_nAnalysisType     = rhs.m_nAnalysisType;
    m_nAreas            = rhs.m_nAreas;
    m_nModel            = rhs.m_nModel;
    m_nRiskFunctionType = rhs.m_nRiskFunctionType;
    m_nReplicas         = rhs.m_nReplicas;
    m_bPowerCalc        = rhs.m_bPowerCalc;
    m_nPower_X          = rhs.m_nPower_X;
    m_nPower_Y          = rhs.m_nPower_Y;
    strcpy(m_szStartDate, rhs.m_szStartDate);
    strcpy(m_szEndDate, rhs.m_szEndDate);
    m_nMaxGeographicClusterSize = rhs.m_nMaxGeographicClusterSize;
    m_nMaxTemporalClusterSize   = rhs.m_nMaxTemporalClusterSize;
    m_bAliveClustersOnly        = rhs.m_bAliveClustersOnly;
    m_nIntervalUnits            = rhs.m_nIntervalUnits;
    m_nIntervalLength           = rhs.m_nIntervalLength;
    m_nTimeAdjustType           = rhs.m_nTimeAdjustType;
    m_nTimeAdjPercent           = rhs.m_nTimeAdjPercent;
    m_bIncludePurelySpatial     = rhs.m_bIncludePurelySpatial;
    m_bIncludePurelyTemporal    = rhs.m_bIncludePurelyTemporal;
    strcpy(m_szCaseFilename,rhs.m_szCaseFilename);
    strcpy(m_szControlFilename,rhs.m_szControlFilename);
    strcpy(m_szPopFilename,rhs.m_szPopFilename);
    strcpy(m_szCoordFilename,rhs.m_szCoordFilename);
    strcpy(m_szGridFilename,rhs.m_szGridFilename);
    m_bSpecialGridFile = rhs.m_bSpecialGridFile;
    m_nPrecision = rhs.m_nPrecision;
    m_nDimension = rhs.m_nDimension;
    m_nCoordType = rhs.m_nCoordType;
    strcpy(m_szOutputFilename,rhs.m_szOutputFilename);
    strcpy(m_szGISFilename,rhs.m_szGISFilename);
    strcpy(m_szLLRFilename,rhs.m_szLLRFilename);
    strcpy(m_szMLClusterFilename,rhs.m_szMLClusterFilename);
    strcpy(m_szRelRiskFilename, rhs.m_szRelRiskFilename);
    m_bSaveSimLogLikelihoods = rhs.m_bSaveSimLogLikelihoods;
    m_bOutputRelRisks        = rhs.m_bOutputRelRisks;
    m_bSequential            = rhs.m_bSequential;
    m_nAnalysisTimes         = rhs.m_nAnalysisTimes;
    m_nCutOffPVal            = rhs.m_nCutOffPVal;
    m_bExactTimes            = rhs.m_bExactTimes;
    m_nClusterType           = rhs.m_nClusterType;
    m_bValidatePriorToCalc   = rhs.m_bValidatePriorToCalc;
    m_bDisplayErrors         = rhs.m_bDisplayErrors;
    
    strcpy(m_szProspStartDate, rhs.m_szProspStartDate);
    m_bOutputCensusAreas     = rhs.m_bOutputCensusAreas;
    m_bMostLikelyClusters    = rhs.m_bMostLikelyClusters;
    m_iCriteriaSecondClusters = rhs.m_iCriteriaSecondClusters;

    m_nMaxClusterSizeType = rhs.m_nMaxClusterSizeType;
}

void CParameters::Free()
{
   if (mp_dEShapes)
      delete [] mp_dEShapes;
   if (mp_nENumbers)
      delete [] mp_nENumbers;
}

bool CParameters::SetParameters(const char* szFilename, bool bValidate)
{
   FILE* pFile;
   char  szTemp [MAX_STR_LEN];
   bool  bValid = true;

   try
      {
      //gpPrintDirection->SatScanPrintf("Reading Parameters.\n");

      if ((pFile = fopen(szFilename, "r")) == NULL)
        SSGenerateException("  Error: Unable to open parameter file.", "SetParameters()");

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
    
      if (bEOF && ((i-1)==MODEL))              // Accept V.1 parameter files
        SetDefaultsV2();
      else if (bEOF && ((i-2) < CRITERIA_SECOND_CLUSTERS))      // Accept V.1.3 parameter files
         SetDefaultsV3();
      else if ( (bEOF && (i-2) < MAX_TEMPORAL_TYPE) || (!bEOF && (i-1) < MAX_TEMPORAL_TYPE ) )
         m_nMaxClusterSizeType = PERCENTAGETYPE;

      if (!SetGISFilename())   //Census areas in reported clusters
        bValid = false;
    
      if (!SetLLRFilename())  //most likely cluster for each centroid
        bValid = false;

      if (!SetMLCFilename()) //most likely cluster for each centroid - file name
        bValid = false;

      if (!SetRelRiskFilename()) //rel risk estimate for each centroid - file name
        bValid = false;
      
      if (bValid && bValidate)
        bValid = ValidateParameters();
       }
   catch (SSException & x)
      {
      x.AddCallpath("SetParameters()", "CParameters");
      throw;
      }
   return bValid;
}

bool CParameters::SetParameter(int nParam, const char* szParam)
{
   bool bValid;
   int  nScanCount, i;
   int  nTemp;

   try
      {
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
        case POWERX       : nScanCount=sscanf(szParam, "%lf", &m_nPower_X); break;
        case POWERY       : nScanCount=sscanf(szParam, "%lf", &m_nPower_Y); break;
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
        case ELLIPSES     : nScanCount=sscanf(szParam, "%i", &m_nNumEllipses); break;
        case ESHAPES      : nScanCount = LoadEShapes(szParam); break;
        case ENUMBERS     : nScanCount = LoadEAngles(szParam); break;
       // case EXTRA4       : nScanCount=sscanf(szParam, "%i", &m_nExtraParam4); break;
        case START_PROSP_SURV:
              nScanCount=sscanf(szParam, "%s", &m_szProspStartDate);
              if ((nScanCount == 0) || (strlen(m_szProspStartDate)==1))  strcpy(m_szProspStartDate, "");
              break;
        case OUTPUT_CENSUS_AREAS:  nScanCount=sscanf(szParam, "%i", &nTemp); m_bOutputCensusAreas = nTemp?true:false; break;;
        case OUTPUT_MOST_LIKE_CLUSTERS: nScanCount=sscanf(szParam, "%i", &nTemp); m_bMostLikelyClusters = nTemp?true:false; break;;
        case CRITERIA_SECOND_CLUSTERS: nScanCount=sscanf(szParam, "%i", &m_iCriteriaSecondClusters); break;
        case MAX_TEMPORAL_TYPE: nScanCount=sscanf(szParam, "%i", &m_nMaxClusterSizeType); break;
      }
    
      if (nParam==POPFILE || nParam==GRIDFILE || nParam==CONTROLFILE)
        bValid = true;
      else
      {
        // Need to be able to handle old parameter files  !!!!!!
        bValid = (nScanCount == 1);
        if (!bValid)
          DisplayParamError(nParam);
      }
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetParameter()", "CParameters");
      throw;
      }
   return bValid;
}

int CParameters::LoadEShapes(const char* szParam)
{
   int i, nCount = 0, nScanCount, iLineLength;
   double dDummyValue;
   bool bOk = true;
   char *sTempLine = 0;

   try
      {
      //Example is   "1.5 2"      DOUBLES
      if (m_nNumEllipses > 0)
         {
         iLineLength = strlen(szParam);
         if (iLineLength == 0)
            SSGenerateException("Parameter line for Ellipsoid shapes is empty","LoadEShapes");
         else
            {
            sTempLine = new char [ iLineLength + 2 ];
            strcpy(sTempLine, szParam);
            }
         if (mp_dEShapes)
            delete[] mp_dEShapes;
         mp_dEShapes = new double[ m_nNumEllipses ];
         for (i = 0; (i < m_nNumEllipses) && bOk; i++)
            {
            TrimLeft(sTempLine);
            nScanCount=sscanf(sTempLine, "%lf", &mp_dEShapes[i]);
            if (nScanCount == 1)
               FindDelimiter(sTempLine, ' ');
            else
               bOk = false;
            }
         }
      if ( ! bOk )
          SSGenerateException("Not enough ellipsoid shape specifications in parameter file","LoadEShapes");
      else  // make sure that there are not too many ellipsoid shapes...
         {
         if (! sTempLine)
            {
            iLineLength = strlen(szParam);
            sTempLine = new char [ iLineLength + 2 ];
            strcpy(sTempLine, szParam);
            }
         TrimLeft(sTempLine);
         nScanCount=sscanf(sTempLine, "%lf", &dDummyValue);
         if (nScanCount == 1)
            SSGenerateException("Too many ellipsoid shape specifications in parameter file.","LoadEShapes");
         else
            nCount = 1;
         }
      delete [] sTempLine;
      }
   catch (SSException & x)
      {
      delete [] sTempLine;
      x.AddCallpath("LoadEShapes()", "CParameters");
      throw;
      }
   return nCount;
}


int CParameters::LoadEAngles(const char* szParam)
{
   int i, nCount = 0, nScanCount, iLineLength;
   double dDummyValue;
   bool bOk = true;
   char *sTempLine = 0;

   try
      {
      //Example is   "1.5 2"      DOUBLES
      if (m_nNumEllipses > 0)
         {
         iLineLength = strlen(szParam);
         if (iLineLength == 0)
            SSGenerateException("Parameter line for Ellipsoid shapes is empty","LoadEAngles()");
         else
            {
            sTempLine = new char [ iLineLength + 2 ];
            strcpy(sTempLine, szParam);
            }
         if (mp_nENumbers)
            delete[] mp_nENumbers;
         mp_nENumbers = new int[ m_nNumEllipses ];
         for (i = 0; (i < m_nNumEllipses) && bOk; i++)
            {
            TrimLeft(sTempLine);
            nScanCount=sscanf(sTempLine, "%i", &mp_nENumbers[i]);
            if (nScanCount == 1)
               FindDelimiter(sTempLine, ' ');
            else
               bOk = false;
            }
         for (i = 0; i < m_nNumEllipses; i++)
           m_lTotalNumEllipses +=  mp_nENumbers[i];
         }
      if ( ! bOk )
          SSGenerateException("Not enough ellipsoid angle specifications in parameter file.","LoadEAngles()");
      else  // make sure that there are not too many ellipsoid shapes...
         {
         if (! sTempLine)
            {
            iLineLength = strlen(szParam);
            sTempLine = new char [ iLineLength + 2 ];
            strcpy(sTempLine, szParam);
            }
         TrimLeft(sTempLine);
         nScanCount=sscanf(sTempLine, "%lf", &dDummyValue);
         if (nScanCount == 1)
            SSGenerateException("Too many ellipsoid angle specifications in parameter file.","LoadEShapes");
         else
            nCount = 1;
         }
      delete [] sTempLine;
      }
   catch (SSException & x)
      {
      delete [] sTempLine;
      x.AddCallpath("LoadEAngles()", "CParameters");
      throw;
      }
   return nCount;
}

void CParameters::TrimLeft(char *sString)
{
   char  * psString;

   try
      {
      psString = sString;
      while (*psString <= ' ' && *psString)
         psString++;
      if (psString != sString)
         strcpy(sString, psString);
       }
   catch (SSException & x)
      {
      x.AddCallpath("TrimLeft()", "CParameters");
      throw;
      }
}

void CParameters::FindDelimiter(char *sString, char cDelimiter)
{
   char  * psString;

   try
      {
      psString = sString;
      while (*psString != cDelimiter && *psString)
         psString++;
      if (psString != sString)
         strcpy(sString, psString);
       }
   catch (SSException & x)
      {
      x.AddCallpath("FindDelimiter()", "CParameters");
      throw;
      }
}

//** Converts m_nMaxClusterSizeType to passed type. */
void CParameters::ConvertMaxTemporalClusterSizeToType(TemporalSizeType eTemporalSizeType) {
  double dValue, dTimeBetween;

  try {
    switch (eTemporalSizeType) {
       PERCENTAGETYPE     : if (m_nMaxClusterSizeType == PERCENTAGETYPE)
                              break;
                            // convert from TIMETYPE to PERCENTAGETYPE
                            dTimeBetween = TimeBetween(CharToJulian(m_szStartDate),CharToJulian(m_szEndDate), m_nIntervalUnits);
                            //m_nMaxTemporalClusterSize should be in time units of m_nIntervalUnits
                            m_nMaxTemporalClusterSize = floor(m_nMaxTemporalClusterSize/dTimeBetween*100);
                            m_nMaxClusterSizeType = PERCENTAGETYPE;
                            break;
       TIMETYPE           : if (m_nMaxClusterSizeType == TIMETYPE)
                              break;
                            // convert from PERCENTAGETYPE to TIMETYPE
                            dTimeBetween = TimeBetween(CharToJulian(m_szStartDate),CharToJulian(m_szEndDate), m_nIntervalUnits);
                            //m_nMaxTemporalClusterSize should be an integer from 1-90
                            m_nMaxTemporalClusterSize = dTimeBetween * m_nMaxTemporalClusterSize/100;
                            m_nMaxClusterSizeType = TIMETYPE;
                            break;
       default            : SSException::Generate("Unknown TemporalSizeType type %d", "ConvertMaxTemporalClusterSizeToType()",
                                                  SSException::Normal, eTemporalSizeType);
    };
  }
  catch (SSException & x) {
    x.AddCallpath("ConvertMaxTemporalClusterSizeToType()", "CParameters");
    throw;
  }
}

bool CParameters::SetMLCFilename() //most likely cluster for each centroid - file name
{
   int  nReportNameLen = strlen(m_szOutputFilename);
   int  nIndex         = nReportNameLen-1;
   bool bDone          = false;
   bool bExtFound      = false;
   bool bReturnValue;

   try
      {
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

      strncpy(m_szMLClusterFilename, m_szOutputFilename, nIndex);
      strcpy(m_szMLClusterFilename+nIndex, ".col.txt");

      if (strcmp(m_szMLClusterFilename, m_szOutputFilename)==0)
        SSGenerateException("  Error: Attempting to write report to COL (clusters in column format) file.\n", "SetMLCFilename()");
      else
        bReturnValue = true;
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetMLCFilename()", "CParameters");
      throw;
      }
   return bReturnValue;
}

bool CParameters::SetRelRiskFilename()  //set rel risk estimate file name
{
   int  nReportNameLen = strlen(m_szOutputFilename);
   int  nIndex         = nReportNameLen-1;
   bool bDone          = false;
   bool bExtFound      = false;
   bool bReturnValue;

   try
      {
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

      strncpy(m_szRelRiskFilename, m_szOutputFilename, nIndex);
      strcpy(m_szRelRiskFilename+nIndex, ".rr.txt");

      if (strcmp(m_szRelRiskFilename, m_szOutputFilename)==0)
         SSGenerateException("  Error: Attempting to write report to RRE file.\n", "SetRelRiskFilename()");
      // Add LLR error check
      else
        bReturnValue = true;
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetRelRiskFilename()", "CParameters");
      throw;
      }
   return bReturnValue;
}

bool CParameters::SetGISFilename()
{
   int  nReportNameLen = strlen(m_szOutputFilename);
   int  nIndex         = nReportNameLen-1;
   bool bDone          = false;
   bool bExtFound      = false;
   bool bReturnValue;

   try
      {
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
      strcpy(m_szGISFilename+nIndex, ".gis.txt");

      if (strcmp(m_szGISFilename, m_szOutputFilename)==0)
         SSGenerateException("  Error: Attempting to write report to GIS file.\n", "SetGISFileName()");
      else
        bReturnValue = true;
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetGISFileName()", "CParameters");
      throw;
      }
   return bReturnValue;
}

bool CParameters::SetLLRFilename()
{
   int  nReportNameLen = strlen(m_szOutputFilename);
   int  nIndex         = nReportNameLen-1;
   bool bDone          = false;
   bool bExtFound      = false;
   bool bReturnValue;

   try
      {
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
      strcpy(m_szLLRFilename+nIndex, ".llr.txt");

    
      if (strcmp(m_szLLRFilename, m_szOutputFilename)==0)
         SSGenerateException("  Error: Attempting to write report to LLR file.\n", "SetLLRFilename()");
      else
         bReturnValue = true;
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetLLRFileName()", "CParameters");
      throw;
      }
   return bReturnValue;
}

bool CParameters::ValidateParameters()
{
   bool  bValid = true;
   bool bValidDate = true;
   FILE* pFile;

   try
      {
      if (m_bValidatePriorToCalc)
      {
    
        if (!(PURELYSPATIAL <= m_nAnalysisType && m_nAnalysisType <= PROSPECTIVESPACETIME))   // use to be <= PURELYTEMPORAL
          bValid = DisplayParamError(ANALYSISTYPE);

        if (!(HIGH <= m_nAreas && m_nAreas<= HIGHANDLOW))
          bValid = DisplayParamError(SCANAREAS);
    
        if (!(POISSON == m_nModel || m_nModel == BERNOULLI || m_nModel == SPACETIMEPERMUTATION))
          bValid = DisplayParamError(MODEL);
    
        if (!(STANDARDRISK == m_nRiskFunctionType || m_nRiskFunctionType == MONOTONERISK))
          bValid = DisplayParamError(RISKFUNCTION);
    
        if (!(m_bSequential==0 || m_bSequential==1))
          bValid = DisplayParamError(SEQUENTIAL);
    
        if (m_bSequential)
        {
          //if (!((1 <= m_nAnalysisTimes) && (m_nAnalysisTimes <= INT_MAX)))
          if (!(1 <= m_nAnalysisTimes))
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
        else if ((m_nCoordType == LATLON) && (m_nNumEllipses > 0))
          bValid = DisplayParamError(ELLIPSES);                                  // Could do a better job of displaying messages here...  just shows line number and thats all...

        // If number of ellipsoids > 0, then criteria for reporting secondary
        // clusters must be "No Restrictions"
        if ((m_nNumEllipses > 0) && (m_iCriteriaSecondClusters != 5))
           {
           SSGenerateException("  Error: Number of Ellipsiods is greater than zero and Criteria for Secondary Clusters is NOT set to No Restrictions.", "ValidateParameters()");
           }

        if (!(m_bSaveSimLogLikelihoods==0 || m_bSaveSimLogLikelihoods==1))
          bValid = DisplayParamError(SAVESIMLL);
    
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
           {
           bValid = DisplayParamError(STARTDATE);
           bValidDate = false;
           }
        else if (!ValidateDateString(m_szEndDate, ENDDATE))
          {
          bValid = DisplayParamError(ENDDATE);
          bValidDate = false;
          }
        else if (strcmp(m_szStartDate, m_szEndDate) > 0)
          {
          bValid = DisplayParamError(ENDDATE);
          bValidDate = false;
          }
    
        // Spatial Options
        if ((m_nAnalysisType == PURELYSPATIAL) || (m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME))
        {
          if (!(0.0 < m_nMaxGeographicClusterSize && m_nMaxGeographicClusterSize <= 50.0)) //GG980716
          bValid = DisplayParamError(GEOSIZE);
        }
        else
          m_nMaxGeographicClusterSize = 50.0; //KR980707 0 GG980716;
    
        // Temporal Options
        if ((m_nAnalysisType == PURELYTEMPORAL) || (m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME))
        {
          if (!(0.0 < m_nMaxTemporalClusterSize && m_nMaxTemporalClusterSize <= 90.0)) //GG980716
            bValid = DisplayParamError(TIMESIZE);
          if ((!PROSPECTIVESPACETIME) && (!(m_bAliveClustersOnly==0 || m_bAliveClustersOnly==1)))
            bValid = DisplayParamError(CLUSTERS);
          if (!(YEAR <= m_nIntervalUnits && m_nIntervalUnits <= m_nPrecision))
            bValid = DisplayParamError(INTERVALUNITS);
          if (!(1 <= m_nIntervalLength && m_nIntervalLength <= TimeBetween(CharToJulian(m_szStartDate), CharToJulian(m_szEndDate), m_nIntervalUnits) ))// Change to Max Interval
            bValid = DisplayParamError(TIMEINTLEN);

          if (m_nModel == BERNOULLI || m_nModel == SPACETIMEPERMUTATION)
          {
            m_nTimeAdjustType = NOTADJUSTED;
            m_nTimeAdjPercent = 0.0;
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
        if ((m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME))
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
      else if (m_nModel == SPACETIMEPERMUTATION)
      {
        if (!(m_nAnalysisType == SPACETIME || m_nAnalysisType == PROSPECTIVESPACETIME))
          bValid = DisplayParamError(ANALYSISTYPE);
        if (m_bIncludePurelySpatial==1)
          bValid = DisplayParamError(PURESPATIAL);
        if (m_bIncludePurelyTemporal==1)
          bValid = DisplayParamError(PURETEMPORAL);
        if (m_bOutputRelRisks==1)
          bValid = DisplayParamError(OUTPUTRR);
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

      if (!(m_bOutputRelRisks==0 || m_bOutputRelRisks==1))
          bValid = DisplayParamError(OUTPUTRR);

      if (!(m_bOutputCensusAreas==0 || m_bOutputCensusAreas==1))
          bValid = DisplayParamError(OUTPUT_CENSUS_AREAS);
      if (!(m_bMostLikelyClusters==0 || m_bMostLikelyClusters==1))
          bValid = DisplayParamError(OUTPUT_MOST_LIKE_CLUSTERS);
      if (!(m_iCriteriaSecondClusters>=0 && m_iCriteriaSecondClusters<=5))
          bValid = DisplayParamError(CRITERIA_SECOND_CLUSTERS);

      // Verify Character Prospective start date (YYYY/MM/DD).
      // THIS IS RUN AFTER STUDY DATES HAVE BEEN CHECKED !!!
      if (bValid && (m_nAnalysisType == PROSPECTIVESPACETIME) && bValidDate)
         {
         if (! ValidateProspectiveStartDate(m_szProspStartDate, m_szStartDate, m_szEndDate))
            bValid = DisplayParamError(START_PROSP_SURV);
         }

      m_bExactTimes = 0;
      m_nDimension  = 0;

      }
   catch (SSException & x)
      {
      x.AddCallpath("ValidateParameters()", "CParameters");
      throw;
      }
   return bValid;
}

bool CParameters::ValidateProspectiveStartDate(char* szProspDate, char *szStartDate, char *szEndDate)
{
   UInt nProspYear, nProspMonth, nProspDay;
   UInt nStartYear, nStartMonth, nStartDay;
   UInt nEndYear, nEndMonth, nEndDay;
   int  nStartScanCount, nEndScanCount, nProspScanCount;
   bool bReturnValue = false;

   try
      {
      nStartScanCount = CharToMDY(&nStartMonth, &nStartDay, &nStartYear, szStartDate);
      nEndScanCount = CharToMDY(&nEndMonth, &nEndDay, &nEndYear, szEndDate);
      nProspScanCount = CharToMDY(&nProspMonth, &nProspDay, &nProspYear, szProspDate);

      if ((nStartScanCount < 1) || (nEndScanCount < 1) || (nProspScanCount < 1))
        return false;
    
      if (m_nPrecision == YEAR || nStartScanCount == 1)
         nStartMonth = 1;
      if (m_nPrecision == YEAR || nEndScanCount == 1)
         nEndMonth = 1;
      if (m_nPrecision == YEAR || nProspScanCount == 1)
         nProspMonth = 1;
    
      if (m_nPrecision == YEAR || m_nPrecision == MONTH)
         {
         if (nStartScanCount == 1 || nStartScanCount == 2)
           nStartDay = 1;
         if (nEndScanCount == 1 || nEndScanCount == 2)
           nEndDay = DaysThisMonth(nEndYear, nEndMonth);
         if (nProspScanCount == 1 || nProspScanCount == 2)
           nProspDay = 1;
         }
    
      if (!(IsDateValid(nStartMonth, nStartDay, nStartYear) && IsDateValid(nEndMonth, nEndDay, nEndYear) && IsDateValid(nProspMonth, nProspDay, nProspYear)))
        bReturnValue = false;
      else
         {
         //Prospective surveillance start date must be between study dates.
         // can be equal to one of them...
         //compute Julian date and check ranges...
         bReturnValue = CheckProspDateRange(nStartYear, nStartMonth, nStartDay,
                                  nEndYear, nEndMonth, nEndDay,
                                  nProspYear, nProspMonth, nProspDay);
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("ValidateDateString()", "CParameters");
      throw;
      }
   return bReturnValue;
}

bool CParameters::CheckProspDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                  int iEndYear, int iEndMonth, int iEndDay,
                                  int iProspYear, int iProspMonth, int iProspDay)
{
   bool bRangeOk = true;
   Julian Start, End, Prosp;
   
   Start = MDYToJulian(iStartMonth, iStartDay, iStartYear);
   End   = MDYToJulian(iEndMonth, iEndDay, iEndYear);
   Prosp = MDYToJulian(iProspMonth, iProspDay, iProspYear);
   if ((Prosp < Start) || (Prosp > End))
      bRangeOk = false;

   return bRangeOk;
}

bool CParameters::ValidateDateString(char* szDate, int nDateType)
{
   UInt nYear, nMonth, nDay;
   int  nScanCount;
   bool bReturnValue;

   try
      {
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
        bReturnValue = false;
      else
        bReturnValue = true;
      }
   catch (SSException & x)
      {
      x.AddCallpath("ValidateDateString()", "CParameters");
      throw;
      }
   return bReturnValue;
}

bool CParameters::ValidateReplications(int nReps)
{
  return (nReps >= 0) && (nReps == 0 || nReps == 9 || nReps == 19 || fmod(nReps+1, 1000) == 0.0);
}

void CParameters::SetDisplayParameters(bool bValue)
{
   m_bDisplayErrors = bValue;
}

bool CParameters::DisplayParamError(int nLine)
{
  // do not throw an exception here...
  // want the entire parameter file verified...
  // do not want "the first" bad parameter thrown...
  //
  // app will throw an exception up return from the main verifyparameter function.
  if (m_bDisplayErrors)
     {
     gpPrintDirection->SatScanPrintWarning("  Error: Invalid value in parameter file, line %i.\n", nLine);
     gpPrintDirection->SatScanPrintWarning("    Variable:  %s", mgsVariableLabels[nLine - 1]);
     }
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
  m_nNumEllipses           = 0;
  mp_dEShapes              = 0;
  mp_nENumbers             = 0;
  //m_nExtraParam4           = 0;
  strcpy(m_szProspStartDate, "");
  m_bOutputCensusAreas     = false;
  m_bMostLikelyClusters    = false;
  m_iCriteriaSecondClusters = 0;
  m_lTotalNumEllipses      = 0;

   //need to convert old parameter analysis settings to new ones..
  if (m_nAnalysisType == 2) m_nAnalysisType = 3;
  else if (m_nAnalysisType == 3) m_nAnalysisType = 2;
}

//**************************************************************************
// THIS ASSUMES THAT IT IS AN OLD SESSION TYPE...
// IF NUMBER OF ELLIPSES IS GREATER THAN ZERO, THEN SOMEONE HAS ALTERED
// AN OLD SESSION TO RUN WITH ELLIPSES AND IT SHOULD WORK WITH THEM...
//
//  SO IF NUMELLIPSES > 0, THEN LEAVE SHAPES, NUMBERS, TOTAL ALONE
//**************************************************************************
void CParameters::SetDefaultsV3()
{
  if (m_nNumEllipses == 0)
     {
     //m_nNumEllipses           = 0;
     mp_dEShapes              = 0;
     mp_nENumbers             = 0;
     m_lTotalNumEllipses      = 0;
     m_iCriteriaSecondClusters = 0;
     }
  else // someone has set up some old session to run with ellipses...
     {
     // must set criteria for ellipses to no restrictions....
     m_iCriteriaSecondClusters = 5;
     }
  strcpy(m_szProspStartDate, "");
  m_bOutputCensusAreas     = false;
  m_bMostLikelyClusters    = false;


  //need to convert old parameter analysis settings to new ones..
  if (m_nAnalysisType == 2) m_nAnalysisType = 3;
  else if (m_nAnalysisType == 3) m_nAnalysisType = 2;
}

bool CParameters::SaveParameters(char* szFilename)
{
   FILE* pFile;
   int   i;

   try
      {
      //gpPrintDirection->SatScanPrintf("Saving Parameters.\n");

      if ((pFile = fopen(szFilename, "w")) == NULL)
        SSGenerateException("  Error: Unable to open parameter file.", "SaveParameters");
        
      if (strlen(m_szGridFilename) > 0)
        m_bSpecialGridFile = true;
      else
        m_bSpecialGridFile = false;
    
     // m_nExtraParam1 = 0;
     // m_nExtraParam2 = 0;
     // m_nExtraParam3 = 0;
     // m_nExtraParam4 = 0;
          
      fprintf(pFile, "%i                     // Analysis Type (1=S, 2=T, 3=RST, 4=PST)\n", m_nAnalysisType);
      fprintf(pFile, "%i                     // Scan Areas (1=High, 2=Low, 3=HighLow)\n", m_nAreas);
      fprintf(pFile, "%s\n", m_szCaseFilename);
      fprintf(pFile, "%s\n", m_szPopFilename);
      fprintf(pFile, "%s\n", m_szCoordFilename);
      fprintf(pFile, "%s\n", m_szOutputFilename);
      fprintf(pFile, "%i                     // Precision of Case Times (0=None, 1=Year, 2=Month, 3=Day)\n", m_nPrecision);
      fprintf(pFile, "%i                     // Not Applicable \n", m_nDimension);
      fprintf(pFile, "%i                     // Use Special Grid File? (0=No, 1=Yes)\n", m_bSpecialGridFile);
      fprintf(pFile, "%s\n", m_szGridFilename);
      fprintf(pFile, "%f                    // Max Geographic Size (<=50%)\n", m_nMaxGeographicClusterSize);
      fprintf(pFile, "%s              // Start Date (YYYY/MM/DD)\n", m_szStartDate);
      fprintf(pFile, "%s            // End Date (YYYY/MM/DD)\n", m_szEndDate);
      fprintf(pFile, "%i                     // Alive Cluster Only? (0=No, 1=Yes)\n", m_bAliveClustersOnly);
      fprintf(pFile, "%i                     // Exact Times? (0)\n", m_bExactTimes);
      fprintf(pFile, "%i                     // Interval Units (0=None, 1=Year, 2=Month, 3=Day)\n", m_nIntervalUnits);
      fprintf(pFile, "%i                     // Interval Length (positive integer)\n", m_nIntervalLength);
      fprintf(pFile, "%i                     // Include Pure Spatial Clusters? (0=No, 1=Yes)\n", m_bIncludePurelySpatial);
      fprintf(pFile, "%f                     // Max Temporal Size (<=90%)\n", m_nMaxTemporalClusterSize);
      fprintf(pFile, "%i                     // Monte Carlo Replications (0, 9, 999, n999)\n", m_nReplicas);
      fprintf(pFile, "%i                     // Probability Model (0=Poisson, 1=Bernoulli or Space-Time Permutation=2)\n", m_nModel);
      fprintf(pFile, "%i                     // Isotonic Scan (0=standard, 1=isotonic)\n", m_nRiskFunctionType);
      fprintf(pFile, "%i                     // P-Values for Two Prespecified LLR's (0=No, 1=Yes)\n", m_bPowerCalc);
      fprintf(pFile, "%f              // LLR #1\n", m_nPower_X);
      fprintf(pFile, "%f              // LLR #2\n", m_nPower_Y);
      fprintf(pFile, "%i                     // Time Trend Adjustment Type (0=None, 1=Nonparametric, 2=Linear)\n", m_nTimeAdjustType);
      fprintf(pFile, "%f                     // Time Trend Percentage (>-100)\n", m_nTimeAdjPercent);
      fprintf(pFile, "%i                     // Include Purely Temporal Clusters? (0=No, 1=Yes)\n", m_bIncludePurelyTemporal);
      fprintf(pFile, "%s\n", m_szControlFilename);
      fprintf(pFile, "%i                     // Coordinates Type (0=Cartesian, 1=Lat/Lon)\n", m_nCoordType);
      fprintf(pFile, "%i                     // Output File: Simulated LLR's? (0=No, 1=Yes)\n", m_bSaveSimLogLikelihoods);
      fprintf(pFile, "%i                     // Sequential Scan (0=No, 1=Yes)\n", m_bSequential);
      fprintf(pFile, "%i                     // Sequential Scan, Max Iterations (1-32,000)\n", m_nAnalysisTimes);
      fprintf(pFile, "%f              // Sequential Scan, Max P-Value (0-1)\n", m_nCutOffPVal);
      fprintf(pFile, "%i                     // Validate Parameters? (0=No, 1=Yes)\n", m_bValidatePriorToCalc);
      fprintf(pFile, "%i                     // Output File:  RRs for Census Areas (0=No, 1=Yes)\n",m_bOutputRelRisks);
      fprintf(pFile, "%i                     // Number of Ellipses\n", m_nNumEllipses);
      for (i = 0; i < m_nNumEllipses; i++)
         fprintf(pFile, "%f ", mp_dEShapes[i]);
      fprintf(pFile, "                   // Ellipse Shapes\n");
      for (i = 0; i < m_nNumEllipses; i++)
         fprintf(pFile, "%i ", mp_nENumbers[i]);
      fprintf(pFile, "                 // Ellipse Angles\n");
      fprintf(pFile, "%s            // Prospective surveillance start date (YYYY/MM/DD). \n", m_szProspStartDate);
      fprintf(pFile, "%i                 // Output File: Census Areas in Reported Clusters\n", m_bOutputCensusAreas);
      fprintf(pFile, "%i                 // Output File: Clusters in Column Format\n", m_bMostLikelyClusters);
      fprintf(pFile, "%i                 //Criteria for Reporting Secondary Clusters\n", m_iCriteriaSecondClusters);

      fprintf(pFile, "%i                 //How Max Temporal Size Should Be Interperated - enum {PERCENTAGETYPE=0, TIMETYPE}\n", m_nMaxClusterSizeType);

      fclose(pFile);
       }
   catch (SSException & x)
      {
      x.AddCallpath("SavesParameter(char *)", "CParameters");
      throw;
      }
   return true;
}

void CParameters::SetPrintDirection(BasePrint *pPrintDirection)
{
   gpPrintDirection = pPrintDirection;
}

void CParameters::DisplayParameters(FILE* fp)
{
   int i;

   try
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
       case PURELYTEMPORAL : fprintf(fp, "Purely Temporal\n"); break;
       case SPACETIME      : fprintf(fp, "Retrospective Space-Time\n"); break;
       case PROSPECTIVESPACETIME: fprintf(fp, "Prospective Space-Time\n"); break;
     }
   
     fprintf(fp, "  Probability Model   : ");
     switch (m_nModel)
     {
       case POISSON   : fprintf(fp, "Poisson\n"); break;
       case BERNOULLI : fprintf(fp, "Bernoulli\n"); break;
       case SPACETIMEPERMUTATION : fprintf(fp, "Space-Time Permutation\n"); break;
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
   
     if (m_nNumEllipses > 0)
        {
        fprintf(fp, "\nEllipses\n");
        fprintf(fp, "----------\n");
        fprintf(fp, "Number of Ellipse Shapes Requested:  %i\n", m_nNumEllipses);
        fprintf(fp, "Shape for Each Ellipse:  ");
        for (i = 0; i < m_nNumEllipses; i++)
           fprintf(fp, "%7.3f ", mp_dEShapes[i]);
        fprintf(fp, "\nNumber of Angles for Each Ellipse Shape:  ");
        for (i = 0; i < m_nNumEllipses; i++)
           fprintf(fp, "%i ", mp_nENumbers[i]);
        }
     fprintf(fp, "\n\nScanning Window\n");
     fprintf(fp, "---------------\n");
   
     if (m_nAnalysisType == PURELYSPATIAL || m_nAnalysisType == SPACETIME || m_nAnalysisType == PROSPECTIVESPACETIME)
       fprintf(fp, "  Maximum Spatial Cluster Size : %.2f\n", m_nMaxGeographicClusterSize);
     if ((m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME))
     {
       fprintf(fp, "  Also Include Purely Temporal Clusters : ");
       switch (m_bIncludePurelyTemporal)
       {
         case true  : fprintf(fp, "Yes\n"); break;
         case false : fprintf(fp, "No\n");  break;
       }
     }
   
     if (m_nAnalysisType == PURELYTEMPORAL || m_nAnalysisType == SPACETIME || (m_nAnalysisType == PROSPECTIVESPACETIME))
       fprintf(fp, "  Maximum Temporal Cluster Size : %.2f\n", m_nMaxTemporalClusterSize);
     if ((m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME))
     {
       fprintf(fp, "  Also Include Purely Spatial Clusters : ");
       switch (m_bIncludePurelySpatial)
       {
         case true  : fprintf(fp, "Yes\n"); break;
         case false : fprintf(fp, "No\n");  break;
       }
     }

     //The "Clusters to Include" do not apply to PROSPECTIVESPACETIME
     if (m_nAnalysisType == PURELYTEMPORAL || m_nAnalysisType == SPACETIME)
     {
       fprintf(fp, "  Clusters to Include : ");
       switch (m_bAliveClustersOnly)
       {
         case ALLCLUSTERS   : fprintf(fp, "All\n"); break;
         case ALIVECLUSTERS : fprintf(fp, "Only those including the study end date\n"); break;
       }
     }
   
     if (m_nAnalysisType == PURELYTEMPORAL || m_nAnalysisType == SPACETIME || (m_nAnalysisType == PROSPECTIVESPACETIME))
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
     if (m_nAnalysisType == PROSPECTIVESPACETIME)
        fprintf(fp, "  Prospective Start Date : %s\n", m_szProspStartDate);
   
     fprintf(fp, "\nOutput\n");
     fprintf(fp, "------\n");
     fprintf(fp, "  Results File : %s\n", m_szOutputFilename);
     if (m_bOutputCensusAreas)  // Output Census areas in Reported Clusters
        fprintf(fp, "  GIS File     : %s\n", m_szGISFilename);
     if (m_bMostLikelyClusters)  // Output Most Likely Cluster for each Centroid
        fprintf(fp, "  MLC File     : %s\n", m_szMLClusterFilename);
     if (m_bOutputRelRisks)
        fprintf(fp, "  RRE File     : %s\n", m_szRelRiskFilename);
     if (m_bSaveSimLogLikelihoods)
       fprintf(fp, "  LLR File     : %s\n", m_szLLRFilename);

     fprintf(fp, "  Criteria for Reporting Secondary Clusters : ");
     switch (m_iCriteriaSecondClusters)
     {
     case NOGEOOVERLAP          : fprintf(fp, "No Geographical Overlap\n"); break;
     case NOCENTROIDSINOTHER    : fprintf(fp, "No Cluster Centroids in Other Clusters\n"); break;
     case NOCENTROIDSINMORELIKE : fprintf(fp, "No Cluster Centroids in More Likely Clusters\n"); break;
     case NOCENTROIDSINLESSLIKE : fprintf(fp, "No Cluster Centroids in Less Likely Clusters\n"); break;
     case NOPAIRSINEACHOTHERS   : fprintf(fp, "No Pairs of Centroids Both in Each Others Clusters\n"); break;
     case NORESTRICTIONS        : fprintf(fp, "No Restrictions = Most Likely Cluster for Each Centroid\n"); break;
     }
     fprintf(fp, "\n________________________________________________________________\n");
      }
   catch (SSException & x)
      {
      x.AddCallpath("DisplayParameters(FILE *)", "CParameters");
      throw;
      }
}

void CParameters::DisplayAnalysisType(FILE* fp)
{
   try
      {
      switch (m_nAnalysisType)
      {
       case PURELYSPATIAL  : fprintf(fp, "Purely Spatial analysis\n"); break;
       case PURELYTEMPORAL : fprintf(fp, "Purely Temporal analysis\n"); break;
       case SPACETIME      : fprintf(fp, "Retrospective Space-Time analysis\n"); break;
       case PROSPECTIVESPACETIME: fprintf(fp, "Prospective Space-Time analysis\n"); break;
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
      else if (m_nModel == SPACETIMEPERMUTATION)
        fprintf(fp, " using the Space-Time Permutation model.\n");
      else
        fprintf(fp, " using unspecified model.\n");


      if ((m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME))
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
    
       }
   catch (SSException & x)
      {
      x.AddCallpath("DisplayAnalysisType(FILE *)", "CParameters");
      throw;
      }
}


void CParameters::DisplayTimeAdjustments(FILE* fp)
{
   try
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
   catch (SSException & x)
      {
      x.AddCallpath("DisplayTimeAdjustments(FILE *)", "CParameters");
      throw;
      }
}


