//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PopulationData.h"
#include "UtilityFunctions.h"
#include "SaTScanData.h"
#include "AsciiPrintFormat.h"
#include "SSException.h"
#include "DataSource.h"

/** Constructor */
CovariateCategory::CovariateCategory(int iPopulationDatesCount, int iCategoryIndex) : gpNextDescriptor(0), gpPopulationList(0) {
  try {
    Init();
    Setup(iPopulationDatesCount, iCategoryIndex);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()", "CovariateCategory");
    throw;
  }
}

/** Destructor */
CovariateCategory::~CovariateCategory() {
  try {
    delete gpNextDescriptor; gpNextDescriptor=0;
    delete[] gpPopulationList; gpPopulationList=0;
  }
  catch(...){}
}

/** Adds population to existing population for date index.
    Throws resolvable_error if population date index is out of range.
    Throws resolvable_error if fPopluation causes population at
    population date index to exceeds numeric limits of float data type. */
void CovariateCategory::AddPopulationAtDateIndex(float fPopluation, unsigned int iDateIndex, const PopulationData& thePopulation) {
  try {
    //validate population date index
    if (iDateIndex > thePopulation.GetNumPopulationDates() - 1)
      throw prg_error("Index %d out of range [size=%u].","AddPopulationAtDateIndex()", iDateIndex, thePopulation.GetNumPopulationDates());
    //check numeric limits of data type will not be exceeded
    if (fPopluation > std::numeric_limits<float>::max() - gpPopulationList[iDateIndex]) {
      char      sDateString[20];
      throw resolvable_error("Error: An internal attempt to add the population of '%.2f' to the current\n"
                             "       population of '%.2f', at date '%s', causes data overflow.\n",
                             fPopluation, gpPopulationList[iDateIndex],
                             JulianToChar(sDateString, thePopulation.GetPopulationDate(iDateIndex)));
    }
    //add population to total at population date index
    gpPopulationList[iDateIndex] += fPopluation;
  }
  catch (prg_exception& x) {
    x.addTrace("AddPopulationAtDateIndex()","CovariateCategory");
    throw;
  }
}

/** Returns population at date index. */
float CovariateCategory::GetPopulationAtDateIndex(unsigned int iDateIndex, const PopulationData & thePopulation) const {
  try {
    if (iDateIndex > thePopulation.GetNumPopulationDates() - 1)
      throw prg_error("Index %d out of range [size=%u].","GetPopulationAtDateIndex()",
                      iDateIndex, thePopulation.GetNumPopulationDates());
  }
  catch (prg_exception& x) {
    x.addTrace("GetPopulationAtDateIndex()","CovariateCategory");
    throw;
  }
  return gpPopulationList[iDateIndex];
}

/** Allocates next PopulationCategory. Returns pointer to allocated object. */
CovariateCategory * CovariateCategory::SetNextDescriptor(int iPopulationListSize, int iCategoryIndex) {
  try {
    delete gpNextDescriptor; gpNextDescriptor=0;
    gpNextDescriptor = new CovariateCategory(iPopulationListSize, iCategoryIndex);
  }
  catch (prg_exception& x) {
    x.addTrace("SetNextDescriptor()","CovariateCategory");
    throw;
  }
  return gpNextDescriptor;
}

/** Sets population at date index. */
void CovariateCategory::SetPopulationAtDateIndex(float fPopluation, unsigned int iDateIndex, const PopulationData & thePopulation) {
  try {
    if (iDateIndex > thePopulation.GetNumPopulationDates() - 1)
      throw prg_error("Index %u out of range [size=%u].","SetPopulationAtDateIndex()",
                      iDateIndex, thePopulation.GetNumPopulationDates());
    gpPopulationList[iDateIndex] = fPopluation;
  }
  catch (prg_exception& x) {
    x.addTrace("SetPopulationAtDateIndex()","CovariateCategory");
    throw;
  }
}

/** Allocates and intializes population list. */
void CovariateCategory::SetPopulationListSize(int iPopulationListSize) {
  try {
    if (gpPopulationList) {delete [] gpPopulationList; gpPopulationList=0;}
    gpPopulationList = new float[iPopulationListSize];
    memset(gpPopulationList, 0, iPopulationListSize * sizeof(*gpPopulationList));
  }
  catch (prg_exception& x) {
    x.addTrace("SetPopulationListSize()","CovariateCategory");
    delete[] gpPopulationList; gpPopulationList=0;
    throw;
  }
}

/** Internal setup function */
void CovariateCategory::Setup(int iPopulationListSize, int iCategoryIndex) {
  try {
    SetPopulationListSize(iPopulationListSize);
    SetCategoryIndex(iCategoryIndex);
  }
  catch (prg_exception& x) {
    x.addTrace("Setup()","CovariateCategory");
    throw;
  }
}

//******************************************************************************

/** constructor */
OrdinalCategory::OrdinalCategory(double dOrdinalNumber, count_t tInitialCount)
                :gdOrdinalNumber(dOrdinalNumber), gtTotalCases(tInitialCount) {}

/** destructor */
OrdinalCategory::~OrdinalCategory() {}

/** Adds case count to cumulative total for ordinal category.
    Throws prg_eror if tCount is negative.
    Throws resolvable_error if category cases exceeds positive
    limits for count_t(long) */
void OrdinalCategory::AddCaseCount(count_t tCount) {
  // validate count is not negative
  if (tCount < 0)
    throw prg_error("Negative case count specifed '%ld'.","AddCaseCount()", tCount);
  gtTotalCases += tCount;
  // check that addition of category case did not exceed numeric limits of data type
  if (gtTotalCases < 0)
    throw resolvable_error("Error: The total number of cases exceeds the maximum of %ld.\n", std::numeric_limits<count_t>::max());
}

/** Decrements number of cases in category. */
void OrdinalCategory::DecrementCaseCount(count_t tCount) {
  gtTotalCases = std::max(0L, gtTotalCases - tCount); 
}

//******************************************************************************

/** constructor */
PopulationData::PopulationData()  {
  Init();
}

/** destructor */
PopulationData::~PopulationData() {
  try {
    for (size_t t=0; t < gCovariateCategoriesPerLocation.size(); ++t)
       delete gCovariateCategoriesPerLocation[t];
  }
  catch (...){}
}

/** Adds case count to internal structure that records counts by population category.
    Throws prg_error if category index is invalid or tCount is negative.
    Throws resolvable_error if category cases exceeds positive
    limits for count_t(long) */
void PopulationData::AddCovariateCategoryCaseCount(int iCategoryIndex, count_t tCount) {
  // validate category index
  if (iCategoryIndex < 0 || iCategoryIndex > (int)gvCovariateCategoryCaseCount.size() - 1)
    throw prg_error("Index '%d' out of range.","AddCovariateCategoryCaseCount()", iCategoryIndex);
  // validate count is not negative
  if (tCount < 0)
    throw prg_error("Negative case count specifed '%ld'.","AddCovariateCategoryCaseCount()", tCount);
  // add to cumulative variable for category at index
  gvCovariateCategoryCaseCount[iCategoryIndex] += tCount;
  // check that addition of category case did not exceed numeric limits of data type
  if (gvCovariateCategoryCaseCount[iCategoryIndex] < 0)
    throw resolvable_error("Error: The total number of cases is greater than the maximum of %ld.\n", std::numeric_limits<count_t>::max());
}

/** Adds control count to internal structure that records counts by population category.
    Throws prg_error if category index is invalid or tCount is negative.
    Throws resolvable_error if category cases exceeds positive
    limits for count_t(long) */
void PopulationData::AddCovariateCategoryControlCount(int iCategoryIndex, count_t Count) {
  // validate category index
  if (iCategoryIndex < 0 || iCategoryIndex > (int)gvCovariateCategoryControlCount.size() - 1)
    throw prg_error("Index %d out of range [size=%u].","AddCovariateCategoryControlCount()", iCategoryIndex, gvCovariateCategoryControlCount.size());
  // validate count is not negative
  if (Count < 0)
    throw prg_error("Negative control count specifed '%ld'.","AddCovariateCategoryControlCount()", Count);
  // add to cumulative variable for category at index
  gvCovariateCategoryControlCount[iCategoryIndex] += Count;
  // check that addition of category control did not exceed numeric limits of data type
  if (gvCovariateCategoryControlCount[iCategoryIndex] < 0)
    throw resolvable_error("Error: The total number of controls is greater than the maximum of %ld.\n", std::numeric_limits<count_t>::max());
}

/** Adds category data to the tract info structure. Caller is responsible for ensuring that
    parameter 'fPopulation' is a positive value. An exception is thrown if 'tTractIndex' is invalid.
    If precision of parameter 'prPopulationDate' is DAY, the passed population is also assigned to
    day following date indicated by prPopulationDate.first. */
void PopulationData::AddCovariateCategoryPopulation(tract_t tTractIndex, unsigned int iCategoryIndex, const std::pair<Julian, DatePrecisionType>& prPopulationDate, float fPopulation) {
  try {
    if (0 > tTractIndex || tTractIndex > (tract_t)gCovariateCategoriesPerLocation.size() - 1 )
      throw prg_error("Index %d out of range [size=%u].","AddCovariateCategoryPopulation()",
                      tTractIndex, gCovariateCategoriesPerLocation.size());

    CovariateCategory & thisDescriptor = GetCovariateCategory(tTractIndex, iCategoryIndex, (int)gvPopulationDates.size());
    AssignPopulation(thisDescriptor, prPopulationDate.first, fPopulation, true);
    //If precision of read population date is day, assign the day afters population to be the same.
    //This is needed for the interpolation process. In method SetPopulationDates(...), it is
    //ensured that this date is listed as one of the possible population dates.
    if (prPopulationDate.second == DAY)
      AssignPopulation(thisDescriptor, prPopulationDate.first + 1, fPopulation, false);
  }
  catch (prg_exception& x) {
    x.addTrace("AddCovariateCategoryPopulation()","PopulationData");
    throw;
  }
}

/** Adds case count to total for category represented by 'dOrdinalNumber'. */
size_t PopulationData::AddOrdinalCategoryCaseCount(double dOrdinalNumber, count_t tCount) {
  std::vector<OrdinalCategory>::iterator itr=gvOrdinalCategories.begin();

  for (; itr != gvOrdinalCategories.end(); ++itr) {
     if (itr->GetOrdinalNumber() == dOrdinalNumber) {
       itr->AddCaseCount(tCount);
       return std::distance(gvOrdinalCategories.begin(), itr);
     }
     if (dOrdinalNumber < itr->GetOrdinalNumber()) {
       itr = gvOrdinalCategories.insert(itr, OrdinalCategory(dOrdinalNumber, tCount));
       return std::distance(gvOrdinalCategories.begin(), itr);
     }
  }

  gvOrdinalCategories.push_back(OrdinalCategory(dOrdinalNumber, tCount));
  return gvOrdinalCategories.size() - 1;
}

/** Adds population count to categories population list. Passing bTrueDate == true
    indicates that the population date was read from the population file; having
    bTrueDate == false indicates that the population date is one of the introduced
    population dates through method: SetPopulationDates(...). */
void PopulationData::AssignPopulation(CovariateCategory& thisCovariateCategory, Julian PopulationDate, float fPopulation, bool bTrueDate) {
  int iPopulationDateIndex, iNumPopulationDates;

  try {
    iPopulationDateIndex = GetPopulationDateIndex(PopulationDate, bTrueDate);
    if (iPopulationDateIndex != -1) {
      iNumPopulationDates = (int)gvPopulationDates.size();
      thisCovariateCategory.AddPopulationAtDateIndex(fPopulation, iPopulationDateIndex, *this);
      //If the study period start date was introduced and this is the population date immediately
      //after it; then assign it the same population. We are assuming that the study period start
      //date has the same population as first actual population date since we can't use interpolation
      //to determine the population.
      if (iPopulationDateIndex == 1 && gbStartAsPopDt)
        thisCovariateCategory.AddPopulationAtDateIndex(fPopulation, 0L, *this);
      //If the study period end date was introduced and this is the population date immediately
      //prior to it; then assign it the same population. We are assuming that the study period end
      //date has the same population as last actual population date since we can't use interpolation
      //to determine the population.
      if (iPopulationDateIndex == iNumPopulationDates - 2 && gbEndAsPopDt)
        thisCovariateCategory.AddPopulationAtDateIndex(fPopulation, iNumPopulationDates - 1, *this);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("AssignPopulation()","PopulationData");
    throw;
  }
}

/** Returns vector that indicates population dates percentage of the whole study
    period. Calculation assumes that the population of a given day refers to the
    beginning of that day. Caller is responsible for ensuring that
    StartDate <= EndDate. A prg_error is thrown if the summation of calculated
    vAlpha elements does not equal 1.0(+/- .0001). */
void PopulationData::CalculateAlpha(std::vector<double>& vAlpha, Julian StartDate, Julian EndDate) const {
  int                   nPopDates = (int)gvPopulationDates.size();
  int                   n, N = nPopDates-2;
  long                  nTotalDays = EndDate + 1 - StartDate;
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
       throw prg_error("Alpha values not calculated correctly.\nThe sum of the alpha values is %8.6lf rather than 1.\n",
                           "CalculateAlpha()", sumalpha);
  }
  catch (prg_exception& x) {
    x.addTrace("CalculateAlpha()","PopulationData");
    throw;
  }
}

/** Prints warning if there are tract categories with cases but no population.
    Throws prg_error if total population for tract is zero. Caller is responsible
    for ensuring 'pCases' points to allocates memory and contains a number of
    elements equal to gTractCategories.size(). */
void PopulationData::CheckCasesHavePopulations(const count_t * pCases, const CSaTScanData& Data) const {
  int                           j, nPEndIndex, nPStartIndex = 0;
  size_t                        tTractIndex;
  const CovariateCategory  *    pCategoryDescriptor;
  std::string                   sBuffer;
  float                         fTractTotalPopulation, fTractCategoryPopulation;
  std::vector<float>            vCategoryTotalPopulation(GetNumCovariateCategories(), 0);

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
    for (tTractIndex=0; tTractIndex < gCovariateCategoriesPerLocation.size(); ++tTractIndex) {
       fTractTotalPopulation = 0;
       pCategoryDescriptor = gCovariateCategoriesPerLocation[tTractIndex];
       while (pCategoryDescriptor) {
          fTractCategoryPopulation = 0;
          for (j=nPStartIndex; j <= nPEndIndex; j++)
             fTractCategoryPopulation += pCategoryDescriptor->GetPopulationAtDateIndex(j, *this);
          fTractTotalPopulation += fTractCategoryPopulation;
          vCategoryTotalPopulation[pCategoryDescriptor->GetCategoryIndex()] += fTractCategoryPopulation;
          pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();
       }
       if (fTractTotalPopulation == 0 && pCases[tTractIndex] > 0)
         throw resolvable_error("Error: The total population is zero for location ID %s but it has %d cases.",
                                Data.GetTInfo()->getLocations().at(tTractIndex)->getIndentifier(), pCases[tTractIndex]);
    }

    //if there is at least one case in a category then the total population in
    //that category can not be zero (summed across all locations)
    for (tTractIndex=0; tTractIndex < gvCovariateCategoryCaseCount.size(); ++tTractIndex) {
       if (gvCovariateCategoryCaseCount[tTractIndex] && !vCategoryTotalPopulation[tTractIndex])
         throw resolvable_error("Error: The total population in covariate category '%s'\n"
                                "       is zero but there is %d case%s specified in the case file.",
                                GetCovariateCategoryAsString(tTractIndex, sBuffer),
                                gvCovariateCategoryCaseCount[tTractIndex],
                                (gvCovariateCategoryCaseCount[tTractIndex] == 1 ? "" : "s"));
    }
  }
  catch (prg_exception& x) {
    x.addTrace("CheckCasesHavePopulations()","PopulationData");
    throw;
  }
}

/** Check to see that no years have a total population of zero. If so, prints
    error message to print direction object and FILE handle (if pointer not NULL).
    Caller is responsible for ensuring that if 'pDisplay' is not NULL, that is points
    to a valid file handle. */
bool PopulationData::CheckZeroPopulations(BasePrint& PrintDirection) const {
  UInt                          month, day, year;
  bool                          bValid = true;
  int                           i, j, nPEndIndex, nPStartIndex = 0;
  const CovariateCategory     * pCategoryDescriptor;

  try {
    if (gbStartAsPopDt)
      nPStartIndex = 1;
    if (gbEndAsPopDt)
      nPEndIndex = (int)gvPopulationDates.size()-2;
    else
      nPEndIndex = (int)gvPopulationDates.size()-1;

    std::vector<float> PopTotalsArray(gvPopulationDates.size(), 0);
    for (i=0; i < (int)gCovariateCategoriesPerLocation.size(); i++) {
       pCategoryDescriptor = gCovariateCategoriesPerLocation[i];
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
          PrintDirection.Printf("Error: The population is zero for all location IDs in %d/%d/%d.\n", BasePrint::P_ERROR, year, month, day);
       }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("CheckZeroPopulations()","PopulationData");
    throw;
  }
  return bValid;
}

/** Attmepts to create new population category through parsing record contained
    by DataSource with covariates indicated to start at 'iScanOffset'. If
    previously specified to aggregate population categories, nothing is done
    and category index returned is zero.
    If no population categories exist at function call, the number of expected
    covariates is set to that of currently parsing record. Subsequent function
    calls will be required to adhere to the number of previously defined
    covariates or an error message will be printed to PrintDirection and returned
    category in returned will be negative one. Upon successful creation of
    population category, index is returned. */
int PopulationData::CreateCovariateCategory(DataSource& Source, short iScanOffset, BasePrint& PrintDirection) {
  unsigned int                                  iCategoryIndex;
  short                                         iNumCovariatesScanned=0;
  std::vector<int>                              vPopulationCategory;
  const char                                  * pCovariate;
  std::vector<std::string>::iterator            itr;
  std::vector<std::vector<int> >::iterator      itr_int;

  if (gbAggregateCovariateCategories)
    iCategoryIndex = 0;

  //create a temporary vector of covariate name indexes
  while ((pCovariate = Source.GetValueAt(iNumCovariatesScanned + iScanOffset)) != 0) {
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
  if (gvCovariateCategories.empty()) {
    //if this is the primary record/first record - set number of covariates
    //we expect to find to remaining records.
    giNumberCovariatesPerCategory = iNumCovariatesScanned;
    gvCovariateCategories.push_back(vPopulationCategory);
    iCategoryIndex = 0;
    gvCovariateCategoryCaseCount.resize(1, 0);
    gvCovariateCategoryControlCount.resize(1, 0);
  }
  else if (iNumCovariatesScanned != giNumberCovariatesPerCategory){
    PrintDirection.Printf("Error: Record %d of %s contains %i covariate%s but expecting %i covariate%s.\n",
                          BasePrint::P_READERROR, Source.GetCurrentRecordIndex(), PrintDirection.GetImpliedFileTypeString().c_str(),
                          iNumCovariatesScanned,(iNumCovariatesScanned == 1 ? "" : "s"),
                          giNumberCovariatesPerCategory, (giNumberCovariatesPerCategory == 1 ? "" : "s"));
    iCategoryIndex = -1;
  }
  else {
    //if list of covariates is unique then add to list of categories, else get lists index
    itr_int = std::find(gvCovariateCategories.begin(), gvCovariateCategories.end(), vPopulationCategory);
    if (itr_int == gvCovariateCategories.end()) {
      gvCovariateCategories.push_back(vPopulationCategory);
      iCategoryIndex = gvCovariateCategories.size() - 1;
      gvCovariateCategoryCaseCount.resize(gvCovariateCategoryCaseCount.size() + 1, 0);
      gvCovariateCategoryControlCount.resize(gvCovariateCategoryControlCount.size() + 1, 0);
    }
    else
      iCategoryIndex = std::distance(gvCovariateCategories.begin(), itr_int);
  }

  return iCategoryIndex;
}

/** Prints formatted text depicting state of population categories. */
void PopulationData::Display(BasePrint& PrintDirection) const {
  size_t        t, j;

  try {
    PrintDirection.Printf("DISPLAY: Number of categories = %i\n", BasePrint::P_STDOUT, gvCovariateCategories.size());
    PrintDirection.Printf("\n#   Category Combination\n", BasePrint::P_STDOUT);
    for (t=0; t < gvCovariateCategories.size(); t++) {
       PrintDirection.Printf("%d     ", BasePrint::P_STDOUT, t);
       for (j=0; j < gvCovariateCategories[t].size(); j++)
          PrintDirection.Printf("%s  ", BasePrint::P_STDOUT, gvCovariateNames[gvCovariateCategories[t][j]].c_str());
       PrintDirection.Printf("\n", BasePrint::P_STDOUT);
    }
    PrintDirection.Printf("\n", BasePrint::P_STDOUT);
  }
  catch (prg_exception& x) {
    x.addTrace("Display()", "PopulationData");
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
  const CovariateCategory     * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gCovariateCategoriesPerLocation.size() - 1)
      throw prg_error("Index %d out of range [size=%u]", "GetAlphaAdjustedPopulation()", t, gCovariateCategoriesPerLocation.size());

    pCategoryDescriptor = GetCovariateCategory(t, iCategoryIndex);
    if (pCategoryDescriptor) {
      for (j=iStartPopulationDateIndex; j < iEndPopulationDateIndex; j++)
         dPopulation = dPopulation + (vAlpha[j] * pCategoryDescriptor->GetPopulationAtDateIndex(j, *this));
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetAlphaAdjustedPopulation()","PopulationData");
    throw;
  }
  return dPopulation;
}

/** Returns the number of cases for category at index. Throws prg_error if
    category index is out of range. */
count_t PopulationData::GetNumCovariateCategoryCases(int iCategoryIndex) const {
  try {
    if (iCategoryIndex < 0 || iCategoryIndex > static_cast<int>(gvCovariateCategoryCaseCount.size()) - 1)
      throw prg_error("Index %d out of range [size=%u].","GetNumCovariateCategoryCases()", iCategoryIndex, gvCovariateCategoryCaseCount.size());

    return gvCovariateCategoryCaseCount[iCategoryIndex];
  }
  catch (prg_exception& x) {
    x.addTrace("GetNumCovariateCategoryCases()","PopulationData");
    throw;
  }
}

/** Returns the number of controls for category at index. Throws prg_error if
    category index is out of range. */
count_t PopulationData::GetNumCovariateCategoryControls(int iCategoryIndex) const {
  try {
    if (iCategoryIndex < 0 || iCategoryIndex > static_cast<int>(gvCovariateCategoryControlCount.size()) - 1)
      throw prg_error("Index %d out of range [size=%u].","GetNumCovariateCategoryControls()", iCategoryIndex, gvCovariateCategoryControlCount.size());

    return gvCovariateCategoryControlCount[iCategoryIndex];
  }
  catch (prg_exception& x) {
    x.addTrace("GetNumCovariateCategoryControls()","PopulationData");
    throw;
  }
}

/** Returns the number of cases for ordinal category at index. Throws prg_error if
    category index is out of range. */
count_t PopulationData::GetNumOrdinalCategoryCases(int iCategoryIndex) const {
  try {
    if (iCategoryIndex < 0 || iCategoryIndex > static_cast<int>(gvOrdinalCategories.size()) - 1)
      throw prg_error("Index %d out of range [size=%u].","GetNumOrdinalCategoryCases()", iCategoryIndex, gvOrdinalCategories.size());

    return gvOrdinalCategories[iCategoryIndex].GetTotalCases();
  }
  catch (prg_exception& x) {
    x.addTrace("GetNumOrdinalCategoryCases()","PopulationData");
    throw;
  }
}

/** internal class initialization */
void PopulationData::Init() {
  giNumberCovariatesPerCategory=0;
  gbAggregateCovariateCategories=false;
  gbStartAsPopDt=false;
  gbEndAsPopDt=false;
}

/** Returns pointer to category class with iCategoryIndex. Returns null pointer
    if tract category not found. Throws prg_error if tTractIndex is out of range.*/
CovariateCategory * PopulationData::GetCovariateCategory(tract_t tTractIndex, unsigned int iCategoryIndex) {
  CovariateCategory   * pCategoryDescriptor;
  bool                  bDone=false;

  if (tTractIndex < 0 || tTractIndex > static_cast<int>(gCovariateCategoriesPerLocation.size()) - 1)
    throw prg_error("Index %d out of range [size=%u].","GetCategoryDescriptor()", tTractIndex, gCovariateCategoriesPerLocation.size());

  pCategoryDescriptor = gCovariateCategoriesPerLocation[tTractIndex];
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
    prg_error if tTractIndex is out of range.*/
const CovariateCategory * PopulationData::GetCovariateCategory(tract_t tTractIndex, unsigned int iCategoryIndex) const {
  CovariateCategory   * pCategoryDescriptor;
  bool                  bDone=false;

  if (tTractIndex < 0 || tTractIndex > static_cast<int>(gCovariateCategoriesPerLocation.size()) - 1)
    throw prg_error("Index %d out of range [size=%u].","GetCategoryDescriptor()", tTractIndex, gCovariateCategoriesPerLocation.size());

  pCategoryDescriptor = gCovariateCategoriesPerLocation[tTractIndex];
  while (pCategoryDescriptor && !bDone) {
       if (pCategoryDescriptor->GetCategoryIndex() == iCategoryIndex)
         bDone = true;
       else
         pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();
  }
  return pCategoryDescriptor;
}

/** Returns referance to category descriptor for tract with iCategoryIndex.
    If descriptor not found, a new node is created for category and appended to list. */
CovariateCategory & PopulationData::GetCovariateCategory(tract_t tTractIndex, unsigned int iCategoryIndex, int iPopulationListSize) {
  CovariateCategory           * pCurrentDescriptor, * pPreviousDescriptor;
  bool                          bFound=false;

  try {
    if (0 > tTractIndex || tTractIndex > (tract_t)gCovariateCategoriesPerLocation.size() - 1)
      throw prg_error("Index %d out of range [size=%u]", "GetPopulationCategory()", tTractIndex, gCovariateCategoriesPerLocation.size());

    if (!gCovariateCategoriesPerLocation[tTractIndex]) {
      gCovariateCategoriesPerLocation[tTractIndex] = new CovariateCategory(iPopulationListSize, iCategoryIndex);
      pCurrentDescriptor = gCovariateCategoriesPerLocation[tTractIndex];
    }
    else {
      pCurrentDescriptor = gCovariateCategoriesPerLocation[tTractIndex];
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
  catch (prg_exception& x) {
    x.addTrace("GetCovariateCategory()","PopulationData");
    throw;
  }
  return *pCurrentDescriptor;
}

/** Returns population category as string of space delimited covariates. Throws
    prg_error if category index if invalid. */
const char * PopulationData::GetCovariateCategoryAsString(int iCategoryIndex, std::string & sBuffer) const {
  size_t        t;

  try {
    if (iCategoryIndex < 0 || iCategoryIndex > (int)(gvCovariateCategories.size() - 1))
      throw prg_error("Index %d out of range [size=%u].", "GetPopulationCategoryAsString()",
                            iCategoryIndex, gvCovariateCategories.size());

    sBuffer = "";
    for (t=0; t < gvCovariateCategories[iCategoryIndex].size(); t++) {
       if (t > 0)
          sBuffer += " ";
       sBuffer += gvCovariateNames[gvCovariateCategories[iCategoryIndex][t]];
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetCovariateCategoryAsString()", "PopulationData");
    throw;
  }
  return sBuffer.c_str();
}

/** Returns category index for passed vector of covariate strings. Returns -1 if not found. */
int PopulationData::GetCovariateCategoryIndex(const std::vector<std::string>& vCovariates) const {
  int           iIndex=-1;
  size_t        t, j;
  bool          bMatch;

  for (t=0; t < gvCovariateCategories.size() && iIndex == -1; t++) {
     for (j=0, bMatch=true; j < gvCovariateCategories[t].size() && bMatch; j++)
        bMatch = gvCovariateNames[gvCovariateCategories[t][j]] == vCovariates[j];
     if (bMatch)
       iIndex = t;
  }
  return iIndex;
}

/** Returns ordinal value for category. */
double PopulationData::GetOrdinalCategoryValue(int iCategoryIndex) const {
  try {
    if (iCategoryIndex < 0 || iCategoryIndex > static_cast<int>(gvOrdinalCategories.size()) - 1)
      throw prg_error("Index %d out of range [size=%u].","GetNumOrdinalCategoryCases()", iCategoryIndex, gvOrdinalCategories.size());

    return gvOrdinalCategories[iCategoryIndex].GetOrdinalNumber();
  }
  catch (prg_exception& x) {
    x.addTrace("GetOrdinalCategoryValue()","PopulationData");
    throw;
  }
}

/** Returns the population for a given year and category in a given tract
    Returns 0 if no population for category. */
float PopulationData::GetPopulation(tract_t t, int iCategoryIndex, int iPopulationDateIndex){
  float                         fValue=0;
  const CovariateCategory     * pCategoryDescriptor;

  try {
    pCategoryDescriptor = GetCovariateCategory(t, iCategoryIndex);
    if (pCategoryDescriptor)
      fValue = pCategoryDescriptor->GetPopulationAtDateIndex(iPopulationDateIndex, *this);
  }
  catch (prg_exception& x) {
    x.addTrace("GetPopulation()","PopulationData");
    throw;
  }
  return fValue;
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
  catch (prg_exception& x) {
    x.addTrace("GetPopulationDate()","PopulationData");
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
    for time interval defined by [nDateIndex .. nMaxDateIndex] on vIntervalStartDates
    vector. Caller is responsible for ensuring that vIntervalStartDates contains
    a number of elements in accordance with [nDateIndex .. nMaxDateIndex + 1]. */
void PopulationData::GetPopUpLowIndex(const std::vector<Julian>& vIntervalStartDates, int nDateIndex, int nMaxDateIndex, int& nUpIndex, int& nLowIndex) const {
  int   i, index;
  bool  bUpFound = false;

  try {
    for (i=0; i < (int)gvPopulationDates.size(); ++i) {
       if (gvPopulationDates[i] <= vIntervalStartDates[nDateIndex])
         nLowIndex = i;

       if (nDateIndex == nMaxDateIndex)
         index = nDateIndex;
       else
         index = nDateIndex+1;

       if (!bUpFound && (gvPopulationDates[i] >= vIntervalStartDates[index])) {
         bUpFound = true;
         nUpIndex = i;
       }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetPopUpLowIndex()","PopulationData");
    throw;
  }
}

/** Returns the risk adjusted population for population date index of tract for
    all categories. Throws prg_error if tract 't' is invalid. Caller is
    responsible for ensuring that 'vRisk' contains a number of elements equal
    to the number of population categories. */
measure_t PopulationData::GetRiskAdjustedPopulation(measure_t& dMeanPopulation, tract_t t, int iPopulationDateIndex, const std::vector<double>& vRisk) const {
  const CovariateCategory     * pCategoryDescriptor;

  try {
    if (0 > t || t > (tract_t)gCovariateCategoriesPerLocation.size() - 1)
      throw prg_error("Index %d out of range [size=%u]", "GetRiskAdjustedPopulation()", t, gCovariateCategoriesPerLocation.size());

    dMeanPopulation = 0.0;
    pCategoryDescriptor = gCovariateCategoriesPerLocation[t];
    while (pCategoryDescriptor) {
         dMeanPopulation = dMeanPopulation + vRisk[pCategoryDescriptor->GetCategoryIndex()] *
                              pCategoryDescriptor->GetPopulationAtDateIndex(iPopulationDateIndex, *this);
         pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();               
    }
  }
  catch (prg_exception& x) {
    x.addTrace("GetRiskAdjustedPopulation()","PopulationData");
    throw;
  }
  return dMeanPopulation;
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

/** Removes cases from internal count for ordinal category. */
void PopulationData::RemoveOrdinalCategoryCases(size_t iCategoryIndex, count_t tCount) {
  try {
    if (iCategoryIndex > gvOrdinalCategories.size() - 1)
      throw prg_error("Index %d out of range [size=%u].","RemoveOrdinalCategoryCases()", iCategoryIndex, gvOrdinalCategories.size());

    gvOrdinalCategories[iCategoryIndex].DecrementCaseCount(tCount);
  }
  catch (prg_exception& x) {
    x.addTrace("RemoveOrdinalCategoryCases()","PopulationData");
    throw;
  }
}

/** Scans for tracts that have population dates which have zero populations.
    Reports such tract populations dates for pDisplay (is not NULL) and
    PrintDirection as formatted warning. */
void PopulationData::ReportZeroPops(const CSaTScanData& Data, FILE *pDisplay, BasePrint& PrintDirection) const {
  int                           i, j, nPEndIndex, nPStartIndex = 0;
  bool                          bZeroFound = false;
  std::string                   sBuffer;
  char                          sDateBuffer[20];
  const CovariateCategory     * pCategoryDescriptor;

  try {
    if (gbStartAsPopDt)
      nPStartIndex = 1;
    if (gbEndAsPopDt)
      nPEndIndex = GetNumPopulationDates() - 2;
    else
      nPEndIndex = GetNumPopulationDates() - 1;

    std::vector<float> PopTotalsArray(gvPopulationDates.size());
    for (i=0; i < (int)gCovariateCategoriesPerLocation.size(); i++) {
       std::fill(PopTotalsArray.begin(), PopTotalsArray.end(), 0);
       pCategoryDescriptor = gCovariateCategoriesPerLocation[i];
       while (pCategoryDescriptor) {
          for (j=nPStartIndex; j <= nPEndIndex; j++)
             PopTotalsArray[j] += pCategoryDescriptor->GetPopulationAtDateIndex(j, *this);
          pCategoryDescriptor = pCategoryDescriptor->GetNextDescriptor();   
       }

       for (j=nPStartIndex; j <= nPEndIndex; j++) {
          if (PopTotalsArray[j]==0) {
            if (!bZeroFound) {
              bZeroFound = true;
              AsciiPrintFormat::PrintSectionSeparatorString(pDisplay, 0, 2);
              fprintf(pDisplay,"Warning: According to the input data, the following locations have a\n");
              fprintf(pDisplay,"         population totaling zero for the specified date(s).\n\n");
              PrintDirection.Printf("Warning: According to the input data, the following locations have a\n"
                                    "         population totaling zero for the specified date(s).\n\n",
                                    BasePrint::P_WARNING);
            }
            JulianToChar(sDateBuffer, gvPopulationDates[j]);
            if (pDisplay)
              fprintf(pDisplay,"         Location %s, %s\n", Data.GetTInfo()->getLocations().at(i)->getIndentifier(), sDateBuffer);
            PrintDirection.Printf("         Location %s, %s\n", BasePrint::P_WARNING, Data.GetTInfo()->getLocations().at(i)->getIndentifier(), sDateBuffer);
          }
       }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ReportZeroPops()","PopulationData");
    throw;
  }
}

/** Sets internal flag that indicates to aggregate population categories. Calling
    this function with parameter false after any population categories have already
    been created through MakePopulationCategory() is not recommended. */
void PopulationData::SetAggregateCovariateCategories(bool b) {
   gbAggregateCovariateCategories = b;

   if (gbAggregateCovariateCategories) {
     gvCovariateCategories.clear();
     giNumberCovariatesPerCategory = 0;
     gvCovariateCategories.push_back(std::vector<int>());
     gvCovariateCategoryCaseCount.resize(1, 0);
     gvCovariateCategoryControlCount.resize(1, 0);
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

