//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "GisUtils.h"
#include "RandomDistribution.h"
#include "cluster.h"
#include "SaTScanData.h"

/* Returns the beginning and ending points of cluster radius segment. */
GisUtils::pointpair_t GisUtils::getClusterRadiusSegmentPoints(const CSaTScanData& datahub, const CCluster& cluster) {
    std::vector<double> vCoordinates, TractCoords;
    std::pair<double, double> prLatitudeLongitude, pointOnCircumference;

    //set focal point of this cluster - cluster centriod
    datahub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), vCoordinates);
    prLatitudeLongitude = ConvertToLatLong(vCoordinates);

	// If this analysis is using a network file, then we'll only have a radius around the cluster center,
	// so force the prLatitudeLongitude == pointOnCircumference block below;
	if (datahub.GetParameters().getUseLocationsNetworkFile()) {
		pointOnCircumference = prLatitudeLongitude;
	} else {
		// get the coordinates of location farthest away from center
		CentroidNeighborCalculator::getTractCoordinates(datahub, cluster, datahub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), cluster.GetNumTractsInCluster()), TractCoords);
		pointOnCircumference = ConvertToLatLong(TractCoords);
	}

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

double GisUtils::EARTH_RADIUS_METERS = 6378137;

/* Returns a collection of sequential points that define a linear ring around center point. 
   This code was addapted from php source at KML Circle Generator (http://dev.bt23.org/keyhole/circlegen/output.phps)
   and Google Earth Circle Generator (http://www.techidiots.net/autoit-scripts/google-earth-circle-generator/circle%20generator.au3/view) */
GisUtils::points_t GisUtils::getPointsOnCircleCircumference(point_t centerPoint, point_t pointOnCircumference) {

    // convert coordinates to radians
    double d_rad = getRadiusInMeters(centerPoint, pointOnCircumference) / EARTH_RADIUS_METERS;
    double lat1 = degrees2radians(centerPoint.first);
    double long1 = degrees2radians(centerPoint.second);
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

double GisUtils::getRadiusInMeters(point_t centerPoint, point_t pointOnCircumference) {
    // convert coordinates to radians
    double lat1 = degrees2radians(centerPoint.first);
    double long1 = degrees2radians(centerPoint.second);
    double lat2 = degrees2radians(pointOnCircumference.first);
    double long2 = degrees2radians(pointOnCircumference.second);

    // get the difference between lat/long coords
    double dlat = lat2 - lat1;
    double dlong = long2 - long1;

    // calculate the radius by computing distance of great circle
    double a = std::pow((std::sin(dlat / 2.0)), 2) + std::cos(lat1) * std::cos(lat2) * std::pow((std::sin(dlong / 2.0)), 2);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    // get distance between points (in meters)
    return EARTH_RADIUS_METERS * c;
}

///////////////////////// VisualizationUtils ///////////////////////////////////////////////////

std::string VisualizationUtils::getRandomHtmlColor() const {
    // The order of expression is aabbggrr, where aa = alpha(00 to ff); bb = blue(00 to ff); gg = green(00 to ff); rr = red(00 to ff).
    long bb = Equilikely(static_cast<long>(0), static_cast<long>(255), _rng),
        gg = Equilikely(static_cast<long>(0), static_cast<long>(255), _rng),
        rr = Equilikely(static_cast<long>(0), static_cast<long>(255), _rng);
    std::stringstream ss;
    ss << "#" << std::hex << std::setw(2) << std::setfill('0') << rr << std::setw(2) << std::setfill('0') << gg << std::setw(2) << std::setfill('0') << bb;
    return ss.str();
}

/* Returns KML color as html color code. */
std::string VisualizationUtils::toHtmlColor(const std::string& colorKML) const {
    if (colorKML.size() != 8) throw prg_error("Can not convert kml color %s to html color.", colorKML.c_str());
    std::stringstream s;
    s << "#" << colorKML.substr(6, 2) << colorKML.substr(4, 2) << colorKML.substr(2, 2);
    return s.str();
}


