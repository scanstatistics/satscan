//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalDataRandomizer.h"

const size_t OrdinalDenominatorDataRandomizer::gtMaximumCategories = 5;

/** Creates randomized under the null hypothesis for Ordinal model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void OrdinalDenominatorDataRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  count_t                            ** ppSimCases, ** ppCases, tRemainingControls=RealSet.getTotalCases();
  measure_t                          ** ppMeasure;
  unsigned int                          iNumTracts=RealSet.getLocationDimension(), iNumIntervals=RealSet.getIntervalDimension(),
                                        iNumCategories=SimSet.getCaseData_Cat().size();
  TwoDimensionArrayHandler<measure_t>   tempMeasure(RealSet.getIntervalDimension(), RealSet.getLocationDimension(), 0);

  // reset seed of random number generator
  SetSeed(iSimulation, SimSet.getSetIndex());
  //create temporary measure array, which is all categories combined together
  ppMeasure = tempMeasure.GetArray();
  for (size_t c=0; c < iNumCategories; ++c) {
     ppCases = RealSet.getCaseData_Cat()[c]->GetArray();
     for (size_t i=0; i < iNumIntervals; ++i)
        for (size_t t=0; t < iNumTracts; ++t)
           ppMeasure[i][t] += ppCases[i][t];
  }
  //randomize data - treating the remaining cases in temporary measure array as 'controls'
  for (size_t c=0; c < iNumCategories - 1; ++c) {
     SimSet.getCaseData_Cat()[c]->Set(0);
     ppSimCases = SimSet.getCaseData_Cat()[c]->GetArray();
     tRemainingControls -= RealSet.getPopulationData().GetNumOrdinalCategoryCases(c);
     RandomizeOrdinalData(RealSet.getPopulationData().GetNumOrdinalCategoryCases(c), tRemainingControls, ppSimCases, ppMeasure, iNumTracts, iNumIntervals);
     //update measure so that assigned cases are removed from measure
     for (unsigned int i=0; i < iNumIntervals; ++i)
        for (unsigned int t=0; t < iNumTracts; ++t)
           ppMeasure[i][t] -= ppSimCases[i][t];
  }
  //after randomizing all categories except last, remaining cases can just be assigned respective category array
  ppSimCases = SimSet.getCaseData_Cat().back()->GetArray();
  for (unsigned int i=0; i < iNumIntervals; ++i)
     for (unsigned int t=0; t < iNumTracts; ++t)
        ppSimCases[i][t] = static_cast<count_t>(ppMeasure[i][t]);
}
//******************************************************************************

/** Creates randomized under the null hypothesis for Ordinal model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void OrdinalPurelyTemporalDenominatorDataRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  count_t                     * pSimCases, * pCases, tRemainingControls=RealSet.getTotalCases();
  measure_t                   * pMeasure;  
  unsigned int                  iNumIntervals=RealSet.getIntervalDimension();
  std::vector<measure_t>        tempMeasure(iNumIntervals, 0);

  // reset seed of random number generator
  SetSeed(iSimulation, SimSet.getSetIndex());
  //create temporary measure array, which is all categories combined together
  pMeasure = &tempMeasure[0];
  for (size_t c=0; c < RealSet.getCaseData_PT_Cat().Get1stDimension(); ++c) {
     pCases = RealSet.getCaseData_PT_Cat().GetArray()[c];
     for (size_t i=0; i < iNumIntervals; ++i)
         pMeasure[i] += pCases[i];
  }
  //randomize data - treating the remaining cases in temporary measure array as 'controls'
  for (size_t c=0; c < SimSet.getCaseData_PT_Cat().Get1stDimension() - 1; ++c) {
     pSimCases = SimSet.getCaseData_PT_Cat().GetArray()[c];
     memset(pSimCases, 0, SimSet.getCaseData_PT_Cat().Get2ndDimension() * sizeof(count_t));
     tRemainingControls -= RealSet.getPopulationData().GetNumOrdinalCategoryCases(c);
     RandomizePurelyTemporalOrdinalData(RealSet.getPopulationData().GetNumOrdinalCategoryCases(c), tRemainingControls,
                                        pSimCases, pMeasure, RealSet.getIntervalDimension());
     //update measure so that assigned cases are removed from measure
     for (unsigned int i=0; i < iNumIntervals; ++i)
        tempMeasure[i] -= pSimCases[i];
  }
  //after randomizing all categories except last, remaining cases can just be assigned respective category array
  pSimCases=SimSet.getCaseData_PT_Cat().GetArray()[SimSet.getCaseData_PT_Cat().Get1stDimension()-1];
  for (unsigned int i=0; i < iNumIntervals; ++i)
    pSimCases[i] = static_cast<count_t>(tempMeasure[i]);
}
//******************************************************************************

/** Assigns randomized data to DataSet objects's data structures. */
void OrdinalPermutedDataRandomizer::AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  int                                   i, tNumTimeIntervals = SimSet.getIntervalDimension();
  unsigned int                          t, tNumTracts = SimSet.getLocationDimension();
  count_t                            ** ppSimCases;

  //initialize counts to zero in all ordinal categories
  for (size_t c=0; c < SimSet.getCaseData_Cat().size(); ++c)
     SimSet.getCaseData_Cat()[c]->Set(0);

  //assign randomized continuous data to measure and measure squared arrays
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted)
    ++SimSet.getCaseData_Cat()[itr_permuted->GetPermutedVariable()]->GetArray()[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second];

  //now set as cumulative
  for (size_t c=0; c < SimSet.getCaseData_Cat().size(); ++c) {
     ppSimCases = SimSet.getCaseData_Cat()[c]->GetArray();
     for (t=0; t < tNumTracts; ++t)
        for (i=tNumTimeIntervals-2; i >= 0; --i)
           ppSimCases[i][t] = ppSimCases[i+1][t] + ppSimCases[i][t];
  }
}

/** From passed data set object, populates internal data structures which represent
    stationary and permuted attributes used during randomized data creation. */
void OrdinalPermutedDataRandomizer::Setup(const RealDataSet& RealSet) {
  count_t               tCase, tCaseCount, ** ppCases;
  unsigned int          i, t, iNumTract=RealSet.getLocationDimension(), iNumIntervals=RealSet.getIntervalDimension();

  gvStationaryAttribute.clear();
  gvPermutedAttribute.clear();
  gvOriginalPermutedAttribute.clear();
  for (size_t c=0; c < RealSet.getCaseData_Cat().size(); ++c) {
     ppCases = RealSet.getCaseData_Cat()[c]->GetArray();
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

 /** Assigns randomized data to DataSet objects's data structures. */
void OrdinalPurelyTemporalPermutedDataRandomizer::AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  int                                   i, tNumTimeIntervals = SimSet.getIntervalDimension();
  count_t                             * pSimCases;

  //initialize counts to zero in all ordinal categories
  for (size_t c=0; c < SimSet.getCaseData_PT_Cat().Get1stDimension(); ++c)
     memset(SimSet.getCaseData_PT_Cat().GetArray()[c], 0, SimSet.getCaseData_PT_Cat().Get2ndDimension() * sizeof(count_t));

  //assign randomized continuous data to measure and measure squared arrays
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted)
    ++SimSet.getCaseData_PT_Cat().GetArray()[itr_permuted->GetPermutedVariable()][itr_stationary->GetStationaryVariable()];

  //now set as cumulative
  for (size_t c=0; c < SimSet.getCaseData_PT_Cat().Get1stDimension(); ++c) {
     pSimCases = SimSet.getCaseData_PT_Cat().GetArray()[c];
     for (i=tNumTimeIntervals-2; i >= 0; --i)
        pSimCases[i] =  pSimCases[i+1] + pSimCases[i];
  }
}

/** From passed data set object, populates internal data structures which represent
    stationary and permuted attributes used during randomized data creation. */
void OrdinalPurelyTemporalPermutedDataRandomizer::Setup(const RealDataSet& RealSet) {
  count_t               tCase, tCaseCount, ** ppCases;
  unsigned int          i, t, iNumIntervals=RealSet.getIntervalDimension();

  gvStationaryAttribute.clear();
  gvPermutedAttribute.clear();
  gvOriginalPermutedAttribute.clear();
  for (size_t c=0; c < RealSet.getCaseData_Cat().size(); ++c) {
     ppCases = RealSet.getCaseData_Cat()[c]->GetArray();
     for (i=0; i < RealSet.getIntervalDimension(); ++i)
        for (t=0; t < RealSet.getLocationDimension(); ++t) {
           tCaseCount = (i == iNumIntervals - 1 ? ppCases[i][t] : ppCases[i][t] - ppCases[i+1][t]);
           for (tCase=0; tCase < tCaseCount; ++tCase) {
             gvStationaryAttribute.push_back(OrdinalPurelyTemporalStationary_t(i));
             gvPermutedAttribute.push_back(PermutedAttribute<int>(c));
             gvOriginalPermutedAttribute.push_back(PermutedAttribute<int>(c));
           }
        }
  }
}

