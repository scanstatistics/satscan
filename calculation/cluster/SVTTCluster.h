//*****************************************************************************
#ifndef __SVTTCLUSTER_H
#define __SVTTCLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "TimeTrend.h"

class CSVTTCluster : public CCluster  {
  private:
    void                        Init() {m_pCumCases=0;m_pCumMeasure=0;m_pRemCases=0;m_pRemMeasure=0;}
    void                        Setup(const CSaTScanData& Data, const count_t* pCasesByTimeInt);
    void                        Setup(const CSVTTCluster & rhs);

  public:
    int                         m_nTotalIntervals;
    count_t                   * m_pCumCases;             // Cases inside the cluster
    measure_t                 * m_pCumMeasure;           // Measure inside the cluster
    count_t                   * m_pRemCases;             // Cases outside the cluster
    measure_t                 * m_pRemMeasure;           // Measure outside the cluster
    count_t                     m_nRemCases;             // Total cases outside the cluster
    CTimeTrend                  m_nTimeTrend;            // Time trend for defined cluster
    CTimeTrend                  m_nRemTimeTrend;         // Time trend for area outside cluster

  public:
    CSVTTCluster(const CSaTScanData& Data, const count_t* pCasesByTimeInt, BasePrint *pPrintDirection);
    CSVTTCluster(const CSVTTCluster & rhs);
    ~CSVTTCluster();

    CSVTTCluster& operator=(const CSVTTCluster& rhs);

    virtual void                AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
    virtual CSVTTCluster      * Clone() const;
    virtual void                DisplayAnnualTimeTrendWithoutTitle(FILE* fp);
    virtual void                DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType) {};
    virtual void                DisplayTimeTrend(FILE* fp, char* szSpacesOnLeft);
    virtual count_t             GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t           GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                InitializeSVTT(tract_t nCenter, const CSaTScanData& Data, const count_t* pCasesByTimeInt);
    virtual void                SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
};

#endif