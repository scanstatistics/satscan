//******************************************************************************
#ifndef MetaTractManagerH
#define MetaTractManagerH
//******************************************************************************
#include "SaTScan.h"
#include "MultipleDimensionArrayHandler.h"
#include "ptr_vector.h"

class TractHandler; // forward declaration
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
                                      if (!strcmp(rhs.getIndentifier(), getIndentifier()) && !(gLocations != rhs.gLocations)) return true;
                                      return false;
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

/** Maintains meta locations, specifically in the context of meta locations as
    defined by meta neighbors input file. */
class MetaLocationManager {
  public:
    typedef std::vector<const MetaLocation*> MetaLocationsRefContainer_t;
    enum state_t                        {accepting=0, closed};

  private:
    /** Maintains collection of meta location definitions. */
    class MetaLocationPool {
      public:
        typedef ptr_vector<MetaLocation>            MetaLocationsContainer_t;
        typedef ptr_vector<AtomicMetaLocation>      AtomicLocationsContainer_t;
        enum addition_status_t                      {Accepting=0, Closed};

      private:
        addition_status_t                   gAdditionStatus;
        MetaLocationsContainer_t            gvMetaLocations;
        AtomicLocationsContainer_t          gvAtomicLocations;
        AtomicLocationsContainer_t          gvPromotedAtomicLocations;

        tract_t                             getAtomicLocationIndex(const std::string& sIdentifier) const;

      public:
        MetaLocationPool() : gAdditionStatus(Accepting) {}
        ~MetaLocationPool() {}

        void                                additionsCompleted(TractHandler& TInfo);
        bool                                addMetaLocation(const std::string& sMetaIdentifier, const std::string& sLocationIndentifiers);
        void                                assignAtomicIndexes(TractHandler& TInfo);
        const MetaLocationsContainer_t    & getLocations() const {return gvMetaLocations;}
        tract_t                             getMetaLocationIndex(const std::string& sMetaIdentifier) const;
        void                                print(TractHandler& TInfo, FILE * stream=stdout) const;
    };

    MetaLocationPool                    gMetaLocationPool;
    MetaLocationsRefContainer_t         gMetaLocations;
    state_t                             geState;
    unsigned int                        giReferencedMetaLocations;

  public:
    MetaLocationManager() : geState(accepting), giReferencedMetaLocations(0) {}
    ~MetaLocationManager() {}

    unsigned int                        addReferenced(unsigned int tMetaLocation);
    std::vector<tract_t>              & getAtomicIndexes(unsigned int tMetaLocation, std::vector<tract_t>& AtomicIndexes) const;
    const MetaLocationsRefContainer_t & getLocations() const {return gMetaLocations;}
    tract_t                             getMetaLocationIndex(const std::string& sMetaIdentifier) const;
    MetaLocationPool                  & getMetaLocationPool();
    const MetaLocationPool            & getMetaLocationPool() const {return gMetaLocationPool;}
    unsigned int                        getNumReferencedLocations() const {return giReferencedMetaLocations;}
    bool                                intersectsTract(unsigned int tMetaLocation, tract_t tTractIndex) const;
    bool                                intersects(unsigned int tMetaLocationL, unsigned int tMetaLocationR) const;
    void                                setStateFixed(bool bIncludePoolRemainders);
};

/** Maintains collection of meta locations, as would be defined during the calculation of neighbors
    about centroids. In particular, this class is used when the option for multiple coordinates is
    not one location per location id (e.g. multiple coordinates per location id). */
class MetaNeighborManager {
  public:
    typedef ptr_vector<MinimalGrowthArray<tract_t> > UnifiedCollection_t;

  private:
    UnifiedCollection_t                 gvUnifiedIndexesCollection;
    bool                                equal(const std::vector<tract_t>& v, const MinimalGrowthArray<tract_t>& a) const;

  public:
    unsigned int                        add(std::vector<tract_t>& unified);
    void                                empty() {gvUnifiedIndexesCollection.killAll();}
    tract_t                             getFirst(unsigned int iCollectionIndex) const;
    std::vector<tract_t>              & getIndexes(unsigned int iCollectionIndex, std::vector<tract_t>& v) const;
    bool                                intersectsTract(unsigned int tMetaLocation, tract_t tTractIndex) const;
    bool                                intersects(unsigned int tMetaLocationL, unsigned int tMetaLocationR) const;
    unsigned int                        size() const {return gvUnifiedIndexesCollection.size();}
};

class CSaTScanData; /** forward class declaration for class MetaManagerProxy */

/** Proxy class for the MetaLocationManager and MetaNeighborManager classes; defining an interface
    that directs access to underlying meta managers.
    It should be noted that in the grand scheme of the project, using both the MetaLocationManager
    and MetaNeighborManager simultaneously is not intended and using both in the contact of this class
    might have unintended results. The MetaLocationManager class is used with the meta/neighbors input
    options; were there are not coordinates. The MetaNeighborManager class is used to assist with
    multiple coordinates per location id. */
class MetaManagerProxy {
  private:
    const MetaLocationManager         * gpMetaLocationManager;
    const MetaNeighborManager         * gpMetaNeighborManager;

  public:
     MetaManagerProxy(const MetaLocationManager& LocationManager, const MetaNeighborManager& NeighborManager);

     std::vector<tract_t>             & getIndexes(unsigned int tMetaLocation, std::vector<tract_t>& Indexes) const;
     unsigned int                       getNumMetaLocations() const;
    bool                                intersectsTract(unsigned int tMetaLocation, tract_t tTractIndex) const;
    bool                                intersects(unsigned int tMetaLocationL, unsigned int tMetaLocationR) const;
};
//******************************************************************************
#endif

