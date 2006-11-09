//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "TimeIntervalRange.h"
#include "SSException.h"
#include "SaTScanDataRead.h"
#include "NormalRandomizer.h"
#include "ExponentialDataSetHandler.h"
#include "ParametersPrint.h"
#include <boost/dynamic_bitset.hpp>
#include "MetaTractManager.h"

/** class constructor */
CSaTScanData::CSaTScanData(const CParameters& Parameters, BasePrint& PrintDirection)
             : gParameters(Parameters), gPrint(PrintDirection),
               m_nStartDate(CharToJulian(Parameters.GetStudyPeriodStartDate().c_str())),
               m_nEndDate(CharToJulian(Parameters.GetStudyPeriodEndDate().c_str())) {
  try {
    Init();
    Setup();
  }
  catch (ZdException & x) {
    x.AddCallpath("constructor", "CSaTScanData");
    throw;
  }
}

/** class destructor */
CSaTScanData::~CSaTScanData() {
  try {
    delete m_pModel;
    delete gpNeighborCountHandler; gpNeighborCountHandler=0;
    delete gpReportedNeighborCountHandler; gpReportedNeighborCountHandler=0;
    delete gpSortedIntHandler; gpSortedIntHandler=0;
    delete gpSortedUShortHandler; gpSortedUShortHandler=0;
  }
  catch (...){}  
}

/** Adjusts passed non cumulative measure are for known relative risks, as
    previously read from user specified file. Caller is responsible for ensuring:
    - that passed 'measure **' points to a multiple dimensional array contained
      by passed RealDataSet object.
    - passed 'measure **' is in fact non-cumulative
    - passed 'measure **' points to valid memory, allocated to dimensions (number
      of time intervals plus one by number of tracts)                            */
void CSaTScanData::AdjustForKnownRelativeRisks(RealDataSet& Set, const TwoDimMeasureArray_t& PopMeasure) {
  measure_t                             c, AdjustedTotalMeasure_t;
  int                                   i;
  tract_t                               t;  
  AdjustmentsIterator_t                 itr;
  TractContainerIteratorConst_t         itr_deque;

  //apply adjustments to relative risks
  for (itr=gRelativeRiskAdjustments.GetAdjustments().begin(); itr != gRelativeRiskAdjustments.GetAdjustments().end(); ++itr) {
     const TractContainer_t & tract_deque = itr->second;
     for (itr_deque=tract_deque.begin(); itr_deque != tract_deque.end(); ++itr_deque)
        AdjustMeasure(Set, PopMeasure, itr->first, (*itr_deque).GetRelativeRisk(), (*itr_deque).GetStartDate(), (*itr_deque).GetEndDate());
  }

  // calculate total adjusted measure
  measure_t ** ppNonCumulativeMeasure = Set.getMeasureData().GetArray();
  for (AdjustedTotalMeasure_t=0, i=0; i < m_nTimeIntervals; ++i)
     for (t=0; t < m_nTracts; ++t)
        AdjustedTotalMeasure_t += ppNonCumulativeMeasure[i][t];
  //Mutlipy the measure for each interval/tract by constant (c) to obtain total
  //adjusted measure (AdjustedTotalMeasure_t) equal to previous total measure (m_nTotalMeasure).
  c = Set.getTotalMeasure()/AdjustedTotalMeasure_t;
  for (i=0; i < m_nTimeIntervals; ++i)
     for (t=0; t < m_nTracts; ++t)
        ppNonCumulativeMeasure[i][t] *= c;
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
bool CSaTScanData::AdjustMeasure(RealDataSet& DataSet, const TwoDimMeasureArray_t& PopMeasure, tract_t Tract, double dRelativeRisk, Julian StartDate, Julian EndDate) {
  const PopulationData & Population = DataSet.getPopulationData();
  measure_t ** pp_m = PopMeasure.GetArray();
  count_t ** ppCases = DataSet.getCaseData().GetArray();
  measure_t ** pNonCumulativeMeasure = DataSet.getMeasureData().GetArray();

  for (int interval=GetTimeIntervalOfDate(StartDate); interval <= GetTimeIntervalOfDate(EndDate); ++interval) {
     Julian AdjustmentStart = std::max(StartDate, gvTimeIntervalStartTimes[interval]);
     Julian AdjustmentEnd = std::min(EndDate, gvTimeIntervalStartTimes[interval+1] - 1);
     //calculate measure for lower interval date to adjustment start date
     measure_t MeasurePre = CalcMeasureForTimeInterval(Population, pp_m, Tract, gvTimeIntervalStartTimes[interval], AdjustmentStart);
     //calculate measure for adjustment period
     measure_t MeasureDuring = CalcMeasureForTimeInterval(Population, pp_m, Tract, AdjustmentStart, AdjustmentEnd+1);
     //calculate measure for adjustment end date to upper interval date
     measure_t MeasurePost = CalcMeasureForTimeInterval(Population, pp_m, Tract, AdjustmentEnd+1, gvTimeIntervalStartTimes[interval+1]);
     //validate that data overflow will not occur
     if (MeasureDuring && (dRelativeRisk > (std::numeric_limits<measure_t>::max() - MeasurePre - MeasurePost) / MeasureDuring))
       GenerateResolvableException("Error: Data overflow occurs when adjusting expected number of cases.\n"
                                   "       The specified relative risk %lf in the adjustment file\n"
                                   "       is too large.\n", "AssignMeasure()", dRelativeRisk);
     //assign adjusted measure                      
     pNonCumulativeMeasure[interval][Tract] = MeasurePre + dRelativeRisk * MeasureDuring + MeasurePost;
     //if measure has been adjusted to zero, check that cases adjusted interval are also zero
     if (pNonCumulativeMeasure[interval][Tract] == 0 && GetCaseCount(ppCases, interval, Tract)) {
       std::string  sStart, sEnd;
       GenerateResolvableException("Error: For locationID '%s', you have adjusted the expected number\n"
                                   "       of cases in the period %s to %s to be zero, but there\n"
                                   "       are cases in that interval.\n"
                                   "       If the expected is zero, the number of cases must also be zero.\n",
                                   "AdjustMeasure()",
                                   (Tract == -1 ? "All" : gTractHandler->getLocations().at(Tract)->getIndentifier()),
                                   JulianToString(sStart, StartDate).c_str(),
                                   JulianToString(sEnd, EndDate).c_str());
       return false;
     }
  }
  return true;
}

/** Iterative analyses will call this function to clear neighbor information and re-calculate neighbors. */
void CSaTScanData::AdjustNeighborCounts(ExecutionType geExecutingType) {
  try {
    bool bDistanceOnlyMax = gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses() &&
                            !gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) &&
                            !gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true);
    //We do not need to recalculate the number of neighbors when the max spatial size restriction is by distance only.
    if (!bDistanceOnlyMax && !gParameters.UseLocationNeighborsFile() && geExecutingType != CENTRICALLY) {
      //Re-calculate neighboring locations about each centroid.
      CentroidNeighborCalculator(*this, gPrint).CalculateNeighbors();
    }
    gvCentroidNeighborStore.DeleteAllElements();
  }
  catch (ZdException &x) {
    x.AddCallpath("AdjustNeighborCounts()", "CSaTScanData");
    throw;
  }
}

/** Allocates three dimensional array that stores tract index for each neighbor
    of each ellipse by grid point combination. Data type of array is conditionally
    either unsigned short or tract_t; based upon number of tracts in coordinates
    file. Note that third dimension is allocated with zero length. Later, when
    neighbors are calculated, the third dimension will be allocated based upon
    the number of calculated neighbors for each (circle/ellipse) and grid point pair.
    Repeated calls to this method cause the third dimension arrays to be re-initialized
    to zero. */
void CSaTScanData::AllocateSortedArray() {
  try {
    if (m_nTracts + (tract_t)(gTractHandler->getMetaLocations().getNumReferencedLocations()) < std::numeric_limits<unsigned short>::max()) {
      if (!gpSortedUShortHandler)
        gpSortedUShortHandler = new ThreeDimensionArrayHandler<unsigned short>(gParameters.GetNumTotalEllipses()+1, m_nGridTracts, 0);
      else
        gpSortedUShortHandler->FreeThirdDimension();
    }
    else {
      if (!gpSortedIntHandler)
        gpSortedIntHandler = new ThreeDimensionArrayHandler<tract_t>(gParameters.GetNumTotalEllipses()+1, m_nGridTracts, 0);
      else
        gpSortedIntHandler->FreeThirdDimension();
    }
    //allocates two-dimensional array that will track the number of neighbors for each shape/grid point combination.
    if (gParameters.GetRestrictingMaximumReportedGeoClusterSize()) {
      if (!gpReportedNeighborCountHandler)
        gpReportedNeighborCountHandler = new TwoDimensionArrayHandler<tract_t>(gParameters.GetNumTotalEllipses() + 1, m_nGridTracts);
      gpReportedNeighborCountHandler->Set(0);
    }
    if (!gpNeighborCountHandler)
      gpNeighborCountHandler = new TwoDimensionArrayHandler<tract_t>(gParameters.GetNumTotalEllipses() + 1, m_nGridTracts);
    gpNeighborCountHandler->Set(0);
    //default neighbor reference type to MAXIMUM, if not set
    SetActiveNeighborReferenceType(geActiveNeighborReferenceType == NOT_SET ? MAXIMUM : geActiveNeighborReferenceType);
  }
  catch (ZdException &x) {
    delete gpSortedUShortHandler; gpSortedUShortHandler=0;
    delete gpSortedIntHandler; gpSortedIntHandler=0;
    delete gpNeighborCountHandler; gpNeighborCountHandler=0;
    delete gpReportedNeighborCountHandler; gpReportedNeighborCountHandler=0;
    x.AddCallpath("AllocateSortedArray()","CSaTScanData");
    throw;
  }
}

/** Set the number of neighbors about ellipse/circle at centroid index. Care must be taken
    when calling this function since it sets the variables which detail the size of the sorted arrays.*/
void CSaTScanData::setNeighborCounts(int iEllipseIndex, tract_t iCentroidIndex, tract_t iNumReportedNeighbors, tract_t iNumMaximumNeighbors) {
  //update neighbor array(s) for number of calculated neighbors
  if (!gpNeighborCountHandler)
    ZdGenerateException("Neighbor array not allocated.","setNeighborCounts()");

  if (gpReportedNeighborCountHandler)
    gpReportedNeighborCountHandler->GetArray()[iEllipseIndex][iCentroidIndex] = iNumReportedNeighbors;
  gpNeighborCountHandler->GetArray()[iEllipseIndex][iCentroidIndex] = iNumMaximumNeighbors;
}

/** Allocates third dimension of sorted at 'array[iEllipseIndex][iCentroidIndex]'
    to length specifed by variable 'iNumMaximumNeighbors'; assigning locations indexes
    as detailed by 'vOrderLocations' variable. Sets multi-dimension arrays which detail
    the number actual and reported neighbors defined in just allocated sorted array. */
void CSaTScanData::AllocateSortedArrayNeighbors(const std::vector<LocationDistance>& vOrderLocations,
                                                int iEllipseIndex, tract_t iCentroidIndex,
                                                tract_t iNumReportedNeighbors, tract_t iNumMaximumNeighbors) {
  try {
    if (gpSortedUShortHandler) {
      delete[] gpSortedUShortHandler->GetArray()[iEllipseIndex][iCentroidIndex];
      gpSortedUShortHandler->GetArray()[iEllipseIndex][iCentroidIndex]=0;
      gpSortedUShortHandler->GetArray()[iEllipseIndex][iCentroidIndex] = new unsigned short[iNumMaximumNeighbors];
      for (tract_t j=iNumMaximumNeighbors-1; j >= 0; j--) /* copy tract numbers */
         gpSortedUShortHandler->GetArray()[iEllipseIndex][iCentroidIndex][j] = static_cast<unsigned short>(vOrderLocations[j].GetTractNumber());
    }
    else if (gpSortedIntHandler) {
      delete[] gpSortedIntHandler->GetArray()[iEllipseIndex][iCentroidIndex];
      gpSortedIntHandler->GetArray()[iEllipseIndex][iCentroidIndex]=0;
      gpSortedIntHandler->GetArray()[iEllipseIndex][iCentroidIndex] = new tract_t[iNumMaximumNeighbors];
      for (tract_t j=iNumMaximumNeighbors-1; j >= 0; j--) /* copy tract numbers */
         gpSortedIntHandler->GetArray()[iEllipseIndex][iCentroidIndex][j] = vOrderLocations[j].GetTractNumber();
    }
    else
      ZdGenerateException("Sorted array not allocated.","AllocateSortedArrayNeighbors()");

    //update neighbor array(s) for number of calculated neighbors
    setNeighborCounts(iEllipseIndex, iCentroidIndex, iNumReportedNeighbors, iNumMaximumNeighbors);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSortedArrayNeighbors()","CSaTScanData");
    throw;
  }
}

/** Allocates third dimension of sorted at 'array[0][*(vLocations.begin())]'
    to length of vLocations vector; assigning locations indexes as detailed by
    vLocations variable. Sets multi-dimension arrays which detail the size of array. */
void CSaTScanData::AllocateSortedArrayNeighbors(tract_t iCentroidIndex, const std::vector<tract_t>& vLocations) {
  
  try {
    if (gpSortedUShortHandler) {
      delete[] gpSortedUShortHandler->GetArray()[0][iCentroidIndex];
      gpSortedUShortHandler->GetArray()[0][iCentroidIndex]=0;
      gpSortedUShortHandler->GetArray()[0][iCentroidIndex] = new unsigned short[vLocations.size()];
      for (tract_t j=(int)vLocations.size()-1; j >= 0; j--) /* copy tract numbers */
         gpSortedUShortHandler->GetArray()[0][iCentroidIndex][j] = static_cast<unsigned short>(vLocations[j]);
    }
    else if (gpSortedIntHandler) {
      delete[] gpSortedIntHandler->GetArray()[0][iCentroidIndex];
      gpSortedIntHandler->GetArray()[0][iCentroidIndex]=0;
      gpSortedIntHandler->GetArray()[0][iCentroidIndex] = new tract_t[vLocations.size()];
      for (tract_t j=(int)vLocations.size()-1; j >= 0; j--) /* copy tract numbers */
         gpSortedIntHandler->GetArray()[0][iCentroidIndex][j] = vLocations[j];
    }
    else
      ZdGenerateException("Sorted array not allocated.","AllocateSortedArrayNeighbors()");

    //update neighbor array(s) for number of calculated neighbors
    setNeighborCounts(0, iCentroidIndex, 0, vLocations.size());
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSortedArrayNeighbors()","CSaTScanData");
    throw;
  }
}

/**************************************************************************************
Calculates measure M for a requested tract and time interval.
Input: Tract, Time Interval, # Pop Points, Measure array for population points
       StudyStartDate, StudyEndDate
Time Interval = [StartDate , EndDate]; EndDate = NextStartDate-1
Note: The measure 'M' is the same measure used for the population points, which is later
calibrated before being put into the measure array.
**************************************************************************************/
measure_t CSaTScanData::CalcMeasureForTimeInterval(const PopulationData & Population, measure_t ** ppPopulationMeasure, tract_t Tract, Julian StartDate, Julian NextStartDate) const {
  int           i, iStartUpperIndex, iNextLowerIndex;
  long          nTotalDays = m_nEndDate+1 - m_nStartDate;
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

/** Calculates expected number of cases for each dataset. Records total
    measure, cases, and population for all datasets. Calls method to determines
    the maximum spatial cluster size. */
void CSaTScanData::CalculateExpectedCases() {
  gPrint.Printf("Calculating the expected number of cases\n", BasePrint::P_STDOUT);
  //calculates expected cases for each dataset
  for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
     CalculateMeasure(gDataSets->GetDataSet(t));
     gtTotalMeasure += gDataSets->GetDataSet(t).getTotalMeasure();
     gtTotalMeasureSq += gDataSets->GetDataSet(t).getTotalMeasureSq();
     gtTotalCases += gDataSets->GetDataSet(t).getTotalCases();
     gtTotalPopulation += gDataSets->GetDataSet(t).getTotalPopulation();
  }
  FreeRelativeRisksAdjustments();
}

/** Calculates expected number of cases for dataset. */
void CSaTScanData::CalculateMeasure(RealDataSet& DataSet) {
  try {
    m_pModel->CalculateMeasure(DataSet);
    //record totals at start, the optional iterative scan feature modifies start values
    DataSet.setTotalCasesAtStart(DataSet.getTotalCases());
    DataSet.setTotalMeasureAtStart(DataSet.getTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CSaTScanData");
    throw;
  }
}

/** Calculates time interval indexes given study period and other time aggregation
    settings. */
void CSaTScanData::CalculateTimeIntervalIndexes() {
  int           iNumCollapsibleIntervals;
  double        dProspectivePeriodLength, dMaxTemporalLengthInUnits;

  // calculate time interval start time indexes
  SetIntervalStartTimes();
  // calculate maximum temporal cluster size, in terms of interval indexes
  SetIntervalCut();
  // calculate date indexes for flexible scanning window
  SetTimeIntervalRangeIndexes();
  // calculate date index for prospective surveillance start date
  if (gParameters.GetIsProspectiveAnalysis()) {
    m_nProspectiveIntervalStart = CalculateProspectiveIntervalStart();
    // If analysis performs simulations and adjusts for earlier analyses, then we can potentially
    // collapse unused time intervals into one based upon the prospective start date and maximum
    // temporal cluster size.
    if (gParameters.GetNumReplicationsRequested() > 0 && gParameters.GetAdjustForEarlierAnalyses()) {
      // For prospective analyses, not all time intervals may be evaluated; consequently some of the
      // initial intervals can be combined into one interval. When evaluating real data, we will only
      // consider 'alive' clusters (clusters where the end date range equals the study period end date). For
      // the simulated data, we will consider historical clusters from prospective start date.
      if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
        // If the maximum temporal cluster size is defined as a percentage of the population at risk,
        // then the maximum cluster size must be calculated for each prospective period.
        for (int iWindowEnd=m_nProspectiveIntervalStart; iWindowEnd <= m_nTimeIntervals; ++iWindowEnd) {
           dProspectivePeriodLength = CalculateNumberOfTimeIntervals(m_nStartDate, gvTimeIntervalStartTimes[iWindowEnd] - 1,
                                                                     gParameters.GetTimeAggregationUnitsType(), 1);
           dMaxTemporalLengthInUnits = floor(dProspectivePeriodLength * gParameters.GetMaximumTemporalClusterSize()/100.0);
           //now calculate number of those time units a cluster can contain with respects to the specified aggregation length
           gvProspectiveIntervalCuts.push_back(static_cast<int>(floor(dMaxTemporalLengthInUnits / gParameters.GetTimeAggregationLength())));
        }
        // Now we know the index of the earliest accessed time interval, we can determine the
        // number of time intervals that can be collapsed.
        iNumCollapsibleIntervals = m_nProspectiveIntervalStart - *(gvProspectiveIntervalCuts.begin());
      }
      else
        // When the maximum temporal cluster size is a fixed period, the number of intervals
        // to collapse is simplier to calculate.
        iNumCollapsibleIntervals = m_nProspectiveIntervalStart - m_nIntervalCut;
    }
    else
      // Else we are either not performing simulations, and therefore not evaluating prospective clusters,
      // or we are not adjusting for previous analyses, and therefore only evaluating 'alive' clusters
      // in both real data and simulated data.
      iNumCollapsibleIntervals = m_nTimeIntervals - m_nIntervalCut;

    // If iNumCollapsedIntervals is at least two, them collapse intervals. The reason we don't collapse when
    // iNumCollapsedIntervals is one is because iNumCollapsedIntervals does not take into account the
    // first time interval, which will be the bucket for the collapsed intervals.
    if (iNumCollapsibleIntervals > 1) {
      // Removes collaped intervals from the data structure which details time interval start times.
      // When input data is read, what would have gone into the respective second interval, third, etc.
      // will be cummulated into first interval.
      gvTimeIntervalStartTimes.erase(gvTimeIntervalStartTimes.begin() + 1, gvTimeIntervalStartTimes.begin() + iNumCollapsibleIntervals);
      // Re-calculate number of time intervals.
      m_nTimeIntervals = gvTimeIntervalStartTimes.size() - 1;
      // Re-calculate index of prospective start date
      m_nProspectiveIntervalStart = CalculateProspectiveIntervalStart();
    }  
  }
}

/********************************************************************
  Finds the measure M for a requested tract and date.
  Input: Date, Tracts, #PopPoints
 ********************************************************************/
measure_t CSaTScanData::DateMeasure(const PopulationData & Population, measure_t ** ppPopulationMeasure, Julian Date, tract_t Tract) const {
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

/** Allocates/deallocates memory to store neighbor information.
    Calls MakeNeighbor() function to calculate neighbors for each centroid. */
void CSaTScanData::FindNeighbors() {
  try {
    CentroidNeighborCalculator(*this, gPrint).CalculateNeighbors();
  }
  catch (ZdException &x) {
    x.AddCallpath("FindNeighbors()","CSaTScanData");
    throw;
  }
}

double CSaTScanData::GetAnnualRate(size_t tSetIndex) const {
  double nYears      = (double)(m_nEndDate+1 - m_nStartDate) / 365.2425;
  double dTotalCases = gDataSets->GetDataSet(tSetIndex).getTotalCases();
  double dTotalPopulation = gDataSets->GetDataSet(tSetIndex).getTotalPopulation();
  double nAnnualRate = (m_nAnnualRatePop*dTotalCases) / (dTotalPopulation*nYears);

  return nAnnualRate;
}

double CSaTScanData::GetAnnualRateAtStart(size_t tSetIndex) const {
  double nYears      = (double)(m_nEndDate+1 - m_nStartDate) / 365.2425;
  double dTotalCasesAtStart = gDataSets->GetDataSet(tSetIndex).getTotalCasesAtStart();
  double dTotalPopulation = gDataSets->GetDataSet(tSetIndex).getTotalPopulation();
  double nAnnualRate = (m_nAnnualRatePop*dTotalCasesAtStart) / (dTotalPopulation*nYears);

  return nAnnualRate;
}

/** Returns the number of cases for a specified tract and time interval.
    Note: iInterval and tTract should be valid indexes of the cases array .**/
count_t CSaTScanData::GetCaseCount(count_t ** ppCumulativeCases, int iInterval, tract_t tTract) const {
  if (iInterval + 1 == m_nTimeIntervals)
    return ppCumulativeCases[iInterval][tTract];
  else
    return ppCumulativeCases[iInterval][tTract] - ppCumulativeCases[iInterval + 1][tTract];
}

/** Returns angle to referenced ellipse at index. Caller is responsible for ensuring
    that iEllipseIndex is within validate range where if ellipses where requested,
    the valid range is 0 to gParameters.GetNumTotalEllipses(). */
double CSaTScanData::GetEllipseAngle(int iEllipseIndex) const {
  return (iEllipseIndex == 0 ? 0.0 : gvEllipseAngles[iEllipseIndex - 1]);
}

/** Returns shape to referenced ellipse at index. Caller is responsible for ensuring
    that iEllipseIndex is within validate range where if ellipses where requested,
    the valid range is 0 to gParameters.GetNumTotalEllipses(). */
double CSaTScanData::GetEllipseShape(int iEllipseIndex) const {
  return (iEllipseIndex == 0 ? 1.0 : gvEllipseShapes[iEllipseIndex - 1]);
}

/** Returns whether data of location at index has been removed as a result of
    being part of most likely cluster in a iterative scan.*/
bool CSaTScanData::GetIsNullifiedLocation(tract_t tLocationIndex) const {
   return std::find(gvNullifiedLocations.begin(), gvNullifiedLocations.end(), tLocationIndex) != gvNullifiedLocations.end();
}

/** For Bernoulli model, returns ratio of total cases / total population for
    iDataSet'th dataset. For all other models, returns 1.*/
double CSaTScanData::GetMeasureAdjustment(size_t tSetIndex) const {
  if (gParameters.GetProbabilityModelType() == BERNOULLI) {
    double dTotalCases = gDataSets->GetDataSet(tSetIndex).getTotalCases();
    double dTotalPopulation = gDataSets->GetDataSet(tSetIndex).getTotalPopulation();
    return dTotalCases / dTotalPopulation;
  }
  else
    return 1.0;
}

/** Input: Date.                                                    **/
/** Returns: Index of the time interval to which the date belongs.   **/
/** If Date does not belong to any time interval, -1 is returned. **/
/** Note: First time interval has index 0.                          **/
int CSaTScanData::GetTimeIntervalOfDate(Julian Date) const {
  int   i=0;

  //check that date is within study period
  if (Date < gvTimeIntervalStartTimes[0] || Date >= gvTimeIntervalStartTimes[m_nTimeIntervals])
    return -1;

  while (Date >=  gvTimeIntervalStartTimes[i+1])
       ++i;

  return i;
}

/** Gets time interval index into interval start times array for end date. */
int CSaTScanData::GetTimeIntervalOfEndDate(Julian EndDate) const {
  int   i, iDateIndex = -1;

  //find index for end date, the interval beyond where date fits
  for (i=m_nTimeIntervals; i > 0  && iDateIndex == -1; --i)
     if (EndDate <= gvTimeIntervalStartTimes[i] - 1 && EndDate > gvTimeIntervalStartTimes[i - 1] - 1)
        iDateIndex = i;
        
  return iDateIndex;
}

/** internal class initializaton */
void CSaTScanData::Init() {
  m_pModel = 0;
  geActiveNeighborReferenceType=NOT_SET;
  gppActiveNeighborArray=0;
  gpNeighborCountHandler=0;
  gpReportedNeighborCountHandler=0;
  gpSortedIntHandler=0;
  gpSortedUShortHandler=0;
  m_nAnnualRatePop = 100000;
  m_nTotalMaxCirclePopulation = 0;
  gtTotalMeasure=0;
  gtTotalCases=0;
  gtTotalPopulation=0;
  m_nFlexibleWindowStartRangeStartIndex=0;
  m_nFlexibleWindowStartRangeEndIndex=0;
  m_nFlexibleWindowEndRangeStartIndex=0;
  m_nFlexibleWindowEndRangeEndIndex=0;
  m_nGridTracts = 0;
  gtTotalMeasureSq = 0;
}

/** Randomizes collection of simulation data in concert with passed collection
    of randomizers. This method for creating randomized data is primarily
    utilized in conjunction with threaded simulations. */
/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CSaTScanData::RandomizeData(RandomizerContainer_t& RandomizerContainer,
                                 SimulationDataContainer_t& SimDataContainer,
                                 unsigned int iSimulationNumber) const {
  try {
    gDataSets->RandomizeData(RandomizerContainer, SimDataContainer, iSimulationNumber);
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeData()","CSaTScanData");
    throw;
  }
}

/** First calls internal methods to prepare internal structure:
    - calculate time interval start times
    - calculate a clusters maximum temporal window length in terms of time intervals
    - calculate indexes of flexible scanning window, if requested
    - calculate start interval index of start date of prospective analyses
    - read input data from files base upon probability model
    Throws ResolvableException if read fails. */
void CSaTScanData::ReadDataFromFiles() {
  try {
    SaTScanDataReader(*this).Read();
    if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION || gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC)
      std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setCaseData_PT_NC));
    CalculateExpectedCases();
    if (gParameters.UseMetaLocationsFile())
      gDataSets->assignMetaLocationData(gDataSets->getDataSets());
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadDataFromFiles()","CSaTScanData");
    throw;
  }
}

/** Removes all cases/controls/measure from data sets, geographically and temporally, for passed cluster object. */
void CSaTScanData::RemoveClusterSignificance(const CCluster& Cluster) {
  tract_t               tStopTract(0), tTractIndex(-1), iNeighborIndex=0;
  std::vector<tract_t>  atomicIndexes;

  try {
    if (Cluster.GetClusterType() == SPACETIMECLUSTER)
      ZdGenerateException("RemoveClusterSignificance() not implemented for cluster type %d.", "RemoveClusterSignificance()", Cluster.GetClusterType());

    tStopTract = (Cluster.GetClusterType() == PURELYTEMPORALCLUSTER ? m_nTracts - 1 : GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), Cluster.GetNumTractsInCluster()));
    while (tTractIndex != tStopTract) {
       tTractIndex = (Cluster.GetClusterType() == PURELYTEMPORALCLUSTER ? tTractIndex + 1 : GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), ++iNeighborIndex));
       // Previous iterations of iterative scan could have had this location as part of the most likely cluster.
       if ((Cluster.GetClusterType() == PURELYSPATIALCLUSTER || Cluster.GetClusterType() == PURELYSPATIALMONOTONECLUSTER || Cluster.GetClusterType() == SPATIALVARTEMPTRENDCLUSTER)
           && GetIsNullifiedLocation(tTractIndex))
         continue;
       if (tTractIndex < m_nTracts)
         RemoveTractSignificance(Cluster, tTractIndex);
       else {//tract is a meta location
         gTractHandler->getMetaLocations().getAtomicIndexes(tTractIndex - m_nTracts, atomicIndexes);
         for (size_t a=0; a < atomicIndexes.size(); ++a) {
           if (!GetIsNullifiedLocation(atomicIndexes[a]))
             RemoveTractSignificance(Cluster, atomicIndexes[a]);
         }
       }
    }

    //now update data sets as needed, given all cluster data has now been removed
    if (gParameters.GetProbabilityModelType() == POISSON) {
      //recalibrate the measure array to equal expected cases
      gtTotalMeasure = 0;
      for (size_t d=0; d < gDataSets->GetNumDataSets(); ++d) {
         RealDataSet& DataSet = gDataSets->GetDataSet(d);
         measure_t tAdjustedTotalMeasure=0;
         measure_t tCalibration = (measure_t)(DataSet.getTotalCases())/(DataSet.getTotalMeasure());
         measure_t ** ppMeasure = DataSet.getMeasureData().GetArray();
         for (int i=0; i < m_nTimeIntervals-1; ++i) for (tract_t t=0; t < m_nTracts; ++t) ppMeasure[i][t] = (ppMeasure[i][t] - ppMeasure[i+1][t]) * tCalibration;
         for (tract_t t=0; t < m_nTracts; ++t) ppMeasure[m_nTimeIntervals - 1][t] *= tCalibration;
         DataSet.setMeasureDataToCumulative();
         for (tract_t t=0; t < m_nTracts; ++t) tAdjustedTotalMeasure += ppMeasure[0][t];
         gDataSets->GetDataSet(d).setTotalMeasure(tAdjustedTotalMeasure);
         gtTotalMeasure += tAdjustedTotalMeasure;
      }
    }
    if (gParameters.GetProbabilityModelType() == EXPONENTIAL) {
      //recalibrate the measure to equal expected cases
      AbstractExponentialRandomizer *pRandomizer;
      gtTotalCases = 0;
      gtTotalMeasure = 0;
      for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
         RealDataSet& DataSet = gDataSets->GetDataSet(t);
         if ((pRandomizer = dynamic_cast<AbstractExponentialRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
           ZdGenerateException("Randomizer could not be dynamically casted to AbstractExponentialRandomizer type.\n", "RemoveClusterSignificance()");
         pRandomizer->AssignFromAttributes(DataSet);
         gtTotalCases += DataSet.getTotalCases();
         gtTotalMeasure += DataSet.getTotalMeasure();
      }
    }
    if (gParameters.GetProbabilityModelType() == NORMAL) {
      //recalculate the data set cases/measure given updated randomizer data
      AbstractNormalRandomizer *pRandomizer;
      gtTotalCases=0;
      gtTotalMeasure=gtTotalMeasureSq=0;
      for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
        RealDataSet& DataSet = gDataSets->GetDataSet(t);
        if ((pRandomizer = dynamic_cast<AbstractNormalRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
          ZdGenerateException("Randomizer could not be dynamically casted to AbstractNormalRandomizer type.\n", "RemoveClusterSignificance()");
        pRandomizer->AssignFromAttributes(DataSet);
        //update class variables that defines totals across all data sets
        gtTotalCases += DataSet.getTotalCases();
        gtTotalMeasure += DataSet.getTotalMeasure();
        gtTotalMeasureSq += DataSet.getTotalMeasureSq();
      }
    }
    //now recalculate purely temporal arrays as needed
    if (gParameters.GetIncludePurelyTemporalClusters() || gParameters.GetIsPurelyTemporalAnalysis()) {
      switch (gParameters.GetProbabilityModelType()) {
       case NORMAL      : std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_PT_Sq));
       case EXPONENTIAL :
       case BERNOULLI   :
       case POISSON     : std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setCaseData_PT));
                          std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_PT)); break;
       case ORDINAL     : std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setCaseData_PT_Cat)); break;
       default : ZdGenerateException("Unknown probability %d model.", "RemoveClusterSignificance()", gParameters.GetProbabilityModelType());
      }
    }
    //now recalculate meta data as needed
    if (gParameters.UseMetaLocationsFile())
      gDataSets->assignMetaLocationData(gDataSets->getDataSets());
  }
  catch (ZdException & x) {
    x.AddCallpath("RemoveClusterSignificance()", "CSaTScanData");
    throw;
  }
}

/** Removes all cases/controls/measure from data sets, geographically and temporally, for
    location at tTractIndex in specified interval range. */
void CSaTScanData::RemoveTractSignificance(const CCluster& Cluster, tract_t tTractIndex) {
  count_t       tCasesInInterval, ** ppCases;
  measure_t     tCalibration, tAdjustedTotalMeasure, tMeasureInInterval, ** ppMeasure , ** ppSqMeasure, tSqMeasureInInterval;

       if (gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI) {
         gtTotalCases = 0;
         gtTotalMeasure = 0;
         if (gParameters.GetProbabilityModelType() == BERNOULLI) gtTotalPopulation = 0;
         for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
           RealDataSet& DataSet = gDataSets->GetDataSet(t);
           ppCases = DataSet.getCaseData().GetArray();
           ppMeasure = DataSet.getMeasureData().GetArray();
           //get cases/measure in earliest interval - we'll need to remove these from intervals earlier than cluster window
           tCasesInInterval = ppCases[Cluster.m_nFirstInterval][tTractIndex] - (Cluster.m_nLastInterval == m_nTimeIntervals ? 0 : ppCases[Cluster.m_nLastInterval][tTractIndex]);
           tMeasureInInterval = ppMeasure[Cluster.m_nFirstInterval][tTractIndex] - (Cluster.m_nLastInterval == m_nTimeIntervals ? 0 : ppMeasure[Cluster.m_nLastInterval][tTractIndex]);
           //zero out cases/measure in clusters defined temporal window
           for (int i=Cluster.m_nFirstInterval; i < Cluster.m_nLastInterval; ++i) {
             ppCases[i][tTractIndex] = (Cluster.m_nLastInterval == m_nTimeIntervals ? 0 : ppCases[Cluster.m_nLastInterval][tTractIndex]);
             ppMeasure[i][tTractIndex] = (Cluster.m_nLastInterval == m_nTimeIntervals ? 0 : ppMeasure[Cluster.m_nLastInterval][tTractIndex]);
           }
           //remove cases/measure from earlier intervals
           for (int i=0; i < Cluster.m_nFirstInterval; ++i) {
             ppCases[i][tTractIndex] -= tCasesInInterval;
             ppMeasure[i][tTractIndex] -= tMeasureInInterval;
           }
           //update totals for data set
           DataSet.setTotalCases(DataSet.getTotalCases() - tCasesInInterval);
           DataSet.setTotalMeasure(DataSet.getTotalMeasure() - tMeasureInInterval);
           if (gParameters.GetProbabilityModelType() == BERNOULLI) DataSet.setTotalControls(DataSet.getTotalControls() - static_cast<count_t>(tMeasureInInterval - tCasesInInterval));
           if (gParameters.GetProbabilityModelType() == BERNOULLI) DataSet.setTotalPopulation(DataSet.getTotalPopulation() - tMeasureInInterval);
           //update class variables that defines totals across all data sets
           gtTotalCases += DataSet.getTotalCases();
           gtTotalMeasure += DataSet.getTotalMeasure();
           if (gParameters.GetProbabilityModelType() == BERNOULLI) gtTotalPopulation += DataSet.getTotalPopulation();
         }
       }
       else if (gParameters.GetProbabilityModelType() == ORDINAL) {
         gtTotalCases = 0;
         gtTotalPopulation = 0;
         for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
           RealDataSet& DataSet = gDataSets->GetDataSet(t);
           PopulationData& thisPopulation = DataSet.getPopulationData();
           // Remove observed cases for location from data set ordinal categories
           for (size_t c=0; c < DataSet.getCaseData_Cat().size(); ++c) {
             ppCases = DataSet.getCategoryCaseData(c).GetArray();
             //get cases in earliest interval - we'll need to remove these from intervals earlier than cluster window
             tCasesInInterval = ppCases[Cluster.m_nFirstInterval][tTractIndex] - (Cluster.m_nLastInterval == m_nTimeIntervals ? 0 : ppCases[Cluster.m_nLastInterval][tTractIndex]);
             //zero out cases/measure in clusters defined temporal window
             for (int i=Cluster.m_nFirstInterval; i < Cluster.m_nLastInterval; ++i)
               ppCases[i][tTractIndex] = (Cluster.m_nLastInterval == m_nTimeIntervals ? 0 : ppCases[Cluster.m_nLastInterval][tTractIndex]);
             //remove cases/measure from earlier intervals
             for (int i=0; i < Cluster.m_nFirstInterval; ++i)
               ppCases[i][tTractIndex] -= tCasesInInterval;
             //update category population
             thisPopulation.RemoveOrdinalCategoryCases(c, tCasesInInterval);
             //update totals for data set
             DataSet.setTotalCases(DataSet.getTotalCases() - tCasesInInterval);
             DataSet.setTotalPopulation(DataSet.getTotalPopulation() - tCasesInInterval);
           }
           //update class variables that defines totals across all data sets
           gtTotalCases += DataSet.getTotalCases();
           gtTotalPopulation += DataSet.getTotalCases();
         }
       }
       else if (gParameters.GetProbabilityModelType() == NORMAL) {
         AbstractNormalRandomizer *pRandomizer;
         for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
           if ((pRandomizer = dynamic_cast<AbstractNormalRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
             ZdGenerateException("Randomizer could not be dynamically casted to AbstractNormalRandomizer type.\n", "RemoveClusterSignificance()");
           //zero out cases/measure in clusters defined spatial/temporal window
           for (int i=Cluster.m_nFirstInterval; i < Cluster.m_nLastInterval; ++i)
             pRandomizer->RemoveCase(i, tTractIndex);
         }
       }
       else if (gParameters.GetProbabilityModelType() == EXPONENTIAL) {
         AbstractExponentialRandomizer *pRandomizer;
         for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
           RealDataSet& DataSet = gDataSets->GetDataSet(t);
           if ((pRandomizer = dynamic_cast<AbstractExponentialRandomizer*>(gDataSets->GetRandomizer(t))) == 0)
             ZdGenerateException("Randomizer could not be dynamically casted to AbstractExponentialRandomizer type.\n", "RemoveClusterSignificance()");
           //zero out cases/measure in clusters defined spatial/temporal window
           for (int i=Cluster.m_nFirstInterval; i < Cluster.m_nLastInterval; ++i)
             pRandomizer->RemoveCase(i, tTractIndex);
         }
       }
       else
         ZdGenerateException("RemoveClusterSignificance() not implemented for %s model.",
                               "RemoveClusterSignificance()", ParametersPrint(gParameters).GetProbabilityModelTypeAsString());
       // Remove location population data as specified in maximum circle population file
       if (gvMaxCirclePopulation.size()) {
         m_nTotalMaxCirclePopulation -= gvMaxCirclePopulation[tTractIndex];
         gvMaxCirclePopulation[tTractIndex] = 0;
       }
       // Add location to collection of nullified locations - note that we're just removing locations' data, not the location.
       if ((Cluster.GetClusterType() == SPATIALVARTEMPTRENDCLUSTER ||
            Cluster.GetClusterType() == PURELYSPATIALCLUSTER ||
            Cluster.GetClusterType() == PURELYSPATIALMONOTONECLUSTER) && !GetIsNullifiedLocation(tTractIndex))
         gvNullifiedLocations.push_back(tTractIndex);
}

/** Set neighbor array pointer requested type. */
void CSaTScanData::SetActiveNeighborReferenceType(ActiveNeighborReferenceType eType) {
  try {
    if (gParameters.GetIsPurelyTemporalAnalysis())
      return;

    switch (eType) {
      case REPORTED : if (gpReportedNeighborCountHandler) {
                        gppActiveNeighborArray = gpReportedNeighborCountHandler->GetArray();
                        break;
                      }
      case MAXIMUM  : if (!gpNeighborCountHandler)
                        ZdGenerateException("Neighbor array not allocated.","SetActiveNeighborReferenceType()");
                      gppActiveNeighborArray = gpNeighborCountHandler->GetArray();
                      break;
      case NOT_SET  : gppActiveNeighborArray=0; break;                
      default       :
        ZdGenerateException("Unknown active neighbor type '%d'.", "SetActiveNeighborReferenceType()", eType);
    };
    geActiveNeighborReferenceType = eType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetActiveNeighborReferenceType()","CSaTScanData");
    throw;
  }
}

/* Calculates the number of time aggregation units to include in potential clusters
   without exceeding the maximum temporal cluster size.*/
void CSaTScanData::SetIntervalCut() {
  double        dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

  try {
    if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
      //calculate the number of time interval units which comprise the maximum
      //temporal cluster size in the study period
      dStudyPeriodLengthInUnits = CalculateNumberOfTimeIntervals(m_nStartDate, m_nEndDate, gParameters.GetTimeAggregationUnitsType(), 1);
      dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * gParameters.GetMaximumTemporalClusterSize()/100.0);
      //now calculate number of those time units a cluster can contain with respects to the specified aggregation length
      m_nIntervalCut = static_cast<int>(floor(dMaxTemporalLengthInUnits / gParameters.GetTimeAggregationLength()));
    }
    else
      m_nIntervalCut = static_cast<int>(gParameters.GetMaximumTemporalClusterSize() / gParameters.GetTimeAggregationLength());

    if (m_nIntervalCut==0)
      //Validation in CParameters should have produced error before this body of code.
      //Only a program error or user selecting not to validate parameters should cause
      //this error to occur.
      ZdException::Generate("The calculated number of time aggregations units in potential clusters is zero.","SetIntervalCut()");
   }
  catch (ZdException &x) {
    x.AddCallpath("SetIntervalCut()","CSaTScanData");
    throw;
  }
}

/** Calculates the time interval start times given study period and time interval
    length. Start times are calculated from the study period end date backwards,
    which means that first time interval could possibly not be the requested time
    interval length. Throws ResolvableException if the time stratified time trend
    adjustment was requested and the number of calculated time intervals is one.*/
void CSaTScanData::SetIntervalStartTimes() {
  Julian                IntervalStartingDate = m_nEndDate+1;
  DecrementableEndDate  DecrementingDate(m_nEndDate, gParameters.GetTimeAggregationUnitsType());

  gvTimeIntervalStartTimes.clear();
  //latest interval start time is the day after study period end date
  gvTimeIntervalStartTimes.push_back(IntervalStartingDate);
  IntervalStartingDate = DecrementingDate.Decrement(gParameters.GetTimeAggregationLength());
  while (IntervalStartingDate > m_nStartDate) {
      //push interval start time onto vector
      gvTimeIntervalStartTimes.push_back(IntervalStartingDate);
      //find the next prior interval start time from current, given length of time intervals
      IntervalStartingDate = DecrementingDate.Decrement(gParameters.GetTimeAggregationLength());
  }
  //push study period start date onto vector
  gvTimeIntervalStartTimes.push_back(m_nStartDate);
  //reverse elements of vector so that elements are ordered: study period start --> 'study period end + 1'
  std::reverse(gvTimeIntervalStartTimes.begin(), gvTimeIntervalStartTimes.end());
  //record number of time intervals, not including 'study period end date + 1' date
  m_nTimeIntervals = (int)gvTimeIntervalStartTimes.size() - 1;

//  PrintJulianDates(gvTimeIntervalStartTimes, "c:\\StartDates.txt");

  if (m_nTimeIntervals <= 1)
    //This error should be catch in the CParameters validation process.
    ZdException::Generate("The number of time intervals was calculated as one. Temporal\n"
                          "and space-time analyses can not be performed on less than one\n"
                          "time interval.\n", "SetIntervalStartTimes()");
}

/* Calculates which time interval the prospective surveillance start date is in.*/
int CSaTScanData::CalculateProspectiveIntervalStart() const {
  int   iDateIndex = m_nTimeIntervals;

  try {
  	if (gParameters.GetAdjustForEarlierAnalyses()) {
      iDateIndex = GetTimeIntervalOfEndDate(CharToJulian(gParameters.GetProspectiveStartDate().c_str()));

      if (iDateIndex < 0)
        GenerateResolvableException("Error: : The start date for prospective analyses '%s' is prior to the study period start date '%s'.\n",
                                    "SetProspectiveIntervalStart()", gParameters.GetProspectiveStartDate().c_str(),
                                    gParameters.GetStudyPeriodStartDate().c_str());
      if (iDateIndex > m_nTimeIntervals)
        GenerateResolvableException("Error: The start date for prospective analyses '%s' occurs after the study period end date '%s'.\n",
                                    "SetProspectiveIntervalStart", gParameters.GetProspectiveStartDate().c_str(),
                                    gParameters.GetStudyPeriodEndDate().c_str());
    }                                
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateProspectiveIntervalStart()","CSaTScanData");
    throw;
  }
  return iDateIndex;
}

/** For all datasets, causes temporal structures to be allocated and set. */
void CSaTScanData::SetPurelyTemporalCases() {
  size_t        t;

  try {
    if (gParameters.GetProbabilityModelType() == ORDINAL)
      for (t=0; t < gDataSets->GetNumDataSets(); ++t)
        gDataSets->GetDataSet(t).setCaseData_PT_Cat();
    else
      for (t=0; t < gDataSets->GetNumDataSets(); ++t)
        gDataSets->GetDataSet(t).setCaseData_PT();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalCases()","CSaTScanData");
    throw;
  }
}

/** Sets indexes of time interval ranges into interval start time array. */
void CSaTScanData::SetTimeIntervalRangeIndexes() {
  std::string   sTimeIntervalType, sMessage, sDateWST, sDateMaxWET; 
  int           iMaxEndWindow, iWindowStart;

  if (gParameters.GetIncludeClustersType() == CLUSTERSINRANGE) {
    //find start range date indexes
    m_nFlexibleWindowStartRangeStartIndex = GetTimeIntervalOfDate(CharToJulian(gParameters.GetStartRangeStartDate().c_str()));
    m_nFlexibleWindowStartRangeEndIndex = GetTimeIntervalOfDate(CharToJulian(gParameters.GetStartRangeEndDate().c_str()));
    //find end range date indexes
    m_nFlexibleWindowEndRangeStartIndex = GetTimeIntervalOfEndDate(CharToJulian(gParameters.GetEndRangeStartDate().c_str()));
    m_nFlexibleWindowEndRangeEndIndex = GetTimeIntervalOfEndDate(CharToJulian(gParameters.GetEndRangeEndDate().c_str()));
    //validate windows will be evaluated - check that there will be clusters evaluated...
    iMaxEndWindow = std::min(m_nFlexibleWindowEndRangeEndIndex, m_nFlexibleWindowStartRangeEndIndex + m_nIntervalCut);
    iWindowStart = std::max(m_nFlexibleWindowEndRangeStartIndex - m_nIntervalCut, m_nFlexibleWindowStartRangeStartIndex);
    if (iWindowStart >= iMaxEndWindow) {
      GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sTimeIntervalType, true, false);
      JulianToString(sDateWST, gvTimeIntervalStartTimes[iWindowStart]);
      JulianToString(sDateMaxWET, gvTimeIntervalStartTimes[iMaxEndWindow] - 1);
      GenerateResolvableException("Error: No clusters will be evaluated.\n"
                                  "       With the incorporation of a maximum temporal cluster size of %i %s,\n"
                                  "       the temporal window scanned has a start time of %s (end range\n"
                                  "       ending time minus %i %s) and a maximum window end time of %s\n"
                                  "       (start range ending time plus %i %s), which results in no windows scanned.",
                                  "Setup()", m_nIntervalCut * gParameters.GetTimeAggregationLength(),
                                  sTimeIntervalType.c_str(), sDateWST.c_str(),
                                  m_nIntervalCut * gParameters.GetTimeAggregationLength(),
                                  sTimeIntervalType.c_str(), sDateMaxWET.c_str(),
                                  m_nIntervalCut * gParameters.GetTimeAggregationLength(), sTimeIntervalType.c_str());
    }
    //The parameter validation checked already whether the end range dates conflicted,
    //but the maxium temporal cluster size may actually cause the range dates to be
    //different than the user defined.
    if (m_nFlexibleWindowEndRangeStartIndex > iMaxEndWindow) {
      GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sTimeIntervalType, true, false);
      JulianToString(sDateMaxWET, gvTimeIntervalStartTimes[iMaxEndWindow] - 1);
      GenerateResolvableException("Error: No clusters will be evaluated.\n"
                                  "       With the incorporation of a maximum temporal cluster size of %i %s\n"
                                  "       the maximum window end time becomes %s (start range ending\n"
                                  "       time plus %i %s), which does not intersect with scanning window end range.\n","Setup()",
                                  m_nIntervalCut * gParameters.GetTimeAggregationLength(), sTimeIntervalType.c_str(),
                                  sDateMaxWET.c_str(), m_nIntervalCut * gParameters.GetTimeAggregationLength(), sTimeIntervalType.c_str());
    }

    // Collapse unused time intervals at end of study period, if possible.
    if (m_nTimeIntervals - iMaxEndWindow > 1)
      gvTimeIntervalStartTimes.erase(gvTimeIntervalStartTimes.end() - (m_nTimeIntervals - iMaxEndWindow), gvTimeIntervalStartTimes.end() - 1);
    // Collapse unused time intervals at beginning of study period, if possible.
    if (iWindowStart > 1)
      gvTimeIntervalStartTimes.erase(gvTimeIntervalStartTimes.begin() + 1, gvTimeIntervalStartTimes.begin() + iWindowStart);
    m_nTimeIntervals = gvTimeIntervalStartTimes.size() - 1;
    // recalculate flexable window indexes
    m_nFlexibleWindowStartRangeStartIndex = GetTimeIntervalOfDate(CharToJulian(gParameters.GetStartRangeStartDate().c_str()));
    m_nFlexibleWindowStartRangeEndIndex = GetTimeIntervalOfDate(CharToJulian(gParameters.GetStartRangeEndDate().c_str()));
    m_nFlexibleWindowEndRangeStartIndex = GetTimeIntervalOfEndDate(CharToJulian(gParameters.GetEndRangeStartDate().c_str()));
    m_nFlexibleWindowEndRangeEndIndex = GetTimeIntervalOfEndDate(CharToJulian(gParameters.GetEndRangeEndDate().c_str()));
  }
}

/** internal setup function */
void CSaTScanData::Setup() {
  int es, ea, lCurrentEllipse=0;

  //For now, compute the angle and store the angle and shape
  //for each ellipsoid.  Maybe transfer info to a different location in the
  //application or compute "on the fly" prior to printing.
  if (gParameters.GetNumTotalEllipses() > 0) {
    gvEllipseAngles.resize(gParameters.GetNumTotalEllipses());
    gvEllipseShapes.resize(gParameters.GetNumTotalEllipses());
    for (es=0; es < gParameters.GetNumRequestedEllipses(); ++es) {
       for (ea=0; ea < gParameters.GetEllipseRotations()[es]; ++ea) {
          gvEllipseAngles[lCurrentEllipse] = PI * ea/gParameters.GetEllipseRotations()[es];
          gvEllipseShapes[lCurrentEllipse] = gParameters.GetEllipseShapes()[es];
          ++lCurrentEllipse;
       }
    }
  }
  gTractHandler.reset(new TractHandler(gParameters.GetIsPurelyTemporalAnalysis(), gParameters.GetMultipleCoordinatesType()));
  if (gParameters.UseSpecialGrid())
    gCentroidsHandler.reset(new CentroidHandler());
  else
    gCentroidsHandler.reset(new CentroidHandlerPassThrow(*gTractHandler));
}

/** Throws exception if case(s) were observed for an interval/location
    but the expected number of cases for interval/location is zero. For
    the Poisson model, this situation is likely the result of incorrect
    data provided in the population file, possibly the case file. For the
    other probability models, this is probably a bug in the code itself. */
void CSaTScanData::ValidateObservedToExpectedCases(const DataSet& Set) const {
  int           i;
  tract_t       t;
  std::string   sStart, sEnd;

  //note that cases must be cummulative and measure non-cummulative
  count_t   ** ppCumulativeCases=Set.getCaseData().GetArray();
  measure_t ** ppNonCumulativeMeasure=Set.getMeasureData().GetArray();

  try {
    for (i=0; i < m_nTimeIntervals; ++i)
       for (t=0; t < m_nTracts; ++t)
          if (!ppNonCumulativeMeasure[i][t] && GetCaseCount(ppCumulativeCases, i, t))
            GenerateResolvableException("Error: For locationID '%s' in time interval %s - %s,\n"
                                        "       the expected number of cases is zero but there were cases observed.\n"
                                        "       Please review the correctness of population and case files.",
                                        "ValidateObservedToExpectedCases()",
                                        gTractHandler->getLocations().at(t)->getIndentifier(),
                                        JulianToString(sStart, gvTimeIntervalStartTimes[i]).c_str(),
                                        JulianToString(sEnd, gvTimeIntervalStartTimes[i + 1] - 1).c_str());
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateObservedToExpectedCases()","CSaTScanData");
    throw;
  }
}

