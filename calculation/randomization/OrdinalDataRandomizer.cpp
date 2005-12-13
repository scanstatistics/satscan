//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalDataRandomizer.h"

const size_t OrdinalDenominatorDataRandomizer::gtMaximumCategories = 5;

/** Creates randomized under the null hypothesis for Ordinal model, assigning data to SimDataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void OrdinalDenominatorDataRandomizer::RandomizeData(const RealDataSet& RealSet, SimDataSet& SimSet, unsigned int iSimulation) {
  count_t                            ** ppSimCases, ** ppCases, tRemainingControls=RealSet.GetTotalCases();
  measure_t                          ** ppMeasure;
  unsigned int                          iNumTracts=RealSet.GetNumTracts(), iNumIntervals=RealSet.GetNumTimeIntervals(),
                                        iNumCategories=SimSet.GetCasesByCategory().size();
  TwoDimensionArrayHandler<measure_t>   tempMeasure(RealSet.GetNumTimeIntervals(), RealSet.GetNumTracts(), 0);

  // reset seed of random number generator
  SetSeed(iSimulation, SimSet.GetSetIndex());
  //create temporary measure array, which is all categories combined together
  ppMeasure = tempMeasure.GetArray();
  for (size_t c=0; c < iNumCategories; ++c) {
     ppCases = RealSet.GetCasesByCategory()[c]->GetArray();
     for (size_t i=0; i < iNumIntervals; ++i)
        for (size_t t=0; t < iNumTracts; ++t)
           ppMeasure[i][t] += ppCases[i][t];
  }
  //randomize data - treating the remaining cases in temporary measure array as 'controls'
  for (size_t c=0; c < iNumCategories - 1; ++c) {
     SimSet.GetCasesByCategory()[c]->Set(0);
     ppSimCases = SimSet.GetCasesByCategory()[c]->GetArray();
     tRemainingControls -= RealSet.GetPopulationData().GetNumOrdinalCategoryCases(c);
     RandomizeOrdinalData(RealSet.GetPopulationData().GetNumOrdinalCategoryCases(c), tRemainingControls, ppSimCases, ppMeasure, iNumTracts, iNumIntervals);
     //update measure so that assigned cases are removed from measure
     for (unsigned int i=0; i < iNumIntervals; ++i)
        for (unsigned int t=0; t < iNumTracts; ++t)
           ppMeasure[i][t] -= ppSimCases[i][t];
  }
  //after randomizing all categories except last, remaining cases can just be assigned respective category array
  ppSimCases = SimSet.GetCasesByCategory().back()->GetArray();
  for (unsigned int i=0; i < iNumIntervals; ++i)
     for (unsigned int t=0; t < iNumTracts; ++t)
        ppSimCases[i][t] = static_cast<count_t>(ppMeasure[i][t]);
}
//******************************************************************************

/** Creates randomized under the null hypothesis for Ordinal model, assigning data to SimDataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void OrdinalPurelyTemporalDenominatorDataRandomizer::RandomizeData(const RealDataSet& RealSet, SimDataSet& SimSet, unsigned int iSimulation) {
  count_t                     * pSimCases, * pCases, tRemainingControls=RealSet.GetTotalCases();
  measure_t                   * pMeasure;  
  unsigned int                  iNumIntervals=RealSet.GetNumTimeIntervals();
  std::vector<measure_t>        tempMeasure(iNumIntervals, 0);

  // reset seed of random number generator
  SetSeed(iSimulation, SimSet.GetSetIndex());
  //create temporary measure array, which is all categories combined together
  pMeasure = &tempMeasure[0];
  for (size_t c=0; c < RealSet.GetPTCategoryCasesArrayHandler().Get1stDimension(); ++c) {
     pCases = RealSet.GetPTCategoryCasesArrayHandler().GetArray()[c];
     for (size_t i=0; i < iNumIntervals; ++i)
         pMeasure[i] += pCases[i];
  }
  //randomize data - treating the remaining cases in temporary measure array as 'controls'
  for (size_t c=0; c < SimSet.GetPTCategoryCasesArrayHandler().Get1stDimension() - 1; ++c) {
     pSimCases = SimSet.GetPTCategoryCasesArrayHandler().GetArray()[c];
     memset(pSimCases, 0, SimSet.GetPTCategoryCasesArrayHandler().Get2ndDimension() * sizeof(count_t));
     tRemainingControls -= RealSet.GetPopulationData().GetNumOrdinalCategoryCases(c);
     RandomizePurelyTemporalOrdinalData(RealSet.GetPopulationData().GetNumOrdinalCategoryCases(c), tRemainingControls,
                                        pSimCases, pMeasure, RealSet.GetNumTimeIntervals());
     //update measure so that assigned cases are removed from measure
     for (unsigned int i=0; i < iNumIntervals; ++i)
        tempMeasure[i] -= pSimCases[i];
  }
  //after randomizing all categories except last, remaining cases can just be assigned respective category array
  pSimCases=SimSet.GetPTCategoryCasesArrayHandler().GetArray()[SimSet.GetPTCategoryCasesArrayHandler().Get1stDimension()-1];
  for (unsigned int i=0; i < iNumIntervals; ++i)
    pSimCases[i] = static_cast<count_t>(tempMeasure[i]);
}
//******************************************************************************

/** Assigns randomized data to SimDataSet objects's data structures. */
void OrdinalPermutedDataRandomizer::AssignRandomizedData(const RealDataSet& RealSet, SimDataSet& SimSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  int                                   i, tNumTimeIntervals = SimSet.GetNumTimeIntervals();
  unsigned int                          t, tNumTracts = SimSet.GetNumTracts();
  count_t                            ** ppSimCases;

  //initialize counts to zero in all ordinal categories
  for (size_t c=0; c < SimSet.GetCasesByCategory().size(); ++c)
     SimSet.GetCasesByCategory()[c]->Set(0);

  //assign randomized continuous data to measure and measure squared arrays
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted)
    ++SimSet.GetCasesByCategory()[itr_permuted->GetPermutedVariable()]->GetArray()[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second];

  //now set as cumulative
  for (size_t c=0; c < SimSet.GetCasesByCategory().size(); ++c) {
     ppSimCases = SimSet.GetCasesByCategory()[c]->GetArray();
     for (t=0; t < tNumTracts; ++t)
        for (i=tNumTimeIntervals-2; i >= 0; --i)
           ppSimCases[i][t] = ppSimCases[i+1][t] + ppSimCases[i][t];
  }
}

/** From passed data set object, populates internal data structures which represent
    stationary and permuted attributes used during randomized data creation. */
void OrdinalPermutedDataRandomizer::Setup(const RealDataSet& RealSet) {
  count_t               tCase, tCaseCount, ** ppCases;
  unsigned int          i, t, iNumTract=RealSet.GetNumTracts(), iNumIntervals=RealSet.GetNumTimeIntervals();

  gvStationaryAttribute.clear();
  gvPermutedAttribute.clear();
  gvOriginalPermutedAttribute.clear();
  for (size_t c=0; c < RealSet.GetCasesByCategory().size(); ++c) {
     ppCases = RealSet.GetCasesByCategory()[c]->GetArray();
     for (i=0; i < iNumIntervals; ++i)
        for (t=0; t < iNumTract; ++t) {
           tCaseCount = (i == iNumIntervals - 1 ? ppCases[i][t] : ppCases[i][t] - ppCases[i+1][t]);
           for (tCase=0; tCase < tCaseCount; ++tCase) {
             gvStationaryAttribute.push_back(OrdinalStationary_t(std::make_pair(i, t)));
             gvPermutedAttribute.push_back(PermutedAttribute<int>(c));
             gvOriginalPermutedAttribute.push_back(PermutedAttribute<int>(c));
           }
        }
  }
}
//******************************************************************************

 /** Assigns randomized data to SimDataSet objects's data structures. */
void OrdinalPurelyTemporalPermutedDataRandomizer::AssignRandomizedData(const RealDataSet& RealSet, SimDataSet& SimSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  int                                   i, tNumTimeIntervals = SimSet.GetNumTimeIntervals();
  count_t                             * pSimCases;

  //initialize counts to zero in all ordinal categories
  for (size_t c=0; c < SimSet.GetPTCategoryCasesArrayHandler().Get1stDimension(); ++c)
     memset(SimSet.GetPTCategoryCasesArrayHandler().GetArray()[c], 0, SimSet.GetPTCategoryCasesArrayHandler().Get2ndDimension() * sizeof(count_t));

  //assign randomized continuous data to measure and measure squared arrays
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted)
    ++SimSet.GetPTCategoryCasesArrayHandler().GetArray()[itr_permuted->GetPermutedVariable()][itr_stationary->GetStationaryVariable()];

  //now set as cumulative
  for (size_t c=0; c < SimSet.GetPTCategoryCasesArrayHandler().Get1stDimension(); ++c) {
     pSimCases = SimSet.GetPTCategoryCasesArrayHandler().GetArray()[c];
     for (i=tNumTimeIntervals-2; i >= 0; --i)
        pSimCases[i] =  pSimCases[i+1] + pSimCases[i];
  }
}

/** From passed data set object, populates internal data structures which represent
    stationary and permuted attributes used during randomized data creation. */
void OrdinalPurelyTemporalPermutedDataRandomizer::Setup(const RealDataSet& RealSet) {
  count_t               tCase, tCaseCount, ** ppCases;
  unsigned int          i, t, iNumIntervals=RealSet.GetNumTimeIntervals();

  gvStationaryAttribute.clear();
  gvPermutedAttribute.clear();
  gvOriginalPermutedAttribute.clear();
  for (size_t c=0; c < RealSet.GetCasesByCategory().size(); ++c) {
     ppCases = RealSet.GetCasesByCategory()[c]->GetArray();
     for (i=0; i < RealSet.GetNumTimeIntervals(); ++i)
        for (t=0; t < RealSet.GetNumTracts(); ++t) {
           tCaseCount = (i == iNumIntervals - 1 ? ppCases[i][t] : ppCases[i][t] - ppCases[i+1][t]);
           for (tCase=0; tCase < tCaseCount; ++tCase) {
             gvStationaryAttribute.push_back(OrdinalPurelyTemporalStationary_t(i));
             gvPermutedAttribute.push_back(PermutedAttribute<int>(c));
             gvOriginalPermutedAttribute.push_back(PermutedAttribute<int>(c));
           }
        }
  }
}

