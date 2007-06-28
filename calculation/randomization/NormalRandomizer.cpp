//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "NormalRandomizer.h"

/** Adds new randomization entry with passed attrbiute values. */
void AbstractNormalRandomizer::AddCase(count_t tCount, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable) {
  for (count_t t=0; t < tCount; ++t) {
    //add stationary values
    gvStationaryAttribute.push_back(NormalStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
    //add permutated value
    gvPermutedAttribute.push_back(NormalPermuted_t(tContinuousVariable));
    //add to vector which maintains original order
    gvOriginalPermutedAttribute.push_back(NormalPermuted_t(tContinuousVariable));
  }
}

/** Allocates and sets appropriate data structures of RealDataSet object from internal
    collection of attributes. */
void AbstractNormalRandomizer::AssignFromAttributes(RealDataSet& RealSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::iterator         itr_permuted=gvOriginalPermutedAttribute.begin();
  int                                   i, tTract, iNumTracts=RealSet.getLocationDimension(), iNumTimeIntervals=RealSet.getIntervalDimension();
  measure_t                          ** ppMeasure, ** ppMeasureAux, tTotalMeasure=0, tTotalMeasureAux=0;
  count_t                            ** ppCases;

  ppCases = RealSet.allocateCaseData().GetArray();
  ppMeasure = RealSet.allocateMeasureData().GetArray();
  ppMeasureAux = RealSet.allocateMeasureData_Aux().GetArray();
  itr_permuted=gvOriginalPermutedAttribute.begin();
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted) {
     ++ppCases[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second];
     ppMeasure[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable();
     tTotalMeasure += itr_permuted->GetPermutedVariable();
     ppMeasureAux[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += std::pow(itr_permuted->GetPermutedVariable(), 2);
     tTotalMeasureAux += std::pow(itr_permuted->GetPermutedVariable(), 2);
  }
  RealSet.setTotalCases(gvOriginalPermutedAttribute.size());
  RealSet.setTotalMeasure(tTotalMeasure);
  RealSet.setTotalMeasureAux(tTotalMeasureAux);
  //now set as cumulative
  for (tTract=0; tTract < iNumTracts; ++tTract)
     for (i=iNumTimeIntervals-2; i >= 0; --i) {
        ppCases[i][tTract] = ppCases[i+1][tTract] + ppCases[i][tTract];
        ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
        ppMeasureAux[i][tTract] = ppMeasureAux[i+1][tTract] + ppMeasureAux[i][tTract];
     }
}

/** Removes all stationary and permuted attributes associated with cases in interval and location. */
void AbstractNormalRandomizer::RemoveCase(int iTimeInterval, tract_t tTractIndex) {
  NormalStationary_t                    tAttribute(std::make_pair(iTimeInterval, tTractIndex));
  StationaryContainer_t::iterator       itr;

  while ((itr=std::find(gvStationaryAttribute.begin(), gvStationaryAttribute.end(), tAttribute)) != gvStationaryAttribute.end()) {
       size_t t = std::distance(gvStationaryAttribute.begin(), itr);
       gvOriginalPermutedAttribute.erase(gvOriginalPermutedAttribute.begin() + t);
       gvStationaryAttribute.erase(itr);
  }
}

//******************************************************************************

/** Assigns randomized data to dataset's simulation measure structures. */
void NormalRandomizer::AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  measure_t                          ** ppMeasure, ** ppMeasureAux;
  int                                   i, tTract, iNumTracts=RealSet.getLocationDimension(), iNumTimeIntervals=RealSet.getIntervalDimension();

  //reset simulation measure arrays to zero
  SimSet.getMeasureData().Set(0);
  SimSet.getMeasureData_Aux().Set(0);
  ppMeasure = SimSet.getMeasureData().GetArray();
  ppMeasureAux = SimSet.getMeasureData_Aux().GetArray();
  //assign randomized continuous data to measure and measure squared arrays
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted) {
     ppMeasure[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable();
     ppMeasureAux[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += pow(itr_permuted->GetPermutedVariable(), 2);
  }
  //now set as cumulative
  for (tTract=0; tTract < iNumTracts; ++tTract)
     for (i=iNumTimeIntervals-2; i >= 0; --i) {
        ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
        ppMeasureAux[i][tTract] = ppMeasureAux[i+1][tTract] + ppMeasureAux[i][tTract];
     }
}
//******************************************************************************

/** Assigns randomized data to dataset's simulation measure structures. */
void NormalPurelyTemporalRandomizer::AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvPermutedAttribute.begin();
  measure_t                           * pMeasure, * pMeasureAux;
  int                                   i, iNumTimeIntervals=RealSet.getIntervalDimension();

  //reset simulation measure arrays to zero
  pMeasure = SimSet.getMeasureData_PT();
  memset(pMeasure, 0, (iNumTimeIntervals+1) * sizeof(measure_t));
  pMeasureAux = SimSet.getMeasureData_PT_Aux();
  memset(pMeasureAux, 0, (iNumTimeIntervals+1) * sizeof(measure_t));
  //assign randomized continuous data to measure and measure squared arrays
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted) {
     pMeasure[itr_stationary->GetStationaryVariable().first] += itr_permuted->GetPermutedVariable();
     pMeasureAux[itr_stationary->GetStationaryVariable().first] += pow(itr_permuted->GetPermutedVariable(), 2);
  }
  //now set as cumulative
  for (i=iNumTimeIntervals-2; i >= 0; --i) {
     pMeasure[i] = pMeasure[i+1] + pMeasure[i];
     pMeasureAux[i] = pMeasureAux[i+1] + pMeasureAux[i];
  }
}

