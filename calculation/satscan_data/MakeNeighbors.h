//*****************************************************************************
#ifndef __MAKENEIGHBORS_H
#define __MAKENEIGHBORS_H
//*****************************************************************************
#include "SaTScan.h"
#include "Tracts.h"
#include "Parameters.h"
#include "TimeEstimate.h"
#include "GridTractCoordinates.h"

/** Eventually this code will be moved to SaTScanData.cpp */

/** tract-distance class for accumulating neighbors */
class TractDistance {
   private:
     tract_t            m_tTractNumber;     /* tract number      */
     float              m_fDistanceSquared; /* distance squared  */

   public:
     TractDistance(tract_t t=0, float f=0) {SetTractNumber(t); SetDistanceSquared(f);}
     ~TractDistance() {}

     const double       GetDistance() const {return sqrt(m_fDistanceSquared);}
     const float      & GetDistanceSquared() const {return m_fDistanceSquared;}
     const tract_t    & GetTractNumber() const {return m_tTractNumber;}
     void               SetDistanceSquared(float f) {m_fDistanceSquared=f;}
     void               SetTractNumber(tract_t t) {m_tTractNumber=t;}
};

/** Function object used to compare TractDistance objects by m_fDistanceSquared. */
class CompareTractDistance {
  private:
    int                 	gi;                     /* loop index */
    double              	gdCoordinateLHS,        /* coordinate variables */
                        	gdCoordinateRHS;
    std::string                 gsLHS, gsRHS;                            
    bool                	gbContinue;             /* stops coordinate comparisons */
    const TractHandler 	      & gTractInformation;      /* tract information */

  public:
    CompareTractDistance(const TractHandler & TractInformation) : gTractInformation(TractInformation) {}

    bool                operator() (const TractDistance& lhs, const TractDistance& rhs);
};

/** Counts neighbors through expected number of cases using measure array. */
tract_t CountNeighborsByMeasure(std::vector<TractDistance>& vTractDistances,
	                        measure_t Measure[], measure_t MaxCircleSize,
                                measure_t nMaxMeasure);

/** Count neighbors through accumulated distance. */
tract_t CountNeighborsByDistance(std::vector<TractDistance>& vTractDistances,
                                 measure_t MaxDistance);
/** For the circle [e = 0] and each ellipsoid [e = 1, 2, ... n], calculate
    the Sorted[] matrix, such that Sorted[e][a][b] is the b-th
    closest neighbor to a, and Sorted[a][0] == a.
    e = circle or ellipse1, ellipse2, etc.
    a = grid point
    b = neighbor tacts ( sorted closest to farthest.. up to maxcirclesize) */
void MakeNeighbors(TractHandler *pTInfo,
                   GInfo *pGInfo,
                   tract_t***  SortedInt,
                   unsigned short ***SortedUShort,
                   tract_t     NumTracts,
                   tract_t     NumGridTracts,
                   measure_t   Measure[],
                   measure_t   MaxCircleSize,
                   measure_t   nMaxMeasureToKeep,
                   tract_t**   NeighborCounts,
                   int         nDimensions,
                   int         iNumEllipses,
                   const std::vector<double>& vEllipseShapes,
                   const std::vector<int>& vNumEllipseRotations,
                   int         iSpatialMaxType,
                   BasePrint  *pPrintDirection);

void PrintNeighbors(long lTotalEllipses, tract_t GridTracts, tract_t ***Sorted, BasePrint *pPrintDirection);

/** MK 5.2001 - This function transforms the x and y coordinates so that circles
    in the transformed space represent ellipsoids in the original space.
    The input is the old X and Y coordinates, the angle of the longest
   axis of the ellipsoid (0<=EllipseAngle<pi), and the EllipseShape (>1),
   which is defined as the length divided by the width.
   The output is the new X and Y coordinates. */
void Transform(double Xold, double Yold, float EllipseAngle, float EllipseShape, double* pXnew, double* pYnew);

//*****************************************************************************
#endif
