//*****************************************************************************
#ifndef __PURELYSPATIALPROSPECTIVECLUSTER_H
#define __PURELYSPATIALPROSPECTIVECLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "UtilityFunctions.h"

class CMeasureList;

class CPurelySpatialProspectiveCluster : public CCluster {
  private:
    count_t                   	             * m_pCumCases;
    measure_t                                * m_pCumMeasure;
    int                                        m_nTotalIntervals;
    int                                        m_nProspectiveStartInterval;
    int                                        m_nNumIntervals; 

    void                        	       Init() {m_pCumCases=0;m_pCumMeasure=0;}
    void			               Setup(const CSaTScanData & Data);
    void                                       Setup(const CPurelySpatialProspectiveCluster& rhs);

  public:
    CPurelySpatialProspectiveCluster(const CSaTScanData & Data, BasePrint *pPrintDirection);
    CPurelySpatialProspectiveCluster(const CPurelySpatialProspectiveCluster& rhs);
    ~CPurelySpatialProspectiveCluster();

     CPurelySpatialProspectiveCluster        & operator=(const CPurelySpatialProspectiveCluster& cluster);

    virtual void                               AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
    inline virtual void                        AssignAsType(const CCluster& rhs) {*this = (CPurelySpatialProspectiveCluster&)rhs;}
    virtual CPurelySpatialProspectiveCluster * Clone() const;
    inline virtual void                        ComputeBestMeasures(CMeasureList & MeasureList);
    virtual count_t                            GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t                          GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                               Initialize(tract_t nCenter);
    virtual void                               SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
};
//*****************************************************************************
#endif