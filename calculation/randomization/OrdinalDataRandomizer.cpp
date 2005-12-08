//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalDataRandomizer.h"

const size_t OrdinalDenominatorDataRandomizer::gtMaximumCategories = 5;

/** constructor */
OrdinalDenominatorDataRandomizer::OrdinalDenominatorDataRandomizer(long lInitialSeed)
                                 :AbstractOrdinalDenominatorDataRandomizer(lInitialSeed) {}

/** destructor */
OrdinalDenominatorDataRandomizer::~OrdinalDenominatorDataRandomizer() {}

/** returns pointer to newly cloned OrdinalDenominatorDataRandomizer */
OrdinalDenominatorDataRandomizer * OrdinalDenominatorDataRandomizer::Clone() const {
  return new OrdinalDenominatorDataRandomizer(*this);
}

/** Randomizes data sets' data for Ordinal model. */
void OrdinalDenominatorDataRandomizer::RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation) {
  count_t        ** ppSimCases=0;
   measure_t     ** ppMeasure=0;
   unsigned int     i, t;

  // reset seed of random number generator
  SetSeed(iSimulation, thisSimSet.GetSetIndex());
  //create measure array
  TwoDimensionArrayHandler<measure_t>  Measure(thisRealSet.GetNumTimeIntervals(), thisRealSet.GetNumTracts());
  SetMeasure(thisRealSet, Measure);
  ppMeasure = Measure.GetArray();
  //randomize data
  count_t tTotalControls = thisRealSet.GetTotalCases();
  for (size_t c=0; c < thisSimSet.GetCasesByCategory().size() - 1; ++c) {
     thisSimSet.GetCasesByCategory()[c]->Set(0);
     ppSimCases = thisSimSet.GetCasesByCategory()[c]->GetArray();
     tTotalControls -= thisRealSet.GetPopulationData().GetNumOrdinalCategoryCases(c);
     RandomizeOrdinalData(thisRealSet.GetPopulationData().GetNumOrdinalCategoryCases(c), tTotalControls,
                          ppSimCases, ppMeasure, thisRealSet.GetNumTracts(), thisRealSet.GetNumTimeIntervals());
     //update measure so that assigned cases are removed from measure
     for (i=0; i < thisRealSet.GetNumTimeIntervals(); ++i)
        for (t=0; t < thisRealSet.GetNumTracts(); ++t)
           ppMeasure[i][t] -= ppSimCases[i][t];
  }
  //after randomizing all categories except last, remaining cases can just be assigned respective category array
  ppSimCases = thisSimSet.GetCasesByCategory().back()->GetArray();
  for (i=0; i < thisRealSet.GetNumTimeIntervals(); ++i)
     for (t=0; t < thisRealSet.GetNumTracts(); ++t)
        ppSimCases[i][t] = static_cast<count_t>(ppMeasure[i][t]);
}

/** Calculates the total measure for ordinal data set, which is the cases in all categories added together. */
void OrdinalDenominatorDataRandomizer::SetMeasure(const RealDataSet& thisRealSet, TwoDimensionArrayHandler<measure_t>& Measure) {
  count_t       ** ppCases=0;
  measure_t     ** ppMeasure = Measure.GetArray();
  size_t           c, i, t;

  Measure.Set(0);
  //assign measure - total cases in all categories
  for (c=0; c < thisRealSet.GetCasesByCategory().size(); ++c) {
     ppCases = thisRealSet.GetCasesByCategory()[c]->GetArray();
     for (i=0; i < thisRealSet.GetNumTimeIntervals(); ++i)
        for (t=0; t < thisRealSet.GetNumTracts(); ++t)
           ppMeasure[i][t] += ppCases[i][t];
  }
}

/** returns pointer to newly cloned PermutatedVariable */
OrdinalPermutedDataRandomizer * OrdinalPermutedDataRandomizer::Clone() const {
  return new OrdinalPermutedDataRandomizer(*this);
}

/** Assigns randomized data to data set's simulation measure structures. */
void OrdinalPermutedDataRandomizer::AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  int                                   i, tNumTimeIntervals = thisSimSet.GetNumTimeIntervals();
  unsigned int                          t, tNumTracts = thisSimSet.GetNumTracts();
  count_t                            ** ppSimCases;

  //initialize counts to zero in all ordinal categories
  for (size_t c=0; c < thisSimSet.GetCasesByCategory().size(); ++c)
     thisSimSet.GetCasesByCategory()[c]->Set(0);

  //assign randomized continuous data to measure and measure squared arrays
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted)
    ++thisSimSet.GetCasesByCategory()[itr_permuted->GetPermutedVariable()]->GetArray()[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second];

  //now set as cumulative
  for (size_t c=0; c < thisSimSet.GetCasesByCategory().size(); ++c) {
     ppSimCases = thisSimSet.GetCasesByCategory()[c]->GetArray();
     for (t=0; t < tNumTracts; ++t)
        for (i=tNumTimeIntervals-2; i >= 0; --i)
           ppSimCases[i][t] = ppSimCases[i+1][t] + ppSimCases[i][t];
  }
}

/** From passed data set object, populates internal data structures which represent
    stationary and permuted attributes used during randomized data creation. */
void OrdinalPermutedDataRandomizer::CreateRandomizationData(const RealDataSet& thisRealSet) {
  count_t               tCase, tCaseCount, ** ppCases;
  unsigned int          i, t, iNumIntervals=thisRealSet.GetNumTimeIntervals();

  gvStationaryAttribute.clear();
  gvPermutedAttribute.clear();
  gvOriginalPermutedAttribute.clear();
  for (size_t c=0; c < thisRealSet.GetCasesByCategory().size(); ++c) {
     ppCases = thisRealSet.GetCasesByCategory()[c]->GetArray();
     for (i=0; i < thisRealSet.GetNumTimeIntervals(); ++i)
        for (t=0; t < thisRealSet.GetNumTracts(); ++t) {
           tCaseCount = (i == iNumIntervals - 1 ? ppCases[i][t] : ppCases[i][t] - ppCases[i+1][t]);
           for (tCase=0; tCase < tCaseCount; ++tCase) {
             gvStationaryAttribute.push_back(OrdinalStationary_t(std::make_pair(i, t)));
             gvPermutedAttribute.push_back(PermutedAttribute<int>(c));
             gvOriginalPermutedAttribute.push_back(PermutedAttribute<int>(c));
           }
        }
  }
}

/** re-initializes and  sorts permutated attribute */
void OrdinalPermutedDataRandomizer::SortPermutedAttribute() {
  // Reset permuted attributes to original order - this is needed to maintain
  // consistancy of output when running in parallel.
  gvPermutedAttribute = gvOriginalPermutedAttribute;

  std::for_each(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), AssignPermutedAttribute<OrdinalPermuted_t>(gRandomNumberGenerator));
  std::sort(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), ComparePermutedAttribute<OrdinalPermuted_t>());
}

