//*****************************************************************************
#ifndef __GRIDTRACTCOORDINATES_H
#define __GRIDTRACTCOORDINATES_H
//*****************************************************************************
#include "SaTScan.h"
#include "Tracts.h"

/** Abstract base class which manages centroid grid points. */
class GInfo {
  public:
    virtual void        displayGridPoints(FILE* pDisplay);
    virtual int         getGridPointDimensions() const = 0;
    virtual tract_t     getNumGridPoints() const = 0;
    virtual void        retrieveCoordinates(tract_t tPoint, std::vector<double> & vRepository) const = 0;
};

/** Manages centroid grid points in class collection structure. */
class CentroidHandler : public GInfo {
   public:
     typedef TractHandler::Coordinates Point_t;
     typedef ptr_vector<Point_t>       PointsContainer_t;
     enum addition_status_t            {Accepting=0, Closed};

   private:
     addition_status_t   gAdditionStatus;
     PointsContainer_t   gvPoints;
     unsigned int        giPointDimensions;

   public:
     CentroidHandler() : GInfo(), gAdditionStatus(Accepting), giPointDimensions(2) {}

     void                additionsCompleted() {gAdditionStatus = Closed;}
     void                addGridPoint(const std::vector<double>& vCoordinates);
     virtual int         getGridPointDimensions() const {return giPointDimensions;}
     virtual tract_t     getNumGridPoints() const {return (tract_t)gvPoints.size();}
     virtual void        retrieveCoordinates(tract_t tPoint, std::vector<double> & vRepository) const {gvPoints.at(tPoint)->retrieve(vRepository);}
     void                setDimensions(unsigned int iPointDimensions);
};

/** Provides an interface to centroid grid points that are actually maintained by the TractHandler class.
    This class is used when the user does not specifiy a special grid file and the locations of the
    TractHandler class are also grid points. */
class CentroidHandlerPassThrow : public GInfo {
   private:
     const TractHandler   & gTractHandler;
     
   public:
     CentroidHandlerPassThrow(const TractHandler& Handler) : GInfo(), gTractHandler(Handler) {}

    virtual int         getGridPointDimensions() const {return gTractHandler.getCoordinateDimensions();}
    virtual tract_t     getNumGridPoints() const {return gTractHandler.getCoordinates().size();}
    virtual void        retrieveCoordinates(tract_t tPoint, std::vector<double> & vRepository) const
                         { gTractHandler.getCoordinates().at(tPoint)->retrieve(vRepository); }
};
//*****************************************************************************
#endif
