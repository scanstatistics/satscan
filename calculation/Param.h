// param.h
// This change was made at 6:18

#ifndef __PARAM_H
#define __PARAM_H

#include "basic.h"

#define PARAMETERS 40

enum { ANALYSISTYPE=1, SCANAREAS, CASEFILE, POPFILE, COORDFILE, OUTPUTFILE, PRECISION,
       DIMENSION, SPECIALGRID, GRIDFILE, GEOSIZE, STARTDATE, ENDDATE, 
       CLUSTERS, EXACTTIMES, INTERVALUNITS, TIMEINTLEN, PURESPATIAL, 
       TIMESIZE, REPLICAS, //ACCEPT, INTRO };
       MODEL, RISKFUNCTION, POWERCALC, POWERX, POWERY,
       TIMETREND, TIMETRENDPERC, PURETEMPORAL, CONTROLFILE, COORDTYPE, SAVESIMLL,
       SEQUENTIAL, SEQNUM, SEQPVAL,
       VALIDATE, OUTPUTRR, EXTRA1, EXTRA2, EXTRA3, EXTRA4 };

enum {PURELYSPATIAL=1, SPACETIME, PURELYTEMPORAL, PURELYSPATIALMONOTONE};  // Analysis,Cluster Type
enum {POISSON=0, BERNOULLI};            // Model
enum {ALLCLUSTERS=0, ALIVECLUSTERS};   // Clusers
enum {STANDARDRISK=0, MONOTONERISK};    // Risk
enum {NONSEQUENTIAL=0, SEQUENTIALANALYSIS};    // Analysis
enum {HIGH=1, LOW, HIGHANDLOW};             // Rates
enum {NOTADJUSTED=0, NONPARAMETRIC, LINEAR}; // Time Trends
enum {CARTESIAN=0, LATLON};                 // Coords Type

class CParameters
{
  public:
    CParameters(bool bDisplayErrors);
    ~CParameters() {};

    int    m_nAnalysisType;               // Analysis (PS, PT, ST).
    int    m_nAreas;                      // Scan for high, low, high&low areas.
    int    m_nModel;                      // Poisson or Bernoulli.
    int    m_nRiskFunctionType;           // Standard or Monotone.
    int    m_nReplicas;                   // Number of MonteCarlo replicas.

    bool   m_bPowerCalc;
    double m_nPower_X;
    double m_nPower_Y;

    // Study dates
    char   m_szStartDate [MAX_STR_LEN]; // Character start date (YYYY/MM/DD).
    char   m_szEndDate [MAX_STR_LEN];   // Character end date (YYYY/MM/DD).

    // Spatial options
    float  m_nMaxGeographicClusterSize;

    // Temporal options
    float  m_nMaxTemporalClusterSize;
    bool   m_bAliveClustersOnly;        // Use alive clusters only?

    int    m_nIntervalUnits;           // Intervals in years, months or days.
    long   m_nIntervalLength;

    int    m_nTimeAdjustType;          // Adjust for time trend: no, discrete, %
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

    int    m_nPrecision;       // Precision of case data: none, years, mon, days.
    int    m_nDimension;       // Dimensions in geographic data
   
    int    m_nCoordType;

    // Results
    char   m_szOutputFilename [MAX_STR_LEN];
    char   m_szGISFilename [MAX_STR_LEN];
    char   m_szLLRFilename [MAX_STR_LEN];

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

    int    m_nExtraParam1;
    int    m_nExtraParam2;
    int    m_nExtraParam3;
    int    m_nExtraParam4;

    bool   m_bDisplayErrors;


    void SetDefaults();
    void SetDefaultsV2();
    bool SetParameters(const char* szFilename);
    bool SetParameter(int nParam, const char* szParam);
    bool SetGISFilename();
    bool SetLLRFilename();

    bool ValidateParameters();
    bool ValidateDateString(char* szDate, int nDateType);
    bool ValidateReplications(int nReps);

    bool DisplayParamError(int nLine);
    void DisplayParameters(FILE* fp);
    void DisplayAnalysisType(FILE* fp);
    void DisplayTimeAdjustments(FILE* fp);

    bool SaveParameters(char* szFilename);

};

#endif
