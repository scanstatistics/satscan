//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "GisUtils.h"
#include "cluster.h"
#include "SaTScanData.h"

/* Returns the beginning and ending points of cluster radius segment. */
GisUtils::pointpair_t GisUtils::getClusterRadiusSegmentPoints(const CSaTScanData& datahub, const CCluster& cluster) {
    std::vector<double> vCoordinates, TractCoords;
    std::pair<double, double> prLatitudeLongitude;

    //set focal point of this cluster - cluster centriod
    datahub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), vCoordinates);
    prLatitudeLongitude = ConvertToLatLong(vCoordinates);
    // get the coordinates of location farthest away from center
    CentroidNeighborCalculator::getTractCoordinates(datahub, cluster, datahub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), cluster.GetNumTractsInCluster()), TractCoords);
    std::pair<double, double> pointOnCircumference = ConvertToLatLong(TractCoords);
    if (prLatitudeLongitude == pointOnCircumference) {
        /* If the cluster only has one location, the centroid, then get the nearest neighbor - per Martin.
           This is more difficult than I initially thought. First, I'm not sure he meant nearest point in 
           terms of any point defined. I think he meant in terms of clusters reported in kml. I'm also thinking
           that this includes points on drawn circles -- otherwise if the cluster locations of neighboring cluster
           are on it's farside, then the clusters could overlap.
        */
        // Temporary implementation until we figure out the best course here.  Move north 5 degrees to create a point on circumference.
        pointOnCircumference.first += (0.000278 * 5.0);
    }
    return std::make_pair(prLatitudeLongitude,pointOnCircumference);
}


/* Returns a collection of sequential points that define a linear ring around center point. 
   This code was addapted from php source at KML Circle Generator (http://dev.bt23.org/keyhole/circlegen/output.phps)
   and Google Earth Circle Generator (http://www.techidiots.net/autoit-scripts/google-earth-circle-generator/circle%20generator.au3/view) */
GisUtils::points_t GisUtils::getPointsOnCircleCircumference(point_t centerPoint, point_t pointOnCircumference) {
    double EARTH_RADIUS_METERS = 6378137;

    // convert coordinates to radians
    double lat1 = degrees2radians(centerPoint.first);
    double long1 = degrees2radians(centerPoint.second);
    double lat2 = degrees2radians(pointOnCircumference.first);
    double long2 = degrees2radians(pointOnCircumference.second);

    // get the difference between lat/long coords
    double dlat = lat2 - lat1;
    double dlong = long2 - long1;

    // calculate the radius by computing distance of great circle
    double a = std::pow((std::sin(dlat/2.0)), 2) + std::cos(lat1) * std::cos(lat2) * std::pow((std::sin(dlong/2.0)), 2);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    // get distance between points (in meters)
    double d = EARTH_RADIUS_METERS * c;
    double d_rad = d/EARTH_RADIUS_METERS;

    points_t circlePoints;
    for (int i=0; i <= 360; ++i) {
        double radial = degrees2radians(i);
        double lat_rad = std::asin(std::sin(lat1) * std::cos(d_rad) + std::cos(lat1) * std::sin(d_rad) * std::cos(radial));
        double dlon_rad = std::atan2(std::sin(radial) * std::sin(d_rad) * std::cos(lat1), std::cos(d_rad) - std::sin(lat1) * std::sin(lat_rad));
        double lon_rad = std::fmod((long1 + dlon_rad + PI), 2 * PI) - PI;
        circlePoints.push_back(point_t(radians2degrees(lon_rad), radians2degrees(lat_rad)));
    }
    return circlePoints;
}
