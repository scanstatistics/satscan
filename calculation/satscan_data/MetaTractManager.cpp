//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MetaTractManager.h"
#include "Tracts.h"
#include "SSException.h"
#include "SaTScanData.h"
#include<boost/tokenizer.hpp>

////////////////// AbstractMetaLocation////////////////////

/** constructor */
AbstractMetaLocation::AbstractMetaLocation(const char * sIdentifier) {
  gsIndentifier = new char[strlen(sIdentifier) + 1];
  strcpy(gsIndentifier, sIdentifier);
}

/** destructor */
AbstractMetaLocation::~AbstractMetaLocation() {
  try {delete[] gsIndentifier;} catch(...){}
}

////////////////// AtomicMetaLocation////////////////////

/** constructor */
AtomicMetaLocation::AtomicMetaLocation(const char * sIdentifier)
                    :AbstractMetaLocation(sIdentifier), giTractIndex(0), gpPromotedMetaLocation(0) {}

/** Returns indication of whether this location contains 'tAtomicIndex'. */
bool AtomicMetaLocation::contains(tract_t tAtomicIndex) const {
  if (gpPromotedMetaLocation) return gpPromotedMetaLocation->contains(tAtomicIndex);
  return tAtomicIndex == giTractIndex;
}

/** Returns indication of whether this location contains 'pMetaLocation'. */
bool AtomicMetaLocation::contains(const MetaLocation& pMetaLocation) const {
  if (gpPromotedMetaLocation) return gpPromotedMetaLocation->contains(pMetaLocation);
  return false;
}

/** Retrieves atomic indexes of contained locations, in sequential order. */
void AtomicMetaLocation::getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const {
  if (gpPromotedMetaLocation) gpPromotedMetaLocation->getAtomicIndexes(AtomicIndexes);
  else AtomicIndexes.push_back(giTractIndex);
}

/** Returns indication of whether passed location object intersect geographically with this object. */
bool AtomicMetaLocation::intersects(const AbstractMetaLocation& pLocation) const {
  if (gpPromotedMetaLocation) return gpPromotedMetaLocation->intersects(pLocation);
  return pLocation.contains(giTractIndex);
}

////////////////// MetaLocation ///////////////////////////

/** constructor */
MetaLocation::MetaLocation(const char * sIdentifier) : AbstractMetaLocation(sIdentifier) {}

/** Adds atomic location object to this objects collection of AbstractMetaLocation locations. */
void MetaLocation::addLocation(const AtomicMetaLocation * pLocation) {
  gLocations.add(pLocation, false);
}

/** Adds meta location object to this objects collection of AbstractMetaLocation locations.
    Throw resolvable_error if 'this' object is contained is passed object. */
void MetaLocation::addLocation(const MetaLocation * pLocation) {
  if (pLocation->contains(*this))
    throw resolvable_error("Error: Circular definition between meta locations '%s' and '%s'.",
                           getIndentifier(), pLocation->getIndentifier());
  gLocations.add(pLocation, false);
}

/** Returns indication of whether this location contains 'tAtomicIndex'. */
bool MetaLocation::contains(tract_t tAtomicIndex) const {
  for (unsigned int t=0; t < gLocations.size(); ++t)
     if (gLocations[t]->contains(tAtomicIndex)) return true;
  return false;
}

/** Returns indication of whether this location contains 'pMetaLocation'. */
bool MetaLocation::contains(const MetaLocation& pMetaLocation) const {
  if (this == &pMetaLocation) return true;
  for (unsigned int t=0; t < gLocations.size(); ++t)
     if (gLocations[t]->contains(pMetaLocation)) return true;
  return false;
}

/** Retrieves atomic indexes of contained locations, in sequential order. */
void MetaLocation::getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const {
  for (unsigned int t=0; t < gLocations.size(); ++t)
     gLocations[t]->getAtomicIndexes(AtomicIndexes);
}

/** Returns indication of whether passed location object intersect geographically with this object. */
bool MetaLocation::intersects(const AbstractMetaLocation& pLocation) const {
  for (unsigned int t=0; t < gLocations.size(); ++t)
     if (gLocations[t]->intersects(pLocation)) return true;
  return false;   
}

/////////////////////////// MetaLocationPool ///////////////////////////////////

/** Adds meta-location with */
bool MetaLocationManager::MetaLocationPool::addMetaLocation(const std::string& sMetaIdentifier, const std::string& sLocationIndentifiers) {
  assert(gAdditionStatus == Accepting);

  if (sMetaIdentifier.size() == 0) return false;
  std::auto_ptr<MetaLocation> pMetaLocation(new MetaLocation(sMetaIdentifier.c_str()));
  tract_t tIndex;
  if ((tIndex = getAtomicLocationIndex(sMetaIdentifier)) != -1) {
    AtomicMetaLocation *pAtomicMetaLocation = gvAtomicLocations[tIndex];
    pAtomicMetaLocation->setAsPromotedMetaLocation(pMetaLocation.get());
    //move(not delete) this AtomicMetaLocation into the promoted container
    gvAtomicLocations.erase(gvAtomicLocations.begin() + tIndex);
    gvPromotedAtomicLocations.push_back(pAtomicMetaLocation);
  }

  std::vector<tract_t> vLocationIndexes;
  boost::tokenizer<boost::escaped_list_separator<char> > identifiers(sLocationIndentifiers);
  for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=identifiers.begin(); itr != identifiers.end(); ++itr) {
     std::string token = (*itr);
     trimString(token);
     if (token.size() == 0) return false;
     if (token == sMetaIdentifier)
       throw resolvable_error("Error: Meta location ID '%s' defines itself as a member.", sMetaIdentifier.c_str());
     else if ((tIndex = getMetaLocationIndex(token)) != -1)
       pMetaLocation->addLocation(gvMetaLocations[tIndex]);
     else {
       //assume for now that token is referencing an atomic location
       std::auto_ptr<AtomicMetaLocation> AtomicLocation(new AtomicMetaLocation(token.c_str()));
       AtomicLocationsContainer_t::iterator itr=std::lower_bound(gvAtomicLocations.begin(), gvAtomicLocations.end(), AtomicLocation.get(), compareIdentifiers());
       if (itr == gvAtomicLocations.end() || strcmp((*itr)->getIndentifier(), token.c_str()))
         itr = gvAtomicLocations.insert(itr, AtomicLocation.release());
       pMetaLocation->addLocation(*itr);
     }
  }

  MetaLocationsContainer_t::iterator itr=std::lower_bound(gvMetaLocations.begin(), gvMetaLocations.end(), pMetaLocation.get(), compareIdentifiers());
  if (itr != gvMetaLocations.end() && !strcmp((*itr)->getIndentifier(), sMetaIdentifier.c_str())) {
    if (*(*itr) == *pMetaLocation) return true; // duplicate record
    throw resolvable_error("Error: Meta location ID '%s' is defined multiple times.", sMetaIdentifier.c_str());
  }
  gvMetaLocations.insert(itr, pMetaLocation.release());
  return true;
}

/** Closes object to further additions of meta location definitions. Adds all accumulated
    atomic locations to TractHandler object. */
void MetaLocationManager::MetaLocationPool::additionsCompleted(TractHandler& TInfo) {
  assert(gAdditionStatus == Accepting);
  gAdditionStatus = Closed;
  AtomicLocationsContainer_t::const_iterator itr=gvAtomicLocations.begin(), itr_end=gvAtomicLocations.end();
  TInfo.setCoordinateDimensions(0);
  for (;itr != itr_end; ++itr)
     TInfo.addLocation((*itr)->getIndentifier());
}

/** Assigns atomic indexes to each atomic location, as gotten from TractHandler object. */
void MetaLocationManager::MetaLocationPool::assignAtomicIndexes(TractHandler& TInfo) {
  assert(gAdditionStatus == Closed);
  assert(TInfo.getAddStatus() == TractHandler::Closed);
  AtomicLocationsContainer_t::const_iterator itr=gvAtomicLocations.begin(), itr_end=gvAtomicLocations.end();
  for (;itr != itr_end; ++itr)
     (*itr)->setTractIndex(TInfo.getLocationIndex((*itr)->getIndentifier()));
}

/** Returns index of location in internal collection atomic locations. Returns negative one if not found. */
tract_t MetaLocationManager::MetaLocationPool::getAtomicLocationIndex(const std::string& sIdentifier) const {
  std::auto_ptr<AtomicMetaLocation> search(new AtomicMetaLocation(sIdentifier.c_str()));
  AtomicLocationsContainer_t::const_iterator itr=std::lower_bound(gvAtomicLocations.begin(), gvAtomicLocations.end(), search.get(), compareIdentifiers());
  if (itr != gvAtomicLocations.end() && !strcmp((*itr)->getIndentifier(), sIdentifier.c_str()))
    return std::distance(gvAtomicLocations.begin(), itr);
  else
    return -1;
}

/** Returns index of location in internal collection meta locations. Returns negative one if not found. */
tract_t MetaLocationManager::MetaLocationPool::getMetaLocationIndex(const std::string& sMetaIdentifier) const {
  std::auto_ptr<MetaLocation> search(new MetaLocation(sMetaIdentifier.c_str()));
  MetaLocationsContainer_t::const_iterator itr=std::lower_bound(gvMetaLocations.begin(), gvMetaLocations.end(), search.get(), compareIdentifiers());
  if (itr != gvMetaLocations.end() && !strcmp((*itr)->getIndentifier(), sMetaIdentifier.c_str()))
    return std::distance(gvMetaLocations.begin(), itr);
  else
    return -1;
}

/** Prints defined meta locations to file stream. */
void MetaLocationManager::MetaLocationPool::print(TractHandler& TInfo, FILE * stream) const {
  FILE * fp=0;
  if (!stream) {
    if ((fp = fopen("MetaLocationPool.print", "w")) == NULL) return;
    stream = fp;
  }

  //print meta locations, top level only 
  MetaLocationsContainer_t::const_iterator itr=gvMetaLocations.begin(), itr_end=gvMetaLocations.end();
  for (;itr != itr_end; ++itr) {
     fprintf(stream, "%s", (*itr)->getIndentifier());
     for (unsigned int t=0; t < (*itr)->getLocations().size(); ++t)
       fprintf(stream, "%s%s", (t == 0 ? "=" : ","), (*itr)->getLocations()[t]->getIndentifier());
     fprintf(stream, "\n");
  }
  //print meta locations, broken down into respective atomic indexes 
  fprintf(stream, "\n");
  std::vector<tract_t> AtomicIndexes;
  for (size_t t=0; t < gvMetaLocations.size(); ++t) {
     fprintf(stream, "%s", gvMetaLocations[t]->getIndentifier());
     AtomicIndexes.clear();
     gvMetaLocations[t]->getAtomicIndexes(AtomicIndexes);
     for (unsigned int t=0; t < AtomicIndexes.size(); ++t)
       fprintf(stream, "%s%s(index=%d)", (t == 0 ? "=" : ","), TInfo.getLocations().at(AtomicIndexes.at(t))->getIndentifier(), AtomicIndexes.at(t));
     fprintf(stream, "\n");
  }
  if (fp) fclose(fp);
}

/////////////////////////// MetaLocationManager ////////////////////////////////

/** Adds meta location at index collection of locations that are directly referenced
    in the neighbors file definition. */
unsigned int MetaLocationManager::addReferenced(unsigned int tMetaLocation) {
  assert(geState == accepting);
  const MetaLocation* pLocation = gMetaLocationPool.getLocations().at(tMetaLocation);
  MetaLocationsRefContainer_t::iterator itr=std::lower_bound(gMetaLocations.begin(), gMetaLocations.end(), pLocation, compareIdentifiers());
  if (itr == gMetaLocations.end() || strcmp((*itr)->getIndentifier(), pLocation->getIndentifier()))
    itr = gMetaLocations.insert(itr, pLocation);
  giReferencedMetaLocations = gMetaLocations.size();
  return std::distance(gMetaLocations.begin(), itr);
}

/** Retrieves atomic indexes as defined by meta location at 'tMetaLocation'. */
std::vector<tract_t> & MetaLocationManager::getAtomicIndexes(unsigned int tMetaLocation, std::vector<tract_t>& AtomicIndexes) const {
  AtomicIndexes.clear();
  gMetaLocations.at(tMetaLocation)->getAtomicIndexes(AtomicIndexes);
  return AtomicIndexes;
}

/** Returns index of location in internal collection meta locations. Returns negative one if not found. */
tract_t MetaLocationManager::getMetaLocationIndex(const std::string& sMetaIdentifier) const {
  std::auto_ptr<MetaLocation> search(new MetaLocation(sMetaIdentifier.c_str()));
  MetaLocationsRefContainer_t::const_iterator itr=std::lower_bound(gMetaLocations.begin(), gMetaLocations.begin() + giReferencedMetaLocations, search.get(), compareIdentifiers());
  if (itr != gMetaLocations.end() && !strcmp((*itr)->getIndentifier(), sMetaIdentifier.c_str()))
    return std::distance(gMetaLocations.begin(), itr);
  else
    return -1;
}

/** Returns reference to meta locations pool. */
MetaLocationManager::MetaLocationPool & MetaLocationManager::getMetaLocationPool() {
  assert(geState == accepting);
  return gMetaLocationPool;
}

/** Returns indication of whether 'tTractIndex' is contained in meta location at 'tMetaLocation'. */
bool MetaLocationManager::intersectsTract(unsigned int tMetaLocation, tract_t tTractIndex) const {
  return gMetaLocations.at(tMetaLocation)->contains(tTractIndex);
}

/** Returns indication of whether two meta locations intersect geographically. */
bool MetaLocationManager::intersects(unsigned int tMetaLocationL, unsigned int tMetaLocationR) const {
  if (tMetaLocationL == tMetaLocationR) return true;
  return gMetaLocations.at(tMetaLocationL)->intersects(*gMetaLocations.at(tMetaLocationR));
}

/** Closes manager to further updates and appends remaining not referenced meta locations
    to list of referenced meta locations (if 'bIncludePoolRemainders' is true). Appending not
    referenced meta locations is needed when user requests the risk estimates file; otherwise
    only meta locations referenced in the neighbors file will be reported in that optional file. 
    Not that the number of referenced meta locations is stored in class variable 'giReferencedMetaLocations'.*/
void MetaLocationManager::setStateFixed(bool bIncludePoolRemainders) {
  assert(geState == accepting);
  if (bIncludePoolRemainders) {
    MetaLocationPool::MetaLocationsContainer_t::const_iterator itr=gMetaLocationPool.getLocations().begin(),
                                                               itr_end=gMetaLocationPool.getLocations().end();
    for (; itr != itr_end; ++itr) {
      MetaLocationsRefContainer_t::iterator itrPos=std::lower_bound(gMetaLocations.begin(), gMetaLocations.begin() + giReferencedMetaLocations, *itr, compareIdentifiers());
      if (itrPos == gMetaLocations.end() || strcmp((*itrPos)->getIndentifier(), (*itr)->getIndentifier()))
        gMetaLocations.push_back(*itr);
    }
  }
  geState = closed;
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

/** Returns first tract index defined for meta location at index 'iCollectionIndex'. */
tract_t MetaNeighborManager::getFirst(unsigned int iCollectionIndex) const {
  return gvUnifiedIndexesCollection.at(iCollectionIndex)->operator[](0);
}

/** Returns tract indexes defined for meta location at index 'iCollectionIndex'. */
std::vector<tract_t> & MetaNeighborManager::getIndexes(unsigned int iCollectionIndex, std::vector<tract_t>& v) const {
   MinimalGrowthArray<tract_t>& indexes = *gvUnifiedIndexesCollection.at(iCollectionIndex);
   v.resize(indexes.size());
   std::copy(&indexes[0], &indexes[0] + indexes.size(), v.begin());
   return v;
}

/** Returns whether tract 'tTractIndex' intersects with any tracts defined by meta location at 'tMetaLocation'. */
bool MetaNeighborManager::intersectsTract(unsigned int tMetaLocation, tract_t tTractIndex) const {
  const MinimalGrowthArray<tract_t>&  a = *gvUnifiedIndexesCollection.at(tMetaLocation);
  for (unsigned int t=0; t < a.size(); ++t)
     if (tTractIndex == a[t]) return true;
  return false;
}

/** Returns whether any tracts defined by meta location at 'tMetaLocationL' intersects with any tracts
    defined by meta location 'tMetaLocationR'. */
bool MetaNeighborManager::intersects(unsigned int tMetaLocationL, unsigned int tMetaLocationR) const {
  const MinimalGrowthArray<tract_t>&  a = *gvUnifiedIndexesCollection.at(tMetaLocationL);
  for (unsigned int t=0; t < a.size(); ++t)
     if (intersectsTract(tMetaLocationR, a[t])) return true;
  return false;
}

///////////////////////////// MetaManagerProxy /////////////////////////////////

/** constructor */
MetaManagerProxy::MetaManagerProxy(const MetaLocationManager& LocationManager, const MetaNeighborManager& NeighborManager)
                 :gpMetaLocationManager(&LocationManager), gpMetaNeighborManager(&NeighborManager) {}

/** Returns tract indexes defined by meta location at 'tMetaLocation'. */
std::vector<tract_t> & MetaManagerProxy::getIndexes(unsigned int tMetaLocation, std::vector<tract_t>& Indexes) const {
  if (tMetaLocation < gpMetaLocationManager->getLocations().size())
    gpMetaLocationManager->getAtomicIndexes(tMetaLocation, Indexes);
  else
    gpMetaNeighborManager->getIndexes(tMetaLocation, Indexes);
  return Indexes;
}

/** Returns number of defined meta locations. */
unsigned int MetaManagerProxy::getNumMetaLocations() const {
  return gpMetaLocationManager->getNumReferencedLocations() + gpMetaNeighborManager->size();
}

/** Returns whether tract 'tTractIndex' intersects with any tracts defined by meta location at 'tMetaLocation'. */
bool MetaManagerProxy::intersectsTract(unsigned int tMetaLocation, tract_t tTractIndex) const {
  if (tMetaLocation < gpMetaLocationManager->getNumReferencedLocations())
    return gpMetaLocationManager->intersectsTract(tMetaLocation, tTractIndex);
  else
    return gpMetaNeighborManager->intersectsTract(tMetaLocation, tTractIndex);
}

/** Returns whether any tracts defined by meta location at 'tMetaLocationL' intersects with any tracts
    defined by meta location 'tMetaLocationR'. */
bool MetaManagerProxy::intersects(unsigned int tMetaLocationL, unsigned int tMetaLocationR) const {
  if (tMetaLocationL < gpMetaLocationManager->getNumReferencedLocations())
    return gpMetaLocationManager->intersects(tMetaLocationL, tMetaLocationR);
  else
    return gpMetaNeighborManager->intersects(tMetaLocationL, tMetaLocationR);
}

