//*****************************************************************************
#ifndef __MAKENEIGHBORS_H
#define __MAKENEIGHBORS_H
//*****************************************************************************
#include "SaTScan.h"
#include "Tracts.h"
#include "Parameters.h"
#include "TimeEstimate.h"
#include "GridTractCoordinates.h"

/** Distance from a centroid to a neighboring location. */
class LocationDistance {
   private:
     tract_t            m_tTractNumber;     /* tract number      */
     float              m_fDistanceSquared; /* distance squared  */

   public:
     LocationDistance(tract_t t=0, float f=0) {SetTractNumber(t); SetDistanceSquared(f);}
     ~LocationDistance() {}

     const double       GetDistance() const {return sqrt(m_fDistanceSquared);}
     const float      & GetDistanceSquared() const {return m_fDistanceSquared;}
     const tract_t    & GetTractNumber() const {return m_tTractNumber;}
     void               SetDistanceSquared(float f) {m_fDistanceSquared=f;}
     void               SetTractNumber(tract_t t) {m_tTractNumber=t;}
};

/** Function object used to compare LocationDistance objects by m_fDistanceSquared. */
class CompareLocationDistance {
  private:
    int                 	gi;                     /* loop index */
    double              	gdCoordinateLHS,        /* coordinate variables */
                        	gdCoordinateRHS;
    std::string                 gsLHS, gsRHS;
    bool                	gbContinue;             /* stops coordinate comparisons */
    const TractHandler 	      & gTractInformation;      /* tract information */

  public:
    CompareLocationDistance(const TractHandler & TractInformation) : gTractInformation(TractInformation) {}

    bool                operator() (const LocationDistance& lhs, const LocationDistance& rhs);
};

class CSaTScanData; /** forward class declaration */

/** Calculates neighboring locations about centroids with versatility as to whether
    calculations are stored in stored array of CSaTScanData object or allocated to
    passed array. */
class CentroidNeighborCalculator {
  private:
    void                CalculateNeighborsByCircles();
    void                CalculateNeighborsByCircles(tract_t tCentroid, tract_t** SortedInt, unsigned short** SortedUShort, int& iNumNeighbors);
    void                CalculateNeighborsByEllipses();
    void                CalculateNeighborsByEllipses(tract_t tEllipseOffset, tract_t tCentroid, tract_t** SortedInt, unsigned short** SortedUShort, int& iNumNeighbors);

  protected:
    CSaTScanData                              & gDataHub;
    const GInfo                               & gCentroidInfo;
    const TractHandler                        & gLocationInfo;
    BasePrint                                 & gPrintDirection;
    measure_t                                   gtMaximumSize;
    std::vector<LocationDistance>               gvCentroidToLocationDistances;
    std::vector<std::pair<double, double> >     gvLocationEllipticCoordinates;

    virtual tract_t     CalculateNumberOfNeighboringLocations() const = 0;
    void                Transform(double Xold, double Yold, float EllipseAngle, float EllipseShape, double* pXnew, double* pYnew);

  public:
    CentroidNeighborCalculator(CSaTScanData& DataHub, BasePrint& PrintDirection, bool bForRealData);
    virtual ~CentroidNeighborCalculator();

    void                CalculateCentroidNeighbors(tract_t tEllipseOffset, tract_t tCentroid, tract_t** ppSortedInt, unsigned short** ppSortedUShort, int& iNumNeighbors);
    void                CalculateEllipticCoordinates(tract_t tEllipseOffset);
    void                CalculateNeighbors();
};

/** Calculates neighboring locations about centroids using relative distance
    of locations as limiting factor. */
class CentroidNeighborCalculatorByDistance : public CentroidNeighborCalculator {
  protected:
    virtual tract_t     CalculateNumberOfNeighboringLocations() const;

  public:
    CentroidNeighborCalculatorByDistance(CSaTScanData& DataHub, BasePrint& PrintDirection, bool bForRealData);
    virtual ~CentroidNeighborCalculatorByDistance();
};

/** Calculates neighboring locations about centroids using population of locations
    as limiting factor. */
class CentroidNeighborCalculatorByPopulation : public CentroidNeighborCalculator {
  protected:
    measure_t                 * gpLocationsPopulation;
    std::vector<measure_t>      gvCalculatedPopulations;
    measure_t                   gtMaxMeasure;

    virtual tract_t             CalculateNumberOfNeighboringLocations() const;

  public:
    CentroidNeighborCalculatorByPopulation(CSaTScanData& DataHub, BasePrint& PrintDirection, bool bForRealData);
    virtual ~CentroidNeighborCalculatorByPopulation();
};
//*****************************************************************************
#endif

