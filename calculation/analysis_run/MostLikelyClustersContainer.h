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
    ZdPointerVector<CCluster>   gvTopClusterList;

    static int                  CompareClustersByRatio(const void *a, const void *b);
    static bool                 PointLiesWithinEllipseArea(double dXPoint, double dYPoint, double dXEllipseCenter, double dYEllipseCenter, double dEllipseRadius, double dEllipseAngle, double dEllipseShape);
    static bool                 CentroidLiesWithinSphereRegion(stsClusterCentroidGeometry const & theCentroid, stsClusterCentroidGeometry const & theSphereCentroid, double dSphereRadius);
    bool                        ShouldRetainCandidateCluster(std::vector<CCluster *> const & vRetainedClusters, CCluster const & CandidateCluster, const CSaTScanData& DataHub, CriteriaSecondaryClustersType eCriterion);
    static double               GetClusterRadius(const CSaTScanData& DataHub, CCluster const & theCluster);

  public:
    MostLikelyClustersContainer();
    ~MostLikelyClustersContainer();

    void                        Add(const CCluster& Cluster);
    void                        Empty();
    tract_t                     GetNumClustersRetained() const {return (tract_t)gvTopClusterList.size();}
    const CCluster            & GetCluster(tract_t tClusterIndex) const;
    const CCluster            & GetTopRankedCluster() const;
    void                        PrintTopClusters(const char * sFilename, unsigned int nHowMany);
    void                        RankTopClusters(const CParameters& Parameters, const CSaTScanData& DataHub);
    void                        SortTopClusters();
    void                        UpdateTopClustersRank(double r);
};
//***************************************************************************
#endif
