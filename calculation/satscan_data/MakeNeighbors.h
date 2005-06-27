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
     tract_t    m_tTractNumber;     /* tract number */
     double     m_dDistance;        /* distance from centroid to location at m_tTractNumber */

   public:
     LocationDistance(tract_t t=0, double d=0) {SetTractNumber(t); SetDistance(d);}
     virtual ~LocationDistance() {}

     double     GetDistance() const {return m_dDistance;}
     tract_t    GetTractNumber() const {return m_tTractNumber;}
     void       SetDistance(double d) {m_dDistance=d;}
     void       SetTractNumber(tract_t t) {m_tTractNumber=t;}
};

/** Function object used to compare LocationDistance objects by m_dDistance. */
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

/** Interface which details calculated neighbor indexes about a specific centroid. */
class CentroidNeighbors {
  friend class CentroidNeighborCalculator;
  private:
    tract_t                       gtCentroid;                             /* centroid index */
    tract_t                       gtEllipseOffset;                        /* ellipse index */
    int                           giNeighbors;                            /* neighbor count - variable returned in get method */
    int                           giMaxNeighbors;                         /* neighbor count - variable which defines max for replication process */
    int                           giMaxReportedNeighbors;                 /* neighbor count - varible whch defines maximum for evaluation of real data */
    std::vector<tract_t>          gvSortedNeighborsIntegerType;           /* conditionally allocated integer vector of neighbor indexes */
    std::vector<unsigned short>   gvSortedNeighborsUnsignedShortType;     /* conditionally allocated unsigned short vector of neighbors indexes */
    tract_t                     * gpSortedNeighborsIntegerType;           /* pointer to integer array of neighbors indexes */
    unsigned short              * gpSortedNeighborsUnsignedShortType;     /* pointer to unsigned short array of neighbors indexes */

    void                Set(tract_t tEllipseOffset, tract_t tCentroid, int iNumNeighbors, int iNumReportedNeighbors, const std::vector<LocationDistance>& vOrderedLocations);

  public:
    CentroidNeighbors();
    ~CentroidNeighbors();

    tract_t             GetEllipseIndex() const {return gtEllipseOffset;}
    tract_t             GetCentroidIndex() const {return gtCentroid;}
    inline tract_t      GetNeighborTractIndex(size_t tNeighborIndex) const;
    inline int          GetNumNeighbors() const {return giNeighbors;}
    CentroidNeighbors & Set(tract_t tEllipseOffset, tract_t tCentroid, const CSaTScanData& DataHub);
    void                SetMaximumClusterSize_RealData() {giNeighbors = giMaxReportedNeighbors;}
    void                SetMaximumClusterSize_SimulatedData() {giNeighbors = giMaxNeighbors;}
};

/** Returns zero based tNeighborIndex'th nearest neighbor's tract index.
    Caller is responsible for ensuring that internal structures have been previously
    set and 'tNeighborIndex' is within zero based range for defined neighbor information. */
inline tract_t CentroidNeighbors::GetNeighborTractIndex(size_t tNeighborIndex) const {
 //assert(tNeighborIndex + 1 <= (size_t)giNeighbors && (gpSortedNeighborsUnsignedShortType || gpSortedNeighborsIntegerType));
 return (tract_t)(gpSortedNeighborsUnsignedShortType ? gpSortedNeighborsUnsignedShortType[tNeighborIndex] : gpSortedNeighborsIntegerType[tNeighborIndex]);
}

/** Calculates neighboring locations about centroids with versatility as to whether
    calculations are stored in stored array of CSaTScanData object or allocated to
    passed array. */
class CentroidNeighborCalculator {
  private:
    void                        CalculateEllipticCoordinates(tract_t tEllipseOffset);
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex);
    void                        CalculateNeighborsByCircles();
    void                        CalculateNeighborsByEllipses();
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

    virtual tract_t             CalculateNumberOfNeighboringLocations(measure_t tMaximumSize) const = 0;
    void                        Transform(double Xold, double Yold, float EllipseAngle, float EllipseShape, double* pXnew, double* pYnew);

  public:
    CentroidNeighborCalculator(const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CentroidNeighborCalculator();

    void                        CalculateNeighbors();
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid);
    void                        CalculateNeighborsAboutCentroid(tract_t tEllipseOffsetIndex, tract_t tCentroidIndex, CentroidNeighbors& Centroid, double dMaxRadius);
};

/** Calculates neighboring locations about centroids using relative distance
    of locations as limiting factor. */
class CentroidNeighborCalculatorByDistance : public CentroidNeighborCalculator {
  protected:
    virtual tract_t             CalculateNumberOfNeighboringLocations(measure_t tMaximumSize) const;

  public:
    CentroidNeighborCalculatorByDistance(const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CentroidNeighborCalculatorByDistance();
};

/** Calculates neighboring locations about centroids using population of locations
    as limiting factor. */
class CentroidNeighborCalculatorByPopulation : public CentroidNeighborCalculator {
  protected:
    measure_t                 * gpLocationsPopulation;
    std::vector<measure_t>      gvCalculatedPopulations;

    virtual tract_t             CalculateNumberOfNeighboringLocations(measure_t tMaximumSize) const;

  public:
    CentroidNeighborCalculatorByPopulation(const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CentroidNeighborCalculatorByPopulation();
};
//*****************************************************************************
#endif

