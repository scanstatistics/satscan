//*****************************************************************************
#ifndef __CLUSTER_H
#define __CLUSTER_H
//*****************************************************************************
#include "SaTScan.h"
#include "SaTScanData.h"
#include "IncidentRate.h"
#include "DataStream.h"
#include "UtilityFunctions.h"
#include "MeasureDeterminant.h"

class stsAreaSpecificData;

class ClusterStreamData {
  private:
    void                                Init() {gTotalCases=0;gTotalMeasure=0;}

  public:
    ClusterStreamData() {Init(); InitializeData();}
    virtual ~ClusterStreamData() {}

    count_t                             gCases;
    measure_t                           gMeasure;
    measure_t                           gSqMeasure;

    count_t                             gTotalCases;
    measure_t                           gTotalMeasure;

    virtual void        InitializeData() {gCases=0;gMeasure=0;gSqMeasure=0;}
};

class AbstractTemporalClusterStreamData : public ClusterStreamData {
   public:
     count_t          * gpCases;
     measure_t        * gpMeasure;
     measure_t        * gpSqMeasure;

   public:
    AbstractTemporalClusterStreamData() {}
    virtual ~AbstractTemporalClusterStreamData() {}

    virtual AbstractTemporalClusterStreamData         * Clone() const {return new AbstractTemporalClusterStreamData(*this);}
};

typedef ZdPointerVector<AbstractTemporalClusterStreamData>  StreamDataContainer_t;
typedef StreamDataContainer_t::iterator                     StreamDataContainerIterator_t;

class CCluster {
  protected:
    BasePrint                 * gpPrintDirection;
    stsAreaSpecificData       * gpAreaData;

  public:
    CCluster(BasePrint *pPrintDirection);
    virtual ~CCluster();

    virtual CCluster          * Clone() const = 0;
    CCluster                  & operator=(const CCluster& rhs);

    virtual count_t             GetCaseCount(unsigned int iStream) const {return 0;}
    virtual measure_t           GetMeasure(unsigned int iStream) const {return 0;}
    virtual void                SetCaseCount(unsigned int iStream, count_t tCount) {}
    virtual void                SetMeasure(unsigned int iStream, measure_t tMeasure) {}

    float                       gfPValue;               // p value of the cluster
    tract_t                     m_Center;             // Center of cluster (index to grid)
    int                         m_iEllipseOffset;     // Link to Circle or Ellipse (top cluster)

    tract_t                     m_nTracts;            // Number of neighboring tracts in cluster
    double                      m_nRatio;             // Likelihood ratio
    double                      m_nLogLikelihood;     // Log Likelihood
    long                        m_nRank;              // Rank based on results of simulations
    double                      m_DuczmalCorrection;  // Duczmal compactness correction, for ellipses
    // Temporal variables
    int                         m_nFirstInterval;     // Index # of first time interval
    int                         m_nLastInterval;      // Index # of last time interval
    Julian                      m_nStartDate;         // Start time of cluster
    Julian                      m_nEndDate;           // End time of cluster
    // Flag variables
    bool                        m_bClusterDefined;    // Has cluster been defined? (tracts, TI's)
    RATE_FUNCPTRTYPE            m_pfRateOfInterest;

    MEAURE_DETERMINANT          gMeasure;
    MEAURE_DETERMINANT_         gMeasure_;
    _MEAURE_DETERMINANT_        g_Measure_;

    inline virtual void AssignAsType(const CCluster& rhs) {*this = rhs;}
    virtual bool        ClusterDefined() {return (m_bClusterDefined==true);}
    const double        ConvertAngleToDegrees(double dAngle) const;
    virtual void        Display(FILE* fp, const     CParameters& Parameters,
                                const CSaTScanData& Data, int nCluster,
                                measure_t nMinMeasure, int iNumSimulations);
    virtual void        DisplayAnnualTimeTrendWithoutTitle(FILE* fp) {/*stub - no action*/}
    virtual void        DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                            int nCluster,  measure_t nMinMeasure,
                                            int iNumSimulations, long lReportHistoryRunNumber,
                                            bool bIncludeRelRisk, bool bIncludePVal,
                                            int nLeftMargin, int nRightMargin,
                                            char cDeliminator, char* szSpacesOnLeft,
                                            bool bFormat = true);
    void                DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& Data,
                                                tract_t nFirstTract, tract_t nLastTract,
                                                int nCluster, measure_t nMinMeasure,
                                                int iNumSimulations, long lReportHistoryRunNumber,
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
    virtual void        DisplayNullOccurrence(FILE* fp, const CSaTScanData& Data, int iNumSimulations, char* szSpacesOnLeft);
    virtual void        DisplayPopulation(FILE* fp, const CSaTScanData& Data, char* szSpacesOnLeft);
    virtual void        DisplayPVal(FILE* fp, int nReplicas, char* szSpacesOnLeft);
    virtual void        DisplayRelativeRisk(FILE* fp, double nMeasureAdjustment,
                                        int nLeftMargin, int nRightMargin,
                                        char cDeliminator, char* szSpacesOnLeft);
    virtual void        DisplaySteps(FILE* fp, char* szSpacesOnLeft) {};
    virtual void        DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType);
    virtual void        DisplayTimeTrend(FILE* fp, char* szSpacesOnLeft) {/*stub - no action*/}
    virtual count_t     GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const {return 0;}
    virtual int         GetClusterType() const = 0;
    double              GetDuczmalCorrectedLogLikelihoodRatio() const;
    double              GetLogLikelihood() {return m_nLogLikelihood;}
    virtual measure_t   GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const {return 0;}
    virtual tract_t     GetNumTractsInnerCircle() { return m_nTracts; }
    const double        GetPVal(int nReps) const {return (double)m_nRank/(double)(nReps+1);}
    const double        GetRelativeRisk(double nMeasureAdjustment) const;
    virtual double      GetRelativeRiskForTract(tract_t tTract, const CSaTScanData & Data) const;
    virtual void        Initialize(tract_t nCenter=0);

    virtual bool        RateIsOfInterest(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure);
    bool                RateIsOfInterest(count_t nTotalCases, measure_t nTotalMeasure);

    void                SetAreaReport(stsAreaSpecificData* pAreaData) { gpAreaData = pAreaData; }
    void                SetCenter(tract_t nCenter);
    void                SetEllipseOffset(int iOffset);
    void                SetDuczmalCorrection(double dEllipseShape);
    double              SetLogLikelihood(double nLogLikelihood) {m_nLogLikelihood = nLogLikelihood; return m_nLogLikelihood;}
    void                SetRate(int nRate);
    double              SetRatio(double nLogLikelihoodForTotal);
    void                SetRatioAndDates(const CSaTScanData& Data);
    virtual void        SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
    void                WriteCoordinates(FILE* fp, CSaTScanData* pData);
    void                WriteLatLongCoords(FILE* fp, CSaTScanData* pData);
};

//*****************************************************************************
#endif
