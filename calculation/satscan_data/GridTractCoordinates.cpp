//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "GridTractCoordinates.h"
#include "SSException.h"

/** Prints coordinates to file stream. */
void GInfo::displayGridPoints(FILE* pDisplay) const {
  std::vector<double> vRepository;
  
  fprintf(pDisplay, "Grid Points: %u\n", getNumGridPoints());
  for (tract_t tPoint=0; tPoint < getNumGridPoints(); ++tPoint) {
    retrieveCoordinates(tPoint, vRepository);
    for (size_t c=0; c < vRepository.size(); ++c) fprintf(pDisplay, "%lf\t", vRepository[c]);
    fprintf(pDisplay, "\n");
  }
}

/** Inserts grid point into internal container, ordered by coordinates. */
void CentroidHandler::addGridPoint(const std::vector<double>& vCoordinates) {
  try {
    if (gAdditionStatus == Closed)
      throw prg_error("This TractHandler object is closed to insertions.", "addGridPoint()");

    //validate that passed coordinates have same dimensions as class has defined
    if (vCoordinates.size() != (unsigned int)giPointDimensions)
      throw prg_error("Passed coordinates have %u dimensions, wanted %i.", "addGridPoint()", vCoordinates.size(), giPointDimensions);

    std::auto_ptr<Point_t> pPoint(new Point_t(vCoordinates, gvPoints.size()));
    //keeping coordinates in sorted order accomplishes two things:
    // 1) makes determination of duplicates much faster
    // 2) consistancy with index into gvPoints -- we sometimes use this attribute to break ties in ranking of clusters
    PointsContainer_t::iterator itrPoint = std::lower_bound(gvPoints.begin(), gvPoints.end(), pPoint.get(), TractHandler::CompareCoordinates());
    //if there exists a grid point with same coordinates, ignore this record
    if (itrPoint == gvPoints.end() ||  *(pPoint.get()) != *(*itrPoint))
      gvPoints.insert(itrPoint, pPoint.release());
  }
  catch (prg_exception& x) {
    x.addTrace("addGridPoint()", "CentroidHandler");
    throw;
  }
}

/** sets dimensions expected for points added to this object. */
void CentroidHandler::setDimensions(unsigned int iPointDimensions) {
  if (gvPoints.size())
    throw prg_error("Changing the coordinate dimensions is not permited once points have been defined.","setDimensions()");
  giPointDimensions = iPointDimensions;
}

