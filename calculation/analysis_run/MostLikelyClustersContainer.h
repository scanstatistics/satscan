//***************************************************************************
#ifndef __MostLikelyClustersContainer_H
#define __MostLikelyClustersContainer_H
//***************************************************************************
#include "cluster.h"

class CSaTScanData;

/** Container class for maintaining the collection of most likely clusters. */
class MostLikelyClustersContainer {
  private:
    ZdPointerVector<CCluster>   gvTopClusterList;
    tract_t                     m_nClustersRetained;

    static int                  CompareClustersByRatio(const void *a, const void *b);
    void                        Init() {m_nClustersRetained=0;}

  public:
    MostLikelyClustersContainer();
    ~MostLikelyClustersContainer();

    void                        Add(const CCluster& Cluster);
    void                        Empty();
    tract_t                     GetNumClustersRetained() const {return m_nClustersRetained;}
    const CCluster            & GetCluster(tract_t tClusterIndex) const;
    const CCluster            & GetTopRankedCluster() const;
    void                        PrintTopClusters(const char * sFilename, int nHowMany);
    void                        RankTopClusters(const CParameters& Parameters, const CSaTScanData& DataHub);
    void                        SortTopClusters();
    void                        UpdateTopClustersRank(double r);
};
//***************************************************************************
#endif
