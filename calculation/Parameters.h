//*****************************************************************************
#ifndef __PARAMETERS_H
#define __PARAMETERS_H
//*****************************************************************************
#include "SatScan.h"
#include "JulianDates.h"

#define PARAMETERS 48

extern const char*      ANALYSIS_HISTORY_FILE;

enum {ANALYSISTYPE=1, SCANAREAS, CASEFILE, POPFILE, COORDFILE, OUTPUTFILE, PRECISION,
      DIMENSION, SPECIALGRID, GRIDFILE, GEOSIZE, STARTDATE, ENDDATE,
      CLUSTERS, EXACTTIMES, INTERVALUNITS, TIMEINTLEN, PURESPATIAL,
      TIMESIZE, REPLICAS, //ACCEPT, INTRO };
      MODEL, RISKFUNCTION, POWERCALC, POWERX, POWERY,
      TIMETREND, TIMETRENDPERC, PURETEMPORAL, CONTROLFILE, COORDTYPE, SAVESIMLL,
      SEQUENTIAL, SEQNUM, SEQPVAL,
      VALIDATE, OUTPUTRR, ELLIPSES, ESHAPES, ENUMBERS, START_PROSP_SURV,
      OUTPUT_CENSUS_AREAS, OUTPUT_MOST_LIKE_CLUSTERS, CRITERIA_SECOND_CLUSTERS,
      MAX_TEMPORAL_TYPE,MAX_SPATIAL_TYPE, RUN_HISTORY_FILENAME, OUTPUTCLUSTERDBF, OUTPUTAREADBF};
enum {PURELYSPATIAL=1, PURELYTEMPORAL, SPACETIME,  PROSPECTIVESPACETIME, PURELYSPATIALMONOTONE}; //analysis, clusters
enum {POISSON=0, BERNOULLI, SPACETIMEPERMUTATION};
enum {ALLCLUSTERS=0, ALIVECLUSTERS};   // Clusers
enum {STANDARDRISK=0, MONOTONERISK};    // Risk
enum {NONSEQUENTIAL=0, SEQUENTIALANALYSIS};    // Analysis
enum {HIGH=1, LOW, HIGHANDLOW};             // Rates
enum {NOTADJUSTED=0, NONPARAMETRIC, LINEAR}; // Time Trends
enum {CARTESIAN=0, LATLON};                 // Coords Type
enum {NOGEOOVERLAP, NOCENTROIDSINOTHER, NOCENTROIDSINMORELIKE,
      NOCENTROIDSINLESSLIKE, NOPAIRSINEACHOTHERS, NORESTRICTIONS}; // Criteria for reporting secondary clusters
enum TemporalSizeType {PERCENTAGETYPE=0, TIMETYPE}; // How Max Temporal Size Should Be Interperated
enum SpatialSizeType {PERCENTAGEOFMEASURETYPE=0, DISTANCETYPE}; // How Max Temporal Size Should Be Interperated

class CParameters
{
  private:
      BasePrint *gpPrintDirection;         /** where to direct 'console' output */
      bool       gbOutputClusterLevelDBF, gbOutputAreaSpecificDBF;
      ZdString   gsRunHistoryFilename;

      void copy(const CParameters &rhs);
      void FindDelimiter(char *sString, char cDelimiter);
      void TrimLeft(char *sString);
      bool ValidHistoryFileName(const ZdString& sRunHistoryFilename);
      
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
    int    m_nModel;                      /** Poisson, Bernoulli or Space-Time Permutation. */
    int    m_nRiskFunctionType;           /** Standard or Monotone. */
    int    m_nReplicas;                   /** Number of MonteCarlo replicas. */

    bool   m_bPowerCalc;
    double m_nPower_X, m_nPower_Y;

    // Study dates
    char   m_szStartDate [MAX_STR_LEN];   /** Character start date (YYYY/MM/DD). */
    char   m_szEndDate [MAX_STR_LEN];     /** Character end date (YYYY/MM/DD). */

    // Spatial options
    float  m_nMaxGeographicClusterSize;
    int    m_nMaxSpatialClusterSizeType;  /** How Max Spatial Size Should Be Interperated - enum {PERCENTAGEOFMEASURETYPE=0, DISTANCETYPE} */

    // Temporal options
    float  m_nMaxTemporalClusterSize;
    bool   m_bAliveClustersOnly;          /** Use alive clusters only? */
    int    m_nMaxClusterSizeType;         /** How Max Temporal Size Should Be Interperated - enum {PERCENTAGETYPE=0, TIMETYPE} */

    int    m_nIntervalUnits;              /** Interval Units (0=None, 1=Year, 2=Month, 3=Day) */
    long   m_nIntervalLength;

    int    m_nTimeAdjustType;             /** Adjust for time trend: no, discrete, % */
    double m_nTimeAdjPercent;

    // Combined temporal and spatial options (Space-Time analysis only)
    bool   m_bIncludePurelySpatial, m_bIncludePurelyTemporal;

    // Data
    char   m_szCaseFilename [MAX_STR_LEN], m_szControlFilename [MAX_STR_LEN],
           m_szPopFilename [MAX_STR_LEN], m_szCoordFilename [MAX_STR_LEN], m_szGridFilename [MAX_STR_LEN];

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

    bool   m_bSaveSimLogLikelihoods, m_bOutputRelRisks;

   // Sequential Analysis
    bool      m_bSequential;        // Sequential analysis? T/F
    int       m_nAnalysisTimes;     // Count used to exit seq analysis
    double    m_nCutOffPVal;        // P-Value used to exit seq analysis

    // Internal options
    bool   m_bExactTimes, m_bValidatePriorToCalc, m_bDisplayErrors;
    int    m_nClusterType;      // Is this used? KR-980606

    //int    m_nExtraParam4;
    char     m_szProspStartDate[MAX_STR_LEN]; /** Character Prospective start date (YYYY/MM/DD). */
    bool     m_bOutputCensusAreas;            /** Output Census areas in Reported Clusters */
    bool     m_bMostLikelyClusters;           /** Output Most Likely Cluster for each Centroid */
    int      m_iCriteriaSecondClusters;       /** Criteria for Reporting Secondary Clusters */



    //Overloaded operators
    CParameters &operator= (const CParameters &rhs);
    

    bool                CheckProspDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                            int iEndYear, int iEndMonth, int iEndDay,
                                            int iProspYear, int iProspMonth, int iProspDay);
    void                ConvertMaxTemporalClusterSizeToType(TemporalSizeType eTemporalSizeType);
    void                DisplayAnalysisType(FILE* fp);
    bool                DisplayParamError(int nLine);
    void                DisplayParameters(FILE* fp);
    void                DisplayTimeAdjustments(FILE* fp);

    void                Free();
    const bool&         GetOutputClusterLevelDBF() const;
    const bool&         GetOutputAreaSpecificDBF() const;
    const ZdString&     GetRunHistoryFilename() const  {return gsRunHistoryFilename;}

    int                 LoadEAngles(const char* szParam);
    int                 LoadEShapes(const char* szParam);
    bool                SaveParameters(char* szFilename);

    void                SetDefaults();
    void                SetDefaultsV2();
    void                SetDefaultsV3();
    void                SetDisplayParameters(bool bValue);
    bool                SetParameters(const char* szFilename, bool bValidate=true);
    bool                SetParameter(int nParam, const char* szParam);
    bool                SetGISFilename();
    bool                SetLLRFilename();
    bool                SetMLCFilename();
    void                SetOutputClusterLevelDBF(const bool& bOutput);
    void                SetOutputAreaSpecificDBF(const bool& bOutput);
    void                SetPrintDirection(BasePrint *pPrintDirection);
    bool                SetRelRiskFilename();
    void                SetRunHistoryFilename(const ZdString& sFilename) {gsRunHistoryFilename = sFilename;}

    bool                ValidateParameters();
    bool                ValidateDateString(char* szDate, int nDateType);
    bool                ValidateReplications(int nReps);
    bool                ValidateProspectiveStartDate(char* szProspDate, char *szStartDate, char *szEndDate);

};

//*****************************************************************************
#endif
