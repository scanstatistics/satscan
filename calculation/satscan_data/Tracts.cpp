//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "Tracts.h"
#include "SaTScanData.h"
#include "AsciiPrintFormat.h"
#include "SSException.h"

/** Constructor */
TractDescriptor::TractDescriptor(const char * sTractIdentifier, const double* pCoordinates, int iDimensions)
                :gsTractIdentifiers(0), gpCoordinates(0) {
  try {
    Setup(sTractIdentifier, pCoordinates, iDimensions);
  }
  catch (ZdException &x) {
    x.AddCallpath("TractDescriptor()","TractDescriptor");
    throw;
  }
}

/** constructor -- no coordinates */
TractDescriptor::TractDescriptor(const char * sTractIdentifier)
                :gsTractIdentifiers(0), gpCoordinates(0) {
  try {
    SetTractIdentifier(sTractIdentifier);
  }
  catch (ZdException &x) {
    x.AddCallpath("TractDescriptor()","TractDescriptor");
    throw;
  }
}

/** Destructor */
TractDescriptor::~TractDescriptor() {
  try {
    delete[] gsTractIdentifiers;
    delete[] gpCoordinates;
  }
  catch(...){}
}

/** Adds additional tract identifier for tract at coordinates.
    Multiple identifiers are provided by tab delimiting. */
void TractDescriptor::AddTractIdentifier(const char * sTractIdentifier) {
  try {
    ZdString   sTemporary(gsTractIdentifiers);

    sTemporary << '\t' << sTractIdentifier;
    delete[] gsTractIdentifiers; gsTractIdentifiers=0;
    gsTractIdentifiers = new char[sTemporary.GetLength() + 1];
    strcpy(gsTractIdentifiers, sTemporary.GetCString());
  }
  catch (ZdException &x) {
    x.AddCallpath("AddTractIdentifier()", "TractDescriptor");
    throw;
  }
}

/** Combines passed tract descriptor with this descriptor. */
void TractDescriptor::Combine(const TractDescriptor * pTractDescriptor, const TractHandler & theTractHandler) {
  try {
    if (! pTractDescriptor)
      ZdGenerateException("Null pointer.","Combine()");

    AddTractIdentifier(pTractDescriptor->GetTractIdentifier());
  }
  catch (ZdException &x) {
    x.AddCallpath("Combine()", "TractDescriptor");
    throw;
  }
}

/** Returns whether coordinate are equal. */
bool TractDescriptor::CompareCoordinates(const TractDescriptor & Descriptor, const TractHandler & theTractHandler) const {
  return !memcmp(GetCoordinates(), Descriptor.GetCoordinates(), theTractHandler.tiGetDimensions() * sizeof(double));
}

/** Returns whether coordinate are equal. */
bool TractDescriptor::CompareCoordinates(const double * pCoordinates, int iDimensions) const {
  return !memcmp(GetCoordinates(), pCoordinates, iDimensions * sizeof(double));
}

/** Get coordinates of tract. */
void TractDescriptor::RetrieveCoordinates(TractHandler const & theTractHandler, std::vector<double> & vRepository) const {
  vRepository.resize(theTractHandler.tiGetDimensions());
  std::copy(gpCoordinates, gpCoordinates + theTractHandler.tiGetDimensions(), vRepository.begin());
}

/** Returns coordinate at dimension. */
double TractDescriptor::GetCoordinatesAtDimension(int iDimension, const TractHandler & theTractHandler) const {
  try {
    if (0 > iDimension || iDimension > theTractHandler.tiGetDimensions() - 1)
      ZdGenerateException("Index out of range [size=%d].", "GetCoordinatesAtDimension()", theTractHandler.tiGetDimensions());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetCoordinatesAtDimension()", "TractDescriptor");
    throw;
  }
  return gpCoordinates[iDimension];
}

/** Returns number of tract identifiers. */
int TractDescriptor::GetNumTractIdentifiers() const {
  int   iNumIdentifiers;

  try {
    ZdStringTokenizer Tokenizer(gsTractIdentifiers, "\t");

    iNumIdentifiers = (int)Tokenizer.GetNumTokens();
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNumTractIdentifiers()","TractDescriptor");
    throw;
  }
  return iNumIdentifiers;
}

/** Returns indexed tract identifier. */
const char * TractDescriptor::GetTractIdentifier(int iTractIdentifierIndex, std::string & sIndentifier) {
  try {
    ZdStringTokenizer Tokenizer(gsTractIdentifiers, "\t");

    if (0 > iTractIdentifierIndex || iTractIdentifierIndex > (int)Tokenizer.GetNumTokens() - 1)
      ZdGenerateException("Index %d out of range [size=%d].","GetTractIdentifier()", ZdException::Normal,
                          iTractIdentifierIndex, Tokenizer.GetNumTokens());

    sIndentifier = Tokenizer.GetToken(iTractIdentifierIndex).GetCString();
  }
  catch (ZdException &x) {
    x.AddCallpath("GetTractIdentifier()", "TractDescriptor");
    throw;
  }
  return sIndentifier.c_str();
}

/** Returns all tract identifiers. */
void TractDescriptor::GetTractIdentifiers(std::vector<std::string>& vIdentifiers) const {
  try {
    vIdentifiers.clear();
    ZdStringTokenizer Tokenizer(gsTractIdentifiers, "\t");
    while (Tokenizer.HasMoreTokens())
         vIdentifiers.push_back(Tokenizer.GetNextToken().GetCString());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetTractIdentifier()", "TractDescriptor");
    throw;
  }
}

/** Set tract coordinates. */
void TractDescriptor::SetCoordinates(const double* pCoordinates, int iDimensions) {
  try {
    if (! pCoordinates)
      ZdGenerateException("Null pointer.","SetCoordinates(const double*,int)");

    if (gpCoordinates) {delete[] gpCoordinates; gpCoordinates=0;}
    gpCoordinates = new double[iDimensions];
    memcpy(gpCoordinates, pCoordinates, iDimensions * sizeof(double));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCoordinates()","TractDescriptor");
    delete[] gpCoordinates; gpCoordinates=0;
    throw;
  }
}

/** Sets tract identifier. Clears previous identifier settings. */
void TractDescriptor::SetTractIdentifier(const char * sTractIdentifier) {
  try {
    if (! sTractIdentifier)
      ZdGenerateException("Null pointer.","SetTractIdentifier()");

    if (gsTractIdentifiers) {delete[] gsTractIdentifiers; gsTractIdentifiers=0;}
    gsTractIdentifiers = new char[strlen(sTractIdentifier) + 1];
    strcpy(gsTractIdentifiers, sTractIdentifier);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetTractIdentifier()","TractDescriptor");
    delete[] gsTractIdentifiers; gsTractIdentifiers=0;
    throw;
  }
}

/** Internal setup function */
void TractDescriptor::Setup(const char * sTractIdentifier, const double* pCoordinates, int iDimensions) {
  try {
    SetTractIdentifier(sTractIdentifier);
    SetCoordinates(pCoordinates, iDimensions);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","TractDescriptor");
    throw;
  }
}

/** Constructor*/
TractHandler::TractHandler(bool bAggregatingTracts) : nDimensions(0), gbAggregatingTracts(bAggregatingTracts) {
  if (gbAggregatingTracts) gvTractDescriptors.push_back(new TractDescriptor("dummy_location"));
}

/** Destructor */
TractHandler::~TractHandler() {}

/** Combines tract identifiers for tracts that mapped to same coordinates.
    Note that this function should be called once after data files are read. */
void TractHandler::tiConcaticateDuplicateTractIdentifiers() {
  std::map<std::string,TractDescriptor*>::iterator     itrmap;

  try {
    for (itrmap=gmDuplicateTracts.begin(); itrmap != gmDuplicateTracts.end(); itrmap++)
       itrmap->second->AddTractIdentifier(itrmap->first.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("tiConcaticateDuplicateTractIdentifiers()", "TractHandler");
    throw;
  }
}

/**
Get the tract coords for the given tract_t index.
*/
void TractHandler::tiRetrieveCoords(tract_t t, std::vector<double> & vRepository) const {
  if ((t < 0) || (t >= (tract_t)gvTractDescriptors.size()))
    ZdException::Generate("index, %d, is out of bounds: [0, %d].", "TractHandler", t, gvTractDescriptors.size() - 1);
  gvTractDescriptors[t]->RetrieveCoordinates(*this, vRepository);
}

/** Compute distance squared between 2 tracts. */
double TractHandler::tiGetDistanceSq(const std::vector<double>& vFirstPoint, const std::vector<double>& vSecondPoint) {
  double        dDistanceSquared=0;

  if (vFirstPoint.size() != vSecondPoint.size())
    ZdGenerateException("First point has %u coordinates and second point has %u.", "tiGetDistanceSq()", vFirstPoint.size(), vSecondPoint.size());

  for (size_t i=0; i < vFirstPoint.size(); ++i)
     dDistanceSquared += (vFirstPoint[i] - vSecondPoint[i]) * (vFirstPoint[i] - vSecondPoint[i]);

  return dDistanceSquared;
}

/** Returns first identifier for tract at index. */
const char * TractHandler::tiGetTid(tract_t t, std::string& sFirst) const {
  try {
    if (gbAggregatingTracts)
      //when aggregation locations, retrieve the location identifier for dummy location
      // - this method is not really useful in this sense but it method is potentially called
      //   when a problem is detected in the algorithm and a program error is generated.
      gvTractDescriptors.back()->GetTractIdentifier(0, sFirst);
    else {
      if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
        ZdException::Generate("Index %d out of range [size=%u]", "tiGetTid()", t, gvTractDescriptors.size());

      gvTractDescriptors[t]->GetTractIdentifier(0, sFirst);
    }  
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetTid()", "TractHandler");
    throw;
  }
  return sFirst.c_str();
}


/** Returns coordinate for tract at specified dimension. */
double TractHandler::tiGetTractCoordinate(tract_t t, int iDimension) const {
  double        dReturn;

  try {
    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
       ZdException::Generate("Index %d out of range [size=%u]", "tiGetTractCoordinate()", t, gvTractDescriptors.size());

    dReturn = gvTractDescriptors[t]->GetCoordinatesAtDimension(iDimension, *this);
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetTractCoordinate()", "TractHandler");
    throw;
  }
  return dReturn;
}

/** Retrieves all location identifiers associated with location at index 't'. */
void TractHandler::tiGetTractIdentifiers(tract_t t, std::vector<std::string>& vIdentifiers) const {
  try {
    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
       ZdException::Generate("Index %d out of range [size=%u]", "tiGetTid()", t, gvTractDescriptors.size());

    gvTractDescriptors[t]->GetTractIdentifiers(vIdentifiers);
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetTractIdentifiers()", "TractHandler");
    throw;
  }
}

/** Searches for tract identifier and returns it's internal index, or -1 if not found. */
tract_t TractHandler::tiGetTractIndex(const char *tid) const {
  ZdPointerVector<TractDescriptor>::const_iterator           itr;
  std::map<std::string,TractDescriptor*>::const_iterator     itrmap;
  tract_t                                                    tPosReturn;

  try {
    if (gbAggregatingTracts)
      //when aggregation locations, all tract identifiers refer to the same index
      return 0;
    
    //check for tract identifier is duplicates map
    itrmap = gmDuplicateTracts.find(std::string(tid));
    if (itrmap != gmDuplicateTracts.end()) {// if found, return position of descriptor in vector
      itr = std::find(gvTractDescriptors.begin(), gvTractDescriptors.end(), itrmap->second);
      tPosReturn =  std::distance(gvTractDescriptors.begin(), itr);
    }
    else {//search for tract identifier in vector
      double Coordinates[1] ={0};
      std::auto_ptr<TractDescriptor> Search(new TractDescriptor(tid, Coordinates, 1));
      itr = lower_bound(gvTractDescriptors.begin(), gvTractDescriptors.end(), Search.get(), CompareTractDescriptorIdentifier());
      if (itr != gvTractDescriptors.end() && !strcmp((*itr)->GetTractIdentifier(),tid))
        tPosReturn = std::distance(gvTractDescriptors.begin(), itr);
      else
        tPosReturn = -1;
    }
  }
  catch (ZdException & x)  {
    x.AddCallpath("tiGetTractIndex()", "TractHandler");
    throw;
  }
  return tPosReturn;
}

/** Insert a tract into internal structure, sorting by tract identifier. Ignores location ids
    which already exist. Returns tract identifers relative index into internal structure. */
tract_t TractHandler::tiInsertTnode(const char *tid) {
  ZdPointerVector<TractDescriptor>::iterator itrPosition;

  try {
    if (gbAggregatingTracts) //when aggregating locations, insertion process always succeeds
      return 0;

    std::auto_ptr<TractDescriptor> Tract(new TractDescriptor(tid));
    itrPosition = lower_bound(gvTractDescriptors.begin(), gvTractDescriptors.end(), Tract.get(), CompareTractDescriptorIdentifier());
    if (itrPosition != gvTractDescriptors.end() && !strcmp((*itrPosition)->GetTractIdentifier(),tid))
      return std::distance(gvTractDescriptors.begin(), itrPosition);
    return std::distance(gvTractDescriptors.begin(), gvTractDescriptors.insert(itrPosition, Tract.release()));
  }
  catch (ZdException & x) {
    x.AddCallpath("tiInsertTnode()", "TractHandler");
    throw;
  }
}

/** Insert a tract into the vector sorting by coordinates. Ignores exact duplicates and
    groups different location ids with the same coordinates. */
void TractHandler::tiInsertTnode(const char *tid, std::vector<double>& vCoordinates) {
  ZdPointerVector<TractDescriptor>::iterator itr, itrPosition;

  try {
    if (gbAggregatingTracts)
      //when aggregating locations, insertion process always succeeds
      return;

    //search for location with these coordinates
    std::auto_ptr<TractDescriptor> Tract(new TractDescriptor(tid, &vCoordinates[0], nDimensions));
    //find insertion point based upon first coordinate
    itr = itrPosition = lower_bound(gvTractDescriptors.begin(), gvTractDescriptors.end(), Tract.get(), CompareTractFirstCoordinate());
    while (itr != gvTractDescriptors.end() && (*itr)->GetCoordinates()[0] == vCoordinates[0]) {
        //there exists a location with same X/latitude coordinate -- are there any identical points?
        if ((*itr)->CompareCoordinates(&vCoordinates[0], nDimensions)) {
           //is the identifier the same -- then this location record is a duplicate -- ignore it
           if (!strcmp((*itr)->GetTractIdentifier(),tid))
             return;
           //different identifiers, so this location references a coordinate already associated with another identifier
           gmDuplicateTracts[tid] = (*itr);
           return;
        }
        ++itr;
    }
    gvTractDescriptors.insert(itrPosition, Tract.release());
  }
  catch (ZdException & x) {
    x.AddCallpath("tiInsertTnode()", "TractHandler");
    throw;
  }
}

/** Prints formatted message to file which details the locations of the coordinates
    file that had identical coordinates and where combined into one location
    for internal usage. */
void TractHandler::tiReportDuplicateTracts(FILE * fDisplay) const {
  std::map<std::string,TractDescriptor*>::const_iterator       itr;
  std::vector<TractDescriptor*>                                vTractsDescriptors;
  std::vector<TractDescriptor*>::iterator                      tract_itr;
  std::vector<std::string>                                     vTractIdentifiers;
  size_t                                                       t;
  AsciiPrintFormat                                             PrintFormat;
  ZdString                                                     sBuffer; 

  try {
    if (gmDuplicateTracts.size()) {
      PrintFormat.SetMarginsAsOverviewSection();
      sBuffer = "\nNote: The coordinates file contains location IDs with identical "
                "coordinates that where combined into one location. In the "
                "optional output files, combined locations are represented by a "
                "single location ID as follows:";
      PrintFormat.PrintAlignedMarginsDataString(fDisplay, sBuffer);
      PrintFormat.PrintSectionSeparatorString(fDisplay, 0, 1, '-');

      //Collect all unique TractDescriptor objects - gmDuplicateTracts stores
      //location ID to object pointer.
      for (itr=gmDuplicateTracts.begin(); itr != gmDuplicateTracts.end(); itr++)
         if (std::find(vTractsDescriptors.begin(), vTractsDescriptors.end(), itr->second) == vTractsDescriptors.end())
           vTractsDescriptors.push_back(itr->second);
      //Print statement detailing primary Location ID, as well as the other
      //locations that were combined with primary. 
      for (tract_itr=vTractsDescriptors.begin(); tract_itr != vTractsDescriptors.end(); tract_itr++) {
         (*tract_itr)->GetTractIdentifiers(vTractIdentifiers);
         //First retrieved location ID is the location that represents all others.
         sBuffer.printf("%s : %s", vTractIdentifiers[0].c_str(), vTractIdentifiers[1].c_str());
         for (t=2; t < vTractIdentifiers.size(); ++t)
            sBuffer << ", " << vTractIdentifiers[t].c_str();
         PrintFormat.PrintAlignedMarginsDataString(fDisplay, sBuffer);
      }
    }
  }
  catch (ZdException & x)  {
    x.AddCallpath("tiReportDuplicateTracts()", "TractHandler");
    throw;
  }
}

/** Orders internal collection of TractDescriptor objects by indentifier labels. Since the insertion
    process did not prevent insertion of duplicate location identifiers, we'll do that now.

    The internal collection of locations needs to be sorted by their identifiers to maintain
    consistancy of output.                                                                             */
void TractHandler::SortTractsByIndentifiers() {
  if (gvTractDescriptors.size() < 2) return;
  //order locations by tract identifier
  std::sort(gvTractDescriptors.begin(), gvTractDescriptors.end(), CompareTractDescriptorIdentifier());
  //search TractDescriptor objects for duplicate location identifiers
  ZdPointerVector<TractDescriptor>::iterator itr=gvTractDescriptors.begin(), itr_end=gvTractDescriptors.end() - 1;
  for (; itr != itr_end; ++itr) {
    //compare current identifier with next identifier
    if (!strcmp((*itr)->GetTractIdentifier(),(*(itr+1))->GetTractIdentifier()))
      GenerateResolvableException("Error: The coordinates for location ID '%s' are defined multiple times in the coordinates file.", "tiInsertTnode()", (*itr)->GetTractIdentifier());
    //search for this indentifier in duplicates -- if many duplicates, this could slow things down
    if (gmDuplicateTracts.find(std::string((*itr)->GetTractIdentifier())) != gmDuplicateTracts.end())
      GenerateResolvableException("Error: The coordinates for location ID '%s' are defined multiple times in the coordinates file.", "tiInsertTnode()", (*itr)->GetTractIdentifier());
  }
}

/** Sets dimensions of location coordinates. If aggregating locations, function just returns; otherwise
    if any locations are already defined - throws an exception. */
void TractHandler::tiSetCoordinateDimensions(int iDimensions) {
  if (gbAggregatingTracts) return; //ignore this when aggregating locations

  if (gvTractDescriptors.size())
    ZdGenerateException("Changing the coordinate dimensions is not permited once locations have been defined.","tiSetCoordinateDimensions()");

  nDimensions = iDimensions;
}

