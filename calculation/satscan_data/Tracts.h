//*****************************************************************************
#ifndef __TRACTS_H
#define __TRACTS_H
//*****************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "Parameters.h"
#include "MultipleDimensionArrayHandler.h"
#include "MetaTractManager.h"
#include "ptr_vector.h"
#include <boost/optional.hpp>

 /** Class representing cartesian coordinates in any number of dimensions. */
class Coordinates {
	protected:
		double        * gpCoordinates;
		unsigned int    giSize;
		unsigned int    giInsertionOrdinal; // tracks insertion ordinal

        static double   distance(const double * lhs, const double * rhs, unsigned int size);

	public:
		Coordinates();
		Coordinates(const Coordinates& rhs);
		Coordinates(const std::vector<double>& coordinates, unsigned int iInsertionOrdinal = 0);
		Coordinates(double x, double y, unsigned int iInsertionOrdinal = 0);
		virtual ~Coordinates();

        static double distanceBetween(const std::vector<double>& vFirstPoint, const std::vector<double>& vSecondPoint);
        static double distanceBetween(const Coordinates& lhs, const Coordinates& rhs);

        double distanceTo(const Coordinates& other) const;
        double distanceTo(const std::vector<double>& other) const;
        bool operator<(const Coordinates& rhs) const;
		virtual bool operator==(const Coordinates& rhs) const {
			if (giSize != rhs.giSize) return false;
			return memcmp(gpCoordinates, rhs.gpCoordinates, giSize * sizeof(double)) == 0;
		}
		virtual bool operator==(const std::vector<double>& coordinates) const {
			if (giSize != coordinates.size()) return false;
			return memcmp(gpCoordinates, &coordinates[0], giSize * sizeof(double)) == 0;
		}
		bool operator!=(const Coordinates& rhs) const {
			if (giSize != rhs.giSize) return true;
			return memcmp(gpCoordinates, rhs.gpCoordinates, giSize * sizeof(double)) != 0;
		}
		virtual bool operator!=(const std::vector<double>& coordinates) const {
			if (giSize != coordinates.size()) return true;
			return memcmp(gpCoordinates, &coordinates[0], giSize * sizeof(double)) != 0;
		}
		double * getCoordinates() const { return gpCoordinates; }
		unsigned int getInsertionOrdinal() const { return giInsertionOrdinal; }
		unsigned int getSize() const { return giSize; }
		std::vector<double>& retrieve(std::vector<double>& Repository) const;
};

/** Function object used to compare Coordinates. */
class CompareCoordinates {
	public:
		bool operator() (const Coordinates * plhs, const Coordinates * prhs) { return *plhs < *prhs; }
};

/* Class representing a location of a population, case control, etc. */
class Location {
	protected:
		std::string _locationname;
        boost::shared_ptr<const Coordinates> _coordinates;
        unsigned int _index; // index in container structure when alphabetically ordered.

	public:
		Location(const std::string& locationname) : _locationname(locationname), _index(0) {}
        Location(const std::string& name, const std::vector<double>& coordinates) : _locationname(name), _coordinates(new Coordinates(coordinates)) {}
        Location(const std::string& name, double x, double y) : _locationname(name), _coordinates(new Coordinates(x, y)) {}
        virtual ~Location() {}

		const std::string& name() const { return _locationname; }
        unsigned int index() const { return _index; }
        void setindex(unsigned int i) { _index = i; }
        virtual bool hascoordinates() const { return _coordinates.get() != 0; }
		virtual const boost::shared_ptr<const Coordinates>& coordinates() const {
            return _coordinates;
		}
        void setCoordinates(const std::vector<double>& coordinates) { 
            if (hascoordinates()) throw prg_error("Attempting to redefine location coordinates.", "setCoordinates()");
            _coordinates.reset(new Coordinates(coordinates));
        }
};

class CompareLocationByName {
	public:
		bool operator() (const boost::shared_ptr<Location>& lhs, const boost::shared_ptr<Location>& rhs) {
			return lhs->name() < rhs->name();
		}
};

class CompareLocationByCoordinates {
    public:
	    bool operator() (const boost::shared_ptr<Location>& lhs, const boost::shared_ptr<Location>& rhs) {
		    return *(lhs->coordinates()) < *(rhs->coordinates());
	    }
};

class ObservationGroupingManager;

/* Add class to manage and maintain the collection of locations. */
class LocationsManager {
	friend class ObservationGroupingManager;

	public:
		typedef std::vector<boost::shared_ptr<Location> > LocationContainer_t;
		typedef std::pair<boost::optional<unsigned int>, const Location*> LocationIdx_t;
		enum AddStatus { Accepted = 0, NameExists, CoordinateExists, WrongDimensions, Duplicate, CoordinateRedefinition };

	protected:
		LocationContainer_t _locations;
        LocationContainer_t _locations_by_coordinates;
        unsigned int _expected_dimensions;
        short _max_identifier_length;


	public:
		LocationsManager(unsigned int dimensions) : _expected_dimensions(dimensions), _max_identifier_length(0) {}
		~LocationsManager() {}

		AddStatus addLocation(const std::string& locationame);
		AddStatus addLocation(const std::string& locationame, const std::vector<double>& coordinates);
        void assignIndexes() {
            for (auto itr = _locations.begin(); itr != _locations.end(); ++itr)
                itr->get()->setindex(std::distance(_locations.begin(), itr));
        }
        boost::optional<boost::shared_ptr<Location> > getLocationForCoordinates(const std::vector<double>& coordinates) const;
        bool getCoordinatesExist(const std::vector<double>& coordinates) const;
		unsigned int expectedDimensions() const { return _expected_dimensions; }
		void setExpectedDimensions(unsigned int i) {
			/*
            if (_locations.size())
				throw prg_error("Unable to set coordinate expected dimensions - locations already exist", "setExpectedDimensions()");
            */
			_expected_dimensions = i;
		}
		const LocationContainer_t& locations() const { return _locations; }
		bool locationExists(const std::string& locationame) const {
			auto itr = std::lower_bound(_locations.begin(), _locations.end(), boost::shared_ptr<Location>(new Location(locationame)), CompareLocationByName());
			return itr != _locations.end() && itr->get()->name() == locationame;
		}
		LocationIdx_t getLocation(const std::string& locationame) const;
        short getMaxNameLength() const { return _max_identifier_length; }
};

/* Observation grouping and it's associated locations. */
class ObservationGrouping {
	friend class ObservationGroupingManager;

	public:
		typedef MinimalGrowthArray<const Location*> LocationsSet_t;
		typedef MinimalGrowthArray<std::string> CombinedGroupNames_t;

	protected:
		std::string _groupname;
		LocationsSet_t _locations;
		CombinedGroupNames_t _combined_with;

		ObservationGrouping(const std::string& groupname) : _groupname(groupname) {}

	public:
		ObservationGrouping(const std::string& groupname, const Location& location) : _groupname(groupname) {
			if (!_locations.exists(&location)) _locations.add(&location, true);
		}
		~ObservationGrouping() {}

		bool operator==(const ObservationGrouping& rhs) const {
			return _groupname == rhs._groupname;
		}

		void addLocation(const Location& location) { 
            _locations.add(&location, false);
            auto comp = [&](const Location * m, const Location * n)-> bool {
                return m->name() < n->name();
            };
            _locations.sort(comp);
        }
		const LocationsSet_t& getLocations() { return _locations; }
		const std::string& groupname() const { return _groupname; }
		const CombinedGroupNames_t& getCombinedWith() const { return _combined_with; }
		void combinedWith(const std::string& other) { if (!_combined_with.exists(other)) _combined_with.add(other, true); }
		CombinedGroupNames_t& retrieveAllIdentifiers(CombinedGroupNames_t& Identifiers) const;
};

class CompareObservationGrouping {
	public:
		bool operator() (const boost::shared_ptr<ObservationGrouping>& lhs, const boost::shared_ptr<ObservationGrouping>& rhs) {
			return lhs->groupname() < rhs->groupname();
		}
};

/* Comparision functor which compares the locations of the ObservationGrouping objects. */
class CompareObservationGroupingByLocation {
	public:
		bool operator() (const boost::shared_ptr<ObservationGrouping> lhs, const boost::shared_ptr<ObservationGrouping> rhs) {
			if (lhs->getLocations().size() != rhs->getLocations().size())
				return lhs->getLocations().size() < rhs->getLocations().size();
			for (unsigned int i=0; i < lhs->getLocations().size(); ++i) {
				if (lhs->getLocations()[i]->name() != rhs->getLocations()[i]->name())
					return lhs->getLocations()[i]->name() < rhs->getLocations()[i]->name();
			}
			return lhs->groupname() < rhs->groupname();
		}
};

/* A class which manages observation groups and their coordinates. */
class ObservationGroupingManager {
	public:
		typedef std::vector<boost::shared_ptr<ObservationGrouping>> ObservationGrouping_t;
		typedef ptr_vector<Coordinates> CoordinatesContainer_t;
        typedef std::map<tract_t, std::map<tract_t, double> > LocationOverrides_t;
        enum AddStatus { Accepted = 0, UnknownLocation, MultipleLocations };
		enum WriteStatus { Accepting = 0, Closed };

	protected:
		WriteStatus _write_status;
		bool _aggregating;
		MultipleCoordinatesType _multiple_coordinates_type;
		ObservationGrouping_t _groupings;
		LocationsManager  _locations_manager;
		std::map<std::string, std::string> gmAggregateTracts;

		size_t                              giNumLocationCoordinates;
		MetaObsGroupManager                 gMetaObsGroupsManager;
		MetaNeighborManager                 gMetaNeighborManager;
		std::auto_ptr<MetaManagerProxy>     gMetaManagerProxy;
		LocationOverrides_t                 _location_distance_overrides;

	public:
		ObservationGroupingManager(bool aggregating, MultipleCoordinatesType multiple_coordinates_type);
		virtual ~ObservationGroupingManager() {}

		void additionsCompleted(bool bReportingRiskEstimates=false);
		LocationsManager::AddStatus addLocation(const std::string& locationname);
		LocationsManager::AddStatus addLocation(const std::string& locationname, const std::vector<double>& coordinates);
        LocationsManager::AddStatus setLocationCoordinates(const std::string& locationname, const std::vector<double>& coordinates);
        bool addLocationsDistanceOverride(tract_t t1, tract_t t2, double distance);
		void assignExplicitCoordinates(CoordinatesContainer_t& coordinates);
		bool getLocationsDistanceOverridesExist() const { return _location_distance_overrides.size() != 0; }
		std::pair<bool, double> getLocationsDistanceOverride(tract_t t1, tract_t t2) const;
		ObservationGroupingManager::AddStatus addObservationGroup(const std::string& groupName, const std::string& locationame);
		boost::optional<size_t> getObservationGroupIndex(const std::string& groupname) const;
		const LocationsManager& getLocationsManager() const { return _locations_manager; }
		size_t getNumLocationCoordinates() const { return giNumLocationCoordinates; }
		const ObservationGrouping_t& getObservationGroups() const { return _groupings; }
        const std::map<std::string, std::string>& getAggregated() const { return gmAggregateTracts; }
        const WriteStatus getWriteStatus() const { return _write_status; }
        void print(FILE* pFile) const;
		void setExpectedCoordinateDimensions(unsigned int expected_dimension) {
			if (_aggregating) return;
			_locations_manager.setExpectedDimensions(expected_dimension);
		}

		ObservationGrouping::CombinedGroupNames_t & retrieveAllIdentifiers(size_t tIndex, ObservationGrouping::CombinedGroupNames_t& Identifiers) const;
		void reportCombinedObsGroups(FILE * fDisplay) const;
		std::string& getGroupname(tract_t tIndex, std::string& groupname) const;

		MetaObsGroupManager               & getMetaObsGroupsManager() { return gMetaObsGroupsManager; }
		const MetaObsGroupManager         & getMetaObsGroupsManager() const { return gMetaObsGroupsManager; }
		const MetaNeighborManager         & getMetaNeighborManager() const { return gMetaNeighborManager; }
		MetaNeighborManager               & getMetaNeighborManager() { return gMetaNeighborManager; }
		const MetaManagerProxy            & getMetaManagerProxy() const { return *gMetaManagerProxy; }
};

//*****************************************************************************
#endif
