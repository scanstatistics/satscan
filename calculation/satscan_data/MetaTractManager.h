//******************************************************************************
#ifndef MetaTractManagerH
#define MetaTractManagerH
//******************************************************************************
#include "SaTScan.h"
#include "MultipleDimensionArrayHandler.h"

class TractHandler; // forward declaration

/** Manages meta location definitions. */
class MetaLocationManager {
  private:
    class MetaLocation; // forward declaration
    /** abtract meta location */
    class AbstractMetaLocation {
      private:
        char                          * gsIndentifier;

      public:
         AbstractMetaLocation(const char * sIdentifier);
         virtual ~AbstractMetaLocation();
         virtual bool                   contains(const MetaLocation& pMetaLocation) const = 0;
         virtual bool                   contains(tract_t tAtomicIndex) const = 0;
         virtual bool                   intersects(const AbstractMetaLocation& pLocation) const = 0;
         virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const = 0;
         const char                   * getIndentifier() const {return gsIndentifier;}
    };
    /** atomic meta location, representing a particular location/tract */
    class AtomicMetaLocation : public AbstractMetaLocation {
      private:
        tract_t                        giTractIndex;
        const MetaLocation           * gpPromotedMetaLocation;

      public:
         AtomicMetaLocation(const char * sIdentifier);

         virtual bool                   contains(const MetaLocation& pMetaLocation) const;
         virtual bool                   contains(tract_t tAtomicIndex) const;
         virtual bool                   intersects(const AbstractMetaLocation& pLocation) const;
         virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const;
         tract_t                        getTractIndex() const {return giTractIndex;}
         void                           setAsPromotedMetaLocation(const MetaLocation * pMetaLocation) {gpPromotedMetaLocation = pMetaLocation;}
         void                           setTractIndex(tract_t iTractIndex) {giTractIndex = iTractIndex;}
    };
    /** meta location, representing a collection of atomic and meta locations */
    class MetaLocation : public AbstractMetaLocation {
      public:
        typedef MinimalGrowthArray<const AbstractMetaLocation*> Locations_t;

      private:
        Locations_t                     gLocations;

      public:
         MetaLocation(const char * sIdentifier);

         bool                           operator==(const MetaLocation& rhs) const {
                                          if (!strcmp(rhs.getIndentifier(), getIndentifier())) return true;
                                          return !(gLocations != rhs.gLocations);
                                        }
         void                           addLocation(const AtomicMetaLocation* pLocation);
         void                           addLocation(const MetaLocation* pLocation);
         virtual bool                   contains(const MetaLocation& pMetaLocation) const;
         virtual bool                   contains(tract_t tAtomicIndex) const;
         virtual bool                   intersects(const AbstractMetaLocation& pLocation) const;
         virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const;
         const Locations_t            & getLocations() const {return gLocations;}
    };
    /** Function object used to compare MetaLocation::gsIndentifier. */
    class compareIdentifiers {
       public:
         bool operator() (const AbstractMetaLocation* lhs, const AbstractMetaLocation* rhs) {
            return strcmp(lhs->getIndentifier(), rhs->getIndentifier()) < 0;
         }
    };

    typedef ZdPointerVector<MetaLocation>       MetaLocationsContainer_t;
    typedef ZdPointerVector<AtomicMetaLocation> AtomicLocationsContainer_t;
    enum addition_status_t                      {Accepting=0, Closed};

    addition_status_t                   gAdditionStatus;
    MetaLocationsContainer_t            gvMetaLocations;
    AtomicLocationsContainer_t          gvAtomicLocations;
    AtomicLocationsContainer_t          gvPromotedAtomicLocations;

    tract_t                             getAtomicLocationIndex(const std::string& sIdentifier) const;

  public:
    MetaLocationManager() : gAdditionStatus(Accepting) {}
    ~MetaLocationManager() {}

    void                                additionsCompleted(TractHandler& TInfo);
    bool                                addMetaLocation(const std::string& sMetaIdentifier, const std::string& sLocationIndentifiers);
    void                                assignAtomicIndexes(TractHandler& TInfo);
    std::vector<tract_t>              & getAtomicIndexes(unsigned int tMetaLocation, std::vector<tract_t>& AtomicIndexes) const;
    const MetaLocationsContainer_t    & getLocations() const {return gvMetaLocations;}
    tract_t                             getMetaLocationIndex(const std::string& sMetaIdentifier) const;
    bool                                intersectsTract(unsigned int tMetaLocation, tract_t tTractIndex) const;
    bool                                intersects(unsigned int tMetaLocationL, unsigned int tMetaLocationR) const;
    void                                print(TractHandler& TInfo, FILE * stream=stdout) const;
};
//******************************************************************************
#endif
