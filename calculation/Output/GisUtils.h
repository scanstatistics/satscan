//******************************************************************************
#ifndef __GisUtils_H
#define __GisUtils_H
//******************************************************************************

#include "RandomNumberGenerator.h"

class CCluster;
class CSaTScanData;

class GisUtils {
    public:
        typedef std::pair<double, double> point_t;
        typedef std::vector<point_t> points_t;
        typedef std::pair<point_t,point_t> pointpair_t;

        static double EARTH_RADIUS_METERS;

        static pointpair_t getClusterRadiusSegmentPoints(const CSaTScanData& datahub, const CCluster& cluster);
        static points_t getPointsOnCircleCircumference(point_t centerPoint, point_t pointOnCircumference);
        static double getRadiusInMeters(point_t centerPoint, point_t pointOnCircumference);
};

class VisualizationUtils {
    protected:
        mutable RandomNumberGenerator _rng;
        std::vector<std::string> _event_color_defaults;

    public:
        VisualizationUtils() {
            _event_color_defaults = { // seperate colors for each group.
                "#f78f0c", "#4d83b2", "#f9cc00", "#73bc86", "#ca1640", "#4dc3c1", "#d1d09e", "#3c494f", "#c5db53",
                "#6c3025", "#66665a", "#106c47", "#f27959", "#b27c4d", "#580a31", "#a19f0c", "#e1d0a3"
            };
        }

        const std::vector<std::string>& getEventColorDefaults() const { return _event_color_defaults; }
        std::string getRandomHtmlColor() const;
        std::string toHtmlColor(const std::string& kmlColor) const;
};

//******************************************************************************
#endif
