//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "Tracts.h"
#include "SaTScanData.h"
#include "AsciiPrintFormat.h"
#include "SSException.h"
#include "cluster.h"
#include "MetaTractManager.h"
#include "LocationNetwork.h"

//////////////// TractHandler::Coordinates class ///////////////////////////////

/** default constructor */
TractHandler::Coordinates::Coordinates() : giInsertionOrdinal(0), giSize(0), gpCoordinates(0) {}

/** constructor */
TractHandler::Coordinates::Coordinates(const std::vector<double>& Coordinates, unsigned int iInsertionOrdinal)
             :giSize(Coordinates.size()), giInsertionOrdinal(iInsertionOrdinal) {
   gpCoordinates = new double[giSize];
   memcpy(gpCoordinates, &Coordinates[0], giSize * sizeof(double));
}

/** Copy constructor. */
TractHandler::Coordinates::Coordinates(const Coordinates& rhs) {
   giSize = rhs.giSize;
   gpCoordinates = new double[giSize];
   memcpy(gpCoordinates, rhs.gpCoordinates, giSize * sizeof(double));
}

/** constructor */
TractHandler::Coordinates::Coordinates(double x, double y, unsigned int iInsertionOrdinal)
             :giSize(2), giInsertionOrdinal(iInsertionOrdinal) {
   gpCoordinates = new double[giSize];
   gpCoordinates[0] = x;
   gpCoordinates[1] = y;
}

/** destructor */
TractHandler::Coordinates::~Coordinates() {
  try {delete[] gpCoordinates;
  } catch(...){}
}

bool TractHandler::Coordinates::operator<(const Coordinates& rhs) const {
  if (giSize != rhs.giSize) return giSize < rhs.giSize;
  size_t t=0;
  while (t < giSize) {
    if (gpCoordinates[t] == rhs.gpCoordinates[t]) ++t;
    else return gpCoordinates[t] < rhs.gpCoordinates[t];
  }
  return false;
}

/** retrieves coordinates into passed vector */
void TractHandler::Coordinates::retrieve(std::vector<double>& Repository) const {
  Repository.resize(giSize);
  std::copy(gpCoordinates, gpCoordinates + giSize, Repository.begin());

}

//////////////// TractHandler::LocationIdentifier class ////////////////////////

/** constructor */
TractHandler::Location::Location(const char * sIdentifier, const Coordinates& aCoordinates) : gsIndentifier(0) {
    if (!sIdentifier) throw prg_error("Null location pointer.", "Location()");
    gsIndentifier = new char[strlen(sIdentifier) + 1];
    strcpy(gsIndentifier, sIdentifier);
	if (aCoordinates.getSize() > 0)
		gvCoordinatesContainer.add(&aCoordinates, true);
}

TractHandler::Location::~Location() {try {delete[] gsIndentifier;}catch(...){}}

/** Associates passed coordinates with this LocationIdentifier object. Throws ResolvableException
    if only one coordinates set permitted per location and passed coordinates do not equal
    already defined coordinates set. */
void TractHandler::Location::addCoordinates(const Coordinates& aCoordinates, MultipleCoordinatesType eMultipleCoordinatesType) {
  //add coordinates in sorted order -- this is needed for consistancy reasons, should we need to break ties in neighbors calculation
  bool bExists = gvCoordinatesContainer.exists(&aCoordinates);
  if (eMultipleCoordinatesType == ONEPERLOCATION && !bExists && gvCoordinatesContainer.size())
    throw resolvable_error("Error: The coordinates for location ID '%s' are defined multiple times in the coordinates file.", gsIndentifier);
  if (!bExists)
     gvCoordinatesContainer.add(&aCoordinates, true);
}

/** Adds passed string as secondary location identifier to this object. */
void TractHandler::Location::addSecondaryIdentifier(const std::string & sIdentifier) {
  gvSecondaryIdentifiers.add(sIdentifier, false);
}


/** Retrieves all location identifiers associated with this object. */

TractHandler::Location::StringContainer_t& TractHandler::Location::retrieveAllIdentifiers(StringContainer_t& Identifiers) const {

   Identifiers.clear();

   Identifiers.add(gsIndentifier, false);
   for (unsigned int i=0; i < gvSecondaryIdentifiers.size(); ++i)
     Identifiers.add(gvSecondaryIdentifiers[i], false);
   return Identifiers;
}


//////////////////////// TractHandler class ////////////////////////////////////

/** Constructor*/
TractHandler::TractHandler(bool bAggregatingTracts, MultipleCoordinatesType eMultipleCoordinatesType)
             :giCoordinateDimensions(0), giMaxIdentifierLength(0), gAdditionStatus(Accepting), giNumLocationCoordinates(0),
              gbAggregatingTracts(bAggregatingTracts), geMultipleCoordinatesType(eMultipleCoordinatesType) {
  if (gbAggregatingTracts) gvLocations.push_back(new Location("All", Coordinates()));
  gMetaManagerProxy.reset(new MetaManagerProxy(gMetaLocationsManager, gMetaNeighborManager));
}

/** This method should be called once all insertions are completed. Scans internal
    collection of location identifiers, looking for duplicates locations. */
void TractHandler::additionsCompleted(bool bReportingRiskEstimates) {
  std::string buffer;

  gAdditionStatus = Closed;
  gMetaLocationsManager.getMetaLocationPool().assignAtomicIndexes(*this);
  gMetaLocationsManager.setStateFixed(bReportingRiskEstimates);

  if (gvLocations.size() < 2 || giCoordinateDimensions == 0) return;

  //sort by first coordinates index
  std::sort(gvLocations.begin(), gvLocations.end(), CompareFirstCoordinatePointer());
  //search for duplicate locations -- those that have identical coordinate indexes
  size_t tSize=gvLocations.size();
  giNumLocationCoordinates=0;
  for (size_t tOuter=0; tOuter < tSize; ++tOuter) {
     giNumLocationCoordinates += gvLocations[tOuter]->getCoordinates().size();
     for (size_t tInner=tOuter+1; tInner < tSize; ++tInner) {
        //since we sorted by the first coordinate pointer, we know that duplicates will be adjacent
        if (gvLocations[tOuter]->getCoordinates() != gvLocations[tInner]->getCoordinates()) break;
        //add identifier of tInner to tOuter, they reference the same coordinate sets
		buffer = gvLocations[tInner]->getIndentifier();
        gvLocations[tOuter]->addSecondaryIdentifier(buffer);
        gmAggregateTracts[buffer] = gvLocations[tOuter]->getIndentifier();
        gvLocations.erase(gvLocations.begin() + tInner);
        tInner = tInner - 1;
        tSize = gvLocations.size();
     }
  }
  //re-sort locations by identifiers - TractHandler::getLocationIndex() relies upon this container being sorted by identifiers
  std::sort(gvLocations.begin(), gvLocations.end(), CompareIdentifiers());
}

/** Insert a tract into internal structure, sorting by tract identifier. Ignores location ids
    which already exist. Returns tract identifers relative index into internal structure. */
size_t TractHandler::addLocation(const char *sIdentifier) {
  assert(gAdditionStatus == Accepting);
  ptr_vector<Location>::iterator itr;

  try {
    if (gbAggregatingTracts) return 0;//when aggregating locations, insertion process always succeeds

    if (!sIdentifier) throw prg_error("Null location pointer.", "addLocation()");

    tract_t tLocationIndex = gMetaLocationsManager.getMetaLocationPool().getMetaLocationIndex(sIdentifier);
    if (tLocationIndex > -1) {
      gMetaLocationsManager.addReferenced(tLocationIndex); return tLocationIndex;
    }

    giMaxIdentifierLength = std::max(strlen(sIdentifier), giMaxIdentifierLength);
    std::auto_ptr<Location> identifier(new Location(sIdentifier, Coordinates()));
    itr = std::lower_bound(gvLocations.begin(), gvLocations.end(), identifier.get(), CompareIdentifiers());
	if (itr == gvLocations.end() || strcmp((*itr)->getIndentifier(), sIdentifier)) {
		itr = gvLocations.insert(itr, identifier.release());
	}
	return std::distance(gvLocations.begin(), itr);
  } catch (prg_exception& x) {
	x.addTrace("addLocation()", "TractHandler");
    throw;
  }
}

/** Inserts a locations identifier into internal structures.  */
void TractHandler::addLocation(const char *sIdentifier, std::vector<double>& vCoordinates, bool onlyIfExists) {
  assert(gAdditionStatus == Accepting);
  try {
    if (gbAggregatingTracts) return; //when aggregating locations, insertion process always succeeds

    if (vCoordinates.size() != giCoordinateDimensions)
      throw prg_error("Coordinate dimension is %u, expected %d.", "addLocation()", vCoordinates.size(), giCoordinateDimensions);

    if (!sIdentifier) throw prg_error("Null location pointer.", "addLocation()");

    giMaxIdentifierLength = std::max(strlen(sIdentifier), giMaxIdentifierLength);
    //insert unique coordinates into collection - ordered by first coordinate, then second coordinate, etc.
    std::auto_ptr<Coordinates> pCoordinates(new Coordinates(vCoordinates, gvCoordinates.size()));
    ptr_vector<Coordinates>::iterator itrCoordinates;
    itrCoordinates = std::lower_bound(gvCoordinates.begin(), gvCoordinates.end(), pCoordinates.get(), CompareCoordinates());
    if (itrCoordinates == gvCoordinates.end() || *(pCoordinates.get()) != *(*itrCoordinates))
      itrCoordinates = gvCoordinates.insert(itrCoordinates, pCoordinates.release());

    //insert into location identifier structure - ordered by indentifer name
    ptr_vector<Location>::iterator itrIdentifiers;
    std::auto_ptr<Location> identifier(new Location(sIdentifier, *(*itrCoordinates)));
    itrIdentifiers = std::lower_bound(gvLocations.begin(), gvLocations.end(), identifier.get(), CompareIdentifiers());
    if (itrIdentifiers != gvLocations.end() && !strcmp((*itrIdentifiers)->getIndentifier(), sIdentifier))
      //when identifier already exists, then note that it is defined at this coordinate as well
      (*itrIdentifiers)->addCoordinates(*(*itrCoordinates), geMultipleCoordinatesType);
    else if (!onlyIfExists)
      //otherwise this is a new identifier
      gvLocations.insert(itrIdentifiers, identifier.release());
  }
  catch (prg_exception& x) {
    x.addTrace("addLocation()", "TractHandler");
    throw;
  }
}

bool TractHandler::addLocationsDistanceOverride(tract_t t1, tract_t t2, double distance) {
	tract_t smaller = std::min(t1, t2), larger = std::max(t1, t2);
	LocationOverrides_t::iterator itrSmaller = _location_distance_overrides.find(smaller);
	if (itrSmaller == _location_distance_overrides.end()) {
		std::map<tract_t, double> v;
		v[larger] = distance;
		_location_distance_overrides[smaller] = v;
	} else {
		std::map<tract_t, double>::iterator itrLarger = itrSmaller->second.find(larger);
		if (itrLarger == itrSmaller->second.end()) {
			std::map<tract_t, double> v;
			v[larger] = distance;
			itrSmaller->second = v;
		} else if (itrLarger->second != distance)
			return false;
	}
	return true;
}

std::pair<bool, double> TractHandler::getLocationsDistanceOverride(tract_t t1, tract_t t2) const {
	tract_t smaller = std::min(t1, t2), larger = std::max(t1, t2);
	LocationOverrides_t::const_iterator itrSmaller = _location_distance_overrides.find(smaller);
	if (itrSmaller != _location_distance_overrides.end()) {
		std::map<tract_t, double>::const_iterator itrLarger = itrSmaller->second.find(larger);
		if (itrLarger != itrSmaller->second.end()) return std::make_pair(true, itrLarger->second);
	}
	return std::make_pair(false, 0.0);
}

/** Returns indication of whether coordinates are currently defined. */
bool TractHandler::getCoordinatesExist(std::vector<double>& vCoordinates) const {
  try {
    std::auto_ptr<Coordinates> pCoordinates(new Coordinates(vCoordinates, gvCoordinates.size()));
    ptr_vector<Coordinates>::const_iterator itrCoordinates;
    itrCoordinates = std::lower_bound(gvCoordinates.begin(), gvCoordinates.end(), pCoordinates.get(), CompareCoordinates());
    if (itrCoordinates == gvCoordinates.end() || *(pCoordinates.get()) != *(*itrCoordinates))
        return false;
  }
  catch (prg_exception& x) {
    x.addTrace("getCoordinatesExist()", "TractHandler");
    throw;
  }
  return true;
}

/** Compute distance squared between two points. */
double TractHandler::getDistanceSquared(const std::vector<double>& vFirstPoint, const std::vector<double>& vSecondPoint) {
  double        dDistanceSquared=0;

  if (vFirstPoint.size() != vSecondPoint.size())
    throw prg_error("First point has %u coordinates and second point has %u.", "getDistanceSquared()", vFirstPoint.size(), vSecondPoint.size());

  for (size_t i=0; i < vFirstPoint.size(); ++i)
     dDistanceSquared += (vFirstPoint[i] - vSecondPoint[i]) * (vFirstPoint[i] - vSecondPoint[i]);

  return dDistanceSquared;
}

/** Returns identifier associated with location at 'tIndex'. If 'tIndex' is greater than
    number of locations, it is assumed to be referencing a meta location. */
const char * TractHandler::getIdentifier(tract_t tIndex) const {
  if ((size_t)tIndex < gvLocations.size())
    return gvLocations.at(tIndex)->getIndentifier();
  else if (gMetaLocationsManager.getLocations().size())
    return gMetaLocationsManager.getLocations().at((size_t)tIndex - gvLocations.size())->getIndentifier();
  else {
    //### still not certain what should be the behavior in this situation ###
    tract_t tFirst = gMetaNeighborManager.getFirst((size_t)tIndex - gvLocations.size());
    return gvLocations.at(tFirst)->getIndentifier();
  }
}

/** Searches for tract identifier and returns it's internal index, or -1 if not found.
    When aggregating locations, always return zero. */
tract_t TractHandler::getLocationIndex(const char *sIdentifier) const {
  tract_t  tPosReturn;

  try {
    if (gbAggregatingTracts)
      //when aggregation locations, all tract identifiers refer to the same index
      return 0;

    if (!sIdentifier) throw prg_error("Null location pointer.", "getLocationIndex()");

    std::string _identifier;

    //first search collection of known aggregated location identifiers 
    std::map<std::string,std::string>::const_iterator itrm = gmAggregateTracts.find(std::string(sIdentifier));
    if (itrm != gmAggregateTracts.end())
        _identifier = itrm->second;
    else
        _identifier = sIdentifier;

    //search for tract identifier in vector
    std::auto_ptr<Location> _search(new Location(_identifier.c_str(), Coordinates()));
    ptr_vector<Location>::const_iterator   itr;
    itr = std::lower_bound(gvLocations.begin(), gvLocations.end(), _search.get(), CompareIdentifiers());
    if (itr != gvLocations.end() && !strcmp((*itr)->getIndentifier(), _identifier.c_str()))
      tPosReturn = std::distance(gvLocations.begin(), itr);
    else
      tPosReturn = -1;
  }
  catch (prg_exception& x)  {
    x.addTrace("getLocationIndex()", "TractHandler");
    throw;
  }
  return tPosReturn;
}

/** Print locations to ASCII file. */
void TractHandler::printLocations(FILE * pFile) const {
   //FILE* pFile;

   try {
      //if ((pFile = fopen(sFilename, "w")) == NULL)
      //  throw resolvable_error("Error: Unable to open top clusters file.\n");
      //else {
        ptr_vector<Location>::const_iterator itr;
        for (itr=gvLocations.begin(); itr != gvLocations.end(); ++itr) {
            fprintf(pFile,"Identifier: %s\n",(*itr)->getIndentifier());
            fprintf(pFile,"Coordinates: ");            
            for (unsigned int t=0; t < (*itr)->getCoordinates().size(); ++t) {
                const Coordinates * pCoords = (*itr)->getCoordinates()[t];
                for (size_t c=0; c < pCoords->getSize(); ++c) {
                    fprintf(pFile," %lf ", pCoords->getCoordinates()[c]);
                }
            }
            fprintf(pFile,"Secondary Identifiers: ");   
            for (unsigned int i=0; i < (*itr)->getSecondaryIdentifiers().size(); ++i) {
                fprintf(pFile," %s ", (*itr)->getSecondaryIdentifiers()[i].c_str() );
            }
            fprintf(pFile, " \n\n");
        }
      //}
      //fclose(pFile); pFile=0;
   }
  catch (prg_exception& x) {
    //if (pFile) fclose(pFile);
    x.addTrace("printLocations()","TractHandler");
    throw;
  }
}

/** Prints formatted message to file which details the locations of the coordinates
    file that had identical coordinates and where combined into one location
    for internal usage. */
void TractHandler::reportCombinedLocations(FILE * fDisplay) const {
  ptr_vector<Location>::const_iterator        itr;
  AsciiPrintFormat                            PrintFormat;
  bool                                        bPrinted=false;

  for (itr=gvLocations.begin(); itr != gvLocations.end(); ++itr) {
     if ((*itr)->getSecondaryIdentifiers().size()) {
       if (!bPrinted) {
         PrintFormat.SetMarginsAsOverviewSection();
         std::string buffer = "\nNote: The coordinates file contains location IDs with identical "
                              "coordinates that were combined into one location. In the "
                              "optional output files, combined locations are represented by a "
                              "single location ID as follows:";
         PrintFormat.PrintAlignedMarginsDataString(fDisplay, buffer);
         PrintFormat.PrintSectionSeparatorString(fDisplay, 0, 1, '-');
         bPrinted=true;
       }
       //First retrieved location ID is the location that represents all others.
       std::string buffer;
       printString(buffer, "%s : %s", (*itr)->getIndentifier(), (*itr)->getSecondaryIdentifiers()[0].c_str());
       for (unsigned int i=1; i < (*itr)->getSecondaryIdentifiers().size(); ++i) {
          buffer += ", "; buffer += (*itr)->getSecondaryIdentifiers()[i].c_str();
       }
       PrintFormat.PrintAlignedMarginsDataString(fDisplay, buffer);
     }
  }
}

/** Retrieves identifiers associated with location at 'tIndex'. If 'tIndex' is greater than
    number of locations, it is assumed to be referencing a meta location. */
TractHandler::Location::StringContainer_t & TractHandler::retrieveAllIdentifiers(tract_t tIndex, TractHandler::Location::StringContainer_t& Identifiers) const {
  if ((size_t)tIndex < gvLocations.size())
    gvLocations.at(tIndex)->retrieveAllIdentifiers(Identifiers);
  else if (gMetaLocationsManager.getLocations().size()) {
    Identifiers.clear();
    Identifiers.add(std::string(gMetaLocationsManager.getLocations().at((size_t)tIndex - gvLocations.size())->getIndentifier()), false);
  }
  else {
    Identifiers.clear();
    std::vector<tract_t> indexes;
    gMetaNeighborManager.getIndexes((size_t)tIndex - gvLocations.size(), indexes);
    for (size_t t=0; t < indexes.size(); ++t) {
      TractHandler::Location::StringContainer_t tract_identifiers;
       gvLocations.at(indexes[t])->retrieveAllIdentifiers(tract_identifiers);
       for (size_t i=0; i < tract_identifiers.size(); ++i)
          Identifiers.add(tract_identifiers[i], false);
    }
  }
  return Identifiers;
}

/** Sets dimensions of location coordinates. If aggregating locations, function just returns; otherwise
    if any locations are already defined - throws an exception. */
void TractHandler::setCoordinateDimensions(size_t iDimensions) {
  if (gbAggregatingTracts) return; //ignore this when aggregating locations

  if (gvCoordinates.size())
    throw prg_error("Changing the coordinate dimensions is not permited once locations have been defined.","setCoordinateDimensions()");

  giCoordinateDimensions = iDimensions;
}

/** Assigns locations and coordinates explicitly. Note that this is a special use function, initially
    designed for use with the homogeneous poisson model. It might have been nicer to abstract this class
    instead of creating this function but that would have involved a significant re-factor with little benefit. */
void TractHandler::assignExplicitCoordinates(CoordinatesContainer_t& coordinates) {
  assert(gAdditionStatus == Accepting);

  try {
    if (gbAggregatingTracts) return; //ignore operation if aggregating tracts

    //If the internal container of coordinates is equal in size to new container, just copy coordinates.
    if (gvCoordinates.size() == coordinates.size()) {
        for (size_t t=0; t < coordinates.size(); ++t) {
            //Verify that coordinates have equal dimensions...
            if (!gvCoordinates[t])
                throw prg_error("not allolcated.", "pushCoordinates()");
            if (!coordinates[t])
                throw prg_error("not allolcated.", "pushCoordinates()");

            if (gvCoordinates[t]->getSize() != coordinates[t]->getSize())
               throw prg_error("Coordinate dimension is %u, expected %d.", "pushCoordinates()", coordinates[t]->getSize(), gvCoordinates[t]->getSize());
             memcpy(gvCoordinates[t]->getCoordinates(), coordinates[t]->getCoordinates(), sizeof(double) * coordinates[t]->getSize() );
        }
    }
    else {
        //New container and internal container are not equal in size -- clear and rebuild.  
        gvLocations.killAll();
        gvCoordinates.killAll();
        gvCoordinates.resize(coordinates.size(), 0);
        gvLocations.resize(coordinates.size(), 0);
        //Note: I'm ignoring meta data, I think it ok to do that. Also, they are not likely used in conjunction with this function.
        for (size_t t=0; t < coordinates.size(); ++t) {
            //Verfy that coordinate dimensions match expected.
            if (coordinates[t]->getSize() != giCoordinateDimensions)
               throw prg_error("Coordinate dimension is %u, expected %d.", "pushCoordinates()", coordinates.size(), giCoordinateDimensions);
            //Create new copy of coordinates object.
            gvCoordinates[t] = new Coordinates(*coordinates[t]);
            //Create dummy location identifier and associate coordinate object.
            gvLocations[t] = new Location("_location_", *gvCoordinates[t]);
        }
        giNumLocationCoordinates = coordinates.size();
    }
  }
  catch (prg_exception& x) {
    x.addTrace("assignExplicitCoordinates()", "TractHandler");
    throw;
  }
}
