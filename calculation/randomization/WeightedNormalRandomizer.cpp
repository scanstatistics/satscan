//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "WeightedNormalRandomizer.h"
#include "SaTScanData.h"
#include "newmat.h"

/** constructor */
AbstractWeightedNormalRandomizer::AbstractWeightedNormalRandomizer(const CSaTScanData& dataHub, long lInitialSeed) 
 :AbstractPermutedDataRandomizer<WeightedNormalStationary_t, WeightedNormalPermuted_t>(dataHub, dataHub.GetParameters().getAdjustForWeeklyTrends(), lInitialSeed), gtFirstRatioConstant(0), gtSecondRatioConstant(0) {}

/** Adds new randomization entry with passed attrbiute values. */
void AbstractWeightedNormalRandomizer::AddCase(count_t tCount, Julian date, tract_t tTractIndex, measure_t tContinuousVariable, double dWeight) {
    // determine which collection of attributes to add these patients.
    StationaryContainerCollection_t::iterator stationaryItr;
    PermutedContainerCollection_t::iterator permutedItr, permutedOrgItr;
    if (_dayOfWeekAdjustment) {
        boost::gregorian::greg_weekday weekday = getWeekDay(date);
        stationaryItr = gvStationaryAttributeCollections.begin() + static_cast<size_t>(weekday.as_number());
        permutedItr = gvPermutedAttributeCollections.begin() + static_cast<size_t>(weekday.as_number());
        permutedOrgItr = gvOriginalPermutedAttributeCollections.begin() + static_cast<size_t>(weekday.as_number());
    } else {
        stationaryItr = gvStationaryAttributeCollections.begin();
        permutedItr = gvPermutedAttributeCollections.begin();
        permutedOrgItr = gvOriginalPermutedAttributeCollections.begin();
    }
    int iTimeInterval = _dataHub.GetTimeIntervalOfDate(date);
    for (count_t t=0; t < tCount; ++t) {
        //add stationary values
        stationaryItr->push_back(WeightedNormalStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
        //add permutated value
        permutedItr->push_back(WeightedNormalPermuted_t(WeightedNormalVariables(tContinuousVariable,dWeight)));
        //add to vector which maintains original order
        permutedOrgItr->push_back(WeightedNormalPermuted_t(WeightedNormalVariables(tContinuousVariable,dWeight)));
    }
}

/** Adds new randomization entry with passed attrbiute values and covariates. */
void AbstractWeightedNormalRandomizer::AddCase(count_t tCount, Julian date, tract_t tTractIndex, measure_t tContinuousVariable, double dWeight, const std::vector<double>& covariates) {
    // determine which collection of attributes to add these patients
    // Note that the Weighted normal model with covaraites is only implemented for purely spatial analyses, so day of week adjustment is not 
    int iTimeInterval = _dataHub.GetTimeIntervalOfDate(date);
    StationaryContainerCollection_t::iterator stationaryItr;
    PermutedContainerCollection_t::iterator permutedItr, permutedOrgItr;
    stationaryItr = gvStationaryAttributeCollections.begin();
    permutedItr = gvPermutedAttributeCollections.begin();
    permutedOrgItr = gvOriginalPermutedAttributeCollections.begin();
    StationaryContainer_t::iterator itr;
    itr = std::lower_bound(stationaryItr->begin(), stationaryItr->end(), WeightedNormalStationary_t(std::make_pair(iTimeInterval, tTractIndex)), CompareWeightedNormalStationary());
    size_t tPosReturn = std::distance(stationaryItr->begin(), itr);
    for (count_t t=0; t < tCount; ++t) {
        //add stationary values
        stationaryItr->insert(stationaryItr->begin() + tPosReturn, WeightedNormalStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
        //add permutated value
        permutedItr->insert(permutedItr->begin() + tPosReturn, WeightedNormalPermuted_t(WeightedNormalVariables(tContinuousVariable,dWeight,covariates)));
        //add to vector which maintains original order
        permutedOrgItr->insert(permutedOrgItr->begin() + tPosReturn, WeightedNormalPermuted_t(WeightedNormalVariables(tContinuousVariable,dWeight,covariates)));
    }
}

/** Allocates and sets appropriate data structures of RealDataSet object from internal
    collection of attributes. */
void AbstractWeightedNormalRandomizer::AssignFromAttributes(RealDataSet& RealSet) {
    measure_t  tTotalMeasure=0, tTotalMeasureAux=0;
    count_t    totalObservations=0;

    // sanity checks
    if (gvStationaryAttributeCollections.size() != gvOriginalPermutedAttributeCollections.size())
        throw prg_error("Size of stationary collection (%u) does not equal size of permuted collection (%u).\n", 
                        "AssignFromAttributes()", gvStationaryAttributeCollections.size(), gvOriginalPermutedAttributeCollections.size());
    for (size_t t=0; t < gvStationaryAttributeCollections.size(); ++t) {
        if (gvStationaryAttributeCollections[t].size() != gvOriginalPermutedAttributeCollections[t].size())
            throw prg_error("Number of stationary attributes (%u) does not equal number of permuted attributes (%u).\n", 
                            "AssignFromAttributes()", gvStationaryAttributeCollections[t].size(), gvOriginalPermutedAttributeCollections[t].size());
    }

    count_t ** ppCases = RealSet.allocateCaseData().GetArray();
    measure_t ** ppMeasure = RealSet.allocateMeasureData().GetArray();
    measure_t ** ppMeasureAux = RealSet.allocateMeasureData_Aux().GetArray();
    gtFirstRatioConstant = gtSecondRatioConstant = 0;
    //first calculate totals -- we need them for const variables
    PermutedContainerCollection_t::iterator itrPC=gvOriginalPermutedAttributeCollections.begin();
    for (; itrPC != gvOriginalPermutedAttributeCollections.end(); ++itrPC) {
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrP != itrPC->end(); ++itrP) {
            tTotalMeasure += itrP->GetPermutedVariable().first * itrP->GetPermutedVariable().second;
            tTotalMeasureAux += itrP->GetPermutedVariable().second;
            gtFirstRatioConstant += std::pow(itrP->GetPermutedVariable().first, 2) * itrP->GetPermutedVariable().second;
        }
    }

    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    for (itrPC=gvOriginalPermutedAttributeCollections.begin(); itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            ++ppCases[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second];
            ppMeasure[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().first * itrP->GetPermutedVariable().second;
            ppMeasureAux[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().second;
            gtSecondRatioConstant += itrP->GetPermutedVariable().second * std::pow(itrP->GetPermutedVariable().first - (tTotalMeasure/tTotalMeasureAux), 2);
        }
        totalObservations += static_cast<count_t>(itrPC->size());
    }
    gtSecondRatioConstant = 0.5 * static_cast<measure_t>(totalObservations) * log(gtSecondRatioConstant);

    RealSet.setTotalCases(totalObservations);
    RealSet.setTotalMeasure(tTotalMeasure);
    RealSet.setTotalMeasureAux(tTotalMeasureAux);
    //now set as cumulative
    int iNumTracts=RealSet.getLocationDimension(), iNumTimeIntervals=RealSet.getIntervalDimension();
    for (int tTract=0; tTract < iNumTracts; ++tTract) {
        for (int i=iNumTimeIntervals-2; i >= 0; --i) {
            ppCases[i][tTract] = ppCases[i+1][tTract] + ppCases[i][tTract];
            ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
            ppMeasureAux[i][tTract] = ppMeasureAux[i+1][tTract] + ppMeasureAux[i][tTract];
        }
    }
}

/** Sets Column vectors that are the weight and rate records as read from file. */
void AbstractWeightedNormalRandomizer::get_wg_deltag(std::auto_ptr<ColumnVector>& wg, std::auto_ptr<ColumnVector>& deltag) const {
    // weighted normal model with covarites is only implemented for purely spatial analyese, so only using first container in collection.
    wg.reset(new ColumnVector(gvPermutedAttributeCollections.begin()->size()));
    deltag.reset(new ColumnVector(gvPermutedAttributeCollections.begin()->size()));

    PermutedContainer_t::const_iterator itrP=gvPermutedAttributeCollections.begin()->begin();
    for (size_t index=0; itrP != gvPermutedAttributeCollections.begin()->end(); ++itrP, ++index) {
        wg->element(index) = itrP->GetPermutedVariable().first;
        deltag->element(index) = itrP->GetPermutedVariable().second;
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
    // weighted normal model with covarites is only implemented for purely spatial analyese, so only using first container in collection.
    size_t numFixed = (bExcludeSelectColumn ? 1 : 2);
    xg.reset(new Matrix(gvPermutedAttributeCollections.begin()->size(), numFixed + gvPermutedAttributeCollections.begin()->front().GetPermutedVariable().getAdditional()->size()));

    PermutedContainer_t::const_iterator itrP=gvPermutedAttributeCollections.begin()->begin();
    for (size_t index=0; itrP != gvPermutedAttributeCollections.begin()->end(); ++itrP, ++index) {
        if (!bExcludeSelectColumn) xg->element(index, 0) = 0;
        xg->element(index, bExcludeSelectColumn ? 0 : 1) = 1;
        const MinimalGrowthArray<double>& covariates = *(itrP->GetPermutedVariable().getAdditional());
        for (size_t tt=0; tt < covariates.size(); ++tt) {
            xg->element(index, numFixed + tt) = covariates[tt];
        }
    }
}

/** Calculates statistics about cluster from internal structures. */
AbstractWeightedNormalRandomizer::ClusterStatistics AbstractWeightedNormalRandomizer::getClusterStatistics(int iIntervalStart, int iIntervalEnd, const std::vector<tract_t>& vTracts) const {
    ClusterStatistics statistics;
    measure_t tTotalWeight=0;
    count_t totalObservations=0;

    statistics.init();
    StationaryContainerCollection_t::const_iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::const_iterator itrPC=gvOriginalPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::const_iterator itrS=itrSC->begin();
        PermutedContainer_t::const_iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            if (itrS->GetStationaryVariable().first >= iIntervalStart && itrS->GetStationaryVariable().first <= iIntervalEnd &&
                std::find(vTracts.begin(), vTracts.end(), itrS->GetStationaryVariable().second) != vTracts.end()) {
                ++statistics.gtObservations;
                statistics.gtMeanIn += itrP->GetPermutedVariable().first;
                statistics.gtWeightedMeanIn += itrP->GetPermutedVariable().second * itrP->GetPermutedVariable().first;
                statistics.gtWeight += itrP->GetPermutedVariable().second;
            } else {
                statistics.gtMeanOut += itrP->GetPermutedVariable().first;
                statistics.gtWeightedMeanOut += itrP->GetPermutedVariable().second * itrP->GetPermutedVariable().first;
            }
            tTotalWeight += itrP->GetPermutedVariable().second;
        }
        totalObservations += static_cast<count_t>(itrPC->size());
    }
    if (statistics.gtObservations) statistics.gtMeanIn /= statistics.gtObservations;
    statistics.gtMeanOut /= static_cast<measure_t>(totalObservations - statistics.gtObservations);

    if (statistics.gtWeight) statistics.gtWeightedMeanIn /= statistics.gtWeight;
    statistics.gtWeightedMeanOut /= tTotalWeight - statistics.gtWeight;

    measure_t tVarianceIn=0, tVarianceOut=0, tWeightedVarianceIn=0, tWeightedVarianceOut=0;

    itrSC=gvStationaryAttributeCollections.begin();
    itrPC=gvOriginalPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::const_iterator itrS=itrSC->begin();
        PermutedContainer_t::const_iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            if (itrS->GetStationaryVariable().first >= iIntervalStart &&
                itrS->GetStationaryVariable().first <= iIntervalEnd &&
                std::find(vTracts.begin(), vTracts.end(), itrS->GetStationaryVariable().second) != vTracts.end()) {
                tVarianceIn += std::pow(itrP->GetPermutedVariable().first - statistics.gtMeanIn, 2);
                tWeightedVarianceIn += itrP->GetPermutedVariable().second * std::pow(itrP->GetPermutedVariable().first - statistics.gtWeightedMeanIn, 2);
            } else {
                tVarianceOut += std::pow(itrP->GetPermutedVariable().first - statistics.gtMeanOut, 2);
                tWeightedVarianceOut += itrP->GetPermutedVariable().second * std::pow(itrP->GetPermutedVariable().first - statistics.gtWeightedMeanOut, 2);
            }
        }
    }
    statistics.gtVariance = (tVarianceIn + tVarianceOut)/static_cast<measure_t>(totalObservations);
    statistics.gtVariance *= (static_cast<measure_t>(totalObservations)/(static_cast<measure_t>(totalObservations - 1)));
    statistics.gtWeightedVariance = (tWeightedVarianceIn + tWeightedVarianceOut)/tTotalWeight;
    statistics.gtWeightedVariance *= (static_cast<measure_t>(totalObservations)/(static_cast<measure_t>(totalObservations - 1)));

  return statistics; 
}

/** Calculates statistics about cluster from internal structures. */
AbstractWeightedNormalRandomizer::ClusterLocationStatistics AbstractWeightedNormalRandomizer::getClusterLocationStatistics(int iIntervalStart, int iIntervalEnd, const std::vector<tract_t>& vTracts) const {
    ClusterLocationStatistics statistics;
    ClusterLocationStatistics::container_t::iterator itrObs, itrWeight;
    measure_t tTotalWeight=0;
    count_t totalObservations=0;

    statistics.init();
    StationaryContainerCollection_t::const_iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::const_iterator itrPC=gvOriginalPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::const_iterator itrS=itrSC->begin();
        PermutedContainer_t::const_iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            if (itrS->GetStationaryVariable().first >= iIntervalStart && itrS->GetStationaryVariable().first <= iIntervalEnd &&
                std::find(vTracts.begin(), vTracts.end(), itrS->GetStationaryVariable().second) != vTracts.end()) {
                ++statistics.gtObservations;
                statistics.gtMeanIn += itrP->GetPermutedVariable().first;
                statistics.gtWeightedMeanIn += itrP->GetPermutedVariable().second * itrP->GetPermutedVariable().first;
                statistics.gtWeight += itrP->GetPermutedVariable().second;

                itrObs = statistics.gtLocTotalObserved.find(itrS->GetStationaryVariable().second);
                if (itrObs == statistics.gtLocTotalObserved.end()) {
                    statistics.gtLocTotalObserved[itrS->GetStationaryVariable().second] = 1;
                    statistics.gtLocMean[itrS->GetStationaryVariable().second] = itrP->GetPermutedVariable().first;
                    statistics.gtLocTotalWeight[itrS->GetStationaryVariable().second] = itrP->GetPermutedVariable().second;
                    statistics.gtLocWeightedMean[itrS->GetStationaryVariable().second] = itrP->GetPermutedVariable().first * itrP->GetPermutedVariable().second;
                } else {
                    ++itrObs->second;
                    statistics.gtLocMean[itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().first;
                    statistics.gtLocTotalWeight[itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().second;
                    statistics.gtLocWeightedMean[itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().first * itrP->GetPermutedVariable().second;
                }
            } else {
                statistics.gtMeanOut += itrP->GetPermutedVariable().first;
                statistics.gtWeightedMeanOut += itrP->GetPermutedVariable().second * itrP->GetPermutedVariable().first;
            }
            tTotalWeight += itrP->GetPermutedVariable().second;
        }
        totalObservations += static_cast<count_t>(itrPC->size());
    }

    if (statistics.gtObservations) statistics.gtMeanIn /= statistics.gtObservations;
    statistics.gtMeanOut /= static_cast<measure_t>(totalObservations - statistics.gtObservations);

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
    DataSetStatistics statistics;
    count_t totalObservations=0;

    statistics.init();
    PermutedContainerCollection_t::const_iterator itrPC=gvOriginalPermutedAttributeCollections.begin();
    for (; itrPC != gvOriginalPermutedAttributeCollections.end(); ++itrPC) {
        PermutedContainer_t::const_iterator itrP=itrPC->begin();
        for (; itrP != itrPC->end(); ++itrP) {
            statistics.gtMean += itrP->GetPermutedVariable().first;
            statistics.gtWeightedMean += itrP->GetPermutedVariable().second * itrP->GetPermutedVariable().first;
            statistics.gtTotalWeight += itrP->GetPermutedVariable().second;
        }
        totalObservations += static_cast<count_t>(itrPC->size());
    }
    statistics.gtMean /= static_cast<measure_t>(totalObservations);
    statistics.gtWeightedMean /= statistics.gtTotalWeight;

    itrPC=gvOriginalPermutedAttributeCollections.begin();
    for (; itrPC != gvOriginalPermutedAttributeCollections.end(); ++itrPC) {
        PermutedContainer_t::const_iterator itrP=itrPC->begin();
        for (; itrP != itrPC->end(); ++itrP) {
            statistics.gtVariance += std::pow(itrP->GetPermutedVariable().first - statistics.gtMean, 2);
            statistics.gtWeightedVariance += itrP->GetPermutedVariable().second * std::pow(itrP->GetPermutedVariable().first - statistics.gtWeightedMean, 2);
        }
    }
    statistics.gtVariance /= static_cast<measure_t>(totalObservations);
    statistics.gtVariance *= (static_cast<measure_t>(totalObservations)/(static_cast<measure_t>(totalObservations - 1)));
    statistics.gtWeightedVariance /= statistics.gtTotalWeight;
    statistics.gtWeightedVariance *= (static_cast<measure_t>(totalObservations)/(static_cast<measure_t>(totalObservations - 1)));

    return statistics;
}

/** Returns whether covariates are present in randomizer entries. */
bool AbstractWeightedNormalRandomizer::getHasCovariates() const {
    // weighted normal model with covarites is only implemented for purely spatial analyese, so only using first container in collection.
    if (gvOriginalPermutedAttributeCollections.begin()->size()) {
        return gvOriginalPermutedAttributeCollections.begin()->front().GetPermutedVariable().getAdditional() != 0;
    } else return false;
}

AbstractWeightedNormalRandomizer::RiskEstimateStatistics AbstractWeightedNormalRandomizer::getRiskEstimateStatistics(const CSaTScanData& DataHub) const {
    RiskEstimateStatistics statistics;
    RiskEstimateStatistics::container_t::iterator itrObs, itrWeight;

    statistics.init(DataHub.GetNumObsGroups() + DataHub.GetNumMetaObsGroups());
    StationaryContainerCollection_t::const_iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::const_iterator itrPC=gvOriginalPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::const_iterator itrS=itrSC->begin();
        PermutedContainer_t::const_iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            statistics.gtTotalObserved[itrS->GetStationaryVariable().second] += 1;
            statistics.gtMean[itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().first;
            statistics.gtTotalWeight[itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().second;
            statistics.gtWeightedMean[itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().first * itrP->GetPermutedVariable().second;
        }
    }
    //Now calculate values for meta locations.
    std::vector<tract_t> atomic;
    for (tract_t t=0; t < DataHub.GetGroupInfo().getMetaManagerProxy().getNumMeta(); ++t) {
        DataHub.GetGroupInfo().getMetaManagerProxy().getIndexes(t, atomic);
        for (std::vector<tract_t>::const_iterator itr=atomic.begin(); itr != atomic.end(); ++itr) {
            statistics.gtTotalObserved[DataHub.GetNumObsGroups() + t] += statistics.gtTotalObserved[*itr];
            statistics.gtMean[DataHub.GetNumObsGroups() + t] += statistics.gtMean[*itr];
            statistics.gtTotalWeight[DataHub.GetNumObsGroups() + t] += statistics.gtTotalWeight[*itr];
            statistics.gtWeightedMean[DataHub.GetNumObsGroups() + t] += statistics.gtWeightedMean[*itr];
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
    WeightedNormalStationary_t tAttribute(std::make_pair(iTimeInterval, tTractIndex));

    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC) {
        StationaryContainer_t::iterator itrS;
        while ((itrS=std::find(itrSC->begin(), itrSC->end(), tAttribute)) != itrSC->end()) {
            PermutedContainerCollection_t::iterator itrPC = gvOriginalPermutedAttributeCollections.begin() + std::distance(gvStationaryAttributeCollections.begin(), itrSC);
            itrPC->erase(itrPC->begin() + std::distance(itrSC->begin(), itrS));
            itrSC->erase(itrS);
        }
    }
}

/** Returns whether every defined location has one and onle only observation. */
bool AbstractWeightedNormalRandomizer::hasUniqueLocationsCoverage(CSaTScanData& DataHub) {
    boost::dynamic_bitset<> locationsSet(DataHub.GetNumObsGroups());

    StationaryContainerCollection_t::const_iterator itrSC=gvStationaryAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC) {
        StationaryContainer_t::const_iterator itrS=itrSC->begin();
        for (; itrS != itrSC->end(); ++itrS) {
            if (locationsSet.test(itrS->GetStationaryVariable().second))
                return false;
            else
                locationsSet.set(itrS->GetStationaryVariable().second);
        }
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
    //reset simulation measure arrays to zero
    SimSet.getMeasureData().Set(0);
    SimSet.getMeasureData_Aux().Set(0);
    //assign randomized continuous data to measure and measure squared arrays
    measure_t ** ppMeasure = SimSet.getMeasureData().GetArray();
    measure_t ** ppMeasureAux = SimSet.getMeasureData_Aux().GetArray();
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            ppMeasure[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().first * itrP->GetPermutedVariable().second;
            ppMeasureAux[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().second;
        }
    }
    //now set as cumulative
    int iNumTracts=RealSet.getLocationDimension(), iNumTimeIntervals=RealSet.getIntervalDimension();
    for (int tTract=0; tTract < iNumTracts; ++tTract) {
        for (int i=iNumTimeIntervals-2; i >= 0; --i) {
            ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
            ppMeasureAux[i][tTract] = ppMeasureAux[i+1][tTract] + ppMeasureAux[i][tTract];
        }
    }
}
//******************************************************************************

/** Assigns randomized data to dataset's simulation measure structures. */
void WeightedNormalPurelyTemporalRandomizer::AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet) {
    int iNumTimeIntervals=RealSet.getIntervalDimension();

    //reset simulation measure arrays to zero
    measure_t * pMeasure = SimSet.getMeasureData_PT();
    memset(pMeasure, 0, (iNumTimeIntervals+1) * sizeof(measure_t));
    measure_t * pMeasureAux = SimSet.getMeasureData_PT_Aux();
    memset(pMeasureAux, 0, (iNumTimeIntervals+1) * sizeof(measure_t));

    //assign randomized continuous data to measure and measure squared arrays
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            pMeasure[itrS->GetStationaryVariable().first] += itrP->GetPermutedVariable().first * itrP->GetPermutedVariable().second;
            pMeasureAux[itrS->GetStationaryVariable().first] += itrP->GetPermutedVariable().second;
        }
    }
    //now set as cumulative
    for (int i=iNumTimeIntervals-2; i >= 0; --i) {
        pMeasure[i] = pMeasure[i+1] + pMeasure[i];
        pMeasureAux[i] = pMeasureAux[i+1] + pMeasureAux[i];
    }
}
