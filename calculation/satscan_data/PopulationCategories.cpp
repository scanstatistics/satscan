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

/** Prints formatted text depicting state of population categories. */
void PopulationCategories::Display(BasePrint & PrintDirection) const {
  size_t        t, j;

  try {
    PrintDirection.SatScanPrintf("DISPLAY: Number of categories = %i\n", gvPopulationCategories.size());
    PrintDirection.SatScanPrintf("\n#   Category Combination\n");
    for (t=0; t < gvPopulationCategories.size(); t++) {
       PrintDirection.SatScanPrintf("%d     ",  t);
       for (j=0; j < gvPopulationCategories[t].size(); j++)
          PrintDirection.SatScanPrintf("%s  ", gvCovariateNames[gvPopulationCategories[t][j]]);
       PrintDirection.SatScanPrintf("\n");
    }
    PrintDirection.SatScanPrintf("\n");
  }
  catch (ZdException &x) {
    x.AddCallpath("Display()", "PopulationCategories");
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

    sBuffer.clear();
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
int PopulationCategories::MakePopulationCategory(StringParser & Parser, int iLineNumber, BasePrint & PrintDirection) {
  int                                           iScanOffset, iCategoryIndex, iNumCovariatesScanned=0;
  std::vector<int>                              vPopulationCategory;
  const char                                  * pCovariate;
  std::vector<std::string>::iterator            itr;
  std::vector<std::vector<int> >::iterator      itr_int;

  iScanOffset = 3; //tract identifier, population date, population, covariate 1, ...
  
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
  }
  else if (iNumCovariatesScanned != giNumberCovariates){
    PrintDirection.PrintInputWarning("Error: Record %d of population file contains %d covariates\n",
                                     iLineNumber, iNumCovariatesScanned);
    PrintDirection.PrintInputWarning("       but the correct number of covariates is %d, ", giNumberCovariates);
    PrintDirection.PrintInputWarning("as defined by first record.\n");
    iCategoryIndex = -1;
  }
  else {
    //if list of covariates is unique then add to list of categories, else get lists index
    itr_int = std::find(gvPopulationCategories.begin(), gvPopulationCategories.end(), vPopulationCategory);
    if (itr_int == gvPopulationCategories.end()) {
      gvPopulationCategories.push_back(vPopulationCategory);
      iCategoryIndex = gvPopulationCategories.size() - 1;
    }
    else
      iCategoryIndex = std::distance(gvPopulationCategories.begin(), itr_int);
  }

  return iCategoryIndex;
}
