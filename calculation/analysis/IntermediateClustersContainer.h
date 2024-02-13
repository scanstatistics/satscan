//******************************************************************************
#ifndef __IntermediateClustersContainer_H
#define __IntermediateClustersContainer_H
//******************************************************************************
#include "cluster.h"
#include "CentroidNeighbors.h"

class CSaTScanData; /** forward class declaration */

typedef std::vector<boost::shared_ptr<CCluster>> SharedClusterVector_t;

class CClusterObject {
    private:
        boost::shared_ptr<CCluster> _cluster;
        int _max_neighbors;

    public:
        CClusterObject(const CCluster& cluster, int max_neighbors=0) : _cluster(cluster.Clone()), _max_neighbors(max_neighbors) {}

        CCluster & getCluster() {
            return *_cluster;
        }
        boost::shared_ptr<CCluster> & getClusterPtr() {
            return _cluster;
        }
        int getMaxNeighbors() {
            return _max_neighbors;
        }
        void resetMaxNeighbors(int iMax) {
            _max_neighbors = iMax;
        }
};

/** A class to maintain collections of CClusterObjects and assist with data scanning. */
class CClusterSet {
    public:
        typedef std::vector<CClusterObject> ClusterContainer_t;
        typedef std::vector<ClusterContainer_t> BestContainer_t;

    protected:
        ClusterContainer_t _cluster_set;

    public:
        CClusterSet() {}
        virtual ~CClusterSet() {}

        static boost::shared_ptr<CClusterSet> getNewCClusterSetObject(const CCluster& cluster, const CSaTScanData& dataHub);

        void add(CClusterObject & clusterObj) {
            _cluster_set.push_back(clusterObj);
        }
        virtual BestContainer_t& getBestClusters() {
            throw prg_error("Not implemented","getBestClusters()");
        }
        virtual void reset(int iCenter) {
            // Resets cluster set to be centered at specified centroid.
            for (auto& clusterObj : _cluster_set)
                clusterObj.getCluster().Initialize(iCenter);
        }
        virtual void resetShape(const CSaTScanData& dataHub, int shapeOffset) {
            // Updates clusters sets to have maxmimum neighbors as defined by circle or ellipse.
            if (dataHub.GetParameters().getReportGiniOptimizedClusters() || dataHub.GetParameters().GetRestrictingMaximumReportedGeoClusterSize()) {
                for (size_t t=0; t < _cluster_set.size(); ++t) {
                    CClusterObject& clusterObj = _cluster_set[t];
                    clusterObj.resetMaxNeighbors(dataHub.GetReportedNeighborMaxsCountArray()[shapeOffset][clusterObj.getCluster().GetCentroidIndex()][t]);
                }
            } else {
                for (auto& clusterObj: _cluster_set)
                    clusterObj.resetMaxNeighbors(dataHub.GetNeighborCountArray()[shapeOffset][clusterObj.getCluster().GetCentroidIndex()]);
            }
        }
        virtual void resetShape(const CentroidNeighbors& Neighbors) {
            // Updates clusters sets to have maxmimum neighbors as defined by circle or ellipse.
            const std::vector<tract_t>& maxReported = Neighbors.getMaxReportedNeighbors();
            assert(_cluster_set.size() == maxReported.size());
            for (size_t t=0; t < _cluster_set.size(); ++t)
                _cluster_set[t].resetMaxNeighbors(maxReported[t]);
        }
        ClusterContainer_t& getSet() {
            return _cluster_set;
        }
        virtual void update(CCluster& runcluster) {
            // Updates cluster objects in set to runcluster if runcluster's LLR is greater.
            for (auto& clusterObj: _cluster_set) {
                if (runcluster.getNumIdentifiers() <= clusterObj.getMaxNeighbors() && clusterObj.getCluster().m_nRatio < runcluster.GetRatio())
                    clusterObj.getCluster().CopyEssentialClassMembers(runcluster);
            }
        }
};

/** A class to maintain collections of CClusterObjects and assist with data scanning. 
    This class extends CClusterSet to assist with finding additional clusters beyond the most likely which does not overlap that MLC temporally. */
class CClusterSetTemporalOverlap : public CClusterSet {
    protected:
        BestContainer_t _best_clusters;
        static double MIN_CLUSTER_LLR_RETAINED;

    public:
        CClusterSetTemporalOverlap(const CCluster& cluster, int intervals) : CClusterSet() {
            boost::shared_ptr<CCluster> _copy_cluster(cluster.Clone());
            _copy_cluster->DeallocateEvaluationAssistClassMembers();
            for (int idxe=1; idxe <= intervals + 1; ++idxe) {
                std::vector<CClusterObject> intervalClusters;
                for (int idxs=0; idxs < idxe; ++idxs)
                    intervalClusters.push_back(CClusterObject(*_copy_cluster));
                _best_clusters.push_back(intervalClusters);
            }
        }
        virtual ~CClusterSetTemporalOverlap() {}

        static CClusterSet::ClusterContainer_t findBestInRange(int startRange, int endRange, CClusterSet::BestContainer_t& bestClustersInSet);
        static void findOtherNotTemporalOverlapping(CClusterSet::ClusterContainer_t& rankClusters, CClusterSet& clusterset, const CSaTScanData& data_hub);

        virtual BestContainer_t& getBestClusters() { return _best_clusters; }
        virtual void reset(int iCenter) {
            // Resets cluster set to be centered at specified centroid.
            CClusterSet::reset(iCenter);
            // Reset all CCluster objects to have an LLR = 0 - meaning unset.
            for (auto& collection : _best_clusters) {
                for (auto& cc : collection) {
                    cc.getCluster().m_nRatio = 0.0; // This should be the only thing needed.
                    //cc.getCluster().m_Center = iCenter; not public // //cc.getCluster().Initialize(iCenter);
                }
            }
        }
        virtual void resetShape(const CSaTScanData& dataHub, int shapeOffset) {
            // Updates clusters sets to have maxmimum neighbors as defined by circle or ellipse.
            CClusterSet::resetShape(dataHub, shapeOffset);
            int maxNeighbors;
            if (dataHub.GetParameters().GetRestrictingMaximumReportedGeoClusterSize())
                maxNeighbors = dataHub.GetReportedNeighborMaxsCountArray()[shapeOffset][_cluster_set.front().getCluster().GetCentroidIndex()][0];
            else
                maxNeighbors = dataHub.GetNeighborCountArray()[shapeOffset][_cluster_set.front().getCluster().GetCentroidIndex()];
            for (auto& collection : _best_clusters) {
                for (auto& clusterObj : collection)
                    clusterObj.resetMaxNeighbors(maxNeighbors);
            }
        }
        virtual void resetShape(const CentroidNeighbors& Neighbors) {
            // Updates clusters sets to have maxmimum neighbors as defined by circle or ellipse.
            CClusterSet::resetShape(Neighbors);
            tract_t maxReported = Neighbors.getMaxReportedNeighbors()[0];
            for (auto& collection : _best_clusters) {
                for (auto& cc : collection)
                    cc.resetMaxNeighbors(maxReported);
            }
        }
        virtual void update(CCluster& runcluster) {
            CClusterSet::update(runcluster);
            // Get the current top cluster at the start and end indexes of the runcluster.
            auto& current = _best_clusters[runcluster.m_nLastInterval][runcluster.m_nFirstInterval];
            // Overwrite the current best with the runcluster if it's LLR is greater.
            runcluster.m_nRatio *= runcluster.GetNonCompactnessPenalty(); // Apply any compactness penalty now - since _best_clusters is irrespective of shape.
            if (runcluster.getNumIdentifiers() <= current.getMaxNeighbors() && current.getCluster().m_nRatio < runcluster.GetRatio())
                current.getCluster().CopyEssentialClassMembers(runcluster);
        }
};

/* A class to manage the collections of CClusterSet objects. We create CClusterSet objects for the 
   circle/network plus one for each elipse evaluated. */
class CClusterSetCollections {
    private:
        const CSaTScanData & _data_hub;
        const CParameters & _parameters;
        ClusterType _cluster_type;
        std::vector<boost::shared_ptr<CClusterSet>> _cluster_sets;

        void setClusterCollections(const CCluster& cluster, size_t count);

    public:
        CClusterSetCollections(const CSaTScanData& data_hub) : _data_hub(data_hub), _parameters(data_hub.GetParameters()), _cluster_type((ClusterType)0){}

        CClusterSet& getClusterSet(int shapeOffset) {
            /** Returns reference to cluster set for shape offset. This function is tightly  coupled to the presumption that
                CAnalysis::GetTopCluster() iterates over circles and ellispes in such a way that an offset of zero is a circle, and
                anything greater offset corresponds to an particular rotation of an ellipse shape. */
            return *_cluster_sets[getClusterSetCollectionIndex(shapeOffset)];
        }
        int getClusterSetCollectionIndex(int shapeOffset);
        SharedClusterVector_t& getTopClusters(SharedClusterVector_t& clusterCollections);
        void resetAboutCentroid(int iCenter);
        void resetAboutCentroidSVTT(int iCenter, const AbstractDataSetGateway& DataGateway);
        void resetNeighborCounts(int shapeOffset) {
            /* reset maximum neighbor counts in all ClusterObjects in 'shapeOffset'th ClusterSet */
            _cluster_sets[getClusterSetCollectionIndex(shapeOffset)]->resetShape(_data_hub, shapeOffset);
        }
        void resetNeighborCounts(const CentroidNeighbors& Neighbors) {
            /* reset maximum neighbor counts in all ClusterObjects in 'shapeOffset'th ClusterSet */
            _cluster_sets[getClusterSetCollectionIndex(Neighbors.GetEllipseIndex())]->resetShape(Neighbors);
        }
        void setClusterNonPersistantNeighborInfo(const CentroidNeighbors& Neighbors);
        void setTopClusters(const CCluster& cluster) {
            /** Initialzies the vector of top clusters to cloned copies of cluster, taking into account whether spatial shape will be a factor in analysis. */
            size_t count = 0;
            //if there are ellipses and a non-compactness penalty, then we need a top cluster for the circle and each ellipse shape
            if (_parameters.GetSpatialWindowType() == ELLIPTIC && _parameters.GetNonCompactnessPenaltyType() != NOPENALTY)
                count = _parameters.GetNumRequestedEllipses() + 1; //_cluster_sets.resize(_parameters.GetNumRequestedEllipses() + 1, clusterSet); // create a ClusterSet for each shape
            else //else there is only one top cluster - regardless of whether there are ellipses
                count = 1; //_cluster_sets.resize(1, clusterSet); // create single ClusterSet -- only one needed
            setClusterCollections(cluster, count);
        }
};
//******************************************************************************
#endif
