//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "Tracts.h"
#include "SaTScanData.h"
#include "AsciiPrintFormat.h"
#include "SSException.h"
#include "cluster.h"
#include "MetaTractManager.h"
#include "LocationNetwork.h"

//////////////// Coordinates class ///////////////////////////////

/** default constructor */
Coordinates::Coordinates() : giInsertionOrdinal(0), giSize(0), gpCoordinates(0) {}

/** constructor */
Coordinates::Coordinates(const std::vector<double>& coordinates, unsigned int iInsertionOrdinal): giSize(coordinates.size()), giInsertionOrdinal(iInsertionOrdinal) {
   gpCoordinates = new double[giSize];
   memcpy(gpCoordinates, &coordinates[0], giSize * sizeof(double));
}

/** Copy constructor. */
Coordinates::Coordinates(const Coordinates& rhs) {
   giSize = rhs.giSize;
   gpCoordinates = new double[giSize];
   memcpy(gpCoordinates, rhs.gpCoordinates, giSize * sizeof(double));
}

/** constructor */
Coordinates::Coordinates(double x, double y, unsigned int iInsertionOrdinal): giSize(2), giInsertionOrdinal(iInsertionOrdinal) {
   gpCoordinates = new double[giSize];
   gpCoordinates[0] = x;
   gpCoordinates[1] = y;
}

/** destructor */
Coordinates::~Coordinates() {
  try {delete[] gpCoordinates;} catch(...){}
}

/* Returns the distance between two sets of coordinates, unchecked. */
double Coordinates::distance(const double * lhs, const double * rhs, unsigned int size) {
    double sum = 0.0;
    for (unsigned int i = 0; i < size; ++i) sum += (lhs[i] - rhs[i]) * (lhs[i] - rhs[i]);
    return std::sqrt(sum);
}

/* Returns the distance between two coordinates. */
double Coordinates::distanceBetween(const std::vector<double>& vFirstPoint, const std::vector<double>& vSecondPoint) {
    if (vFirstPoint.size() != vSecondPoint.size() || !vFirstPoint.size() || !vSecondPoint.size())
        throw prg_error("Unable to calculate distance between %u coordinates and %u.", "distanceBetween()", vFirstPoint.size(), vSecondPoint.size());
    return distance(&vFirstPoint[0], &vSecondPoint[0], vFirstPoint.size());
}

/* Returns the distance between two coordinates. */
double Coordinates::distanceBetween(const Coordinates& lhs, const Coordinates& rhs) {
    if (lhs.getSize() != rhs.getSize() || !lhs.getSize() || !rhs.getSize())
        throw prg_error("Unable to calculate distance between %u coordinates and %u.", "distanceBetween()", lhs.getSize(), rhs.getSize());
    return distance(lhs.getCoordinates(), rhs.getCoordinates(), lhs.getSize());
}

/* Returns the distance to another coordinate. */
double Coordinates::distanceTo(const Coordinates& other) const {
    return distanceBetween(*this, other);
}

/* Returns the distance to another coordinate. */
double Coordinates::distanceTo(const std::vector<double>& other) const {
    if (getSize() != other.size() || !getSize() || !other.size())
        throw prg_error("Unable to calculate distance between %u coordinates and %u.", "distanceBetween()", getSize(), other.size());
    return distance(getCoordinates(), &other[0], getSize());
}

bool Coordinates::operator<(const Coordinates& rhs) const {
  if (giSize != rhs.giSize) return giSize < rhs.giSize;
  size_t t=0;
  while (t < giSize) {
      if (gpCoordinates[t] != rhs.gpCoordinates[t])
          return gpCoordinates[t] < rhs.gpCoordinates[t];
      ++t;
  }
  return false;
}

/** retrieves coordinates into passed vector */
std::vector<double>& Coordinates::retrieve(std::vector<double>& Repository) const {
  Repository.resize(giSize);
  std::copy(gpCoordinates, gpCoordinates + giSize, Repository.begin());
  return Repository;
}

//////////////// LocationsManager class ///////////////////////////////

LocationsManager::AddStatus LocationsManager::addLocation(const std::string& locationame) {
	boost::shared_ptr<Location> location(new Location(locationame));
	auto itr = std::lower_bound(_locations.begin(), _locations.end(), location, CompareLocationByName());
	if (itr != _locations.end() && itr->get()->name() == locationame)
		return LocationsManager::NameExists;
	_locations.insert(itr, location);
    _max_identifier_length = std::max(_max_identifier_length, static_cast<short>(locationame.size()));
    return LocationsManager::Accepted;
}

/* Adds location with coordinates to collection of known locations. */
LocationsManager::AddStatus LocationsManager::addLocation(const std::string& name, const std::vector<double>& coordinates) {
    // Verify that coordinate dimensions match expected.
    if (coordinates.size() != _expected_dimensions) return LocationsManager::WrongDimensions;
    boost::shared_ptr<Location> location(new Location(name, coordinates, _locations.size()));
    // Check whether this location already exists by name.
    auto itrByName = std::lower_bound(_locations.begin(), _locations.end(), location, CompareLocationByName());
    if (itrByName != _locations.end() && itrByName->get()->name() == name)
        // It exists but maybe this is just the same location definition. Otherwise we're trying to redefine it's coordinates.
        return *(itrByName->get()->coordinates()) != coordinates ? LocationsManager::CoordinateRedefinition : LocationsManager::Duplicate;
    else {
        // New location - check whether this set of coordinates is already defined for another.
        auto itrByCoordinates = std::lower_bound(_locations_by_coordinates.begin(), _locations_by_coordinates.end(), location, CompareLocationByCoordinates());
        if (itrByCoordinates != _locations_by_coordinates.end() && *(itrByCoordinates->get()->coordinates()) == coordinates) {
            if (name < itrByCoordinates->get()->name()) {
                // In order to keep consistent with the name of the location when combining, always choose the alpha-ordered location name.
                itrByCoordinates->get()->rename(name);
                std::sort(_locations.begin(), _locations.end(), CompareLocationByName()); // Now we need to resort.
            }
            return LocationsManager::CoordinateExists; // The location is defined at the same coordinates as an existing location.
        }
        // Insert location with coordinates into sorted structures.
        _locations.insert(itrByName, location);
        _locations_by_coordinates.insert(std::lower_bound(_locations_by_coordinates.begin(), _locations_by_coordinates.end(), location, CompareLocationByCoordinates()), location);
        _max_identifier_length = std::max(_max_identifier_length, static_cast<short>(name.size()));
        return LocationsManager::Accepted;
    }
}

/* Returns the location object with specified coordinates. */
boost::optional<boost::shared_ptr<Location> > LocationsManager::getLocationForCoordinates(const std::vector<double>& coordinates) const {
    boost::shared_ptr<Location> location(new Location("", coordinates, 0));
    auto itrByCoordinates = std::lower_bound(_locations_by_coordinates.begin(), _locations_by_coordinates.end(), location, CompareLocationByCoordinates());
    if (itrByCoordinates != _locations_by_coordinates.end() && *(itrByCoordinates->get()->coordinates()) == coordinates)
        return boost::optional<boost::shared_ptr<Location> >(*itrByCoordinates);
    return boost::optional<boost::shared_ptr<Location> >(boost::none);
}

/** Returns indication of whether coordinates are currently defined. */
bool LocationsManager::getCoordinatesExist(const std::vector<double>& coordinates) const {
    return getLocationForCoordinates(coordinates) != boost::none;

	//boost::shared_ptr<Location> location(new Location("", coordinates));
	//auto itrByCoordinates = std::lower_bound(_locations_by_coordinates.begin(), _locations_by_coordinates.end(), location, CompareLocationByCoordinates());
	//return (itrByCoordinates != _locations_by_coordinates.end() && *(itrByCoordinates->get()->coordinates()) == coordinates);
}

LocationsManager::LocationIdx_t LocationsManager::getLocation(const std::string& locationame) const {
	auto itr = std::lower_bound(_locations.begin(), _locations.end(), boost::shared_ptr<Location>(new Location(locationame)), CompareLocationByName());
	boost::optional<unsigned int> idx(boost::none);
	if (itr != _locations.end() && itr->get()->name() == locationame)
		return LocationIdx_t(boost::optional<unsigned int>(static_cast<unsigned int>(std::distance(_locations.begin(), itr))), itr->get());
	return LocationIdx_t(boost::optional<unsigned int>(boost::none), 0);
}

//////////////// Identifier //////////////////////////////////

Identifier::CombinedIdentifierNames_t& Identifier::retrieveAll(CombinedIdentifierNames_t& Identifiers) const {
	Identifiers.clear();
	Identifiers.add(_name, false);
	for (unsigned int i=0; i < _combined_with.size(); ++i)
		Identifiers.add(_combined_with[i], false);
	return Identifiers;
}

//////////////// IdentifiersManager ///////////////////////////

IdentifiersManager::IdentifiersManager(bool aggregating, MultipleCoordinatesType multiple_coordinates_type) :
	_aggregating_identifiers(aggregating), _multiple_coordinates_type(multiple_coordinates_type), _locations_manager(0), _write_status(Accepting), _num_location_coordinates(0) {
	if (_aggregating_identifiers) {
        _locations_manager.addLocation("All");
        _identifiers.push_back(boost::shared_ptr<Identifier>(new Identifier("All", *_locations_manager.locations().front())));
	}
	_meta_manager_proxy.reset(new MetaManagerProxy(_meta_identifiers_manager, _meta_neighbor_manager));
}

/* Adds location w/o coordinates to the collections of defined locations. If multiple coordinates type is defined as one
   per location, also defines the location as an identifier. Returns the addition status of the location.*/
LocationsManager::AddStatus IdentifiersManager::addLocation(const std::string& locationname) {
	if (_aggregating_identifiers) return LocationsManager::Accepted;

    tract_t tLocationIndex = _meta_identifiers_manager.getMetaPool().getMetaIndex(locationname);
    if (tLocationIndex > -1) {
        _meta_identifiers_manager.addReferenced(tLocationIndex); 
        return LocationsManager::Accepted;
    }

	LocationsManager::AddStatus status = _locations_manager.addLocation(locationname);
	if (status == LocationsManager::Accepted && _multiple_coordinates_type == ONEPERLOCATION)
		addIdentifier(locationname, locationname);
	return status;
}

/* Adds location w/ coordinates to the collections of defined locations. If multiple coordinates type is defined as one
   per location, also defines the location as an identifier. Returns the addition status of the location.*/
LocationsManager::AddStatus IdentifiersManager::addLocation(const std::string& locationname, const std::vector<double>& coordinates) {
	if (_aggregating_identifiers) return LocationsManager::Accepted;
	LocationsManager::AddStatus status = _locations_manager.addLocation(locationname, coordinates);
    if (status == LocationsManager::Accepted && _multiple_coordinates_type == ONEPERLOCATION)
        addIdentifier(locationname, locationname);
    else if (status == LocationsManager::CoordinateExists && _multiple_coordinates_type == ONEPERLOCATION) {
        // This should get picked up in the step which combines identifiers at the same coordinates.
        AddStatus gStatus = addIdentifier(locationname, _locations_manager.getLocationForCoordinates(coordinates).get()->name());
        return gStatus == MultipleLocations ? LocationsManager::CoordinateRedefinition : LocationsManager::Accepted;
        //_locations_manager.getLocationForCoordinates(coordinates).
    }    //combinedWith(const std::string& other)
	return status;
}

/* Sets the coordinates of a known location which doesn't currently have a coordinates defined. */
LocationsManager::AddStatus IdentifiersManager::setLocationCoordinates(const std::string& locationname, const std::vector<double>& coordinates) {
    if (_aggregating_identifiers) return LocationsManager::Accepted;
    if (coordinates.size() != _locations_manager.expectedDimensions()) return LocationsManager::WrongDimensions;
    LocationsManager::LocationIdx_t location = _locations_manager.getLocation(locationname);
    // First check to see if this location is currently defined - skip record if it isn't (i.e. it's not in the network).
    if (location.first == boost::none) return LocationsManager::Accepted;
    auto locationWithCoordinates = _locations_manager.getLocationForCoordinates(coordinates);
    /* I'm not really sure what the correct behavior is here. This function is used in the context of settings the coordinates
       of locations within a user defined network. If locations within the network are unique positions, then how could two or
       more locations have the same coordinates yet have different network connections possibly? */
    if (locationWithCoordinates && locationWithCoordinates->get()->name() != locationname)
        return LocationsManager::CoordinateExists;
    // Assign coordinates, if not already set - otherwise check that we're not trying to assign location to different coordinates.
    if (!location.second->hascoordinates()) {
        const_cast<Location*>(location.second)->setCoordinates(coordinates);
        return LocationsManager::Accepted;
    }
    return *(location.second->coordinates()) != coordinates ? LocationsManager::CoordinateRedefinition : LocationsManager::Accepted;
}

/* Adds identifier to collection if does not yet exist, otherwise added to collection. Location is then added to collection. */
IdentifiersManager::AddStatus IdentifiersManager::addIdentifier(const std::string& identifierName, const std::string& locationame) {
	if (_aggregating_identifiers) return IdentifiersManager::Accepted;
	LocationsManager::LocationIdx_t location = _locations_manager.getLocation(locationame);
	if (location.first == boost::none) return IdentifiersManager::UnknownLocation;
	boost::shared_ptr<Identifier> identifierObj(new Identifier(identifierName, *location.second));
	auto itr = std::lower_bound(_identifiers.begin(), _identifiers.end(), identifierObj, CompareIdenitifers());
	if (itr != _identifiers.end() && identifierName == itr->get()->name()) {
		if (_multiple_coordinates_type == ONEPERLOCATION && itr->get()->getLocations().size())
			return IdentifiersManager::MultipleLocations;
        if (!itr->get()->getLocations().exists(location.second))
		    itr->get()->addLocation(*location.second);
	} else
		_identifiers.insert(itr, identifierObj);
	return IdentifiersManager::Accepted;
}

/* Returns the internal index of named identifier. */
boost::optional<size_t> IdentifiersManager::getIdentifierIndex(const std::string& identifiername) const {
	if (_aggregating_identifiers) return boost::make_optional(static_cast<size_t>(0));

    std::string iname;
    // first search collection of known aggregated identifiers names
    std::map<std::string, std::string>::const_iterator itrm = aggregated_identifiers.find(identifiername);
    if (itrm != aggregated_identifiers.end())
        iname = itrm->second;
    else
        iname = identifiername;

	auto itr = std::lower_bound(
		_identifiers.begin(), _identifiers.end(),
		boost::shared_ptr<Identifier>(new Identifier(iname)), CompareIdenitifers()
	);
	if (itr != _identifiers.end() && iname == itr->get()->name())
		return boost::make_optional(static_cast<size_t>(std::distance(_identifiers.begin(), itr)));
	return boost::none;
}

/** Combines Identifier objects which have the same locations. This ensures that identifiers that are at the same location(s) will
    be evaluated together. This method should be called once all insertions are completed. */
void IdentifiersManager::additionsCompleted(bool bReportingRiskEstimates) {
	_write_status = Closed;
	_meta_identifiers_manager.getMetaPool().assignAtomicIndexes(*this);
	_meta_identifiers_manager.setStateFixed(bReportingRiskEstimates);

    // Assign index to locations.
    _locations_manager.assignIndexes();

	if (_identifiers.size() < 2 || _locations_manager.expectedDimensions() == 0) return;

	// sort by coordinates
	std::sort(_identifiers.begin(), _identifiers.end(), CompareIdenitifersByLocation());
	// search for duplicate identifiers -- those that have identical location(s)
	size_t tSize = _identifiers.size();
	_num_location_coordinates = 0;
	for (size_t tOuter = 0; tOuter < tSize; ++tOuter) {
		_num_location_coordinates += _identifiers[tOuter]->getLocations().size();
		for (size_t tInner = tOuter + 1; tInner < tSize; ++tInner) {
			if (_identifiers[tOuter]->getLocations() != _identifiers[tInner]->getLocations()) break;
			//add identifier of tInner to tOuter, they reference the same coordinate sets
			_identifiers[tOuter]->combinedWith(_identifiers[tInner]->name());
			aggregated_identifiers[_identifiers[tInner]->name()] = _identifiers[tOuter]->name();
			_identifiers.erase(_identifiers.begin() + tInner);
			tInner = tInner - 1;
			tSize = _identifiers.size();
		}
	}
	//re-sort identifiers so that the index of the identifier objects is consistent.
	std::sort(_identifiers.begin(), _identifiers.end(), CompareIdenitifers());
}

void IdentifiersManager::print(FILE* pFile) const {
	fprintf(pFile, "Total Identifiers: %u, Total Locations: %u\n\n", getIdentifiers().size(), getLocationsManager().locations().size());
	for (auto itrg = getIdentifiers().begin(); itrg != getIdentifiers().end(); ++itrg) {
		fprintf(pFile, "Identifier: %s, ", itrg->get()->name().c_str());
		for (unsigned int i = 0; i < itrg->get()->getLocations().size(); ++i) {
			fprintf(pFile, "Locations: %s, Coordinates: ", itrg->get()->getLocations()[i]->name().c_str());
			for (unsigned int c=0; c < itrg->get()->getLocations()[i]->coordinates()->getSize(); ++c)
				fprintf(pFile, "%g ", itrg->get()->getLocations()[i]->coordinates()->getCoordinates()[c]);
		}
		fprintf(pFile, "\n");
	}
}

Identifier::CombinedIdentifierNames_t & IdentifiersManager::retrieveAll(size_t tIndex, Identifier::CombinedIdentifierNames_t& Identifiers) const {
	if ((size_t)tIndex < getIdentifiers().size())
		getIdentifiers()[tIndex]->retrieveAll(Identifiers);
	else if (_meta_identifiers_manager.getMetaIdentifiers().size()) {
		Identifiers.clear();
		Identifiers.add(std::string(_meta_identifiers_manager.getMetaIdentifiers().at((size_t)tIndex - getIdentifiers().size())->getIndentifier()), false);
	} else {
		Identifiers.clear();
		std::vector<tract_t> indexes;
		_meta_neighbor_manager.getIndexes((size_t)tIndex - getIdentifiers().size(), indexes);
		for (size_t t = 0; t < indexes.size(); ++t) {
            Identifier::CombinedIdentifierNames_t tract_identifiers;
			getIdentifiers().at(indexes[t])->retrieveAll(tract_identifiers);
			for (size_t i = 0; i < tract_identifiers.size(); ++i)
				Identifiers.add(tract_identifiers[i], false);
		}
	}
	return Identifiers;
}

/** Prints formatted message to file which details the identifiers that are at the same locations and where combined
    into one identifer object for internal usage. 
    Technically this function is only useful when the multiple coordinates type is one per observation.
    We don't report identifiers in the cluster results, just locations in the cluster. When multiple coordinates
    type is one per observation, the identifiers and locations will be the same set. */
void IdentifiersManager::reportCombinedIdentifiers(FILE * fDisplay) const {
	AsciiPrintFormat PrintFormat;
	bool bPrinted = false;

	for (auto itr= getIdentifiers().begin(); itr != getIdentifiers().end(); ++itr) {
		if (itr->get()->getCombinedWith().size()) {
			if (!bPrinted) {
				PrintFormat.SetMarginsAsOverviewSection();
				std::string buffer = "\nNote: The coordinates file contains location IDs with identical coordinates that were combined into one location. In the "
					"optional output files, combined locations are represented by a single location ID as follows:";
				PrintFormat.PrintAlignedMarginsDataString(fDisplay, buffer);
				PrintFormat.PrintSectionSeparatorString(fDisplay, 0, 1, '-');
				bPrinted = true;
			}
			//First retrieved location ID is the location that represents all others.
			std::string buffer;
			printString(buffer, "%s : %s", itr->get()->name().c_str(), itr->get()->getCombinedWith()[0].c_str());
			for (unsigned int i = 1; i < itr->get()->getCombinedWith().size(); ++i) {
				buffer += ", "; buffer += itr->get()->getCombinedWith()[i].c_str();
			}
			PrintFormat.PrintAlignedMarginsDataString(fDisplay, buffer);
		}
	}
}

/** Returns identifier associated with location at 'tIndex'. If 'tIndex' is greater than
number of locations, it is assumed to be referencing a meta location. */
std::string& IdentifiersManager::getIdentifierNameAtIndex(tract_t tIndex, std::string& name) const {
	if ((size_t)tIndex < getIdentifiers().size())
        name = getIdentifiers().at(tIndex)->name();
	else if (_meta_identifiers_manager.getMetaIdentifiers().size())
        name = _meta_identifiers_manager.getMetaIdentifiers().at((size_t)tIndex - getIdentifiers().size())->getIndentifier();
	else {
		//### still not certain what should be the behavior in this situation ###
		tract_t tFirst = _meta_neighbor_manager.getFirst((size_t)tIndex - getIdentifiers().size());
        name = getIdentifiers().at(tFirst)->name();
	}
    return name;
}

bool IdentifiersManager::addLocationsDistanceOverride(tract_t t1, tract_t t2, double distance) {
	tract_t smaller = std::min(t1, t2), larger = std::max(t1, t2);
	LocationOverrides_t::iterator itrSmaller = _location_distance_overrides.find(smaller);
	if (itrSmaller == _location_distance_overrides.end()) {
		std::map<tract_t, double> v;
		v[larger] = distance;
		_location_distance_overrides[smaller] = v;
	}
	else {
		std::map<tract_t, double>::iterator itrLarger = itrSmaller->second.find(larger);
		if (itrLarger == itrSmaller->second.end()) {
			std::map<tract_t, double> v;
			v[larger] = distance;
			itrSmaller->second = v;
		}
		else if (itrLarger->second != distance)
			return false;
	}
	return true;
}

std::pair<bool, double> IdentifiersManager::getLocationsDistanceOverride(tract_t t1, tract_t t2) const {
	tract_t smaller = std::min(t1, t2), larger = std::max(t1, t2);
	LocationOverrides_t::const_iterator itrSmaller = _location_distance_overrides.find(smaller);
	if (itrSmaller != _location_distance_overrides.end()) {
		std::map<tract_t, double>::const_iterator itrLarger = itrSmaller->second.find(larger);
		if (itrLarger != itrSmaller->second.end()) return std::make_pair(true, itrLarger->second);
	}
	return std::make_pair(false, 0.0);
}

/** Assigns locations and coordinates explicitly. Note that this is a special use function, initially
designed for use with the homogeneous poisson model. It might have been nicer to abstract this class
instead of creating this function but that would have involved a significant re-factor with little benefit. */
void IdentifiersManager::assignExplicitCoordinates(CoordinatesContainer_t& coordinates) {
	assert(_write_status == Accepting);

	try {
		if (_aggregating_identifiers) return; //ignore operation if aggregating identifiers

		// If the internal container of coordinates is equal in size to new container, just copy coordinates.
		if (_locations_manager.locations().size() == coordinates.size()) {
			for (size_t t = 0; t < coordinates.size(); ++t) {
				//Verify that coordinates have equal dimensions...
				if (!coordinates[t])
					throw prg_error("not allolcated.", "pushCoordinates()");
				if (_locations_manager.locations()[t]->coordinates()->getSize() != coordinates[t]->getSize())
					throw prg_error("Coordinate dimension is %u, expected %d.", "pushCoordinates()", coordinates[t]->getSize(), _locations_manager.locations()[t]->coordinates()->getSize());
				memcpy(_locations_manager.locations()[t]->coordinates()->getCoordinates(), coordinates[t]->getCoordinates(), sizeof(double) * coordinates[t]->getSize());
			}
		} else {
			//New container and internal container are not equal in size -- clear and rebuild.  
			_identifiers.clear();
			_locations_manager._locations.clear();
			_locations_manager._locations.resize(coordinates.size(), 0);
			_identifiers.resize(coordinates.size(), 0);
			//Note: I'm ignoring meta data, I think it ok to do that. Also, they are not likely used in conjunction with this function.
			std::vector<double> repo;
			for (size_t t = 0; t < coordinates.size(); ++t) {
				//Verfy that coordinate dimensions match expected.
				if (coordinates[t]->getSize() != _locations_manager.expectedDimensions())
					throw prg_error("Coordinate dimension is %u, expected %d.", "pushCoordinates()", coordinates.size(), _locations_manager.expectedDimensions());
				//Create new copy of coordinates object.
				boost::shared_ptr<Location> location(new Location("_location_", coordinates[t]->retrieve(repo), coordinates[t]->getInsertionOrdinal()));
				_locations_manager._locations[t] = location;
				//Create dummy location identifier and associate coordinate object.
				_identifiers[t] = boost::shared_ptr<Identifier>(new Identifier("_location_", *location.get()));
			}
			_num_location_coordinates = coordinates.size();
		}
	}
	catch (prg_exception& x) {
		x.addTrace("assignExplicitCoordinates()", "IdentifiersManager");
		throw;
	}
}
