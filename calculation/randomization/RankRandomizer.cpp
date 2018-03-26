//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "RankRandomizer.h"
#include "SaTScanData.h"
#include <numeric> 

//***************** AbstractRankRandomizer *************************************************************

AbstractRankRandomizer::AbstractRankRandomizer(const CSaTScanData& dataHub, long lInitialSeed) 
 : AbstractPermutedDataRandomizer<RankStationary_t, RankPermuted_t>(dataHub, dataHub.GetParameters().getAdjustForWeeklyTrends(), lInitialSeed){}

/** Adds new randomization entry with passed attrbiute values. */
void AbstractRankRandomizer::AddCase(Julian date, tract_t tract, measure_t variable, double rank) {
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
    //add stationary values
    stationaryItr->push_back(RankStationary_t(std::make_pair(_dataHub.GetTimeIntervalOfDate(date), tract)));
    //add permutated value
    permutedItr->push_back(RankPermuted_t(std::make_pair(variable, rank)));
    //add to vector which maintains original order
    permutedOrgItr->push_back(RankPermuted_t(std::make_pair(variable, rank)));
}

/** Allocates and sets appropriate data structures of RealDataSet object from internal
    collection of attributes. */
void AbstractRankRandomizer::AssignFromAttributes(RankRecordCollection_t& records, RealDataSet& RealSet) {
    // First we need to assign rank to the record in collection, so we first sort the collection by the attribute descending.
    std::sort(records.begin(), records.end(), std::greater<RankRecord>());
    // Now apply a ranking to each element. We'll need to deal with ties in specific manner - ties receive a rank equal to the average of the ranks they span.
    unsigned int rank(records.size());
    std::vector<double> ranks;
    RankRecordCollection_t::iterator left=records.begin(), right=left + 1;
    ranks.push_back(static_cast<double>(rank));
    while (1) {
        if (right == records.end() || left->_attribute != right->_attribute) {
            double average_rank = std::accumulate(ranks.begin(), ranks.end(), 0.0) / static_cast<double>(ranks.size());
            while (left != right) {
                left->_rank = average_rank;
                ++left;
            }
            if (right == records.end()) break;
            ranks.clear();
            ranks.push_back(--rank);
            ++right;
        } else {
            ranks.push_back(--rank);
            ++right;
        }
    }
    // Add records to internal data structures
    double ranksum = 0.0, num_records(static_cast<double>(records.size())), ranksum_expected(num_records * (num_records + 1.0) / 2.0);
    for (left = records.begin(); left != records.end(); ++left) {
        ranksum += left->_rank;
        AddCase(left->_date, left->_tract, left->_attribute, left->_rank);
    }
    // Check - sum of the ranks should equal n(n + 1) / 2, n = records.size()
    if (ranksum != ranksum_expected)
        throw prg_error("Sum of ranks %g does not equal expected value %u\n", "AssignFromAttributes()", ranksum, ranksum_expected);
    // Set data set totals.
    RealSet.setTotalCases(records.size());
    RealSet.setTotalMeasure(ranksum);

    // sanity checks
    if (gvStationaryAttributeCollections.size() != gvOriginalPermutedAttributeCollections.size())
        throw prg_error("Size of stationary collection (%u) does not equal size of permuted collection (%u).\n", 
                        "AssignFromAttributes()", gvStationaryAttributeCollections.size(), gvOriginalPermutedAttributeCollections.size());
    for (size_t t=0; t < gvStationaryAttributeCollections.size(); ++t) {
        if (gvStationaryAttributeCollections[t].size() != gvOriginalPermutedAttributeCollections[t].size())
            throw prg_error("Number of stationary attributes (%u) does not equal number of permuted attributes (%u).\n", 
                            "AssignFromAttributes()", gvStationaryAttributeCollections[t].size(), gvOriginalPermutedAttributeCollections[t].size());
    }

    // Now assign values to data structures of RealSet object.
    count_t ** ppCases = RealSet.allocateCaseData().GetArray();
    measure_t ** ppMeasure = RealSet.allocateMeasureData().GetArray();
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvOriginalPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            ++ppCases[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second];
            ppMeasure[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().second;
        }
    }
    //now set as cumulative
    int iNumTracts = RealSet.getLocationDimension(), iNumTimeIntervals = RealSet.getIntervalDimension();
    for (int tTract=0; tTract < iNumTracts; ++tTract) {
        for (int i=iNumTimeIntervals-2; i >= 0; --i) {
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
            ppMeasure[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable().second;
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
            pMeasure[itrS->GetStationaryVariable().first] += itrP->GetPermutedVariable().second;
        }
    }
    //now set as cumulative
    int iNumTimeIntervals=SimSet.getIntervalDimension();
    for (int i=iNumTimeIntervals-2; i >= 0; --i)
        pMeasure[i] = pMeasure[i+1] + pMeasure[i];
}
