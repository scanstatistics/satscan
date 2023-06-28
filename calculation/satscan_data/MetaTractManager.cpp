//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MetaTractManager.h"
#include "Tracts.h"
#include "SSException.h"
#include "SaTScanData.h"
#include<boost/tokenizer.hpp>

////////////////// AbstractMetaObsGroup////////////////////

/** constructor */
AbstractMetaObsGroup::AbstractMetaObsGroup(const char * sIdentifier) {
  gsIndentifier = new char[strlen(sIdentifier) + 1];
  strcpy(gsIndentifier, sIdentifier);
}

/** destructor */
AbstractMetaObsGroup::~AbstractMetaObsGroup() {
  try {delete[] gsIndentifier;} catch(...){}
}

////////////////// AtomicMetaObsGroup////////////////////

/** constructor */
AtomicMetaObsGroup::AtomicMetaObsGroup(const char * sIdentifier)
                    :AbstractMetaObsGroup(sIdentifier), _obs_group_index(0), gpPromotedMetaObsGroup(0) {}

/** Returns indication of whether this metaobs-group contains 'tAtomicIndex'. */
bool AtomicMetaObsGroup::contains(tract_t tAtomicIndex) const {
  if (gpPromotedMetaObsGroup) return gpPromotedMetaObsGroup->contains(tAtomicIndex);
  return tAtomicIndex == _obs_group_index;
}

/** Returns indication of whether this meta obs-group contains 'pMetaLocation'. */
bool AtomicMetaObsGroup::contains(const MetaObsGroup& object) const {
  if (gpPromotedMetaObsGroup) return gpPromotedMetaObsGroup->contains(object);
  return false;
}

/** Retrieves atomic indexes contained, in sequential order. */
void AtomicMetaObsGroup::getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const {
  if (gpPromotedMetaObsGroup) gpPromotedMetaObsGroup->getAtomicIndexes(AtomicIndexes);
  else AtomicIndexes.push_back(_obs_group_index);
}

/** Returns indication of whether passed observation group object intersect geographically with this object. */
bool AtomicMetaObsGroup::intersects(const AbstractMetaObsGroup& object) const {
  if (gpPromotedMetaObsGroup) return gpPromotedMetaObsGroup->intersects(object);
  return object.contains(_obs_group_index);
}

////////////////// MetaObsGroup ///////////////////////////

/** constructor */
MetaObsGroup::MetaObsGroup(const char * sIdentifier) : AbstractMetaObsGroup(sIdentifier) {}

/** Adds atomic object to this objects collection of AbstractMetaObsGroups. */
void MetaObsGroup::addMetaObsGroup(const AtomicMetaObsGroup * object) {
  _meta_obs_groups.add(object, false);
}

/** Adds meta obs-group object to this objects collection of AbstractMetaObsGroup locations.
    Throw resolvable_error if 'this' object is contained is passed object. */
void MetaObsGroup::addMetaObsGroup(const MetaObsGroup * object) {
  if (object->contains(*this)) throw resolvable_error("Error: Circular definition between meta locations '%s' and '%s'.", getIndentifier(), object->getIndentifier());
  _meta_obs_groups.add(object, false);
}

/** Returns indication of whether this object contains 'tAtomicIndex'. */
bool MetaObsGroup::contains(tract_t tAtomicIndex) const {
  for (unsigned int t=0; t < _meta_obs_groups.size(); ++t)
     if (_meta_obs_groups[t]->contains(tAtomicIndex)) return true;
  return false;
}

/** Returns indication of whether this meta obs-group contains 'pMetaLocation'. */
bool MetaObsGroup::contains(const MetaObsGroup& object) const {
  if (this == &object) return true;
  for (unsigned int t=0; t < _meta_obs_groups.size(); ++t)
     if (_meta_obs_groups[t]->contains(object)) return true;
  return false;
}

/** Retrieves atomic indexes of contained groups, in sequential order. */
void MetaObsGroup::getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const {
  for (unsigned int t=0; t < _meta_obs_groups.size(); ++t)
     _meta_obs_groups[t]->getAtomicIndexes(AtomicIndexes);
}

/** Returns indication of whether passed object intersect geographically with this object. */
bool MetaObsGroup::intersects(const AbstractMetaObsGroup& object) const {
  for (unsigned int t=0; t < _meta_obs_groups.size(); ++t)
     if (_meta_obs_groups[t]->intersects(object)) return true;
  return false;   
}

/////////////////////////// MetaObsGroupPool ///////////////////////////////////

/** Adds meta obs-group and obs-groups that define the the meta obs-group. */
bool MetaObsGroupManager::MetaObsGroupPool::addMetaObsGroup(const std::string& sMetaIdentifier, const std::string& neighborsCSV) {
  assert(_addition_status == Accepting);

  if (sMetaIdentifier.size() == 0) return false;
  std::auto_ptr<MetaObsGroup> pMetaObsGroup(new MetaObsGroup(sMetaIdentifier.c_str()));
  tract_t tIndex;
  if ((tIndex = getAtomicObsGroupIndex(sMetaIdentifier)) != -1) {
    AtomicMetaObsGroup * object = _atomic_obs_groups[tIndex];
    object->setAsPromotedMetaObsGroup(pMetaObsGroup.get());
    //move(not delete) this AtomicMetaObsGroup into the promoted container
    _atomic_obs_groups.erase(_atomic_obs_groups.begin() + tIndex);
    _promoted_atomic_obs_groups.push_back(object);
  }

  boost::tokenizer<boost::escaped_list_separator<char> > identifiers(neighborsCSV);
  for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=identifiers.begin(); itr != identifiers.end(); ++itr) {
     std::string token = (*itr);
     trimString(token);
     if (token.size() == 0) return false;
     if (token == sMetaIdentifier)
       throw resolvable_error("Error: Meta location ID '%s' defines itself as a member.", sMetaIdentifier.c_str());
     else if ((tIndex = getMetaIndex(token)) != -1)
         pMetaObsGroup->addMetaObsGroup(_meta_obs_groups[tIndex]);
     else {
       //assume for now that token is referencing an atomic observation group
       std::auto_ptr<AtomicMetaObsGroup> atomic(new AtomicMetaObsGroup(token.c_str()));
       AtomicObsGroupContainer_t::iterator itr=std::lower_bound(_atomic_obs_groups.begin(), _atomic_obs_groups.end(), atomic.get(), compareIdentifiers());
       if (itr == _atomic_obs_groups.end() || strcmp((*itr)->getIndentifier(), token.c_str()))
         itr = _atomic_obs_groups.insert(itr, atomic.release());
       pMetaObsGroup->addMetaObsGroup(*itr);
     }
  }

  MetaObsGroupContainer_t::iterator itr=std::lower_bound(_meta_obs_groups.begin(), _meta_obs_groups.end(), pMetaObsGroup.get(), compareIdentifiers());
  if (itr != _meta_obs_groups.end() && !strcmp((*itr)->getIndentifier(), sMetaIdentifier.c_str())) {
    if (*(*itr) == *pMetaObsGroup) return true; // duplicate record
    throw resolvable_error("Error: Meta location ID '%s' is defined multiple times.", sMetaIdentifier.c_str());
  }
  _meta_obs_groups.insert(itr, pMetaObsGroup.release());
  return true;
}

/** Closes object to further additions of meta location definitions. Adds all accumulated atomic locations to ObservationGroupingManager object. */
void MetaObsGroupManager::MetaObsGroupPool::additionsCompleted(ObservationGroupingManager& groupManager) {
	assert(_addition_status == Accepting);
	_addition_status = Closed;
	groupManager.setExpectedCoordinateDimensions(0);
    for (auto agr : _atomic_obs_groups)
        groupManager.addLocation(agr->getIndentifier());
}

/** Assigns atomic indexes to each atomic obs-group, as gotten from ObservationGroupingManager object. */
void MetaObsGroupManager::MetaObsGroupPool::assignAtomicIndexes(ObservationGroupingManager& groupManager) {
	assert(_addition_status == Closed);
	assert(groupManager.getWriteStatus() == ObservationGroupingManager::Closed);
	for (auto agr: _atomic_obs_groups) {
		auto groupIdx = groupManager.getObservationGroupIndex(agr->getIndentifier());
		if (groupIdx) agr->setObGroupIndex(*groupIdx);
	}
}

/** Returns index of location in internal collection atomic locations. Returns negative one if not found. */
tract_t MetaObsGroupManager::MetaObsGroupPool::getAtomicObsGroupIndex(const std::string& sIdentifier) const {
  std::auto_ptr<AtomicMetaObsGroup> search(new AtomicMetaObsGroup(sIdentifier.c_str()));
  AtomicObsGroupContainer_t::const_iterator itr=std::lower_bound(_atomic_obs_groups.begin(), _atomic_obs_groups.end(), search.get(), compareIdentifiers());
  if (itr != _atomic_obs_groups.end() && !strcmp((*itr)->getIndentifier(), sIdentifier.c_str()))
    return std::distance(_atomic_obs_groups.begin(), itr);
  else
    return -1;
}

/** Returns index of obs-group in internal collection meta obs-group. Returns negative one if not found. */
tract_t MetaObsGroupManager::MetaObsGroupPool::getMetaIndex(const std::string& sMetaIdentifier) const {
  std::auto_ptr<MetaObsGroup> search(new MetaObsGroup(sMetaIdentifier.c_str()));
  MetaObsGroupContainer_t::const_iterator itr=std::lower_bound(_meta_obs_groups.begin(), _meta_obs_groups.end(), search.get(), compareIdentifiers());
  if (itr != _meta_obs_groups.end() && !strcmp((*itr)->getIndentifier(), sMetaIdentifier.c_str()))
    return std::distance(_meta_obs_groups.begin(), itr);
  else
    return -1;
}

/** Prints defined meta obs-group to file stream. */
void MetaObsGroupManager::MetaObsGroupPool::print(const ObservationGroupingManager& groupManager, FILE * stream) const {
	FILE * fp = 0;
	if (!stream) {
		if ((fp = fopen("MetaLocationPool.print", "w")) == NULL) return;
		stream = fp;
	}

	//print meta locations, top level only 
	for (auto gr: _meta_obs_groups) {
		fprintf(stream, "%s", gr->getIndentifier());
		for (unsigned int t = 0; t < gr->getMetaObsGroups().size(); ++t)
			fprintf(stream, "%s%s", (t == 0 ? "=" : ","), gr->getMetaObsGroups()[t]->getIndentifier());
		fprintf(stream, "\n");
	}
	//print meta locations, broken down into respective atomic indexes 
	fprintf(stream, "\n");
	std::vector<tract_t> AtomicIndexes;
	for (auto gr: _meta_obs_groups) {
		fprintf(stream, "%s", gr->getIndentifier());
		AtomicIndexes.clear();
		gr->getAtomicIndexes(AtomicIndexes);
		for (unsigned int t = 0; t < AtomicIndexes.size(); ++t)
			fprintf(stream, "%s%s(index=%d)", (t == 0 ? "=" : ","), groupManager.getObservationGroups().at(AtomicIndexes.at(t))->groupname().c_str(), AtomicIndexes.at(t));
		fprintf(stream, "\n");
	}
	if (fp) fclose(fp);
}

/////////////////////////// MetaObsGroupManager ////////////////////////////////

/** Adds meta observation group at index collection that are directly referenced in the neighbors file definition. */
unsigned int MetaObsGroupManager::addReferenced(unsigned int metaIdx) {
  assert(_state == accepting);
  const MetaObsGroup* object = _meta_obs_group_pool.getMetaObsGroups().at(metaIdx);
  MetaObsGroupRefContainer_t::iterator itr=std::lower_bound(_meta_obs_groups.begin(), _meta_obs_groups.end(), object, compareIdentifiers());
  if (itr == _meta_obs_groups.end() || strcmp((*itr)->getIndentifier(), object->getIndentifier()))
    itr = _meta_obs_groups.insert(itr, object);
  _referenced_meta_obs_groups = _meta_obs_groups.size();
  return std::distance(_meta_obs_groups.begin(), itr);
}

/** Retrieves atomic indexes as defined by meta obs-group at 'metaIdx'. */
std::vector<tract_t> & MetaObsGroupManager::getAtomicIndexes(unsigned int metaIdx, std::vector<tract_t>& AtomicIndexes) const {
  AtomicIndexes.clear();
  _meta_obs_groups.at(metaIdx)->getAtomicIndexes(AtomicIndexes);
  return AtomicIndexes;
}

/** Returns index of obs-group in internal collection meta obs-group. Returns negative one if not found. */
tract_t MetaObsGroupManager::getMetaIndex(const std::string& sMetaIdentifier) const {
  std::auto_ptr<MetaObsGroup> search(new MetaObsGroup(sMetaIdentifier.c_str()));
  MetaObsGroupRefContainer_t::const_iterator itr=std::lower_bound(_meta_obs_groups.begin(), _meta_obs_groups.begin() + _referenced_meta_obs_groups, search.get(), compareIdentifiers());
  if (itr != _meta_obs_groups.end() && !strcmp((*itr)->getIndentifier(), sMetaIdentifier.c_str()))
    return std::distance(_meta_obs_groups.begin(), itr);
  else
    return -1;
}

/** Returns reference to meta obs-group pool. */
MetaObsGroupManager::MetaObsGroupPool & MetaObsGroupManager::getMetaPool() {
  assert(_state == accepting);
  return _meta_obs_group_pool;
}

/** Returns indication of whether 'obsGroupIdx' is contained in meta obs-group at 'metaIdx'. */
bool MetaObsGroupManager::intersectsObsGroup(unsigned int metaIdx, tract_t obsGroupIdx) const {
  return _meta_obs_groups.at(metaIdx)->contains(obsGroupIdx);
}

/** Returns indication of whether two meta obs-groups intersect geographically. */
bool MetaObsGroupManager::intersects(unsigned int left, unsigned int right) const {
  return left == right || _meta_obs_groups.at(left)->intersects(*_meta_obs_groups.at(right));
}

/** Closes manager to further updates and appends remaining not referenced meta obs-groups
    to list of referenced meta obs-groups (if 'bIncludePoolRemainders' is true). Appending not
    referenced meta obs-groups is needed when user requests the risk estimates file; otherwise
    only meta obs-groups referenced in the neighbors file will be reported in that optional file. 
    Note that the number of referenced meta obs-groups is stored in class variable '_referenced_meta_obs_groups'.*/
void MetaObsGroupManager::setStateFixed(bool bIncludePoolRemainders) {
    assert(_state == accepting);
    if (bIncludePoolRemainders) {
        for (auto gr: _meta_obs_group_pool.getMetaObsGroups()) {
            MetaObsGroupRefContainer_t::iterator itrPos = std::lower_bound(_meta_obs_groups.begin(), _meta_obs_groups.begin() + _referenced_meta_obs_groups, gr, compareIdentifiers());
            if (itrPos == _meta_obs_groups.end() || strcmp((*itrPos)->getIndentifier(), gr->getIndentifier()))
                _meta_obs_groups.push_back(gr);
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

/** Returns first obs-group index defined for meta obs-group at index 'iCollectionIndex'. */
tract_t MetaNeighborManager::getFirst(unsigned int iCollectionIndex) const {
  return gvUnifiedIndexesCollection.at(iCollectionIndex)->operator[](0);
}

/** Returns obs-group indexes defined for meta obs-group at index 'iCollectionIndex'. */
std::vector<tract_t> & MetaNeighborManager::getIndexes(unsigned int iCollectionIndex, std::vector<tract_t>& v) const {
   MinimalGrowthArray<tract_t>& indexes = *gvUnifiedIndexesCollection.at(iCollectionIndex);
   v.resize(indexes.size());
   std::copy(&indexes[0], &indexes[0] + indexes.size(), v.begin());
   return v;
}

/** Returns whether obs-group 'obsGroupIdx' intersects with any obs-group defined by meta location at 'metaIdx'. */
bool MetaNeighborManager::intersectsObsGroup(unsigned int metaIdx, tract_t obsGroupIdx) const {
  const MinimalGrowthArray<tract_t>&  a = *gvUnifiedIndexesCollection.at(metaIdx);
  for (unsigned int t=0; t < a.size(); ++t)
     if (obsGroupIdx == a[t]) return true;
  return false;
}

/** Returns whether any obs-group defined by meta obs-group at 'left' intersects with any obs-group defined by meta obs-group 'right'. */
bool MetaNeighborManager::intersects(unsigned int left, unsigned int right) const {
  const MinimalGrowthArray<tract_t>&  a = *gvUnifiedIndexesCollection.at(left);
  for (unsigned int t=0; t < a.size(); ++t)
     if (intersectsObsGroup(right, a[t])) return true;
  return false;
}

///////////////////////////// MetaManagerProxy /////////////////////////////////

/** constructor */
MetaManagerProxy::MetaManagerProxy(const MetaObsGroupManager& metaObsGroupsManager, const MetaNeighborManager& NeighborManager)
                 :gpMetaObsGroupManager(&metaObsGroupsManager), gpMetaNeighborManager(&NeighborManager) {}

/** Returns obs-group indexes defined by meta location at 'metaIdx'. */
std::vector<tract_t> & MetaManagerProxy::getIndexes(unsigned int metaIdx, std::vector<tract_t>& Indexes) const {
  if (metaIdx < gpMetaObsGroupManager->getMetaObsGroups().size())
    gpMetaObsGroupManager->getAtomicIndexes(metaIdx, Indexes);
  else
    gpMetaNeighborManager->getIndexes(metaIdx, Indexes);
  return Indexes;
}

/** Returns number of defined meta observation groups. */
unsigned int MetaManagerProxy::getNumMeta() const {
  return gpMetaObsGroupManager->getNumReferenced() + gpMetaNeighborManager->size();
}

/** Returns whether 'obsGroupIdx' intersects with any obs-group defined by meta obs-group at 'metaIdx'. */
bool MetaManagerProxy::intersectsObsGroup(unsigned int metaIdx, tract_t obsGroupIdx) const {
  if (metaIdx < gpMetaObsGroupManager->getNumReferenced())
    return gpMetaObsGroupManager->intersectsObsGroup(metaIdx, obsGroupIdx);
  else
    return gpMetaNeighborManager->intersectsObsGroup(metaIdx, obsGroupIdx);
}

/** Returns whether any tracts defined by meta obs-group at 'left' intersects with any tracts defined by meta obs-group 'right'. */
bool MetaManagerProxy::intersects(unsigned int left, unsigned int right) const {
  if (left < gpMetaObsGroupManager->getNumReferenced())
    return gpMetaObsGroupManager->intersects(left, right);
  else
    return gpMetaNeighborManager->intersects(left, right);
}

