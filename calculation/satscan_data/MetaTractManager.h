//******************************************************************************
#ifndef MetaTractManagerH
#define MetaTractManagerH
//******************************************************************************
#include "SaTScan.h"
#include "MultipleDimensionArrayHandler.h"
#include "ptr_vector.h"
#include <cstring>

class ObservationGroupingManager; // forward declaration
class MetaObsGroup; // forward declaration

/** abtract meta observation group */
class AbstractMetaObsGroup {
  private:
    char                          * gsIndentifier;

  public:
     AbstractMetaObsGroup(const char * sIdentifier);
     virtual ~AbstractMetaObsGroup();
     virtual bool                   contains(const MetaObsGroup& object) const = 0;
     virtual bool                   contains(tract_t tAtomicIndex) const = 0;
     virtual bool                   intersects(const AbstractMetaObsGroup& object) const = 0;
     virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const = 0;
     const char                   * getIndentifier() const {return gsIndentifier;}
};
/** atomic meta observation group, representing a particular observation group in data */
class AtomicMetaObsGroup : public AbstractMetaObsGroup {
  private:
    tract_t                        _obs_group_index;
    const MetaObsGroup           * gpPromotedMetaObsGroup;

  public:
     AtomicMetaObsGroup(const char * sIdentifier);

     virtual bool                   contains(const MetaObsGroup& object) const;
     virtual bool                   contains(tract_t tAtomicIndex) const;
     virtual bool                   intersects(const AbstractMetaObsGroup& object) const;
     virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const;
     tract_t                        getObsGroupIndex() const {return _obs_group_index;}
     void                           setAsPromotedMetaObsGroup(const MetaObsGroup * pMetaObsGroup) {gpPromotedMetaObsGroup = pMetaObsGroup;}
     void                           setObGroupIndex(tract_t idx) {_obs_group_index = idx;}
};
/** meta observation group, representing a collection of atomic and meta observation group */
class MetaObsGroup : public AbstractMetaObsGroup {
  public:
    typedef MinimalGrowthArray<const AbstractMetaObsGroup*> MetaObsGroups_t;

  private:
    MetaObsGroups_t                     _meta_obs_groups;

  public:
     MetaObsGroup(const char * sIdentifier);

     bool                           operator==(const MetaObsGroup& rhs) const {
                                      if (!strcmp(rhs.getIndentifier(), getIndentifier()) && !(_meta_obs_groups != rhs._meta_obs_groups)) return true;
                                      return false;
                                    }
     void                           addMetaObsGroup(const AtomicMetaObsGroup* object);
     void                           addMetaObsGroup(const MetaObsGroup* object);
     virtual bool                   contains(const MetaObsGroup& object) const;
     virtual bool                   contains(tract_t tAtomicIndex) const;
     virtual bool                   intersects(const AbstractMetaObsGroup& object) const;
     virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const;
     const MetaObsGroups_t        & getMetaObsGroups() const {return _meta_obs_groups;}
};
/** Function object used to compare MetaObsGroup::gsIndentifier. */
class compareIdentifiers {
   public:
     bool operator() (const AbstractMetaObsGroup* lhs, const AbstractMetaObsGroup* rhs) {
        return strcmp(lhs->getIndentifier(), rhs->getIndentifier()) < 0;
     }
};

/** Maintains meta observation groups - specifically in the context of meta observation groups as defined by meta neighbors input file. */
class MetaObsGroupManager {
  public:
    typedef std::vector<const MetaObsGroup*> MetaObsGroupRefContainer_t;
    enum state_t {accepting=0, closed};

  private:
    /** Maintains collection of meta observation group definitions. */
    class MetaObsGroupPool {
      public:
        typedef ptr_vector<MetaObsGroup> MetaObsGroupContainer_t;
        typedef ptr_vector<AtomicMetaObsGroup> AtomicObsGroupContainer_t;
        enum addition_status_t {Accepting=0, Closed};

      private:
        addition_status_t                  _addition_status;
        MetaObsGroupContainer_t            _meta_obs_groups;
        AtomicObsGroupContainer_t          _atomic_obs_groups;
        AtomicObsGroupContainer_t          _promoted_atomic_obs_groups;

        tract_t getAtomicObsGroupIndex(const std::string& sIdentifier) const;

      public:
        MetaObsGroupPool() : _addition_status(Accepting) {}
        ~MetaObsGroupPool() {}

		void                                additionsCompleted(ObservationGroupingManager& groupManager);
		bool                                addMetaObsGroup(const std::string& sMetaIdentifier, const std::string& neighborsCSV);
		void                                assignAtomicIndexes(ObservationGroupingManager& groupManager);
		const MetaObsGroupContainer_t     & getMetaObsGroups() const {return _meta_obs_groups;}
        tract_t                             getMetaIndex(const std::string& sMetaIdentifier) const;
		void                                print(const ObservationGroupingManager& groupManager, FILE * stream = stdout) const;
	};

    MetaObsGroupPool                    _meta_obs_group_pool;
    MetaObsGroupRefContainer_t          _meta_obs_groups;
    state_t                             _state;
    unsigned int                        _referenced_meta_obs_groups;

  public:
    MetaObsGroupManager() : _state(accepting), _referenced_meta_obs_groups(0) {}
    ~MetaObsGroupManager() {}

    unsigned int                        addReferenced(unsigned int metaIdx);
    std::vector<tract_t>              & getAtomicIndexes(unsigned int metaIdx, std::vector<tract_t>& AtomicIndexes) const;
    const MetaObsGroupRefContainer_t & getMetaObsGroups() const {return _meta_obs_groups;}
    tract_t                             getMetaIndex(const std::string& sMetaIdentifier) const;
    MetaObsGroupPool                  & getMetaPool();
    const MetaObsGroupPool            & getMetaPool() const {return _meta_obs_group_pool;}
    unsigned int                        getNumReferenced() const {return _referenced_meta_obs_groups;}
    bool                                intersectsObsGroup(unsigned int metaIdx, tract_t obsGroupIdx) const;
    bool                                intersects(unsigned int left, unsigned int right) const;
    void                                setStateFixed(bool bIncludePoolRemainders);
};

/** Maintains collection of meta observation groups as would be defined during the calculation of neighbors about centroids. 
    In particular this class is used with multiple coordinates to ensure that observation groups at the same location
    enter the cluster as the same time. */
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
    bool                                intersectsObsGroup(unsigned int metaIdx, tract_t obsGroupIdx) const;
    bool                                intersects(unsigned int left, unsigned int right) const;
    unsigned int                        size() const {return gvUnifiedIndexesCollection.size();}
};

class CSaTScanData; /** forward class declaration for class MetaManagerProxy */

/** Proxy class for the MetaObsGroupManager and MetaNeighborManager classes; defining an interface that directs access to underlying meta managers.
    It should be noted that in the grand scheme of the project, using both the MetaObsGroupManager and MetaNeighborManager simultaneously is not
    intended and using both in the contact of this class might have unintended results. */
class MetaManagerProxy {
  private:
    const MetaObsGroupManager * gpMetaObsGroupManager;
    const MetaNeighborManager * gpMetaNeighborManager;

  public:
     MetaManagerProxy(const MetaObsGroupManager& metaObsGroupsManager, const MetaNeighborManager& NeighborManager);

     std::vector<tract_t>             & getIndexes(unsigned int metaIdx, std::vector<tract_t>& Indexes) const;
     unsigned int                       getNumMeta() const;
     bool                               intersectsObsGroup(unsigned int metaIdx, tract_t obsGroupIdx) const;
     bool                               intersects(unsigned int left, unsigned int right) const;
};
//******************************************************************************
#endif

