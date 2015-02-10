//*****************************************************************************
#ifndef __LocationRelevance_H
#define __LocationRelevance_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include <boost/dynamic_bitset.hpp>

class CParameters;
class CSaTScanData;
class MostLikelyClustersContainer;

class LocationRelevance {
    public:
        typedef std::vector<unsigned int> Relevance_Container_t;

        Relevance_Container_t _most_likely_only; // most likely cluster only
        Relevance_Container_t _hierarchical; // hierarchical clusters
        /* We're disabling the gini portion for the time being: https://www.squishlist.com/ims/satscan/66323/
        Relevance_Container_t _gini_optimal; // optimal gini clusters
        Relevance_Container_t _gini_maxima; // maxima matching gini clusters
        Relevance_Container_t _hierarchical_gini_optimal; // hierarchical or optimal gini clusters
        Relevance_Container_t _hierarchical_gini_maxima; // hierarchical or maxima matching gini clusters
        */

        LocationRelevance() {}
        LocationRelevance(const CSaTScanData& data_hub);

        static void updatePresence(const CSaTScanData& data_hub, const MostLikelyClustersContainer& mlc, unsigned int maximum, boost::dynamic_bitset<>& location_presence);
        static void updateRelevance(const boost::dynamic_bitset<>& location_presence, Relevance_Container_t& location_relevance);
        static void update(const CSaTScanData& data_hub, const MostLikelyClustersContainer& mlc, unsigned int maximum, boost::dynamic_bitset<>& location_presence, Relevance_Container_t& location_relevance);
};
//*****************************************************************************
#endif
