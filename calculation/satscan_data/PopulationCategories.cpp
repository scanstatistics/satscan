#include "SaTScan.h"
#pragma hdrstop
#include "PopulationCategories.h"
#include "UtilityFunctions.h"

/** constructor */
PopulationCategories::PopulationCategories() {
  Init();
}

/** destructor */
PopulationCategories::~PopulationCategories() {}

void PopulationCategories::AddCaseCount(int iCategoryIndex, count_t Count) {
  if (iCategoryIndex < 0 || iCategoryIndex > (int)gvCategoryCasesCount.size() - 1)
    ZdGenerateException("Index '%d' out of range.","AddCaseCount()", iCategoryIndex);
  gvCategoryCasesCount[iCategoryIndex] += Count;
  if (gvCategoryCasesCount[iCategoryIndex] < 0)
    SSGenerateException("Error: Total cases greater than maximum allowed of %ld.\n", "AddCaseCount()", std::numeric_limits<count_t>::max());
}

void PopulationCategories::AddControlCount(int iCategoryIndex, count_t Count) {
  if (iCategoryIndex < 0 || iCategoryIndex > (int)gvCategoryControlsCount.size() - 1)
    ZdGenerateException("Index '%d' out of range.","AddControlCount()", iCategoryIndex);
  gvCategoryControlsCount[iCategoryIndex] += Count;
  if (gvCategoryControlsCount[iCategoryIndex] < 0)
    SSGenerateException("Error: Total controls greater than maximum allowed of %ld.\n", "AddControlCount()", std::numeric_limits<count_t>::max());
}

/** Prints formatted text depicting state of population categories. */
void PopulationCategories::Display(BasePrint & PrintDirection) const {
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
    x.AddCallpath("Display()", "PopulationCategories");
    throw;
  }
}

count_t PopulationCategories::GetNumCategoryCases(int iCategoryIndex) const {
  try {
    if (iCategoryIndex < 0 || iCategoryIndex > static_cast<int>(gvCategoryCasesCount.size()) - 1)
      ZdGenerateException("Index '%d' out of ranges.","GetNumCategoryCases()");

    return gvCategoryCasesCount[iCategoryIndex];
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNumCategoryCases()", "PopulationCategories");
    throw;
  }
}

count_t PopulationCategories::GetNumCategoryControls(int iCategoryIndex) const {
  try {
    if (iCategoryIndex < 0 || iCategoryIndex > static_cast<int>(gvCategoryControlsCount.size()) - 1)
      ZdGenerateException("Index '%d' out of ranges.","GetNumCategoryControls()");

    return gvCategoryControlsCount[iCategoryIndex];
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNumCategoryControls()", "PopulationCategories");
    throw;
  }
}

/** Returns category index for passed vector of covariate strings. Returns -1 if not found. */
int PopulationCategories::GetPopulationCategoryIndex(const std::vector<std::string>& vCategoryCovariates) const {
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
const char * PopulationCategories::GetPopulationCategoryAsString(int iCategoryIndex, std::string & sBuffer) const {
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
    x.AddCallpath("GetPopulationCategoryAsString()", "PopulationCategories");
    throw;
  }
  return sBuffer.c_str();
}

/** Creates new population category and returns category index. */
int PopulationCategories::MakePopulationCategory(const char* szDescription, StringParser & Parser, int iScanOffset, BasePrint & PrintDirection) {
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

void PopulationCategories::SetAggregateCategories(bool b) {
   gbAggregateCategories = b;

   if (gbAggregateCategories) {
     gvPopulationCategories.clear();
     giNumberCovariates = 0;
     gvPopulationCategories.push_back(std::vector<int>());
     gvCategoryCasesCount.resize(1, 0);
     gvCategoryControlsCount.resize(1, 0);
   }
}

