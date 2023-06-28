//*****************************************************************************
#ifndef __PURELYSPATIALHOMOGENEOUSPOISSONCLUSTER_H
#define __PURELYSPATIALHOMOGENEOUSPOISSONCLUSTER_H
//*****************************************************************************
#include "cluster.h"
#include "UtilityFunctions.h"
#include "MeasureList.h"
#include "ClusterData.h"
#include "IntermediateClustersContainer.h"

/** cluster class for purely spatial analysis and purely spatial cluster
    of space-time analysis */
class PurelySpatialHomogeneousPoissonCluster : public CCluster {
  private:
    SpatialHomogeneousData        * gpClusterData;

  public:
    PurelySpatialHomogeneousPoissonCluster(const AbstractDataSetGateway & DataGateway);
    PurelySpatialHomogeneousPoissonCluster(const PurelySpatialHomogeneousPoissonCluster& rhs);
    virtual ~PurelySpatialHomogeneousPoissonCluster();

    PurelySpatialHomogeneousPoissonCluster                   & operator=(const PurelySpatialHomogeneousPoissonCluster& rhs);
    virtual PurelySpatialHomogeneousPoissonCluster           * Clone() const;
    virtual inline AbstractClusterData       * GetClusterData() {return gpClusterData;}
    virtual inline const AbstractClusterData * GetClusterData() const {return gpClusterData;}

    void                                CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                                                   const CentroidNeighbors& CentroidDef,
                                                                                   const CentroidNeighborCalculator::DistanceContainer_t& locDist,
                                                                                   CClusterSet& clusterSet,
                                                                                   AbstractLikelihoodCalculator& Calculator);
    virtual void                        CopyEssentialClassMembers(const CCluster& rhs);
    virtual void                        DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual ClusterType                 GetClusterType() const {return PURELYSPATIALHOMOGENEOUSCLUSTER;}
    virtual std::string               & GetEndDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    virtual measure_t                   GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0, bool adjusted=true) const;
    virtual count_t                     GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual std::string               & GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    virtual void                        Initialize(tract_t nCenter=0);
};
//*****************************************************************************
#endif
