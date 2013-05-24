//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "NormalRandomizer.h"
#include "SaTScanData.h"

/** constructor */
AbstractNormalRandomizer::AbstractNormalRandomizer(const CSaTScanData& dataHub, long lInitialSeed) 
 :AbstractPermutedDataRandomizer<NormalStationary_t, NormalPermuted_t>(dataHub, dataHub.GetParameters().getAdjustForWeeklyTrends(), lInitialSeed), _dataHub(dataHub) {}

/** Adds new randomization entry with passed attrbiute values. */
void AbstractNormalRandomizer::AddCase(count_t tCount, Julian date, tract_t tTractIndex, measure_t tContinuousVariable) {
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
    // add cases to respective attribute containers
    int iTimeInterval = _dataHub.GetTimeIntervalOfDate(date);
    for (count_t t=0; t < tCount; ++t) {
        //add stationary values
        stationaryItr->push_back(NormalStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
        //add permutated value
        permutedItr->push_back(NormalPermuted_t(tContinuousVariable));
        //add to vector which maintains original order
        permutedOrgItr->push_back(NormalPermuted_t(tContinuousVariable));
    }
}

/** Allocates and sets appropriate data structures of RealDataSet object from internal
    collection of attributes. */
void AbstractNormalRandomizer::AssignFromAttributes(RealDataSet& RealSet) {
    measure_t tTotalMeasure=0, tTotalMeasureAux=0;
    count_t   totalCases=0;

    // sanity checks
    if (gvStationaryAttributeCollections.size() != gvOriginalPermutedAttributeCollections.size())
        throw prg_error("Size of stationary collection (%u) does not equal size of permuted collection (%u).\n", 
                        "AssignFromAttributes()", gvStationaryAttributeCollections.size(), gvOriginalPermutedAttributeCollections.size());
    for (size_t t=0; t < gvStationaryAttributeCollections.size(); ++t) {
        if (gvStationaryAttributeCollections[t].size() != gvOriginalPermutedAttributeCollections[t].size())
            throw prg_error("Number of stationary attributes (%u) does not equal number of permuted attributes (%u).\n", 
                            "AssignFromAttributes()", gvStationaryAttributeCollections[t].size(), gvOriginalPermutedAttributeCollections[t].size());
    }
    //assign attributes to case and measure arrays
    count_t   ** ppCases = RealSet.allocateCaseData().GetArray();
    measure_t ** ppMeasure = RealSet.allocateMeasureData().GetArray();
    measure_t ** ppMeasureAux = RealSet.allocateMeasureData_Aux().GetArray();
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvOriginalPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            ++ppCases[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second];
            ppMeasure[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable();
            tTotalMeasure += itrP->GetPermutedVariable();
            ppMeasureAux[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += std::pow(itrP->GetPermutedVariable(), 2);
            tTotalMeasureAux += std::pow(itrP->GetPermutedVariable(), 2);
        }
        totalCases += static_cast<count_t>(itrPC->size());
    }
    RealSet.setTotalCases(totalCases);
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

/** Removes all stationary and permuted attributes associated with cases in interval and location. */
void AbstractNormalRandomizer::RemoveCase(int iTimeInterval, tract_t tTractIndex) {
    NormalStationary_t tAttribute(std::make_pair(iTimeInterval, tTractIndex));
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

//******************************************************************************

/** Assigns randomized data to dataset's simulation measure structures. */
void NormalRandomizer::AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet) {
    //reset simulation measure arrays to zero
    SimSet.getMeasureData().Set(0);
    SimSet.getMeasureData_Aux().Set(0);
    //assign randomized continuous data to measure and measure squared arrays
    measure_t  ** ppMeasure = SimSet.getMeasureData().GetArray();
    measure_t  ** ppMeasureAux = SimSet.getMeasureData_Aux().GetArray();
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            ppMeasure[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += itrP->GetPermutedVariable();
            ppMeasureAux[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += pow(itrP->GetPermutedVariable(), 2);
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
void NormalPurelyTemporalRandomizer::AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet) {
    //reset simulation measure arrays to zero
    int iNumTimeIntervals=RealSet.getIntervalDimension();
    measure_t  * pMeasure = SimSet.getMeasureData_PT();
    memset(pMeasure, 0, (iNumTimeIntervals+1) * sizeof(measure_t));
    measure_t  * pMeasureAux = SimSet.getMeasureData_PT_Aux();
    memset(pMeasureAux, 0, (iNumTimeIntervals+1) * sizeof(measure_t));

    //assign randomized continuous data to measure and measure squared arrays
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            pMeasure[itrS->GetStationaryVariable().first] += itrP->GetPermutedVariable();
            pMeasureAux[itrS->GetStationaryVariable().first] += pow(itrP->GetPermutedVariable(), 2);
        }
    }
    //now set as cumulative
    for (int i=iNumTimeIntervals-2; i >= 0; --i) {
        pMeasure[i] = pMeasure[i+1] + pMeasure[i];
        pMeasureAux[i] = pMeasureAux[i+1] + pMeasureAux[i];
    }
}
