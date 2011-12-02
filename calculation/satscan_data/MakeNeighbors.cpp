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

/** constructor */
CentroidNeighborCalculator::CentroidNeighborCalculator(const CSaTScanData& DataHub, BasePrint& PrintDirection)
                           :gPrintDirection(PrintDirection), gParameters(DataHub.GetParameters()),
                            gCentroidInfo(*DataHub.GetGInfo()), gLocationInfo(*DataHub.GetTInfo()), 
                            gvEllipseAngles(DataHub.gvEllipseAngles), gvEllipseShapes(DataHub.gvEllipseShapes),
                            gNumTracts(DataHub.GetNumTracts()), gtCurrentEllipseCoordinates(0),
                            gPrimaryNeighbors((CALCULATE_NEIGHBORS_METHOD)0,0), 
							gSecondaryNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,0), 
							gTertiaryNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,0), 
                            gPrimaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()), 
							gSecondaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()), 
							gTertiaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()) {
  gvCentroidToLocationDistances.resize(gLocationInfo.getNumLocationCoordinates());
  SetupPopulationArrays(DataHub);
  //calculate reported and actual maximum spatial clusters sizes
  CalculateMaximumSpatialClusterSize(DataHub);
  CalculateMaximumReportedSpatialClusterSize(DataHub);
}

/** constructor */
CentroidNeighborCalculator::CentroidNeighborCalculator(const CSaTScanData& DataHub, const TractHandler& tractHandler, const GInfo& gridInfo, BasePrint& PrintDirection)
                           :gPrintDirection(PrintDirection), gParameters(DataHub.GetParameters()),
                            gCentroidInfo(gridInfo), gLocationInfo(tractHandler), 
                            gvEllipseAngles(DataHub.gvEllipseAngles), gvEllipseShapes(DataHub.gvEllipseShapes),
                            gNumTracts(DataHub.GetNumTracts()), gtCurrentEllipseCoordinates(0),
                            gPrimaryNeighbors((CALCULATE_NEIGHBORS_METHOD)0,0), 
							gSecondaryNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,0), 
							gTertiaryNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,0), 
                            gPrimaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()), 
							gSecondaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()), 
							gTertiaryReportedNeighbors((CALCULATE_NEIGHBORS_LIMIT_METHOD)0,std::vector<measure_t>()) {
  gvCentroidToLocationDistances.resize(DataHub.GetTInfo()->getNumLocationCoordinates());
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
  double                dAngle=GetEllipseAngle(tEllipseOffset),
                        dShape=GetEllipseShape(tEllipseOffset);

  if (tEllipseOffset == 0 || gtCurrentEllipseCoordinates == tEllipseOffset)
    return;

  gvLocationEllipticCoordinates.resize(gLocationInfo.getCoordinates().size());
  for (size_t t=0; t < gLocationInfo.getCoordinates().size(); ++t) {
     const TractHandler::Coordinates* pCoords = gLocationInfo.getCoordinates()[t];
     std::pair<double, double>& eCoords = gvLocationEllipticCoordinates[pCoords->getInsertionOrdinal()];
     Transform(pCoords->getCoordinates()[0], pCoords->getCoordinates()[1], dAngle, dShape, &(eCoords.first), &(eCoords.second));
  }
  gtCurrentEllipseCoordinates = tEllipseOffset;
}

/** Determines the maximum clusters size for reported clusters and assigns appropriate function pointers. */
void CentroidNeighborCalculator::CalculateMaximumReportedSpatialClusterSize(const CSaTScanData& dataHub) {
  measure_t               tPopulation, tTotalPopulation=0;
  const DataSetHandler  & DataSetHandler = dataHub.GetDataSetHandler();
  ReportedCalcPair_t   * pNextToSet = &gPrimaryReportedNeighbors;

  if (!(gParameters.GetRestrictingMaximumReportedGeoClusterSize() || gParameters.optimizeSpatialClusterSize())) return;

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

/** Calculates neighboring locations about each centroid; storing results in sorted
    array contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighbors(const CSaTScanData& dataHub) {
  try {
    if (gParameters.UseLocationNeighborsFile()) {
      std::pair<int, std::vector<int> >   prNeighborsCount;
      boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
      gPrintDirection.Printf("Calculating maximum circles\n", BasePrint::P_STDOUT);
      //Calculate maximum neighboring locations about each centroid for circular regions
      for (tract_t t=0; t < dataHub.m_nGridTracts; ++t) {
         gvCentroidToLocationDistances.resize(dataHub.GetNeighborCountArray()[0][t]);
         //assign gvCentroidToLocationDistances from existing sorted array (populated during neighbors file read)
         for (size_t c=0; c < gvCentroidToLocationDistances.size(); ++c)
           gvCentroidToLocationDistances[c].Set(dataHub.GetNeighbor(0, t, c+1), 0, 0);
         CalculateNeighborsForCurrentState(prNeighborsCount);
         const_cast<CSaTScanData&>(dataHub).setNeighborCounts(0, t, prNeighborsCount.second, prNeighborsCount.first);
         if (t == 9) ReportTimeEstimate(StartTime, dataHub.m_nGridTracts, t, &gPrintDirection);
      }
    }
    else {
      const_cast<CSaTScanData&>(dataHub).AllocateSortedArray();
      CalculateNeighborsByCircles(dataHub);
      CalculateNeighborsByEllipses(dataHub);
    }  
  }
  catch (prg_exception& x) {
    x.addTrace("CalculateNeighbors()", "CentroidNeighborCalculator");
    throw;
  }
}

/** Calculates closest neighbor's distances to all locations from ellipse/centroid, sorted from closest
    to most distant; storing in gvCentroidToLocationDistances class member. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex) {
  CenterLocationDistancesAbout(tEllipseOffsetIndex, tCentroidIndex);
  //sort locations clostest to farthest
  std::sort(gvCentroidToLocationDistances.begin(), gvCentroidToLocationDistances.end(), CompareLocationDistance(gLocationInfo));
  switch (gParameters.GetMultipleCoordinatesType()) {
    case ONEPERLOCATION : break;
    case ATLEASTONELOCATION :
    { boost::dynamic_bitset<> theSet(gNumTracts);
      std::vector<LocationDistance>::iterator itr=gvCentroidToLocationDistances.begin();
      for (;itr != gvCentroidToLocationDistances.end(); ++itr) {
          if (theSet.test(itr->GetTractNumber()))
            itr = gvCentroidToLocationDistances.erase(itr) - 1;
          else
            theSet.set(itr->GetTractNumber());
      }
    } break;
    case ALLLOCATIONS :
    { boost::dynamic_bitset<> theSet(gNumTracts);
      std::vector<LocationDistance>::iterator itr(gvCentroidToLocationDistances.end() - 1);
      for (;; --itr) {
          if (theSet.test(itr->GetTractNumber()))
            itr = gvCentroidToLocationDistances.erase(itr);
          else
            theSet.set(itr->GetTractNumber());
          if (itr == gvCentroidToLocationDistances.begin()) break;
      }
    } break;
    default : throw prg_error("Unknown multiple coordinate type '%d'.", "CalculateNeighborsAboutCentroid", gParameters.GetMultipleCoordinatesType());
  }
}

/** Calculates neighboring locations about centroid for given ellipse offset and centroid;
    storing results in CentroidNeighbors object. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid) {
  std::pair<int, std::vector<tract_t> > prNeighborsCount;

  CalculateNeighborsAboutCentroid(tEllipseOffsetIndex, tCentroidIndex);
  CalculateNeighborsForCurrentState(prNeighborsCount);
  AdjustedNeighborCountsForMultipleCoordinates(prNeighborsCount);
  CoupleLocationsAtSameCoordinates(prNeighborsCount);
  Centroid.Set(tEllipseOffsetIndex, tCentroidIndex, prNeighborsCount.first, prNeighborsCount.second, gvCentroidToLocationDistances);
}

/** Calculates neighboring locations about centroid for given ellipse offset and centroid.
    Does not sort by increasing distance from centroid but instead iterates through each
    location comparing distance from centroid to location against cluster radius;
    storing results in CentroidNeighbors object. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid, double dMaxRadius) {
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(), itr_end=gvCentroidToLocationDistances.end();

  CenterLocationDistancesAbout(tEllipseOffsetIndex, tCentroidIndex);
  //for each location - determine if distance from centroid to location is in/or at radius
  if (gvCentroidToLocationDistances.size() < (size_t)std::numeric_limits<unsigned short>::max()) {
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

/** Calculates neighboring locations about each centroid through expanding circle;
    storing results in sorted array contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighborsByCircles(const CSaTScanData& dataHub) {
  boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
  std::pair<int, std::vector<tract_t> >   prNeighborsCount;

  gPrintDirection.Printf("Constructing the circles\n", BasePrint::P_STDOUT);
  //Calculate neighboring locations about each centroid for circular regions
  for (tract_t t=0; t < dataHub.m_nGridTracts; ++t) {
     CalculateNeighborsAboutCentroid(0, t);
     CalculateNeighborsForCurrentState(prNeighborsCount);
     AdjustedNeighborCountsForMultipleCoordinates(prNeighborsCount);
     CoupleLocationsAtSameCoordinates(prNeighborsCount);
     const_cast<CSaTScanData&>(dataHub).AllocateSortedArrayNeighbors(gvCentroidToLocationDistances, 0, t, prNeighborsCount.second, prNeighborsCount.first);
     if (t == 9) ReportTimeEstimate(StartTime, dataHub.m_nGridTracts, t, &gPrintDirection);
  }
}

/** Calculates neighboring locations about each centroid by distance; storing
    results in multiple dimension arrays contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighborsByEllipses(const CSaTScanData& dataHub) {
  boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
  std::pair<int, std::vector<int> >   prNeighborsCount;

  //only perform calculation if ellipses requested
  if (!gParameters.GetSpatialWindowType() == ELLIPTIC)
    return;

  gPrintDirection.Printf("Constructing the ellipsoids\n", BasePrint::P_STDOUT);
  //Calculate neighboring locations about each centroid for elliptical regions
  for (int i=1; i <= gParameters.GetNumTotalEllipses(); ++i) {
     for (tract_t t=0; t < dataHub.m_nGridTracts; ++t) {
        CalculateNeighborsAboutCentroid(i, t);
        CalculateNeighborsForCurrentState(prNeighborsCount);
        AdjustedNeighborCountsForMultipleCoordinates(prNeighborsCount);
        CoupleLocationsAtSameCoordinates(prNeighborsCount);
        const_cast<CSaTScanData&>(dataHub).AllocateSortedArrayNeighbors(gvCentroidToLocationDistances, i, t, prNeighborsCount.second, prNeighborsCount.first);
        if (t == 9 && i == 1) ReportTimeEstimate(StartTime, dataHub.m_nGridTracts * gParameters.GetNumTotalEllipses(), t, &gPrintDirection);
     }
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
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
  tract_t                                       tCount=0;

  for (; itr != itr_end && itr->GetDistance() <= tMaximumSize; ++itr) ++tCount;

  return tCount;
}

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size. */
tract_t CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance(measure_t tMaximumSize, count_t tMaximumNeighbors) const {
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
  tract_t                                       tCount=0;

  for (; itr != itr_end && tCount + 1 <= tMaximumNeighbors && itr->GetDistance() <= tMaximumSize; ++itr) ++tCount;

  return tCount;
}

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size file. */
tract_t CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByMaxCirclePopulation(measure_t tMaximumSize) const {
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
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
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
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
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
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
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
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
  std::vector<double>           vCentroidCoordinates, vLocationCoordinates;

  gCentroidInfo.retrieveCoordinates(tCentroidIndex, vCentroidCoordinates);
  gvCentroidToLocationDistances.resize(gLocationInfo.getNumLocationCoordinates());
  TractHandler::LocationsContainer_t::const_iterator  itr=gLocationInfo.getLocations().begin(),
                                                      itr_end=gLocationInfo.getLocations().end();
  if (tEllipseOffsetIndex == 0) {
    //calculate distances from centroid to each location
    for (tract_t k=0, i=0; itr != itr_end; ++itr, ++k) {
       //if (!(*itr)->isEvaluatedLocation())
       //    //skip locations that are marked as not evaluated in cluster expansion
       //    continue;

       for (unsigned int c=0; c < (*itr)->getCoordinates().size(); ++c) {
         (*itr)->getCoordinates()[c]->retrieve(vLocationCoordinates);
         gvCentroidToLocationDistances[i].Set(k, std::sqrt(gLocationInfo.getDistanceSquared(vCentroidCoordinates, vLocationCoordinates)), c);
         ++i;
       }
    }
  }
  else {
    //tranform centroid coordinates into elliptical space
    Transform(vCentroidCoordinates[0], vCentroidCoordinates[1], GetEllipseAngle(tEllipseOffsetIndex),
              GetEllipseShape(tEllipseOffsetIndex), &vCentroidCoordinates[0], &vCentroidCoordinates[1]);
    vLocationCoordinates.resize(2);
    CalculateEllipticCoordinates(tEllipseOffsetIndex);
    //calculate distances from centroid to each location
    for (tract_t k=0, i=0; itr != itr_end; ++itr, ++k) {
       //if (!(*itr)->isEvaluatedLocation())
       //    //skip locations that are marked as not evaluated in cluster expansion
       //    continue;

       for (unsigned int c=0; c < (*itr)->getCoordinates().size(); ++c) {
          unsigned int iPosition = (*itr)->getCoordinates()[c]->getInsertionOrdinal();
          vLocationCoordinates[0] = gvLocationEllipticCoordinates[iPosition].first;
          vLocationCoordinates[1] = gvLocationEllipticCoordinates[iPosition].second;
          gvCentroidToLocationDistances[i].Set(k, std::sqrt(gLocationInfo.getDistanceSquared(vCentroidCoordinates, vLocationCoordinates)), c);
          ++i;
       }
    }
  }
}

/** Scans gvCentroidToLocationDistances for adjacent locations that reference the same coordinates.
    These will have to grouped into a 'meta-location' to ensure that these neighbors are evaluated
    at the same time, as one location. This method relies on gvCentroidToLocationDistances being
    sorted by distance from target ellipse/centroid and that locations referencing the same coordinates
    are adjacent to each other. */
void CentroidNeighborCalculator::CoupleLocationsAtSameCoordinates(std::pair<int, std::vector<int> >& prNeighborsCount) {
  if (gParameters.GetMultipleCoordinatesType() == ONEPERLOCATION) return;
  std::vector<LocationDistance>::iterator tGroupStart=gvCentroidToLocationDistances.begin(), tGroupEnd=gvCentroidToLocationDistances.end();
  for (int tCurrent=0; tCurrent < prNeighborsCount.first; ++tCurrent) {
    LocationDistance& curr = gvCentroidToLocationDistances[tCurrent];
    const TractHandler::Coordinates * currCoords = gLocationInfo.getLocations()[curr.GetTractNumber()]->getCoordinates()[curr.GetRelativeCoordinateIndex()];
    LocationDistance& next = gvCentroidToLocationDistances[tCurrent+1];
    const TractHandler::Coordinates * nextCoords = gLocationInfo.getLocations()[next.GetTractNumber()]->getCoordinates()[next.GetRelativeCoordinateIndex()];
    if (currCoords == nextCoords)
      tGroupEnd = gvCentroidToLocationDistances.begin() + (tCurrent + 1);
    else if (tGroupEnd != gvCentroidToLocationDistances.end()) {
      //create meta location that is locations from tGroupStart to tGroupEnd
      std::vector<int> indexes;
      for (std::vector<LocationDistance>::iterator itr=tGroupStart; itr != tGroupEnd+1; ++itr)
         indexes.push_back(itr->GetTractNumber());
      LocationDistance meta_location(gNumTracts + const_cast<TractHandler&>(gLocationInfo).getMetaNeighborManager().add(indexes), tGroupStart->GetDistance());
      //replace tGroupStart to tGroupEnd with new LocationDistance that is the meta location
      tGroupStart = gvCentroidToLocationDistances.erase(tGroupStart, tGroupEnd + 1);
      tGroupStart = gvCentroidToLocationDistances.insert(tGroupStart, meta_location) + 1;
      tCurrent = std::distance(gvCentroidToLocationDistances.begin(), tGroupStart) - 1;
      tGroupEnd = gvCentroidToLocationDistances.end();
      //adjust actual neighbor count to account for grouped locations
      prNeighborsCount.first -= indexes.size() - 1;
      //adjust reported neighbor count for grouped locations if insertion point overlapped with reported neighbors
      for (size_t t=0; t < prNeighborsCount.second.size(); ++t) {
        if (tCurrent < prNeighborsCount.second[t]) //tCurrent is pointing to inserted meta location
            prNeighborsCount.second[t] -= std::min((size_t)prNeighborsCount.second[t], indexes.size() - 1);
      }
    }
    else {
      ++tGroupStart;//advance group start
    }
  }
}

/** Calculates the adjusted number of neighbors for current state of gvCentroidToLocationDistances object,
    ensuring that either all locations that reference the same coordinate are included or excluded.
    This method relies on gvCentroidToLocationDistances being sorted such that locations referencing the
    same coordinates are adjacent to each other. */
int CentroidNeighborCalculator::getAdjustedNeighborCountsForMultipleCoordinates(int iNeigborsCount) {
  if (iNeigborsCount != 0 && (size_t)iNeigborsCount != gvCentroidToLocationDistances.size()) {
    //get coordinates of current farthest neighbor
    LocationDistance& currMax = gvCentroidToLocationDistances[iNeigborsCount-1];
    const TractHandler::Coordinates * currMaxCoords = gLocationInfo.getLocations()[currMax.GetTractNumber()]->getCoordinates()[currMax.GetRelativeCoordinateIndex()];
    //get coordinates of neighbor after current farthest
    LocationDistance& beyondMax = gvCentroidToLocationDistances[iNeigborsCount];
    const TractHandler::Coordinates * beyondMaxCoords = gLocationInfo.getLocations()[beyondMax.GetTractNumber()]->getCoordinates()[beyondMax.GetRelativeCoordinateIndex()];
    while (currMaxCoords == beyondMaxCoords) {
         if (--iNeigborsCount < 1) break;
         LocationDistance& currMax = gvCentroidToLocationDistances[iNeigborsCount-1];
         currMaxCoords = gLocationInfo.getLocations()[currMax.GetTractNumber()]->getCoordinates()[currMax.GetRelativeCoordinateIndex()];
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
void CentroidNeighborCalculator::getTractCoordinates(const CSaTScanData& DataHub, const CCluster& Cluster, tract_t tTract, std::vector<double>& Coordinates) {
  if (DataHub.GetParameters().GetMultipleCoordinatesType() == ONEPERLOCATION ||
      (tTract < DataHub.GetNumTracts() &&  DataHub.GetTInfo()->getLocations()[tTract]->getCoordinates().size() == 1)) {
    DataHub.GetTInfo()->getLocations()[tTract]->getCoordinates()[0]->retrieve(Coordinates);
  }
  else {//we'll have to determine which coordinate matches specified MultipleCoordinatesType
    double                              dCurrent, dDistance,
                                        dAngle=DataHub.GetEllipseAngle(Cluster.GetEllipseOffset()),
                                        dShape=DataHub.GetEllipseShape(Cluster.GetEllipseOffset());
    std::vector<double>                 ClusterCenter, TractCoords;
    const TractHandler::Coordinates   * pCoordinates, * pTarget=0;

    DataHub.GetGInfo()->retrieveCoordinates(Cluster.GetCentroidIndex(), ClusterCenter);
    if (Cluster.GetEllipseOffset() > 0)
      Transform(ClusterCenter[0], ClusterCenter[1], dAngle, dShape, &ClusterCenter[0], &ClusterCenter[1]);
    //if tract index refers to a meta neighbor index, retrieve any of the indexes it maps to, then proceed
    if (tTract >= DataHub.GetNumTracts())
      tTract = DataHub.GetTInfo()->getMetaNeighborManager().getFirst(tTract - DataHub.GetNumTracts());
    for (unsigned int i=0; i < DataHub.GetTInfo()->getLocations()[tTract]->getCoordinates().size(); ++i) {
       pCoordinates = DataHub.GetTInfo()->getLocations()[tTract]->getCoordinates()[i];
       pCoordinates->retrieve(TractCoords);
       if (Cluster.GetEllipseOffset() > 0)
         Transform(TractCoords[0], TractCoords[1], dAngle, dShape, &TractCoords[0], &TractCoords[1]);
       dDistance = std::sqrt(TractHandler::getDistanceSquared(ClusterCenter, TractCoords));
       if (DataHub.GetParameters().GetMultipleCoordinatesType() == ATLEASTONELOCATION) {
         //searching for the closest coordinate
         if (!pTarget || dCurrent > dDistance) {pTarget = pCoordinates; dCurrent = dDistance;}
       }
       else if (DataHub.GetParameters().GetMultipleCoordinatesType() == ALLLOCATIONS) {
         //searching for the farthest coordinate
         if (!pTarget || dCurrent < dDistance) {pTarget = pCoordinates; dCurrent = dDistance;}
       }
       else
         throw prg_error("Unknown multiple coordinates type '%d'.", "getTractCoordinates()", DataHub.GetParameters().GetMultipleCoordinatesType());
    }
    pTarget->retrieve(Coordinates);
  }
}

/** Debug function. Prints elements of gvCentroidToLocationDistances object to FILE stream in formatted ASCII text. */
void CentroidNeighborCalculator::printCentroidToLocationDistances(size_t tMaxToPrint, FILE * stream) {
  FILE * fp=0;
  if (!stream) {
    if ((fp = fopen("CentroidToLocationDistances.print", "a+")) == NULL) return;
    stream = fp;
  }
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(), itr_end;
  itr_end = (tMaxToPrint >= gvCentroidToLocationDistances.size() ? gvCentroidToLocationDistances.end() : gvCentroidToLocationDistances.begin() + tMaxToPrint);
  fprintf(stream, "\nCentroidToLocationDistances (size=%d):\n", std::distance(itr, itr_end));
  for (; itr != itr_end; ++itr) {
    if (itr->GetTractNumber() >= gNumTracts) {
      std::vector<tract_t> indexes;
      gLocationInfo.getMetaNeighborManager().getIndexes(itr->GetTractNumber() - gNumTracts, indexes);
      fprintf(stream, "distance=%.6lf\t\tindexes=", itr->GetDistance());
      for (size_t i=0; i < indexes.size(); ++i)
        fprintf(stream, "%s%d", (i == 0 ? "" : ","), indexes.at(i));
      fprintf(stream, "\n");
    }
    else {
      const TractHandler::Location* pcurr = gLocationInfo.getLocations()[itr->GetTractNumber()];
      const TractHandler::Coordinates * pCoords = pcurr->getCoordinates()[itr->GetRelativeCoordinateIndex()];
      for (size_t t=0; t < pCoords->getSize(); ++t)
        fprintf(stream, "%s%g", (t == 0 ? "coordinates=": ","), pCoords->getCoordinates()[t]);
      fprintf(stream, "\t\tdistance=%.6lf\t\tname='%s'\t\tindex=%d\n", itr->GetDistance(), pcurr->getIndentifier(), itr->GetTractNumber());
    }
  }
  if (fp) fclose(fp);
}

/** Resizes passed vector to #of tracts plus # of meta locations. Assigns meta location data. */
void CentroidNeighborCalculator::setMetaLocations(std::vector<measure_t>& popMeasure) {
  popMeasure.resize(gNumTracts + gLocationInfo.getMetaLocations().getNumReferencedLocations(), 0);

  std::vector<tract_t> atomicIndexes;
  for (size_t t=0; t < gLocationInfo.getMetaLocations().getNumReferencedLocations(); ++t) {
     gLocationInfo.getMetaLocations().getAtomicIndexes(t, atomicIndexes);
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
        gvCalculatedPopulations.resize(dataHub.GetNumTracts(), 0);
        for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
          pCases = DataSetHandler.GetDataSet(t).getCaseData().GetArray()[0];
          for (int j=0; j < dataHub.GetNumTracts(); ++j)
             gvCalculatedPopulations[j] += pCases[j];
        }
        setMetaLocations(gvCalculatedPopulations);
        gpPopulation = &gvCalculatedPopulations[0]; break;
      case CATEGORICAL :
      case ORDINAL :
        //For the Ordinal model, populations for each location are calculated by adding up the
        //total individuals represented in the catgory case arrays.
        gvCalculatedPopulations.resize(dataHub.GetNumTracts(), 0);
        for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
           for (unsigned int k=0; k < DataSetHandler.GetDataSet(t).getCaseData_Cat().size(); ++k) {
             pCases = DataSetHandler.GetDataSet(t).getCaseData_Cat()[k]->GetArray()[0];
             for (int j=0; j < dataHub.GetNumTracts(); ++j)
                gvCalculatedPopulations[j] += pCases[j];
           }
        }
        setMetaLocations(gvCalculatedPopulations);
        gpPopulation = &gvCalculatedPopulations[0];  break;
      case EXPONENTIAL:
        // consider population as cases and non-censored cases
        gvCalculatedPopulations.resize(dataHub.GetNumTracts(), 0);
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
        gvCalculatedPopulations.resize(dataHub.GetNumTracts(), 0);
        //Population is calculated from all data sets
        for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
           measure_t * pMeasure = DataSetHandler.GetDataSet(t).getMeasureData().GetArray()[0];
           for (int j=0; j < dataHub.GetNumTracts(); ++j)
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

