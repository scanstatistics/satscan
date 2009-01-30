//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "WeightedNormalRandomizer.h"
#include "SaTScanData.h"

/** Adds new randomization entry with passed attrbiute values. */
void AbstractWeightedNormalRandomizer::AddCase(count_t tCount, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, double dWeight) {
  for (count_t t=0; t < tCount; ++t) {
    //add stationary values
    gvStationaryAttribute.push_back(WeightedNormalStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
    //add permutated value
    gvPermutedAttribute.push_back(WeightedNormalPermuted_t(std::make_pair(tContinuousVariable,dWeight)));
    //add to vector which maintains original order
    gvOriginalPermutedAttribute.push_back(WeightedNormalPermuted_t(std::make_pair(tContinuousVariable,dWeight)));
  }
}

/** Allocates and sets appropriate data structures of RealDataSet object from internal
    collection of attributes. */
void AbstractWeightedNormalRandomizer::AssignFromAttributes(RealDataSet& RealSet) {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::iterator         itr_permuted=gvOriginalPermutedAttribute.begin();
  int                                   i, tTract, iNumTracts=RealSet.getLocationDimension(), iNumTimeIntervals=RealSet.getIntervalDimension();
  measure_t                          ** ppMeasure, ** ppMeasureAux, tTotalMeasure=0, tTotalMeasureAux=0;
  count_t                            ** ppCases;

  ppCases = RealSet.allocateCaseData().GetArray();
  ppMeasure = RealSet.allocateMeasureData().GetArray();
  ppMeasureAux = RealSet.allocateMeasureData_Aux().GetArray();
  itr_permuted=gvOriginalPermutedAttribute.begin();

  gtFirstRatioConstant = gtSecondRatioConstant = 0;
  //first calculate totals -- we need them for const variables
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted) {
     tTotalMeasure += itr_permuted->GetPermutedVariable().first * itr_permuted->GetPermutedVariable().second;
     tTotalMeasureAux += itr_permuted->GetPermutedVariable().second;
     gtFirstRatioConstant += std::pow(itr_permuted->GetPermutedVariable().first, 2) * itr_permuted->GetPermutedVariable().second;
  }

  itr_permuted=gvOriginalPermutedAttribute.begin();
  itr_stationary=gvStationaryAttribute.begin();
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted) {
     ++ppCases[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second];
     ppMeasure[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable().first * itr_permuted->GetPermutedVariable().second;
     ppMeasureAux[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable().second;
     gtSecondRatioConstant += itr_permuted->GetPermutedVariable().second * std::pow(itr_permuted->GetPermutedVariable().first - (tTotalMeasure/tTotalMeasureAux), 2);
  }
  gtSecondRatioConstant = 0.5 * gvOriginalPermutedAttribute.size() * log(gtSecondRatioConstant);

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

/** Calculates statistics about cluster from internal structures. */
AbstractWeightedNormalRandomizer::ClusterStatistics AbstractWeightedNormalRandomizer::getClusterStatistics(int iIntervalStart, int iIntervalEnd, std::vector<tract_t>& vTracts) const {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvOriginalPermutedAttribute.begin();
  ClusterStatistics statistics;
  measure_t tTotalWeight=0;

  statistics.init();  
  for (;itr_stationary != itr_end; ++itr_stationary, ++itr_permuted) {
     if (itr_stationary->GetStationaryVariable().first >= iIntervalStart &&
         itr_stationary->GetStationaryVariable().first <= iIntervalEnd &&
         std::find(vTracts.begin(), vTracts.end(), itr_stationary->GetStationaryVariable().second) != vTracts.end()) {
       ++statistics.gtObservations;
       statistics.gtMeanIn += itr_permuted->GetPermutedVariable().first;
       statistics.gtWeightedMeanIn += itr_permuted->GetPermutedVariable().second * itr_permuted->GetPermutedVariable().first;
       statistics.gtWeight += itr_permuted->GetPermutedVariable().second;
     } else {
       statistics.gtMeanOut += itr_permuted->GetPermutedVariable().first;
       statistics.gtWeightedMeanOut += itr_permuted->GetPermutedVariable().second * itr_permuted->GetPermutedVariable().first;
     }
     tTotalWeight += itr_permuted->GetPermutedVariable().second;
  }
  if (statistics.gtObservations) statistics.gtMeanIn /= statistics.gtObservations;
  statistics.gtMeanOut /= static_cast<measure_t>(gvOriginalPermutedAttribute.size() - statistics.gtObservations);

  if (statistics.gtWeight) statistics.gtWeightedMeanIn /= statistics.gtWeight;
  statistics.gtWeightedMeanOut /= tTotalWeight - statistics.gtWeight;

  measure_t tVarianceIn=0, tVarianceOut=0, tWeightedVarianceIn=0, tWeightedVarianceOut=0;
  itr_stationary=gvStationaryAttribute.begin();
  itr_permuted=gvOriginalPermutedAttribute.begin();
  for (;itr_stationary != itr_end; ++itr_stationary, ++itr_permuted) {
     if (itr_stationary->GetStationaryVariable().first >= iIntervalStart &&
         itr_stationary->GetStationaryVariable().first <= iIntervalEnd &&
         std::find(vTracts.begin(), vTracts.end(), itr_stationary->GetStationaryVariable().second) != vTracts.end()) {
         tVarianceIn += std::pow(itr_permuted->GetPermutedVariable().first - statistics.gtMeanIn, 2);
         tWeightedVarianceIn += itr_permuted->GetPermutedVariable().second * std::pow(itr_permuted->GetPermutedVariable().first - statistics.gtWeightedMeanIn, 2);
     } else {
         tVarianceOut += std::pow(itr_permuted->GetPermutedVariable().first - statistics.gtMeanOut, 2);
         tWeightedVarianceOut += itr_permuted->GetPermutedVariable().second * std::pow(itr_permuted->GetPermutedVariable().first - statistics.gtWeightedMeanOut, 2);
     }
  }

  statistics.gtVariance = (tVarianceIn + tVarianceOut)/static_cast<measure_t>(gvOriginalPermutedAttribute.size());
  statistics.gtVariance *= (static_cast<measure_t>(gvOriginalPermutedAttribute.size())/(static_cast<measure_t>(gvOriginalPermutedAttribute.size() - 1)));
  statistics.gtWeightedVariance = (tWeightedVarianceIn + tWeightedVarianceOut)/tTotalWeight;
  statistics.gtWeightedVariance *= (static_cast<measure_t>(gvOriginalPermutedAttribute.size())/(static_cast<measure_t>(gvOriginalPermutedAttribute.size() - 1)));

  return statistics; 
}

/** Calculates statistics about cluster from internal structures. */
AbstractWeightedNormalRandomizer::ClusterLocationStatistics AbstractWeightedNormalRandomizer::getClusterLocationStatistics(int iIntervalStart, int iIntervalEnd, std::vector<tract_t>& vTracts) const {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator   itr_permuted=gvOriginalPermutedAttribute.begin();
  ClusterLocationStatistics statistics;
  ClusterLocationStatistics::container_t::iterator itrObs, itrWeight;
  measure_t tTotalWeight=0;

  statistics.init();  
  for (;itr_stationary != itr_end; ++itr_stationary, ++itr_permuted) {
     if (itr_stationary->GetStationaryVariable().first >= iIntervalStart &&
         itr_stationary->GetStationaryVariable().first <= iIntervalEnd &&
         std::find(vTracts.begin(), vTracts.end(), itr_stationary->GetStationaryVariable().second) != vTracts.end()) {
        ++statistics.gtObservations;
        statistics.gtMeanIn += itr_permuted->GetPermutedVariable().first;
        statistics.gtWeightedMeanIn += itr_permuted->GetPermutedVariable().second * itr_permuted->GetPermutedVariable().first;
        statistics.gtWeight += itr_permuted->GetPermutedVariable().second;

        itrObs = statistics.gtLocTotalObserved.find(itr_stationary->GetStationaryVariable().second);
        if (itrObs == statistics.gtLocTotalObserved.end()) {
           statistics.gtLocTotalObserved[itr_stationary->GetStationaryVariable().second] = 1;
           statistics.gtLocMean[itr_stationary->GetStationaryVariable().second] = itr_permuted->GetPermutedVariable().first;
           statistics.gtLocTotalWeight[itr_stationary->GetStationaryVariable().second] = itr_permuted->GetPermutedVariable().second;
           statistics.gtLocWeightedMean[itr_stationary->GetStationaryVariable().second] = itr_permuted->GetPermutedVariable().first * itr_permuted->GetPermutedVariable().second;
        }
        else {
           ++itrObs->second;
           statistics.gtLocMean[itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable().first;
           statistics.gtLocTotalWeight[itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable().second;
           statistics.gtLocWeightedMean[itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable().first * itr_permuted->GetPermutedVariable().second;
        }
     } else {
       statistics.gtMeanOut += itr_permuted->GetPermutedVariable().first;
       statistics.gtWeightedMeanOut += itr_permuted->GetPermutedVariable().second * itr_permuted->GetPermutedVariable().first;
     }
     tTotalWeight += itr_permuted->GetPermutedVariable().second;
  }
  if (statistics.gtObservations) statistics.gtMeanIn /= statistics.gtObservations;
  statistics.gtMeanOut /= static_cast<measure_t>(gvOriginalPermutedAttribute.size() - statistics.gtObservations);

  if (statistics.gtWeight) statistics.gtWeightedMeanIn /= statistics.gtWeight;
  statistics.gtWeightedMeanOut /= tTotalWeight - statistics.gtWeight;

  itrObs = statistics.gtLocTotalObserved.begin();
  itrWeight = statistics.gtLocTotalWeight.begin();
  for (; itrObs != statistics.gtLocTotalObserved.end(); ++itrObs, ++itrWeight) {
      statistics.gtLocMean[itrObs->first] /= itrObs->second;
      statistics.gtLocWeightedMean[itrWeight->first] /= itrWeight->second;
  }

  return statistics; 
}

/** Calculates statistics about entire data set from internal structures. */
AbstractWeightedNormalRandomizer::DataSetStatistics AbstractWeightedNormalRandomizer::getDataSetStatistics() const {
  PermutedContainer_t::const_iterator itr_permuted=gvOriginalPermutedAttribute.begin(), itr_end=gvOriginalPermutedAttribute.end();
  DataSetStatistics statistics;

  statistics.init();
  for (;itr_permuted != itr_end; ++itr_permuted) {
      statistics.gtMean += itr_permuted->GetPermutedVariable().first;
      statistics.gtWeightedMean += itr_permuted->GetPermutedVariable().second * itr_permuted->GetPermutedVariable().first;
      statistics.gtTotalWeight += itr_permuted->GetPermutedVariable().second;
  }
  statistics.gtMean /= static_cast<measure_t>(gvOriginalPermutedAttribute.size());
  statistics.gtWeightedMean /= statistics.gtTotalWeight;

  itr_permuted=gvOriginalPermutedAttribute.begin();
  for (;itr_permuted != itr_end; ++itr_permuted) {
      statistics.gtVariance += std::pow(itr_permuted->GetPermutedVariable().first - statistics.gtMean, 2);
      statistics.gtWeightedVariance += itr_permuted->GetPermutedVariable().second * std::pow(itr_permuted->GetPermutedVariable().first - statistics.gtWeightedMean, 2);
  }
  statistics.gtVariance /= static_cast<measure_t>(gvOriginalPermutedAttribute.size());
  statistics.gtVariance *= (static_cast<measure_t>(gvOriginalPermutedAttribute.size())/(static_cast<measure_t>(gvOriginalPermutedAttribute.size() - 1)));
  statistics.gtWeightedVariance /= statistics.gtTotalWeight;
  statistics.gtWeightedVariance *= (static_cast<measure_t>(gvOriginalPermutedAttribute.size())/(static_cast<measure_t>(gvOriginalPermutedAttribute.size() - 1)));

  return statistics;
}

AbstractWeightedNormalRandomizer::RiskEstimateStatistics AbstractWeightedNormalRandomizer::getRiskEstimateStatistics(const CSaTScanData& DataHub) const {
  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  PermutedContainer_t::const_iterator itr_permuted=gvOriginalPermutedAttribute.begin();
  RiskEstimateStatistics statistics;
  RiskEstimateStatistics::container_t::iterator itrObs, itrWeight;

  statistics.init(DataHub.GetNumTracts() + DataHub.GetNumMetaTracts());
  for (; itr_stationary != itr_end; ++itr_stationary, ++itr_permuted) {
      statistics.gtTotalObserved[itr_stationary->GetStationaryVariable().second] += 1;
      statistics.gtMean[itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable().first;
      statistics.gtTotalWeight[itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable().second;
      statistics.gtWeightedMean[itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable().first * itr_permuted->GetPermutedVariable().second;
  }
  //Now calculate values for meta locations.
  std::vector<tract_t> atomic;
  for (tract_t t=0; t < DataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations(); ++t) {
      DataHub.GetTInfo()->getMetaManagerProxy().getIndexes(t, atomic);
      for (std::vector<tract_t>::const_iterator itr=atomic.begin(); itr != atomic.end(); ++itr) {
          statistics.gtTotalObserved[DataHub.GetNumTracts() + t] += statistics.gtTotalObserved[*itr];
          statistics.gtMean[DataHub.GetNumTracts() + t] += statistics.gtMean[*itr];
          statistics.gtTotalWeight[DataHub.GetNumTracts() + t] += statistics.gtTotalWeight[*itr];
          statistics.gtWeightedMean[DataHub.GetNumTracts() + t] += statistics.gtWeightedMean[*itr];
      }
  }

  itrObs = statistics.gtTotalObserved.begin();
  itrWeight = statistics.gtTotalWeight.begin();
  for (; itrObs != statistics.gtTotalObserved.end(); ++itrObs, ++itrWeight) {
      if (itrObs->second) statistics.gtMean[itrObs->first] /= itrObs->second;
      if (itrWeight->second) statistics.gtWeightedMean[itrWeight->first] /= itrWeight->second;
  }

  return statistics;
}

/** Removes all stationary and permuted attributes associated with cases in interval and location. */
void AbstractWeightedNormalRandomizer::RemoveCase(int iTimeInterval, tract_t tTractIndex) {
  WeightedNormalStationary_t            tAttribute(std::make_pair(iTimeInterval, tTractIndex));
  StationaryContainer_t::iterator       itr;

  while ((itr=std::find(gvStationaryAttribute.begin(), gvStationaryAttribute.end(), tAttribute)) != gvStationaryAttribute.end()) {
       size_t t = std::distance(gvStationaryAttribute.begin(), itr);
       gvOriginalPermutedAttribute.erase(gvOriginalPermutedAttribute.begin() + t);
       gvStationaryAttribute.erase(itr);
  }
}

//******************************************************************************

/** Assigns randomized data to dataset's simulation measure structures. */
void WeightedNormalRandomizer::AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet) {
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
     ppMeasure[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable().first * itr_permuted->GetPermutedVariable().second;
     ppMeasureAux[itr_stationary->GetStationaryVariable().first][itr_stationary->GetStationaryVariable().second] += itr_permuted->GetPermutedVariable().second;
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
void WeightedNormalPurelyTemporalRandomizer::AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet) {
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
     pMeasure[itr_stationary->GetStationaryVariable().first] += itr_permuted->GetPermutedVariable().first * itr_permuted->GetPermutedVariable().second;
     pMeasureAux[itr_stationary->GetStationaryVariable().first] += itr_permuted->GetPermutedVariable().second;
  }
  //now set as cumulative
  for (i=iNumTimeIntervals-2; i >= 0; --i) {
     pMeasure[i] = pMeasure[i+1] + pMeasure[i];
     pMeasureAux[i] = pMeasureAux[i+1] + pMeasureAux[i];
  }
}

