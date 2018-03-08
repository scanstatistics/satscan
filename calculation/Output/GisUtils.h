//******************************************************************************
#ifndef __GisUtils_H
#define __GisUtils_H
//******************************************************************************

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

//******************************************************************************
#endif
