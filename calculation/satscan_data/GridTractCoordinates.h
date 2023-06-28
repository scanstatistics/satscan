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
        virtual ~GInfo() {}

        typedef std::pair<bool, IntervalRange_t> FocusInterval_t;

        virtual void                displayGridPoints(FILE* pDisplay) const;
        virtual int                 getGridPointDimensions() const = 0;
        virtual tract_t             getNumGridPoints() const = 0;
        bool                        hasFocusIntervals() const {return _has_focus_intervals;}
        virtual void                retrieveCoordinates(tract_t tPoint, std::vector<double> & vRepository) const = 0;
        virtual FocusInterval_t     retrieveFocusInterval(tract_t tPoint) const = 0;
        virtual tract_t             retrieveLocationIndex(tract_t tPoint) const { return tPoint;  }
};

/** Manages centroid grid points in class collection structure. */
class CentroidHandler : public GInfo {
    public:
        enum addition_status_t              {Accepting=0, Closed};
        //typedef Coordinates                 Point_t;

        class GridPoint {        
            public:
                boost::shared_ptr<Coordinates>          _point;
                boost::shared_ptr<IntervalRange_t>  _interval_range;

                GridPoint(const std::vector<double>& coordinates, unsigned int ordinal) {_point.reset(new Coordinates(coordinates, ordinal));}
                bool operator<(const GridPoint& rhs) const {
                    if (*(_point.get()) == *(rhs._point.get())) {
                        // compare interval range -- they have one
                        if (_interval_range.get() == 0 && rhs._interval_range.get() == 0) return false;
                        if (_interval_range.get() == 0) return true;
                        if (rhs._interval_range.get() == 0) return false;
                        const IntervalRange_t left(*_interval_range.get());
                        const IntervalRange_t right(*rhs._interval_range.get());
                        if (left.get<0>() == right.get<0>()) {
                            if (left.get<1>() == right.get<1>()) {
                                if (left.get<2>() == right.get<2>()) {
                                    return left.get<3>() == right.get<3>() ? false : left.get<3>() < right.get<3>();
                                } else return left.get<2>() < right.get<2>();
                            } else return left.get<1>() < right.get<1>();
                        } else return left.get<0>() < right.get<0>();

                        //return left.get<0>() < right.get<0>() && left.get<1>() < right.get<1>() && left.get<2>() < right.get<2>() && left.get<3>() < right.get<3>();
                    }
                    return *(_point.get()) < *(rhs._point.get());
                }
                bool operator==(const GridPoint& rhs) const {
                    if (*(_point.get()) == *(rhs._point.get())) {
                        // compare interval range -- they have one
                        if (_interval_range.get() == 0 && rhs._interval_range.get() == 0) return true;
                        if (_interval_range.get() == 0 || rhs._interval_range.get() == 0) return false;
                        const IntervalRange_t left(*_interval_range.get());
                        const IntervalRange_t right(*rhs._interval_range.get());
                        return left.get<0>() == right.get<0>() && left.get<1>() == right.get<1>() && left.get<2>() == right.get<2>() && left.get<3>() == right.get<3>();
                    }
                    return false;
                }
                bool operator!=(const GridPoint& rhs) const {
                    return !(*this == rhs);
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

/** Provides an interface to centroid grid points that are actually maintained by the LocationsManager class.
    This class is used when the user does not specifiy a special grid file and the coordinates of the LocationsManager class are also used as grid points. */
class LocationsCentroidHandlerPassThrough : public GInfo {
	private:
		const LocationsManager & _locations_manager;

	public:
		LocationsCentroidHandlerPassThrough(const LocationsManager& manager) : GInfo(), _locations_manager(manager) {}

	virtual int         getGridPointDimensions() const { return _locations_manager.expectedDimensions(); }
	virtual tract_t     getNumGridPoints() const { return static_cast<tract_t>(_locations_manager.locations().size()); }
	virtual void        retrieveCoordinates(tract_t tPoint, std::vector<double> & vRepository) const;
	FocusInterval_t     retrieveFocusInterval(tract_t tPoint) const { throw prg_error("Not implemented for LocationsCentroidHandlerPassThrough class.", "retrieveFocusInterval()"); }
};

class Network;
class NetworkNode;

/** Provides an interface to centroid grid points that are actually nodes of the Network class.
    This class is used when the user does not specifiy a special grid file and the coordinates of the nodes in the network are also used as grid points. */
class NetworkCentroidHandlerPassThrough : public GInfo {
    private:
        const Network & _network;
        mutable std::vector<const NetworkNode*> _networknodes;

        void initialize() const;

    public:
        NetworkCentroidHandlerPassThrough(const Network& network);

        virtual int         getGridPointDimensions() const;
        const Location &    getCentroidLocation(tract_t tPoint) const;
        virtual tract_t     getNumGridPoints() const;
        virtual void        retrieveCoordinates(tract_t tPoint, std::vector<double> & vRepository) const;
        FocusInterval_t     retrieveFocusInterval(tract_t tPoint) const { throw prg_error("Not implemented for NetworkCentroidHandlerPassThrough class.", "retrieveFocusInterval()"); }
        virtual tract_t retrieveLocationIndex(tract_t tPoint) const;
};
//*****************************************************************************
#endif
