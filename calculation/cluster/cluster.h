//*****************************************************************************
#ifndef __CLUSTER_H
#define __CLUSTER_H
//*****************************************************************************
#include "SaTScan.h"
#include "SaTScanData.h"
#include "IncidentRate.h"
#include "DataStream.h"
#include "UtilityFunctions.h"
#include "ClusterDataFactory.h"

class stsAreaSpecificData;

class CCluster {
  protected:
    stsAreaSpecificData       * gpAreaData;

  public:
    CCluster();
    virtual ~CCluster();

    inline virtual void         AssignAsType(const CCluster& rhs) {*this = rhs;}
    virtual CCluster          * Clone() const = 0;
    CCluster                  & operator=(const CCluster& rhs);

    virtual count_t             GetCaseCount(unsigned int iStream) const {return 0;}
    virtual measure_t           GetMeasure(unsigned int iStream) const {return 0;}

    float                       gfPValue;               // p value of the cluster
    tract_t                     m_Center;             // Center of cluster (index to grid)
    int                         m_iEllipseOffset;     // Link to Circle or Ellipse (top cluster)

    tract_t                     m_nTracts;            // Number of neighboring tracts in cluster
    double                      m_nRatio;             // Likelihood ratio
    long                        m_nRank;              // Rank based on results of simulations
    double                      m_DuczmalCorrection;  // Duczmal compactness correction, for ellipses
    // Temporal variables
    int                         m_nFirstInterval;     // Index # of first time interval
    int                         m_nLastInterval;      // Index # of last time interval
    Julian                      m_nStartDate;         // Start time of cluster
    Julian                      m_nEndDate;           // End time of cluster


    RATE_FUNCPTRTYPE            m_pfRateOfInterest;

    virtual bool        ClusterDefined() {return m_nTracts;}
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
    virtual AbstractClusterData * GetClusterData() = 0;
    virtual int         GetClusterType() const = 0;
    virtual measure_t   GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const {return 0;}
    virtual tract_t     GetNumTractsInnerCircle() { return m_nTracts; }
    const double        GetPVal(int nReps) const {return (double)m_nRank/(double)(nReps+1);}
    const double        GetRelativeRisk(double nMeasureAdjustment) const;
    virtual double      GetRelativeRiskForTract(tract_t tTract, const CSaTScanData & Data) const;
    virtual void        Initialize(tract_t nCenter=0);
    void                SetAreaReport(stsAreaSpecificData* pAreaData) { gpAreaData = pAreaData; }
    void                SetCenter(tract_t nCenter);
    void                SetEllipseOffset(int iOffset);
    void                SetDuczmalCorrection(double dEllipseShape);
    void                SetRate(int nRate);
    virtual void        SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
    void                WriteCoordinates(FILE* fp, CSaTScanData* pData);
    void                WriteLatLongCoords(FILE* fp, CSaTScanData* pData);
};

//*****************************************************************************
#endif
