//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ExponentialRandomizer.h"

/** constructor */
PermutedExponentialAttributes::PermutedExponentialAttributes(double dVariable, unsigned short uCensored)
                              :PermutedVariable(dVariable), guCensored(uCensored) {}

/** destructor */
PermutedExponentialAttributes::~PermutedExponentialAttributes() {}

/** returns pointer to newly cloned PermutatedVariable */
PermutedExponentialAttributes * PermutedExponentialAttributes::Clone() const {
  return new PermutedExponentialAttributes(*this);
}


/** constructor */
ExponentialRandomizer::ExponentialRandomizer() : AbstractPermutedDataRandomizer() {}

/** destructor */
ExponentialRandomizer::~ExponentialRandomizer() {}

/** returns pointer to newly cloned ExponetialRandomizer */
ExponentialRandomizer * ExponentialRandomizer::Clone() const {
  return new ExponentialRandomizer(*this);
}

/** Adds new randomization entry with passed values. */
void ExponentialRandomizer::AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, count_t tCensored) {
  //add stationary values
  gvStationaryAttribute.push_back(SpaceTimeStationaryAttribute(iTimeInterval, tTractIndex));
  //add permutated value
  gvPermutedAttribute.push_back(0);
  gvPermutedAttribute[gvPermutedAttribute.size() - 1] = new PermutedExponentialAttributes(tContinuousVariable, tCensored);
}


/** Assigns data in randomizer to case and measure structures.
    NOTE: Correctness of passed iNumTimeIntervals and iNumTracts, in relation
          to passed arrays is responsibility of caller. */
void ExponentialRandomizer::Assign(count_t ** ppCases, measure_t ** ppMeasure, int iNumTimeIntervals, int iNumTracts) {
  std::vector<SpaceTimeStationaryAttribute>::iterator            itr_stationary=gvStationaryAttribute.begin();
  ZdPointerVector<PermutedExponentialAttributes>::const_iterator itr_permuted=gvPermutedAttribute.begin();
  int                                                            i, tTract;

  //assign randomized continuous data to measure
  for (; itr_stationary != gvStationaryAttribute.end(); ++itr_permuted, ++itr_stationary) {
     ppMeasure[itr_stationary->GetTimeInterval()][itr_stationary->GetTractIndex()] += (*itr_permuted)->GetVariable();
     ppCases[itr_stationary->GetTimeInterval()][itr_stationary->GetTractIndex()] += (*itr_permuted)->GetCensored();
  }

  //now set as cumulative
  for (tTract=0; tTract < iNumTracts; ++tTract)
     for (i=iNumTimeIntervals-2; i >= 0; --i) {
        ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
        ppCases[i][tTract] = ppCases[i+1][tTract] + ppCases[i][tTract];
     }
}

/** Assigns randomized data to dataset's simulation measure structures. */
void ExponentialRandomizer::AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet) {
  //reset simulation case structure to zero
  thisSimSet.ResetCumulativeCaseArray();
  //reset simulation measure array to zero
  thisSimSet.GetMeasureArrayHandler().Set(0);
  Assign(thisSimSet.GetCaseArray(), thisSimSet.GetMeasureArray(), thisRealSet.GetNumTimeIntervals(), thisRealSet.GetNumTracts());
}

/** re-initializes and  sorts permutated attribute */
void ExponentialRandomizer::SortPermutedAttribute() {
  std::for_each(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), AssignPermutedAttribute(gRandomNumberGenerator));
  std::sort(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), ComparePermutedAttribute());
}

