//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MakeNeighbors.h"
#include "SaTScanData.h"
#include "ExponentialRandomizer.h"
#include "SSException.h" 

/** constructor */
CentroidNeighborCalculator::CentroidNeighborCalculator(const CSaTScanData& DataHub, BasePrint& PrintDirection)
                           :gDataHub(DataHub), gPrintDirection(PrintDirection), gtCurrentEllipseCoordinates(0),
                            gCentroidInfo(*DataHub.GetGInfo()), gLocationInfo(*DataHub.GetTInfo()),
                            gPrimaryNeighbors(0,0), gSecondaryNeighbors(0,0), gTertiaryNeighbors(0,0),
                            gPrimaryReportedNeighbors(0,0), gSecondaryReportedNeighbors(0,0), gTertiaryReportedNeighbors(0,0) {

  //allocate vector of LocationDistance objects
  for (tract_t t=0; t < gDataHub.GetNumTracts(); ++t)
    gvCentroidToLocationDistances.push_back(LocationDistance(t));
  SetupPopulationArrays();
  //calculate reported and actual maximum spatial clusters sizes
  CalculateMaximumSpatialClusterSize();
  CalculateMaximumReportedSpatialClusterSize();
}

/** destructor */
CentroidNeighborCalculator::~CentroidNeighborCalculator() {}

/** Transforms the x and y coordinates for each location so that circles
    in the transformed space represent ellipsoids in the original space.
    Stores transformed coordinates in internal array. */
void CentroidNeighborCalculator::CalculateEllipticCoordinates(tract_t tEllipseOffset) {
  double                                                dAngle, dShape;
  std::vector<double>                                   vCoordinates;
  std::vector<std::pair<double, double> >::iterator     itr;

  if (tEllipseOffset == 0 || gtCurrentEllipseCoordinates == tEllipseOffset)
    return;

  gvLocationEllipticCoordinates.resize(gDataHub.GetNumTracts());
  itr=gvLocationEllipticCoordinates.begin();
  dAngle = gDataHub.GetEllipseAngle(tEllipseOffset);
  dShape = gDataHub.GetEllipseShape(tEllipseOffset);
  for (tract_t t=0; t < gDataHub.GetNumTracts(); ++t, ++itr) {
     gLocationInfo.tiRetrieveCoords(t, vCoordinates);
     Transform(vCoordinates[0], vCoordinates[1], dAngle, dShape, &(itr->first), &(itr->second));
  }
  gtCurrentEllipseCoordinates = tEllipseOffset;
}

/** Determines the maximum clusters size for reported clusters and assigns appropriate function pointers. */
void CentroidNeighborCalculator::CalculateMaximumReportedSpatialClusterSize() {
  measure_t               tPopulation, tTotalPopulation=0;
  const CParameters     & Parameters = gDataHub.GetParameters();
  const DataSetHandler  & DataSetHandler = gDataHub.GetDataSetHandler();
  SecondaryCalcPair_t   * pNextToSet = &gPrimaryReportedNeighbors;

  try {
    if (!Parameters.GetRestrictingMaximumReportedGeoClusterSize()) return;

    if (!(Parameters.GetAnalysisType() == PROSPECTIVESPACETIME && Parameters.GetAdjustForEarlierAnalyses())) {
      //NOTE: When input data is defined in multiple data sets, the maximum spatial cluster size is calculated
      //      as a percentage of the total population in all data sets.
      for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
         if (Parameters.GetProbabilityModelType() == ORDINAL || Parameters.GetProbabilityModelType() == NORMAL)
           tPopulation = DataSetHandler.GetDataSet(t).GetTotalCases();
         else if (Parameters.GetProbabilityModelType() == EXPONENTIAL)
           tPopulation = DataSetHandler.GetDataSet(t).GetTotalPopulation();
         else
           tPopulation = DataSetHandler.GetDataSet(t).GetTotalMeasure();
         if (tPopulation > std::numeric_limits<measure_t>::max() - tTotalPopulation)
           GenerateResolvableException("Error: The total population, summed over of all data sets, exceeds the maximum value allowed of %lf.\n",
                                       "CalculateMaximumReportedSpatialClusterSize()", std::numeric_limits<measure_t>::max());
         tTotalPopulation += tPopulation;
      }
      pNextToSet->first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulationAtRisk;
      pNextToSet->second = (Parameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, true) / 100.0) * tTotalPopulation;
      pNextToSet = &gSecondaryReportedNeighbors;
    }
    if (Parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true)) {
      //set maximum circle size based upon percentage of population defined in maximum circle size file
      pNextToSet->first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByMaxCirclePopulation;
      pNextToSet->second = (Parameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true) / 100.0) * gDataHub.GetMaxCirclePopulationSize();
      pNextToSet = pNextToSet == &gPrimaryReportedNeighbors ? &gSecondaryReportedNeighbors : &gTertiaryReportedNeighbors;
    }
    //set maximum circle size based upon a distance
    if (Parameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true)) {
      pNextToSet->first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance;
      pNextToSet->second = Parameters.GetMaxSpatialSizeForType(MAXDISTANCE, true);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMaximumReportedSpatialClusterSize()","CentroidNeighborCalculator");
    throw;
  }
}

/** Determines the maximum clusters size for clusters and assigns appropriate function pointers. */
void CentroidNeighborCalculator::CalculateMaximumSpatialClusterSize() {
  measure_t               tPopulation, tTotalPopulation=0;
  const CParameters     & Parameters = gDataHub.GetParameters();
  const DataSetHandler  & DataSetHandler = gDataHub.GetDataSetHandler();
  SecondaryCalcPair_t   * pNextToSet = &gSecondaryNeighbors;

  try {
    if (!(Parameters.GetAnalysisType() == PROSPECTIVESPACETIME && Parameters.GetAdjustForEarlierAnalyses())) {
      //set maximum circle size based upon percentage of defined population
      //NOTE: When input data is defined in multiple data sets, the maximum spatial cluster size is calculated
      //      as a percentage of the total population in all data sets.
      for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
         if (Parameters.GetProbabilityModelType() == ORDINAL || Parameters.GetProbabilityModelType() == NORMAL)
           tPopulation = DataSetHandler.GetDataSet(t).GetTotalCases();
         else if (Parameters.GetProbabilityModelType() == EXPONENTIAL)
           tPopulation = DataSetHandler.GetDataSet(t).GetTotalPopulation();
         else
           tPopulation = DataSetHandler.GetDataSet(t).GetTotalMeasure();
         if (tPopulation > std::numeric_limits<measure_t>::max() - tTotalPopulation)
           GenerateResolvableException("Error: The total population, summed over of all data sets, exceeds the maximum value allowed of %lf.\n",
                                       "CalculateMaximumSpatialClusterSize()", std::numeric_limits<measure_t>::max());
         tTotalPopulation += tPopulation;
      }
      gPrimaryNeighbors.first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulationAtRisk;
      gPrimaryNeighbors.second = (Parameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false) / 100.0) * tTotalPopulation;
    }
    //set maximum circle size based upon percentage of population defined in maximum circle size file
    if (Parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false)) {
      if (!gPrimaryNeighbors.first) {
        gPrimaryNeighbors.first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByMaxCirclePopulation;
        gPrimaryNeighbors.second = (Parameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) / 100.0) * gDataHub.GetMaxCirclePopulationSize();
        pNextToSet = &gSecondaryNeighbors;
      }
      else {
        pNextToSet->first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByMaxCirclePopulation;
        pNextToSet->second = (Parameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) / 100.0) * gDataHub.GetMaxCirclePopulationSize();
        pNextToSet = &gTertiaryNeighbors;
      }
    }  
    //set maximum circle size based upon a distance
    if (Parameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false)) {
      if (!gPrimaryNeighbors.first) {
        gPrimaryNeighbors.first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance;
        gPrimaryNeighbors.second = Parameters.GetMaxSpatialSizeForType(MAXDISTANCE, false);
      }
      else {
        pNextToSet->first = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance;
        pNextToSet->second = Parameters.GetMaxSpatialSizeForType(MAXDISTANCE, false);
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMaximumSpatialClusterSize()","CentroidNeighborCalculator");
    throw;
  }
}

/** Calculates neighboring locations about each centroid; storing results in sorted
    array contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighbors() {
  try {
    const_cast<CSaTScanData&>(gDataHub).AllocateSortedArray();
    CalculateNeighborsByCircles();
    CalculateNeighborsByEllipses();
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateNeighbors()", "CentroidNeighborCalculator");
    throw;
  }
}

/** Calculates closest neighbor's distances to all locations from ellipse/centroid, sorted from closest
    to most distant; storing in gvCentroidToLocationDistances class member. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex) {
  CenterLocationDistancesAbout(tEllipseOffsetIndex, tCentroidIndex);
  //sort such locations are clostest to farthest
  std::sort(gvCentroidToLocationDistances.begin(), gvCentroidToLocationDistances.end(), CompareLocationDistance(gLocationInfo));
}

/** Calculates neighboring locations about centroid for given ellipse offset and centroid;
    storing results in CentroidNeighbors object. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid) {
  std::pair<int, int> prNeighborsCount;

  CalculateNeighborsAboutCentroid(tEllipseOffsetIndex, tCentroidIndex);
  CalculateNeighborsForCurrentState(prNeighborsCount);
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
void CentroidNeighborCalculator::CalculateNeighborsByCircles() {
  boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
  int                   iNumReportedNeighbors, iNumNeighbors;
  std::pair<int, int>   prNeighborsCount;

  gPrintDirection.Printf("Constructing the circles\n", BasePrint::P_STDOUT);
  //Calculate neighboring locations about each centroid for circular regions
  for (tract_t t=0; t < gDataHub.m_nGridTracts; ++t) {
     CalculateNeighborsAboutCentroid(0, t);
     CalculateNeighborsForCurrentState(prNeighborsCount);
     const_cast<CSaTScanData&>(gDataHub).AllocateSortedArrayNeighbors(gvCentroidToLocationDistances, 0, t, prNeighborsCount.second, prNeighborsCount.first);
     if (t == 9) ReportTimeEstimate(StartTime, gDataHub.m_nGridTracts, t, &gPrintDirection);
  }
}

/** Calculates neighboring locations about each centroid by distance; storing
    results in multiple dimension arrays contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighborsByEllipses() {
  boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
  int                   iNumReportedNeighbors, iNumNeighbors;
  std::pair<int, int>   prNeighborsCount;

  //only perform calculation if ellipses requested
  if (!gDataHub.GetParameters().GetSpatialWindowType() == ELLIPTIC)
    return;

  gPrintDirection.Printf("Constructing the ellipsoids\n", BasePrint::P_STDOUT);
  //Calculate neighboring locations about each centroid for elliptical regions
  for (int i=1; i <= gDataHub.GetParameters().GetNumTotalEllipses(); ++i) {
     for (tract_t t=0; t < gDataHub.m_nGridTracts; ++t) {
        CalculateNeighborsAboutCentroid(i, t);
        CalculateNeighborsForCurrentState(prNeighborsCount);
        const_cast<CSaTScanData&>(gDataHub).AllocateSortedArrayNeighbors(gvCentroidToLocationDistances, i, t, prNeighborsCount.second, prNeighborsCount.first);
        if (t == 9 && i == 1) ReportTimeEstimate(StartTime, gDataHub.m_nGridTracts * gDataHub.GetParameters().GetNumTotalEllipses(), t, &gPrintDirection);
     }
  }
}

/** Given current state of class data members, calculates the number of neighbors in real data and simulation data. */
void CentroidNeighborCalculator::CalculateNeighborsForCurrentState(std::pair<int, int>& prNeigborsCount) const {
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
}


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
  std::vector<double>    vCentroidCoordinates, vLocationCoordinates;

  if (tEllipseOffsetIndex == 0) {
    gCentroidInfo.giRetrieveCoords(tCentroidIndex, vCentroidCoordinates);
    //calculate distances from centroid to each location
    for (tract_t k=0; k < gDataHub.GetNumTracts(); ++k) {
       gvCentroidToLocationDistances[k].SetTractNumber(k);
       gLocationInfo.tiRetrieveCoords(k, vLocationCoordinates);
       gvCentroidToLocationDistances[k].SetDistance(std::sqrt(gLocationInfo.tiGetDistanceSq(vCentroidCoordinates, vLocationCoordinates)));
    }
  }
  else {
    gCentroidInfo.giRetrieveCoords(tCentroidIndex, vCentroidCoordinates);
    //tranform centroid coordinates into elliptical space
    Transform(vCentroidCoordinates[0], vCentroidCoordinates[1], gDataHub.GetEllipseAngle(tEllipseOffsetIndex),
              gDataHub.GetEllipseShape(tEllipseOffsetIndex), &vCentroidCoordinates[0], &vCentroidCoordinates[1]);
    vLocationCoordinates.resize(2);
    CalculateEllipticCoordinates(tEllipseOffsetIndex);
    //calculate distances from centroid to each location
    for (tract_t k=0; k < gDataHub.GetNumTracts(); ++k) {
       vLocationCoordinates[0] = gvLocationEllipticCoordinates[k].first;
       vLocationCoordinates[1] = gvLocationEllipticCoordinates[k].second;
       gvCentroidToLocationDistances[k].SetTractNumber(k);
       gvCentroidToLocationDistances[k].SetDistance(std::sqrt(gLocationInfo.tiGetDistanceSq(vCentroidCoordinates, vLocationCoordinates)));
    }
  }
}

/** Based upon parameter settings, stores references to population arrays maintained by
    CSaTScanData object or calculates population, storing in class member structure. */
void CentroidNeighborCalculator::SetupPopulationArrays() {
  count_t                     * pCases;
  const ExponentialRandomizer * pRandomizer;
  measure_t                     tPopulation, tTotalPopulation=0;
  const CParameters           & Parameters = gDataHub.GetParameters();
  const DataSetHandler        & DataSetHandler = gDataHub.GetDataSetHandler();

  //store reference to population defined in max circle file if either restricting in either real or simulation process
  if (Parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) || Parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true))
    gpMaxCircleFilePopulation = &gDataHub.GetMaxCirclePopulationArray()[0];
  //prospective space-time analyses, using prospective start date, do not use the population at risk to restrict maximum spatial size
  if (!(Parameters.GetAnalysisType() == PROSPECTIVESPACETIME && Parameters.GetAdjustForEarlierAnalyses())) {
    switch (Parameters.GetProbabilityModelType()) {
      case NORMAL  :
        gvCalculatedPopulations.resize(gDataHub.GetNumTracts(), 0);
        for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
           pCases = DataSetHandler.GetDataSet(t).GetCaseArray()[0];
           for (int j=0; j < gDataHub.GetNumTracts(); ++j)
             gvCalculatedPopulations[j] += pCases[j];
        }
        gpPopulation = &gvCalculatedPopulations[0]; break;
      case ORDINAL :
        //For the Ordinal model, populations for each location are calculated by adding up the
        //total individuals represented in the catgory case arrays.
        gvCalculatedPopulations.resize(gDataHub.GetNumTracts(), 0);
        //Population is calculated from first data set - even when multiple data sets are defined.
        for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
           for (unsigned int k=0; k < DataSetHandler.GetDataSet(t).GetCasesByCategory().size(); ++k) {
             pCases = DataSetHandler.GetDataSet(t).GetCasesByCategory()[k]->GetArray()[0];
             for (int j=0; j < gDataHub.GetNumTracts(); ++j)
                gvCalculatedPopulations[j] += pCases[j];
           }
        }
        gpPopulation = &gvCalculatedPopulations[0]; break;
      case EXPONENTIAL:
        // consider population as cases and non-censored cases
        gvCalculatedPopulations.resize(gDataHub.GetNumTracts(), 0);
        //Population is calculated from first data set - even when multiple data sets are defined.
        for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
          pRandomizer = dynamic_cast<const ExponentialRandomizer*>(DataSetHandler.GetRandomizer(t));
          if (!pRandomizer) ZdGenerateException("Randomizer failed cast to ExponentialRandomizer.", "CalculateMaximumReportedSpatialClusterSize()");
            pRandomizer->CalculateMaxCirclePopulationArray(gvCalculatedPopulations, false);
        }
        gpPopulation = &gvCalculatedPopulations[0]; break;
      default :
        gvCalculatedPopulations.resize(gDataHub.GetNumTracts(), 0);
        //Population is calculated from all data sets
        for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
           measure_t * pMeasure = DataSetHandler.GetDataSet(t).GetMeasureArray()[0];
           for (int j=0; j < gDataHub.GetNumTracts(); ++j)
             gvCalculatedPopulations[j] += pMeasure[j];
        }
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

