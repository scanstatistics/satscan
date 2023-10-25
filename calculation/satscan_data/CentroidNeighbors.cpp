//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "CentroidNeighbors.h"
#include "SaTScanData.h"
#include "SSException.h"

/** Comparison function for LocationDistance objects. */
bool CompareIdentifierDistance::operator() (const DistanceToCentroid& lhs, const DistanceToCentroid& rhs) {
	//first check whether distances are equal - we may need to break a tie
	if (lhs.GetDistance() == rhs.GetDistance()) {
		// break ties in a controlled scheme:
		//   - lesser coordinate breaks tie, not for any particular reason
		//     that was the decision made by Martin
		//   - if all coordinates are equal, then continue on to next set of associated coordinates
		//   - finally compare number of associated coordinates

		const Identifier::LocationsSet_t& llhs = _identifier_mgr.getIdentifiers()[lhs.GetTractNumber()]->getLocations();
		const Identifier::LocationsSet_t& rrhs = _identifier_mgr.getIdentifiers()[rhs.GetTractNumber()]->getLocations();

		if (*(llhs[lhs.GetRelativeCoordinateIndex()]->coordinates()) == *(rrhs[rhs.GetRelativeCoordinateIndex()]->coordinates()))
			return false; //equalness is resulting from same coordinates
		else
			// else compare using Coordinates::operator<(const Coordinates& rhs) const
			// this results in first lesser coordinate breaking tie, decision made by Martin not for any particular reason
			return *(llhs[lhs.GetRelativeCoordinateIndex()]->coordinates()) < *(rrhs[rhs.GetRelativeCoordinateIndex()]->coordinates());
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
CentroidNeighbors::CentroidNeighbors(tract_t tEllipseOffset, const CSaTScanData& DataHub, tract_t tCentroid)
                  : gtCentroid(0), gtEllipseOffset(tEllipseOffset), giNeighbors(0), giMaxNeighbors(0),
                    giMaxReportedNeighbors(0), gpSortedNeighborsIntegerType(0),
                    gpSortedNeighborsUnsignedShortType(0), gppSortedNeighborsIntegerType(0),
                    gppSortedNeighborsUnsignedShortType(0), gpNeighborArray(0) {
                    
  gpNeighborArray = DataHub.GetNeighborCountArray()[gtEllipseOffset];
  if (DataHub.GetSortedArrayAsTract_T(tEllipseOffset))
    gppSortedNeighborsIntegerType = DataHub.GetSortedArrayAsTract_T(gtEllipseOffset);
  else
    gppSortedNeighborsUnsignedShortType = DataHub.GetSortedArrayAsUShort_T(gtEllipseOffset);
  if (tCentroid >= 0) Set(tCentroid);
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
void CentroidNeighbors::Set(tract_t tEllipseOffset, tract_t tCentroid, int iNumNeighbors, const std::vector<tract_t>& maxReportedNeighbors, const std::vector<DistanceToCentroid>& vOrderedLocations) {

  //conditionally allocate unsigned short vs tract_t
  if (vOrderedLocations.size() < (size_t)std::numeric_limits<unsigned short>::max()) {
    gvSortedNeighborsUnsignedShortType.resize(iNumNeighbors);
    gpSortedNeighborsUnsignedShortType = (iNumNeighbors ? &gvSortedNeighborsUnsignedShortType[0] : 0);
    for (tract_t j=iNumNeighbors-1; j >= 0; j--) /* copy tract numbers */
       gpSortedNeighborsUnsignedShortType[j] = static_cast<unsigned short>(vOrderedLocations[j].GetTractNumber());
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
  giNeighbors = giMaxReportedNeighbors = maxReportedNeighbors.size() > 0 ? maxReportedNeighbors.back() : 0;
  gvMaxReportedNeighbors = maxReportedNeighbors;
}

