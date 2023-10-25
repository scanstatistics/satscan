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

 /** Class representing coordinates in any number of dimensions. */
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
        bool operator()(const Coordinates & other) {
            return *this < other;
        }
        double * getCoordinates() const { return gpCoordinates; }
		unsigned int getInsertionOrdinal() const { return giInsertionOrdinal; }
		unsigned int getSize() const { return giSize; }
		std::vector<double>& retrieve(std::vector<double>& Repository) const;
};

/* Class representing a geographical location. */
class Location {
	protected:
		std::string _location_name;
        boost::shared_ptr<const Coordinates> _coordinates;
        unsigned int _index; // index in container structure when alphabetically ordered.

	public:
		Location(const std::string& locationname) : _location_name(locationname), _index(0) {}
        Location(const std::string& name, const std::vector<double>& coordinates, unsigned int iInsertionOrdinal) : _location_name(name), _coordinates(new Coordinates(coordinates, iInsertionOrdinal)) {}
        Location(const std::string& name, double x, double y, unsigned int iInsertionOrdinal) : _location_name(name), _coordinates(new Coordinates(x, y, iInsertionOrdinal)) {}
        virtual ~Location() {}

		const std::string& name() const { return _location_name; }
        void rename(const std::string& name) { _location_name = name; }
        unsigned int index() const { return _index; }
        void setindex(unsigned int i) { _index = i; }
        virtual bool hascoordinates() const { return _coordinates.get() != 0; }
		virtual const boost::shared_ptr<const Coordinates>& coordinates() const { return _coordinates; }
        void setCoordinates(const std::vector<double>& coordinates) { 
            if (hascoordinates()) throw prg_error("Attempting to redefine location coordinates.", "setCoordinates()");
            _coordinates.reset(new Coordinates(coordinates));
        }
};

/* Defines operator() to compare by Location names. */
class CompareLocationByName {
	public:
		bool operator() (const boost::shared_ptr<Location>& lhs, const boost::shared_ptr<Location>& rhs) {
			return lhs->name() < rhs->name();
		}
};

/* Defines operator() to compare by Location coordinates. */
class CompareLocationByCoordinates {
    public:
	    bool operator() (const boost::shared_ptr<Location>& lhs, const boost::shared_ptr<Location>& rhs) {
		    return *(lhs->coordinates()) < *(rhs->coordinates());
	    }
};

class IdentifiersManager; // forward class declaration
/* A class to oversee the addition and access to all locations. */
class LocationsManager {
	friend class IdentifiersManager;

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
            // Assigns the Locations relative position to it's class member - for easy determine later.
            for (auto itr = _locations.begin(); itr != _locations.end(); ++itr)
                itr->get()->setindex(std::distance(_locations.begin(), itr));
        }
        boost::optional<boost::shared_ptr<Location> > getLocationForCoordinates(const std::vector<double>& coordinates) const;
        bool getCoordinatesExist(const std::vector<double>& coordinates) const;
		unsigned int expectedDimensions() const { return _expected_dimensions; }
		void setExpectedDimensions(unsigned int i) {
			/* if (_locations.size()) throw prg_error("Unable to set coordinate expected dimensions - locations already exist", "setExpectedDimensions()"); */
			_expected_dimensions = i;
		}
		const LocationContainer_t& locations() const { return _locations; }
        const LocationContainer_t& locationsByCoordinates() const { return _locations_by_coordinates; }
        bool locationExists(const std::string& locationame) const {
			auto itr = std::lower_bound(_locations.begin(), _locations.end(), boost::shared_ptr<Location>(new Location(locationame)), CompareLocationByName());
			return itr != _locations.end() && itr->get()->name() == locationame;
		}
		LocationIdx_t getLocation(const std::string& locationame) const;
        short getMaxNameLength() const { return _max_identifier_length; }
};

/* Analytic identifier and it's associated locations. */
class Identifier {
	friend class IdentifiersManager;

	public:
		typedef MinimalGrowthArray<const Location*> LocationsSet_t;
		typedef MinimalGrowthArray<std::string> CombinedIdentifierNames_t;

	protected:
		std::string _name;
		LocationsSet_t _locations;
		CombinedIdentifierNames_t _combined_with;

		Identifier(const std::string& name) : _name(name) {}

	public:
		Identifier(const std::string& name, const Location& location) : _name(name) {
			if (!_locations.exists(&location)) _locations.add(&location, true);
		}
		~Identifier() {}

		bool operator==(const Identifier& rhs) const {
			return _name == rhs._name;
		}

		void addLocation(const Location& location) { 
            _locations.add(&location, false);
            auto comp = [&](const Location * m, const Location * n)-> bool {
                return m->name() < n->name();
            };
            _locations.sort(comp);
        }
		const LocationsSet_t& getLocations() { return _locations; }
		const std::string& name() const { return _name; }
		const CombinedIdentifierNames_t& getCombinedWith() const { return _combined_with; }
		void combinedWith(const std::string& other) { if (!_combined_with.exists(other)) _combined_with.add(other, true); }
		CombinedIdentifierNames_t& retrieveAll(CombinedIdentifierNames_t& Identifiers) const;
};

/* Defines operator() to compare by Identifier names. */
class CompareIdenitifers {
	public:
		bool operator() (const boost::shared_ptr<Identifier>& lhs, const boost::shared_ptr<Identifier>& rhs) {
			return lhs->name() < rhs->name();
		}
};

/* Defines operator() to compare by Identifier Location names. */
class CompareIdenitifersByLocation {
	public:
		bool operator() (const boost::shared_ptr<Identifier> lhs, const boost::shared_ptr<Identifier> rhs) {
			if (lhs->getLocations().size() != rhs->getLocations().size())
				return lhs->getLocations().size() < rhs->getLocations().size();
			for (unsigned int i=0; i < lhs->getLocations().size(); ++i) {
				if (lhs->getLocations()[i]->name() != rhs->getLocations()[i]->name())
					return lhs->getLocations()[i]->name() < rhs->getLocations()[i]->name();
			}
			return lhs->name() < rhs->name();
		}
};

/* Manages Identifier objects and their coordinates. */
class IdentifiersManager {
	public:
		typedef std::vector<boost::shared_ptr<Identifier>> Identifiers_t;
		typedef ptr_vector<Coordinates> CoordinatesContainer_t;
        typedef std::map<tract_t, std::map<tract_t, double> > LocationOverrides_t;
        enum AddStatus { Accepted = 0, UnknownLocation, MultipleLocations };
		enum WriteStatus { Accepting = 0, Closed };

	protected:
		WriteStatus _write_status;
		bool _aggregating_identifiers;
		MultipleCoordinatesType _multiple_coordinates_type;
		Identifiers_t _identifiers;
		LocationsManager _locations_manager;
		std::map<std::string, std::string> aggregated_identifiers;

		size_t                              _num_location_coordinates;
		MetaIdentifierManager               _meta_identifiers_manager;
		MetaNeighborManager                 _meta_neighbor_manager;
		std::auto_ptr<MetaManagerProxy>     _meta_manager_proxy;
		LocationOverrides_t                 _location_distance_overrides;

	public:
		IdentifiersManager(bool aggregating, MultipleCoordinatesType multiple_coordinates_type);
		virtual ~IdentifiersManager() {}

        bool                                aggregatingIdentifiers() const { return _aggregating_identifiers; }
        IdentifiersManager::AddStatus       addIdentifier(const std::string& identifierName, const std::string& locationame);
        void                                additionsCompleted(bool bReportingRiskEstimates=false);
		LocationsManager::AddStatus         addLocation(const std::string& locationname);
		LocationsManager::AddStatus         addLocation(const std::string& locationname, const std::vector<double>& coordinates);
        bool                                addLocationsDistanceOverride(tract_t t1, tract_t t2, double distance);
		void                                assignExplicitCoordinates(CoordinatesContainer_t& coordinates);
        const std::map<std::string, std::string>& getAggregated() const { return aggregated_identifiers; }
        std::string                       & getIdentifierNameAtIndex(tract_t tIndex, std::string& name) const;
        bool                                getLocationsDistanceOverridesExist() const { return _location_distance_overrides.size() != 0; }
		std::pair<bool, double>             getLocationsDistanceOverride(tract_t t1, tract_t t2) const;
		boost::optional<size_t>             getIdentifierIndex(const std::string& identifiername) const;
        const Identifiers_t               & getIdentifiers() const { return _identifiers; }
        const LocationsManager            & getLocationsManager() const { return _locations_manager; }
		size_t                              getNumLocationCoordinates() const { return _num_location_coordinates; }
        const WriteStatus                   getWriteStatus() const { return _write_status; }
        void                                print(FILE* pFile) const;
        void                                reportCombinedIdentifiers(FILE * fDisplay) const;
        Identifier::CombinedIdentifierNames_t & retrieveAll(size_t tIndex, Identifier::CombinedIdentifierNames_t& Identifiers) const;
        void                                setExpectedCoordinateDimensions(unsigned int expected_dimension) {
			if (_aggregating_identifiers) return;
			_locations_manager.setExpectedDimensions(expected_dimension);
		}
        LocationsManager::AddStatus         setLocationCoordinates(const std::string& locationname, const std::vector<double>& coordinates);

		MetaIdentifierManager             & getMetaIdentifiersManager() { return _meta_identifiers_manager; }
		const MetaIdentifierManager       & getMetaIdentifiersManager() const { return _meta_identifiers_manager; }
		const MetaNeighborManager         & getMetaNeighborManager() const { return _meta_neighbor_manager; }
		MetaNeighborManager               & getMetaNeighborManager() { return _meta_neighbor_manager; }
		const MetaManagerProxy            & getMetaManagerProxy() const { return *_meta_manager_proxy; }
};

//*****************************************************************************
#endif
