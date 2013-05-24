//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalDataRandomizer.h"
#include "SaTScanData.h"

const size_t OrdinalDenominatorDataRandomizer::gtMaximumCategories = 5;

//********************** OrdinalDenominatorDataRandomizer ********************************************************

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

//********************** OrdinalPurelyTemporalDenominatorDataRandomizer ********************************************************

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

//********************* OrdinalDenominatorDataDayOfWeekRandomizer ****************************************************

//OrdinalDenominatorDataDayOfWeekRandomizer::OrdinalDenominatorDataDayOfWeekRandomizer(boost::gregorian::greg_weekday last_weekday, long lInitialSeed)
//                                          :AbstractOrdinalDenominatorDataRandomizer(last_weekday, lInitialSeed) {}
//
///** Creates randomized under the null hypothesis for Ordinal model, assigning data to DataSet objects structures.
//    Random number generator seed initialized based upon 'iSimulation' index. */
//void OrdinalDenominatorDataDayOfWeekRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
//    count_t                            ** ppSimCases, ** ppCases, tRemainingControls=RealSet.getTotalCases();
//    measure_t                          ** ppMeasure;
//    unsigned int                          iNumTracts=RealSet.getLocationDimension(), iNumIntervals=RealSet.getIntervalDimension(),
//                                          iNumCategories=SimSet.getCaseData_Cat().size();
//    TwoDimensionArrayHandler<measure_t>   tempMeasure(RealSet.getIntervalDimension(), RealSet.getLocationDimension(), 0);
//
//    // reset seed of random number generator
//    SetSeed(iSimulation, SimSet.getSetIndex());
//    //create temporary measure array, which is all categories combined together
//    ppMeasure = tempMeasure.GetArray();
//
//    // reset simulation data for each category
//    for (size_t c=0; c < iNumCategories - 1; ++c)
//        SimSet.getCaseData_Cat()[c]->Set(0);
//
//    RealDataSet::CategoryCountsByWeekDay_t categoryWeekDayCases = RealSet.getTotalCategoryCasesByWeekDay();
//    // First calculate the total number of cases for each day across all categories.
//    std::map<boost::gregorian::greg_weekday, count_t> casesByWeekDay;
//    for (WeekDaysContainer_t::iterator itrWeekDay=_weekdays.begin(); itrWeekDay != _weekdays.end(); ++itrWeekDay) {
//        for (RealDataSet::CategoryCountsByWeekDay_t::iterator itr=categoryWeekDayCases.begin(); itr != categoryWeekDayCases.end(); ++itr) {
//            for (size_t c=0; c < iNumCategories - 1; ++c) {
//                casesByWeekDay[itr->first] += itr->second[c];
//            }
//        }
//    }
//    // Now randomize the each week day separately -- treating remaining cases in other categories (for this week day) as controls.
//    for (WeekDaysContainer_t::iterator itrWeekDay=_weekdays.begin(); itrWeekDay != _weekdays.end(); ++itrWeekDay) {
//        // set the number of controls equal to the total cases for current day of week
//        count_t remaining_weekday_controls = casesByWeekDay[*itrWeekDay];
//        // initialize measure array to category cases
//        tempMeasure.Set(0);
//        for (size_t c=0; c < iNumCategories; ++c) {
//            ppCases = RealSet.getCaseData_Cat()[c]->GetArray();
//            for (size_t i=0; i < iNumIntervals; ++i)
//                for (size_t t=0; t < iNumTracts; ++t)
//                    ppMeasure[i][t] += ppCases[i][t];
//        }
//
//        for (size_t c=0; c < iNumCategories - 1; ++c) {
//            ppSimCases = SimSet.getCaseData_Cat()[c]->GetArray();
//            count_t category_weekdayCounts = categoryWeekDayCases[*itrWeekDay][c];
//            remaining_weekday_controls -= category_weekdayCounts;
//            tRemainingControls -= category_weekdayCounts;
//            RandomizeOrdinalDataForDayOfWeek(category_weekdayCounts,
//                                             remaining_weekday_controls,
//                                             ppSimCases,
//                                             ppMeasure,
//                                             iNumTracts,
//                                             iNumIntervals - std::distance(_weekdays.begin(), itrWeekDay),
//                                             itrWeekDay == _weekdays.end() - 1,
//                                             RealSet.getPopulationData().GetNumOrdinalCategoryCases(c),
//                                             tRemainingControls,
//                                             iNumIntervals);
//            // *** This is not right:
//            //        1) The ppSimCases is not cumulative until last day of week.
//            //        2) The process of setting the sim cases as cumulative does not seem correct.
//            // ***
//            //update measure so that assigned cases are removed from measure
//            for (unsigned int i=0; i < iNumIntervals; ++i)
//                for (unsigned int t=0; t < iNumTracts; ++t)
//                    ppMeasure[i][t] -= ppSimCases[i][t];
//        }
//        // *** This step is likely missing correct assigment for current day in week.
//        //after randomizing all categories except last, remaining cases can just be assigned respective category array
//        ppSimCases = SimSet.getCaseData_Cat().back()->GetArray();
//        for (unsigned int i=0; i < iNumIntervals; ++i)
//            for (unsigned int t=0; t < iNumTracts; ++t)
//                ppSimCases[i][t] = static_cast<count_t>(ppMeasure[i][t]);
//    }
//}

//********************** OrdinalPermutedDataRandomizer ************************************************************

OrdinalPermutedDataRandomizer::OrdinalPermutedDataRandomizer(const CSaTScanData& dataHub, long lInitialSeed)
    :AbstractPermutedDataRandomizer<OrdinalStationary_t, OrdinalPermuted_t>(dataHub, dataHub.GetParameters().getAdjustForWeeklyTrends(), lInitialSeed) {}

/** Assigns randomized data to DataSet objects's data structures. */
void OrdinalPermutedDataRandomizer::AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet) {
    //initialize counts to zero in all ordinal categories
    for (size_t c=0; c < SimSet.getCaseData_Cat().size(); ++c)
        SimSet.getCaseData_Cat()[c]->Set(0);

    //assign randomized continuous data to measure and measure squared arrays
    StationaryContainerCollection_t::const_iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::const_iterator itrPC=gvPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::const_iterator itrS=itrSC->begin();
        PermutedContainer_t::const_iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            ++SimSet.getCaseData_Cat()[itrP->GetPermutedVariable()]->GetArray()[itrS->GetStationaryVariable().first][itrS->GetStationaryVariable().second];
        }
    }
    //now set as cumulative
    int          tNumTimeIntervals = SimSet.getIntervalDimension();
    unsigned int tNumTracts = SimSet.getLocationDimension();
    for (size_t c=0; c < SimSet.getCaseData_Cat().size(); ++c) {
        count_t ** ppSimCases = SimSet.getCaseData_Cat()[c]->GetArray();
        for (unsigned int t=0; t < tNumTracts; ++t)
            for (int i=tNumTimeIntervals-2; i >= 0; --i)
                ppSimCases[i][t] = ppSimCases[i+1][t] + ppSimCases[i][t];
    }
}

/** From passed data set object, populates internal data structures which represent
    stationary and permuted attributes used during randomized data creation. */
void OrdinalPermutedDataRandomizer::setPermutedData(const RealDataSet& RealSet) {
    count_t      tCase, tCaseCount;
    unsigned int iNumTract=RealSet.getLocationDimension(), iNumIntervals=RealSet.getIntervalDimension();
    StationaryContainerCollection_t::iterator stationaryItr;
    PermutedContainerCollection_t::iterator permutedItr, permutedOrgItr;
    for (size_t c=0; c < RealSet.getCaseData_Cat().size(); ++c) {
        count_t ** ppCases = RealSet.getCaseData_Cat()[c]->GetArray();
        for (unsigned int i=0; i < iNumIntervals; ++i) {
            // determine which cotainer in collections to add entries
            if (_dayOfWeekAdjustment) {
                Julian date = _dataHub.intervalIndexToJulian(i);
                boost::gregorian::greg_weekday weekday = getWeekDay(date);
                stationaryItr = gvStationaryAttributeCollections.begin() + static_cast<size_t>(weekday.as_number());
                permutedItr = gvPermutedAttributeCollections.begin() + static_cast<size_t>(weekday.as_number());
                permutedOrgItr = gvOriginalPermutedAttributeCollections.begin() + static_cast<size_t>(weekday.as_number());
            } else {
                stationaryItr = gvStationaryAttributeCollections.begin();
                permutedItr = gvPermutedAttributeCollections.begin();
                permutedOrgItr = gvOriginalPermutedAttributeCollections.begin();
            }
            for (unsigned int t=0; t < iNumTract; ++t) {
                tCaseCount = (i == iNumIntervals - 1 ? ppCases[i][t] : ppCases[i][t] - ppCases[i+1][t]);
                for (tCase=0; tCase < tCaseCount; ++tCase) {
                    stationaryItr->push_back(OrdinalStationary_t(std::make_pair(i, t)));
                    permutedItr->push_back(PermutedAttribute<int>(c));
                    permutedOrgItr->push_back(PermutedAttribute<int>(c));
                }
            }
        }
    }
}

//********************** OrdinalPurelyTemporalPermutedDataRandomizer ********************************************************

OrdinalPurelyTemporalPermutedDataRandomizer::OrdinalPurelyTemporalPermutedDataRandomizer(const CSaTScanData& dataHub, long lInitialSeed)
 :AbstractPermutedDataRandomizer<OrdinalPurelyTemporalStationary_t, OrdinalPermuted_t>(dataHub, dataHub.GetParameters().getAdjustForWeeklyTrends(), lInitialSeed) {}

 /** Assigns randomized data to DataSet objects's data structures. */
void OrdinalPurelyTemporalPermutedDataRandomizer::AssignRandomizedData(const RealDataSet& RealSet, DataSet& SimSet) {
    //initialize counts to zero in all ordinal categories
    for (size_t c=0; c < SimSet.getCaseData_PT_Cat().Get1stDimension(); ++c)
        memset(SimSet.getCaseData_PT_Cat().GetArray()[c], 0, SimSet.getCaseData_PT_Cat().Get2ndDimension() * sizeof(count_t));

    //assign randomized continuous data to measure and measure squared arrays
    StationaryContainerCollection_t::const_iterator itrSC=gvStationaryAttributeCollections.begin();
    PermutedContainerCollection_t::const_iterator itrPC=gvPermutedAttributeCollections.begin();
    for (; itrSC != gvStationaryAttributeCollections.end(); ++itrSC, ++itrPC) {
        StationaryContainer_t::const_iterator itrS=itrSC->begin();
        PermutedContainer_t::const_iterator itrP=itrPC->begin();
        for (; itrS != itrSC->end(); ++itrS, ++itrP) {
            ++SimSet.getCaseData_PT_Cat().GetArray()[itrP->GetPermutedVariable()][itrS->GetStationaryVariable()];
        }
    }
    //now set as cumulative
    int tNumTimeIntervals = SimSet.getIntervalDimension();
    for (size_t c=0; c < SimSet.getCaseData_PT_Cat().Get1stDimension(); ++c) {
        count_t * pSimCases = SimSet.getCaseData_PT_Cat().GetArray()[c];
        for (int i=tNumTimeIntervals-2; i >= 0; --i)
            pSimCases[i] =  pSimCases[i+1] + pSimCases[i];
    }
}

/** From passed data set object, populates internal data structures which represent
    stationary and permuted attributes used during randomized data creation. */
void OrdinalPurelyTemporalPermutedDataRandomizer::setPermutedData(const RealDataSet& RealSet) {
    count_t tCase, tCaseCount;
    unsigned int  iNumIntervals=RealSet.getIntervalDimension();
    StationaryContainerCollection_t::iterator stationaryItr;
    PermutedContainerCollection_t::iterator permutedItr, permutedOrgItr;
    for (size_t c=0; c < RealSet.getCaseData_Cat().size(); ++c) {
        count_t ** ppCases = RealSet.getCaseData_Cat()[c]->GetArray();
        for (unsigned int i=0; i < RealSet.getIntervalDimension(); ++i) {
            // determine which cotainer in collections to add entries
            if (_dayOfWeekAdjustment) {
                Julian date = _dataHub.intervalIndexToJulian(i);
                boost::gregorian::greg_weekday weekday = getWeekDay(date);
                stationaryItr = gvStationaryAttributeCollections.begin() + static_cast<size_t>(weekday.as_number());
                permutedItr = gvPermutedAttributeCollections.begin() + static_cast<size_t>(weekday.as_number());
                permutedOrgItr = gvOriginalPermutedAttributeCollections.begin() + static_cast<size_t>(weekday.as_number());
            } else {
                stationaryItr = gvStationaryAttributeCollections.begin();
                permutedItr = gvPermutedAttributeCollections.begin();
                permutedOrgItr = gvOriginalPermutedAttributeCollections.begin();
            }
            for (unsigned int t=0; t < RealSet.getLocationDimension(); ++t) {
                tCaseCount = (i == iNumIntervals - 1 ? ppCases[i][t] : ppCases[i][t] - ppCases[i+1][t]);
                for (tCase=0; tCase < tCaseCount; ++tCase) {
                    stationaryItr->push_back(OrdinalPurelyTemporalStationary_t(i));
                    permutedItr->push_back(PermutedAttribute<int>(c));
                    permutedOrgItr->push_back(PermutedAttribute<int>(c));
                }
            }
        }
    }
}
