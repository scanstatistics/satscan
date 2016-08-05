//******************************************************************************
#ifndef __ClusterScatterChart_H
#define __ClusterScatterChart_h
//******************************************************************************
#include<vector>
#include "SimulationVariables.h"
#include "MostLikelyClustersContainer.h"
#include <fstream>

class CSaTScanData;

class CartesianGraph {
    typedef std::vector<std::pair<double, double> > points_t;
    static const char * HTML_FILE_EXT;
    static const char * FILE_SUFFIX_EXT;
    static const char * TEMPLATE;

protected:
    const CSaTScanData                & _dataHub;
    const MostLikelyClustersContainer & _clusters;
    const SimulationVariables         & _simVars;

    static std::stringstream          & templateReplace(std::stringstream& templateText, const std::string& replaceStub, const std::string& replaceWith);
    std::string                       & getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const;

public:
    CartesianGraph(const CSaTScanData& dataHub, const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars)
        :_dataHub(dataHub), _clusters(clusters), _simVars(simVars) {}
    ~CartesianGraph() {}

    void generateChart();
    static FileName& getFilename(FileName& filename);

    //static std::string                & changeColor(std::string& s, long i, RandomNumberGenerator & rng);

};
//******************************************************************************
#endif
