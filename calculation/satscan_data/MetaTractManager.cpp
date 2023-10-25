//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MetaTractManager.h"
#include "Tracts.h"
#include "SSException.h"
#include "SaTScanData.h"
#include<boost/tokenizer.hpp>

////////////////// AbstractMetaIdentifier////////////////////

/** constructor */
AbstractMetaIdentifier::AbstractMetaIdentifier(const char * sIdentifier) {
  gsIndentifier = new char[strlen(sIdentifier) + 1];
  strcpy(gsIndentifier, sIdentifier);
}

/** destructor */
AbstractMetaIdentifier::~AbstractMetaIdentifier() {
  try {delete[] gsIndentifier;} catch(...){}
}

////////////////// AtomicMetaIdentifier////////////////////

/** constructor */
AtomicMetaIdentifier::AtomicMetaIdentifier(const char * sIdentifier)
                    :AbstractMetaIdentifier(sIdentifier), _identifier_index(0), gpPromotedMetaIdentifier(0) {}

/** Returns indication of whether this meta identifier contains 'tAtomicIndex'. */
bool AtomicMetaIdentifier::contains(tract_t tAtomicIndex) const {
  if (gpPromotedMetaIdentifier) return gpPromotedMetaIdentifier->contains(tAtomicIndex);
  return tAtomicIndex == _identifier_index;
}

/** Returns indication of whether this meta identifier contains passed object. */
bool AtomicMetaIdentifier::contains(const MetaIdentifier& object) const {
  if (gpPromotedMetaIdentifier) return gpPromotedMetaIdentifier->contains(object);
  return false;
}

/** Retrieves atomic indexes contained, in sequential order. */
void AtomicMetaIdentifier::getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const {
  if (gpPromotedMetaIdentifier) gpPromotedMetaIdentifier->getAtomicIndexes(AtomicIndexes);
  else AtomicIndexes.push_back(_identifier_index);
}

/** Returns indication of whether passed identifier object intersect geographically with this object. */
bool AtomicMetaIdentifier::intersects(const AbstractMetaIdentifier& object) const {
  if (gpPromotedMetaIdentifier) return gpPromotedMetaIdentifier->intersects(object);
  return object.contains(_identifier_index);
}

////////////////// MetaIdentifier ///////////////////////////

/** constructor */
MetaIdentifier::MetaIdentifier(const char * sIdentifier) : AbstractMetaIdentifier(sIdentifier) {}

/** Adds atomic object to this objects collection of AbstractMetaIdentifiers. */
void MetaIdentifier::addMetaIdentifier(const AtomicMetaIdentifier * object) {
  _meta_identifiers.add(object, false);
}

/** Adds meta identifier object to this objects collection of AbstractMetaIdentifier locations.
    Throw resolvable_error if 'this' object is contained is passed object. */
void MetaIdentifier::addMetaIdentifier(const MetaIdentifier * object) {
  if (object->contains(*this)) throw resolvable_error("Error: Circular definition between meta locations '%s' and '%s'.", getIndentifier(), object->getIndentifier());
  _meta_identifiers.add(object, false);
}

/** Returns indication of whether this object contains 'tAtomicIndex'. */
bool MetaIdentifier::contains(tract_t tAtomicIndex) const {
  for (unsigned int t=0; t < _meta_identifiers.size(); ++t)
     if (_meta_identifiers[t]->contains(tAtomicIndex)) return true;
  return false;
}

/** Returns indication of whether this meta identifier contains passed object. */
bool MetaIdentifier::contains(const MetaIdentifier& object) const {
  if (this == &object) return true;
  for (unsigned int t=0; t < _meta_identifiers.size(); ++t)
     if (_meta_identifiers[t]->contains(object)) return true;
  return false;
}

/** Retrieves atomic indexes of contained identifiers, in sequential order. */
void MetaIdentifier::getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const {
  for (unsigned int t=0; t < _meta_identifiers.size(); ++t)
     _meta_identifiers[t]->getAtomicIndexes(AtomicIndexes);
}

/** Returns indication of whether passed object intersect geographically with this object. */
bool MetaIdentifier::intersects(const AbstractMetaIdentifier& object) const {
  for (unsigned int t=0; t < _meta_identifiers.size(); ++t)
     if (_meta_identifiers[t]->intersects(object)) return true;
  return false;   
}

/////////////////////////// MetaIdentifierPool ///////////////////////////////////

/** Adds meta identifier and identifiers of passed record. */
bool MetaIdentifierManager::MetaIdentifierPool::addMetaIdentifier(const std::string& sMetaIdentifier, const std::string& neighborsCSV) {
  assert(_addition_status == Accepting);

  if (sMetaIdentifier.size() == 0) return false;
  std::auto_ptr<MetaIdentifier> pMetaIdentifier(new MetaIdentifier(sMetaIdentifier.c_str()));
  tract_t tIndex;
  if ((tIndex = getAtomicidentifierIndex(sMetaIdentifier)) != -1) {
    AtomicMetaIdentifier * object = _atomic_identifiers[tIndex];
    object->setAsPromotedMetaIdentifier(pMetaIdentifier.get());
    //move(not delete) this AtomicMetaIdentifier into the promoted container
    _atomic_identifiers.erase(_atomic_identifiers.begin() + tIndex);
    _promoted_atomic_identifiers.push_back(object);
  }

  boost::tokenizer<boost::escaped_list_separator<char> > identifiers(neighborsCSV);
  for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=identifiers.begin(); itr != identifiers.end(); ++itr) {
     std::string token = (*itr);
     trimString(token);
     if (token.size() == 0) return false;
     if (token == sMetaIdentifier)
       throw resolvable_error("Error: Meta location ID '%s' defines itself as a member.", sMetaIdentifier.c_str());
     else if ((tIndex = getMetaIndex(token)) != -1)
         pMetaIdentifier->addMetaIdentifier(_meta_identifiers[tIndex]);
     else {
       //assume for now that token is referencing an atomic identifier
       std::auto_ptr<AtomicMetaIdentifier> atomic(new AtomicMetaIdentifier(token.c_str()));
       AtomicIdentifiersContainer_t::iterator itr=std::lower_bound(_atomic_identifiers.begin(), _atomic_identifiers.end(), atomic.get(), compareIdentifiers());
       if (itr == _atomic_identifiers.end() || strcmp((*itr)->getIndentifier(), token.c_str()))
         itr = _atomic_identifiers.insert(itr, atomic.release());
       pMetaIdentifier->addMetaIdentifier(*itr);
     }
  }

  MetaIdentifierContainer_t::iterator itr=std::lower_bound(_meta_identifiers.begin(), _meta_identifiers.end(), pMetaIdentifier.get(), compareIdentifiers());
  if (itr != _meta_identifiers.end() && !strcmp((*itr)->getIndentifier(), sMetaIdentifier.c_str())) {
    if (*(*itr) == *pMetaIdentifier) return true; // duplicate record
    throw resolvable_error("Error: Meta location ID '%s' is defined multiple times.", sMetaIdentifier.c_str());
  }
  _meta_identifiers.insert(itr, pMetaIdentifier.release());
  return true;
}

/** Closes object to further additions of meta identifiers. Adds all accumulated atomic identifiers to IdentifiersManager object. */
void MetaIdentifierManager::MetaIdentifierPool::additionsCompleted(IdentifiersManager& identifierMgr) {
	assert(_addition_status == Accepting);
	_addition_status = Closed;
    identifierMgr.setExpectedCoordinateDimensions(0);
    for (const auto agr : _atomic_identifiers)
        identifierMgr.addLocation(agr->getIndentifier());
}

/** Assigns atomic indexes to each atomic identifiers, as gotten from IdentifiersManager object. */
void MetaIdentifierManager::MetaIdentifierPool::assignAtomicIndexes(IdentifiersManager& identifierMgr) {
	assert(_addition_status == Closed);
	assert(identifierMgr.getWriteStatus() == IdentifiersManager::Closed);
	for (const auto agr: _atomic_identifiers) {
		auto identifierIdx = identifierMgr.getIdentifierIndex(agr->getIndentifier());
		if (identifierIdx) agr->setIdentifierIndex(*identifierIdx);
	}
}

/** Returns index of identifier in internal collection atomic identifiers. Returns negative one if not found. */
tract_t MetaIdentifierManager::MetaIdentifierPool::getAtomicidentifierIndex(const std::string& sIdentifier) const {
  std::auto_ptr<AtomicMetaIdentifier> search(new AtomicMetaIdentifier(sIdentifier.c_str()));
  AtomicIdentifiersContainer_t::const_iterator itr=std::lower_bound(_atomic_identifiers.begin(), _atomic_identifiers.end(), search.get(), compareIdentifiers());
  if (itr != _atomic_identifiers.end() && !strcmp((*itr)->getIndentifier(), sIdentifier.c_str()))
    return std::distance(_atomic_identifiers.begin(), itr);
  else
    return -1;
}

/** Returns index of identifier in internal collection meta identifiers. Returns negative one if not found. */
tract_t MetaIdentifierManager::MetaIdentifierPool::getMetaIndex(const std::string& sMetaIdentifier) const {
  std::auto_ptr<MetaIdentifier> search(new MetaIdentifier(sMetaIdentifier.c_str()));
  MetaIdentifierContainer_t::const_iterator itr=std::lower_bound(_meta_identifiers.begin(), _meta_identifiers.end(), search.get(), compareIdentifiers());
  if (itr != _meta_identifiers.end() && !strcmp((*itr)->getIndentifier(), sMetaIdentifier.c_str()))
    return std::distance(_meta_identifiers.begin(), itr);
  else
    return -1;
}

/** Prints defined meta identifiers to file stream. */
void MetaIdentifierManager::MetaIdentifierPool::print(const IdentifiersManager& identifierMgr, FILE * stream) const {
	FILE * fp = 0;
	if (!stream) {
		if ((fp = fopen("MetaLocationPool.print", "w")) == NULL) return;
		stream = fp;
	}

	//print meta locations, top level only 
	for (const auto gr: _meta_identifiers) {
		fprintf(stream, "%s", gr->getIndentifier());
		for (unsigned int t = 0; t < gr->getMetaIdentifiers().size(); ++t)
			fprintf(stream, "%s%s", (t == 0 ? "=" : ","), gr->getMetaIdentifiers()[t]->getIndentifier());
		fprintf(stream, "\n");
	}
	//print meta locations, broken down into respective atomic indexes 
	fprintf(stream, "\n");
	std::vector<tract_t> AtomicIndexes;
	for (const auto gr: _meta_identifiers) {
		fprintf(stream, "%s", gr->getIndentifier());
		AtomicIndexes.clear();
		gr->getAtomicIndexes(AtomicIndexes);
		for (unsigned int t = 0; t < AtomicIndexes.size(); ++t)
			fprintf(stream, "%s%s(index=%d)", (t == 0 ? "=" : ","), identifierMgr.getIdentifiers().at(AtomicIndexes.at(t))->name().c_str(), AtomicIndexes.at(t));
		fprintf(stream, "\n");
	}
	if (fp) fclose(fp);
}

/////////////////////////// MetaIdentifierManager ////////////////////////////////

/** Adds meta identifier at index collection that are directly referenced in the neighbors file definition. */
unsigned int MetaIdentifierManager::addReferenced(unsigned int metaIdx) {
  assert(_state == accepting);
  const MetaIdentifier* object = _meta_identifier_pool.getMetaIdentifiers().at(metaIdx);
  MetaIdentifierRefContainer_t::iterator itr=std::lower_bound(_meta_identifiers.begin(), _meta_identifiers.end(), object, compareIdentifiers());
  if (itr == _meta_identifiers.end() || strcmp((*itr)->getIndentifier(), object->getIndentifier()))
    itr = _meta_identifiers.insert(itr, object);
  _referenced_meta_identifiers = _meta_identifiers.size();
  return std::distance(_meta_identifiers.begin(), itr);
}

/** Retrieves atomic indexes as defined by meta identifier at 'metaIdx'. */
std::vector<tract_t> & MetaIdentifierManager::getAtomicIndexes(unsigned int metaIdx, std::vector<tract_t>& AtomicIndexes) const {
  AtomicIndexes.clear();
  _meta_identifiers.at(metaIdx)->getAtomicIndexes(AtomicIndexes);
  return AtomicIndexes;
}

/** Returns index of identifier in internal collection meta identifiers. Returns negative one if not found. */
tract_t MetaIdentifierManager::getMetaIndex(const std::string& sMetaIdentifier) const {
  std::auto_ptr<MetaIdentifier> search(new MetaIdentifier(sMetaIdentifier.c_str()));
  MetaIdentifierRefContainer_t::const_iterator itr=std::lower_bound(_meta_identifiers.begin(), _meta_identifiers.begin() + _referenced_meta_identifiers, search.get(), compareIdentifiers());
  if (itr != _meta_identifiers.end() && !strcmp((*itr)->getIndentifier(), sMetaIdentifier.c_str()))
    return std::distance(_meta_identifiers.begin(), itr);
  else
    return -1;
}

/** Returns reference to meta identifiers pool. */
MetaIdentifierManager::MetaIdentifierPool & MetaIdentifierManager::getMetaPool() {
  assert(_state == accepting);
  return _meta_identifier_pool;
}

/** Returns indication of whether 'identifierIdx' is contained in meta identifer at 'metaIdx'. */
bool MetaIdentifierManager::intersectsIdentifier(unsigned int metaIdx, tract_t identifierIdx) const {
  return _meta_identifiers.at(metaIdx)->contains(identifierIdx);
}

/** Returns indication of whether two meta identifers intersect geographically. */
bool MetaIdentifierManager::intersects(unsigned int left, unsigned int right) const {
  return left == right || _meta_identifiers.at(left)->intersects(*_meta_identifiers.at(right));
}

/** Closes manager to further updates and appends remaining not referenced meta identifers
    to list of referenced meta identifers (if 'bIncludePoolRemainders' is true). Appending not
    referenced meta identifers is needed when user requests the risk estimates file; otherwise
    only meta identifers referenced in the neighbors file will be reported in that optional file. 
    Note that the number of referenced meta identifers is stored in class variable '_referenced_meta_identifiers'.*/
void MetaIdentifierManager::setStateFixed(bool bIncludePoolRemainders) {
    assert(_state == accepting);
    if (bIncludePoolRemainders) {
        for (const auto gr: _meta_identifier_pool.getMetaIdentifiers()) {
            MetaIdentifierRefContainer_t::iterator itrPos = std::lower_bound(_meta_identifiers.begin(), _meta_identifiers.begin() + _referenced_meta_identifiers, gr, compareIdentifiers());
            if (itrPos == _meta_identifiers.end() || strcmp((*itrPos)->getIndentifier(), gr->getIndentifier()))
                _meta_identifiers.push_back(gr);
        }
    }
    _state = closed;
}

///////////////////////////// MetaNeighborManager //////////////////////////////

unsigned int MetaNeighborManager::add(std::vector<tract_t>& unified) {
  if (unified.size() == 0)
    throw prg_error("Can not add vector with size zero.","location_index_unifier::add()");
  //first sort vector
  std::sort(unified.begin(), unified.end());
  //now determine whether this collection has already been defined
  UnifiedCollection_t::iterator itr=gvUnifiedIndexesCollection.begin(), itr_end=gvUnifiedIndexesCollection.end();
  for (; itr != itr_end; ++itr) {
     if (equal(unified, **itr)) return std::distance(gvUnifiedIndexesCollection.begin(), itr);
  }
  gvUnifiedIndexesCollection.push_back(new MinimalGrowthArray<tract_t>(unified));
  return gvUnifiedIndexesCollection.size() - 1;
}

/** Returns whether passed vector and array contain idential values, in the same order. */
bool MetaNeighborManager::equal(const std::vector<tract_t>& v, const MinimalGrowthArray<tract_t>& a) const {
  if (v.size() != a.size()) return false;
  for (size_t t=0; t < v.size(); ++t)
     if (v[t] != a[t]) return false;
  return true;
}

/** Returns first identifer index defined for meta identifer at index 'iCollectionIndex'. */
tract_t MetaNeighborManager::getFirst(unsigned int iCollectionIndex) const {
  return gvUnifiedIndexesCollection.at(iCollectionIndex)->operator[](0);
}

/** Returns identifer indexes defined for meta identifer at index 'iCollectionIndex'. */
std::vector<tract_t> & MetaNeighborManager::getIndexes(unsigned int iCollectionIndex, std::vector<tract_t>& v) const {
   MinimalGrowthArray<tract_t>& indexes = *gvUnifiedIndexesCollection.at(iCollectionIndex);
   v.resize(indexes.size());
   std::copy(&indexes[0], &indexes[0] + indexes.size(), v.begin());
   return v;
}

/** Returns whether identifer 'identiferIdx' intersects with any identifer defined by meta location at 'metaIdx'. */
bool MetaNeighborManager::intersectsIdentifier(unsigned int metaIdx, tract_t identiferIdx) const {
  const MinimalGrowthArray<tract_t>&  a = *gvUnifiedIndexesCollection.at(metaIdx);
  for (unsigned int t=0; t < a.size(); ++t)
     if (identiferIdx == a[t]) return true;
  return false;
}

/** Returns whether any identifer defined by meta identifer at 'left' intersects with any identifer defined by meta identifer 'right'. */
bool MetaNeighborManager::intersects(unsigned int left, unsigned int right) const {
  const MinimalGrowthArray<tract_t>&  a = *gvUnifiedIndexesCollection.at(left);
  for (unsigned int t=0; t < a.size(); ++t)
     if (intersectsIdentifier(right, a[t])) return true;
  return false;
}

///////////////////////////// MetaManagerProxy /////////////////////////////////

/** constructor */
MetaManagerProxy::MetaManagerProxy(const MetaIdentifierManager& metaIdentifierMgr, const MetaNeighborManager& NeighboMgr)
                 :gpMetaIdentifierManager(&metaIdentifierMgr), gpMetaNeighborManager(&NeighboMgr) {}

/** Returns identifer indexes defined by meta location at 'metaIdx'. */
std::vector<tract_t> & MetaManagerProxy::getIndexes(unsigned int metaIdx, std::vector<tract_t>& Indexes) const {
  if (metaIdx < gpMetaIdentifierManager->getMetaIdentifiers().size())
    gpMetaIdentifierManager->getAtomicIndexes(metaIdx, Indexes);
  else
    gpMetaNeighborManager->getIndexes(metaIdx, Indexes);
  return Indexes;
}

/** Returns number of defined meta identifer. */
unsigned int MetaManagerProxy::getNumMeta() const {
  return gpMetaIdentifierManager->getNumReferenced() + gpMetaNeighborManager->size();
}

/** Returns whether 'identiferIdx' intersects with any identifer defined by meta identifer at 'metaIdx'. */
bool MetaManagerProxy::intersectsIdentifier(unsigned int metaIdx, tract_t identiferIdx) const {
  if (metaIdx < gpMetaIdentifierManager->getNumReferenced())
    return gpMetaIdentifierManager->intersectsIdentifier(metaIdx, identiferIdx);
  else
    return gpMetaNeighborManager->intersectsIdentifier(metaIdx, identiferIdx);
}

/** Returns whether any identifiers defined by meta identifer at 'left' intersects with any identifiers defined by meta identifer 'right'. */
bool MetaManagerProxy::intersects(unsigned int left, unsigned int right) const {
  if (left < gpMetaIdentifierManager->getNumReferenced())
    return gpMetaIdentifierManager->intersects(left, right);
  else
    return gpMetaNeighborManager->intersects(left, right);
}

