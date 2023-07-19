//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MakeNeighbors.h"
#include "SaTScanData.h"
#include "ExponentialRandomizer.h"
#include "SSException.h"
#include "cluster.h" 
#include <boost/dynamic_bitset.hpp>
#include "HomogeneousPoissonDataSetHandler.h"
#include "LocationNetwork.h"
#include "Toolkit.h"
#include "GisUtils.h"

/** constructor */
CentroidNeighborCalculator::CentroidNeighborCalculator(const CSaTScanData& DataHub, BasePrint& PrintDirection)
                           :gPrintDirection(PrintDirection), gParameters(DataHub.GetParameters()),
                            gCentroidInfo(*DataHub.GetGInfo()), _observation_groups(DataHub.GetGroupInfo()),
                            gvEllipseAngles(DataHub.gvEllipseAngles), gvEllipseShapes(DataHub.gvEllipseShapes),
                            gNumTracts(DataHub.GetNumObsGroups()), gtCurrentEllipseCoordinates(0),
                            gPrimaryNeighbors((CALCULATE_NEIGHBORS_METHOD)0,0), 
                            gSecondaryNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,0), 
                            gTertiaryNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,0), 
                            gPrimaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()), 
                            gSecondaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()), 
                            gTertiaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()) {
  gvCentroidDistances.resize(_observation_groups.getNumLocationCoordinates());
  SetupPopulationArrays(DataHub);
  //calculate reported and actual maximum spatial clusters sizes
  CalculateMaximumSpatialClusterSize(DataHub);
  CalculateMaximumReportedSpatialClusterSize(DataHub);
}

/** constructor */
CentroidNeighborCalculator::CentroidNeighborCalculator(const CSaTScanData& DataHub, const ObservationGroupingManager& groupInfo, const GInfo& gridInfo, BasePrint& PrintDirection)
                           :gPrintDirection(PrintDirection), gParameters(DataHub.GetParameters()),
                            gCentroidInfo(gridInfo), _observation_groups(groupInfo),
                            gvEllipseAngles(DataHub.gvEllipseAngles), gvEllipseShapes(DataHub.gvEllipseShapes),
                            gNumTracts(DataHub.GetNumObsGroups()), gtCurrentEllipseCoordinates(0),
                            gPrimaryNeighbors((CALCULATE_NEIGHBORS_METHOD)0,0), 
                            gSecondaryNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,0), 
                            gTertiaryNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,0), 
                            gPrimaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()), 
                            gSecondaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()), 
                            gTertiaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()) {
  gvCentroidDistances.resize(_observation_groups.getNumLocationCoordinates());
  SetupPopulationArrays(DataHub);
  //calculate reported and actual maximum spatial clusters sizes
  CalculateMaximumSpatialClusterSize(DataHub);
  CalculateMaximumReportedSpatialClusterSize(DataHub);
}

/** destructor */
CentroidNeighborCalculator::~CentroidNeighborCalculator() {}

/** Adjusts neighbor counts such that elements at prNeigborsCount.first and prNeigborsCount.second do
    not reference the same coordinates as the element after them. Doing this ensures that all locations
    that reference the same coordinate are either included or excluded in spatial window. */
void CentroidNeighborCalculator::AdjustedNeighborCountsForMultipleCoordinates(std::pair<int, std::vector<int> >& prNeigborsCount) {
  if (gParameters.GetMultipleCoordinatesType() != ONEPERLOCATION) {
    prNeigborsCount.first = getAdjustedNeighborCountsForMultipleCoordinates(prNeigborsCount.first);

    //bool bCalcSecond = prNeigborsCount.first != prNeigborsCount.second;
    for (size_t t=0; t < prNeigborsCount.second.size(); ++t)
       prNeigborsCount.second[t] = getAdjustedNeighborCountsForMultipleCoordinates(prNeigborsCount.second[t]);
  }
}

/** Transforms the x and y coordinates for each location so that circles
    in the transformed space represent ellipsoids in the original space.
    Stores transformed coordinates in internal array. */
void CentroidNeighborCalculator::CalculateEllipticCoordinates(tract_t tEllipseOffset) {
	if (tEllipseOffset == 0 || gtCurrentEllipseCoordinates == tEllipseOffset) return;
	
	double dAngle=GetEllipseAngle(tEllipseOffset), dShape=GetEllipseShape(tEllipseOffset);
	gvLocationEllipticCoordinates.resize(_observation_groups.getLocationsManager().locations().size());
	auto itr=_observation_groups.getLocationsManager().locations().begin(), itrEnd=_observation_groups.getLocationsManager().locations().end();
	for (; itr != itrEnd; ++itr) {
		std::pair<double, double>& eCoords = gvLocationEllipticCoordinates[itr->get()->coordinates()->getInsertionOrdinal()];
		Transform(itr->get()->coordinates()->getCoordinates()[0], itr->get()->coordinates()->getCoordinates()[1], dAngle, dShape, &(eCoords.first), &(eCoords.second));
	}
	gtCurrentEllipseCoordinates = tEllipseOffset;
}

/** Determines the maximum clusters size for reported clusters and assigns appropriate function pointers. */
void CentroidNeighborCalculator::CalculateMaximumReportedSpatialClusterSize(const CSaTScanData& dataHub) {
  measure_t               tPopulation, tTotalPopulation=0;
  const DataSetHandler  & DataSetHandler = dataHub.GetDataSetHandler();
  ReportedCalcPair_t   * pNextToSet = &gPrimaryReportedNeighbors;

  if (!(gParameters.GetRestrictingMaximumReportedGeoClusterSize() || gParameters.getReportGiniOptimizedClusters())) return;

  if (gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON) {
     const HomogeneousPoissonDataSetHandler * pHandler = dynamic_cast<const HomogeneousPoissonDataSetHandler*>(&DataSetHandler);
     if (!pHandler)
       throw prg_error("Could not cast to HomogeneousPoissonDataSetHandler type.","CalculateMaximumReportedSpatialClusterSize()");
     //Calculate the maximum circle radius as percentage of total area in regions.
     measure_t tempMaxRadius = sqrt(pHandler->getTotalArea()/PI);
     std::vector<measure_t> maximums;
     for (size_t t=0; t < gParameters.getExecuteSpatialWindowStops().size(); ++t) {
        double maxRadius = tempMaxRadius * (gParameters.getExecuteSpatialWindowStops()[t] / 100.0);
        //If also restricting max size by fixed distance, take the minimum value of the two.
        if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true))
            maxRadius = std::min(gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, true), maxRadius);
        maximums.push_back(maxRadius);
     }
     pNextToSet->first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance;
     pNextToSet->second = maximums;
     return; // homogeneous poisson only uses this property during neighbors calculation
  }

  if (!(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses())) {
    //NOTE: When input data is defined in multiple data sets, the maximum spatial cluster size is calculated
    //      as a percentage of the total population in all data sets.
    for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
       if (gParameters.GetProbabilityModelType() == ORDINAL ||
           gParameters.GetProbabilityModelType() == CATEGORICAL ||
           gParameters.GetProbabilityModelType() == NORMAL)
         tPopulation = DataSetHandler.GetDataSet(t).getTotalCases();
       else if (gParameters.GetProbabilityModelType() == EXPONENTIAL)
         tPopulation = DataSetHandler.GetDataSet(t).getTotalPopulation();
       else
         tPopulation = DataSetHandler.GetDataSet(t).getTotalMeasure();
       if (tPopulation > std::numeric_limits<measure_t>::max() - tTotalPopulation)
         throw resolvable_error("Error: The total population, summed over of all data sets, exceeds the maximum value allowed of %lf.\n",
                                std::numeric_limits<measure_t>::max());
       tTotalPopulation += tPopulation;
    }
    pNextToSet->first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulationAtRisk;
    std::vector<measure_t> maximums;
    for (size_t t=0; t < gParameters.getExecuteSpatialWindowStops().size(); ++t) {
        maximums.push_back((gParameters.getExecuteSpatialWindowStops()[t] / 100.0) * tTotalPopulation);
    }
    pNextToSet->second = maximums;
    pNextToSet = &gSecondaryReportedNeighbors;
  }
  if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true)) {
    //set maximum circle size based upon percentage of population defined in maximum circle size file
    pNextToSet->first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByMaxCirclePopulation;
    pNextToSet->second = std::vector<measure_t>(1, (gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true) / 100.0) * dataHub.GetMaxCirclePopulationSize());
    pNextToSet = pNextToSet == &gPrimaryReportedNeighbors ? &gSecondaryReportedNeighbors : &gTertiaryReportedNeighbors;
  }
  //set maximum circle size based upon a distance
  if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true)) {
    pNextToSet->first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance;
    pNextToSet->second = std::vector<measure_t>(1, gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, true));
  }
}

/** Determines the maximum clusters size for clusters and assigns appropriate function pointers. */
void CentroidNeighborCalculator::CalculateMaximumSpatialClusterSize(const CSaTScanData& dataHub) {
  measure_t               tPopulation, tTotalPopulation=0;
  const DataSetHandler  & DataSetHandler = dataHub.GetDataSetHandler();
  SecondaryCalcPair_t   * pNextToSet = &gSecondaryNeighbors;

  if (gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON) {
     const HomogeneousPoissonDataSetHandler * pHandler = dynamic_cast<const HomogeneousPoissonDataSetHandler*>(&DataSetHandler);
     if (!pHandler)
       throw prg_error("Could not cast to HomogeneousPoissonDataSetHandler type.","CalculateMaximumSpatialClusterSize()");
     //Calculate the maximum circle radius as percentage of total area in regions.
     measure_t maxRadius = sqrt(pHandler->getTotalArea()/PI);
      maxRadius *= (gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false) / 100.0);
     //If also restricting max size by fixed distance, take the minimum value of the two.
     if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false))
         maxRadius = std::min(gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, false), maxRadius);
      gPrimaryNeighbors.first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance;
      gPrimaryNeighbors.second = maxRadius;
     return; // homogeneous poisson only uses this property during neighbors calculation
  }
   
  if (!(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses())) {
    //set maximum circle size based upon percentage of defined population
    //NOTE: When input data is defined in multiple data sets, the maximum spatial cluster size is calculated
    //      as a percentage of the total population in all data sets.
    for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
       if (gParameters.GetProbabilityModelType() == ORDINAL || 
           gParameters.GetProbabilityModelType() == CATEGORICAL ||
           gParameters.GetProbabilityModelType() == NORMAL)
         tPopulation = DataSetHandler.GetDataSet(t).getTotalCases();
       else if (gParameters.GetProbabilityModelType() == EXPONENTIAL)
         tPopulation = DataSetHandler.GetDataSet(t).getTotalPopulation();
       else
         tPopulation = DataSetHandler.GetDataSet(t).getTotalMeasure();
       if (tPopulation > std::numeric_limits<measure_t>::max() - tTotalPopulation)
         throw resolvable_error("Error: The total population, summed over of all data sets, exceeds the maximum value allowed of %lf.\n",
                                std::numeric_limits<measure_t>::max());
       tTotalPopulation += tPopulation;
    }
    gPrimaryNeighbors.first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulationAtRisk;
    gPrimaryNeighbors.second = (gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false) / 100.0) * tTotalPopulation;
  }
  //set maximum circle size based upon percentage of population defined in maximum circle size file
  if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false)) {
    if (!gPrimaryNeighbors.first) {
      gPrimaryNeighbors.first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByMaxCirclePopulation;
      gPrimaryNeighbors.second = (gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) / 100.0) * dataHub.GetMaxCirclePopulationSize();
      pNextToSet = &gSecondaryNeighbors;
    }
    else {
      pNextToSet->first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByMaxCirclePopulation;
      pNextToSet->second = (gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) / 100.0) * dataHub.GetMaxCirclePopulationSize();
      pNextToSet = &gTertiaryNeighbors;
    }
  }
  //set maximum circle size based upon a distance
  if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false)) {
    if (!gPrimaryNeighbors.first) {
      gPrimaryNeighbors.first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance;
      gPrimaryNeighbors.second = gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, false);
    }
    else {
      pNextToSet->first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance;
      pNextToSet->second = gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, false);
    }
  }
}

/** Calculates neighboring locations about each centroid; storing results in sorted array contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighbors(const CSaTScanData& dataHub) {
    try {
        if (gParameters.UseLocationNeighborsFile()) {
            std::pair<int, std::vector<int> >   prNeighborsCount;
            boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
            gPrintDirection.Printf("Calculating maximum circles\n", BasePrint::P_STDOUT);
            bool frequent_estimations = false;
            int modulas = std::max(1, static_cast<int>(std::floor(dataHub.m_nGridTracts * STANDARD_MODULAS_PERCENT)));
            //Calculate maximum neighboring locations about each centroid for circular regions
            for (tract_t t = 0; t < dataHub.m_nGridTracts; ++t) {
                gvCentroidDistances.resize(dataHub.GetNeighborCountArray()[0][t]);
                //assign gvCentroidToLocationDistances from existing sorted array (populated during neighbors file read)
                for (size_t c = 0; c < gvCentroidDistances.size(); ++c)
                    gvCentroidDistances[c].Set(dataHub.GetNeighbor(0, t, c + 1), 0, 0);
                CalculateNeighborsForCurrentState(prNeighborsCount);
                const_cast<CSaTScanData&>(dataHub).setNeighborCounts(0, t, prNeighborsCount.second, prNeighborsCount.first);
                if (t == 9 || (frequent_estimations && ((t + 1) % modulas == 0)))
                    frequent_estimations = ReportTimeEstimate(StartTime, dataHub.m_nGridTracts, t + 1, gPrintDirection, false, t != 9) > FREQUENT_ESTIMATES_SECONDS;
            }
        } else if (gParameters.getUseLocationsNetworkFile()) {
            const_cast<CSaTScanData&>(dataHub).AllocateSortedArray();
            CalculateNeighborsByNetwork(dataHub);
        } else {
            const_cast<CSaTScanData&>(dataHub).AllocateSortedArray();
            CalculateNeighborsByCircles(dataHub);
            CalculateNeighborsByEllipses(dataHub);
        }  
    } catch (prg_exception& x) {
        x.addTrace("CalculateNeighbors()", "CentroidNeighborCalculator");
        throw;
    }
}

/* Removes multiple coordinates for locations from based on MultipleDataSetPurposeType setting.  */
void CentroidNeighborCalculator::ReduceMultipleCoordinates() {
    switch (gParameters.GetMultipleCoordinatesType()) {
        case ONEPERLOCATION: break;
        case ATLEASTONELOCATION: { 
                boost::dynamic_bitset<> theSet(gNumTracts);
                std::vector<DistanceToCentroid>::iterator itr = gvCentroidDistances.begin();
                for (; itr != gvCentroidDistances.end(); ++itr) {
                    if (theSet.test(itr->GetTractNumber()))
                        itr = gvCentroidDistances.erase(itr) - 1;
                    else
                        theSet.set(itr->GetTractNumber());
                }
            } break;
        case ALLLOCATIONS: { 
                boost::dynamic_bitset<> theSet(gNumTracts);
                std::vector<DistanceToCentroid>::iterator itr(gvCentroidDistances.end() - 1);
                for (;; --itr) {
                    if (theSet.test(itr->GetTractNumber()))
                        itr = gvCentroidDistances.erase(itr);
                    else
                        theSet.set(itr->GetTractNumber());
                    if (itr == gvCentroidDistances.begin()) break;
                }
        } break;
        default: throw prg_error("Unknown multiple coordinate type '%d'.", "CalculateNeighborsAboutCentroid", gParameters.GetMultipleCoordinatesType());
    }
}

/** Calculates closest neighbor's distances to all locations from ellipse/centroid, sorted from closest
    to most distant; storing in gvCentroidDistances class member. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex) {
    CenterLocationDistancesAbout(tEllipseOffsetIndex, tCentroidIndex);
    //sort locations clostest to farthest
    std::sort(gvCentroidDistances.begin(), gvCentroidDistances.end(), CompareGroupDistance(_observation_groups));
    // Apply the multiple coordinates setting to the collection of location distances.
    ReduceMultipleCoordinates();
}

/** Calculates neighboring locations about centroid for given ellipse offset and centroid;
    storing results in CentroidNeighbors object. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid) {
  std::pair<int, std::vector<tract_t> > prNeighborsCount;

  CalculateNeighborsAboutCentroid(tEllipseOffsetIndex, tCentroidIndex);
  CalculateNeighborsForCurrentState(prNeighborsCount);
  AdjustedNeighborCountsForMultipleCoordinates(prNeighborsCount);
  CoupleLocationsAtSameCoordinates(prNeighborsCount);
  Centroid.Set(tEllipseOffsetIndex, tCentroidIndex, prNeighborsCount.first, prNeighborsCount.second, gvCentroidDistances);
}

/** Calculates neighboring locations about centroid for given ellipse offset and centroid.
    Does not sort by increasing distance from centroid but instead iterates through each
    location comparing distance from centroid to location against cluster radius;
    storing results in CentroidNeighbors object. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid, double dMaxRadius) {
  std::vector<DistanceToCentroid>::const_iterator itr=gvCentroidDistances.begin(), itr_end=gvCentroidDistances.end();

  CenterLocationDistancesAbout(tEllipseOffsetIndex, tCentroidIndex);
  //for each location - determine if distance from centroid to location is in/or at radius
  if (gvCentroidDistances.size() < (size_t)std::numeric_limits<unsigned short>::max()) {
    Centroid.gvSortedNeighborsUnsignedShortType.clear();
    for (; itr != itr_end; ++itr)
       if (itr->GetDistance() <= dMaxRadius)
         Centroid.gvSortedNeighborsUnsignedShortType.push_back(static_cast<unsigned short>(itr->GetTractNumber()));
    Centroid.giNeighbors = Centroid.giMaxReportedNeighbors = Centroid.giMaxNeighbors = Centroid.gvSortedNeighborsUnsignedShortType.size();
    Centroid.gpSortedNeighborsUnsignedShortType = (Centroid.giNeighbors ? &Centroid.gvSortedNeighborsUnsignedShortType[0] : 0);
    Centroid.gpSortedNeighborsIntegerType = 0;
  }
  else {
    Centroid.gvSortedNeighborsIntegerType.clear();
    for (; itr != itr_end; ++itr)
       if (itr->GetDistance() <= dMaxRadius)
         Centroid.gvSortedNeighborsIntegerType.push_back(itr->GetTractNumber());
    Centroid.giNeighbors = Centroid.giMaxReportedNeighbors = Centroid.giMaxNeighbors = Centroid.gvSortedNeighborsIntegerType.size();
    Centroid.gpSortedNeighborsIntegerType = (Centroid.giNeighbors ? &Centroid.gvSortedNeighborsIntegerType[0] : 0);
    Centroid.gpSortedNeighborsUnsignedShortType = 0;
  }
  Centroid.gtCentroid = tCentroidIndex;
  Centroid.gtEllipseOffset = tEllipseOffsetIndex;
}

/* Returns the locations in this cluster. When settings allow only a single coordinate, this is just the group/tracts/locations in the cluster. When there
   are possibly multiple coordinates per group/tract, then we need to derive this information. */
void CentroidNeighborCalculator::getLocationsAboutCluster(const CSaTScanData& dataHub, const CCluster& cluster, boost::dynamic_bitset<>* bLocations, std::vector<tract_t>* vLocations) {
    // First search the cache for this information.
    auto itrCache = dataHub._cluster_locations_cache.find(cluster.GetCentroidIndex());
    if (itrCache != dataHub._cluster_locations_cache.end()) {
        if (bLocations) *bLocations = itrCache->second.first;
        if (vLocations) *vLocations = itrCache->second.second;
        return;
    }
    // Otherwise derive the cluster locations.
    boost::dynamic_bitset<> bindexes(dataHub.GetGroupInfo().getLocationsManager().locations().size());
    std::vector<tract_t> vindexes;
    if (dataHub.GetParameters().GetMultipleCoordinatesType() == ONEPERLOCATION) {
        cluster.getGroupIndexes(dataHub, vindexes, true);
        for (auto idx: vindexes) bindexes.set(idx);
    } else if (dataHub.GetParameters().getUseLocationsNetworkFile()) {
        NetworkLocationContainer_t networkLocations;
        dataHub.getClusterNetworkLocations(cluster, networkLocations);
        for (auto nodeLoc: networkLocations) {
            bindexes.set(nodeLoc.first->getLocationIndex());
            vindexes.push_back(nodeLoc.first->getLocationIndex());
        }
    } else if (cluster.GetRadiusDefined()) {
        // Determine which locations are within the circle/ellipse by calculating the distance from centroid to each location,
        // then sorting, and finally taking only those within the radius of cluster.
        std::vector<std::pair<tract_t, double> > groupdistances;
        double distance, clusterRadius = cluster.GetCartesianRadius();
        std::vector<double> clusterCenter, locationCoordinates;
        dataHub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), clusterCenter);
        auto locations = dataHub.GetGroupInfo().getLocationsManager().locations();
        if (cluster.GetEllipseOffset() == 0) { // circular
            for (auto itr = locations.begin(); itr != locations.end(); ++itr) {
                distance = Coordinates::distanceBetween(clusterCenter, itr->get()->coordinates()->retrieve(locationCoordinates));
                if (distance <= clusterRadius) groupdistances.push_back(std::make_pair(std::distance(locations.begin(), itr), distance));
            }
        } else { // elliptic
            double angle = dataHub.gvEllipseAngles[cluster.GetEllipseOffset() - 1], shape = dataHub.gvEllipseShapes[cluster.GetEllipseOffset() - 1];
            Transform(clusterCenter[0], clusterCenter[1], angle, dataHub.gvEllipseShapes[cluster.GetEllipseOffset() - 1], &clusterCenter[0], &clusterCenter[1]);
            for (auto itr = locations.begin(); itr != locations.end(); ++itr) {
                itr->get()->coordinates()->retrieve(locationCoordinates);
                Transform(locationCoordinates[0], locationCoordinates[1], angle, shape, &locationCoordinates[0], &locationCoordinates[1]);
                distance = Coordinates::distanceBetween(clusterCenter, locationCoordinates);
                if (distance <= clusterRadius) groupdistances.push_back(std::make_pair(std::distance(locations.begin(), itr), distance));
            }
        }
        // Now sort the collection of distances, least to greatest, so the order matches how the location wold enter the cluster.
        std::sort(groupdistances.begin(), groupdistances.end(), [](const std::pair<tract_t, double> &left, const std::pair<tract_t, double> &right) {
            if (left.second == right.second)
                return left.first < right.first;
            return left.second < right.second;
        });
        // Now record those observation group indexes to bitset and vector for response and storage.
        for (auto d : groupdistances) { bindexes.set(d.first); vindexes.push_back(d.first); }
    } else 
        throw prg_error("Unknown situation encountered when attempting to get cluster locations.", "getLocationsAboutCluster");
    // Assign cluster locations data to requested data structure(s).
    if (bLocations) *bLocations = bindexes;
    if (vLocations) *vLocations = vindexes;
    // Store data in cache, associated by cluster centroid index.
    dataHub._cluster_locations_cache.insert(std::make_pair(cluster.GetCentroidIndex(), std::make_pair(bindexes, vindexes)));
}

/** Calculates neighboring locations about each centroid through expanding circle;
    storing results in sorted array contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighborsByCircles(const CSaTScanData& dataHub) {
    boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
    std::pair<int, std::vector<tract_t> >   prNeighborsCount;

    bool frequent_estimations = false;
    int modulas = std::max(1, static_cast<int>(std::floor(dataHub.m_nGridTracts * 0.25)));

    gPrintDirection.Printf("Constructing the circles\n", BasePrint::P_STDOUT);
    //Calculate neighboring locations about each centroid for circular regions
    for (tract_t t=0; t < dataHub.m_nGridTracts; ++t) {
        CalculateNeighborsAboutCentroid(0, t);
        CalculateNeighborsForCurrentState(prNeighborsCount);
        AdjustedNeighborCountsForMultipleCoordinates(prNeighborsCount);
        CoupleLocationsAtSameCoordinates(prNeighborsCount);
        const_cast<CSaTScanData&>(dataHub).AllocateSortedArrayNeighbors(gvCentroidDistances, 0, t, prNeighborsCount.second, prNeighborsCount.first);

        if (t == 9 || (frequent_estimations && ((t + 1) % modulas == 0)))
            frequent_estimations = ReportTimeEstimate(StartTime, dataHub.m_nGridTracts, t + 1, gPrintDirection, false, t != 9) > FREQUENT_ESTIMATES_SECONDS;
    }
}

/** Calculates neighboring locations about each centroid by distance; storing
    results in multiple dimension arrays contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighborsByEllipses(const CSaTScanData& dataHub) {
    boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
    std::pair<int, std::vector<int> >   prNeighborsCount;

    //only perform calculation if ellipses requested
    if (gParameters.GetSpatialWindowType() != ELLIPTIC)
        return;

    bool frequent_estimations = false;
    int modulas = std::max(1, static_cast<int>(std::floor((dataHub.m_nGridTracts * gParameters.GetNumTotalEllipses()) * 0.25)));

    gPrintDirection.Printf("Constructing the ellipsoids\n", BasePrint::P_STDOUT);
    //Calculate neighboring locations about each centroid for elliptical regions
    for (int i=1; i <= gParameters.GetNumTotalEllipses(); ++i) {
        for (tract_t t=0; t < dataHub.m_nGridTracts; ++t) {
            CalculateNeighborsAboutCentroid(i, t);
            CalculateNeighborsForCurrentState(prNeighborsCount);
            AdjustedNeighborCountsForMultipleCoordinates(prNeighborsCount);
            CoupleLocationsAtSameCoordinates(prNeighborsCount);
            const_cast<CSaTScanData&>(dataHub).AllocateSortedArrayNeighbors(gvCentroidDistances, i, t, prNeighborsCount.second, prNeighborsCount.first);

            if ((i == 1 && t == 9) || (frequent_estimations && ((i * (t + 1)) % modulas == 0)))
                frequent_estimations = ReportTimeEstimate(StartTime, dataHub.m_nGridTracts * gParameters.GetNumTotalEllipses(), t + 1, gPrintDirection, false, (i != 1 && t != 9)) > FREQUENT_ESTIMATES_SECONDS;

        }
    }
}

/** Calculates neighboring locations about each centroid through expanding network;
storing results in sorted array contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighborsByNetwork(const CSaTScanData& dataHub) {
    boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
    std::pair<int, std::vector<tract_t> >   prNeighborsCount;
    const Network & locationNetwork = dataHub.refLocationNetwork();

    bool frequent_estimations = false;
    int modulas = std::max(1, static_cast<int>(std::floor(static_cast<double>(locationNetwork.getNodes().size()) * 0.25)));

    gPrintDirection.Printf("Constructing the network\n", BasePrint::P_STDOUT);

    // Create a temporary structure to map locations to groups. The network structure is in terms of locations, not groups.
    std::map<const Location*, MinimalGrowthArray<size_t> > _groupings_to_locations;
    for (auto groupItr = _observation_groups.getObservationGroups().begin(); groupItr != _observation_groups.getObservationGroups().end(); ++groupItr) {
        size_t groupIdx = std::distance(_observation_groups.getObservationGroups().begin(), groupItr);
        for (unsigned int i = 0; i < groupItr->get()->getLocations().size(); ++i) {
            const Location * plocation = groupItr->get()->getLocations()[i];
            if (_groupings_to_locations.find(plocation) == _groupings_to_locations.end())
                _groupings_to_locations[plocation] = MinimalGrowthArray<size_t>();
            _groupings_to_locations[plocation].add(groupIdx, true);
        }
    }

    // Calculate neighboring nodes about each node in network.
    Network::NetworkContainer_t::const_iterator itrNode=locationNetwork.getNodes().begin(), end=locationNetwork.getNodes().end();
    for (; itrNode != end; ++itrNode) {
        size_t t = std::distance(locationNetwork.getNodes().begin(), itrNode);
        NetworkLocationContainer_t locationPath;
        locationNetwork.buildNeighborsAboutNode(itrNode->second, locationPath, _observation_groups.getLocationsManager().locations().size());
        //gvCentroidDistances.resize(_observation_groups.getNumLocationCoordinates());
        gvCentroidDistances.clear();
        unsigned int c = 0;
        for (auto itrLoc = locationPath.begin(); itrLoc != locationPath.end(); ++itrLoc) {
            // Translate location index into group index.
            const Location & location = itrLoc->first->getLocation();
            // Which groups are at this location in the network.
            MinimalGrowthArray<size_t>& groups = _groupings_to_locations[&location];
            for (unsigned int g=0; g < groups.size(); ++g) {
                //gvCentroidDistances[c].Set(groups[g], itrLoc->second, 0);
                gvCentroidDistances.push_back(DistanceToCentroid(groups[g], itrLoc->second, 0));
                ++c;
            }
        }

        //gvCentroidDistances = locationPath;

        ReduceMultipleCoordinates();
        CalculateNeighborsForCurrentState(prNeighborsCount);
        AdjustedNeighborCountsForMultipleCoordinates(prNeighborsCount);
        CoupleLocationsAtSameCoordinates(prNeighborsCount);
        const_cast<CSaTScanData&>(dataHub).AllocateSortedArrayNeighbors(gvCentroidDistances, 0, t /*itrNode->first*/, prNeighborsCount.second, prNeighborsCount.first);
        if (t /*itrNode->first*/ == 9 || (frequent_estimations && ((t /*itrNode->first*/ + 1) % modulas == 0)))
            frequent_estimations = ReportTimeEstimate(StartTime, dataHub.m_nGridTracts, t /*itrNode->first*/ + 1, gPrintDirection, false, t /*itrNode->first*/ != 9) > FREQUENT_ESTIMATES_SECONDS;
    }
}


/** Given current state of class data members, calculates the number of neighbors in real data and simulation data. */
void CentroidNeighborCalculator::CalculateNeighborsForCurrentState(std::pair<int, std::vector<int> >& prNeigborsCount) const {
  prNeigborsCount.first = (this->*gPrimaryNeighbors.first)(gPrimaryNeighbors.second);
  if (gSecondaryNeighbors.first) {
    prNeigborsCount.first = (this->*gSecondaryNeighbors.first)(gSecondaryNeighbors.second, prNeigborsCount.first);
    if (gTertiaryNeighbors.first)
      prNeigborsCount.first = (this->*gTertiaryNeighbors.first)(gTertiaryNeighbors.second, prNeigborsCount.first);
  }
  prNeigborsCount.second.clear();
  if (gPrimaryReportedNeighbors.first) {
     int numNeighbors = prNeigborsCount.first;
     std::vector<double> SWS(gPrimaryReportedNeighbors.second);
     std::reverse(SWS.begin(),SWS.end()); // ----> if GINI, then [2,5,10 ..., 50] else [user specified max e.g. 50%]
     for (size_t i=0; i < SWS.size(); ++i) {
         numNeighbors = (this->*gPrimaryReportedNeighbors.first)( SWS[i], numNeighbors );
         if (gSecondaryReportedNeighbors.first) {
             numNeighbors = (this->*gSecondaryReportedNeighbors.first)(gSecondaryReportedNeighbors.second.back(), numNeighbors);
             if (gTertiaryReportedNeighbors.first)
                numNeighbors = (this->*gTertiaryReportedNeighbors.first)(gTertiaryReportedNeighbors.second.back(), numNeighbors);
         }
         prNeigborsCount.second.push_back( numNeighbors );
     }
     std::reverse(prNeigborsCount.second.begin(),prNeigborsCount.second.end());
  } else {
      prNeigborsCount.second.push_back(prNeigborsCount.first);
  }
}


/** Given current state of class data members, calculates the number of neighbors in real data and simulation data. */
/*void CentroidNeighborCalculator::CalculateNeighborsForCurrentState(std::pair<int, int>& prNeigborsCount) const {
  prNeigborsCount.first = (this->*gPrimaryNeighbors.first)(gPrimaryNeighbors.second);
  if (gSecondaryNeighbors.first) {
    prNeigborsCount.first = (this->*gSecondaryNeighbors.first)(gSecondaryNeighbors.second, prNeigborsCount.first);
    if (gTertiaryNeighbors.first)
      prNeigborsCount.first = (this->*gTertiaryNeighbors.first)(gTertiaryNeighbors.second, prNeigborsCount.first);
  }
  prNeigborsCount.second = (!gPrimaryReportedNeighbors.first ? prNeigborsCount.first : (this->*gPrimaryReportedNeighbors.first)(gPrimaryReportedNeighbors.second, prNeigborsCount.first));
  if (gSecondaryReportedNeighbors.first) {
    prNeigborsCount.second = (this->*gSecondaryReportedNeighbors.first)(gSecondaryReportedNeighbors.second, prNeigborsCount.second);
    if (gTertiaryReportedNeighbors.first)
      prNeigborsCount.second = (this->*gTertiaryReportedNeighbors.first)(gTertiaryReportedNeighbors.second, prNeigborsCount.second);
  }
}*/

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size. */
tract_t CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance(measure_t tMaximumSize) const {
  std::vector<DistanceToCentroid>::const_iterator itr=gvCentroidDistances.begin(),
                                                itr_end=gvCentroidDistances.end();
  tract_t                                       tCount=0;

  for (; itr != itr_end && itr->GetDistance() <= tMaximumSize; ++itr) ++tCount;

  return tCount;
}

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size. */
tract_t CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance(measure_t tMaximumSize, count_t tMaximumNeighbors) const {
  std::vector<DistanceToCentroid>::const_iterator itr=gvCentroidDistances.begin(),
                                                itr_end=gvCentroidDistances.end();
  tract_t                                       tCount=0;

  for (; itr != itr_end && tCount + 1 <= tMaximumNeighbors && itr->GetDistance() <= tMaximumSize; ++itr) ++tCount;

  return tCount;
}

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size file. */
tract_t CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByMaxCirclePopulation(measure_t tMaximumSize) const {
  std::vector<DistanceToCentroid>::const_iterator itr=gvCentroidDistances.begin(),
                                                itr_end=gvCentroidDistances.end();
  tract_t                                       tCount=0;
  measure_t                                     tCumMeasure=0;

  for (; itr != itr_end && (tCumMeasure + gpMaxCircleFilePopulation[itr->GetTractNumber()]) <= tMaximumSize; ++itr) {
     tCumMeasure += gpMaxCircleFilePopulation[itr->GetTractNumber()];
     if (tCumMeasure <= tMaximumSize) ++tCount;
  }
  return tCount;
}

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size file. */
tract_t CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByMaxCirclePopulation(measure_t tMaximumSize, count_t tMaximumNeighbors) const {
  std::vector<DistanceToCentroid>::const_iterator itr=gvCentroidDistances.begin(),
                                                itr_end=gvCentroidDistances.end();
  tract_t                                       tCount=0;
  measure_t                                     tCumMeasure=0;

  for (; itr != itr_end &&
         tCount + 1 <= tMaximumNeighbors &&
         tCumMeasure + gpMaxCircleFilePopulation[itr->GetTractNumber()] <= tMaximumSize; ++itr) {

     tCumMeasure += gpMaxCircleFilePopulation[itr->GetTractNumber()];
     ++tCount;
  }
  return tCount;
}

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size. */
tract_t CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulationAtRisk(measure_t tMaximumSize, count_t tMaximumNeighbors) const {
  std::vector<DistanceToCentroid>::const_iterator itr=gvCentroidDistances.begin(),
                                                itr_end=gvCentroidDistances.end();
  tract_t                                       tCount=0;
  measure_t                                     tCumMeasure=0;

  for (; itr != itr_end &&
         tCount + 1 <= tMaximumNeighbors &&
         tCumMeasure + gpPopulation[itr->GetTractNumber()] <= tMaximumSize; ++itr) {

     tCumMeasure += gpPopulation[itr->GetTractNumber()];
     ++tCount;
  }
  return tCount;
}

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances and maximum circle size. */
tract_t CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulationAtRisk(measure_t tMaximumSize) const {
  std::vector<DistanceToCentroid>::const_iterator itr=gvCentroidDistances.begin(),
                                                itr_end=gvCentroidDistances.end();
  tract_t                                       tCount=0;
  measure_t                                     tCumMeasure=0;

  for (; itr != itr_end && (tCumMeasure + gpPopulation[itr->GetTractNumber()]) <= tMaximumSize; ++itr) {
     tCumMeasure += gpPopulation[itr->GetTractNumber()];
     if (tCumMeasure <= tMaximumSize) ++tCount;
  }
  return tCount;
}

/** Calculates distances from centroid to all locations, storing in gvCentroidToLocationDistances object. */
void CentroidNeighborCalculator::CenterLocationDistancesAbout(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex) {
    std::vector<double> vCentroidCoordinates, vLocationCoordinates;

    gCentroidInfo.retrieveCoordinates(tCentroidIndex, vCentroidCoordinates);
    gvCentroidDistances.resize(_observation_groups.getNumLocationCoordinates());
	auto itrGroup = _observation_groups.getObservationGroups().begin(), itrGroupEnd = _observation_groups.getObservationGroups().end();
    if (tEllipseOffsetIndex == 0) {
        //calculate distances from centroid to each location
		for (tract_t k=0, i=0; itrGroup != itrGroupEnd; ++itrGroup, ++k) {
            //if (!(*itr)->isEvaluatedLocation())
            //    //skip locations that are marked as not evaluated in cluster expansion
            //    continue;
			for (unsigned int locIdx = 0; locIdx < itrGroup->get()->getLocations().size(); ++locIdx) {
				itrGroup->get()->getLocations()[locIdx]->coordinates()->retrieve(vLocationCoordinates);
				gvCentroidDistances[i].Set(k, Coordinates::distanceBetween(vCentroidCoordinates, vLocationCoordinates), locIdx);
				++i;
			}
        }
    } else {
        //transform centroid coordinates into elliptical space
        Transform(vCentroidCoordinates[0], vCentroidCoordinates[1], GetEllipseAngle(tEllipseOffsetIndex), GetEllipseShape(tEllipseOffsetIndex), &vCentroidCoordinates[0], &vCentroidCoordinates[1]);
        vLocationCoordinates.resize(2);
        CalculateEllipticCoordinates(tEllipseOffsetIndex);
        //calculate distances from centroid to each location
		for (tract_t k = 0, i = 0; itrGroup != itrGroupEnd; ++itrGroup, ++k) {
            //if (!(*itr)->isEvaluatedLocation())
            //    //skip locations that are marked as not evaluated in cluster expansion
            //    continue;
			for (unsigned int locIdx = 0; locIdx < itrGroup->get()->getLocations().size(); ++locIdx) {
				unsigned int iPosition = itrGroup->get()->getLocations()[locIdx]->coordinates()->getInsertionOrdinal();
				vLocationCoordinates[0] = gvLocationEllipticCoordinates[iPosition].first;
				vLocationCoordinates[1] = gvLocationEllipticCoordinates[iPosition].second;
				gvCentroidDistances[i].Set(k, Coordinates::distanceBetween(vCentroidCoordinates, vLocationCoordinates), locIdx);
				++i;
			}
        }
    }
}

/** Scans gvCentroidToLocationDistances for adjacent locations that reference the same coordinates.
    These will have to grouped into a 'meta-location' to ensure that these neighbors are evaluated
    at the same time, as one observation group. This method relies on gvCentroidToLocationDistances being
    sorted by distance from target ellipse/centroid and that locations referencing the same coordinates
    are adjacent to each other. */
void CentroidNeighborCalculator::CoupleLocationsAtSameCoordinates(std::pair<int, std::vector<int> >& prNeighborsCount) {
    if (gParameters.GetMultipleCoordinatesType() == ONEPERLOCATION) return;
    std::vector<DistanceToCentroid>::iterator tGroupStart=gvCentroidDistances.begin(), tGroupEnd=gvCentroidDistances.end();
    for (int tCurrent=0; tCurrent < prNeighborsCount.first; ++tCurrent) {
        DistanceToCentroid& curr = gvCentroidDistances[tCurrent];
        const Coordinates * currCoords = _observation_groups.getObservationGroups()[curr.GetTractNumber()]->getLocations()[curr.GetRelativeCoordinateIndex()]->coordinates().get();
        DistanceToCentroid& next = gvCentroidDistances[tCurrent+1];
        const Coordinates * nextCoords = _observation_groups.getObservationGroups()[next.GetTractNumber()]->getLocations()[next.GetRelativeCoordinateIndex()]->coordinates().get();
        if (*currCoords == *nextCoords)
            tGroupEnd = gvCentroidDistances.begin() + (tCurrent + 1);
        else if (tGroupEnd != gvCentroidDistances.end()) {
            //create meta location that is locations from tGroupStart to tGroupEnd
            std::vector<int> indexes;
            for (std::vector<DistanceToCentroid>::iterator itr=tGroupStart; itr != tGroupEnd+1; ++itr)
                indexes.push_back(itr->GetTractNumber());
            DistanceToCentroid meta_location(gNumTracts + const_cast<ObservationGroupingManager&>(_observation_groups).getMetaNeighborManager().add(indexes), tGroupStart->GetDistance());
            //replace tGroupStart to tGroupEnd with new LocationDistance that is the meta location
            tGroupStart = gvCentroidDistances.erase(tGroupStart, tGroupEnd + 1);
            tGroupStart = gvCentroidDistances.insert(tGroupStart, meta_location) + 1;
            tCurrent = std::distance(gvCentroidDistances.begin(), tGroupStart) - 1;
            tGroupEnd = gvCentroidDistances.end();
            //adjust actual neighbor count to account for grouped locations
            prNeighborsCount.first -= indexes.size() - 1;
            //adjust reported neighbor count for grouped locations if insertion point overlapped with reported neighbors
            for (size_t t=0; t < prNeighborsCount.second.size(); ++t) {
                if (tCurrent < prNeighborsCount.second[t]) //tCurrent is pointing to inserted meta location
                    prNeighborsCount.second[t] -= std::min((size_t)prNeighborsCount.second[t], indexes.size() - 1);
            }
        } else {
            ++tGroupStart;//advance group start
        }
    }
}

/** Calculates the adjusted number of neighbors for current state of gvCentroidToLocationDistances object,
    ensuring that either all locations that reference the same coordinate are included or excluded.
    This method relies on gvCentroidToLocationDistances being sorted such that locations referencing the
    same coordinates are adjacent to each other. */
int CentroidNeighborCalculator::getAdjustedNeighborCountsForMultipleCoordinates(int iNeigborsCount) {
    if (iNeigborsCount != 0 && (size_t)iNeigborsCount != gvCentroidDistances.size()) {
        //get coordinates of current farthest neighbor
        DistanceToCentroid& currMax = gvCentroidDistances[iNeigborsCount-1];
        const Coordinates * currMaxCoords = _observation_groups.getObservationGroups()[currMax.GetTractNumber()]->getLocations()[currMax.GetRelativeCoordinateIndex()]->coordinates().get();
        //get coordinates of neighbor after current farthest
        DistanceToCentroid& beyondMax = gvCentroidDistances[iNeigborsCount];
        const Coordinates * beyondMaxCoords = _observation_groups.getObservationGroups()[beyondMax.GetTractNumber()]->getLocations()[beyondMax.GetRelativeCoordinateIndex()]->coordinates().get();
        while (*currMaxCoords == *beyondMaxCoords) {
             if (--iNeigborsCount < 1) break;
             DistanceToCentroid& currMax = gvCentroidDistances[iNeigborsCount-1];
             currMaxCoords = _observation_groups.getObservationGroups()[currMax.GetTractNumber()]->getLocations()[currMax.GetRelativeCoordinateIndex()]->coordinates().get();
        }
    }
    return iNeigborsCount;
}

/** Returns angle to referenced ellipse at index. Caller is responsible for ensuring
    that iEllipseIndex is within validate range where if ellipses where requested,
    the valid range is 0 to gParameters.GetNumTotalEllipses(). */
double CentroidNeighborCalculator::GetEllipseAngle(int iEllipseIndex) const {
  return (iEllipseIndex == 0 ? 0.0 : gvEllipseAngles[iEllipseIndex - 1]);
}

/** Returns shape to referenced ellipse at index. Caller is responsible for ensuring
    that iEllipseIndex is within validate range where if ellipses where requested,
    the valid range is 0 to gParameters.GetNumTotalEllipses(). */
double CentroidNeighborCalculator::GetEllipseShape(int iEllipseIndex) const {
  return (iEllipseIndex == 0 ? 1.0 : gvEllipseShapes[iEllipseIndex - 1]);
}

/** Returns the coordinates of tract given centroid of passed cluster object.
    This process takes into account the MultipleCoordinatesType. */
std::vector<double>& CentroidNeighborCalculator::getTractCoordinates(const CSaTScanData& DataHub, const CCluster& Cluster, tract_t tTract, std::vector<double>& tractCoordinates, NetworkLocationContainer_t * clusterNetwork)  {
    // If tract index refers to a meta neighbor index, retrieve the index it maps to then proceed.
    if (tTract >= DataHub.GetNumObsGroups()) tTract = DataHub.GetGroupInfo().getMetaNeighborManager().getFirst(tTract - DataHub.GetNumObsGroups());
    if (DataHub.GetParameters().GetMultipleCoordinatesType() == ONEPERLOCATION || DataHub.GetGroupInfo().getObservationGroups()[tTract]->getLocations().size() == 1) {
        // If user have specified that multiple coordinates are one-per-location or this tract has only only coordinate, just retrieve the coordinates.
        return DataHub.GetGroupInfo().getObservationGroups()[tTract]->getLocations()[0]->coordinates()->retrieve(tractCoordinates);
    } else if (DataHub.GetParameters().getUseLocationsNetworkFile() && Cluster.getNumObservationGroups() > 1) {
        boost::shared_ptr<NetworkLocationContainer_t> localClusterNetwork;
        if (!clusterNetwork) {
            // NetworkLocationContainer_t wasn't passed as function argument, so calculate this information now.
            localClusterNetwork.reset(new NetworkLocationContainer_t());
            clusterNetwork = localClusterNetwork.get();
            const Location& centroidLocation = dynamic_cast<const NetworkCentroidHandlerPassThrough*>(DataHub.GetGInfo())->getCentroidLocation(Cluster.GetCentroidIndex());
            const NetworkNode& centroidNode = DataHub.refLocationNetwork().getNodes().find(DataHub.GetGroupInfo().getLocationsManager().getLocation(centroidLocation.name()).first.get())->second;
            DataHub.refLocationNetwork().buildNeighborsAboutNode(centroidNode, *clusterNetwork, DataHub.GetGroupInfo().getLocationsManager().locations().size());
        }
        const Coordinates *pTarget = 0;
        double dCurrent = -1;
        for (unsigned int i = 0; i < DataHub.GetGroupInfo().getObservationGroups()[tTract]->getLocations().size(); ++i) {
            const Location * tractLocation = DataHub.GetGroupInfo().getObservationGroups()[tTract]->getLocations()[i];
            // tractLocation->index()
            auto tractNode = &(DataHub.refLocationNetwork().getNodes().find(DataHub.GetGroupInfo().getLocationsManager().getLocation(tractLocation->name()).first.get())->second);
            double dDistance = -1;
            for (auto itr = clusterNetwork->begin(); itr != clusterNetwork->end(); ++itr) {
                if (itr->first == tractNode) {
                    dDistance = itr->second;
                    break;
                }
            }

            if (dDistance == -1) continue;
            //if (dDistance == -1) throw prg_error("Unable to determine distance between cluster center location '%s'.", "getDistanceBetween()", tractLocation->name().c_str());

            //dDistance = DataHub.refLocationNetwork().getDistanceBetween(centroidLocation, *tractLocation, DataHub.GetGroupInfo());
            switch (DataHub.GetParameters().GetMultipleCoordinatesType()) {
                case ATLEASTONELOCATION: if (!pTarget || dCurrent > dDistance) { pTarget = tractLocation->coordinates().get(); dCurrent = dDistance; } break; // Searching for the closest coordinate.
                case ALLLOCATIONS: if (!pTarget || dCurrent < dDistance) { pTarget = tractLocation->coordinates().get(); dCurrent = dDistance; } break; //Searching for the farthest coordinate.
                default: throw prg_error("Unknown multiple coordinates type '%d'.", "getTractCoordinates()", DataHub.GetParameters().GetMultipleCoordinatesType());
            }
        }
        if (!pTarget) throw prg_error("Unable to determine distance between cluster center and tract '%d'.", "getDistanceBetween()", tTract);
        return pTarget->retrieve(tractCoordinates);
    } else {
        // Otherwise figure out which coordinate the neighbors calculation determined as the actual coordinate it used for this tract.
		double dCurrent, dDistance,	dAngle = DataHub.GetEllipseAngle(Cluster.GetEllipseOffset()), dShape = DataHub.GetEllipseShape(Cluster.GetEllipseOffset());
		std::vector<double> clusterCenter, tractCoords;
		const Coordinates * pCoordinates, *pTarget = 0;
        // Retrieve the coordinates of the cluster centroid, transforming if needed.
		DataHub.GetGInfo()->retrieveCoordinates(Cluster.GetCentroidIndex(), clusterCenter);
		if (Cluster.GetEllipseOffset() > 0)	Transform(clusterCenter[0], clusterCenter[1], dAngle, dShape, &clusterCenter[0], &clusterCenter[1]);
        // Iterate through tract coordinates to find the correct one by distance from centroid.
		for (unsigned int i=0; i < DataHub.GetGroupInfo().getObservationGroups()[tTract]->getLocations().size(); ++i) {
			pCoordinates = DataHub.GetGroupInfo().getObservationGroups()[tTract]->getLocations()[i]->coordinates().get();
			pCoordinates->retrieve(tractCoords);
			if (Cluster.GetEllipseOffset() > 0) Transform(tractCoords[0], tractCoords[1], dAngle, dShape, &tractCoords[0], &tractCoords[1]);
            // Calculate the distance from cluster center to this coordinate, then determine if this is the best candidate.
			dDistance = Coordinates::distanceBetween(clusterCenter, tractCoords);
            switch (DataHub.GetParameters().GetMultipleCoordinatesType()) {
                case ATLEASTONELOCATION: if (!pTarget || dCurrent > dDistance) { pTarget = pCoordinates; dCurrent = dDistance; } break; // Searching for the closest coordinate.
                case ALLLOCATIONS: if (!pTarget || dCurrent < dDistance) { pTarget = pCoordinates; dCurrent = dDistance; } break; //Searching for the farthest coordinate.
                default: throw prg_error("Unknown multiple coordinates type '%d'.", "getTractCoordinates()", DataHub.GetParameters().GetMultipleCoordinatesType());
            }
		}
		return pTarget->retrieve(tractCoordinates);
	}
}

/** Debug function. Prints elements of gvCentroidToLocationDistances object to FILE stream in formatted ASCII text. */
void CentroidNeighborCalculator::printCentroidToLocationDistances(size_t tMaxToPrint, FILE * stream) {
  FILE * fp=0;
  if (!stream) {
    if ((fp = fopen("CentroidToLocationDistances.print", "a+")) == NULL) return;
    stream = fp;
  }
  std::vector<DistanceToCentroid>::const_iterator itr=gvCentroidDistances.begin(), itr_end;
  itr_end = (tMaxToPrint >= gvCentroidDistances.size() ? gvCentroidDistances.end() : gvCentroidDistances.begin() + tMaxToPrint);
  fprintf(stream, "\nCentroidToLocationDistances (size=%ld):\n", std::distance(itr, itr_end));
  for (; itr != itr_end; ++itr) {
    if (itr->GetTractNumber() >= gNumTracts) {
      std::vector<tract_t> indexes;
	  _observation_groups.getMetaNeighborManager().getIndexes(itr->GetTractNumber() - gNumTracts, indexes);
      fprintf(stream, "distance=%.6lf\t\tindexes=", itr->GetDistance());
      for (size_t i=0; i < indexes.size(); ++i)
        fprintf(stream, "%s%d", (i == 0 ? "" : ","), indexes.at(i));
      fprintf(stream, "\n");
    }
    else {
	  auto group = _observation_groups.getObservationGroups()[itr->GetTractNumber()];
	  auto pCoords = group->getLocations()[itr->GetRelativeCoordinateIndex()]->coordinates().get();
      for (size_t t=0; t < pCoords->getSize(); ++t)
        fprintf(stream, "%s%g", (t == 0 ? "coordinates=": ","), pCoords->getCoordinates()[t]);
      fprintf(stream, "\t\tdistance=%.6lf\t\tname='%s'\t\tindex=%d\n", itr->GetDistance(), group->groupname().c_str(), itr->GetTractNumber());
    }
  }
  if (fp) fclose(fp);
}

/** Resizes passed vector to #of tracts plus # of meta locations. Assigns meta location data. */
void CentroidNeighborCalculator::setMetaLocations(std::vector<measure_t>& popMeasure) {
  popMeasure.resize(gNumTracts + _observation_groups.getMetaObsGroupsManager().getNumReferenced(), 0);

  std::vector<tract_t> atomicIndexes;
  for (size_t t=0; t < _observation_groups.getMetaObsGroupsManager().getNumReferenced(); ++t) {
	  _observation_groups.getMetaObsGroupsManager().getAtomicIndexes(t, atomicIndexes);
     for (size_t a=0; a < atomicIndexes.size(); ++a)
       popMeasure[(size_t)gNumTracts + t] += popMeasure[atomicIndexes[a]];
  }
}

/** Based upon parameter settings, stores references to population arrays maintained by
    CSaTScanData object or calculates population, storing in class member structure. */
void CentroidNeighborCalculator::SetupPopulationArrays(const CSaTScanData& dataHub) {
  count_t                     * pCases;
  const ExponentialRandomizer * pRandomizer;
  const DataSetHandler        & DataSetHandler = dataHub.GetDataSetHandler();

  //store reference to population defined in max circle file if either restricting in either real or simulation process
  if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) || gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true))
    gpMaxCircleFilePopulation = &dataHub.GetMaxCirclePopulationArray()[0];
  //prospective space-time analyses, using prospective start date, do not use the population at risk to restrict maximum spatial size
  if (!(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses())) {
    switch (gParameters.GetProbabilityModelType()) {
      case NORMAL  :
        gvCalculatedPopulations.resize(dataHub.GetNumObsGroups(), 0);
        for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
          pCases = DataSetHandler.GetDataSet(t).getCaseData().GetArray()[0];
          for (int j=0; j < dataHub.GetNumObsGroups(); ++j)
             gvCalculatedPopulations[j] += pCases[j];
        }
        setMetaLocations(gvCalculatedPopulations);
        gpPopulation = &gvCalculatedPopulations[0]; break;
      case CATEGORICAL :
      case ORDINAL :
        //For the Ordinal model, populations for each location are calculated by adding up the
        //total individuals represented in the catgory case arrays.
        gvCalculatedPopulations.resize(dataHub.GetNumObsGroups(), 0);
        for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
           for (unsigned int k=0; k < DataSetHandler.GetDataSet(t).getCaseData_Cat().size(); ++k) {
             pCases = DataSetHandler.GetDataSet(t).getCaseData_Cat()[k]->GetArray()[0];
             for (int j=0; j < dataHub.GetNumObsGroups(); ++j)
                gvCalculatedPopulations[j] += pCases[j];
           }
        }
        setMetaLocations(gvCalculatedPopulations);
        gpPopulation = &gvCalculatedPopulations[0];  break;
      case EXPONENTIAL:
        // consider population as cases and non-censored cases
        gvCalculatedPopulations.resize(dataHub.GetNumObsGroups(), 0);
        for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
          pRandomizer = dynamic_cast<const ExponentialRandomizer*>(DataSetHandler.GetRandomizer(t));
          if (!pRandomizer) throw prg_error("Randomizer failed cast to ExponentialRandomizer.", "CalculateMaximumReportedSpatialClusterSize()");
            pRandomizer->CalculateMaxCirclePopulationArray(gvCalculatedPopulations, false);
        }
        setMetaLocations(gvCalculatedPopulations);
        gpPopulation = &gvCalculatedPopulations[0]; break;
      case HOMOGENEOUSPOISSON:
          break; // This model strictly uses distance to calculate neighbors ...
      default :
        gvCalculatedPopulations.resize(dataHub.GetNumObsGroups(), 0);
        //Population is calculated from all data sets
        for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
           measure_t * pMeasure = DataSetHandler.GetDataSet(t).getMeasureData().GetArray()[0];
           for (int j=0; j < dataHub.GetNumObsGroups(); ++j)
             gvCalculatedPopulations[j] += pMeasure[j];
        }
        setMetaLocations(gvCalculatedPopulations);
        gpPopulation = &gvCalculatedPopulations[0]; break;
    }
  }
}

/** This function transforms the x and y coordinates so that circles
    in the transformed space represent ellipsoids in the original space.
    The input is the old X and Y coordinates, the angle of the longest
    axis of the ellipsoid (0<=EllipseAngle<pi), and the EllipseShape (>1),
    which is defined as the length divided by the width.
    The output is the new X and Y coordinates.     - MK 5.2001 */
void CentroidNeighborCalculator::Transform(double Xold, double Yold, float EllipseAngle, float EllipseShape, double* pXnew, double* pYnew) {
   double Xnew;
   double Ynew;
   float beta;    // slope of the line perpendicular to the ellipsoids longest axis, y=beta*x
   double Xp,Yp;   // projection of (Xold,Yold) onto the line perpendicular to the ellipsoids longest axis
   float Weight;

   Weight=1/EllipseShape;

   if (EllipseAngle==0) {
     Xnew=Xold*Weight;
     Ynew=Yold;
   }
   else {
     beta=-cos(EllipseAngle)/sin(EllipseAngle);
     Xp=(Xold+beta*Yold)/(1+beta*beta);
     Yp=beta*Xp;
     Xnew=Xold*Weight+Xp*(1-Weight);
     Ynew=Yold*Weight+Yp*(1-Weight);
   }
   
   *pXnew=Xnew;
   *pYnew=Ynew;
}

