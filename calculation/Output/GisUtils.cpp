//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "GisUtils.h"
#include "RandomDistribution.h"
#include "cluster.h"
#include "SaTScanData.h"

Network::Connection_Details_t GisUtils::getClusterConnections(const NetworkLocationContainer_t& networkLocations) {
    // First create a set of all locations that are in this location container.
    std::set<const Location *> locations;
    for (auto itr = networkLocations.begin(); itr != networkLocations.end(); ++itr)
        locations.emplace(&(itr->first->getLocation()));

    // Now construct the edges between network locations.
    Network::Connection_Details_t connections;
    for (auto itr = networkLocations.begin(); itr != networkLocations.end(); ++itr) {
        const Location* nodeLocation = &(itr->first->getLocation());
        // Define edge from this node to another connecting node if other is in locations.
        for (auto itrC = itr->first->getConnections().begin(); itrC != itr->first->getConnections().end(); ++itrC) {
                const Location * connectionLocation = &(itrC->get<0>()->getLocation());
                if (locations.find(connectionLocation) == locations.end()) continue;
                connections.emplace(Network::Connection_Detail_t(std::min(nodeLocation, connectionLocation), std::max(nodeLocation, connectionLocation)));
        }
    }
    return connections;
}

/* Returns the unique set of network connections. */
Network::Connection_Details_t GisUtils::getNetworkConnections(const Network& network) {
    Network::Connection_Details_t connections;
    for (const auto& node : network.getNodes()) {
        const Location* nodeLocation = &node.second.getLocation();
        // Define edge from this node to connecting nodes.
        for (const auto& connectedNode: node.second.getConnections()) {
            const Location * connectionLocation = &connectedNode.get<0>()->getLocation();
            connections.emplace(Network::Connection_Detail_t(std::min(nodeLocation, connectionLocation), std::max(nodeLocation, connectionLocation)));
        }
    }
    return connections;
}

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
		CentroidNeighborCalculator::getTractCoordinates(datahub, cluster, datahub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), cluster.getNumIdentifiers()), TractCoords);
		pointOnCircumference = ConvertToLatLong(TractCoords);
	}

    if (prLatitudeLongitude == pointOnCircumference) {
        // 1 arcsec = 0.000278
        // 1/2 arcsec translates into roughly 26 meters in Google Earth, basically one house/small apartment
        pointOnCircumference.first += (0.000278 * 0.5);
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

/* Efficiently calculates the distance between the two furthest locations within the cluster by calculating the irregular diamond about the most northern,
   southern, eastern, and western locations in cluster. Then finding the locations outside that shape which are the furthest apart. */
double GisUtils::calculateDistanceFurthestLocations(const CSaTScanData& datahub, const CCluster& cluster) {
    const CParameters& params = datahub.GetParameters();
    if (cluster.GetClusterType() == PURELYTEMPORALCLUSTER || (params.GetCoordinatesType() == CARTESIAN && datahub.getLocationsManager().expectedDimensions() != 2))
        return -2.0; // not implemented for these situations
    bool spansAntiMeridian = false;
    std::vector<double> coordinates;
    auto transformCoordinates = [&coordinates](const CParameters& parameters, bool spansAntiMeridian) { // transforms 3-dimensional coordinates back to latitude/longitude
        if (parameters.GetCoordinatesType() != LATLON) return;
        point_t prLatitudeLongitude = ConvertToLatLong(coordinates);
        coordinates.clear();
        coordinates.push_back(spansAntiMeridian && prLatitudeLongitude.second < 0.0 ? prLatitudeLongitude.second + 360.0 : prLatitudeLongitude.second); // X-coordinate
        coordinates.push_back(prLatitudeLongitude.first);  // Y-coordinate
    };
    // Find the four cluster locations that are furthest north, south, east, and west. (There are usually four, but could be three or even just two).
    std::vector<tract_t> vLocations;
    CentroidNeighborCalculator::getLocationsAboutCluster(datahub, cluster, 0, &vLocations);
    const auto& locations = datahub.getLocationsManager().locations();
    point_t northMost, southMost, westMost, eastMost;
    auto calculateMosts = [&coordinates, &northMost, &southMost, &westMost, &eastMost, transformCoordinates](const CSaTScanData& datahub, const std::vector<tract_t>& vLocations, bool spansAntiMeridian) {
        northMost = point_t(0.0, -std::numeric_limits<double>::max()); southMost = point_t(0.0, std::numeric_limits<double>::max());
        westMost = point_t(std::numeric_limits<double>::max(), 0.0); eastMost = point_t(-std::numeric_limits<double>::max(), 0.0);
        const auto& locations = datahub.getLocationsManager().locations();
        for (auto index : vLocations) {
            locations[index].get()->coordinates()->retrieve(coordinates);
            // transform 3-d coordinates back to latitude/longitude - this will allow use to calculate slope and intercept in either coordinate system
            transformCoordinates(datahub.GetParameters(), spansAntiMeridian);
            if (coordinates.back() > northMost.second) { northMost.first = coordinates.front(); northMost.second = coordinates.back(); } // is it the northen most
            if (coordinates.back() < southMost.second) { southMost.first = coordinates.front(); southMost.second = coordinates.back(); } // is it the southern most
            if (coordinates.front() < westMost.first) { westMost.first = coordinates.front(); westMost.second = coordinates.back(); } // is it the western most
            if (coordinates.front() > eastMost.first) { eastMost.first = coordinates.front(); eastMost.second = coordinates.back(); } // is it the eastern most
        }
        /*printf(
            "NorthMost: (%g, %g)\nSouthMost: (%g, %g)\nEastMost: (%g, %g)\nWesthMost: (%g, %g)\n",
            northMost.first, northMost.second, southMost.first, southMost.second, eastMost.first, eastMost.second, westMost.first, westMost.second
        );*/
    };
    calculateMosts(datahub, vLocations, spansAntiMeridian);
    // There are some issues with our algorithm when northern or southern points near the poles, so don't calculate span for clusters in those regions.
    if (params.GetCoordinatesType() == LATLON && (northMost.second > 80.0 || southMost.second < -80.0)) return -2.0;
    // Special consideration when cluster crosses the 180 merian (antimeridian).
    if (params.GetCoordinatesType() == LATLON && eastMost.first - westMost.first > 300) {
        // Special situation when cluster spans antimeridian - mark this a special case then recalculate mosts.
        spansAntiMeridian = true;
        calculateMosts(datahub, vLocations, spansAntiMeridian);
    }
    auto getslopeintercept = [](const point_t& northsouth, const point_t& eastwest) { // calculates slope and intercept of line
        std::pair<double, double> line;
        line.first = (eastwest.first - northsouth.first) ? (eastwest.second - northsouth.second) / (eastwest.first - northsouth.first): 0.0; // slope
        line.second = eastwest.second - line.first * eastwest.first; // intercept
        return line;
    };
    auto lineNE = getslopeintercept(northMost, eastMost), lineNW = getslopeintercept(northMost, westMost);
    auto lineSE = getslopeintercept(southMost, eastMost), lineSW = getslopeintercept(southMost, westMost);
    auto includeNorth = [](const std::vector<double>& coordinates, const std::pair<double, double>& line) {
        return coordinates.back() > line.second + line.first * coordinates.front();
    };
    auto includeSouth = [](const std::vector<double>& coordinates, const std::pair<double, double>& line) {
        return coordinates.back() < line.second + line.first * coordinates.front();
    };
    // Attempt to assign each location in the cluster to a bin - the hope is that the majority don't fall into any bin.
    std::set<point_t> binNE = { northMost, eastMost }, binNW = { northMost, westMost }, binSE = { southMost, eastMost }, binSW = { southMost, westMost }/*, binCatchAll*/;
    for (auto index : vLocations) { // Figure out to which bin each location belongs (might not belong in any bin).
        locations[index].get()->coordinates()->retrieve(coordinates);
        transformCoordinates(params, spansAntiMeridian); // transform as needed
        if (includeNorth(coordinates, lineNE)) {
            binNE.insert(point_t(coordinates.front(), coordinates.back()));
        } else if (includeNorth(coordinates, lineNW)) {
            binNW.insert(point_t(coordinates.front(), coordinates.back()));
        } else if (includeSouth(coordinates, lineSE)) {
            binSE.insert(point_t(coordinates.front(), coordinates.back()));
        } else if (includeSouth(coordinates, lineSW)) {
            binSW.insert(point_t(coordinates.front(), coordinates.back()));
        } /*else  binCatchAll.insert(point_t(coordinates.front(), coordinates.back())); */
    }
    /*printf("Total locations: %u\n", vLocations.size());
    auto printBin = [](const std::string& name, const std::set<point_t>& bin) {
        printf("bin %s size = %u\n", name.c_str(), bin.size()); for (auto point : bin) { printf("(%g, %g)\n", point.first, point.second); }
    };
    printBin("north-east", binNE); printBin("north-west", binNW); printBin("south-east", binSE); printBin("south-west", binSW);*/ // printBin("catch-all", binCatchAll);
    // Variables and function to facilitate the maximum distance.
    double maxDistance = 0.0;
    std::pair<point_t, point_t> furthestPoints;
    auto furthestCartesian = [&furthestPoints, &maxDistance](const std::set<point_t>& binNorth, const std::set<point_t>& binSouth) {
        double distance;
        for (auto pointN : binNorth) {
            for (auto pointS : binSouth) {
                distance = std::sqrt(std::pow(pointN.first - pointS.first, 2) + std::pow(pointN.second - pointS.second, 2));
                if (distance > maxDistance) { maxDistance = distance; furthestPoints = std::make_pair(pointN, pointS); }
            }
        }
    };
    auto furthestGeographic = [&furthestPoints, &maxDistance](const std::set<point_t>& binNorth, const std::set<point_t>& binSouth) {
        double distance;
        std::vector<double> pointNorth = {0.0, 0.0, 0.0}, pointSouth = { 0.0, 0.0, 0.0 };
        for (auto pointN : binNorth) {
            ConvertFromLatLong(pointN.second, pointN.first, pointNorth); // convert to 3-d space for distance calculation
            for (auto pointS : binSouth) {
                ConvertFromLatLong(pointS.second, pointS.first, pointSouth); // convert to 3-d space for distance calculation
                distance = Coordinates::distanceBetween(pointNorth, pointSouth);
                if (distance > maxDistance) { maxDistance = distance; furthestPoints = std::make_pair(pointN, pointS); }
            }
        }
    };
    if (params.GetCoordinatesType() == LATLON) {
        furthestGeographic(binNE, binSW); // For all the locations in the NE bin, calculate the distances to all locations in the SW bin.    
        furthestGeographic(binNW, binSE); // For all the locations in the NW bin, calculate the distances to all the locations in the SE bin.
    } else {
        furthestCartesian(binNE, binSW); // For all the locations in the NE bin, calculate the distances to all locations in the SW bin.    
        furthestCartesian(binNW, binSE); // For all the locations in the NW bin, calculate the distances to all the locations in the SE bin.
    }
    if (params.GetCoordinatesType() == LATLON && maxDistance) // Recalculate the distance between furthest coordinates in terms if kilometers.
        maxDistance = 2 * EARTH_RADIUS_km * asin(maxDistance / (2 * EARTH_RADIUS_km));
    /*printf(
        "furthest points are: (%g, %g) to (%g, %g) with distance %g.\n\n",
        furthestPoints.first.first, furthestPoints.first.second, furthestPoints.second.first, furthestPoints.second.second, maxDistance
    );*/
    return maxDistance;
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

/** Returns the value used by the noUiSlider control of visualizations such as Google Maps and Cartesian graph. */
double VisualizationUtils::getSliderValue(const CSaTScanData& datahub, const CCluster& cluster, unsigned int iReportedCluster, const SimulationVariables& simVars) {
    const auto& parameters = datahub.GetParameters();
    if (parameters.GetIsProspectiveAnalysis() && !datahub.isDrilldown()) {
        const double MIN_RECURRANCE_VALUE = 0.0, MAX_RECURRANCE_VALUE = 730000000;
        if (cluster.reportableRecurrenceInterval(parameters, simVars))
            return std::min(cluster.GetRecurrenceInterval(datahub, iReportedCluster, simVars).second, MAX_RECURRANCE_VALUE);
        return MIN_RECURRANCE_VALUE;
    } 
    const double MIN_PVALUE_VALUE = 0.000001, MAX_PVALUE_VALUE = 1.0;
    if (cluster.reportablePValue(parameters, simVars))
        return std::max(cluster.getReportingPValue(parameters, simVars, parameters.GetIsIterativeScanning() || iReportedCluster == 1), MIN_PVALUE_VALUE);
    return MAX_PVALUE_VALUE;
}

/** Returns cluster legend to be used as popup in html page. */
std::string& VisualizationUtils::getHtmlClusterLegend(const CCluster& cluster, int iCluster, const CSaTScanData& datahub, std::string& legend) {
    std::string buffer, buffer2, buffer3, buffer4;
    std::stringstream  legendLines;
    const CParameters& parameters = datahub.GetParameters();
    unsigned int currSetIdx = std::numeric_limits<unsigned int>::max(), numFilesSets = parameters.getNumFileSets();

    legendLines << "<div style=\"text-decoration:underline;font-weight:bold;margin-bottom:5px;\">Cluster " << iCluster + 1 << "</div>";
    if (numFilesSets == 1) {
        legendLines << "<table border=\"0\" style=\"width:100%;\">";
        for (auto rptline: cluster.getReportLinesCache()) {
            legendLines << printString(buffer,
                "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">%s:</th><td style=\"white-space:nowrap;text-align:left;\">%s</td></tr>",
                htmlencode(rptline.first, buffer2).c_str(), htmlencode(rptline.second.first, buffer3).c_str()
            );
        }
        legendLines << "</table>";
    } else {
        std::stringstream clusterLines, clusterDataSetLines;
        clusterLines << "<table border=\"0\" style=\"width:100%;\">";
        for (auto rptline : cluster.getReportLinesCache()) {
            if (rptline.second.second == std::numeric_limits<unsigned int>::max()) { // cluster level
                clusterLines << printString(buffer,
                    "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;color:#333;font-weight:400;\">%s:</th><td style=\"white-space:nowrap;text-align:right;\">%s</td></tr>",
                    htmlencode(rptline.first, buffer2).c_str(), htmlencode(rptline.second.first, buffer3).c_str()
                );
            } else { // cluster data set level
                if (currSetIdx != rptline.second.second) {
                    if (currSetIdx != std::numeric_limits<unsigned int>::max()) clusterDataSetLines << "</table>";
                    clusterDataSetLines << "<table border=\"0\" style=\"width:100%;\">";
                    clusterDataSetLines << "<caption style=\"text-align:left;white-space:nowrap;padding:2px 0 2px 0;text-decoration:underline;font-weight:bold;color:#555;\">";
                    clusterDataSetLines << getWrappedText(htmlencode(
                            printString(buffer4, "%s (set %i)", 
                            parameters.getDataSourceNames()[datahub.GetDataSetHandler().getDataSetRelativeIndex(rptline.second.second)].c_str(),
                            (rptline.second.second + 1)), buffer2, false
                        ), 0, 50, "<br>", buffer3
                    );


                    clusterDataSetLines << "</caption>";
                    currSetIdx = rptline.second.second;
                }
                clusterDataSetLines << printString(buffer,
                    "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;color:#333;font-weight:400;\">%s:</th><td style=\"white-space:nowrap;text-align:right;\">%s</td></tr>",
                    htmlencode(rptline.first, buffer2).c_str(), htmlencode(rptline.second.first, buffer3).c_str()
                );
            }
        }
        legendLines << clusterLines.str() << "</table>" << clusterDataSetLines.str() << "</table>";
    }
    legend = legendLines.str();
    std::replace(legend.begin(), legend.end(), '\n', ' ');
    return legend;
}