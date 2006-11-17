//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "CentroidNeighbors.h"
#include "SaTScanData.h"
#include "SSException.h"

/** Comparison function for LocationDistance objects. */
bool CompareLocationDistance::operator() (const LocationDistance& lhs, const LocationDistance& rhs) {
  //first check whether distances are equal - we may need to break a tie
  if (lhs.GetDistance() == rhs.GetDistance()) {
    // if lhs and rhs reference same location - just return false
    if (lhs.GetTractNumber() == rhs.GetTractNumber())
      return false;
    // break ties in a controlled scheme:
    //   - compare coordinates starting at first dimension and
    //       continue until last dimension(if needed)
    //   - lesser coordinate breaks tie, not for any particular reason
    //     that was the decision made by Martin
    //   - if all coordinates are equal, then continue on to next set of associated coordinates
    //   - finally compare number of associated coordinates
    const TractHandler::Location::CoordsContainer_t& llhs = gTractInformation.getLocations()[lhs.GetTractNumber()]->getCoordinates();
    const TractHandler::Location::CoordsContainer_t& rrhs = gTractInformation.getLocations()[rhs.GetTractNumber()]->getCoordinates();
    for (unsigned int i=0, iMax=std::min(llhs.size(), rrhs.size()); i < iMax; ++i) {
       if (llhs[i] != rrhs[i])
         return *(llhs[i]) != *(rrhs[i]);
    }
    if (llhs.size() == rrhs.size()) throw prg_error("Dulpicate coordinates encountered.", "operator()");
    return llhs.size() < rrhs.size();
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
  giNeighbors = giMaxReportedNeighbors = iNumReportedNeighbors;
}

