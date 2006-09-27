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
  std::strcpy(gsIndentifier, sIdentifier);
}

/** destructor */
MetaLocationManager::AbstractMetaLocation::~AbstractMetaLocation() {
  try {delete[] gsIndentifier;} catch(...){}
}

////////////////// MetaLocationManager::AbstractMetaLocation////////////////////

/** constructor */
MetaLocationManager::AtomicMetaLocation::AtomicMetaLocation(const char * sIdentifier)
                    :AbstractMetaLocation(sIdentifier), giTractIndex(0), gpPromotedMetaLocation(0) {}

void MetaLocationManager::AtomicMetaLocation::getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const {
  if (gpPromotedMetaLocation)
    gpPromotedMetaLocation->getAtomicIndexes(AtomicIndexes);
  else
    AtomicIndexes.push_back(giTractIndex);
}

////////////////// MetaLocationManager::MetaLocation ///////////////////////////

/** constructor */
MetaLocationManager::MetaLocation::MetaLocation(const char * sIdentifier) : AbstractMetaLocation(sIdentifier) {}

void MetaLocationManager::MetaLocation::addLocation(const AbstractMetaLocation * pLocation) {
//  if (gLocations.exists(pLocation)) error
  gLocations.add(pLocation, false);
}

void MetaLocationManager::MetaLocation::getAtomicIndexes(std::vector<tract_t>& AtomicIndexes) const {
  for (unsigned int t=0; t < gLocations.size(); ++t)
     gLocations[t]->getAtomicIndexes(AtomicIndexes);
}

/////////////////////////// MetaLocationManager ////////////////////////////////

/** Adds meta-location with */
void MetaLocationManager::addMetaLocation(const std::string& sMetaIdentifier, const std::string& sLocationIndentifiers) {
  assert(gAdditionStatus == Accepting);
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
     trim(token);
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
    if (*(*itr) == *pMetaLocation) return; // duplicate record
    GenerateResolvableException("Error: Meta location ID '%s' is defined multiple times.", "addMetaLocation()", sMetaIdentifier.c_str());
  }
  gvMetaLocations.insert(itr, pMetaLocation.release());
  
}

void MetaLocationManager::additionsCompleted() {
  assert(gAdditionStatus == Accepting);
  gAdditionStatus = Closed;
  AtomicLocationsContainer_t::const_iterator itr=gvAtomicLocations.begin(), itr_end=gvAtomicLocations.end();
  gTInfo.setCoordinateDimensions(0);
  for (;itr != itr_end; ++itr)
     gTInfo.addLocation((*itr)->getIndentifier());
}

void MetaLocationManager::generateAtomicIndexes() {
  assert(gAdditionStatus == Closed);
  AtomicLocationsContainer_t::const_iterator itr=gvAtomicLocations.begin(), itr_end=gvAtomicLocations.end();
  for (;itr != itr_end; ++itr)
     (*itr)->setTractIndex(gTInfo.getLocationIndex((*itr)->getIndentifier()));
}

std::vector<tract_t> & MetaLocationManager::getAtomicIndexes(unsigned int tMetaLocation, std::vector<tract_t>& AtomicIndexes) const {
  AtomicIndexes.clear();
  gvMetaLocations.at(tMetaLocation)->getAtomicIndexes(AtomicIndexes);
  return AtomicIndexes;
}

tract_t MetaLocationManager::getAtomicLocationIndex(const std::string& sIdentifier) const {
  std::auto_ptr<AtomicMetaLocation> search(new AtomicMetaLocation(sIdentifier.c_str()));
  AtomicLocationsContainer_t::const_iterator itr=std::lower_bound(gvAtomicLocations.begin(), gvAtomicLocations.end(), search.get(), compareIdentifiers());
  if (itr != gvAtomicLocations.end() && !strcmp((*itr)->getIndentifier(), sIdentifier.c_str()))
    return std::distance(gvAtomicLocations.begin(), itr);
  else
    return -1;
}

tract_t MetaLocationManager::getMetaLocationIndex(const std::string& sMetaIdentifier) const {
  std::auto_ptr<MetaLocation> search(new MetaLocation(sMetaIdentifier.c_str()));
  MetaLocationsContainer_t::const_iterator itr=std::lower_bound(gvMetaLocations.begin(), gvMetaLocations.end(), search.get(), compareIdentifiers());
  if (itr != gvMetaLocations.end() && !strcmp((*itr)->getIndentifier(), sMetaIdentifier.c_str()))
    return std::distance(gvMetaLocations.begin(), itr);
  else
    return -1;
}

void MetaLocationManager::print(FILE * stream) const {
  FILE * fp=0;
  if (!stream) {
    if ((fp = fopen("MetaLocationManager.print", "w")) == NULL) return;
    stream = fp;
  }

  MetaLocationsContainer_t::const_iterator itr=gvMetaLocations.begin(), itr_end=gvMetaLocations.end();
  for (;itr != itr_end; ++itr) {
     fprintf(stream, "%s", (*itr)->getIndentifier());
     for (unsigned int t=0; t < (*itr)->getLocations().size(); ++t)
       fprintf(stream, "%s%s", (t == 0 ? "=" : ","), (*itr)->getLocations()[t]->getIndentifier());
     fprintf(stream, "\n");
  }
  fprintf(stream, "\n");
  std::vector<tract_t> AtomicIndexes;
  for (size_t t=0; t < gvMetaLocations.size(); ++t) {
     fprintf(stream, "%s", gvMetaLocations[t]->getIndentifier());
     AtomicIndexes.clear();
     getAtomicIndexes(t, AtomicIndexes);
     for (unsigned int t=0; t < AtomicIndexes.size(); ++t)
       fprintf(stream, "%s%s(index=%d)", (t == 0 ? "=" : ","), gTInfo.getLocations().at(AtomicIndexes.at(t))->getIndentifier(), AtomicIndexes.at(t));
     fprintf(stream, "\n");
  }
  fclose(fp);
}

