//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop                 
//---------------------------------------------------------------------------
#include "SpaceTimeRandomizer.h"

/** constructor */
PermutedTime::PermutedTime(int iTimeInterval) :  PermutedAttribute(), giTimeIntervalIndex(iTimeInterval) {}

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

/** Assigns randomized data to data stream's simulation case array. */
void SpaceTimeRandomizer::AssignRandomizedData(DataStream & thisStream) {
  size_t          tCategory, tCase;
  int             iInterval, tNumTimeIntervals = thisStream.GetNumTimeIntervals();
  unsigned int    tTract, tNumTracts = thisStream.GetNumTracts();
  count_t      ** ppSimCases = thisStream.GetSimCaseArray();

  //reset simulation case structure to zero
  thisStream.ResetCumulativeSimCaseArray();

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

/** Re-assigns random number to permuted attribute and sorts. */
void SpaceTimeRandomizer::SortPermutedAttribute() {
  for (size_t t=0; t < gCategoryAttributes.size(); ++t) {
    ZdPointerVector<PermutedTime> & theseAttributes = gCategoryAttributes[t].gvPermutedAttribute;
    std::for_each(theseAttributes.begin(), theseAttributes.end(), AssignPermutedAttribute(gRandomNumberGenerator));
    std::sort(theseAttributes.begin(), theseAttributes.end(), ComparePermutedAttribute());
  }
}




