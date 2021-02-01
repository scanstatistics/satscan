//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DenominatorDataRandomizer.h"

//********************* AbstractOrdinalDenominatorDataRandomizer ********************************************

AbstractOrdinalDenominatorDataRandomizer::AbstractOrdinalDenominatorDataRandomizer(boost::gregorian::greg_weekday last_weekday, long lInitialSeed)
                                         :AbstractDenominatorDataRandomizer(lInitialSeed) {
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

/** Each of the totalMeasure number of individuals (sum of cases and controls),
    are randomized to either be a case or a control. The output is an array with
    the indices of the TotalCounts number of cases. For example, if there are
    20 cases and 80 controls, the output is an array the the indices between
    0 and 99 that correspond to the randomized cases. (MK Oct 27, 2003)         */
void AbstractOrdinalDenominatorDataRandomizer::MakeDataB(count_t tTotalCounts, measure_t tTotalMeasure, std::vector<count_t>& RandCounts) {
  int           i;
  count_t       nCumCounts=0;
  double        x, ratio;

  RandCounts.resize(tTotalCounts);
  for (i=0; i < tTotalMeasure; ++i) {
     x = gRandomNumberGenerator.GetRandomDouble();
     ratio = (double) (tTotalCounts - nCumCounts) / (tTotalMeasure - i);
     if (x <= ratio) {
       RandCounts[nCumCounts] = i;
       nCumCounts++;
     }
  }
}

/** Distributes cases into simulation case array, where individuals are initially dichotomized into cases and
    controls then each randomly assigned to be a case or a control. Caller is responsible for ensuring that
    passed array pointers are allocated and dimensions match that of passed tract and locations variables. */
void AbstractOrdinalDenominatorDataRandomizer::RandomizeOrdinalData(count_t tNumCases,
                                                                    count_t tNumControls,
                                                                    count_t** ppSimCases,
                                                                    measure_t** ppMeasure,
                                                                    int tNumTracts,
                                                                    int tNumTimeIntervals) {
  count_t               nCumCounts, nCumMeasure;
  std::vector<count_t>	RandCounts;

  nCumCounts = tNumCases < tNumControls ? tNumCases : tNumControls;
  MakeDataB(nCumCounts, tNumCases + tNumControls, RandCounts);
  nCumMeasure = tNumCases + tNumControls - 1;
  for (int t=tNumTracts-1; t >= 0; --t) {
     for (int i = tNumTimeIntervals-1; i >= 0; --i) {
        if (i == tNumTimeIntervals-1)
          nCumMeasure -= (count_t)(ppMeasure[i][t]);
        else
          nCumMeasure -= (count_t)(ppMeasure[i][t] - ppMeasure[i+1][t]);
        while (nCumCounts > 0 && RandCounts[nCumCounts-1] > nCumMeasure) {
             ppSimCases[i][t]++;
             nCumCounts--;
        }
     }
  }
  //now set as cumulative, in respect to time intervals
  for (int i=tNumTimeIntervals-2; i >= 0; i--)
     for (int t=0; t < tNumTracts; t++)
        ppSimCases[i][t] += ppSimCases[i+1][t];
  //now reverse everything if Controls < Cases
  if (tNumCases >= tNumControls) {
    for (int t=0; t < tNumTracts; ++t)
       for (int i=0; i < tNumTimeIntervals; ++i)
          ppSimCases[i][t] = (count_t)(ppMeasure[i][t]) - ppSimCases[i][t];
  }
}

/** Distributes cases into simulation case array where individuals are initially dichotomized into cases and controls 
   then each randomly assigned to be a case or a control. This procedure is specialized for time stratification - meaning that
   cases and controls are randomized and distributed in each time interval separately. */
void AbstractOrdinalDenominatorDataRandomizer::RandomizeOrdinalDataTimeStratified(const RealDataSet& RealSet, DataSet& SimSet) {
    std::vector<count_t> RandCounts;
    int tNumTracts = RealSet.getLocationDimension(), tNumTimeIntervals = RealSet.getIntervalDimension();
    count_t intervalCases, intervalControls, cumulativeCounts, cumulativeMeasure, * pCasesPerInterval = RealSet.getCaseData_PT_NC(), ** ppSimCases = SimSet.getCaseData().GetArray();
    measure_t * pMeasurePerInterval = RealSet.getMeasureData_PT_NC(), ** ppMeasure = RealSet.getMeasureData().GetArray();

    // randomize and distribute cases/controls in each time interval separately
    for (int i = tNumTimeIntervals - 1; i >= 0; --i) {
        intervalCases = pCasesPerInterval[i];
        intervalControls = (count_t)pMeasurePerInterval[i] - intervalCases;
        if (intervalCases == 0 && intervalControls == 0)
            continue; // No cases or controls, move to next interval.
        cumulativeCounts = intervalCases < intervalControls ? intervalCases : intervalControls;
        MakeDataB(cumulativeCounts, intervalCases + intervalControls, RandCounts);
        cumulativeMeasure = intervalCases + intervalControls - 1;
        // Distribute counts among tracts in this time interval.
        for (int t=tNumTracts-1; t >= 0; --t) {
            // Decrement cumulative measure by the measure of this tract, in this time interval.
            cumulativeMeasure -= (i == tNumTimeIntervals - 1 ? (count_t)(ppMeasure[i][t]) : (count_t)(ppMeasure[i][t] - ppMeasure[i + 1][t]));
            while (cumulativeCounts > 0 && RandCounts[cumulativeCounts - 1] > cumulativeMeasure) {
                ++ppSimCases[i][t];
                --cumulativeCounts;
            }
        }
        // Now reverse everything if Controls < Cases in current time interval -- ppSimCases is not cumulative but ppMeasure is.
        if (intervalControls <= intervalCases) {
            for (int t = tNumTracts - 1; t >= 0; --t)
                ppSimCases[i][t] = (count_t)(ppMeasure[i][t] - (i == tNumTimeIntervals - 1 ? 0.0 : ppMeasure[i + 1][t])) - ppSimCases[i][t];
        }
    }
    // now set as cumulative in respect to time intervals
    for (int i = tNumTimeIntervals - 2; i >= 0; i--)
        for (int t = 0; t < tNumTracts; t++)
            ppSimCases[i][t] += ppSimCases[i + 1][t];
}


/** Distributes cases into simulation case array where individuals are initially dichotomized into cases and controls
then each randomly assigned to be a case or a control. This procedure is specialized for spatial stratification - meaning that
cases and controls are randomized and distributed for each location separately. */
void AbstractOrdinalDenominatorDataRandomizer::RandomizeOrdinalDataSpatialStratified(const RealDataSet& RealSet, DataSet& SimSet) {
    std::vector<count_t> RandCounts;
    int tNumTracts = RealSet.getLocationDimension(), tNumTimeIntervals = RealSet.getIntervalDimension();
    count_t locCases, locControls, cumulativeCounts, cumulativeMeasure, ** ppCases = RealSet.getCaseData().GetArray(), ** ppSimCases = SimSet.getCaseData().GetArray();
    measure_t ** ppMeasure = RealSet.getMeasureData().GetArray();

    // randomize and distribute cases/controls in each location separately
    for (int t = tNumTracts - 1; t >= 0; --t) {
        locCases = ppCases[0][t];
        locControls = (count_t)ppMeasure[0][t] - locCases;
        if (locCases == 0 && locControls == 0)
            continue; // No cases or controls, move to next location.
        cumulativeCounts = locCases < locControls ? locCases : locControls;
        MakeDataB(cumulativeCounts, locCases + locControls, RandCounts);
        cumulativeMeasure = locCases + locControls - 1;
        // Distribute counts among intervals of this tract.
        for (int i = tNumTimeIntervals - 1; i >= 0; --i) {
            // Decrement cumulative measure by the measure of this interval, for current tract.
            cumulativeMeasure -= (i == tNumTimeIntervals - 1 ? (count_t)(ppMeasure[i][t]) : (count_t)(ppMeasure[i][t] - ppMeasure[i + 1][t]));
            while (cumulativeCounts > 0 && RandCounts[cumulativeCounts - 1] > cumulativeMeasure) {
                ++ppSimCases[i][t];
                --cumulativeCounts;
            }
        }
        // Now reverse everything if Controls < Cases in current time interval -- ppSimCases is not cumulative but ppMeasure is.
        if (locControls <= locCases) {
            for (int i = tNumTimeIntervals - 1; i >= 0; --i)
                ppSimCases[i][t] = (count_t)(ppMeasure[i][t] - (i == tNumTimeIntervals - 1 ? 0.0 : ppMeasure[i + 1][t])) - ppSimCases[i][t];
        }
    }
    // now set as cumulative in respect to time intervals
    for (int i = tNumTimeIntervals - 2; i >= 0; i--)
        for (int t = 0; t < tNumTracts; t++)
            ppSimCases[i][t] += ppSimCases[i + 1][t];
}

/** Distributes cases into simulation case array, where individuals are initially dichotomized into cases and
    controls then each randomly assigned to be a case or a control. optimized for purely temporal analyses.
    Caller is responsible for ensuring that passed array pointers are allocated and dimensions match that of
    passed tract and locations variables. */
void AbstractOrdinalDenominatorDataRandomizer::RandomizePurelyTemporalOrdinalData(count_t tNumCases,
                                                                                  count_t tNumControls,
                                                                                  count_t* pSimCases,
                                                                                  measure_t* pMeasure,
                                                                                  int tNumTimeIntervals) {
  count_t               nCumCounts, nCumMeasure;
  std::vector<count_t>	RandCounts;

  nCumCounts = tNumCases < tNumControls ? tNumCases : tNumControls;
  MakeDataB(nCumCounts, tNumCases + tNumControls, RandCounts);
  nCumMeasure = tNumCases + tNumControls - 1;
  for (int i = tNumTimeIntervals-1; i >= 0; --i) {
     nCumMeasure -= (i == tNumTimeIntervals - 1  ? (count_t)(pMeasure[i]) : (count_t)(pMeasure[i] - pMeasure[i+1]));
     while (nCumCounts > 0 && RandCounts[nCumCounts-1] > nCumMeasure) {
          ++pSimCases[i];
          --nCumCounts;
     }
  }
  //now set as cumulative, in respect to time intervals
  for (int i=tNumTimeIntervals-2; i >= 0; i--)
     pSimCases[i] += pSimCases[i+1];
  //now reverse everything if Controls < Cases
  if (tNumCases >= tNumControls)
    for (int i=0; i < tNumTimeIntervals; ++i)
       pSimCases[i] = (count_t)(pMeasure[i]) - pSimCases[i];
}


/** Distributes cases into simulation case array, where individuals are initially dichotomized into cases and
    controls then each randomly assigned to be a case or a control. Caller is responsible for ensuring that
    passed array pointers are allocated and dimensions match that of passed tract and locations variables. 
    This version of the ordinal randomization is specialized for the day of week adjustment. */
void AbstractOrdinalDenominatorDataRandomizer::RandomizeOrdinalDataForDayOfWeek(count_t totalCasesForWeekDay, 
                                                                                count_t totalControlForWeekDay, 
                                                                                count_t** ppSimCases,
                                                                                measure_t** ppMeasure, 
                                                                                int tNumTracts, 
                                                                                int lastTimeInterval,
                                                                                bool lastDay,
                                                                                count_t tTotalCases, 
                                                                                count_t tTotalControls,
                                                                                int tNumTimeIntervals) {
  count_t               nCumCounts, nCumMeasure;
  std::vector<count_t>	RandCounts;

  nCumCounts = totalCasesForWeekDay < totalControlForWeekDay ? totalCasesForWeekDay : totalControlForWeekDay;
  MakeDataB(nCumCounts, totalCasesForWeekDay + totalControlForWeekDay, RandCounts);
  nCumMeasure = totalCasesForWeekDay + totalControlForWeekDay - 1;
  for (int t=tNumTracts-1; t >= 0; --t) {
     for (int i = lastTimeInterval - 1; i >= 0;) {
        if (i == lastTimeInterval - 1)
          nCumMeasure -= (count_t)(ppMeasure[i][t]);
        else
          nCumMeasure -= (count_t)(ppMeasure[i][t] - ppMeasure[i+1][t]);
        while (nCumCounts > 0 && RandCounts[nCumCounts-1] > nCumMeasure) {
             ppSimCases[i][t]++;
             nCumCounts--;
        }
        i -= 7; // go back 7 days to previous week, same day of week
     }
  }
  
  if (lastDay) {
    //now set as cumulative, in respect to time intervals
    for (int i=tNumTimeIntervals-2; i >= 0; i--)
        for (int t=0; t < tNumTracts; t++)
            ppSimCases[i][t] += ppSimCases[i+1][t];
    //now reverse everything if Controls < Cases
    if (tTotalCases >= tTotalControls) {
        for (int t=0; t < tNumTracts; ++t)
            for (int i=0; i < tNumTimeIntervals; ++i)
            ppSimCases[i][t] = (count_t)(ppMeasure[i][t]) - ppSimCases[i][t];
    }
  }
}

/** Distributes cases into simulation case array, where individuals are initially dichotomized into cases and
    controls then each randomly assigned to be a case or a control. Caller is responsible for ensuring that
    passed array pointers are allocated and dimensions match that of passed tract and locations variables. 
    This version of the ordinal randomization is specialized for the day of week adjustment. */
void AbstractOrdinalDenominatorDataRandomizer::RandomizePurelyTemporalOrdinalDataForDayOfWeek(count_t totalCasesForWeekDay,
                                                                                              count_t totalControlForWeekDay,
                                                                                              count_t* pSimCases,
                                                                                              measure_t* pMeasure,
                                                                                              int tNumTracts,
                                                                                              int lastTimeInterval,
                                                                                              bool lastDay,
                                                                                              count_t tTotalCases,
                                                                                              count_t tTotalControls,
                                                                                              int tNumTimeIntervals) {
  count_t nCumCounts, nCumMeasure;
  std::vector<count_t> RandCounts;

  nCumCounts = totalCasesForWeekDay < totalControlForWeekDay ? totalCasesForWeekDay : totalControlForWeekDay;
  MakeDataB(nCumCounts, totalCasesForWeekDay + totalControlForWeekDay, RandCounts);
  nCumMeasure = totalCasesForWeekDay + totalControlForWeekDay - 1;
  for (int t=tNumTracts-1; t >= 0; --t) {
     for (int i = lastTimeInterval - 1; i >= 0;) {
        if (i == lastTimeInterval - 1)
          nCumMeasure -= (count_t)(pMeasure[i]);
        else
          nCumMeasure -= (count_t)(pMeasure[i] - pMeasure[i+1]);
        while (nCumCounts > 0 && RandCounts[nCumCounts-1] > nCumMeasure) {
             pSimCases[i]++;
             nCumCounts--;
        }
        i -= 7; // go back 7 days to previous week, same day of week
     }
  }
  
  if (lastDay) {
    //now set as cumulative, in respect to time intervals
    for (int i=tNumTimeIntervals-2; i >= 0; i--)
        pSimCases[i] += pSimCases[i+1];
    //now reverse everything if Controls < Cases
    if (tTotalCases >= tTotalControls) {
        for (int i=0; i < tNumTimeIntervals; ++i)
            pSimCases[i] = (count_t)(pMeasure[i]) - pSimCases[i];
    }
  }
}
