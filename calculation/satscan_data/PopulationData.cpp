#include "SaTScan.h"
#pragma hdrstop
#include "PopulationData.h"
#include "UtilityFunctions.h"
#include "SaTScanData.h"

/** Constructor */
PopulationCategory::PopulationCategory(int iPopulationDatesCount, int iCategoryIndex) : gpNextDescriptor(0), gpPopulationList(0) {
  try {
    Init();
    Setup(iPopulationDatesCount, iCategoryIndex);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()", "PopulationCategory");
    throw;
  }
}

/** Destructor */
PopulationCategory::~PopulationCategory() {
  try {
    delete gpNextDescriptor; gpNextDescriptor=0;
    delete[] gpPopulationList; gpPopulationList=0;
  }
  catch(...){}
}

/** Adds population to existing population for date index. */
void PopulationCategory::AddPopulationAtDateIndex(float fPopluation, unsigned int iDateIndex, const PopulationData & thePopulation) {
  try {
    if (iDateIndex > thePopulation.GetNumPopulationDates() - 1)
      ZdGenerateException("Index %d out of range(0 - %d).","AddPopulationAtDateIndex()",
                          iDateIndex, thePopulation.GetNumPopulationDates() -1);

    if (gpPopulationList[iDateIndex] + fPopluation < 0) {
      char      sDateString[20];

      SSGenerateException("Error: Attempt to add population of '%.2f' to current population of '%.2f' at date '%s' causes data overflow.",
                          "AddPopulationAtDateIndex()", fPopluation, gpPopulationList[iDateIndex],
                          JulianToChar(sDateString, thePopulation.GetPopulationDate(iDateIndex)));
    }

    gpPopulationList[iDateIndex] += fPopluation;
  }
  catch (ZdException &x) {
    x.AddCallpath("AddPopulationAtDateIndex()","PopulationCategory");
    throw;
  }
}

/** Returns population at date index. */
float PopulationCategory::GetPopulationAtDateIndex(unsigned int iDateIndex, const PopulationData & thePopulation) const {
  try {
    if (iDateIndex > thePopulation.GetNumPopulationDates() - 1)
      ZdGenerateException("Index %d out of range(0 - %d).","", ZdException::Normal, iDateIndex, thePopulation.GetNumPopulationDates() - 1);
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPopulationAtDateIndex()","PopulationCategory");
    throw;
  }
  return gpPopulationList[iDateIndex];
}

/** Allocates next PopulationCategory. Returns pointer to allocated object. */
PopulationCategory * PopulationCategory::SetNextDescriptor(int iPopulationListSize, int iCategoryIndex) {
  try {
    delete gpNextDescriptor; gpNextDescriptor=0;
    gpNextDescriptor = new PopulationCategory(iPopulationListSize, iCategoryIndex);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetNextDescriptor()","PopulationCategory");
    throw;
  }
  return gpNextDescriptor;
}

/** Sets population at date index. */
void PopulationCategory::SetPopulationAtDateIndex(float fPopluation, unsigned int iDateIndex, const PopulationData & thePopulation) {
  try {
    if (iDateIndex > thePopulation.GetNumPopulationDates() - 1)
      ZdGenerateException("Index %d out of range(0 - %d).","", ZdException::Normal, iDateIndex, thePopulation.GetNumPopulationDates() - 1);
    gpPopulationList[iDateIndex] = fPopluation;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPopulationAtDateIndex()","PopulationCategory");
    throw;
  }
}

/** Allocates and intializes population list. */
void PopulationCategory::SetPopulationListSize(int iPopulationListSize) {
  try {
    if (gpPopulationList) {delete [] gpPopulationList; gpPopulationList=0;}
    gpPopulationList = new float[iPopulationListSize];
    memset(gpPopulationList, 0, iPopulationListSize * sizeof(long));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPopulationListSize()","PopulationCategory");
    delete[] gpPopulationList; gpPopulationList=0;
    throw;
  }
}

/** Internal setup function */
void PopulationCategory::Setup(int iPopulationListSize, int iCategoryIndex) {
  try {
    SetPopulationListSize(iPopulationListSize);
    SetCategoryIndex(iCategoryIndex);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","PopulationCategory");
    throw;
  }
}


/** constructor */
PopulationData::PopulationData()  {
  Init();
}

/** destructor */
PopulationData::~PopulationData() {
  try {
    for (size_t t=0; t < gTractCategories.size(); ++t)
       delete gTractCategories[t];
  }
  catch (...){}
}

void PopulationData::AddCaseCount(int iCategoryIndex, count_t Count) {
  if (iCategoryIndex < 0 || iCategoryIndex > (int)gvCategoryCasesCount.size() - 1)
    ZdGenerateException("Index '%d' out of range.","AddCaseCount()", iCategoryIndex);
  gvCategoryCasesCount[iCategoryIndex] += Count;
  if (gvCategoryCasesCount[iCategoryIndex] < 0)
    SSGenerateException("Error: Total cases greater than maximum allowed of %ld.\n", "AddCaseCount()", std::numeric_limits<count_t>::max());
}

/** Adds a category to the tract info structure. */
void PopulationData::AddCategoryToTract(tract_t tTractIndex, unsigned int iCategoryIndex, Julian PopulationDate, float fPopulation) {
  try {
    if (0 > tTractIndex || tTractIndex > (tract_t)gTractCategories.size() - 1 )
      ZdGenerateException("Index %d out of range(0 - %d).","AddPopulation()",
                          ZdException::Normal, tTractIndex, gTractCategories.size() - 1);

    PopulationCategory & thisDescriptor = GetPopulationCategory(tTractIndex, iCategoryIndex, (int)gvPopulationDates.size());
    AssignPopulation(thisDescriptor, PopulationDate, fPopulation);
  }
  catch (ZdException &x) {
    x.AddCallpath("AddCategoryToTract()","PopulationData");
    throw;
  }
}

void PopulationData::AddControlCount(int iCategoryIndex, count_t Count) {
  if (iCategoryIndex < 0 || iCategoryIndex > (int)gvCategoryControlsCount.size() - 1)
    ZdGenerateException("Index '%d' out of range.","AddControlCount()", iCategoryIndex);
  gvCategoryControlsCount[iCategoryIndex] += Count;
  if (gvCategoryControlsCount[iCategoryIndex] < 0)
    SSGenerateException("Error: Total controls greater than maximum allowed of %ld.\n", "AddControlCount()", std::numeric_limits<count_t>::max());
}

/** Assigns a population count to the appropriate location in the population list.
    Note: An exception probably should be thrown here when population date index
          if not found, but previous code did not.
          Is this correct behavior?
          Should an exception be thrown or will that break previous program design? */
void PopulationData::AssignPopulation(PopulationCategory & thisPopulationCategory, Julian PopulationDate, float fPopulation) {
  int iPopulationDateIndex, iNumPopulationDates;

  try {
    iPopulationDateIndex = GetPopulationDateIndex(PopulationDate);
    if (iPopulationDateIndex != -1) {
      iNumPopulationDates = (int)gvPopulationDates.size();
      thisPopulationCategory.AddPopulationAtDateIndex(fPopulation, iPopulationDateIndex, *this);


      if (iPopulationDateIndex == 1 && gbStartAsPopDt)
        thisPopulationCategory.AddPopulationAtDateIndex(fPopulation, 0L, *this);

      if (iPopulationDateIndex == iNumPopulationDates - 2 && gbEndAsPopDt)
        thisPopulationCategory.AddPopulationAtDateIndex(fPopulation, iNumPopulationDates - 1, *this);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("AssignPopulation()","PopulationData");
    throw;
  }
}

/** These calculations assumes that the population of a given day refers to the beginning of that day.
    Returns array that indicates population dates percentage of the whole study period.               */
void PopulationData::CalculateAlpha(double** pAlpha, Julian StartDate, Julian EndDate) const {
  int                   nPopDates = (int)gvPopulationDates.size();
  int                   n, N = nPopDates-2;
  long                  nTotalYears = TimeBetween(StartDate, EndDate, DAY)/*EndDate-StartDate*/ ;
  double                sumalpha;

   try {
     *pAlpha = (double*)Smalloc((nPopDates+1) * sizeof(double), 0);

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
                           "CalculateAlpha()", sumalpha);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateAlpha()","PopulationData");
    throw;
  }
}

/** Prints warning if there are tract categories with cases but no population.
    Throws exception if total population for tract is zero. */
void PopulationData::CheckCasesHavePopulations(const count_t * pCases, CSaTScanData & Data) const {
  int                           i, j, nPEndIndex, nPStartIndex = 0;
  const PopulationCategory *    pCategoryDescriptor;
  std::string                   sBuffer, sBuffer2;
  double                        dTractPopulation, dCategoryTotal;
  count_t                       iTractCaseCount;
//  count_t                     * pCases(Data.GetCasesArray()[0]);

  try {
    if (gbStartAsPopDt)
      nPStartIndex = 1;
    if (gbEndAsPopDt)
      nPEndIndex = (int)gvPopulationDates.size() - 2;
    else
      nPEndIndex = (int)gvPopulationDates.size() - 1;

    //NOTE: Because of the design error with reading the case file, the tract handler
    //      class no longer records number of cases for each tract/category. So this
    //      check has been removed until that code is updated. 
    for (i=0; i < (int)gTractCategories.size(); i++) {
       dTractPopulation = 0;
       //iTractCaseCount = 0;
       pCategoryDescriptor = gTractCategories[i];
       while (pCategoryDescriptor) {
          dCategoryTotal = 0;
          //iTractCaseCount += pCategoryDescriptor->GetCaseCount();
          for (j=nPStartIndex; j <= nPEndIndex; j++)
             dCategoryTotal += pCategoryDescriptor->GetPopulationAtDateIndex(j, *this);
          dTractPopulation += dCategoryTotal;
          //if (dCategoryTotal == 0 && pCategoryDescriptor->GetCaseCount() > 0) {
          //  if (gpPopulation->GetNumPopulation() > 1)
          //    //If there is only one population category, then this warning is redundant as the error
          //    //message below will be displayed with same information. So we only want to
          //    //show this warning if there is more than one covariate for this location.
          //    gpPrintDirection->SatScanPrintWarning("Warning: Tract %s  covariate %s has %d cases but zero population.\n",
          //                     gvTractDescriptors[i]->GetTractIdentifier(0, sBuffer),
          //                      gpPopulationCategories->GetPopulationCategoryAsString(pCategoryDescriptor->GetCategoryIndex(), sBuffer2),
          //                      pCategoryDescriptor->GetCaseCount());
          //}
          pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();
       }

       if (dTractPopulation == 0 && pCases[i] > 0)
         SSGenerateException("Error: Total population is zero for tract %s but has %d cases.",
                             "CheckCasesHavePopulations()",
                             Data.GetTInfo()->tiGetTid(i, sBuffer), pCases[i]);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CheckCasesHavePopulations()","PopulationData");
    throw;
  }
}

/** Check to see that no years have a total population of zero. */
bool PopulationData::CheckZeroPopulations(FILE *pDisplay, BasePrint * pPrintDirection) const {
  UInt                          month, day, year;
  bool                          bValid = true;
  float                       * PopTotalsArray = 0;
  int                           i, j, nPEndIndex, nPStartIndex = 0;
  const PopulationCategory    * pCategoryDescriptor;  

  try {
    if (gbStartAsPopDt)
      nPStartIndex = 1;
    if (gbEndAsPopDt)
      nPEndIndex = (int)gvPopulationDates.size()-2;
    else
      nPEndIndex = (int)gvPopulationDates.size()-1;

    PopTotalsArray = (float*)Smalloc((int)gvPopulationDates.size() *sizeof(float), 0);
    memset(PopTotalsArray, 0, (int)gvPopulationDates.size() * sizeof(float));

    for (i=0; i < (int)gTractCategories.size(); i++) {
       pCategoryDescriptor = gTractCategories[i];
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
          pPrintDirection->SatScanPrintWarning("Error: Population of zero found for all tracts in %d.\n", year);
       }
    }

    free (PopTotalsArray);
  }
  catch (ZdException &x) {
    free (PopTotalsArray);
    x.AddCallpath("CheckZeroPopulations()","PopulationData");
    throw;
  }
  return bValid;
}

/** Prints formatted text depicting state of population categories. */
void PopulationData::Display(BasePrint & PrintDirection) const {
  size_t        t, j;

  try {
    PrintDirection.SatScanPrintf("DISPLAY: Number of categories = %i\n", gvPopulationCategories.size());
    PrintDirection.SatScanPrintf("\n#   Category Combination\n");
    for (t=0; t < gvPopulationCategories.size(); t++) {
       PrintDirection.SatScanPrintf("%d     ",  t);
       for (j=0; j < gvPopulationCategories[t].size(); j++)
          PrintDirection.SatScanPrintf("%s  ", gvCovariateNames[gvPopulationCategories[t][j]].c_str());
       PrintDirection.SatScanPrintf("\n");
    }
    PrintDirection.SatScanPrintf("\n");
  }
  catch (ZdException &x) {
    x.AddCallpath("Display()", "PopulationData");
    throw;
  }
}

/** Determines which available population years should be used. */
void PopulationData::FindPopDatesToUse(std::vector<Julian>& PopulationDates, Julian StartDate, Julian EndDate,
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
  catch (ZdException &x) {
    x.AddCallpath("FindPopDatesToUse()","PopulationData");
    throw;
  }
}

/** Returns the population for category of tract between date intervals. */
double PopulationData::GetAlphaAdjustedPopulation(double & dPopulation, tract_t t,
                                                        int iCategoryIndex, int iStartPopulationDateIndex,
                                                        int iEndPopulationDateIndex, double Alpha[]) {
  int                           j;
  const PopulationCategory    * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gTractCategories.size() - 1)
      ZdException::Generate("Index %d out of range(0 - %d)", "GetAlphaAdjustedPopulation()", t, gTractCategories.size() - 1);

    pCategoryDescriptor = GetCategoryDescriptor(t, iCategoryIndex);
    if (pCategoryDescriptor) {
      for (j=iStartPopulationDateIndex; j < iEndPopulationDateIndex; j++)
         dPopulation = dPopulation + (Alpha[j] * pCategoryDescriptor->GetPopulationAtDateIndex(j, *this));
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetAlphaAdjustedPopulation()","PopulationData");
    throw;
  }
  return dPopulation;
}

/** Finds the date of the largest PopDate < Date. If none exist return -1. */
int PopulationData::LowerPopIndex(Julian Date) const {
  int i;

  if (GetPopulationDate(0) >= Date )
    return -1;

  i=0;
  while (GetPopulationDate(i+1) < Date)
       i++;
  return i;
}

count_t PopulationData::GetNumCategoryCases(int iCategoryIndex) const {
  try {
    if (iCategoryIndex < 0 || iCategoryIndex > static_cast<int>(gvCategoryCasesCount.size()) - 1)
      ZdGenerateException("Index '%d' out of ranges.","GetNumCategoryCases()");

    return gvCategoryCasesCount[iCategoryIndex];
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNumCategoryCases()", "PopulationData");
    throw;
  }
}

count_t PopulationData::GetNumCategoryControls(int iCategoryIndex) const {
  try {
    if (iCategoryIndex < 0 || iCategoryIndex > static_cast<int>(gvCategoryControlsCount.size()) - 1)
      ZdGenerateException("Index '%d' out of ranges.","GetNumCategoryControls()");

    return gvCategoryControlsCount[iCategoryIndex];
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNumCategoryControls()", "PopulationData");
    throw;
  }
}

/** Returns pointer to category class with iCategoryIndex. Returns null pointer if not found. */
PopulationCategory * PopulationData::GetCategoryDescriptor(tract_t tTractIndex, unsigned int iCategoryIndex) {
  PopulationCategory  * pCategoryDescriptor;
  bool                  bDone=false;

  pCategoryDescriptor = gTractCategories[tTractIndex];
  while (pCategoryDescriptor && !bDone) {
       if (pCategoryDescriptor->GetCategoryIndex() == iCategoryIndex)
         bDone = true;
       else
         pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();
  }
  return pCategoryDescriptor;
}

/** Returns the population for a given year and category in a given tract
    Returns 0 if no population for category.                              */
float PopulationData::GetPopulation(tract_t t, int iCategoryIndex, int iPopulationDateIndex){
  float                         fValue=0;
  const PopulationCategory    * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gTractCategories.size() - 1)
       ZdException::Generate("Index %d out of range(0 - %d)", "tiGetPop()", t, gTractCategories.size() - 1);

    pCategoryDescriptor = GetCategoryDescriptor(t, iCategoryIndex);
    if (pCategoryDescriptor)
      fValue = pCategoryDescriptor->GetPopulationAtDateIndex(iPopulationDateIndex, *this);
  }
  catch (ZdException &x) {
    x.AddCallpath("tiGetPopulation()","PopulationData");
    throw;
  }
  return fValue;
}

/** Returns referance to category descriptor for tract with iCategoryIndex.
    If descriptor not found, a new node is created for category and appended to list. */
PopulationCategory & PopulationData::GetPopulationCategory(tract_t tTractIndex, unsigned int iCategoryIndex, int iPopulationListSize) {
  PopulationCategory          * pCurrentDescriptor, * pPreviousDescriptor;
  bool                          bFound=false;

  try {
    if (!gTractCategories[tTractIndex]) {
      gTractCategories[tTractIndex] = new PopulationCategory(iPopulationListSize, iCategoryIndex);
      pCurrentDescriptor = gTractCategories[tTractIndex];
    }
    else {
      pCurrentDescriptor = gTractCategories[tTractIndex];
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
    x.AddCallpath("GetPopulationCategory()","PopulationData");
    throw;
  }
  return *pCurrentDescriptor;
}

/** Returns category index for passed vector of covariate strings. Returns -1 if not found. */
int PopulationData::GetPopulationCategoryIndex(const std::vector<std::string>& vCategoryCovariates) const {
  int           iIndex=-1;
  size_t        t, j;
  bool          bMatch;

  for (t=0; t < gvPopulationCategories.size() && iIndex == -1; t++) {
     for (j=0, bMatch=true; j < gvPopulationCategories[t].size() && bMatch; j++)
        bMatch = gvCovariateNames[gvPopulationCategories[t][j]] == vCategoryCovariates[j];
     if (bMatch)
       iIndex = t;
  }
  return iIndex;
}

/** Returns population category as string of space delimited covariates. */
const char * PopulationData::GetPopulationCategoryAsString(int iCategoryIndex, std::string & sBuffer) const {
  size_t        t;

  try {
    if (iCategoryIndex < 0 || iCategoryIndex > (int)(gvPopulationCategories.size() - 1))
      ZdException::Generate("Index out of range '%d', range is 0 - %d.", "GetPopulationCategoryAsString()",
                            iCategoryIndex, gvPopulationCategories.size() - 1);

    sBuffer = "";
    for (t=0; t < gvPopulationCategories[iCategoryIndex].size(); t++) {
       if (t > 0)
          sBuffer += " ";
       sBuffer += gvCovariateNames[gvPopulationCategories[iCategoryIndex][t]];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPopulationCategoryAsString()", "PopulationData");
    throw;
  }
  return sBuffer.c_str();
}

/** Returns the population date for a given index into the Pop date array. */
Julian PopulationData::GetPopulationDate(int iPopulationDateIndex) const {
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
  catch (ZdException &x) {
    x.AddCallpath("GetPopulationDate()","PopulationData");
    throw;
  }
  return ReturnDate;
}

/** Returns the index into the Pop date array for a given date. */
int PopulationData::GetPopulationDateIndex(Julian Date) const {
  int           i, iReturn = - 1;
  bool          bFound=false;

  try {
    for (i=0; i < (int)gvPopulationDates.size() && !bFound; ++i)
       if (Date == gvPopulationDates[i]) {
         iReturn = i;
         bFound = true;
       }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPopulationDateIndex()","PopulationData");
    throw;
  }
  return iReturn; /** was returning -1 if not found !!!!!*/
} 

/** Returns the indeces to population dates that bound a given interval date. */
int PopulationData::GetPopUpLowIndex(Julian* pDates, int nDateIndex, int nMaxDateIndex,
                                           int* nUpIndex, int* nLowIndex) const {
  int   i, index;
  bool  bUpFound = false;

  try {
    /*  if (nDateIndex == nMaxDateIndex)
          return(0);
    */

    for (i=0; i < (int)gvPopulationDates.size(); ++i) {
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
  catch (ZdException &x) {
    x.AddCallpath("tiGetPopUpLowIndex()","PopulationData");
    throw;
  }
  return 1;
}

/** Returns the population for population date index of tract for all categories. */
double PopulationData::GetRiskAdjustedPopulation(measure_t & dMeanPopulation, tract_t t, int iPopulationDateIndex, double Risk[]) const {
  const PopulationCategory    * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gTractCategories.size() - 1)
      ZdException::Generate("Index %d out of range(0 - %d)", "GetRiskAdjustedPopulation()", t, gTractCategories.size() - 1);

    dMeanPopulation = 0.0;
    pCategoryDescriptor = gTractCategories[t];
    while (pCategoryDescriptor) {
         dMeanPopulation = dMeanPopulation + Risk[pCategoryDescriptor->GetCategoryIndex()] *
                              pCategoryDescriptor->GetPopulationAtDateIndex(iPopulationDateIndex, *this);
         pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();               
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetRiskAdjustedPopulation()","PopulationData");
    throw;
  }
  return dMeanPopulation;
}

/** Creates new population category and returns category index. */
int PopulationData::MakePopulationCategory(const char* szDescription, StringParser & Parser, int iScanOffset, BasePrint & PrintDirection) {
  int                                           iCategoryIndex, iNumCovariatesScanned=0;
  std::vector<int>                              vPopulationCategory;
  const char                                  * pCovariate;
  std::vector<std::string>::iterator            itr;
  std::vector<std::vector<int> >::iterator      itr_int;

//  iScanOffset = 3; //tract identifier, population date, population, covariate 1, ...

  if (gbAggregateCategories)
    iCategoryIndex = 0;

  //create a temporary vector of covariate name indexes
  while ((pCovariate = Parser.GetWord(iNumCovariatesScanned + iScanOffset)) != 0) {
       iNumCovariatesScanned++;
       itr = std::find(gvCovariateNames.begin(), gvCovariateNames.end(), pCovariate);
       if (itr == gvCovariateNames.end()) {
         gvCovariateNames.push_back(pCovariate);
         vPopulationCategory.push_back(gvCovariateNames.size() - 1);
       }
       else
         vPopulationCategory.push_back(std::distance(gvCovariateNames.begin(), itr));
  }

  //first list of covariates sets precedence - remaining categories read must
  //have the same number of covariates
  if (gvPopulationCategories.empty()) {
    //if this is the primary record/first record - set number of covariates
    //we expect to find to remaining records.
    giNumberCovariates = iNumCovariatesScanned;
    gvPopulationCategories.push_back(vPopulationCategory);
    iCategoryIndex = 0;
    gvCategoryCasesCount.resize(1, 0);
    gvCategoryControlsCount.resize(1, 0);
  }
  else if (iNumCovariatesScanned != giNumberCovariates){
    PrintDirection.PrintInputWarning("Error: Record %d of %s file contains %d covariate%s but expecting %d covariate%s.",
                                     Parser.GetReadCount(), szDescription,
                                     iNumCovariatesScanned,(iNumCovariatesScanned == 1 ? "" : "s"),
                                     giNumberCovariates, (giNumberCovariates == 1 ? "" : "s"));
    iCategoryIndex = -1;
  }
  else {
    //if list of covariates is unique then add to list of categories, else get lists index
    itr_int = std::find(gvPopulationCategories.begin(), gvPopulationCategories.end(), vPopulationCategory);
    if (itr_int == gvPopulationCategories.end()) {
      gvPopulationCategories.push_back(vPopulationCategory);
      iCategoryIndex = gvPopulationCategories.size() - 1;
      gvCategoryCasesCount.resize(gvCategoryCasesCount.size() + 1, 0);
      gvCategoryControlsCount.resize(gvCategoryCasesCount.size() + 1, 0);
    }
    else
      iCategoryIndex = std::distance(gvPopulationCategories.begin(), itr_int);
  }

  return iCategoryIndex;
}

/** Look for and display tracts with zero population for any population year. */
void PopulationData::ReportZeroPops(CSaTScanData & Data, FILE *pDisplay, BasePrint * pPrintDirection) const {
  int                           i, j, nPEndIndex, nPStartIndex = 0;
  UInt                          month, day, year;
  bool                          bZeroFound = false;
  float                       * PopTotalsArray = 0;
  std::string                   sBuffer;
  char                          sDateBuffer[20];
  const PopulationCategory    * pCategoryDescriptor;

  try {
    if (gbStartAsPopDt)
      nPStartIndex = 1;
    if (gbEndAsPopDt)
      nPEndIndex = GetNumPopulationDates() - 2;
    else
      nPEndIndex = GetNumPopulationDates() - 1;

    PopTotalsArray = (float*)Smalloc(GetNumPopulationDates() *sizeof(float), pPrintDirection);

    for (i=0; i < (int)gTractCategories.size(); i++) {
       memset(PopTotalsArray, 0, GetNumPopulationDates() * sizeof(float));
       pCategoryDescriptor = gTractCategories[i];
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
              pPrintDirection->SatScanPrintWarning("Warning: According to the input data, the following tracts have a \n");
              pPrintDirection->SatScanPrintWarning("         population totaling zero for the specified date(s).\n\n");
            }
            //JulianToMDY(&month, &day, &year, gvPopulationDates[j]);
            JulianToChar(sDateBuffer, gvPopulationDates[j]);
            fprintf(pDisplay,"         Tract %s, %s\n", Data.GetTInfo()->tiGetTid(i, sBuffer), sDateBuffer);
            pPrintDirection->SatScanPrintWarning("         Tract %s, %s\n", Data.GetTInfo()->tiGetTid(i, sBuffer), sDateBuffer);
          }
       }
    }

    free (PopTotalsArray);
  }
  catch (ZdException &x) {
    free (PopTotalsArray);
    x.AddCallpath("tiReportZeroPops()","PopulationData");
    throw;
  }
}

void PopulationData::SetAggregateCategories(bool b) {
   gbAggregateCategories = b;

   if (gbAggregateCategories) {
     gvPopulationCategories.clear();
     giNumberCovariates = 0;
     gvPopulationCategories.push_back(std::vector<int>());
     gvCategoryCasesCount.resize(1, 0);
     gvCategoryControlsCount.resize(1, 0);
   }
}

/** Initializes the Population dates vector. */
void PopulationData::SetupPopDates(std::vector<Julian>& PopulationDates, Julian StartDate,
                                         Julian EndDate, BasePrint * pPrintDirection) {
  int  n, nSourceOffset, nDestOffset, nDatesUsed, nPopDates;

  try {
    FindPopDatesToUse(PopulationDates, StartDate, EndDate,
                        &nSourceOffset, &nDestOffset, &gbStartAsPopDt, &gbEndAsPopDt,
                        &nDatesUsed, &nPopDates);

    gvPopulationDates.resize(nPopDates);

    if (gbStartAsPopDt)
      gvPopulationDates[0] = StartDate;

    for (n=0; n<nDatesUsed; n++)
       gvPopulationDates[n + nDestOffset] = PopulationDates[n + nSourceOffset];

    if (gbEndAsPopDt)
      gvPopulationDates[nPopDates-1] = EndDate+1;

/* debug */
#if 0
       DisplayDatesArray(pDates, nDates, "Array of Potential Pop Dates", stdout);

       JulianToChar(szDate, StartDate);
      pPrintDirection->SatScanPrintf("\nStart Date = %s\n", szDate);
       JulianToChar(szDate, EndDate);
      pPrintDirection->SatScanPrintf("End Date   = %s\n", szDate);

      pPrintDirection->SatScanPrintf("\nSource offset      = %i\n", nSourceOffset);
      pPrintDirection->SatScanPrintf("Destination offset = %i\n", nDestOffset);
      pPrintDirection->SatScanPrintf("Add Start          = %i\n", bStartAsPopDt);
      pPrintDirection->SatScanPrintf("Add End            = %i\n", bEndAsPopDt);
      pPrintDirection->SatScanPrintf("Number Dates Used  = %i\n", nDatesUsed);
      pPrintDirection->SatScanPrintf("Number Pop Dates   = %i\n", nPopDates);
      pPrintDirection->SatScanPrintf("\n");

      pPrintDirection->SatScanPrintf("<Press any key to continue>");
       c = getc(stdin);

       DisplayDatesArray(pPopDates, nPopDates, "Array of Selected Pop Dates", stdout);
#endif
      }
  catch (ZdException &x) {
    x.AddCallpath("SetupPopDates()","PopulationData");
    throw;
  }
}

/** Finds the date of the smallest PopDate > Date. If none exist return -1. */
int PopulationData::UpperPopIndex(Julian Date) const {
  int   i;

  if (GetPopulationDate(GetNumPopulationDates() - 1) <= Date)
    return -1;

  i = GetNumPopulationDates() - 1;
  while (GetPopulationDate(i-1) > Date)
       i--;

  return i;
}

