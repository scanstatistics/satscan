#include "SaTScan.h"
#pragma hdrstop
#include "SaTScanData.h"
#include "TimeIntervalRange.h"

CSaTScanData::CSaTScanData(CParameters* pParameters, BasePrint *pPrintDirection) {
  try {
    Init();
    Setup(pParameters, pPrintDirection);
  }
  catch (ZdException & x) {
    x.AddCallpath("CSaTScanData(CParameters *)", "CSaTScanData");
    throw;
  }
}

CSaTScanData::~CSaTScanData() {
  try {
    delete gpDataStreams; gpDataStreams=0;
    delete m_pModel;
    delete gpNeighborCountHandler; gpNeighborCountHandler=0;
    delete gpSortedIntHandler; gpSortedIntHandler=0;
    delete gpSortedUShortHandler; gpSortedUShortHandler=0;
    delete [] mdE_Angles;
    delete [] mdE_Shapes;
    free(m_pIntervalStartTimes);
    delete gpTInfo;
    delete gpGInfo;
    delete gpMaxWindowLengthIndicator; gpMaxWindowLengthIndicator=0;
  }
  catch (...){}  
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
bool CSaTScanData::AdjustMeasure(measure_t ** pNonCumulativeMeasure, tract_t Tract, double dRelativeRisk, Julian StartDate, Julian EndDate) {
  int                                   interval;
  Julian                                AdjustmentStart, AdjustmentEnd, IntervalLength;
  measure_t                             Adjustment_t, fProportionAdjusted, tMaxMeasure_tValue;
  measure_t				MeasurePre, MeasurePost, MeasureDuring, ** pp_m;
  ZdString                              s;
  std::numeric_limits<measure_t>        measure_limit;

  tMaxMeasure_tValue = measure_limit.max();

  //NOTE: The adjustment for known relative risks is hard coded to the first
  //      data stream for the time being.
  DataStream & thisStream = gpDataStreams->GetStream(0);
  PopulationData & Population = thisStream.GetPopulationData();
  pp_m = thisStream.GetPopulationMeasureArray();
  count_t ** ppCases = thisStream.GetCaseArray();

  for (interval=GetTimeIntervalOfDate(StartDate); interval <= GetTimeIntervalOfDate(EndDate); ++interval) {
     AdjustmentStart = max(StartDate, m_pIntervalStartTimes[interval]);
     AdjustmentEnd = min(EndDate, m_pIntervalStartTimes[interval+1] - 1);
     //calculate measure for lower interval date to adjustment start date
     MeasurePre = CalcMeasureForTimeInterval(Population, pp_m, Tract, m_pIntervalStartTimes[interval], AdjustmentStart);
     //calculate measure for adjustment period
     MeasureDuring = CalcMeasureForTimeInterval(Population, pp_m, Tract, AdjustmentStart, AdjustmentEnd+1);
     //calculate measure for adjustment end date to upper interval date
     MeasurePost = CalcMeasureForTimeInterval(Population, pp_m, Tract, AdjustmentEnd+1, m_pIntervalStartTimes[interval+1]);
     //validate that data overflow will not occur
     if (MeasureDuring && (dRelativeRisk > (tMaxMeasure_tValue - MeasurePre - MeasurePost) / MeasureDuring))
       SSGenerateException("Error: Data overflow occurs when adjusting expected number of cases.\n"
                           "       The specified relative risk %lf in the adjustment file\n"
                           "       is too large.\n", "AssignMeasure()", dRelativeRisk);
     //assign adjusted measure                      
     pNonCumulativeMeasure[interval][Tract] = MeasurePre + dRelativeRisk * MeasureDuring + MeasurePost;
     //if measure has been adjusted to zero, check that cases adjusted interval are also zero
     if (pNonCumulativeMeasure[interval][Tract] == 0 && GetCaseCount(ppCases, interval, Tract)) {
       ZdString         sStart, sEnd;
       std::string      sId;
       SSGenerateException("Error: For locationID '%s', you have adjusted the expected number\n"
                           "       of cases in the period %s to %s to be zero, but there\n"
                           "       are cases in that interval.\n"
                           "       If the expected is zero, the number of cases must also be zero.\n",
                           "AdjustMeasure()",
                           (Tract == -1 ? "All" : gpTInfo->tiGetTid(Tract, sId)),
                           JulianToString(sStart, StartDate).GetCString(),
                           JulianToString(sEnd, EndDate).GetCString());
       return false;
     }
  }
  return true;
}

/** Sequential analyses will call this function to clear neighbor information and
    re-calculate neighbors. Note that only when the maximum spatial cluster size
    is specified as a percentage of the population that this operation need be
    performed between iterations of a sequential scan. */
void CSaTScanData::AdjustNeighborCounts() {
  try {
    //Deallocate neighbor information in sorted structures.
    if (m_pParameters->GetMaxGeoClusterSizeTypeIsPopulationBased()) {
      //Free/clear previous interation's neighbor information.
      if (gpSortedUShortHandler)
        gpSortedUShortHandler->FreeThirdDimension();
      else
        gpSortedIntHandler->FreeThirdDimension();
      gpNeighborCountHandler->Set(0);
 
      //Recompute neighbors.
      MakeNeighbors(gpTInfo, gpGInfo, (gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0),
                    (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0),
                    static_cast<tract_t>(m_nTotalTractsAtStart), m_nGridTracts,
                    (gvCircleMeasure.size() ? &gvCircleMeasure[0] : gpDataStreams->GetStream(0/*for now*/).GetMeasureArray()[0]),
                    m_nMaxCircleSize, m_nMaxCircleSize,
                    gpNeighborCountHandler->GetArray(), m_pParameters->GetDimensionsOfData(),
                    m_pParameters->GetNumRequestedEllipses(), m_pParameters->GetEllipseShapes(),
                    m_pParameters->GetEllipseRotations(), m_pParameters->GetMaxGeographicClusterSizeType(), gpPrint);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("AdjustNeighborCounts()", "CSaTScanData");
    throw;
  }
}

/** allocates two-dimensional array that will track the number of neighbors
    for each shape/grid point combination. */
void CSaTScanData::AllocateNeighborArray() {
  try {
    gpNeighborCountHandler = new TwoDimensionArrayHandler<tract_t>(m_pParameters->GetNumTotalEllipses() + 1, m_nGridTracts, 0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateNeighborArray()","CSaTScanData");
    throw;
  }
}

/** Allocates multi-dimensional array that stores tract index for each neighbor
    of each ellipse/grid point combination. */
void CSaTScanData::AllocateSortedArray() {
  try {
    if (m_nTracts < std::numeric_limits<unsigned short>::max())
      gpSortedUShortHandler = new ThreeDimensionArrayHandler<unsigned short>(m_pParameters->GetNumTotalEllipses()+1, m_nGridTracts, 0);
    else
      gpSortedIntHandler = new ThreeDimensionArrayHandler<tract_t>(m_pParameters->GetNumTotalEllipses()+1, m_nGridTracts, 0);
  }
  catch (ZdException &x) {
    delete gpSortedUShortHandler; gpSortedUShortHandler=0;
    delete gpSortedIntHandler; gpSortedIntHandler=0;
    x.AddCallpath("AllocateSortedArray()","CSaTScanData");
    throw;
  }
}

void CSaTScanData::AllocateSimulationStructures() {
  ProbabiltyModelType   eProbabiltyModelType(m_pParameters->GetProbabiltyModelType());
  AnalysisType          eAnalysisType(m_pParameters->GetAnalysisType());

  try {
    //allocate simulation case arrays
    if (eProbabiltyModelType != 10/*normal*/ && eProbabiltyModelType != 11/*rank*/)
      gpDataStreams->AllocateSimulationCases();
    //allocate simulation measure arrays
    if (eProbabiltyModelType == 10/*normal*/ || eProbabiltyModelType == 11/*rank*/ || eProbabiltyModelType == 12/*survival*/)
      gpDataStreams->AllocateSimulationMeasure();
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationStructures()","CSaTScanData");
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
measure_t CSaTScanData::CalcMeasureForTimeInterval(PopulationData & Population, measure_t ** ppPopulationMeasure, tract_t Tract, Julian StartDate, Julian NextStartDate) {
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

/** calculates expected number of cases */
bool CSaTScanData::CalculateExpectedCases() {
  size_t        t;
  bool          bReturn=true;

  gpPrint->SatScanPrintf("Calculating expected number of cases\n");
  //calculates expected cases for each data stream
  for (t=0; t < gpDataStreams->GetNumStreams() && bReturn; ++t) {
     bReturn = CalculateMeasure(gpDataStreams->GetStream(t));
     gtTotalMeasure += gpDataStreams->GetStream(t).GetTotalMeasure();
     gtTotalCases += gpDataStreams->GetStream(t).GetTotalCases();
     gtTotalPopulation += gpDataStreams->GetStream(t).GetTotalPopulation(); 
  }
  SetMaxCircleSize();

  return bReturn;
}

bool CSaTScanData::CalculateMeasure(DataStream & thisStream) {
  bool bReturn;

  try {
    SetAdditionalCaseArrays(thisStream);
    bReturn = (m_pModel->CalculateMeasure(thisStream));
    thisStream.SetTotalCasesAtStart(thisStream.GetTotalCases());
    thisStream.SetTotalControlsAtStart(thisStream.GetTotalControls());
    thisStream.SetTotalMeasureAtStart(thisStream.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CSaTScanData");
    throw;
  }
  return bReturn;
}

int CSaTScanData::ComputeNewCutoffInterval(Julian jStartDate, Julian jEndDate) {
   int  iIntervalCut;
   long lTimeBetween;

   if (m_pParameters->GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
     lTimeBetween = static_cast<long>(floor((TimeBetween(jStartDate, jEndDate, m_pParameters->GetTimeIntervalUnitsType()))*m_pParameters->GetMaximumTemporalClusterSize()/100.0));
     iIntervalCut = static_cast<int>(floor(lTimeBetween / m_pParameters->GetTimeIntervalLength()));
   }
   else
     iIntervalCut = m_nIntervalCut;

   return iIntervalCut;
}

/********************************************************************
  Finds the measure M for a requested tract and date.
  Input: Date, Tracts, #PopPoints
 ********************************************************************/
measure_t CSaTScanData::DateMeasure(PopulationData & Population, measure_t ** ppPopulationMeasure, Julian Date, tract_t Tract) {
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
bool CSaTScanData::FindNeighbors(bool bSimulations) {
  int           i, j;
  double        dMaxCircleSize;

  try {
    //if this iteration of call not simulations
    if (! bSimulations) {
      AllocateSortedArray();
      AllocateNeighborArray();
      //for real data, settings my indicate to report only smaller clusters
      dMaxCircleSize = (m_pParameters->GetRestrictingMaximumReportedGeoClusterSize() ? m_nMaxReportedCircleSize : m_nMaxCircleSize);
    }
    else {
      //when this functions is called for simualtions, we need to deallocate memory that
      //will be allocated once again in MakeNeighbors()
      if (gpSortedIntHandler)
        gpSortedIntHandler->FreeThirdDimension();
      else
        gpSortedUShortHandler->FreeThirdDimension();
      gpNeighborCountHandler->Set(0);
      dMaxCircleSize = m_nMaxCircleSize;
    }  

    //NOTE: The measure from first data stream is used when calculating neighbors,
    //      at least for the time being.
    if (m_pParameters->GetIsSequentialScanning())
        MakeNeighbors(gpTInfo, gpGInfo, (gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0),
                      (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0), m_nTracts, m_nGridTracts,
                      (gvCircleMeasure.size() ? &gvCircleMeasure[0] : gpDataStreams->GetStream(0).GetMeasureArray()[0]),
                      dMaxCircleSize, gpDataStreams->GetStream(0).GetTotalMeasure(),
                      gpNeighborCountHandler->GetArray(), m_pParameters->GetDimensionsOfData(), m_pParameters->GetNumRequestedEllipses(),
                      m_pParameters->GetEllipseShapes(), m_pParameters->GetEllipseRotations(),
                      m_pParameters->GetMaxGeographicClusterSizeType(), gpPrint);
    else
        MakeNeighbors(gpTInfo, gpGInfo, (gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0),
                      (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0), m_nTracts, m_nGridTracts,
                      (gvCircleMeasure.size() ? &gvCircleMeasure[0] : gpDataStreams->GetStream(0).GetMeasureArray()[0]),
                      dMaxCircleSize, dMaxCircleSize, gpNeighborCountHandler->GetArray(),
                      m_pParameters->GetDimensionsOfData(), m_pParameters->GetNumRequestedEllipses(),
                      m_pParameters->GetEllipseShapes(), m_pParameters->GetEllipseRotations(),
                      m_pParameters->GetMaxGeographicClusterSizeType(), gpPrint);

   }
   catch (ZdException & x) {
      x.AddCallpath("FindNeighbors()", "CSaTScanData");
      throw;
   }
  return true;
}

double CSaTScanData::GetAnnualRate() const {
  double nYears      = (double)(m_nEndDate+1 - m_nStartDate) / 365.2425;
  //NOTE: This function is hard code to report only the annual rate based
  //      upon data from first data stream, at least for the time being. 
  double dTotalCases = gpDataStreams->GetStream(0).GetTotalCases();
  double dTotalPopulation = gpDataStreams->GetStream(0).GetTotalPopulation();
  double nAnnualRate = (m_nAnnualRatePop*dTotalCases) / (dTotalPopulation*nYears);

  return nAnnualRate;
}

double CSaTScanData::GetAnnualRateAtStart() const {
  double nYears      = (double)(m_nEndDate+1 - m_nStartDate) / 365.2425;
  //NOTE: This function is hard code to report only the annual rate based
  //      upon data from first data stream, at least for the time being. 
  double dTotalCasesAtStart = gpDataStreams->GetStream(0).GetTotalCasesAtStart();
  double dTotalPopulation = gpDataStreams->GetStream(0).GetTotalPopulation();
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

//Measure Adjustment used when calculating relative risk/expected counts
//to disply in report file.
double CSaTScanData::GetMeasureAdjustment() const {
  if (m_pParameters->GetProbabiltyModelType() == POISSON || m_pParameters->GetProbabiltyModelType() == SPACETIMEPERMUTATION)
    return 1.0;
  else if (m_pParameters->GetProbabiltyModelType() == BERNOULLI) {
    //NOTE: This function is hard code to report only the measure adjustment
    //      based upon data from first data stream, at least for the time being.
    double dTotalCases = gpDataStreams->GetStream(0).GetTotalCases();
    double dTotalPopulation = gpDataStreams->GetStream(0).GetTotalPopulation();
    return dTotalCases / dTotalPopulation;
  }
  else
    return 0.0;
}

/**********************************************************************
 Return "nearness"-th closest neighbor to "t"
 (nearness == 1 returns "t").
 **********************************************************************/
tract_t CSaTScanData::GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const {
   if (gpSortedUShortHandler)
      return (tract_t)gpSortedUShortHandler->GetArray()[iEllipse][t][nearness - 1];
   else
      return gpSortedIntHandler->GetArray()[iEllipse][t][nearness - 1];
}

/** Input: Date.                                                    **/   
/** Returns: Index of the time interval to which the date belongs.   **/
/** If Date does not belong to any time interval, -1 is returned. **/
/** Note: First time interval has index 0.                          **/
int CSaTScanData::GetTimeIntervalOfDate(Julian Date) const {
  int   i=0;

  //check that date is within study period
  if (Date < m_pIntervalStartTimes[0] || Date >= m_pIntervalStartTimes[m_nTimeIntervals])
    return -1;

  while (Date >=  m_pIntervalStartTimes[i+1])
       ++i;
       
  return i;
}

void CSaTScanData::Init() {
  gpTInfo = 0;
  gpGInfo = 0;
  m_pModel = 0;
  gpDataStreams = 0;
  gpNeighborCountHandler=0;
  gpSortedIntHandler=0;
  gpSortedUShortHandler=0;
  m_pIntervalStartTimes = 0;
  m_nAnnualRatePop = 100000;
  mdE_Angles = 0;
  mdE_Shapes = 0;
  m_nMaxReportedCircleSize = 0;
  gpMaxWindowLengthIndicator = 0;
  m_nTotalMaxCirclePopulation = 0;
  gtTotalMeasure=0;
  gtTotalCases=0;
  gtTotalPopulation=0;
}

void CSaTScanData::MakeData(int iSimulationNumber, DataStreamGateway & DataGateway) {
   try {
     for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
        m_pModel->MakeData(iSimulationNumber, DataGateway.GetDataStreamInterface(t), t);
   }
   catch (ZdException & x) {
      x.AddCallpath("MakeData()", "CSaTScanData");
      throw;
   }
}

/** reads data from input files for a Bernoulli probability model */
bool CSaTScanData::ReadBernoulliData() {
  size_t        t;

  try {
    if (!ReadCoordinatesFile())
      return false;

    gpDataStreams = new DataStreamHandler(*this, gpPrint);
    for (t=0; t < gpDataStreams->GetNumStreams(); ++t) {
      gpDataStreams->GetStream(t).SetAggregateCategories(true); 
      if (!gpDataStreams->ReadCaseFile(t))
        return false;
      if (!gpDataStreams->ReadControlFile(t))
        return false;
    }    
    if (m_pParameters->UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (m_pParameters->UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    delete gpDataStreams; gpDataStreams=0;
    x.AddCallpath("ReadBernoulliData()","CSaTScanData");
    throw;
  }
  return true;
}

void CSaTScanData::ReadDataFromFiles() {
  try {
    SetStartAndEndDates();
    SetNumTimeIntervals();
    SetIntervalCut();
    SetIntervalStartTimes();
    SetTimeIntervalRangeIndexes();
    if (m_pParameters->GetIsProspectiveAnalysis())
      SetProspectiveIntervalStart();
    SetMaxTemporalWindowLengthIndicator();
    if (! m_pModel->ReadData())
      SSGenerateException("\nProblem encountered reading in data.", "ReadDataFromFiles");
    gpTInfo->tiConcaticateDuplicateTractIdentifiers();
    gpGInfo->giFindDuplicateCoords(stderr);
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadDataFromFiles()", "CSaTScanData");
    throw;
  }
}

/** reads data from input files for a Poisson probability model */
bool CSaTScanData::ReadPoissonData() {
  size_t        t;

  try {
    if (!ReadCoordinatesFile())
      return false;

    gpDataStreams = new DataStreamHandler(*this, gpPrint);
    for (t=0; t < gpDataStreams->GetNumStreams(); ++t) {
       if (!gpDataStreams->ReadPopulationFile(t))
         return false;
       if (!gpDataStreams->ReadCaseFile(t))
         return false;
       gpDataStreams->GetStream(t).CheckPopulationDataCases(*this);
    }
    if (m_pParameters->UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (m_pParameters->UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    delete gpDataStreams; gpDataStreams=0;
    x.AddCallpath("ReadPoissonData()","CSaTScanData");
    throw;
  }
  return true;
}

/** reads data from input files for a Space-Time Permutation probability model */
bool CSaTScanData::ReadSpaceTimePermutationData() {
  size_t        t;

  try {
    if (!ReadCoordinatesFile())
      return false;
    if (m_pParameters->UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
       return false;
    gpDataStreams = new DataStreamHandler(*this, gpPrint);
    for (t=0; t < gpDataStreams->GetNumStreams(); ++t)
       if (!gpDataStreams->ReadCaseFile(t))
         return false;
    if (m_pParameters->UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    delete gpDataStreams; gpDataStreams=0;
    x.AddCallpath("ReadSpaceTimePermutationData()","CSaTScanData");
    throw;
  }
  return true;
}

void CSaTScanData::RemoveTractSignificance(tract_t tTractIndex) {
  count_t       tTotalCases, tTotalControls;
  measure_t     tTotalMeasure;

  try {
    for (size_t t=0; t < gpDataStreams->GetNumStreams(); ++t) {
       DataStream & thisStream = gpDataStreams->GetStream(t);
       tTotalCases = thisStream.GetTotalCases();
       tTotalCases -= thisStream.GetCaseArray()[0][tTractIndex];
       thisStream.GetCaseArray()[0][tTractIndex] = 0;
       thisStream.SetTotalCases(tTotalCases);
       tTotalMeasure = thisStream.GetTotalMeasure();
       tTotalMeasure -= thisStream.GetMeasureArray()[0][tTractIndex];
       thisStream.GetMeasureArray()[0][tTractIndex] = 0;
       thisStream.SetTotalMeasure(tTotalMeasure);
       if (m_pParameters->GetProbabiltyModelType() == BERNOULLI) {
         tTotalControls = thisStream.GetTotalControls();
         tTotalControls -= thisStream.GetControlArray()[0][tTractIndex];
         thisStream.GetControlArray()[0][tTractIndex] = 0;
         thisStream.SetTotalControls(tTotalControls);
       }
       if (t == 0 && gvCircleMeasure.size()) {
         m_nTotalMaxCirclePopulation -= gvCircleMeasure[tTractIndex];
         gvCircleMeasure[tTractIndex] = 0;
       }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("RemoveTractSignificance()", "CSaTScanData");
    throw;
  }
}

/** Conditionally allocates and sets additional case arrays. */
void CSaTScanData::SetAdditionalCaseArrays(DataStream & thisStream) {
  try {
    if (m_pParameters->GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION ||
        m_pParameters->GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC)
      thisStream.SetCasesByTimeInterval();  
  }
  catch (ZdException &x) {
    x.AddCallpath("SetAdditionalCaseArrays()","CSaTScanData");
    throw;
  }
}

/** Creates max window length object which dictates maximum temporal window
    length during analysis simluations. */
void CSaTScanData::SetMaxTemporalWindowLengthIndicator() {
  try {
    if (m_pParameters->GetNumReplicationsRequested() > 0) { //no needed if no simulations performed
      if (m_pParameters->GetIsProspectiveAnalysis() && m_pParameters->GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE)
        gpMaxWindowLengthIndicator = new ProspectiveMaxWindowLengthIndicator(*this);
      else
        gpMaxWindowLengthIndicator = new FixedMaxWindowLengthIndicator(*this);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMaxTemporalWindowLengthIndicator()","CSaTScanData");
    throw;
  }
}

/* Calculates the number of time intervals to include in potential clusters
   without exceeding the maximum cluster size with respect to time.*/
void CSaTScanData::SetIntervalCut() {
  ZdString      sIntervalCutMessage, sTimeIntervalType;
  long          lMaxTemporalLength, lStudyPeriodLength;

  try {
    if (m_nTimeIntervals == 1)
      m_nIntervalCut = 1;
    else if (m_nTimeIntervals > 1) {
      if (m_pParameters->GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
        lStudyPeriodLength = TimeBetween(m_nStartDate, m_nEndDate, m_pParameters->GetTimeIntervalUnitsType());
        lMaxTemporalLength = static_cast<long>(floor(lStudyPeriodLength * m_pParameters->GetMaximumTemporalClusterSize()/100.0));
        m_nIntervalCut = static_cast<int>(floor(lMaxTemporalLength / m_pParameters->GetTimeIntervalLength()));
      }
      else
        m_nIntervalCut = static_cast<int>(m_pParameters->GetMaximumTemporalClusterSize() / m_pParameters->GetTimeIntervalLength());
    }

    if (m_nIntervalCut==0) {
      switch (m_pParameters->GetTimeIntervalUnitsType()) {
          case YEAR  : sTimeIntervalType = "year"; break;
          case MONTH : sTimeIntervalType = "month"; break;
          case DAY   : sTimeIntervalType = "day"; break;
          // if we get here, there is an error somewhere else
          default: sTimeIntervalType = "none"; break;
        };

        if (m_pParameters->GetMaximumTemporalClusterSizeType() == TIMETYPE) {
          sIntervalCutMessage << "Error: A maximum temporal cluster size of %g %s%s is less than one %d %s time interval.\n";
          sIntervalCutMessage << "       No clusters can be found.\n";
          SSGenerateException(sIntervalCutMessage.GetCString(), "SetIntervalCut()",
                              m_pParameters->GetMaximumTemporalClusterSize(), sTimeIntervalType.GetCString(),
                              (m_pParameters->GetMaximumTemporalClusterSize() == 1 ? "" : "s"),
                              m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString());
        }
        else if (m_pParameters->GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
          sIntervalCutMessage << "Error: A maximum temporal cluster size that is %g percent of a %d %s study period\n";
          sIntervalCutMessage << "       equates to %d %s%s, which is less than one %d %s time interval.\n";
          sIntervalCutMessage << "       No clusters can be found.\n";
          SSGenerateException(sIntervalCutMessage.GetCString(), "SetIntervalCut()",
                              m_pParameters->GetMaximumTemporalClusterSize(),
                              lStudyPeriodLength, sTimeIntervalType.GetCString(),
                              lMaxTemporalLength, sTimeIntervalType.GetCString(), (lMaxTemporalLength == 1 ? "" : "s"),
                              m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString());
        }
      }
   }
  catch (ZdException &x) {
    x.AddCallpath("SetIntervalCut()","CSaTScanData");
    throw;
  }
}

/** Set array of interval start times. */
void CSaTScanData::SetIntervalStartTimes() {
   // Not neccessary for purely spatial?
   try {
      m_pIntervalStartTimes = (Julian*) Smalloc((m_nTimeIntervals+1)*sizeof(Julian), gpPrint);

      m_pIntervalStartTimes[0] = m_nStartDate;
      m_pIntervalStartTimes[m_nTimeIntervals] = m_nEndDate+1;

      for (int i = m_nTimeIntervals-1; i > 0; --i)
         m_pIntervalStartTimes[i] = DecrementDate(m_pIntervalStartTimes[i+1], m_pParameters->GetTimeIntervalUnitsType(), m_pParameters->GetTimeIntervalLength());
   }
   catch (ZdException & x) {
      x.AddCallpath("SetIntervalStartTimes()", "CSaTScanData");
      throw;
   }
}

/** Causes maximum circle size to be set based on parameters settings. */
void CSaTScanData::SetMaxCircleSize() {
  //NOTE: The measure from the first data stream is used when calculating
  //      the maximum circle size, at least for the time being. 
  measure_t tTotalMeasure = gpDataStreams->GetStream(0).GetTotalMeasure();

  try {
    switch (m_pParameters->GetMaxGeographicClusterSizeType()) {
      case PERCENTOFPOPULATIONFILETYPE :
           m_nMaxCircleSize = (m_pParameters->GetMaximumGeographicClusterSize() / 100.0) * m_nTotalMaxCirclePopulation;
           if (m_pParameters->GetRestrictingMaximumReportedGeoClusterSize())
             m_nMaxReportedCircleSize = (m_pParameters->GetMaximumReportedGeoClusterSize() / 100.0) * m_nTotalMaxCirclePopulation;
           break;
      case PERCENTOFPOPULATIONTYPE :
           m_nMaxCircleSize = (m_pParameters->GetMaximumGeographicClusterSize() / 100.0) * tTotalMeasure;
           if (m_pParameters->GetRestrictingMaximumReportedGeoClusterSize())
             m_nMaxReportedCircleSize = (m_pParameters->GetMaximumReportedGeoClusterSize() / 100.0) * tTotalMeasure;
           break;
      case DISTANCETYPE :
           m_nMaxCircleSize = m_pParameters->GetMaximumGeographicClusterSize();
           if (m_pParameters->GetRestrictingMaximumReportedGeoClusterSize())
             m_nMaxReportedCircleSize = m_pParameters->GetMaximumReportedGeoClusterSize();
           break;
      default : ZdException::Generate("Unknown maximum spatial cluster type: '%i'.", "SetMaxCircleSize()",
                                      m_pParameters->GetMaxGeographicClusterSizeType());
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMaxCircleSize()","CSaTScanData");
    throw;
  }
}

void CSaTScanData::SetNumTimeIntervals() {
  long nTime = TimeBetween(m_nStartDate, m_nEndDate, m_pParameters->GetTimeIntervalUnitsType());
  m_nTimeIntervals = (int)ceil((float)nTime / (float)m_pParameters->GetTimeIntervalLength());

  if (m_pParameters->GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION && m_nTimeIntervals <= 1)
    SSGenerateException("Error: Time stratified randomization adjustment requires more than\n"
                        "       one time interval.\n", "SetNumTimeIntervals()");
}

/** allocates probability model */
/*void CSaTScanData::SetProbabilityModel() {
  try {
    switch (m_pParameters->GetProbabiltyModelType()) {
       case POISSON              : m_pModel = new CPoissonModel(*m_pParameters, *this, *gpPrint);   break;
       case BERNOULLI            : m_pModel = new CBernoulliModel(*m_pParameters, *this, *gpPrint); break;
       case SPACETIMEPERMUTATION : m_pModel = new CSpaceTimePermutationModel(*m_pParameters, *this, *gpPrint); break;
       default : ZdException::Generate("Unknown probability model type: '%d'.\n",
                                       "SetProbabilityModel()", m_pParameters->GetProbabiltyModelType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProbabilityModel()","CSaTScanData");
    throw;
  }
} */

/* Calculates which time interval the prospectice space-time start date is in.*/
/* MAKE SURE THIS IS EXECUTED AFTER THE  m_nTimeIntervals VARIABLE HAS BEEN SET */
void CSaTScanData::SetProspectiveIntervalStart() {
  long    lTime;
  Julian  lProspStartDate;

  try {
    lProspStartDate = m_pParameters->GetProspectiveStartDateAsJulian();
    lTime = TimeBetween(lProspStartDate, m_nEndDate, m_pParameters->GetTimeIntervalUnitsType());
    m_nProspectiveIntervalStart = m_nTimeIntervals - (int)ceil((float)lTime/(float)m_pParameters->GetTimeIntervalLength()) + 1;

    if (m_nProspectiveIntervalStart < 0)
      SSGenerateException("Error: The prospective start date '%s' is prior to the study period start date '%s'.\n",
                          "SetProspectiveIntervalStart()", m_pParameters->GetProspectiveStartDate().c_str(),
                          m_pParameters->GetStudyPeriodStartDate().c_str());
    if (m_nProspectiveIntervalStart > m_nTimeIntervals)
      SSGenerateException("Error: The prospective start date '%s' occurs after the study period end date '%s'.\n",
                          "SetProspectiveIntervalStart", m_pParameters->GetProspectiveStartDate().c_str(),
                          m_pParameters->GetStudyPeriodEndDate().c_str());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProspectiveIntervalStart()","CSaTScanData");
    throw;
  }
}

void CSaTScanData::SetPurelyTemporalCases() {
  size_t        t;

  try {
    for (t=0; t < gpDataStreams->GetNumStreams(); ++t)
      gpDataStreams->GetStream(t).SetPTCasesArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalCases()","CSaTScanData");
    throw;
  }
}

void CSaTScanData::SetStartAndEndDates() {
  try {
    m_nStartDate = m_pParameters->GetStudyPeriodStartDateAsJulian();
    m_nEndDate   = m_pParameters->GetStudyPeriodEndDateAsJulian();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetStartAndEndDates()","CSaTScanData");
    throw;
  }
}

/** Sets time interval end range index into interval start times array. */
void CSaTScanData::SetScanningWindowEndRangeIndex(Julian EndRangeDate, int & iEndRangeDateIndex) {
  int   i;

  //find index for end range end date, the interval where date fits, rounding to higher index
  iEndRangeDateIndex = -1;
  for (i=m_nTimeIntervals; i > 0  && iEndRangeDateIndex == -1; --i)
     if (EndRangeDate <= m_pIntervalStartTimes[i] - 1 && EndRangeDate > m_pIntervalStartTimes[i - 1] - 1)
        iEndRangeDateIndex = i;
}

/** Sets time interval start range index into interval start times array. */
void CSaTScanData::SetScanningWindowStartRangeIndex(Julian StartRangeDate, int & iStartRangeDateIndex) {
  int   i;

  iStartRangeDateIndex = -1;
  for (i=0; i < m_nTimeIntervals && iStartRangeDateIndex == -1; ++i)
     if (StartRangeDate >= m_pIntervalStartTimes[i] && StartRangeDate < m_pIntervalStartTimes[i + 1])
       iStartRangeDateIndex = i;
}

/** Sets indexes of time interval ranges into interval start time array. */
void CSaTScanData::SetTimeIntervalRangeIndexes() {
  ZdString      sTimeIntervalType, sMessage;
  char          sDateWST[50], sDateMaxWET[50]; 
  int           iMaxEndWindow, iWindowStart;

  if (m_pParameters->GetIncludeClustersType() == CLUSTERSINRANGE) {
    //find start range date indexes
    SetScanningWindowStartRangeIndex(m_pParameters->GetStartRangeDateAsJulian(m_pParameters->GetStartRangeStartDate()), m_nStartRangeStartDateIndex);
    SetScanningWindowStartRangeIndex(m_pParameters->GetStartRangeDateAsJulian(m_pParameters->GetStartRangeEndDate()), m_nStartRangeEndDateIndex);
    //find end range date indexes
    SetScanningWindowEndRangeIndex(m_pParameters->GetEndRangeDateAsJulian(m_pParameters->GetEndRangeStartDate()), m_nEndRangeStartDateIndex);
    SetScanningWindowEndRangeIndex(m_pParameters->GetEndRangeDateAsJulian(m_pParameters->GetEndRangeEndDate()), m_nEndRangeEndDateIndex);
    //validate windows will be evaluated
    //check that there will be clusters evaluated...
    iMaxEndWindow = std::min(m_nEndRangeEndDateIndex, m_nStartRangeEndDateIndex + m_nIntervalCut);
    iWindowStart = std::max(m_nEndRangeStartDateIndex - m_nIntervalCut, m_nStartRangeStartDateIndex);
    if (iWindowStart >= iMaxEndWindow) {
      sTimeIntervalType = m_pParameters->GetDatePrecisionAsString(m_pParameters->GetTimeIntervalUnitsType());
      sTimeIntervalType.ToLowercase();
      JulianToChar(sDateWST, m_pIntervalStartTimes[iWindowStart]);
      JulianToChar(sDateMaxWET, m_pIntervalStartTimes[iMaxEndWindow] - 1);
      SSGenerateException("Error: No clusters will be evaluated.\n"
                          "       With the incorporation of a maximum temporal cluster size of %i %s,\n"
                          "       the temporal window scanned has a start time of %s (end range\n"
                          "       ending time minus %i %s) and a maximum window end time of %s\n"
                          "       (start range ending time plus %i %s), which results in no windows scanned."
                          , "Setup()", m_nIntervalCut * m_pParameters->GetTimeIntervalLength(),
                          sTimeIntervalType.GetCString(), sDateWST,
                          m_nIntervalCut * m_pParameters->GetTimeIntervalLength(),
                          sTimeIntervalType.GetCString(), sDateMaxWET,
                          m_nIntervalCut * m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString());
    }
    //The parameter validation checked already whether the end range dates conflicted,
    //but the maxium temporal cluster size may actually cause the range dates to be
    //different than the user defined.
    if (m_nEndRangeStartDateIndex > iMaxEndWindow) {
      sTimeIntervalType = m_pParameters->GetDatePrecisionAsString(m_pParameters->GetTimeIntervalUnitsType());
      sTimeIntervalType.ToLowercase();
      JulianToChar(sDateMaxWET, m_pIntervalStartTimes[iMaxEndWindow] - 1);
      SSGenerateException("Error: No clusters will be evaluated.\n"
                          "       With the incorporation of a maximum temporal cluster size of %i %s\n"
                          "       the maximum window end time becomes %s (start range ending\n"
                          "       time plus %i %s), which does not intersect with scanning window end range.\n","Setup()",
                          m_nIntervalCut * m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString(),
                          sDateMaxWET, m_nIntervalCut * m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString());
    }
  }
}

/** internal setup function */
void CSaTScanData::Setup(CParameters* pParameters, BasePrint *pPrintDirection) {
  long  lCurrentEllipse=0;

  try {
    gpPrint = pPrintDirection;
    m_pParameters = pParameters;

    gpTInfo = new TractHandler(*pPrintDirection); 
    gpGInfo = new GInfo(pPrintDirection);
    //SetProbabilityModel();
    //For now, compute the angle and store the angle and shape
    //for each ellipsoid.  Maybe transfer info to a different location in the
    //application or compute "on the fly" prior to printing.
    if (m_pParameters->GetNumTotalEllipses() > 0) {
      mdE_Angles = new double[m_pParameters->GetNumTotalEllipses()];
      mdE_Shapes = new double[m_pParameters->GetNumTotalEllipses()];
      for (int es = 0; es < pParameters->GetNumRequestedEllipses(); ++es) {
         for (int ea = 0; ea < m_pParameters->GetEllipseRotations()[es]; ++ea) {
            mdE_Angles[lCurrentEllipse]=PI*ea/m_pParameters->GetEllipseRotations()[es];
            mdE_Shapes[lCurrentEllipse]= m_pParameters->GetEllipseShapes()[es];
            ++lCurrentEllipse;
         }
      }
    }
  }
  catch (ZdException &x) {
    delete gpTInfo;
    delete gpGInfo;
    delete mdE_Angles;
    delete mdE_Shapes;
    x.AddCallpath("Setup()","CSaTScanData");
    throw;
  }
}

/** Throws exception if case(s) were observed for an interval/location
    but the expected number of cases for interval/location is zero. For
    the Poisson model, this situation is likely the result of incorrect
    data provided in the population file, possibly the case file. For the
    other probability models, this is probably a bug in the code itself. */
void CSaTScanData::ValidateObservedToExpectedCases(count_t ** ppCumulativeCases, measure_t ** ppNonCumulativeMeasure) const {
  int           i;
  tract_t       t;
  ZdString      sStart, sEnd;
  std::string   sId;

  try {
    for (i=0; i < m_nTimeIntervals; ++i)
       for (t=0; t < m_nTracts; ++t)
          if (!ppNonCumulativeMeasure[i][t] && GetCaseCount(ppCumulativeCases, i, t))
            SSGenerateException("Error: For locationID '%s' in time interval %s - %s,\n"
                                "       the expected number of cases is zero but there were cases observed.\n"
                                "       Please review the correctness of population and case files.",
                                "ValidateObservedToExpectedCases()",
                                gpTInfo->tiGetTid(t, sId),
                                JulianToString(sStart, m_pIntervalStartTimes[i]).GetCString(),
                                JulianToString(sEnd, m_pIntervalStartTimes[i + 1] - 1).GetCString());
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateObservedToExpectedCases()","CSaTScanData");
    throw;
  }
}
