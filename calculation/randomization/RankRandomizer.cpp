//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "RankRandomizer.h"

/** Adds new randomization entry with passed attrbiute values. */
void AbstractRankRandomizer::AddCase(count_t tCount, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable) {
  for (count_t t=0; t < tCount; ++t) {
    //add stationary values
    gvStationaryAttribute.push_back(RankStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
    //add permutated value
    gvPermutedAttribute.push_back(RankPermuted_t(tContinuousVariable));
    //add to vector which maintains original order
    gvOriginalPermutedAttribute.push_back(RankPermuted_t(tContinuousVariable));
  }
}

/** Allocates and sets appropriate data structures of RealDataSet object from internal
    collection of attributes. */
void AbstractRankRandomizer::AssignFromAttributes(RealDataSet& RealSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  int                                   i, tTract, iNumTracts=RealSet.getLocationDimension(), iNumTimeIntervals=RealSet.getIntervalDimension();
  measure_t                          ** ppMeasure;
  count_t                            ** ppCases;

  ppCases = RealSet.allocateCaseData().GetArray();
  ppMeasure = RealSet.allocateMeasureData().GetArray();
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted) {
     ++ppCases[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second];
     ppMeasure[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable();
  }

  //now set as cumulative
  for (tTract=0; tTract < iNumTracts; ++tTract)
     for (i=iNumTimeIntervals-2; i >= 0; --i) {
        ppCases[i][tTract] = ppCases[i+1][tTract] + ppCases[i][tTract];
        ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
     }
}

//******************************************************************************

/** Assigns randomized data to data set's simulation measure structures. */
void RankRandomizer::AssignRandomizedData(const RealDataSet&, DataSet& SimSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  int                                   i, tTract, iNumTracts = SimSet.getLocationDimension(), iNumTimeIntervals=SimSet.getIntervalDimension();
  measure_t                          ** ppMeasure;

  //reset simulation measure arrays to zero
  SimSet.getMeasureData().Set(0);
  ppMeasure = SimSet.getMeasureData().GetArray();

  //assign randomized continuous data to measure and measure squared arrays
  for (; itr_stationary != itr_end; ++itr_permuted, ++itr_stationary)
     ppMeasure[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable();

  //now set as cumulative
  for (tTract=0; tTract < iNumTracts; ++tTract)
     for (i=iNumTimeIntervals-2; i >= 0; --i)
        ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
}

//******************************************************************************

/** Assigns randomized data to data set's simulation measure structures. */
void RankPurelyTemporalRandomizer::AssignRandomizedData(const RealDataSet&, DataSet& SimSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  int                                   i, iNumTimeIntervals=SimSet.getIntervalDimension();
  measure_t                           * pMeasure;

  //reset simulation measure arrays to zero
  SimSet.getMeasureData().Set(0);
  pMeasure = SimSet.getMeasureData_PT();

  //assign randomized continuous data to measure and measure squared arrays
  for (; itr_stationary != itr_end; ++itr_permuted, ++itr_stationary)
     pMeasure[itr_stationary->GetStationaryVariable().first] += itr_permuted->GetPermutedVariable();

  //now set as cumulative
  for (i=iNumTimeIntervals-2; i >= 0; --i)
     pMeasure[i] = pMeasure[i+1] + pMeasure[i];
}

