//******************************************************************************
#ifndef __IntermediateClustersContainer_H
#define __IntermediateClustersContainer_H
//******************************************************************************
#include "cluster.h"
#include "CentroidNeighbors.h"

class CSaTScanData; /** forward class declaration */

typedef std::vector<boost::shared_ptr<CCluster> > SharedClusterVector_t;

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

class CClusterSet {
  private:
    std::vector<CClusterObject> _clusterSet;

  public:
    //CClusterSet() {}
    //CClusterSet(size_t count, const CClusterObject & clusterObj) {set(count, clusterObj);}

    void add(CClusterObject & clusterObj) {
        _clusterSet.push_back(clusterObj);
    }
    size_t numInSet() const {
        return _clusterSet.size();
    }
    void resetShape(const CSaTScanData& dataHub, int shapeOffset) {
        if (dataHub.GetParameters().getReportGiniOptimizedClusters() || dataHub.GetParameters().GetRestrictingMaximumReportedGeoClusterSize()) {
            for (size_t t=0; t < _clusterSet.size(); ++t) {
                CClusterObject& clusterObj = _clusterSet[t];
                clusterObj.resetMaxNeighbors(dataHub.GetReportedNeighborMaxsCountArray()[shapeOffset][clusterObj.getCluster().GetCentroidIndex()][t]);
            }
        } else {
            for (size_t t=0; t < _clusterSet.size(); ++t) {
                CClusterObject& clusterObj = _clusterSet[t];
                clusterObj.resetMaxNeighbors(dataHub.GetNeighborCountArray()[shapeOffset][clusterObj.getCluster().GetCentroidIndex()]);
            }
        }
    }
    void resetShape(const CentroidNeighbors& Neighbors) {
        const std::vector<tract_t>& maxReported = Neighbors.getMaxReportedNeighbors();
        assert(_clusterSet.size() == maxReported.size());
        for (size_t t=0; t < _clusterSet.size(); ++t) {
            _clusterSet[t].resetMaxNeighbors(maxReported[t]);
        }
    }
    CClusterObject & get(size_t tClusterObj) {
        return _clusterSet.at(tClusterObj);
    }
    void maximizeClusterSet() {
        return;
        /*for (size_t t=0; t < _clusterSet.size() - 1; ++t) {
            CClusterObject&  clusterObj(_clusterSet[t]);
            for (size_t s=t+1; s < _clusterSet.size(); ++s) {
                CClusterObject&  nextClusterObj(_clusterSet[s]);
                if (macro_less_than(nextClusterObj.getCluster().m_nRatio, clusterObj.getCluster().m_nRatio, DBL_CMP_TOLERANCE)) {
                    nextClusterObj.getCluster().CopyEssentialClassMembers(clusterObj.getCluster());
                }
            }
        }*/
    }
    void set(size_t count, const CClusterObject & clusterObj) {
        _clusterSet.resize(count, clusterObj);
    }
    bool update(const CCluster& runcluster) {
        bool any=false;
        for (size_t t=0; t < _clusterSet.size(); ++t) {
            CClusterObject&  clusterObj(_clusterSet[t]);
            if (runcluster.getNumObservationGroups() <= clusterObj.getMaxNeighbors() && clusterObj.getCluster().m_nRatio < runcluster.GetRatio()) {
                clusterObj.getCluster().CopyEssentialClassMembers(runcluster);
                any=true;
                //break; // shortcut updating remainder of clusters with larger maximum neighbors -- call maximizeClusterSet() 
            }
        } return any;
    }
};

class CClusterSetCollections {
  private:
    const CSaTScanData & _dataHub;
    const CParameters & _parameters;
    std::vector<CClusterSet> _clusterSetCollections;

    void setClusterCollections(const CCluster& cluster, size_t count) {
        _clusterSetCollections.clear();
        for (size_t t=0; t < count; ++t) {
            CClusterSet clusterSet;
            for (size_t t=0; t < _parameters.getExecuteSpatialWindowStops().size(); ++t) {
                CClusterObject clusterObj(cluster);
                clusterSet.add(clusterObj);
            }
            _clusterSetCollections.push_back(clusterSet);
        }
    }

  public:
    CClusterSetCollections(const CSaTScanData& dataHub) : _dataHub(dataHub), _parameters(dataHub.GetParameters()) {}

    /* reset all CCluster objects to center about centroid 'iCenter' -- calls CCluster::Initialize(int) */
    void reset(int iCenter) {
        for (size_t shape=0; shape < _clusterSetCollections.size(); ++shape) {
            CClusterSet& clusterSet = _clusterSetCollections[shape];
            for (size_t maxStopIndex=0; maxStopIndex < clusterSet.numInSet(); ++maxStopIndex) 
                clusterSet.get(maxStopIndex).getCluster().Initialize(iCenter);
        }
    }
    /* reset all CCluster objects to center about centroid 'iCenter' -- calls CCluster::Initialize(int) */
    void resetSVTT(int iCenter, const AbstractDataSetGateway& DataGateway);

    /* set persistant CCluster information for clusters ellipse offset. */
    void setClusterNonPersistantNeighborInfo(const CentroidNeighbors& Neighbors) {
        for (size_t shape=0; shape < _clusterSetCollections.size(); ++shape) {
            CClusterSet& clusterSet = _clusterSetCollections[shape];
            for (size_t maxStopIndex=0; maxStopIndex < clusterSet.numInSet(); ++maxStopIndex) {
                  CCluster& cluster = clusterSet.get(maxStopIndex).getCluster();
                  if (cluster.GetEllipseOffset() == Neighbors.GetEllipseIndex())
                    cluster.SetNonPersistantNeighborInfo(_dataHub, Neighbors);
            }
        }
    }
    /* returns index into class vector _clusterSetCollections for 'shapeOffset' */
    int getClusterSetCollectionIndex(int shapeOffset) {
        // determine the correct ClusterSet for shapeOffset -- when no compactness penalty, index is always 0
        if (shapeOffset && _parameters.GetSpatialWindowType() == ELLIPTIC && _parameters.GetNonCompactnessPenaltyType() != NOPENALTY) {
            for (int iEllipse=0,iBoundry=0; iEllipse < _parameters.GetNumRequestedEllipses(); ++iEllipse) {
                //Get the number of angles this ellipse shape rotates through.
                iBoundry += _parameters.GetEllipseRotations()[iEllipse];
                if (shapeOffset <= iBoundry) return iEllipse + 1;
            }
        } return 0;
    }
    /* reset maximum neighbor counts in all ClusterObjects in 'shapeOffset'th ClusterSet */
    void resetNeighborCounts(int shapeOffset) {
        _clusterSetCollections.at(getClusterSetCollectionIndex(shapeOffset)).resetShape(_dataHub, shapeOffset);
    }
    /* reset maximum neighbor counts in all ClusterObjects in 'shapeOffset'th ClusterSet */
    void resetNeighborCounts(const CentroidNeighbors& Neighbors) {
        _clusterSetCollections.at(getClusterSetCollectionIndex(Neighbors.GetEllipseIndex())).resetShape(Neighbors);
    }
    /** Initialzies the vector of top clusters to cloned copies of cluster, taking into account whether spatial shape will be a factor in analysis. */
    void setTopClusters(const CCluster& cluster) {
        size_t count = 0;
        //if there are ellipses and a non-compactness penalty, then we need a top cluster for the circle and each ellipse shape
        if (_parameters.GetSpatialWindowType() == ELLIPTIC && _parameters.GetNonCompactnessPenaltyType() != NOPENALTY)
            count = _parameters.GetNumRequestedEllipses() + 1; //_clusterSetCollections.resize(_parameters.GetNumRequestedEllipses() + 1, clusterSet); // create a ClusterSet for each shape
        else //else there is only one top cluster - regardless of whether there are ellipses
            count = 1; //_clusterSetCollections.resize(1, clusterSet); // create single ClusterSet -- only one needed
        setClusterCollections(cluster, count);
    }
    /** Returns reference to top cluster for shape offset. This function is tightly  coupled to the presumption that 
        CAnalysis::GetTopCluster() iterates over circles and ellispes in such a way that an offset of zero is a circle, and
        anything greater offset corresponds to an particular rotation of an ellipse shape. */
    CClusterSet& getClusterSet(int shapeOffset) {
        return _clusterSetCollections.at(getClusterSetCollectionIndex(shapeOffset));
    }

    /* Returns the top clusters among all shapes -- for specified spatial window stops, taking into account the option of non-compactness penalizing for ellispes. */
    SharedClusterVector_t& getTopClusters(SharedClusterVector_t& topClusterCollection) {
        // create collection of top clusters for each spatial window stop
        topClusterCollection.clear();

        // accumulate pointers CClusterObject of spatial stops into separate vectors
        std::vector< std::vector<CClusterObject*> > stopCollections(_parameters.getExecuteSpatialWindowStops().size());
        for (size_t t=0; t < _clusterSetCollections.size(); ++t) {
            CClusterSet& clusterSet = _clusterSetCollections.at(t);
            for (size_t maxStopIndex=0; maxStopIndex < clusterSet.numInSet(); ++maxStopIndex)
                stopCollections.at(maxStopIndex).push_back(&clusterSet.get(maxStopIndex));
        }
        // now calculate the top cluster of each shape at each spatial window stop
        for (size_t s=0; s < stopCollections.size(); ++s) {
            std::vector<CClusterObject*>& stopClusters =  stopCollections[s];
            boost::shared_ptr<CCluster> topCluster(stopClusters[0]->getClusterPtr());
            //apply compactness correction
            topCluster->m_nRatio *= topCluster->GetNonCompactnessPenalty();
            //if the there are ellipses, compare current top cluster against them
            //note: we don't have to be concerned with whether we are comparing circles and ellipses,
            //     the adjusted loglikelihood ratio for a circle is just the loglikelihood ratio
            for (size_t t=1; t < stopClusters.size(); ++t) {
                if (stopClusters[t]->getCluster().ClusterDefined()) {
                    //apply compactness correction
                    stopClusters[t]->getCluster().m_nRatio *= stopClusters[t]->getCluster().GetNonCompactnessPenalty();
                    //compare against current top cluster
                    if (std::fabs(stopClusters[t]->getCluster().m_nRatio - topCluster->m_nRatio) > DBL_CMP_TOLERANCE && stopClusters[t]->getCluster().m_nRatio > topCluster->m_nRatio)
                        topCluster = stopClusters[t]->getClusterPtr();
                }  
            }
            topClusterCollection.push_back(topCluster);
        }
        return topClusterCollection;
    }
};

//******************************************************************************
#endif
