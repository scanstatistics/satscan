//***************************************************************************
#ifndef __MostLikelyClustersContainer_H
#define __MostLikelyClustersContainer_H
//***************************************************************************
#include "cluster.h"

class stsClusterCentroidGeometry
{
  std::vector<double> gvCoordinates;

public:
  stsClusterCentroidGeometry(long lDimensionCount) { gvCoordinates.resize(lDimensionCount); }

  template <typename Sequence>
  stsClusterCentroidGeometry(Sequence const & theCoordinates) {
    gvCoordinates.resize(theCoordinates.size());
    std::vector<double>::iterator itrCurr = gvCoordinates.begin();
    typename Sequence::const_iterator itrSourceCurr = theCoordinates.begin();
    std::vector<double>::iterator itrEnd(gvCoordinates.end());
    for (; itrCurr != itrEnd; ++itrCurr, ++itrSourceCurr) {
      *itrCurr = *itrSourceCurr;
    }
  }

  template <typename T>
  stsClusterCentroidGeometry(T const * aCoordinates, unsigned uCoordinateCount) {
    gvCoordinates.resize(uCoordinateCount);
    std::vector<double>::iterator itrCurr = gvCoordinates.begin();
    T const * itrSourceCurr = aCoordinates;
    std::vector<double>::iterator itrEnd(gvCoordinates.end());
    for (; itrCurr != itrEnd; ++itrCurr, ++itrSourceCurr) {
      *itrCurr = *itrSourceCurr;
    }
  }

  long GetDimensionCount() const { return gvCoordinates.size(); }
  std::vector<double> const & GetCoordinates() const { return gvCoordinates; }

  double DistanceTo(stsClusterCentroidGeometry const & other) const {
    if (GetDimensionCount() != other.GetDimensionCount())
      ZdException::Generate("Cannot calculate distance between cluster of %d dimensions and cluster of %d dimensions.", "stsClusterCentroid", GetDimensionCount(), other.GetDimensionCount());
    double dSum = 0.0;
    std::vector<double>::const_iterator itrCurr(gvCoordinates.begin());
    std::vector<double>::const_iterator itrOtherCurr(other.gvCoordinates.begin());
    std::vector<double>::const_iterator itrEnd(gvCoordinates.end());
    for (; itrCurr != itrEnd; ++itrCurr, ++itrOtherCurr) {
      dSum += std::pow(*itrCurr - *itrOtherCurr, 2);
    }
    return std::sqrt(dSum);
  }
};

class CSaTScanData;

/** Container class for maintaining the collection of most likely clusters. */
class MostLikelyClustersContainer {
  private:
    //CCluster comparison functor used to order CClusters in descending order
    //by evaluating clusters calculated loglikelihood ratio. When ratios are
    //equal, clusters centroid index is used to break tie. Note that purely
    //temporal cluster types are rank higher than other cluster types with
    //same LLR. 
    class CompareClustersRatios {
       public:
         bool                   operator() (const CCluster* pCluster1, const CCluster* pCluster2)
                                {
                                  if (pCluster1->m_nRatio == pCluster2->m_nRatio) {
                                    //rank a purely temporal cluster higher than other cluster types
                                    //when rank is the same -- there will be at most one pt cluster in list 
                                    if (pCluster1->GetClusterType() == PURELYTEMPORALCLUSTER)
                                       return true;
                                    if (pCluster2->GetClusterType() == PURELYTEMPORALCLUSTER)
                                       return false;
                                    //if ratios are equal, lesser centroid index ranks greater
                                    return (pCluster1->GetCentroidIndex() < pCluster2->GetCentroidIndex());
                                  }
                                  return pCluster1->m_nRatio > pCluster2->m_nRatio;
                                }
    };

    ZdPointerVector<CCluster>   gvTopClusterList;
    static unsigned long        MAX_RANKED_CLUSTERS;

    static bool                 CentroidLiesWithinSphereRegion(stsClusterCentroidGeometry const & theCentroid, stsClusterCentroidGeometry const & theSphereCentroid, double dSphereRadius);
    static double               GetClusterRadius(const CSaTScanData& DataHub, CCluster const & theCluster);
    static bool                 HasTractsInCommon(const CSaTScanData& DataHub, const CCluster& ClusterOne, const CCluster& ClusterTwo);
    static bool                 PointLiesWithinEllipseArea(double dXPoint, double dYPoint, double dXEllipseCenter, double dYEllipseCenter, double dEllipseRadius, double dEllipseAngle, double dEllipseShape);
    bool                        ShouldRetainCandidateCluster(std::vector<CCluster *> const & vRetainedClusters, CCluster const & CandidateCluster, const CSaTScanData& DataHub, CriteriaSecondaryClustersType eCriterion);
    void                        SortTopClusters();

  public:
    MostLikelyClustersContainer();
    ~MostLikelyClustersContainer();

    void                        Add(const CCluster& Cluster);
    void                        Add(std::auto_ptr<CCluster>& pCluster);
    void                        Empty();
    tract_t                     GetNumClustersRetained() const {return (tract_t)gvTopClusterList.size();}
    const CCluster            & GetCluster(tract_t tClusterIndex) const;
    const CCluster            & GetTopRankedCluster() const;
    void                        PrintTopClusters(const char * sFilename, const CSaTScanData& DataHub);
    void                        RankTopClusters(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& gPrintDirection);
    void                        UpdateTopClustersRank(double r);
};
//***************************************************************************
#endif
