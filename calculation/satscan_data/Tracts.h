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
    //specialized templated C array class -- used instead of standard template library containers
    //Reasoning: Standard library containers do not provide a public means for which to define growth,
    //           but instead commonly grow in chunks (Borland ~256b). In the context of the TractHandler
    //           class, we really want certain arrays to growth in an 'as needed' manner; otherwise
    //           we could be wasting considerable amounts of memory.
    template <class T>
    class MinimalGrowthArray {
      private:
        T             * gpArray;
        unsigned int    giSize;

      public:
        MinimalGrowthArray() : giSize(0), gpArray(0) {}
        ~MinimalGrowthArray() {try {delete[] gpArray;}catch(...){}}

        const T       & operator[](const unsigned int i) const {return gpArray[i];}
        bool            operator!=(const MinimalGrowthArray<T> & rhs) const {
                          if (giSize != rhs.giSize) return true;
                          for (unsigned int i=0; i < giSize; ++i) if (gpArray[i]!=rhs.gpArray[i]) return true;
                          return false;
                        }
        void            add(const T& x, bool bSort) {
                          T * p = new T[giSize + 1];
                          for (unsigned int i=0; i < giSize; ++i) p[i] = gpArray[i];
                          std::swap(p, gpArray); delete[] p;
                          gpArray[giSize] = x; ++giSize; if (bSort) std::sort(gpArray, gpArray + giSize);
                        }
        void            clear() {delete[] gpArray; gpArray=0; giSize=0;}
        bool            exists(const T& x) const {
                          for (unsigned int i=0; i < giSize; ++i) if (gpArray[i] == x) return true;
                          return false;
                        }
        unsigned int    size() const {return giSize;}
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

    typedef ZdPointerVector<Coordinates> CoordinatesContainer_t;
    typedef ZdPointerVector<Location>    LocationsContainer_t;

  private:
    /** Function object used to compare LocationIdentifier::gvCoordinatesContainer[0]. */
    class CompareFirstCoordinatePointer {
       public:
         bool operator() (const Location * lhs, const Location * rhs) {
            return lhs->getCoordinates()[0] < rhs->getCoordinates()[0];
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
    size_t                              giNumLocationCoordinates;

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
    size_t                              getNumLocationCoordinates() const {return giNumLocationCoordinates;}
    void                                reportCombinedLocations(FILE * fDisplay) const;
    void                                setCoordinateDimensions(size_t iDimensions);
};
//*****************************************************************************
#endif
