//*****************************************************************************
#ifndef __PURELYSPATIALPROSPECTIVECLUSTER_H
#define __PURELYSPATIALPROSPECTIVECLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "UtilityFunctions.h"

class CMeasureList; /** class declaration defined in other unit */

/** cluster class for purely spatial cluster used in replications of prospective
    space-time analysis */
class CPurelySpatialProspectiveCluster : public CCluster {
  private:
    count_t                   	             * m_pCumCases;
    measure_t                                * m_pCumMeasure;
    measure_t                                * m_pCumMeasureSquared;
    int                                        m_nTotalIntervals;
    int                                        m_nProspectiveStartInterval;
    int                                        m_nNumIntervals; 

    void                        	       Init() {m_pCumCases=0;m_pCumMeasure=0;m_pCumMeasureSquared=0;}
    void			               Setup(const CSaTScanData & Data);
    void                                       Setup(const CPurelySpatialProspectiveCluster& rhs);

  public:
    CPurelySpatialProspectiveCluster(const CSaTScanData & Data, BasePrint *pPrintDirection);
    CPurelySpatialProspectiveCluster(const CPurelySpatialProspectiveCluster& rhs);
    ~CPurelySpatialProspectiveCluster();

     CPurelySpatialProspectiveCluster        & operator=(const CPurelySpatialProspectiveCluster& cluster);

    virtual void                               AddNeighbor(tract_t tNeighbor, const DataStreamInterface & Interface);
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