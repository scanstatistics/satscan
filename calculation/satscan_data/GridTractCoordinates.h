//*****************************************************************************
#ifndef __GRIDTRACTCOORDINATES_H
#define __GRIDTRACTCOORDINATES_H
//*****************************************************************************
#include "SaTScan.h"
#include "Tracts.h"
#include "boost/shared_ptr.hpp"

/** Abstract base class which manages centroid grid points. */
class GInfo {
    protected:
        bool                        _has_focus_intervals;

    public:
        GInfo() : _has_focus_intervals(false) {}

        typedef std::pair<bool, IntervalRange_t> FocusInterval_t;

        virtual void                displayGridPoints(FILE* pDisplay) const;
        virtual int                 getGridPointDimensions() const = 0;
        virtual tract_t             getNumGridPoints() const = 0;
        bool                        hasFocusIntervals() const {return _has_focus_intervals;}
        virtual void                retrieveCoordinates(tract_t tPoint, std::vector<double> & vRepository) const = 0;
        virtual FocusInterval_t     retrieveFocusInterval(tract_t tPoint) const = 0;
};

/** Manages centroid grid points in class collection structure. */
class CentroidHandler : public GInfo {
    public:
        enum addition_status_t              {Accepting=0, Closed};
        typedef TractHandler::Coordinates   Point_t;

        class GridPoint {        
            public:
                boost::shared_ptr<Point_t>          _point;
                boost::shared_ptr<IntervalRange_t>  _interval_range;

                GridPoint(const std::vector<double>& Coordinates, unsigned int ordinal) {_point.reset(new Point_t(Coordinates, ordinal));}
                bool operator<(const GridPoint& rhs) const {
                    return *(_point.get()) < *(rhs._point.get());
                }
                bool operator!=(const GridPoint& rhs) const {
                    return *(_point.get()) != *(rhs._point.get());
                }
        };
        typedef std::vector<GridPoint>      GridPointsContainer_t;

    private:
        addition_status_t                   gAdditionStatus;
        GridPointsContainer_t               _grid_points;
        unsigned int                        giPointDimensions;

    public:
        CentroidHandler() : GInfo(), gAdditionStatus(Accepting), giPointDimensions(2) {}

        void                                additionsCompleted() {gAdditionStatus = Closed;}
        void                                addGridPoint(const std::vector<double>& vCoordinates, const FocusInterval_t& foucsInterval);
        virtual int                         getGridPointDimensions() const {return giPointDimensions;}
        virtual tract_t                     getNumGridPoints() const {return (tract_t)_grid_points.size();}
        virtual void                        retrieveCoordinates(tract_t tPoint, std::vector<double> & vRepository) const {_grid_points.at(tPoint)._point.get()->retrieve(vRepository);}
        void                                setDimensions(unsigned int iPointDimensions);
        FocusInterval_t                     retrieveFocusInterval(tract_t tPoint) const;
};

/** Provides an interface to centroid grid points that are actually maintained by the TractHandler class.
    This class is used when the user does not specifiy a special grid file and the locations of the
    TractHandler class are also grid points. */
class CentroidHandlerPassThrough : public GInfo {
   private:
     const TractHandler   & gTractHandler;
     
   public:
     CentroidHandlerPassThrough(const TractHandler& Handler) : GInfo(), gTractHandler(Handler) {}

    virtual int         getGridPointDimensions() const {return gTractHandler.getCoordinateDimensions();}
    virtual tract_t     getNumGridPoints() const {return gTractHandler.getCoordinates().size();}
    virtual void        retrieveCoordinates(tract_t tPoint, std::vector<double> & vRepository) const { gTractHandler.getCoordinates().at(tPoint)->retrieve(vRepository); }
    FocusInterval_t     retrieveFocusInterval(tract_t tPoint) const { throw prg_error("Not implemented for CentroidHandlerPassThrough class.", "retrieveFocusInterval()"); }
};
//*****************************************************************************
#endif
