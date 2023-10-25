//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "LocationRelevance.h"
#include "MostLikelyClustersContainer.h"
#include "SaTScanData.h"

LocationRelevance::LocationRelevance(const CSaTScanData& data_hub) {
    const CParameters& parameters = data_hub.GetParameters();
    unsigned int numIdentifiers = static_cast<unsigned int>(data_hub.GetNumIdentifiers() + data_hub.GetNumMetaIdentifiers());
    if (!(parameters.getReportHierarchicalClusters() || parameters.getReportGiniOptimizedClusters())) {
        _most_likely_only.resize(numIdentifiers, 0);
    }
    if (parameters.getReportHierarchicalClusters()) {
        _hierarchical.resize(numIdentifiers, 0);
    }
    /* We're disabling the gini portion for the time being: https://www.squishlist.com/ims/satscan/66323/
    if (parameters.getReportGiniOptimizedClusters()) {
        _gini_optimal.resize(numIdentifiers, 0);
        _gini_maxima.resize(numIdentifiers, 0);
    }
    if (parameters.getReportHierarchicalClusters() && parameters.getReportGiniOptimizedClusters()) {
        _hierarchical_gini_optimal.resize(numIdentifiers, 0);
        _hierarchical_gini_maxima.resize(numIdentifiers, 0);
    }
    */
}

void LocationRelevance::updatePresence(const CSaTScanData& data_hub, const MostLikelyClustersContainer& mlc, unsigned int maximum, boost::dynamic_bitset<>& location_presence) {
    location_presence.reset();
    maximum = std::min(maximum, static_cast<unsigned int>(mlc.GetNumClustersRetained()));
    for (unsigned int c=0; c < maximum; ++c) {
        const MostLikelyClustersContainer::Cluster_t& cluster = const_cast<MostLikelyClustersContainer&>(mlc).GetClusterRef(static_cast<tract_t>(c));
        for (tract_t t=1; t <= cluster->getNumIdentifiers(); ++t) {
            tract_t tTract = data_hub.GetNeighbor(cluster->GetEllipseOffset(), cluster->GetCentroidIndex(), t, cluster->GetCartesianRadius());
            if (!data_hub.isNullifiedIdentifier(tTract)) {
                location_presence.set(tTract);
            }
        }
    }
}

void LocationRelevance::updateRelevance(const boost::dynamic_bitset<>& location_presence, Relevance_Container_t& location_relevance) {
    assert(location_presence.size() == location_relevance.size());
    for (boost::dynamic_bitset<>::size_type b=location_presence.find_first(); b != boost::dynamic_bitset<>::npos; b=location_presence.find_next(b)) {
        ++location_relevance[b];
    }
}

void LocationRelevance::update(const CSaTScanData& data_hub, const MostLikelyClustersContainer& mlc, unsigned int maximum, boost::dynamic_bitset<>& location_presence, Relevance_Container_t& location_relevance) {
    updatePresence(data_hub, mlc, maximum, location_presence);
    updateRelevance(location_presence, location_relevance);
}
