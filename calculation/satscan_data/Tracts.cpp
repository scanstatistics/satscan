#include "SaTScan.h"
#pragma hdrstop
#include "Tracts.h"

/** Constructor */
CategoryDescriptor::CategoryDescriptor(int iPopulationDatesCount, int iCategoryIndex) : gpNextDescriptor(0), gpPopulationList(0) {
  try {
    Init();
    Setup(iPopulationDatesCount, iCategoryIndex);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()", "CategoryDescriptor");
    throw;
  }
}

/** Destructor */
CategoryDescriptor::~CategoryDescriptor() {
  try {
    delete gpNextDescriptor; gpNextDescriptor=0;
    delete[] gpPopulationList; gpPopulationList=0;
  }
  catch(...){}
}

void CategoryDescriptor::AddCaseCount(count_t tCaseCount) {
  try {
    if (gtCaseCount + tCaseCount < 0)
      SSGenerateException("Error: Attempt to add cases of '%d' to current cases of '%d' causes data overflow.",
                          "AddCaseCount()", tCaseCount, gtCaseCount);

    gtCaseCount += tCaseCount;
  }
  catch (ZdException &x) {
    x.AddCallpath("AddCaseCount()","CategoryDescriptor");
    throw;
  }
}

/** Adds population to existing population for date index. */
void CategoryDescriptor::AddPopulationAtDateIndex(float fPopluation, int iDateIndex, const TractHandler & theTractHandler) {
  try {
    if (0 > iDateIndex || iDateIndex > theTractHandler.tiGetNumPopDates() - 1)
      ZdGenerateException("Index %d out of range(0 - %d).","AddPopulationAtDateIndex()",
                          iDateIndex, theTractHandler.tiGetNumPopDates() -1);

    if (gpPopulationList[iDateIndex] + fPopluation < 0) {
      char      sDateString[20];

      SSGenerateException("Error: Attempt to add population of '%.2f' to current population of '%.2f' at date '%s' causes data overflow.",
                          "AddPopulationAtDateIndex()", fPopluation, gpPopulationList[iDateIndex],
                          JulianToChar(sDateString, theTractHandler.tiGetPopDate(iDateIndex)));
    }

    gpPopulationList[iDateIndex] += fPopluation;
  }
  catch (ZdException &x) {
    x.AddCallpath("AddPopulationAtDateIndex()","CategoryDescriptor");
    throw;
  }
}

/** Combines passed category descriptor data with this descriptor. Throws exception
    if category indexes are different. Does not bother next pointer.                */
void CategoryDescriptor::Combine(const CategoryDescriptor * pCategoryDescriptor, const TractHandler & theTractHandler) {
  int   i;

  try {
    if (! pCategoryDescriptor || giCategoryIndex != pCategoryDescriptor->giCategoryIndex)
      ZdGenerateException("Unable to combine category descriptors with different indexes(%d/%d).",
                          "Combine()", ZdException::Normal, giCategoryIndex, pCategoryDescriptor->giCategoryIndex);

    for (i=0; i < theTractHandler.tiGetNumPopDates(); i++)
      gpPopulationList[i] += pCategoryDescriptor->gpPopulationList[i];
    gtCaseCount += pCategoryDescriptor->gtCaseCount;
  }
  catch (ZdException &x) {
    x.AddCallpath("Combine()","CategoryDescriptor");
    throw;
  }
}

/** Returns population at date index. */
float CategoryDescriptor::GetPopulationAtDateIndex(int iDateIndex, const TractHandler & theTractHandler) const {
  try {
    if (0 > iDateIndex || iDateIndex > theTractHandler.tiGetNumPopDates() - 1)
      ZdGenerateException("Index %d out of range(0 - %d).","", ZdException::Normal, iDateIndex, theTractHandler.tiGetNumPopDates() - 1);
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPopulationAtDateIndex()","CategoryDescriptor");
    throw;
  }
  return gpPopulationList[iDateIndex];
}

void CategoryDescriptor::SetCaseCount(count_t tCaseCount) {
  try {
    if (tCaseCount < 0)
      ZdGenerateException("Error: Invalid case count '%d'.", "SetCaseCount()", tCaseCount);
      
    gtCaseCount = tCaseCount;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCaseCount()","CategoryDescriptor");
    throw;
  }
}

/** Allocates next CategoryDescriptor. Returns pointer to allocated object. */
CategoryDescriptor * CategoryDescriptor::SetNextDescriptor(int iPopulationListSize, int iCategoryIndex) {
  try {
    delete gpNextDescriptor; gpNextDescriptor=0;
    gpNextDescriptor = new CategoryDescriptor(iPopulationListSize, iCategoryIndex);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetNextDescriptor()","CategoryDescriptor");
    throw;
  }
  return gpNextDescriptor;
}

/** Sets population at date index. */
void CategoryDescriptor::SetPopulationAtDateIndex(float fPopluation, int iDateIndex, const TractHandler & theTractHandler) {
  try {
    if (0 > iDateIndex || iDateIndex > theTractHandler.tiGetNumPopDates() - 1)
      ZdGenerateException("Index %d out of range(0 - %d).","", ZdException::Normal, iDateIndex, theTractHandler.tiGetNumPopDates() - 1);
    gpPopulationList[iDateIndex] = fPopluation;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPopulationAtDateIndex()","CategoryDescriptor");
    throw;
  }
}

/** Allocates and intializes population list. */
void CategoryDescriptor::SetPopulationListSize(int iPopulationListSize) {
  try {
    if (gpPopulationList) {delete [] gpPopulationList; gpPopulationList=0;}
    gpPopulationList = new float[iPopulationListSize];
    memset(gpPopulationList, 0, iPopulationListSize * sizeof(long));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPopulationListSize()","CategoryDescriptor");
    delete[] gpPopulationList; gpPopulationList=0;
    throw;
  }
}

/** Internal setup function */
void CategoryDescriptor::Setup(int iPopulationListSize, int iCategoryIndex) {
  try {
    SetPopulationListSize(iPopulationListSize);
    SetCategoryIndex(iCategoryIndex);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CategoryDescriptor");
    throw;
  }
}

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
    delete gpCategoryDescriptorsList;
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

/** Combines passed tract descriptor with this descriptor. Combines all like categories
    and creates new categories in list for those that don't exist in this descriptor
    but do exist in passed descriptor. */
void TractDescriptor::Combine(const TractDescriptor * pTractDescriptor, const TractHandler & theTractHandler) {
  int                           i;
  const CategoryDescriptor    * p2;

  try {
    if (! pTractDescriptor)
      ZdGenerateException("Null pointer.","Combine()");

    AddTractIdentifier(pTractDescriptor->GetTractIdentifier());
    for (i=0; i < theTractHandler.tiGetNumCategories(); i++) {
       p2 = pTractDescriptor->GetCategoryDescriptor(i);
       if (p2)
         GetCategoryDescriptor(i, theTractHandler.tiGetNumPopDates()).Combine(p2, theTractHandler);
    }
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

/** Returns referance to category descriptor for tract with iCategoryIndex.
    If descriptor not found, a new node is created for category and appended to list. */
CategoryDescriptor & TractDescriptor::GetCategoryDescriptor(int iCategoryIndex, int iPopulationListSize) {
  CategoryDescriptor          * pCurrentDescriptor, * pPreviousDescriptor;
  bool                          bFound=false;

  try {
    if (! gpCategoryDescriptorsList) {
      gpCategoryDescriptorsList = new CategoryDescriptor(iPopulationListSize, iCategoryIndex);
      pCurrentDescriptor = gpCategoryDescriptorsList;
    }
    else {
      pCurrentDescriptor = gpCategoryDescriptorsList;
      while (pCurrentDescriptor && !bFound) {
           if (pCurrentDescriptor->GetCategoryIndex() == iCategoryIndex)
             bFound = true;
           else {
             pPreviousDescriptor = pCurrentDescriptor;
             pCurrentDescriptor = pPreviousDescriptor->GetNextDescriptor();
           }
      }

      if (! bFound)
        pCurrentDescriptor = pPreviousDescriptor->SetNextDescriptor(iPopulationListSize, iCategoryIndex);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetCategoryDescriptor()", "TractDescriptor");
    throw;
  }
  return *pCurrentDescriptor;
}

/** Returns pointer to category class with iCategoryIndex. Returns null pointer if not found. */
CategoryDescriptor * TractDescriptor::GetCategoryDescriptor(int iCategoryIndex) {
  CategoryDescriptor  * pCategoryDescriptor;
  bool                  bDone=false;

  pCategoryDescriptor = gpCategoryDescriptorsList;
  while (pCategoryDescriptor && !bDone) {
       if (pCategoryDescriptor->GetCategoryIndex() == iCategoryIndex)
         bDone = true;
       else
         pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();
  }
  return pCategoryDescriptor;
}

/** Returns const pointer to category class with iCategoryIndex. Returns null pointer if not found. */
const CategoryDescriptor * TractDescriptor::GetCategoryDescriptor(int iCategoryIndex) const {
  const CategoryDescriptor  * pCategoryDescriptor;
  bool                        bDone=false;

  pCategoryDescriptor = gpCategoryDescriptorsList;
  while (pCategoryDescriptor && !bDone) {
       if (pCategoryDescriptor->GetCategoryIndex() == iCategoryIndex)
         bDone = true;
       else
         pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();
  }
  return pCategoryDescriptor;
}

/** Returns pointer to category list head. */
CategoryDescriptor * TractDescriptor::GetCategoryDescriptorList() {
  return gpCategoryDescriptorsList;
}

/** Returns const pointer to category list head. */
const CategoryDescriptor * TractDescriptor::GetCategoryDescriptorList() const {
  return gpCategoryDescriptorsList;
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
TractHandler::TractHandler(const PopulationCategories & thePopulationCategories, BasePrint & PrintDirection) {
  Init();
  Setup(thePopulationCategories, PrintDirection);
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
  bStartAsPopDt = false;
  bEndAsPopDt   = false;
  nDimensions   = 0;
  gpSearchTractDescriptor=0;
  gpPopulationCategories = 0;
}

/** Adds a category to the tract info structure. */
void TractHandler::tiAddCategoryToTract(tract_t tTractIndex, int iCategoryIndex, Julian PopulationDate, float fPopulation) {
  try {
    if (0 > tTractIndex || tTractIndex > (tract_t)gvTractDescriptors.size() - 1 )
      ZdGenerateException("Index %d out of range(0 - %d).","tiAddCategoryToTract()",
                          ZdException::Normal, tTractIndex, gvTractDescriptors.size() - 1);

    CategoryDescriptor & thisDescriptor =
           gvTractDescriptors[tTractIndex]->GetCategoryDescriptor(iCategoryIndex, (int)gvPopulationDates.size());
    tiAssignPopulation(thisDescriptor, PopulationDate, fPopulation);
  }
  catch (ZdException & x) {
    x.AddCallpath("tiAddCategoryToTract()", "TractHandler");
    throw;
  }
}

///** Sets the case count for tract category.
//    Returns 1 if tract category found else 0. */
int TractHandler::tiAddCount(tract_t t, int iCategoryIndex, count_t Count) {
  int                      iReturn=0;
  CategoryDescriptor     * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
       ZdException::Generate("Index %d out of range(0 - %d)", "tiAddCount()", t, gvTractDescriptors.size() - 1);

   pCategoryDescriptor = gvTractDescriptors[t]->GetCategoryDescriptor(iCategoryIndex);
    if (pCategoryDescriptor) {
       iReturn = 1;
       pCategoryDescriptor->AddCaseCount(Count);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("tiAddCount()", "TractHandler");
    throw;
  }
  return iReturn;
}

/** Assigns a population count to the appropriate location in the population list.
    Note: An exception probably should be thrown here when population date index
          if not found, but previous code did not.
          Is this correct behavior?
          Should an exception be thrown or will that break previous program design? */
void TractHandler::tiAssignPopulation(CategoryDescriptor & thisCategoryDescriptor, Julian PopulationDate, float fPopulation) {
  int iPopulationDateIndex, iNumPopulationDates;

  try {
    iPopulationDateIndex = tiGetPopDateIndex(PopulationDate);
    if (iPopulationDateIndex != -1) {
      iNumPopulationDates = (int)gvPopulationDates.size();
      thisCategoryDescriptor.AddPopulationAtDateIndex(fPopulation, iPopulationDateIndex, *this);


      if (iPopulationDateIndex == 1 && bStartAsPopDt)
        thisCategoryDescriptor.AddPopulationAtDateIndex(fPopulation, 0L, *this);

      if (iPopulationDateIndex == iNumPopulationDates - 2 && bEndAsPopDt)
        thisCategoryDescriptor.AddPopulationAtDateIndex(fPopulation, iNumPopulationDates - 1, *this);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("tiAssignPopulation()", "TractHandler");
    throw;
  }
}

/** These calculations assumes that the population of a given day refers to the beginning of that day.
    Returns array that indicates population dates percentage of the whole study period.               */
void TractHandler::tiCalculateAlpha(double** pAlpha, Julian StartDate, Julian EndDate) const {
  int                   nPopDates = (int)gvPopulationDates.size();
  int                   n, N = nPopDates-2;
  long                  nTotalYears = TimeBetween(StartDate, EndDate, DAY)/*EndDate-StartDate*/ ;
  double                sumalpha;

   try {
     *pAlpha = (double*)Smalloc((nPopDates+1) * sizeof(double), gpPrintDirection);

     if (N==0) {
       (*pAlpha)[0] = 0.5*((gvPopulationDates[1]-StartDate)+(gvPopulationDates[1]-(EndDate+1)))/(double)(gvPopulationDates[1]-gvPopulationDates[0]);
       (*pAlpha)[1] = 0.5*((StartDate-gvPopulationDates[0])+((EndDate+1)-gvPopulationDates[0]))/(double)(gvPopulationDates[1]-gvPopulationDates[0]);
     }
     else if(N==1) {
       (*pAlpha)[0] = (0.5*((double)(gvPopulationDates[1]-StartDate)/(gvPopulationDates[1]-gvPopulationDates[0]))*(gvPopulationDates[1]-StartDate)) / (double)nTotalYears;
       (*pAlpha)[1] = (0.5*(gvPopulationDates[1]-StartDate)*(1+((double)(StartDate-gvPopulationDates[0])/(gvPopulationDates[1]-gvPopulationDates[0])))) / (double)nTotalYears
                       + (0.5*(double)(EndDate+1-gvPopulationDates[N])*(1+((double)(gvPopulationDates[N+1]-(EndDate+1))/(gvPopulationDates[N+1]-gvPopulationDates[N])))) /  (double)nTotalYears;
       (*pAlpha)[N+1] = (0.5*((double)(EndDate+1-gvPopulationDates[N])/(gvPopulationDates[N+1]-gvPopulationDates[N]))*(EndDate+1-gvPopulationDates[N])) / nTotalYears;
     }
     else {
       (*pAlpha)[0] = (0.5*((double)(gvPopulationDates[1]-StartDate)/(gvPopulationDates[1]-gvPopulationDates[0]))*(gvPopulationDates[1]-StartDate)) / (double)nTotalYears;
       (*pAlpha)[1] = (0.5*(gvPopulationDates[2]-gvPopulationDates[1]) + 0.5*(gvPopulationDates[1]-StartDate)*(1+((double)(StartDate-gvPopulationDates[0])/(gvPopulationDates[1]-gvPopulationDates[0])))) / (double)nTotalYears;
       for (n = 2; n < N; n++) {
         (*pAlpha)[n] = 0.5*(double)(gvPopulationDates[n+1] - gvPopulationDates[n-1]) / (double)nTotalYears;
       }
       (*pAlpha)[N]   = (0.5*(gvPopulationDates[N]-gvPopulationDates[N-1]) + 0.5*(double)(EndDate+1-gvPopulationDates[N])*(1+((double)(gvPopulationDates[N+1]-(EndDate+1))/(gvPopulationDates[N+1]-gvPopulationDates[N])))) /  (double)nTotalYears;
       (*pAlpha)[N+1] = (0.5*((double)(EndDate+1-gvPopulationDates[N])/(gvPopulationDates[N+1]-gvPopulationDates[N]))*(EndDate+1-gvPopulationDates[N])) / nTotalYears;
     }

#if 0 /* DEBUG */
     printf("\nTotal years = %ld, N=%ld\n", nTotalYears, N);
     gpPrintDirection->SatScanPrintf("Pop\nIndex   PopDates        Alpha\n");
     for (n=0;n<=N+1;n++) {
       JulianToChar(szDate,pPopDates[n]);
       gpPrintDirection->SatScanPrintf("%i       %s        %f\n",n, szDate, (*pAlpha)[n]);
     }
     gpPrintDirection->SatScanPrintf("\n");
#endif

     /* Bug check, seeing that alpha values add to one. */
     sumalpha = 0;
     for (n = 0; n <= N+1; n++) sumalpha = sumalpha + (*pAlpha)[n];
     if (sumalpha>1.0001 || sumalpha<0.9999)
       ZdGenerateException("Alpha values not calculated correctly.\nThe sum of the alpha values is %8.6lf rather than 1.\n",
                           "tiCalculateAlpha()", sumalpha);
  }
  catch (ZdException & x) {
    x.AddCallpath("tiCalculateAlpha()", "TractHandler");
    throw;
  }
}

/** Prints warning if there are tract categories with cases but no population.
    Throws exception if total population for tract is zero. */
void TractHandler::tiCheckCasesHavePopulations() const {
  int                           i, j, nPEndIndex, nPStartIndex = 0;
  const CategoryDescriptor *    pCategoryDescriptor;
  std::string                   sBuffer, sBuffer2;
  double                        dTractPopulation, dCategoryTotal;
  count_t                       iTractCaseCount;

  try {
    if (bStartAsPopDt)
      nPStartIndex = 1;
    if (bEndAsPopDt)
      nPEndIndex = (int)gvPopulationDates.size() - 2;
    else
      nPEndIndex = (int)gvPopulationDates.size() - 1;

    for (i=0; i < (int)gvTractDescriptors.size(); i++) {
       dTractPopulation = 0;
       iTractCaseCount = 0;
       pCategoryDescriptor = gvTractDescriptors[i]->GetCategoryDescriptorList();
       while (pCategoryDescriptor) {
          dCategoryTotal = 0;
          iTractCaseCount += pCategoryDescriptor->GetCaseCount();
          for (j=nPStartIndex; j <= nPEndIndex; j++)
             dCategoryTotal += pCategoryDescriptor->GetPopulationAtDateIndex(j, *this);
          dTractPopulation += dCategoryTotal;
          if (dCategoryTotal == 0 && pCategoryDescriptor->GetCaseCount() > 0) {
            if (gpPopulationCategories->GetNumPopulationCategories() > 1)
              //If there is only one population category, then this warning is redundant as the error
              //message below will be displayed with same information. So we only want to
              //show this warning if there is more than one covariate for this location.
              gpPrintDirection->SatScanPrintWarning("Warning: Tract %s  covariate %s has %d cases but zero population.\n",
                                gvTractDescriptors[i]->GetTractIdentifier(0, sBuffer),
                                gpPopulationCategories->GetPopulationCategoryAsString(pCategoryDescriptor->GetCategoryIndex(), sBuffer2),
                                pCategoryDescriptor->GetCaseCount());
          }
          pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();
       }

       if (dTractPopulation == 0 && iTractCaseCount > 0)
         SSGenerateException("Error: Total population is zero for tract %s but has %d cases.",
                             "tiCheckCasesHavePopulations()",
                             gvTractDescriptors[i]->GetTractIdentifier(0, sBuffer), iTractCaseCount);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("tiCheckCasesHavePopulations()", "TractHandler");
    throw;
  }
}

/** Check to see that no years have a total population of zero. */
bool TractHandler::tiCheckZeroPopulations(FILE *pDisplay) const {
  UInt                          month, day, year;
  bool                          bValid = true;
  float                       * PopTotalsArray = 0;
  int                           i, j, nPEndIndex, nPStartIndex = 0;
  const CategoryDescriptor    * pCategoryDescriptor;  

  try {
    if (bStartAsPopDt)
      nPStartIndex = 1;
    if (bEndAsPopDt)
      nPEndIndex = (int)gvPopulationDates.size()-2;
    else
      nPEndIndex = (int)gvPopulationDates.size()-1;

    PopTotalsArray = (float*)Smalloc((int)gvPopulationDates.size() *sizeof(float), gpPrintDirection);
    memset(PopTotalsArray, 0, (int)gvPopulationDates.size() * sizeof(float));

    for (i=0; i < (int)gvTractDescriptors.size(); i++) {
       pCategoryDescriptor = gvTractDescriptors[i]->GetCategoryDescriptorList();
       while (pCategoryDescriptor) {
          for (j=nPStartIndex; j <= nPEndIndex; j++)
             PopTotalsArray[j] += pCategoryDescriptor->GetPopulationAtDateIndex(j, *this);
          pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();
       }
    }

    for (j=nPStartIndex; j <= nPEndIndex; j++) {
       if (PopTotalsArray[j]==0) {
          bValid = false;
          JulianToMDY(&month, &day, &year, gvPopulationDates[j]);
          fprintf(pDisplay, "Error: Population of zero found for all tracts in %d.\n", year);
          gpPrintDirection->SatScanPrintWarning("Error: Population of zero found for all tracts in %d.\n", year);
       }
    }

    free (PopTotalsArray);
  }
  catch (ZdException & x) {
    free (PopTotalsArray);
    x.AddCallpath("tiCheckZeroPopulations()", "TractHandler");
    throw;
  }
  return bValid;
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

/** Determines which available population years should be used. */
void TractHandler::tiFindPopDatesToUse(std::vector<Julian>& PopulationDates, Julian StartDate, Julian EndDate,
                                       int* pnSourceOffset, int* pnDestOffset, bool* pbAddStart, bool* pbAddEnd,
                                       int* pnDatesUsed, int* pnPopDates) {
  int    n, nLastIndex, nDates;
  bool   bStartFound=false, bEndFound=false;

  try {
    *pnSourceOffset = 0;
    *pnDestOffset   = 0;
    *pbAddStart     = false;
    *pbAddEnd       = false;

    /* Determine which pop dates to use */
    nDates = (int)PopulationDates.size();
    for (n=0; n < nDates; n++) {
       if (!bStartFound) {
         if (PopulationDates[n] > StartDate) {
           bStartFound = true;
           if (n==0) {
             *pbAddStart = true;
             *pnDestOffset = 1;
           }
         }
         else
          *pnSourceOffset = n;
       }

       if (!bEndFound) {
         nLastIndex = n;
         if (PopulationDates[n] > EndDate)
           bEndFound = true;
       }
    }

    *pnDatesUsed = nLastIndex - *pnSourceOffset + 1;
    *pnPopDates  = *pnDatesUsed;

    if (*pbAddStart)
      *pnPopDates = *pnPopDates+1;

    if (!bEndFound && nLastIndex==(nDates-1)) {
      *pbAddEnd = true;
      *pnPopDates = *pnPopDates+1;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("tiFindPopDatesToUse()", "TractHandler");
    throw;
  }
}

/** Returns the population for category of tract between date intervals. */
double TractHandler::tiGetAlphaAdjustedPopulation(double & dPopulation, tract_t t, int iCategoryIndex,
                                                  int iStartPopulationDateIndex, int iEndPopulationDateIndex,
                                                  double Alpha[]) const {
  int                           j;
  const CategoryDescriptor    * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
      ZdException::Generate("Index %d out of range(0 - %d)", "tiGetAlphaAdjustedPopulation()", t, gvTractDescriptors.size() - 1);

    pCategoryDescriptor = gvTractDescriptors[t]->GetCategoryDescriptor(iCategoryIndex);
    if (pCategoryDescriptor) {
      for (j=iStartPopulationDateIndex; j < iEndPopulationDateIndex; j++)
         dPopulation = dPopulation + (Alpha[j] * pCategoryDescriptor->GetPopulationAtDateIndex(j, *this));
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetAlphaAdjustedPopulation()", "TractHandler");
    throw;
  }
  return dPopulation;
}

/** Returns the tract coords for the given tract_t index.
    Allocate memory for array - caller is responsible for freeing. */
void TractHandler::tiGetCoords(tract_t t, double** pCoords) const {
  try {
    *pCoords = (double*)Smalloc(nDimensions * sizeof(double), gpPrintDirection);

    if (0 <= t && t < (tract_t)gvTractDescriptors.size())
      gvTractDescriptors[t]->GetCoordinates(*pCoords, *this);
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetCoords()", "TractHandler");
    throw;
  }
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

/** Returns the case count for a given category in a given tract
    Returns 0 if category for tract does not exist.               */
count_t TractHandler::tiGetCount(tract_t t, int iCategoryIndex) const {
  count_t                       tValue=0;
  const CategoryDescriptor    * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
       ZdException::Generate("Index %d out of range(0 - %d)", "tiGetCount()", t, gvTractDescriptors.size() - 1);


   pCategoryDescriptor = gvTractDescriptors[t]->GetCategoryDescriptor(iCategoryIndex);
    if (pCategoryDescriptor)
      tValue = pCategoryDescriptor->GetCaseCount();
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetCount()", "TractHandler");
    throw;
  }
  return tValue;
}

/** Compute distance sqaured between 2 tracts. */
double TractHandler::tiGetDistanceSq(double* pCoords, double* pCoords2) const {
  int           i;
  double        dDistanceSquared=0;

  for (i=0; i < nDimensions; i++)
     dDistanceSquared += (pCoords[i] - pCoords2[i]) * (pCoords[i] - pCoords2[i]);

  return dDistanceSquared;
}

/** Returns the population date for a given index into the Pop date array. */
Julian TractHandler::tiGetPopDate(int iPopulationDateIndex) const {
  Julian        ReturnDate;

  try {
    //NOTE: This section of code is a bit strange. Julian is defined as unsigned long
    //      but here we are potentially assigning it a negative number. Only code in
    //      AssignMeasure() functions access this function currently. In return section
    //      , value as a Julian is compared against -1. 
    if (0 > iPopulationDateIndex || iPopulationDateIndex > (int)gvPopulationDates.size() - 1)
      ReturnDate = -1;
    else
      ReturnDate = gvPopulationDates[iPopulationDateIndex];
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetPop()", "TractHandler");
    throw;
  }
  return ReturnDate;
}

/** Returns the index into the Pop date array for a given date. */
int TractHandler::tiGetPopDateIndex(Julian Date) {
  int  i, iReturn=-1;

  try {
    for (i=0; i < tiGetNumPopDates() && iReturn == -1; i++)
       if (Date == gvPopulationDates[i])
         iReturn = i;
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetPopDateIndex()", "TractHandler");
    throw;
  }
  return iReturn;
} 

/** Returns the population for a given year and category in a given tract
    Returns 0 if no population for category.                              */
float TractHandler::tiGetPopulation(tract_t t, int iCategoryIndex, int iPopulationDateIndex) const {
  float                         fValue=0;
  const CategoryDescriptor    * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
       ZdException::Generate("Index %d out of range(0 - %d)", "tiGetPop()", t, gvTractDescriptors.size() - 1);

    pCategoryDescriptor = gvTractDescriptors[t]->GetCategoryDescriptor(iCategoryIndex);
    if (pCategoryDescriptor)
      fValue = pCategoryDescriptor->GetPopulationAtDateIndex(iPopulationDateIndex, *this);
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetPopulation()", "TractHandler");
    throw;
  }
  return fValue;
}

/** Returns the indeces to population dates that bound a given interval date. */
int TractHandler::tiGetPopUpLowIndex(Julian* pDates, int nDateIndex, int nMaxDateIndex,
                                     int* nUpIndex, int* nLowIndex) const {
  int   i, index;
  bool  bUpFound = false;

  try {
    /*  if (nDateIndex == nMaxDateIndex)
          return(0);
    */

    for (i=0; i < tiGetNumPopDates(); i++) {
       if (gvPopulationDates[i] <= pDates[nDateIndex])
         *nLowIndex = i;

       if (nDateIndex == nMaxDateIndex)
         index = nDateIndex;
       else
         index = nDateIndex+1;

       if (!bUpFound && (gvPopulationDates[i] >= pDates[index])) {
         bUpFound = true;
         *nUpIndex = i;
       }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetPopUpLowIndex()", "TractHandler");
    throw;
  }
  return 1;
}

/** Returns the population for population date index of tract for all categories. */
double TractHandler::tiGetRiskAdjustedPopulation(measure_t & dMeanPopulation, tract_t t, int iPopulationDateIndex, double Risk[]) const {
  const CategoryDescriptor    * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
      ZdException::Generate("Index %d out of range(0 - %d)", "tiGetRiskAdjustedPopulation()", t, gvTractDescriptors.size() - 1);

    dMeanPopulation = 0.0;
    pCategoryDescriptor = gvTractDescriptors[t]->GetCategoryDescriptorList();
    while (pCategoryDescriptor) {
         dMeanPopulation = dMeanPopulation + Risk[pCategoryDescriptor->GetCategoryIndex()] *
                              pCategoryDescriptor->GetPopulationAtDateIndex(iPopulationDateIndex, *this);
         pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();               
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("tiGetRiskAdjustedPopulation()", "TractHandler");
    throw;
  }
  return dMeanPopulation;
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
tract_t TractHandler::tiGetTractIndex(const char *tid) {
  ZdPointerVector<TractDescriptor>::iterator           itr;
  std::map<std::string,TractDescriptor*>::iterator     itrmap;
  tract_t                                              tPosReturn;

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
      SSGenerateException("Error: Tract %s is specified multiple times in geographical file.", "tiInsertTnode()", tid);
    else {//search for tract identifier in vector
      gpSearchTractDescriptor->SetTractIdentifier(tid);
      itrPosition = lower_bound(gvTractDescriptors.begin(), gvTractDescriptors.end(), gpSearchTractDescriptor, CompareTractDescriptorIdentifier());
      if (itrPosition != gvTractDescriptors.end() && !strcmp((*itrPosition)->GetTractIdentifier(),tid))
        SSGenerateException("Error: Tract %s is specified multiple times in geographical file.", "tiInsertTnode()", tid);
    }

    //check that coordinates are not duplicate
    for (itrCoordinates=gvTractDescriptors.begin(); itrCoordinates != gvTractDescriptors.end() && !bDuplicate; itrCoordinates++)
       if ((*itrCoordinates)->CompareCoordinates(reinterpret_cast<double*>(vCoordinates.begin()), nDimensions)) {
         gmDuplicateTracts[tid] = (*itrCoordinates);
         bDuplicate = true;
       }

    if (! bDuplicate) {
      pTractDescriptor = new TractDescriptor(tid, reinterpret_cast<double*>(vCoordinates.begin()), nDimensions);
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

/** Prints indication, to file, of tracts that had identical coordinates which
    where combined into one tract. */
void TractHandler::tiReportDuplicateTracts(FILE * fDisplay) const {
  std::map<std::string,TractDescriptor*>::const_iterator       itr;
  std::vector<TractDescriptor*>                                vTractsDescriptors;
  std::vector<TractDescriptor*>::iterator                      tract_itr;
  std::vector<std::string>                                     vTractIdentifiers;
  size_t                                                       t;
  const char * sSeperator = "---------------------------------------------------------------";
  unsigned int                                                 iPos, iIdLength, iIndent, iMaxWidth;

  try {
    if (gmDuplicateTracts.size()) {
      fprintf(fDisplay, "\nNote: The geographical file contains locations with matching\n");
      fprintf(fDisplay, "coordinates that where combined into one location.\n");
      fprintf(fDisplay, "When creating additional outputs, combined locations are\n");
      fprintf(fDisplay, "treated as one location.\n");
      fprintf(fDisplay, sSeperator);
      iMaxWidth = strlen(sSeperator);

      for (itr=gmDuplicateTracts.begin(); itr != gmDuplicateTracts.end(); itr++)
         if (std::find(vTractsDescriptors.begin(), vTractsDescriptors.end(), itr->second) == vTractsDescriptors.end())
           vTractsDescriptors.push_back(itr->second);

      for (tract_itr=vTractsDescriptors.begin(); tract_itr != vTractsDescriptors.end(); tract_itr++) {
         (*tract_itr)->GetTractIdentifiers(vTractIdentifiers);
         for (t=0; t < vTractIdentifiers.size(); t++) {
            if (t == 0) {
              fprintf(fDisplay, "\n%s : ", vTractIdentifiers[t].c_str());
              iPos = strlen(vTractIdentifiers[t].c_str()) + 3/*other characters*/;
              iIndent = iPos;
            }
            else if (t < vTractIdentifiers.size() - 1) {
              iIdLength = strlen(vTractIdentifiers[t].c_str()) + 1/*comma*/;
              if (iPos  + iIdLength > iMaxWidth) {
                fprintf(fDisplay, "\n");
                iPos = 0;
                while (++iPos <= iIndent)
                     fprintf(fDisplay, " ");
                iPos = iIdLength + 2/*two spaces*/;
              }
              else
                iPos += iIdLength;
              fprintf(fDisplay, "%s, ", vTractIdentifiers[t].c_str());
            }
            else {
               if (iPos + 2/*other characters*/ + strlen(vTractIdentifiers[t].c_str()) > iMaxWidth) {
                fprintf(fDisplay, "\n");
                iPos = 0;
                while (++iPos <= iIndent)
                     fprintf(fDisplay, " ");
               }
              fprintf(fDisplay, "%s", vTractIdentifiers[t].c_str());
            }
         }
      }
      fprintf(fDisplay, "\n");
    }
  }
  catch (ZdException & x)  {
    x.AddCallpath("tiReportDuplicateTracts()", "TractHandler");
    throw;
  }
}

/** Look for and display tracts with zero population for any population year. */
void TractHandler::tiReportZeroPops(FILE *pDisplay) const {
  int                           i, j, nPEndIndex, nPStartIndex = 0;
  UInt                          month, day, year;
  bool                          bZeroFound = false;
  float                       * PopTotalsArray = 0;
  std::string                   sBuffer;
  char                          sDateBuffer[20];
  const CategoryDescriptor    * pCategoryDescriptor;

  try {
    if (bStartAsPopDt)
      nPStartIndex = 1;
    if (bEndAsPopDt)
      nPEndIndex = tiGetNumPopDates() - 2;
    else
      nPEndIndex = tiGetNumPopDates() - 1;

    PopTotalsArray = (float*)Smalloc(tiGetNumPopDates() *sizeof(float), gpPrintDirection);

    for (i=0; i < (int)gvTractDescriptors.size(); i++) {
       memset(PopTotalsArray, 0, tiGetNumPopDates() * sizeof(float));
       pCategoryDescriptor = gvTractDescriptors[i]->GetCategoryDescriptorList();
       while (pCategoryDescriptor) {
          for (j=nPStartIndex; j <= nPEndIndex; j++)
             PopTotalsArray[j] += pCategoryDescriptor->GetPopulationAtDateIndex(j, *this);
          pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();   
       }

       for (j=nPStartIndex; j <= nPEndIndex; j++) {
          if (PopTotalsArray[j]==0) {
            if (!bZeroFound) {
              bZeroFound = true;
              fprintf(pDisplay,"\n________________________________________________________________\n\n");
              fprintf(pDisplay,"Warning: According to the input data, the following tracts have a \n");
              fprintf(pDisplay,"         population totaling zero for the specified date(s).\n\n");
              gpPrintDirection->SatScanPrintWarning("\n________________________________________________________________\n\n");
              gpPrintDirection->SatScanPrintWarning("Warning: According to the input data, the following tracts have a \n");
              gpPrintDirection->SatScanPrintWarning("         population totaling zero for the specified date(s).\n\n");
            }
            //JulianToMDY(&month, &day, &year, gvPopulationDates[j]);
            JulianToChar(sDateBuffer, gvPopulationDates[j]);
            fprintf(pDisplay,"         Tract %s, %s\n", gvTractDescriptors[i]->GetTractIdentifier(0, sBuffer), sDateBuffer);
            gpPrintDirection->SatScanPrintWarning("         Tract %s, %s\n", gvTractDescriptors[i]->GetTractIdentifier(0, sBuffer), sDateBuffer);
          }
       }
    }

    free (PopTotalsArray);
  }
  catch (ZdException & x) {
    free (PopTotalsArray);
    x.AddCallpath("tiReportZeroPops()", "TractHandler");
    throw;
  }
}

///** Sets the case count for tract category.
//    Returns 1 if tract category found else 0. */
int TractHandler::tiSetCount(tract_t t, int iCategoryIndex, count_t Count) {
  int                      iReturn=0;
  CategoryDescriptor     * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gvTractDescriptors.size() - 1)
       ZdException::Generate("Index %d out of range(0 - %d)", "tiSetCount()", t, gvTractDescriptors.size() - 1);

   pCategoryDescriptor = gvTractDescriptors[t]->GetCategoryDescriptor(iCategoryIndex);
    if (pCategoryDescriptor) {
       iReturn = 1;
       pCategoryDescriptor->SetCaseCount(Count);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("tiSetCount()", "TractHandler");
    throw;
  }
  return iReturn;
}

/** Internal setup function. */
void TractHandler::Setup(const PopulationCategories & thePopulationCategories, BasePrint & PrintDirection) {
  double Coordinates[1] ={0}; 

  try {
    gpPrintDirection = &PrintDirection;
    gpSearchTractDescriptor = new TractDescriptor(" ", Coordinates, 1);
    gpPopulationCategories = &thePopulationCategories;
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()", "TractHandler");
    delete gpSearchTractDescriptor; gpSearchTractDescriptor=0;
    throw;
  }
}

/** Initializes the Population dates vector. */
void TractHandler::tiSetupPopDates(std::vector<Julian>& PopulationDates, Julian StartDate, Julian EndDate) {
  int  n, nSourceOffset, nDestOffset, nDatesUsed, nPopDates;

  try {
    tiFindPopDatesToUse(PopulationDates, StartDate, EndDate,
                        &nSourceOffset, &nDestOffset, &bStartAsPopDt, &bEndAsPopDt,
                        &nDatesUsed, &nPopDates);

    gvPopulationDates.resize(nPopDates);

    if (bStartAsPopDt)
      gvPopulationDates[0] = StartDate;

    for (n=0; n<nDatesUsed; n++)
       gvPopulationDates[n + nDestOffset] = PopulationDates[n + nSourceOffset];

    if (bEndAsPopDt)
      gvPopulationDates[nPopDates-1] = EndDate+1;

/* debug */
#if 0
       DisplayDatesArray(pDates, nDates, "Array of Potential Pop Dates", stdout);

       JulianToChar(szDate, StartDate);
      gpPrintDirection->SatScanPrintf("\nStart Date = %s\n", szDate);
       JulianToChar(szDate, EndDate);
      gpPrintDirection->SatScanPrintf("End Date   = %s\n", szDate);
     
      gpPrintDirection->SatScanPrintf("\nSource offset      = %i\n", nSourceOffset);
      gpPrintDirection->SatScanPrintf("Destination offset = %i\n", nDestOffset);
      gpPrintDirection->SatScanPrintf("Add Start          = %i\n", bStartAsPopDt);
      gpPrintDirection->SatScanPrintf("Add End            = %i\n", bEndAsPopDt);
      gpPrintDirection->SatScanPrintf("Number Dates Used  = %i\n", nDatesUsed);
      gpPrintDirection->SatScanPrintf("Number Pop Dates   = %i\n", nPopDates);
      gpPrintDirection->SatScanPrintf("\n");
     
      gpPrintDirection->SatScanPrintf("<Press any key to continue>");
       c = getc(stdin);
     
       DisplayDatesArray(pPopDates, nPopDates, "Array of Selected Pop Dates", stdout);
#endif
      }
  catch (ZdException & x) {
    x.AddCallpath("tiSetupPopDates()", "TractHandler");
    throw;
  }
}
