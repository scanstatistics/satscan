//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SaTScanData.h"
#include "TimeIntervalRange.h"
#include "RankDataStreamHandler.h"
#include "SurvivalDataStreamHandler.h"
#include "NormalDataStreamHandler.h"
#include "PoissonDataStreamHandler.h"
#include "BernoulliDataStreamHandler.h"
#include "SpaceTimePermutationDataStreamHandler.h"

/** class constructor */
CSaTScanData::CSaTScanData(const CParameters& Parameters, BasePrint& PrintDirection)
             : gParameters(Parameters), gPrint(PrintDirection),
               m_nStartDate(Parameters.GetStudyPeriodStartDateAsJulian()),
               m_nEndDate(Parameters.GetStudyPeriodEndDateAsJulian()) {
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
    delete gpDataStreams; gpDataStreams=0;
    delete m_pModel;
    delete gpNeighborCountHandler; gpNeighborCountHandler=0;
    delete gpSortedIntHandler; gpSortedIntHandler=0;
    delete gpSortedUShortHandler; gpSortedUShortHandler=0;
    delete [] mdE_Angles;
    delete [] mdE_Shapes;
  }
  catch (...){}  
}

/** Adjusts passed non cumulative measure are for known relative risks, as
    previously read from user specified file. Caller is responsible for ensuring:
    - that passed 'measure **' points to a multiple dimensional array contained
      by passed RealDataStream object.
    - passed 'measure **' is in fact non-cumulative
    - passed 'measure **' points to valid memory, allocated to dimensions (number
      of time intervals plus one by number of tracts)                            */
void CSaTScanData::AdjustForKnownRelativeRisks(RealDataStream& thisStream, measure_t ** ppNonCumulativeMeasure) {
  measure_t                             c, AdjustedTotalMeasure_t;
  int                                   i;
  tract_t                               t;  
  AdjustmentsIterator_t                 itr;
  TractContainerIteratorConst_t         itr_deque;

  //apply adjustments to relative risks
  for (itr=gRelativeRiskAdjustments.GetAdjustments().begin(); itr != gRelativeRiskAdjustments.GetAdjustments().end(); ++itr) {
     const TractContainer_t & tract_deque = itr->second;
     for (itr_deque=tract_deque.begin(); itr_deque != tract_deque.end(); ++itr_deque)
        AdjustMeasure(thisStream, ppNonCumulativeMeasure, itr->first, (*itr_deque).GetRelativeRisk(),
                      (*itr_deque).GetStartDate(), (*itr_deque).GetEndDate());
  }

  // calculate total adjusted measure
  for (AdjustedTotalMeasure_t=0, i=0; i < m_nTimeIntervals; ++i)
     for (t=0; t < m_nTracts; ++t)
        AdjustedTotalMeasure_t += ppNonCumulativeMeasure[i][t];
  //Mutlipy the measure for each interval/tract by constant (c) to obtain total
  //adjusted measure (AdjustedTotalMeasure_t) equal to previous total measure (m_nTotalMeasure).
  c = thisStream.GetTotalMeasure()/AdjustedTotalMeasure_t;
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
bool CSaTScanData::AdjustMeasure(RealDataStream& thisStream, measure_t ** pNonCumulativeMeasure, tract_t Tract, double dRelativeRisk, Julian StartDate, Julian EndDate) {
  int                                   interval;
  Julian                                AdjustmentStart, AdjustmentEnd, IntervalLength;
  measure_t                             Adjustment_t, fProportionAdjusted, tMaxMeasure_tValue;
  measure_t				MeasurePre, MeasurePost, MeasureDuring, ** pp_m;
  ZdString                              s;
  std::numeric_limits<measure_t>        measure_limit;

  tMaxMeasure_tValue = measure_limit.max();

  //NOTE: The adjustment for known relative risks is hard coded to the first
  //      data stream for the time being.
  PopulationData & Population = thisStream.GetPopulationData();
  pp_m = thisStream.GetPopulationMeasureArray();
  count_t ** ppCases = thisStream.GetCaseArray();

  for (interval=GetTimeIntervalOfDate(StartDate); interval <= GetTimeIntervalOfDate(EndDate); ++interval) {
     AdjustmentStart = std::max(StartDate, gvTimeIntervalStartTimes[interval]);
     AdjustmentEnd = std::min(EndDate, gvTimeIntervalStartTimes[interval+1] - 1);
     //calculate measure for lower interval date to adjustment start date
     MeasurePre = CalcMeasureForTimeInterval(Population, pp_m, Tract, gvTimeIntervalStartTimes[interval], AdjustmentStart);
     //calculate measure for adjustment period
     MeasureDuring = CalcMeasureForTimeInterval(Population, pp_m, Tract, AdjustmentStart, AdjustmentEnd+1);
     //calculate measure for adjustment end date to upper interval date
     MeasurePost = CalcMeasureForTimeInterval(Population, pp_m, Tract, AdjustmentEnd+1, gvTimeIntervalStartTimes[interval+1]);
     //validate that data overflow will not occur
     if (MeasureDuring && (dRelativeRisk > (tMaxMeasure_tValue - MeasurePre - MeasurePost) / MeasureDuring))
       GenerateResolvableException("Error: Data overflow occurs when adjusting expected number of cases.\n"
                                   "       The specified relative risk %lf in the adjustment file\n"
                                   "       is too large.\n", "AssignMeasure()", dRelativeRisk);
     //assign adjusted measure                      
     pNonCumulativeMeasure[interval][Tract] = MeasurePre + dRelativeRisk * MeasureDuring + MeasurePost;
     //if measure has been adjusted to zero, check that cases adjusted interval are also zero
     if (pNonCumulativeMeasure[interval][Tract] == 0 && GetCaseCount(ppCases, interval, Tract)) {
       ZdString         sStart, sEnd;
       std::string      sId;
       GenerateResolvableException("Error: For locationID '%s', you have adjusted the expected number\n"
                                   "       of cases in the period %s to %s to be zero, but there\n"
                                   "       are cases in that interval.\n"
                                   "       If the expected is zero, the number of cases must also be zero.\n",
                                   "AdjustMeasure()",
                                   (Tract == -1 ? "All" : gTractHandler.tiGetTid(Tract, sId)),
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
    if (gParameters.GetMaxGeoClusterSizeTypeIsPopulationBased()) {
      //Free/clear previous interation's neighbor information.
      if (gpSortedUShortHandler)
        gpSortedUShortHandler->FreeThirdDimension();
      else
        gpSortedIntHandler->FreeThirdDimension();
      gpNeighborCountHandler->Set(0);
 
      //Recompute neighbors.
      MakeNeighbors(&gTractHandler, &gCentroidsHandler, (gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0),
                    (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0),
                    static_cast<tract_t>(m_nTotalTractsAtStart), m_nGridTracts,
                    (gvCircleMeasure.size() ? &gvCircleMeasure[0] : gpDataStreams->GetStream(0/*for now*/).GetMeasureArray()[0]),
                    m_nMaxCircleSize, m_nMaxCircleSize,
                    gpNeighborCountHandler->GetArray(), gParameters.GetDimensionsOfData(),
                    gParameters.GetNumRequestedEllipses(), gParameters.GetEllipseShapes(),
                    gParameters.GetEllipseRotations(), gParameters.GetMaxGeographicClusterSizeType(), &gPrint);
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
    gpNeighborCountHandler = new TwoDimensionArrayHandler<tract_t>(gParameters.GetNumTotalEllipses() + 1, m_nGridTracts, 0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateNeighborArray()","CSaTScanData");
    throw;
  }
}

/** Allocates three dimensional array that stores tract index for each neighbor
    of each ellipse by grid point combination. Data type of array is conditionally
    either unsigned short or tract_t; based upon number of tracts in coordinates
    file. Note that third dimension is allocated with zero length. Later, when
    neighbors are calculated, the third dimension will be re-allocated based upon
    the number of calculated neighbors for each (circle/ellipse) and grid point pair. */
void CSaTScanData::AllocateSortedArray() {
  try {
    if (m_nTracts < std::numeric_limits<unsigned short>::max())
      gpSortedUShortHandler = new ThreeDimensionArrayHandler<unsigned short>(gParameters.GetNumTotalEllipses()+1, m_nGridTracts, 0);
    else
      gpSortedIntHandler = new ThreeDimensionArrayHandler<tract_t>(gParameters.GetNumTotalEllipses()+1, m_nGridTracts, 0);
  }
  catch (ZdException &x) {
    delete gpSortedUShortHandler; gpSortedUShortHandler=0;
    delete gpSortedIntHandler; gpSortedIntHandler=0;
    x.AddCallpath("AllocateSortedArray()","CSaTScanData");
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

/** Calculates expected number of cases for each data stream. Records total
    measure, cases, and population for all streams. Calls method to determines
    the maximum spatial cluster size. */
void CSaTScanData::CalculateExpectedCases() {
  size_t        t;

  gPrint.SatScanPrintf("Calculating the expected number of cases\n");
  //calculates expected cases for each data stream
  for (t=0; t < gpDataStreams->GetNumStreams(); ++t) {
     CalculateMeasure(gpDataStreams->GetStream(t));
     gtTotalMeasure += gpDataStreams->GetStream(t).GetTotalMeasure();
     gtTotalCases += gpDataStreams->GetStream(t).GetTotalCases();
     gtTotalPopulation += gpDataStreams->GetStream(t).GetTotalPopulation();
  }
  SetMaxCircleSize();
  FreeRelativeRisksAdjustments();
}

/** Calculates expected number of cases for data stream. */
void CSaTScanData::CalculateMeasure(RealDataStream & thisStream) {
  try {
    SetAdditionalCaseArrays(thisStream);
    m_pModel->CalculateMeasure(thisStream);
    //record totals at start, the optional sequential scan feature modifies start values
    thisStream.SetTotalCasesAtStart(thisStream.GetTotalCases());
    thisStream.SetTotalControlsAtStart(thisStream.GetTotalControls());
    thisStream.SetTotalMeasureAtStart(thisStream.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CSaTScanData");
    throw;
  }
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
void CSaTScanData::FindNeighbors(bool bSimulations) {
  int           i, j;
  double        dMaxCircleSize;

  try {
    //if this iteration of call not simulations
    if (! bSimulations) {
      AllocateSortedArray();
      AllocateNeighborArray();
      //for real data, settings my indicate to report only smaller clusters
      dMaxCircleSize = (gParameters.GetRestrictingMaximumReportedGeoClusterSize() ? m_nMaxReportedCircleSize : m_nMaxCircleSize);
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
    if (gParameters.GetIsSequentialScanning())
        MakeNeighbors(&gTractHandler, &gCentroidsHandler, (gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0),
                      (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0), m_nTracts, m_nGridTracts,
                      (gvCircleMeasure.size() ? &gvCircleMeasure[0] : gpDataStreams->GetStream(0).GetMeasureArray()[0]),
                      dMaxCircleSize, gpDataStreams->GetStream(0).GetTotalMeasure(),
                      gpNeighborCountHandler->GetArray(), gParameters.GetDimensionsOfData(), gParameters.GetNumRequestedEllipses(),
                      gParameters.GetEllipseShapes(), gParameters.GetEllipseRotations(),
                      gParameters.GetMaxGeographicClusterSizeType(), &gPrint);
    else
        MakeNeighbors(&gTractHandler, &gCentroidsHandler, (gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0),
                      (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0), m_nTracts, m_nGridTracts,
                      (gvCircleMeasure.size() ? &gvCircleMeasure[0] : gpDataStreams->GetStream(0).GetMeasureArray()[0]),
                      dMaxCircleSize, dMaxCircleSize, gpNeighborCountHandler->GetArray(),
                      gParameters.GetDimensionsOfData(), gParameters.GetNumRequestedEllipses(),
                      gParameters.GetEllipseShapes(), gParameters.GetEllipseRotations(),
                      gParameters.GetMaxGeographicClusterSizeType(), &gPrint);

   }
   catch (ZdException & x) {
      x.AddCallpath("FindNeighbors()", "CSaTScanData");
      throw;
   }
}

double CSaTScanData::GetAnnualRateAtStart(unsigned int iStream) const {
  double nYears      = (double)(m_nEndDate+1 - m_nStartDate) / 365.2425;
  double dTotalCasesAtStart = gpDataStreams->GetStream(iStream).GetTotalCasesAtStart();
  double dTotalPopulation = gpDataStreams->GetStream(iStream).GetTotalPopulation();
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

/** For Bernoulli model, returns ratio of total cases / total population for
    iStream'th data stream. For all other models, returns 1.*/
double CSaTScanData::GetMeasureAdjustment(unsigned int iStream) const {
  if (gParameters.GetProbabiltyModelType() == BERNOULLI) {
    double dTotalCases = gpDataStreams->GetStream(iStream).GetTotalCases();
    double dTotalPopulation = gpDataStreams->GetStream(iStream).GetTotalPopulation();
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

/** internal class initializaton */
void CSaTScanData::Init() {
  m_pModel = 0;
  gpDataStreams = 0;
  gpNeighborCountHandler=0;
  gpSortedIntHandler=0;
  gpSortedUShortHandler=0;
  m_nAnnualRatePop = 100000;
  mdE_Angles = 0;
  mdE_Shapes = 0;
  m_nMaxReportedCircleSize = 0;
  m_nTotalMaxCirclePopulation = 0;
  gtTotalMeasure=0;
  gtTotalCases=0;
  gtTotalPopulation=0;
}

/** Randomizes collection of simulation data in concert with passed collection
    of randomizers. This method for creating randomized data is primarily
    utilized in conjunction with threaded simulations. */
/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CSaTScanData::RandomizeData(RandomizerContainer_t& RandomizerContainer,
                                 SimulationDataContainer_t& SimDataContainer,
                                 unsigned int iSimulationNumber) const {
  try {
    gpDataStreams->RandomizeData(RandomizerContainer, SimDataContainer, iSimulationNumber);
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeData()","CSaTScanData");
    throw;
  }
}

/** reads data from input files for a Bernoulli probability model */
bool CSaTScanData::ReadBernoulliData() {
  size_t        t;

  try {
    if (!ReadCoordinatesFile())
      return false;

    gpDataStreams = new BernoulliDataStreamHandler(*this, gPrint);
    if (!gpDataStreams->ReadData())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    delete gpDataStreams; gpDataStreams=0;
    x.AddCallpath("ReadBernoulliData()","CSaTScanData");
    throw;
  }
  return true;
}

/** First calls internal methods to prepare internal structure:
    - calculate time interval start times
    - calculate a clusters maximum temporal window length in terms of time intervals
    - calculate indexes of flexible scanning window, if requested
    - calculate start interval index of start date of prospective analyses
    - read input data from files base upon probability model
    Throws ResolvableException if read fails. */
void CSaTScanData::ReadDataFromFiles() {
  bool  bReadSuccess;

  try {
    SetIntervalStartTimes();
    SetIntervalCut();
    SetTimeIntervalRangeIndexes();
    if (gParameters.GetIsProspectiveAnalysis())
      SetProspectiveIntervalStart();
    switch (gParameters.GetProbabiltyModelType()) {
      case POISSON              : bReadSuccess = ReadPoissonData(); break;
      case BERNOULLI            : bReadSuccess = ReadBernoulliData(); break;
      case SPACETIMEPERMUTATION : bReadSuccess = ReadSpaceTimePermutationData(); break;
      case NORMAL               : bReadSuccess = ReadNormalData(); break;
      case SURVIVAL             : bReadSuccess = ReadSurvivalData(); break;
      case RANK                 : bReadSuccess = ReadRankData(); break;
      default :
        ZdGenerateException("Unknown probability model type '%d'.","ReadDataFromFiles()", gParameters.GetProbabiltyModelType());
    };
    if (!bReadSuccess)
      GenerateResolvableException("\nProblem encountered when reading the data from the input files.", "ReadDataFromFiles");
    //now that all data has been read, the tract handler can combine references locations with duplicate coordinates
    gTractHandler.tiConcaticateDuplicateTractIdentifiers();
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadDataFromFiles()", "CSaTScanData");
    throw;
  }
}

/** reads data from input files for a normal probability model */
bool CSaTScanData::ReadNormalData() {
  size_t        t;

  try {
    if (!ReadCoordinatesFile())
      return false;
    gpDataStreams = new NormalDataStreamHandler(*this, gPrint);
    if (!gpDataStreams->ReadData())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    delete gpDataStreams; gpDataStreams=0;
    x.AddCallpath("ReadNormalData()","CSaTScanData");
    throw;
  }
  return true;
}

/** reads data from input files for a Poisson probability model */
bool CSaTScanData::ReadPoissonData() {
  size_t        t;

  try {
    if (!ReadCoordinatesFile())
      return false;

    gpDataStreams = new PoissonDataStreamHandler(*this, gPrint);
    if (!gpDataStreams->ReadData())
      return false;
    if (gParameters.UseAdjustmentForRelativeRisksFile() && !ReadAdjustmentsByRelativeRisksFile())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    delete gpDataStreams; gpDataStreams=0;
    x.AddCallpath("ReadPoissonData()","CSaTScanData");
    throw;
  }
  return true;
}

/** reads data from input files for a Rank probability model */
bool CSaTScanData::ReadRankData() {
  size_t        t;

  try {
    if (!ReadCoordinatesFile())
      return false;
    gpDataStreams = new RankDataStreamHandler(*this, gPrint);
    if (!gpDataStreams->ReadData())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    delete gpDataStreams; gpDataStreams=0;
    x.AddCallpath("ReadRankData()","CSaTScanData");
    throw;
  }
  return true;
}

/** reads data from input files for a space-time permutation probability model */
bool CSaTScanData::ReadSpaceTimePermutationData() {
  size_t        t;

  try {
    if (!ReadCoordinatesFile())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
       return false;
    gpDataStreams = new SpaceTimePermutationDataStreamHandler(*this, gPrint);
    if (!gpDataStreams->ReadData())
      return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    delete gpDataStreams; gpDataStreams=0;
    x.AddCallpath("ReadSpaceTimePermutationData()","CSaTScanData");
    throw;
  }
  return true;
}

/** reads data from input files for a Survival probability model */
bool CSaTScanData::ReadSurvivalData() {
  size_t        t;

  try {
    if (!ReadCoordinatesFile())
      return false;
    gpDataStreams = new SurvivalDataStreamHandler(*this, gPrint);
    if (!gpDataStreams->ReadData())
      return false;
    if (gParameters.UseMaxCirclePopulationFile() && !ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    delete gpDataStreams; gpDataStreams=0;
    x.AddCallpath("ReadRankData()","CSaTScanData");
    throw;
  }
  return true;
}

/** For tract at tTractIndex - zeros case, control, and population data from data
    stream structures. This function is utilized by the optional sequential scan
    feature and is designed only for purely spatial analyses at this time.*/
void CSaTScanData::RemoveTractSignificance(tract_t tTractIndex) {
  count_t       tTotalCases, tTotalControls;
  measure_t     tTotalMeasure;

  try {
    for (size_t t=0; t < gpDataStreams->GetNumStreams(); ++t) {
       RealDataStream & thisStream = gpDataStreams->GetStream(t);
       tTotalCases = thisStream.GetTotalCases();
       tTotalCases -= thisStream.GetCaseArray()[0][tTractIndex];
       thisStream.GetCaseArray()[0][tTractIndex] = 0;
       thisStream.SetTotalCases(tTotalCases);
       tTotalMeasure = thisStream.GetTotalMeasure();
       tTotalMeasure -= thisStream.GetMeasureArray()[0][tTractIndex];
       thisStream.GetMeasureArray()[0][tTractIndex] = 0;
       thisStream.SetTotalMeasure(tTotalMeasure);
       if (gParameters.GetProbabiltyModelType() == BERNOULLI) {
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
void CSaTScanData::SetAdditionalCaseArrays(RealDataStream & thisStream) {
  try {
    if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION ||
        gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC)
      thisStream.SetCasesByTimeInterval();  
  }
  catch (ZdException &x) {
    x.AddCallpath("SetAdditionalCaseArrays()","CSaTScanData");
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
      if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
        lStudyPeriodLength = TimeBetween(m_nStartDate, m_nEndDate, gParameters.GetTimeIntervalUnitsType());
        lMaxTemporalLength = static_cast<long>(floor(lStudyPeriodLength * gParameters.GetMaximumTemporalClusterSize()/100.0));
        m_nIntervalCut = static_cast<int>(floor(lMaxTemporalLength / gParameters.GetTimeIntervalLength()));
      }
      else
        m_nIntervalCut = static_cast<int>(gParameters.GetMaximumTemporalClusterSize() / gParameters.GetTimeIntervalLength());
    }

    if (m_nIntervalCut==0) {
      switch (gParameters.GetTimeIntervalUnitsType()) {
          case YEAR  : sTimeIntervalType = "year"; break;
          case MONTH : sTimeIntervalType = "month"; break;
          case DAY   : sTimeIntervalType = "day"; break;
          // if we get here, there is an error somewhere else
          default: sTimeIntervalType = "none"; break;
        };

        if (gParameters.GetMaximumTemporalClusterSizeType() == TIMETYPE) {
          sIntervalCutMessage << "Error: A maximum temporal cluster size of %g %s%s is less than one %d %s time interval.\n";
          sIntervalCutMessage << "       No clusters can be found.\n";
          GenerateResolvableException(sIntervalCutMessage.GetCString(), "SetIntervalCut()",
                                      gParameters.GetMaximumTemporalClusterSize(), sTimeIntervalType.GetCString(),
                                      (gParameters.GetMaximumTemporalClusterSize() == 1 ? "" : "s"),
                                      gParameters.GetTimeIntervalLength(), sTimeIntervalType.GetCString());
        }
        else if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
          sIntervalCutMessage << "Error: A maximum temporal cluster size that is %g percent of a %d %s study period\n";
          sIntervalCutMessage << "       equates to %d %s%s, which is less than one %d %s time interval.\n";
          sIntervalCutMessage << "       No clusters can be found.\n";
          GenerateResolvableException(sIntervalCutMessage.GetCString(), "SetIntervalCut()",
                                      gParameters.GetMaximumTemporalClusterSize(),
                                      lStudyPeriodLength, sTimeIntervalType.GetCString(),
                                      lMaxTemporalLength, sTimeIntervalType.GetCString(), (lMaxTemporalLength == 1 ? "" : "s"),
                                      gParameters.GetTimeIntervalLength(), sTimeIntervalType.GetCString());
        }
      }
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
  Julian IntervalStartingDate = m_nEndDate+1;

  gvTimeIntervalStartTimes.clear();
  //latest interval start time is the day after study period end date
  gvTimeIntervalStartTimes.push_back(IntervalStartingDate);
  //find the next prior interval start time from previous, given length of time intervals
  IntervalStartingDate = DecrementDate(IntervalStartingDate, gParameters.GetTimeIntervalUnitsType(), gParameters.GetTimeIntervalLength());
  //continue decrementing from current interval start time until study period start date boundry hit 
  while (IntervalStartingDate > m_nStartDate) {
      //push interval start time onto vector
      gvTimeIntervalStartTimes.push_back(IntervalStartingDate);
      //find the next prior interval start time from current, given length of time intervals
      IntervalStartingDate = DecrementDate(IntervalStartingDate, gParameters.GetTimeIntervalUnitsType(), gParameters.GetTimeIntervalLength());
  }
  //push study period start date onto vector
  gvTimeIntervalStartTimes.push_back(m_nStartDate);
  //reverse elements of vector so that elements are ordered: study period start --> 'study period end + 1'
  std::reverse(gvTimeIntervalStartTimes.begin(), gvTimeIntervalStartTimes.end());
  //record number of time intervals, not including 'study period end date + 1' date
  m_nTimeIntervals = (int)gvTimeIntervalStartTimes.size() - 1;

  if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION && m_nTimeIntervals <= 1)
    GenerateResolvableException("Error: The time stratified randomization adjustment requires more than\n"
                                "       one time interval.\n", "SetIntervalStartTimes()");
}

/** Causes maximum circle size to be set based on parameters settings. */
void CSaTScanData::SetMaxCircleSize() {
  //NOTE: The measure from the first data stream is used when calculating
  //      the maximum circle size, at least for the time being. 
  measure_t tTotalMeasure = gpDataStreams->GetStream(0).GetTotalMeasure();

  try {
    switch (gParameters.GetMaxGeographicClusterSizeType()) {
      case PERCENTOFPOPULATIONFILETYPE :
           m_nMaxCircleSize = (gParameters.GetMaximumGeographicClusterSize() / 100.0) * m_nTotalMaxCirclePopulation;
           if (gParameters.GetRestrictingMaximumReportedGeoClusterSize())
             m_nMaxReportedCircleSize = (gParameters.GetMaximumReportedGeoClusterSize() / 100.0) * m_nTotalMaxCirclePopulation;
           break;
      case PERCENTOFPOPULATIONTYPE :
           m_nMaxCircleSize = (gParameters.GetMaximumGeographicClusterSize() / 100.0) * tTotalMeasure;
           if (gParameters.GetRestrictingMaximumReportedGeoClusterSize())
             m_nMaxReportedCircleSize = (gParameters.GetMaximumReportedGeoClusterSize() / 100.0) * tTotalMeasure;
           break;
      case DISTANCETYPE :
           m_nMaxCircleSize = gParameters.GetMaximumGeographicClusterSize();
           if (gParameters.GetRestrictingMaximumReportedGeoClusterSize())
             m_nMaxReportedCircleSize = gParameters.GetMaximumReportedGeoClusterSize();
           break;
      default : ZdException::Generate("Unknown maximum spatial cluster type: '%i'.", "SetMaxCircleSize()",
                                      gParameters.GetMaxGeographicClusterSizeType());
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMaxCircleSize()","CSaTScanData");
    throw;
  }
}

/* Calculates which time interval the prospectice space-time start date is in.*/
/* MAKE SURE THIS IS EXECUTED AFTER THE  m_nTimeIntervals VARIABLE HAS BEEN SET */
void CSaTScanData::SetProspectiveIntervalStart() {
  long    lTime;
  Julian  lProspStartDate;

  try {
    lProspStartDate = gParameters.GetProspectiveStartDateAsJulian();
    lTime = TimeBetween(lProspStartDate, m_nEndDate, gParameters.GetTimeIntervalUnitsType());
    m_nProspectiveIntervalStart = m_nTimeIntervals - (int)ceil((float)lTime/(float)gParameters.GetTimeIntervalLength()) + 1;

    if (m_nProspectiveIntervalStart < 0)
      GenerateResolvableException("Error: : The start date for prospective analyses '%s' is prior to the study period start date '%s'.\n",
                                  "SetProspectiveIntervalStart()", gParameters.GetProspectiveStartDate().c_str(),
                                  gParameters.GetStudyPeriodStartDate().c_str());
    if (m_nProspectiveIntervalStart > m_nTimeIntervals)
      GenerateResolvableException("Error: The start date for prospective analyses '%s' occurs after the study period end date '%s'.\n",
                                  "SetProspectiveIntervalStart", gParameters.GetProspectiveStartDate().c_str(),
                                  gParameters.GetStudyPeriodEndDate().c_str());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProspectiveIntervalStart()","CSaTScanData");
    throw;
  }
}

/** For all data streams, causes temporal structures to be allocated and set. */
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

/** Sets time interval end range index into interval start times array. */
void CSaTScanData::SetScanningWindowEndRangeIndex(Julian EndRangeDate, int& iEndRangeDateIndex) {
  int   i;

  //find index for end range end date, the interval where date fits, rounding to higher index
  iEndRangeDateIndex = -1;
  for (i=m_nTimeIntervals; i > 0  && iEndRangeDateIndex == -1; --i)
     if (EndRangeDate <= gvTimeIntervalStartTimes[i] - 1 && EndRangeDate > gvTimeIntervalStartTimes[i - 1] - 1)
        iEndRangeDateIndex = i;
}

/** Sets time interval start range index into interval start times array. */
void CSaTScanData::SetScanningWindowStartRangeIndex(Julian StartRangeDate, int& iStartRangeDateIndex) {
  int   i;

  iStartRangeDateIndex = -1;
  for (i=0; i < m_nTimeIntervals && iStartRangeDateIndex == -1; ++i)
     if (StartRangeDate >= gvTimeIntervalStartTimes[i] && StartRangeDate < gvTimeIntervalStartTimes[i + 1])
       iStartRangeDateIndex = i;
}

/** Sets indexes of time interval ranges into interval start time array. */
void CSaTScanData::SetTimeIntervalRangeIndexes() {
  ZdString      sTimeIntervalType, sMessage;
  char          sDateWST[50], sDateMaxWET[50]; 
  int           iMaxEndWindow, iWindowStart;

  if (gParameters.GetIncludeClustersType() == CLUSTERSINRANGE) {
    //find start range date indexes
    SetScanningWindowStartRangeIndex(gParameters.GetStartRangeDateAsJulian(gParameters.GetStartRangeStartDate()), m_nStartRangeStartDateIndex);
    SetScanningWindowStartRangeIndex(gParameters.GetStartRangeDateAsJulian(gParameters.GetStartRangeEndDate()), m_nStartRangeEndDateIndex);
    //find end range date indexes
    SetScanningWindowEndRangeIndex(gParameters.GetEndRangeDateAsJulian(gParameters.GetEndRangeStartDate()), m_nEndRangeStartDateIndex);
    SetScanningWindowEndRangeIndex(gParameters.GetEndRangeDateAsJulian(gParameters.GetEndRangeEndDate()), m_nEndRangeEndDateIndex);
    //validate windows will be evaluated
    //check that there will be clusters evaluated...
    iMaxEndWindow = std::min(m_nEndRangeEndDateIndex, m_nStartRangeEndDateIndex + m_nIntervalCut);
    iWindowStart = std::max(m_nEndRangeStartDateIndex - m_nIntervalCut, m_nStartRangeStartDateIndex);
    if (iWindowStart >= iMaxEndWindow) {
      sTimeIntervalType = gParameters.GetDatePrecisionAsString(gParameters.GetTimeIntervalUnitsType());
      sTimeIntervalType.ToLowercase();
      JulianToChar(sDateWST, gvTimeIntervalStartTimes[iWindowStart]);
      JulianToChar(sDateMaxWET, gvTimeIntervalStartTimes[iMaxEndWindow] - 1);
      GenerateResolvableException("Error: No clusters will be evaluated.\n"
                                  "       With the incorporation of a maximum temporal cluster size of %i %s,\n"
                                  "       the temporal window scanned has a start time of %s (end range\n"
                                  "       ending time minus %i %s) and a maximum window end time of %s\n"
                                  "       (start range ending time plus %i %s), which results in no windows scanned.",
                                  "Setup()", m_nIntervalCut * gParameters.GetTimeIntervalLength(),
                                  sTimeIntervalType.GetCString(), sDateWST,
                                  m_nIntervalCut * gParameters.GetTimeIntervalLength(),
                                  sTimeIntervalType.GetCString(), sDateMaxWET,
                                  m_nIntervalCut * gParameters.GetTimeIntervalLength(), sTimeIntervalType.GetCString());
    }
    //The parameter validation checked already whether the end range dates conflicted,
    //but the maxium temporal cluster size may actually cause the range dates to be
    //different than the user defined.
    if (m_nEndRangeStartDateIndex > iMaxEndWindow) {
      sTimeIntervalType = gParameters.GetDatePrecisionAsString(gParameters.GetTimeIntervalUnitsType());
      sTimeIntervalType.ToLowercase();
      JulianToChar(sDateMaxWET, gvTimeIntervalStartTimes[iMaxEndWindow] - 1);
      GenerateResolvableException("Error: No clusters will be evaluated.\n"
                                  "       With the incorporation of a maximum temporal cluster size of %i %s\n"
                                  "       the maximum window end time becomes %s (start range ending\n"
                                  "       time plus %i %s), which does not intersect with scanning window end range.\n","Setup()",
                                  m_nIntervalCut * gParameters.GetTimeIntervalLength(), sTimeIntervalType.GetCString(),
                                  sDateMaxWET, m_nIntervalCut * gParameters.GetTimeIntervalLength(), sTimeIntervalType.GetCString());
    }
  }
}

/** internal setup function */
void CSaTScanData::Setup() {
  long  lCurrentEllipse=0;

  try {
    //For now, compute the angle and store the angle and shape
    //for each ellipsoid.  Maybe transfer info to a different location in the
    //application or compute "on the fly" prior to printing.
    if (gParameters.GetNumTotalEllipses() > 0) {
      mdE_Angles = new double[gParameters.GetNumTotalEllipses()];
      mdE_Shapes = new double[gParameters.GetNumTotalEllipses()];
      for (int es = 0; es < gParameters.GetNumRequestedEllipses(); ++es) {
         for (int ea = 0; ea < gParameters.GetEllipseRotations()[es]; ++ea) {
            mdE_Angles[lCurrentEllipse]=PI*ea/gParameters.GetEllipseRotations()[es];
            mdE_Shapes[lCurrentEllipse]= gParameters.GetEllipseShapes()[es];
            ++lCurrentEllipse;
         }
      }
    }
  }
  catch (ZdException &x) {
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
            GenerateResolvableException("Error: For locationID '%s' in time interval %s - %s,\n"
                                        "       the expected number of cases is zero but there were cases observed.\n"
                                        "       Please review the correctness of population and case files.",
                                        "ValidateObservedToExpectedCases()",
                                        gTractHandler.tiGetTid(t, sId),
                                        JulianToString(sStart, gvTimeIntervalStartTimes[i]).GetCString(),
                                        JulianToString(sEnd, gvTimeIntervalStartTimes[i + 1] - 1).GetCString());
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateObservedToExpectedCases()","CSaTScanData");
    throw;
  }
}
