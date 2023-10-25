//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "MostLikelyClustersContainer.h"
#include "SaTScanData.h"
#include "SSException.h" 
#include "ClusterSupplement.h"
#include "GisUtils.h"
#include <functional>
#include <numeric>

unsigned long MostLikelyClustersContainer::MAX_RANKED_CLUSTERS = 500;
unsigned long MostLikelyClustersContainer::MAX_BRUTE_FORCE_LOCATIONS = 50000;

// Returns the number of places in this cluster.
unsigned int MostLikelyClustersContainer::CompareClustersRatios::getNumNodesInCluster(const MostLikelyClustersContainer::Cluster_t& cluster) {
    if (_datahub.GetParameters().GetMultipleCoordinatesType() == ONEPERLOCATION) 
        return static_cast<unsigned int>(cluster->getNumIdentifiers());
    boost::dynamic_bitset<> clusterlocations;
    CentroidNeighborCalculator::getLocationsAboutCluster(_datahub, *cluster, &clusterlocations);
    return clusterlocations.size();
}

/** constructor */
MostLikelyClustersContainer::MostLikelyClustersContainer(double maximum_window_size) : _maximum_window_size(maximum_window_size) {}

/** Adds clone of passed cluster object to list of top clusters. */
void MostLikelyClustersContainer::Add(const CCluster& Cluster) {
  if (Cluster.ClusterDefined() && Cluster.GetRatio() > 0) {
    gvTopClusterList.push_back(boost::shared_ptr<CCluster>(Cluster.Clone()));
    gvTopClusterList.back()->DeallocateEvaluationAssistClassMembers();
  }
}

/** Adds cluster object to list of top clusters, taking ownership. */
void MostLikelyClustersContainer::Add(std::auto_ptr<CCluster>& pCluster) {
  if (pCluster.get() && pCluster->ClusterDefined() && pCluster->GetRatio() > 0) {
    gvTopClusterList.push_back(boost::shared_ptr<CCluster>(pCluster.release()));
    gvTopClusterList.back()->DeallocateEvaluationAssistClassMembers();
  }
}

//Does the point at 'theCentroid' lie within the spherical region described by
//'theCircleCentroid' and 'dCircleRadius'?
bool MostLikelyClustersContainer::CentroidLiesWithinSphereRegion(stsClusterCentroidGeometry const & theCentroid, stsClusterCentroidGeometry const & theSphereCentroid, double dSphereRadius) {
  bool bResult = false;
  try {
//----------------------------------float_stuff
//These lines of code exist to force identical behavior to the previous code.
//Comment them out and uncomment the double_stuff when you want higher precision.
    float fDistance = (float)theSphereCentroid.DistanceTo(theCentroid);
    float fSphereRadius = (float)dSphereRadius;
    bResult = fDistance <= fSphereRadius;
//----------------------------------double_stuff
//    bResult = theSphereCentroid.DistanceTo(theCentroid) <= dSphereRadius;
  }
  catch (prg_exception& x) {
     x.addTrace("CentroidLiesWithinSphereRegion()","MostLikelyClustersContainer");
     throw;
  }
  return bResult;
}

/** Combines passed collection with this collection of clusters. */
void MostLikelyClustersContainer::combine(const MostLikelyClustersContainer& other, const CSaTScanData& DataHub, const SimulationVariables& simVars, bool markAsGini) {
    const CParameters& params = DataHub.GetParameters();
    double p_cutoff = params.getGiniIndexPValueCutoff();
    bool markAlwaysOn = params.GetNumReplicationsRequested() < MIN_SIMULATION_RPT_PVALUE;

    if (gvTopClusterList.size() == 0) {
        gvTopClusterList = other.gvTopClusterList;
        for (ClusterList_t::iterator itrThis=gvTopClusterList.begin(); itrThis != gvTopClusterList.end(); ++itrThis) {
            if (markAlwaysOn || macro_less_than((*itrThis)->getReportingPValue(params, simVars, false), p_cutoff, DBL_CMP_TOLERANCE)) {
                (*itrThis)->setAsGiniCluster(markAsGini);
            }
        }
    } else {
        ClusterList_t combineClusters;
        ClusterList_t::const_iterator itrOther=other.gvTopClusterList.begin(), itrEndOther=other.gvTopClusterList.end();
        for (;itrOther != itrEndOther; ++itrOther) {
            // create bit set of cluster locations
            std::auto_ptr<boost::dynamic_bitset<> > otherSet;
            // iterate through this cluster collections to see if other cluster is duplicate of cluster already in set
            ClusterList_t::iterator itrThis=gvTopClusterList.begin(), itrThisEnd=gvTopClusterList.end();
            bool isDuplicate = false;
            for (;!isDuplicate && itrThis != itrThisEnd; ++itrThis) {
                // clusters are identical in terms of time interval and cluster locations
                if ((*itrOther)->m_nFirstInterval == (*itrThis)->m_nFirstInterval && 
                    (*itrOther)->m_nLastInterval == (*itrThis)->m_nLastInterval && 
                    (*itrOther)->getNumIdentifiers() == (*itrThis)->getNumIdentifiers()) {
                    if (!otherSet.get()) {
                        // allocate and set 'other' clusters bitset
                        otherSet.reset(new boost::dynamic_bitset<>(DataHub.GetNumIdentifiers() + DataHub.GetNumMetaIdentifiers()));
                        getClusterIdentifiersSet(DataHub, **itrOther, *otherSet.get());
                    }
                    boost::dynamic_bitset<> thisSet(DataHub.GetNumIdentifiers() + DataHub.GetNumMetaIdentifiers());
                    getClusterIdentifiersSet(DataHub, **itrThis, thisSet);
                    //thisSet &= *otherSet;
                    //isDuplicate = thisSet.count() == (*itrThis)->getNumIdentifiers();
                    //thisSet &= *otherSet;
                    isDuplicate = thisSet == *otherSet;
                    if (isDuplicate && markAsGini) {
                        // if these are duplicate clusters, we still want to ensure that existing one is marked as a gini cluster
                        if (markAlwaysOn || macro_less_than((*itrThis)->getReportingPValue(params, simVars, false), p_cutoff, DBL_CMP_TOLERANCE)) {
                            (*itrThis)->setAsGiniCluster(markAsGini);
                        }
                    }
                }
            }
            if (!isDuplicate) {
                // cluster is not a duplicate, retain in temporary collection
                combineClusters.push_back(*itrOther);
            }
        }
        // now add the non-duplicate clusters to cluster collection
        ClusterList_t::const_iterator itr=combineClusters.begin(), itrEnd=combineClusters.end();
        for (;itr != itrEnd; ++itr) {
            if (markAlwaysOn || macro_less_than((*itr)->getReportingPValue(params, simVars, false), p_cutoff, DBL_CMP_TOLERANCE)) {
                (*itr)->setAsGiniCluster(markAsGini);
            }
            gvTopClusterList.push_back(*itr);
        }
    }
}

/** Removes all cluster objects from list. */
void MostLikelyClustersContainer::Empty() {
    gvTopClusterList.clear();
}

/** Returns reference to cluster object at passed index. Throws exception if index is out of range.*/
const CCluster& MostLikelyClustersContainer::GetCluster(tract_t tClusterIndex) const {
  return *gvTopClusterList.at(tClusterIndex);
}

/** Returns reference to cluster object at passed index. Throws exception if index is out of range.*/
MostLikelyClustersContainer::Cluster_t& MostLikelyClustersContainer::GetClusterRef(tract_t tClusterIndex) {
  return gvTopClusterList.at(tClusterIndex);
}

/** According to information contained in 'DataHub', what is the radius of cluster? */
double MostLikelyClustersContainer::GetClusterRadius(const CSaTScanData& DataHub, CCluster const & theCluster) {
    double dResult;
    std::vector<double> ClusterCenter, NeighborCoords;
  
    try {
        if (theCluster.GetRadiusDefined())
            return theCluster.GetCartesianRadius(); //return radius already calculated
        else {
            DataHub.GetGInfo()->retrieveCoordinates(theCluster.GetCentroidIndex(), ClusterCenter);
            CentroidNeighborCalculator::getTractCoordinates(
                DataHub, theCluster, DataHub.GetNeighbor(theCluster.GetEllipseOffset(), theCluster.GetCentroidIndex(), theCluster.getNumIdentifiers()), NeighborCoords
            );
            dResult = Coordinates::distanceBetween(ClusterCenter, NeighborCoords);
        }
    } catch (prg_exception& x) {
        x.addTrace("GetClusterRadius()","MostLikelyClustersContainer");
        throw;
    }
    return dResult;
}

double MostLikelyClustersContainer::getClicCoefficient(const CSaTScanData& DataHub, const SimulationVariables& simVars, double p_cutoff) const {
    const CParameters & params(DataHub.GetParameters());
    double numClusters=0;
    double totalLLR=0;
    double totalPopulation=0;

    //create a copy of top cluster pointers
    ClusterList_t::const_iterator itrCurr=gvTopClusterList.begin(), itrEnd=gvTopClusterList.end();
    std::vector<CCluster *> sortClusters;
    for (size_t t=0; t < gvTopClusterList.size(); ++t) {
        double p_value = gvTopClusterList[t]->getReportingPValue(params, simVars, t==0);
        if (params.GetNumReplicationsRequested() < MIN_SIMULATION_RPT_PVALUE || macro_less_than(gvTopClusterList[t]->getReportingPValue(params, simVars, t==0), p_cutoff, DBL_CMP_TOLERANCE)) {
            const CCluster* cluster = gvTopClusterList[t].get();
            totalLLR += cluster->GetRatio()/cluster->GetNonCompactnessPenalty();
            //TODO: only Poisson (and using population file) and Bernoulli
            totalPopulation += DataHub.GetProbabilityModel().GetPopulation(0, *cluster, DataHub);
            ++numClusters;
        }
    }
    //Question proposed in meeting: Can we use proportion of total population instead?
    return -2.0 * totalLLR + log(totalPopulation) * numClusters;
}

/* Returns significant clusters for p-value cutoff. */
MostLikelyClustersContainer::ClusterList_t & MostLikelyClustersContainer::getSignificantClusters(const CSaTScanData& DataHub, const SimulationVariables& simVars, double p_cutoff, ClusterList_t & clusters) const {
    const CParameters & params(DataHub.GetParameters());
    clusters.clear();
    for (ClusterList_t::const_iterator itr=gvTopClusterList.begin(); itr != gvTopClusterList.end(); ++itr) {
        double p_value = (*itr)->getReportingPValue(params, simVars, itr == gvTopClusterList.begin());
        if (params.GetNumReplicationsRequested() < MIN_SIMULATION_RPT_PVALUE || macro_less_than(p_value, p_cutoff, DBL_CMP_TOLERANCE))
            clusters.push_back((*itr));
    }
    return clusters;
}

/* Returns significant clusters in most likely cluster list. */
MostLikelyClustersContainer::ClusterList_t & MostLikelyClustersContainer::getSignificantClusters(const CSaTScanData& DataHub, const SimulationVariables& simVars, ClusterList_t & clusters, ClusterSupplementInfo& supplement) const {
    const CParameters & params(DataHub.GetParameters());
    double min_ratio_to_report(0.001);
    clusters.clear();
    for (ClusterList_t::const_iterator itr=gvTopClusterList.begin(); itr != gvTopClusterList.end(); ++itr) {
        if ((*itr)->isSignificant(DataHub, std::distance(gvTopClusterList.begin(), itr) + 1, simVars)) {
            clusters.push_back((*itr));
            supplement.addCluster(*(*itr), std::distance(gvTopClusterList.begin(), itr) + 1);
        }
    }
    return clusters;
}

/* Calculates the GINI coefficient for the current collection of clusters. */
double MostLikelyClustersContainer::getGiniCoefficient(const CSaTScanData& DataHub, const SimulationVariables& simVars, boost::optional<double> p_value_cutoff, boost::optional<unsigned int> atmost) const {
    double giniCoefficient=0.0, totalCases = static_cast<double>(DataHub.GetTotalCases()), totalMeasure = DataHub.GetTotalMeasure();
    const CParameters & params(DataHub.GetParameters());
    unsigned int numDataSets = DataHub.GetDataSetHandler().GetNumDataSets();
    //create a copy of top cluster pointers
    ClusterList_t::const_iterator itrCurr=gvTopClusterList.begin(), itrEnd=gvTopClusterList.end();
    ClusterList_t sortClusters;
    size_t tMax = atmost ? std::min(*atmost, static_cast<unsigned int>(gvTopClusterList.size())) : gvTopClusterList.size();
    for (size_t t=0; t < tMax; ++t) {
        // optionally restricting clusters by p-value
        if (p_value_cutoff) {
            double p_value = gvTopClusterList[t]->getReportingPValue(params, simVars, t==0);
            if (params.GetNumReplicationsRequested() < MIN_SIMULATION_RPT_PVALUE || macro_less_than(p_value, *p_value_cutoff, DBL_CMP_TOLERANCE))
                sortClusters.push_back(gvTopClusterList[t]);
        } else
            sortClusters.push_back(gvTopClusterList[t]);
    }
    if (numDataSets > 1 && params.GetMultipleDataSetPurposeType() != ADJUSTMENT)
        throw prg_error("Gini coefficient calculation not implemented for multiple data set using '%d' purpose.", "getGiniCoefficient()", params.GetMultipleDataSetPurposeType());
    if (sortClusters.size() == 0) return giniCoefficient;
    // In order for index calculation to be correct when scanning low or high+low rates, we need to insert
    // a cluster that is the remainder of data outside of the clusters used in calcuation. 
    std::vector<count_t> remainderCases(numDataSets, 0);
    std::vector<measure_t> remainderMeasure(numDataSets, 0);
    for (size_t t=0; t < sortClusters.size(); ++t) {
        const CCluster& cluster = *(sortClusters[t]);
        for (size_t s=0; s < numDataSets; ++s) {
            remainderCases[s] += cluster.GetClusterData()->GetCaseCount(s);
            remainderMeasure[s] += cluster.GetClusterData()->GetMeasure(s);
        }
    }
    boost::shared_ptr<CCluster> remainderCluster((*sortClusters.begin())->Clone());
    for (unsigned int s=0; s < numDataSets; ++s) {
        remainderCluster->GetClusterData()->setCaseCount(DataHub.GetDataSetHandler().GetDataSet(s).getTotalCases() - remainderCases[s], s);
        remainderCluster->GetClusterData()->setMeasure(DataHub.GetDataSetHandler().GetDataSet(s).getTotalMeasure() - remainderMeasure[s], s);
    }
    remainderCluster->m_nRatio = 0;
    sortClusters.push_back(remainderCluster);
    // sort cluster collection by it's observerd/expected value
    std::sort(sortClusters.begin(), sortClusters.end(), CompareClustersObservedDivExpected(DataHub));
    //create structures to help calculating areas
    std::vector<double> height(sortClusters.size() + 1, 0.0);
    std::vector<double> base(sortClusters.size() + 1, 0.0);
    //initialize vectors to get respective lengths
    height.front() = base.front() = 1.0;
    for (size_t t=1; t < height.size() - 1; ++t) {
        const CCluster& cluster = *(sortClusters[t-1]);
        switch (params.GetProbabilityModelType()) {
            case POISSON:
            case BERNOULLI:
            case SPACETIMEPERMUTATION:
            case EXPONENTIAL:
            case UNIFORMTIME:
            case HOMOGENEOUSPOISSON: {
                double observed=0, expected=0;
                for (unsigned int s=0; s < numDataSets; ++s) {
                    observed += static_cast<double>(cluster.GetObservedCount(s));
                    expected += cluster.GetExpectedCount(DataHub, s);
                }
                height[t] = height[t-1] - observed/totalCases;
                base[t] = base[t-1] - expected/totalMeasure;
            } break;
            case NORMAL: {
                // Martin: After some thinking, I believe we can do this one. Instead of the expected, use the observed number of cases
                // in the cluster times the overall mean in the whole study region. Instead of the observed, use the observed 
                // number of cases in the cluster times the mean in the cluster.
                double observed=0, expected=0,clusterObserved=0,clusterExpected=0,overallMean=totalMeasure/totalCases;
                const DataSetHandler& dataSets(DataHub.GetDataSetHandler());
                for (unsigned int s=0; s < numDataSets; ++s) {
                    double overall_mean_dataset = dataSets.GetDataSet(s).getTotalMeasure()/dataSets.GetDataSet(s).getTotalCases();
                    clusterObserved = static_cast<double>(cluster.GetObservedCount(s));
                    clusterExpected = cluster.GetExpectedCount(DataHub, s);
                    observed += clusterObserved * clusterExpected/clusterObserved;
                    expected += clusterObserved * overall_mean_dataset;
                }
                height[t] = height[t-1] - observed/(overallMean * totalCases);
                base[t] = base[t-1] - expected/(overallMean * totalCases);
            } break;
            case ORDINAL:
            case CATEGORICAL:
            case RANK:
            default : throw prg_error("Unknown Probability Model Type '%d'.", "getGiniCoefficient()", params.GetProbabilityModelType());
        };
    }
    std::reverse(height.begin(),height.end());
    std::reverse(base.begin(),base.end());
    // GINI coefficient - see the wikipedia for the details -- GINI = 1-2B = 1 - sum of (base1-base0)*(height1+height0)'s
    double _2B_ = 0.0;
    for (size_t t=0; t < height.size() - 1; ++t)
        _2B_ += (base[t+1] - base[t]) * (height[t+1] + height[t]);
    giniCoefficient = 1.0 - _2B_;
    return giniCoefficient;
}

/** Returns constant reference to most likely cluster. Throws exception if
    there are no clusters to return. */
const CCluster& MostLikelyClustersContainer::GetTopRankedCluster() const {
  try {
    if (gvTopClusterList.size() == 0)
      throw prg_error("No clusters in container.","GetTopRankedCluster()");
  }
  catch (prg_exception& x) {
    x.addTrace("GetTopRankedCluster()","MostLikelyClustersContainer");
    throw;
  }
  return *gvTopClusterList[0];
}

/** Sets bitset to those atomic identifiers included in this cluster. */
void MostLikelyClustersContainer::getClusterIdentifiersSet(const CSaTScanData& DataHub, const CCluster& theCluster, boost::dynamic_bitset<>& theSet) {
    theSet.reset();
    std::vector<tract_t> indexes;
    for (auto index: theCluster.getIdentifierIndexes(DataHub, indexes, true))
        theSet.set(index);
}

/** Returns indication of whether geographical overlap exists between passed
    clusters by examining tract locations to comprise the cluster.*/
bool MostLikelyClustersContainer::HasAnyTractsInCommon(const CSaTScanData& DataHub, const CCluster& ClusterOne, const CCluster& ClusterTwo) const {
    tract_t tTwoNumTracts = ClusterTwo.getNumIdentifiers(), tOneNumTracts = ClusterOne.getNumIdentifiers();
    int iTwoOffset = ClusterTwo.GetEllipseOffset(), iOneOffset = ClusterOne.GetEllipseOffset();

    if (ClusterOne.GetRadiusDefined() && ClusterTwo.GetRadiusDefined() && DataHub.GetParameters().GetMultipleCoordinatesType() != ATLEASTONELOCATION) {
        /* If certain relationships exist between clusters, then we don't need to actually compare each identifier within them. 
           These two 'shortcuts' are meant to allow possible determination of overlap knowing only centroids and previously calculated radii (centric analyses). 
           When the analysis uses multiple coordinates, this shortcut is not allowed when the setting is to include in the cluster if at least
           one location is within the cluster -- in certain situations, overlap can occur where one cluster contains one coordinate while the other
           cluster contains a different coordinate of the same identifier. ** Networks don't define cluster radius. */
        std::vector<double> vClusterOneCoords, vClusterTwoCoords;
        DataHub.GetGInfo()->retrieveCoordinates(ClusterOne.GetCentroidIndex(), vClusterOneCoords);
        double ClusterOneRadius = ClusterOne.GetCartesianRadius();
        double ClusterTwoRadius = ClusterTwo.GetCartesianRadius();
        DataHub.GetGInfo()->retrieveCoordinates(ClusterTwo.GetCentroidIndex(), vClusterTwoCoords);
        double dDistanceBetween = stsClusterCentroidGeometry(vClusterOneCoords).DistanceTo(stsClusterCentroidGeometry(vClusterTwoCoords));

        //Note: Since there might be a fractional difference in the calculation of the radii and/or distance, require that the
        //compared values differ by more than 0.000000001 -- otherwise we might trigger a false positive.
        double dNumericalDeviation = 0.000000001;
        //we can say for certain that they don't have tracts in common if their circles don't overlap
        double dClusterOneMajorAxis = ClusterOneRadius * DataHub.GetEllipseShape(iOneOffset);
        double dClusterTwoMajorAxis = ClusterTwoRadius * DataHub.GetEllipseShape(iTwoOffset);
        if (std::fabs(dDistanceBetween - (dClusterOneMajorAxis + dClusterTwoMajorAxis)) > dNumericalDeviation && dDistanceBetween > dClusterOneMajorAxis + dClusterTwoMajorAxis)
            return false;

        //we can say that they do overlap if the centroid of second cluster is within radius of first cluster
        //or vice versa, centroid of first cluster is within radius of second cluster
        if (DataHub.GetParameters().UseSpecialGrid()) {
            if ((std::fabs(ClusterOneRadius - (dDistanceBetween + ClusterTwoRadius)) > dNumericalDeviation && ClusterOneRadius >= dDistanceBetween + ClusterTwoRadius) ||
                (std::fabs(ClusterTwoRadius - (dDistanceBetween + ClusterOneRadius)) > dNumericalDeviation && ClusterTwoRadius >= dDistanceBetween + ClusterOneRadius)) {
                return true;
            }
        } else {
            if ((std::fabs(dDistanceBetween - ClusterOneRadius) > dNumericalDeviation && dDistanceBetween <= ClusterOneRadius) ||
                (std::fabs(dDistanceBetween - ClusterTwoRadius) > dNumericalDeviation && dDistanceBetween <= ClusterTwoRadius)) {
                return true;
            }
        }
        // If the distance of any location in one cluster - to the center of the other cluster is less than the radius of the other cluster, then they have overlapping locations.
	    bool useLocationToRadiusMethod = (
		    (iOneOffset == 0 || iTwoOffset == 0) && // one of the clusters is circular
            DataHub.getIdentifierInfo().getMetaManagerProxy().getNumMeta() == 0 && // meta locations were not used
            DataHub.GetParameters().GetMultipleCoordinatesType() == ONEPERLOCATION // not using multiple coordinates
	    );
        if (useLocationToRadiusMethod) {
            // determine which cluster has the fewest number of locations.
            const CCluster * testCluster = tOneNumTracts <= tTwoNumTracts ? &ClusterOne : &ClusterTwo;
            const CCluster * otherCluster = tOneNumTracts <= tTwoNumTracts ? &ClusterTwo : &ClusterOne;
            // Override selection of smaller cluster if the other cluster is elliptical.
            if (otherCluster->GetEllipseOffset() > 0)
                std::swap(testCluster, otherCluster);
            tract_t tracts = testCluster->getNumIdentifiers(), centroid = testCluster->GetCentroidIndex();
            double radius = testCluster->GetCartesianRadius();
            int offset = testCluster->GetEllipseOffset();
            // get the coordinates for center of other cluster
            std::vector<double> centroidCoordinates, locationCoordinates;
            double compare_radius = otherCluster->GetCartesianRadius();
            DataHub.GetGInfo()->retrieveCoordinates(otherCluster->GetCentroidIndex(), centroidCoordinates);
		    for (tract_t t = 1; t <= tracts; ++t) {
			    tract_t location = DataHub.GetNeighbor(offset, centroid, t, radius);
			    const Identifier::LocationsSet_t& locations = DataHub.getIdentifierInfo().getIdentifiers()[location]->getLocations();
			    for (unsigned int c = 0; c < locations.size(); ++c) {
				    locations[c]->coordinates()->retrieve(locationCoordinates);
				    if (!macro_less_than(compare_radius, Coordinates::distanceBetween(centroidCoordinates, locationCoordinates), DBL_CMP_TOLERANCE))
					    return true;
			    }
		    }
            return false;
        }
    }
    // When multiple coordinates type is not one per identifier, then utilize CentroidNeighborCalculator method to obtain
    // each clusters locations then check for overlap. ** This new method might be better than the brute force in following code.
    if (DataHub.GetParameters().GetMultipleCoordinatesType() != ONEPERLOCATION) {
        boost::dynamic_bitset<> cluster1locations, cluster2locations;
        CentroidNeighborCalculator::getLocationsAboutCluster(DataHub, ClusterOne, &cluster1locations);
        CentroidNeighborCalculator::getLocationsAboutCluster(DataHub, ClusterTwo, &cluster2locations);
        cluster1locations &= cluster2locations; // get intersection of cluster locations sets
        return cluster1locations.any();
    }

    // Check for overlap using brute force - comparing farthest locations in each cluster first.
    // This process iterates through the locations of each cluster, starting on the right top and 
    // walking backwards via diagonal succession (top down). The following matrix examples the sequence.
    // 5| 21 16 11  7  4  2  1
    // 4| 26 22 17 12  8  5  3
    // 3| 30 27 23 18 13  9  6
    // 2| 33 31 28 24 19 14 10
    // 1| 35 34 32 29 25 20 15
    //   ----------------------
    //     1  2  3  4  5  6  7

    // First determine the cluster with the greatest number of locations and define as x axis, the other on the y axis.
    const CCluster& xCluster = tOneNumTracts > tTwoNumTracts ? ClusterOne : ClusterTwo;
    const CCluster& yCluster = tOneNumTracts > tTwoNumTracts ? ClusterTwo : ClusterOne;
    tract_t x_tracts = xCluster.getNumIdentifiers(), y_tracts = yCluster.getNumIdentifiers(), x_centroid = xCluster.GetCentroidIndex(), y_centroid = yCluster.GetCentroidIndex();
    int x_offset = xCluster.GetEllipseOffset(), y_offset = yCluster.GetEllipseOffset();
    // Iterate through all the left leaning diagonals of the matrix.
    unsigned int totalDiagonals = x_tracts + y_tracts - 1;
    for (unsigned int diagonalIdx=0; diagonalIdx < totalDiagonals; ++diagonalIdx) {
        // Given the example matrix above, the first diagnonal is comparing just location at index 7 of xCluster to index 5 of yCluster.
        // Diagonal 2 with be  6 to 5, 7 to 3; diagonal 3 would be 5 to 5, 6 to 4, 7 to 3; and so on.
        unsigned int xMin = diagonalIdx >= static_cast<unsigned int>(x_tracts) ? 1 : static_cast<unsigned int>(x_tracts) - diagonalIdx;
        unsigned int xMax = static_cast<unsigned int>(x_tracts) - (diagonalIdx < static_cast<unsigned int>(y_tracts) ? 0 : (diagonalIdx + 1) - static_cast<unsigned int>(y_tracts));
        unsigned int yMax = static_cast<unsigned int>(y_tracts) - (diagonalIdx < static_cast<unsigned int>(x_tracts) ? 0 : (diagonalIdx + 1) - static_cast<unsigned int>(x_tracts));
        unsigned int yMin = diagonalIdx >= static_cast<unsigned int>(y_tracts) ? 1 : static_cast<unsigned int>(y_tracts) - diagonalIdx;
        for (unsigned int x=xMin, y=yMax; ; ) {
            tract_t x_location = DataHub.GetNeighbor(x_offset, x_centroid, x, xCluster.GetCartesianRadius());
            tract_t y_location = DataHub.GetNeighbor(y_offset, y_centroid, y, yCluster.GetCartesianRadius());
            if (x_location < DataHub.GetNumIdentifiers() && y_location < DataHub.GetNumIdentifiers()) {
                // both x_location and y_location are atomic locations
                if (x_location == y_location)
                    return true;
            } else if (x_location >= DataHub.GetNumIdentifiers() && y_location >= DataHub.GetNumIdentifiers()) {
                // both x_location and y_location are meta locations
                if (DataHub.getIdentifierInfo().getMetaManagerProxy().intersects(x_location - DataHub.GetNumIdentifiers(), y_location - DataHub.GetNumIdentifiers()))
                    return true;
            } else if (x_location < DataHub.GetNumIdentifiers()) {
                // x_location is an atomic location and y_location is a meta location
                if (DataHub.getIdentifierInfo().getMetaManagerProxy().intersectsIdentifier(y_location - DataHub.GetNumIdentifiers(), x_location))
                    return true;
            } else { // y_location < DataHub.GetNumIdentifiers()
                // y_location is an atomic location and x_location is a meta location
                if (DataHub.getIdentifierInfo().getMetaManagerProxy().intersectsIdentifier(x_location - DataHub.GetNumIdentifiers(), y_location))
                    return true;
            }
            // Stop looping once both x and y variables reach respective min or max.
            if (x == xMax && y == yMin) break;
            if (x < xMax) ++x;
            if (y > yMin) --y;
        }
    }
    return false;
}

/* Returns whether a tract is contained within cluster using brute force check for overlap in tracts. 
   This method can be utilized when using a network file since the clusters can't be assumed to be circular or elliptical. */
bool MostLikelyClustersContainer::clusterContainsTract(const CSaTScanData& DataHub, tract_t clusterCenter, const CCluster& Cluster) {
    if (DataHub.GetParameters().GetMultipleCoordinatesType() != ONEPERLOCATION) {
        boost::dynamic_bitset<> clusterlocations;
        CentroidNeighborCalculator::getLocationsAboutCluster(DataHub, Cluster, &clusterlocations);
        return clusterlocations.test(clusterCenter);
    }
    for (tract_t t = Cluster.getNumIdentifiers(); t > 0; --t) {
        if (clusterCenter == DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), t, Cluster.GetCartesianRadius()))
            return true;
    }
    return false;
}

//Does the point at (dXPoint, dYPoint) lie within the two-dimensional region
//covered by the ellipse described by (dXEllipseCenter, dYEllipseCenter,
//'dEllipseRadius', 'dEllipseAngle', 'dEllipseShape')?
//require
// non_negative_radius: dEllipseRadius >= 0.0
// valid_shape: dEllipseShape >= 1.0
bool MostLikelyClustersContainer::PointLiesWithinEllipseArea(double dXPoint, double dYPoint, double dXEllipseCenter, double dYEllipseCenter, double dEllipseRadius, double dEllipseAngle, double dEllipseShape) {
  bool bResult = false;
  try {
    //assume dEllipseAngle is in radians
    double c = dEllipseRadius * std::sqrt(std::pow(dEllipseShape, 2) - 1);
    double dXFocus1 = dXEllipseCenter + (c * std::cos(dEllipseAngle));
    double dYFocus1 = dYEllipseCenter + (c * std::sin(dEllipseAngle));
    double dXFocus2 = dXEllipseCenter - (c * std::cos(dEllipseAngle));
    double dYFocus2 = dYEllipseCenter - (c * std::sin(dEllipseAngle));
    double dDistance1 = std::sqrt( std::pow(dXPoint - dXFocus1, 2) + std::pow(dYPoint - dYFocus1, 2) );
    double dDistance2 = std::sqrt( std::pow(dXPoint - dXFocus2, 2) + std::pow(dYPoint - dYFocus2, 2) );

//----------------------------------float_stuff
//These lines of code exist to force fewer digits of precision (and even floats
//may preserve too many).  Comment them out and uncomment the double_stuff when
//you want even higher precision.
    float fDistance1 = (float)dDistance1;
    float fDistance2 = (float)dDistance2;
    float fEllipseRadius = (float)dEllipseRadius;
    float fEllipseShape = (float)dEllipseShape;
    bResult = (fDistance1 + fDistance2) <= (2 * fEllipseRadius * fEllipseShape);
//----------------------------------double_stuff
//    bResult = (dDistance1 + dDistance2) <= (2 * dEllipseRadius * dEllipseShape);
  }
  catch (prg_exception& x) {
     x.addTrace("PointLiesWithinEllipseArea()","MostLikelyClustersContainer");
     throw;
  }
  return bResult;
}

/** Prints properties of cluster objects in top cluster list to ASCII file. */
void MostLikelyClustersContainer::PrintTopClusters(const char * sFilename, const CSaTScanData& DataHub) {
   FILE* pFile;
   ClusterSupplementInfo clusterSupplement;

   try {
      if ((pFile = fopen(sFilename, "w")) == NULL)
        throw resolvable_error("Error: Unable to open top clusters file.\n");
      else {


        for (size_t i=0; i < gvTopClusterList.size(); ++i) {
          /*
          fprintf(pFile, "GridTract:  %i\n", i);
          fprintf(pFile, "  Ellipe Offset:  %i\n", gvTopClusterList[i]->GetEllipseOffset());
          fprintf(pFile, "         Center:  %i\n", gvTopClusterList[i]->GetCentroidIndex());
          //fprintf(pFile, "        Measure:  %f\n", gvTopClusterList[i]->GetExpectedCount(DataHub));
          fprintf(pFile, "         Tracts:  %i\n", gvTopClusterList[i]->getNumIdentifiers());
          fprintf(pFile, "LikelihoodRatio:  %f\n", gvTopClusterList[i]->m_nRatio);
          fprintf(pFile, "           Rank:  %u\n", gvTopClusterList[i]->GetRank());
          fprintf(pFile, "   Cart. Radius:  %lf\n", gvTopClusterList[i]->GetCartesianRadius());
          */
          SimulationVariables simTemp; 
		  clusterSupplement.addCluster(*gvTopClusterList[i], i + 1);
          gvTopClusterList[i]->Display(pFile, DataHub, clusterSupplement, simTemp);
          fprintf(pFile, " \n");
          fprintf(pFile, " \n");
        }
      }
      fclose(pFile); pFile=0;
   }
  catch (prg_exception& x) {
    if (pFile) fclose(pFile);
    x.addTrace("PrintTopClusters()","MostLikelyClustersContainer");
    throw;
  }
}

/** Reorders list of top cluster objects by sorting by loglikelihood ratio in
    descending order. Clusters that are not 'defined' are removed from top
    clusters list. Retained clusters of list are further restricted by:
    for iterative scan
      - only most likely cluster retained
    reporting secondary clusters, no restrictions
      - limit list size to the number of tract locations
    reporting secondary clusters, all other
      - limit list size to minimum(number of grid points, MAX_RANKED_CLUSTERS)
      - remove clusters based upon specified criteria
    Note that this function should not be called with cluster list containing
    purely temporal clusters. The ranking performed is based soley on geographical
    orientation.   */
void MostLikelyClustersContainer::rankClusters(const CSaTScanData& DataHub, CriteriaSecondaryClustersType eOverlapType, BasePrint& print, unsigned int numKeepOverride) {
   const CParameters& parameters(DataHub.GetParameters());
   unsigned int       uClustersToKeepEachPass;

   try {
     //return if analysis is purely temporal -- there will be at most one cluster
     if (parameters.GetIsPurelyTemporalAnalysis()) return;
     //return from function if no clusters retained
     if (!gvTopClusterList.size()) return;
     //determine maximum number of clusters to retain
     if (numKeepOverride > 0) {
         uClustersToKeepEachPass = numKeepOverride;
     }
     else if (parameters.GetIsIterativeScanning())
       uClustersToKeepEachPass = 1;
     else if (eOverlapType == NORESTRICTIONS)
       uClustersToKeepEachPass = static_cast<unsigned long>(DataHub.GetNumIdentifiers());
     else
       uClustersToKeepEachPass = std::min(static_cast<unsigned long>(DataHub.m_nGridTracts), MAX_RANKED_CLUSTERS);
     //sort by descending m_ratio
     std::sort(gvTopClusterList.begin(), gvTopClusterList.end(), CompareClustersRatios(DataHub, this));

     if (eOverlapType != NORESTRICTIONS)
       print.Printf("Checking the Overlapping Nature of Clusters\n", BasePrint::P_STDOUT);
     //remove geographically overlapping clusters
     if (gvTopClusterList.size() > 0) {
       ClusterList_t vRetainedClusters;
       size_t tNumSpatialRetained=0;
       ClusterList_t::iterator itrCurr, itrEnd;
       for (itrCurr=gvTopClusterList.begin(),itrEnd=gvTopClusterList.end(); (tNumSpatialRetained < uClustersToKeepEachPass) && (itrCurr != itrEnd); ++itrCurr) {
          if (ShouldRetainCandidateCluster(vRetainedClusters, **itrCurr, DataHub, eOverlapType)) {
            //transfer ownership of cluster to vRetainedClusters
              vRetainedClusters.push_back(*itrCurr);
            //since previous version had it so that the purely temporal cluster was added after
            //this ranking process, we don't want to consider a purely temporal cluster as
            //part of the retained list when determining whether list is at maximum size
            if ((*itrCurr)->GetClusterType() != PURELYTEMPORALCLUSTER) ++tNumSpatialRetained;
            //*itrCurr = 0;
          }
       }
       gvTopClusterList.clear();
       gvTopClusterList.resize(vRetainedClusters.size());
       std::copy(vRetainedClusters.begin(), vRetainedClusters.end(), gvTopClusterList.begin());
     }
   } catch (prg_exception& x) {
     x.addTrace("rankClusters()", "MostLikelyClustersContainer");
     throw;
   }
}

//Given an index (into gvTopClusterList) of a candidate cluster and a supporting
//collection of information ('DataHub'), determine whether or not the candidate
//cluster should be retained according to a criterion.
//The clusters in gvTopClusterList before 'uCandidateIndex' are assumed to be
//"more likely" than the candidate.
//If the criterion is NOGEOOVERLAP then none of the clusters may be non-circular.
//require
//
bool MostLikelyClustersContainer::ShouldRetainCandidateCluster(ClusterList_t const & vRetainedClusters, CCluster const & CandidateCluster, const CSaTScanData& DataHub, CriteriaSecondaryClustersType eCriterion) {
    bool                            bResult=true;
    double                          dCandidateRadius, dCurrRadius, dDistanceCenters;
    std::vector<double>             vCandidateCenterCoords, vCurrCenterCoords;
    ClusterList_t::const_iterator   itrCurr, itrEnd;

    try {
        //criteria with no restrictions includes all clusters
        if (eCriterion == NORESTRICTIONS)
            return true;
        //this function currently discriminates by geographical orientation only - so candiddate cluster can not be purely temporal 
        if (CandidateCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
            return true; //always keep purely temporal cluster - we don't apply geographical overlap for these clusters
        if (eCriterion == NOGEOOVERLAP) { // specialized code for no geographical overlap
            std::auto_ptr<stsClusterCentroidGeometry> CandidateCenter;
            // we will potentially use the radius shortcut method if many locations and candidate cluster is circular
            bool shouldShortCut = false; 
            // We believe the new brute force algorithm will produce faster times such that this shortcut method is not needed.
            // The shortcut method isn't ideal since it does not ensure non-overlap in cluster locations, but only in cluster radius.
            if (shouldShortCut) {
                dCandidateRadius = GetClusterRadius(DataHub, CandidateCluster);
                // retrieve coordinates of candidate cluster
                DataHub.GetGInfo()->retrieveCoordinates(CandidateCluster.GetCentroidIndex(), vCandidateCenterCoords);
		        CandidateCenter.reset(new stsClusterCentroidGeometry(vCandidateCenterCoords));
            }
            for (itrCurr=vRetainedClusters.begin(), itrEnd=vRetainedClusters.end(); bResult && (itrCurr != itrEnd); ++itrCurr) {
                if ((*itrCurr)->GetClusterType() == PURELYTEMPORALCLUSTER)
                    continue; //skip comparison against retained purely temporal cluster - can't compare
                //if **itrCurr and CandidateCluster are both not elliptical and there are many locations; do not use brute force HasAnyTractsInCommon() but instead use shortcut.
                if (shouldShortCut && (**itrCurr).GetEllipseOffset() == 0) {
                    CCluster const & currCluster = **itrCurr;
                    DataHub.GetGInfo()->retrieveCoordinates(currCluster.GetCentroidIndex(), vCurrCenterCoords);
                    stsClusterCentroidGeometry currCenter(vCurrCenterCoords);
                    dCurrRadius = GetClusterRadius(DataHub, currCluster);
                    dDistanceCenters = CandidateCenter->DistanceTo(currCenter) - (dCurrRadius + dCandidateRadius);
                    bResult = std::fabs(dDistanceCenters) > DBL_CMP_TOLERANCE && dDistanceCenters > 0.0;
                } else {
                    bResult = !HasAnyTractsInCommon(DataHub, **itrCurr, CandidateCluster);
                }
            }
        } else { // standard geographical overlap checking
            std::auto_ptr<stsClusterCentroidGeometry> currCenter, CandidateCenter;
            if (!(DataHub.GetParameters().getUseLocationsNetworkFile() || DataHub.GetParameters().UseLocationNeighborsFile())) {
                // retrieve coordinates of candidate cluster
                DataHub.GetGInfo()->retrieveCoordinates(CandidateCluster.GetCentroidIndex(), vCandidateCenterCoords);
                CandidateCenter.reset(new stsClusterCentroidGeometry(vCandidateCenterCoords));
                //validate conditions:
                switch (eCriterion) {
                    case NOCENTROIDSINOTHER: //no cluster centroids in any other clusters
                    case NOCENTROIDSINMORELIKE: //no cluster centroids in more likely clusters
                    case NOCENTROIDSINLESSLIKE: //no cluster centroids in less likely clusters
                    case NOPAIRSINEACHOTHERS: //no pairs of centroids in each others clusters
                        if ((CandidateCluster.GetEllipseOffset() > 0) && (CandidateCenter->GetDimensionCount() > 2))
                            throw prg_error("For ellipses, cannot have more than 2 dimensions (got %d).", "MostLikelyClustersContainer", CandidateCenter->GetDimensionCount());
                        break;
                    case NOGEOOVERLAP: break; //no geographical overlap
                    default:  throw prg_error("Unknown Criteria for Reporting Secondary Clusters, '%d'.","MostLikelyClustersContainer", eCriterion);
                }
                dCandidateRadius = GetClusterRadius(DataHub, CandidateCluster);
            }
            for (itrCurr=vRetainedClusters.begin(), itrEnd=vRetainedClusters.end(); bResult && (itrCurr != itrEnd); ++itrCurr) {
                //skip comparison against retained purely temporal cluster - can't compare
                if ((*itrCurr)->GetClusterType() == PURELYTEMPORALCLUSTER) continue;
                CCluster const & currCluster = **itrCurr;
                if (!(DataHub.GetParameters().getUseLocationsNetworkFile() || DataHub.GetParameters().UseLocationNeighborsFile())) {
                    DataHub.GetGInfo()->retrieveCoordinates(currCluster.GetCentroidIndex(), vCurrCenterCoords);
                    currCenter.reset(new stsClusterCentroidGeometry(vCurrCenterCoords));
                    dCurrRadius = GetClusterRadius(DataHub, currCluster);
                }
                switch (eCriterion) {
                    case NOGEOOVERLAP:
                        bResult = CandidateCenter->DistanceTo(*currCenter) > (dCurrRadius + dCandidateRadius); break;
                    case NOCENTROIDSINOTHER: {//no cluster centroids in any other clusters
			            if (DataHub.GetParameters().getUseLocationsNetworkFile() || DataHub.GetParameters().UseLocationNeighborsFile())
				            bResult = !(
                                clusterContainsTract(DataHub, DataHub.GetGInfo()->retrieveLocationIndex(CandidateCluster.GetCentroidIndex()), **itrCurr) ||
                                clusterContainsTract(DataHub, DataHub.GetGInfo()->retrieveLocationIndex(itrCurr->get()->GetCentroidIndex()), CandidateCluster)
                            );
                        else if (CandidateCluster.GetEllipseOffset() > 0) {
                            if (currCluster.GetEllipseOffset() > 0) {//both are ellipses
                                bResult = !(
                                    PointLiesWithinEllipseArea(CandidateCenter->GetCoordinates()[0], CandidateCenter->GetCoordinates()[1], currCenter->GetCoordinates()[0], currCenter->GetCoordinates()[1], dCurrRadius, DataHub.GetEllipseAngle(currCluster.GetEllipseOffset()), DataHub.GetEllipseShape(currCluster.GetEllipseOffset()))
                                    || PointLiesWithinEllipseArea(currCenter->GetCoordinates()[0], currCenter->GetCoordinates()[1], CandidateCenter->GetCoordinates()[0], CandidateCenter->GetCoordinates()[1], dCandidateRadius, DataHub.GetEllipseAngle(CandidateCluster.GetEllipseOffset()), DataHub.GetEllipseShape(CandidateCluster.GetEllipseOffset()))
                                );
                            } else {//candidate is ellipse, curr is circle
                                bResult = !(
                                    CentroidLiesWithinSphereRegion(*CandidateCenter, *currCenter, dCurrRadius)
                                    || PointLiesWithinEllipseArea(currCenter->GetCoordinates()[0], currCenter->GetCoordinates()[1], CandidateCenter->GetCoordinates()[0], CandidateCenter->GetCoordinates()[1], dCandidateRadius, DataHub.GetEllipseAngle(CandidateCluster.GetEllipseOffset()), DataHub.GetEllipseShape(CandidateCluster.GetEllipseOffset()))
                                );
                            }
                        } else {
                            if (currCluster.GetEllipseOffset() > 0) {//candidate is circle, curr is ellipse
                                bResult = !(
                                    PointLiesWithinEllipseArea(CandidateCenter->GetCoordinates()[0], CandidateCenter->GetCoordinates()[1], currCenter->GetCoordinates()[0], currCenter->GetCoordinates()[1], dCurrRadius, DataHub.GetEllipseAngle(currCluster.GetEllipseOffset()), DataHub.GetEllipseShape(currCluster.GetEllipseOffset()))
                                    || CentroidLiesWithinSphereRegion(*currCenter, *CandidateCenter, dCandidateRadius)
                                );
                            } else {//both are circles
                                bResult = !(CentroidLiesWithinSphereRegion(*CandidateCenter, *currCenter, dCurrRadius) || CentroidLiesWithinSphereRegion(*currCenter, *CandidateCenter, dCandidateRadius));
                            }
                        }
                    } break;
                    case NOCENTROIDSINMORELIKE: {//no cluster centroids in more likely clusters
			            if (DataHub.GetParameters().getUseLocationsNetworkFile() || DataHub.GetParameters().UseLocationNeighborsFile())
				            bResult = !clusterContainsTract(DataHub, DataHub.GetGInfo()->retrieveLocationIndex(CandidateCluster.GetCentroidIndex()), **itrCurr);
                        else if (currCluster.GetEllipseOffset() == 0)
                            bResult = !CentroidLiesWithinSphereRegion(*CandidateCenter, *currCenter, dCurrRadius);
                        else
                            bResult = !(PointLiesWithinEllipseArea(CandidateCenter->GetCoordinates()[0], CandidateCenter->GetCoordinates()[1], currCenter->GetCoordinates()[0], currCenter->GetCoordinates()[1], dCurrRadius, DataHub.GetEllipseAngle(currCluster.GetEllipseOffset()), DataHub.GetEllipseShape(currCluster.GetEllipseOffset())));
                    } break;
                    case NOCENTROIDSINLESSLIKE: {//no cluster centroids in less likely clusters
			            if (DataHub.GetParameters().getUseLocationsNetworkFile() || DataHub.GetParameters().UseLocationNeighborsFile())
				            bResult = !clusterContainsTract(DataHub, DataHub.GetGInfo()->retrieveLocationIndex(itrCurr->get()->GetCentroidIndex()), CandidateCluster);
			            else if (CandidateCluster.GetEllipseOffset() == 0)
                            bResult = !CentroidLiesWithinSphereRegion(*currCenter, *CandidateCenter, dCandidateRadius);
                        else
                            bResult = !PointLiesWithinEllipseArea(currCenter->GetCoordinates()[0], currCenter->GetCoordinates()[1], CandidateCenter->GetCoordinates()[0], CandidateCenter->GetCoordinates()[1], dCandidateRadius, DataHub.GetEllipseAngle(CandidateCluster.GetEllipseOffset()), DataHub.GetEllipseShape(CandidateCluster.GetEllipseOffset()));
                    } break;
                    case NOPAIRSINEACHOTHERS: {//no pairs of centroids in each others clusters
			            if (DataHub.GetParameters().getUseLocationsNetworkFile() || DataHub.GetParameters().UseLocationNeighborsFile())
				            bResult = !clusterContainsTract(DataHub, DataHub.GetGInfo()->retrieveLocationIndex(CandidateCluster.GetCentroidIndex()), **itrCurr) ||
                            !clusterContainsTract(DataHub, DataHub.GetGInfo()->retrieveLocationIndex(itrCurr->get()->GetCentroidIndex()), CandidateCluster);
			            else if (CandidateCluster.GetEllipseOffset() > 0) {
                            if (currCluster.GetEllipseOffset() > 0) {//both are ellipses
                                bResult = (
                                    !PointLiesWithinEllipseArea(CandidateCenter->GetCoordinates()[0], CandidateCenter->GetCoordinates()[1], currCenter->GetCoordinates()[0], currCenter->GetCoordinates()[1], dCurrRadius, DataHub.GetEllipseAngle(currCluster.GetEllipseOffset()), DataHub.GetEllipseShape(currCluster.GetEllipseOffset()))
                                    || !PointLiesWithinEllipseArea(currCenter->GetCoordinates()[0], currCenter->GetCoordinates()[1], CandidateCenter->GetCoordinates()[0], CandidateCenter->GetCoordinates()[1], dCandidateRadius, DataHub.GetEllipseAngle(CandidateCluster.GetEllipseOffset()), DataHub.GetEllipseShape(CandidateCluster.GetEllipseOffset()))
                                );
                            } else {//candidate is ellipse, curr is circle
                                bResult = (
                                    !CentroidLiesWithinSphereRegion(*CandidateCenter, *currCenter, dCurrRadius)
                                    || !PointLiesWithinEllipseArea(currCenter->GetCoordinates()[0], currCenter->GetCoordinates()[1], CandidateCenter->GetCoordinates()[0], CandidateCenter->GetCoordinates()[1], dCandidateRadius, DataHub.GetEllipseAngle(CandidateCluster.GetEllipseOffset()), DataHub.GetEllipseShape(CandidateCluster.GetEllipseOffset()))
                                );
                            }
                        } else {
                            if (currCluster.GetEllipseOffset() > 0) {//candidate is circle, curr is ellipse
                                bResult = (
                                    !PointLiesWithinEllipseArea(CandidateCenter->GetCoordinates()[0], CandidateCenter->GetCoordinates()[1], currCenter->GetCoordinates()[0], currCenter->GetCoordinates()[1], dCurrRadius, DataHub.GetEllipseAngle(currCluster.GetEllipseOffset()), DataHub.GetEllipseShape(currCluster.GetEllipseOffset()))
                                    || !CentroidLiesWithinSphereRegion(*currCenter, *CandidateCenter, dCandidateRadius)
                                );
                            } else {//both are circles
                                bResult = !CentroidLiesWithinSphereRegion(*CandidateCenter, *currCenter, dCurrRadius) || !CentroidLiesWithinSphereRegion(*currCenter, *CandidateCenter, dCandidateRadius);
                            }
                        }
                    } break;
                    default:  throw prg_error("Unknown Criteria for Reporting Secondary Clusters '%d'.","MostLikelyClustersContainer", eCriterion);
                }
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("ShouldRetainCandidateCluster()", "MostLikelyClustersContainer");
        throw;
    }
    return bResult;
}

/* Sets all clusters in collection as Gini clusters. */
void MostLikelyClustersContainer::setClustersGini() {
    ClusterList_t::iterator itr(gvTopClusterList.begin()), itrEnd(gvTopClusterList.end());
    for (; itr != itrEnd; ++itr) {
        (*itr)->setAsGiniCluster(true);
    }
}

/* Sets all clusters in collection as Hierarchical clusters. */
void MostLikelyClustersContainer::setClustersHierarchical() {
    ClusterList_t::iterator itr(gvTopClusterList.begin()), itrEnd(gvTopClusterList.end());
    for (; itr != itrEnd; ++itr) {
        (*itr)->setAsHierarchicalCluster(true);
    }
}


/** sorts collection by cluster LLR, descending order */
void MostLikelyClustersContainer::sort(const CSaTScanData& DataHub) {
     std::sort(gvTopClusterList.begin(), gvTopClusterList.end(), CompareClustersRatios(DataHub));
}
