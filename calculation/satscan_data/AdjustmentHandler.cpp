#include "SaTScan.h"
#pragma hdrstop
#include "AdjustmentHandler.h"
#include "SSException.h" 
#include "SaTScanData.h"

/** constructor */
RelativeRiskAdjustment::RelativeRiskAdjustment(measure_t dRelativeRisk, Julian StartDate, Julian EndDate) {
  try {
    SetRelativeRisk(dRelativeRisk);
    SetStartDate(StartDate);
    SetEndDate(EndDate);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()", "RelativeRiskAdjustment");
    throw;
  }
}

void RelativeRiskAdjustment::MultiplyRisk(measure_t dRisk) {
  try {
    if (dRisk > std::numeric_limits<measure_t>::max()/gdRelativeRisk)
      throw resolvable_error("Error: Data overflow occurs when adjusting expected number of cases.\n"
                             "       The combined relative risk %g and %g in the adjustment file\n"
                             "       is too large.\n", gdRelativeRisk, dRisk);

    gdRelativeRisk *= dRisk;
  }
  catch (prg_exception& x) {
    x.addTrace("MultiplyRisk()", "RelativeRiskAdjustment");
    throw;
  }
}

////////////////// RelativeRiskAdjustmentHandler /////////////////////////

/** defines relative risk adjustment for passed data */
void RelativeRiskAdjustmentHandler::add(tract_t tTractIndex, measure_t dRelativeRisk, Julian StartDate, Julian EndDate) {
  size_t iStartIndex, iEndIndex;
  Julian TempDate;
  measure_t TempRisk;
  TractContainerIterator_t itr_Start, itr_End;

  try {
    //find std::deque for tTractIndex
    if (gTractAdjustments.find(tTractIndex) == gTractAdjustments.end())
      gTractAdjustments[tTractIndex] = TractContainer_t();
    TractContainer_t & Adjustments = gTractAdjustments[tTractIndex];

    //determine adjustment period indexes into existing adjustment periods
    if ((itr_Start = GetMaxPeriodIndex(Adjustments, StartDate)) == Adjustments.end()) {
      //start date does not exist or is beyond existing periods - just push onto back
      Adjustments.push_back(RelativeRiskAdjustment(dRelativeRisk, StartDate, EndDate));
      return;
    }
    else
      iStartIndex = std::distance(Adjustments.begin(), itr_Start);
    //create new period if new adjustment period goes beyond any existing period
    if ((itr_End = GetMaxPeriodIndex(Adjustments, EndDate)) == Adjustments.end()) {
      Adjustments.push_back(RelativeRiskAdjustment(1, Adjustments.back().GetEndDate() + 1, EndDate));
      iEndIndex = Adjustments.size() - 1;
    }
    else
      iEndIndex = std::distance(Adjustments.begin(), itr_End);

    if (iStartIndex == iEndIndex && StartDate < Adjustments[iStartIndex].GetStartDate() && EndDate < Adjustments[iStartIndex].GetStartDate())
      //new adjustment period entirely within period were there are no existing adjustments
      Adjustments.insert(Adjustments.begin() + iStartIndex, RelativeRiskAdjustment(dRelativeRisk, StartDate, EndDate));
    else if (iStartIndex == iEndIndex && StartDate > Adjustments[iStartIndex].GetStartDate() && EndDate < Adjustments[iStartIndex].GetEndDate()) {
      // neither date of new adjustment period is on existing adjustment periods boundry
      TempDate = Adjustments[iStartIndex].GetEndDate();
      TempRisk = Adjustments[iStartIndex].GetRelativeRisk();
      Adjustments[iStartIndex].SetEndDate(StartDate - 1);
      Adjustments.insert(Adjustments.begin() + (iStartIndex + 1), RelativeRiskAdjustment(TempRisk * dRelativeRisk, StartDate, EndDate));
      Adjustments.insert(Adjustments.begin() + (iStartIndex + 2), RelativeRiskAdjustment(TempRisk, EndDate + 1, TempDate));
    }
    else if (iStartIndex == iEndIndex && StartDate == Adjustments[iStartIndex].GetStartDate() && EndDate == Adjustments[iStartIndex].GetEndDate())
      Adjustments[iStartIndex].MultiplyRisk(dRelativeRisk);
    else { //now work with StartDate and EndDate separately
      if (StartDate < Adjustments[iStartIndex].GetStartDate()) {
        //insert new adjustment before existing - the relative risk will be set in loop below
        Adjustments.insert(Adjustments.begin() + iStartIndex, RelativeRiskAdjustment(1, StartDate, Adjustments[iStartIndex].GetStartDate() - 1));
        ++iEndIndex;
      }
      else if (StartDate > Adjustments[iStartIndex].GetStartDate()) {
        TempDate = Adjustments[iStartIndex].GetEndDate();
        TempRisk = Adjustments[iStartIndex].GetRelativeRisk();
        //adjust existing periods end date and add new adjustment after it
        //the relative risk for added period will be updated in loop below
        Adjustments[iStartIndex].SetEndDate(StartDate - 1);
        Adjustments.insert(Adjustments.begin() + (iStartIndex + 1), RelativeRiskAdjustment(TempRisk, StartDate, TempDate));
        ++iStartIndex;
        ++iEndIndex;
      }
      //else falls on boundry -- nothing to do for StartDate
      //... and now the end date of new adjustment period
      if (EndDate < Adjustments[iEndIndex].GetStartDate())
        //insert new adjustment period that fills period down to next lower existing period
        //the relative risk will be set below, before loop
        Adjustments.insert(Adjustments.begin() + iEndIndex, RelativeRiskAdjustment(1, Adjustments[iEndIndex - 1].GetEndDate() + 1, EndDate));
      else if (EndDate >= Adjustments[iEndIndex].GetStartDate() && EndDate < Adjustments[iEndIndex].GetEndDate()) {
        TempDate = Adjustments[iEndIndex].GetStartDate();
        TempRisk = Adjustments[iEndIndex].GetRelativeRisk();
        //insert new period with upper limit determined by new adjustment end date
        //the relative risk will be set below, before loop
        Adjustments[iEndIndex].SetStartDate(EndDate + 1);
        Adjustments.insert(Adjustments.begin() + iEndIndex, RelativeRiskAdjustment(TempRisk, TempDate, EndDate));
      }
      //else falls on boundry - nothing to do for EndDate
      
      //at this point, adjustment at iEndIndex represents upper boundry for new period
      //that why we don't update risk in above code
      Adjustments[iEndIndex].MultiplyRisk(dRelativeRisk);
      //create new adjustment periods in existing adjustment period between StartDate and EndDate to cause continious adjustment period
      //ensure two things - the period from StartDate to EndDate is contiguous(i.e. fill 'open' periods between adjustments dates)
      //                  - update relative risk for existing adjustments within new period
      for (size_t i=iEndIndex; i > iStartIndex; --i) {
         if (Adjustments[i].GetStartDate() - 1 != Adjustments[i - 1].GetEndDate()) {
           Adjustments.insert(Adjustments.begin() + i,
                              RelativeRiskAdjustment(dRelativeRisk, Adjustments[i - 1].GetEndDate() + 1,
                                                     Adjustments[i].GetStartDate() - 1));
           ++i; // still need to check between inserted item and current i - 1 element                                          
         }
         else //update relative risk
           Adjustments[i - 1].MultiplyRisk(dRelativeRisk);
      }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("add()", "RelativeRiskAdjustmentHandler");
    throw;
  }
}

/**************************************************************************************
Adjusts the measure for a particular tract and a set of time intervals, reflecting an
increased or decreased relative risk in a specified adjustment time period. For time
intervals completely within the adjustment period, the measure is simply multiplied by
the relative risk. For time intervals that are only partly within the adjustment period,
only that proportion is multiplied by the relative risk, and the other proportion remains
the same, after which they are added.
Input: Tract, Adjustment Time Period, Relative Risk
*****************************************************************************************/
bool RelativeRiskAdjustmentHandler::AdjustMeasure(const TwoDimMeasureArray_t& Measure, 
                                                  const PopulationData & Population, 
                                                  tract_t Tract, 
                                                  double dRelativeRisk, 
                                                  Julian StartDate, 
                                                  Julian EndDate, 
                                                  TwoDimMeasureArray_t& adjustMeasure, 
                                                  const TwoDimCountArray_t * pCases) const {
  measure_t ** pp_m = Measure.GetArray();
  count_t ** ppCases = pCases ? pCases->GetArray() : 0;
  measure_t ** pNonCumulativeMeasure = adjustMeasure.GetArray();

  for (int interval=_dataHub.CSaTScanData::GetTimeIntervalOfDate(StartDate); interval <= _dataHub.CSaTScanData::GetTimeIntervalOfDate(EndDate); ++interval) {
     Julian AdjustmentStart = std::max(StartDate, _dataHub.CSaTScanData::GetTimeIntervalStartTimes()[interval]);
     Julian AdjustmentEnd = std::min(EndDate, _dataHub.CSaTScanData::GetTimeIntervalStartTimes()[interval+1] - 1);
     //calculate measure for lower interval date to adjustment start date
     measure_t MeasurePre = CalcMeasureForTimeInterval(Population, pp_m, Tract, _dataHub.CSaTScanData::GetTimeIntervalStartTimes()[interval], AdjustmentStart);
     //calculate measure for adjustment period
     measure_t MeasureDuring = CalcMeasureForTimeInterval(Population, pp_m, Tract, AdjustmentStart, AdjustmentEnd+1);
     //calculate measure for adjustment end date to upper interval date
     measure_t MeasurePost = CalcMeasureForTimeInterval(Population, pp_m, Tract, AdjustmentEnd+1, _dataHub.CSaTScanData::GetTimeIntervalStartTimes()[interval+1]);
     //validate that data overflow will not occur
     if (MeasureDuring && (dRelativeRisk > (std::numeric_limits<measure_t>::max() - MeasurePre - MeasurePost) / MeasureDuring))
       throw resolvable_error("Error: Data overflow occurs when adjusting expected number of cases.\n"
                              "       The specified relative risk %lf in the adjustment file\n"
                              "       is too large.\n", dRelativeRisk);
     //assign adjusted measure                      
     pNonCumulativeMeasure[interval][Tract] = MeasurePre + dRelativeRisk * MeasureDuring + MeasurePost;

     //if measure has been adjusted to zero, check that cases adjusted interval are also zero
     if (ppCases && pNonCumulativeMeasure[interval][Tract] == 0 && getCaseCount(ppCases, interval, Tract)) {
       std::string  sStart, sEnd;
       throw resolvable_error("Error: For locationID '%s', you have adjusted the expected number\n"
                              "       of cases in the period %s to %s to be zero, but there\n"
                              "       are cases in that interval.\n"
                              "       If the expected is zero, the number of cases must also be zero.\n",
                              (Tract == -1 ? "All" : _dataHub.GetGroupInfo().getObservationGroups().at(Tract)->groupname().c_str()),
                              JulianToString(sStart, StartDate, _dataHub.GetParameters().GetPrecisionOfTimesType()).c_str(),
                              JulianToString(sEnd, EndDate, _dataHub.GetParameters().GetPrecisionOfTimesType()).c_str());
     }
  }
  return true;
}

/** Adjusts passed non cumulative measure are for known relative risks, as
    previously read from user specified file. Caller is responsible for ensuring:
    - that passed 'measure **' points to a multiple dimensional array contained
      by passed RealDataSet object.
    - passed 'measure **' is in fact non-cumulative
    - passed 'measure **' points to valid memory, allocated to dimensions (number
      of time intervals plus one by number of tracts)                            */
void RelativeRiskAdjustmentHandler::apply(const TwoDimMeasureArray_t& Measure, const PopulationData & Population, measure_t totalMeasure, TwoDimMeasureArray_t& adjustMeasure, const TwoDimCountArray_t * pCases) const {
  if (!gTractAdjustments.size()) return;

  //apply adjustments to relative risks
  for (AdjustmentsIterator_t itr=gTractAdjustments.begin(); itr != gTractAdjustments.end(); ++itr) {
     const TractContainer_t & tract_deque = itr->second;
     for (TractContainerIteratorConst_t itr_deque=tract_deque.begin(); itr_deque != tract_deque.end(); ++itr_deque)
         AdjustMeasure(Measure, Population, itr->first, (*itr_deque).GetRelativeRisk(), (*itr_deque).GetStartDate(), (*itr_deque).GetEndDate(), adjustMeasure, pCases);
  }

  // calculate total adjusted measure
  measure_t ** ppNonCumulativeMeasure = adjustMeasure.GetArray(), adjustedTotalMeasure_t=0;
  for (int i=0; i < adjustMeasure.Get1stDimension(); ++i)
     for (tract_t t=0; t < adjustMeasure.Get2ndDimension(); ++t)
        adjustedTotalMeasure_t += ppNonCumulativeMeasure[i][t];
  //Mutlipy the measure for each interval/tract by constant (c) to obtain total
  //adjusted measure (AdjustedTotalMeasure_t) equal to previous total measure (m_nTotalMeasure).
  measure_t c = totalMeasure/adjustedTotalMeasure_t;
  for (int i=0; i < adjustMeasure.Get1stDimension(); ++i)
     for (tract_t t=0; t < adjustMeasure.Get2ndDimension(); ++t)
        ppNonCumulativeMeasure[i][t] *= c;
}

/**************************************************************************************
Calculates measure M for a requested tract and time interval.
Input: Tract, Time Interval, # Pop Points, Measure array for population points
       StudyStartDate, StudyEndDate
Time Interval = [StartDate , EndDate]; EndDate = NextStartDate-1
Note: The measure 'M' is the same measure used for the population points, which is later
calibrated before being put into the measure array.
**************************************************************************************/
measure_t RelativeRiskAdjustmentHandler::CalcMeasureForTimeInterval(const PopulationData & Population, measure_t ** ppPopulationMeasure, tract_t Tract, Julian StartDate, Julian NextStartDate) const {
  int           i, iStartUpperIndex, iNextLowerIndex;
  long          nTotalDays = _dataHub.GetStudyPeriodEndDate() + 1 - _dataHub.GetStudyPeriodStartDate();
  measure_t     SumMeasure;

  if (StartDate >= NextStartDate )
    return 0;

  SumMeasure = 0;
  iStartUpperIndex = Population.UpperPopIndex(StartDate);
  iNextLowerIndex = Population.LowerPopIndex(NextStartDate);

  if (iStartUpperIndex <= iNextLowerIndex) {
    SumMeasure += 0.5 * (DateMeasure(Population, ppPopulationMeasure, StartDate, Tract) + ppPopulationMeasure[iStartUpperIndex][Tract]) *
                  (Population.GetPopulationDate(iStartUpperIndex) - StartDate);
    for (i=iStartUpperIndex; i < iNextLowerIndex; ++i)
       SumMeasure += 0.5 * (ppPopulationMeasure[i][Tract] + ppPopulationMeasure[i+1][Tract] ) *
                    (Population.GetPopulationDate(i+1) - Population.GetPopulationDate(i));
    SumMeasure += 0.5 * (DateMeasure(Population, ppPopulationMeasure, NextStartDate, Tract) + ppPopulationMeasure[iNextLowerIndex][Tract])
                  * (NextStartDate - Population.GetPopulationDate(iNextLowerIndex));
  }
  else
    SumMeasure += 0.5 * (DateMeasure(Population, ppPopulationMeasure, StartDate,Tract) +
                        DateMeasure(Population, ppPopulationMeasure, NextStartDate,Tract)) * (NextStartDate - StartDate);

   return SumMeasure / nTotalDays;
}

/********************************************************************
  Finds the measure M for a requested tract and date.
  Input: Date, Tracts, #PopPoints
 ********************************************************************/
measure_t RelativeRiskAdjustmentHandler::DateMeasure(const PopulationData & Population, measure_t ** ppPopulationMeasure, Julian Date, tract_t Tract) const {
  int           iPopDateIndex=0, iNumPopDates;
  measure_t     tRelativePosition;

  iNumPopDates = Population.GetNumPopulationDates();
  
  if (Date <= Population.GetPopulationDate(0))
    return ppPopulationMeasure[0][Tract];
  else if (Date >= Population.GetPopulationDate(iNumPopDates - 1))
    return ppPopulationMeasure[iNumPopDates - 1][Tract];
  else {
    /** Finds the index of the last PopDate before or on the Date **/
    while (Population.GetPopulationDate(iPopDateIndex+1) <= Date)
        iPopDateIndex++;
    //Calculates the relative position of the Date between the Previous PopDate and
    //the following PopDate, on a scale from zero to one.
    tRelativePosition = (measure_t)(Date - Population.GetPopulationDate(iPopDateIndex)) /
                        (measure_t)(Population.GetPopulationDate(iPopDateIndex+1) - Population.GetPopulationDate(iPopDateIndex));
    //Calculates measure M at the time of the StartDate
    return (1 - tRelativePosition) * ppPopulationMeasure[iPopDateIndex][Tract] + tRelativePosition * ppPopulationMeasure[iPopDateIndex+1][Tract];
  }
}

/** Returns the number of cases for a specified tract and time interval.
    Note: iInterval and tTract should be valid indexes of the cases array .**/
count_t RelativeRiskAdjustmentHandler::getCaseCount(count_t ** ppCumulativeCases, int iInterval, tract_t tTract) const {
  if (iInterval + 1 == _dataHub.CSaTScanData::GetNumTimeIntervals())
    return ppCumulativeCases[iInterval][tTract];
  else
    return ppCumulativeCases[iInterval][tTract] - ppCumulativeCases[iInterval + 1][tTract];
}

/** Returns the top most bounding index from existing adjustments periods.
    Iterator returned indicates that either date is within items period or in
    'open' period below item. If Date is beyond any existing items, iterator
    equals container.end().                                                   */
TractContainerIterator_t RelativeRiskAdjustmentHandler::GetMaxPeriodIndex(TractContainer_t & Container, Julian Date) {
  TractContainerIterator_t      itr, itr_index;

  //find date index
  itr_index = Container.end();
  for (itr=Container.begin(); itr != Container.end() && itr_index == Container.end(); ++itr)
     if (Date <= itr->GetEndDate())
       itr_index = itr;

  return itr_index;
} 

/** Prints all defined adjustments to text file. */
void RelativeRiskAdjustmentHandler::print(ObservationGroupingManager & groups) {
  AdjustmentsIterator_t                 itr;
  TractContainerIteratorConst_t         itr_deque;
  std::string                           sStart, sEnd;
  FILE                                * pFile=0;

  if ((pFile = fopen("c:\\Adustments.txt", "w")) == NULL)
    throw prg_error("Unable to create adjustments outpt file.","PrintAdjustments()");

  for (itr=gTractAdjustments.begin(); itr != gTractAdjustments.end(); ++itr) {
     const TractContainer_t & tract_deque = itr->second;
     fprintf(pFile, "Tract %s:\n", groups.getObservationGroups().at(itr->first)->groupname().c_str());
     for (itr_deque=tract_deque.begin(); itr_deque != tract_deque.end(); ++itr_deque) {
        JulianToString(sStart,(*itr_deque).GetStartDate(), DAY);
        JulianToString(sEnd, (*itr_deque).GetEndDate(), DAY);
        fprintf(pFile, "%lf\t%s\t%s\n", (*itr_deque).GetRelativeRisk(), sStart.c_str(), sEnd.c_str());
     }
     fprintf(pFile, "\n\n");
  }
  if (pFile) fclose(pFile);
}

