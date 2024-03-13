//******************************************************************************
#ifndef __ClusterScatterChart_H
#define __ClusterScatterChart_H
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
        double area() const {
            return (_largestX - _smallestX) * (_largestY - _smallestY);
        }
        void setproportional() {
            double xrange = fabs(ceil(_largestX) - floor(_smallestX));
            double yrange = fabs(ceil(_largestY) - floor(_smallestY));
            if (xrange < yrange) {
                double upper = (yrange - xrange) / 2.0;
                double lower = (yrange - xrange) - upper;
                _largestX = static_cast<long>(_largestX + upper);
                _smallestX = floor(_smallestX - lower);
            } else {
                _largestX = static_cast<long>(std::max(xrange, yrange) + _smallestX);
                _smallestX = floor(_smallestX);
            }

            if (xrange > yrange) {
                double upper = (xrange - yrange) / 2.0;
                double lower = (xrange - yrange) - upper;
                _largestY = static_cast<long>(_largestY + upper);
                _smallestY = floor(_smallestY - lower);
            } else {
                _largestY = static_cast<long>(std::max(xrange, yrange) + _smallestY);
                _smallestY = floor(_smallestY);
            }
        }

        double  _largestX;
        double _largestY;
        double _smallestX; 
        double _smallestY;
    };

    protected:
        const CSaTScanData                & _dataHub;
        RegionSettings                      _clusterRegion;
        RegionSettings                      _entireRegion;
        std::stringstream                   _cluster_definitions;
        boost::dynamic_bitset<>             _cluster_locations;
        unsigned int                        _clusters_written;
        double                              _median_parallel;
        std::stringstream                   _cluster_options;

        std::string                       & getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const;
        std::vector<double>               & transform(std::vector<double>& vCoordinates);

    public:
        CartesianGraph(const CSaTScanData& dataHub);
        ~CartesianGraph() {}

        void add(const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars, unsigned int iteration);
        void finalize();

        static FileName& getFilename(FileName& filename);
};
//******************************************************************************
#endif
