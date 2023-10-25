//******************************************************************************
#ifndef MetaTractManagerH
#define MetaTractManagerH
//******************************************************************************
#include "SaTScan.h"
#include "MultipleDimensionArrayHandler.h"
#include "ptr_vector.h"
#include <cstring>

class IdentifiersManager; // forward declaration
class MetaIdentifier; // forward declaration

/* Meta Identifiers are used in 2 contexts:
   The first is when the user has selected to define neighbors using the non-Euclidean neighbors file and also defines
   the meta locations file. It's arguably true that you can also refer to this situations as locations vs indentifiers,
   but since data structures are allocated in terms of idntifiers, I think indentifiers is preferred - also note that
   non-Euclidean neighbors is not implemented with any kind of multiple coordinates feature.
   The second is when using the multiple coordinates feature and we're defining the circles, ellipses and networks of
   which we'll scan for clusters. In this situation, we use meta identifiers to ensure that identifiers at the same 
   location will enter the cluster together as one.
*/

/** abtract meta identifier */
class AbstractMetaIdentifier {
  private:
    char                          * gsIndentifier;

  public:
     AbstractMetaIdentifier(const char * sIdentifier);
     virtual ~AbstractMetaIdentifier();
     virtual bool                   contains(const MetaIdentifier& object) const = 0;
     virtual bool                   contains(tract_t tAtomicIndex) const = 0;
     virtual bool                   intersects(const AbstractMetaIdentifier& object) const = 0;
     virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const = 0;
     const char                   * getIndentifier() const {return gsIndentifier;}
};
/** atomic meta identifier, representing a particular identifier in data */
class AtomicMetaIdentifier : public AbstractMetaIdentifier {
  private:
    tract_t                        _identifier_index;
    const MetaIdentifier         * gpPromotedMetaIdentifier;

  public:
     AtomicMetaIdentifier(const char * sIdentifier);

     virtual bool                   contains(const MetaIdentifier& object) const;
     virtual bool                   contains(tract_t tAtomicIndex) const;
     virtual bool                   intersects(const AbstractMetaIdentifier& object) const;
     virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const;
     tract_t                        getIdentifierIndex() const {return _identifier_index;}
     void                           setAsPromotedMetaIdentifier(const MetaIdentifier * object) {gpPromotedMetaIdentifier = object;}
     void                           setIdentifierIndex(tract_t idx) {_identifier_index = idx;}
};
/** meta identifier, representing a collection of atomic and meta identifiers */
class MetaIdentifier : public AbstractMetaIdentifier {
  public:
    typedef MinimalGrowthArray<const AbstractMetaIdentifier*> MetaIdentifiers_t;

  private:
    MetaIdentifiers_t               _meta_identifiers;

  public:
     MetaIdentifier(const char * sIdentifier);

     bool                           operator==(const MetaIdentifier& rhs) const {
                                      if (!strcmp(rhs.getIndentifier(), getIndentifier()) && !(_meta_identifiers != rhs._meta_identifiers)) return true;
                                      return false;
                                    }
     void                           addMetaIdentifier(const AtomicMetaIdentifier* object);
     void                           addMetaIdentifier(const MetaIdentifier* object);
     virtual bool                   contains(const MetaIdentifier& object) const;
     virtual bool                   contains(tract_t tAtomicIndex) const;
     virtual bool                   intersects(const AbstractMetaIdentifier& object) const;
     virtual void                   getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const;
     const MetaIdentifiers_t      & getMetaIdentifiers() const {return _meta_identifiers;}
};
/** Function object used to compare MetaIdentifier::gsIndentifier. */
class compareIdentifiers {
   public:
     bool operator() (const AbstractMetaIdentifier* lhs, const AbstractMetaIdentifier* rhs) {
        return strcmp(lhs->getIndentifier(), rhs->getIndentifier()) < 0;
     }
};

/** Maintains meta identifiers - specifically in the context of meta identifiers as defined by meta neighbors input file. */
class MetaIdentifierManager {
  public:
    typedef std::vector<const MetaIdentifier*> MetaIdentifierRefContainer_t;
    enum state_t {accepting=0, closed};

  private:
    /** Maintains collection of meta identifier definitions. */
    class MetaIdentifierPool {
      public:
        typedef ptr_vector<MetaIdentifier> MetaIdentifierContainer_t;
        typedef ptr_vector<AtomicMetaIdentifier> AtomicIdentifiersContainer_t;
        enum addition_status_t {Accepting=0, Closed};

      private:
        addition_status_t                   _addition_status;
        MetaIdentifierContainer_t           _meta_identifiers;
        AtomicIdentifiersContainer_t        _atomic_identifiers;
        AtomicIdentifiersContainer_t        _promoted_atomic_identifiers;

        tract_t getAtomicidentifierIndex(const std::string& sIdentifier) const;

      public:
        MetaIdentifierPool() : _addition_status(Accepting) {}
        ~MetaIdentifierPool() {}

		void                                additionsCompleted(IdentifiersManager& identifierMgr);
		bool                                addMetaIdentifier(const std::string& sMetaIdentifier, const std::string& neighborsCSV);
		void                                assignAtomicIndexes(IdentifiersManager& identifierMgr);
		const MetaIdentifierContainer_t   & getMetaIdentifiers() const {return _meta_identifiers;}
        tract_t                             getMetaIndex(const std::string& sMetaIdentifier) const;
		void                                print(const IdentifiersManager& identifierMgr, FILE * stream = stdout) const;
	};

    MetaIdentifierPool                    _meta_identifier_pool;
    MetaIdentifierRefContainer_t          _meta_identifiers;
    state_t                               _state;
    unsigned int                          _referenced_meta_identifiers;

  public:
    MetaIdentifierManager() : _state(accepting), _referenced_meta_identifiers(0) {}
    ~MetaIdentifierManager() {}

    unsigned int                        addReferenced(unsigned int metaIdx);
    std::vector<tract_t>              & getAtomicIndexes(unsigned int metaIdx, std::vector<tract_t>& AtomicIndexes) const;
    const MetaIdentifierRefContainer_t & getMetaIdentifiers() const {return _meta_identifiers;}
    tract_t                             getMetaIndex(const std::string& sMetaIdentifier) const;
    MetaIdentifierPool                  & getMetaPool();
    const MetaIdentifierPool            & getMetaPool() const {return _meta_identifier_pool;}
    unsigned int                        getNumReferenced() const {return _referenced_meta_identifiers;}
    bool                                intersectsIdentifier(unsigned int metaIdx, tract_t identifierIdx) const;
    bool                                intersects(unsigned int left, unsigned int right) const;
    void                                setStateFixed(bool bIncludePoolRemainders);
};

/** Maintains collection of meta identifiers as would be defined during the calculation of neighbors about centroids. 
    In particular this class is used with multiple coordinates to ensure that identifiers at the same location enter the cluster as the same time. */
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
    bool                                intersectsIdentifier(unsigned int metaIdx, tract_t identifierIdx) const;
    bool                                intersects(unsigned int left, unsigned int right) const;
    unsigned int                        size() const {return gvUnifiedIndexesCollection.size();}
};

class CSaTScanData; /** forward class declaration for class MetaManagerProxy */

/** Proxy class for the MetaIdentifierManager and MetaNeighborManager classes; defining an interface that directs access to underlying meta managers.
    It should be noted that in the grand scheme of the project, using both the MetaIdentifierManager and MetaNeighborManager simultaneously is not
    intended and using both in the contact of this class might have unintended results. */
class MetaManagerProxy {
  private:
    const MetaIdentifierManager * gpMetaIdentifierManager;
    const MetaNeighborManager * gpMetaNeighborManager;

  public:
     MetaManagerProxy(const MetaIdentifierManager& metaIdentifierMgr, const MetaNeighborManager& NeighborMgr);

     std::vector<tract_t>             & getIndexes(unsigned int metaIdx, std::vector<tract_t>& Indexes) const;
     unsigned int                       getNumMeta() const;
     bool                               intersectsIdentifier(unsigned int metaIdx, tract_t identifierIdx) const;
     bool                               intersects(unsigned int left, unsigned int right) const;
};
//******************************************************************************
#endif

