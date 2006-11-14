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
        Coordinates(const std::vector<double>& Coordinates, unsigned int iInsertionOrdinal);
        ~Coordinates();

        bool            operator<(const Coordinates& rhs) const {
                           if (giSize != rhs.giSize) return giSize < rhs.giSize;
                           size_t t=0;
                           while (t < giSize) {
                             if (gpCoordinates[t] == rhs.gpCoordinates[t]) ++t;
                             else return gpCoordinates[t] < rhs.gpCoordinates[t];
                           }
                           return false;
                        }
        bool            operator!=(const Coordinates& rhs) const {
                           if (giSize != rhs.giSize) return true;
                           return memcmp(gpCoordinates, rhs.gpCoordinates, giSize * sizeof(double));
                        }
        bool            operator==(const Coordinates& rhs) const {
                           if (giSize != rhs.giSize) return false;
                           return !memcmp(gpCoordinates, rhs.gpCoordinates, giSize * sizeof(double));
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

       public:
         Location(const char * sIdentifier, const Coordinates& aCoordinates);
         ~Location() {try {delete[] gsIndentifier;}catch(...){}}

         void                           addCoordinates(const Coordinates& aCoordinates, MultipleCoordinatesType eMultipleCoordinatesType);
         void                           addSecondaryIdentifier(const std::string & sIdentifier);
         StringContainer_t            & retrieveAllIdentifiers(StringContainer_t& Identifiers) const;
         const CoordsContainer_t      & getCoordinates() const {return gvCoordinatesContainer;}
         const char                   * getIndentifier() const {return gsIndentifier;}
         const StringContainer_t      & getSecondaryIdentifiers() const {return gvSecondaryIdentifiers;}
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
    enum addition_status_t               {Accepting=0, Closed};

  private:
    /** Function object used to compare LocationIdentifier::gvCoordinatesContainer[0]. */
    class CompareFirstCoordinatePointer {
       public:
         bool operator() (const Location * lhs, const Location * rhs) {
            return lhs->getCoordinates()[0] < rhs->getCoordinates()[0];
         }
    };

    addition_status_t                   gAdditionStatus;
    CoordinatesContainer_t              gvCoordinates;
    LocationsContainer_t                gvLocations;
    size_t                              giCoordinateDimensions;
    std::map<std::string,tract_t>       gmAggregateTracts;
    bool                                gbAggregatingTracts;
    MultipleCoordinatesType             geMultipleCoordinatesType;
    size_t                              giMaxIdentifierLength;
    size_t                              giNumLocationCoordinates;
    MetaLocationManager                 gMetaLocationsManager;

  public:
    TractHandler(bool bAggregatingTracts, MultipleCoordinatesType eMultipleCoordinatesType);
    ~TractHandler() {}

    void                                additionsCompleted(bool bReportingRiskEstimates=false);
    void                                addLocation(const char *sIdentifier);
    void                                addLocation(const char *sIdentifier, std::vector<double>& vCoordinates);
    addition_status_t                   getAddStatus() const {return gAdditionStatus;}
    const CoordinatesContainer_t      & getCoordinates() const {return gvCoordinates;}
    int                                 getCoordinateDimensions() const {return giCoordinateDimensions;}
    static double                       getDistanceSquared(const std::vector<double>& vFirstPoint, const std::vector<double>& vSecondPoint);
    const char                        * getIdentifier(tract_t tIndex) const;
    const LocationsContainer_t        & getLocations() const {return gvLocations;}
    tract_t                             getLocationIndex(const char *sIdentifier) const;
    size_t                              getMaxIdentifierLength() const {return giMaxIdentifierLength;}
    MetaLocationManager               & getMetaLocations() {return gMetaLocationsManager;}
    const MetaLocationManager         & getMetaLocations() const {return gMetaLocationsManager;}
    size_t                              getNumLocationCoordinates() const {return giNumLocationCoordinates;}
    void                                reportCombinedLocations(FILE * fDisplay) const;
    Location::StringContainer_t       & retrieveAllIdentifiers(tract_t tIndex, Location::StringContainer_t& Identifiers) const;
    void                                setCoordinateDimensions(size_t iDimensions);
};
//*****************************************************************************
#endif
