//*****************************************************************************
#ifndef __TRACTS_H
#define __TRACTS_H
//*****************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "Parameters.h"

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

        bool            operator<(const Coordinates& rhs) const;
        bool            operator!=(const Coordinates& rhs) const;
        bool            operator==(const Coordinates& rhs) const;
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
        typedef std::vector<const Coordinates*> CoordsContainer_t;
        typedef std::deque<std::string>        StringContainer_t;

       private:
         std::string                    gsIndentifier;
         CoordsContainer_t              gvCoordinatesContainer;
         StringContainer_t              gvSecondaryIdentifiers;

       public:
         Location(const std::string& sIdentifier, const Coordinates& aCoordinates);

         void                           addCoordinates(const Coordinates& aCoordinates, MultipleCoordinatesType eMultipleCoordinatesType);
         void                           addSecondaryIdentifier(const std::string & sIdentifier);
         StringContainer_t            & retrieveAllIdentifiers(StringContainer_t& Identifiers) const;
         const CoordsContainer_t      & getCoordinatesContainer() const {return gvCoordinatesContainer;}
         const std::string            & getIndentifier() const {return gsIndentifier;}
         const StringContainer_t      & getSecondaryIdentifiers() const {return gvSecondaryIdentifiers;}
    };
    /** Function object used to compare LocationIdentifier::gsIndentifier. */
    class CompareIdentifiers {
       public:
         bool operator() (const Location * lhs, const Location * rhs) {
            return (strcmp(lhs->getIndentifier().c_str(), rhs->getIndentifier().c_str()) < 0);
         }
    };

    typedef ZdPointerVector<Coordinates> CoordinatesContainer_t;
    typedef ZdPointerVector<Location>    LocationsContainer_t;

  private:
    /** Function object used to compare LocationIdentifier::gvCoordinatesContainer[0]. */
    class CompareFirstCoordinatePointer {
       public:
         bool operator() (const Location * lhs, const Location * rhs) {
            return lhs->getCoordinatesContainer()[0] < rhs->getCoordinatesContainer()[0];
         }
    };

    enum addition_status_t              {Accepting=0, Closed};
    addition_status_t                   gAdditionStatus;
    CoordinatesContainer_t              gvCoordinates;
    LocationsContainer_t                gvLocations;
    size_t                              giCoordinateDimensions;
    std::map<std::string,tract_t>       gmAggregateTracts;
    bool                                gbAggregatingTracts;
    MultipleCoordinatesType             geMultipleCoordinatesType;
    size_t                              giMaxIdentifierLength;

  public:
    TractHandler(bool bAggregatingTracts, MultipleCoordinatesType eMultipleCoordinatesType);
    ~TractHandler() {}

    void                                additionsCompleted();
    tract_t                             addLocation(const char *sIdentifier);
    void                                addLocation(const char *sIdentifier, std::vector<double>& vCoordinates);
    const CoordinatesContainer_t      & getCoordinates() const {return gvCoordinates;}
    int                                 getCoordinateDimensions() const {return giCoordinateDimensions;}
    static double                       getDistanceSquared(const std::vector<double>& vFirstPoint, const std::vector<double>& vSecondPoint);
    const LocationsContainer_t        & getLocations() const {return gvLocations;}
    tract_t                             getLocationIndex(const char *sIdentifier) const;
    size_t                              getMaxIdentifierLength() const {return giMaxIdentifierLength;}
    void                                reportCombinedLocations(FILE * fDisplay) const;
    void                                setCoordinateDimensions(size_t iDimensions);
};
//*****************************************************************************
#endif
