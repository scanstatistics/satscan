//*****************************************************************************
#ifndef __GRIDTRACTCOORDINATES_H
#define __GRIDTRACTCOORDINATES_H
//*****************************************************************************
#include "SaTScan.h"

class TractHandler;

/** Abstract base class which manages centroid grid points. */
class GInfo {
  public:
    virtual int         giGetDimensions() const = 0;
    virtual void        giDisplayGridTractInfo(FILE* pDisplay) = 0;
    virtual tract_t     giGetNumTracts() const = 0;
    virtual void        giRetrieveCoords(tract_t t, std::vector<double> & vRepository) const = 0;
};

/** Manages centroid grid points in class collection structure. */
class CentroidHandler : public GInfo {
   private:
     class GridPoint {
       public:
         GridPoint(const char * sLabel, const std::vector<double>& vCoordinates);
         GridPoint(const double* pCoordinates, int size);
         ~GridPoint();

         char   * gsLabel;
         double * gpCoordinates;
     };
     //functor that compares grid point labels
     class Comparelabel {
       public:
         bool operator() (GridPoint * lhs, GridPoint * rhs) {return (strcmp(lhs->gsLabel, rhs->gsLabel) < 0);}
     };
     //functor that compares first coordinate of grid point
     class CompareFirstCoordinate {
       public:
         bool operator() (GridPoint * lhs, GridPoint * rhs) {return (lhs->gpCoordinates[0] < rhs->gpCoordinates[0]);}
     };

     ZdPointerVector<GridPoint>  gvGridPoints;
     int 	                 nDimensions;

   public:
     CentroidHandler() : GInfo() {}

     virtual void        giDisplayGridTractInfo(FILE* pDisplay);
     virtual int         giGetDimensions() const {return nDimensions;}
     virtual tract_t     giGetNumTracts() const {return (tract_t)gvGridPoints.size();}
     void                giInsertGnode(const char *gid, std::vector<double>& vCoordinates);
     virtual void        giRetrieveCoords(tract_t t, std::vector<double> & vRepository) const;
     void                giSetDimensions(int nDim) {nDimensions = nDim;}
     void                SortGridPointsByLabel();
};

/** Provides an interface to centroid grid points that are actually maintained by the TractHandler class.
    This class is used when the user does not specifiy a special grid file and the locations of the
    TractHandler class are also grid points. */
class CentroidHandlerPassThrow : public GInfo {
   private:
     const TractHandler   & gTractHandler;
     
   public:
     CentroidHandlerPassThrow(const TractHandler& Handler) : GInfo(), gTractHandler(Handler) {}

    virtual void        giDisplayGridTractInfo(FILE* pDisplay) {}
    virtual int         giGetDimensions() const;
    virtual tract_t     giGetNumTracts() const;
    virtual void        giRetrieveCoords(tract_t t, std::vector<double> & vRepository) const;
};
//*****************************************************************************
#endif
