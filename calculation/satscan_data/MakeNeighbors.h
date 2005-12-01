//*****************************************************************************
#ifndef __MAKENEIGHBORS_H
#define __MAKENEIGHBORS_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "GridTractCoordinates.h"
#include "CentroidNeighbors.h"

class CSaTScanData; /** forward class declaration */

/** Calculates neighboring locations about centroids with versatility as to whether
    calculations are stored in stored array of CSaTScanData object or allocated to
    passed array. */
class CentroidNeighborCalculator {
  private:
    typedef tract_t             (CentroidNeighborCalculator:: *CALCULATE_NEIGHBORS_METHOD) (measure_t) const;
    typedef tract_t             (CentroidNeighborCalculator:: *CALCULATE_REPORTED_NEIGHBORS_METHOD) (measure_t, count_t) const;

    CALCULATE_NEIGHBORS_METHOD           gpNeighborCalculationMethod;
    CALCULATE_REPORTED_NEIGHBORS_METHOD  gpReportedNeighborCalculationMethod;

    void                        CalculateEllipticCoordinates(tract_t tEllipseOffset);
    void                        CalculateMaximumSpatialClusterSize();
    void                        CalculateMaximumReportedSpatialClusterSize();
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex);
    void                        CalculateNeighborsByCircles();
    void                        CalculateNeighborsByEllipses();
    tract_t                     CalculateNumberOfNeighboringLocationsByDistance(measure_t tMaximumSize) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByDistance(measure_t tMaximumSize, count_t tMaximumNeighbors) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByPopulation(measure_t tMaximumSize) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByPopulation(measure_t tMaximumSize, count_t tMaximumNeighbors) const;
    void                        CenterLocationDistancesAbout(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex);

  protected:
    const CSaTScanData                        & gDataHub;
    const GInfo                               & gCentroidInfo;
    const TractHandler                        & gLocationInfo;
    BasePrint                                 & gPrintDirection;
    measure_t                                   gtMaximumSize;
    measure_t                                   gtMaximumReportedSize;
    std::vector<LocationDistance>               gvCentroidToLocationDistances;
    std::vector<std::pair<double, double> >     gvLocationEllipticCoordinates;
    tract_t                                     gtCurrentEllipseCoordinates;
    const measure_t                           * gpLocationsPopulation;
    const measure_t                           * gpLocationsPopulationReported;
    std::vector<measure_t>                      gvCalculatedPopulations;

    void                        Transform(double Xold, double Yold, float EllipseAngle, float EllipseShape, double* pXnew, double* pYnew);

  public:
    CentroidNeighborCalculator(const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CentroidNeighborCalculator();

    void                        CalculateNeighbors();
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid);
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid, double dMaxRadius);
};
//*****************************************************************************
#endif

