//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "WeightedNormalRandomizer.h"
#include "SaTScanData.h"
#include "newmat.h"

/** Adds new randomization entry with passed attrbiute values. */
void AbstractWeightedNormalRandomizer::AddCase(count_t tCount, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, double dWeight) {
  for (count_t t=0; t < tCount; ++t) {
    //add stationary values
    gvStationaryAttribute.push_back(WeightedNormalStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
    //add permutated value
    gvPermutedAttribute.push_back(WeightedNormalPermuted_t(WeightedNormalVariables(tContinuousVariable,dWeight)));
    //add to vector which maintains original order
    gvOriginalPermutedAttribute.push_back(WeightedNormalPermuted_t(WeightedNormalVariables(tContinuousVariable,dWeight)));
  }
}

/** Adds new randomization entry with passed attrbiute values. */
void AbstractWeightedNormalRandomizer::AddCase(count_t tCount, int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, double dWeight, const std::vector<double>& covariates) {
  StationaryContainer_t::iterator itr;
  itr = std::lower_bound(gvStationaryAttribute.begin(), gvStationaryAttribute.end(), WeightedNormalStationary_t(std::make_pair(iTimeInterval, tTractIndex)), CompareWeightedNormalStationary());
  size_t tPosReturn = std::distance(gvStationaryAttribute.begin(), itr);

  for (count_t t=0; t < tCount; ++t) {
    //add stationary values
    gvStationaryAttribute.insert(gvStationaryAttribute.begin() + tPosReturn, WeightedNormalStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
    //gvStationaryAttribute.push_back(WeightedNormalStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
    //add permutated value
    gvPermutedAttribute.insert(gvPermutedAttribute.begin() + tPosReturn, WeightedNormalPermuted_t(WeightedNormalVariables(tContinuousVariable,dWeight,covariates)));
    //gvPermutedAttribute.push_back(WeightedNormalPermuted_t(WeightedNormalVariables(tContinuousVariable,dWeight,covariates)));
    //add to vector which maintains original order
    gvOriginalPermutedAttribute.insert(gvOriginalPermutedAttribute.begin() + tPosReturn, WeightedNormalPermuted_t(WeightedNormalVariables(tContinuousVariable,dWeight,covariates)));
    //gvOriginalPermutedAttribute.push_back(WeightedNormalPermuted_t(WeightedNormalVariables(tContinuousVariable,dWeight,covariates)));
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

/** Sets Column vectors that are the weight and rate records as read from file. */
void AbstractWeightedNormalRandomizer::get_wg_deltag(std::auto_ptr<ColumnVector>& wg, std::auto_ptr<ColumnVector>& deltag) const {
  wg.reset(new ColumnVector(gvPermutedAttribute.size()));
  deltag.reset(new ColumnVector(gvPermutedAttribute.size()));
  for (size_t t=0; t < gvPermutedAttribute.size(); ++t) {
      wg->element(t) = gvPermutedAttribute[t].GetPermutedVariable().first;
      deltag->element(t) = gvPermutedAttribute[t].GetPermutedVariable().second;
  }
}

/** Sets matrix that has (# of observations rows) by (2 plus # of covariates columns) 
    -- if bExcludeSelectColumn is true; columns is (1 plus # of covariates). 
    column 1: selection column indicating observation in cluster 
    column 2: all rows set to 1
    column 3: first covariate
    column 4: second covariate
    etc. 
*/
void AbstractWeightedNormalRandomizer::get_xg(std::auto_ptr<Matrix>& xg, bool bExcludeSelectColumn) const {
  size_t numFixed = (bExcludeSelectColumn ? 1 : 2);
  xg.reset(new Matrix(gvPermutedAttribute.size(), numFixed + gvPermutedAttribute.front().GetPermutedVariable().getAdditional()->size()));
  for (size_t t=0; t < gvPermutedAttribute.size(); ++t) {
      if (!bExcludeSelectColumn) xg->element(t, 0) = 0;
      xg->element(t, bExcludeSelectColumn ? 0 : 1) = 1;
      const MinimalGrowthArray<double>& covariates = *gvPermutedAttribute[t].GetPermutedVariable().getAdditional();
      for (size_t tt=0; tt < covariates.size(); ++tt) {
          xg->element(t, numFixed + tt) = covariates[tt];
      }
  }
}

/** Calculates statistics about cluster from internal structures. */
AbstractWeightedNormalRandomizer::ClusterStatistics AbstractWeightedNormalRandomizer::getClusterStatistics(int iIntervalStart, int iIntervalEnd, const std::vector<tract_t>& vTracts) const {
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
AbstractWeightedNormalRandomizer::ClusterLocationStatistics AbstractWeightedNormalRandomizer::getClusterLocationStatistics(int iIntervalStart, int iIntervalEnd, const std::vector<tract_t>& vTracts) const {
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

/** Returns whether covariates are present in randomizer entries. */
bool AbstractWeightedNormalRandomizer::getHasCovariates() const {
    if (gvOriginalPermutedAttribute.size()) {
        return gvOriginalPermutedAttribute.front().GetPermutedVariable().getAdditional() != 0;
    } else return false;
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

/** Returns whether every defined location has one and onle only observation. */
bool AbstractWeightedNormalRandomizer::hasUniqueLocationsCoverage(CSaTScanData& DataHub) {
  boost::dynamic_bitset<> locationsSet(DataHub.GetNumTracts());

  StationaryContainer_t::const_iterator itr_stationary=gvStationaryAttribute.begin(), itr_end=gvStationaryAttribute.end();
  for (; itr_stationary != itr_end; ++itr_stationary) {
      if (locationsSet.test(itr_stationary->GetStationaryVariable().second))
         return false;
      else
         locationsSet.set(itr_stationary->GetStationaryVariable().second);
  }

/*  
  //For all 'off' bits, add location as no
  std::vector<double> v;
  for (boost::dynamic_bitset<>::size_type i=0; i < locationsSet.size(); ++i) {
      if (!locationsSet[i]) {
         // set location not evaluated
         DataHub.setLocationNotEvaluated(i);
         // Even though we are not evaluating this location, we need to add a dummy record.
         // This record is needed because we rely on relative tract index in many places and
         // to pretend to one does not exist cause major problems.
         AddCase(1,0,i,
                 gvPermutedAttribute.back().GetPermutedVariable().first, 
                 gvPermutedAttribute.back().GetPermutedVariable().second, 
                 gvPermutedAttribute.back().GetPermutedVariable().getAdditional()->get(v));
      }
  }
*/
  return locationsSet.size() == locationsSet.count();
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

