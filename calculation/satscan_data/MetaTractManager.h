//******************************************************************************
#ifndef MetaTractManagerH
#define MetaTractManagerH
//******************************************************************************
#include "SaTScan.h"
#include "MultipleDimensionArrayHandler.h"

class TractHandler;

class MetaLocationManager {
  private:
    class AbstractMetaLocation {
      private:
        char                          * gsIndentifier;
        
      public:
         AbstractMetaLocation(const char * sIdentifier);
         virtual ~AbstractMetaLocation();
         virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const = 0;
         const char                   * getIndentifier() const {return gsIndentifier;}
    };
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
         void                           addLocation(const AbstractMetaLocation* pLocation);
         virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const;
         const Locations_t            & getLocations() const {return gLocations;}
    };
    class AtomicMetaLocation : public AbstractMetaLocation {
      private:
        tract_t                        giTractIndex;
        const MetaLocation           * gpPromotedMetaLocation;

      public:
         AtomicMetaLocation(const char * sIdentifier);

         virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const;
         tract_t                        getTractIndex() const {return giTractIndex;}
         void                           setAsPromotedMetaLocation(const MetaLocation * pMetaLocation) {gpPromotedMetaLocation = pMetaLocation;}
         void                           setTractIndex(tract_t iTractIndex) {giTractIndex = iTractIndex;}
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

    TractHandler                      & gTInfo;
    addition_status_t                   gAdditionStatus;
    MetaLocationsContainer_t            gvMetaLocations;
    AtomicLocationsContainer_t          gvAtomicLocations;
    AtomicLocationsContainer_t          gvPromotedAtomicLocations;

    tract_t                             getAtomicLocationIndex(const std::string& sIdentifier) const;

  public:
    MetaLocationManager(TractHandler& TInfo) : gTInfo(TInfo), gAdditionStatus(Accepting) {}
    ~MetaLocationManager() {}

    void                                additionsCompleted();
    void                                addMetaLocation(const std::string& sMetaIdentifier, const std::string& sLocationIndentifiers);
    void                                generateAtomicIndexes();
    std::vector<tract_t>              & getAtomicIndexes(unsigned int tMetaLocation, std::vector<tract_t>& AtomicIndexes) const;
    const MetaLocationsContainer_t    & getLocations() const {return gvMetaLocations;}
    tract_t                             getMetaLocationIndex(const std::string& sMetaIdentifier) const;
    void                                print(FILE * stream=stdout) const;
};
//******************************************************************************
#endif
