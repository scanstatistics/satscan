#include "SaTScan.h"
#pragma hdrstop
#include "AdjustmentHandler.h"

/** constructor */
RelativeRiskAdjustment::RelativeRiskAdjustment(measure_t dRelativeRisk, Julian StartDate, Julian EndDate) {
  try {
    SetRelativeRisk(dRelativeRisk);
    SetStartDate(StartDate);
    SetEndDate(EndDate);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()", "RelativeRiskAdjustment");
    throw;
  }
}

/** destructor */
RelativeRiskAdjustment::~RelativeRiskAdjustment() {}

void RelativeRiskAdjustment::MultiplyRisk(measure_t dRisk) {
  try {
    if (dRisk > std::numeric_limits<measure_t>::max()/gdRelativeRisk)
      GenerateResolvableException("Error: Data overflow occurs when adjusting expected number of cases.\n"
                                  "       The combined relative risk %lf and %lf in the adjustment file\n"
                                  "       is too large.\n", "AssignMeasure()", gdRelativeRisk, dRisk);

    gdRelativeRisk *= dRisk;
  }
  catch (ZdException &x) {
    x.AddCallpath("MultiplyRisk()", "RelativeRiskAdjustment");
    throw;
  }
}


/** constructor */
RelativeRiskAdjustmentHandler::RelativeRiskAdjustmentHandler() {}

/** destructor */
RelativeRiskAdjustmentHandler::~RelativeRiskAdjustmentHandler() {}

/** defines relative risk adjustment for passed data */
void RelativeRiskAdjustmentHandler::AddAdjustmentData(tract_t tTractIndex, measure_t dRelativeRisk, Julian StartDate, Julian EndDate) {
  size_t				i, iStartIndex, iEndIndex;
  Julian                                TempDate;
  measure_t                             TempRisk;
  TractContainerIterator_t              itr_Start, itr_End;

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
      for (i=iEndIndex; i > iStartIndex; --i) {
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
  catch (ZdException &x) {
    x.AddCallpath("AddAdjustmentData()", "RelativeRiskAdjustmentHandler");
    throw;
  }
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
void RelativeRiskAdjustmentHandler::PrintAdjustments(TractHandler & tHandler) {
  AdjustmentsIterator_t                 itr;
  TractContainerIteratorConst_t         itr_deque;
  std::string                           sTractId;
  ZdString                              sStart, sEnd;
  FILE                                * pFile;  

  if ((pFile = fopen("c:\\Adustments.txt", "w")) == NULL)
    ZdGenerateException("Unable to create adjustments outpt file.","PrintAdjustments()");

  for (itr=gTractAdjustments.begin(); itr != gTractAdjustments.end(); ++itr) {
     tHandler.tiGetTid(itr->first, sTractId);
     const TractContainer_t & tract_deque = itr->second;
     fprintf(pFile, "Tract %s:\n", sTractId.c_str());
     for (itr_deque=tract_deque.begin(); itr_deque != tract_deque.end(); ++itr_deque) {
        JulianToString(sStart,(*itr_deque).GetStartDate());
        JulianToString(sEnd, (*itr_deque).GetEndDate());
        fprintf(pFile, "%lf\t%s\t%s\n", (*itr_deque).GetRelativeRisk(), sStart.GetCString(), sEnd.GetCString());
     }
     fprintf(pFile, "\n\n");
  }
  fclose(pFile);
}


