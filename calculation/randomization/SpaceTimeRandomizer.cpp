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
SpaceTimeRandomizer::SpaceTimeRandomizer(long lInitialSeed) : AbstractPermutedDataRandomizer(lInitialSeed) {}

/** destructor */
SpaceTimeRandomizer::~SpaceTimeRandomizer() {}

/** returns a newly cloned SpaceTimeRandomizer */
SpaceTimeRandomizer * SpaceTimeRandomizer::Clone() const {
  return new SpaceTimeRandomizer(*this);
}

/** Assigns randomized data to datasets' simulation case array. */
void SpaceTimeRandomizer::AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet) {
  int                                   iInterval, tNumTimeIntervals = thisRealSet.GetNumTimeIntervals();
  unsigned int                          tTract, tNumTracts = thisRealSet.GetNumTracts();
  count_t                            ** ppSimCases = thisSimSet.GetCaseArray();
  CategoryContainer_t::const_iterator   itr_category=gCategoryAttributes.begin(), itr_cat_end=gCategoryAttributes.end();
  StationaryContainer_t::const_iterator itr_stationary;
  PermutedContainer_t::const_iterator   itr_permuted, itr_end;

  //reset simulation case structure to zero
  thisSimSet.ResetCumulativeCaseArray();

  //assign permuted attribute to simulation case array
  for (; itr_category != itr_cat_end; ++itr_category) {
     const StationaryContainer_t& Stationary = itr_category->gvStationaryAttribute;
     itr_stationary = Stationary.begin();
     const PermutedContainer_t& Permuted = itr_category->gvPermutedAttribute;
     itr_permuted = Permuted.begin();
     itr_end = Permuted.end();
     for (; itr_permuted != itr_end; ++itr_permuted, ++itr_stationary)
        ppSimCases[itr_permuted->GetTimeInterval()][*itr_stationary]++;
  }

  //now set as cumulative
  for (tTract=0; tTract < tNumTracts; ++tTract)
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
  count_t            ** ppCases=0;

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
              //add to vector which will maintain original order
              theseCategoryAttributes.gvOriginalPermutedAttribute.push_back(PermutedTime(i));
           }
           vCummulatedCases[j] += iNumCases;
        }
     }
  }
}

/** Re-assigns random number to permuted attribute and sorts. */
void SpaceTimeRandomizer::SortPermutedAttribute() {
  for (size_t t=0; t < gCategoryAttributes.size(); ++t) {
    PermutedContainer_t & theseAttributes = gCategoryAttributes[t].gvPermutedAttribute;

    // Restore permuted attributes to original order - this is needed to maintain
    // consistancy of output when running in parallel.
    theseAttributes = gCategoryAttributes[t].gvOriginalPermutedAttribute;
    //assign random number to each
    std::for_each(theseAttributes.begin(), theseAttributes.end(), AssignPermutedAttribute(gRandomNumberGenerator));
    //randomize time intervals
    std::sort(theseAttributes.begin(), theseAttributes.end(), ComparePermutedAttribute());
  }
}




