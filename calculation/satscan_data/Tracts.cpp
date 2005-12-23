//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "Tracts.h"
#include "SaTScanData.h"
#include "AsciiPrintFormat.h"
#include "SSException.h"

/** Constructor */
TractDescriptor::TractDescriptor(const char * sTractIdentifier, const double* pCoordinates, int iDimensions) {
  try {
    Init();
    Setup(sTractIdentifier, pCoordinates, iDimensions);
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

/** Returns coordinates of tract. */
double * TractDescriptor::GetCoordinates(double* pCoordinates, const TractHandler & theTractHandler) const {
  try {
    if (! pCoordinates)
      ZdGenerateException("Null pointer.","GetCoordinates(double*)");

    memcpy(pCoordinates, gpCoordinates, theTractHandler.tiGetDimensions() * sizeof(double));
  }
  catch (ZdException &x) {
    x.AddCallpath("GetCoordinates()","TractDescriptor");
    throw;
  }
  return pCoordinates;
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
      ZdGenerateException("Index %d out of range(0 - %d).","", ZdException::Normal, iDimension, theTractHandler.tiGetDimensions() - 1);
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
      ZdGenerateException("Index %d out of range(0 - %d).","GetTractIdentifier()", ZdException::Normal,
                          iTractIdentifierIndex, (int)Tokenizer.GetNumTokens() - 1);

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
TractHandler::TractHandler() {
  Init();
  Setup();
}

/** Destructor */
TractHandler::~TractHandler() {
  try {
    delete gpSearchTractDescriptor;
  }
  catch(...){}
}

/** Internal initialization. */
void TractHandler::Init() {
  nDimensions   = 0;
  gpSearchTractDescriptor=0;
}

/** Prints error when duplicate coordinates are found.
    Returns whether duplicates coordinate where found. */
tract_t TractHandler::tiCombineDuplicatesByCoordinates() {
  ZdPointerVector<TractDescriptor>::iterator         itrMajor, itrMinor;

  try {
    itrMajor = gvTractDescriptors.begin();
    while (itrMajor != gvTractDescriptors.end()) {
         itrMinor = itrMajor;
         itrMinor++;
         while (itrMinor != gvTractDescriptors.end()) {
              if ((*itrMajor)->CompareCoordinates(*(*itrMinor), *this)) {
                (*itrMajor)->Combine((*itrMinor), *this);
                itrMinor = gvTractDescriptors.erase(itrMinor);
              }
              else
                itrMinor++;
         }
         itrMajor++;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("tiCombineDuplicatesByCoordinates()", "TractHandler");
    throw;
  }
  return (tract_t)gvTractDescriptors.size();
}

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

/** Returns the tract coords for the given tract_t index.
    Allocate memory for array - caller is responsible for freeing. */
void TractHandler::tiGetCoords(tract_t t, double** pCoords) const {
  try {
    *pCoords = (double*)Smalloc(nDimensions * sizeof(double));
    if (0 <= t && t < (tract_t)gvTractDescriptors.size())
      gvTractDescriptors[t]->GetCoordinates(*pCoords, *this);
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetCoords()", "TractHandler");
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

/** Returns the tract coords for the given tract_t index. */
void TractHandler::tiGetCoords2(tract_t t, double* pCoords) const {
  try {
    if (0 <= t || t < (tract_t)gvTractDescriptors.size())
      gvTractDescriptors[t]->GetCoordinates(pCoords, *this);
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetCoords2()", "TractHandler");
    throw;
  }
}

/** Compute distance squared between 2 tracts. */
double TractHandler::tiGetDistanceSq(double* pCoords, double* pCoords2) const {
  int           i;
  double        dDistanceSquared=0;

  for (i=0; i < nDimensions; i++)
     dDistanceSquared += (pCoords[i] - pCoords2[i]) * (pCoords[i] - pCoords2[i]);

  return dDistanceSquared;
}

/** Returns first identifier for tract. */
const char * TractHandler::tiGetTid(tract_t t, std::string& sFirst) const {
  try {
    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
      ZdException::Generate("Index %d out of range(0 - %d)", "tiGetTid()", t, gvTractDescriptors.size() - 1);

    gvTractDescriptors[t]->GetTractIdentifier(0, sFirst);
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetTid()", "TractHandler");
    throw;
  }
  return sFirst.c_str();
}


///** Returns first tract name (tid)for the given tract_t index. */
//const char * TractHandler::tiGetTid(tract_t t) const {
//  try {
//    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
//      ZdException::Generate("Index %d out of range(0 - %d)", "tiGetTid()", t, gvTractDescriptors.size() - 1);
//  }
//  catch (ZdException & x) {
//    x.AddCallpath("tiGetTid()", "TractHandler");
//    throw;
//  }
// return gvTractDescriptors[t]->GetTractIdentifier();
//}

/** Returns coordinate for tract at specified dimension. */
double TractHandler::tiGetTractCoordinate(tract_t t, int iDimension) const {
  double        dReturn;

  try {
    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
       ZdException::Generate("Index %d out of range(0 - %d)", "tiGetTractCoordinate()", t, gvTractDescriptors.size() - 1);

    dReturn = gvTractDescriptors[t]->GetCoordinatesAtDimension(iDimension, *this);
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetTractCoordinate()", "TractHandler");
    throw;
  }
  return dReturn;
}

void TractHandler::tiGetTractIdentifiers(tract_t t, std::vector<std::string>& vIdentifiers) const {
  try {
    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
       ZdException::Generate("Index %d out of range(0 - %d)", "tiGetTid()", t, gvTractDescriptors.size() - 1);

    gvTractDescriptors[t]->GetTractIdentifiers(vIdentifiers);
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetTractIdentifiers()", "TractHandler");
    throw;
  }
}

/** Searches tract-id "tid".  Returns the index, or -1 if not found. */
tract_t TractHandler::tiGetTractIndex(const char *tid) const {
  ZdPointerVector<TractDescriptor>::const_iterator           itr;
  std::map<std::string,TractDescriptor*>::const_iterator     itrmap;
  tract_t                                                    tPosReturn;

  try {
    //check for tract identifier is duplicates map
    itrmap = gmDuplicateTracts.find(std::string(tid));
    if (itrmap != gmDuplicateTracts.end()) {// if found, return position of descriptor in vector
      itr = std::find(gvTractDescriptors.begin(), gvTractDescriptors.end(), itrmap->second);
      tPosReturn =  std::distance(gvTractDescriptors.begin(), itr);
    }
    else {//search for tract identifier in vector
      gpSearchTractDescriptor->SetTractIdentifier(tid);
      itr = lower_bound(gvTractDescriptors.begin(), gvTractDescriptors.end(), gpSearchTractDescriptor, CompareTractDescriptorIdentifier());
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

/** Insert a tract into the vector sorting by tract identifier.
    Sorted insert appears to be done solely for TractHandler::tiGetTractIndex(char *tid).

    Return value: 0 = duplicate tract ID 1 = success */
int TractHandler::tiInsertTnode(const char *tid, std::vector<double>& vCoordinates) {
  std::map<std::string,TractDescriptor*>::iterator     itrmap;
  ZdPointerVector<TractDescriptor>::iterator           itrCoordinates, itrPosition;
  TractDescriptor                                    * pTractDescriptor=0;
  bool                                                 bDuplicate=false;

  try {
    //check for tract identifier is duplicates map
    itrmap = gmDuplicateTracts.find(std::string(tid));
    if (itrmap != gmDuplicateTracts.end())
      GenerateResolvableException("Error: Location ID '%s' is specified multiple times in the coordinates file.", "tiInsertTnode()", tid);
    else {//search for tract identifier in vector
      gpSearchTractDescriptor->SetTractIdentifier(tid);
      itrPosition = lower_bound(gvTractDescriptors.begin(), gvTractDescriptors.end(), gpSearchTractDescriptor, CompareTractDescriptorIdentifier());
      if (itrPosition != gvTractDescriptors.end() && !strcmp((*itrPosition)->GetTractIdentifier(),tid))
        GenerateResolvableException("Error: Location ID '%s' is specified multiple times in the coordinates file.", "tiInsertTnode()", tid);
    }

    //check that coordinates are not duplicate
    for (itrCoordinates=gvTractDescriptors.begin(); itrCoordinates != gvTractDescriptors.end() && !bDuplicate; itrCoordinates++)
       if ((*itrCoordinates)->CompareCoordinates(&vCoordinates[0], nDimensions)) {
         gmDuplicateTracts[tid] = (*itrCoordinates);
         bDuplicate = true;
       }

    if (! bDuplicate) {
      pTractDescriptor = new TractDescriptor(tid, &vCoordinates[0], nDimensions);
      gvTractDescriptors.insert(itrPosition, pTractDescriptor);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("tiInsertTnode()", "TractHandler");
    delete pTractDescriptor;
    throw;
  }
  return(1);
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
      sBuffer = "Note: The coordinates file contains location IDs with identical "
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

/** Internal setup function. */
void TractHandler::Setup() {
  double Coordinates[1] ={0};

  try {
    gpSearchTractDescriptor = new TractDescriptor(" ", Coordinates, 1);
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()", "TractHandler");
    delete gpSearchTractDescriptor; gpSearchTractDescriptor=0;
    throw;
  }
}
