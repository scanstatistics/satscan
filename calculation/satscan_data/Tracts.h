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

/**********************************************************************
 file: Tracts.h
 Header file for tinfo.c, which abstracts tract, population & case
 count information
 **********************************************************************/

class CSaTScanData;
class CCluster;

/** Manages locations and their coordinates. */
class TractHandler {
  public:
    /** class representing coordinates of location -- tracks insertion ordinal */
    class Coordinates {
      private:
        double        * gpCoordinates;
        unsigned int    giSize;
        unsigned int    giInsertionOrdinal;

      public:
        Coordinates();
        Coordinates(const Coordinates& rhs);
        Coordinates(const std::vector<double>& Coordinates, unsigned int iInsertionOrdinal);
        Coordinates(double x, double y, unsigned int iInsertionOrdinal=0);
        ~Coordinates();

        bool            operator<(const Coordinates& rhs) const;
        bool            operator!=(const Coordinates& rhs) const {
                           if (giSize != rhs.giSize) return true;
                           return memcmp(gpCoordinates, rhs.gpCoordinates, giSize * sizeof(double)) != 0;
                        }
        bool            operator==(const Coordinates& rhs) const {
                           if (giSize != rhs.giSize) return false;
                           return memcmp(gpCoordinates, rhs.gpCoordinates, giSize * sizeof(double)) == 0;
                        }
        double        * getCoordinates() const {return gpCoordinates;}
        unsigned int    getInsertionOrdinal() const {return giInsertionOrdinal;}
        unsigned int    getSize() const {return giSize;}
        void            retrieve(std::vector<double>& Repository) const;
    };
    /** Function object used to compare Coordinates::gpCoordinates */
    class CompareCoordinates {
       public:
         bool operator() (const Coordinates * plhs, const Coordinates * prhs) { return *plhs < *prhs; }
    };
    /** class representing location defined in coordinates file -- with functionality
        that permits association with mutliple coordinates and location identifiers. */
    class Location {
       public:
        typedef MinimalGrowthArray<const Coordinates*> CoordsContainer_t;
        typedef MinimalGrowthArray<std::string>        StringContainer_t;

       private:
         char                         * gsIndentifier;
         CoordsContainer_t              gvCoordinatesContainer;
         StringContainer_t              gvSecondaryIdentifiers;
         //bool                           gbEvaluated;

       public:
         Location(const char * sIdentifier, const Coordinates& aCoordinates);
         ~Location();

         void                           addCoordinates(const Coordinates& aCoordinates, MultipleCoordinatesType eMultipleCoordinatesType);
         void                           addSecondaryIdentifier(const std::string & sIdentifier);
         //bool                           isEvaluatedLocation() const {return gbEvaluated;}
         StringContainer_t            & retrieveAllIdentifiers(StringContainer_t& Identifiers) const;
         const CoordsContainer_t      & getCoordinates() const {return gvCoordinatesContainer;}
         const char                   * getIndentifier() const {return gsIndentifier;}
         const StringContainer_t      & getSecondaryIdentifiers() const {return gvSecondaryIdentifiers;}
         //void                           setEvaluatedLocation(bool b) {gbEvaluated = b;}
    };
    /** Function object used to compare LocationIdentifier::gsIndentifier. */
    class CompareIdentifiers {
       public:
         bool operator() (const Location * lhs, const Location * rhs) {
            return strcmp(lhs->getIndentifier(), rhs->getIndentifier()) < 0;
         }
    };

    typedef ptr_vector<Coordinates>      CoordinatesContainer_t;
    typedef ptr_vector<Location>         LocationsContainer_t;
	typedef std::map<tract_t, std::map<tract_t, double> > LocationOverrides_t;
    enum addition_status_t               {Accepting=0, Closed};

  private:
    /** Function object used to compare LocationIdentifier::gvCoordinatesContainer[0]. */
    class CompareFirstCoordinatePointer {
       public:
         bool operator() (const Location * lhs, const Location * rhs) {
            // First verify that location defines coordinates - this situation is possible with 
            if (lhs->getCoordinates().size() == 0 || rhs->getCoordinates().size() == 0)
                return strcmp(lhs->getIndentifier(), rhs->getIndentifier()) < 0;
            if (lhs->getCoordinates()[0] == rhs->getCoordinates()[0])
              return strcmp(lhs->getIndentifier(), rhs->getIndentifier()) < 0;
            return lhs->getCoordinates()[0] < rhs->getCoordinates()[0];
         }
    };

    addition_status_t                   gAdditionStatus;
    CoordinatesContainer_t              gvCoordinates;
    LocationsContainer_t                gvLocations;
    size_t                              giCoordinateDimensions;
    std::map<std::string,std::string>   gmAggregateTracts;
    bool                                gbAggregatingTracts;
    MultipleCoordinatesType             geMultipleCoordinatesType;
    size_t                              giMaxIdentifierLength;
    size_t                              giNumLocationCoordinates;
    MetaLocationManager                 gMetaLocationsManager;
    MetaNeighborManager                 gMetaNeighborManager;
    std::auto_ptr<MetaManagerProxy>     gMetaManagerProxy;
	LocationOverrides_t                 _location_distance_overrides;

  public:
    TractHandler(bool bAggregatingTracts, MultipleCoordinatesType eMultipleCoordinatesType);
    ~TractHandler() {}

    void                                additionsCompleted(bool bReportingRiskEstimates=false);
    size_t                              addLocation(const char *sIdentifier);
    void                                addLocation(const char *sIdentifier, std::vector<double>& vCoordinates, bool onlyIfExists=false);
	bool                                addLocationsDistanceOverride(tract_t t1, tract_t t2, double distance);
    void                                assignExplicitCoordinates(CoordinatesContainer_t& coordinates);
    addition_status_t                   getAddStatus() const {return gAdditionStatus;}
    const CoordinatesContainer_t      & getCoordinates() const {return gvCoordinates;}
    int                                 getCoordinateDimensions() const {return giCoordinateDimensions;}
    bool                                getCoordinatesExist(std::vector<double>& vCoordinates) const;
    static double                       getDistanceSquared(const std::vector<double>& vFirstPoint, const std::vector<double>& vSecondPoint);
    const char                        * getIdentifier(tract_t tIndex) const;
    const LocationsContainer_t        & getLocations() const {return gvLocations;}
    tract_t                             getLocationIndex(const char *sIdentifier) const;
	std::pair<bool, double>             getLocationsDistanceOverride(tract_t t1, tract_t t2) const;
	bool                                getLocationsDistanceOverridesExist() const { return _location_distance_overrides.size() != 0; }
	size_t                              getMaxIdentifierLength() const {return giMaxIdentifierLength;}
    MetaLocationManager               & getMetaLocations() {return gMetaLocationsManager;}
    const MetaLocationManager         & getMetaLocations() const {return gMetaLocationsManager;}
    const MetaNeighborManager         & getMetaNeighborManager() const {return gMetaNeighborManager;}
    MetaNeighborManager               & getMetaNeighborManager() {return gMetaNeighborManager;}
    const MetaManagerProxy            & getMetaManagerProxy() const {return *gMetaManagerProxy;}
    size_t                              getNumLocationCoordinates() const {return giNumLocationCoordinates;}
    void                                printLocations(FILE * pFile) const;
    void                                reportCombinedLocations(FILE * fDisplay) const;
    Location::StringContainer_t       & retrieveAllIdentifiers(tract_t tIndex, Location::StringContainer_t& Identifiers) const;
    void                                setCoordinateDimensions(size_t iDimensions);
    //void                                setLocationNotEvaluating(tract_t tIndex) {gvLocations.at(tIndex)->setEvaluatedLocation(false);}
};
//*****************************************************************************
#endif
