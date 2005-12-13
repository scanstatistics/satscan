//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeRandomizer.h"
#include "DataSet.h"

/** constructor */
SpaceTimeRandomizer::SpaceTimeRandomizer(long lInitialSeed) : AbstractRandomizer(lInitialSeed) {}

/** destructor */
SpaceTimeRandomizer::~SpaceTimeRandomizer() {}

/** returns a newly cloned SpaceTimeRandomizer */
SpaceTimeRandomizer * SpaceTimeRandomizer::Clone() const {
  return new SpaceTimeRandomizer(*this);
}

/** Assigns randomized data to SimDataSet objects' 'interval by location' case array. */
void SpaceTimeRandomizer::AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet) {
  int                                   iInterval, tNumTimeIntervals = thisRealSet.GetNumTimeIntervals();
  unsigned int                          tTract, tNumTracts = thisRealSet.GetNumTracts();
  count_t                            ** ppSimCases = thisSimSet.GetCaseArray();
  CategoryContainer_t::const_iterator   itr_category=gCategoryAttributes.begin(), itr_cat_end=gCategoryAttributes.end();
  StationaryContainer_t::const_iterator itr_stationary;
  PermutedContainer_t::const_iterator   itr_permuted, itr_end;

  //reset simulation case structure to zero
  thisSimSet.GetCaseArrayHandler().Set(0);

  //assign permuted attribute to simulation case array
  for (; itr_category != itr_cat_end; ++itr_category) {
     const StationaryContainer_t& Stationary = itr_category->gvStationaryAttribute;
     itr_stationary = Stationary.begin();
     const PermutedContainer_t& Permuted = itr_category->gvPermutedAttribute;
     itr_permuted = Permuted.begin();
     itr_end = Permuted.end();
     for (; itr_permuted != itr_end; ++itr_permuted, ++itr_stationary)
        ppSimCases[itr_permuted->GetPermutedVariable()][*itr_stationary]++;
  }

  //now set as cumulative
  for (tTract=0; tTract < tNumTracts; ++tTract)
     for (iInterval=tNumTimeIntervals-2; iInterval >= 0; --iInterval)
        ppSimCases[iInterval][tTract] = ppSimCases[iInterval+1][tTract] + ppSimCases[iInterval][tTract];
}

/** Creates structures used during randomizations - this function should be
    called once all case data has been read from file.
    NOTE: Inorder for this randomization process to remain consistant with
          previous versions, this function must performed as is; otherwise the initial
          ordering, when randomization starts, will differ. */
void SpaceTimeRandomizer::CreateRandomizationData(const RealDataSet& thisRealSet) {
  int	                iNumIntervals=thisRealSet.GetNumTimeIntervals();
  unsigned int          iNumCases, iNumCategories(thisRealSet.GetPopulationData().GetNumCovariateCategories());
  count_t            ** ppCases=0;

  gCategoryAttributes.resize(iNumCategories);
  for (unsigned int c=0; c < iNumCategories; ++c) {
     CategoryGrouping & theseCategoryAttributes = gCategoryAttributes[c];
     ppCases = thisRealSet.GetCategoryCaseArray(c);
     for (int i=iNumIntervals; i > 0; --i) {
        for (unsigned int j=0; j < thisRealSet.GetNumTracts(); ++j) {
           iNumCases = ppCases[i-1][j] - (i == iNumIntervals ? 0 : ppCases[i][j]);
           for (unsigned int k=0; k < iNumCases; ++k) {
              theseCategoryAttributes.gvStationaryAttribute.push_back(j);
              //add to vector which will maintain original order
              theseCategoryAttributes.gvOriginalPermutedAttribute.push_back(PermutedAttribute<int>(i-1));
           }
         }
     }
  }
}

/** Generates randomized data, assigning to 'interval by location' case array of SimDataSet object. */
void SpaceTimeRandomizer::RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation) {
  //set seed for simulation number
  SetSeed(iSimulation, thisSimSet.GetSetIndex());
  //assign random numbers to permuted attribute and sort
  SortPermutedAttribute();
  //re-assign dataset's simulation data
  AssignRandomizedData(thisRealSet, thisSimSet);
}

/** Re-assigns random number of permuted attribute and sorts. */
void SpaceTimeRandomizer::SortPermutedAttribute() {
  for (size_t t=0; t < gCategoryAttributes.size(); ++t) {
    PermutedContainer_t & theseAttributes = gCategoryAttributes[t].gvPermutedAttribute;

    // Restore permuted attributes to original order - this is needed to maintain
    // consistancy of output when running in parallel.
    theseAttributes = gCategoryAttributes[t].gvOriginalPermutedAttribute;
    //assign random number to each
    std::for_each(theseAttributes.begin(), theseAttributes.end(), AssignPermutedAttribute<PermutedAttribute<int> >(gRandomNumberGenerator));
    //randomize time intervals
    std::sort(theseAttributes.begin(), theseAttributes.end(), ComparePermutedAttribute<PermutedAttribute<int> >());
  }
}




