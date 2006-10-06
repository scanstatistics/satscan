//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MetaTractManager.h"
#include "Tracts.h"
#include "SSException.h"
#include<boost/tokenizer.hpp>

////////////////// MetaLocationManager::AbstractMetaLocation////////////////////

/** constructor */
MetaLocationManager::AbstractMetaLocation::AbstractMetaLocation(const char * sIdentifier) {
  gsIndentifier = new char[strlen(sIdentifier) + 1];
  strcpy(gsIndentifier, sIdentifier);
}

/** destructor */
MetaLocationManager::AbstractMetaLocation::~AbstractMetaLocation() {
  try {delete[] gsIndentifier;} catch(...){}
}

////////////////// MetaLocationManager::AtomicMetaLocation////////////////////

/** constructor */
MetaLocationManager::AtomicMetaLocation::AtomicMetaLocation(const char * sIdentifier)
                    :AbstractMetaLocation(sIdentifier), giTractIndex(0), gpPromotedMetaLocation(0) {}

/** Returns indication of whether this location contains 'tAtomicIndex'. */
bool MetaLocationManager::AtomicMetaLocation::contains(tract_t tAtomicIndex) const {
  if (gpPromotedMetaLocation) return gpPromotedMetaLocation->contains(tAtomicIndex);
  return tAtomicIndex == giTractIndex;
}

/** Returns indication of whether this location contains 'pMetaLocation'. */
bool MetaLocationManager::AtomicMetaLocation::contains(const MetaLocation& pMetaLocation) const {
  if (gpPromotedMetaLocation) return gpPromotedMetaLocation->contains(pMetaLocation);
  return false;
}

/** Retrieves atomic indexes of contained locations, in sequential order. */
void MetaLocationManager::AtomicMetaLocation::getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const {
  if (gpPromotedMetaLocation) gpPromotedMetaLocation->getAtomicIndexes(AtomicIndexes);
  else AtomicIndexes.push_back(giTractIndex);
}

/** Returns indication of whether passed location object intersect geographically with this object. */
bool MetaLocationManager::AtomicMetaLocation::intersects(const AbstractMetaLocation& pLocation) const {
  if (gpPromotedMetaLocation) return gpPromotedMetaLocation->intersects(pLocation);
  return pLocation.contains(giTractIndex);
}

////////////////// MetaLocationManager::MetaLocation ///////////////////////////

/** constructor */
MetaLocationManager::MetaLocation::MetaLocation(const char * sIdentifier) : AbstractMetaLocation(sIdentifier) {}

/** Adds atomic location object to this objects collection of AbstractMetaLocation locations. */
void MetaLocationManager::MetaLocation::addLocation(const AtomicMetaLocation * pLocation) {
  gLocations.add(pLocation, false);
}

/** Adds meta location object to this objects collection of AbstractMetaLocation locations.
    Throw ZdExeption if 'this' object is contained is passed object. */
void MetaLocationManager::MetaLocation::addLocation(const MetaLocation * pLocation) {
  if (pLocation->contains(*this))
    GenerateResolvableException("Error: Circular definition between meta locations '%s' and '%s'.",
                                "addMetaLocation()", getIndentifier(), pLocation->getIndentifier());
  gLocations.add(pLocation, false);
}

/** Returns indication of whether this location contains 'tAtomicIndex'. */
bool MetaLocationManager::MetaLocation::contains(tract_t tAtomicIndex) const {
  for (unsigned int t=0; t < gLocations.size(); ++t)
     if (gLocations[t]->contains(tAtomicIndex)) return true;
  return false;
}

/** Returns indication of whether this location contains 'pMetaLocation'. */
bool MetaLocationManager::MetaLocation::contains(const MetaLocation& pMetaLocation) const {
  if (this == &pMetaLocation) return true;
  for (unsigned int t=0; t < gLocations.size(); ++t)
     if (gLocations[t]->contains(pMetaLocation)) return true;
  return false;
}

/** Retrieves atomic indexes of contained locations, in sequential order. */
void MetaLocationManager::MetaLocation::getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const {
  for (unsigned int t=0; t < gLocations.size(); ++t)
     gLocations[t]->getAtomicIndexes(AtomicIndexes);
}

/** Returns indication of whether passed location object intersect geographically with this object. */
bool MetaLocationManager::MetaLocation::intersects(const AbstractMetaLocation& pLocation) const {
  for (unsigned int t=0; t < gLocations.size(); ++t)
     if (gLocations[t]->intersects(pLocation)) return true;
  return false;   
}

/////////////////////////// MetaLocationManager ////////////////////////////////

/** Adds meta-location with */
bool MetaLocationManager::addMetaLocation(const std::string& sMetaIdentifier, const std::string& sLocationIndentifiers) {
  assert(gAdditionStatus == Accepting);

  if (sMetaIdentifier.size() == 0) return false;
  std::auto_ptr<MetaLocation> pMetaLocation(new MetaLocation(sMetaIdentifier.c_str()));
  tract_t tIndex;
  if ((tIndex = getAtomicLocationIndex(sMetaIdentifier.c_str())) != -1) {
    AtomicMetaLocation *pAtomicMetaLocation = gvAtomicLocations[tIndex];
    pAtomicMetaLocation->setAsPromotedMetaLocation(pMetaLocation.get());
    gvAtomicLocations.RemoveElement(tIndex);
    gvPromotedAtomicLocations.push_back(pAtomicMetaLocation);
  }

  std::vector<tract_t> vLocationIndexes;
  boost::tokenizer<boost::escaped_list_separator<char> > identifiers(sLocationIndentifiers);
  for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=identifiers.begin(); itr != identifiers.end(); ++itr) {
     std::string token = (*itr);
     trimString(token);
     if (token.size() == 0) return false;
     if (token == sMetaIdentifier)
       GenerateResolvableException("Error: Meta location ID '%s' defines itself as a member.", "addMetaLocation()", sMetaIdentifier.c_str());
     else if ((tIndex = getMetaLocationIndex(token.c_str())) != -1)
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
    GenerateResolvableException("Error: Meta location ID '%s' is defined multiple times.", "addMetaLocation()", sMetaIdentifier.c_str());
  }
  gvMetaLocations.insert(itr, pMetaLocation.release());
  return true;
}

/** Closes object to further additions of meta location definitions. Adds all accumulated
    atomic locations to TractHandler object. */
void MetaLocationManager::additionsCompleted(TractHandler& TInfo) {
  assert(gAdditionStatus == Accepting);
  gAdditionStatus = Closed;
  AtomicLocationsContainer_t::const_iterator itr=gvAtomicLocations.begin(), itr_end=gvAtomicLocations.end();
  TInfo.setCoordinateDimensions(0);
  for (;itr != itr_end; ++itr)
     TInfo.addLocation((*itr)->getIndentifier());
}

/** Assigns atomic indexes to each atomic location, as gotten from TractHandler object. */
void MetaLocationManager::assignAtomicIndexes(TractHandler& TInfo) {
  assert(gAdditionStatus == Closed);
  assert(TInfo.getAddStatus() == TractHandler::Closed);
  AtomicLocationsContainer_t::const_iterator itr=gvAtomicLocations.begin(), itr_end=gvAtomicLocations.end();
  for (;itr != itr_end; ++itr)
     (*itr)->setTractIndex(TInfo.getLocationIndex((*itr)->getIndentifier()));
}

/** Retrieves atomic indexes as defined by meta location at 'tMetaLocation'. */
std::vector<tract_t> & MetaLocationManager::getAtomicIndexes(unsigned int tMetaLocation, std::vector<tract_t>& AtomicIndexes) const {
  AtomicIndexes.clear();
  gvMetaLocations.at(tMetaLocation)->getAtomicIndexes(AtomicIndexes);
  return AtomicIndexes;
}

/** Returns index of location in internal collection atomic locations. Returns negative one if not found. */
tract_t MetaLocationManager::getAtomicLocationIndex(const std::string& sIdentifier) const {
  std::auto_ptr<AtomicMetaLocation> search(new AtomicMetaLocation(sIdentifier.c_str()));
  AtomicLocationsContainer_t::const_iterator itr=std::lower_bound(gvAtomicLocations.begin(), gvAtomicLocations.end(), search.get(), compareIdentifiers());
  if (itr != gvAtomicLocations.end() && !strcmp((*itr)->getIndentifier(), sIdentifier.c_str()))
    return std::distance(gvAtomicLocations.begin(), itr);
  else
    return -1;
}

/** Returns index of location in internal collection meta locations. Returns negative one if not found. */
tract_t MetaLocationManager::getMetaLocationIndex(const std::string& sMetaIdentifier) const {
  std::auto_ptr<MetaLocation> search(new MetaLocation(sMetaIdentifier.c_str()));
  MetaLocationsContainer_t::const_iterator itr=std::lower_bound(gvMetaLocations.begin(), gvMetaLocations.end(), search.get(), compareIdentifiers());
  if (itr != gvMetaLocations.end() && !strcmp((*itr)->getIndentifier(), sMetaIdentifier.c_str()))
    return std::distance(gvMetaLocations.begin(), itr);
  else
    return -1;
}

/** Returns indication of whether 'tTractIndex' is contained in meta location at 'tMetaLocation'. */
bool MetaLocationManager::intersectsTract(unsigned int tMetaLocation, tract_t tTractIndex) const {
  return gvMetaLocations.at(tMetaLocation)->contains(tTractIndex);
}

/** Returns indication of whether two meta locations intersect geographically. */
bool MetaLocationManager::intersects(unsigned int tMetaLocationL, unsigned int tMetaLocationR) const {
  if (tMetaLocationL == tMetaLocationR) return true;
  return gvMetaLocations.at(tMetaLocationL)->intersects(*gvMetaLocations.at(tMetaLocationR));
}

/** Prints defined meta locations to file stream. */
void MetaLocationManager::print(TractHandler& TInfo, FILE * stream) const {
  FILE * fp=0;
  if (!stream) {
    if ((fp = fopen("MetaLocationManager.print", "w")) == NULL) return;
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
     getAtomicIndexes(t, AtomicIndexes);
     for (unsigned int t=0; t < AtomicIndexes.size(); ++t)
       fprintf(stream, "%s%s(index=%d)", (t == 0 ? "=" : ","), TInfo.getLocations().at(AtomicIndexes.at(t))->getIndentifier(), AtomicIndexes.at(t));
     fprintf(stream, "\n");
  }
  fclose(fp);
}

