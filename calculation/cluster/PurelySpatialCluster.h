//*****************************************************************************
#ifndef __PURELYSPATIALCLUSTER_H
#define __PURELYSPATIALCLUSTER_H
//*****************************************************************************
#include "cluster.h"
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
    CPurelySpatialCluster(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataSetGateway & DataGateway, int iRate);
    CPurelySpatialCluster(const AbstractClusterDataFactory * pClusterFactory, const DataSetInterface & Interface, int iRate);
    CPurelySpatialCluster(const CPurelySpatialCluster& rhs);
    virtual ~CPurelySpatialCluster();

    inline virtual void                        AssignAsType(const CCluster& rhs) {*this = (CPurelySpatialCluster&)rhs;}
    CPurelySpatialCluster                    & operator=(const CPurelySpatialCluster& rhs);
    virtual CPurelySpatialCluster            * Clone() const;
    virtual inline AbstractClusterData       * GetClusterData() {return gpClusterData;}
    virtual inline const AbstractClusterData * GetClusterData() const {return gpClusterData;}

    void                                AddNeighborDataAndCompare(tract_t tEllipseOffset,
                                                                  tract_t tCentroid,
                                                                  const AbtractDataSetGateway & DataGateway,
                                                                  const CSaTScanData * pData,
                                                                  CPurelySpatialCluster & TopCluster,
                                                                  AbstractLikelihoodCalculator & Calculator);
    virtual void                        DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual ClusterType                 GetClusterType() const {return PURELYSPATIALCLUSTER;}
    virtual ZdString                  & GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    virtual measure_t                   GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual count_t                     GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual ZdString                  & GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    virtual void                        Initialize(tract_t nCenter=0);
};
//*****************************************************************************
#endif
