//***************************************************************************
#ifndef __MostLikelyClustersContainer_H
#define __MostLikelyClustersContainer_H
//***************************************************************************
#include "cluster.h"
#include "ptr_vector.h"
#include "SSException.h"
#include "boost/shared_ptr.hpp"
#include <boost/dynamic_bitset.hpp>
#include <boost/optional.hpp>

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
      throw prg_error("Cannot calculate distance between cluster of %d dimensions and cluster of %d dimensions.", "stsClusterCentroid", GetDimensionCount(), other.GetDimensionCount());
    double dSum = 0.0;
    std::vector<double>::const_iterator itrCurr(gvCoordinates.begin());
    std::vector<double>::const_iterator itrOtherCurr(other.gvCoordinates.begin());
    std::vector<double>::const_iterator itrEnd(gvCoordinates.end());
    for (; itrCurr != itrEnd; ++itrCurr, ++itrOtherCurr) {
      dSum += (*itrCurr - *itrOtherCurr) * (*itrCurr - *itrOtherCurr); 
    }
    return std::sqrt(dSum);
  }
};

class CSaTScanData;
class CentroidNeighborCalculator;

/** Container class for maintaining the collection of most likely clusters. */
class MostLikelyClustersContainer {
 public:
    typedef boost::shared_ptr<CCluster> Cluster_t;
    typedef std::vector<Cluster_t> ClusterList_t;

    /* CCluster comparison functor used to order CClusters in descending order by evaluating clusters calculated
       loglikelihood ratio. When ratios are equal, clusters centroid index is used to break tie. Purely temporal
       clusters will rank higher than other cluster types in tie break situation. */
    class CompareClustersRatios {
        protected:
            const CSaTScanData& _datahub;
            MostLikelyClustersContainer * _mlc_container;

            unsigned int getNumNodesInCluster(const Cluster_t& cluster);

        public:
            CompareClustersRatios(const CSaTScanData& datahub, MostLikelyClustersContainer * mlc_container=0): _datahub(datahub), _mlc_container(mlc_container) {}

            bool operator() (const Cluster_t& pCluster1, const Cluster_t& pCluster2) {
                if (std::fabs(pCluster1->m_nRatio - pCluster2->m_nRatio) < DBL_CMP_TOLERANCE) {
                    //rank a purely temporal cluster higher than other cluster types
                    //when rank is the same -- there will be at most one pt cluster in list 
                    if (pCluster1->GetClusterType() == PURELYTEMPORALCLUSTER)
                        return true;
                    if (pCluster2->GetClusterType() == PURELYTEMPORALCLUSTER)
                        return false;
                    // Prefer the cluster with fewer identifiers.
                    if (pCluster1->getNumIdentifiers() < pCluster2->getNumIdentifiers())
                        return true;
                    if (pCluster2->getNumIdentifiers() < pCluster1->getNumIdentifiers())
                        return false;

                    // Clusters have the same number of locations - additional tie breakers.
                    if (_datahub.GetParameters().GetMultipleCoordinatesType() != ONEPERLOCATION) {
                        // This is a multiple coordinates analysis.
                        if (_datahub.GetParameters().getUseLocationsNetworkFile()) {
                            // Prefer the cluster with fewer nodes.
                            unsigned int cluster1Nodes = getNumNodesInCluster(pCluster1);
                            unsigned int cluster2Nodes = getNumNodesInCluster(pCluster2);
                            if (cluster1Nodes < cluster2Nodes)
                                return true;
                            if (cluster2Nodes < cluster1Nodes)
                                return false;
                        } else {
                            // Prefer the cluster with the smaller radius.
                            if (pCluster1->GetCartesianRadius() < pCluster2->GetCartesianRadius())
                                return true;
                            if (pCluster2->GetCartesianRadius() < pCluster1->GetCartesianRadius())
                                return false;
                        }
                    }

                    //if ratios are equal, lesser centroid index ranks greater
                    if (pCluster1->mostCentralIdentifierIdx() != pCluster2->mostCentralIdentifierIdx())
                        return (pCluster1->mostCentralIdentifierIdx() < pCluster2->mostCentralIdentifierIdx());
                    else
                        return (pCluster1->GetCentroidIndex() < pCluster2->GetCentroidIndex());
                } 
                return pCluster1->m_nRatio > pCluster2->m_nRatio;
            }
    };

  protected:
    class CompareClustersObservedDivExpected {
       private:
           const CSaTScanData & _dataHub;
           const CParameters & _parameters;
           unsigned int _numDataSets;

       public:
         CompareClustersObservedDivExpected(const CSaTScanData& DataHub) : _dataHub(DataHub), _parameters(DataHub.GetParameters()), _numDataSets(DataHub.GetDataSetHandler().GetNumDataSets()) {
             // Verify that method is called for supported models -- Ordinal, Multinomial, Batched and Rank are not currently.
             if (_parameters.GetProbabilityModelType() == ORDINAL || _parameters.GetProbabilityModelType() == CATEGORICAL ||
                 _parameters.GetProbabilityModelType() == BATCHED || _parameters.GetProbabilityModelType() == RANK)
               throw prg_error("CompareClustersObservedDivExpected() not implemented for model('%d').","CompareClustersObservedDivExpected()", _parameters.GetProbabilityModelType());
         }
         bool operator() (const Cluster_t& pCluster1, const Cluster_t& pCluster2) {
             measure_t lhsObserved=0, rhsObserved=0, lhsExpected=0, rhsExpected=0;
             if (_parameters.GetProbabilityModelType() == NORMAL) {
                double lhsClusterObserved=0,lhsClusterExpected=0,rhsClusterObserved=0,rhsClusterExpected=0;
                const DataSetHandler& dataSets(_dataHub.GetDataSetHandler());
                for (unsigned int s=0; s < _numDataSets; ++s) {
                    double datasetMean = dataSets.GetDataSet(s).getTotalMeasure()/dataSets.GetDataSet(s).getTotalCases();
                    lhsClusterObserved = static_cast<measure_t>(pCluster1->GetObservedCount(s));
                    rhsClusterObserved = static_cast<measure_t>(pCluster2->GetObservedCount(s));
                    lhsClusterExpected = pCluster1->GetExpectedCount(_dataHub, s);
                    rhsClusterExpected = pCluster2->GetExpectedCount(_dataHub, s);
                    lhsObserved += lhsClusterObserved * lhsClusterExpected/lhsClusterObserved;
                    lhsExpected += lhsClusterObserved * datasetMean;
                    rhsObserved += rhsClusterObserved * rhsClusterExpected/rhsClusterObserved;
                    rhsExpected += rhsClusterObserved * datasetMean;
                }
             } else {
                for (unsigned int s=0; s < _numDataSets; ++s) {
                    lhsObserved += static_cast<measure_t>(pCluster1->GetObservedCount(s));
                    rhsObserved += static_cast<measure_t>(pCluster2->GetObservedCount(s));
                    lhsExpected += pCluster1->GetExpectedCount(_dataHub, s);
                    rhsExpected += pCluster2->GetExpectedCount(_dataHub, s);
                }
            }
            double lhsODE = lhsExpected ? lhsObserved/lhsExpected : 0;
            double rhsODE = rhsExpected ? rhsObserved/rhsExpected : 0;
            if (macro_equal(lhsODE, rhsODE, DBL_CMP_TOLERANCE))
                return pCluster1->GetRatio() > pCluster2->GetRatio();
            return lhsODE > rhsODE;
         }
    };
    ClusterList_t               gvTopClusterList;
    static unsigned long        MAX_RANKED_CLUSTERS;
    static unsigned long        MAX_BRUTE_FORCE_LOCATIONS;
    double                      _maximum_window_size;

    bool                        ShouldRetainCandidateCluster(ClusterList_t const & vRetainedClusters, CCluster const & CandidateCluster, const CSaTScanData& DataHub, CriteriaSecondaryClustersType eCriterion);

  public:
    MostLikelyClustersContainer(double maximum_window_size);

    void                        Add(const CCluster& Cluster);
    void                        Add(std::auto_ptr<CCluster>& pCluster);
    static bool                 CentroidLiesWithinSphereRegion(stsClusterCentroidGeometry const & theCentroid, stsClusterCentroidGeometry const & theSphereCentroid, double dSphereRadius);
    void                        combine(const MostLikelyClustersContainer& other, const CSaTScanData& DataHub, const SimulationVariables& simVars, bool markAsGini);
    void                        Empty();
    tract_t                     GetNumClustersRetained() const {return (tract_t)gvTopClusterList.size();}
    const CCluster            & GetCluster(tract_t tClusterIndex) const;
    static void                 getClusterIdentifiersSet(const CSaTScanData& DataHub, const CCluster& theCluster, boost::dynamic_bitset<>& theSet);
    static double               GetClusterRadius(const CSaTScanData& DataHub, CCluster const & theCluster);
    Cluster_t                 & GetClusterRef(tract_t tClusterIndex);
    double                      getClicCoefficient(const CSaTScanData& DataHub, const SimulationVariables& simVars, double p_cutoff) const;
    double                      getGiniCoefficient(const CSaTScanData& DataHub, const SimulationVariables& simVars, boost::optional<double> p_value_cutoff = boost::optional<double>(), boost::optional<unsigned int> atmost = boost::optional<unsigned int>()) const;
    ClusterList_t             & getSignificantClusters(const CSaTScanData& DataHub, const SimulationVariables& simVars, double p_cutoff, ClusterList_t & clusters) const;
    ClusterList_t             & getSignificantClusters(const CSaTScanData& DataHub, const SimulationVariables& simVars, ClusterList_t & clusters, ClusterSupplementInfo& supplement) const;
    const CCluster            & GetTopRankedCluster() const;
    double                      getMaximumWindowSize() const {return _maximum_window_size;}
    bool                        HasAnyTractsInCommon(const CSaTScanData& DataHub, const CCluster& ClusterOne, const CCluster& ClusterTwo) const;
    bool                        clusterContainsTract(const CSaTScanData& DataHub, tract_t clusterCenter, const CCluster& Cluster);
    static bool                 PointLiesWithinEllipseArea(double dXPoint, double dYPoint, double dXEllipseCenter, double dYEllipseCenter, double dEllipseRadius, double dEllipseAngle, double dEllipseShape);
    void                        PrintTopClusters(const char * sFilename, const CSaTScanData& DataHub);
    void                        rankClusters(const CSaTScanData& DataHub, CriteriaSecondaryClustersType eOverlapType, BasePrint& print, unsigned int numKeepOverride=0);
    void                        setClustersGini();
    void                        setClustersHierarchical();
    void                        sort(const CSaTScanData& DataHub);
};

typedef std::vector<MostLikelyClustersContainer> MLC_Collections_t;
//***************************************************************************
#endif
