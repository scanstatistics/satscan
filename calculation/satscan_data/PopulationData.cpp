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

    if (fPopluation > std::numeric_limits<float>::max() - gpPopulationList[iDateIndex]) {
      char      sDateString[20];

      GenerateResolvableException("Error: An internal attempt to add the population of '%.2f' to the current\n"
                                  "       population of '%.2f', at date '%s', causes data overflow.\n",
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

/** Adds case count to internal structure that records counts by population category.
    Throws ZdException if category index is invalid or category cases exceeds positive
    limits for count_t(signed int). Caller of function is responsible for ensuring
    that parameter 'Count' is a postive count_t(signed int). */
void PopulationData::AddCaseCount(int iCategoryIndex, count_t Count) {
  if (iCategoryIndex < 0 || iCategoryIndex > (int)gvCategoryCasesCount.size() - 1)
    ZdGenerateException("Index '%d' out of range.","AddCaseCount()", iCategoryIndex);
  gvCategoryCasesCount[iCategoryIndex] += Count;
  if (gvCategoryCasesCount[iCategoryIndex] < 0)
    GenerateResolvableException("Error: The total number of cases is greater than the maximum allowed of %ld.\n", "AddCaseCount()", std::numeric_limits<count_t>::max());
}

/** Adds category data to the tract info structure. Caller is responsible for ensuring that
    parameter 'fPopulation' is a positive value. An exception is thrown if 'tTractIndex' is invalid.
    If precision of parameter 'prPopulationDate' is DAY, the passed population is also assigned to
    day following date indicated by prPopulationDate.first. */
void PopulationData::AddCategoryToTract(tract_t tTractIndex, unsigned int iCategoryIndex, const std::pair<Julian, DatePrecisionType>& prPopulationDate, float fPopulation) {
  try {
    if (0 > tTractIndex || tTractIndex > (tract_t)gTractCategories.size() - 1 )
      ZdGenerateException("Index %d out of range(0 - %d).","AddPopulation()",
                          ZdException::Normal, tTractIndex, gTractCategories.size() - 1);

    PopulationCategory & thisDescriptor = GetPopulationCategory(tTractIndex, iCategoryIndex, (int)gvPopulationDates.size());
    AssignPopulation(thisDescriptor, prPopulationDate.first, fPopulation, true);
    //If precision of read population date is day, assign the day afters population to be the same.
    //This is needed for the interpolation process. In method SetPopulationDates(...), it is
    //ensured that this date is listed as one of the possible population dates.
    if (prPopulationDate.second == DAY)
      AssignPopulation(thisDescriptor, prPopulationDate.first + 1, fPopulation, false);
  }
  catch (ZdException &x) {
    x.AddCallpath("AddCategoryToTract()","PopulationData");
    throw;
  }
}

/** Adds control count to internal structure that records counts by population category.
    Throws ZdException if category index is invalid or category controls exceeds positive
    limits for count_t(signed int). Caller of function is responsible for ensuring
    that parameter 'Count' is a postive count_t(signed int). */
void PopulationData::AddControlCount(int iCategoryIndex, count_t Count) {
  if (iCategoryIndex < 0 || iCategoryIndex > (int)gvCategoryControlsCount.size() - 1)
    ZdGenerateException("Index '%d' out of range.","AddControlCount()", iCategoryIndex);
  gvCategoryControlsCount[iCategoryIndex] += Count;
  if (gvCategoryControlsCount[iCategoryIndex] < 0)
    GenerateResolvableException("Error: The total number of controls is greater than the maximum of %ld.\n", "AddControlCount()", std::numeric_limits<count_t>::max());
}

/** Adds population count to categories population list. Passing bTrueDate == true
    indicates that the population date was read from the population file; having
    bTrueDate == false indicates that the population date is one of the introduced
    population dates through method: SetPopulationDates(...). */
void PopulationData::AssignPopulation(PopulationCategory & thisPopulationCategory, Julian PopulationDate, float fPopulation, bool bTrueDate) {
  int iPopulationDateIndex, iNumPopulationDates;

  try {
    iPopulationDateIndex = GetPopulationDateIndex(PopulationDate, bTrueDate);
    if (iPopulationDateIndex != -1) {
      iNumPopulationDates = (int)gvPopulationDates.size();
      thisPopulationCategory.AddPopulationAtDateIndex(fPopulation, iPopulationDateIndex, *this);
      //If the study period start date was introduced and this is the population date immediately
      //after it; then assign it the same population. We are assuming that the study period start
      //date has the same population as first actual population date since we can't use interpolation
      //to determine the population.
      if (iPopulationDateIndex == 1 && gbStartAsPopDt)
        thisPopulationCategory.AddPopulationAtDateIndex(fPopulation, 0L, *this);
      //If the study period end date was introduced and this is the population date immediately
      //prior to it; then assign it the same population. We are assuming that the study period end
      //date has the same population as last actual population date since we can't use interpolation
      //to determine the population.
      if (iPopulationDateIndex == iNumPopulationDates - 2 && gbEndAsPopDt)
        thisPopulationCategory.AddPopulationAtDateIndex(fPopulation, iNumPopulationDates - 1, *this);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("AssignPopulation()","PopulationData");
    throw;
  }
}

/** Returns vector that indicates population dates percentage of the whole study
    period. Calculation assumes that the population of a given day refers to the
    beginning of that day. Caller is responsible for ensuring that
    StartDate <= EndDate. A ZdException is thrown if the summation of calculated
    vAlpha elements does not equal 1.0(+/- .0001). */
void PopulationData::CalculateAlpha(std::vector<double>& vAlpha, Julian StartDate, Julian EndDate) const {
  int                   nPopDates = (int)gvPopulationDates.size();
  int                   n, N = nPopDates-2;
  long                  nTotalDays = TimeBetween(StartDate, EndDate, DAY);
  double                sumalpha;

   try {
     vAlpha.resize(nPopDates+1, 0);

     if (N==0) {
       vAlpha[0] = 0.5*((gvPopulationDates[1]-StartDate)+(gvPopulationDates[1]-(EndDate+1)))/(double)(gvPopulationDates[1]-gvPopulationDates[0]);
       vAlpha[1] = 0.5*((StartDate-gvPopulationDates[0])+((EndDate+1)-gvPopulationDates[0]))/(double)(gvPopulationDates[1]-gvPopulationDates[0]);
     }
     else if(N==1) {
       vAlpha[0] = (0.5*((double)(gvPopulationDates[1]-StartDate)/(gvPopulationDates[1]-gvPopulationDates[0]))*(gvPopulationDates[1]-StartDate)) / (double)nTotalDays;
       vAlpha[1] = (0.5*(gvPopulationDates[1]-StartDate)*(1+((double)(StartDate-gvPopulationDates[0])/(gvPopulationDates[1]-gvPopulationDates[0])))) / (double)nTotalDays
                       + (0.5*(double)(EndDate+1-gvPopulationDates[N])*(1+((double)(gvPopulationDates[N+1]-(EndDate+1))/(gvPopulationDates[N+1]-gvPopulationDates[N])))) /  (double)nTotalDays;
       vAlpha[N+1] = (0.5*((double)(EndDate+1-gvPopulationDates[N])/(gvPopulationDates[N+1]-gvPopulationDates[N]))*(EndDate+1-gvPopulationDates[N])) / nTotalDays;
     }
     else {
       vAlpha[0] = (0.5*((double)(gvPopulationDates[1]-StartDate)/(gvPopulationDates[1]-gvPopulationDates[0]))*(gvPopulationDates[1]-StartDate)) / (double)nTotalDays;
       vAlpha[1] = (0.5*(gvPopulationDates[2]-gvPopulationDates[1]) + 0.5*(gvPopulationDates[1]-StartDate)*(1+((double)(StartDate-gvPopulationDates[0])/(gvPopulationDates[1]-gvPopulationDates[0])))) / (double)nTotalDays;
       for (n = 2; n < N; n++) {
         vAlpha[n] = 0.5*(double)(gvPopulationDates[n+1] - gvPopulationDates[n-1]) / (double)nTotalDays;
       }
       vAlpha[N]   = (0.5*(gvPopulationDates[N]-gvPopulationDates[N-1]) + 0.5*(double)(EndDate+1-gvPopulationDates[N])*(1+((double)(gvPopulationDates[N+1]-(EndDate+1))/(gvPopulationDates[N+1]-gvPopulationDates[N])))) /  (double)nTotalDays;
       vAlpha[N+1] = (0.5*((double)(EndDate+1-gvPopulationDates[N])/(gvPopulationDates[N+1]-gvPopulationDates[N]))*(EndDate+1-gvPopulationDates[N])) / nTotalDays;
     }

     /* Bug check, seeing that alpha values add to one. */
     sumalpha = 0;
     for (n = 0; n <= N+1; ++n)
       sumalpha = sumalpha + vAlpha[n];
     if (sumalpha > 1.0001 || sumalpha < 0.9999)
       ZdGenerateException("Alpha values not calculated correctly.\nThe sum of the alpha values is %8.6lf rather than 1.\n",
                           "CalculateAlpha()", sumalpha);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateAlpha()","PopulationData");
    throw;
  }
}

/** Prints warning if there are tract categories with cases but no population.
    Throws ZdException if total population for tract is zero. Caller is responsible
    for ensuring 'pCases' points to allocates memory and contains a number of
    elements equal to gTractCategories.size(). */
void PopulationData::CheckCasesHavePopulations(const count_t * pCases, const CSaTScanData& Data) const {
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
         GenerateResolvableException("Error: The total population is zero for location ID %s but it has %d cases.",
                                     "CheckCasesHavePopulations()",
                                     Data.GetTInfo()->tiGetTid(i, sBuffer), pCases[i]);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CheckCasesHavePopulations()","PopulationData");
    throw;
  }
}

/** Check to see that no years have a total population of zero. If so, prints
    error message to print direction object and FILE handle (if pointer not NULL).
    Caller is responsible for ensuring that if 'pDisplay' is not NULL, that is points
    to a valid file handle. */
bool PopulationData::CheckZeroPopulations(FILE *pDisplay, BasePrint& PrintDirection) const {
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
          if (pDisplay)
            fprintf(pDisplay, "Error: The population is zero for all location IDs in %d.\n", year);
          PrintDirection.SatScanPrintWarning("Error: The population is zero for all location IDs in %d.\n", year);
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
void PopulationData::Display(BasePrint& PrintDirection) const {
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

/** Adds the population for category of tract between date intervals. If
    tract_t t is not a valid index, an exception id thrown. Caller is responsible
    for ensuring that parameter 'vAlpha' contains a number of elements is in
    accordance with range [iStartPopulationDateIndex .. iEndPopulationDateIndex). */
double PopulationData::GetAlphaAdjustedPopulation(double& dPopulation, tract_t t, int iCategoryIndex,
                                                  int iStartPopulationDateIndex, int iEndPopulationDateIndex,
                                                  const std::vector<double>& vAlpha) const {
  int                           j;
  const PopulationCategory    * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gTractCategories.size() - 1)
      ZdException::Generate("Index %d out of range(0 - %d)", "GetAlphaAdjustedPopulation()", t, gTractCategories.size() - 1);

    pCategoryDescriptor = GetCategoryDescriptor(t, iCategoryIndex);
    if (pCategoryDescriptor) {
      for (j=iStartPopulationDateIndex; j < iEndPopulationDateIndex; j++)
         dPopulation = dPopulation + (vAlpha[j] * pCategoryDescriptor->GetPopulationAtDateIndex(j, *this));
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetAlphaAdjustedPopulation()","PopulationData");
    throw;
  }
  return dPopulation;
}

/** Finds the date of the largest population date < 'Date'. If 'Date' is less than
    or equal to first population date, negative one is returned; else returns
    index of found population date. */
int PopulationData::LowerPopIndex(Julian Date) const {
  int i;

  if (GetPopulationDate(0) >= Date )
    return -1;

  i=0;
  while (GetPopulationDate(i+1) < Date)
       i++;
  return i;
}

/** Returns the number of cases for category at index. Throws ZdException if
    category index is out of range. */
count_t PopulationData::GetNumCategoryCases(int iCategoryIndex) const {
  try {
    if (iCategoryIndex < 0 || iCategoryIndex > static_cast<int>(gvCategoryCasesCount.size()) - 1)
      ZdGenerateException("Index '%d' out of ranges.","GetNumCategoryCases()");

    return gvCategoryCasesCount[iCategoryIndex];
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNumCategoryCases()","PopulationData");
    throw;
  }
}

/** Returns the number of controls for category at index. Throws ZdException if
    category index is out of range. */
count_t PopulationData::GetNumCategoryControls(int iCategoryIndex) const {
  try {
    if (iCategoryIndex < 0 || iCategoryIndex > static_cast<int>(gvCategoryControlsCount.size()) - 1)
      ZdGenerateException("Index '%d' out of ranges.","GetNumCategoryControls()");

    return gvCategoryControlsCount[iCategoryIndex];
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNumCategoryControls()","PopulationData");
    throw;
  }
}

/** Returns pointer to category class with iCategoryIndex. Returns null pointer
    if tract category not found. Throws ZdException if tTractIndex is out of range.*/
PopulationCategory * PopulationData::GetCategoryDescriptor(tract_t tTractIndex, unsigned int iCategoryIndex) {
  PopulationCategory  * pCategoryDescriptor;
  bool                  bDone=false;

  if (tTractIndex < 0 || tTractIndex > static_cast<int>(gTractCategories.size()) - 1)
    ZdGenerateException("Index '%d' out of ranges.","GetCategoryDescriptor()");

  pCategoryDescriptor = gTractCategories[tTractIndex];
  while (pCategoryDescriptor && !bDone) {
       if (pCategoryDescriptor->GetCategoryIndex() == iCategoryIndex)
         bDone = true;
       else
         pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();
  }
  return pCategoryDescriptor;
}

/** Returns pointer to category class with iCategoryIndex. Returns null pointer
    if tract category not found else returns const pointer to object. Throws
    ZdException if tTractIndex is out of range.*/
const PopulationCategory * PopulationData::GetCategoryDescriptor(tract_t tTractIndex, unsigned int iCategoryIndex) const {
  PopulationCategory  * pCategoryDescriptor;
  bool                  bDone=false;

  if (tTractIndex < 0 || tTractIndex > static_cast<int>(gTractCategories.size()) - 1)
    ZdGenerateException("Index '%d' out of ranges.","GetCategoryDescriptor()");

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
    Returns 0 if no population for category. */
float PopulationData::GetPopulation(tract_t t, int iCategoryIndex, int iPopulationDateIndex){
  float                         fValue=0;
  const PopulationCategory    * pCategoryDescriptor;

  try {
    pCategoryDescriptor = GetCategoryDescriptor(t, iCategoryIndex);
    if (pCategoryDescriptor)
      fValue = pCategoryDescriptor->GetPopulationAtDateIndex(iPopulationDateIndex, *this);
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPopulation()","PopulationData");
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
    if (0 > tTractIndex || tTractIndex > (tract_t)gTractCategories.size() - 1)
      ZdException::Generate("Index %d out of range(0 - %d)", "GetPopulationCategory()", tTractIndex, gTractCategories.size() - 1);

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

/** Returns population category as string of space delimited covariates. Throws
    ZdException if category index if invalid. */
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

/** Returns the population date for a given index into the population date array.
    NOTE: If the population date index is invalid, negative one is returned. This
          is a bit strange as Julian is defined as unsigned long. If caller is not
          positive that passed index is valid, returned value should be compared
          against 'static_cast<Julian>(-1)'. */
Julian PopulationData::GetPopulationDate(int iPopulationDateIndex) const {
  Julian        ReturnDate;

  try {
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

/** Returns the index into the population date array for a given date. Pass
    bReverse == true if the population date was read from the population file;
    pass bReverse == false if the date is one of the dates introduced in method
    SetPopulationDates(...).
    The returned index could be -1; this does not necessarily indicate a problem.
    In SetPopulationDates(...), the list of population dates, as read from file,
    is adjusted based upon the specified study period. That process can cause
    population dates to be removed. */
int PopulationData::GetPopulationDateIndex(Julian Date, bool bTrueDate) const {
  int           i, iReturn = - 1;
  bool          bFound=false;

  //NOTE: If there was an identical date introduced in SetPopulationDates(...),
  //      these dates will be adjacent in the population date array.
  if (bTrueDate)
    //Look for a population date that was read from population file. The date
    //we are looking for is the first matching date, searching from most recent
    //date to less recent. That was how SetPopulationDates(...) created array.
    for (i=gvPopulationDates.size() - 1; i >= 0 && !bFound; --i) {
       if (Date == gvPopulationDates[i]) {
         iReturn = i;
         bFound = true;
       }
  }
  else {
    //Look for a population date that was introduced in SetPopulationDates(...).
    //The date we are looking for is the first matching date, searching from less
    //recent date to most recent. That was how SetPopulationDates(...) created array.
    for (i=0; i < (int)gvPopulationDates.size() && !bFound; ++i)
       if (Date == gvPopulationDates[i]) {
         iReturn = i;
         bFound = true;
       }
  }

  return iReturn;
} 

/** Sets the indeces to population dates that mark the lower and upper boundaries
    for time interval defined by [nDateIndex .. nMaxDateIndex] on pDates array.
    Caller is responsible for ensuring that pDates points to valid memory and
    contains a number of elements in accordance with [nDateIndex .. nMaxDateIndex + 1]. */
void PopulationData::GetPopUpLowIndex(const Julian* pDates, int nDateIndex, int nMaxDateIndex, int& nUpIndex, int& nLowIndex) const {
  int   i, index;
  bool  bUpFound = false;

  try {
    for (i=0; i < (int)gvPopulationDates.size(); ++i) {
       if (gvPopulationDates[i] <= pDates[nDateIndex])
         nLowIndex = i;

       if (nDateIndex == nMaxDateIndex)
         index = nDateIndex;
       else
         index = nDateIndex+1;

       if (!bUpFound && (gvPopulationDates[i] >= pDates[index])) {
         bUpFound = true;
         nUpIndex = i;
       }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPopUpLowIndex()","PopulationData");
    throw;
  }
}

/** Returns the risk adjusted population for population date index of tract for
    all categories. Throws ZdException if tract 't' is invalid. Caller is
    responsible for ensuring that 'vRisk' contains a number of elements equal
    to the number of population categories. */
measure_t PopulationData::GetRiskAdjustedPopulation(measure_t& dMeanPopulation, tract_t t, int iPopulationDateIndex, const std::vector<double>& vRisk) const {
  const PopulationCategory    * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gTractCategories.size() - 1)
      ZdException::Generate("Index %d out of range(0 - %d)", "GetRiskAdjustedPopulation()", t, gTractCategories.size() - 1);

    dMeanPopulation = 0.0;
    pCategoryDescriptor = gTractCategories[t];
    while (pCategoryDescriptor) {
         dMeanPopulation = dMeanPopulation + vRisk[pCategoryDescriptor->GetCategoryIndex()] *
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

/** Attmepts to create new population category through parsing record contained
    by StringParser with covariates indicated to start at 'iScanOffset'. If
    previously specified to aggregate population categories, nothing is done
    and category index returned is zero.
    If no population categories exist at function call, the number of expected
    covariates is set to that of currently parsing record. Subsequent function
    calls will be required to adhere to the number of previously defined
    covariates or an error message will be printed to PrintDirection and returned
    category in returned will be negative one. Upon successful creation of
    population category, index is returned. */
int PopulationData::MakePopulationCategory(StringParser& Parser, unsigned int iScanOffset, BasePrint& PrintDirection) {
  unsigned int                                  iCategoryIndex, iNumCovariatesScanned=0;
  std::vector<int>                              vPopulationCategory;
  const char                                  * pCovariate;
  std::vector<std::string>::iterator            itr;
  std::vector<std::vector<int> >::iterator      itr_int;

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
  else if (iNumCovariatesScanned != static_cast<unsigned int>(giNumberCovariates)){
    PrintDirection.PrintInputWarning("Error: Record %d of %s contains %d covariate%s but expecting %d covariate%s.",
                                     Parser.GetReadCount(), PrintDirection.GetImpliedFileTypeString().c_str(),
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

/** Scans for tracts that have population dates which have zero populations.
    Reports such tract populations dates for pDisplay (is not NULL) and
    PrintDirection as formatted warning. */
void PopulationData::ReportZeroPops(const CSaTScanData& Data, FILE *pDisplay, BasePrint& PrintDirection) const {
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

    PopTotalsArray = (float*)Smalloc(GetNumPopulationDates() * sizeof(float));

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
              AsciiPrintFormat::PrintSectionSeparatorString(pDisplay, 1, 2);
              fprintf(pDisplay,"Warning: According to the input data, the following locations have a\n");
              fprintf(pDisplay,"         population totaling zero for the specified date(s).\n\n");
              PrintDirection.SatScanPrintWarning("Warning: According to the input data, the following locations have a\n");
              PrintDirection.SatScanPrintWarning("         population totaling zero for the specified date(s).\n\n");
            }
            JulianToChar(sDateBuffer, gvPopulationDates[j]);
            if (pDisplay)
              fprintf(pDisplay,"         Location %s, %s\n", Data.GetTInfo()->tiGetTid(i, sBuffer), sDateBuffer);
            PrintDirection.SatScanPrintWarning("         Location %s, %s\n", Data.GetTInfo()->tiGetTid(i, sBuffer), sDateBuffer);
          }
       }
    }

    free (PopTotalsArray);
  }
  catch (ZdException &x) {
    free (PopTotalsArray);
    x.AddCallpath("ReportZeroPops()","PopulationData");
    throw;
  }
}

/** Sets internal flag that indicates to aggregate population categories. Calling
    this function with parameter false after any population categories have already
    been created through MakePopulationCategory() is not recommended. */
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

/** Determines which population dates should be keep based upon specified study
    period. Later we will use interpolation to estimate the population on dates
    that were not supplied by input data with respect to the study period. Caller
    is responsible for ensuring that StartDate <= EndDate. */
void PopulationData::SetPopulationDates(std::vector<std::pair<Julian, DatePrecisionType> >& PopulationDates,
                                        Julian StartDate, Julian EndDate) {
  unsigned int  n, iLastIndexedDateIndex, iDateIndexOffset=0, iRetainedDates, iTotalPopulationDates;
  bool          bStartFound=false, bEndFound=false;

  //first insert additional population dates for those read with precision of day
  for (n=0; n < PopulationDates.size(); ++n)
     if (PopulationDates[n].second == DAY) {
       PopulationDates.insert(PopulationDates.begin() + (n + 1), std::make_pair(PopulationDates[n].first + 1, DAY));
       ++n;
     }
  //Loop over input defined population period to ascertain which population dates to keep.
  for (n=0; n < PopulationDates.size() && (!bStartFound || !bEndFound); ++n) {
     if (!bStartFound) {
       //With respects to interpolation, we can only make use of one population date prior
       //to the study period start date. Keep first prior population date but ignore rest.
       if (PopulationDates[n].first > StartDate) {
         bStartFound = true;
         //If the study period start date is less than all population dates, it will be
         //added as first population date. Note that in this case, the population for the
         //start date will be assumed to that of first population date.
         gbStartAsPopDt = (n == 0);
       }
       else
         //Keep advancing offset index - we're removing population dates.
         iDateIndexOffset = n;
         //Note that the first population date could be equal to the study period start
         //date. That date is kept through the nSourceOffset equaling 0 and therefore
         //the calculated number of population dates retained is not effected.
     }
     if (!bEndFound) {
       iLastIndexedDateIndex = n;
       //With respects to interpolation, we can only make use of one population date after
       //to the study period end date. Keep first post population date but ignore rest.
       if (PopulationDates[n].first > EndDate)
         bEndFound = true;
     }
  }
  //Calculate number of population dates retained from original list and total
  //number of population dates.                                                                                                                   
  iTotalPopulationDates = iRetainedDates = iLastIndexedDateIndex - iDateIndexOffset + 1;
  gbEndAsPopDt = !bEndFound && (iLastIndexedDateIndex == PopulationDates.size() - 1);
  if (gbStartAsPopDt)
    ++iTotalPopulationDates;
  if (gbEndAsPopDt)
    ++iTotalPopulationDates;
  //Copy retained population dates to class container.
  gvPopulationDates.clear();
  gvPopulationDates.reserve(iTotalPopulationDates);
  if (gbStartAsPopDt)
    gvPopulationDates.push_back(StartDate);
  for (n=0; n < iRetainedDates; ++n)
     gvPopulationDates.push_back(PopulationDates[n + iDateIndexOffset].first);
  if (gbEndAsPopDt)
    gvPopulationDates.push_back(EndDate + 1);                                         
}

/** Finds the date of the smallest PopDate > Date. If none exist return -1. */
/** Finds the date of the largest population date > 'Date'. If 'Date' is greater
    than or equal to last population date, negative one is returned; else returns
    index of found population date. */
int PopulationData::UpperPopIndex(Julian Date) const {
  int   i;

  if (GetPopulationDate(GetNumPopulationDates() - 1) <= Date)
    return -1;

  i = GetNumPopulationDates() - 1;
  while (GetPopulationDate(i-1) > Date)
       i--;

  return i;
}

