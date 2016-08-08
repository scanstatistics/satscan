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

    class RegionSettings {
    public:
        RegionSettings() :
            _largestX(-std::numeric_limits<double>::max()), _largestY(-std::numeric_limits<double>::max()),
            _smallestX(std::numeric_limits<double>::max()), _smallestY(std::numeric_limits<double>::max()) {}

        bool in(double x, double y) {
            return (y > _smallestY && y < _largestY) && (x > _smallestX && x < _largestX);
        }

        double  _largestX;
        double _largestY;
        double _smallestX; 
        double _smallestY;
    };

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
