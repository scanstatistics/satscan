//*****************************************************************************
#ifndef __PURELYSPATIALCLUSTER_H
#define __PURELYSPATIALCLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "UtilityFunctions.h"
#include "MeasureList.h"
#include "ClusterDataFactory.h"

/** cluster class for purely spatial analysis and purely spatial cluster
    of space-time analysis */
class CPurelySpatialCluster : public CCluster {
  private:
    AbstractSpatialClusterData        * gpClusterData;

    void                                Setup(const CSaTScanData & Data);
    void                                Setup(const CPurelySpatialCluster& rhs);

  public:
    CPurelySpatialCluster(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway, int iRate, BasePrint *pPrintDirection);
    CPurelySpatialCluster(const AbstractClusterDataFactory * pClusterFactory, const DataStreamInterface & Interface, int iRate, BasePrint *pPrintDirection);
    CPurelySpatialCluster(const CPurelySpatialCluster& rhs);
    virtual ~CPurelySpatialCluster();

    inline virtual void                 AssignAsType(const CCluster& rhs) {*this = (CPurelySpatialCluster&)rhs;}
    virtual CPurelySpatialCluster     * Clone() const;
    CPurelySpatialCluster             & operator=(const CPurelySpatialCluster& rhs);

    void                                AddNeighborDataAndCompare(const AbtractDataStreamGateway & DataGateway,
                                                                  const CSaTScanData * pData,
                                                                  CPurelySpatialCluster & TopCluster,                                                                  
                                                                  AbstractLikelihoodCalculator & Calculator);
    virtual void                        DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType);
    virtual count_t                     GetCaseCount(unsigned int iStream) const {return gpClusterData->GetCaseCount(iStream);}
    virtual count_t                     GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual inline AbstractClusterData * GetClusterData() {return gpClusterData;}
    virtual int                         GetClusterType() const {return PURELYSPATIAL;}
    virtual measure_t                   GetMeasure(unsigned int iStream) const {return gpClusterData->GetMeasure(iStream);}
    virtual measure_t                   GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                        Initialize(tract_t nCenter=0);
    virtual void                        SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
};
//*****************************************************************************
#endif