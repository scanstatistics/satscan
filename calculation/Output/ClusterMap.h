//******************************************************************************
#ifndef __ClusterMap_H
#define __ClusterMap_H
//******************************************************************************
#include<vector>
#include "SimulationVariables.h"
#include "MostLikelyClustersContainer.h"
#include <fstream>

class CSaTScanData;

class ClusterMap {
    typedef std::vector<std::pair<double, double> > points_t;
    static const char * HTML_FILE_EXT;
    static const char * FILE_SUFFIX_EXT;
    static const char * TEMPLATE;
    static const char * API_KEY;

protected:
    const CSaTScanData                & _dataHub;
    std::stringstream                   _cluster_definitions;
    boost::dynamic_bitset<>             _cluster_locations;
    unsigned int                        _clusters_written;

    static std::stringstream          & templateReplace(std::stringstream& templateText, const std::string& replaceStub, const std::string& replaceWith);
    std::string                       & getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const;

public:
    ClusterMap(const CSaTScanData& dataHub);
    ~ClusterMap() {}

    void add(const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars);
    void finalize();

    static FileName& getFilename(FileName& filename);
};
//******************************************************************************
#endif
