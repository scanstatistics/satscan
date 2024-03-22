//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "SSException.h"
#include "IntermediateClustersContainer.h"
#include "SVTTCluster.h"

/** Returns a new cluster set object. */
boost::shared_ptr<CClusterSet> CClusterSet::getNewCClusterSetObject(const CCluster& cluster, const CSaTScanData& dataHub) {
    if (!dataHub.GetParameters().GetIsIterativeScanning() &&
        (dataHub.GetParameters().GetAnalysisType() == SPACETIME || dataHub.GetParameters().GetAnalysisType() == PURELYTEMPORAL))
        return boost::shared_ptr<CClusterSet>(new CClusterSetTemporalOverlap(cluster, dataHub.GetNumTimeIntervals()));
    else
        return boost::shared_ptr<CClusterSet>(new CClusterSet());
}

double CClusterSetTemporalOverlap::MIN_CLUSTER_LLR_RETAINED = 1.0;

/** Returns a collection of CClusterObjects that are the results of finding the best cluster in range [startRange, endRange],
    then recursively searching both the lower and upper ranges of that best cluster. */
CClusterSet::ClusterContainer_t CClusterSetTemporalOverlap::findBestInRange(int startRange, int endRange, CClusterSet::BestContainer_t& bestClustersInSet) {
    CClusterSet::ClusterContainer_t keepers;
    // Search within the range for the cluster with the best LLR.
    CClusterObject* bestInRange = 0;
    for (int endIdx = endRange; endIdx >= startRange + 1; --endIdx) {
        for (int startIdx = endIdx - 1; startIdx >= startRange; --startIdx) {
            CClusterObject& testCluster = bestClustersInSet[endIdx][startIdx];
            if (testCluster.getCluster().m_nRatio > MIN_CLUSTER_LLR_RETAINED) {
                //testCluster.getCluster().m_nRatio *= testCluster.getCluster().GetNonCompactnessPenalty(); // Already applied any compactness penalty.
                if (!bestInRange) bestInRange = &testCluster;
                else if (testCluster.getCluster().getNumIdentifiers() <= bestInRange->getMaxNeighbors() && bestInRange->getCluster().m_nRatio < testCluster.getCluster().GetRatio())
                    bestInRange = &testCluster;
            }
        }
    }
    if (bestInRange) {
        // We found a cluster with at least the minimum LLR in the current range. 
        // Now recursively search ranges above and below the time window of that found cluster.
        keepers.push_back(*bestInRange);
        for (auto& cluster : findBestInRange(startRange, bestInRange->getCluster().m_nFirstInterval - 1, bestClustersInSet))
            keepers.push_back(cluster);
        for (auto& cluster : findBestInRange(bestInRange->getCluster().m_nLastInterval + 1, endRange, bestClustersInSet))
            keepers.push_back(cluster);
    }
    return keepers;
}

void CClusterSetTemporalOverlap::findOtherNotTemporalOverlapping(CClusterSet::ClusterContainer_t& rankClusters, CClusterSet& clusterset, const CSaTScanData& data_hub) {
    assert(rankClusters.size() == 1);
    // Search ranges below cluster window recursively to find other non-overlapping cluster candidates.
    CClusterSet::BestContainer_t& bestClustersInSet = clusterset.getBestClusters();
    for (auto& cluster : findBestInRange(0, rankClusters.front().getCluster().m_nFirstInterval - 1, bestClustersInSet))
        rankClusters.push_back(cluster);
    // Search ranges above cluster window recursively to find other non-overlapping cluster candidates.
    for (auto& cluster : findBestInRange(rankClusters.front().getCluster().m_nLastInterval + 1, data_hub.GetNumTimeIntervals() - 1, bestClustersInSet))
        rankClusters.push_back(cluster);
    // Now sort by LLR descending
    std::sort(rankClusters.begin(), rankClusters.end(), [](CClusterObject& left, CClusterObject& right) {
        if (left.getCluster().m_nRatio == right.getCluster().m_nRatio) // prefer smaller clusters if same LLR
            return left.getCluster().getNumIdentifiers() < right.getCluster().getNumIdentifiers();
        return left.getCluster().m_nRatio > right.getCluster().m_nRatio;
    });
}

/* returns index into class vector _cluster_sets for 'shapeOffset' */
int CClusterSetCollections::getClusterSetCollectionIndex(int shapeOffset) {
    // determine the correct ClusterSet for shapeOffset -- when no compactness penalty, index is always 0
    if (shapeOffset && _parameters.GetSpatialWindowType() == ELLIPTIC && _parameters.GetNonCompactnessPenaltyType() != NOPENALTY) {
        for (int iEllipse = 0, iBoundry = 0; iEllipse < _parameters.GetNumRequestedEllipses(); ++iEllipse) {
            //Get the number of angles this ellipse shape rotates through.
            iBoundry += _parameters.GetEllipseRotations()[iEllipse];
            if (shapeOffset <= iBoundry) return iEllipse + 1;
        }
    } return 0;
}

/* Returns the top clusters among all shapes -- 
   for specified spatial window stops, taking into account the option of non-compactness penalizing for ellispes. */
SharedClusterVector_t& CClusterSetCollections::getTopClusters(SharedClusterVector_t& clusterCollection) {
    clusterCollection.clear(); // clear collection of any existing objects.

    // Special behavior for retrospective space-time analyses
    // We're retaining the best clusters about current centroid which do not overlap temporally.
    if (_parameters.GetAnalysisType() == SPACETIME && !_parameters.GetIsIterativeScanning() && _cluster_type == SPACETIMECLUSTER) {
        // Accumulate the top cluster(s) for circle and possibly ellipses into collection of best clusters.
        CClusterSet::ClusterContainer_t rankClusters;
        for (auto& clusterSetObj : _cluster_sets) {
            for (auto& clusterObj : clusterSetObj->getSet()) {
                if (clusterObj.getCluster().m_nRatio) {
                    clusterObj.getCluster().m_nRatio *= clusterObj.getCluster().GetNonCompactnessPenalty();
                    rankClusters.push_back(clusterObj.getCluster());
                }
            }
        }
        if (!rankClusters.size()) return clusterCollection; // return empty collection
        // Now sort by LLR descending to determine the best cluster among the cluster sets.
        std::sort(rankClusters.begin(), rankClusters.end(), [](CClusterObject& left, CClusterObject& right) {
            if (left.getCluster().m_nRatio == right.getCluster().m_nRatio) // prefer smaller clusters if same LLR
                return left.getCluster().getNumIdentifiers() < right.getCluster().getNumIdentifiers();
            return left.getCluster().m_nRatio > right.getCluster().m_nRatio;
        });
        rankClusters.erase(rankClusters.begin() + 1, rankClusters.end()); // remove all but the top ranked cluster
        CClusterSetTemporalOverlap::findOtherNotTemporalOverlapping(rankClusters, *_cluster_sets.back(), _data_hub);
        for (auto& cluster : rankClusters)
            clusterCollection.push_back(cluster.getClusterPtr());
        return clusterCollection;
    }

    // accumulate pointers CClusterObject of spatial stops into separate vectors
    std::vector<std::vector<CClusterObject*>> stopCollections(_parameters.getExecuteSpatialWindowStops().size());
    for (auto& clusterSet : _cluster_sets) {
        for (size_t maxStopIndex = 0; maxStopIndex < clusterSet->getSet().size(); ++maxStopIndex)
            stopCollections[maxStopIndex].push_back(&clusterSet->getSet()[maxStopIndex]);
    }
    // now calculate the top cluster of each shape at each spatial window stop
    for (auto& stopClusters: stopCollections) {
        boost::shared_ptr<CCluster> topCluster(stopClusters[0]->getClusterPtr());
        //apply compactness correction
        topCluster->m_nRatio *= topCluster->GetNonCompactnessPenalty();
        //if the there are ellipses, compare current top cluster against them
        //note: we don't have to be concerned with whether we are comparing circles and ellipses,
        //     the adjusted loglikelihood ratio for a circle is just the loglikelihood ratio
        for (size_t t = 1; t < stopClusters.size(); ++t) {
            if (stopClusters[t]->getCluster().ClusterDefined()) {
                //apply compactness correction
                stopClusters[t]->getCluster().m_nRatio *= stopClusters[t]->getCluster().GetNonCompactnessPenalty();
                //compare against current top cluster
                if (std::fabs(stopClusters[t]->getCluster().m_nRatio - topCluster->m_nRatio) > DBL_CMP_TOLERANCE && stopClusters[t]->getCluster().m_nRatio > topCluster->m_nRatio)
                    topCluster = stopClusters[t]->getClusterPtr();
            }
        }
        clusterCollection.push_back(topCluster);
    }
    return clusterCollection;
}

/* Reset all CCluster objects to center about centroid. */
void CClusterSetCollections::resetAboutCentroid(int iCenter) {
    for (auto& clusterSet : _cluster_sets)
        clusterSet->reset(iCenter);
}

/* Reset all CCluster objects to center about centroid. */
void CClusterSetCollections::resetAboutCentroidSVTT(int iCenter, const AbstractDataSetGateway& DataGateway) {
    for (auto& clusterSet: _cluster_sets) {
        for (auto& clusterObj : clusterSet->getSet()) {
            CSVTTCluster * pCluster = dynamic_cast<CSVTTCluster*>(&clusterObj.getCluster());
            if (pCluster) pCluster->InitializeSVTT(iCenter, DataGateway);
        }
    }
}

/** Allocates the cluster set collection from passed template cluster and count specified - number of ellipses + the circle.  */
void CClusterSetCollections::setClusterCollections(const CCluster& cluster, size_t count) {
    _cluster_sets.clear();
    _cluster_type = cluster.GetClusterType();
    for (size_t t = 0; t < count; ++t) { // one cluster set for each ellipse/circle
        boost::shared_ptr<CClusterSet> cluster_set;
        if (!_parameters.GetIsIterativeScanning() && _parameters.GetAnalysisType() == SPACETIME && _cluster_type == SPACETIMECLUSTER)
            cluster_set.reset(new CClusterSetTemporalOverlap(cluster, _data_hub.GetNumTimeIntervals()));
        else
            cluster_set.reset(new CClusterSet());
        // When analysis uses gini, there will be more than one spatial window stop (for the different max. spatial sizes of gini).
        for (size_t t = 0; t < _parameters.getExecuteSpatialWindowStops().size(); ++t) {
            CClusterObject addMe(cluster);
            cluster_set->add(addMe);
        }
        _cluster_sets.push_back(cluster_set);
    }
}

/* set persistant CCluster information for clusters ellipse offset. */
void CClusterSetCollections::setClusterNonPersistantNeighborInfo(const CentroidNeighbors& Neighbors) {
    for (auto & clusterSet: _cluster_sets) {
        for (auto& clusterObj : clusterSet->getSet()) {
            if (clusterObj.getCluster().GetEllipseOffset() == Neighbors.GetEllipseIndex())
                clusterObj.getCluster().SetNonPersistantNeighborInfo(_data_hub, Neighbors);
        }
        // If the cluster set is CClusterSetTemporalOverlap, we need to update best clusters as well.
        CClusterSetTemporalOverlap * ccsto = dynamic_cast<CClusterSetTemporalOverlap*>(clusterSet.get());
        if (ccsto) {
            for (auto& collection : ccsto->getBestClusters()) {
                for (auto& clusterObj : collection) {
                    if (clusterObj.getCluster().m_nRatio && clusterObj.getCluster().GetEllipseOffset() == Neighbors.GetEllipseIndex())
                        clusterObj.getCluster().SetNonPersistantNeighborInfo(_data_hub, Neighbors);
                }
            }
        }
    }
}
