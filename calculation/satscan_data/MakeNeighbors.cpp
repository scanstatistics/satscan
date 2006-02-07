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
                            gtMaximumSize(0), gtMaximumReportedSize(0),
                            gpLocationsPopulation(0), gpLocationsPopulationReported(0),
                            gpNeighborCalculationMethod(0), gpReportedNeighborCalculationMethod(0) {

  //allocate vector of LocationDistance objects
  for (tract_t t=0; t < gDataHub.GetNumTracts(); ++t)
    gvCentroidToLocationDistances.push_back(LocationDistance(t));
  //calculate reported and actual maximum spatial clusters sizes
  CalculateMaximumSpatialClusterSize();
  CalculateMaximumReportedSpatialClusterSize();
}

/** destructor */
CentroidNeighborCalculator::~CentroidNeighborCalculator() {}

/** Calculates closest neighbor's distances to all locations from ellipse/centroid, sorted from closest
    to most distant; storing in gvCentroidToLocationDistances class member. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex) {
  CenterLocationDistancesAbout(tEllipseOffsetIndex, tCentroidIndex);
  //sort such locations are clostest to farthest
  std::sort(gvCentroidToLocationDistances.begin(), gvCentroidToLocationDistances.end(), CompareLocationDistance(gLocationInfo));
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

/** Determines the maximum clusters size for reported clusters, assigns population array
    pointer (as needed) and assigns CALCULATE_REPORTED_NEIGHBORS_METHOD function pointer. */
void CentroidNeighborCalculator::CalculateMaximumReportedSpatialClusterSize() {
  measure_t               tPopulation, tTotalPopulation=0;
  const CParameters     & Parameters = gDataHub.GetParameters();
  const DataSetHandler  & DataSetHandler = gDataHub.GetDataSetHandler();

  try {
    if (Parameters.GetRestrictingMaximumReportedGeoClusterSize()) {
      switch (Parameters.GetMaxReportedGeographicClusterSizeType()) {
        case PERCENTOFMAXCIRCLEFILE :
          gtMaximumReportedSize = (Parameters.GetMaximumReportedGeoClusterSize() / 100.0) * gDataHub.GetMaxCirclePopulationSize();
          gpReportedNeighborCalculationMethod = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulation;
          gpLocationsPopulationReported = &gDataHub.GetMaxCirclePopulationArray()[0];
          break;
        case PERCENTOFPOPULATION :
          //NOTE: When input data is defined in multiple data sets, the maximum spatial cluster size is calculated
          //      as a percentage of the total population in all data sets. 
          for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
             if (Parameters.GetProbabilityModelType() == ORDINAL)
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
          gtMaximumReportedSize = (Parameters.GetMaximumReportedGeoClusterSize() / 100.0) * tTotalPopulation;
          gpReportedNeighborCalculationMethod = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulation;

          if (gDataHub.GetParameters().GetProbabilityModelType() == ORDINAL) {
            if (gvCalculatedPopulations.empty()) {
              //For the Ordinal model, populations for each location are calculated by adding up the
              //total individuals represented in the catgory case arrays.
              gvCalculatedPopulations.resize(gDataHub.GetNumTracts(), 0);
              //Population is calculated from first data set - even when multiple data sets are defined.
              for (unsigned int k=0; k < gDataHub.GetDataSetHandler().GetDataSet().GetCasesByCategory().size(); ++k) {
                 count_t** ppCases = gDataHub.GetDataSetHandler().GetDataSet().GetCasesByCategory()[k]->GetArray();
                 for (int j=0; j < gDataHub.GetNumTracts(); ++j)
                   gvCalculatedPopulations[j] += ppCases[0][j];
              }
            }
            gpLocationsPopulationReported = &gvCalculatedPopulations[0];
          }
          else if (gDataHub.GetParameters().GetProbabilityModelType() == EXPONENTIAL) {
            if (gvCalculatedPopulations.empty()) {
               // consider population as cases and non-censored cases
               gvCalculatedPopulations.assign(gDataHub.GetNumTracts(), 0);
               //Population is calculated from first data set - even when multiple data sets are defined.
               const ExponentialRandomizer * pRandomizer = dynamic_cast<const ExponentialRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(0));
               if (!pRandomizer) ZdGenerateException("Randomizer failed cast to ExponentialRandomizer.", "CalculateMaximumReportedSpatialClusterSize()");
               pRandomizer->CalculateMaxCirclePopulationArray(gvCalculatedPopulations);
            }
            gpLocationsPopulationReported = &gvCalculatedPopulations[0];
          }
          else
            //Population is calculated from first data set - even when multiple data sets are defined.
            gpLocationsPopulationReported = gDataHub.GetDataSetHandler().GetDataSet().GetMeasureArray()[0];
          break;
        case MAXDISTANCE :
          gtMaximumReportedSize = Parameters.GetMaximumReportedGeoClusterSize();
          gpReportedNeighborCalculationMethod = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance;
          break;
        default : ZdException::Generate("Unknown maximum spatial cluster type: '%i'.", "CalculateMaximumReportedSpatialClusterSize()",
                                        Parameters.GetMaxGeographicClusterSizeType());
      };
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMaximumReportedSpatialClusterSize()","CentroidNeighborCalculator");
    throw;
  }
}

/** Determines the maximum clusters size, assigns population array pointer (as needed)
    and assigns CALCULATE_NEIGHBORS_METHOD function pointer. */
void CentroidNeighborCalculator::CalculateMaximumSpatialClusterSize() {
  measure_t               tPopulation, tTotalPopulation=0;
  const CParameters     & Parameters = gDataHub.GetParameters();
  const DataSetHandler  & DataSetHandler = gDataHub.GetDataSetHandler();

  try {
    switch (Parameters.GetMaxGeographicClusterSizeType()) {
      case PERCENTOFMAXCIRCLEFILE :
           gtMaximumSize = (Parameters.GetMaximumGeographicClusterSize() / 100.0) * gDataHub.GetMaxCirclePopulationSize();
           gpNeighborCalculationMethod = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulation;
           gpLocationsPopulation = const_cast<measure_t*>((&gDataHub.GetMaxCirclePopulationArray()[0]));
           break;
      case PERCENTOFPOPULATION :
           //NOTE: When input data is defined in multiple data sets, the maximum spatial cluster size is calculated
           //      as a percentage of the total population in all data sets. 
           for (size_t t=0; t < DataSetHandler.GetNumDataSets(); ++t) {
              if (Parameters.GetProbabilityModelType() == ORDINAL)
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
           gtMaximumSize = (Parameters.GetMaximumGeographicClusterSize() / 100.0) * tTotalPopulation;
           gpNeighborCalculationMethod = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulation;

           if (gDataHub.GetParameters().GetProbabilityModelType() == ORDINAL) {
             //For the Ordinal model, populations for each location are calculated by adding up the
             //total individuals represented in the catgory case arrays.
             gvCalculatedPopulations.resize(gDataHub.GetNumTracts(), 0);
             //Population is calculated from first data set - even when multiple data sets are defined.
             for (unsigned int k=0; k < gDataHub.GetDataSetHandler().GetDataSet().GetCasesByCategory().size(); ++k) {
                count_t** ppCases = gDataHub.GetDataSetHandler().GetDataSet().GetCasesByCategory()[k]->GetArray();
                for (int j=0; j < gDataHub.GetNumTracts(); ++j)
                  gvCalculatedPopulations[j] += ppCases[0][j];
             }
             gpLocationsPopulation = &gvCalculatedPopulations[0];
           }
           else if (gDataHub.GetParameters().GetProbabilityModelType() == EXPONENTIAL) {
             // consider population as cases and non-censored cases
             gvCalculatedPopulations.assign(gDataHub.GetNumTracts(), 0);
             //Population is calculated from first data set - even when multiple data sets are defined.
             const ExponentialRandomizer * pRandomizer = dynamic_cast<const ExponentialRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(0));
             if (!pRandomizer) ZdGenerateException("Randomizer failed cast to ExponentialRandomizer.", "constructor()");
             pRandomizer->CalculateMaxCirclePopulationArray(gvCalculatedPopulations);
             gpLocationsPopulation = &gvCalculatedPopulations[0];
           }
           else
             //Population is calculated from first data set - even when multiple data sets are defined.
             gpLocationsPopulation = gDataHub.GetDataSetHandler().GetDataSet().GetMeasureArray()[0];
           break;
      case MAXDISTANCE :
           gtMaximumSize = Parameters.GetMaximumGeographicClusterSize();
           gpNeighborCalculationMethod = &CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByDistance;
           break;
      default : ZdException::Generate("Unknown maximum spatial cluster type: '%i'.", "CalculateMaximumSpatialClusterSize()",
                                      Parameters.GetMaxGeographicClusterSizeType());
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMaximumSpatialClusterSize()","CentroidNeighborCalculator");
    throw;
  }
}

/** Calculates neighboring locations about centroid for given ellipse offset and centroid;
    storing results in CentroidNeighbors object. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid) {
  int iNumNeighbors, iNumReportedNeighbors;

  CalculateNeighborsAboutCentroid(tEllipseOffsetIndex, tCentroidIndex);
  iNumNeighbors = (this->*gpNeighborCalculationMethod)(gtMaximumSize);
  iNumReportedNeighbors = (!gpReportedNeighborCalculationMethod ? iNumNeighbors : (this->*gpReportedNeighborCalculationMethod)(gtMaximumReportedSize, iNumNeighbors));
  Centroid.Set(tEllipseOffsetIndex, tCentroidIndex, iNumNeighbors, iNumReportedNeighbors, gvCentroidToLocationDistances);
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

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size. */
tract_t CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulation(measure_t tMaximumSize) const {
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
  tract_t                                       tCount=0;
  measure_t                                     tCumMeasure=0;

  for (; itr != itr_end && (tCumMeasure + gpLocationsPopulation[itr->GetTractNumber()]) <= tMaximumSize; ++itr) {
     tCumMeasure += gpLocationsPopulation[itr->GetTractNumber()];
     if (tCumMeasure <= tMaximumSize) ++tCount;
  }
  return tCount;
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
tract_t CentroidNeighborCalculator::CalculateNumberOfNeighboringLocationsByPopulation(measure_t tMaximumSize, count_t tMaximumNeighbors) const {
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
  tract_t                                       tCount=0;
  measure_t                                     tCumMeasure=0;

  for (; itr != itr_end &&
         tCount + 1 <= tMaximumNeighbors &&
         tCumMeasure + gpLocationsPopulationReported[itr->GetTractNumber()] <= tMaximumSize; ++itr) {

     tCumMeasure += gpLocationsPopulationReported[itr->GetTractNumber()];
     ++tCount;
  }
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

/** Calculates neighboring locations about each centroid through expanding circle;
    storing results in sorted array contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighborsByCircles() {
  boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
  int                   iNumReportedNeighbors, iNumNeighbors;

  gPrintDirection.Printf("Constructing the circles\n", BasePrint::P_STDOUT);
  //Calculate neighboring locations about each centroid for circular regions
  for (tract_t t=0; t < gDataHub.m_nGridTracts; ++t) {
     CalculateNeighborsAboutCentroid(0, t);
     iNumNeighbors = (this->*gpNeighborCalculationMethod)(gtMaximumSize);
     iNumReportedNeighbors = (!gpReportedNeighborCalculationMethod ? iNumNeighbors : (this->*gpReportedNeighborCalculationMethod)(gtMaximumReportedSize, iNumNeighbors));
     const_cast<CSaTScanData&>(gDataHub).AllocateSortedArrayNeighbors(gvCentroidToLocationDistances, 0, t, iNumReportedNeighbors, iNumNeighbors);
     if (t == 9) ReportTimeEstimate(StartTime, gDataHub.m_nGridTracts, t, &gPrintDirection);
  }
}

/** Calculates neighboring locations about each centroid by distance; storing
    results in multiple dimension arrays contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighborsByEllipses() {
  boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
  int                           iNumReportedNeighbors, iNumNeighbors;

  //only perform calculation if ellipses requested
  if (!gDataHub.GetParameters().GetSpatialWindowType() == ELLIPTIC)
    return;

  gPrintDirection.Printf("Constructing the ellipsoids\n", BasePrint::P_STDOUT);
  //Calculate neighboring locations about each centroid for elliptical regions
  for (int i=1; i <= gDataHub.GetParameters().GetNumTotalEllipses(); ++i) {
     for (tract_t t=0; t < gDataHub.m_nGridTracts; ++t) {
        CalculateNeighborsAboutCentroid(i, t);
        iNumNeighbors = (this->*gpNeighborCalculationMethod)(gtMaximumSize);
        iNumReportedNeighbors = (!gpReportedNeighborCalculationMethod ? iNumNeighbors : (this->*gpReportedNeighborCalculationMethod)(gtMaximumReportedSize, iNumNeighbors));
        const_cast<CSaTScanData&>(gDataHub).AllocateSortedArrayNeighbors(gvCentroidToLocationDistances, i, t, iNumReportedNeighbors, iNumNeighbors);
        if (t == 9 && i == 1) ReportTimeEstimate(StartTime, gDataHub.m_nGridTracts * gDataHub.GetParameters().GetNumTotalEllipses(), t, &gPrintDirection);
     }
  }
}

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

