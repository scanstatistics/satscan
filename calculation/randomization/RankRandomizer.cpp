//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "RankRandomizer.h"
#include "SaTScanData.h"

//***************** AbstractRankRandomizer *************************************************************

AbstractRankRandomizer::AbstractRankRandomizer(const CSaTScanData& dataHub, long lInitialSeed) 
 : AbstractPermutedDataRandomizer<RankStationary_t, RankPermuted_t>(dataHub, dataHub.GetParameters().getAdjustForWeeklyTrends(), lInitialSeed) {}

/** Adds new randomization entry with passed attrbiute values. */
void AbstractRankRandomizer::AddCase(count_t tCount, Julian date, tract_t tTractIndex, measure_t tContinuousVariable) {
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
        permutedItr = gvOriginalPermutedAttributeCollections.begin();
        permutedOrgItr = gvOriginalPermutedAttributeCollections.begin();
    }

    int iTimeInterval = _dataHub.GetTimeIntervalOfDate(date);
    for (count_t t=0; t < tCount; ++t) {
        //add stationary values
        stationaryItr->push_back(RankStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
        //add permutated value
        permutedItr->push_back(RankPermuted_t(tContinuousVariable));
        //add to vector which maintains original order
        permutedOrgItr->push_back(RankPermuted_t(tContinuousVariable));
    }
}

/** Allocates and sets appropriate data structures of RealDataSet object from internal
    collection of attributes. */
void AbstractRankRandomizer::AssignFromAttributes(RealDataSet& RealSet) {
    int       i, tTract, iNumTracts=RealSet.getLocationDimension(), iNumTimeIntervals=RealSet.getIntervalDimension();
    measure_t ** ppMeasure;
    count_t   ** ppCases;

    // sanity checks
    if (gvStationaryAttributeCollections.size() != gvOriginalPermutedAttributeCollections.size())
        throw prg_error("Size of stationary collection (%u) does not equal size of permuted collection (%u).\n", 
                        "AssignFromAttributes()", gvStationaryAttributeCollections.size(), gvOriginalPermutedAttributeCollections.size());
    for (size_t t=0; t < gvStationaryAttributeCollections.size(); ++t) {
        if (gvStationaryAttributeCollections[t].size() != gvOriginalPermutedAttributeCollections[t].size())
            throw prg_error("Number of stationary attributes (%u) does not equal number of permuted attributes (%u).\n", 
                            "AssignFromAttributes()", gvStationaryAttributeCollections[t].size(), gvOriginalPermutedAttributeCollections[t].size());
    }

    ppCases = RealSet.allocateCaseData().GetArray();
    ppMeasure = RealSet.allocateMeasureData().GetArray();
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvOriginalPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            ++ppCases[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second];
            ppMeasure[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable();
        }
    }
    //now set as cumulative
    for (tTract=0; tTract < iNumTracts; ++tTract) {
        for (i=iNumTimeIntervals-2; i >= 0; --i) {
            ppCases[i][tTract] = ppCases[i+1][tTract] + ppCases[i][tTract];
            ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
        }
    }
}

//***************** RankRandomizer *************************************************************

/** Assigns randomized data to data set's simulation measure structures. */
void RankRandomizer::AssignRandomizedData(const RealDataSet&, DataSet& SimSet) {
    //reset simulation measure arrays to zero
    SimSet.getMeasureData().Set(0);
    //assign randomized continuous data to measure
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvPermutedAttributeCollections.begin();
    measure_t ** ppMeasure = SimSet.getMeasureData().GetArray();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            ppMeasure[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable();
        }
    }
    //now set as cumulative
    int tTract, iNumTracts = SimSet.getLocationDimension(), iNumTimeIntervals=SimSet.getIntervalDimension();
    for (tTract=0; tTract < iNumTracts; ++tTract) {
        for (int i=iNumTimeIntervals-2; i >= 0; --i)
            ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
    }
}

//**************** RankPurelyTemporalRandomizer **************************************************************

/** Assigns randomized data to data set's simulation measure structures. */
void RankPurelyTemporalRandomizer::AssignRandomizedData(const RealDataSet&, DataSet& SimSet) {
    //reset simulation measure arrays to zero
    SimSet.getMeasureData().Set(0);
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvPermutedAttributeCollections.begin();
    measure_t * pMeasure = SimSet.getMeasureData_PT();
    //assign randomized continuous data to measure
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            pMeasure[itrS->GetStationaryVariable().first] += itrP->GetPermutedVariable();
        }
    }
    //now set as cumulative
    int iNumTimeIntervals=SimSet.getIntervalDimension();
    for (int i=iNumTimeIntervals-2; i >= 0; --i)
        pMeasure[i] = pMeasure[i+1] + pMeasure[i];
}
