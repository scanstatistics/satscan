//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliRandomizer.h"
#include "SaTScanData.h"
#include "boost/date_time/gregorian/gregorian.hpp"

//******************** BernoulliNullHypothesisRandomizer **************************************************************

/** Creates randomized under the null hypothesis for Bernoulli model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void BernoulliNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  //reset seed of random number generator
  SetSeed(iSimulation, SimSet.getSetIndex());
  SimSet.getCaseData().Set(0);
  //randomize data
  RandomizeOrdinalData(RealSet.getTotalCases(), RealSet.getTotalControls(), SimSet.getCaseData().GetArray(),
                       RealSet.getMeasureData().GetArray(), RealSet.getLocationDimension(), RealSet.getIntervalDimension());
}

//******************** BernoulliNullHypothesisTimeStratifiedRandomizer **************************************************************

/** Creates randomized data under the null hypothesis for Bernoulli model while adjusting for temporal trends nonparametrically. 
    Assignsdata to DataSet objects structures. Random number generator seed initialized based upon 'iSimulation' index. */
void BernoulliNullHypothesisTimeStratifiedRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
    //reset seed of random number generator
    SetSeed(iSimulation, SimSet.getSetIndex());
    SimSet.getCaseData().Set(0);
    //randomize data
    RandomizeOrdinalDataTimeStratified(RealSet, SimSet);
}

//******************** BernoulliNullHypothesisSpatialStratifiedRandomizer **************************************************************

/** Creates randomized data under the null hypothesis for Bernoulli model while adjusting for purely spatial clusters with stratified randomization.
Assignsdata to DataSet objects structures. Random number generator seed initialized based upon 'iSimulation' index. */
void BernoulliNullHypothesisSpatialStratifiedRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
    //reset seed of random number generator
    SetSeed(iSimulation, SimSet.getSetIndex());
    SimSet.getCaseData().Set(0);
    //randomize data
    RandomizeOrdinalDataSpatialStratified(RealSet, SimSet);
}

//******************** BernoulliPurelyTemporalNullHypothesisRandomizer ************************************************

/** Creates randomized under the null hypothesis for Bernoulli model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void BernoulliPurelyTemporalNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  //reset seed of random number generator
  SetSeed(iSimulation, SimSet.getSetIndex());
  memset(SimSet.getCaseData_PT(), 0, (RealSet.getIntervalDimension()+1) * sizeof(count_t));
  //randomize data
  RandomizePurelyTemporalOrdinalData(RealSet.getTotalCases(), RealSet.getTotalControls(), SimSet.getCaseData_PT(), RealSet.getMeasureData_PT(), RealSet.getIntervalDimension());
}

//********************* AbstractBernoulliNullHypothesisDayOfWeekRandomizer ********************************************
/*
AbstractBernoulliNullHypothesisDayOfWeekRandomizer::AbstractBernoulliNullHypothesisDayOfWeekRandomizer(boost::gregorian::greg_weekday last_weekday, long lInitialSeed)
                                                   :AbstractOrdinalDenominatorDataRandomizer(lInitialSeed) {
   // week day for last index of all time intervals
   _weekdays.push_back(last_weekday);
   // add previous days of week
   boost::gregorian::greg_weekday week_day = last_weekday;
   for(;;) {
       if (week_day == boost::date_time::Sunday) break;
       week_day = boost::gregorian::greg_weekday(week_day.as_number() - 1);
       _weekdays.push_back(week_day);
   }
   // add days of week before Monday
   week_day = boost::date_time::Saturday;
   for(;;) {
       if (week_day == last_weekday) break;
       _weekdays.push_back(week_day);
       week_day = boost::gregorian::greg_weekday(week_day.as_number() - 1);
   }
}
*/
//********************* BernoulliNullHypothesisDayOfWeekRandomizer ****************************************************

BernoulliNullHypothesisDayOfWeekRandomizer::BernoulliNullHypothesisDayOfWeekRandomizer(boost::gregorian::greg_weekday last_weekday, long lInitialSeed)
                                           :AbstractOrdinalDenominatorDataRandomizer(last_weekday, lInitialSeed) {}

/** Creates randomized under the null hypothesis for Bernoulli model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void BernoulliNullHypothesisDayOfWeekRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  //reset seed of random number generato  
  SetSeed(iSimulation, SimSet.getSetIndex());
  SimSet.getCaseData().Set(0);

  for (WeekDaysContainer_t::iterator itrWeekDay=_weekdays.begin(); itrWeekDay != _weekdays.end(); ++itrWeekDay) {
      RandomizeOrdinalDataForDayOfWeek(RealSet.getTotalCasesByWeekDay()[*itrWeekDay],
                                       RealSet.getTotalControlByWeekDay()[*itrWeekDay],
                                       SimSet.getCaseData().GetArray(),
                                       RealSet.getMeasureData().GetArray(),
                                       RealSet.getLocationDimension(),
                                       RealSet.getIntervalDimension() - std::distance(_weekdays.begin(), itrWeekDay),
                                       itrWeekDay == _weekdays.end() - 1,
                                       RealSet.getTotalCases(),
                                       RealSet.getTotalControls(),
                                       RealSet.getIntervalDimension());
  }
}

//********************* BernoulliPurelyTemporalNullHypothesisDayOfWeekRandomizer **************************************
BernoulliPurelyTemporalNullHypothesisDayOfWeekRandomizer::BernoulliPurelyTemporalNullHypothesisDayOfWeekRandomizer(boost::gregorian::greg_weekday last_weekday, long lInitialSeed)
                                                         :AbstractOrdinalDenominatorDataRandomizer(last_weekday, lInitialSeed) {}

/** Creates randomized under the null hypothesis for Bernoulli model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void BernoulliPurelyTemporalNullHypothesisDayOfWeekRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  //reset seed of random number generator
  SetSeed(iSimulation, SimSet.getSetIndex());
  memset(SimSet.getCaseData_PT(), 0, (RealSet.getIntervalDimension()+1) * sizeof(count_t));

  //boost::date_time::weekdays week_day = boost::date_time::Sunday;
  for (WeekDaysContainer_t::iterator itrWeekDay=_weekdays.begin(); itrWeekDay != _weekdays.end(); ++itrWeekDay) {
      RandomizePurelyTemporalOrdinalDataForDayOfWeek(RealSet.getTotalCasesByWeekDay()[*itrWeekDay],
                                                     RealSet.getTotalControlByWeekDay()[*itrWeekDay],
                                                     SimSet.getCaseData_PT(), 
                                                     RealSet.getMeasureData_PT(),
                                                     RealSet.getLocationDimension(),
                                                     RealSet.getIntervalDimension() - std::distance(_weekdays.begin(), itrWeekDay),
                                                     itrWeekDay == _weekdays.end() - 1,
                                                     RealSet.getTotalCases(),
                                                     RealSet.getTotalControls(),
                                                     RealSet.getIntervalDimension());
  }
}
