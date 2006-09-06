//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "Tracts.h"
#include "SaTScanData.h"
#include "AsciiPrintFormat.h"
#include "SSException.h"
#include "cluster.h"

//////////////// TractHandler::Coordinates class ///////////////////////////////

/** default constructor */
TractHandler::Coordinates::Coordinates() : giInsertionOrdinal(0), giSize(0), gpCoordinates(0) {}

/** constructor */
TractHandler::Coordinates::Coordinates(const std::vector<double>& Coordinates, unsigned int iInsertionOrdinal)
             :giSize(Coordinates.size()), giInsertionOrdinal(iInsertionOrdinal) {
   gpCoordinates = new double[giSize];
   memcpy(gpCoordinates, &Coordinates[0], giSize * sizeof(double));
}

/** destructor */
TractHandler::Coordinates::~Coordinates() {
  try {delete[] gpCoordinates;
  } catch(...){}
}

/** overloaded operator not equals */
bool TractHandler::Coordinates::operator!=(const Coordinates& rhs) const {
  if (giSize != rhs.giSize) return true;
  return memcmp(gpCoordinates, rhs.gpCoordinates, giSize * sizeof(double));
}

/** overloaded operator less than */
bool TractHandler::Coordinates::operator<(const Coordinates& rhs) const {
  if (giSize != rhs.giSize) return giSize < rhs.giSize;

  size_t t=0;
  while (t < giSize) {
      if (gpCoordinates[t] == rhs.gpCoordinates[t]) ++t;
      else return gpCoordinates[t] < rhs.gpCoordinates[t];
  }
  return false;
}

/** overloaded operator equals */
bool TractHandler::Coordinates::operator==(const Coordinates& rhs) const {
  if (giSize != rhs.giSize) return false;
  return !memcmp(gpCoordinates, rhs.gpCoordinates, giSize * sizeof(double));
}

/** retrieves coordinates into passed vector */
void TractHandler::Coordinates::retrieve(std::vector<double>& Repository) const {
  Repository.resize(giSize);
  std::copy(gpCoordinates, gpCoordinates + giSize, Repository.begin());

}

//////////////// TractHandler::LocationIdentifier class ////////////////////////

/** constructor */
TractHandler::Location::Location(const std::string& sIdentifier, const Coordinates& aCoordinates) : gsIndentifier(sIdentifier) {
  gvCoordinatesContainer.push_back(&aCoordinates);
}

/** Associates passed coordinates with this LocationIdentifier object. Throws ResolvableException
    if only one coordinates set permitted per location and passed coordinates do not equal
    already defined coordinates set. */
void TractHandler::Location::addCoordinates(const Coordinates& aCoordinates, MultipleCoordinatesType eMultipleCoordinatesType) {
  //add coordinates in sorted order -- this is needed for consistancy reasons, should we need to break ties in neighbors calculation
  CoordsContainer_t::iterator itr = std::lower_bound(gvCoordinatesContainer.begin(), gvCoordinatesContainer.end(), &aCoordinates, CompareCoordinates());
  if (eMultipleCoordinatesType == ONEPERLOCATION && (itr == gvCoordinatesContainer.end() || *(*itr) != aCoordinates))
    GenerateResolvableException("Error: The coordinates for location ID '%s' are defined multiple times in the coordinates file.", "addCoordinateIndex()", gsIndentifier.c_str());
  if (itr == gvCoordinatesContainer.end() || *(*itr) != aCoordinates)
   gvCoordinatesContainer.insert(itr, &aCoordinates);
}

/** Adds passed string as secondary location identifier to this object. */
void TractHandler::Location::addSecondaryIdentifier(const std::string & sIdentifier) {
  gvSecondaryIdentifiers.push_back(sIdentifier);
}

/** Retrieves all location identifiers associated with this object. */
TractHandler::Location::StringContainer_t& TractHandler::Location::retrieveAllIdentifiers(StringContainer_t& Identifiers) const {
   Identifiers.clear();
   Identifiers.push_back(gsIndentifier);
   for (size_t s=0; s < gvSecondaryIdentifiers.size(); ++s)
     Identifiers.push_back(gvSecondaryIdentifiers.at(s));
   return Identifiers;
}

//////////////////////// TractHandler class ////////////////////////////////////

/** Constructor*/
TractHandler::TractHandler(bool bAggregatingTracts, MultipleCoordinatesType eMultipleCoordinatesType)
             :giCoordinateDimensions(0), giMaxIdentifierLength(0), gAdditionStatus(Accepting),
              gbAggregatingTracts(bAggregatingTracts), geMultipleCoordinatesType(eMultipleCoordinatesType) {
  if (gbAggregatingTracts) gvLocations.push_back(new Location("All", Coordinates()));
}

/** This method should be called once all insertions are completed. Scans internal
    collection of location identifiers, looking for duplicates locations. */
void TractHandler::additionsCompleted() {
  gAdditionStatus = Closed;
  if (gvLocations.size() < 2 || giCoordinateDimensions == 0) return;

  //sort by first coordinates index
  std::sort(gvLocations.begin(), gvLocations.end(), CompareFirstCoordinatePointer());
  //search for duplicate locations -- those that have identical coordinate indexes
  size_t tSize=gvLocations.size();
  for (size_t tOuter=0; tOuter < tSize; ++tOuter) {
     for (size_t tInner=tOuter+1; tInner < tSize; ++tInner) {
        //since we sorted by the first coordinate pointer, we can say for sure that they do not equal if first is not equal
        if (gvLocations[tOuter]->getCoordinatesContainer()[0] != gvLocations[tInner]->getCoordinatesContainer()[0]) {
          //skip to next iterator in outer loop
          break;
        }
        //we know the first coordinate pointer is equal, but how about the rest, might be of different sizes
        if (gvLocations[tOuter]->getCoordinatesContainer() == gvLocations[tInner]->getCoordinatesContainer()) {
          if (gvLocations[tOuter]->getIndentifier() != gvLocations[tInner]->getIndentifier()) {
            //add identifier of itrNext to itrThis, they reference the same coordinate sets
            gvLocations[tOuter]->addSecondaryIdentifier(gvLocations[tInner]->getIndentifier());
            gmAggregateTracts[gvLocations[tInner]->getIndentifier()] = tOuter;
          }
          gvLocations.erase(gvLocations.begin() + tInner);
          tInner = tInner - 1;
          tSize = gvLocations.size();
        }
     }
  }
  //re-sort locations by identifiers - TractHandler::getLocationIndex() relies upon this container being sorted by identifiers
  std::sort(gvLocations.begin(), gvLocations.end(), CompareIdentifiers());
}

/** Insert a tract into internal structure, sorting by tract identifier. Ignores location ids
    which already exist. Returns tract identifers relative index into internal structure. */
tract_t TractHandler::addLocation(const char *sIdentifier) {
  ZdPointerVector<Location>::iterator itr;

  try {
    if (gAdditionStatus == Closed)
      ZdGenerateException("This TractHandler object is closed to insertions.", "addLocation()");

    if (gbAggregatingTracts) //when aggregating locations, insertion process always succeeds
      return 0;

    giMaxIdentifierLength = std::max(strlen(sIdentifier), giMaxIdentifierLength);
    std::auto_ptr<Location> identifier(new Location(sIdentifier, Coordinates()));
    itr = std::lower_bound(gvLocations.begin(), gvLocations.end(), identifier.get(), CompareIdentifiers());
    if (itr != gvLocations.end() && !strcmp((*itr)->getIndentifier().c_str(), sIdentifier))
      return std::distance(gvLocations.begin(), itr);
    return std::distance(gvLocations.begin(), gvLocations.insert(itr, identifier.release()));
  }
  catch (ZdException & x) {
    x.AddCallpath("addLocation()", "TractHandler");
    throw;
  }
}

/** Inserts a locations identifier into internal structures.  */
void TractHandler::addLocation(const char *sIdentifier, std::vector<double>& vCoordinates) {
  try {
    if (gAdditionStatus == Closed)
      ZdGenerateException("This TractHandler object is closed to insertions.", "addLocation()");

    if (gbAggregatingTracts) return; //when aggregating locations, insertion process always succeeds

    if (vCoordinates.size() != giCoordinateDimensions)
      ZdGenerateException("Coordinate dimension is %u, expected %d.", "addLocation()", vCoordinates.size(), giCoordinateDimensions);

    giMaxIdentifierLength = std::max(strlen(sIdentifier), giMaxIdentifierLength);
    //insert unique coordinates into collection - ordered by first coordinate, then second coordinate, etc.
    std::auto_ptr<Coordinates> pCoordinates(new Coordinates(vCoordinates, gvCoordinates.size()));
    ZdPointerVector<Coordinates>::iterator itrCoordinates;
    itrCoordinates = std::lower_bound(gvCoordinates.begin(), gvCoordinates.end(), pCoordinates.get(), CompareCoordinates());
    if (itrCoordinates == gvCoordinates.end() || *(pCoordinates.get()) != *(*itrCoordinates))
      itrCoordinates = gvCoordinates.insert(itrCoordinates, pCoordinates.release());

    //insert into location identifier structure - ordered by indentifer name
    ZdPointerVector<Location>::iterator itrIdentifiers;
    std::auto_ptr<Location> identifier(new Location(sIdentifier, *(*itrCoordinates)));
    itrIdentifiers = std::lower_bound(gvLocations.begin(), gvLocations.end(), identifier.get(), CompareIdentifiers());
    if (itrIdentifiers != gvLocations.end() && !strcmp((*itrIdentifiers)->getIndentifier().c_str(), sIdentifier))
      //when identifier already exists, then note that it is defined at this coordinate as well
      (*itrIdentifiers)->addCoordinates(*(*itrCoordinates), geMultipleCoordinatesType);
    else
      //otherwise this is a new identifier
      gvLocations.push_back(identifier.release());
  }
  catch (ZdException & x) {
    x.AddCallpath("addLocation()", "TractHandler");
    throw;
  }
}

/** Compute distance squared between two points. */
double TractHandler::getDistanceSquared(const std::vector<double>& vFirstPoint, const std::vector<double>& vSecondPoint) {
  double        dDistanceSquared=0;

  if (vFirstPoint.size() != vSecondPoint.size())
    ZdGenerateException("First point has %u coordinates and second point has %u.", "getDistanceSquared()", vFirstPoint.size(), vSecondPoint.size());

  for (size_t i=0; i < vFirstPoint.size(); ++i)
     dDistanceSquared += (vFirstPoint[i] - vSecondPoint[i]) * (vFirstPoint[i] - vSecondPoint[i]);

  return dDistanceSquared;
}

/** Searches for tract identifier and returns it's internal index, or -1 if not found.
    When aggregating locations, always return zero. */
tract_t TractHandler::getLocationIndex(const char *sIdentifier) const {
  tract_t  tPosReturn;

  try {
    if (gbAggregatingTracts)
      //when aggregation locations, all tract identifiers refer to the same index
      return 0;

    //first search collection of known aggregated location identifiers 
    std::map<std::string,tract_t>::const_iterator itrm = gmAggregateTracts.find(std::string(sIdentifier));
    if (itrm != gmAggregateTracts.end())
      return itrm->second;

    //search for tract identifier in vector
    ZdPointerVector<Location>::const_iterator   itr;
    std::auto_ptr<Location> identifier(new Location(sIdentifier, Coordinates()));
    itr = std::lower_bound(gvLocations.begin(), gvLocations.end(), identifier.get(), CompareIdentifiers());
    if (itr != gvLocations.end() && !strcmp((*itr)->getIndentifier().c_str(), sIdentifier))
      tPosReturn = std::distance(gvLocations.begin(), itr);
    else
      tPosReturn = -1;
  }
  catch (ZdException & x)  {
    x.AddCallpath("getLocationIndex()", "TractHandler");
    throw;
  }
  return tPosReturn;
}

/** Prints formatted message to file which details the locations of the coordinates
    file that had identical coordinates and where combined into one location
    for internal usage. */
void TractHandler::reportCombinedLocations(FILE * fDisplay) const {
  ZdPointerVector<Location>::const_iterator   itr;
  AsciiPrintFormat                            PrintFormat;
  bool                                        bPrinted=false;

  try {
    for (itr=gvLocations.begin(); itr != gvLocations.end(); ++itr) {
       if ((*itr)->getSecondaryIdentifiers().size()) {
         if (!bPrinted) {
           PrintFormat.SetMarginsAsOverviewSection();
           ZdString sBuffer = "\nNote: The coordinates file contains location IDs with identical "
                              "coordinates that where combined into one location. In the "
                              "optional output files, combined locations are represented by a "
                              "single location ID as follows:";
           PrintFormat.PrintAlignedMarginsDataString(fDisplay, sBuffer);
           PrintFormat.PrintSectionSeparatorString(fDisplay, 0, 1, '-');
           bPrinted=true;
         }
         //First retrieved location ID is the location that represents all others.
         ZdString  sBuffer;
         sBuffer.printf("%s : %s", (*itr)->getIndentifier().c_str(), (*itr)->getSecondaryIdentifiers()[0].c_str());
         for (size_t t=1; t < (*itr)->getSecondaryIdentifiers().size(); ++t)
            sBuffer << ", " << (*itr)->getSecondaryIdentifiers()[t].c_str();
         PrintFormat.PrintAlignedMarginsDataString(fDisplay, sBuffer);
       }
    }
  }
  catch (ZdException & x)  {
    x.AddCallpath("reportCombinedLocations()", "TractHandler");
    throw;
  }
}

/** Sets dimensions of location coordinates. If aggregating locations, function just returns; otherwise
    if any locations are already defined - throws an exception. */
void TractHandler::setCoordinateDimensions(size_t iDimensions) {
  if (gbAggregatingTracts) return; //ignore this when aggregating locations

  if (gvCoordinates.size())
    ZdGenerateException("Changing the coordinate dimensions is not permited once locations have been defined.","setCoordinateDimensions()");

  giCoordinateDimensions = iDimensions;
}

