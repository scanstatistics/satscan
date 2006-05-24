//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "GridTractCoordinates.h"
#include "Tracts.h"

/** constructor */
CentroidHandler::GridPoint::GridPoint(const char * sLabel, const std::vector<double>& vCoordinates) {
   gsLabel = new char[std::strlen(sLabel) + 1];
   strcpy(gsLabel, sLabel);
   gpCoordinates = new double[vCoordinates.size()];
   std::memcpy(gpCoordinates, &vCoordinates[0], vCoordinates.size() * sizeof(double));
}

/** constructor */
CentroidHandler::GridPoint::GridPoint(const double* pCoordinates, int size) {
   gsLabel = new char[1];
   gsLabel[0] = '/0';
   gpCoordinates = new double[size];
   std::memcpy(gpCoordinates, pCoordinates, size * sizeof(double));
}

/** destructor  */
CentroidHandler::GridPoint::~GridPoint() {
  try {
    delete[] gsLabel;
    delete[] gpCoordinates;
  }
  catch (...){}
}

void CentroidHandler::giDisplayGridTractInfo(FILE* pDisplay) {
  fprintf(pDisplay, "GID        x           y\n");
  for (size_t i=0; i< gvGridPoints.size(); ++i)
    fprintf(pDisplay, "%s   %f   %f\n", gvGridPoints[i]->gsLabel, gvGridPoints[i]->gpCoordinates[0], gvGridPoints[i]->gpCoordinates[1]);
  fprintf(pDisplay, "\n");
}

/** Inserts grid point into internal container, ordered by first coordinate. */
void CentroidHandler::giInsertGnode(const char *gid, std::vector<double>& vCoordinates) {
  try {
    //validate that passed coordinates have same dimensions as class has defined
    if (vCoordinates.size() != (unsigned int)nDimensions)
      ZdGenerateException("Passed coordinates have %u dimensions, wanted %i.", "giInsertGnode()", vCoordinates.size(), nDimensions);

    ZdPointerVector<GridPoint>::iterator  itr, itrPosition;
    std::auto_ptr<GridPoint> Point(new GridPoint(gid, vCoordinates));
    //find insertion position based upon first coordinate
    itr = itrPosition = lower_bound(gvGridPoints.begin(), gvGridPoints.end(), Point.get(), CompareFirstCoordinate());
    //if there exists a grid point with same coordinates, ignore this record
    while (itr != gvGridPoints.end() && (*itr)->gpCoordinates[0] == vCoordinates[0]) {
       if (!std::memcmp((*itr)->gpCoordinates, &vCoordinates[0], nDimensions * sizeof(double))) return; //ignore duplicates
       ++itr;
    }
    gvGridPoints.insert(itrPosition, Point.release());
  }
  catch (ZdException &x) {
    x.AddCallpath("giInsertGnode()", "CentroidHandler");
    throw;
  }
}

/** Retrieves coordinates of grid point at index 't'. */
void CentroidHandler::giRetrieveCoords(tract_t t, std::vector<double> & vRepository) const {
  if (t < 0 || t >= (tract_t)gvGridPoints.size())
    ZdException::Generate("Index %d is out of bounds: [size=%d].", "CentroidHandler", t, gvGridPoints.size());

  vRepository.resize(nDimensions);
  std::copy(gvGridPoints[t]->gpCoordinates, gvGridPoints[t]->gpCoordinates + nDimensions, vRepository.begin());
}

/** Sorts accumulated grid points by specified label. The internal collection of
    grid points needs to be sorted by their labels to maintain consistancy of output. */
void CentroidHandler::SortGridPointsByLabel() {
  std::sort(gvGridPoints.begin(), gvGridPoints.end(), Comparelabel());
}


int CentroidHandlerPassThrow::giGetDimensions() const {
  return gTractHandler.tiGetDimensions();
}

tract_t CentroidHandlerPassThrow::giGetNumTracts() const {
  return gTractHandler.tiGetNumTracts();
}

void CentroidHandlerPassThrow::giRetrieveCoords(tract_t t, std::vector<double> & vRepository) const {
  gTractHandler.tiRetrieveCoords(t, vRepository);
}

