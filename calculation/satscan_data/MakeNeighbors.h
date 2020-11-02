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
  public:
    typedef std::vector<LocationDistance> LocationDistContainer_t;

  private:
    typedef tract_t (CentroidNeighborCalculator:: *CALCULATE_NEIGHBORS_METHOD) (measure_t) const;
    typedef tract_t (CentroidNeighborCalculator:: *CALCULATE_NEIGHBORS_LIMIT_METHOD) (measure_t, count_t) const;
    typedef std::pair<CALCULATE_NEIGHBORS_METHOD, measure_t> PrimaryCalcPair_t;
    typedef std::pair<CALCULATE_NEIGHBORS_LIMIT_METHOD, measure_t> SecondaryCalcPair_t;
    typedef std::pair<CALCULATE_NEIGHBORS_LIMIT_METHOD, std::vector<measure_t> > ReportedCalcPair_t;

    PrimaryCalcPair_t           gPrimaryNeighbors;
    SecondaryCalcPair_t         gSecondaryNeighbors;
    SecondaryCalcPair_t         gTertiaryNeighbors;
    ReportedCalcPair_t         gPrimaryReportedNeighbors;
    ReportedCalcPair_t         gSecondaryReportedNeighbors;
    ReportedCalcPair_t         gTertiaryReportedNeighbors;
    const measure_t           * gpPopulation;
    const measure_t           * gpMaxCircleFilePopulation;
    const CParameters         & gParameters;
    const GInfo               & gCentroidInfo;
    const TractHandler        & gLocationInfo;
    BasePrint                 & gPrintDirection;
    tract_t                     gtCurrentEllipseCoordinates;
    std::vector<measure_t>      gvCalculatedPopulations;
    LocationDistContainer_t     gvCentroidToLocationDistances;
    std::vector<std::pair<double, double> > gvLocationEllipticCoordinates;
    tract_t                     gNumTracts;
    std::vector<double>         gvEllipseAngles;
    std::vector<double>         gvEllipseShapes; 

    void                        AdjustedNeighborCountsForMultipleCoordinates(std::pair<int, std::vector<int> >& prNeighborsCount);
    void                        CalculateEllipticCoordinates(tract_t tEllipseOffset);
    void                        CalculateMaximumSpatialClusterSize(const CSaTScanData& dataHub);
    void                        CalculateMaximumReportedSpatialClusterSize(const CSaTScanData& dataHub);
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex);
    void                        CalculateNeighborsByCircles(const CSaTScanData& dataHub);
    void                        CalculateNeighborsByEllipses(const CSaTScanData& dataHub);
	void                        CalculateNeighborsByNetwork(const CSaTScanData& dataHub);

    void                        CalculateNeighborsForCurrentState(std::pair<int, std::vector<int> >& prNeigborsCount) const;
    //void                        CalculateNeighborsForCurrentState(std::pair<int, int>& prNeigborsCount) const;

    tract_t                     CalculateNumberOfNeighboringLocationsByDistance(measure_t tMaximumSize) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByDistance(measure_t tMaximumSize, count_t tMaximumNeighbors) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByMaxCirclePopulation(measure_t tMaximumSize) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByMaxCirclePopulation(measure_t tMaximumSize, count_t tMaximumNeighbors) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByPopulationAtRisk(measure_t tMaximumSize) const;
    tract_t                     CalculateNumberOfNeighboringLocationsByPopulationAtRisk(measure_t tMaximumSize, count_t tMaximumNeighbors) const;
    void                        CenterLocationDistancesAbout(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex);
    void                        CoupleLocationsAtSameCoordinates(std::pair<int, std::vector<int> >& prNeighborsCount);
    int                         getAdjustedNeighborCountsForMultipleCoordinates(int iNeigborsCount);
    double                      GetEllipseAngle(int iEllipseIndex) const;
    double                      GetEllipseShape(int iEllipseIndex) const;
    void                        printCentroidToLocationDistances(size_t tMaxToPrint, FILE * stream=stdout);
    void                        setMetaLocations(std::vector<measure_t>& popMeasure);
    void                        SetupPopulationArrays(const CSaTScanData& dataHub);

  public:
    CentroidNeighborCalculator(const CSaTScanData& DataHub, BasePrint& PrintDirection);
    CentroidNeighborCalculator(const CSaTScanData& DataHub, const TractHandler& tractHandler, const GInfo& gridInfo, BasePrint& PrintDirection);
    virtual ~CentroidNeighborCalculator();

    void                        CalculateNeighbors(const CSaTScanData& dataHub);
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid);
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid, double dMaxRadius);
    const LocationDistContainer_t & getLocationDistances() const {return gvCentroidToLocationDistances;}
    static void                 getTractCoordinates(const CSaTScanData& DataHub, const CCluster& Cluster, tract_t tTract, std::vector<double>& Coordinates);
    static  void                Transform(double Xold, double Yold, float EllipseAngle, float EllipseShape, double* pXnew, double* pYnew);
};
//*****************************************************************************
#endif

