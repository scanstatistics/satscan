//*****************************************************************************
#ifndef __PARAMETERS_H
#define __PARAMETERS_H
//*****************************************************************************
#include "SatScan.h"
#include "JulianDates.h"

#define PARAMETERS 43

enum { ANALYSISTYPE=1, SCANAREAS, CASEFILE, POPFILE, COORDFILE, OUTPUTFILE, PRECISION,
       DIMENSION, SPECIALGRID, GRIDFILE, GEOSIZE, STARTDATE, ENDDATE,
       CLUSTERS, EXACTTIMES, INTERVALUNITS, TIMEINTLEN, PURESPATIAL,
       TIMESIZE, REPLICAS, //ACCEPT, INTRO };
       MODEL, RISKFUNCTION, POWERCALC, POWERX, POWERY,
       TIMETREND, TIMETRENDPERC, PURETEMPORAL, CONTROLFILE, COORDTYPE, SAVESIMLL,
       SEQUENTIAL, SEQNUM, SEQPVAL,
       VALIDATE, OUTPUTRR, ELLIPSES, ESHAPES, ENUMBERS, START_PROSP_SURV,
        OUTPUT_CENSUS_AREAS, OUTPUT_MOST_LIKE_CLUSTERS, CRITERIA_SECOND_CLUSTERS};

enum {PURELYSPATIAL=1, PURELYTEMPORAL, SPACETIME,  PROSPECTIVESPACETIME, PURELYSPATIALMONOTONE};  // Analysis,Cluster Type
enum {POISSON=0, BERNOULLI};            // Model
enum {ALLCLUSTERS=0, ALIVECLUSTERS};   // Clusers
enum {STANDARDRISK=0, MONOTONERISK};    // Risk
enum {NONSEQUENTIAL=0, SEQUENTIALANALYSIS};    // Analysis
enum {HIGH=1, LOW, HIGHANDLOW};             // Rates
enum {NOTADJUSTED=0, NONPARAMETRIC, LINEAR}; // Time Trends
enum {CARTESIAN=0, LATLON};                 // Coords Type
enum {NOGEOOVERLAP, NOCENTROIDSINOTHER, NOCENTROIDSINMORELIKE,
      NOCENTROIDSINLESSLIKE, NOPAIRSINEACHOTHERS, NORESTRICTIONS }; // Criteria for reporting secondary clusters

class CParameters
{
  private:
     BasePrint *gpPrintDirection;         /** where to direct 'console' output */
  public:
    CParameters(bool bDisplayErrors);
    CParameters(const CParameters &other);
    ~CParameters();

    int     m_nNumEllipses;               /** Number of ellipsoids requested */
    double *mp_dEShapes;                  /** Shape of each ellipsoid */
    int    *mp_nENumbers;                 /** Number of rotations for each ellipsoid */
    long    m_lTotalNumEllipses;          /** Total number of Ellipses (ellipses by each shape) */

    int    m_nAnalysisType;               /** Analysis (PS, PT, ST-Retro, ST-Prospective). */
    int    m_nAreas;                      /** Scan for high, low, high&low areas. */
    int    m_nModel;                      /** Poisson or Bernoulli. */
    int    m_nRiskFunctionType;           /** Standard or Monotone. */
    int    m_nReplicas;                   /** Number of MonteCarlo replicas. */

    bool   m_bPowerCalc;
    double m_nPower_X;
    double m_nPower_Y;

    // Study dates
    char   m_szStartDate [MAX_STR_LEN];   /** Character start date (YYYY/MM/DD). */
    char   m_szEndDate [MAX_STR_LEN];     /** Character end date (YYYY/MM/DD). */

    // Spatial options
    float  m_nMaxGeographicClusterSize;

    // Temporal options
    float  m_nMaxTemporalClusterSize;
    bool   m_bAliveClustersOnly;          /** Use alive clusters only? */

    int    m_nIntervalUnits;              /** Interval Units (0=None, 1=Year, 2=Month, 3=Day) */
    long   m_nIntervalLength;

    int    m_nTimeAdjustType;             /** Adjust for time trend: no, discrete, % */
    double m_nTimeAdjPercent;

    // Combined temporal and spatial options (Space-Time analysis only)
    bool   m_bIncludePurelySpatial;
    bool   m_bIncludePurelyTemporal;

    // Data
    char   m_szCaseFilename [MAX_STR_LEN];
    char   m_szControlFilename [MAX_STR_LEN];
    char   m_szPopFilename [MAX_STR_LEN];
    char   m_szCoordFilename [MAX_STR_LEN];
    char   m_szGridFilename [MAX_STR_LEN];

    bool   m_bSpecialGridFile; 

    int    m_nPrecision;                  /** Precision of case data: none, years, mon, days. */
    int    m_nDimension;                  /** Dimensions in geographic data */

    int    m_nCoordType;                  /** Coordinates Type (0=Cartesian, 1=Lat/Lon) */

    // Results
    char   m_szOutputFilename [MAX_STR_LEN];   /** results file name */
    char   m_szGISFilename [MAX_STR_LEN];      /** output Census areas in reported clusters */
    char   m_szLLRFilename [MAX_STR_LEN];      /** simulated log likelihood ratios */
    char   m_szMLClusterFilename[MAX_STR_LEN]; /** most likely cluster for each centroid */
    char   m_szRelRiskFilename[MAX_STR_LEN];   /** relative risk estimates for each census area */

    bool   m_bSaveSimLogLikelihoods;
    bool   m_bOutputRelRisks;

   // Sequential Analysis
    bool      m_bSequential;        // Sequential analysis? T/F
    int       m_nAnalysisTimes;     // Count used to exit seq analysis
    double    m_nCutOffPVal;        // P-Value used to exit seq analysis

    // Internal options
    bool   m_bExactTimes;
    int    m_nClusterType;      // Is this used? KR-980606

    bool   m_bValidatePriorToCalc;


    //int    m_nExtraParam4;
    char     m_szProspStartDate[MAX_STR_LEN]; /** Character Prospective start date (YYYY/MM/DD). */
    bool     m_bOutputCensusAreas;            /** Output Census areas in Reported Clusters */
    bool     m_bMostLikelyClusters;           /** Output Most Likely Cluster for each Centroid */
    int      m_iCriteriaSecondClusters;       /** Criteria for Reporting Secondary Clusters */

    bool   m_bDisplayErrors;

    void Free();

    int  LoadEAngles(const char* szParam);
    int  LoadEShapes(const char* szParam);
    
    void SetDefaults();
    void SetDefaultsV2();
    void SetDefaultsV3();
    bool SetParameters(const char* szFilename);
    bool SetParameter(int nParam, const char* szParam);
    bool SetGISFilename();
    bool SetLLRFilename();
    bool SetMLCFilename();
    bool SetRelRiskFilename();

    bool ValidateParameters();
    bool ValidateDateString(char* szDate, int nDateType);
    bool ValidateReplications(int nReps);
    bool ValidateProspectiveStartDate(char* szProspDate, char *szStartDate, char *szEndDate);
    bool CheckProspDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                  int iEndYear, int iEndMonth, int iEndDay,
                                  int iProspYear, int iProspMonth, int iProspDay);

    bool DisplayParamError(int nLine);
    void DisplayParameters(FILE* fp);
    void DisplayAnalysisType(FILE* fp);
    void DisplayTimeAdjustments(FILE* fp);

    bool SaveParameters(char* szFilename);
    void SetDisplayParameters(bool bValue);

    //Overloaded operators
   CParameters &operator= (const CParameters &rhs);
    void SetPrintDirection(BasePrint *pPrintDirection);
   private:
      void copy(const CParameters &rhs);
      void FindDelimiter(char *sString, char cDelimiter);
      void TrimLeft(char *sString);
};

//*****************************************************************************
#endif
