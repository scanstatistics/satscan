//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "MakeNeighbors.h"
#include "SaTScanData.h"

/** Comparison function for LocationDistance objects. */
bool CompareLocationDistance::operator() (const LocationDistance& lhs, const LocationDistance& rhs) {
  //first check whether distances are equal - we may need to break a tie
  if (lhs.GetDistanceSquared() == rhs.GetDistanceSquared()) {
    // break ties in a controlled scheme:
    //   - compare coordinates starting at first dimension and
    //       continue until last dimension(if needed)
    //   - lesser coordinate breaks tie, not for any particular reason
    //     that was the decision made by Martin Kulldorph.
    //   - if all coordinates are equal, than something is wrong as
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
    return (lhs.GetDistanceSquared() < rhs.GetDistanceSquared());
}

/** constructor */
CentroidNeighborCalculator::CentroidNeighborCalculator(CSaTScanData& DataHub, BasePrint& PrintDirection, bool bForRealData)
                           :gDataHub(DataHub), gPrintDirection(PrintDirection),
                            gCentroidInfo(*DataHub.GetGInfo()), gLocationInfo(*DataHub.GetTInfo()) {
  //allocate vector of LocationDistance objects
  for (tract_t t=0; t < gDataHub.GetNumTracts(); ++t)
     gvCentroidToLocationDistances.push_back(LocationDistance(t));
  //determine maximum size of circle/ellipse
  if (bForRealData && gDataHub.GetParameters().GetRestrictingMaximumReportedGeoClusterSize())
    gtMaximumSize = gDataHub.GetMaxReportedCircleSize();
  else
    gtMaximumSize = gDataHub.GetMaxCircleSize();
}

/** destructor */
CentroidNeighborCalculator::~CentroidNeighborCalculator() {}

/** Calculates neighboring locations about centroid. If ellipse offset is zero, invokes
    CalculateNeighborsByCircles() else invokes CalculateNeighborsByEllipses(). Parameters
    'ppSortedInt' and 'ppSortedUShort' are the address of arrays, not 2D arrays. This function
    is meant to be the public access method used in code refactoring where the sorted array is not utilized.
    NOTE: Caller is responsible of ensuring that 'tEllipseOffset' and 'tCentroid'
          are valid indexes. Either 'ppSortedInt' or 'ppSortedUShort' must be supplied, which
          will be allocated to the size of 'iNumNeighbors' and caller is responsible for deleting array. */
void CentroidNeighborCalculator::CalculateCentroidNeighbors(tract_t tEllipseOffset, tract_t tCentroid,
                                                            tract_t** ppSortedInt, unsigned short** ppSortedUShort, int& iNumNeighbors) {
  if (tEllipseOffset == 0)
    CalculateNeighborsByCircles(tCentroid, ppSortedInt, ppSortedUShort, iNumNeighbors);
  else
    CalculateNeighborsByEllipses(tEllipseOffset, tCentroid, ppSortedInt, ppSortedUShort, iNumNeighbors);
}

/** Transforms the x and y coordinates for each location so that circles
    in the transformed space represent ellipsoids in the original space.
    Stores transformed coordinates in internal array. */
void CentroidNeighborCalculator::CalculateEllipticCoordinates(tract_t tEllipseOffset) {
  double                                                dAngle, dShape;
  std::vector<double>                                   vCoordinates;
  std::vector<std::pair<double, double> >::iterator     itr;

  if (tEllipseOffset == 0)
    return; //offset zero is a circle

  gvLocationEllipticCoordinates.resize(gDataHub.GetNumTracts());
  itr=gvLocationEllipticCoordinates.begin();
  dAngle = gDataHub.GetEllipseAngle(tEllipseOffset);
  dShape = gDataHub.GetEllipseShape(tEllipseOffset);
  for (tract_t t=0; t < gDataHub.GetNumTracts(); ++t, ++itr) {
     gLocationInfo.tiRetrieveCoords(t, vCoordinates);
     Transform(vCoordinates[0], vCoordinates[1], dAngle, dShape, &(itr->first), &(itr->second));
  }
}

/** Calculates neighboring locations about each centroid; storing results in sorted
    array contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighbors() {
  try {
    gDataHub.AllocateSortedArray();
    CalculateNeighborsByCircles();
    CalculateNeighborsByEllipses();
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateNeighbors()", "CentroidNeighborCalculator");
    throw;
  }
}

/** Calculates neighboring locations about each centroid through expanding circle;
    storing results in sorted array contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighborsByCircles() {
  clock_t               tStartTime = clock();

  gPrintDirection.SatScanPrintf("Constructing the circles\n");
  //Calculate neighboring locations about each centroid for circular regions
  if (gDataHub.GetSortedArrayAsTract_T(0))
    for (tract_t t=0; t < gDataHub.m_nGridTracts; ++t) {
       CalculateNeighborsByCircles(t, &(gDataHub.GetSortedArrayAsTract_T(0)[t]),  NULL, gDataHub.GetNeighborCountArray()[0][t]);
       if (t==9) ReportTimeEstimate(tStartTime, gDataHub.m_nGridTracts, t+1, &gPrintDirection);
    }
  else
    for (tract_t t=0; t < gDataHub.m_nGridTracts; ++t) {
       CalculateNeighborsByCircles(t, NULL, &(gDataHub.GetSortedArrayAsUShort_T(0)[t]), gDataHub.GetNeighborCountArray()[0][t]);
       if (t==9) ReportTimeEstimate(tStartTime, gDataHub.m_nGridTracts, t+1, &gPrintDirection);
    }
}

/** Calculates neighboring locations about centroid through expanding circles.
    Parameters 'ppSortedInt' and 'ppSortedUShort' are the address of arrays, not 2D arrays.
    NOTE: Caller is responsible of ensuring that 'tCentroid' is a valid index. Either
          'ppSortedInt' or 'ppSortedUShort' must be supplied, which will be allocated to the size
          of 'iNumNeighbors' and caller is responsible for deleting array. */
void CentroidNeighborCalculator::CalculateNeighborsByCircles(tract_t tCentroid, tract_t** ppSortedInt, unsigned short** ppSortedUShort, int& iNumNeighbors) {
  std::vector<double>    vCentroidCoordinates, vLocationCoordinates;

  gCentroidInfo.giRetrieveCoords(tCentroid, vCentroidCoordinates);
  //calculate distances from centroid to each location
  for (tract_t k=0; k < gDataHub.GetNumTracts(); ++k) {
     gvCentroidToLocationDistances[k].SetTractNumber(k);
     gLocationInfo.tiRetrieveCoords(k, vLocationCoordinates);
     gvCentroidToLocationDistances[k].SetDistanceSquared(gLocationInfo.tiGetDistanceSq(&vCentroidCoordinates[0], &vLocationCoordinates[0]));
  }
  //sort such locations are clostest to farthest
  std::stable_sort(gvCentroidToLocationDistances.begin(), gvCentroidToLocationDistances.end(), CompareLocationDistance(gLocationInfo));
  //calculate closest locations with respect to maximum circle size
  iNumNeighbors += CalculateNumberOfNeighboringLocations();
  //allocate array to store location indexes
  if (ppSortedInt) {
    *ppSortedInt = new tract_t[iNumNeighbors];
     for (tract_t j=iNumNeighbors-1; j >= 0; j--) /* copy tract numbers */
        (*ppSortedInt)[j] = gvCentroidToLocationDistances[j].GetTractNumber();
  }
  else {
    *ppSortedUShort = new unsigned short[iNumNeighbors];
    for (tract_t j=iNumNeighbors-1; j >= 0; j--) /* copy tract numbers */
       (*ppSortedUShort)[j] = gvCentroidToLocationDistances[j].GetTractNumber();
  }
}

/** Calculates neighboring locations about each centroid by distance; storing
    results in multiple dimension arrays contained in CSaTScanData object. */
void CentroidNeighborCalculator::CalculateNeighborsByEllipses() {
  clock_t                       tStartTime = clock();

  //only perform calculation if ellipses requested
  if (!gDataHub.GetParameters().GetNumRequestedEllipses())
    return;

  gPrintDirection.SatScanPrintf("Constructing the ellipsoids\n");
  //Calculate neighboring locations about each centroid for elliptical regions
  for (int i=1; i <= gDataHub.GetParameters().GetNumTotalEllipses(); ++i) {
     //tranform location coordinates for this elliptical angle/shape
     CalculateEllipticCoordinates(i);
     if (gDataHub.GetSortedArrayAsTract_T(0))
       for (tract_t t=0; t < gDataHub.m_nGridTracts; ++t) {
          CalculateNeighborsByEllipses(i, t, &(gDataHub.GetSortedArrayAsTract_T(i)[t]),  NULL, gDataHub.GetNeighborCountArray()[i][t]);
          if (t == 9 && i == 1) ReportTimeEstimate(tStartTime, gDataHub.m_nGridTracts * gDataHub.GetParameters().GetNumTotalEllipses(), t, &gPrintDirection);
       }
     else
       for (tract_t t=0; t < gDataHub.m_nGridTracts; ++t) {
          CalculateNeighborsByEllipses(i, t, NULL, &(gDataHub.GetSortedArrayAsUShort_T(i)[t]), gDataHub.GetNeighborCountArray()[i][t]);
          if (t == 9 && i == 1) ReportTimeEstimate(tStartTime, gDataHub.m_nGridTracts * gDataHub.GetParameters().GetNumTotalEllipses(), t, &gPrintDirection);
       }
  }
}

/** Calculates neighboring locations about centroid through expanding circles.
    Parameters 'ppSortedInt' and 'ppSortedUShort' are the address of arrays, not 2D arrays.
    NOTE: Caller is responsible of ensuring that 'tEllipseOffset' and 'tCentroid'
          are valid indexes. Either 'ppSortedInt' or 'ppSortedUShort' must be supplied, which
          will be allocated to the size of 'iNumNeighbors' and caller is responsible for deleting array. */
void CentroidNeighborCalculator::CalculateNeighborsByEllipses(tract_t tEllipseOffset, tract_t tCentroid, tract_t** ppSortedInt, unsigned short** ppSortedUShort, int& iNumNeighbors) {
  std::vector<double>    vCentroidCoordinates, vLocationCoordinates;

  gCentroidInfo.giRetrieveCoords(tCentroid, vCentroidCoordinates);
  //tranform centroid coordinates into elliptical space
  Transform(vCentroidCoordinates[0], vCentroidCoordinates[1], gDataHub.GetEllipseAngle(tEllipseOffset),
            gDataHub.GetEllipseShape(tEllipseOffset), &vCentroidCoordinates[0], &vCentroidCoordinates[1]);
  vLocationCoordinates.resize(2);
  //calculate distances from centroid to each location
  for (tract_t k=0; k < gDataHub.GetNumTracts(); ++k) {
     vLocationCoordinates[0] = gvLocationEllipticCoordinates[k].first;
     vLocationCoordinates[1] = gvLocationEllipticCoordinates[k].second;
     gvCentroidToLocationDistances[k].SetTractNumber(k);
     gvCentroidToLocationDistances[k].SetDistanceSquared(gLocationInfo.tiGetDistanceSq(&vCentroidCoordinates[0], &vLocationCoordinates[0]));
  }
  //sort such locations are clostest to farthest
  std::stable_sort(gvCentroidToLocationDistances.begin(), gvCentroidToLocationDistances.end(), CompareLocationDistance(gLocationInfo));
  //calculate closest locations with respect to maximum circle size
  iNumNeighbors += CalculateNumberOfNeighboringLocations();
  //allocate array to store location indexes
  if (ppSortedInt) {
    *ppSortedInt = new tract_t[iNumNeighbors];
     for (tract_t j=iNumNeighbors-1; j >= 0; j--) /* copy tract numbers */
        (*ppSortedInt)[j] = gvCentroidToLocationDistances[j].GetTractNumber();
  }
  else {
    *ppSortedUShort = new unsigned short[iNumNeighbors];
    for (tract_t j=iNumNeighbors-1; j >= 0; j--) /* copy tract numbers */
       (*ppSortedUShort)[j] = gvCentroidToLocationDistances[j].GetTractNumber();
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

/** constructor */
CentroidNeighborCalculatorByDistance::CentroidNeighborCalculatorByDistance(CSaTScanData& DataHub, BasePrint& PrintDirection, bool bForRealData)
                                     :CentroidNeighborCalculator(DataHub, PrintDirection, bForRealData){}

/** destructor */
CentroidNeighborCalculatorByDistance::~CentroidNeighborCalculatorByDistance() {}

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size. */
tract_t CentroidNeighborCalculatorByDistance::CalculateNumberOfNeighboringLocations() const {
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
  tract_t                                       tCount=0;

  for (; itr != itr_end && itr->GetDistance() <= gtMaximumSize; ++itr, ++tCount);

  return tCount;
}


/** constructor */
CentroidNeighborCalculatorByPopulation::CentroidNeighborCalculatorByPopulation(CSaTScanData& DataHub, BasePrint& PrintDirection, bool bForRealData)
                                       :CentroidNeighborCalculator(DataHub, PrintDirection, bForRealData){
  count_t    ** ppCases=0;
  int           j;

  if (gDataHub.GetParameters().GetMaxGeographicClusterSizeType() == PERCENTOFPOPULATIONFILETYPE)
    gpLocationsPopulation = const_cast<measure_t*>((&gDataHub.GetPopulationArray()[0]));
  else if (gDataHub.GetParameters().GetProbabilityModelType() == ORDINAL) {
    //For the Ordinal and Survival models, populations for each location is calculated by adding up the
    //total individuals represented in the catgory case arrays.
    gvCalculatedPopulations.resize(gDataHub.GetNumTracts(), 0);
    for (unsigned int k=0; k < gDataHub.GetDataStreamHandler().GetStream(0).GetCasesByCategory().size(); ++k) {
       ppCases = gDataHub.GetDataStreamHandler().GetStream(0).GetCasesByCategory()[k]->GetArray();
       for (j=0; j < gDataHub.GetNumTracts(); ++j)
          gvCalculatedPopulations[j] += ppCases[0][j];
    }

    gpLocationsPopulation = &gvCalculatedPopulations[0];
  }
  else if (gDataHub.GetParameters().GetProbabilityModelType() == SURVIVAL) {
    ZdGenerateException("Don't know how to get data for Survival yet.","constructor()");
  }
  else
    gpLocationsPopulation = gDataHub.GetDataStreamHandler().GetStream(0).GetMeasureArray()[0];


  //$$ This section of code will need revision since GetStream(0).GetTotalMeasure() is always zero for
  //$$ the Ordinal model. Need to talk with Martin regarding the expected functionality of this feature. 

  //determine maximum measure ---- NOT SURE WHY THIS IS; MIGHT HAVE TO DO WITH SEQUENTIAL SCAN
     // Actually, if MaxMeasure to be kept, neighbors don't need to be counted...KR-980327
  if (gDataHub.GetParameters().GetIsSequentialScanning() && gDataHub.GetDataStreamHandler().GetStream(0).GetTotalMeasure() > gtMaximumSize)
    gtMaxMeasure = gDataHub.GetDataStreamHandler().GetStream(0).GetTotalMeasure();
  else
    gtMaxMeasure = gtMaximumSize;
}

/** destructor */
CentroidNeighborCalculatorByPopulation::~CentroidNeighborCalculatorByPopulation() {}

/** Calculates the number of neighboring locations as defined in gvCentroidToLocationDistances
    and maximum circle size. */
tract_t CentroidNeighborCalculatorByPopulation::CalculateNumberOfNeighboringLocations() const {
  std::vector<LocationDistance>::const_iterator itr=gvCentroidToLocationDistances.begin(),
                                                itr_end=gvCentroidToLocationDistances.end();
  tract_t                                       tCount=0;
  measure_t                                     tCumMeasure=0;

  for (; itr != itr_end && (tCumMeasure + gpLocationsPopulation[itr->GetTractNumber()]) <= gtMaxMeasure; ++itr) {
     tCumMeasure += gpLocationsPopulation[itr->GetTractNumber()];
     if (tCumMeasure <= gtMaximumSize)
       ++tCount;
  }
  return tCount;
}

