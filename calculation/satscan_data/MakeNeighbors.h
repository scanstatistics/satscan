//*****************************************************************************
#ifndef __MAKENEIGHBORS_H
#define __MAKENEIGHBORS_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "GridTractCoordinates.h"
#include "CentroidNeighbors.h"

class CSaTScanData; /** forward class declaration */
class CCluster;

/** Calculates neighboring locations about centroids with versatility as to whether
    calculations are stored in stored array of CSaTScanData object or allocated to
    passed array. */
class CentroidNeighborCalculator {
  private:
    typedef tract_t (CentroidNeighborCalculator:: *CALCULATE_NEIGHBORS_METHOD) (measure_t) const;
    typedef tract_t (CentroidNeighborCalculator:: *CALCULATE_NEIGHBORS_LIMIT_METHOD) (measure_t, count_t) const;
    typedef std::pair<CALCULATE_NEIGHBORS_METHOD, measure_t> PrimaryCalcPair_t;
    typedef std::pair<CALCULATE_NEIGHBORS_LIMIT_METHOD, measure_t> SecondaryCalcPair_t;

    PrimaryCalcPair_t           gPrimaryNeighbors;
    SecondaryCalcPair_t         gSecondaryNeighbors;
    SecondaryCalcPair_t         gTertiaryNeighbors;
    SecondaryCalcPair_t         gPrimaryReportedNeighbors;
    SecondaryCalcPair_t         gSecondaryReportedNeighbors;
    SecondaryCalcPair_t         gTertiaryReportedNeighbors;
    const measure_t           * gpPopulation;
    const measure_t           * gpMaxCircleFilePopulation;
    const CSaTScanData        & gDataHub;
    const GInfo               & gCentroidInfo;
    const TractHandler        & gLocationInfo;
    BasePrint                 & gPrintDirection;
    tract_t                     gtCurrentEllipseCoordinates;
    std::vector<measure_t>      gvCalculatedPopulations;
    std::vector<LocationDistance> gvCentroidToLocationDistances;
    std::vector<std::pair<double, double> > gvLocationEllipticCoordinates;

    void                        CalculateEllipticCoordinates(tract_t tEllipseOffset);
    void                        CalculateMaximumSpatialClusterSize();
    void                        CalculateMaximumReportedSpatialClusterSize();
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex);
    void                        CalculateNeighborsByCircles();
    void                        CalculateNeighborsByEllipses();
    void                        CalculateNeighborsForCurrentState(std::pair<int, int>& prNeigborsCount) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByDistance(measure_t tMaximumSize) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByDistance(measure_t tMaximumSize, count_t tMaximumNeighbors) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByMaxCirclePopulation(measure_t tMaximumSize) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByMaxCirclePopulation(measure_t tMaximumSize, count_t tMaximumNeighbors) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByPopulationAtRisk(measure_t tMaximumSize) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByPopulationAtRisk(measure_t tMaximumSize, count_t tMaximumNeighbors) const;
    void                        CenterLocationDistancesAbout(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex);
    void                        setMetaLocations(std::vector<measure_t>& popMeasure);
    void                        SetupPopulationArrays();

  public:
    CentroidNeighborCalculator(const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CentroidNeighborCalculator();

    void                        CalculateNeighbors();
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid);
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid, double dMaxRadius);
    static void                 getTractCoordinates(const CSaTScanData& DataHub, const CCluster& Cluster, tract_t tTract, std::vector<double>& Coordinates);
    static  void                Transform(double Xold, double Yold, float EllipseAngle, float EllipseShape, double* pXnew, double* pYnew);
};
//*****************************************************************************
#endif

