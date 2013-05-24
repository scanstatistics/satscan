//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ExponentialRandomizer.h"
#include "SSException.h"
#include "SaTScanData.h"

/** constructor */
AbstractExponentialRandomizer::AbstractExponentialRandomizer(const CSaTScanData& dataHub, long lInitialSeed) 
 :AbstractPermutedDataRandomizer<ExponentialStationary_t, ExponentialPermuted_t>(dataHub, dataHub.GetParameters().getAdjustForWeeklyTrends(), lInitialSeed) {}

/** Adds new randomization attributes with passed values. */
void AbstractExponentialRandomizer::AddPatients(count_t tNumPatients, Julian date, tract_t tTractIndex, measure_t tContinuousVariable, count_t tCensored) {
    StationaryContainerCollection_t::iterator stationaryItr;
    PermutedContainerCollection_t::iterator permutedItr;
    // determine which collection of attributes to add these patients.
    if (_dayOfWeekAdjustment) {
        boost::gregorian::greg_weekday weekday = getWeekDay(date);
        stationaryItr = gvStationaryAttributeCollections.begin() + static_cast<size_t>(weekday.as_number());
        permutedItr = gvOriginalPermutedAttributeCollections.begin() + static_cast<size_t>(weekday.as_number());
    } else {
        stationaryItr = gvStationaryAttributeCollections.begin();
        permutedItr = gvOriginalPermutedAttributeCollections.begin();
    }
    int iTimeInterval = _dataHub.GetTimeIntervalOfDate(date);
    for (count_t i=0; i < tNumPatients; ++i) {
        //add stationary values
        stationaryItr->push_back(ExponentialStationary_t(std::make_pair(iTimeInterval, tTractIndex)));
        //add permutated value
        permutedItr->push_back(ExponentialPermuted_t(std::make_pair(tContinuousVariable, static_cast<unsigned short>(tCensored))));
    }
}

/** Calibrates accumulated continuous variables and assigns data to RealDataSet objects' structures. */
void AbstractExponentialRandomizer::AssignFromAttributes(RealDataSet& RealSet) {
    count_t tTotalCases=0;
    measure_t tTotalMeasure=0, tCalibratedMeasure = 0, totalPopulation=0;

    try {
        // sanity checks
        if (gvStationaryAttributeCollections.size() != gvOriginalPermutedAttributeCollections.size())
            throw prg_error("Size of stationary collection (%u) does not equal size of permuted collection (%u).\n", 
                            "AssignFromAttributes()", gvStationaryAttributeCollections.size(), gvOriginalPermutedAttributeCollections.size());
        for (size_t t=0; t < gvStationaryAttributeCollections.size(); ++t) {
            if (gvStationaryAttributeCollections[t].size() != gvOriginalPermutedAttributeCollections[t].size())
                throw prg_error("Number of stationary attributes (%u) does not equal number of permuted attributes (%u).\n", 
                                "AssignFromAttributes()", gvStationaryAttributeCollections[t].size(), gvOriginalPermutedAttributeCollections[t].size());
        }
        // calcuate the total number of cases and expected cases
        for (PermutedContainerCollection_t::iterator itrPC=gvOriginalPermutedAttributeCollections.begin(); itrPC != gvOriginalPermutedAttributeCollections.end(); ++itrPC) {
            for (PermutedContainer_t::iterator itrPermuted=itrPC->begin(); itrPermuted != itrPC->end(); ++itrPermuted) {
                if ((*itrPermuted).GetPermutedVariable().second == 0) ++tTotalCases;
                tTotalMeasure += (*itrPermuted).GetPermutedVariable().first;
            }
        }
        //calibrate permuted continuous variable
        if (tTotalMeasure == 0)
            return; //when performing iterative scan, it is possible that total measure has become zero
        measure_t tCalibration = (measure_t)tTotalCases/tTotalMeasure;
        for (PermutedContainerCollection_t::iterator itrPC=gvOriginalPermutedAttributeCollections.begin(); itrPC != gvOriginalPermutedAttributeCollections.end(); ++itrPC) {
            for (PermutedContainer_t::iterator itrPermuted=itrPC->begin(); itrPermuted != itrPC->end(); ++itrPermuted) {
                (*itrPermuted).ReferencePermutedVariable().first *= tCalibration;
                tCalibratedMeasure += (*itrPermuted).GetPermutedVariable().first;
            }
            totalPopulation += static_cast<measure_t>(itrPC->size());
        }
        //validate that calibration worked
        if (fabs((measure_t)tTotalCases - tCalibratedMeasure) > 0.0001)
            throw prg_error("The total measure '%8.6lf' is not equal to the total number of cases '%ld'.\n", "AssignFromAttributes()", tCalibratedMeasure, tTotalCases);
        //assign totals for observed and expected in this data set
        RealSet.setTotalCases(tTotalCases);
        RealSet.setTotalMeasure(tCalibratedMeasure);
        RealSet.setTotalPopulation(totalPopulation);

        count_t ** ppCases = RealSet.allocateCaseData().GetArray();
        count_t ** ppCensoredCases = RealSet.allocateCaseData_Censored().GetArray();
        measure_t ** ppMeasure = RealSet.allocateMeasureData().GetArray();
        //assign data to measure, cases, and censored cases arrays
        StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
        PermutedContainerCollection_t::iterator itrPC=gvOriginalPermutedAttributeCollections.begin();
        for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
            StationaryContainer_t::iterator itrS=itrSC->begin();
            PermutedContainer_t::iterator itrP=itrPC->begin();
            for (; itrS != itrSC->end(); ++itrS, ++itrP) {
                ppMeasure[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += (*itrP).GetPermutedVariable().first;
                ppCases[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += ((*itrP).GetPermutedVariable().second ? 0 : 1);
                if ((*itrP).GetPermutedVariable().second)
                    ++ppCensoredCases[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second];
            }
        }
        //now set as cumulative, in respect to time intervals
        int iNumTracts = RealSet.getLocationDimension(), iNumTimeIntervals = RealSet.getIntervalDimension();
        for (int tTract=0; tTract < iNumTracts; ++tTract) {
            for (int i=iNumTimeIntervals-2; i >= 0; --i) {
                ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
                ppCases[i][tTract] = ppCases[i+1][tTract] + ppCases[i][tTract];
                ppCensoredCases[i][tTract] = ppCensoredCases[i+1][tTract] + ppCensoredCases[i][tTract];
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("AssignFromAttributes()","AbstractExponentialRandomizer");
        throw;
    }
}

/** Removes all stationary and permuted attributes associated with cases in interval and location. */
void AbstractExponentialRandomizer::RemoveCase(int iTimeInterval, tract_t tTractIndex) {
    ExponentialStationary_t tAttribute(std::make_pair(iTimeInterval, tTractIndex));
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

/** Assigns randomized data to DataSet objects' structures. */
void ExponentialRandomizer::AssignRandomizedData(const RealDataSet&, DataSet& SimSet) {
    // reset case and measure data to zero
    SimSet.getCaseData().Set(0);
    SimSet.getMeasureData().Set(0);

    //assign randomized continuous data to measure and case arrays
    count_t   ** ppCases = SimSet.getCaseData().GetArray();
    measure_t ** ppMeasure = SimSet.getMeasureData().GetArray();   
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            ppMeasure[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += (*itrP).GetPermutedVariable().first;
            ppCases[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second] += ((*itrP).GetPermutedVariable().second ? 0 : 1);
        }
    }
    //now set as cumulative, in respect to time intervals
    int iNumTracts = SimSet.getLocationDimension(), iNumTimeIntervals = SimSet.getIntervalDimension();
    for (int tTract=0; tTract < iNumTracts; ++tTract) {
        for (int i=iNumTimeIntervals-2; i >= 0; --i) {
            ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
            ppCases[i][tTract] = ppCases[i+1][tTract] + ppCases[i][tTract];
        }
    }
}

/** Calculates the total populations for each location - both censored and uncensored data.
    Caller is responsible for sizing vector to number of locations in input data. */
std::vector<double>& ExponentialRandomizer::CalculateMaxCirclePopulationArray(std::vector<double>& vMaxCirclePopulation, bool bZeroFirst) const {
    if (bZeroFirst)
        std::fill(vMaxCirclePopulation.begin(), vMaxCirclePopulation.end(), 0);

    //assign population array for accumulated data
    StationaryContainerCollection_t::const_iterator itrSC=gvStationaryAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC) {
        StationaryContainer_t::const_iterator itrS=itrSC->begin();
        for (; itrS != itrSC->end(); ++itrS) {
            ++vMaxCirclePopulation[itrS->GetStationaryVariable().second];
        }
    }
    return vMaxCirclePopulation;
}

//******************************************************************************

/** Assigns randomized data to DataSet objects' purely temporal structures. */
void ExponentialPurelyTemporalRandomizer::AssignRandomizedData(const RealDataSet&, DataSet& SimSet) {
    // reset case and measure data to zero
    int iNumTimeIntervals = SimSet.getIntervalDimension();
    count_t   * pCases = SimSet.getCaseData_PT();
    measure_t * pMeasure = SimSet.getMeasureData_PT();
    memset(pCases, 0, (iNumTimeIntervals+1) * sizeof(count_t));
    memset(pMeasure, 0, (iNumTimeIntervals+1) * sizeof(measure_t));

    //assign randomized continuous data to measure and case arrays
    StationaryContainerCollection_t::iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::iterator itrPC=gvPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::iterator itrS=itrSC->begin();
        PermutedContainer_t::iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            pMeasure[itrS->GetStationaryVariable().first] += (*itrP).GetPermutedVariable().first;
            pCases[itrS->GetStationaryVariable().first] += ((*itrP).GetPermutedVariable().second ? 0 : 1);
        }
    }
    //now set as cumulative, in respect to time intervals
    for (int i=iNumTimeIntervals-2; i >= 0; --i) {
        pMeasure[i] = pMeasure[i+1] + pMeasure[i];
        pCases[i] = pCases[i+1] + pCases[i];
    }
}
