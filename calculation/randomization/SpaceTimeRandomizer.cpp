//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeRandomizer.h"
#include "DataSet.h"

/** constructor */
PermutedTime::PermutedTime(int iTimeInterval) : PermutedAttribute(), giTimeIntervalIndex(iTimeInterval) {}

/** destructor */
PermutedTime::~PermutedTime() {}

/** returns a newly cloned PermutedTime */
PermutedTime * PermutedTime::Clone() const {
  return new PermutedTime(*this);
}

/** constructor */
SpaceTimeRandomizer::SpaceTimeRandomizer() : AbstractPermutedDataRandomizer() {}

/** destructor */
SpaceTimeRandomizer::~SpaceTimeRandomizer() {}

/** returns a newly cloned SpaceTimeRandomizer */
SpaceTimeRandomizer * SpaceTimeRandomizer::Clone() const {
  return new SpaceTimeRandomizer(*this);
}

/** Adds new randomization entry with passed values. */
void SpaceTimeRandomizer::AddCase(unsigned int iCategory, int iTimeInterval, tract_t tTractIndex) {
  if (iCategory >= gCategoryAttributes.size())
    gCategoryAttributes.resize(iCategory + 1);

  gCategoryAttributes[iCategory].gvStationaryAttribute.push_back(tTractIndex);
  gCategoryAttributes[iCategory].gvPermutedAttribute.push_back(0);
  gCategoryAttributes[iCategory].gvPermutedAttribute[gCategoryAttributes[iCategory].gvPermutedAttribute.size() - 1] = new PermutedTime(iTimeInterval);
}

/** Assigns randomized data to datasets' simulation case array. */
void SpaceTimeRandomizer::AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet) {
  size_t          tCategory, tCase;
  int             iInterval, tNumTimeIntervals = thisRealSet.GetNumTimeIntervals();
  unsigned int    tTract, tNumTracts = thisRealSet.GetNumTracts();
  count_t      ** ppSimCases = thisSimSet.GetCaseArray();

  //reset simulation case structure to zero
  thisSimSet.ResetCumulativeCaseArray();

  //assign permuted attribute to simulation case array
  for (size_t t=0; t < gCategoryAttributes.size(); ++t) {
     std::vector<tract_t>& Stationary = gCategoryAttributes[t].gvStationaryAttribute;
     ZdPointerVector<PermutedTime>& Permuted = gCategoryAttributes[t].gvPermutedAttribute;
    for (tCase=0; tCase < Permuted.size(); ++tCase)
        ppSimCases[Permuted[tCase]->GetTimeInterval()][Stationary[tCase]]++;
  }

  //now set as cumulative
  for (tTract=0; tTract < tNumTracts; tTract++)
     for (iInterval=tNumTimeIntervals-2; iInterval >= 0; --iInterval)
        ppSimCases[iInterval][tTract] = ppSimCases[iInterval+1][tTract] + ppSimCases[iInterval][tTract];
}

/** Creates structures used during randomizations - this functions should be
    called once all cases data has been read from file.
    NOTE: Inorder for this randomization process to remain consistant with
          previous versions, this function and the sorting of the time interval
          in SortPermutedAttribute() must be done. */
void SpaceTimeRandomizer::CreateRandomizationData(const RealDataSet& thisRealSet) {
  int	                i;
  unsigned int          j, k, c, iNumCases, iNumCategories(thisRealSet.GetPopulationData().GetNumCovariateCategories());
  std::vector<int>      vCummulatedCases;
  count_t               iMaxCasesPerCategory, ** ppCases=0;

  gCategoryAttributes.resize(iNumCategories);
  vCummulatedCases.resize(thisRealSet.GetNumTracts());
  
  for (c=0; c < iNumCategories; ++c) {
     CategoryGrouping & theseCategoryAttributes = gCategoryAttributes[c];
     memset(&vCummulatedCases[0], 0, thisRealSet.GetNumTracts()*sizeof(int));
     ppCases = thisRealSet.GetCategoryCaseArray(c);
     for (i=thisRealSet.GetNumTimeIntervals() - 1; i >= 0; --i) {
        for (j=0; j < thisRealSet.GetNumTracts(); ++j) {
           iNumCases = ppCases[i][j] - vCummulatedCases[j];
           for (k=0; k < iNumCases; ++k) {
              theseCategoryAttributes.gvStationaryAttribute.push_back(j);
              theseCategoryAttributes.gvPermutedAttribute.push_back(0);
              theseCategoryAttributes.gvPermutedAttribute[theseCategoryAttributes.gvPermutedAttribute.size() - 1] = new PermutedTime(i);
           }   
           vCummulatedCases[j] += iNumCases;
        }
     }
  }
}

/** Re-assigns random number to permuted attribute and sorts. */
void SpaceTimeRandomizer::SortPermutedAttribute() {
  for (size_t t=0; t < gCategoryAttributes.size(); ++t) {
    ZdPointerVector<PermutedTime> & theseAttributes = gCategoryAttributes[t].gvPermutedAttribute;

    //re-sort time intervals to descending order - this is needed to maintain consistancy
    //with previous versions
    std::sort(theseAttributes.begin(), theseAttributes.end(), ComparePermutedTime());
    //assign random number to each
    std::for_each(theseAttributes.begin(), theseAttributes.end(), AssignPermutedAttribute(gRandomNumberGenerator));
    //randomize time intervals
    std::sort(theseAttributes.begin(), theseAttributes.end(), ComparePermutedAttribute());
  }
}




