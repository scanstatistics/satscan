//*****************************************************************************
#ifndef __CLUSTER_H
#define __CLUSTER_H
//*****************************************************************************
#include "SaTScan.h"
#include "SaTScanData.h"
#include "IncidentRate.h"

class stsAreaSpecificData;

class CCluster {
  protected:
    BasePrint *                 gpPrintDirection;
    stsAreaSpecificData*        gpAreaData;
  public:
    CCluster(BasePrint *pPrintDirection);
    virtual ~CCluster();

    float       gfPValue;               // p value of the cluster

    tract_t    m_Center;             // Center of cluster (index to grid)

    int        m_iEllipseOffset;     // Link to Circle or Ellipse (top cluster)

    count_t    m_nCases;             // Number of cases in cluster
    measure_t  m_nMeasure;           // Expected count for cluster
    tract_t    m_nTracts;            // Number of neighboring tracts in cluster
    double     m_nRatio;             // Likelihood ratio
    double     m_nLogLikelihood;     // Log Likelihood
    long       m_nRank;              // Rank based on results of simulations

    tract_t    m_nSteps;             // Number of concentric steps in cluster
                                     //  (Monotone)
    // Temporal variables
    int        m_nFirstInterval;     // Index # of first time interval
    int        m_nLastInterval;      // Index # of last time interval
    Julian     m_nStartDate;         // Start time of cluster
    Julian     m_nEndDate;           // End time of cluster

    // Flag variables
    bool       m_bClusterInit;       // Has cluster been initialized?
    bool       m_bClusterDefined;    // Has cluster been defined? (tracts, TI's)
    bool       m_bClusterSet;        // Have all values (ratio, dts) been set?
    bool       m_bLogLSet;           // Has the loglikelihood be assigned?
    bool       m_bRatioSet;          // Has the loglikelihood ratio been set?
    int        m_nClusterType;       // Type of cluster

    RATE_FUNCPTRTYPE m_pfRateOfInterest;

    CCluster& operator =(const CCluster& cluster);
    virtual void Initialize(tract_t nCenter=0);

    virtual void        AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n) {};
    virtual bool        ClusterDefined() {return (m_bClusterDefined==true);}
    double              ConvertAngleToDegrees(double dAngle);
    virtual void        Display(FILE*     fp,
                                const     CParameters& Parameters,
                                const CSaTScanData& Data,
                                int       nCluster,
                                 measure_t nMinMeasure);
    virtual void        DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                            int nCluster,  measure_t nMinMeasure, int nReplicas,
                                            long lReportHistoryRunNumber,
                                            bool bIncludeRelRisk, bool bIncludePVal,
                                            int nLeftMargin, int nRightMargin,
                                            char cDeliminator, char* szSpacesOnLeft,
                                            bool bFormat = true);
    void                DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& Data,
                                                tract_t nFirstTract, tract_t nLastTract,
                                                int nCluster, measure_t nMinMeasure, int nReplicas,
                                                long lReportHistoryRunNumber,
                                                bool bIncludeRelRisk, bool bIncludePVal,
                                                int nLeftMargin, int nRightMargin,
                                                char cDeliminator, char* szSpacesOnLeft,
                                                bool bFormat = true);
    virtual void        DisplayCoordinates(FILE* fp, const CSaTScanData& Data,
                                        int nLeftMargin, int nRightMargin,
                                        char cDeliminator, char* szSpacesOnLeft);
    virtual void        DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data,
                                        int nLeftMargin, int nRightMargin,
                                        char cDeliminator, char* szSpacesOnLeft);
    virtual void        DisplayPopulation(FILE* fp, const CSaTScanData& Data, char* szSpacesOnLeft);

    virtual void        DisplayPVal(FILE* fp, int nReplicas, char* szSpacesOnLeft);
    virtual void        DisplayRelativeRisk(FILE* fp, double nMeasureAdjustment,
                                        int nLeftMargin, int nRightMargin,
                                        char cDeliminator, char* szSpacesOnLeft);
    virtual void        DisplaySteps(FILE* fp, char* szSpacesOnLeft) {};
    virtual void        DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType);

    virtual count_t     GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const {return 0;}
    int                 GetClusterType()   {return m_nClusterType;}
    double              GetLogLikelihood() {return m_nLogLikelihood;}
    virtual measure_t   GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const {return 0;}
    tract_t             GetNumCircles()    {return m_nSteps;}
    virtual tract_t     GetNumTractsInnerCircle() { return m_nTracts; }
    double              GetPVal(int nReps) {return (double)m_nRank/(double)(nReps+1);}
    double              GetRelativeRisk(double nMeasureAdjustment);
    virtual double      GetRelativeRiskForTract(tract_t tTract, const CSaTScanData & Data) const;

    bool                RateIsOfInterest(count_t nTotalCases, measure_t nTotalMeasure);

    void                SetAreaReport(stsAreaSpecificData* pAreaData) { gpAreaData = pAreaData; }
    void                SetCenter(tract_t nCenter);
    void                SetEllipseOffset(int iOffset);
    double              SetLogLikelihood(double nLogLikelihood) {m_nLogLikelihood = nLogLikelihood; return m_nLogLikelihood;}
    void                SetRate(int nRate);
    double              SetRatio(double nLogLikelihoodForTotal);
    void                SetRatioAndDates(const CSaTScanData& Data);
    virtual void        SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);

    void                WriteCoordinates(FILE* fp, CSaTScanData* pData);
    void                WriteLatLongCoords(FILE* fp, CSaTScanData* pData);

    //    double  GetPopulation(const CSaTScanData& Data);
    //  double  GetRelativeRisk()  {return ((double)(m_nCases))/m_nMeasure;};
   //    double SetLogLikelihood(count_t N, measure_t U);
};

//*****************************************************************************
#endif
