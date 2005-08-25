//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MakeNeighbors.h"
#include "SaTScanData.h"
#include "ExponentialRandomizer.h"

/** Comparison function for LocationDistance objects. */
bool CompareLocationDistance::operator() (const LocationDistance& lhs, const LocationDistance& rhs) {
  //first check whether distances are equal - we may need to break a tie
  if (lhs.GetDistance() == rhs.GetDistance()) {
    // break ties in a controlled scheme:
    //   - compare coordinates starting at first dimension and
    //       continue until last dimension(if needed)
    //   - lesser coordinate breaks tie, not for any particular reason
    //     that was the decision made by Martin
    //   - if all coordinates are equal, then something is wrong as
    //     duplicate coordinates should have been handled by this point
    //     in program execution. Throw exception - else we've lost tie
    //     breaking control.
    for (gbContinue=true,gi=0; gi < gTractInformation.tiGetDimensions() && gbContinue; gi++) {
       gdCoordinateLHS = gTractInformation.tiGetTractCoordinate(lhs.GetTractNumber(), gi);
       gdCoordinateRHS = gTractInformation.tiGetTractCoordinate(rhs.GetTractNumber(), gi);
       gbContinue = (gdCoordinateLHS == gdCoordinateRHS);
    }
    if (gbContinue) // Done comparing coordinates, are they duplicates?
      ZdGenerateException("Identical coordinates found during sort comparison for tracts \"%s\" and \"%s\".",
                          "CompareTractDistance()",
                          gTractInformation.tiGetTid(lhs.GetTractNumber(), gsLHS),
                          gTractInformation.tiGetTid(rhs.GetTractNumber(), gsRHS));
    return (gdCoordinateLHS < gdCoordinateRHS);
  }
  //distances not equal, compare as normal
  else
    return (lhs.GetDistance() < rhs.GetDistance());
}

//******************************************************************************

/** constructor */
CentroidNeighbors::CentroidNeighbors()
                  :gtCentroid(0), gtEllipseOffset(0), giNeighbors(0), giMaxNeighbors(0),
                   giMaxReportedNeighbors(0), gpSortedNeighborsIntegerType(0),
                   gpSortedNeighborsUnsignedShortType(0), gppSortedNeighborsIntegerType(0),
                   gppSortedNeighborsUnsignedShortType(0), gpNeighborArray(0) {}

/** constructor */
CentroidNeighbors::CentroidNeighbors(tract_t tEllipseOffset, const CSaTScanData& DataHub)
                  : gtCentroid(0), gtEllipseOffset(tEllipseOffset), giNeighbors(0), giMaxNeighbors(0),
                    giMaxReportedNeighbors(0), gpSortedNeighborsIntegerType(0),
                    gpSortedNeighborsUnsignedShortType(0), gppSortedNeighborsIntegerType(0),
                    gppSortedNeighborsUnsignedShortType(0), gpNeighborArray(0) {
                    
  gpNeighborArray = DataHub.GetNeighborCountArray()[gtEllipseOffset];
  if (DataHub.GetSortedArrayAsTract_T(tEllipseOffset))
    gppSortedNeighborsIntegerType = DataHub.GetSortedArrayAsTract_T(gtEllipseOffset);
  else
    gppSortedNeighborsUnsignedShortType = DataHub.GetSortedArrayAsUShort_T(gtEllipseOffset);
}

/** destructor */
CentroidNeighbors::~CentroidNeighbors() {}

/** Sets class members to define locations about centroid index / ellipse index.
    The neighbor information referenced is that which is calculated by CentroidNeighborCalculator
    object; caller is responsible for ensuring that:
        1) tEllipseOffset, tCentroid, iNumNeighbors and iNumReportedNeighbors are valid indexes
        
    Allocates vector of either integers or unsigned shorts, based upon specified number of
    neighbors for centroid (iNumNeighbors). Sets maxium number of neighbors variable returned
    through GetNumNeighbors() method to that of 'iNumReportedNeighbors' variable. */
void CentroidNeighbors::Set(tract_t tEllipseOffset, tract_t tCentroid, int iNumNeighbors, int iNumReportedNeighbors, const std::vector<LocationDistance>& vOrderedLocations) {

  //conditionally allocate unsigned short vs tract_t
  if (vOrderedLocations.size() < (size_t)std::numeric_limits<unsigned short>::max()) {
    gvSortedNeighborsUnsignedShortType.resize(iNumNeighbors);
    gpSortedNeighborsUnsignedShortType = (iNumNeighbors ? &gvSortedNeighborsUnsignedShortType[0] : 0);
    for (tract_t j=iNumNeighbors-1; j >= 0; j--) /* copy tract numbers */
       gpSortedNeighborsUnsignedShortType[j] = vOrderedLocations[j].GetTractNumber();
  }
  else {
    gvSortedNeighborsIntegerType.resize(iNumNeighbors);
    gpSortedNeighborsIntegerType = (iNumNeighbors ? &gvSortedNeighborsIntegerType[0] : 0);
    for (tract_t j=iNumNeighbors-1; j >= 0; j--) /* copy tract numbers */
       gpSortedNeighborsIntegerType[j] = vOrderedLocations[j].GetTractNumber();
  }

  gtCentroid = tCentroid;
  gtEllipseOffset = tEllipseOffset;
  giMaxNeighbors = iNumNeighbors;
  giNeighbors = giMaxReportedNeighbors = iNumReportedNeighbors;
}

//******************************************************************************

/** constructor */
CentroidNeighborCalculator::CentroidNeighborCalculator(const CSaTScanData& DataHub, BasePrint& PrintDirection)
                           :gDataHub(DataHub), gPrintDirection(PrintDirection), gtCurrentEllipseCoordinates(0),
                            gCentroidInfo(*DataHub.GetGInfo()), gLocationInfo(*DataHub.GetTInfo()) {
  //allocate vector of LocationDistance objects
  for (tract_t t=0; t < gDataHub.GetNumTracts(); ++t)
     gvCentroidToLocationDistances.push_back(LocationDistance(t));
  //determine maximum size of circle/ellipse
  gtMaximumSize = gDataHub.GetMaxCircleSize();
  if (gDataHub.GetParameters().GetRestrictingMaximumReportedGeoClusterSize())
    gtMaximumReportedSize = gDataHub.GetMaxReportedCircleSize();
  else
    gtMaximumReportedSize = gtMaximumSize;
}

/** destructor */
CentroidNeighborCalculator::~CentroidNeighborCalculator() {}

/** Calculates closest neighbor's distances to all locations from ellipse/centroid, sorted from closest
    to most distant; storing in gvCentroidToLocationDistances class member. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex) {
  CenterLocationDistancesAbout(tEllipseOffsetIndex, tCentroidIndex);
  //sort such locations are clostest to farthest
  std::stable_sort(gvCentroidToLocationDistances.begin(), gvCentroidToLocationDistances.end(), CompareLocationDistance(gLocationInfo));
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

/** Calculates neighboring locations about centroid for given ellipse offset and centroid;
    storing results in CentroidNeighbors object. */
void CentroidNeighborCalculator::CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid) {
  int iNumNeighbors, iNumReportedNeighbors;

  CalculateNeighborsAboutCentroid(tEllipseOffsetIndex, tCentroidIndex);
  iNumNeighbors = CalculateNumberOfNeighboringLocations(gtMaximumSize);
  iNumReportedNeighbors = (gtMaximumSize == gtMaximumReportedSize ? iNumNeighbors : CalculateNumberOfNeighboringLocations(gtMaximumReportedSize));
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

/** Calculates neighboring locations about each centroid through expanding circle;
    storing results in sorted array contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighborsByCircles() {
  boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
  int                   iNumReportedNeighbors, iNumNeighbors;

  gPrintDirection.SatScanPrintf("Constructing the circles\n");
  //Calculate neighboring locations about each centroid for circular regions
  for (tract_t t=0; t < gDataHub.m_nGridTracts; ++t) {
     CalculateNeighborsAboutCentroid(0, t);
     iNumNeighbors = CalculateNumberOfNeighboringLocations(gtMaximumSize);
     iNumReportedNeighbors = (gtMaximumSize == gtMaximumReportedSize ? iNumNeighbors : CalculateNumberOfNeighboringLocations(gtMaximumReportedSize));
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
  if (!gDataHub.GetParameters().GetNumRequestedEllipses())
    return;

  gPrintDirection.SatScanPrintf("Constructing the ellipsoids\n");
  //Calculate neighboring locations about each centroid for elliptical regions
  for (int i=1; i <= gDataHub.GetParameters().GetNumTotalEllipses(); ++i) {
     for (tract_t t=0; t < gDataHub.m_nGridTracts; ++t) {
        CalculateNeighborsAboutCentroid(i, t);
        iNumNeighbors = CalculateNumberOfNeighboringLocations(gtMaximumSize);
        iNumReportedNeighbors = (gtMaximumSize == gtMaximumReportedSize ? iNumNeighbors : CalculateNumberOfNeighboringLocations(gtMaximumReportedSize));
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
       gvCentroidToLocationDistances[k].SetDistance(std::sqrt(gLocationInfo.tiGetDistanceSq(&vCentroidCoordinates[0], &vLocationCoordinates[0])));
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
       gvCentroidToLocationDistances[k].SetDistance(std::sqrt(gLocationInfo.tiGetDistanceSq(&vCentroidCoordinates[0], &vLocationCoordinates[0])));
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

//******************************************************************************

/** constructor */
CentroidNeighborCalculatorByDistance::CentroidNeighborCalculatorByDistance(const CSaTScanData& DataHub, BasePrint& PrintDirection)
                                     :CentroidNeighborCalculator(DataHub, PrintDirection){}

/** destructor */
CentroidNeighborCalculatorByDistance::~CentroidNeighborCalculatorByDistance() {}

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size. */
tract_t CentroidNeighborCalculatorByDistance::CalculateNumberOfNeighboringLocations(measure_t tMaximumSize) const {
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
  tract_t                                       tCount=0;

  for (; itr != itr_end && itr->GetDistance() <= tMaximumSize; ++itr) ++tCount;

  return tCount;
}

//******************************************************************************

/** constructor */
CentroidNeighborCalculatorByPopulation::CentroidNeighborCalculatorByPopulation(const CSaTScanData& DataHub, BasePrint& PrintDirection)
                                       :CentroidNeighborCalculator(DataHub, PrintDirection){
  count_t    ** ppCases=0;
  int           j;

  if (gDataHub.GetParameters().GetMaxGeographicClusterSizeType() == PERCENTOFPOPULATIONFILETYPE)
    gpLocationsPopulation = const_cast<measure_t*>((&gDataHub.GetMaxCirclePopulationArray()[0]));
  else if (gDataHub.GetParameters().GetProbabilityModelType() == ORDINAL) {
    //For the Ordinal model, populations for each location are calculated by adding up the
    //total individuals represented in the catgory case arrays.
    gvCalculatedPopulations.resize(gDataHub.GetNumTracts(), 0);
    //Population is calculated from first data set - even when multiple data sets are defined.
    for (unsigned int k=0; k < gDataHub.GetDataSetHandler().GetDataSet().GetCasesByCategory().size(); ++k) {
       ppCases = gDataHub.GetDataSetHandler().GetDataSet().GetCasesByCategory()[k]->GetArray();
       for (j=0; j < gDataHub.GetNumTracts(); ++j)
          gvCalculatedPopulations[j] += ppCases[0][j];
    }

    gpLocationsPopulation = &gvCalculatedPopulations[0];
  }
  else if (gDataHub.GetParameters().GetProbabilityModelType() == EXPONENTIAL) {
    // consider population as cases and non-censored cases
    gvCalculatedPopulations.assign(gDataHub.GetNumTracts(), 0);
    //Population is calculated from first data set - even when multiple data sets are defined.
    const ExponentialRandomizer * pRandomizer = dynamic_cast<const ExponentialRandomizer*>(gDataHub.GetDataSetHandler().GetRandomizer(0));
    if (!pRandomizer)
      ZdGenerateException("Randomizer failed cast to ExponentialRandomizer.", "constructor()");
    pRandomizer->CalculateMaxCirclePopulationArray(gvCalculatedPopulations);
    gpLocationsPopulation = &gvCalculatedPopulations[0];
  }
  else
    //Population is calculated from first data set - even when multiple data sets are defined.
    gpLocationsPopulation = gDataHub.GetDataSetHandler().GetDataSet().GetMeasureArray()[0];
}

/** destructor */
CentroidNeighborCalculatorByPopulation::~CentroidNeighborCalculatorByPopulation() {}

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size. */
tract_t CentroidNeighborCalculatorByPopulation::CalculateNumberOfNeighboringLocations(measure_t tMaximumSize) const {
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
  tract_t                                       tCount=0;
  measure_t                                     tCumMeasure=0;

  for (; itr != itr_end && (tCumMeasure + gpLocationsPopulation[itr->GetTractNumber()]) <= tMaximumSize; ++itr) {
     tCumMeasure += gpLocationsPopulation[itr->GetTractNumber()];
     if (tCumMeasure <= tMaximumSize)
       ++tCount;
  }
  return tCount;
}

