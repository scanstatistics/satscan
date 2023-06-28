//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "GridTractCoordinates.h"
#include "SSException.h"
#include "LocationNetwork.h"

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
void CentroidHandler::addGridPoint(const std::vector<double>& vCoordinates, const FocusInterval_t& foucsInterval) {
  try {
    if (gAdditionStatus == Closed)
      throw prg_error("This CentroidHandler object is closed to insertions.", "addGridPoint()");

    //validate that passed coordinates have same dimensions as class has defined
    if (vCoordinates.size() != (unsigned int)giPointDimensions)
      throw prg_error("Passed coordinates have %u dimensions, wanted %i.", "addGridPoint()", vCoordinates.size(), giPointDimensions);

    GridPoint point(vCoordinates, _grid_points.size());
    if (foucsInterval.first) {
        point._interval_range.reset(new IntervalRange_t(foucsInterval.second));
        _has_focus_intervals = true;
    }

    //keeping coordinates in sorted order accomplishes two things:
    // 1) makes determination of duplicates much faster
    // 2) consistancy with index into gvPoints -- we sometimes use this attribute to break ties in ranking of clusters
    GridPointsContainer_t::iterator itrGridPoint = std::lower_bound(_grid_points.begin(), _grid_points.end(), point);
    //if there exists a grid point with same coordinates, ignore this record
    if (itrGridPoint == _grid_points.end() || point != (*itrGridPoint))
        _grid_points.insert(itrGridPoint, point);
  } catch (prg_exception& x) {
    x.addTrace("addGridPoint()", "CentroidHandler");
    throw;
  }
}

/** Returns focus interval range for point, if any. First item in returned pair indicates whether point has focus interval. */
CentroidHandler::FocusInterval_t CentroidHandler::retrieveFocusInterval(tract_t tPoint) const {
    IntervalRange_t * interval = _grid_points[tPoint]._interval_range.get();
    return (interval ? std::make_pair(true, *interval) : std::make_pair(false, IntervalRange_t(0,0,0,0)));
}

/** sets dimensions expected for points added to this object. */
void CentroidHandler::setDimensions(unsigned int iPointDimensions) {
  if (_grid_points.size())
    throw prg_error("Changing the coordinate dimensions is not permited once points have been defined.","setDimensions()");
  giPointDimensions = iPointDimensions;
}

//////////////////////// LocationsCentroidHandlerPassThrough /////////////////////////////

void LocationsCentroidHandlerPassThrough::retrieveCoordinates(tract_t tPoint, std::vector<double> & vRepository) const {
	_locations_manager.locations()[tPoint]->coordinates()->retrieve(vRepository);
}

//////////////////////// NetworkCentroidHandlerPassThrough /////////////////////////////

NetworkCentroidHandlerPassThrough::NetworkCentroidHandlerPassThrough(const Network& network) : GInfo(), _network(network) {
    initialize();
}

/* Initializes collection of NetworkNode pointers from defined network nodes. */
void NetworkCentroidHandlerPassThrough::initialize() const {
    _networknodes.reserve(_network.getNodes().size());
    for (auto itr = _network.getNodes().begin(); itr != _network.getNodes().end(); ++itr)
        _networknodes.push_back(&(itr->second));
}

int NetworkCentroidHandlerPassThrough::getGridPointDimensions() const { 
    return (*_networknodes.begin())->getLocation().coordinates()->getSize();
}

/* Returns the network location associated with this grid point. */
const Location & NetworkCentroidHandlerPassThrough::getCentroidLocation(tract_t tPoint) const {
    return _networknodes.at(tPoint)->getLocation();
}

/* Returns the number of nodes in the network - which are being used as grid points/centroids of clusters. */
tract_t NetworkCentroidHandlerPassThrough::getNumGridPoints() const {
    if (_networknodes.empty()) initialize();
    return static_cast<tract_t>(_networknodes.size());
}

/* Retrieves the coordinates of the network node at grid point index. */
void NetworkCentroidHandlerPassThrough::retrieveCoordinates(tract_t tPoint, std::vector<double> & vRepository) const {
    if (_networknodes.empty()) initialize();
    getCentroidLocation(tPoint).coordinates()->retrieve(vRepository);
}

tract_t NetworkCentroidHandlerPassThrough::retrieveLocationIndex(tract_t tPoint) const {
    return static_cast<tract_t>(_networknodes.at(tPoint)->getLocationIndex());
}